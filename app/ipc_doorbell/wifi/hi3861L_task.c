
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
#include "wifi_ext.h"
#include "eg_http_server.h"
#include "dev_desc.h"
#include "database.h"
//#include "wifi-tools/voiceprint_cfg.h"
#include "asm/port_waked_up.h"
#include "gpio.h"
#include "hi3861l_task.h"


extern unsigned int time_lapse(unsigned int *handle, unsigned int time_out);
extern int http_virfile_reg(const char *path, const char *contents, unsigned long len);

struct fenice_config conf;
extern int user_cmd_cb(int cmd, char *buffer, int len, void *priv);
static struct server *ctp = NULL;
static u8 mac_addr[6];
static char ssid[64];
static void *wifi_dev = NULL;
static void *cli_hdl = NULL;
//CTP CDP 端口号




static struct ctp_server_info server_info = {
    .ctp_vaild = true,
    .ctp_port = CTP_CTRL_PORT,
    .cdp_vaild = true,
    .cdp_port = CDP_CTRL_PORT,
    .k_alive_type = CTP_ALIVE,
    /*.k_alive_type = CDP_ALIVE,*/
};





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

int hi3861_set_lan_setting_info(void *priv)
{
    set_lan_setting_info(priv);
    return 0;
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


    /*     printf("ftpd server init \n"); */
    /* extern void ftpd_vfs_interface_cfg(void); */
    /* ftpd_vfs_interface_cfg(); */
    /* stupid_ftpd_init("MAXUSERS=2\nUSER=FTPX 12345678     0:/      2   A\n", NULL); */

    printf("network mssdp init\n");
    network_mssdp_init();

#if 1
    //网络测试工具，使用iperf
    extern void iperf_test(void);
    iperf_test();
#endif

    /*
     *代码段功能:修改RTSP的URL
     *默认配置  :URL为rtsp://192.168.1.1/avi_pcm_rt/front.sd,//(avi_pcma_rt 传G7111音频)传JPEG实时流
     *
     */
#if 0
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
    struct fenice_config conf;

    strncpy(conf.protocol, "UDP", 3);
    conf.exit = fenice_video_rec_exit;
    conf.setup = fenice_video_rec_setup;
    conf.get_video_info = fenice_get_video_info;
    conf.get_audio_info = fenice_get_audio_info;
    conf.set_media_info = fenice_set_media_info;
    conf.port = RTSP_PORT;  // 当为0时,用默认端口554
    stream_media_server_init(&conf);


#ifdef CONFIG_EG_HTTP_SERVER
    eg_protocol_init();
#endif
}


void net_app_uninit(void)
{
    puts("ctp server init\n");
    server_close(ctp);

    puts("http server init\n");
    http_get_server_uninit(); //8080

    preview_uninit(); //2226

    playback_uninit();

    puts("ftpd server uninit\n");
    stupid_ftpd_uninit();


    video_rt_tcp_server_uninit();

#if __SDRAM_SIZE__ >= (8 * 1024 * 1024)
    extern void stream_media_server_uninit(void);
    stream_media_server_uninit();
#endif
}








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
#if defined CONFIG_VIDEO1_ENABLE
    return CONFIG_REC_PATH_1;
#elif defined CONFIG_VIDEO3_ENABLE
    return CONFIG_REC_PATH_3;
#else
    return CONFIG_REC_PATH_1;
#endif
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





u8 *wifi_get_module_mac_addr(void)
{
    return (u8 *)&mac_addr;
}

void log_print_time()
{

}


void sdio_irq_isr(void *priv)
{
    extern int sdio_irq_sem_post(void);
    sdio_irq_sem_post();
    sdio_irq_sem_post();
}


