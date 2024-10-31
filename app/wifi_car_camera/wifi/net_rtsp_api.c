#include "server/rt_stream_pkg.h"   //head info .h
#include "server/video_server.h"//app_struct
#include "server/video_dec_server.h"//dec_struct
#include "action.h"//it.action = ACTION_NET_SCR...
#include "net_config.h"

#ifdef CONFIG_RTSP_CLIENT_ENABLE

#define JPG_MAX_SIZE 200*1024
struct __NET_RTSP_INFO {
    u8 state;
    void *video_dec;
};
struct BUFF_RET {
    char *frm_buf;
    unsigned int size;
};

static struct __NET_RTSP_INFO  net_rtsp_info = {0};

int net_rtsp_devinit(void)
{
    struct application *app = NULL;
    if (net_rtsp_info.state) {
        printf("\n [WARING] %s multiple init\n", __func__);
        return 0;
    }
    struct video_dec_arg arg_recv = {0};
    arg_recv.dev_name = "video_dec";
    arg_recv.video_buf_size = JPG_MAX_SIZE;
    net_rtsp_info.video_dec = server_open("video_dec_server", &arg_recv);
    if (net_rtsp_info.video_dec == NULL) {
        printf("\n [ERROR] video_dec_server open err\n");
        goto EXIT;
    }
    struct intent it;
    app = get_current_app();
    printf("\n app-> name = %s\n", app->name);
    if (!app) {
        printf("err net_rtsp_devinit !!!\n");
        goto EXIT;
    }
    it.name = "net_video_rec";
    it.action = ACTION_NET_RTSP_REC_OPEN;
    start_app(&it);
    net_rtsp_info.state = 1;
    return 0;

EXIT:
    if (net_rtsp_info.video_dec) {
        server_close(net_rtsp_info.video_dec);
        net_rtsp_info.video_dec = NULL;
    }
    return -1;
}

int net_rtsp_dec(struct BUFF_RET *info, struct __NET_RTSP_INFO *rtsp_info)
{
    int err;
    if (!rtsp_info->state) {
        printf("\n [WARING] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    union video_dec_req dec_req = {0};
    dec_req.dec.fb = "fb1";
    dec_req.dec.left = 0;
    dec_req.dec.top = 0;

    /*printf("\n--------------  w = %d\n",info->src_w);  */
    /*printf("\n-------------  h = %d\n",info->src_h);  */

    /* 注意：帧率过高则需要降帧显示 */

    dec_req.dec.width = 640;//指定分辨率显示
    dec_req.dec.height = 480;

    /*dec_req.dec.width = LCD_DEV_WIDTH;//全屏显示,*/
    /*dec_req.dec.height = LCD_DEV_HIGHT;*/

    dec_req.dec.preview = 1;
    //dec_req.dec.audio_p.buf = NULL;
    //dec_req.dec.audio_p.buf_len = 10 * 1024;
    dec_req.dec.image.buf = info->frm_buf;
    dec_req.dec.image.size = info->size;
    /*err = server_request(rtsp_info->video_dec, VIDEO_REQ_DEC_IMAGE, &dec_req);*/
    if (err) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    return 0;
EXIT:
    return -1;
}

void net_rtsp_callback(char *frm_buf, unsigned int sz)
{
    static struct BUFF_RET ret_buf ;
    if (1) {
        ret_buf.frm_buf = frm_buf;
        ret_buf.size = sz;
        net_rtsp_dec(&ret_buf, &net_rtsp_info);
    }
}

void net_rtsp_init(void)
{
    int re = net_rtsp_devinit();
    if (re == 0) {
        printf("\n  DEV - sueccess \n ");
    }
}
void net_rtsp_uninit(void)
{
    if (net_rtsp_info.video_dec) {
        server_close(net_rtsp_info.video_dec);
        net_rtsp_info.video_dec = NULL;
        printf("net_rtsp_uninit ... \n\n");
    }
    net_rtsp_info.state = 0;

    struct intent it;
    struct application *app = NULL;
    app = get_current_app();
    if (!app) {
        printf("err net_rtsp_devinit !!!\n");
        return ;
    }
    it.name = "net_video_rec";
    it.action = ACTION_NET_RTSP_REC_CLOSE;
    start_app(&it);
}
#endif
