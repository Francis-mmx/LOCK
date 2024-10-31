#include "system/includes.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "action.h"
#include "os/os_compat.h"
#include "net_config.h"
#include "net_scr_api.h"

#ifdef CONFIG_NET_SCR

#define JPG_BUF_MAX_SIZE  200 * 1024
#define UDP_MAX_RECV  1 * 1472
#define CHECK_CODE 0x88
#define CHECK_CODE_NUM 32

extern u32 timer_get_ms(void);
/*打开该宏意味着使用TCP为数据传输通道(暂不支持)*/
#define USE_TCP_PROTOCOL

static struct __NET_SCR_INFO  net_scr_info;
static u8 recv_buf[UDP_MAX_RECV];
static u32 old_frame_seq = 0;
/*为了退出接收线程，需要是全局*/
static void *sock_hdl = NULL;




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

static int save_yuv_data(struct __JPG_INFO *image)
{
    static char *yuv_buf = NULL;

    if (yuv_buf == NULL) {
        yuv_buf = malloc(image->src_w * image->src_h * 3 / 2);
        if (!yuv_buf) {
            return -1;
        }
    }
    struct jpeg_decode_req req = {0};
    req.output_type = 0;
    req.input.data.buf = image->buf;
    req.input.data.len = image->buf_len;
    req.buf_y = yuv_buf;
    req.buf_u = req.buf_y + image->src_w * image->src_h;
    req.buf_v = req.buf_u + (image->src_w * image->src_h) / 4;
    req.buf_width = image->src_w;
    req.buf_height = image->src_h;
    req.out_width = image->src_w;
    req.out_height = image->src_h;
    jpeg_decode_one_image(&req);
    static void *fd = NULL;

    fd = fopen(CONFIG_ROOT_PATH"test****.yuv", "w+");
    if (fd) {
        fwrite(fd, req.buf_y, image->src_w * image->src_h * 3 / 2);
        fclose(fd);
        return 0;
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

static int get_jpg_packet(char *buf, int len, struct __JPG_INFO *jpg)
{
    static u32 total_payload_len = 0;
    static u8 finish = 0;
    u32 position = 0;
    u8 frame_type;
    u32 cur_frame_seq, frame_offset, slice_data_len, frame_size;
    u32 frame_head_size = sizeof(struct frm_head);

    if (len < frame_head_size) {
        printf("\n[ERROR] recv_len err\n");
        goto ERROR;
    }

    do {

        struct frm_head  *head_info = (struct frm_head *)(buf + position);
        frame_type = head_info->type & 0x7F;

        cur_frame_seq = head_info->seq;
        frame_offset = head_info->offset;
        slice_data_len = head_info->payload_size;
        frame_size = head_info->frm_sz;
        //图片大小大于缓冲buf，不处理
        if (frame_size > JPG_BUF_MAX_SIZE) {
            printf("\n [MSG] frame_size too large\n");
            goto ERROR;
        }
        len -= (frame_head_size + slice_data_len);
        if (len < 0) {
            printf("\n[ERROR] remain len err\n");
            goto ERROR;
        }

        switch (frame_type) {

        case JPEG_TYPE_VIDEO:
            //如果当前的seq小于旧的seq,说明是旧的数据包,跳过不处理
            if (cur_frame_seq < old_frame_seq) {
                //printf("\n [MSG] recv old packet");
                goto continue_deal;
            }
            //如果当前seq大于旧的seq,认为是新的数据包,组包重新初始
            if (cur_frame_seq > old_frame_seq) {
                if (total_payload_len && (finish == 0)) {
                    //printf("\n [MSG] lose packet or disorder packet\n");
                }
                old_frame_seq = cur_frame_seq;
                total_payload_len = 0;
                finish = 0;
                memset(jpg->buf, CHECK_CODE, JPG_BUF_MAX_SIZE);

            }
            //校验是不是接收到重复的包
            if (bytecmp((char *)jpg->buf + frame_offset, CHECK_CODE, CHECK_CODE_NUM) != 0) {
                //printf("\n [MSG] repeat the packet\n");
                goto continue_deal;

            }
            memcpy(jpg->buf + frame_offset, (buf + position) + frame_head_size, slice_data_len);
            total_payload_len += slice_data_len;
            if (total_payload_len == frame_size) { //如果数据量相等,说明帧数据接收完成
                jpg->buf_len = total_payload_len;
                finish = 1;
                return 0;//返回0表示完成
            }
continue_deal:
            //position是剩余数据的偏移量
            position += (frame_head_size + slice_data_len);
            break;

        default:
            printf("\n [WARING] %s - %d\n", __FUNCTION__, __LINE__);
            break;

        }
    } while (len > 0);


ERROR:
    return -1;
}


void jpg_recv_thread(void *priv)
{

    int recv_len = 0;
    int finish_flag;
    u32 tstart = 0, tdiff = 0;
    u8 fps_cnt = 0;

//    gpio_direction_input(IO_PORTA_11);
//    gpio_set_pull_up(IO_PORTA_11, 1);


    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
#ifdef USE_TCP_PROTOCOL
    struct frm_head head_info;
    u32 frm_sz;

    sock_hdl = sock_accept(net_scr_info.sock_hdl, &src_addr, &addrlen, NULL, NULL);
    if (!sock_hdl) {
        goto EXIT;
    }
    if (src_addr.sin_addr.s_addr != net_scr_info.cfg.cli_addr.sin_addr.s_addr) {
        printf("\n[WARING] %s accept addr err\n", __func__);
        goto EXIT;
    }
#endif // USE_TCP_PROTOCOL

    struct __JPG_INFO *jpg = (struct __JPG_INFO *) calloc(1, sizeof(struct __JPG_INFO) + sizeof(u8) * JPG_BUF_MAX_SIZE);
    if (!jpg) {
        printf("\n[ERROR]  jpg calloc \n");
    }

    jpg->src_w = net_scr_info.cfg.src_w;
    jpg->src_h = net_scr_info.cfg.src_h;
    while (1) {
#ifdef USE_TCP_PROTOCOL
        recv_len = sock_recv(sock_hdl, recv_buf, sizeof(struct frm_head), MSG_WAITALL);
        if (recv_len <= 0  || recv_len != sizeof(struct frm_head)) {
            printf("\nrecv_len = %d head = %d\n", recv_len, sizeof(struct frm_head));
            free(jpg);
            goto EXIT;
        }
        memcpy(&head_info, recv_buf, sizeof(struct frm_head));
        frm_sz = head_info.frm_sz;
        //printf("\n frm_sz = %d\n",frm_sz);

        recv_len = sock_recv(sock_hdl, jpg->buf, frm_sz, MSG_WAITALL);
        if (recv_len <= 0 || recv_len != frm_sz) {
            printf("\nrecv_len = %d frm_sz %d\n", recv_len, frm_sz);
            free(jpg);
            goto EXIT;
        }
        jpg->buf_len = recv_len;
        finish_flag = 0;
#else
        recv_len = sock_recvfrom(net_scr_info.sock_hdl, recv_buf, sizeof(recv_buf), 0, &src_addr, &addrlen);
        if (recv_len < 0) {
            free(jpg);
            break;
        }

        if (src_addr.sin_addr.s_addr != net_scr_info.cfg.cli_addr.sin_addr.s_addr) {
            printf("\n[WARING] %s data source err\n", __func__);
            continue;
        }
        finish_flag = get_jpg_packet(recv_buf, recv_len, jpg);
#endif // USE_TCP_PROTOCOL

        if (!finish_flag) {

            net_scr_dec(jpg);
            finish_flag = -1;
            fps_cnt++;
            //打开则会将收到的图片保存到卡中，用于判断数据源是否正确
#if 0
            //save_jpg_data(jpg);
            save_yuv_data(jpg);
#endif
            os_time_dly(3);
        }
        if (!tstart) {
            tstart = timer_get_ms();
        } else {
            tdiff = timer_get_ms() - tstart;
            if (tdiff >= 1000) {
                printf("\n [MSG]fps_count = %d\n", fps_cnt *  1000 / tdiff);
                tstart = 0;
                fps_cnt = 0;
            }
        }
    }
EXIT:
    printf("\n[MSG] %s EXIT\n", __func__);
}

static int net_scr_sock_init(void)
{
    int ret;
#ifdef USE_TCP_PROTOCOL
    net_scr_info.sock_hdl = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);
#else
    net_scr_info.sock_hdl = sock_reg(AF_INET, SOCK_DGRAM, 0, NULL, NULL);
#endif

    if (net_scr_info.sock_hdl == NULL) {
        printf("%s %d->Error in socket()\n", __func__, __LINE__);
        goto EXIT;
    }
    if (sock_set_reuseaddr(net_scr_info.sock_hdl)) {
        printf("%s %d->Error in sock_set_reuseaddr(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }
    struct sockaddr_in _ss;
    struct sockaddr_in *dest_addr = &_ss;
    dest_addr->sin_family = AF_INET;
    dest_addr->sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.2.205");//htonl(INADDR_ANY);
#ifdef USE_TCP_PROTOCOL
    dest_addr->sin_port  = htons(NET_SCR_PORT);
#else
    dest_addr->sin_port  = htons(NET_SCR_PORT);
#endif

    ret = sock_bind(net_scr_info.sock_hdl, (struct sockaddr *)dest_addr, sizeof(struct sockaddr));
    if (ret) {
        printf("%s %d->Error in bind(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }
#ifdef USE_TCP_PROTOCOL
    ret = sock_listen(net_scr_info.sock_hdl, 0xFF);
    if (ret) {
        printf("%s %d->Error in listen(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }
#endif // USE_TCP_PROTOCOL

    return 0;

EXIT:
    if (net_scr_info.sock_hdl) {
        sock_unreg(net_scr_info.sock_hdl);
        net_scr_info.sock_hdl = NULL;
    }
    return -1;
}
static int net_scr_sock_uninit(void)
{

#ifdef USE_TCP_PROTOCOL
    sock_unreg(sock_hdl);
#endif // USE_TCP_PROTOCOL
    sock_unreg(net_scr_info.sock_hdl);
    net_scr_info.sock_hdl = NULL;
    return 0;
}


int net_scr_init(struct __NET_SCR_CFG *cfg)
{
    struct intent it = {0};
    struct application *app = NULL;
    if (net_scr_info.state) {
        printf("\n [WARING] %s multiple init\n", __func__);
        goto EXIT;
    }
    //返回非零值表示失败
    if (net_scr_sock_init()) {
        printf("\n [WARING] socket init err\n");
        goto EXIT;
    }
    struct video_dec_arg arg = {0};
    arg.dev_name = "video_dec";
    //arg.audio_buf_size = 128;
    arg.video_buf_size = JPG_BUF_MAX_SIZE;
    net_scr_info.video_dec = server_open("video_dec_server", &arg);
    if (net_scr_info.video_dec == NULL) {
        printf("\n [ERROR] video_dec_server open err\n");
        goto EXIT;
    }
    app = get_current_app();
    printf("\n app-> name = %s\n", app->name);
    if (!app || (strcmp("video_rec", app->name) && strcmp("net_video_rec", app->name))) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    it.name = app->name;
    it.action = ACTION_NET_SCR_REC_OPEN;
    start_app(&it);
    old_frame_seq = 0;
    memcpy(&net_scr_info.cfg, cfg, sizeof(struct __NET_SCR_CFG));
    printf("\n[MSG]net_scr_info.cfg.src_w = %d\n", net_scr_info.cfg.src_w);
    printf("\n[MSG]net_scr_info.cfg.src_h = %d\n", net_scr_info.cfg.src_h);
    int err = thread_fork("JPG_RECV_THREAD", 15, 0x1000, 0,  &net_scr_info.pid, jpg_recv_thread, 0);
    if (err) {
        printf("\n [WARING] creat thread err\n");
        goto EXIT;
    }
    net_scr_info.state = 1;
    return 0;

EXIT:
    if (net_scr_info.video_dec) {
        server_close(net_scr_info.video_dec);
        net_scr_info.video_dec = NULL;
    }
    return -1;
}

int net_scr_dec(struct __JPG_INFO *info)
{
    int err;
    if (!net_scr_info.state) {
        printf("\n [WARING] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    union video_dec_req dec_req = {0};
    dec_req.dec.fb = "fb1";
    dec_req.dec.left = 0;
    dec_req.dec.top = 0;

//    printf("\n  w = %d\n",info->src_w);
//    printf("\n  h = %d\n",info->src_h);

    dec_req.dec.width = info->src_w;//指定分辨率显示
    dec_req.dec.height = info->src_h;

    /*dec_req.dec.width = LCD_DEV_WIDTH;//全屏显示*/
    /*dec_req.dec.height = LCD_DEV_HIGHT;*/

    //dec_req.dec.audio_p.buf = NULL;
    //dec_req.dec.audio_p.buf_len = 10 * 1024;
    dec_req.dec.preview = 1;
    dec_req.dec.image.buf = info->buf;
    dec_req.dec.image.size = info->buf_len;
    /*err = server_request(net_scr_info.video_dec, VIDEO_REQ_DEC_IMAGE, &dec_req);*/
    if (err) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    return 0;
EXIT:
    return -1;
}

int net_scr_uninit(struct __NET_SCR_CFG *cfg)
{
    int err;
    struct intent it = {0};
    struct application *app = NULL;
    union video_dec_req dec_req = {0};

    if (!net_scr_info.state || memcmp((char *)&net_scr_info.cfg.cli_addr + 2, (char *)&cfg->cli_addr + 2, 6)) {
        printf("\n [WARING] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    net_scr_sock_uninit();
    thread_kill(&net_scr_info.pid, KILL_WAIT);
    err = server_request(net_scr_info.video_dec, VIDEO_REQ_DEC_STOP, &dec_req);
    if (err) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    server_close(net_scr_info.video_dec);
    net_scr_info.video_dec = NULL;
    net_scr_info.state = 0;
    app = get_current_app();
    if (!app || (strcmp("video_rec", app->name) && strcmp("net_video_rec", app->name))) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    it.name = app->name;
    it.action = ACTION_NET_SCR_REC_CLOSE;
    start_app(&it);

    return 0;

EXIT:
    return -1;
}
u8 get_net_scr_status(void)
{

    return net_scr_info.state;
}

#endif

