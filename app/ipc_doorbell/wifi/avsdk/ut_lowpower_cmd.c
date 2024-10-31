//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
#define _STDIO_H_
#include "xciot_api.h"
#include "xciot_type.h"
#include "iotsdk_xspp.h"
#include "ut_dev_ipc_cmd.h"
#include "system/includes.h"
#include "server/wifi_connect.h"
#include "lwip.h"
#include "lwip/inet.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "doorbell_event.h"

extern int atoi(const char *);

enum {
    CMD_SET_POWER_MODE = 591,
    CMD_GET_WIFI_VERSION = 10101,
    CMD_STA_NET_CONFIG = 10102,
    CMD_AP_NET_CONFIG = 10103,
    CMD_GET_NETWORK_STATUS = 10104,
    CMD_SET_IP = 10105,//热点时使用
    CMD_GET_IP = 10106,
    CMD_SET_MAC = 10107,
    CMD_GET_MAC = 10108,
    CMD_GET_QOS = 10109,//信号强度查询
    CMD_SET_PIR = 10110,
    CMD_GET_PIR_STATUS = 10111,
    CMD_GET_WAKEUP_SOURCE = 10112,
    CMD_SET_FILTER = 10113,
    CMD_RESET_SET = 10114,
    CMD_GET_ROMDATA = 10115,
    CMD_PUSH_FILE = 10116,
    CMD_SLEEP = 10117,
    CMD_GET_BATTERY_VALUE = 10118,
    CMD_GET_WIFI_LIST = 10119,
    CMD_SET_V_THRESHOLD = 10120,
    CMD_GET_V_THRESHOLD = 10121,
    CMD_SET_TAMPER = 10122,
    CMD_GET_TAMPER = 10123,
    CMD_GET_ALLCFG = 10124,
    CMD_GET_DID_INFO = 30016,
    CMD_PUT_DID_INFO = 30007,
};



struct BATTARY_INFO {
    int64_t batteryLevel;
    int64_t powerModel;
    int64_t voltage;
    int64_t use_time;
};



static struct WIFI_LIST_INFO wifi_list_info[10];
static struct NETWORK_INFO network_info = {
    .qos = -46,
};
static struct BATTARY_INFO battary_info;
static int network_status = 0;
static int sys_tamper_state;
static int sys_pir_state;
static int pir_trigger_state;

int get_network_status()
{
    return network_status;
}

void set_tamper_state(int state)
{
    sys_tamper_state = state;
}

int get_tamper_state()
{
    return sys_tamper_state;
}

void set_pir_state(int state)
{
    sys_pir_state = state;
}

int get_pir_state()
{
    return sys_pir_state;
}

int get_pir_trigger_state()
{
    return pir_trigger_state;
}



struct WIFI_LIST_INFO *get_wifi_list_info(void)
{
    return &wifi_list_info;
}

int get_wifi_list_info_cnt(void)
{
    return sizeof(wifi_list_info) / sizeof(struct WIFI_LIST_INFO);
}

int system_get_battary_info_voltage(void)
{
    return battary_info.voltage;;
}
u8 system_get_battary_info_batteryLevel(void)
{
#define  FULL_POWER_VOLTAGE   4000
#define  LOW_POWER_VOLTAGE    3600
#define  POWER_INTERVAL       (FULL_POWER_VOLTAGE - LOW_POWER_VOLTAGE)
    int voltage;
    u8 batteryLevel;
    voltage = battary_info.voltage;
    if (voltage > FULL_POWER_VOLTAGE) {
        voltage = FULL_POWER_VOLTAGE;
    }
    if (voltage < LOW_POWER_VOLTAGE) {
        voltage = LOW_POWER_VOLTAGE;
    }
//    printf("\n voltage = %d  \n",voltage);
    int diff_interval = (FULL_POWER_VOLTAGE - voltage);
    voltage = POWER_INTERVAL - (diff_interval > POWER_INTERVAL ? 0 : diff_interval);
    batteryLevel = (voltage * 100) / POWER_INTERVAL;
//    printf("\n batteryLevel = %d\n",batteryLevel);
    if (batteryLevel < 5) {
        batteryLevel = 5;
    }


    return batteryLevel;
}

