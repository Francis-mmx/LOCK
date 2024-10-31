#include "app_config.h"

#ifdef CONFIG_BOARD_GR202

#include "system/includes.h"
#include "device/av10_spi.h"
#include "asm/ldo.h"
#include "vm_api.h"
#include "device/wifi_dev.h"

// *INDENT-OFF*

UART2_PLATFORM_DATA_BEGIN(uart2_data)
    .baudrate = 460800,
    .tx_pin = IO_PORTH_00,
    .flags = UART_DEBUG,
UART2_PLATFORM_DATA_END();

#ifdef CONFIG_SD2_ENABLE

static int sd2_io_detect(const struct sdmmc_platform_data *data)
{
    return 1;
}

SD2_PLATFORM_DATA_BEGIN(sd2_data)
    .port 					= 'A',
    .priority 				= 3,
    .data_width 			= 1,
	.speed 					= 40000000,
    /*.detect_mode 			= SD_CMD_DECT,*/
    .detect_mode 			= SD_CLK_DECT,
    .detect_func 			= sdmmc_2_clk_detect,
    /*.detect_mode 			= SD_IO_DECT,
    .detect_func 			= sd2_io_detect,*/
SD2_PLATFORM_DATA_END()

#endif //CONFIG_SD2_ENABLE

HW_IIC0_PLATFORM_DATA_BEGIN(hw_iic0_data)
    .clk_pin = IO_PORTA_06,
    .dat_pin = IO_PORTA_08,
    .baudrate = 0x3f,//3f:385k
HW_IIC0_PLATFORM_DATA_END()


SW_IIC_PLATFORM_DATA_BEGIN(sw_iic_data)
    .clk_pin = IO_PORTD_01,
    .dat_pin = IO_PORTD_02,
    .sw_iic_delay = 50,
SW_IIC_PLATFORM_DATA_END()


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
enum {
    ADC_PORTA_0 = 0,
    ADC_PORTA_5 = 1,
    ADC_PORTG_6 = 2,
    ADC_PORTG_7 = 3,
    ADC_PORTH_0 = 4,
    ADC_PORTH_1 = 5,
    ADC_PORTH_5 = 6,
    ADC_PORTH_7 = 7,
    ADC_PORTH_9 = 8,
    ADC_PORTH_10 = 9,
    ADC_PORTH_12 = 10,
    ADC_PORTH_13 = 11,
    ADC_PORTD_0 = 12,
    ADC_ANALOG_TEST = 13,
    ADC_RTC = 14,
    ADC_LDO_VBG = 15,
};

/*-------------ADKEY GROUP 1----------------*/

#define ADKEY_UPLOAD_R  22

#define ADC0_0A   (0x3FF)
#define ADC0_09   (0x3FF)
#define ADC0_08   (0x3FF)
#define ADC0_07   (0x3FF)
#define ADC0_06   (0x3FF * 51  / (51  + ADKEY_UPLOAD_R))
#define ADC0_05   (0x3FF * 33  / (33  + ADKEY_UPLOAD_R))
#define ADC0_04   (0x3FF * 24  / (24  + ADKEY_UPLOAD_R))
#define ADC0_03   (0x3FF * 15  / (15  + ADKEY_UPLOAD_R))
#define ADC0_02   (0x3FF * 10  / (10  + ADKEY_UPLOAD_R))
#define ADC0_01   (0x3FF * 3   / (3   + ADKEY_UPLOAD_R))
#define ADC0_00   (0)

#define ADKEY_V_0      	((ADC0_09 + ADC0_0A)/2)
#define ADKEY_V_1 		((ADC0_08 + ADC0_09)/2)
#define ADKEY_V_2 		((ADC0_07 + ADC0_08)/2)
#define ADKEY_V_3 		((ADC0_06 + ADC0_07)/2)
#define ADKEY_V_4 		((ADC0_05 + ADC0_06)/2)
#define ADKEY_V_5 		((ADC0_04 + ADC0_05)/2)
#define ADKEY_V_6 		((ADC0_03 + ADC0_04)/2)
#define ADKEY_V_7 		((ADC0_02 + ADC0_03)/2)
#define ADKEY_V_8 		((ADC0_01 + ADC0_02)/2)
#define ADKEY_V_9 		((ADC0_00 + ADC0_01)/2)
#define ADKEY_V_10 		(ADC0_00)

//五个按键：OK ,  MEN/MODE, POWER,  UP,  DOWN
ADKEY_PLATFORM_DATA_BEGIN(adkey_data)
.io 		= IO_PORTH_01,
.ad_channel = 5,
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
        NO_KEY,    /*1*/
        NO_KEY,    /*2*/
        NO_KEY,    /*3*/
        KEY_MENU,    /*4*/
        KEY_DOWN,    /*5*/
        KEY_UP,   /*6*/
        KEY_MODE,    /*7*/
        KEY_F1,  /*8*/
        KEY_ENC,    /*9*/
        KEY_OK,  /*10*/
    },
},
ADKEY_PLATFORM_DATA_END()

int key_event_remap(struct sys_event *e)
{
    return true;
}


#endif


unsigned char read_power_key()
{
    return 0;
}

/*
 * spi0接falsh
 */
