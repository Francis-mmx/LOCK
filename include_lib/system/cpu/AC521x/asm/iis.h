#ifndef __IIS_H__
#define __IIS_H__

#include "generic/typedef.h"


#define DIGITAL_VOL_CTRL        0		/*!< 是否采用数字音量缩放   */
#define MAX_IIS_VOL             31		/*!< 数字音量阶梯表最大阶梯数   */


#define IIS_PORTH          0
#define IIS_PORTF          1


struct iis_platform_data {
    u8 channel_in;
    u8 channel_out;
    u8 port_sel;
    u8 data_width;
    u8 mclk_output;
    u8 slave_mode;
};


void dv15_iis_channel_on(u8 channel);

void dv15_iis_channel_off(u8 channel);

int dv15_iis_open(struct iis_platform_data *pd);

void dv15_iis_close(void);

int dv15_iis_set_sample_rate(int sample_rate);

void dv15_iis_set_data_handler(void *priv, void (*cb)(void *, u8 *data, int len, u8));

void dv15_iis_irq_handler(void);

void dv15_iis_set_vol(u8 vol);

void dv15_iis_set_max_vol(u8 max_iis_vol);

u16 get_digital_tab_vol(u8 vol);

#endif

