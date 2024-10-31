#include "sock_api/sock_api.h"
#include "lwip/sockets.h"//struct sockaddr_in
#include "server/rt_stream_pkg.h"   //head info .h
#include "server/video_server.h"//app_struct
#include "server/video_dec_server.h"//dec_struct
#include "server/ctp_server.h"//enum ctp_cli_msg_type
#include "time_compile.h"
#include "net_scr_api.h"
#include "action.h"

/*#define CHOICE_ONE_TALK  //打开该宏按键控制单边对讲 没有开默认 从机一直有声音 按键控制主机声音   */
#define TWO_WAY_TALK     //双向对讲
/*#define CHECK_FRAME      //计算视频帧数*/

enum CRL_CHOICE {
    KEEP_LIVE = 0,
    CLOSE_STREAM,//1
    OPEN_STREAM, //2
    CLOSE_TALK,  //3
    OPEN_TALK,   //4
};

#define CHEAK_KEEP_LIVE_TIME  10000    //检查心跳包的周期 10MS * mun
#define SEND_KEEP_LIVE_TIME  5000    //检查心跳包的周期 10MS * mun

#define DEST_PORT 3333  //发送命令端口
#define CONN_PORT 2224  //接收数据端口
#define DEST_IP_SERVER "192.168.1.1" //主机IP
#define USER_IP_SERVER "192.168.1.2" //本机IP

#define SOCK_RECV_SIZE 8*1024 //sock一次接收大小 实际上收不到这么多  //音频包一次接收950左右 视频包1472
#define PCM_BUF_SIZE   9000 //接收音频包

#define JPG_MAX_SIZE 100*1024
#define JPG_FPS 20 //
#define JPG_SRC_W 640 //输出显示的大小 程序会根据屏幕大小合理的显示出来  分辨率不对会引起花屏现象
#define JPG_SRC_H 480
/*#define JPG_SRC_W 1280   */
/*#define JPG_SRC_H 720  */

#define CHECK_CODE 0x88 //填充数据值
#define CHECK_CODE_NUM 32 //检查填充量

struct __NET_CTP_INFO {
    u8 state;
    void *video_dec;
};
static struct __NET_CTP_INFO  net_ctp_info = {0};

struct ctp_hdl {
    struct list_head entry;
    int id;
    int recv_thread_pid;
    void *sock_hdl;
    struct sockaddr_in dest_addr;
    int (*cb_func)(void *hdl, enum ctp_cli_msg_type type, const char *topic, const char *content, void *priv);
    void *priv;
    char *topic;
    unsigned int timeout;
    unsigned int timehdl;
    unsigned int timecheck;
    unsigned int timecheckout;
};

static struct ctp_hdl *hdl = {0};

static void *rx_hdl = NULL;
static void *sockfd = NULL;

static u8 audio_dec_flag = 0;
static u8 tx_talk_init_flag = 0;
static int recv_task_pid = 0;
static u8 timeid = 0;
static u8 timeid2 = 0;
static u8 key_time = 0;//记录按键状态

static u32 total_payload_len = 0;
static u32 jpeg_total_payload_len = 0;
static u8 finish = 0;
static u8 jpeg_finish = 0;
static u32 old_frame_seq = 0;//记录音频数据包号
static u32 jpeg_old_frame_seq = 0;//记录视频数据包号

static int tx_talk_net_init(void)
{
    extern void init_sock_receive_voice(void);
    init_sock_receive_voice();

    return 0;
}


