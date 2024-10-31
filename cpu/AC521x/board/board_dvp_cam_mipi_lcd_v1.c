#include "app_config.h"

#ifdef CONFIG_BOARD_DVP_CAM_MIPI_LCD_V1
#include "system/includes.h"
#include "gSensor_manage.h"
#include "device/av10_spi.h"
#include "vm_api.h"

// *INDENT-OFF*


#ifdef CONFIG_PWM_ENABLE
#include "asm/pwm.h"
#endif

UART0_PLATFORM_DATA_BEGIN(uart0_data)
	.baudrate = 115200,//460800,//115200,
    .tx_pin = IO_PORTA_06,
	.flags = UART_DEBUG,
	UART0_PLATFORM_DATA_END();

UART1_PLATFORM_DATA_BEGIN(uart1_data)
	.baudrate = 115200,//460800,//115200,
	.tx_pin = IO_PORTH_09,
	.rx_pin = IO_PORTH_10,
	.flags = UART_DEBUG,
UART1_PLATFORM_DATA_END();

UART2_PLATFORM_DATA_BEGIN(uart2_data)
	.baudrate = 460800,
    .tx_pin = IO_PORTA_06,
	.flags = UART_DEBUG,
UART2_PLATFORM_DATA_END();

UART3_PLATFORM_DATA_BEGIN(uart3_data)
	.baudrate = 115200,
    .tx_pin = IO_PORTA_06,
    .rx_pin = IO_PORTA_07,
    .flags = UART_DEBUG,
UART3_PLATFORM_DATA_END();


#ifdef CONFIG_SD0_ENABLE

int sdmmc_0_io_detect(const struct sdmmc_platform_data *data)
{
    static u8 init = 0;

    if (!init) {
        init = 1;
        gpio_direction_input(IO_PORTH_12);
        gpio_set_pull_up(IO_PORTH_12, 1);
        gpio_set_pull_down(IO_PORTH_12, 0);
    }

    return !gpio_read(IO_PORTH_12);
}

SD0_PLATFORM_DATA_BEGIN(sd0_data)
	.port 					= 'B',
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
	.detect_mode 			= SD_IO_DECT,
	.detect_func 			= sdmmc_1_io_detect,
    /*.power                  = sdmmc_power,*/
SD1_PLATFORM_DATA_END()

#endif //CONFIG_SD1_ENABLE

#ifdef CONFIG_SD2_ENABLE

int sdmmc_2_io_detect(const struct sdmmc_platform_data *data)
{

    return 1;
}
SD2_PLATFORM_DATA_BEGIN(sd2_data)
	.port 					= 'B',
	.priority 				= 3,
	.data_width 			= 4,
	.speed 					= 40000000,
	.detect_mode 			= SD_IO_DECT,
	.detect_func 			= sdmmc_2_io_detect,
SD2_PLATFORM_DATA_END()

#endif //CONFIG_SD2_ENABLE

HW_IIC0_PLATFORM_DATA_BEGIN(hw_iic0_data)
	.clk_pin = IO_PORTD_01,
	.dat_pin = IO_PORTD_02,
	.baudrate = 0x1f,//300k  0x50 250k
HW_IIC0_PLATFORM_DATA_END()

#if 0
HW_IIC1_PLATFORM_DATA_BEGIN(hw_iic1_data)
	.clk_pin = IO_PORTB_00,//IO_PORTD_14,
	.dat_pin = IO_PORTB_01,//IO_PORTD_15,
	.baudrate = 0x1f,//300k  0x50 250k
HW_IIC1_PLATFORM_DATA_END()
#endif

SW_IIC_PLATFORM_DATA_BEGIN(sw_iic_data)
	.clk_pin = IO_PORTD_01,
	.dat_pin = IO_PORTD_02,
	.sw_iic_delay = 50,
SW_IIC_PLATFORM_DATA_END()


