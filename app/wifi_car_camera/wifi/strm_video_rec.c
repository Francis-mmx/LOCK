
#include "strm_video_rec.h"
#include "vrec_osd.h"
#include "user_isp_cfg.h"

static struct strm_video_hdl fv_rec_handler;
static struct net_video_hdl *net_video = NULL;
static struct video_rec_hdl *rec_handler = NULL;

#define AUDIO_VOLUME	64

#define sizeof_this     (sizeof(struct video_rec_hdl))
#define __this_net		(net_video)//非RTSP句柄,共用BUF,节省内存
#define __this_strm 	(&fv_rec_handler)
#define __this  		(rec_handler)

extern int net_video_rec_get_uvc_id(void);
extern int net_video_rec_state(void);
extern void *get_video_rec0_handler(void);
extern void *get_video_rec1_handler(void);
static int fv_video_rec_start(void);
static int fv_video_rec_stop(void);
static int fv_video_rec_close(char close);

#ifdef __CPU_AC521x__
static const u16 strm_rec_pix_w[] = {1280, 640};
static const u16 strm_rec_pix_h[] = {720,  480};
#else
static const u16 strm_rec_pix_w[] = {1920, 1280, 640};
static const u16 strm_rec_pix_h[] = {1088, 720,  480};
#endif


struct fenice_source_info s_info = {
    .type = STRM_SOURCE_VIDEO0,//摄像头类型
    .width = 640,//分辨率
    .height = 480,
    .fps = STRM_VIDEO_REC_FPS0,//帧率
    .sample_rate = 0,//采样率，默认配置为0
    .channel_num = 1,//通道数
};

/*码率控制，根据具体分辨率设置*/
static int strm_video_rec_get_abr(u32 width)
{
    if (width <= 384) {
        return 2500;
    } else if (width <= 640) {
        return 3000;//2000;
    } else if (width <= 1280) {
        return 3500;
        /* return 10000; */
    } else if (width <= 1920) {
        return 5000;
    } else {
        return 18000;
    }
}
int strm_video_rec_get_fps(void)
{
    int ret;
    int fp = 20;
    ret = net_video_rec_state();
    if (ret) {
        fp = LOCAL_VIDEO_REC_FPS;
    } else {
        fp = STRM_VIDEO_REC_FPS0;
    }
    ret = fp ? fp : 25;
    return ret;
}
int strm_video_rec_get_list_vframe(void)
{
    return __this_strm->fbuf_fcnt;
}
void strm_video_rec_pkg_get_video_in_frame(char *fbuf, u32 frame_size)
{
    __this_strm->fbuf_fcnt++;
    __this_strm->fbuf_ffil += frame_size;
}
void strm_video_rec_pkg_get_video_out_frame(char *fbuf, u32 frame_size)
{
    if (__this_strm->fbuf_fcnt) {
        __this_strm->fbuf_fcnt--;
    }
    if (__this_strm->fbuf_ffil) {
        __this_strm->fbuf_ffil -= frame_size;
    }
}
int strm_video_buff_set_frame_cnt(void)
{
#ifdef STRM_VIDEO_BUFF_FRAME_CNT
    return STRM_VIDEO_BUFF_FRAME_CNT;
#else
    return 0;
#endif
}
int strm_video_rec_get_drop_fp(void)
{
#ifdef STRM_VIDEO_REC_DROP_REAl_FP
    return STRM_VIDEO_REC_DROP_REAl_FP;
#else
    return 0;
#endif
}
int strm_video_rec_get_state(void)
{
    int ret;
    u32 sdram_size = __SDRAM_SIZE__;
    if (__this) {
        ret = __this && __this->state == VIDREC_STA_START;
        if (ret && (__this_strm->width > 640 || (dev_online("uvc") || dev_online("video1.*") || sdram_size <= 2 * 1024 * 1024))) { //单路720丢帧， 双路录像丢帧
            return 1;
        }
    }
    return 0;
}

