#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "asm/avo_cfg.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_AVOUT_HD

static void lcd_ahd_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    /* if (on) { */
    /* gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value); */
    /* } else { */
    /* gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value); */
    /* } */
}

REGISTER_IMD_DEVICE(lcd_ahd_dev) = {
    .info = {
        .test_mode       = false,
        .test_mode_color = 0x008080,
        .background_color = 0x000000,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT + 4,
        .target_xres     = LCD_DEV_WIDTH,
        .target_yres     = LCD_DEV_HIGHT,
        .format          = FORMAT_YUV422,
        .interlaced_mode = INTERLACED_NONE,
        .interlaced_1st_filed = ODD_FILED,

        .len 			 = LEN_256,
        .rotate          = ROTATE_0,

        .adjust = {
            .y_gain = 0x100,
            .u_gain = 0x100,
            .v_gain = 0x100,
            .r_gain = 0x80,
            .g_gain = 0x80,
            .b_gain = 0x80,

            .r_coe0 = 0x80,
            .r_coe1 = 0,
            .r_coe2 = 0,

            .g_coe0 = 0,
            .g_coe1 = 0x80,
            .g_coe2 = 0,

            .b_coe0 = 0,
            .b_coe1 = 0,
            .b_coe2 = 0x80,

            .r_gma  = 100,
            .g_gma  = 100,
            .b_gma  = 100,
            //~~~~~~~~~~~~~~~~~~//
            /* .y_gain = 0x100, */
            /* .u_gain = 0x100 + 32, */
            /* .v_gain = 0x100 + 32, */
            /* .y_offs = -2, */
            /* .u_offs = 1, */
            /* .v_offs = 1, */
            /* .r_gain = 0x80, */
            /* .g_gain = 0x80, */
            /* .b_gain = 0x80 + 4, */

            /* .r_coe0 = 0x80 + 8, */
            /* .r_coe1 = 0 - 4, */
            /* .r_coe2 = 0 - 4, */

            /* .g_coe0 = 0 - 4, */
            /* .g_coe1 = 0x80 + 8, */
            /* .g_coe2 = 0 - 4, */

            /* .b_coe0 = 0 + 4, */
            /* .b_coe1 = 0 - 4, */
            /* .b_coe2 = 0x80 + 4, */

            /* .r_gma  = 100, */
            /* .g_gma  = 100, */
            /* .b_gma  = 100, */
        },
    },

    .drive_mode      = MODE_AVOUT,
    /* .avout_mode 	 = CVE_PAL_1280 | CVE_COLOR_BAR, */
    .avout_mode 	 = CVE_PAL_1280 /*| CVE_COLOR_BAR*/,
    .pll0_nf         = 60,
    .pll0_nr         = 2160,/* 2160*12M/60 = 432M */
    /*
     * PG6 PG7处挂12M晶振
     */
    /* .clk_cfg         = IMD_INTERNAL | OSC_12M | PLL0_CLK | DIVA_3 | DIVB_1, */
    /* .clk_cfg         = IMD_INTERNAL | XOSC0_12M | PLL0_CLK | DIVA_3 | DIVB_1, */
    .clk_cfg         = IMD_INTERNAL | OSC_12M | PLL0_CLK | DIVA_3 | DIVB_1,
    .ncycle          = CYCLE_ONE,

    .dclk_cfg        = CLK_DIS | CLK_NEGATIVE,
    .sync0_cfg       = CLK_DIS | SIGNAL_TYPE_DEN | CLK_NEGATIVE,
    .sync1_cfg       = CLK_DIS | SIGNAL_TYPE_HSYNC | CLK_NEGATIVE,
    .sync2_cfg       = CLK_DIS | SIGNAL_TYPE_VSYNC | CLK_NEGATIVE,
    .data_width      = PORT_1BIT,
    .timing = {
        .hori_total 			= 864 * 2,
        .hori_sync 	        	= 138 * 2,
        .hori_back_porth 		= 138 * 2,
        .hori_pixel 			= 1280,

        .vert_total 			= 313,
        .vert_sync 		        = 21,
        .vert_back_porth_odd 	= 23,
        .vert_back_porth_even 	= 24,
        .vert_pixel 			= 720,
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .logo = "lcd_avout_hd",
    .init = NULL,
    .dev  = (void *) &lcd_ahd_dev,
    .type = LCD_DVP_RGB,
    .bl_ctrl = lcd_ahd_backctrl,
};
#endif
