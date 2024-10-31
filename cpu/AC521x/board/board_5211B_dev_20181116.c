#include "app_config.h"

#ifdef CONFIG_BOARD_5211B_DEV_20181116

#include "system/includes.h"
#include "device/av10_spi.h"
#include "device/uart.h"
#include "vm_api.h"
#include "touch_panel_manager.h"

#ifdef CONFIG_WIFI_ENABLE
#include "device/wifi_dev.h"
#include "net_config.h"
#endif

#ifdef CONFIG_GSENSOR_ENABLE
#include "gSensor_manage.h"
#endif

#ifdef CONFIG_PWM_ENABLE
#include "asm/pwm.h"
#endif

// *INDENT-OFF*

#define UART_REMAP_PIN	IO_PORTA_12

//PORTA_10_11,PORTG_6_7,PORTH_12_11,PORTD_4_5,
UART2_PLATFORM_DATA_BEGIN(uart2_data)
	.baudrate = 460800,//115200,
    .port = PORT_REMAP,
	.output_channel = OUTPUT_CHANNEL0,//OUTPUT_CHANNEL0 - OUTPUT_CHANNEL3
	.tx_pin = UART_REMAP_PIN,
	.flags = UART_DEBUG,
UART2_PLATFORM_DATA_END();



//使用映射IO:设置port=PORT_REMAP,channel选择映射通道OUTPUT_CHANNEL0-3，tx_pin、rx_pin分别设置发送接收IO引脚
//PORTH_4_3,PORTF_0_1,PORTD_0_1,PORTH_0_1,
UART0_PLATFORM_DATA_BEGIN(uart0_data)
    .baudrate = 115200,
    .port = PORTD_4_5,
	.output_channel = OUTPUT_CHANNEL3,//OUTPUT_CHANNEL0 - OUTPUT_CHANNEL3
	.input_channel = INPUT_CHANNEL3,//INPUT_CHANNEL0 - INPUT_CHANNEL3
    .tx_pin = IO_PORTD_04,
    .rx_pin = IO_PORTD_05,
	.max_continue_recv_cnt = 1024,
	.idle_sys_clk_cnt = 500000,
	.clk_src = LSB_CLK,
UART0_PLATFORM_DATA_END();



#ifdef CONFIG_SD0_ENABLE
#define SD0_DET IO_PORTG_06
int sdmmc_0_io_detect(const struct sdmmc_platform_data *data)
{

    static u8 init = 0;

    if (!init) {
        init = 1;
        gpio_direction_input(SD0_DET);
        gpio_set_pull_up(SD0_DET, 1);
        gpio_set_pull_down(SD0_DET, 0);
    }

    return !gpio_read(SD0_DET);
}

SD0_PLATFORM_DATA_BEGIN(sd0_data)
	.port 					= 'C',
	.priority 				= 3,
	.data_width 			= 4,
	.speed 					= 40000000,
	.detect_mode 			= SD_IO_DECT,
	.detect_func 			= sdmmc_0_io_detect,
SD0_PLATFORM_DATA_END()

#endif //CONFIG_SD0_ENABLE

#ifdef CONFIG_SD1_ENABLE

int sdmmc_1_io_detect(const struct sdmmc_platform_data *data)
{
    static u8 init = 0;

    return 1;

    if (!init) {
        init = 1;
        gpio_direction_input(IO_PORTA_11);
        gpio_set_pull_up(IO_PORTA_11, 1);
        gpio_set_pull_down(IO_PORTA_11, 0);
    }

    return !gpio_read(IO_PORTA_11);

}

static void sdmmc_power(int on)
{
    gpio_direction_output(IO_PORTB_11, !on);
}

SD1_PLATFORM_DATA_BEGIN(sd1_data)
	.port 					= 'A',
	.priority 				= 3,
	.data_width 			= 4,
	.speed 					= 40000000,
	.detect_mode 			= SD_CMD_DECT,
	.detect_func 			= NULL,
SD1_PLATFORM_DATA_END()

#endif //CONFIG_SD1_ENABLE

#ifdef CONFIG_SD2_ENABLE