static int fv_video_rec_open(void)
{
    int ret;
    int id = 0;
    int buf_size[] = {VREC0_FBUF_SIZE, VREC1_FBUF_SIZE, VREC2_FBUF_SIZE};
    int net_buf_size[] = {NET_VREC0_FBUF_SIZE, NET_VREC1_FBUF_SIZE};

    if (__this_strm->state == VIDREC_STA_START || __this_strm->state_ch2 == VIDREC_STA_START) {
        printf("video opened \n\n");
        return 0;
    }

    if (s_info.type == STRM_SOURCE_VIDEO1) {
#ifdef CONFIG_VIDEO1_ENABLE
        id = 1;
#endif
#ifdef CONFIG_VIDEO2_ENABLE
        id = 2;
#endif
    }

    __this_net = (struct net_video_hdl *)get_net_video_rec_handler();
    __this = (struct video_rec_hdl *)get_video_rec_handler();

    u32 sdram_size = __SDRAM_SIZE__;
    int rec_state = (__this->state == VIDREC_STA_START && ((net_video_rec_state() > 640 && s_info.width > 640) || dev_online("uvc") || sdram_size <= (2 * 1024 * 1024)))\
                    || __this->state != VIDREC_STA_START;

    if (__this->video_buf[id] && rec_state) {
        __this_strm->video_buf = __this->video_buf[id];
        __this_strm->video_buf_size = buf_size[id];
        __this_strm->vbuf_share = TRUE;
    } else if (__this_net->net_v0_fbuf) {
        __this_strm->video_buf =  __this_net->net_v0_fbuf;
        __this_strm->video_buf_size = __this_net->net_v0_fbuf_size;
        __this_strm->vbuf_share = TRUE;
    } else {
        __this_strm->vbuf_share = FALSE;
        __this_strm->video_buf_size = net_buf_size[id ? 1 : 0];
        if (!__this_strm->video_buf) {
            __this_strm->video_buf = malloc(__this_strm->video_buf_size);
            if (!__this_strm->video_buf) {
                printf("malloc fv_v0_buf err\n");
                return -1;
            }
        }
    }
    if (__this->audio_buf) {
        __this_strm->audio_buf = __this->audio_buf;
        __this_strm->audio_buf_size = AUDIO_BUF_SIZE;
        __this_strm->abuf_share = TRUE;
    } else if (__this_net->audio_buf) {
        __this_strm->audio_buf = __this_net->audio_buf;
        __this_strm->audio_buf_size = NET_AUDIO_BUF_SIZE;
        __this_strm->abuf_share = TRUE;
    } else {
        __this_strm->abuf_share = FALSE;
        __this_strm->audio_buf_size = NET_AUDIO_BUF_SIZE;
        if (!__this_strm->audio_buf) {
            __this_strm->audio_buf = malloc(__this_strm->audio_buf_size);
            if (!__this_strm->audio_buf) {
                printf("malloc fv_audio_buf err\n");
                free(__this_strm->video_buf);
                return -ENOMEM;
            }
        }
    }
    ret = fv_video_rec_start();
    return ret;
}

