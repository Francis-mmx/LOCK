#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef MIPI_LCD_800X480_BST40_L5

//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
const static u8 init_cmd_list[] = {

    _W, DELAY(100), PACKET_DCS, SIZE(1), 0x11,

    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xb0, 0x04,
    _W, DELAY(0), PACKET_GENERIC, SIZE(3), 0xb3, 0x12, 0x00,

    _W, DELAY(0), PACKET_GENERIC, SIZE(3), 0xc0, 0x02, 0x87,
    _W, DELAY(0), PACKET_GENERIC, SIZE(16), 0xC1, 0x23, 0x31, 0x99, 0x21, 0x20, 0x00, 0x10, 0x28, 0x0C, 0x0A, 0x00, 0x00, 0x00, 0x21, 0x01,

    _W, DELAY(0), PACKET_GENERIC, SIZE(25), 0xC8, 0x01, 0x0A, 0x12, 0x1C, 0x2B, 0x45, 0x3F, 0x29, 0x17, 0x13, 0x0F, 0x04, 0x01, 0x0A, 0x12, 0x1C, 0x2B, 0x45, 0x3F, 0x29, 0x17, 0x13, 0x0F, 0x04,
    _W, DELAY(0), PACKET_GENERIC, SIZE(25), 0xC9, 0x01, 0x0A, 0x12, 0x1C, 0x2B, 0x45, 0x3F, 0x29, 0x17, 0x13, 0x0F, 0x04, 0x01, 0x0A, 0x12, 0x1C, 0x2B, 0x45, 0x3F, 0x29, 0x17, 0x13, 0x0F, 0x04,

    _W, DELAY(0), PACKET_GENERIC, SIZE(25), 0xCA, 0x01, 0x0A, 0x12, 0x1C, 0x2B, 0x45, 0x3F, 0x29, 0x17, 0x13, 0x0F, 0x04, 0x01, 0x0A, 0x12, 0x1C, 0x2B, 0x45, 0x3F, 0x29, 0x17, 0x13, 0x0F, 0x04,

    _W, DELAY(0), PACKET_GENERIC, SIZE(17), 0xD0, 0x99, 0x03, 0xCE, 0xA6, 0x00, 0x43, 0x20, 0x10, 0x01, 0x00, 0x01, 0x01, 0x00, 0x03, 0x01, 0x00,

    _W, DELAY(0), PACKET_GENERIC, SIZE(8), 0xD1, 0x18, 0x0C, 0x23, 0x03, 0x75, 0x02, 0x50,

    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xd3, 0x33,
    _W, DELAY(0), PACKET_GENERIC, SIZE(3), 0xd5, 0x2a, 0x2a,
    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xd6, 0x28,
    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xda, 0x01,
    _W, DELAY(0), PACKET_GENERIC, SIZE(3), 0xde, 0x01, 0x4f,
    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xe6, 0x4f,
    _W, DELAY(0), PACKET_GENERIC, SIZE(2), 0xfa, 0x03,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x36, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3a, 0x77,


    _W, DELAY(200), PACKET_DCS, SIZE(1), 0x29,

    /* _R, DELAY(10), PACKET_DCS, SIZE(2),0x0a,0x01, */

};



#define lane_num 2

#define bpp_num  24

//垂直时序要求比较严
/* #define freq (321*54/48)//300 */
/* #define freq (321*62.6/48)//300 */
/* #define freq (321*62/48)//300 */
#define freq (321*68/48)//300
#define vsa_line 4//1
#define vbp_line 12//23
#define vda_line 800
#define vfp_line 10
/* #define vsa_line 4//1 */
/* #define vbp_line 8//23 */
/* #define vda_line 800 */
/* #define vfp_line 6 */

#define hsa_pixel  80
#define hbp_pixel  70
#define hda_pixel  480
#define hfp_pixel  120