int sdmmc_2_io_detect(const struct sdmmc_platform_data *data)
{
#define SD_IO_DECT_PIN IO_PORTH_12
    static u8 init = 0;
    if (!init) {
        init = 1;
        gpio_direction_input(SD_IO_DECT_PIN);
        gpio_set_pull_up(SD_IO_DECT_PIN, 1);
        gpio_set_pull_down(SD_IO_DECT_PIN, 0);
    }

    return !gpio_read(SD_IO_DECT_PIN);
}

SD2_PLATFORM_DATA_BEGIN(sd2_data)
	.port 					= 'A',
	.priority 				= 3,
	.data_width 			= 4,
	.speed 					= 40000000,
	.detect_mode 			= SD_IO_DECT,
	.detect_func 			= sdmmc_2_io_detect,
SD2_PLATFORM_DATA_END()

#endif //CONFIG_SD2_ENABLE

//#if 0
SW_IIC_PLATFORM_DATA_BEGIN(hw_iic0_data)
	.clk_pin = IO_PORTA_06,
	.dat_pin = IO_PORTA_08,
	.sw_iic_delay = 100,
SW_IIC_PLATFORM_DATA_END()
//#else
//HW_IIC0_PLATFORM_DATA_BEGIN(hw_iic0_data)
//	.clk_pin = IO_PORTA_06,
//	.dat_pin = IO_PORTA_08,
//	.baudrate = 0x3f,//3f:385k
//HW_IIC0_PLATFORM_DATA_END()
//#endif

//#if 0
//HW_IIC1_PLATFORM_DATA_BEGIN(hw_iic0_data)
//	.clk_pin = IO_PORTA_06,//IO_PORTD_14,
//	.dat_pin = IO_PORTA_08,//IO_PORTD_15,
//	.baudrate = 0x1f,//300k  0x50 250k
//HW_IIC1_PLATFORM_DATA_END()
//#endif

//SW_IIC_PLATFORM_DATA_BEGIN(sw_iic_data)
//	.clk_pin = IO_PORTD_01,
//	.dat_pin = IO_PORTD_02,
//	.sw_iic_delay = 50,
//SW_IIC_PLATFORM_DATA_END()

//SW_IIC_PLATFORM_DATA_BEGIN(sw_iic1_data)
//	.clk_pin = IO_PORTF_07,
//	.dat_pin = IO_PORTF_02,
//	.sw_iic_delay = 50,
//SW_IIC_PLATFORM_DATA_END()



#ifdef CONFIG_DISPLAY_ENABLE

LCD_PLATFORM_DATA_BEGIN(lcd_data)
	.interface = LCD_MIPI,
	.lcd_io = {
        .backlight = IO_PORTH_08,
        .backlight_value = 1,

		.lcd_reset = IO_PORTH_07,
        .lcd_cs    = -1,
        .lcd_lane  = -1,
        .lcd_rs    = -1,
        .lcd_spi_ck= -1,
        .lcd_spi_di= -1,
        .lcd_spi_do= -1,
	}
LCD_PLATFORM_DATA_END()

#endif


#ifdef CONFIG_TOUCH_PANEL_ENABLE

extern const struct device_operations touch_panel_dev_ops;

SW_TOUCH_PANEL_PLATFORM_DATA_BEGIN(touch_panel_data)
    .enable         = 1,
    .iic_dev        = "iic1",
    .rst_pin        = IO_PORTA_09,
    .int_pin        = IO_PORTA_07,
    ._MAX_POINT     = 1,
    ._MAX_X         = 480,
    ._MAX_Y         = 800,
    ._INT_TRIGGER   = 3,
    ._X2Y_EN        = 0,
    ._X_MIRRORING   = 1,
    ._Y_MIRRORING   = 1,
    ._DEBUGP        = 0,
    ._DEBUGE        = 0,
    .points         ={
        .point_num  = 0,
    }
SW_TOUCH_PANEL_PLATFORM_DATA_END()
#endif //CONFIG_TOUCH_PANEL_ENABLE


#ifdef CONFIG_VIDEO0_ENABLE

