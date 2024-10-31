#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "asm/pap.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_SSD1963

#define SSD1963_PORT_8BITS
/* #define SSD1963_PORT_16BITS */

#define  HDP  799	//Horizontal Display Period
#define  HT   1000	//Horizontal Total
#define  HPS  51	//LLINE Pulse Start Position
#define  LPS  3		//Horizontal Display Period Start Position
#define  HPW  8		//LLINE Pulse Width


#define  VDP  479	//Vertical Display Period
#define  VT   530	//Vertical Total
#define  VPS  24	//LFRAME Pulse Start Position
#define  FPS  23	//Vertical Display Period Start Positio
#define  VPW  3 	//LFRAME Pulse Width

#define SSD1963_RST_H()		PORTH_DIR &=~BIT(0);PORTH_OUT |= BIT(0)
#define SSD1963_RST_L()		PORTH_DIR &=~BIT(0);PORTH_OUT &=~BIT(0)
#define SSD1963_CS_EN()		PORTH_DIR &=~BIT(1);PORTH_OUT &=~BIT(1)
#define SSD1963_CS_DIS()	PORTH_DIR &=~BIT(1);PORTH_OUT |= BIT(1)
#define SSD1963_RS_H()		PORTH_DIR &=~BIT(5);PORTH_OUT |= BIT(5)
#define SSD1963_RS_L()		PORTH_DIR &=~BIT(5);PORTH_OUT &=~BIT(5)



extern void delay_2ms(int cnt);


static void *pap = NULL;

void LCD_WR_REG(u16 reg)
{
    SSD1963_CS_EN();
    SSD1963_RS_L();
    dev_write(pap, &reg, sizeof(reg));
    SSD1963_CS_DIS();
}

void LCD_WR_DAT(u16 dat)
{
    SSD1963_CS_EN();
    SSD1963_RS_H();
    dev_write(pap, &dat, sizeof(dat));
    SSD1963_CS_DIS();
}

void LCD_Clear(u16 color)
{
    u16 w, h;

    puts("lcd clear in...\n");

    /*--------设置刷屏窗口--------*/
    /*  X轴  */
    LCD_WR_REG(0X002A);
    LCD_WR_DAT(0);
    LCD_WR_DAT(0);
    LCD_WR_DAT(HDP >> 8);
    LCD_WR_DAT(HDP & 0XFF);

    /*  Y轴  */
    LCD_WR_REG(0X002B);
    LCD_WR_DAT(0);
    LCD_WR_DAT(0);
    LCD_WR_DAT(VDP >> 8);
    LCD_WR_DAT(VDP & 0X00FF);

    LCD_WR_REG(0X002C);

#if 0
    for (h = 0; h < 480; h++) {
        for (w = 0; w < 800; w++) {
            LCD_WR_DAT(color);
        }
    }
#else
    SSD1963_CS_EN();
    SSD1963_RS_H();
    static u8 pap_buf[800 * 2];
    for (w = 0; w < 800; w++) {
        pap_buf[2 * w] = color >> 8;
        pap_buf[2 * w + 1] = color;
    }
    for (h = 0; h < 480; h++) {
        dev_write(pap, pap_buf, 800 * 2);
    }
    SSD1963_CS_DIS();
#endif

    puts("lcd clear out...\n");
}