static struct mipi_dev mipi_dev_t = {
    .info = {
#ifdef MULTI_LCD_EN
        .xres 			= 800,//LCD_DEV_WIDTH,
        .yres 			= 480,//LCD_DEV_HIGHT,
#else
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
#endif
        .target_xres        = 800,
        .target_yres 	    = 480,
        /* .target_xres        = LCD_DEV_WIDTH, */
        /* .target_yres 	    = LCD_DEV_HIGHT, */

        /* .test_mode 		= false, */
        .test_mode 		= false,
        .test_mode_color    = 0xff0000,
        .background_color   = 0x000000,
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,
        .interlaced_mode    = INTERLACED_NONE,
        /* .rotate         = ROTATE_90, */
        .rotate         = ROTATE_270,
#if (DOUBLE_720_SMALL_SCR == 0)
        .layer_buf_num = 2,
#else
        .layer_buf_num = 3,
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
        /*              .x0_lane = lane_en  | lane_d0, */
        /* .x1_lane = lane_en  | lane_d1, */
        /* .x2_lane = lane_en  | lane_clk, */
#if (DOUBLE_720_SMALL_SCR == 0)
        .x0_lane = lane_en | lane_ex | lane_d0,
        .x1_lane = lane_en | lane_ex | lane_d1,
        .x2_lane = lane_en | lane_ex | lane_clk,

        .x3_lane = lane_dis,
        .x4_lane = lane_dis,
#else
        .x0_lane = lane_dis,//lane_en | lane_d3,
        .x1_lane = lane_dis,//lane_en | lane_d2,
        .x2_lane = lane_en | lane_clk,
        .x3_lane = lane_en | lane_d1,
        .x4_lane = lane_en | lane_d0,
#endif

    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,//视频模式 类似于dump panel
        .sync_mode  = SYNC_EVENT_MODE,
        .color_mode = COLOR_FORMAT_RGB888,
        .pixel_type = PIXEL_RGB888,
        .virtual_ch = 0,
        .hs_eotp_en = true,

        .dsi_vdo_vsa_v  = vsa_line,
        .dsi_vdo_vbp_v  = vbp_line,
        .dsi_vdo_vact_v = vda_line,
        .dsi_vdo_vfp_v  = vfp_line,
        /*
                .dsi_vdo_hsa_v  = (lane_num * hsa_pixel) - 10,
                .dsi_vdo_hbp_v  = (lane_num * hbp_pixel) - 10,
                .dsi_vdo_hact_v = (bpp_num  * hda_pixel) / 8,
                .dsi_vdo_hfp_v  = (lane_num * hfp_pixel) - 6,

                .dsi_vdo_bllp0_v = hsa_pixel + hbp_pixel + hda_pixel + hfp_pixel,//?+hda_pixel
                .dsi_vdo_bllp1_v = 0,*/

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
        .tval_c_rdy = 400,
        /* .tval_c_rdy = 64, */
    },
    .pll_freq = (freq * 2),
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if ((u8) - 1 == data->lcd_io.backlight) {
        return;
    }
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

static void __lcd_reset(int gpio)
{
    gpio_direction_output(gpio, 0);
    delay_2ms(50);
}

static void __lcd_reset_release(int gpio)
{
    delay_2ms(50);
    gpio_direction_output(gpio, 1);
}
static int dsi_vdo_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */

    puts("dsi vdo init 1.\n");
    if ((u8) - 1 != lcd_reset) {
        puts("dsi vdo init 2.\n");
        /* gpio_direction_output(data->lcd_io.lcd_cs, 1); */
        gpio_direction_input(data->lcd_io.lcd_cs);

        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);

        /*
        mipi_dev_t.lcd_reset = __lcd_reset;
        mipi_dev_t.lcd_reset_release = __lcd_reset_release;
        mipi_dev_t.reset_gpio = lcd_reset;
        while(1){
        PORTG_OUT ^= BIT(4);
        }
        */
    }

    dsi_dev_init(&mipi_dev_t);
    return 0;
}


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .logo    = "bst40",
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .bl_ctrl_flags = BL_CTRL_BACKLIGHT,
};

#endif
