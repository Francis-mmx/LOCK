/* #define _STDIO_H_ */
#include "xciot_api.h"
#include "system/includes.h"
#include "action.h"
#include "doorbell_event.h"


void doorbell_event_task(void *priv)
{
    int res;
    int msg[32];
    void *fd;
    int ret;
    struct intent it;
    struct application *app;
    u32 free_addr;
    u32 free_space;

#if 0
    extern u8 *g_ref_addr[2];
    g_ref_addr[0] = malloc(3575584 + 1024);
    g_ref_addr[1] = malloc(3575584 + 1024);
    if (!g_ref_addr[0] ||  !g_ref_addr[1]) {
        printf("\n %s %d\n", __func__, __LINE__);
        while (1);
    }
#endif


    isp_effect_init();

    while (1) {

        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case DOORBELL_EVENT_PLAY_VOICE:
                    puts("\nDOORBELL_EVENT_PLAY_VOICE\n");
                    doorbell_play_voice(msg[2]);
                    break;
                case DOORBELL_EVENT_IRCUT_OFF:
                    puts("DOORBELL_EVENT_IRCUT_OFF\n");
                    isp_effect_user_day();
                    break;
                case DOORBELL_EVENT_IRCUT_ON:
                    puts("DOORBELL_EVENT_IRCUT_ON\n");
                    isp_effect_user_night();
                    break;
                case DOORBELL_EVENT_IN_USB:
                    ;
                    extern char get_MassProduction(void);
                    if (!get_MassProduction()) {
                        puts("DOORBELL_EVENT_IN_USB\n");
                        struct intent it;
                        init_intent(&it);
                        user_net_video_rec_close(1);
                        if (get_video_rec_state()) {
                            video_rec_control_stop();
                        }
                        it.name = "usb_app";
                        it.action = ACTION_USB_SLAVE_MAIN;
                        start_app(&it);
                        fd = fopen(CONFIG_ROOT_PATH"uvc.bin", "r");
                        if (fd) {
                            it.data = "usb:uvc";
                            fclose(fd);
                        } else {
                            it.data = "usb:msd";
                            user_net_video_rec_open(1);
                        }
                        it.name	= "usb_app";
                        it.action = ACTION_USB_SLAVE_SET_CONFIG;
                        start_app(&it);
                    }

                    break;

                case DOORBELL_EVENT_OPEN_RT_STREAM:
                    puts("DOORBELL_EVENT_OPEN_RT_STREAM\n");
                    extern void *get_uvc_camera_device(void);
                    if (!get_uvc_camera_device()) {
                        user_net_video_rec_open(1);
                    }

                    break;
                case DOORBELL_EVENT_CLOSE_RT_STREAM:
                    puts("DOORBELL_EVENT_CLOSE_RT_STREAM\n");
                    user_net_video_rec_close(1);
                    break;
                case DOORBELL_EVENT_REOPEN_RT_STREAM:
                    puts("DOORBELL_EVENT_REOPEN_RT_STREAM\n");
                    user_net_video_rec_close(1);
                    if (get_video_rec_state()) {
                        video_rec_control_stop();
                        video_rec_control_start();
                    }
                    user_net_video_rec_open(1);
                    break;
                case DOORBELL_EVENT_START_REC:
                    puts("DOORBELL_EVENT_START_REC\n");
                    if (!get_video_rec_state()) {
                        user_net_video_rec_close(1);
                        video_rec_control_start();
                        user_net_video_rec_open(1);
                    }
                    break;
                case DOORBELL_EVENT_STOP_REC:
                    puts("DOORBELL_EVENT_STOP_REC\n");
                    if (get_video_rec_state()) {
                        user_net_video_rec_close(1);
                        video_rec_control_stop();
                        user_net_video_rec_open(1);
                    }
                    break;
                case DOORBELL_EVENT_CHANGE_CYC_TIME:
                    puts("DOORBELL_EVENT_CHANGE_CYC_TIME\n");
                    db_update("cyc", msg[2]);
                    db_flush();
#if 0
                    if (get_video_rec_state()) {
                        user_net_video_rec_close(1);
                        video_rec_control_stop();
                        video_rec_control_start();
                        user_net_video_rec_open(1);
                    }
#endif
                    break;
                case DOORBELL_EVENT_ENTER_SLEEP:
                    puts("DOORBELL_EVENT_ENTER_SLEEP \n");
                    user_net_video_rec_close(1);
                    //如果在录像就需要停止录像
                    video_rec_control_stop();

#ifdef CONFIG_AEC_DNS_TEST_ENABLE
                    extern void aec_dns_test_file_save();
                    aec_dns_test_file_save();

                    msleep(5000);//延时5s 让文件保存成功
#endif
                    os_taskq_post("hi_channel_task", 2, HI_CHANNEL_CMD_SLEEP, msg[2]);
                    break;
                case DOORBELL_EVENT_SD_FORMAT:
                    puts("DOORBELL_EVENT_SD_FORMAT \n");
                    if (get_video_rec_state()) {
                        user_net_video_rec_close(1);
                        video_rec_control_stop();
                        user_net_video_rec_open(1);
                    }
                    storage_format_thread_create((void *)msg[2]);
                    break;

                case DOORBELL_EVENT_RESET_CMD:
                    user_net_video_rec_close(1);
                    //如果在录像就需要停止录像
                    video_rec_control_stop();
                    os_taskq_post("hi_channel_task", 2, HI_CHANNEL_CMD_RESET_SET, msg[2]);
                    break;
                case DOORBELL_EVENT_AP_CHANGE_STA:
                    user_net_video_rec_close(1);
                    //如果在录像就需要停止录像
                    video_rec_control_stop();
                    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_AP_CHANGE_STA);
                    break;
                case DOORBELL_EVENT_PIR_STATUS_GET:
                    if (get_video_rec_state()) {
                        user_net_video_rec_close(1);
                        video_rec_control_stop();
                        user_net_video_rec_open(1);
                    }
                    extern u8 is_pir_wakeup_status();
                    if (is_pir_wakeup_status()) {
                        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_PIR);
                    } else {
                        extern void set_system_running(u8 value);
                        set_system_running(0);
                    }
                    break;
                default:
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
void doorbell_block_event_task(void *priv)
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
                case DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE:
                    puts("\nDOORBELL_EVENT_GET_GET_WAKEUP_SOURCE\n");
                    extern void doorbell_app_deal_wakeup_source(int status);
                    doorbell_app_deal_wakeup_source(msg[2]);
                    break;
                case DOORBELL_EVENT_REPORT_QOS:
                    puts("DOORBELL_EVENT_REPORT_QOS\n");
                    extern void doorbell_query_qos_timer_add(void);
                    doorbell_query_qos_timer_add();
                case DOORBELL_EVENT_REPORT_BATTARY:
                    ;
