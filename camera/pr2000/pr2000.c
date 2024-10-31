#include "pr2000.h"
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "generic/jiffies.h"

static u8 avin_fps();
static u32 reset_gpio[2] = {-1, -1};

//0xB8
//0xB9
#define PR2000K_PSTA		1
#define PR2000K_NSTA		2

/*S_IIC_FUN pr2000k_iic_fun;*/
static void *iic = NULL;
static u8 g_pn_status = -1;

#define WRCMD 0xB8
#define RDCMD 0xB9

unsigned char wrPR2000KReg(u16 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        log_d(">>> 0\n");
        goto __wend;
    }

    delay(50);
    delay(50);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        log_d(">>> 1\n");
        goto __wend;
    }

    delay(50);
    delay(50);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        /* if (dev_ioctl(iic, IIC_IOCTL_TX, regDat)) { */
        ret = 0;
        log_d(">>> 2\n");
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

unsigned char rdPR2000KReg(u16 regID, unsigned char *regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        log_d(">>> 3\n");
        goto __rend;
    }

    delay(50);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        log_d(">>> 4\n");
        goto __rend;
    }

    delay(50);


    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, RDCMD)) {
        ret = 0;
        log_d(">>> 5\n");
        goto __rend;
    }

    delay(50);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

void pr2000k_reset(u8 isp_dev)
{
    u32 gpio;

    if (isp_dev == ISP_DEV_0) {
        gpio = reset_gpio[0];
    } else {
        gpio = reset_gpio[1];
    }

    gpio_direction_output(gpio, 1);
    delay(50000);
    gpio_direction_output(gpio, 0);
    delay(50000);
    delay(50000);
    delay(50000);
    delay(50000);
    gpio_direction_output(gpio, 1);
}

s32 pr2000k_id_check()
{
    u16 id = 0;
    u16 id1 = 0;

    delay(50000);
    /* while(1) */
    {

        wrPR2000KReg(0xff, 0x00);

        rdPR2000KReg(0xfc, (unsigned char *)&id);
        id <<= 8;
        rdPR2000KReg(0xfd, (unsigned char *)&id1);
        id |= id1;
        puts("\nid:");
        /* put_u16hex(id); */
        log_d("id 0x%x\n", id);

    }
    if (id == 0x2000) {
        puts("\npr2000k_id_check succ\n");
        return 1;
    }

    return 0;
}


/*void PR2000K_iic_set(u8 isp_dev)*/
/*{*/
/*iic_select(&pr2000k_iic_fun, isp_dev);*/
/*}*/


static u8 cur_sensor_type = 0xff;
static void av_sensor_power_ctrl(u32 _power_gpio, u32 on_off)
{
    u32 gpio = _power_gpio;
    gpio_direction_output(gpio, on_off);
    delay(5000);
}

s32 pr2000k_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio)
{
    if (isp_dev == ISP_DEV_0) {
        log_d("pr200k_check\n");
        return -1;
    }

    puts("\npr2000k_id check\n");
    /* put_u8hex(isp_dev); */
    log_d("isp_dev 0x%x\n", isp_dev);
    puts("\n");
    if (!iic) {
        if (isp_dev == ISP_DEV_0) {

            printf("~~~~~~~~~~~~~run here open iic0\r\r") ;
            iic = dev_open("iic0", 0);
        } else {
            printf("~~~~~~~~~~~~~run here open iic0\r\r") ;
            iic = dev_open("iic0", 0);
        }
        if (!iic) {
            return -1;
        }
    } else {
        if (cur_sensor_type != isp_dev) {
            return -1;
        }
    }

    reset_gpio[isp_dev] = _reset_gpio;
    av_sensor_power_ctrl(_power_gpio, 1);
    /* av_sensor_power_ctrl(_power_gpio, 0); */
    pr2000k_reset(isp_dev);
    /*PR2000K_iic_set(isp_dev);*/
#if 1
    if (0 == pr2000k_id_check()) {
        dev_close(iic);
        iic = NULL;

        puts("\npr2000k_id check fail\n\n");

        return -1;
    }
#endif


    cur_sensor_type = isp_dev;

    return 0;
}


s32 pr2000k_set_output_size(u16 *width, u16 *height, u8 *freq)
{
    /*
        if (avin_fps()) {
            *freq = 25;
            g_pn_status = PR2000K_PSTA;
        } else {
            *freq = 30;
            g_pn_status = PR2000K_NSTA;
        }
    */
//    *freq = 25;
//    *width = 1920;
//    *height = 1080;

    return 0;
}

