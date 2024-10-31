#define _STDIO_H_
#include "app_config.h"
#include "xciot_api.h"
#include "system/includes.h"
#include "action.h"
#include "server/network_mssdp.h"
#include "streaming_media_server/fenice_config.h"
#include "sock_api/sock_api.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "doorbell_event.h"

static int  storage_format_timer_id;
static int percent;



static void storage_format_timer(void *arg)
{
    int32_t  conn_id = (int32_t)(arg);

    percent += 2;
    if (percent < 99) {
        avsdk_storage_format_rate(conn_id, 0, percent);
    }

}

void storage_format_thread(void *arg)
{

    int32_t  conn_id = (int32_t)(arg);
    printf("conn_id: %d ", conn_id);
    // here we notice the user progress of upgrading

    avsdk_storage_format_rate(conn_id, 0, 0);
    storage_format_timer_id = sys_timer_add(arg, storage_format_timer, 1000);
    int err = storage_device_format();
    percent = err ? -1 : 100;
    // reach progress 100%, upgrade finish success
    avsdk_storage_format_rate(conn_id, 0, percent);
    sys_timer_del(storage_format_timer_id);
    storage_format_timer_id = 0;
    extern int cloud_playback_list_clear(void);
    cloud_playback_list_clear();
    percent = 0;

    return ;
}
void storage_format_thread_create(void *arg)
{
    thread_fork("storage_format_thread", 20, 0x1000, 0, 0, storage_format_thread, arg);
}


static int doorbell_query_battery_timer_id;

void doorbell_query_battery_timer(void *priv)
{
    post_msg_doorbell_task("doorbell_heart_task", 1, DOORBELL_EVENT_QUERY_BATTARY);
}
void doorbell_query_battery_timer_add(void)
{
    if (!doorbell_query_battery_timer_id) {
        doorbell_query_battery_timer_id = sys_timer_add_to_task("sys_timer", NULL, doorbell_query_battery_timer, 2500);
    }
}

static int doorbell_query_qos_timer_id;
void doorbell_query_qos_timer(void *priv)
{
    extern u8 get_app_connect_flag(void);
    if (get_app_connect_flag()) {
        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_QOS);
    }

}

void doorbell_query_qos_timer_add(void)
{
    if (!doorbell_query_qos_timer_id) {
        doorbell_query_qos_timer_id = sys_timer_add_to_task("sys_timer", NULL, doorbell_query_qos_timer, 5 * 60 * 1000);
    }
}

static int doorbell_netcfg_timeout_timer_id;
void doorbell_netcfg_timeout_timer(void *priv)
{
    if (!get_sdcard_upgrade_status()) {
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_ENTER_SLEEP, NULL);
    }
    doorbell_netcfg_timeout_timer_id = 0;
}

void doorbell_netcfg_timeout_timer_add(void)
{
    if (!doorbell_netcfg_timeout_timer_id) {
        doorbell_netcfg_timeout_timer_id = sys_timeout_add_to_task("sys_timer", NULL, doorbell_netcfg_timeout_timer, CONFIG_NETCONFIG_TIMEOUT);
    }
}

void doorbell_netcfg_timeout_timer_del(void)
{
    if (doorbell_netcfg_timeout_timer_id) {
        sys_timer_del(doorbell_netcfg_timeout_timer_id);
    }
}

void doorbell_blue_led_off_timer(void *priv)
{
    extern void doorbell_red_blue_off(void);
    doorbell_red_blue_off();
}

#define RT_TALK_BUFFER_SIZE     256 * 1024
static char buffer[RT_TALK_BUFFER_SIZE];
static cbuffer_t __cbuffer;
static void *audio_dev = NULL;
static void audio_dec_irq_handler(void *priv, void *data, int len)
{
    int rlen = len;
    static u8 buf[1024];
    u16 data_len;
    cbuffer_t *cbuffer = (cbuffer_t *)priv;
    data_len = cbuf_read(cbuffer, buf, rlen);
    if (data_len == 0) {
        memset((u8 *)data, 0, len);
        int bindex = 0;
        dev_ioctl(audio_dev, AUDIOC_STREAM_OFF, (u32)&bindex);
    } else {

        memcpy(data, buf, data_len);
        if (data_len < rlen) {
            memset((u8 *)data + data_len, 0, len - data_len);
        }
    }
}