static int lcd_ssd1963_init(void *_data)
{
    puts("lcd ssd1963 init...\n");

    struct pap_info pap_info_t;
#ifdef SSD1963_PORT_8BITS
    pap_info_t.datawidth 	= PAP_PORT_8BITS;
    pap_info_t.endian    	= PAP_BE;//8bit必须为大端
    pap_info_t.cycle     	= PAP_CYCLE_ONE;
#endif
#ifdef SSD1963_PORT_16BITS
    pap_info_t.datawidth 	= PAP_PORT_16BITS;
    pap_info_t.endian    	= PAP_LE;//8bit必须为大端
    pap_info_t.cycle     	= PAP_CYCLE_ONE;
#endif
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open("pap", &pap_info_t);
    ASSERT(pap, "open pap device fail");

    /*lcd reset*/
    SSD1963_RST_H();
    delay_2ms(50);
    SSD1963_RST_L();
    delay_2ms(50);
    SSD1963_RST_H();

    LCD_WR_REG(0x00E2);		/* 命令E2H Set PLL MN*/
    LCD_WR_DAT(0x0023);		/* 参数1 设置VCO频率 SSD1963外部晶振频率为10MHz，VCO要求大于250MHz 36倍频，最后得360MHz*/
    LCD_WR_DAT(0x0002);		/* 参数2 设置PLL频率 M=2, PLL=360/(M+1)=120MHz*/
    LCD_WR_DAT(0x0004);		/* 参数3 使设置生效*/
    LCD_WR_REG(0x00E0);  	/* Set PLL */
    LCD_WR_DAT(0x0001);   	/* PLL enable  */
    delay_2ms(50);	       	/* 延时100ms   */
    LCD_WR_REG(0x00E0);
    LCD_WR_DAT(0x0003);
    delay_2ms(50);
    LCD_WR_REG(0x0001);   	/*  软件复位命令，没有参数 */
    delay_2ms(50);
    LCD_WR_REG(0x00E6);	  	/*  Set LSHIFT Frequency */
    LCD_WR_DAT(0x0004);
    LCD_WR_DAT(0x0093);
    LCD_WR_DAT(0x00e0);
    LCD_WR_REG(0x00B0);	    /*命令Set LCD Mode */
    LCD_WR_DAT(0x0000);
    LCD_WR_DAT(0x0000);

    LCD_WR_DAT((HDP >> 8) & 0X00FF); /*Set the horizontal panel size*/
    LCD_WR_DAT(HDP & 0X00FF);
    LCD_WR_DAT((VDP >> 8) & 0X00FF); /*Set the horizontal panel size*/
    LCD_WR_DAT(VDP & 0X00FF);
    LCD_WR_DAT(0x0000);

    LCD_WR_REG(0x00B4);	           		/*Set Horizontal Period */
    LCD_WR_DAT((HT >> 8) & 0X00FF);		//Set HT
    LCD_WR_DAT(HT & 0X00FF);
    LCD_WR_DAT((HPS >> 8) & 0X00FF); 	//Set HPS
    LCD_WR_DAT(HPS & 0X00FF);
    LCD_WR_DAT(HPW);			   		//Set HPW
    LCD_WR_DAT((LPS >> 8) & 0X00FF); 	//Set HPS
    LCD_WR_DAT(LPS & 0X00FF);
    LCD_WR_DAT(0x0000);

    LCD_WR_REG(0x00B6);	           		//VSYNC
    LCD_WR_DAT((VT >> 8) & 0X00FF); 	//Set VT
    LCD_WR_DAT(VT & 0X00FF);
    LCD_WR_DAT((VPS >> 8) & 0X00FF); 	//Set VPS
    LCD_WR_DAT(VPS & 0X00FF);
    LCD_WR_DAT(VPW);			   		//Set VPW
    LCD_WR_DAT((FPS >> 8) & 0X00FF); 	//Set FPS
    LCD_WR_DAT(FPS & 0X00FF);

    LCD_WR_REG(0x00BA);
    LCD_WR_DAT(0x0005);           		//GPIO[3:0] out 1

    LCD_WR_REG(0x00B8);
    LCD_WR_DAT(0x0007);    				//GPIO3=input, GPIO[2:0]=output
    LCD_WR_DAT(0x0001);    				//GPIO0 normal

    LCD_WR_REG(0x0035);       			//tear on
    LCD_WR_DAT(0x0000);

    LCD_WR_REG(0x0036);       			//rotation
    /* LCD_WR_DAT(0x0008); */
    LCD_WR_DAT(0x00a8);

    /*
     * set pixel data interface
     * 000:8-bit
     * 001:12-bit
     * 010:16-bit packed
     * 011:16-bit(565 format)
     * 100:18-bit
     * 101:24-bit
     * 110:9-bit
     * */
    LCD_WR_REG(0x00F0);     //pixel data interface
#ifdef SSD1963_PORT_8BITS
    LCD_WR_DAT(0x0000);
#endif
#ifdef SSD1963_PORT_16BITS
    LCD_WR_DAT(0x0003);
#endif


    //delay_2ms(50);
    LCD_WR_REG(0x0029);        //display on

    LCD_WR_REG(0x00BE);        //set PWM for B/L
    LCD_WR_DAT(0x0006);
    LCD_WR_DAT(0x0080);

    LCD_WR_DAT(0x0001);
    LCD_WR_DAT(0x00f0);
    LCD_WR_DAT(0x0000);
    LCD_WR_DAT(0x0000);

    LCD_WR_REG(0x00d0);//设置动态背光控制配置
    LCD_WR_DAT(0x000d);

    /* while(1) { */
    LCD_Clear(0xF800);
    delay_2ms(250);
    LCD_Clear(0x07E0);
    delay_2ms(250);
    LCD_Clear(0x003F);
    delay_2ms(250);
    /* } */

    LCD_WR_REG(0X002A);
    LCD_WR_DAT(0);
    LCD_WR_DAT(0);
    LCD_WR_DAT(HDP >> 8);
    LCD_WR_DAT(HDP & 0XFF);

    /*  Y轴  */
    LCD_WR_REG(0X002B);
    LCD_WR_DAT(0);
    LCD_WR_DAT(0);
    LCD_WR_DAT(VDP >> 8);
    LCD_WR_DAT(VDP & 0X00FF);

    LCD_WR_REG(0X002C);
    SSD1963_CS_EN();
    SSD1963_RS_H();

    /* dev_close(pap); */

    return 0;
}