static const struct camera_platform_data camera0_data = {
    .xclk_gpio      = -1,
	.reset_gpio     = IO_PORTD_00,
    .online_detect  = NULL,
	.pwdn_gpio      = -1,
    .power_value    = 0,
	.interface      = SEN_INTERFACE0,//SEN_INTERFACE_CSI2,
    .dvp={
        .pclk_gpio   = IO_PORTD_03,
        .hsync_gpio  = IO_PORTD_04,
        .vsync_gpio  = IO_PORTD_05,
		.io_function_sel = DVP_SENSOR0(0),
        .data_gpio={
                IO_PORTD_15,
                IO_PORTD_14,
                IO_PORTD_13,
                IO_PORTD_12,
                IO_PORTD_11,
                IO_PORTD_10,
                IO_PORTD_09,
                IO_PORTD_08,
                IO_PORTD_07,
                IO_PORTD_06,
        },
    }
};

static const struct video_subdevice_data video0_subdev_data[] = {
    { VIDEO_TAG_CAMERA, (void *)&camera0_data },
};
static const struct video_platform_data video0_data = {
    .data = video0_subdev_data,
    .num = ARRAY_SIZE(video0_subdev_data),
};

#endif



#ifdef CONFIG_VIDEO1_ENABLE

static bool camera1_online_detect()
{
    static u8 init = 0;
#define AVIN_DET_PIN IO_PORTH_13
    if (!init) {
        init = 1;
        gpio_direction_input(AVIN_DET_PIN);
        gpio_set_pull_up(AVIN_DET_PIN, 0);
        gpio_set_pull_down(AVIN_DET_PIN, 0);
    }

    return !gpio_read(AVIN_DET_PIN);
}

static const struct camera_platform_data camera1_data = {
    .xclk_gpio      = -1,
	.reset_gpio     = IO_PORTF_01,
	.pwdn_gpio      = -1,
    .power_value    = 1,
	.interface      = SEN_INTERFACE1,
    .online_detect  = camera1_online_detect,
    .dvp = {
        .pclk_gpio  = IO_PORTF_04,
        .hsync_gpio = -1,
        .vsync_gpio = -1,
		.io_function_sel = DVP_SENSOR1(1),
        .data_gpio  = {
            -1,//IO_PORTG_06,
            -1,//IO_PORTG_05,
            IO_PORTG_04,
            IO_PORTG_03,
            IO_PORTG_02,
            IO_PORTG_01,
            IO_PORTG_00,
            IO_PORTF_07,
            IO_PORTF_06,
            IO_PORTF_05,
        },
    }
};

static const struct video_subdevice_data video1_subdev_data[] = {
    { VIDEO_TAG_CAMERA, (void *)&camera1_data },
};
static const struct video_platform_data video1_data = {
    .data = video1_subdev_data,
    .num = ARRAY_SIZE(video1_subdev_data),
};
#endif


#ifdef CONFIG_VIDEO2_ENABLE


UVC_PLATFORM_DATA_BEGIN(uvc_data)
    .width = 640,
    .height = 480,
    .fps = 25,
    .mem_size = 1*1024 * 1024,
    .timeout = 3000,//ms
    .put_msg = 0,
UVC_PLATFORM_DATA_END()

static const struct video_subdevice_data video2_subdev_data[] = {
    { VIDEO_TAG_UVC, (void *)&uvc_data },
};
static const struct video_platform_data video2_data = {
    .data = video2_subdev_data,
    .num = ARRAY_SIZE(video2_subdev_data),
};

#endif

USB_PLATFORM_DATA_BEGIN(usb_data)
    .id = 0,
    .online_check_cnt = 5,
    .offline_check_cnt = 20,//250
    .isr_priority = 6,
    .host_ot = 20,
    .host_speed = 1,
    .slave_ot = 10,
    .ctl_irq_int = HUSB_CTL_INT,
USB_PLATFORM_DATA_END()


#ifdef CONFIG_ADKEY_ENABLE
/*-------------ADKEY GROUP 1----------------*/
#define ADC0_33   (0x3FF)
#define ADC0_30   (0x3ff*30/33) //0x3A2
#define ADC0_27   (0x3ff*27/33) //0x345
#define ADC0_23   (0x3ff*23/33) //0x2C9
#define ADC0_20   (0x3ff*20/33) //0x26C
#define ADC0_17   (0x3ff*17/33) //0x1F0
#define ADC0_13   (0x3ff*13/33) //0x193
#define ADC0_12   (0x3ff*11/33) //
#define ADC0_10   (0x3ff*10/33) //
#define ADC0_07   (0x3ff*07/33) //0x136
#define ADC0_04   (0x3ff*04/33) //0x136
#define ADC0_03   (0x3ff*03/33) //0xD9
#define ADC0_02   (0x3ff*02/33) //0xD9
#define ADC0_01   (0x3ff*01/33) //0x7C
#define ADC0_00   (0)