static void activity(u8 choice)
{
    /* 命令 */
    const char topic_0[] = {"CTP_KEEP_ALIVE"};//打开数据流命令 包括音频视频
    const char topic_1[] = {"CLOSE_RT_STREAM"};
    const char topic_2[] = {"RT_TALK_CTL"};//
    const char topic_3[] = {"VIDEO_CTRL"};//
    const char topic_4[] = {"OPEN_RT_STREAM"};//打开数据流命令 包括音频视频

    /* 命令参数 */
    const char content_0[] = {"{\"op\":\"PUT\",\"param\":{}}"};
    const char content_1[] = {"{\"op\":\"PUT\",\"param\":{\"status\":\"1\"}}"}; //某个状态开
    const char content_2[] = {"{\"op\":\"PUT\",\"param\":{\"status\":\"0\"}}"}; //某个状态关
    const char content_4[] = {"{\"op\":\"PUT\",\"param\":{\"rate\":\"8000\",\"w\":\"640\",\"fps\":\"20\",\"h\":\"480\",\"format\":\"1\"}}"};//视频数据流请求参数

    extern int rx_close_audio_dec(void *priv);
    extern int rx_open_audio_dec(void *priv);

    switch (choice) {

    case KEEP_LIVE:
        if (ctp_cli_send(hdl, (char *)topic_0, (char *)&content_0) == 0) {
        } else {
            printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        }
        break;

    case CLOSE_STREAM://关闭视频
        if (ctp_cli_send(hdl, (char *)topic_1, (char *)&content_1) == 0) {
        } else {
            printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        }
        break;

    case OPEN_STREAM://开打视频
        if (ctp_cli_send(hdl, (char *)topic_4, (char *)&content_4) == 0) {
        } else {
            printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        }
        break;

    case CLOSE_TALK://关闭对讲 //该命令关闭主机接收数据线程 本机为从机
        if (ctp_cli_send(hdl, (char *)topic_2, (char *)&content_2) == 0) {
        } else {
            printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        }

#ifdef CHOICE_ONE_TALK
        if (!audio_dec_flag) {
            audio_dec_flag = 1;
            rx_open_audio_dec(rx_hdl);
        }
#endif

#ifdef TWO_WAY_TALK
        if (audio_dec_flag) {
            audio_dec_flag = 0 ;
            rx_close_audio_dec(rx_hdl);
        }
#endif
        break;

    case OPEN_TALK://打开对讲
        if (ctp_cli_send(hdl, (char *)topic_2, (char *)&content_1) == 0) {
        } else {
            printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        }
#ifdef CHOICE_ONE_TALK
        if (audio_dec_flag) {
            audio_dec_flag = 0 ;
            rx_close_audio_dec(rx_hdl);
        }
#endif

#ifdef TWO_WAY_TALK
        if (!audio_dec_flag) {
            audio_dec_flag = 1;
            rx_open_audio_dec(rx_hdl);
        }
#endif
        break;
    }
}

void adkey_crl_talk(void)//单击按键
{
    if (tx_talk_init_flag) {
        if (key_time) { //按键第一次按下是选项打开对讲
            activity(CLOSE_TALK);
            printf("CLOSE_TALK");
        } else {
            activity(OPEN_TALK);
            printf("OPEN_TALK");
        }
        key_time = ~key_time;
    }
}

