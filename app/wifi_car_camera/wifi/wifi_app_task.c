
#include "server/wifi_connect.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "os/os_compat.h"
#include "wifi_ext.h"
#include "system/init.h"
#include "lwip.h"
#include "server/ctp_server.h"
#include "server/net_server.h"
#include "device/device.h"
#include "system/app_core.h"
#include "server/server_core.h"
#include "action.h"
#include "system/timer.h"
#include "http/http_server.h"
#include "asm/debug.h"
#include "net_config.h"
#include "app_config.h"
#include "ftpserver/stupid-ftpd.h"
#include "device/wifi_dev.h"
#include "server/network_mssdp.h"
#include "streaming_media_server/fenice_config.h"
#include "system/timer.h"
#include "server/video_rt_tcp.h"
#include "server/net2video.h"
#include "p2p_dev.h"
#include "wifi_ext.h"
#include "eg_http_server.h"
#include "dev_desc.h"
#include "database.h"
#include "net/wifi-tools/voiceprint_cfg.h"


extern unsigned int time_lapse(unsigned int *handle, unsigned int time_out);
extern int http_virfile_reg(const char *path, const char *contents, unsigned long len);
extern int http_virfile_unreg(void);
extern int set_hostapd_config_file_normol_mode(void);
extern int set_hostapd_config_file_5g_mode(void);

struct fenice_config conf;
extern int user_cmd_cb(int cmd, char *buffer, int len, void *priv);
static struct server *ctp = NULL;
static u8 _net_dhcp_ready ;
static u8 mac_addr[6];
static char ssid[64];
static void *wifi_dev = NULL;
static void *cli_hdl = NULL;

//CTP CDP 端口号
#define WIFI_APP_TASK_NAME "wifi_app_task"

extern u8 airkiss_calcrc_bytes(u8 *p, unsigned int num_of_bytes);

enum WIFI_APP_MSG_CODE {
    WIFI_MSG_TICK_1_SEC,
    WIFI_MSG_SMP_CFG_START,
    WIFI_MSG_SMP_CFG_STOP,
    WIFI_MSG_SMP_CFG_COMPLETED,
    WIFI_MSG_SMP_CFG_TIMEOUT,
    WIFI_MSG_STA_SCAN_COMPLETED,
    WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC,
    WIFI_MSG_STA_DISCONNECTED,
    WIFI_MSG_AP_DISCONNECTED,
};


static struct ctp_server_info server_info = {
    .ctp_vaild = true,
    .ctp_port = CTP_CTRL_PORT,
    .cdp_vaild = true,
    .cdp_port = CDP_CTRL_PORT,
    .k_alive_type = CTP_ALIVE,
    /*.k_alive_type = CDP_ALIVE,*/
};


static struct airkiss_result {
    struct smp_cfg_result result;
    char scan_ssid_found;
} airkiss_result;

static struct voiceprint_result {
    char rand_str[8];
} voiceprint_result;

int net_dhcp_ready()
{
    return (_net_dhcp_ready);
}

static void wifi_app_timer_func(void *p)
{
    os_taskq_post(WIFI_APP_TASK_NAME, 1, WIFI_MSG_TICK_1_SEC);
}

static void wifi_taskq_post(int msg)
{
    int ret = 0;
    u8 retry = 0;

    do {
        ret = os_taskq_post(WIFI_APP_TASK_NAME, 1, msg);

        if (ret == OS_NO_ERR) {
            break;
        }

        msleep(50);
        retry++;
    } while (retry < 5);

    if (ret != OS_NO_ERR) {
        printf("post msg %d to wifi_app_task fail !!! \n", msg);
    }
}