struct adpcm_state {
    int valprev;
    int index;
};
static int old_volume;
static int audio_dev_init(void)
{
    struct audio_format f = {0};
    if (!audio_dev) {
        cbuf_init(&__cbuffer, buffer, RT_TALK_BUFFER_SIZE);
        audio_dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
        f.volume        = net_video_rec_get_dac_volume();
        f.channel       = 1;
        f.sample_rate   = net_video_rec_get_audio_rate();
        f.priority      = 1;
//        f.type          = 0x1;
        f.frame_len     = 1024;
        f.sample_source     = "dac";
        int err = dev_ioctl(audio_dev, AUDIOC_SET_FMT, (u32)&f);
        u32 arg[2];
        arg[0] = (u32)&__cbuffer;
        arg[1] = (u32)audio_dec_irq_handler;
        dev_ioctl(audio_dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);
        old_volume = f.volume;
    } else if (audio_dev && old_volume != net_video_rec_get_dac_volume()) {
        dev_close(audio_dev);
        cbuf_clear(&__cbuffer);
        audio_dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
        f.volume        = net_video_rec_get_dac_volume();
        f.channel       = 1;
        f.sample_rate   = net_video_rec_get_audio_rate();
        f.priority      = 1;
//        f.type          = 0x1;
        f.frame_len     = 1024;
        f.sample_source     = "dac";
        int err = dev_ioctl(audio_dev, AUDIOC_SET_FMT, (u32)&f);
        u32 arg[2];
        arg[0] = (u32)&__cbuffer;
        arg[1] = (u32)audio_dec_irq_handler;
        dev_ioctl(audio_dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);
        old_volume = f.volume;
    }
}




extern void adpcm_decoder(char *indata, short *outdata, int len, struct adpcm_state *state);
static void doorbell_play_voice_file(void *path)
{

    int len;
    void *fd = NULL;
    char *input_buffer = NULL;
    char *output_buffer = NULL;
    struct adpcm_state state = {0};


    audio_dev_init();

    fd = fopen(path, "r");
    if (!fd) {
        return;
    }
    len = flen(fd);
    printf("\n>>>>>>>>>>>>>>>>>>>>>> len =%d \n", len);
    input_buffer = malloc(len);
    output_buffer = malloc(len * 4);
    if (!input_buffer || !output_buffer) {

        goto __exit;
    }

    fread(fd, input_buffer, len);


    adpcm_decoder(input_buffer, output_buffer, len * 2, &state);

    cbuf_write(&__cbuffer, output_buffer, len * 4);

    int bindex = 0;

    dev_ioctl(audio_dev, AUDIOC_STREAM_ON, (u32)&bindex);


    doorbell_event_dac_mute(1);

__exit:
    if (input_buffer) {
        free(input_buffer);
    }
    if (output_buffer) {
        free(output_buffer);
    }
    fclose(fd);
}

void doorbell_play_voice(const char *name)
{
    int ret;
    static int retry;
    char path[128];

    snprintf(path, sizeof(path), "mnt/spiflash/audlogo/%s", name);

    doorbell_play_voice_file(path);

    if (!strcmp(name, "dingdong.adp")) {
        doorbell_blue_led_on();
        sys_timeout_add(NULL, doorbell_blue_led_off_timer, 3000);
    }

    if (!strcmp(name, "NetCfgEnter.adp")) {
        doorbell_red_led_on();
//        doorbell_netcfg_timeout_timer_add();
    }

    if (!strcmp(name, "SsidRecv.adp")) {
        doorbell_red_led_off();
//        doorbell_netcfg_timeout_timer_del();
    }

}


struct isp_effect {
    u8 *buf;
    u32 size;
};
static const char *isp_cfg_file[] = {
    "mnt/spiflash/res/isp_cfg_1.bin",
    "mnt/spiflash/res/isp_cfg_4.bin",
};
static struct isp_effect effect[sizeof(isp_cfg_file) / sizeof(char *)];

void isp_effect_init(void)
{
    void *fd = NULL;
    printf("\n sizeof(effect) / sizeof(struct isp_effect) = %d \n", sizeof(effect) / sizeof(struct isp_effect));
    for (u8 i = 0; i < sizeof(effect) / sizeof(struct isp_effect); i++) {
        fd = fopen(isp_cfg_file[i], "r");
        if (!fd) {
            puts("cfg file err.\n");
            return;
        }
        effect[i].size = flen(fd);
        effect[i].buf = malloc(effect[i].size);
        if (!effect[i].buf) {
            puts("no mem.\n");
            return;
        }
        fread(fd, effect[i].buf, effect[i].size);
        fclose(fd);
    }
}


void isp_effect_user_day(void)
{
    if (!effect[0].buf) {
        return;
    }
    extern void doorbell_irled_off(void);
    doorbell_irled_off();
    if (ircut_set_isp_scenes(0, effect[0].buf, effect[0].size)) {
        //关闭IRCUT失败，置IRCUT状态为开启状态
        set_ircut_status(IRCUT_ON);
    } else {
        //关闭ircut
        extern void doorbell_ircut_off();
        doorbell_ircut_off();
    }
}

