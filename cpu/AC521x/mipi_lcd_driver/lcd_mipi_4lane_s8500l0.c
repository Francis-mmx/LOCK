#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_MIPI_4LANE_S8500L0

//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
const static u8 init_cmd_list[] = {
    // _R, DELAY(10), PACKET_DCS, SIZE(2), 0x0a,0x01,
    // _R, DELAY(10), PACKET_DCS, SIZE(2), 0x0c,0x01,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD, 0xAA, //Enable function table
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0E, 0x0D, //VCOM= -0.685V
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x18, 0xFF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x19, 0x3F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1A, 0xCC,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x32, 0x02, //R32[4]:horizontal flip;更改180度方向
    //    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x32, 0x01,//R32[4]:horizontal flip;更改180度方向
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A, 0x2D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E, 0x2A, //VGMN= -5V
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F, 0x2D, //VGMP voltage setting
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x51, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(23), 0x52, 0x13, 0x13, 0x00, 0x00, 0x02, 0x02, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x0A, 0x04, 0x08, 0x06, 0x06, 0x08, 0x04, 0x0A, 0x13, 0x13, //gamma
    _W, DELAY(0), PACKET_DCS, SIZE(14), 0x53, 0x3F, 0x0A, 0x0C, 0x0B, 0x12, 0x13, 0x0D, 0x03, 0x08, 0x06, 0x05, 0x0B, 0x14, //gamma
    _W, DELAY(0), PACKET_DCS, SIZE(14), 0x54, 0x3F, 0x0A, 0x0C, 0x0B, 0x12, 0x13, 0x0D, 0x03, 0x08, 0x06, 0x05, 0x0B, 0x14,
    _W, DELAY(0), PACKET_DCS, SIZE(13), 0x55, 0x00, 0x00, 0x02, 0x02, 0x08, 0x08, 0x1E, 0x1E, 0x00, 0x00, 0x0F, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x56, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(23), 0x59, 0x13, 0x13, 0x01, 0x01, 0x03, 0x03, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x0B, 0x05, 0x09, 0x07, 0x07, 0x09, 0x05, 0x0B, 0x13, 0x13, //GIP Control
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x67, 0x12,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6B, 0x48,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C, 0x88,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x71, 0xE3,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x73, 0xF0,
    _W, DELAY(10), PACKET_DCS, SIZE(2), 0x74, 0x91,
    _W, DELAY(10), PACKET_DCS, SIZE(2), 0x75, 0x03,

    _W, DELAY(600), PACKET_DCS, SIZE(2), 0x3C, 0x48,
    _W, DELAY(100), PACKET_DCS, SIZE(2), 0x5E, 0x42,
    /* _W, DELAY(10), PACKET_DCS, SIZE(2), 0x63, 0xC6, */
    _W, DELAY(100), PACKET_DCS, SIZE(2), 0x4D, 0x00,

    /* //    _W, DELAY(60), PACKET_DCS, SIZE(2), 0x3C, 0x48, */
    /* //    _W, DELAY(60), PACKET_DCS, SIZE(2), 0x3C, 0x48, */
    /* //    _W, DELAY(10), PACKET_DCS, SIZE(2), 0x01,0x00, */
    /* //    _W, DELAY(10), PACKET_DCS, SIZE(2), 0x11,0x00, */
    /* //    _W, DELAY(10), PACKET_DCS, SIZE(2), 0x29,0x01, */



    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD, 0xAA,//Enable function table */
    /* _W, DELAY(0), PACKET_DCS, SIZE(23), 0x52, 0x13, 0x13, 0x00, 0x00, 0x02, 0x02, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x0A, 0x04, 0x08, 0x06, 0x06, 0x08, 0x04, 0x0A, 0x13, 0x13, //gamma */
    /* _W, DELAY(0), PACKET_DCS, SIZE(23), 0x59, 0x13, 0x13, 0x01, 0x01, 0x03, 0x03, 0x13, 0x12, 0x13, 0x12, 0x13, 0x13, 0x0B, 0x05, 0x09, 0x07, 0x07, 0x09, 0x05, 0x0B, 0x13, 0x13, //GIP Control */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x67, 0x12, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C, 0x88, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x56, 0x08, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(13), 0x55, 0x00, 0x00, 0x02, 0x02, 0x08, 0x08, 0x1E, 0x1E, 0x00, 0x00, 0x0F, 0x0F, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6B, 0x48, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x18, 0xFF, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x19, 0x3F, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1A, 0xCC, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x32, 0x02,//R32[4]:horizontal flip;更改180度方向 */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A, 0x2D, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D, 0x00, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E, 0x2A,//VGMN= -5V */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F, 0x2D,//VGMP voltage setting */
    /* _W, DELAY(0), PACKET_DCS, SIZE(14), 0x53, 0x3F, 0x0A, 0x0C, 0x0B, 0x12, 0x13, 0x0D, 0x03, 0x08, 0x06, 0x05, 0x0B, 0x14, //gamma */
    /* _W, DELAY(0), PACKET_DCS, SIZE(14), 0x54, 0x3F, 0x0A, 0x0C, 0x0B, 0x12, 0x13, 0x0D, 0x03, 0x08, 0x06, 0x05, 0x0B, 0x14, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x51, 0x80, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0E, 0x0D,//VCOM= -0.685V */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x73, 0xF0, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x74, 0x91, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x75, 0x03, */
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x71, 0xE3, */

    /* _W, DELAY(100), PACKET_DCS, SIZE(2), 0x3C, 0x48, */
    /* _W, DELAY(10), PACKET_DCS, SIZE(2), 0x7A, 0x17, */
    /* _W, DELAY(10), PACKET_DCS, SIZE(2), 0x4A, 0x03, */
    /* //    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5E, 0x42, */
    /* _W, DELAY(10), PACKET_DCS, SIZE(2), 0x4D, 0x00, */

};