//失去连接于主机的连接
static void lose_keep_live(void *priv)
{

    printf("\n-----------------------Host disconnected----------------------\n");

    int err;
    union video_dec_req dec_req = {0};
    extern void ctp_cli_unreg(void *handle);
    ctp_cli_unreg(hdl);//断开sock发送命令
    extern void mic_record_stop(void);
    mic_record_stop(); //退出音频发送线程
    extern int rx_close_audio_dec(void *priv);
    rx_close_audio_dec(rx_hdl);//关闭用于接收数据音频解码
    extern int rx_close_audio_server(void *priv);
    rx_close_audio_server(rx_hdl);//关闭音频服务

    sock_unreg(sockfd);//断开sock接收数据

    if (net_ctp_info.video_dec) { //关闭视频解码服务
        err = server_request(net_ctp_info.video_dec, VIDEO_REQ_DEC_STOP, &dec_req);

        if (err) {
            printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        }
        server_close(net_ctp_info.video_dec);
        net_ctp_info.video_dec = NULL;
    }

    thread_kill(&recv_task_pid, KILL_FORCE); //退出音频视频接收线程

    sys_timer_del(timeid);//删除定时器
    sys_timer_del(timeid2);

    total_payload_len = 0; //复位一些全局变量
    jpeg_total_payload_len = 0;
    finish = 0;
    jpeg_finish = 0;
    old_frame_seq = 0;//记录音频数据包号
    jpeg_old_frame_seq = 0;//记录视频数据包号
    printf("\n-----------------------disconnected end----------------------\n");
}
static void send_keep_live(void *priv)
{
    activity(KEEP_LIVE);
}
static int ctp_callback(void *hdl, enum ctp_cli_msg_type type, const char *topic, const char *parm_list, void *priv)
{
    if (!timeid) { //开启定时器用于检测超时未收到心跳包进入退出
        timeid = sys_timer_add(NULL, lose_keep_live, CHEAK_KEEP_LIVE_TIME);
        printf("time1 = %d\n", timeid);

        if (!timeid) {
            printf("add_time_fail\n");
        }
    }

    if (type == CTP_CLI_RECV_MSG) { //收到主机发回的回信 复位定时器
        sys_timer_modify(timeid, CHEAK_KEEP_LIVE_TIME);
    }

    if (!timeid2) { //开启定时器定时发送心跳包
        timeid2 = sys_timer_add(NULL, send_keep_live, SEND_KEEP_LIVE_TIME);
        printf("time2 = %d\n", timeid2);

        if (!timeid) {
            printf("add_time_fail\n");
        }
    }

    /*if(type == CTP_CLI_DISCONNECT)*/
    /*return -1;*/

    return 0;
}

static int net_ctp_video_dec_init(void)
{
    struct video_dec_arg arg_recv = {0};

    arg_recv.dev_name = "video_dec";
    arg_recv.video_buf_size = JPG_MAX_SIZE;
    net_ctp_info.video_dec = server_open("video_dec_server", &arg_recv);

    if (net_ctp_info.video_dec == NULL) {
        printf("\n [ERROR] video_dec_server open err\n");
        goto EXIT;
    }

    return 0;

EXIT:

    if (net_ctp_info.video_dec) {
        server_close(net_ctp_info.video_dec);
        net_ctp_info.video_dec = NULL;
    }

    return -1;
}
static int bytecmp(unsigned char *p, unsigned char ch, unsigned int num)
{
    while (num-- > 0) {
        if (*p++ != ch) {
            return -1;
        }
    }

    return 0;
}