LCD_PLATFORM_DATA_BEGIN(lcd_data)
	.interface = LCD_MIPI,
	.lcd_io = {
        .backlight = IO_PORTG_02,
        .backlight_value = 1,

		.lcd_reset = IO_PORTG_04,
        .lcd_cs    = -1,
        .lcd_rs    = -1,
        .lcd_spi_ck= -1,
        .lcd_spi_di= -1,
        .lcd_spi_do= -1,
	}
LCD_PLATFORM_DATA_END()





#ifdef CONFIG_VIDEO0_ENABLE

static const struct camera_platform_data camera0_data = {
    .xclk_gpio      = IO_PORTH_13,
	.reset_gpio     = IO_PORTD_00,
    .online_detect  = NULL,
	.pwdn_gpio      = IO_PORTA_12,
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

    return 1;

    if (!init) {
        init = 1;
        gpio_direction_input(IO_PORTA_10);
        gpio_set_pull_up(IO_PORTA_10, 0);
        gpio_set_pull_down(IO_PORTA_10, 0);
    }

    return !gpio_read(IO_PORTA_10);
}

static const struct camera_platform_data camera1_data = {
    .xclk_gpio      = -1,
	.reset_gpio     = IO_PORTA_11,
	.pwdn_gpio      = -1,//IO_PORTB_12,
    .power_value    = 1,
	.interface      = SEN_INTERFACE1,
    .online_detect  = camera1_online_detect,
    .dvp = {
        .pclk_gpio  = IO_PORTF_04,
        .hsync_gpio = IO_PORTF_02,
        .vsync_gpio = IO_PORTF_03,
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
    .width = 1280,//1280,
    .height = 720,//480,
    .fps = 25,
    .mem_size = 1 * 1024 * 1024,
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
    .online_check_cnt = 3,
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
#define ADC0_14   (0x3ff*14/33) //0x193
#define ADC0_13   (0x3ff*13/33) //0x136
#define ADC0_10   (0x3ff*10/33) //0xD9
#define ADC0_07   (0x3ff*07/33) //0xD9
#define ADC0_04   (0x3ff*04/33) //0x7C
#define ADC0_00   (0)

#define ADKEY_V_0      	((ADC0_33 + ADC0_30)/2)
#define ADKEY_V_1 		((ADC0_30 + ADC0_27)/2)
#define ADKEY_V_2 		((ADC0_27 + ADC0_23)/2)
#define ADKEY_V_3 		((ADC0_23 + ADC0_20)/2)
#define ADKEY_V_4 		((ADC0_20 + ADC0_17)/2)
#define ADKEY_V_5 		((ADC0_17 + ADC0_13)/2)
#define ADKEY_V_6 		((ADC0_13 + ADC0_10)/2)//up 0.7
#define ADKEY_V_7 		((ADC0_10 + ADC0_07)/2)//down 0.4
#define ADKEY_V_8 		((ADC0_07 + ADC0_04)/2)//ok 0.2
#define ADKEY_V_9 		((ADC0_04 + ADC0_00)/2)//menu/mode 0.1
#define ADKEY_V_10 		(ADC0_00)

//五个按键：OK ,  MEN/MODE, POWER,  UP,  DOWN
ADKEY_PLATFORM_DATA_BEGIN(adkey_data)
	.io 		= IO_PORTA_05,
	.ad_channel = 1,
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
			NO_KEY,  /*0*/
			NO_KEY,
			NO_KEY,
			NO_KEY,
			NO_KEY,
			NO_KEY,  /*5*/
			KEY_MODE,
			KEY_DOWN,
			KEY_OK,
			KEY_UP,//long KEY_MENU,// kick KEY_MODE
			KEY_MENU,//,
		},
	},