#define freq 320//(96*2)

/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

//#define vsa_line 2
//#define vbp_line 28
//#define vda_line 1296
//#define vfp_line 10
//
//#define hsa_pixel  2
//#define hbp_pixel  80
//#define hda_pixel  336
//#define hfp_pixel  80

#define hsa_pixel  2//2
#define hbp_pixel  28//80
#define hda_pixel  1296//336
#define hfp_pixel  10//80

#define vsa_line 2//2
#define vbp_line 80//28
#define vda_line 336//1296
#define vfp_line 80//10

REGISTER_MIPI_DEVICE(mipi_dev_t) = {
    .info = {
        .xres               = LCD_DEV_WIDTH,
        .yres               = LCD_DEV_HIGHT,
        .target_xres        = LCD_DEV_WIDTH,
        .target_yres        = LCD_DEV_HIGHT,
        .test_mode          = false,
        .test_mode_color    = 0xff0000,
        .background_color   = 0xff0000,
        .format             = FORMAT_RGB888,
        .rotate             = ROTATE_0,
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
    {
        .x0_lane = lane_en | lane_clk,
        .x1_lane = lane_en | lane_d0,
        .x2_lane = lane_en | lane_d1,
        .x3_lane = lane_en | lane_d2,
        .x4_lane = lane_en | lane_d3,
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_mode  = SYNC_EVENT_MODE,
        .color_mode = COLOR_FORMAT_RGB888,
        .pixel_type = PIXEL_RGB888,
        .virtual_ch	= 0,
        .hs_eotp_en = false,

        .dsi_vdo_vsa_v  = vsa_line,
        .dsi_vdo_vbp_v  = vbp_line,
        .dsi_vdo_vact_v = vda_line,
        .dsi_vdo_vfp_v  = vfp_line,

        .dsi_vdo_hsa_v   = ((bpp_num * hsa_pixel) / 8) - 10,
        .dsi_vdo_hbp_v   = ((bpp_num * hbp_pixel) / 8) - 10,
        .dsi_vdo_hact_v  = ((bpp_num * hda_pixel) / 8),
        .dsi_vdo_hfp_v   = ((bpp_num * hfp_pixel) / 8) - 6,
        .dsi_vdo_bllp0_v = ((bpp_num * (hbp_pixel + hda_pixel + hfp_pixel) / 8) - 10),
        .dsi_vdo_bllp1_v = ((bpp_num * hda_pixel) / 8),
    },
    {
        .tval_lpx   = ((80     * freq / 1000) / 2 - 1),
        .tval_wkup  = ((100000 * freq / 1000) / 8 - 1),
        .tval_c_pre = ((40     * freq / 1000) / 2 - 1),
        .tval_c_sot = ((300    * freq / 1000) / 2 - 1),
        .tval_c_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_c_brk = ((150    * freq / 1000) / 2 - 1),
        .tval_d_pre = ((60     * freq / 1000) / 2 - 1),
        .tval_d_sot = ((160    * freq / 1000) / 2 - 1),
        .tval_d_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_d_brk = ((150    * freq / 1000) / 2 - 1),
        .tval_c_rdy = 400/* 64 */,
    },
    .pll_freq = 768,/* 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 */
    .pll_division = MIPI_PLL_DIV4,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
};

static int lcd_mipi_s8500l0_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if (-1 != lcd_reset) {
        gpio_set_pull_up(lcd_reset, 1);
        gpio_direction_input(lcd_reset);
        delay_2ms(10);
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(10);
//        gpio_direction_output(lcd_reset, 1);
        gpio_set_pull_up(lcd_reset, 1);
        gpio_direction_input(lcd_reset);
        delay_2ms(10);
    }

    dsi_dev_init(&mipi_dev_t);
    /* dsi_video_kickstart(); */
    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (-1 == data->lcd_io.backlight) {
        return;
    }
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = lcd_mipi_s8500l0_init,
    .bl_ctrl = mipi_backlight_ctrl,
};

#endif
