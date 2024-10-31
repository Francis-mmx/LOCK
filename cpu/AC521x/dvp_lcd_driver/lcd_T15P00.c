#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_OTA5182A

extern void delay_2ms(int cnt);

//CS  PC5->PV5
#define TFT_CS_OUT()    PORTB_DIR &= ~BIT(5);PORTB_HD |= BIT(5)
#define TFT_CS_EN()     PORTB_OUT &= ~BIT(5)
#define TFT_CS_DIS()    PORTB_OUT |= BIT(5)
//SDA PC6->PV6
#define TFT_SDA_OUT()   PORTB_DIR &= ~BIT(6);PORTB_HD |= BIT(6)
#define TFT_SDA_IN()    PORTB_DIR |=  BIT(6)
#define TFT_SDA_H()     PORTB_OUT |=  BIT(6)
#define TFT_SDA_L()     PORTB_OUT &= ~BIT(6)
#define TFT_SDA_R()     PORTB_IN&BIT(6)
//SCL PC7->PV7
#define TFT_SCL_OUT()   PORTB_DIR &= ~BIT(7);PORTB_HD |= BIT(7)
#define TFT_SCL_H()     PORTB_OUT |=  BIT(7)
#define TFT_SCL_L()     PORTB_OUT &= ~BIT(7)

//STB PH4->PH2
#define TFT_STB_OUT()   PORTD_DIR &= ~BIT(12)
#define TFT_STB_H()     PORTD_OUT |=  BIT(12)
#define TFT_STB_L()     PORTD_OUT &= ~BIT(12)

//RESET PH5->PC3
#define TFT_RESET_H()   PORTD_DIR &= ~BIT(13);PORTD_OUT |= BIT(13)
#define TFT_RESET_L()   PORTD_DIR &= ~BIT(13);PORTD_OUT &=~BIT(13)


//RGB 8BIT + dummy
#define RGB_320x240_NTSC    0
#define RGB_320x288_PAL     1
#define RGB_360x240_NTSC    2
#define RGB_360x288_PAL     3
#define RAWSTR_MODE         4
#define RAWDLT_MODE         5
#define CCIR_MODE           6

#define FORMAT              RAWSTR_MODE      //RGB_MODE有效


static void delay_50ns(u16 cnt)//380ns
{
    while (cnt--) {
        delay(/*8*/800);//50ns
    }
}

static void wr_reg(u8 reg, u16 dat)
{
    char i;
    u8 addr = 0;

    TFT_CS_EN();
    TFT_SDA_OUT();
    TFT_SCL_OUT();
    delay_50ns(1);

    addr = (reg << 1); //write addr

    for (i = 4; i >= 0; i--) {
        TFT_SCL_L();
        delay_50ns(1);
        //addr <<= 1;

        if (addr & BIT(i)) {
            TFT_SDA_H();
        } else {
            TFT_SDA_L();
        }

        TFT_SCL_H();
        delay_50ns(1);
    }

    //dat <<= 5;

    for (i = 10; i >= 0; i--) {
        TFT_SCL_L();
        delay_50ns(1);

        if (dat & BIT(i)) {
            TFT_SDA_H();
        } else {
            TFT_SDA_L();
        }
        //dat <<= 1;
        //delay_50ns(1);

        TFT_SCL_H();
        delay_50ns(1);
    }
    TFT_CS_DIS();
    delay_50ns(8);

}


static u16 rd_reg(u8 reg)
{
    char i;
    u8 addr;
    u16 dat = 0;
    TFT_CS_EN();
    TFT_SDA_OUT();
    delay_50ns(1);

    addr = (reg << 4) | BIT(3); //read addr

    for (i = 4; i >= 0; i--) {
        TFT_SCL_L();
        delay_50ns(1);

        if (addr & BIT(7)) {
            TFT_SDA_H();
        } else {
            TFT_SDA_L();
        }
        addr <<= 1;

        TFT_SCL_H();
        delay_50ns(1);
    }

    TFT_SDA_IN();
    for (i = 10; i >= 0; i--) {
        TFT_SCL_L();
        delay_50ns(1);

        TFT_SCL_H();
        delay_50ns(1);

        dat <<= 1;
        if (TFT_SDA_R()) {
            dat |= BIT(0);
        }

        delay_50ns(1);
    }
    TFT_CS_DIS();
    delay_50ns(8);
    return dat;
}