//接收音频视频数据
static int get_pcm_jpeg_packet(char *buf, int len, char *pcm_buf, int *pcm_buf_len, struct __JPG_INFO *info)
{

    u32 position = 0;
    u32 jpg_position = 0;
    u32 frame_head_size = sizeof(struct frm_head);
    u8 frame_type;
    u32 cur_frame_seq;
    u32 frame_offset;
    u32 slice_data_len;
    u32 frame_size;

    static  u8 ps = 0;

    if (len < frame_head_size) {
        printf("\n%s %d->data err\n", __func__, __LINE__);
        goto ERROR;
    }

    do {
        struct frm_head  *head_info = (struct frm_head *)(buf + position);
        frame_type = head_info->type & 0x7F; //数据类型 是音频 还是 视频
        cur_frame_seq = head_info->seq;//帧序号
        frame_offset = head_info->offset;//当前帧偏移
        slice_data_len = head_info->payload_size;//当前数据长度
        frame_size = head_info->frm_sz;//帧头长度
        len -= (frame_head_size + slice_data_len);//如果帧头长+数据长度!=包长度

        if (len < 0) {
            printf("\n%s %d->data err\n", __func__, __LINE__); //认为是错误的数据包
            goto ERROR;
        }

        switch (frame_type) {
#if 1

        case PCM_TYPE_AUDIO:

            if (cur_frame_seq < old_frame_seq) { //如果当前的seq小于旧的seq,说明是旧的数据包,跳过不处理
                printf("\n%s %d->recv old seq\n", __func__, __LINE__);
                goto continue_deal;

            } else if (cur_frame_seq > old_frame_seq) { //如果当前seq大于旧的seq,认为是新的数据包,接收包初始化

                if (total_payload_len && (finish == 0)) {
                    printf("\n%s %d->recv old seq\n", __func__, __LINE__);
                }

                old_frame_seq = cur_frame_seq;
                total_payload_len = 0;
                finish = 0;
                memset(pcm_buf, CHECK_CODE, PCM_BUF_SIZE);
            }

            if (bytecmp((unsigned char *)pcm_buf + frame_offset, CHECK_CODE, CHECK_CODE_NUM) != 0) {
                printf("\n%s %d->repeat seq\n", __func__, __LINE__);
                goto continue_deal;

            }

            if (frame_offset + slice_data_len > PCM_BUF_SIZE) {
                printf("\n%s %d->large data pcm buf too small\n", __func__, __LINE__);
                goto ERROR;
            }

            memcpy(pcm_buf + frame_offset, (buf + position) + frame_head_size, slice_data_len);//对应数据放置到对应的位置
            total_payload_len += slice_data_len;//累加总长度

            if (total_payload_len == frame_size) { //如果数据量相等,说明帧数据接收完成
                *pcm_buf_len = total_payload_len;
                finish = 1;
                /*printf("\n%s %d->finish\n", __func__, __LINE__);*/
                return 1;
            }

            goto continue_deal;
#endif

        case JPEG_TYPE_VIDEO:
            if (frame_offset + slice_data_len > JPG_MAX_SIZE) {
                printf("\n%s %d->large data pcm buf too small\n", __func__, __LINE__);
                goto ERROR;
            }

            if (cur_frame_seq < jpeg_old_frame_seq) { //如果当前的seq小于旧的seq,说明是旧的数据包,跳过不处理
                printf("\n%s %d->jpeg_recv old seq\n", __func__, __LINE__);
                goto continue_deal;

            } else if (cur_frame_seq > jpeg_old_frame_seq) { //如果当前seq大于旧的seq,认为是新的数据包,接收包初始化

                if (jpeg_total_payload_len && (jpeg_finish == 0)) {
                    printf("\n [MSG] lose packet or disorder packet\n");
                }

                jpeg_old_frame_seq = cur_frame_seq;
                jpeg_total_payload_len = 0;
                jpeg_finish = 0;
                memset(info->buf, CHECK_CODE, JPG_MAX_SIZE);
            }

            if (bytecmp((unsigned char *)info->buf + frame_offset, CHECK_CODE, CHECK_CODE_NUM) != 0) {
                printf("\n%s %d->jpeg_repeat seq\n", __func__, __LINE__);
                goto continue_deal;

            }

            memcpy(info->buf + frame_offset, (buf + position) + frame_head_size, slice_data_len);//对应数据放置到对应的位置
            jpeg_total_payload_len += slice_data_len;//累加总长度

            if (jpeg_total_payload_len == frame_size) { //如果数据量相等,说明帧数据接收完成
                info->buf_len = jpeg_total_payload_len;
                jpeg_finish = 1;
                /*printf("\n%s %d->finish\n", __func__, __LINE__);*/
                return 2;
            }

continue_deal:
            position += (frame_head_size + slice_data_len);
            break;

        default:
            printf("\n%s %d---type >> %d\n", __func__, __LINE__, frame_type);
            break;
        }
    } while (len > 0);

    return 0;
ERROR:

    return -1;

}
static int save_jpg_data(struct __JPG_INFO *image)
{
    void *fd = NULL;
    fd = fopen(CONFIG_ROOT_PATH"image******.jpg", "w+");

    if (fd) {
        fwrite(fd, image->buf, image->buf_len);
        fclose(fd);
        return 0;
    }

    return -1;
}