void wifi_smp_connect(char *ssid, char *pwd, void *rand_str)
{
    struct cfg_info info = {0};

    if (ssid) {
        strcpy(voiceprint_result.rand_str, rand_str);
        info.mode = STA_MODE;
        info.ssid = ssid;
        info.pwd = pwd;
        dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
        dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
    } else {

        dev_ioctl(wifi_dev, DEV_GET_WIFI_SMP_RESULT, (u32)&info);

        if (info.smp_cfg.type) {
            printf("\r\n AIRKISS INFO, SSID = %s, PWD = %s, ssid_crc = 0x%x, ran_val = 0x%x \r\n", info.smp_cfg.ssid, info.smp_cfg.passphrase, info.smp_cfg.ssid_crc, info.smp_cfg.random_val);
            airkiss_result.result.type = 1 ;
            airkiss_result.result.ssid_crc = info.smp_cfg.ssid_crc;
            airkiss_result.result.random_val = info.smp_cfg.random_val;
            strcpy(airkiss_result.result.ssid, info.smp_cfg.ssid);
            strcpy(airkiss_result.result.passphrase, info.smp_cfg.passphrase);


            if (info.smp_cfg.type != AIRKISS_SMP_CFG
                || airkiss_result.result.ssid_crc == airkiss_calcrc_bytes((u8 *)airkiss_result.result.ssid, strlen(airkiss_result.result.ssid))) {
                airkiss_result.scan_ssid_found = 1;
                info.mode = STA_MODE;
                info.ssid = info.smp_cfg.ssid;
                info.pwd = info.smp_cfg.passphrase;
                dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
                dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
            } else {
                dev_ioctl(wifi_dev, DEV_NET_SCANF, 0);
            }
        } else {
            info.mode = STA_MODE;
            info.ssid = info.smp_cfg.ssid;
            info.pwd = info.smp_cfg.passphrase;
            dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
            dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
        }
    }
}

static void airkiss_ssid_check(void)
{
    u32 i;
    struct cfg_info info = {0};

    if (airkiss_result.result.type == 0 ||  airkiss_result.scan_ssid_found) {
        return;
    }

    info.sta_ssid_num = 32;
    info.sta_ssid_info = malloc(info.sta_ssid_num * sizeof(struct netdev_ssid_info));
    if (info.sta_ssid_info == NULL) {
        return;
    }
    dev_ioctl(wifi_dev, DEV_GET_STA_SSID_INFO, (u32)&info);

    for (i = 0; i < info.sta_ssid_num; i++) {
        if (!strncmp(airkiss_result.result.ssid, info.sta_ssid_info[i].ssid, strlen(airkiss_result.result.ssid))) {
CHECK_AIRKISS_SSID_CRC:

            if (airkiss_result.result.ssid_crc == airkiss_calcrc_bytes((u8 *)info.sta_ssid_info[i].ssid, strlen(info.sta_ssid_info[i].ssid))) {
                printf("find airkiss ssid = [%s]\r\n", info.sta_ssid_info[i].ssid);
                strcpy(airkiss_result.result.ssid, info.sta_ssid_info[i].ssid);
                airkiss_result.scan_ssid_found = 1;

                info.mode = STA_MODE;
                info.ssid = airkiss_result.result.ssid;
                info.pwd = airkiss_result.result.passphrase;
                dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);

                dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);

                free(info.sta_ssid_info);
                return;
            }
        } else {
            /*goto CHECK_AIRKISS_SSID_CRC;*/
        }
    }

    free(info.sta_ssid_info);
    printf("cannot found airkiss ssid[%s] !!! \n\n", airkiss_result.result.ssid);
}


