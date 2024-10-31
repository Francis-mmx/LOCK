#ifndef ARCH_SDMMC_H
#define ARCH_SDMMC_H


#include "device/sdmmc.h"

struct sdmmc_platform_data {
    char port;
    u8 irq;
    u8 data_width;
    u8 priority;
    u8 detect_mode;
    u8 detect_time_interval;
    u32 speed;
    volatile u16 *sfr;
    int (*detect_func)(const struct sdmmc_platform_data *);
    void (*port_init)(const struct sdmmc_platform_data *, int mode);
    void (*power)(int on);
};

#define SD0_PLATFORM_DATA_BEGIN(data) \
	static const struct sdmmc_platform_data data = {


#define SD0_PLATFORM_DATA_END() \
	.irq 					= SD0_INT, \
    .sfr                    = &SD0_CON0, \
	.port_init 				= sdmmc_0_port_init, \
	.detect_time_interval 	= 50, \
};

extern const struct device_operations sd0_dev_ops;

extern void sdmmc_0_port_init(const struct sdmmc_platform_data *, int mode);
extern int sdmmc_0_clk_detect(const struct sdmmc_platform_data *);







#define SD1_PLATFORM_DATA_BEGIN(data) \
	static const struct sdmmc_platform_data data = {


#define SD1_PLATFORM_DATA_END() \
	.irq 					= SD1_INT, \
    .sfr                    = &SD1_CON0, \
	.port_init 				= sdmmc_1_port_init, \
	.detect_time_interval 	= 50, \
};


extern void sdmmc_1_port_init(const struct sdmmc_platform_data *, int mode);
extern int sdmmc_1_clk_detect(const struct sdmmc_platform_data *);



extern const struct device_operations sd_dev_ops;






#define SD2_PLATFORM_DATA_BEGIN(data) \
	static const struct sdmmc_platform_data data = {


#define SD2_PLATFORM_DATA_END() \
	.irq 					= SD2_INT, \
    .sfr                    = &SD2_CON0, \
	.port_init 				= sdmmc_2_port_init, \
	.detect_time_interval 	= 50, \
};

extern const struct device_operations sd2_dev_ops;

extern void sdmmc_2_port_init(const struct sdmmc_platform_data *, int mode);
extern int sdmmc_2_clk_detect(const struct sdmmc_platform_data *);






#endif

