#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "database.h"
#include "system/timer.h"
extern void set_timer_io_pwm(u8 output_channel, const char *timer_name, unsigned int gpio, u8 duty_per);
void pwm_ch0_backlight_init(u8 backlight_io);
#ifdef LCD_DSI_VDO_4LANE_YZN101IBGG2001

//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
const static u8 init_cmd_list[] = {
#if 0
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb7, 0x50, 0x00,
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb8, 0x00, 0x00,
    _W, DELAY(10), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb9, 0x00, 0x00,
    _W, DELAY(10), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xbA, 0x14, 0x42,
    _W, DELAY(10), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xbb, 0x03, 0x00,
    _W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb9, 0x01, 0x00,
    _W, DELAY(10), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xde, 0x03, 0x00,
    _W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xc9, 0x02, 0x23,


    //////////////////Initial  CODE///////////////////////
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb7, 0x10, 0x02,
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xbd, 0x00, 0x00,
    _W, DELAY(300), PACKET_DCS_WITHOUT_BTA, SIZE(1), 0x11,
    _W, DELAY(300), PACKET_DCS_WITHOUT_BTA, SIZE(1), 0x29,

////////////////////Initial  CODE/////////////////////

//SSD2828_Initial
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb7, 0x50, 0x00,
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb8, 0x00, 0x00,
    _W, DELAY(10), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb9, 0x00, 0x00,
    _W, DELAY(10), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xbA, 0x2b, 0x82,
    _W, DELAY(1), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xbb, 0x07, 0x00,
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb9, 0x01, 0x00,
    _W, DELAY(5), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xca, 0x01, 0x23,
    _W, DELAY(5), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xcb, 0x10, 0x05,
    _W, DELAY(5), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xcc, 0x05, 0x10,
    _W, DELAY(5), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xd0, 0x00, 0x00,

    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb6, 0x03, 0x00,
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xde, 0x03, 0x00,
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xd6, 0x05, 0x00,
    _W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(3), 0xb7, 0x4b, 0x02,
    _W, DELAY(0), PACKET_DCS_WITHOUT_BTA, SIZE(1), 0x2c,
#endif
//_W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x80, 0x9f,
//_W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x81, 0xBC,
//_W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x82, 0x18,
//_W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x83, 0x88,
//_W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x84, 0x4f,
//_W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x85, 0xd2,
//_W, DELAY(100), PACKET_DCS_WITHOUT_BTA, SIZE(2), 0x86, 0x88,



};

#define freq 323
/*#define freq 407*/

/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24
#define lane_num 4

//#define vsa_line 2
//#define vbp_line 160
//#define vda_line 1024
//#define vfp_line 160
//
//#define hsa_pixel  8
//#define hbp_pixel  23
//#define hda_pixel  600
//#define hfp_pixel  12

//#define vsa_line 2
//#define vbp_line 17
//#define vda_line 1024
//#define vfp_line 10
//
//#define hsa_pixel  1
//#define hbp_pixel  42
//#define hda_pixel  600
//#define hfp_pixel  44


//#define vsa_line 2
//#define vbp_line 20
//#define vda_line 1024
//#define vfp_line 22 -10
//
//#define hsa_pixel 24
//#define hbp_pixel 160 - 140
//#define hda_pixel 600
//#define hfp_pixel 160 + 240


/* #define vsa_line 2 */
/* #define vbp_line 22 */
/* #define vda_line 600 */
/* #define vfp_line 12 */

/* #define hsa_pixel 24 */
/* #define hbp_pixel 160 */
/* #define hda_pixel 1024 */
/* #define hfp_pixel 160// + 240 */
#define vsa_line 3
#define vbp_line 12
#define vda_line 608
#define vfp_line 16

#define hsa_pixel  20
#define hbp_pixel  40
#define hda_pixel  1024
#define hfp_pixel  40// + (1024 - 600)