static void airkiss_broadcast(void)
{
    int i, ret;
    int onOff = 1;
    int sock;
    struct sockaddr_in dest_addr;

    if (airkiss_result.result.type == 0) {
        return;
    }

    puts("airkiss_broadcast random_val \n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1) {
        printf("%s %d->Error in socket()\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_port = 0;
    ret = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if (ret == -1) {
        printf("%s %d->Error in bind()\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                     (char *)&onOff, sizeof(onOff));

    if (ret == -1) {
        printf("%s %d->Error in setsockopt() SO_BROADCAST\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    inet_pton(AF_INET, "255.255.255.255", &dest_addr.sin_addr.s_addr);
    dest_addr.sin_port = htons(10000);

    for (i = 0; i < 8; i++) {
        ret = sendto(sock, (unsigned char *)&airkiss_result.result.random_val, 1, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

        if (ret == -1) {
            printf("%s %d->Error in sendto\n", __FUNCTION__, __LINE__);
        }

        msleep(20);
    }

    memset(&airkiss_result, 0, sizeof(airkiss_result));

EXIT:

    if (sock != -1) {
        closesocket(sock);
    }
}


static unsigned int best_ch, least_cnt = -1;

static void get_best_ch_fn(unsigned int ch, unsigned int cnt)
{
    if (ch >= 12) {
        return;
    }

    if (cnt < least_cnt) { //cnt 越小 该信道干扰越小, 并且 禁止13信道
        least_cnt = cnt;
        best_ch = ch;
    }
}

#if defined CONFIG_NET_CLIENT

extern void video_disconnect_all_cli();
static int wifi_ipc_state_cb(void *priv, int on)
{
    if (on) {
    } else {
        video_disconnect_all_cli();
    }

    return 0;
}

#endif

#if defined CONFIG_NET_SERVER
extern void ipc_connect();
extern void ipc_disconnect();
static int wifi_ipc_state_cb(void *priv, int on)
{
    if (on) {
        ipc_connect();
    } else {
        ipc_disconnect();
    }

    return 0;
}


#endif

void wifi_enter_smp_cfg_mode(void)
{
    struct cfg_info info = {0};
    info.timeout = 100;
    /*dev_ioctl(wifi_dev, DEV_SET_SMP_AIRKISS_AES_ON_OFF, 1);*/
    dev_ioctl(wifi_dev, DEV_SMP_MODE, 0);
    dev_ioctl(wifi_dev, DEV_SET_SMP_CONFIG_TIMEOUT_SEC, (u32)&info);
}
int get_wifi_is_smp_mode(void)
{
    struct cfg_info info = {0};
    info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    return info.mode == SMP_CFG_MODE;
}

void wifi_return_sta_mode(void)
{
    struct cfg_info info = {0};
    info.mode = STA_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
}


static void wifi_smp_connect_task(void *p)
{
    struct cfg_info info;
    dev_ioctl(wifi_dev, DEV_GET_WIFI_SMP_RESULT, (u32)&info);

    info.mode = STA_MODE;
    dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
}

u8 *wifi_get_module_mac_addr(void)
{
    return (u8 *)&mac_addr;
}

static void wifi_sta_to_ap_mode_change(void)//用在STA模式密码不对或者找不到SSID，自动切换AP模式
{
    struct cfg_info info;
    info.mode = AP_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);

    if (!strcmp("", info.ssid)) {
        dev_ioctl(wifi_dev, DEV_GET_MAC, (u32)&mac_addr);
        sprintf(ssid, WIFI_CAM_PREFIX"%02x%02x%02x%02x%02x%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        info.ssid = ssid;
        info.pwd = "";
    }

    info.mode = AP_MODE;
    info.force_default_mode = 1;
    dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);
    puts("\n\nwifi_sta_to_ap_mode_change->STA to AP , system reset...\n\n");
    cpu_reset();
}
static void wifi_first_setup(void)
{
    struct cfg_info info;
    info.mode = AP_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    dev_ioctl(wifi_dev, DEV_GET_MAC, (u32)&mac_addr);

    if ((info.mode != AP_MODE && info.mode != STA_MODE) || !strstr(info.ssid, WIFI_CAM_NAME_MATCH)) {
        sprintf(ssid, WIFI_CAM_PREFIX"%02x%02x%02x%02x%02x%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        info.ssid = ssid;
        info.pwd = WIFI_CAM_WIFI_PWD;
        info.mode = AP_MODE;
        info.force_default_mode = 1;
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);
    } else {
        info.mode = AP_MODE;
        info.force_default_mode = 0;
        /*info.force_default_mode = 1;*/
        /*info.mode = SMP_CFG_MODE;*/
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);
    }

    extern char get_MassProduction(void);
    if (get_MassProduction()) {
        info.mode = STA_MODE;
        info.ssid = "test";
        info.pwd = "12345678";
        info.force_default_mode = 1;
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);
    }
}
static int network_user_callback(void *network_ctx, enum NETWORK_EVENT state, void *priv)
{

    struct cfg_info info;

    switch (state) {

    case WIFI_EVENT_MODULE_INIT:

#if 0
        gpio_set_hd(IO_PORTG_00, 0);
        gpio_set_hd(IO_PORTG_01, 0);
        gpio_set_hd(IO_PORTG_02, 0);
        gpio_set_hd(IO_PORTG_03, 0);
        gpio_set_hd(IO_PORTG_04, 0);
        gpio_set_hd(IO_PORTG_05, 0);

#endif


//wifi module port seting
        info.port_status = 0;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_CS, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_WKUP, (u32)&info);

        msleep(100);
        info.ssid = WIFI_CAM_PREFIX;
        info.pwd = "";
        info.mode = AP_MODE;
        info.force_default_mode = 0;
        dev_ioctl(wifi_dev, DEV_SET_DEFAULT_MODE, (u32)&info);

        break;

    case WIFI_EVENT_MODULE_START:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START\n");

//void linked_info_dump(int padapter, u8 benable);
//void *netdev_priv(struct net_device *dev);
//void  *net_dev_find(void);
//linked_info_dump(*(int*)netdev_priv(net_dev_find()), 1);

#if 0 //是否使能自动扫描获取最佳信道
        int rtw_scan(void);
        rtw_scan();//扫描1秒
        rtw_scan();//扫描1秒
        rtw_scan();//扫描1秒
        int rtw_get_best_ch(void (*get_best_ch_fn)(int, int));
        rtw_get_best_ch(get_best_ch_fn);
        printf("hostapd_set_wifi_channel = %d\r\n", best_ch);
        int hostapd_set_wifi_channel(int channel);
        hostapd_set_wifi_channel(best_ch);
#endif


//设置VM保存AP_MODE
#if defined (WIFI_CAM_SUFFIX)
        sprintf(ssid, WIFI_CAM_PREFIX WIFI_CAM_SUFFIX);
#else
        wifi_first_setup();

#if 0
        dev_ioctl(wifi_dev, DEV_GET_MAC, (u32)&mac_addr);
        sprintf(ssid, WIFI_CAM_PREFIX"%02x%02x%02x%02x%02x%02x"
                , mac_addr[0]
                , mac_addr[1]
                , mac_addr[2]
                , mac_addr[3]
                , mac_addr[4]
                , mac_addr[5]);
        info.ssid = ssid;
        info.pwd = WIFI_CAM_WIFI_PWD;
        info.mode = AP_MODE;
        info.force_default_mode = 0;
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);

#endif
#endif
        break;

    case WIFI_EVENT_MODULE_STOP:
        puts("|network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        break;

    case WIFI_EVENT_AP_START:
        puts("|network_user_callback->WIFI_EVENT_AP_START\n");
        break;

    case WIFI_EVENT_AP_STOP:
        puts("|network_user_callback->WIFI_EVENT_AP_STOP\n");

#ifdef USE_MARVEL8801
        //8801要if 1，8189不能开，开了切换不了模式
        info.port_status = 0;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;

    case WIFI_EVENT_STA_START:
        puts("|network_user_callback->WIFI_EVENT_STA_START\n");
        info.timeout = 20;//20s timeout
        dev_ioctl(wifi_dev, DEV_SET_STA_MODE_TIMEOUT_SEC, (u32)&info);
        break;

    case WIFI_EVENT_MODULE_START_ERR:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;

    case WIFI_EVENT_STA_STOP:
        puts("|network_user_callback->WIFI_EVENT_STA_STOP\n");
#ifdef USE_MARVEL8801
        info.port_status = 0;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;

    case WIFI_EVENT_STA_DISCONNECT:
        puts("|network_user_callback->WIFI_STA_DISCONNECT\n");
        wifi_taskq_post(WIFI_MSG_STA_DISCONNECTED);
#ifdef CONFIG_RTSP_CLIENT_ENABLE
        void net_rtsp_uninit(void);
        void rtsp_cli_uninit(void);
        rtsp_cli_uninit();
        //net_rtsp_uninit();
#endif
        break;

    case WIFI_EVENT_STA_SCAN_COMPLETED:

        /* if(wpa_supplicant_get_state() != STA_WPA_CONNECT_COMPLETED) */
        /* { */
        puts("|network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
        wifi_taskq_post(WIFI_MSG_STA_SCAN_COMPLETED);
        /* } */
        break;

    case WIFI_EVENT_STA_CONNECT_SUCC:
        dev_ioctl(wifi_dev, DEV_GET_WIFI_CHANNEL, (u32)&info);
        printf("|network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", info.sta_channel);

        break;

    case WIFI_EVENT_MP_TEST_START:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        /*extern spec_uart_init(void);*/
        /*extern wifi_mp_uart_test(void);*/
        /*spec_uart_init();*/
        // extern int spec_uart_send(char *buf, u32 len);
        // set_putbyte_remap((void*)-1);   // spec_uart_send
        /*wifi_mp_uart_test();*/
        break;

    case WIFI_EVENT_MP_TEST_STOP:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;

    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
#ifdef CONFIG_STA_AUTO_MODE	//打开STA模式扫描连接扫描不到wifi名称，自动切回AP模式，防止死循环扫描无法回AP模式
        static u8 cnt = 0;
        cnt++;

        if (cnt >= 3) {
            wifi_sta_to_ap_mode_change();
        }

#endif
        break;

    case WIFI_EVENT_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL .....\n");
#ifdef CONFIG_STA_AUTO_MODE	//打开STA模式连接路由器密码错误，自动切回AP模式，防止死循环连接路由器
        wifi_sta_to_ap_mode_change();
#endif
        break;

    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");
        wifi_taskq_post(WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC);

#ifdef SLAVE_STA_MODE
        extern void *net_ctp_init(void);
        net_ctp_init();
#endif

#ifdef CONFIG_RTSP_CLIENT_ENABLE
        extern void net_rtsp_callback(char *frm_buf, unsigned int sz);
        extern void net_rtsp_init(void);
        extern int rtsp_cli_init(char *url, void *(callback)());//librtspcli - 函数声明

        net_rtsp_init();
        rtsp_cli_init("rtsp://192.168.1.1/avi_pcm_rt/front.sd", net_rtsp_callback); //初始化RTSP  url - 服务器为设备
#endif
        break;

    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_P2P_START:
        puts("|network_user_callback->WIFI_EVENT_P2P_START\n");
        break;

    case WIFI_EVENT_P2P_STOP:
        puts("|network_user_callback->WIFI_EVENT_P2P_STOP\n");
        break;

    case WIFI_EVENT_P2P_GC_DISCONNECTED:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_DISCONNECTED\n");
        break;

    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC\n");
        break;

    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_SMP_CFG_START:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_START\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_START);
        break;

    case WIFI_EVENT_SMP_CFG_STOP:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_STOP\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_STOP);
        break;

    case WIFI_EVENT_SMP_CFG_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_TIMEOUT\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_TIMEOUT);
        break;

    case WIFI_EVENT_SMP_CFG_COMPLETED:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_COMPLETED\n");
        wifi_taskq_post(WIFI_MSG_SMP_CFG_COMPLETED);
        break;

    case WIFI_EVENT_PM_SUSPEND:
        puts("|network_user_callback->WIFI_EVENT_PM_SUSPEND\n");
        break;

    case WIFI_EVENT_PM_RESUME:
        puts("|network_user_callback->WIFI_EVENT_PM_RESUME\n");
        break;

    case WIFI_EVENT_AP_ON_ASSOC:
        ;
        struct eth_addr *hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_ASSOC hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);

        break;

    case WIFI_EVENT_AP_ON_DISCONNECTED:
        hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_DISCONNECTED hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);

#ifdef CONFIG_RTSP_SERVER_ENABLE
        //强制断开所有RTSP链接,实际上应该只断开响应客户端
        void stream_media_server_dhwaddr_close(struct eth_addr * dhwaddr);
        stream_media_server_dhwaddr_close(hwaddr);
#endif
        ctp_keep_alive_find_dhwaddr_disconnect((struct eth_addr *)hwaddr->addr);
        cdp_keep_alive_find_dhwaddr_disconnect((struct eth_addr *)hwaddr->addr);
        wifi_taskq_post(WIFI_MSG_AP_DISCONNECTED);
        break;

    default:
        break;
    }


    return 0;
}


