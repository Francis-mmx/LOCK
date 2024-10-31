#include "generic/typedef.h"
#include "asm/cpu.h"
#include "asm/dsi.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_DSI_DCS_2LANE_24BIT
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
const static u8 init_cmd_list[] = {
_W, DELAY(120), PACKET_DCS, SIZE(1), 0x11,


_W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF,0x77,0x01,0x00,0x00,0x13,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xEF,0x08,
_W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF,0x77,0x01,0x00,0x00,0x10,
_W, DELAY(0), PACKET_DCS, SIZE(3), 0xC0,    //Display Line Setting
0x63,   //3B
0x00,   //01

_W, DELAY(0), PACKET_DCS, SIZE(3), 0xC1,   //Porch Control
0x0C,   //VBP=12 //0x0A
0x02,   //VFP=02

_W, DELAY(0), PACKET_DCS, SIZE(3), 0xC2,   //Inversion selection & Frame Rate Control
0x01,0x07,

_W, DELAY(0), PACKET_DCS, SIZE(2), 0xCC,0x10,
_W, DELAY(0), PACKET_DCS, SIZE(17), 0xB0,  //Positive Voltage Gamma Control
0xCD,0x18,0x1F,0x0F,0x13,0x08,0x09,0x08,0x08,0x24,0x03,0x10,0x0E,0x21,0x24,0x0B,

_W, DELAY(0), PACKET_DCS, SIZE(17), 0xB1,  //Negative Voltage Gamma Control
0xC3,0x0F,0x18,0x0B,0x0F,0x05,0x09,0x09,0x08,0x24,0x06,0x13,0x13,0x28,0x2D,0x15,

_W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF,0x77,0x01,0x00,0x00,0x11,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB0,0x5D,

_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB1,  //VCOM amplitude setting
0x3F,

_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB2,  //VGH Voltage setting
0x82,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB3,0x80,

_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB5,  //VGL Voltage setting
0x45,

_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB7,0x85,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB8,0x21,
_W, DELAY(0), PACKET_DCS, SIZE(3), 0xB9,0x10,0x1F,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xBB,0x03,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xBC,0x3E,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xC1,0x78,
_W, DELAY(0), PACKET_DCS, SIZE(2), 0xC2,0x78,
_W, DELAY(10), PACKET_DCS, SIZE(2), 0xD0,0x88,

_W, DELAY(0), PACKET_DCS, SIZE(4), 0xE0,0x00,0x00,0x02,
_W, DELAY(0), PACKET_DCS, SIZE(12), 0xE1,0x04,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20,
_W, DELAY(0), PACKET_DCS, SIZE(13), 0xE2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(5), 0xE3,0x00,0x00,0x33,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(3), 0xE4,0x22,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(17), 0xE5,0x04,0x34,0x9A,0xA0,0x06,0x34,0x9A,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(5), 0xE6,0x00,0x00,0x33,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(3), 0xE7,0x22,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(17), 0xE8,0x05,0x34,0x9A,0xA0,0x07,0x34,0x9A,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(8), 0xEB,0x02,0x00,0x40,0x40,0x00,0x00,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(3), 0xEC,0x00,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(17), 0xED,0xFA,0x45,0x0B,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xB0,0x54,0xAF,

_W, DELAY(0), PACKET_DCS, SIZE(7), 0xEF,0x08,0x08,0x08,0x45,0x3F,
0x54,

_W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF,0x77,0x01,0x00,0x00,0x13,
_W, DELAY(0), PACKET_DCS, SIZE(3), 0xE8,0x00,0x0E,
_W, DELAY(120), PACKET_DCS, SIZE(2), 0x11,0x00,

_W, DELAY(0), PACKET_DCS, SIZE(3), 0xE8,0x00,0x0C,

_W, DELAY(0), PACKET_DCS, SIZE(3), 0xE8,0x00,0x00,
_W, DELAY(0), PACKET_DCS, SIZE(6), 0xFF,0x77,0x01,0x00,0x00,0x00,
_W, DELAY(20), PACKET_DCS, SIZE(2), 0x29,0x00,

};

