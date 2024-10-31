#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_480x272_8BITS

static void lcd_480x272_8bits_backctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}
REGISTER_IMD_DEVICE(lcd_480x272_8bits_dev) = {
    .info = {
        .test_mode 	     = false,
        .test_mode_color = 0xFF0000,
        .background_color = 0x000000,
        .interlaced_mode = INTERLACED_NONE,
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .target_xres     = LCD_DEV_WIDTH,
        .target_yres     = LCD_DEV_HIGHT,

        .rotate          = ROTATE_0,
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

    .drive_mode      = MODE_RGB_DE_SYNC,
    .ncycle          = CYCLE_THREE,
    .dclk_cfg 		 = CLK_EN,
#if (defined CONFIG_BOARD_5212B_DEV_20181116 \
		|| defined CONFIG_BOARD_5214_DEV_20181211 \
		|| defined CONFIG_BOARD_5212_DEV_20181213 \
		|| defined CONFIG_BOARD_5214_SOCKET_20181221 \
		|| defined CONFIG_BOARD_5213C4_8MSDRAM)
    .sync0_cfg       = SIGNAL_TYPE_HSYNC | CLK_NEGATIVE | CLK_EN,
    .sync1_cfg       = SIGNAL_TYPE_VSYNC | CLK_NEGATIVE | CLK_EN,
    .sync2_cfg       = SIGNAL_TYPE_DEN | /* CLK_NEGATIVE | */ CLK_EN,
#else

#ifdef CONFIG_BOARD_5211B_20190419
    .sync0_cfg       = SIGNAL_TYPE_HSYNC | CLK_NEGATIVE | CLK_EN,
    .sync1_cfg       = SIGNAL_TYPE_VSYNC | CLK_NEGATIVE | CLK_EN,
    .sync2_cfg       = SIGNAL_TYPE_DEN | /* CLK_NEGATIVE | */ CLK_EN,
#else
    .sync0_cfg       = SIGNAL_TYPE_DEN | /* CLK_NEGATIVE | */ CLK_EN,
    .sync1_cfg       = SIGNAL_TYPE_HSYNC | CLK_NEGATIVE | CLK_EN,
    .sync2_cfg       = SIGNAL_TYPE_VSYNC | CLK_NEGATIVE | CLK_EN,
#endif

#endif
    .data_width      = PORT_8BITS,
    .clk_cfg    	 = PLL2_CLK | DIVA_7 | DIVB_2,

    .timing = {
        .hori_total 			= 625 * 3, //60Hz
        .hori_sync 				= 1 * 3,
        .hori_back_porth 		= 40 * 3,
        .hori_pixel 			= 480,

        .vert_total 			= 288,
        .vert_sync 				= 1,
        .vert_back_porth_odd 	= 8,
        .vert_back_porth_even 	= 0,
        .vert_pixel		 		= 272,
    },
};

static int lcd_480x272_8bits_init(void *_data)
{
    printf("lcd 480x272 8bits init ...\n");
    return 0;
}

REGISTER_LCD_DEVICE_DRIVE(lcd_dev_8bits)  = {
    .logo   = "lcd_480x272_8bits",
    .type   = LCD_DVP_RGB,
    .init   = lcd_480x272_8bits_init,
    .dev    = (void *) &lcd_480x272_8bits_dev,
    .bl_ctrl = lcd_480x272_8bits_backctrl,
};


#endif