static void wifi_set_lan_setting_info(void)
{
    struct lan_setting lan_setting_info = {

        .WIRELESS_IP_ADDR0  = 192,
        .WIRELESS_IP_ADDR1  = 168,
        .WIRELESS_IP_ADDR2  = 1,
        .WIRELESS_IP_ADDR3  = 1,

        .WIRELESS_NETMASK0  = 255,
        .WIRELESS_NETMASK1  = 255,
        .WIRELESS_NETMASK2  = 255,
        .WIRELESS_NETMASK3  = 0,

        .WIRELESS_GATEWAY0  = 192,
        .WIRELESS_GATEWAY1  = 168,
        .WIRELESS_GATEWAY2  = 1,
        .WIRELESS_GATEWAY3  = 1,

        .SERVER_IPADDR1  = 192,
        .SERVER_IPADDR2  = 168,
        .SERVER_IPADDR3  = 1,
        .SERVER_IPADDR4  = 1,

        .CLIENT_IPADDR1  = 192,
        .CLIENT_IPADDR2  = 168,
        .CLIENT_IPADDR3  = 1,
        .CLIENT_IPADDR4  = 2,

        .SUB_NET_MASK1   = 255,
        .SUB_NET_MASK2   = 255,
        .SUB_NET_MASK3   = 255,
        .SUB_NET_MASK4   = 0,
    };

    struct cfg_info info;
    info.__lan_setting_info = &lan_setting_info;
    dev_ioctl(wifi_dev, DEV_SET_LAN_SETTING, (u32)&info);
}


