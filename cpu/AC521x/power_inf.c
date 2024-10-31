#include "device/device.h"
#include "device/key_driver.h"
#include "timer.h"
#include "database.h"
#include "event.h"
#include "asm/adc.h"
#include "asm/rtc.h"
#include "os/os_api.h"
#include "power_manage.h"
#include "asm/power_inf.h"
#include "system/spinlock.h"
#include "generic/ascii.h"
#include "asm/gpio.h"

static DEFINE_SPINLOCK(lock);

extern struct power_platform_data sys_power_data;
static struct power_platform_data *__this = NULL;

extern int rtc_vdd50_enable();

/*系统上电原因打印*/
static int system_reset_reason_check(void)
{
    volatile u32 reset_flag;
    reset_flag = (PWR_CON & 0xe0) >> 5;

    switch (reset_flag) {
    case 0:
        log_v("=====power on reset=====\n");
        break;
    case 1:
        log_v("=====VCM reset======\n");
        break;
    case 2:
        log_v("=====PR2 4s reset=====\n");
        break;
    case 3:
        log_v("=====LVD lower power reset=====\n");
        break;
    case 4:
        log_v("=====WDT reset=====\n");
        break;
    case 5:
        log_v("=====sofware reset=====\n");
        break;
    default:
        log_v("=====other reason======\n");
        break;
    }
    return reset_flag;
}

static void sys_poweroff(void *arg)
{
    while (__this->read_power_key());
    rtc_poweroff();
}

static int port_wakeup_set_config(const char *port, int enable)
{
    struct wkup_io_map *p;
    int cnt = 0;
    static struct rtc_wkup_cfg rtc_cfg = {0};

    if (!port) {
        log_e("%s() port invalid\n", __FUNCTION__);
        return -EINVAL;
    }
    for (p = __this->wkup_map; p->wkup_port != 0; p++) {
        if (!strcmp(port, p->wkup_port)) {
            switch (p->portr) {
            case WKUP_IO_PR1:
                rtc_cfg.pr1.edge = p->edge;
                rtc_cfg.pr1.port_en = enable;
                break;
            case WKUP_IO_PR2:
                rtc_cfg.pr2.edge = p->edge;
                rtc_cfg.pr2.port_en = enable;
                break;
            case WKUP_IO_PR3:
                rtc_cfg.pr3.edge = p->edge;
                rtc_cfg.pr3.port_en = enable;
                break;
            }
            break;
        }
    }
    if (p->wkup_port == 0) {
        log_e("%s() port not found\n", __FUNCTION__);
        return -EINVAL;
    }
    cnt = rtc_cfg.pr1.port_en + rtc_cfg.pr2.port_en + rtc_cfg.pr3.port_en;
    rtc_cfg.wkup_en = cnt ? 1 : 0;
    rtc_wkup_ctrl(&rtc_cfg);
    rtc_vdd50_enable();  //re enable rtc_v50 adc detect
    return 0;
}

static void usb_wakeup_enable()
{
    struct rtc_wkup_cfg rtc_cfg = {0};
    struct wkup_io_map *p;
    const char *ch;
    int cnt = 0;

    for (p = __this->wkup_map; p->wkup_port != 0 && !cnt; p++) {
        ch = p->wkup_port;
        while (*ch && !cnt) {
            if (*ch == 'u' || *ch == 'U') {
                if (!ASCII_StrCmpNoCase(ch, "usb", 3)) {
                    switch (p->portr) {
                    case WKUP_IO_PR1:
                        rtc_cfg.pr1.edge = p->edge;
                        rtc_cfg.pr1.port_en = 1;
                        break;
                    case WKUP_IO_PR2:
                        rtc_cfg.pr2.edge = p->edge;
                        rtc_cfg.pr2.port_en = 1;
                        break;
                    case WKUP_IO_PR3:
                        rtc_cfg.pr3.edge = p->edge;
                        rtc_cfg.pr3.port_en = 1;
                        break;
                    }
                    cnt = 1;
                }
            }
            ch++;
        }
    }
    cnt = rtc_cfg.pr1.port_en + rtc_cfg.pr2.port_en + rtc_cfg.pr3.port_en;
    rtc_cfg.wkup_en = cnt ? 1 : 0;
    rtc_wkup_ctrl(&rtc_cfg);
    rtc_vdd50_enable();  //re enable rtc_v50 adc detect
}

