#ifndef _POWER_INF_H_
#define _POWER_INF_H_

#include "generic/typedef.h"
#include "asm/rtc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wkup_io_map {
    const char *wkup_port;
    int portr;
    int edge; //0 leading edge, 1 falling edge
};

struct power_platform_data {
    u16 voltage_table[10][2];
    struct wkup_io_map wkup_map[5];
    u8(*read_power_key)();
    u8(*pwr_ctl)(u8 on_off);
    u16 min_bat_power_val;
    u16 max_bat_power_val;
    u8(*charger_online)(void);
    u32(*charger_gpio)(void);
};

#define POWER_PLATFORM_DATA_BEGIN(data) \
	const struct power_platform_data data = {

#define POWER_PLATFORM_DATA_END() \
	};

#define ENABLE_SAMPLE_VAL 	1

#ifdef __plusplus
}
#endif
#endif