#define ADKEY_V_0      	((ADC0_33 + ADC0_30)/2)
#define ADKEY_V_1 		((ADC0_30 + ADC0_27)/2)
#define ADKEY_V_2 		((ADC0_27 + ADC0_23)/2)
#define ADKEY_V_3 		((ADC0_23 + ADC0_20)/2)
#define ADKEY_V_4 		((ADC0_20 + ADC0_17)/2)
#define ADKEY_V_5 		((ADC0_20 + ADC0_13)/2)//talk
#define ADKEY_V_6 		((ADC0_13 + ADC0_07)/2)//ok 1.4
#define ADKEY_V_7 		((ADC0_10 + ADC0_07)/2)//down 1.0
#define ADKEY_V_8 		((ADC0_07 + ADC0_01)/2)//up 0.7
#define ADKEY_V_9 		((ADC0_02 + ADC0_02)/2)//menu/mode 0.4
#define ADKEY_V_10 		(ADC0_00)//menu

//五个按键：OK ,  MEN/MODE, POWER,  UP,  DOWN
ADKEY_PLATFORM_DATA_BEGIN(adkey_data)
	.io 		= -1,
	.ad_channel = 0,
	.table 	= {
		.ad_value = {
			ADKEY_V_0,
			ADKEY_V_1,
			ADKEY_V_2,
			ADKEY_V_3,
			ADKEY_V_4,
			ADKEY_V_5,
			ADKEY_V_6,
			ADKEY_V_7,
			ADKEY_V_8,
			ADKEY_V_9,
			ADKEY_V_10,
		},
		.key_value = {
			NO_KEY,    /*0*/
			NO_KEY,
			NO_KEY,
			NO_KEY,
			NO_KEY,
			NO_KEY,  /*5*/
			NO_KEY,
			NO_KEY,
			NO_KEY,
			NO_KEY,//long KEY_MENU,// kick KEY_MODE
			NO_KEY,//,
		},
	},
ADKEY_PLATFORM_DATA_END()
int key_event_remap(struct sys_event *e)
{
    if(e->u.key.event == KEY_EVENT_HOLD) {
        return false;
    }

    if (e->u.key.value == KEY_POWER) {
        if(e->u.key.event == KEY_EVENT_HOLD) {
            return false;
        }
        if (e->u.key.event == KEY_EVENT_CLICK) {
            e->u.key.value = KEY_OK;
            e->u.key.event = KEY_EVENT_CLICK;
            return true;
        }
    }
	return true;
}


#endif


#ifdef CONFIG_IOKEY_ENABLE
/*
 * power键
 */
#define POWER_PIN IO_PORTA_02
const struct iokey_port iokey_list[] = {
    {
        .port = POWER_PIN,
        .press_value = 1,
        .key_value = KEY_POWER,
    }
};

const struct iokey_platform_data iokey_data = {
    .num = ARRAY_SIZE(iokey_list),
    .port = iokey_list,
};

unsigned char read_power_key()
{
    gpio_set_pull_down(POWER_PIN,1);
	gpio_set_pull_up(POWER_PIN,0);
	return (gpio_read(POWER_PIN));
}
#else

unsigned char read_power_key()
{
    return 0;
}

#endif


/*
 * spi0接falsh
 */
#ifdef CONFIG_PSRAM_ENABLE
SPI0_PLATFORM_DATA_BEGIN(spi0_data)
	.clk    = 100000000,
	.mode   = SPI_QUAD_MODE,
	.port   = 'B',
SPI0_PLATFORM_DATA_END()
#else

SPI0_PLATFORM_DATA_BEGIN(spi0_data)
	.clk    = 40000000,
	.mode   = SPI_DUAL_MODE,
	.port   = 'A',
SPI0_PLATFORM_DATA_END()
#endif




	/*
	 * spi1
	 */
SPI1_PLATFORM_DATA_BEGIN(spi1_data)
	.clk    = 48000000,
	.mode   = SPI_DUAL_MODE,
	.port   = 'D',