static u8 avin_valid_signal()
{
    u8 DetVideo = 0;
    u8 LockStatus = 0;
    u8 j;
    /* wrPR2000KReg(0xff, 0x00); */
    for (j = 0; j < 10; j++) {
        wrPR2000KReg(0xff, 0x00);
        rdPR2000KReg(0x00, &DetVideo);
        rdPR2000KReg(0x01, &LockStatus);
        /* log_d(" DetVideo %x\n", DetVideo); */
        /* log_d(" LockStatus %x\n", LockStatus); */
        delay(50000);
        delay(50000);

        if ((LockStatus & 0xff) && (DetVideo & BIT(3))) {
            /* if (LockStatus & 0xff) { */
            return 1;
        }
    }
    return 0;
}

static int wait_signal_valid()
{
    u32 time;

    if (avin_valid_signal()) {
        //信号有效等50ms 待信号稳定
        time = jiffies + msecs_to_jiffies(50);
        while (1) {
            if (time_after(jiffies, time)) {
                puts("\n xxxxx pr2000k valid\n");
                return 0;
            }

        }
    } else {
        //信号无效等100ms
        time = jiffies + msecs_to_jiffies(100);
        while (!avin_valid_signal()) {
            if (time_after(jiffies, time)) {
                puts("\n xxxxx pr2000k no no no no validxx \n");
                /* return  0; */
                return -1;
            }
        }
    }

    return  0;
}

extern void pr2000k_init();
int PR2000K_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    pr2000k_init();

    if (wait_signal_valid() != 0) {
        return -1;
    }
    /* *format = SEN_IN_FORMAT_UYVY; */
    pr2000k_set_output_size(width, height, frame_freq);

    return 0;
}

s32 pr2000k_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\npr2000k_init \n");
    u8 init_cnt = 0;
__retry_init:
    if (PR2000K_config_SENSOR(width, height, format, frame_freq) != 0) {
        puts("\npr2000k_init fail\n");
        /* return -1; */
        if (++init_cnt > 5) {
            return -1;
        }
        goto __retry_init;
    }
    return 0;
}