u8 system_get_battary_info_powerModel(void)
{
    return battary_info.powerModel;
}

struct NETWORK_INFO *sys_get_network_info(void)
{
    return &network_info;
}
u8 system_get_network_info_qos(void)
{
    u8 qos;
    if (network_info.qos >= -25) {
        qos = 6;
    } else if (network_info.qos >= -35 && network_info.qos  < -25) {
        qos = 6;
    } else if (network_info.qos >= -45 && network_info.qos  < -35) {
        qos = 5;
    } else if (network_info.qos >= -50 && network_info.qos  < -45) {
        qos = 5;
    } else if (network_info.qos >= -60 && network_info.qos  < -50) {
        qos = 3;
    } else if (network_info.qos < -60) {
        qos = 3;
    }
    printf("\n qos = %d network_info.qos = %lld \n", qos, network_info.qos);
    return qos;
}


uint64_t hal_GetUtcTime(void)
{
    //获取utc ms 时间
    return 0;
}


int avsdk_cmd_set_power_mode(int value)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_SET_POWER_MODE;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = value; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_get_wifi_version(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_WIFI_VERSION;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_get_wifi_list(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_WIFI_LIST;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


int avsdk_cmd_set_v_threshold(int64_t V0, int64_t V1) //V0此电压不唤醒 V1此电压禁止唤醒
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_SET_V_THRESHOLD;//命令cid
    ppiot.cmds[0].pint_count = 2;//包含1个int参数
    ppiot.cmds[0].pint[0] = V0; //没有意义
    ppiot.cmds[0].pint[1] = V1; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


int avsdk_cmd_reset_set(int value) //1 重启 2 恢复出厂设置
{
    int rc = 0;

    printf("\n %s value = %d\n", __func__, value);
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_RESET_SET;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = value; //没有意义
    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_get_v_threshold(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_V_THRESHOLD;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


//对应命令CMD_STA_NET_CONFIG，sta网络配置
int avsdk_cmd_sta_net_cionfig(char *ssid, char *pwd)
{
    int rc = 0;
    printf("\n>>> %s %d\n", __func__, __LINE__);
    iot_PpiotCmd ppiot = {0};
    struct WIFI_INIT_INFO wifi_info = {0};;
    wifi_info.mode = STA_MODE;
    strcpy(wifi_info.ssid, ssid);
    strcpy(wifi_info.pwd, pwd);
    db_update_buffer(WIFI_INFO, (char *)&wifi_info, sizeof(struct WIFI_INIT_INFO));

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_STA_NET_CONFIG;//命令cid
    ppiot.cmds[0].pstr_count = 2;//包含2个str字符串参数
    /*填充字符串*/
    snprintf(ppiot.cmds[0].pstr[0], sizeof(ppiot.cmds[0].pstr[0]), "%s", ssid);
    snprintf(ppiot.cmds[0].pstr[1], sizeof(ppiot.cmds[0].pstr[1]), "%s", pwd);
    /*发送ppiot结构体到WiFi模块*/
    extern void clean_fastdata_to_flash(void);
    clean_fastdata_to_flash();
    rc = xspp_tx_data(&ppiot, 0);
    printf("\n>>> %s %d\n", __func__, __LINE__);
    return rc;
}


int avsdk_cmd_get_mac(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_MAC;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


int avsdk_cmd_get_network_status(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_NETWORK_STATUS;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_get_ip(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_IP;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}
int avsdk_cmd_get_qos(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_QOS;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


int avsdk_cmd_get_wakeup_source(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_WAKEUP_SOURCE;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


int avsdk_cmd_set_pir(int value)
{
    printf("\n %s value = %d\n", __func__, value);
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_SET_PIR;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = value; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_set_tamper(int value)
{
    printf("\n %s value = %d\n", __func__, value);
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_SET_TAMPER;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = value; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_get_pir_status(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_PIR_STATUS;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


//对应命令CMD_SLEEP，进入休眠模式
int avsdk_cmd_sleep(iot_sleep_t *sleep)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    printf("\n avsdk_cmd_sleep enter \n");

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_SLEEP;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含一个int参数
    ppiot.cmds[0].pint[0] = 1; //code,1进入低功耗模式

    if (sleep) {
        extern int avsdk_tools_sleep2byte(iot_sleep_t *sleep, uint8_t *outArr, uint16_t *arrlen);
        /*将avsdk_sleep获取的iot_sleep_t转成bytes数组，并填入ppiot内*/
        rc = avsdk_tools_sleep2byte(sleep, ppiot.cmds[0].pbs.bytes, &ppiot.cmds[0].pbs.size);
        if (rc) {
            ppiot.cmds[0].pbs.size = 0;
            puts("\n avsdk_tools_sleep2byte err\n");
        }
    }
    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    extern void doorbell_sleep_timeout(void);
    doorbell_sleep_timeout();
    printf("\n avsdk_cmd_sleep exit \n");
    return rc;
}


int avsdk_cmd_get_battery_value(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};
    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_BATTERY_VALUE;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_push_file(struct cloud_upgrade_info *info)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};
    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_PUSH_FILE;//命令cid
    ppiot.cmds[0].pint_count = 3;//包含1个int参数
    ppiot.cmds[0].pint[0] = info->total_size;
    ppiot.cmds[0].pint[1] = info->offset;
    ppiot.cmds[0].pint[2] = info->type;
    ppiot.cmds[0].pstr_count = 2;//包含1个int参数
    /*填充字符串*/
    snprintf(ppiot.cmds[0].pstr[0], sizeof(ppiot.cmds[0].pstr[0]), "%s", info->name);
    snprintf(ppiot.cmds[0].pstr[1], sizeof(ppiot.cmds[0].pstr[1]), "%s", info->md5);
#if 0
    printf("\n >>>>>>>>>>>>>>>>>info->name = %s\n", info->name);
    printf("\n info->name = %s\n", info->name);
    printf("\n info->md5 = %s\n", info->md5);
    printf("\n info->len = %d\n", info->len);
#endif
    ppiot.cmds[0].pbs.size = info->len;
    memcpy(ppiot.cmds[0].pbs.bytes, info->buffer, info->len);
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

int avsdk_cmd_put_did_info(u8 *cfg, u32 len)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};
    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_PUT_DID_INFO;//命令cid
    ppiot.cmds[0].pint_count = 3;//包含1个int参数
    ppiot.cmds[0].pint[0] = len;
    ppiot.cmds[0].pint[1] = 0;
    ppiot.cmds[0].pint[2] = 0;
    ppiot.cmds[0].pstr_count = 2;//包含1个int参数
    /*填充字符串*/
    char did[64] = {""};
    extern int doorbell_read_cfg_did(char *buf, int buf_len);
    doorbell_read_cfg_did(did, sizeof(did));
    snprintf(ppiot.cmds[0].pstr[0], sizeof(ppiot.cmds[0].pstr[0]), "%s", did);
    snprintf(ppiot.cmds[0].pstr[1], sizeof(ppiot.cmds[0].pstr[1]), "%s", "NULL");
#if 1
    printf("\n >>>>>>>>>>>>>>>>name = %s\n", did);;
    printf("\n info->len = %d\n", len);
#endif
    ppiot.cmds[0].pbs.size = len;
    memcpy(ppiot.cmds[0].pbs.bytes, cfg, len);
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}


int avsdk_cmd_get_all_cfg(void)
{
    int rc = 0;
    iot_PpiotCmd ppiot = {0};

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_GET_ALLCFG;//命令cid
    ppiot.cmds[0].pint_count = 1;//包含1个int参数
    ppiot.cmds[0].pint[0] = 0; //没有意义

    /*发送ppiot结构体到WiFi模块*/
    rc = xspp_tx_data(&ppiot, 0);
    return rc;
}

//对应命令CMD_AP_NET_CONFIG，sta网络配置
int avsdk_cmd_ap_net_cionfig(char *ssid, char *pwd)
{
    int rc = 0;
    printf("\n>>> %s %d\n", __func__, __LINE__);
    iot_PpiotCmd ppiot = {0};
    struct WIFI_INIT_INFO wifi_info = {0};;
    wifi_info.mode = AP_MODE;
    strcpy(wifi_info.ssid, ssid);
    strcpy(wifi_info.pwd, pwd);
    db_update_buffer(WIFI_INFO, (char *)&wifi_info, sizeof(struct WIFI_INIT_INFO));

    ppiot.cmds_count = 1;//固定值1
    ppiot.cmds[0].cid = CMD_AP_NET_CONFIG;//命令cid
    ppiot.cmds[0].pstr_count = 2;//包含2个str字符串参数
    /*填充字符串*/
    snprintf(ppiot.cmds[0].pstr[0], sizeof(ppiot.cmds[0].pstr[0]), "%s", ssid);
    snprintf(ppiot.cmds[0].pstr[1], sizeof(ppiot.cmds[0].pstr[1]), "%s", pwd);
    /*发送ppiot结构体到WiFi模块*/
    extern void clean_fastdata_to_flash(void);
    clean_fastdata_to_flash();
    rc = xspp_tx_data(&ppiot, 0);
    printf("\n>>> %s %d\n", __func__, __LINE__);
    return rc;
}

static int avsdk_cmd_callback(iot_PpiotCmd *cmd, void *ud)
{
    char *str = NULL;
    int64_t status;
    u8 mac_addr[6] = {0};


    if (get_sdcard_upgrade_status() &&
        cmd->cmds[0].cid !=  CMD_PUSH_FILE &&
        cmd->cmds[0].cid !=  CMD_GET_BATTERY_VALUE) {
        //ota的时候只处理心跳指令和CMD_PUSH_FILE指令
        return 0;
    }

    switch (cmd->cmds[0].cid) {
    case CMD_GET_WIFI_VERSION:
        puts("\n CMD_GET_WIFI_VERSION \n");
        printf("\n WIFI_VERSION hdv = %s\n", &cmd->cmds[0].pstr[0]);
        printf("\n WIFI_VERSION swv = %s\n", &cmd->cmds[0].pstr[1]);
        extern void set_hardware_version(char *hv, char *sv);
        set_hardware_version(&cmd->cmds[0].pstr[0], &cmd->cmds[0].pstr[1]);
        break;
    case CMD_STA_NET_CONFIG:
        puts("\n CMD_STA_NET_CONFIG \n");
        status = cmd->cmds[0].pint[0];
        //todo:收到CMD_STA_NET_CONFIG命令的应答
        break;
    case CMD_GET_MAC:
        puts("\n CMD_GET_MAC \n");
        str = &cmd->cmds[0].pstr[0];
        u8 mac_addr[6] = {0};
        strcpy(network_info.mac, str);
        sscanf(str, "%x:%x:%x:%x:%x:%x", &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]);
        put_buf(mac_addr, 6);
        netdev_set_mac_addr(mac_addr);
        break;
    case CMD_GET_IP:
        puts("\n CMD_GET_IP \n");
        struct lan_setting lan_setting_info = {0};
        u32 ip_info;
        char *ip = &cmd->cmds[0].pstr[0];
        char local_ip[24];
        Get_IPAddress(1, local_ip);
        if (!strcmp(ip, "0.0.0.0") || !strcmp(ip, local_ip)) {
            break;
        }
        printf("\n ip = %s\n", ip);
        strcpy(network_info.ip, ip);
        ip_info  = inet_addr(ip);
        memcpy(&lan_setting_info.WIRELESS_IP_ADDR0, &ip_info, sizeof(int));

        char *gw = &cmd->cmds[0].pstr[1];
        strcpy(network_info.gw, gw);
        ip_info = inet_addr(gw);
        memcpy(&lan_setting_info.WIRELESS_GATEWAY0, &ip_info, sizeof(int));

        char *nm = &cmd->cmds[0].pstr[2];
        strcpy(network_info.nm, nm);
        ip_info = inet_addr(nm);
        memcpy(&lan_setting_info.WIRELESS_NETMASK0, &ip_info, sizeof(int));

        hi3861_set_lan_setting_info(&lan_setting_info);




        struct WIFI_INIT_INFO wifi_info = {0};;
        if (db_select_buffer(WIFI_INFO, (char *)&wifi_info, sizeof(struct WIFI_INIT_INFO)) == sizeof(struct WIFI_INIT_INFO)) {
            printf("\n wifi_info.ssid = %s  wifi_info.pwd = %s\n", wifi_info.ssid, wifi_info.pwd);
            strcpy(network_info.ssid, wifi_info.ssid);
        }

        Init_LwIP(1, 0);
//        lwip_netif_set_up(1);
//        lwip_set_default_netif(WIFI_NETIF);
//        lwip_renew(WIFI_NETIF, 0);

        char *dns1 =  &cmd->cmds[0].pstr[3];
        printf("\n dns1 = %s\n", dns1);
        strcpy(network_info.dns1, dns1);
        ip_addr_t dnsserver;
        dnsserver.addr = inet_addr(dns1);
        void dns_setserver(u8_t numdns, const ip_addr_t *dnsserver);
        dns_setserver(0, &dnsserver);

        const ip_addr_t *dns_getserver(u8_t numdns);
        ip_addr_t *__dnsserver;
        __dnsserver = dns_getserver(0);
        printf("\n dns1get = %s\n", inet_ntoa(__dnsserver->addr));

        char *dns2 =  &cmd->cmds[0].pstr[4];
        printf("\n dns2 = %s\n", dns2);
        strcpy(network_info.dns2, dns2);
        dnsserver.addr = inet_addr(dns2);
        dns_setserver(1, &dnsserver);

        if (!get_net_cfg_info(NULL, NULL, NULL)) {
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgSucc.adp");
        }
#ifdef CONFIG_IPERF_ENABLE
//网络测试工具，使用iperf
        extern void iperf_test(void);
        iperf_test();
        return 0;
#endif

        extern char get_MassProduction(void);
        extern  void mass_production_init(void);
        if (get_MassProduction()) {
            mass_production_init();
        } else {
            extern int avsdk_main(void);
            avsdk_main();
        }
        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_QOS);

        break;

    case CMD_GET_QOS:
        puts("\n CMD_GET_QOS \n");
        network_info.qos = cmd->cmds[0].pint[0];
        printf("\n qos = %lld\n", network_info.qos);
        post_msg_doorbell_task("doorbell_block_event_task", 2, DOORBELL_EVENT_REPORT_QOS, 0);
        break;
    case CMD_SET_PIR:
        puts("\n CMD_SET_PIR \n");
        break;
    case CMD_GET_PIR_STATUS:
        puts("\n CMD_GET_PIR_STATUS \n");
        int on  = cmd->cmds[0].pint[0];
        pir_trigger_state = cmd->cmds[0].pint[1];
        printf("\n on = %d\n", on);
        printf("\npir_status = %d\n", pir_trigger_state);
        extern void set_system_running(u8 value);
        set_system_running(0);
        if (pir_trigger_state) {
            set_wakeup_status(PIR_WAKEUP);
        }
        break;
    case CMD_GET_WAKEUP_SOURCE:
        if (get_MassProduction()) {
            break;
        }
        status = cmd->cmds[0].pint[0];
        puts("\nCMD_GET_WAKEUP_SOURCE");
        printf("\nwakeup status = %d", status);
        printf("\n0:INIT_WAKEUP\n1:NETWORK_WAKEUP\n2:PIR_WAKEUP\n3:KEY_WAKEUP\n4:BREAK_WAKEUP\n5:LOWPOWER_WAKEUP\n\n");
        if (network_status == 0 && status != INIT_WAKEUP) {
            return 0;
        }
        if (status == 6) {
            status = KEY_WAKEUP;
        }
        post_msg_doorbell_task("doorbell_block_event_task", 2, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, status);

        break;
    case CMD_GET_NETWORK_STATUS:
        status = cmd->cmds[0].pint[0];
        puts("\nCMD_GET_NETWORK_STATUS");
        printf("\nnetwork status = %d", status);
        printf("\n0:NETWORK_CONFIG\n1:CONNECTING\n2:CONNECT_SUC\n3:DISCONNECT"\
               "\n4:DHCP_SUC\n5:DHCP_FAIL\n6:NOT_FOUND_SSID\n7:PASSWORD_ERR"\
               "\n8:OTHER_ERR\n9:PLATFORM_CONNECTED\n10:PLATFORM_CONNECTED_ERR"\
               "\n11:AP_MODE_CFG\n12:AP_MODE_OPEN\n\n");

        if (status == NETWORK_CONFIG) {
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgEnter.adp");
            extern void qr_net_cfg_init(void);
            qr_net_cfg_init();
        } else if (status == CONNECTING || status ==  CONNECT_SUC) {

        } else if (status == DISCONNECT) {
            if (network_status == CONNECT_SUC) {
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetDisc.adp");
            }
        } else if (status == DHCP_SUC) {
            avsdk_cmd_get_ip();
        } else if (status == DHCP_FAIL || status == NOT_FOUND_SSID || status == PASSWORD_ERR || status == OTHER_ERR) {
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgFail.adp");
        } else if (status == PLATFORM_CONNECTED) {

        } else if (status == PLATFORM_CONNECTED_ERR) {

        } else if (status == AP_MODE_CFG) {
            extern void doorbell_read_cfg_info(u8 * cfg, int len);
            char *cfg = calloc(1, 1024);
            if (!cfg) {
                printf("\n %s %d\n", __func__, __LINE__);
                return 0;
            }
            doorbell_read_cfg_info(cfg, 1024);
            char *str = NULL;
            char did[32] = {""};
            char ssid[33];
            char pwd[64];
            if (strlen(cfg)) {

                printf("\n %s %d\n", __func__, __LINE__);
                str = strstr(cfg, "did = ");
                if (str) {
                    str += strlen("did = ");
                    memset(did, 0, sizeof(did));
                    memcpy(did, str, strstr(str, "\n") - str);
                    printf("\n did = %s\n", did);
                }
            }
            printf("\n %s %d\n", __func__, __LINE__);
            if (strlen(did)) {
                printf("\n %s %d\n", __func__, __LINE__);
                str = did + strlen(did);
                str -= 6;
                snprintf(ssid, sizeof(ssid), "DOORBELL%s", str);
                printf("\n ssid = %s\n", ssid);
                avsdk_cmd_ap_net_cionfig(ssid, "12345678");
            }
            free(cfg);
        } else if (status == AP_MODE_OPEN) {

        }
        network_status = status;
        break;
    case CMD_SLEEP:
        puts("\n CMD_SLEEP \n");
        break;

    case CMD_GET_BATTERY_VALUE:

        battary_info.batteryLevel = cmd->cmds[0].pint[0];
        battary_info.powerModel = cmd->cmds[0].pint[1];
        battary_info.voltage = cmd->cmds[0].pint[2];
        battary_info.use_time = cmd->cmds[0].pint[3];
#if 0
        puts("\n CMD_GET_BATTERY_VALUE \n");
        printf("\n batteryLevel = %d \n", battary_info.batteryLevel);
        printf("\n powerModel = %d \n", battary_info.powerModel);
        printf("\n voltage = %d \n", battary_info.voltage);
        printf("\n use_time = %d \n", battary_info.use_time);
#endif
        post_msg_doorbell_task("doorbell_block_event_task", 2, DOORBELL_EVENT_REPORT_BATTARY, 0);
        extern void doorbell_query_battery_sem_post(void);
        doorbell_query_battery_sem_post();
        break;
    case CMD_PUSH_FILE:
        puts("\n CMD_PUSH_FILE \n");
        extern void cloud_upgrade_sem_post();
        cloud_upgrade_sem_post();
        break;
    case CMD_SET_V_THRESHOLD:
        break;
    case CMD_GET_V_THRESHOLD:
        printf("\n >>>>>>>> V0 = %lld\n", cmd->cmds[0].pint[0]);
        printf("\n >>>>>>>> V1 = %lld\n", cmd->cmds[0].pint[1]);
        break;
    case CMD_GET_WIFI_LIST:
        cmd->cmds[0].pbs.bytes[cmd->cmds[0].pbs.size] = '\0';
        printf("\n wifi_list = %s \n", cmd->cmds[0].pbs.bytes);
        json_object *new_obj = NULL;
        json_object *tmp_obj = NULL;
        json_object *data = NULL;
        json_object *key = NULL;
        new_obj = json_tokener_parse(cmd->cmds[0].pbs.bytes);

        tmp_obj = json_object_object_get(new_obj, "wifis");
        for (int i = 0; i <  json_object_array_length(tmp_obj); i++) {
            data = json_object_array_get_idx(tmp_obj, i);
            key = json_object_object_get(data, "ssid");
            if (strlen(json_object_get_string(key)) > 0 && i < get_wifi_list_info_cnt()) {
                snprintf(wifi_list_info[i].ssid, sizeof(wifi_list_info[i].ssid), json_object_get_string(key));
                printf("\n wifi_list_info[%d].ssid = %s \n", i, wifi_list_info[i].ssid);
                key = json_object_object_get(data, "qos");
                wifi_list_info[i].qos = json_object_get_int(key);
                printf(" wifi_list_info[%d].qos = %d ", i, wifi_list_info[i].qos);
            }
        }
        json_object_put(new_obj);
        break;
    case CMD_GET_ALLCFG:

        puts("\n CMD_GET_ALLCFG \n");
        status = cmd->cmds[0].pint[0];
        printf("\nnetwork status = %d", status);
        printf("\n0:NETWORK_CONFIG\n1:CONNECTING\n2:CONNECT_SUC\n3:DISCONNECT"\
               "\n4:DHCP_SUC\n5:DHCP_FAIL\n6:NOT_FOUND_SSID\n7:PASSWORD_ERR"\
               "\n8:OTHER_ERR\n9:PLATFORM_CONNECTED\n10:PLATFORM_CONNECTED_ERR"\
               "\n11:AP_MODE_CFG\n12:AP_MODE_OPEN\n\n");

        if (status == NETWORK_CONFIG) {
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgEnter.adp");
            extern void qr_net_cfg_init(void);
            qr_net_cfg_init();
        } else if (status == CONNECTING || status ==  CONNECT_SUC) {

        } else if (status == DISCONNECT) {
            if (network_status == CONNECT_SUC) {
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetDisc.adp");
            }
        } else if (status == DHCP_SUC) {
            avsdk_cmd_get_ip();
        } else if (status == DHCP_FAIL || status == NOT_FOUND_SSID || status == PASSWORD_ERR || status == OTHER_ERR) {
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgFail.adp");
        } else if (status == PLATFORM_CONNECTED) {

        } else if (status == PLATFORM_CONNECTED_ERR) {

        } else if (status == AP_MODE_CFG) {
            char *str = NULL;
            char did[32] = {""};
            char ssid[33], pwd[64];
            extern int doorbell_read_cfg_did(char *buf, int buf_len);
            doorbell_read_cfg_did(did, sizeof(did));
            if (strlen(did)) {
                str = did + strlen(did);
                str -= 6;
                snprintf(ssid, sizeof(ssid), "LLM_%s", str);
                printf("\n ssid = %s\n", ssid);
                avsdk_cmd_ap_net_cionfig(ssid, "12345678");
            }
        } else if (status == AP_MODE_OPEN) {
            avsdk_cmd_get_ip();
        }
        network_status = status;

        sys_tamper_state =  cmd->cmds[0].pint[1];
        sys_pir_state  = cmd->cmds[0].pint[2];
        printf("\nsys_tamper_state = %d \n", sys_tamper_state);
        printf("\nsys_pir_state = %d\n", sys_pir_state);
        printf("\n V0 = %lld\n", cmd->cmds[0].pint[3]);
        printf("\n V1 = %lld\n", cmd->cmds[0].pint[4]);
        battary_info.batteryLevel =  cmd->cmds[0].pint[5];
        battary_info.powerModel =  cmd->cmds[0].pint[6];
        battary_info.voltage =  cmd->cmds[0].pint[7];
        printf("\n battary_info.batteryLevel = %lld\n", battary_info.batteryLevel);
        printf("\n battary_info.powerModel = %lld\n", battary_info.powerModel);
        printf("\n battary_info.voltage = %lld\n", battary_info.voltage);

        str =  &cmd->cmds[0].pstr[0];
        strcpy(network_info.mac, str);
        sscanf(str, "%x:%x:%x:%x:%x:%x", &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]);
        put_buf(mac_addr, 6);
        netdev_set_mac_addr(mac_addr);

        printf("\n WIFI_VERSION hdv = %s\n", &cmd->cmds[0].pstr[1]);
        printf("\n WIFI_VERSION swv = %s\n", &cmd->cmds[0].pstr[2]);

        extern void set_hardware_version(char *hv, char *sv);
        set_hardware_version(&cmd->cmds[0].pstr[1], &cmd->cmds[0].pstr[2]);
        //获取唤醒原因
        avsdk_cmd_get_wakeup_source();

        post_msg_doorbell_task("doorbell_block_event_task", 2, DOORBELL_EVENT_REPORT_BATTARY, 0);

        avsdk_cmd_get_wifi_list();

#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
//    	post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_OPEN_RT_STREAM);
#endif // CONFIG_USB_UVC_AND_UAC_ENABLE

        break;
    case CMD_SET_POWER_MODE:
        break;
    case CMD_GET_DID_INFO:
        printf("\n CMD_GET_DID_INFO \n");
        extern void doorbell_read_cfg_info(u8 * cfg, int len);
        char *cfg = calloc(1, 1024);
        if (!cfg) {
            printf("\n %s %d\n", __func__, __LINE__);
            return 0;
        }
        doorbell_read_cfg_info(cfg, 1024);
        avsdk_cmd_put_did_info(cfg, strlen(cfg));
        free(cfg);
        break;
    default:
        break;
    }

    return 0;
}

int avsdk_cmd_recv(const uint8_t *buf, uint32_t len)
{
    return xspp_rx_data(buf, len);
}

int avsdk_cmd_send(uint8_t *buf, size_t size, void *ud)
{
//    puts("\n >>>avsdk_cmd_send \n");
    oal_recieve_user_msg(buf, size);
    return 0;
}

int avsdk_cmd_init(void)
{
    /*
    **用户发送函数
    **获取utc ms的函数
    **用户消息处理回调函数
    */
    avsdk_set_loglevel(LOG_LEVEL_INFO);
    xspp_init(avsdk_cmd_send, hal_GetUtcTime, avsdk_cmd_callback, NULL);
    return 0;
}