static int fv_video_rec_doing_isp_scenes(void *p)
{
    if (__this_strm->isp_scenes_status) {
        return 0;
    }

    stop_update_isp_scenes();
    if ((int)p == 1) {
        if (__this_strm->video_rec0 && __this_strm->state == VIDREC_STA_START) {
            __this_strm->isp_scenes_status = 1;
            return start_update_isp_scenes(__this_strm->video_rec0);
        }
        if (__this_strm->video_rec1 && __this_strm->state == VIDREC_STA_START) {
            __this_strm->isp_scenes_status = 2;
            return start_update_isp_scenes(__this_strm->video_rec1);
        }
    }
    __this_strm->isp_scenes_status = 0;

    return 1;
}
static int fv_video_rec_start_isp_scenes()
{
    return wait_completion(isp_scenes_switch_timer_done, fv_video_rec_doing_isp_scenes, (void *)1);
}
static int fv_video_rec_stop_isp_scenes()
{
    if (!__this_strm->isp_scenes_status) {
        return 0;
    }
    __this_strm->isp_scenes_status = 0;
    stop_update_isp_scenes();
    return 0;
}
static void strm_video_rec_server_event_handler(void *priv, int argc, int *argv)
{
    /*
     *该回调函数会在录像过程中，写卡出错被当前录像APP调用，例如录像过程中突然拔卡
     */
    switch (argv[0]) {
    case VIDEO_SERVER_PKG_NET_ERR:
        fv_video_rec_close(1);
        break;
    default :
        break;
    }
}
static int fv_video_rec_start(void)
{
    int err;
    char name[32] = {0};
    union video_req req = {0};
    struct video_text_osd text_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u8 id;
    u8 ch;

    //单路720录像 且 实时流720 或 双路录像 则公用句柄
    u32 sdram_size = __SDRAM_SIZE__;
    int rec_state = (__this->state == VIDREC_STA_START && ((net_video_rec_state() > 640 && s_info.width > 640) || dev_online("uvc") || sdram_size <= (2 * 1024 * 1024)));
    u8 res = db_select("res");

    __this_strm->fbuf_fcnt = 0;
    __this_strm->fbuf_ffil = 0;

    req.rec.width 	= s_info.width;
    req.rec.height 	= s_info.height;
    req.rec.IP_interval = 0;
    req.rec.three_way_type = 0;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;

    ch = 1;

    if (s_info.type == STRM_SOURCE_VIDEO0) {
        printf("----ACTION_VIDEO0_OPEN_RT_STREAM-----\n\n");
        __this_strm->video_id = 0;
        req.rec.file    = __this->file[0];
        puts("start_video_rec0 \n");
        if (!rec_state) {
            if (!__this_strm->video_rec0) {
                sprintf(name, "video0.%d", 2);
                __this_strm->video_rec0 = server_open("video_server", name);
                if (!__this_strm->video_rec0) {
                    return VREC_ERR_V0_SERVER_OPEN;
                }
                server_register_event_handler(__this_strm->video_rec0, NULL, strm_video_rec_server_event_handler);
            }
        } else {
            id = __this_strm->video_id ? 1 : 0;
            req.rec.width = strm_rec_pix_w[res];
            req.rec.height = strm_rec_pix_h[res];
            __this_strm->video_rec0 = get_video_rec0_handler();
        }
    } else if (s_info.type == STRM_SOURCE_VIDEO1) {
        printf("----ACTION_VIDEO1_OPEN_RT_STREAM-----\n\n");
#ifdef CONFIG_VIDEO1_ENABLE
        puts("start_video_rec1 \n");
        __this_strm->video_id = 1;
        if (!rec_state) {
            if (!__this_strm->video_rec1) {
                sprintf(name, "video1.%d", 2);
                __this_strm->video_rec1 = server_open("video_server", name);
                if (!__this_strm->video_rec1) {
                    return VREC_ERR_V1_SERVER_OPEN;
                }
                server_register_event_handler(__this_strm->video_rec1, NULL, strm_video_rec_server_event_handler);
            }
        } else {
            id = __this_strm->video_id ? 1 : 0;
            req.rec.width = strm_rec_pix_w[id];
            req.rec.height = strm_rec_pix_h[id];
            __this_strm->video_rec1 = get_video_rec1_handler();
        }
        req.rec.file = __this->file[1];
        req.rec.IP_interval = 0;
#endif

#ifdef CONFIG_VIDEO2_ENABLE
        void *uvc_fd;
        struct uvc_capability uvc_cap;

        if (!dev_online("uvc")) {
            return -1;
        }
        puts("start_video_rec2 \n");
        __this_strm->video_id = 2;
        req.rec.file    = __this->file[2];
        if (!rec_state) {
            if (!__this_strm->video_rec1) {
                sprintf(name, "video2.%d", 2);
                __this_strm->video_rec1 = server_open("video_server", name);
                if (!__this_strm->video_rec1) {
                    return VREC_ERR_V1_SERVER_OPEN;
                }
                server_register_event_handler(__this_strm->video_rec1, NULL, strm_video_rec_server_event_handler);
            }
        } else {
            id = __this_strm->video_id ? 1 : 0;
            req.rec.width = strm_rec_pix_w[id];
            req.rec.height = strm_rec_pix_h[id];
            __this_strm->video_rec1 = get_video_rec1_handler();
        }
        req.rec.camera_type = VIDEO_CAMERA_UVC;
#if THREE_WAY_ENABLE
        req.rec.three_way_type = VIDEO_THREE_WAY_JPEG;
        req.rec.IP_interval = 99;
#else
        req.rec.three_way_type = 0;
        req.rec.IP_interval = 0;
#endif
        req.rec.width   = UVC_ENC_WIDTH;
        req.rec.height  = UVC_ENC_HEIGH;
        __this->uvc_width = req.rec.width;
        __this->uvc_height = req.rec.height;
        req.rec.uvc_id = __this->uvc_id;
        req.rec.file = __this->file[2];
#endif
    } else {
        printf("err unknown video type !!!!!\n\n");
        return -EINVAL;
    }

    if (!rec_state) {
        req.rec.fname   = "vid_***.avi";
    }

    __this_strm->width = req.rec.width;
    __this_strm->height = req.rec.height;

    req.rec.state 	= VIDEO_STATE_START;

#ifdef __CPU_AC521x__
    req.rec.format  = STRM_VIDEO_FMT_AVI;
#else
    req.rec.format  = STRM_VIDEO_FMT_MOV;
#endif
    req.rec.channel = __this_strm->channel = ch;
    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps 	= 0;
    req.rec.real_fps    = strm_video_rec_get_fps();
    req.rec.audio.sample_rate = s_info.sample_rate;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = AUDIO_VOLUME;
    req.rec.audio.buf = __this_strm->audio_buf;
    req.rec.audio.buf_len = __this_strm->audio_buf_size;
    req.rec.pkg_mute.aud_mute = !db_select("mic");
    req.rec.abr_kbps = strm_video_rec_get_abr(req.rec.width);
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

    text_osd.font_w = 16;
    text_osd.font_h = 32;

    max_one_line_strnum = strlen(video_rec_osd_buf);//20;
    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }
    text_osd.x = 0;//(req.rec.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (req.rec.height - text_osd.font_h * osd_line_num) / 16 * 16;