SPI0_PLATFORM_DATA_BEGIN(spi0_data)
    .clk    = 80000000,
    .mode   = SPI_ODD_MODE,
    .port   = 'A',
SPI0_PLATFORM_DATA_END()

const struct spiflash_platform_data spiflash_data = {
    .name           = "spi0",
    .mode           = FAST_READ_OUTPUT_MODE,//FAST_READ_IO_MODE,
	.sfc_run_mode   = SFC_FAST_READ_DUAL_OUTPUT_MODE,
};





const struct dac_platform_data dac_data = {
    .ldo_id = 1,
    .pa_mute_port = IO_PORTA_08,
    .pa_mute_value = 0,
    .differ_output = 1,
};

const struct adc_platform_data adc_data = {
    .mic_channel = LADC_CH_MIC_R,
    .linein_channel = LADC_CH_LINE0_L,
    .ldo_sel = 1,
};

const struct audio_pf_data audio_pf_d = {
    .adc_pf_data = &adc_data,
    .dac_pf_data = &dac_data,
};
const struct audio_platform_data audio_data = {
    .private_data = (void *) &audio_pf_d,
};

USB_CAMERA_PLATFORM_DATA_BEGIN(usb_camera0_data)
    .open_log = 1,
USB_CAMERA_PLATFORM_DATA_END()

USB_CAMERA_PLATFORM_DATA_BEGIN(usb_camera1_data)
    .open_log = 1,
USB_CAMERA_PLATFORM_DATA_END()


#ifdef CONFIG_WIFI_ENABLE
WIFI_PLATFORM_DATA_BEGIN(wifi_data)
    .module = RTL8189E,
    .sdio_parm = SDIO_GRP_1 | SDIO_PORT_2 | SDIO_1_BIT_DATA | SDIO_CLOCK_20M | SDIO_DATA1_IRQ,
    .wakeup_port = -1,//IO_PORTB_11,
    .cs_port = -1,//IO_PORTB_12,
    .power_port = -1,//IO_PORTB_13,
WIFI_PLATFORM_DATA_END()
#endif

void av_parking_det_init()
{
}

unsigned char av_parking_det_status()
{
    return  0;
}
unsigned char PWR_CTL(unsigned char on_off)
{
    return 0;
}

#define USB_WKUP_IO 	IO_PORT_PR_01
/*#define GSEN_WKUP_IO 	IO_PORT_PR_02*/
unsigned char usb_is_charging()
{
#if 0
    static unsigned char init = 0;
    if (!init) {
        init = 1;
        gpio_direction_input(USB_WKUP_IO);
        gpio_set_pull_up(USB_WKUP_IO, 0);
        gpio_set_pull_down(USB_WKUP_IO, 0);
        gpio_set_die(USB_WKUP_IO, 1);
        delay(10);
    }

    return (gpio_read(USB_WKUP_IO));//no usb charing == false
#else
    return 0;
#endif
}

unsigned int get_usb_wkup_gpio()
{
    return (USB_WKUP_IO);
}

POWER_PLATFORM_DATA_BEGIN(sys_power_data)
    .wkup_map = {
        {"wkup_usb", WKUP_IO_PR1, 0},
        /*{"wkup_gsen", WKUP_IO_PR2, 0},*/
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
    .charger_online = usb_is_charging,
    .charger_gpio  = get_usb_wkup_gpio,
    .read_power_key = read_power_key,
    .pwr_ctl = PWR_CTL,
POWER_PLATFORM_DATA_END()



REGISTER_DEVICES(device_table) = {

    { "iic0",  &iic_dev_ops, (void *) &hw_iic0_data },

    { "audio", &audio_dev_ops, (void *) &audio_data },

#ifdef CONFIG_SD2_ENABLE
    { "sd2",  &sd_dev_ops, (void *) &sd2_data },
#endif

#ifdef CONFIG_ADKEY_ENABLE
    { "adkey", &key_dev_ops, (void *) &adkey_data },
#endif

#ifndef CONFIG_SFC_ENABLE
    { "spi0", &spi_dev_ops, (void *)&spi0_data },
    { "spiflash", &spiflash_dev_ops, (void *)&spiflash_data },
#else
    { "spiflash", &sfcflash_dev_ops, (void *)&spiflash_data },
#endif

#ifdef CONFIG_WIFI_ENABLE
    { "wifi",  &wifi_dev_ops, (void *) &wifi_data},
#endif
    {"rtc", &rtc_dev_ops, NULL},
    {"vm",  &vm_dev_ops, NULL},

};


#ifdef CONFIG_DEBUG_ENABLE
void debug_uart_init()
{
    avdd18_crtl(AVDD18_18063, 0);
    uart_init(&uart2_data);
}
#endif

void board_early_init()
{
    lvd_cfg(0);
    sys_power_low_voltage_shutdown(370, 0);
}

void board_init()
{
    gpio_direction_input(IO_PORTF_01);

    //关闭jtag
    SDTAP_CON = 0;

    gpio_set_hd(IO_PORTA_04, 0);
    //关闭PIN RST
    //临时关闭，打开会4秒复位
    rtc_pin_reset_ctrl(0);

    avdd28_ctrl(AVDD28_2803, 0);

    devices_init();

}

#endif