void isp_effect_user_night(void)
{
    if (!effect[0].buf) {
        return;
    }
    //开启ircut
    if (ircut_set_isp_scenes(1, effect[1].buf, effect[1].size)) {
        //开启IRCUT失败，置IRCUT状态为关闭状态
        set_ircut_status(IRCUT_OFF);
    } else {

        extern void doorbell_ircut_on();
        doorbell_ircut_on();
        //打开红外灯
        extern void doorbell_irled_on(void);
        doorbell_irled_on();
    }
}



void avsdk_cmd_sleep_timer(void *priv)
{
    printf("\n >>>>>>>>>>>>>>%s\n", __func__, __LINE__);
    hi3861l_reset();
    printf("\n >>>>>>>>>>>>>>%s\n", __func__, __LINE__);
}

void doorbell_sleep_timeout(void)
{
    sys_timeout_add(NULL, avsdk_cmd_sleep_timer, 15 * 1000);
}

static int doorbell_wait_completion_id;


void video_rec_control_start_cb(void *priv)
{
    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_START_REC);
    wait_completion_del(doorbell_wait_completion_id);
    doorbell_wait_completion_id = 0;
}
int video_rec_control_start_condition(void)
{
    if (is_video_rec_mode() && storage_device_ready()) {
        return 1;
    }
    return 0;
}

static u8 need_notify_wakeup_status(void)
{
    static int timeout = 0;
    //5s内重复按过滤
    if (timeout == 0 || (timer_get_ms() - timeout) >= 3000) {
        timeout = timer_get_ms();
        return 1;
    }
    return 0;
}

void doorbell_app_deal_wakeup_source(int status)
{
    if (status == INIT_WAKEUP) {
        set_wakeup_status(status);
    } else if (status == NETWORK_WAKEUP) {
        set_wakeup_status(status);
    } else if (status == PIR_WAKEUP || status == KEY_WAKEUP) {
        //按键唤醒间隔3s播提示音
        if (status == KEY_WAKEUP) {
            if (need_notify_wakeup_status()) {
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "dingdong.adp");
                extern void doorbell_433_ctrl(void);
                doorbell_433_ctrl();
            }
        }
        set_wakeup_status(status);

        if (get_video_rec_state() == 0 && doorbell_wait_completion_id == 0) {
            char *str = NULL;
            if (status == PIR_WAKEUP) {
                str = "PIR";
            } else if (status == KEY_WAKEUP) {
                str = "KEY";
            }
            set_rec_file_name_prefix(str);
//            doorbell_wait_completion_id = wait_completion(video_rec_control_start_condition, video_rec_control_start_cb, (void *)0);
        }
    } else if (status == BREAK_WAKEUP) {
        set_wakeup_status(status);
        //增加强拆提示音
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "alarm.adp");
    } else if (status == LOWPOWER_WAKEUP) {
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "LowBattery.adp");
        set_wakeup_status(status);
    }
}

void mass_production_socket_accpet(void *priv)
{
    void *sock_hdl = priv;
    void *cli = NULL;
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int ret;
    char *buf = calloc(1, 1024);
    if (!buf) {
        printf("\n %s %d no mem\n", __func__, __LINE__);
        goto __exit;
    }

    while (1) {
        cli  = sock_accept(sock_hdl, (struct sockaddr *)&addr, &len, NULL, NULL);
        if (cli == NULL) {
            printf("\n[ %s sock_accept err ]\n", __func__);
            continue;
        }
        memset(buf, 0, 1024);
        int offset = 0;
__recv_again:
        ret = sock_recv(cli, buf + offset, 1024 - offset, 0);
        if (ret > 0) {
            offset += ret;
            json_object *root = NULL;
            json_object *key_cmd = NULL;
            json_object *key_len = NULL;
            json_object *key_data = NULL;
            root =  json_tokener_parse(buf);
            if (root) {
                key_cmd =  json_object_object_get(root, "cmd");
                key_len =  json_object_object_get(root, "len");
                key_data =  json_object_object_get(root, "data");
                if (key_cmd && key_len && key_data) {
                    extern void doorbell_write_cfg_buf_info(void *buf, int len);
                    doorbell_write_cfg_buf_info(json_object_get_string(key_data), json_object_get_int(key_len));
                    char str[] = "{\"cmd\":0,\"len\":3,\"data\":\"suc\"}";
                    sock_send(cli, str, sizeof(str), 0);
                    printf("\n did data is write ok\n");
                    sock_unreg(cli);
                    continue;
                }
            } else {
                printf("\n%s %d no enough data\n", __func__, __LINE__);
                goto __recv_again;
            }
        } else {
            printf("\n%s %d cli disconnect\n", __func__, __LINE__);
            sock_unreg(cli);
            continue;
        }
    }
__exit:
    if (buf) {
        free(buf);
    }
    if (cli) {
        sock_unreg(cli);
    }
    if (sock_hdl) {
        sock_unreg(sock_hdl);
    }

}

