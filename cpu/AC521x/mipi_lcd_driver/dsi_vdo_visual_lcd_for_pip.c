#include "generic/typedef.h"
#include "asm/cpu.h"
#include "asm/dsi.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_DSI_VDO_VISUAL_LCD_FOR_PIP


//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list


const static u8 init_cmd_list[] = {
};

#define freq 360

/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24
#define lane_num 2

#define vsa_line 2
#define vbp_line 4
#define vda_line LCD_DEV_HIGHT
#define vfp_line 10

#define hsa_pixel  8
#define hbp_pixel  42
#define hda_pixel  LCD_DEV_WIDTH
#define hfp_pixel  44

REGISTER_MIPI_DEVICE(mipi_dev_t) = {
    .info = {
        .xres 			    = LCD_DEV_WIDTH,
        .yres 			    = LCD_DEV_HIGHT,
        .target_xres 	    = LCD_DEV_WIDTH,
        .target_yres 	    = LCD_DEV_HIGHT,
        .test_mode 		    = false,
        .test_mode_color    = 0x0000ff/* 0x373737 */,
        .background_color   = 0x000000,
        .interlaced_mode    = INTERLACED_NONE,
        .format 		    = FORMAT_RGB888,
        .len 			    = LEN_256,
        .rotate             = ROTATE_0,
        .layer_buf_num      = 2,
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
        //touch V1

        //5403
        /* .x0_lane =  lane_dis, */
        /* .x1_lane =  lane_en | lane_clk, */
        /* .x2_lane =  lane_en | lane_d1, */
        /* .x3_lane =  lane_en | lane_d0, */
        /* .x4_lane =  lane_dis, */

        //lingxiang
        /* .x0_lane = lane_en | lane_ex | lane_d0, */
        /* .x1_lane = lane_en | lane_ex | lane_d1, */
        /* .x2_lane = lane_en | lane_ex | lane_clk, */
        /* .x3_lane = lane_en | lane_ex | lane_d2, */
        /* .x4_lane = lane_en | lane_ex | lane_d3, */

        //JL84_AC5218_DVP_MIPI_WIFI_V1.0_20181011
        /* .x0_lane =  lane_en | lane_d3, */
        /* .x1_lane =  lane_en | lane_d2, */
        /* .x2_lane =  lane_en | lane_clk, */
        /* .x3_lane =  lane_en | lane_d1, */
        /* .x4_lane =  lane_en | lane_d0, */

#if defined CONFIG_BOARD_5211B_DEV_20181116
        .x0_lane = lane_en | lane_d0,
        .x1_lane = lane_en | lane_d1,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_dis,
        .x4_lane = lane_dis,
#elif defined CONFIG_BOARD_5214B_20180709
        .x0_lane = lane_dis,
        .x1_lane = lane_dis,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_en | lane_d1,
        .x4_lane = lane_en | lane_d0,
#else
        .x0_lane = lane_en | lane_clk,
        .x1_lane = lane_en | lane_d0,
        .x2_lane = lane_en | lane_d1,
        .x3_lane = lane_dis,
        .x4_lane = lane_dis,
#endif
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_mode  = SYNC_PULSE_MODE,
        .color_mode = COLOR_FORMAT_RGB888,
        .pixel_type = PIXEL_RGB888,
        .virtual_ch = 0,
        .hs_eotp_en = true,

        .dsi_vdo_vsa_v  = vsa_line,
        .dsi_vdo_vbp_v  = vbp_line,
        .dsi_vdo_vact_v = vda_line,
        .dsi_vdo_vfp_v  = vfp_line,

        .dsi_vdo_hsa_v  = (lane_num * hsa_pixel) - 10,
        .dsi_vdo_hbp_v  = (lane_num * hbp_pixel) - 10,
        .dsi_vdo_hact_v = (bpp_num  * hda_pixel) / 8,
        .dsi_vdo_hfp_v  = (lane_num * hfp_pixel) - 6,
        .dsi_vdo_bllp0_v = hsa_pixel + hbp_pixel + hda_pixel + hfp_pixel,//?+hda_pixel

        /* .dsi_vdo_hsa_v   = ((bpp_num * hsa_pixel) / 8) - 10, */
        /* .dsi_vdo_hbp_v   = ((bpp_num * hbp_pixel) / 8) - 10, */
        /* .dsi_vdo_hact_v  = ((bpp_num * hda_pixel) / 8), */
        /* .dsi_vdo_hfp_v   = ((bpp_num * hfp_pixel) / 8) - 6, */

        /* .dsi_vdo_bllp0_v = ((bpp_num * (hbp_pixel + hda_pixel + hfp_pixel) / 8) - 10), */
        /* .dsi_vdo_bllp1_v = ((bpp_num * hda_pixel) / 8), */
    },
    {
        /* 以下参数只需修改freq */
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
    .pll_freq = 720,/* 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 */
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};

static int dsi_vdo_4lane_st7701s_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    puts("dsi_dev_init in\n");
    dsi_dev_init(&mipi_dev_t);
    puts("dsi_dev_init out\n");


    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
}


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .logo = "mipi_4lane_ST7701S",
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_4lane_st7701s_init,
    .bl_ctrl = mipi_backlight_ctrl,
};


#endif