static void out_to_lcd(struct __JPG_INFO *info)
{
    int err;
    union video_dec_req dec_req = {0};
    dec_req.dec.fb = "fb1";
    dec_req.dec.left = 0;
    dec_req.dec.top = 0;
    dec_req.dec.width = JPG_SRC_W;//指定分辨率显示
    dec_req.dec.height = JPG_SRC_H;
    dec_req.dec.preview = 1;
    dec_req.dec.image.buf = info->buf;
    dec_req.dec.image.size = info->buf_len;

    err = server_request(net_ctp_info.video_dec, VIDEO_REQ_DEC_IMAGE, &dec_req);

    if (err) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
    }
}

static char pcm_buf[PCM_BUF_SIZE];
static char recv_buf[SOCK_RECV_SIZE];

static void thread_socket_recv(void *sockfd)
{
    int recv_len;
    int pcm_buf_len;
    char ret;
#ifdef CHECK_FRAME
    extern u32 timer_get_ms(void);
#endif
    extern void rx_play_voice(void *priv, void *buf, u32 len);
    extern void *open_audio_server(void);
    rx_hdl = open_audio_server();

    if (net_ctp_video_dec_init() == -1) {
        printf("\n%s %d->\net_ctp_video_dec_init ERROR \n", __func__, __LINE__);
    }

    struct __JPG_INFO *jpg = (struct __JPG_INFO *) calloc(1, sizeof(struct __JPG_INFO) + sizeof(u8) * JPG_MAX_SIZE);

    if (!jpg) {
        printf("\n[ERROR]  jpg calloc \n");
        goto EXIT;
    }

    while (1) {

        recv_len = sock_recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, NULL, NULL);

        if (recv_len <= 0) {
            printf("\n%s %d->exit\n", __func__, __LINE__);
            goto EXIT;
        }

//计数视频帧数
#ifdef CHECK_FRAME
        static  tstart = 0, tdiff = 0;
        static  fps_cnt = 0;

        if (!tstart) {
            tstart = timer_get_ms();
        } else {
            tdiff = timer_get_ms() - tstart;

            if (tdiff >= 1000) {
                printf("\n [MSG]fps_count = %d\n", fps_cnt *  1000 / tdiff);//开启可以显示打印视频帧数
                tstart = 0;
                fps_cnt = 0;
            }
        }

#endif

        ret = get_pcm_jpeg_packet(recv_buf, recv_len, pcm_buf, &pcm_buf_len, jpg);

        if (ret == 1) { //播放音频数据
            rx_play_voice(rx_hdl, pcm_buf, pcm_buf_len);
        }

        if (ret == 2) { //播放视频数据
            /*save_jpg_data(jpg);//保存视频图片到SD卡用于调试*/
            out_to_lcd(jpg);//显示wifi接收到的视频数据
#ifdef CHECK_FRAME
            fps_cnt++;
#endif
            os_time_dly(3);
        }

        if (ret < 0) {
            printf("\n%s %d->exit\n", __func__, __LINE__);
            goto EXIT;
        }
    }

EXIT:
    free(jpg);
}

static void net_ctp_recv(void)
{
    struct sockaddr_in conn_addr;
    int ret;

    sockfd = sock_reg(AF_INET, SOCK_DGRAM, 0, NULL, NULL);

    if (sockfd == NULL) {
        printf("\n [ERROR] %s - %d\n", __func__, __LINE__);
    }

    if (sock_set_reuseaddr(sockfd)) {
        printf("\n [ERROR] %s - %d\n", __func__, __LINE__);
    }

    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = inet_addr(USER_IP_SERVER) ;
    conn_addr.sin_port = htons(CONN_PORT);

    if (sock_bind(sockfd, (struct sockaddr *)&conn_addr, sizeof(struct sockaddr)) == -1) {
        printf("\n [ERROR] %s - %d\n",  __func__, __LINE__);
    } else {
        if ((thread_fork("thread_socket_recv", 15, 2 * 1024, 0, &recv_task_pid, thread_socket_recv, sockfd))) {
            printf("\n [ERROR] %s - %d\n", __func__, __LINE__);
        }
    }
}

