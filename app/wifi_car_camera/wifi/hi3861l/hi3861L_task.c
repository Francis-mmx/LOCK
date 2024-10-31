#include "cpu/AC521x/asm/cpu.h"
#include "hi3861l_task.h"
#include "os/os_compat.h"

#if 0

static u8 mac_addr[6];


u8 *wifi_get_module_mac_addr(void)
{
    return (u8 *)&mac_addr;
}


void hi3861l_task_q_post(u8 *data, u32 len)
{
    os_taskq_post("hi_channel_task", 3, HI_CHANNEL_RECV_DATA, data, len);
}


void hi_channel_task(void *priv)
{
    int res;
    int msg[32];

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case HI_CHANNEL_RECV_DATA:
                    puts("\n HI_CHANNEL_RECV_DATA \n");
                    extern int avsdk_cmd_recv(const uint8_t *buf, uint32_t len);
                    avsdk_cmd_recv((char *)msg[2], msg[3]);
                    free((void *)msg[2]);
                    break;
                case HI_CHANNEL_NETWORK_CONFIG:
                    puts("\n HI_CHANNEL_NETWORK_CONFIG \n");
                    /* void qr_net_cfg_uninit(void); */
                    /* qr_net_cfg_uninit(); */
                    /* extern int get_net_cfg_info(char *ssid, char *pwd, char *vcode); */
                    extern int avsdk_cmd_sta_net_cionfig(char *ssid, char *pwd);
                    void get_wifi_ssid_pwd(u8 * ssid, u8 * pwd);
                    char ssid[33] = {0};
                    char pwd[64] = {0};
                    get_wifi_ssid_pwd(ssid, pwd);
                    /* get_net_cfg_info(ssid, pwd, NULL); */
                    avsdk_cmd_sta_net_cionfig(ssid, pwd);
                    break;
                case HI_CHANNEL_CMD_SLEEP:
                    puts("\n HI_CHANNEL_CMD_SLEEP \n");

//                            extern int avsdk_stop(void);
//                            avsdk_stop();
                    extern int avsdk_cmd_sleep(void *sleep);
                    avsdk_cmd_sleep(msg[2]);

                    break;
                case HI_CHANNEL_CMD_GET_WAKEUP_SOURCE:
                    puts("\n HI_CHANNEL_CMD_GET_WAKEUP_SOURCE \n");
                    extern int avsdk_cmd_get_wakeup_source(void);
                    avsdk_cmd_get_wakeup_source();
                    break;
                case HI_CHANNEL_CMD_OTA:
                    puts("\n HI_CHANNEL_CMD_OTA \n");
                    /* extern int avsdk_cmd_push_file(struct cloud_upgrade_info * info); */
                    /* avsdk_cmd_push_file((struct cloud_upgrade_info *)msg[2]); */
                    break;

                case HI_CHANNEL_CMD_SET_PIR:
                    puts("\n HI_CHANNEL_CMD_SET_PIR \n");
                    extern int avsdk_cmd_set_pir(int value);
                    extern int get_pir_state();
                    avsdk_cmd_set_pir(get_pir_state());
                    break;
                case HI_CHANNEL_CMD_GET_QOS:
                    puts("\n HI_CHANNEL_CMD_GET_QOS \n");
                    extern int avsdk_cmd_get_qos(void);
                    avsdk_cmd_get_qos();
                    break;
                case HI_CHANNEL_CMD_GET_BATTERY_VALUE:
                    puts("\n HI_CHANNEL_CMD_GET_BATTERY_VALUE \n");
                    extern int avsdk_cmd_get_battery_value(void);
                    avsdk_cmd_get_battery_value();
                    break;
                case HI_CHANNEL_CMD_RESET_SET:
                    puts("\n HI_CHANNEL_CMD_RESET_SET \n");
//                            extern int avsdk_stop(void);
//                            avsdk_stop();

                    int avsdk_cmd_reset_set(int value) ;//1 重启 2 恢复出厂设置
                    avsdk_cmd_reset_set(msg[2]);
                    break;
                case HI_CHANNEL_CMD_GET_WIFI_LIST:
                    puts("\n HI_CHANNEL_CMD_GET_WIFI_LIST \n");
                    extern int avsdk_cmd_get_wifi_list(void);
                    avsdk_cmd_get_wifi_list();
                    break;
                case HI_CHANNEL_CMD_GET_V_THRESHOLD:
                    puts("\n HI_CHANNEL_CMD_GET_V_THRESHOLD \n");
                    extern int avsdk_cmd_get_v_threshold(void);
                    avsdk_cmd_get_v_threshold();
                    break;
                case HI_CHANNEL_CMD_SET_TAMPER:
                    puts("\n HI_CHANNEL_CMD_SET_TAMPER \n");
                    extern int avsdk_cmd_set_tamper(int value);
                    extern int get_tamper_state();
                    avsdk_cmd_set_tamper(get_tamper_state());
                    break;
                case HI_CHANNEL_CMD_GET_ALL_CFG:
                    puts("\n HI_CHANNEL_CMD_GET_ALL_CFG \n");
                    extern int avsdk_cmd_get_all_cfg(void);
                    avsdk_cmd_get_all_cfg();
                    break;
                case HI_CHANNEL_CMD_GET_PIR:
                    avsdk_cmd_get_pir_status();
                    break;
                }

                break;

            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}


int Hi3861L_init(void)//主要是create wifi 线程的
{
    puts("Hi3861L_init \n");
    thread_fork("hi_channel_task", 18, 0x2E00, 64, 0, hi_channel_task, NULL);
    return 0;
}

#endif

uint64_t get_utc_ms(void)
{
    return 0;
}





