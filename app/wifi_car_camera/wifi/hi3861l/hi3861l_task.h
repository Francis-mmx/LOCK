#ifndef __HI3861L_TASK_H__
#define __HI3861L_TASK_H__

enum HI_CHANNEL_MSG {
    HI_CHANNEL_RECV_DATA = 0,  //hichannel收到数据
    HI_CHANNEL_NETWORK_CONFIG = 1,//配网
    HI_CHANNEL_CMD_SLEEP = 2,//休眠
    HI_CHANNEL_CMD_GET_WAKEUP_SOURCE = 3,//获取唤醒原因
    HI_CHANNEL_CMD_OTA = 4,//发送ota命令
    HI_CHANNEL_CMD_SET_PIR = 5,//发送ota命令
    HI_CHANNEL_CMD_GET_QOS = 6,
    HI_CHANNEL_CMD_GET_BATTERY_VALUE = 7,
    HI_CHANNEL_CMD_RESET_SET = 8,
    HI_CHANNEL_CMD_GET_WIFI_LIST = 9,
    HI_CHANNEL_CMD_GET_V_THRESHOLD = 10,
    HI_CHANNEL_CMD_SET_TAMPER = 11,
    HI_CHANNEL_CMD_GET_ALL_CFG = 12,
    HI_CHANNEL_CMD_GET_PIR = 13,
};

struct NETWORK_INFO {
    char ssid[33];
    char ip[16];
    char gw[16];
    char nm[16];
    char dns1[16];
    char dns2[16];
    u8 mac[16];
    long long qos;
};

struct WIFI_LIST_INFO {
    char ssid[33];
    int qos;
};

#endif // __HI3861L_TASK_H__