static int alarm_wakeup_set_config(u32 sec, int enable)
{
    alarm_wkup_ctrl(enable, sec);
    rtc_vdd50_enable();  //re enable rtc_v50 adc detect
    return 0;
}

//唤醒原因记录
static int wkup_reason_check(char *reason, int max_len)
{
    u32 tmp;
    u32 len;
    int cnt = 0;
    struct wkup_io_map *p;

    if (!reason || !max_len) {
        log_e("%s() string buffer invalid\n", __FUNCTION__);
        return -EINVAL;
    }

    system_reset_reason_check();

    for (int i = 0; i < 5; i++) {
        if (__this->read_power_key()) {
            cnt++;
        }
    }
    if (cnt >= 4) {
        log_v("power key press wake up\n");
        len = max_len > strlen(PWR_WKUP_PWR_ON) ? strlen(PWR_WKUP_PWR_ON) : max_len - 1;
        strncpy(reason, PWR_WKUP_PWR_ON, len);
    } else {
        tmp = rtc_wkup_reason();
        if (tmp & (WKUP_IO_PR1 | WKUP_IO_PR2 | WKUP_IO_PR3)) {
            len = max_len > strlen(PWR_WKUP_PORT) ? strlen(PWR_WKUP_PORT) : max_len - 1;
            for (p = __this->wkup_map; p->wkup_port != 0; p++) {
                if (tmp & p->portr) {
                    strncpy(reason, PWR_WKUP_PORT, len);
                    max_len -= len;

                    if (max_len > 1) {
                        reason[len] = ':';
                        reason[len + 1] = '\0';
                        max_len -= 1;
                        len = max_len > strlen(p->wkup_port) ? strlen(p->wkup_port) : max_len - 1;
                        strncpy(reason + strlen(reason), p->wkup_port, len);
                        log_v("portr wakeup: %s\n", p->wkup_port);
                        break;
                    }
                }
            }
        } else if (tmp & ABNORMAL_RESET) {
            //仅接电池中途复位不开机
            log_v("abnormal wakeup\n");
            len = max_len > strlen(PWR_WKUP_ABNORMAL) ? strlen(PWR_WKUP_ABNORMAL) : max_len - 1;
            strncpy(reason, PWR_WKUP_ABNORMAL, len);
#if 0	//有需要则打开
            if (!__this->charger_online()) {
                rtc_pin_reset_ctrl(0);
                usb_wakeup_enable();
                sys_poweroff(0);
            }
#endif
        } else if (tmp & BAT_POWER_FIRST) {
            //第一次接电池上电不开机
            len = max_len > strlen(PWR_WKUP_PWR_ON) ? strlen(PWR_WKUP_PWR_ON) : max_len - 1;
            strncpy(reason, PWR_WKUP_PWR_ON, len);
#if 0	//默认第一次上电开机，有需要则打开
            if (!__this->charger_online()) {
                rtc_pin_reset_ctrl(0);
                usb_wakeup_enable();
                sys_poweroff(0);
            }
#endif
        } else if (tmp & WKUP_SHORT_KEY) {
            //短按开机不开机
            log_v("short key on\n");
            len = max_len > strlen(PWR_WKUP_SHORT_KEY) ? strlen(PWR_WKUP_SHORT_KEY) : max_len - 1;
            strncpy(reason, PWR_WKUP_SHORT_KEY, len);
#if 1
            rtc_pin_reset_ctrl(0);
            usb_wakeup_enable();
            sys_poweroff(0);
#endif
        } else if (tmp & WKUP_ALARM) {//闹钟唤醒最后判断
            log_v("alarm wakeup\n");
            len = max_len > strlen(PWR_WKUP_ALARM) ? strlen(PWR_WKUP_ALARM) : max_len - 1;
            strncpy(reason, PWR_WKUP_ALARM, len);
        }
    }
    usb_wakeup_enable();
    __this->pwr_ctl(1);
    return strlen(reason);
}


static void ldoin_updata();


REGISTER_ADC_SCAN(ldoin_scan)
.channel = AD_CH14_RTC_V50,
 .value = 0,
  .updata = ldoin_updata,
};

static int ldoin_sum = 0;
static int ldoin_cnt = 0;

static void ldoin_updata()
{
    spin_lock(&lock);
    ldoin_sum += ldoin_scan.value;
    ldoin_cnt++;
    spin_unlock(&lock);
}

static void ldo_vbg_updata();