static void ota5182a_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u16 dat[16];
    u8 i;
    printf("\nT15P00 init...");

    TFT_CS_OUT();
    TFT_SDA_OUT();
    TFT_SCL_OUT();
    TFT_STB_OUT();

    TFT_CS_DIS();
    TFT_SDA_H();
    TFT_SCL_H();
    TFT_STB_L();

    TFT_RESET_H();
    delay_2ms(50);
    TFT_RESET_L();
    delay_2ms(50);
    TFT_RESET_H();

    wr_reg(0x00, 0x0f);
    wr_reg(0x00, 0x05);
    delay_2ms(50);

    wr_reg(0x00, 0x0f);
    wr_reg(0x00, 0x05);
    delay_2ms(50);
    wr_reg(0x00, 0x0f);
    //wr_reg(0x0f,0x546);
    //wr_reg(0x09,0x60);
    wr_reg(0x05, 0x00);
    delay_2ms(50);
    wr_reg(0x03, 0x08);
    wr_reg(0x07, 0x40);

    wr_reg(0x04, 0x00); //select scan direction

#if (FORMAT == RAWSTR_MODE)||(FORMAT == RAWDLT_MODE)
#if (FORMAT == RAWSTR_MODE)
    wr_reg(0x04, 0x03); //select scan direction
#else
    wr_reg(0x04, 0x00); //select scan direction
#endif
    wr_reg(0x06, 0x00);
#elif (FORMAT == RGB_320x288_PAL)
    wr_reg(0x06, 0x19);
    wr_reg(0x08, 0x04);
    wr_reg(0x0a, 0x04);
#elif (FORMAT == RGB_360x288_PAL)
    wr_reg(0x06, 0x1a);
    wr_reg(0x08, 0x04);
    wr_reg(0x0a, 0x04);
#elif (FORMAT == RGB_320x240_NTSC)
    wr_reg(0x06, 0x01);
    wr_reg(0x08,/*0x04*/0x00);
    wr_reg(0x0a,/*0x04*/0x04);
#elif (FORMAT == RGB_360x240_NTSC)
    wr_reg(0x06, 0x02);
    wr_reg(0x08, 0x04);
    wr_reg(0x0a, 0x04);
#elif (FORMAT == CCIR_MODE)
    wr_reg(0x0f, 0x546);
    wr_reg(0x09, 0x60);

    wr_reg(0x0c, 0x05);
    wr_reg(0x0e, 0x11);
    //---CCIR656---
    wr_reg(0x06, 0x07);
    wr_reg(0x08, 0x01);
    wr_reg(0x0a, 0x20);
    //-------------
    wr_reg(0x07, 0x40);
#endif
    /* wr_reg(0x08,0x3); */
    /* wr_reg(0x07,0x20); */

    /* wr_reg(0x05,0x20); */
    /* wr_reg(0x02,0x3); */


    for (i = 0; i < 16; i++) {
        printf("\nreg %02d: %02x", i, rd_reg(i));
    }
}

