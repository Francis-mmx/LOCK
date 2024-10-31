#ifndef __DOORBELL_EVENT_H__
#define __DOORBELL_EVENT_H__
#include "app_config.h"
#include "hi3861l_task.h"
#include "cloud_upgrade.h"

enum {
    DOORBELL_EVENT_PLAY_VOICE = 0,
    DOORBELL_EVENT_IRCUT_OFF = 1,
    DOORBELL_EVENT_IRCUT_ON = 2,
    DOORBELL_EVENT_IN_USB = 3,
    DOORBELL_EVENT_REPORT_BATTARY = 4,
    DOORBELL_EVENT_REPORT_QOS = 5,
    DOORBELL_EVENT_OPEN_RT_STREAM = 6,
    DOORBELL_EVENT_CLOSE_RT_STREAM = 7,
    DOORBELL_EVENT_START_REC = 8,
    DOORBELL_EVENT_STOP_REC = 9,
    DOORBELL_EVENT_CHANGE_CYC_TIME = 10,
    DOORBELL_EVENT_REOPEN_RT_STREAM = 11,
    DOORBELL_EVENT_ENTER_SLEEP = 12,
    DOORBELL_EVENT_SD_FORMAT = 13,
    DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE = 14,
    DOORBELL_EVENT_RESET_CMD = 15,
    DOORBELL_EVENT_AP_CHANGE_STA = 16,
    DOORBELL_EVENT_PIR_STATUS_GET = 17,
    DOORBELL_EVENT_QUERY_BATTARY = 18,
};


enum WAKEUP_STATUS {
    INIT_WAKEUP = 0,//初始状态
    NETWORK_WAKEUP = 1,//网络唤醒
    PIR_WAKEUP = 2,//人体红外唤醒
    KEY_WAKEUP = 3,//按键唤醒
    BREAK_WAKEUP = 4,//强拆唤醒
    LOWPOWER_WAKEUP = 5,//低电唤醒
};

enum NETWORK_STATUS {
    NETWORK_CONFIG = 0,//处于配网模式
    CONNECTING = 1,//正常连接
    CONNECT_SUC = 2,//连接成功
    DISCONNECT = 3,//断开连接
    DHCP_SUC = 4,//DHCP成功
    DHCP_FAIL = 5,//DHCP失败
    NOT_FOUND_SSID = 6,//没有发现热点
    PASSWORD_ERR = 7,//密码错误
    OTHER_ERR = 8, //其他错误
    PLATFORM_CONNECTED = 9,
    PLATFORM_CONNECTED_ERR = 10,
    AP_MODE_CFG = 11,//等待配置AP模式信息
    AP_MODE_OPEN = 12,//AP模式打卡成功
};


enum IRCUT_STATUS {
    IRCUT_INIT = 0,
    IRCUT_ON = 1,
    IRCUT_OFF = 2,
};

enum {
    CLOUD_STORAGE_START = 0,
    CLOUD_STORAGE_STARTING = 1,
    CLOUD_STORAGE_STOPING = 2,
    CLOUD_STORAGE_STOP = 3,
    CLOUD_STORAGE_IDLE = 4,
};

enum {
    RT_AUDIO_CTRL_START = 0,
    RT_AUDIO_CTRL_WRITE = 1,
    RT_AUDIO_CTRL_STOP  = 2,
};

#define FASTSTART_VM_INDEX    52


#define FIRMWARE_VM_INDEX   56
#define CHECKTIME_VM_INDEX  57
#define TIMEZONE_VM_INDEX   58





#define post_msg_doorbell_task(name,argc, ...)                  \
    do{                                                         \
        int err =  os_taskq_post(name,argc, __VA_ARGS__);       \
        if(err){                                                \
            log_e("\n %s %d err = %d\n",__func__,__LINE__,err); \
        }                                                       \
    }while(0)

/***
系統类
*/
extern int os_taskq_post(const char *name, int argc, ...);

/***
时间类的
*/
extern void msleep(unsigned int ms);
extern u32 timer_get_ms(void);
extern void delay_2ms(int cnt);
/***
led控制类
*/
extern void doorbell_blue_led_on(void);
extern void doorbell_blue_led_off(void);
extern void doorbell_red_led_on(void);
extern void doorbell_red_led_off(void);

/***
快速启动参数获取
*/
extern int read_data_for_flash(char *data, int len);
extern int write_data_to_flash(char *data, int len);

/***
唤醒原因类
*/
extern void set_wakeup_status(u8 status);

/***
平台标志类
*/
extern u8 get_cloud_storage_up_state(void);
extern u8 get_avsdk_connect_flag(void);

/***
系统状态类
*/

extern void get_flash_space_info(u32 *free_addr, u32 *free_space);
extern u8 get_video_rec_state(void);
extern void set_ircut_status(int status);
extern int storage_device_ready();
extern void *get_video_rec_handler(void);
extern u8 system_get_battary_info_batteryLevel(void);
extern u8 system_get_battary_info_powerModel(void);

/***
系统控制类
*/

extern int video_rec_control_start(void);
extern int video_rec_control_stop(void);
extern int ircut_set_isp_scenes(u8 on, char *buf, int size);
extern int storage_device_format();
extern int user_net_video_rec_open(char forward);
extern int user_net_video_rec_close(char forward);
extern int video0_rec_get_iframe(void);
extern void storage_format_thread_create(void *arg);
extern void doorbell_netcfg_timeout_timer_add(void);
extern void doorbell_netcfg_timeout_timer_del(void);
extern void doorbell_query_battery_timer_add(void);
extern void doorbell_query_qos_timer_add(void);
extern void doorbell_play_voice(const char *name);
extern void isp_effect_init(void);
extern void isp_effect_user_day(void);
extern void isp_effect_user_night(void);

/***
获取信息类
*/
extern struct NETWORK_INFO *sys_get_network_info(void);
extern int net_video_rec_get_audio_rate();
//extern void Get_IPAddress(char is_wireless, char *ipaddr);
extern void *get_usb_uvc_hdl();
extern u8 get_sdcard_upgrade_status();
extern u8 is_video_rec_mode();
extern u8 is_usb_mode(void);
extern u8 system_get_network_info_qos();
extern int get_net_cfg_info(char *ssid, char *pwd, char *vcode);
extern u8 system_get_network_info_qos(void);
extern struct NETWORK_INFO *sys_get_network_info(void);
extern const char *get_rec_path_1();
extern int net_video_rec_get_audio_rate();
extern int net_video_rec_get_fps(void) ;
extern unsigned long long get_utc_ms(void);
extern u8 get_wakeup_status();
extern u8 get_avsdk_connect_flag(void);
extern int net_video_rec_get_adc_volume();
extern int net_video_rec_get_dac_volume();
/***
设置信息类
*/
extern void set_net_cfg_info(char *ssid, char *pwd, char *vcode);
extern void netdev_set_mac_addr(char *mac_addr);
extern int cloud_storage_video_write(int type, u8 is_key_frame, int difftime, char *buffer, int len);
extern int cloud_storage_audio_write(int type, int difftime, char *buffer, int len);
extern int cloud_rt_talk_init(void);;
extern int cloud_rt_talk_uninit(void);
extern void doorbell_event_dac_mute(int state);
extern void set_rec_file_name_prefix(const char *str);
extern void hi3861l_reset(void);
/***
hichannel 通道*/
extern void oal_recieve_user_msg(u8 *buf, u32 len);


#endif // __DOORBELL_EVENT_H