ADKEY_PLATFORM_DATA_END()
int key_event_remap(struct sys_event *e)
{
	static u8 m_flag = 0;

    if (e->u.key.value == KEY_MODE) {
	   	if(e->u.key.event == KEY_EVENT_HOLD) {
			return false;
		}

		if (e->u.key.event == KEY_EVENT_LONG) {
			m_flag = 1;
			e->u.key.value = KEY_MENU;
			e->u.key.event = KEY_EVENT_CLICK;
		}
    }
	return true;
}


#endif


#ifdef CONFIG_IOKEY_ENABLE
/*
 * power键
 */
const struct iokey_port iokey_list[] = {
    {
        .port = IO_PORTA_13,
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
	gpio_direction_input(IO_PORTA_13);
	return (gpio_read(IO_PORTA_13));
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
SPI0_PLATFORM_DATA_BEGIN(spi0_data)
	.clk    = 20000000,
	.mode   = SPI_DUAL_MODE,
	.port   = 'A',
SPI0_PLATFORM_DATA_END()
	/*
	 * spi1
	 */
SPI1_PLATFORM_DATA_BEGIN(spi1_data)
	.clk    = 10000000,
	.mode   = SPI_ODD_MODE,
	.port   = 'D',
SPI1_PLATFORM_DATA_END()

const struct spiflash_platform_data spiflash_data = {
	.name           = "spi0",
	.mode           = FAST_READ_OUTPUT_MODE,//FAST_READ_IO_MODE,
};





const struct dac_platform_data dac_data = {
    .ldo_id = 1,
};

const struct adc_platform_data adc_data = {
    .ladc_in_channel = LADC_CH_MIC_R,
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
extern const struct device_operations gsensor_dev_ops;
#endif // CONFIG_GSENSOR_ENABLE


#ifdef CONFIG_AV10_SPI_ENABLE
extern const struct device_operations _spi_dev_ops;
//以下io为临时配置，还需根据原理图来调整
SW_SPI_PLATFORM_DATA_BEGIN(sw_spi_data)
	.pin_cs = IO_PORTD_02,
	.pin_clk = IO_PORTD_00,
	.pin_in  = IO_PORTD_01,
	.pin_out = IO_PORTD_01,
SW_SPI_PLATFORM_DATA_END()
#endif // CONFIG_AV10_SPI_ENABLE



void av_parking_det_init()
{
    gpio_direction_input(IO_PORTA_09);
}

unsigned char av_parking_det_status()
{
    return (!gpio_read(IO_PORTA_09));
}
unsigned char PWR_CTL(unsigned char on_off)
{
    return 0;
}

#define USB_WKUP_IO 	IO_PORT_PR_01
#define GSEN_WKUP_IO 	IO_PORT_PR_02
unsigned char usb_is_charging()
{
#if 0
	static unsigned char init = 0;
    return 1;
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

POWER_PLATFORM_DATA_BEGIN(sys_power_data)
    .wkup_map = {
        {"wkup_gsen", WKUP_IO_PR2, 0},
        {"wkup_usb", WKUP_IO_PR1, 0},
        {0, 0, 0}
    },
    .voltage_table = {
        {365, 10},
        {370, 20},
        {373, 30},
        {376, 40},
        {379, 50},
        {387, 60},
        {390, 70},
        {397, 80},
        {408, 90},
        {422, 100},
    },
    .min_bat_power_val = 350,
    .max_bat_power_val = 420,
    .charger_online = usb_is_charging,
    .read_power_key = read_power_key,
    .pwr_ctl = PWR_CTL,
POWER_PLATFORM_DATA_END()

unsigned int get_usb_wkup_gpio()
{
	return (USB_WKUP_IO);
}

#ifdef CONFIG_PWM_ENABLE
PWM_PLATFORM_DATA_BEGIN(pwm_data)
	.port = PWM_PORTF,
PWM_PLATFORM_DATA_END()
#endif

const struct device_operations pap_dev_ops;

REGISTER_DEVICES(device_table) = {

#ifdef CONFIG_PAP_ENABLE
    { "pap",   &pap_dev_ops, NULL},
#endif
	{ "imr",   &imr_dev_ops, NULL},
	{ "imd",   &imd_dev_ops, NULL},
	/* { "mipi",   &mipi_dev_ops, NULL}, */
	{ "lcd",   &lcd_dev_ops, (void*)&lcd_data},

    { "iic0",  &iic_dev_ops, (void *)&hw_iic0_data },
    /*{ "iic0",  &iic_dev_ops, (void *)&sw_iic_data },*/
    /*{ "iic1",  &iic_dev_ops, (void *)&sw_iic_data },*/

    { "audio", &audio_dev_ops, (void *)&audio_data },

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

#ifdef CONFIG_VIDEO_DEC_ENABLE
    { "video_dec",  &video_dev_ops, NULL },
#endif

	{ "fb0",  &fb_dev_ops, NULL },
	{ "fb1",  &fb_dev_ops, NULL },
	/* { "fb2",  &fb_dev_ops, NULL }, */

//    { "videoengine",  &video_engine_ops, NULL },
#ifndef CONFIG_SFC_ENABLE
    { "spi0", &spi_dev_ops, (void *)&spi0_data },
#endif
    { "spiflash", &spiflash_dev_ops, (void *)&spiflash_data },

#ifdef CONFIG_ETH_PHY_ENABLE
    /* { "net-phy",  &eth_phy_dev_ops, (void *) &net_phy_data}, */
#endif

	{ "usb_cam0",  &usb_cam_dev_ops, (void *)&usb_camera0_data },
	{ "usb_cam1",  &usb_cam_dev_ops, (void *)&usb_camera1_data },

#ifdef CONFIG_GSENSOR_ENABLE
    {"gsensor", &gsensor_dev_ops, NULL},
#endif

    {"rtc", &rtc_dev_ops, NULL},
 //   {"vm",  &vm_dev_ops, NULL},

    {"uvc", &uvc_dev_ops, NULL},
    {"usb0", &usb_dev_ops, (void *)&usb_data},

#ifdef CONFIG_PWM_ENABLE
	{"pwm",  &pwm_dev_ops, (void *)&pwm_data},
#endif
};

// *INDENT-ON*

#ifdef CONFIG_DEBUG_ENABLE
void debug_uart_init()
{
    /*CLK_CON1 |= BIT(5);*/
#if 0
    uart_init(&uart0_data);
    /*
    if (uart0_data.tx_pin == IO_PORTG_14) {
        IOMC1 &= ~(0x0f << 8);
        gpio_direction_output(IO_PORTG_14, 0);
        gpio_set_pull_up(IO_PORTG_14, 1);
        gpio_set_pull_down(IO_PORTG_14, 1);
        gpio_set_die(IO_PORTG_14, 0);
    }
    */
    IOMC3 &= ~BIT(25);
#else
    uart_init(&uart2_data);
    if (uart2_data.tx_pin == IO_PORTA_06) {
        IOMC1 &= ~(0x0f << 8);
        IOMC1 |= (0x2 << 8);
        gpio_direction_output(IO_PORTA_06, 0);
        gpio_set_pull_up(IO_PORTA_06, 1);
        gpio_set_pull_down(IO_PORTA_06, 1);
        gpio_set_die(IO_PORTA_06, 0);
    }

#endif
}
#endif

void board_init()
{
    /*CLK_CON0 &= ~BIT(13);//uboot某测试时钟io pe2输出关断*/
    gpio_direction_output(IO_PORTH_12, 0);

    gpio_direction_input(IO_PORTF_01);

    // sd power ctrl pin
//    gpio_direction_output(IO_PORTB_11, 0);
#ifdef CONFIG_VIDEO3_ENABLE
    //UVC 后拉供电控制
    gpio_direction_output(IO_PORTB_12, 1);
#endif

//    mipi_phy_con0 &= ~BIT(23);//增加这一句 关闭mipi ldo

    devices_init();

}

#endif