void mass_production_socket_init(void)
{
    void *sock_hdl = NULL;

    sock_hdl = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);
    u32 opt = 1;
    if (sock_setsockopt(sock_hdl, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("\n[ %s sock_bind fail ]\n", __FILE__);
        goto exit;
    }
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    local_addr.sin_port = htons(3335);
    if (sock_bind(sock_hdl, (struct sockaddr *)&local_addr, sizeof(struct sockaddr))) {
        printf("\n[ %s sock_bind fail ]\n", __FILE__);
        goto exit;
    }
    sock_listen(sock_hdl, 0x5);
    thread_fork("mass_production_socket_accpet", 25, 0x400, 0, 0, mass_production_socket_accpet, sock_hdl);
exit:
    return;
}




void mass_production_init(void)
{

    printf("network mssdp init\n");
    network_mssdp_init();
    /*
    *代码段功能:修改RTSP的URL
    *默认配置  :URL为rtsp://192.168.1.1/avi_pcm_rt/front.sd,//(avi_pcma_rt 传G7111音频)传JPEG实时流
    *
    */
#if 1
    char *user_custom_name = "mass_pro_rt";
    char *user_custom_content =
        "stream\r\n \
    file_ext_name 264\r\n \
    media_source live\r\n \
    priority 1\r\n \
    payload_type 96\r\n \
    clock_rate 90000\r\n \
    encoding_name H264\r\n \
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

    strncpy(conf.protocol, "TCP", 3);
    conf.exit = fenice_video_rec_exit;
    conf.setup = fenice_video_rec_setup;
    conf.get_video_info = fenice_get_video_info;
    conf.get_audio_info = fenice_get_audio_info;
    conf.set_media_info = fenice_set_media_info;
    conf.port = 0;  // 当为0时,用默认端口554
    stream_media_server_init(&conf);

    mass_production_socket_init();
}


extern int doorbell_event_init(void);//主要是create wifi 线程的
extern int Hi3861L_init(void) ;//主要是create wifi 线程的
extern int avsdk_cmd_init(void);
extern s32 hi3861l_hichannel_init();
extern int sdcard_upgrade_init();
extern void doorbell_write_cfg_info(void);
extern int cloud_playback_list_init();
extern int cloud_rt_talk_init(void);
extern int cloud_storage_init();


void doorbell_app_init()
{
    printf("storage_device_ready() = %d \n", storage_device_ready());
    extern void net_video_handler_init(void);
    net_video_handler_init();
    /*初始化与Hi3861L命令传输协议*/
    avsdk_cmd_init();
    /*使底层sdio接收数据线程先运行起来*/

    /*建立门铃的事件处理函数，所有的门铃事件统一由这个任务处理*/
    doorbell_event_init();
    /*建立与Hi3861L 通信的任务，所有命令的发送和接受统一由此函数处理*/
    Hi3861L_init();


    /*初始化hichannel通道，建立跟Hi3861L的通信*/
    if (hi3861l_hichannel_init()) {
        hi3861l_reset();
        return;
    }


    /*创建升级需要使用的信号量*/
    extern int cloud_upgrade_init();
    cloud_upgrade_init();

    /*进行卡升级检查*/
    if (sdcard_upgrade_init()) {
        /*如果需要卡升级，不跑其他流程*/
        return;
    }
    /*检查光敏电阻的阻值*/
//    extern int light_adc_init(void);
//    light_adc_init();

    /*检查是否需要从sd卡读取授权文件*/
    doorbell_write_cfg_info();
    /*初始化云存上传任务*/
    cloud_storage_init();
    /*初始化对讲功能*/
    cloud_rt_talk_init();

    post_msg_doorbell_task("doorbell_heart_task", 1, DOORBELL_EVENT_QUERY_BATTARY);
    /*发送获取所有配置信息的命令给3861L*/

    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_ALL_CFG);
    // msleep(200);
    /*初始化回看的文件列表*/
    cloud_playback_list_init();

    /*启动休眠休眠检查线程*/
//    extern void doorbell_sleep_thread_init(void);
//    doorbell_sleep_thread_init();

}