SPI1_PLATFORM_DATA_END()

SFC_SPI_PLATFORM_DATA_BEGIN(sfc_spi_data)
    .clk        = 20000000,
    .mode       = SPI_DUAL_MODE,
    .ro_mode    = FAST_READ_OUTPUT_MODE,
SFC_SPI_PLATFORM_DATA_END()

const struct spiflash_platform_data spiflash_data = {
#ifdef CONFIG_PSRAM_ENABLE
	.name           = "sfc_spi0",
#else
    .name           = "spi0",
#endif
	.mode           = FAST_READ_OUTPUT_MODE,//FAST_READ_IO_MODE,
//	.sfc_run_mode   = SFC_FAST_READ_DUAL_OUTPUT_MODE,
};

#ifdef CONFIG_PSRAM_ENABLE
const struct spiram_platform_data psram_data = {
    .name           = "spi0",
    .mode           = FAST_READ_OUTPUT_MODE,
};
#endif

const struct dac_platform_data dac_data = {
    .ldo_id = 1,
    .pa_mute_port = 0xff,
    .pa_mute_value = 0,
    .differ_output = 1,
};

const struct adc_platform_data adc_data = {
    .mic_channel = LADC_CH_MIC_R,
    .linein_channel = LADC_LINE0_MASK,
	.ldo_sel = 1,
};

const struct audio_pf_data audio_pf_d = {
	.adc_pf_data = &adc_data,
	.dac_pf_data = &dac_data,
};
const struct audio_platform_data audio_data = {
	.private_data = (void *)&audio_pf_d,
};

USB_CAMERA_PLATFORM_DATA_BEGIN(usb_camera0_data)
    .open_log = 1,
USB_CAMERA_PLATFORM_DATA_END()

USB_CAMERA_PLATFORM_DATA_BEGIN(usb_camera1_data)
    .open_log = 1,
USB_CAMERA_PLATFORM_DATA_END()

#ifdef CONFIG_GSENSOR_ENABLE

const struct gsensor_platform_data gsensor_data = {
    .iic = "iic0",
};


#endif // CONFIG_GSENSOR_ENABLE


#ifdef CONFIG_AV10_SPI_ENABLE
extern const struct device_operations _spi_dev_ops;
//以下io为临时配置，还需根据原理图来调整
SW_SPI_PLATFORM_DATA_BEGIN(sw_spi_data)
	.pin_cs = IO_PORTG_05,
	.pin_clk = IO_PORTG_06,
	.pin_in  = IO_PORTG_07,
	.pin_out = IO_PORTG_07,
SW_SPI_PLATFORM_DATA_END()
#endif // CONFIG_AV10_SPI_ENABLE


#ifdef CONFIG_WIFI_ENABLE
WIFI_PLATFORM_DATA_BEGIN(wifi_data)
#ifdef CONFIG_USB_WIFI_ENABLE
    .module = RTL8188E,
    .sdio_parm = 0,
    .wakeup_port = -1,
    .cs_port = -1,
    .power_port = -1,
#else
    .module = RTL8189E,
    //.module = HI3861L,
#ifdef CONFIG_NET_TCP_ENABLE /*V1.0.6-P4以上,TCP图传需要开轮训,才能解决速度慢问题*/
    .sdio_parm = SDIO_GRP_1 | SDIO_PORT_3 | SDIO_4_BIT_DATA | SDIO_CLOCK_26M,
#else
    .sdio_parm = SDIO_GRP_1 | SDIO_PORT_3 | SDIO_4_BIT_DATA | SDIO_CLOCK_26M | SDIO_DATA1_IRQ,
    //.sdio_parm = SDIO_GRP_1|SDIO_PORT_2| SDIO_1_BIT_DATA /*|SDIO_DATA1_IRQ */ | 20 * 1000000,
#endif
    .wakeup_port = -1,//IO_PORTG_07,
    .cs_port = -1,//IO_PORTB_12,
    .power_port = -1,//IO_PORTG_07,
#endif
WIFI_PLATFORM_DATA_END()

u32 get_wifi_sdio_parm(void)
{
    return wifi_data.sdio_parm;
}


void hi3861l_reset(void)
{
    printf("%s\n", __func__);
    gpio_direction_output(IO_PORTD_12, 0); //让Hi3861L复位
}