#ifdef __CPU_AC521x__
    text_osd.direction = 1;
#else
    text_osd.direction = 0;
#endif
    text_osd.osd_yuv = 0xe20095;
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
    text_osd.font_matrix_base = osd_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_str_matrix);

    req.rec.text_osd = 0;
    if (db_select("dat")) {
        req.rec.text_osd = &text_osd;
    }
    req.rec.slow_motion = 0;
#ifdef CONFIG_VIDEO1_ENABLE
    req.rec.tlp_time = 0;
#endif
#ifdef CONFIG_VIDEO2_ENABLE
    req.rec.tlp_time = db_select("gap");
#endif

    if (req.rec.tlp_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        req.rec.real_fps = 1000 / req.rec.tlp_time;

    }
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }

    req.rec.buf = __this_strm->video_buf;
    req.rec.buf_len = __this_strm->video_buf_size;
    req.rec.rec_small_pic 	= 0;

    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    if (rec_state) {
        req.rec.cycle_time = req.rec.cycle_time * 60;
    } else {
        req.rec.cycle_time = 0;
    }

    if (s_info.type == STRM_SOURCE_VIDEO0) {
        err = server_request(__this_strm->video_rec0, VIDEO_REQ_REC, &req);
    } else if (s_info.type == STRM_SOURCE_VIDEO1) {
        err = server_request(__this_strm->video_rec1, VIDEO_REQ_REC, &req);
    }
    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return -EINVAL;
    }
    if (rec_state) {
        __this_strm->state_ch2 = VIDREC_STA_START;
    } else {
        __this_strm->state = VIDREC_STA_START;
    }
    fv_video_rec_start_isp_scenes();
    return 0;
}