void *net_ctp_init(void)
{
    struct sockaddr_in dest_addr;

    /* 定义命令数组 */
    const char topic_1[] = {"APP_ACCESS"};//app创建命令
    const char topic_2[] = {"DATE_TIME"};//时间命令
    const char topic_3[] = {"OPEN_RT_STREAM"};//打开数据流命令 包括音频视频
    const char topic_4[] = {"VIDEO_CTRL"};//打开数据流命令 包括音频视频

    /* 各个命令的参数 */
    const char content_0[] = {"{\"op\":\"GET\"}"};
    /*const char content_1[] = {"{\"op\":\"PUT\",\"param\":{\"type\":\"1\",\"ver\":\"1.0\"}}"};*/
    const char content_1[] = {"{\"op\":\"PUT\",\"param\":{\"type\":\"0\",\"ver\":\"20700\"}}"};
    char *Str_time;
    sprintf((char *)&Str_time, "{\"op\":\"PUT\",\"param\":{\"date\":\"%d%02d%02d%02d%02d%02d\"}}", YEAR, MONTH, DAY, HOUR, MINUTE, SECOND);
    printf("\n\n DATA - %d:%d:%d  - %d:%d:%d \n\n", YEAR, MONTH, DAY, HOUR, MINUTE, SECOND);

    //选择摄像头传输的视频数据是什么大小
    /*const char content_3[] = {"{\"op\":\"PUT\",\"param\":{\"rate\":\"8000\",\"w\":\"1280\",\"fps\":\"20\",\"h\":\"720\",\"format\":\"1\"}}"};*/
    const char content_3[] = {"{\"op\":\"PUT\",\"param\":{\"rate\":\"8000\",\"w\":\"640\",\"fps\":\"20\",\"h\":\"480\",\"format\":\"1\"}}"};

    const char content_4[] = {"{\"op\":\"GET\",\"param\":{}"};

    memset(&dest_addr, 0x0, sizeof(dest_addr));

    net_ctp_recv();//建立sock并创建接收数据线程

    ctp_cli_init();//CTP连接初始化
    /* 建立CTP连接 */
    memset(&dest_addr, 0x0, sizeof(dest_addr)); //对server_info初始化操作，全部清零
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(DEST_IP_SERVER) ;
    dest_addr.sin_port = htons(DEST_PORT);
    //建立连接并创建消息回调函数

    timeid = 0 ;
    timeid2 = 0 ;
    key_time = 0 ;
    hdl = ctp_cli_reg(1, &dest_addr, ctp_callback, NULL);

    if (hdl == NULL) {
        printf("\n--------------CTP_CMD_cli connect failed----------------\n");
    } else {
        printf("\n--------------CTP_CMD_cli connect success---------------\n");
    }

    //初始化对讲并创建对讲发送数据线程
    tx_talk_net_init();
    tx_talk_init_flag = 1;//标记已经完成对讲线程创建
    audio_dec_flag = 1; //标记解码服务已经初始化

    if (ctp_cli_send(hdl, (char *)&topic_1, (char *)&content_1) == 0) {
    } else {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto ERROR;
    }

    if (ctp_cli_send(hdl, (char *)&topic_2, (char *)&Str_time) == 0) {
    } else {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto ERROR;
    }

    if (ctp_cli_send(hdl, (char *)&topic_3, (char *)&content_3) == 0) {
    } else {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto ERROR;
    }

    if (ctp_cli_send(hdl, (char *)&topic_4, (char *)&content_4) == 0) {
    } else {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto ERROR;
    }

    return hdl->sock_hdl;

ERROR:

    return NULL;
}