/* static  */
void pr2000k_init()
{

//wrPR2000KReg(0xff,0x01);
//wrPR2000KReg(0x39,0x0f);

#if 1//720P 驱动
    wrPR2000KReg(0xff, 0x00);
    wrPR2000KReg(0x10, 0x82);
    wrPR2000KReg(0x11, 0x07);
    wrPR2000KReg(0x12, 0x00);
    wrPR2000KReg(0x13, 0x11); //00 11 22 33SHARPNESS
    wrPR2000KReg(0x14, 0x21);
    wrPR2000KReg(0x15, 0x44);
    wrPR2000KReg(0x16, 0x0d);
    wrPR2000KReg(0x40, 0x00);
    wrPR2000KReg(0x47, 0x3a);
    wrPR2000KReg(0x4e, 0x3f);
    wrPR2000KReg(0x80, 0x56);
    wrPR2000KReg(0x81, 0x0e);
    wrPR2000KReg(0x82, 0x0d);
    wrPR2000KReg(0x84, 0x30);
    wrPR2000KReg(0x86, 0x30);
    wrPR2000KReg(0x87, 0x00);
    wrPR2000KReg(0x8a, 0x00);
    wrPR2000KReg(0x90, 0x00);
    wrPR2000KReg(0x91, 0x00);
    wrPR2000KReg(0x92, 0x00);
    wrPR2000KReg(0x94, 0xff);
    wrPR2000KReg(0x95, 0xff);
    wrPR2000KReg(0x96, 0xff);
    wrPR2000KReg(0xa0, 0x00);
    wrPR2000KReg(0xa1, 0x20);
    wrPR2000KReg(0xa4, 0x01);
    wrPR2000KReg(0xa5, 0xe3);
    wrPR2000KReg(0xa6, 0x00);
    wrPR2000KReg(0xa7, 0x12);
    wrPR2000KReg(0xa8, 0x00);
    wrPR2000KReg(0xd0, 0x30);
    wrPR2000KReg(0xd1, 0x08);
    wrPR2000KReg(0xd2, 0x21);
    wrPR2000KReg(0xd3, 0x00);
    wrPR2000KReg(0xd8, 0x31);
    wrPR2000KReg(0xd9, 0x08);
    wrPR2000KReg(0xda, 0x21);
    wrPR2000KReg(0xe0, 0x39);
    wrPR2000KReg(0xe1, 0x90);
    wrPR2000KReg(0xe2, 0x38);
    wrPR2000KReg(0xe3, 0x19);
    wrPR2000KReg(0xe4, 0x19);
    /* wrPR2000KReg(0xea,0x01);//0x02 */
    wrPR2000KReg(0xea, 0x02); // 0x02//0x01 sel pclk_out1 or pclk_out2
    wrPR2000KReg(0xeb, 0xff);
    wrPR2000KReg(0xf1, 0x44);
    wrPR2000KReg(0xf2, 0x01);

    wrPR2000KReg(0xff, 0x01);
    wrPR2000KReg(0x00, 0xe4);
    wrPR2000KReg(0x01, 0x61);
    wrPR2000KReg(0x02, 0x00);
    wrPR2000KReg(0x03, 0x57);
    wrPR2000KReg(0x04, 0x0c);
    wrPR2000KReg(0x05, 0x88);
    wrPR2000KReg(0x06, 0x04);
    wrPR2000KReg(0x07, 0xb2);
    wrPR2000KReg(0x08, 0x44);
    wrPR2000KReg(0x09, 0x34);
    wrPR2000KReg(0x0a, 0x02);
    wrPR2000KReg(0x0b, 0x14);
    wrPR2000KReg(0x0c, 0x04);
    wrPR2000KReg(0x0d, 0x08);
    wrPR2000KReg(0x0e, 0x5e);
    wrPR2000KReg(0x0f, 0x5e);
    wrPR2000KReg(0x10, 0x26);
    wrPR2000KReg(0x11, 0x01);
    wrPR2000KReg(0x12, 0x45);
    wrPR2000KReg(0x13, 0x0c);
    wrPR2000KReg(0x14, 0x00);
    wrPR2000KReg(0x15, 0x1b);
    wrPR2000KReg(0x16, 0xd0);
    wrPR2000KReg(0x17, 0x00);
    wrPR2000KReg(0x18, 0x41);
    wrPR2000KReg(0x19, 0x46);
    wrPR2000KReg(0x1a, 0x22);
    wrPR2000KReg(0x1b, 0x05);
    wrPR2000KReg(0x1c, 0xea);
    wrPR2000KReg(0x1d, 0x45);
    wrPR2000KReg(0x1e, 0x4c);
    wrPR2000KReg(0x1f, 0x00);
    wrPR2000KReg(0x20, 0x80); //80 contrast
    wrPR2000KReg(0x21, 0x80); // 80BRIGHTNESS
    wrPR2000KReg(0x22, 0x98); //98 SAT
    wrPR2000KReg(0x23, 0x80);
    wrPR2000KReg(0x24, 0x80);
    wrPR2000KReg(0x25, 0x80);
    wrPR2000KReg(0x26, 0x84);
    wrPR2000KReg(0x27, 0x82);
    wrPR2000KReg(0x28, 0x00);
    wrPR2000KReg(0x29, 0x7d);
    wrPR2000KReg(0x2a, 0x00);
    wrPR2000KReg(0x2b, 0x00);
    wrPR2000KReg(0x2c, 0x00);
    wrPR2000KReg(0x2d, 0x00);
    wrPR2000KReg(0x2e, 0x00);
    wrPR2000KReg(0x2f, 0x00);
    wrPR2000KReg(0x30, 0x00);
    wrPR2000KReg(0x31, 0x00);
    wrPR2000KReg(0x32, 0xc0);
    wrPR2000KReg(0x33, 0x14);
    wrPR2000KReg(0x34, 0x14);
    wrPR2000KReg(0x35, 0x80);
    wrPR2000KReg(0x36, 0x80);
    wrPR2000KReg(0x37, 0xaa);
    wrPR2000KReg(0x38, 0x48);
    wrPR2000KReg(0x39, 0x08); //0X08
    wrPR2000KReg(0x3a, 0x27);
    wrPR2000KReg(0x3b, 0x02);
    wrPR2000KReg(0x3c, 0x01);
    wrPR2000KReg(0x3d, 0x23);
    wrPR2000KReg(0x3e, 0x02);
    wrPR2000KReg(0x3f, 0xc4);
    wrPR2000KReg(0x40, 0x05);
    wrPR2000KReg(0x41, 0x55);
    wrPR2000KReg(0x42, 0x01);
    wrPR2000KReg(0x43, 0x33);
    wrPR2000KReg(0x44, 0x6a);
    wrPR2000KReg(0x45, 0x00);
    wrPR2000KReg(0x46, 0x09);
    wrPR2000KReg(0x47, 0xe2);
    wrPR2000KReg(0x48, 0x01);
    wrPR2000KReg(0x49, 0x00);
    wrPR2000KReg(0x4a, 0x7b);
    wrPR2000KReg(0x4b, 0x60);
    wrPR2000KReg(0x4c, 0x00);
    wrPR2000KReg(0x4d, 0x4a);
    wrPR2000KReg(0x4e, 0x00);
    wrPR2000KReg(0x4f, 0x2c);
    wrPR2000KReg(0x50, 0x01);
    wrPR2000KReg(0x51, 0x28);
    wrPR2000KReg(0x52, 0x40);
    wrPR2000KReg(0x53, 0x0c);
    wrPR2000KReg(0x54, 0x0f);
    wrPR2000KReg(0x55, 0x8d);
    wrPR2000KReg(0x70, 0x06);
    wrPR2000KReg(0x71, 0x08);
    wrPR2000KReg(0x72, 0x0a);
    wrPR2000KReg(0x73, 0x0c);
    wrPR2000KReg(0x74, 0x0e);
    wrPR2000KReg(0x75, 0x10);
    wrPR2000KReg(0x76, 0x12);
    wrPR2000KReg(0x77, 0x14);
    wrPR2000KReg(0x78, 0x06);
    wrPR2000KReg(0x79, 0x08);
    wrPR2000KReg(0x7a, 0x0a);
    wrPR2000KReg(0x7b, 0x0c);
    wrPR2000KReg(0x7c, 0x0e);
    wrPR2000KReg(0x7d, 0x10);
    wrPR2000KReg(0x7e, 0x12);
    wrPR2000KReg(0x7f, 0x14);
    wrPR2000KReg(0x80, 0x00);
    wrPR2000KReg(0x81, 0x09);
    wrPR2000KReg(0x82, 0x00);
    wrPR2000KReg(0x83, 0x07);
    wrPR2000KReg(0x84, 0x00);
    wrPR2000KReg(0x85, 0x14);
    wrPR2000KReg(0x86, 0x03);
    wrPR2000KReg(0x87, 0x36);
    wrPR2000KReg(0x88, 0x0c);
    wrPR2000KReg(0x89, 0x76);
    wrPR2000KReg(0x8a, 0x0c);
    wrPR2000KReg(0x8b, 0x76);
    wrPR2000KReg(0x8c, 0x0b);
    wrPR2000KReg(0x8d, 0xe0);
    wrPR2000KReg(0x8e, 0x06);
    wrPR2000KReg(0x8f, 0x66);
    wrPR2000KReg(0x90, 0x06);
    wrPR2000KReg(0x91, 0x8f);
    wrPR2000KReg(0x92, 0x73);
    wrPR2000KReg(0x93, 0x39);
    wrPR2000KReg(0x94, 0x0f);
    wrPR2000KReg(0x95, 0x5e);
    wrPR2000KReg(0x96, 0x09);
    wrPR2000KReg(0x97, 0x26);
    wrPR2000KReg(0x98, 0x1c);
    wrPR2000KReg(0x99, 0x20);
    wrPR2000KReg(0x9a, 0x17);
    wrPR2000KReg(0x9b, 0x70);
    wrPR2000KReg(0x9c, 0x0e);
    wrPR2000KReg(0x9d, 0x10);
    wrPR2000KReg(0x9e, 0x0b);
    wrPR2000KReg(0x9f, 0xb8);
    wrPR2000KReg(0xa0, 0x01);
    wrPR2000KReg(0xa1, 0xc2);
    wrPR2000KReg(0xa2, 0x01);
    wrPR2000KReg(0xa3, 0xb8);
    wrPR2000KReg(0xa4, 0x00);
    wrPR2000KReg(0xa5, 0xe1);
    wrPR2000KReg(0xa6, 0x00);
    wrPR2000KReg(0xa7, 0xc6);
    wrPR2000KReg(0xa8, 0x01);
    wrPR2000KReg(0xa9, 0x7c);
    wrPR2000KReg(0xaa, 0x01);
    wrPR2000KReg(0xab, 0x7c);
    wrPR2000KReg(0xac, 0x00);
    wrPR2000KReg(0xad, 0xea);
    wrPR2000KReg(0xae, 0x00);
    wrPR2000KReg(0xaf, 0xea);
    wrPR2000KReg(0xb0, 0x0b);
    wrPR2000KReg(0xb1, 0x99);
    wrPR2000KReg(0xb2, 0x12);
    wrPR2000KReg(0xb3, 0xca);
    wrPR2000KReg(0xb4, 0x00);
    wrPR2000KReg(0xb5, 0x17);
    wrPR2000KReg(0xb6, 0x08);
    wrPR2000KReg(0xb7, 0xe8);
    wrPR2000KReg(0xb8, 0xb0);
    wrPR2000KReg(0xb9, 0xce);
    wrPR2000KReg(0xba, 0x90);
    wrPR2000KReg(0xbb, 0x00);
    wrPR2000KReg(0xbc, 0x00);
    wrPR2000KReg(0xbd, 0x04);
    wrPR2000KReg(0xbe, 0x05);
    wrPR2000KReg(0xbf, 0x00);
    wrPR2000KReg(0xc0, 0x00);
    wrPR2000KReg(0xc1, 0x12);
    wrPR2000KReg(0xc2, 0x02);
    wrPR2000KReg(0xc3, 0xd0);
    wrPR2000KReg(0xff, 0x01);
    wrPR2000KReg(0x54, 0x0e);
    wrPR2000KReg(0xff, 0x01);
    wrPR2000KReg(0x54, 0x0f);

#else //1080P 驱动

//                      Page0 sys
    wrPR2000KReg(0xff, 0x00);
    wrPR2000KReg(0x10, 0x83);
    wrPR2000KReg(0x11, 0x07);
    wrPR2000KReg(0x12, 0x00);
    wrPR2000KReg(0x13, 0x00);
    wrPR2000KReg(0x14, 0x21);   //b[1:0); => Select Camera Input. VinP(1),VinN(3),Differ(0).
    wrPR2000KReg(0x15, 0x44);
    wrPR2000KReg(0x16, 0x0d);
    wrPR2000KReg(0x40, 0x00);
    wrPR2000KReg(0x47, 0x3a);
    wrPR2000KReg(0x4e, 0x3f);
    wrPR2000KReg(0x80, 0x56);
    wrPR2000KReg(0x81, 0x0e);
    wrPR2000KReg(0x82, 0x0d);
    wrPR2000KReg(0x84, 0x30);
    wrPR2000KReg(0x86, 0x20);
    wrPR2000KReg(0x87, 0x00);
    wrPR2000KReg(0x8a, 0x00);
    wrPR2000KReg(0x90, 0x00);
    wrPR2000KReg(0x91, 0x00);
    wrPR2000KReg(0x92, 0x00);
    wrPR2000KReg(0x94, 0xff);
    wrPR2000KReg(0x95, 0xff);
    wrPR2000KReg(0x96, 0xff);
    wrPR2000KReg(0xa0, 0x00);
    wrPR2000KReg(0xa1, 0x20);
    wrPR2000KReg(0xa4, 0x01);
    wrPR2000KReg(0xa5, 0xe3);
    wrPR2000KReg(0xa6, 0x00);
    wrPR2000KReg(0xa7, 0x12);
    wrPR2000KReg(0xa8, 0x00);
    wrPR2000KReg(0xd0, 0x30);
    wrPR2000KReg(0xd1, 0x08);
    wrPR2000KReg(0xd2, 0x21);
    wrPR2000KReg(0xd3, 0x00);
    wrPR2000KReg(0xd8, 0x30);
    wrPR2000KReg(0xd9, 0x08);
    wrPR2000KReg(0xda, 0x21);
    wrPR2000KReg(0xe0, 0x35);
    wrPR2000KReg(0xe1, 0x80);
    wrPR2000KReg(0xe2, 0x18);
    wrPR2000KReg(0xe3, 0x00); //0x00
    wrPR2000KReg(0xe4, 0x00); //0x00
    /* wrPR2000KReg(0xea,0x01);// 0x02//0x01 sel pclk_out1 or pclk_out2 */
    wrPR2000KReg(0xea, 0x02); // 0x02//0x01 sel pclk_out1 or pclk_out2
    wrPR2000KReg(0xeb, 0xff);
    wrPR2000KReg(0xf1, 0x44);
    wrPR2000KReg(0xf2, 0x01);

//                      Page1 vdec
    wrPR2000KReg(0xff, 0x01);
    wrPR2000KReg(0x00, 0xe4);
    wrPR2000KReg(0x01, 0x61);
    wrPR2000KReg(0x02, 0x00);
    wrPR2000KReg(0x03, 0x57);
    wrPR2000KReg(0x04, 0x0c);
    wrPR2000KReg(0x05, 0x88);
    wrPR2000KReg(0x06, 0x04);
    wrPR2000KReg(0x07, 0xb2);
    wrPR2000KReg(0x08, 0x44);
    wrPR2000KReg(0x09, 0x34);
    wrPR2000KReg(0x0a, 0x02);
    wrPR2000KReg(0x0b, 0x14);
    wrPR2000KReg(0x0c, 0x04);
    wrPR2000KReg(0x0d, 0x08);
    wrPR2000KReg(0x0e, 0x5e);
    wrPR2000KReg(0x0f, 0x5e);
    wrPR2000KReg(0x10, 0x26);
    wrPR2000KReg(0x11, 0x00);
    wrPR2000KReg(0x12, 0x87);
    wrPR2000KReg(0x13, 0x24);
    wrPR2000KReg(0x14, 0x80);
    wrPR2000KReg(0x15, 0x2a);
    wrPR2000KReg(0x16, 0x38);
    wrPR2000KReg(0x17, 0x00);
    wrPR2000KReg(0x18, 0x80);
    wrPR2000KReg(0x19, 0x48);
    wrPR2000KReg(0x1a, 0x6c);
    wrPR2000KReg(0x1b, 0x05);
    wrPR2000KReg(0x1c, 0x61);
    wrPR2000KReg(0x1d, 0x07);
    wrPR2000KReg(0x1e, 0x7e);
    wrPR2000KReg(0x1f, 0x80);
    wrPR2000KReg(0x20, 0x80);
    wrPR2000KReg(0x21, 0x80);
    wrPR2000KReg(0x22, 0x90);
    wrPR2000KReg(0x23, 0x80);
    wrPR2000KReg(0x24, 0x80);
    wrPR2000KReg(0x25, 0x80);
    wrPR2000KReg(0x26, 0x84);
    wrPR2000KReg(0x27, 0x82);
    wrPR2000KReg(0x28, 0x00);
    wrPR2000KReg(0x29, 0xff);
    wrPR2000KReg(0x2a, 0xff);
    wrPR2000KReg(0x2b, 0x00);
    wrPR2000KReg(0x2c, 0x00);
    wrPR2000KReg(0x2d, 0x00);
    wrPR2000KReg(0x2e, 0x00);
    wrPR2000KReg(0x2f, 0x00);
    wrPR2000KReg(0x30, 0x00);
    wrPR2000KReg(0x31, 0x00);
    wrPR2000KReg(0x32, 0xc0);
    wrPR2000KReg(0x33, 0x14);
    wrPR2000KReg(0x34, 0x14);
    wrPR2000KReg(0x35, 0x80);
    wrPR2000KReg(0x36, 0x80);
    wrPR2000KReg(0x37, 0xad);
    wrPR2000KReg(0x38, 0x4b);
    wrPR2000KReg(0x39, 0x08);
    wrPR2000KReg(0x3a, 0x21);
    wrPR2000KReg(0x3b, 0x02);
    wrPR2000KReg(0x3c, 0x01);
    wrPR2000KReg(0x3d, 0x23);
    wrPR2000KReg(0x3e, 0x05);
    wrPR2000KReg(0x3f, 0xc8);
    wrPR2000KReg(0x40, 0x05);
    wrPR2000KReg(0x41, 0x55);
    wrPR2000KReg(0x42, 0x01);
    wrPR2000KReg(0x43, 0x38);
    wrPR2000KReg(0x44, 0x6a);
    wrPR2000KReg(0x45, 0x00);
    wrPR2000KReg(0x46, 0x14);
    wrPR2000KReg(0x47, 0xb0);
    wrPR2000KReg(0x48, 0xdf);
    wrPR2000KReg(0x49, 0x00);
    wrPR2000KReg(0x4a, 0x7b);
    wrPR2000KReg(0x4b, 0x60);
    wrPR2000KReg(0x4c, 0x00);
    wrPR2000KReg(0x4d, 0x26);
    wrPR2000KReg(0x4e, 0x00);
    wrPR2000KReg(0x4f, 0x24);
    wrPR2000KReg(0x50, 0x01);
    wrPR2000KReg(0x51, 0x28);
    wrPR2000KReg(0x52, 0x40);
    wrPR2000KReg(0x53, 0x0c);
    wrPR2000KReg(0x54, 0x0f);
    wrPR2000KReg(0x55, 0x8d);
    wrPR2000KReg(0x70, 0x06);
    wrPR2000KReg(0x71, 0x08);
    wrPR2000KReg(0x72, 0x0a);
    wrPR2000KReg(0x73, 0x0c);
    wrPR2000KReg(0x74, 0x0e);
    wrPR2000KReg(0x75, 0x10);
    wrPR2000KReg(0x76, 0x12);
    wrPR2000KReg(0x77, 0x14);
    wrPR2000KReg(0x78, 0x06);
    wrPR2000KReg(0x79, 0x08);
    wrPR2000KReg(0x7a, 0x0a);
    wrPR2000KReg(0x7b, 0x0c);
    wrPR2000KReg(0x7c, 0x0e);
    wrPR2000KReg(0x7d, 0x10);
    wrPR2000KReg(0x7e, 0x12);
    wrPR2000KReg(0x7f, 0x14);
    wrPR2000KReg(0x80, 0x00);
    wrPR2000KReg(0x81, 0x09);
    wrPR2000KReg(0x82, 0x00);
    wrPR2000KReg(0x83, 0x07);
    wrPR2000KReg(0x84, 0x00);
    wrPR2000KReg(0x85, 0x14);
    wrPR2000KReg(0x86, 0x03);
    wrPR2000KReg(0x87, 0x36);
    wrPR2000KReg(0x88, 0x06);
    wrPR2000KReg(0x89, 0x3b);
    wrPR2000KReg(0x8a, 0x06);
    wrPR2000KReg(0x8b, 0x3b);
    wrPR2000KReg(0x8c, 0x08);
    wrPR2000KReg(0x8d, 0xe8);
    wrPR2000KReg(0x8e, 0x06);
    wrPR2000KReg(0x8f, 0x66);
    wrPR2000KReg(0x90, 0x03);
    wrPR2000KReg(0x91, 0x47);
    wrPR2000KReg(0x92, 0x73);
    wrPR2000KReg(0x93, 0x39);
    wrPR2000KReg(0x94, 0x0f);
    wrPR2000KReg(0x95, 0x5e);
    wrPR2000KReg(0x96, 0x04);
    wrPR2000KReg(0x97, 0x9c);
    wrPR2000KReg(0x98, 0x1c);
    wrPR2000KReg(0x99, 0x20);
    wrPR2000KReg(0x9a, 0x17);
    wrPR2000KReg(0x9b, 0x70);
    wrPR2000KReg(0x9c, 0x0e);
    wrPR2000KReg(0x9d, 0x10);
    wrPR2000KReg(0x9e, 0x0b);
    wrPR2000KReg(0x9f, 0xb8);
    wrPR2000KReg(0xa0, 0x01);
    wrPR2000KReg(0xa1, 0xc2);
    wrPR2000KReg(0xa2, 0x01);
    wrPR2000KReg(0xa3, 0xb8);
    wrPR2000KReg(0xa4, 0x00);
    wrPR2000KReg(0xa5, 0xe1);
    wrPR2000KReg(0xa6, 0x00);
    wrPR2000KReg(0xa7, 0xc6);
    wrPR2000KReg(0xa8, 0x01);
    wrPR2000KReg(0xa9, 0x7c);
    wrPR2000KReg(0xaa, 0x01);
    wrPR2000KReg(0xab, 0x7c);
    wrPR2000KReg(0xac, 0x00);
    wrPR2000KReg(0xad, 0xea);
    wrPR2000KReg(0xae, 0x00);
    wrPR2000KReg(0xaf, 0xea);
    wrPR2000KReg(0xb0, 0x05);
    wrPR2000KReg(0xb1, 0xcc);
    wrPR2000KReg(0xb2, 0x09);
    wrPR2000KReg(0xb3, 0x6d);
    wrPR2000KReg(0xb4, 0x00);
    wrPR2000KReg(0xb5, 0x17);
    wrPR2000KReg(0xb6, 0x08);
    wrPR2000KReg(0xb7, 0xe8);
    wrPR2000KReg(0xb8, 0xb0);
    wrPR2000KReg(0xb9, 0xce);
    wrPR2000KReg(0xba, 0x90);
    wrPR2000KReg(0xbb, 0x00);
    wrPR2000KReg(0xbc, 0x00);
    wrPR2000KReg(0xbd, 0x04);
    wrPR2000KReg(0xbe, 0x07);
    wrPR2000KReg(0xbf, 0x80);
    wrPR2000KReg(0xc0, 0x00);
    wrPR2000KReg(0xc1, 0x20);
    wrPR2000KReg(0xc2, 0x04);
    wrPR2000KReg(0xc3, 0x38);

    wrPR2000KReg(0xff, 0x01);
    wrPR2000KReg(0x54, 0x0e);
    wrPR2000KReg(0xff, 0x01);
    wrPR2000KReg(0x54, 0x0f);


#endif

    wrPR2000KReg(0xff, 0x01);
    wrPR2000KReg(0x39, 0x0f);

#if 0
//open color bar
    u8 temp ;
    wrPR2000KReg(0xff, 0x01);
    rdPR2000KReg(0x4f, &temp);
    temp = (temp | 0x10);
    wrPR2000KReg(0x4f, temp);

    wrPR2000KReg(0xff, 0x02);
    rdPR2000KReg(0x80, &temp);
    temp = (temp | 0xb0);
    wrPR2000KReg(0x80, temp);
#endif

    delay(50000);

    /* u8 DetVideo; */
    /* u8 LockStatus; */
    /* u8 reg10; */
    /* u8 i; */

    /* wrPR2000KReg(0xff,0x00); */

    /* for (i = 0;i < 3;i++) */
    /* { */
    /* rdPR2000KReg(0x00, &DetVideo); */
    /* rdPR2000KReg(0x01, &LockStatus); */
    /* rdPR2000KReg(0x10, &reg10); */

    /* log_i("reg00 %x\n", DetVideo); */
    /* log_i("reg01 %x\n", LockStatus); */
    /* log_i("reg10 %x\n", reg10); */
    /* log_i("\n\n"); */
    /* delay(5000); */
    /* } */

}