int get_wifi_module_support_mode(void)
{
	int ret = (wifi_data.module == RTL8822B || wifi_data.module == RTL8822C);
	return ret;
}
#endif

void av_parking_det_init()
{
//    gpio_direction_input(IO_PORTA_09);
}

unsigned char av_parking_det_status()
{
    return 0;
//    return (!gpio_read(IO_PORTA_09));
}
unsigned char PWR_CTL(unsigned char on_off)
{
    return 0;
}

#define USB_WKUP_IO 	IO_PORT_PR_02
#define TP_WKUP_IO 	IO_PORT_PR_01
unsigned char usb_is_charging()
{
#if 1
	static unsigned char init = 0;
	if (!init){
		init = 1;
		gpio_direction_input(USB_WKUP_IO);
		gpio_set_pull_up(USB_WKUP_IO, 0);
		gpio_set_pull_down(USB_WKUP_IO, 0);
		gpio_set_die(USB_WKUP_IO, 1);
		delay(10);
	}

	return (gpio_read(USB_WKUP_IO));//no usb charing == false
#else
	return 1;
#endif
}

unsigned int get_usb_wkup_gpio()
{
	return (USB_WKUP_IO);
}

POWER_PLATFORM_DATA_BEGIN(sys_power_data)
    .wkup_map = {
        {"wkup_tp", WKUP_IO_PR1, 0},
        {"wkup_usb", WKUP_IO_PR2, 0},
        {0, 0, 0}
    },
    .voltage_table = {
        {330, 10},
        {345, 20},
        {351, 30},
        {356, 40},
        {359, 50},
        {367, 60},
        {370, 70},
        {377, 80},
        {388, 90},
        {400, 100},
    },
    .min_bat_power_val = 320,
    .max_bat_power_val = 400,
    .charger_online = usb_is_charging,
    .charger_gpio  = get_usb_wkup_gpio,
    .read_power_key = read_power_key,
    .pwr_ctl = PWR_CTL,
POWER_PLATFORM_DATA_END()

#ifdef CONFIG_PWM_ENABLE
PWM_PLATFORM_DATA_BEGIN(pwm_data)
	.port = PWM_PORTH,
PWM_PLATFORM_DATA_END()
#endif