static void lcd_t15p00_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_IMD_DEVICE(lcd_t15p00_dev) = {
    .info = {
#if (FORMAT == RAWSTR_MODE)||(FORMAT == RAWDLT_MODE)
        .format          = FORMAT_RGB888,
#elif (FORMAT == RGB_320x240_NTSC)||(FORMAT == RGB_320x288_PAL)||(FORMAT == RGB_360x240_NTSC)||(FORMAT == RGB_360x288_PAL)
        .format          = FORMAT_RGB888,
#elif (FORMAT == CCIR_MODE)
        .format          = FORMAT_ITU656,
#endif
        .test_mode 	     = false,
        .test_mode_color = 0x0000FF,
        .background_color = 0x0000FF,
        .len 			 = LEN_256,

        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .target_xres     = LCD_DEV_WIDTH,
        .target_yres     = LCD_DEV_HIGHT,
        .rotate          = ROTATE_0,
        .interlaced_mode = INTERLACED_NONE,

        .adjust = {
            .y_gain = 0x100,
            .u_gain = 0x100,
            .v_gain = 0x100,
            .r_gain = 0x80,
            .g_gain = 0x80,
            .b_gain = 0x80,
            .r_coe0 = 0x80,
            .g_coe1 = 0x80,
            .b_coe2 = 0x80,
            .r_gma  = 100,
            .g_gma  = 100,
            .b_gma  = 100,
        },
    },

#if (FORMAT == RAWSTR_MODE)||(FORMAT == RAWDLT_MODE)
    /* .image_width     = 480, */
    .ncycle          = CYCLE_THREE,

    .dclk_cfg        = CLK_EN  | CLK_NEGATIVE,
    .sync0_cfg       = SIGNAL_TYPE_DEN  | CLK_EN | CLK_NEGATIVE,
    .sync1_cfg       = SIGNAL_TYPE_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_cfg       = SIGNAL_TYPE_VSYNC | CLK_EN | CLK_NEGATIVE,
    .clk_cfg    = PLL2_CLK | DIVA_3 | DIVB_2,

#if (FORMAT == RAWSTR_MODE)
    //R G B
    .raw_mode        = RAW_STR_MODE,
    .raw_odd_phase   = PHASE_G,
    .raw_even_phase  = PHASE_R,
#else
    .raw_mode        = RAW_DLT_MODE,
    .raw_odd_phase   = PHASE_B,
    .raw_even_phase  = PHASE_R,
#endif


#elif (FORMAT == RGB_320x240_NTSC)||(FORMAT == RGB_320x288_PAL)||(FORMAT == RGB_360x240_NTSC)||(FORMAT == RGB_360x288_PAL)
    .ncycle          = CYCLE_FOUR,
    .dclk_cfg        = CLK_EN | CLK_NEGATIVE,
    .sync0_cfg       = SIGNAL_TYPE_DEN  |/*CLK_EN|*/CLK_NEGATIVE,
    .sync1_cfg       = SIGNAL_TYPE_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_cfg       = SIGNAL_TYPE_VSYNC | CLK_EN | CLK_NEGATIVE,
    .interlaced_mode = INTERLACED_NONE,
    .raw_odd_phase   = PHASE_R,
    .raw_even_phase  = PHASE_R,
    .clk_cfg    = PLL2_CLK | DIVA_3 | DIVB_2, //DIVA无效，默认为1
#elif (FORMAT == CCIR_MODE)
    .ncycle          = CYCLE_TWO,
    .dclk_cfg        = CLK_EN  /*| CLK_NEGATIVE*/,
    .sync0_cfg       = SIGNAL_TYPE_DEN /* |CLK_EN|CLK_NEGATIVE*/,
    .sync1_cfg       = SIGNAL_TYPE_HSYNC | CLK_EN | CLK_NEGATIVE,
    .sync2_cfg       = SIGNAL_TYPE_VSYNC | CLK_EN | CLK_NEGATIVE,
    .interlaced_mode = INTERLACED_ALL,
    .clk_cfg         = PLL2_CLK | DIVA_3 | DIVB_2,
#endif
    .drive_mode      = MODE_RGB_DE_SYNC,
    .data_width      = PORT_8BITS,
    .timing = {
#if (FORMAT == CCIR_MODE)
        .hori_total 		    = 1716,
        .hori_sync 		        = 268,
        .hori_back_porth    	= 272,
        .hori_pixel 	        = 720,

        .vert_total 		    = 262,
        .vert_sync 		        = 42,
        .vert_back_porth_odd 	= 20,
        .vert_back_porth_even 	= 20,
        .vert_pixel 	        = 240,
#elif (FORMAT == RGB_320x240_NTSC)
        .hori_total 		    = 1560,
        .hori_sync 		        = 1,
        .hori_back_porth 	    = 252,
        .hori_pixel 	        = 320,

        .vert_total 		    = 262 + 2,
        .vert_sync 		        = 1,
        .vert_back_porth_odd 	= 16,
        .vert_back_porth_even 	= 16,
        .vert_pixel 	        = 240
#elif (FORMAT == RGB_320x288_PAL)
        .hori_total 		    = 1560,
        .hori_sync 		        = 1,
        .hori_back_porth 	    = 252,
        .hori_pixel 	        = 320,

        .vert_total 		    = 312,
        .vert_sync 		        = 1,
        .vert_back_porth_odd 	= 18,
        .vert_back_porth_even 	= 19,
        .vert_pixel 	        = 288
#elif (FORMAT == RGB_360x240_NTSC)
        .hori_total 		    = 1716,
        .hori_sync 		        = 1,
        .hori_back_porth 	    = 252,
        .hori_pixel 	        = 360,

        .vert_total 	    	= 262,
        .vert_sync 		        = 1,
        .vert_back_porth_odd 	= 16,
        .vert_back_porth_even 	= 16,
        .vert_pixel 	        = 240,
#elif (FORMAT == RGB_360x288_PAL)
        .hori_total 		    = 1716,
        .hori_sync 		        = 1,
        .hori_back_porth 	    = 252,
        .hori_pixel 	        = 360,

        .vert_total 		    = 312 + 2,
        .vert_sync 		        = 1,
        .vert_back_porth_odd 	= 16,
        .vert_back_porth_even 	= 16,
        .vert_pixel 	        = 288,
#elif (FORMAT == RAWSTR_MODE)||(FORMAT == RAWDLT_MODE)
        .hori_total 		    = 617,
        .hori_sync 		        = 1,
        .hori_back_porth 	    = 100,
        .hori_pixel 	        = 480 / 3,

        .vert_total 		    = 262,
        .vert_sync 		        = 1,
        .vert_back_porth_odd 	= 13,
        .vert_back_porth_even 	= 14,
        .vert_pixel 	        = 240,
#endif
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type    = LCD_DVP_RGB,
    .init 	 = ota5182a_init,
    .dev  	 = &lcd_t15p00_dev,
    .bl_ctrl = lcd_t15p00_backctrl,
};
#endif