void net_app_init(void)
{
    ctp = server_open("ctp_server", (void *)&server_info);

    if (!ctp) {
        printf("ctp server fail\n");
    }

    puts("http server init\n");
    http_virfile_reg(DEV_DESC_PATH, DEV_DESC_CONTENT, strlen(DEV_DESC_CONTENT)); //注册虚拟文件描述文档,可在dev_desc.h修改
    http_get_server_init(HTTP_PORT); //8080

    printf("video preview playback init \n");
    preview_init(VIDEO_PREVIEW_PORT, NULL); //2226
    playback_init(VIDEO_PLAYBACK_PORT, NULL);
    video_rt_tcp_server_init(2229);


    printf("ftpd server init \n");
    extern void ftpd_vfs_interface_cfg(void);
    ftpd_vfs_interface_cfg();
    stupid_ftpd_init("MAXUSERS=2\nUSER=FTPX 12345678     0:/      2   A\n", NULL);

    printf("network mssdp init\n");
    network_mssdp_init();

#ifdef CONFIG_IPERF_ENABLE
    //网络测试工具，使用iperf
    extern void iperf_test(void);
    iperf_test();
#endif
    /*
     *代码段功能:修改RTSP的URL
     *默认配置  :URL为rtsp://192.168.1.1/avi_pcm_rt/front.sd,//(avi_pcma_rt 传G7111音频)传JPEG实时流
     *
     */
#ifdef CONFIG_RTSP_SERVER_ENABLE
#if 1
    char *user_custom_name = "avi_pcma_rt";
    char *user_custom_content =
        "stream\r\n \
	file_ext_name avi\r\n \
	media_source live\r\n \
	priority 1\r\n \
	payload_type 26\r\n \
	clock_rate 90000\r\n \
	encoding_name JPEG\r\n \
	coding_type frame\r\n \
	byte_per_pckt 1458\r\n \
	stream_end\r\n \
	stream\r\n \
	file_ext_name pcm\r\n \
	media_source live\r\n \
	priority 1\r\n \
	payload_type 8\r\n \
	encoding_name PCMA\r\n \
	clock_rate 8000\r\n \
	stream_end";
    extern void rtsp_modify_url(const char *user_custom_name, const char *user_custom_content);
    rtsp_modify_url(user_custom_name, user_custom_content);
#endif

    extern int stream_media_server_init(struct fenice_config * conf);
    extern int fenice_get_video_info(struct fenice_source_info * info);
    extern int fenice_get_audio_info(struct fenice_source_info * info);
    extern int fenice_set_media_info(struct fenice_source_info * info);
    extern int fenice_video_rec_setup(void);
    extern int fenice_video_rec_exit(void);
    struct fenice_config conf = {0};

    strncpy(conf.protocol, "UDP", 3);
    conf.exit = fenice_video_rec_exit;
    conf.setup = fenice_video_rec_setup;
    conf.get_video_info = fenice_get_video_info;
    conf.get_audio_info = fenice_get_audio_info;
    conf.set_media_info = fenice_set_media_info;
    conf.port = RTSP_PORT;  // 当为0时,用默认端口554
    stream_media_server_init(&conf);
#endif

#ifdef CONFIG_EG_HTTP_SERVER
    eg_protocol_init();
#endif
}