static int fv_video_rec_stop(void)
{
    union video_req req = {0};
    int err;

    req.rec.channel = __this_strm->channel;  /* video0的sd卡录像为:channel0,所以这里不能在占用channel0 */
    req.rec.state = VIDEO_STATE_STOP;
    if (s_info.type == STRM_SOURCE_VIDEO0) {
        if (__this_strm->video_rec0) {
            printf("----ACTION_VIDEO0_CLOSE_RT_STREAM-----\n\n");
            err = server_request(__this_strm->video_rec0, VIDEO_REQ_REC, &req);
            if (err != 0) {
                printf("ERR:stop video rec0 err 0x%x\n", err);
                return VREC_ERR_V0_REQ_STOP;
            }
        }
    } else if (s_info.type == STRM_SOURCE_VIDEO1) {
        if (__this_strm->video_rec1) {
            printf("----ACTION_VIDEO1_CLOSE_RT_STREAM-----\n\n");
            err = server_request(__this_strm->video_rec1, VIDEO_REQ_REC, &req);
            if (err != 0) {
                printf("ERR:stop video rec1 err 0x%x\n", err);
                return VREC_ERR_V0_REQ_STOP;
            }
        }
    }
    __this_strm->fbuf_fcnt = 0;
    __this_strm->fbuf_ffil = 0;
    printf("fv_video_rec_stop ok \n");
    return 0;
}


static int fv_video_rec_close(char close)
{
    int err;

    if (__this_strm->state != VIDREC_STA_START && __this_strm->state_ch2 != VIDREC_STA_START) {
        printf("err : __this_strm->state : %d , ch2 : %d \n", __this_strm->state, __this_strm->state_ch2);
        return 0;
    }
    fv_video_rec_stop_isp_scenes();

    __this_strm->video_id = 0;
    err = fv_video_rec_stop();
    if (err) {
        printf("fv_video_rec_stop err !!!\n");
        return -1;
    }

    if (s_info.type == STRM_SOURCE_VIDEO0) {
        if (__this_strm->video_rec0 && close && __this_strm->state == VIDREC_STA_START && __this_strm->state_ch2 != VIDREC_STA_START) {
            __this_strm->state == VIDREC_STA_STARTING;
            server_close(__this_strm->video_rec0);
            __this_strm->video_rec0 = NULL;
            printf("rtso server_close\n");
        }
    } else if (s_info.type == STRM_SOURCE_VIDEO1) {
        if (__this_strm->video_rec1 && close && __this_strm->state == VIDREC_STA_START && __this_strm->state_ch2 != VIDREC_STA_START) {
            __this_strm->state == VIDREC_STA_STARTING;
            server_close(__this_strm->video_rec1);
            __this_strm->video_rec1 = NULL;
        }
    }

    if (__this_strm->video_buf && !__this_strm->vbuf_share && close) {
        free(__this_strm->video_buf);
        __this_strm->video_buf = NULL;
    }
    if (__this_strm->audio_buf && !__this_strm->abuf_share && close) {
        free(__this_strm->audio_buf);
        __this_strm->audio_buf = NULL;
    }

    if (__this_strm->state_ch2 == VIDREC_STA_START) {
        __this_strm->video_rec0 = NULL;
        __this_strm->video_rec1 = NULL;
    }

    if (close) {
        __this_strm->video_buf = NULL;
        __this_strm->audio_buf = NULL;
    }
    __this_strm->vbuf_share = FALSE;
    __this_strm->abuf_share = FALSE;
    __this_strm->state_ch2 = VIDREC_STA_STOP;
    __this_strm->state = VIDREC_STA_STOP;

    fv_video_rec_start_isp_scenes();
    printf("stream_media close \n\n");
    return 0;
}
void fenice_video_rec_open(void)
{
    int ret;

    ret = fv_video_rec_open();
    if (ret) {
        printf("fv_video_rec_open err!!!\n");
    }
}
void fenice_video_rec_close(void)
{
    int ret;
    ret = fv_video_rec_close(1);
    if (ret) {
        printf("fv_video_rec_close err!!!\n");
    }
    //RTSP恢复默认
    s_info.type 		= STRM_SOURCE_VIDEO0;//摄像头类型
    s_info.width 		= 640;//分辨率
    s_info.height 		= 480;
    s_info.fps 			= STRM_VIDEO_REC_FPS0;//帧率
    s_info.sample_rate 	= 0;//采样率，默认配置为0
    s_info.channel_num 	= 1;//通道数
}
int strm_video_rec_open(void)//主要用于录像控制RTSP
{
    int ret = 0;
    if (__this_strm->is_open) {
        ret = fv_video_rec_open();
    }
    return ret;
}
int strm_video_rec_close(u8 close)//主要用于录像控制RTSP
{
    int ret = 0;
    if (__this_strm->is_open) {
        ret = fv_video_rec_close(close);
    }
    return ret;
}
/* 用于开启实时流时,stream_media_server回调 */
int fenice_video_rec_setup(void)
{
    if (!__this_strm->is_open) {//注意：RTSP可能会多次调用open，务必加判断，防止多次open！
        __this_strm->is_open = TRUE;
        fenice_video_rec_open();
    }
    return 0;
}