REGISTER_DEVICES(device_table) = {

#ifdef CONFIG_PAP_ENABLE
    { "pap",   &pap_dev_ops, NULL},
#endif

#ifdef CONFIG_DISPLAY_ENABLE
	{ "imr",   &imr_dev_ops, NULL},
	{ "imd",   &imd_dev_ops, NULL},
    /*{ "mipi",   &mipi_dev_ops, NULL},*/
    { "lcd",   &lcd_dev_ops, (void*)&lcd_data},

	{ "fb0",  &fb_dev_ops, NULL },
	{ "fb1",  &fb_dev_ops, NULL },
    { "fb2",  &fb_dev_ops, NULL },
    { "fb4",  &fb_dev_ops, NULL },
#endif

//    { "iic0",  &iic_dev_ops, (void *)&hw_iic0_data },
	/* { "iic0",  &iic_dev_ops, (void *)&sw_iic_data }, */
    /*{ "iic1",  &iic_dev_ops, (void *)&sw_iic_data },*/
#ifdef CONFIG_TOUCH_PANEL_ENABLE
    { "iic1",  &iic_dev_ops, (void *)&hw_iic0_data },
    { "touch_panel", &touch_panel_dev_ops, (void *)&touch_panel_data},
#endif //CONFIG_TOUCH_PANEL_ENABLE


//    { "audio", &audio_dev_ops, (void *)&audio_data },

#ifdef CONFIG_AV10_SPI_ENABLE
    { "avin_spi",  &_spi_dev_ops, (void *)&sw_spi_data },
#endif

#ifdef CONFIG_SD0_ENABLE
    { "sd0",  &sd_dev_ops, (void *)&sd0_data },
#endif

#ifdef CONFIG_SD1_ENABLE
   { "sd1",  &sd_dev_ops, (void *)&sd1_data },
#endif

#ifdef CONFIG_SD2_ENABLE
    { "sd2",  &sd_dev_ops, (void *)&sd2_data },
#endif

#ifdef CONFIG_ADKEY_ENABLE
    { "adkey", &key_dev_ops, (void *)&adkey_data },
#endif
    /*{ "uart_key", &key_dev_ops, NULL },*/

#ifdef CONFIG_IOKEY_ENABLE
    { "iokey", &key_dev_ops, (void *)&iokey_data },
#endif

#ifdef CONFIG_VIDEO0_ENABLE
    { "video0.*",  &video_dev_ops, (void *)&video0_data},
#else
    { "video0.*",  &video_dev_ops, (void*)NULL},
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    { "video1.*",  &video_dev_ops, (void *)&video1_data },
#endif

#ifdef CONFIG_VIDEO2_ENABLE
    { "video2.*",  &video_dev_ops, (void *)&video2_data },
#endif

#ifdef CONFIG_VIDEO5_ENABLE
    { "video5.*",  &video_dev_ops, (void *)NULL },
#endif

#ifdef CONFIG_VIDEO_DEC_ENABLE
    { "video_dec",  &video_dev_ops, NULL },
#endif

#ifndef CONFIG_SFC_ENABLE
    { "spi0", &spi_dev_ops, (void *)&spi0_data },
#ifdef CONFIG_PSRAM_ENABLE
    { "sfc_spi0", &sfc_spi_dev_ops, (void *)&sfc_spi_data },
    { "spiram",   &spiram_dev_ops,  (void *)&psram_data },
#endif
    { "spiflash", &spiflash_dev_ops, (void *)&spiflash_data },

#else
	{ "spiflash", &sfcflash_dev_ops, (void *)&spiflash_data },
#endif


#ifdef CONFIG_ETH_PHY_ENABLE
    /* { "net-phy",  &eth_phy_dev_ops, (void *) &net_phy_data}, */
#endif

	{ "usb_cam0",  &usb_cam_dev_ops, (void *)&usb_camera0_data },
	{ "usb_cam1",  &usb_cam_dev_ops, (void *)&usb_camera1_data },

#ifdef CONFIG_GSENSOR_ENABLE
    {"gsensor", &gsensor_dev_ops, (void *)&gsensor_data},
#endif

#ifdef CONFIG_WIFI_ENABLE
	{ "wifi",  &wifi_dev_ops, (void *) &wifi_data},
#endif
    {"rtc", &rtc_dev_ops, NULL},
    {"vm",  &vm_dev_ops, NULL},

    {"uvc", &uvc_dev_ops, NULL},

#ifdef CONFIG_USB_WIFI_ENABLE
    {"usb0", &usbwifi_dev_ops, (void *)&usb_data},
#else
    {"usb0", &usb_dev_ops, (void *)&usb_data},
#endif


#ifdef CONFIG_PWM_ENABLE
	{"pwm",  &pwm_dev_ops, (void *)&pwm_data},
#endif
	{"uart0",&uart_dev_ops,(void *)&uart0_data},
//	{"uart2",&uart_dev_ops,(void *)&uart2_data},

};

// *INDENT-ON*

#ifdef CONFIG_DEBUG_ENABLE
void debug_uart_init()
{
    /*CLK_CON1 |= BIT(5);*/
    uart_init(&uart2_data);
}
#endif


void spec_uart_init();
void board_init()
{

    //LDO输出脚
//    gpio_direction_input(IO_PORTF_00);
//    gpio_direction_input(IO_PORTF_01);

    //关闭jtag
    SDTAP_CON = 0;

    //关闭PIN RST
    //临时关闭，打开会4秒复位
    rtc_pin_reset_ctrl(0);

    // sd power ctrl pin
//    gpio_direction_output(IO_PORTB_11, 0);
#ifdef CONFIG_VIDEO2_ENABLE
    //UVC 后拉供电控制
    /* gpio_direction_output(IO_PORTH_12, 1); */
#endif

    LDO_CON |= (BIT(10) | BIT(9));//2.8V 1.8V输出使能
//    LDO_CON &= ~BIT(6);
    //mipi_phy_con0 &= ~BIT(23);//增加这一句 关闭mipi ldo
    //
    devices_init();

    spec_uart_init();

}
//int spiflash_set_size(u32 flash_id)
//{
//    return 1 * 1024 * 1024;
//}
#endif