REGISTER_ADC_SCAN(ldo_vbg_scan)
.channel = AD_CH15_LDO_VBG,
 .value = 0,
  .updata = ldo_vbg_updata,
};

static int ldo_vbg_sum = 0;
static int ldo_vbg_cnt = 0;

static void ldo_vbg_updata()
{
    spin_lock(&lock);
    ldo_vbg_sum += ldo_vbg_scan.value;
    ldo_vbg_cnt++;
    spin_unlock(&lock);
}

#if 0
static struct adc_scan ldo_vbg_scan;

static int ldo_vbg_init(struct key_driver *key, void *arg)
{
    LDO_CON |= BIT(11);//VBG_EN
    LDO_CON |= BIT(15);//ADCOE

    return 0;
}

static u16 ldo_vbg_get_value(struct key_driver *key)
{
    return ldo_vbg_scan.value;
}

static const struct key_driver_ops ldo_vbg_driver_ops = {
    .init       = ldo_vbg_init,
    .get_value  = ldo_vbg_get_value,
};

REGISTER_KEY_DRIVER(ldo_vbg_driver) = {
    .name = "ad_ldo",
    .ops  = &ldo_vbg_driver_ops,
};
#endif


static int adc_scan_init()
{
    ADC_EN(1);
    ADCEN(1);
    ADC_BAUD(0x7);
    ADC_WTIME(0x1);
    return 0;
}

static void adc_scan_process(void)
{
    ADCSEL(AD_CH15_LDO_VBG);
    KITSTART();
    while (!ADC_PND());
    ldo_vbg_scan.value = GPADC_RES;

    rtc_adc_enable(0);
    ADCSEL(AD_CH14_RTC_V50);
    KITSTART();
    while (!ADC_PND());
    ldoin_scan.value = GPADC_RES;
}

static int get_battery_voltage()
{
    static u16 in_val = 0;
    static u16 refer_val = 0;
    static u16 val = 0;

    if (cpu_irq_disabled()) {
        adc_scan_process();
        refer_val = ldo_vbg_scan.value;
        in_val = ldoin_scan.value;
    } else {
        spin_lock(&lock);
        if (ldo_vbg_cnt && ldoin_cnt) {
            refer_val = ldo_vbg_sum / ldo_vbg_cnt;
            in_val = ldoin_sum / ldoin_cnt;
            ldo_vbg_sum = 0;
            ldo_vbg_cnt = 0;
            ldoin_sum = 0;
            ldoin_cnt = 0;
        }
        spin_unlock(&lock);
    }

#if ENABLE_SAMPLE_VAL
    const u8 vbg_volt = 126;  //此值需要通过测量val校准
    val = in_val * vbg_volt / refer_val * 3;
#else
    val = (in_val * 3 * LDO_REFERENCE_VOL + 0x181 * 2) / refer_val;
#endif

    /* printf("%d : %d : %d\n", in_val, refer_val, val); */
    /* printf("%d ", vol); */
    /* printf("0x%x\n", vol); */

    return val;
}

/*static int ad_filter(void)
{
    int i = 0;
    int sum = 0;

    [>do {<]
    sum += get_battery_val();
    [>} while (++i < 10);<]
    [>sum = sum / 10;<]

    return sum;
}*/

int __attribute__((weak)) voltage_to_persent(int voltage)
{
    for (int i = 0; i < 10; i++) {
        if (voltage <= __this->voltage_table[i][0]) {
            return __this->voltage_table[i][1];
        }
    }
    return 100;
}

u16 __attribute__((weak)) voltage_get_min_val(void)
{
    return __this->min_bat_power_val;
}


static int charger_online(void)
{
    return __this->charger_online();
}

static void power_early_init()
{
    __this = &sys_power_data;

    LDO_CON |= BIT(11);//VBG_EN
    LDO_CON |= BIT(15);//ADCOE
    /*ldo_vbg_init(NULL, NULL);
    ldoin_init(NULL, NULL);*/
    rtc_vdd50_enable();

    adc_scan_init();
}


REGISTER_SYS_POWER_HAL_OPS(sys_power) = {
    .init = power_early_init,
    .poweroff = sys_poweroff,
    .wakeup_check = wkup_reason_check,
    .port_wakeup_config = port_wakeup_set_config,
    .alarm_wakeup_config = alarm_wakeup_set_config,
    .get_battery_voltage  = get_battery_voltage,
    .charger_online = charger_online,
};