/* 用于实时流异常退出时,stream_media_server回调 */
int fenice_video_rec_exit(void)
{
    if (__this_strm->is_open) {//注意：RTSP可能会多次调用close，务必加判断，防止多次close！
        __this_strm->is_open = FALSE;
        fenice_video_rec_close();
    }
    return 0;
}

void *get_strm_video_rec_handler(void)
{
    return (void *)&fv_rec_handler;
}

int fenice_set_media_info(struct fenice_source_info *info)
{
    int ret = net_video_rec_state();

    s_info.type = info->type == 0 ? STRM_SOURCE_VIDEO0 : STRM_SOURCE_VIDEO1;

    if (s_info.type == STRM_SOURCE_VIDEO0) {
        if (ret == 640) {//录像共用通道跟随录像设置
            info->width = 640;
            info->height = 480;
        } else if (ret == 1280) {
            info->width = 1280;
            info->height = 720;
        }
    }
    s_info.width = info->width;
    s_info.height = info->height;
    __this_strm->width = s_info.width;
    __this_strm->height = s_info.height;
    printf("strm media info : %s , w : %d , h : %d \n"
           , s_info.type == STRM_SOURCE_VIDEO1 ? "video1/2" : "video0"
           , s_info.width
           , s_info.height);
    return 0;
}

int fenice_get_audio_info(struct fenice_source_info *info)
{
    s_info.sample_rate = VIDEO_REC_AUDIO_SAMPLE_RATE;
    info->width = s_info.width;
    info->height = s_info.height;
    info->fps = s_info.fps;
    info->sample_rate = s_info.sample_rate;//rtsp选择打开,将采样率设置为8000
    info->channel_num = s_info.channel_num;
    printf("strm audio info : %d , %d\n", info->sample_rate, info->channel_num);
    return 0;
}

int fenice_get_video_info(struct fenice_source_info *info)
{
    int ret = net_video_rec_state();

    if (s_info.type == STRM_SOURCE_VIDEO0) {
        if (ret == 640) {//录像共用通道跟随录像设置
            s_info.width = 640;
            s_info.height = 480;
        } else if (ret == 1280) {
            s_info.width = 1280;
            s_info.height = 720;
        }
    }

    __this_strm->width = s_info.width;
    __this_strm->height = s_info.height;
    info->width = s_info.width;
    info->height = s_info.height;
    info->fps = s_info.fps;
    info->sample_rate = s_info.sample_rate;
    info->channel_num = s_info.channel_num;
    printf("strm video info : %d , %d\n", info->sample_rate, info->channel_num);
    return 0;
}




