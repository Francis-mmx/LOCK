#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imd.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "os/os_compat.h"

#ifdef LCD_DSI_VDO_4LANE_MIPI_EK79030

//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list


const static u8 init_cmd_list[] = {
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD,0xAA, */
    /* _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11, */
    /* _R, DELAY(0), PACKET_DCS, SIZE(2), 0xFA,0x01, */

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x65, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A, 0x14,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x32, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x36, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x67, 0x82,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x69, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x68, 0x00,

    _W, DELAY(0), PACKET_DCS, SIZE(20), 0x53, 0x19, 0x17, 0x15, 0x12, 0x12, 0x12, 0x13, 0x15, 0x15, 0x10, 0x0C, 0x0A, 0x0A, 0x0C, 0x0B, 0x0C, 0x09, 0x07, 0x06,
    _W, DELAY(0), PACKET_DCS, SIZE(20), 0x54, 0x19, 0x16, 0x14, 0x11, 0x11, 0x11, 0x13, 0x15, 0x15, 0x10, 0x0B, 0x09, 0x09, 0x0B, 0x0B, 0x0C, 0x09, 0x07, 0x06,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x29, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2A, 0x0C,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C, 0x81,
    _W, DELAY(0), PACKET_DCS, SIZE(9), 0x55, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0x56, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0x57, 0x00, 0x00, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x30, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x39, 0x11,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x33, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x35, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F, 0x3D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x41, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x73, 0x30,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x74, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x76, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x77, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x28, 0x31,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7c, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2e, 0x04,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4c, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x47, 0x16,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x48, 0x6A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x50, 0xc0,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x78, 0x6E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2D, 0x31,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x63, 0x04, //4 lane
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x63, 0x02, // 2 lane */

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4D, 0x00,

    _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11,
};

#define freq 446
/*#define freq 407*/

/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24
#define lane_num 4

#define vsa_line 2
#define vbp_line 17
#define vda_line 1280
#define vfp_line 10

#define hsa_pixel  8
#define hbp_pixel  42
#define hda_pixel  720
#define hfp_pixel  44

REGISTER_MIPI_DEVICE(mipi_dev_t) = {
    .info = {
        .xres 			    = LCD_DEV_WIDTH,
        .yres 			    = LCD_DEV_HIGHT,
#if (DOUBLE_720 == 0 && defined CONFIG_NET_ENABLE)
        .target_xres 	    = 1280/* LCD_DEV_WIDTH */,
        .target_yres 	    = 480/* LCD_DEV_HIGHT */,
#else
        .target_xres 	    = LCD_DEV_WIDTH,
        .target_yres 	    = LCD_DEV_HIGHT,
#endif
        .test_mode 		    = false,
        .test_mode_color    = 0x0000ff/* 0x373737 */,
        .background_color   = 0x000000,
        .interlaced_mode    = INTERLACED_NONE,
        .format 		    = FORMAT_RGB888,
        .len 			    = LEN_256,
        .rotate             = ROTATE_90,
#if (DOUBLE_720 == 0 && !defined CONFIG_NET_ENABLE)
        .layer_buf_num      = 2,
#else
        .layer_buf_num      = 3,
#endif
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

#if (defined CONFIG_BOARD_5211B_DEV_20181116 \
		|| defined CONFIG_BOARD_5218B_DEV_20181130)
        .x0_lane = lane_en | lane_d0,
        .x1_lane = lane_en | lane_d1,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_en | lane_d2,
        .x4_lane = lane_en | lane_d3,
#elif (defined CONFIG_BOARD_5214B_20180709 \
		|| defined CONFIG_BOARD_5218B_20181130 \
		|| defined CONFIG_BOARD_5211B_20180912 \
		|| defined CONFIG_BOARD_5218B_20181229)
        .x0_lane = lane_en | lane_d3,
        .x1_lane = lane_en | lane_d2,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_en | lane_d1,
        .x4_lane = lane_en | lane_d0,
#else
        .x0_lane = lane_en | lane_clk,
        .x1_lane = lane_en | lane_d0,
        .x2_lane = lane_en | lane_d1,
        .x3_lane = lane_en | lane_d2,
        .x4_lane = lane_en | lane_d3,
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
    /* .pll_freq = 892,[> 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 <] */
    .pll_freq = 1040,//freq * 2,/* 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 */
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};

static void read_data_func(void *p)
{
    u8 status;
    u8 param;
    static u8 cnt = 0;
    while (1) {
        dsi_task_con |= BIT(7);
        status = dcs_send_short_p1_bta(0xCD, 0xAA);
        printf("staus0=%d\n", status);
        delay(0x100);
        status = dcs_send_short_p0_bta(0x11);
        printf("staus1=%d\n", status);
        delay_10ms(120 / 10 + 1);

        status = dcs_read_parm(0xfa, &param, 0x01);
        delay(0x100);
        printf("\nparam=%d 0x%x\n", param, status);
        if (status == 10) {
            cnt++;
            printf("read data ok %d\n", cnt);
        }
        dsi_task_con &= ~BIT(7);

    }

}
static int dsi_vdo_4lane_ek79030_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 lcd_lane  = data->lcd_io.lcd_lane;

    printf("EK79030 init...\n");
    /*
     * lcd reset
     */

    if ((u8) - 1 != lcd_lane) { /*4 lane*/
        gpio_direction_output(lcd_lane, 1);
    }
    if ((u8) - 1 != (u8)lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }

    dsi_dev_init(&mipi_dev_t);

    /* os_task_create(read_data_func, NULL, 20, 4000, 0, "read_data_task"); */

    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (-1 == (s8)data->lcd_io.backlight) {
        return;
    }
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .logo = "mipi_4lane_EK79030",
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_4lane_ek79030_init,
    .bl_ctrl = mipi_backlight_ctrl,
};

#endif