void net_app_uninit(void)
{
    puts("ctp server init\n");
    server_close(ctp);

    http_virfile_unreg();

    puts("http server init\n");
    http_get_server_uninit(); //8080

    preview_uninit(); //2226

    playback_uninit();

    puts("ftpd server uninit\n");
    stupid_ftpd_uninit();

    network_mssdp_uninit();

    video_rt_tcp_server_uninit();

#ifdef CONFIG_RTSP_SERVER_ENABLE
    extern void stream_media_server_uninit(void);
    stream_media_server_uninit();
#endif
}

void wifi_on(void)
{
    static u8 flag;
    dev_ioctl(wifi_dev, DEV_NETWORK_START, 0);
    if (!flag) {
        flag = 1;
        net_app_init();
    }
}


void wifi_off(void)
{
    //net_app_uninit();//不需要卸载网络服务
    dev_ioctl(wifi_dev, DEV_NETWORK_STOP, 0);
}

/*#define RTOS_STACK_CHECK_ENABLE*/
void malloc_debug_start(void);
void malloc_debug_show(void);
void malloc_stats(void);
#ifdef RTOS_STACK_CHECK_ENABLE
static void rtos_stack_check_func(void *p)
{
#if 0
    char pWriteBuffer[2048];
    extern void vTaskList(char *pcWriteBuffer);
    vTaskList((char *)&pWriteBuffer);
    printf(" \n\ntask_name          task_state priority stack task_num\n%s\n", pWriteBuffer);
    malloc_stats();
    //extern void malloc_debug_start(void);
    //malloc_debug_start();
    //malloc_debug_show();
#endif
    extern void get_task_state(void *parm);
    get_task_state(NULL);//1分钟以内调用一次才准确
}
#endif