s32 pr2000k_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

static u8 avin_fps()
{
    /* u8 status ; */
    /* u8 j; */
    u8 DetVideo;
    wrPR2000KReg(0xff, 0x00);
    rdPR2000KReg(0x00, &DetVideo);

    if ((DetVideo & 0x30) == 0) {
        return 1;
    }
    return 0;
    /* rdPR2000KReg(0x88, &status); */
    /* if (status & BIT(5)) { */
    /* return 1; */
    /* } */
    /* return 0; */
}



static u8 avin_mode_det(void)
{
    u8 new_status;

    /*
        if (avin_fps()) {
            new_status = PR2000K_PSTA;
        } else {
            new_status = PR2000K_NSTA;
        }

        if (g_pn_status != new_status) {
            return 1;
        }
    */
    return 0;
}

REGISTER_CAMERA(PR2000K) = {
    .logo 				= 	"PR2000K",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_UYVY,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_BT656,//SEN_MBUS_BT656,
    /* .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B | SEN_MBUS_PCLK_SAMPLE_FALLING|SEN_MBUS_DATA_REVERSE, */
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_PCLK_SAMPLE_FALLING | SEN_MBUS_DATA_REVERSE,
    /* .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_PCLK_SAMPLE_RISING |SEN_MBUS_DATA_REVERSE, */

    .fps         		= 	25,

    .sen_size 			= 	{1280, 720},//{1280, 720},
    .isp_size 			= 	{1280, 720},
    /* .sen_size 			= 	{1280-64, 720-64},//{1280, 720}, */
    /* .isp_size 			= 	{1280-64, 720-64}, */


    .ops                =   {
        .avin_fps           =   avin_fps,
        .avin_valid_signal  =   avin_valid_signal,
        .avin_mode_det      =   avin_mode_det,
        .sensor_check 		= 	pr2000k_check,
        .init 		        = 	pr2000k_initialize,
        .set_size_fps 		=	pr2000k_set_output_size,
        .power_ctrl         =   pr2000k_power_ctl,


        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	NULL,
        .read_reg 		    =	NULL,
    }
};