#if 0
                    puts("DOORBELL_EVENT_REPORT_BATTARY\n");
#endif

                    extern u8 get_app_connect_flag(void);
                    if (get_app_connect_flag()) {
                        static int old_batteryLevel = 0, old_powerModel = 0, old_qos = 0;
//
                        if (!old_batteryLevel || msg[2]) {
//                                //第一次上报延时200ms
//                                dev_base_info_t info = {0};
//                                info.batteryLevel = system_get_battary_info_batteryLevel();
//                                info.powerModel = system_get_battary_info_powerModel();
//                                info.netQos = system_get_network_info_qos();
//                                info.timeOffset = db_select("tzone");
//                                info.ttcmdCount = 0;
//                                avsdk_push_info(&info);
////
//                                old_batteryLevel = system_get_battary_info_batteryLevel();
//                                old_powerModel = system_get_battary_info_powerModel();
//                                old_qos = system_get_network_info_qos();
                        } else if ((old_batteryLevel - system_get_battary_info_batteryLevel() > 50) || old_powerModel != system_get_battary_info_powerModel() || old_qos != system_get_network_info_qos()) {
//
//                                dev_base_info_t info = {0};
//                                info.batteryLevel = system_get_battary_info_batteryLevel();
//                                info.powerModel = system_get_battary_info_powerModel();
//                                info.netQos = system_get_network_info_qos();
//                                info.timeOffset = db_select("tzone");
//                                info.ttcmdCount = 0;
//                                avsdk_push_info(&info);
////
//                                old_batteryLevel = system_get_battary_info_batteryLevel();
//                                old_powerModel = system_get_battary_info_powerModel();
//                                old_qos = system_get_network_info_qos();
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

static OS_SEM query_battery_timer_sem;
void doorbell_query_battery_sem_post(void)
{
    os_sem_post(&query_battery_timer_sem);
}

void doorbell_heart_task(void *priv)
{

    int res;
    int msg[32];

    os_sem_create(&query_battery_timer_sem, 0);

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case DOORBELL_EVENT_QUERY_BATTARY:
                    doorbell_query_battery_timer_add();
                    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_BATTERY_VALUE);
                    static int lost_heartbeat_cnt = 0;
                    if (os_sem_pend(&query_battery_timer_sem, 200) != OS_NO_ERR) {
                        lost_heartbeat_cnt++;
                        if (lost_heartbeat_cnt >= 5) {
                            printf("\nHI_CHANNEL_CMD_GET_BATTERY_VALUE no replay and lost_heartbeat_cnt >= 5 \n");
                            extern void hi3861l_reset(void);
                            hi3861l_reset();
                        }
                    } else {
                        lost_heartbeat_cnt = 0;
                    }
                    os_sem_set(&query_battery_timer_sem, 0);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

int doorbell_event_init(void)//主要是create wifi 线程的
{

    puts("doorbell_event_init \n");
    //负责处理实时消息
    thread_fork("doorbell_event_task", 18, 0xC00, 256, 0, doorbell_event_task, NULL);
    //负责处理有可能出现阻塞或者不需要立马响应的消息
    thread_fork("doorbell_block_event_task", 18, 0xC00, 256, 0, doorbell_block_event_task, NULL);
    //负责一些需要定时去查询的消息
    thread_fork("doorbell_heart_task", 18, 0xC00, 256, 0, doorbell_heart_task, NULL);

    return 0;
}
//late_initcall(doorbell_event_init);