void wifi_app_task(void *priv)
{
    int res, err = 0;
    int msg[32];
    unsigned int timehdl = 0;
    struct cfg_info info = {0};

    printf(">>>>>>> wifi dev running <<<<<<<<\r\n");
#ifdef CONFIG_FORCE_RESET_VM
    extern void vm_eraser(void);
    vm_eraser();
#endif

#ifdef USE_5G2G_TOW_CHANNEL
    char wfmode = db_select("wfmode");
    if (wfmode == 1) {
        set_hostapd_config_file_5g_mode();
        db_update("wfmode", 1);
        db_flush();
    } else {
        set_hostapd_config_file_normol_mode();
        db_update("wfmode", 0);
        db_flush();
    }
#endif

    wifi_dev = dev_open("wifi", NULL);

    if (!wifi_dev) {
        printf(">>>>>>> wifi dev open err !!!! <<<<<<<<\r\n");
        return ;

    }

#ifdef RTOS_STACK_CHECK_ENABLE
    sys_timer_add(NULL, rtos_stack_check_func, 30 * 1000);
#endif
    info.cb = network_user_callback;
    info.net_priv = NULL;
    dev_ioctl(wifi_dev, DEV_SET_CB_FUNC, (u32)&info);

#if 0
    printf("\n >>>> DEV_SET_WIFI_POWER_SAVE<<<<   \n");
    dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER_SAVE, 0);//打开就启用低功耗模式, 只有STA模式才有用
#endif

#if 1
    printf("\n >>>> DEV_SET_WIFI_TX_PWR_BY_RATE<<<   \n");

    info.tx_pwr_lmt_enable = 0;//  解除WIFI发送功率限制
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_LMT_ENABLE, (u32)&info);
    info.tx_pwr_by_rate = 1;// 设置WIFI根据不同datarate打不同power
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_BY_RATE, (u32)&info);
#endif


    wifi_set_lan_setting_info();

    wifi_on();

#if 0
    info.p2p_role = 0;
    dev_ioctl(wifi_dev, DEV_P2P_MODE, (u32)&info);
#endif

    sys_timer_add(NULL, wifi_app_timer_func, 1000);

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));

        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case WIFI_MSG_TICK_1_SEC:
                    if (time_lapse(&timehdl, 5 * 1000)) {
                        malloc_stats();
                        if (wifi_module_is_init()) {
                            printf("WIFI U= %d KB/s, D= %d KB/s\r\n", wifi_get_upload_rate() / 1024, wifi_get_download_rate() / 1024);
#if 0
                            /*stats_display();*/

                            if (get_cur_wifi_info()->mode == STA_MODE) {
                                get_rx_signal();
                            }

#endif
                        }
                    }

                    break;

                case WIFI_MSG_SMP_CFG_START:
                    memset(&airkiss_result, 0, sizeof(airkiss_result));
                    /*voiceprint_cfg_start();*/
                    break;

                case WIFI_MSG_SMP_CFG_STOP:
                    /*voiceprint_cfg_stop();*/
                    break;

                case WIFI_MSG_SMP_CFG_COMPLETED:
                    wifi_smp_connect(NULL, NULL, NULL);
                    break;

                case WIFI_MSG_SMP_CFG_TIMEOUT: {
                    struct sys_event evt;
                    evt.arg = "net";
                    evt.type = SYS_NET_EVENT;
                    evt.u.net.event = NET_EVENT_SMP_CFG_TIMEOUT;
                    sys_event_notify(&evt);
                }
                break;

                case WIFI_MSG_STA_SCAN_COMPLETED:
                    airkiss_ssid_check();
                    break;

                case WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC:
                    airkiss_broadcast();
                    /*voiceprint_broadcast();*/
                    _net_dhcp_ready = 1;
                    {
                        struct sys_event evt;
                        evt.arg = "net";
                        evt.type = SYS_NET_EVENT;
                        evt.u.net.event = NET_EVENT_CONNECTED;
                        sys_event_notify(&evt);
                    }


