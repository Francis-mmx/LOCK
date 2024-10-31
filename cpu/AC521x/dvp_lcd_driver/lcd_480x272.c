#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"


#ifdef LCD_480x272

static void lcd_480x272_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


REGISTER_IMD_DEVICE(lcd_480x272_dev) = {
    .info = {
        .test_mode 	     = false,
        .test_mode_color = 0xFF0000,
        .background_color = 0x000000,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .target_xres     = LCD_DEV_WIDTH,
        .target_yres     = LCD_DEV_HIGHT,

        .rotate          = ROTATE_0,
        .len 			 = LEN_256,
        .interlaced_mode = INTERLACED_NONE,

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

    .drive_mode      = MODE_RGB_DE_SYNC,
    .ncycle          = CYCLE_ONE,
    .dclk_cfg 		 = CLK_EN,
    .sync0_cfg       = SIGNAL_TYPE_DEN | /* CLK_NEGATIVE | */ CLK_EN,
    .sync1_cfg       = SIGNAL_TYPE_HSYNC | CLK_NEGATIVE | CLK_EN,
    .sync2_cfg       = SIGNAL_TYPE_VSYNC | CLK_NEGATIVE | CLK_EN,
    .data_width      = PORT_18BITS,
    /* .data_width      = PORT_24BITS, */
    .clk_cfg    	 = PLL2_CLK | DIVA_5 | DIVB_8,

    .timing = {
        .hori_total 			= 625 + 257,//60Hz
        .hori_sync 		        = 1,
        .hori_back_porth 		= 40,
        .hori_pixel 			= 480,

        .vert_total 			= 288,
        .vert_sync 		        = 1,
        .vert_back_porth_odd 	= 8,
        .vert_back_porth_even 	= 0,
        .vert_pixel		 		= 272,
    },
};

static int lcd_480x272_init(void *_data)
{
    printf("lcd 480x272 init ...\n");
    return 0;
}

REGISTER_LCD_DEVICE_DRIVE(lcd_dev)  = {
    .logo   = "lcd_480x272",
    .type   = LCD_DVP_RGB,
    .init   = lcd_480x272_init,
    .dev    = (void *) &lcd_480x272_dev,
    .bl_ctrl = lcd_480x272_backctrl,
};

#endif