int sdio_irq_sel(void)
{
    static u8 flag = 0;
    if (!flag) {
        flag = 1;
        extern  void *Port_Wakeup_Reg(PORT_EVENT_E event, unsigned int gpio, PORT_EDGE_E edge, void (*handler)(void *priv), void *priv);
        Port_Wakeup_Reg(EVENT_IO_1, IO_PORTE_03, EDGE_POSITIVE, sdio_irq_isr, NULL);
    }
    return 0 ;
}


void hi3861l_task_q_post(u8 *data, u32 len)
{
    os_taskq_post("hi_channel_task", 3, HI_CHANNEL_RECV_DATA, data, len);
}

void wifi_rate_log_output(void *priv)
{
    printf("\n WIFI U= %d KB/s, D= %d KB/s\r\n", wifi_get_upload_rate() / 1024, wifi_get_download_rate() / 1024);
}

void hi_channel_task(void *priv)
{
    int res;
    int msg[32];
    char ssid[33];
    char pwd[64];

    sys_timer_add(NULL, wifi_rate_log_output, 1000);
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case HI_CHANNEL_RECV_DATA:
                    ;
//                            puts("\n HI_CHANNEL_RECV_DATA \n");
                    extern int avsdk_cmd_recv(const uint8_t *buf, uint32_t len);
                    avsdk_cmd_recv((char *)msg[2], msg[3]);
                    free((void *)msg[2]);
                    break;
                case HI_CHANNEL_NETWORK_CONFIG:
                    puts("\n HI_CHANNEL_NETWORK_CONFIG \n");
                    void qr_net_cfg_uninit(void);
                    qr_net_cfg_uninit();
                    extern int get_net_cfg_info(char *ssid, char *pwd, char *vcode);
                    extern int avsdk_cmd_sta_net_cionfig(char *ssid, char *pwd);

                    get_net_cfg_info(ssid, pwd, NULL);
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
                    extern int avsdk_cmd_push_file(struct cloud_upgrade_info * info);
                    avsdk_cmd_push_file((struct cloud_upgrade_info *)msg[2]);
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
                    ;
#if 0
                    puts("\n HI_CHANNEL_CMD_GET_BATTERY_VALUE \n");
#endif
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

                    extern int avsdk_cmd_set_power_mode(int value);
                    avsdk_cmd_set_power_mode(db_select("lowpower"));
                    msleep(20);
                    extern int avsdk_cmd_set_v_threshold(int64_t V0, int64_t V1) ;//V0此电压不唤醒 V1此电压禁止唤醒;
                    avsdk_cmd_set_v_threshold(3550, 3500);
                    msleep(20);
//                    avsdk_cmd_sta_net_cionfig("JL7F", "szjieli@1");
//                    msleep(20);

                    extern int avsdk_cmd_get_all_cfg(void);
                    avsdk_cmd_get_all_cfg();
                    break;
                case HI_CHANNEL_CMD_GET_PIR:
                    ;
                    int avsdk_cmd_get_pir_status(void);
                    avsdk_cmd_get_pir_status();
                    break;
                case HI_CHANNEL_CMD_SET_POWER_MODE:
                    ;
                    int avsdk_cmd_set_power_mode(int value);
                    avsdk_cmd_set_power_mode(db_select("lowpower"));
                    break;
                case HI_CHANNEL_CMD_AP_CHANGE_STA:
                    ;
                    extern int get_net_cfg_info(char *ssid, char *pwd, char *vcode);
                    extern int avsdk_cmd_sta_net_cionfig(char *ssid, char *pwd);

                    get_net_cfg_info(ssid, pwd, NULL);
                    avsdk_cmd_sta_net_cionfig(ssid, pwd);
                    msleep(100);
                    int avsdk_cmd_reset_set(int value) ;//1 重启 2 恢复出厂设置
                    avsdk_cmd_reset_set(1);
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
//    avi_unpkg_example_init();
    thread_fork("hi_channel_task", 17, 0x2E00, 64, 0, hi_channel_task, NULL);
    return 0;
}
//late_initcall(Hi3861L_init);
















