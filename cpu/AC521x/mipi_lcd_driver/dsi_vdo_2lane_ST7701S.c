#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imd.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "os/os_compat.h"

#ifdef LCD_DSI_VDO_2LANE_MIPI_ST7701S

//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list


const static u8 init_cmd_list[] = {
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD,0xAA, */
    /* _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11, */
    /* _R, DELAY(0), PACKET_DCS, SIZE(2), 0xFA,0x01, */
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF, 0x77, 0x01, 0x00, 0x00, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC0, 0x63, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC1, 0x11, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC2, 0x31, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCC, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xB0, 0x00, 0x05, 0x0F, 0x0D, 0x13, 0x07, 0x01, 0x08, 0x09, 0x1E, 0x05, 0x12, 0x10, 0xB2, 0x2F, 0x18,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xB1, 0x00, 0x0F, 0x17, 0x0C, 0x0D, 0x05, 0x01, 0x08, 0x08, 0x1E, 0x05, 0x13, 0x11, 0xB2, 0x2F, 0x18,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF, 0x77, 0x01, 0x00, 0x00, 0x11,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB0, 0x8C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB1, 0x5D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB2, 0x07,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB3, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB7, 0x47,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB7, 0x85,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB8, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB9, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC1, 0x78,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC2, 0x78,
    _W, DELAY(100), PACKET_DCS, SIZE(2), 0xD0, 0x88,
    _W, DELAY(0), PACKET_DCS, SIZE(4), 0xE0, 0x00, 0x00, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(12), 0xE1, 0x08, 0x00, 0x0A, 0x00, 0x07, 0x00, 0x09, 0x00, 0x00, 0x33, 0x33,
    _W, DELAY(0), PACKET_DCS, SIZE(14), 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xE3, 0x00, 0x00, 0x33, 0x33,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xE4, 0x44, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xE5, 0x0E, 0x60, 0xA0, 0xA0, 0x10, 0x60, 0xA0, 0xA0, 0x0A, 0x60, 0xA0, 0xA0, 0x0C, 0x60, 0xA0, 0xA0,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0xE6, 0x00, 0x00, 0x33, 0x33,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xE7, 0x44, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xE8, 0x0D, 0x60, 0xA0, 0xA0, 0x0F, 0x60, 0xA0, 0xA0, 0x09, 0x60, 0xA0, 0xA0, 0x0B, 0x60, 0xA0, 0xA0,
    _W, DELAY(0), PACKET_DCS, SIZE(8), 0xEB, 0x02, 0x01, 0xE4, 0xE4, 0x44, 0x00, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xEC, 0x02, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0xED, 0xAB, 0x89, 0x76, 0x54, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0x45, 0x67, 0x98, 0xBA,
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF, 0x77, 0x01, 0x00, 0x00, 0x00,
    _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11,
    _W, DELAY(0), PACKET_DCS, SIZE(1), 0x29,
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
#define vda_line 800
#define vfp_line 10

#define hsa_pixel  8
#define hbp_pixel  42
#define hda_pixel  480
#define hfp_pixel  44

REGISTER_MIPI_DEVICE(mipi_dev_t) = {
    .info = {
        .xres 			    = LCD_DEV_WIDTH,
        .yres 			    = LCD_DEV_HIGHT,
        .target_xres 	    = 800/* LCD_DEV_WIDTH */,
        .target_yres 	    = 480/* LCD_DEV_HIGHT */,
        /* .target_xres 	    = LCD_DEV_WIDTH, */
        /* .target_yres 	    = LCD_DEV_HIGHT, */
        .test_mode 		    = false,
        .test_mode_color    = 0x0000ff/* 0x373737 */,
        .background_color   = 0x000000,
        .interlaced_mode    = INTERLACED_NONE,
        .format 		    = FORMAT_RGB888,
        .len 			    = LEN_256,
        .rotate             = ROTATE_270,
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

#if (defined CONFIG_BOARD_5211B_DEV_20181116 \
		|| defined CONFIG_BOARD_5218B_DEV_20181130)

        .x0_lane = lane_en | lane_d0,
        .x1_lane = lane_en | lane_d1,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_dis,
        .x4_lane = lane_dis,
#elif (defined CONFIG_BOARD_5218B_DEV_IPC_DOORBELL_20181130)
        .x0_lane = lane_dis,
        .x1_lane = lane_dis,
        .x2_lane = lane_en | lane_d0 | lane_ex,
        .x3_lane = lane_en | lane_d1 | lane_ex,
        .x4_lane = lane_en | lane_clk | lane_ex,


#elif (defined CONFIG_BOARD_5214B_20180709 || defined CONFIG_BOARD_5211B_20180912)
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
static int dsi_vdo_4lane_st7701s_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 lcd_lane  = data->lcd_io.lcd_lane;

    printf("ST7701S init...\n");
    /*
     * lcd reset
     */

    if (-1 != lcd_lane) { /*4 lane*/
        gpio_direction_output(lcd_lane, 1);
    }
    if (-1 != (s8)lcd_reset) {
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
    .logo = "mipi_4lane_ST7701S",
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_4lane_st7701s_init,
    .bl_ctrl = mipi_backlight_ctrl,
};

#endif