REGISTER_MIPI_DEVICE(mipi_dev_t) = {
    .info = {
        .xres 			    = LCD_DEV_WIDTH,
        .yres 			    = LCD_DEV_HIGHT,
        .target_xres        = LCD_DEV_WIDTH,
        .target_yres 	    = LCD_DEV_HIGHT,
        .test_mode 		    = false,
        .test_mode_color 	= 0xff0000,
        .background_color   = 0x000000,
        .interlaced_mode    = INTERLACED_NONE,
        .format 		    = FORMAT_RGB888,
        .len 			    = LEN_256,
        .rotate             = ROTATE_0,
#if (DOUBLE_720 == 0 && !defined CONFIG_NET_ENABLE)
        .layer_buf_num      = 2,
#else
        .layer_buf_num      = 2,
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
    /* { */
    /* .x0_lane = lane_en |lane_ex| lane_d3, */
    /* .x1_lane = lane_en |lane_ex| lane_d2, */
    /* .x2_lane = lane_en |lane_ex|lane_clk, */
    /* .x3_lane = lane_en |lane_ex| lane_d1, */
    /* .x4_lane = lane_en |lane_ex|lane_d0, */
    /* }, */
    {
        .x0_lane = lane_en | lane_d0,
        .x1_lane = lane_en | lane_d1,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_en | lane_d2,
        .x4_lane = lane_en | lane_d3,
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

//        .dsi_vdo_hsa_v  = (lane_num * hsa_pixel) - 10,
//        .dsi_vdo_hbp_v  = (lane_num * hbp_pixel) - 10,
//        .dsi_vdo_hact_v = (bpp_num  * hda_pixel) / 8,
//        .dsi_vdo_hfp_v  = (lane_num * hfp_pixel) - 6,
//        .dsi_vdo_bllp0_v = hsa_pixel + hbp_pixel + hda_pixel + hfp_pixel,//?+hda_pixel
        .dsi_vdo_hsa_v   = ((bpp_num * hsa_pixel) / 8) - 10,
        .dsi_vdo_hbp_v   = ((bpp_num * hbp_pixel) / 8) - 10,
        .dsi_vdo_hact_v  = ((bpp_num * hda_pixel) / 8),
        .dsi_vdo_hfp_v   = ((bpp_num * hfp_pixel) / 8) - 6,

        .dsi_vdo_bllp0_v = ((bpp_num * (hbp_pixel + hda_pixel + hfp_pixel) / 8) - 10),
        .dsi_vdo_bllp1_v = ((bpp_num * hda_pixel) / 8),
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

    .pll_freq = freq * 2,/* 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 */
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
};
void set_lcd_bl_level(u8 level)
{
    printf(">>> set_lcd_bl_level:%d\n", level);
//    u8 level_table[] = {10,20,30,40,50,60,70,80,90,100};
    u8 level_table[] = {10, 20, 40, 60, 80, 100};
    set_timer_io_pwm(3, "timer3", IO_PORTF_00, level_table[level]);

}

static int dsi_vdo_init(void *_data)
{
    log_d("\n\n lcd dsi_vdo_init\n\n");
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 lcd_lane  = data->lcd_io.lcd_lane;

    /*
     * lcd reset
     */
    if ((u8) - 1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }

    gpio_direction_output(lcd_lane, 1);
    /* gpio_direction_output(IO_PORTG_07, 1); */

    dsi_dev_init(&mipi_dev_t);
    //pwm_ch0_backlight_init(data->lcd_io.backlight);
    //gpio_direction_output(IO_PORTF_00, 1);
    //log_d("\n\n pwm lcd io %d\n\n",data->lcd_io.backlight);
    //set_lcd_bl_level(db_select("lcdb"));

    return 0;
}


static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    log_d("\n\n lcd mipi_backlight_ctrl %d %d\n\n", data->lcd_io.backlight, on);
    if ((u8) - 1 == data->lcd_io.backlight) {
        return;
    }
    static u8 flag = 0;


    if (on) {
        if (!flag) {
            flag = 1;
            delay_10ms(200);
            /* sys_timeout_add_to_task("sys_timer",(void *)db_select("lcdb"),set_lcd_bl_level,1000); */
            gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
            return;
        }
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);

        /* set_lcd_bl_level(db_select("lcdb")); */
    } else {
        /* set_timer_io_pwm(3 , "timer3",IO_PORTF_00,0); */
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


struct color_correct *get_lcd_color_adjust(void)
{
    return &mipi_dev_t.info.adjust;

}


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_init,
    .bl_ctrl = mipi_backlight_ctrl,
};

#endif