void ssd1963_set_pos()
{
    LCD_WR_REG(0X002A);
    LCD_WR_DAT(0);
    LCD_WR_DAT(0);
    LCD_WR_DAT(HDP >> 8);
    LCD_WR_DAT(HDP & 0XFF);

    /*  Y轴  */
    LCD_WR_REG(0X002B);
    LCD_WR_DAT(0);
    LCD_WR_DAT(0);
    LCD_WR_DAT(VDP >> 8);
    LCD_WR_DAT(VDP & 0X00FF);

    LCD_WR_REG(0X002C);
    SSD1963_CS_EN();
    SSD1963_RS_H();
}

static void lcd_ssd1963_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


REGISTER_IMD_DEVICE(lcd_ssd1963_dev) = {
    .info = {
        .test_mode 	     = false,
        .test_mode_color = 0xFF0000,
        .background_color = 0x0000FF,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .target_xres     = LCD_DEV_WIDTH,
        .target_yres     = LCD_DEV_HIGHT,
        .interlaced_mode = INTERLACED_NONE,

        .rotate          = ROTATE_270,
        .len 			 = LEN_256,

        .format          = FORMAT_RGB888,
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

#ifdef SSD1963_PORT_8BITS
    .drive_mode      = MODE_MCU,
    .ncycle          = CYCLE_THREE,
    .dclk_cfg 		 = CLK_DIS,
    .sync0_cfg       = SIGNAL_TYPE_DEN  |  CLK_NEGATIVE  |  CLK_EN,
    .sync1_cfg       = SIGNAL_TYPE_HSYNC | CLK_NEGATIVE | CLK_DIS,
    .sync2_cfg       = SIGNAL_TYPE_VSYNC | CLK_NEGATIVE | CLK_DIS,
    .data_width      = PORT_8BITS,
    .clk_cfg    	 = PLL2_CLK | DIVA_1 | DIVB_1,

    .timing = {//13.7 fps
        .hori_total 			= 520 * 3,//60Hz
        .hori_sync 		        = 20 * 3,
        .hori_back_porth 		= 20 * 3,
        .hori_pixel 			= 480,

        .vert_total 			= 840,
        .vert_sync 		        = 20,
        .vert_back_porth_odd 	= 20,
        .vert_back_porth_even 	= 0,
        .vert_pixel		 		= 800,
    },
#endif
#ifdef SSD1963_PORT_16BITS
    .drive_mode      = MODE_MCU,
    .ncycle          = CYCLE_ONE,
    .dclk_cfg 		 = CLK_DIS,
    .sync0_cfg       = SIGNAL_TYPE_DEN |  CLK_NEGATIVE |  CLK_EN,
    .sync1_cfg       = SIGNAL_TYPE_HSYNC | CLK_NEGATIVE | CLK_DIS,
    .sync2_cfg       = SIGNAL_TYPE_VSYNC | CLK_NEGATIVE | CLK_DIS,
    .data_width      = PORT_16BITS,
    .clk_cfg    	 = PLL2_CLK | DIVA_1 | DIVB_1,

    .timing = {
        .hori_total 			= 520,//60Hz
        .hori_sync 		        = 20,
        .hori_back_porth 		= 20,
        .hori_pixel 			= 480,

        .vert_total 			= 840,
        .vert_sync 		        = 20,
        .vert_back_porth_odd 	= 20,
        .vert_back_porth_even 	= 0,
        .vert_pixel		 		= 800,
    },
#endif
};

REGISTER_LCD_DEVICE_DRIVE(lcd_dev)  = {
    .logo   = "lcd_ssd1963",
    .type   = LCD_DVP_MCU,
    .init   = lcd_ssd1963_init,
    .dev    = (void *) &lcd_ssd1963_dev,
    .bl_ctrl = lcd_ssd1963_backctrl,
};

#endif