#ifdef CONFIG_EG_HTTP_CLIENT
                    extern int https_client_init(void);
                    https_client_init();
#endif
                    break;

                case WIFI_MSG_STA_DISCONNECTED:
                    _net_dhcp_ready = 0;
                    {
                        struct sys_event evt;
                        evt.arg = "net";
                        evt.type = SYS_NET_EVENT;
                        evt.u.net.event = NET_EVENT_DISCONNECTED;
                        sys_event_notify(&evt);
                    }
                    break;
                case WIFI_MSG_AP_DISCONNECTED:
                    break;
                default :
                    break;
                }

                break;

            default:
                break;
            }

            break;
        }
    }
}


int wireless_net_init(void)//主要是create wifi 线程的
{
    puts("wifi early init \n");
    thread_fork(WIFI_APP_TASK_NAME, 10, 1600, 64, 0, wifi_app_task, NULL);
    return 0;
}
#if defined CONFIG_WIFI_ENABLE
late_initcall(wireless_net_init);
#endif


unsigned short DUMP_PORT()
{
    return _DUMP_PORT;
}

unsigned short FORWARD_PORT()
{
    return _FORWARD_PORT;
}

unsigned short BEHIND_PORT()
{
    return _BEHIND_PORT;
}

const char *get_rec_path_0()
{
    return NULL;
}
const char *get_rec_path_1()
{
    return CONFIG_REC_PATH_0;
}
const char *get_rec_path_2()
{
    return CONFIG_REC_PATH_1;
}
const char *get_rec_path_3()
{
    return CONFIG_REC_PATH_2;
}
const char *get_rec_emr_path_1()
{
#ifdef CONFIG_EMR_REC_PATH_0
    return CONFIG_EMR_REC_PATH_0;
#else
    return NULL;
#endif
}
const char *get_rec_emr_path_2()
{
#ifdef CONFIG_EMR_REC_PATH_1
    return CONFIG_EMR_REC_PATH_1;
#else
    return NULL;
#endif
}
const char *get_rec_emr_path_3()
{
#ifdef CONFIG_EMR_REC_PATH_2
    return CONFIG_EMR_REC_PATH_2;
#else
    return NULL;
#endif
}
const char *get_root_path()
{
    return CONFIG_ROOT_PATH;
}

static struct cfg_info wifi_info;
char *get_wifi_ssid(void)
{
    wifi_info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&wifi_info);
    return wifi_info.ssid;
}


char *get_wifi_pwd(void)
{
    wifi_info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&wifi_info);
    return wifi_info.pwd;
}

void get_mac_addr(u8 *buf)
{
    dev_ioctl(wifi_dev, DEV_GET_MAC, (u32)buf);
}
#if defined CONFIG_NET_SERVER
static void dhcp_charge_device_event_handler(struct sys_event *event)
{
    if (!strncmp(event->arg, "dhcp_srv", 8)) {
        if (event->u.dev.event == DEVICE_EVENT_CHANGE) {
            wifi_ipc_state_cb(NULL, 1);

        }
    }
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, dhcp_charge_device_event_handler, 0);

#endif

#if 0
void sdio_recv_pkg_irq(void)
{
    static u32 thdll, count222;
    int ret22;
    ret22 = time_lapse(&thdll, 1000);

    if (ret22) {
        printf("sdio_recv_cnt = %d,  %d \r\n", ret22, count222);
        count222 = 0;
    }

    ++count222;
}
#endif