#define freq 321

#define bpp_num  24

//垂直时序要求比较严
#define vsa_line 4
#define vbp_line 20
#define vda_line 800
#define vfp_line 24

#define hsa_pixel  20
#define hbp_pixel  60
#define hda_pixel  480
#define hfp_pixel  60

REGISTER_MIPI_DEVICE(mipi_dev_t) = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .target_xres 	= 480,
        .target_yres 	= 800,
        .test_mode 		= false,
        .test_mode_color = 0x0000ff,
        .background_color = 0x000000,
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,

        .rotate         = ROTATE_0,

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
        .x0_lane = lane_en | lane_ex |lane_d0,
        .x1_lane = lane_en | lane_ex |lane_d1,
        .x2_lane = lane_en | lane_ex |lane_clk ,
        .x3_lane = lane_dis ,
        .x4_lane = lane_dis ,
    },
    {
        .video_mode = VIDEO_STREAM_VIDEO,//视频模式 类似于dump panel
        .sync_mode  = SYNC_PULSE_MODE,
        .color_mode = COLOR_FORMAT_RGB888,
        .pixel_type = PIXEL_RGB888,
        .virtual_ch   = 0,
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
    .pll_freq = 642,/* 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 */
    .pll_division = MIPI_PLL_DIV2,

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
};

//------------------------------------------------------//
// dsi run
//------------------------------------------------------//
static int dsi_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if ((u8) - 1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }

    dsi_dev_init(&mipi_dev_t);

    return 0;
}

#define PWM_PRD 0x300
/*************************************************************
*Description: PWM占空比设置函数
*Arguments	: per: 占空比(0-100)
*************************************************************/
void pwm_duty_cycle(u8 per)
{
	u32 val;
	val = PWM_PRD*per/100;
	T3_PWM = val;
}

static int pwm_backlight_init()
{
#if 1
    //IOMC1 &= ~(BIT(11)|BIT(10)|BIT(9)|BIT(8));
	//IOMC1 |= BIT(10)|BIT(9)|BIT(8); //将channel0输出选择为TMR3_PWM_OUT


    //IOMC1 &= ~(BIT(15)|BIT(14)|BIT(13)|BIT(12));
	//IOMC1 |= BIT(15)|BIT(13)|BIT(12); //将channel1输出选择为TMR3_PWM_OUT

	//IOMC1 &= ~(BIT(19)|BIT(18)|BIT(17)|BIT(16));
	//IOMC1 |= BIT(18)|BIT(17)|BIT(16); //将channel2输出选择为TMR3_PWM_OUT


	IOMC1 &= ~(BIT(23)|BIT(22)|BIT(21)|BIT(20));
	IOMC1 |= BIT(22)|BIT(21)|BIT(20); //将channel3输出选择为TMR3_PWM_OUT


	gpio_direction_output(IO_PORTH_08, 1);
	gpio_set_pull_up(IO_PORTH_08, 1);
	gpio_set_pull_down(IO_PORTH_08, 1);
	gpio_set_die(IO_PORTH_08, 1);

    T3_CNT = 0x100;

	T3_PRD = PWM_PRD;
	T3_PWM = 0;

	T3_CON |= BIT(0);
#else
	gpio_direction_output(IO_PORTG_12, 1);
#endif

    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    static u8 frist_lcd_backlight = 1;

    if (on) {
        if (frist_lcd_backlight) {
            frist_lcd_backlight = 0;
            pwm_backlight_init();
        }
        pwm_duty_cycle(80);
//        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
//        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
        pwm_duty_cycle(0);
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type = LCD_MIPI,
    .init = dsi_init,
    .dev  = &mipi_dev_t,
    .bl_ctrl = mipi_backlight_ctrl,
};
#endif
