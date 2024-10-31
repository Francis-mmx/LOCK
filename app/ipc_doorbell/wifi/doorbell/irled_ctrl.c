#include "asm/adkey.h"
#include "asm/adc.h"
#include "generic/gpio.h"
#include "device/key_driver.h"
#include "device/device.h"
#include "system/init.h"
#include "system/database.h"
#include "system/timer.h"
#include "doorbell_event.h"

extern int os_taskq_post(const char *name, int argc, ...);
extern int ircut_get_isp_scenes_flag(void);
extern u8 get_video_rec_state(void);


#define SWITCH_THRESHOLD    2    //1s 切换

REGISTER_ADC_SCAN(light_adc_scan)
.value = 0,
 .channel = 10,
};
static int ircut_status = -1; // -1:表示初始状态 0:表示关，1表示开
void set_ircut_status(int status)
{
    ircut_status = status;
}
void light_adc_value_check(void *priv)
{
    static int night_cnt, day_cnt;
    static int is_night = 0;//0:表示白天 ：1表示黑夜

    if (ircut_get_isp_scenes_flag()) {
        return;
    }

    if (light_adc_scan.value >= 950) {
        night_cnt++;
        if (night_cnt > SWITCH_THRESHOLD) {
            night_cnt = 0;
            day_cnt = 0;
            is_night = 1;
        }
    } else {
        day_cnt++;
        if (day_cnt > SWITCH_THRESHOLD) {
            night_cnt = 0;
            day_cnt = 0;
            is_night = 0;
        }
    }
    u8 ircut = db_select("ircut");    /** 1 常开；2 常关； 3 自动*/
    if (ircut == 1) {
        if (ircut_status != IRCUT_ON) {
            ircut_status = IRCUT_ON;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_ON);
        }
    } else if (ircut == 2) {
        if (ircut_status != IRCUT_OFF) {
            ircut_status = IRCUT_OFF;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_OFF);
        }
    } else if (ircut == 3) {
        if (ircut_status != IRCUT_ON && is_night == 1) {
            ircut_status = IRCUT_ON;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_ON);
        } else if (ircut_status != IRCUT_OFF && is_night == 0) {
            ircut_status = IRCUT_OFF;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_OFF);
        }
    }
}

int light_adc_init(void)
{
    static int light_adc_init_flag = 0;
    if (light_adc_init_flag) {
        return 0;
    }
    light_adc_init_flag = 1;
    extern void light_adc_channel_init(void);
    light_adc_channel_init();


    sys_timer_add_to_task("sys_timer", NULL, light_adc_value_check, 500);
    return 0;
}

//late_initcall(light_adc_init);
