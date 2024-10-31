#ifndef CPU_ADC_H
#define CPU_ADC_H

#include "typedef.h"

/*
 * ADC通道选择
 */
enum {
    AD_CH00_PA02,
    AD_CH01_PA05,
    AD_CH02_PG06,
    AD_CH03_PG07,
    AD_CH04_PH00,
    AD_CH05_PH01,
    AD_CH06_PH05,
    AD_CH07_PH07,
    AD_CH08_PH08,
    AD_CH09_PH09,
    AD_CH10_PH12,
    AD_CH11_PH13,
    AD_CH12_PD00,
    AD_CH13_ANA_DET,
    AD_CH14_RTC_V50,
    AD_CH15_LDO_VBG,
    AD_CH14_RTC_PR1,
    AD_CH14_RTC_PR2,
};

#define ADC_WTIME(x)    GPADC_CON = (GPADC_CON & ~(0xf<<12)) | (x<<12)
#define ADCSEL(x)       GPADC_CON = (GPADC_CON & ~(0xf<<8))  | (x<<8)
#define ADC_PND()       (GPADC_CON & BIT(7))
#define KITSTART()      GPADC_CON |= BIT(6)
#define ADC_IE(x)       GPADC_CON = (GPADC_CON & ~BIT(5)) | (x<<5)
#define ADC_EN(x)       GPADC_CON = (GPADC_CON & ~BIT(4)) | (x<<4)
#define ADCEN(x)        GPADC_CON = (GPADC_CON & ~BIT(3)) | (x<<3)
#define ADC_BAUD(x)     GPADC_CON = (GPADC_CON & ~(BIT(2)| BIT(1)| BIT(0))) | (x)

struct adc_scan {
    u8 channel;
    u16 value;
    void (*updata)();
};

extern struct adc_scan adc_scan_begin[];
extern struct adc_scan adc_scan_end[];

#define REGISTER_ADC_SCAN(scan) \
	static struct adc_scan scan sec(.adc_scan) = {

#endif

