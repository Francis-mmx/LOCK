#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_engine_server.h"
#include "net_video_rec.h"
#include "video_system.h"
#include "gSensor_manage.h"
#include "user_isp_cfg.h"
#include "action.h"
#include "style.h"
#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "vrec_icon_osd.h"
#include "app_database.h"
#include "storage_device.h"
#include "server/ctp_server.h"
#include "os/os_compat.h"
#include "net_config.h"
#include "net_server.h"
#include "server/net_server.h"
#include "strm_video_rec.h"


static int net_rt_video1_open(struct intent *it);
static int net_rt_video0_open(struct intent *it);
static int net_video_rec_start(u8 mark);
static int net_video_rec_stop(u8 close);
static int net_video_rec0_ch2_stop(u8 close);
static int net_video_rec1_ch2_stop(u8 close);
static int net_video_rec1_stop(u8 close);
static int net_video_rec_take_photo(void (*callback)(char *buffer, int len));
static int net_video_rec_malloc_buf(char id);

#define AUDIO_VOLUME	64
#define NET_VIDEO_REC_SERVER_NAME	"net_video_server"
#define TAKE_PHOTO_MEM_LEN			150 * 1024

#define	NET_VIDEO_ERR(x)	{if(x) printf("######## %s %d err !!!! \n\n\n",__func__,__LINE__);}

static int net_rt_video0_stop(struct intent *it);
static int net_rt_video1_stop(struct intent *it);
static int net_video_rec_start_isp_scenes();
static int net_video_rec_stop_isp_scenes();
static int net_video_rec_control(void *_run_cmd);

static char file_str[64];
static int net_vd_msg[2];

#ifdef __CPU_AC521x__
static const u16 net_rec_pix_w[] = {1280, 640};
static const u16 net_rec_pix_h[] = {720,  480};
#else
static const u16 net_rec_pix_w[] = {1920, 1280, 640};
static const u16 net_rec_pix_h[] = {1088, 720,  480};
#endif

static const u16 net_pic_pix_w[] = {320, 640, 1280};
static const u16 net_pic_pix_h[] = {240, 480, 720 };

static struct net_video_hdl net_rec_handler = {0};
static struct strm_video_hdl *fv_rec_handler = NULL;
static struct video_rec_hdl *rec_handler = NULL;

#define __this_net  (&net_rec_handler)
#define __this_strm	(fv_rec_handler)
#define __this 	(rec_handler)


extern int cmd_video_rec_ctl(char start);

static OS_MUTEX net_vdrec_mutex;

int app_video_rec_test()
{
    static char start = 1;
    cmd_video_rec_ctl(start);//参数：1 启动APP录像， 0 停止APP录像
    start = !start;
    return 0;
}
/*
 *start user net video rec , 必须在net_config.h配置宏CONFIG_NET_USR_ENABLE 和NET_USR_PATH
 */
int user_net_video_rec_open(char forward)
{
    int ret;
    u8 open = 2;//char type : 0 audio , 1 video , 2 audio and video
    struct intent it;
    struct rt_stream_app_info info;

    info.width = 1280;
    info.height = 720;

    //帧率 net_config.h : NET_VIDEO_REC_FPS0/NET_VIDEO_REC_FPS1

    it.data = &open;
    it.exdata = (u32)&info;

    if (__this_net->is_open) {
        return 0;
    }
    __this_net->is_open = TRUE;
    if (forward) {
        ret = net_rt_video0_open(&it);
    } else {
        ret = net_rt_video1_open(&it);
    }
    return ret;
}

int user_net_video_rec_close(char forward)
{
    int ret;
    struct intent it;
    u8 close = 2;//char type : 0 audio , 1 video , 2 audio and video

    it.data = &close;
    if (!__this_net->is_open) {
        return 0;
    }
    if (forward) {
        ret = net_rt_video0_stop(&it);
    } else {
        ret = net_rt_video1_stop(&it);
    }
    __this_net->is_open = FALSE;
    return ret;
}

void user_net_video_rec_take_photo_cb(char *buf, int len)//必须打开user_net_video_rec_open()
{
    if (buf && len) {
        printf("take photo success \n");
        put_buf(buf, 32);
        /*
        //目录1写卡例子
        FILE *fd = fopen(CAMERA0_CAP_PATH"IMG_***.jpg","w+");
        if (fd) {
        	fwrite(fd,buf,len);
        	fclose(fd);
        }
        */
    }
}
//qua : 0 240p, 1 480p, 2 720p
int user_net_video_rec_take_photo(int qua, void (*callback)(char *buf, int len))//必须打开user_net_video_rec_open()
{
    db_update("qua", qua);
    net_video_rec_take_photo(callback);
    return 0;
}
//example 720P: user_net_video_rec_take_photo(2, user_net_video_rec_take_photo_cb);
int user_net_video_rec_take_photo_test(void)
{
    user_net_video_rec_take_photo(2, user_net_video_rec_take_photo_cb);
    return 0;
}
/*
 *end of user net video rec
 */

int net_video_rec_get_list_vframe(void)
{
    return __this_net->fbuf_fcnt;
}
void net_video_rec_pkg_get_video_in_frame(char *fbuf, u32 frame_size)
{
    __this_net->fbuf_fcnt++;
    __this_net->fbuf_ffil += frame_size;
}
void net_video_rec_pkg_get_video_out_frame(char *fbuf, u32 frame_size)
{
    if (__this_net->fbuf_fcnt) {
        __this_net->fbuf_fcnt--;
    }
    if (__this_net->fbuf_ffil) {
        __this_net->fbuf_ffil -= frame_size;
    }
}
void net_video_handler_init(void)
{
    fv_rec_handler = (struct strm_video_hdl *)get_strm_video_rec_handler();
    rec_handler  = (struct video_rec_hdl *)get_video_rec_handler();
}

void *get_net_video_rec_handler(void)
{
    return (void *)&net_rec_handler;
}

void *get_video_rec0_handler(void)
{
    return (void *)__this->video_rec0;
}

void *get_video_rec1_handler(void)
{
#ifdef CONFIG_VIDEO1_ENABLE
    return (void *)__this->video_rec1;
#endif
#ifdef CONFIG_VIDEO2_ENABLE
    return (void *)__this->video_rec2;
#endif
    return NULL;
}

int net_video_rec_dropframe_state(void)
{
    u32 sdram_size = __SDRAM_SIZE__;
    u8 res = db_select("res");
    int ret = __this && __this->state == VIDREC_STA_START;

    if (ret && ((__this_net->net_state_ch2 == VIDREC_STA_START && net_rec_pix_w[res] > 640 && !__this_net->video_id) || \
                dev_online("uvc") || dev_online("video1.*") || sdram_size <= 2 * 1024 * 1024)) { //单路720丢帧， 双路录像丢帧, SFC 2M版本录像丢帧
        return net_rec_pix_w[res];
    }
    return 0;
}
int net_video_rec_state(void)//获取录像状态，返回值大于0（1280/640）成功，失败 0
{
    u8 res = db_select("res");
    int ret = __this && __this->state == VIDREC_STA_START;

    if (ret) {
        return net_rec_pix_w[res];
    }
    return 0;
}
int net_pkg_get_video_size(int *width, int *height)
{
    u8 id = __this_net->video_id ? 1 : 0;
    *width = __this_net->net_videoreq[id].rec.width;
    *height = __this_net->net_videoreq[id].rec.height;
    return 0;
}
int net_video_buff_set_frame_cnt(void)
{
#ifdef NET_VIDEO_BUFF_FRAME_CNT
    return NET_VIDEO_BUFF_FRAME_CNT;
#else
    return 0;
#endif
}
u8 net_video_rec_get_drop_fp(void)
{
#ifdef NET_VIDEO_REC_DROP_REAl_FP
    return NET_VIDEO_REC_DROP_REAl_FP;
#else
    return 0;
#endif
}
u8 net_video_rec_get_lose_fram(void)
{
#ifdef NET_VIDEO_REC_LOSE_FRAME_CNT
    return NET_VIDEO_REC_LOSE_FRAME_CNT;
#else
    return 0;
#endif
}
u8 net_video_rec_get_send_fram(void)
{
#ifdef NET_VIDEO_REC_SEND_FRAME_CNT
    return  NET_VIDEO_REC_SEND_FRAME_CNT;
#else
    return 0;
#endif
}
int net_video_rec_get_fps(void)
{
//    int ret;
//    int fp = 20;
//    ret = net_video_rec_state();
//    if (ret) {
//        fp = LOCAL_VIDEO_REC_FPS;
//    } else {
//        if (!__this_net->video_id) {
//            fp = NET_VIDEO_REC_FPS0;
//        } else {
//            fp = NET_VIDEO_REC_FPS1;
//        }
//    }
//    ret = fp ? fp : 25;
    return 25;
}

void net_video_rec_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_VIDEO_REC;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);

#endif

}
void net_video_rec_fmt_notify(void)
{
    char buf[32];
#if defined CONFIG_ENABLE_VLIST
    extern void FILE_DELETE(char *__fname, u8 create_file);
    FILE_DELETE(NULL, __this_net->is_open || __this_strm->is_open);
    sprintf(buf, "frm:1");
    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "FORMAT", "NOTIFY", buf);
#endif
}

char *video_rec_finish_get_name(FILE *fd, int index, u8 is_emf)  //index ：video0前视0，video1则1，video2则2 , is_emf 紧急文件
{
    static char path[128] ALIGNE(32) = {0};
    u8 name[64];
    u8 *dir;
    int err;

#ifdef CONFIG_ENABLE_VLIST
    memset(path, 0, sizeof(path));
    err = fget_name(fd, name, 64);
    /*printf("finish_get_name: %s \n", name);*/
    if (err <= 0) {
        return NULL;
    }
    if (index < 0) {
        strcpy(path, name);
        return path;
    }
    switch (index) {
    case 0:
        dir = CONFIG_REC_PATH_0;
        break;
    case 1:
        dir = CONFIG_REC_PATH_1;
        break;
    case 2:
        dir = CONFIG_REC_PATH_2;
        break;
    default:
        return NULL;
    }
#ifdef CONFIG_EMR_DIR_ENABLE
    if (is_emf) {
        sprintf(path, "%s%s%s", dir, CONFIG_EMR_REC_DIR, name);
    } else
#endif
    {
        sprintf(path, "%s%s", dir, name);
    }

    return  path;
#else
    return NULL;
#endif
}

void net_video_rec_reopen(void)
{
    NET_VIDEO_ERR(strm_video_rec_close(1));
    NET_VIDEO_ERR(net_video_rec_stop(1));
    __this_net->fbuf_fcnt = 0;
    __this_net->fbuf_ffil = 0;
    NET_VIDEO_ERR(strm_video_rec_open());
    NET_VIDEO_ERR(net_video_rec_start(1));
}
int video_rec_finish_notify(char *path)
{
    int err = 0;

    if (__this_net->video_rec_err) {
        __this_net->video_rec_err = FALSE;
        return 0;
    }
    os_mutex_pend(&net_vdrec_mutex, 0);
#ifdef CONFIG_ENABLE_VLIST
    FILE_LIST_ADD(0, (const char *)path, __this_net->is_open  || __this_strm->is_open);
#endif
    os_mutex_post(&net_vdrec_mutex);
    return err;
}
int video_rec_delect_notify(FILE *fd, int id)
{
    int err = 0;
    if (__this_net->video_rec_err) {
        __this_net->video_rec_err = FALSE;
        return 0;
    }
#ifdef CONFIG_ENABLE_VLIST
    char *delect_path;
    os_mutex_pend(&net_vdrec_mutex, 0);
    char *path = video_rec_finish_get_name(fd, id, 0);
    if (path == NULL) {
        os_mutex_post(&net_vdrec_mutex);
        return -1;
    }
    FILE_DELETE((const char *)path, __this_net->is_open  || __this_strm->is_open);
    os_mutex_post(&net_vdrec_mutex);
#endif
    return err;
}

int video_rec_err_notify(const char *method)
{
    int err = 0;
    char *err_path;

    os_mutex_pend(&net_vdrec_mutex, 0);
    if (method && !strcmp((const char *)method, "VIDEO_REC_ERR")) {
        __this_net->video_rec_err = TRUE;
    }
    os_mutex_post(&net_vdrec_mutex);
    return err;
}
int video_rec_state_notify(void)
{
    int err = 0;
    net_vd_msg[0] = NET_VIDREC_STATE_NOTIFY;
    err = os_taskq_post_msg(NET_VIDEO_REC_SERVER_NAME, 1, (int)net_vd_msg);
    return err;
}

int video_rec_start_notify(void)
{
    return net_video_rec_control(0);//启动录像之前需要关闭实时流
}
int video_rec_all_stop_notify(void)
{
    int err = 0;
    net_vd_msg[0] = NET_VIDREC_STA_STOP;
    err = os_taskq_post_msg(NET_VIDEO_REC_SERVER_NAME, 1, (int)net_vd_msg);
    return err;
}
void net_video_rec_status_notify(void)
{
    char buf[128];
    u8 sta = 0;
    if (__this->state == VIDREC_STA_START) {
        sta = 1;
    }
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "status:%d", sta);
    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CTRL", "NOTIFY", buf);
}
int net_video_rec_event_notify(void)
{
    char buf[128];
    u32 res = db_select("res2");
#ifdef CONFIG_VIDEO1_ENABLE
    if (dev_online("video1.*"))
#else
#ifdef CONFIG_VIDEO2_ENABLE
    if (dev_online("uvc"))
#else
    if (0)
#endif
#endif
    {
        switch (res) {
        case VIDEO_RES_1080P:
            sprintf(buf, "status:1,h:%d,w:%d,fps:%d,rate:%d,format:1", 720, 1280, net_video_rec_get_fps(), net_video_rec_get_audio_rate());
            break;
        case VIDEO_RES_720P:
            sprintf(buf, "status:1,h:%d,w:%d,fps:%d,rate:%d,format:1", 480, 640, net_video_rec_get_fps(), net_video_rec_get_audio_rate());
            break;
        case VIDEO_RES_VGA:
            sprintf(buf, "status:1,h:%d,w:%d,fps:%d,rate:%d,format:1", 480, 640, net_video_rec_get_fps(), net_video_rec_get_audio_rate());
            break;
        default:
            sprintf(buf, "status:1,h:%d,w:%d,fps:%d,rate:%d,format:1", 480, 640, net_video_rec_get_fps(), net_video_rec_get_audio_rate());
            break;
        }
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "PULL_VIDEO_STATUS", "NOTIFY", buf);
    } else {
        strcpy(buf, "status:0");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "PULL_VIDEO_STATUS", "NOTIFY", buf);
    }
    return 0;
}
int net_video_rec_event_stop(void)
{
    strm_video_rec_close(1);
    net_video_rec_stop(1);
    return 0;
}
int net_video_rec_event_start(void)
{
    strm_video_rec_open();
    net_rt_video0_open(NULL);
    net_rt_video1_open(NULL);
    return 0;
}
int video_rec_sd_event_ctp_notify(char state)
{
    char buf[128];
    printf("~~~ : %s , %d\n\n", __func__, state);

#if defined CONFIG_ENABLE_VLIST
    if (!state) {
        FILE_REMOVE_ALL();
    }
#endif
    sprintf(buf, "online:%d", state);
    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "SD_STATUS", "NOTIFY", buf);

    if (state) {
        u32 space = 0;
        struct vfs_partition *part = NULL;
        if (storage_device_ready() == 0) {
            msleep(200);
            if (storage_device_ready()) {
                goto sd_scan;
            }
            printf("---%s , storage_device_not_ready !!!!\n\n", __func__);
            CTP_CMD_COMBINED(NULL, CTP_SD_OFFLINE, "TF_CAP", "NOTIFY", CTP_SD_OFFLINE_MSG);
        } else {
sd_scan:
            part = fget_partition(CONFIG_ROOT_PATH);
            fget_free_space(CONFIG_ROOT_PATH, &space);
            sprintf(buf, "left:%d,total:%d", space / 1024, part->total_size / 1024);
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "TF_CAP", "NOTIFY", buf);
        }
    }

    return 0;
}

//NET USE API
static void video_rec_get_app_status(struct intent *it)
{
    it->data = (const char *)__this;
    it->exdata = (u32)__this_net;
}
static void video_rec_get_path(struct intent *it)
{

    u8  buf[32];
#ifdef CONFIG_VIDEO0_ENABLE

    if (!strcmp(it->data, "video_rec0") && __this->file[0] != NULL) {
        fget_name((FILE *)__this->file[0], buf, sizeof(buf));
        sprintf(file_str, CONFIG_REC_PATH_0"%s", buf);
        it->data = file_str;
    }

#endif

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    if (!strcmp(it->data, "video_rec1") && __this->file[1] != NULL) {
        fget_name((FILE *)__this->file[1], buf, sizeof(buf));
        sprintf(file_str, CONFIG_REC_PATH_1"%s", buf);
        it->data = file_str;
    } else if (!strcmp(it->data, "video_rec2") && __this->file[2] != NULL) {
        fget_name((FILE *)__this->file[2], buf, sizeof(buf));
        sprintf(file_str, CONFIG_REC_PATH_1"%s", buf);
        it->data = file_str;
    }


    else
#endif
    {
        puts("get file name fail\n");
        it->data = NULL;
    }

    printf("file %s \n", it->data);
}
static void ctp_cmd_notity(const char *path)
{
#if defined CONFIG_ENABLE_VLIST
    FILE_LIST_ADD(0, path, __this_net->is_open || __this_strm->is_open);
#endif
}

int net_video_rec_get_audio_rate()
{
    return VIDEO_REC_AUDIO_SAMPLE_RATE;
}
static int video_cyc_file(u32 sel)
{
    /*
        struct server *server = NULL;
        union video_req req = {0};
        if (sel == 0) {
            server = __this->video_rec0;

            req.rec.channel = 0;
        } else if (sel == 1) {
            server = __this->video_rec1;
            req.rec.channel = 1;
        } else {
            server = __this->video_rec2;
            req.rec.channel = 2;
        }
        if ((__this->state != VIDREC_STA_START) || (server == NULL)) {
            return -EINVAL;
        }

        req.rec.state   = VIDEO_STATE_CYC_FILE;
        req.rec.cyc_file = 1;
        int err = server_request(server, VIDEO_REQ_REC, &req);
        if (err != 0) {
            return -EINVAL;
        }
    */
    return 0;
}


/*后拉专用*/
/*
 *  *场景切换使能函数，如果显示打开就用显示句柄控制，否则再尝试用录像句柄控制
 *   */
static int net_video_rec_doing_isp_scenes(void *p)
{
    if (__this_net->isp_scenes_status) {
        return 0;
    }

    stop_update_isp_scenes();
    if ((int)p) {
        if (__this_net->net_video_rec && ((__this_net->net_state == VIDREC_STA_START) ||
                                          (__this_net->net_state == VIDREC_STA_STARTING))) {
            __this_net->isp_scenes_status = 1;
            return start_update_isp_scenes(__this_net->net_video_rec);
        } else if (__this_net->net_video_rec1 && ((__this_net->net_state1 == VIDREC_STA_START) ||
                   (__this_net->net_state1 == VIDREC_STA_STARTING))) {
            __this_net->isp_scenes_status = 2;
            return start_update_isp_scenes(__this_net->net_video_rec1);
        }
    }

    __this_net->isp_scenes_status = 0;

    return 1;
}
static int net_video_rec_start_isp_scenes()
{
    return wait_completion(isp_scenes_switch_timer_done, net_video_rec_doing_isp_scenes, (void *)1);
}
static int net_video_rec_stop_isp_scenes()
{
    if (!__this_net->isp_scenes_status) {
        return 0;
    }
    __this_net->isp_scenes_status = 0;
    stop_update_isp_scenes();
    return 0;
}
static int set_label_config(u16 image_width, u16 image_height, u32 font_color,
                            struct video_text_osd *label)
{
    static char label_format[128] ALIGNE(64) = "yyyy-nn-dd hh:mm:ss";
    if (!label) {
        return 0;
    }

    /*
     *日期标签设置
     *1920以下使用 16x32字体大小，以上使用32x64字体
     */
#ifdef __CPU_AC521x__
    label->direction = 1;
#else
    label->direction = 0;
#endif
    if (image_width > 1920) {
        return -1;

    } else {
        label->font_w = 16;
        label->font_h = 32;
        label->text_format = label_format;
        label->font_matrix_table = osd_str_total;
        label->font_matrix_base = osd_str_matrix;
        label->font_matrix_len = sizeof(osd_str_matrix);
    }
    label->osd_yuv = font_color;

    label->x = (image_width - strlen(label_format) * label->font_w) / 64 * 64;
    label->y = (image_height - label->font_h - 16) / 16 * 16;


    return 0;
}

int video_rec_cap_photo(char *buf, int len)
{
    u32 *flen;
    u8 *cap_img;
    if (__this_net->cap_image) {
        cap_img = &__this_net->cap_image;
    } else {
        cap_img = &__this_strm->cap_image;
    }
    if (*cap_img && __this->cap_buf) {
        flen = __this->cap_buf;
        memcpy(__this->cap_buf + 4, buf, len);
        *flen = len;
        *cap_img = FALSE;
    }
    return 0;
}
static int net_video_rec_take_photo(void (*callback)(char *buffer, int len))
{
    struct server *server = NULL;
    union video_req req = {0};
    int err, i;
    char *path;
    char buf[128] = {0};
    char video_name_buf[20] = {0};
    char name_buf[20];
    char rec_flag = 0;
    char buf_lag = 0;
    struct video_text_osd text_osd;
    u32 *image_len;
    int time = 0;
    u8 *cap_img;
    u8 video_index = 0;
    u8 id;

    /*不开实时流不能拍照*/

    if (!(__this_strm->state == VIDREC_STA_START || __this_strm->state_ch2 == VIDREC_STA_START)
        && !(__this_net->net_state == VIDREC_STA_START || __this_net->net_state_ch2 == VIDREC_STA_START)
        && !(__this_net->net_state1 == VIDREC_STA_START || __this_net->net_state1_ch2 == VIDREC_STA_START)) {
        printf("waring :net video not open or video record running \n");
        goto error;
    }

#ifdef CONFIG_VIDEO5_ENABLE
    goto error;
#endif

    u8 res = db_select("res");
    u8 qua = db_select("qua");
    if (qua > 2) {//防止读VM出错
        qua = 2;
    }
    __this_net->cap_image = FALSE;
    __this_strm->cap_image = FALSE;
    if (__this->state == VIDREC_STA_START) { //录像抓拍
        if (__this_net->net_state_ch2 == VIDREC_STA_START || __this_net->net_state1_ch2 == VIDREC_STA_START || \
            (__this_net->net_state == VIDREC_STA_START || __this_net->net_state1 == VIDREC_STA_START)) {
            id = __this_net->video_id ? 1 : 0;
            if (__this_net->video_id) {
                server = __this_net->net_video_rec1;
            } else {
                server = __this_net->net_video_rec;
            }
            if (callback && !(dev_online("uvc") | dev_online("video1.*"))) {
                if (res == 0 && qua == 2 && !id) {//录像720，实时流VGA，同源拍照
                    __this_net->cap_image = TRUE;
                    goto user_cap;
                }
                req.icap.width = net_pic_pix_w[qua];
                req.icap.height = net_pic_pix_h[qua];
            } else {
                req.icap.width =  __this_net->net_videoreq[id].rec.width;
                req.icap.height = __this_net->net_videoreq[id].rec.height;
            }
            video_index = __this_net->video_id;
        } else if (__this_strm->state_ch2 == VIDREC_STA_START || __this_strm->state == VIDREC_STA_START) { //RTSP实时流拍照
            if (__this_strm->video_id) {
                server = __this_strm->video_rec1;
            } else {
                server = __this_strm->video_rec0;
            }
            if (callback && !(dev_online("uvc") | dev_online("video1.*"))) {
                if (res == 0 && qua == 2 && !__this_strm->video_id) {//录像720，实时流VGA，同源拍照
                    __this_strm->cap_image = TRUE;
                    goto user_cap;
                }
                req.icap.width = net_pic_pix_w[qua];
                req.icap.height = net_pic_pix_h[qua];
            } else {
                req.icap.width = __this_strm->width;
                req.icap.height = __this_strm->height;
            }
            video_index = __this_strm->video_id;
        } else {
            server = __this->video_rec0;
            req.icap.width = net_rec_pix_w[res];
            req.icap.height = net_rec_pix_h[res];
        }
    } else { //不录像分支
        req.icap.width = net_pic_pix_w[qua];
        req.icap.height = net_pic_pix_h[qua];
        if (__this_strm->state == VIDREC_STA_START  && req.icap.width <= __this_strm->width) {//RTSP拍照,分辨率小于等于优先权
            if (!__this_strm->video_id) {
                server = __this_strm->video_rec0;
            } else {
                server = __this_strm->video_rec1;
            }
            video_index = __this_strm->video_id;
            printf("use_strm_video%d\n\n", __this_strm->video_id);
        } else {
            id = __this_net->video_id ? 1 : 0;
            if ((__this_net->net_state == VIDREC_STA_START || __this_net->net_state1 == VIDREC_STA_START)
                && req.icap.width <= __this_net->net_videoreq[id].rec.width) {//私有实时流拍照,分辨率小于等于优先权
                if (!__this_net->video_id) {
                    server = __this_net->net_video_rec;
                } else {
                    server = __this_net->net_video_rec1;
                }
                video_index = __this_net->video_id;
            } else if (__this->state == VIDREC_STA_STOP ||  __this->state == VIDREC_STA_IDLE) {//不录像拍照
                if (__this_strm->state == VIDREC_STA_START) {
                    if (!__this_strm->video_id) {
                        server = __this_strm->video_rec0;
                    } else {
                        server = __this_strm->video_rec1;
                    }
                    sprintf(video_name_buf, "video%d.3", __this_strm->video_id);
                    video_index = __this_strm->video_id;
                    if (video_index && qua >= 1) { //后视最大VGA,内存不足，防止后视拍照设置大于720
                        qua = 1;
                        printf("use_strm2_video%d\n\n", __this_strm->video_id);
                        goto take_photo;
                    }
                } else if (__this_net->net_state == VIDREC_STA_START || __this_net->net_state1 == VIDREC_STA_START) {
                    if (!__this_net->video_id) {
                        server = __this_net->net_video_rec;
                    } else {
                        server = __this_net->net_video_rec1;
                    }
                    sprintf(video_name_buf, "video%d.3", __this_net->video_id);
                    video_index = __this_net->video_id;
                    if (video_index && qua >= 1) { //后视最大VGA,内存不足，防止后视拍照设置大于720
                        qua = 1;
                        printf("use_net2_video%d\n\n", __this_net->video_id);
                        goto take_photo;
                    }
                } else {
                    goto error;
                }

                rec_flag = 1;
                printf("open_server_%s \n\n", video_name_buf);
                server = server_open("video_server", video_name_buf);
            } else {
                printf("take photos err !!\n");
                goto error;
            }
        }
    }

take_photo:

    if (!server) {
        printf("\nserver open err !!!\n");
        goto error;
    }

    //重置拍照分辨率
    if (__this->state == VIDREC_STA_STOP) {
        req.icap.width = net_pic_pix_w[qua];
        req.icap.height = net_pic_pix_h[qua];
    }
    if (!__this->cap_buf && __this->state == VIDREC_STA_STOP) {
        for (i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
            if (__this->video_buf[i] != NULL && i != video_index) {
                __this->cap_buf = __this->video_buf[i];
                switch (i) {
                case 0:
                    req.icap.buf_size = VREC0_FBUF_SIZE;
                    break;
                case 1:
                    req.icap.buf_size = VREC1_FBUF_SIZE;
                    break;
                case 2:
                    req.icap.buf_size = VREC2_FBUF_SIZE;
                    break;
                default:
                    break;
                }
            }
        }
    }
    if (!__this->cap_buf) {
        __this->cap_buf = (u8 *)malloc(TAKE_PHOTO_MEM_LEN);
        if (!__this->cap_buf) {
            printf("\ntake photo no mem !!!\n");
            goto error;
        }
        req.icap.buf_size = TAKE_PHOTO_MEM_LEN;
        buf_lag = 1;
    }

    req.icap.quality = VIDEO_MID_Q;
    req.icap.buf = __this->cap_buf;
    req.rec.text_osd = NULL;
    req.rec.graph_osd = NULL;
    req.icap.text_label = NULL;

    /*
     * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     */
    if (db_select("dat")) {
        req.icap.text_label = &text_osd;
        set_label_config(req.icap.width, req.icap.height, 0xe20095, req.icap.text_label);
    }

    req.icap.file_name = name_buf;
    __this->photo_camera_sel = video_index;
    switch (__this->photo_camera_sel) {
    case 0 :
        req.icap.path = CAMERA0_CAP_PATH"IMG_****.jpg";
        path = CAMERA0_CAP_PATH;
        break;
    case 1 :
        req.icap.path = CAMERA1_CAP_PATH"IMG_****.jpg";
        path = CAMERA1_CAP_PATH;
        break;
    case 2 :
        req.icap.path = CAMERA2_CAP_PATH"IMG_****.jpg";
        path = CAMERA2_CAP_PATH;
        break;
    default:
        break;
    }

    if (callback) {
        image_len = __this->cap_buf;
        req.icap.save_cap_buf = TRUE;//保存到cap_buff写TRUE，数据格式，前4字节为数据长度，4字节后为有效数据
    }

    err = server_request(server, VIDEO_REQ_IMAGE_CAPTURE, &req);
    if (err != 0) {
        puts("\n\n\ntake photo err\n\n\n");
        goto error;
    }

    if (callback) {
        if (image_len && __this->cap_buf) {
            callback(__this->cap_buf + 4, *image_len);
        } else {
            callback(NULL, 0);
        }
    } else {
#if defined CONFIG_ENABLE_VLIST
        sprintf(buf, "%s%s", path, req.icap.file_name);
        printf("%s\n\n", buf);
        FILE_LIST_ADD(0, buf, __this_net->is_open || __this_strm->is_open);
#endif
    }
    if (rec_flag) {
        server_close(server);
    }
    if (buf_lag) {
        free(__this->cap_buf);
    }
    __this->cap_buf = NULL;
    return 0;

user_cap:
    if (!__this->cap_buf) {
        __this->cap_buf = (u8 *)zalloc(TAKE_PHOTO_MEM_LEN);
        if (!__this->cap_buf) {
            printf("\nuser take photo no mem !!!\n");
            goto error;
        }
        req.icap.buf_size = TAKE_PHOTO_MEM_LEN;
        buf_lag = 1;
    }
    if (__this_net->cap_image) {
        cap_img = &__this_net->cap_image;
    } else {
        cap_img = &__this_strm->cap_image;
    }
    while (*cap_img) {
        os_time_dly(1);
        time++;
        if (time > 300) { //3s timer out
            printf("user take photo timer out\n\n");
            break;
        }
    }
    if (!(*cap_img) && callback && time < 300) {
        image_len = __this->cap_buf;
        callback(__this->cap_buf + 4, *image_len);
    }
    if (buf_lag) {
        free(__this->cap_buf);
    }
    __this->cap_buf = NULL;
    return 0;

error:
#if defined CONFIG_ENABLE_VLIST
    CTP_CMD_COMBINED(NULL, CTP_REQUEST, "PHOTO_CTRL", "NOTIFY", CTP_REQUEST_MSG);
#endif
    if (callback) {
        callback(NULL, 0);
    }
    if (server && rec_flag) {
        server_close(server);
    }
    if (buf_lag) {
        free(__this->cap_buf);
    }
    __this->cap_buf = NULL;
    return -EINVAL;
}



extern int atoi(const char *);
static void rt_stream_cmd_analysis(u8 chl,  u32 add)
{
    char *key;
    char *value;
    struct rt_stream_app_info *info = (struct rt_stream_app_info *)add;

    __this_net->net_videoreq[chl].rec.width = info->width;
    __this_net->net_videoreq[chl].rec.height = info->height;
    __this_net->net_videoreq[chl].rec.format = info->type - NET_VIDEO_FMT_AVI;
#ifdef __CPU_AC521x__
    __this_net->net_videoreq[chl].rec.format = 0;
#endif
    __this_net->net_videoreq[chl].rec.fps = net_video_rec_get_fps();
    __this_net->net_videoreq[chl].rec.real_fps = net_video_rec_get_fps();
    __this_net->priv = info->priv;
    printf(">>>>>>>> ch %d , info->type : %d ,w:%d , h:%d\n\n", chl, info->type, info->width, info->height);

}

static void net_video_rec_set_bitrate(unsigned int bits_rate)
{
    union video_req req = {0};

    req.rec.channel = __this_net->channel;

    req.rec.state = VIDEO_STATE_RESET_BITS_RATE;
    req.rec.abr_kbps = bits_rate;

    server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
}

/*码率控制，根据具体分辨率设置*/
static int net_video_rec_get_abr(u32 width)
{
    if (__this_net->net_state_ch2 == VIDREC_STA_STARTING || __this_net->net_state1_ch2 == VIDREC_STA_STARTING) { //共用通道时候需要获取本地录像的码率设置
        return video_rec_get_abr_from(width);
    } else { //其他情况设置网络实时流的码率
        if (width <= 720) {
            return 2500;
        } else if (width <= 1280) {
            return 5000;
        } else {
            return 0;
        }
    }
}

static void net_rec_dev_server_event_handler(void *priv, int argc, int *argv)
{
    int mark = 2;
    struct intent it;
    /*
     *该回调函数会在录像过程中，写卡出错被当前录像APP调用，例如录像过程中突然拔卡
     */
    switch (argv[0]) {
    case VIDEO_SERVER_UVM_ERR:
        log_e("APP_UVM_DEAL_ERR\n");
        break;
    case VIDEO_SERVER_PKG_ERR:
        log_e("video_server_pkg_err\n");
        if (__this->state == VIDREC_STA_START) {
            net_video_rec_control(0);
        }
        break;
    case VIDEO_SERVER_PKG_END:
        if (db_select("cyc")) {
            /*video_rec_savefile((int)priv);*/
        } else {
            net_video_rec_control(0);
        }
        break;
    case VIDEO_SERVER_PKG_NET_ERR:

        init_intent(&it);
        it.data = &mark;
        net_rt_video0_stop(&it);
        net_rt_video1_stop(&it);
        strm_video_rec_close(1);
        break;
    default :
        /*log_e("unknow rec server cmd %x , %x!\n", argv[0], (int)priv);*/
        break;
    }
}

static int net_video_rec0_start()
{
    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;
    char buf[128];
    u8 res = db_select("res");

    err = net_video_rec_malloc_buf(0);
    if (err) {
        return err;
    }

    if (!__this_net->net_video_rec) {
#ifdef CONFIG_VIDEO5_ENABLE
        __this_net->net_video_rec = server_open("video_server", "video5.0");
#else
        __this_net->net_video_rec = server_open("video_server", "video0.1");
#endif
        if (!__this_net->net_video_rec) {
            return VREC_ERR_V0_SERVER_OPEN;
        }

        server_register_event_handler(__this_net->net_video_rec, (void *)0, net_rec_dev_server_event_handler);
    }

    __this_net->video_id = 0;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.channel = __this_net->channel = 1;
    req.rec.width 	= __this_net->net_videoreq[0].rec.width;
    req.rec.height 	= __this_net->net_videoreq[0].rec.height;
    printf(">>>>>>width=%d    height=%d\n\n\n\n", __this_net->net_videoreq[0].rec.width, __this_net->net_videoreq[0].rec.height);
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_0;
    req.rec.file    = __this->file[0];

    if (__this_net->net_videoreq[0].rec.format == 1) {
        puts("in h264\n\n");
        req.rec.format 	= NET_VIDEO_FMT_MOV;
    } else {
        req.rec.format 	= NET_VIDEO_FMT_AVI;
    }
    if (__this_net->net_videoreq[0].rec.format == 1) {
        req.rec.fname    = "vid_***.mov";
    } else {
        req.rec.fname    = "vid_***.avi";
    }

    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps = 0;
    req.rec.real_fps = net_video_rec_get_fps();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video0_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;
    req.rec.audio.sample_rate = VIDEO_REC_AUDIO_SAMPLE_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = AUDIO_VOLUME;
    req.rec.audio.buf = __this_net->audio_buf;
    req.rec.audio.buf_len = __this_net->audio_buf_size;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    /*
    *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
    *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
    * roio_ratio : 区域比例系数
    */
    req.rec.abr_kbps = net_video_rec_get_abr(req.rec.width);

#if defined __CPU_AC5401__
    req.rec.IP_interval = 0;
#elif defined __CPU_AC5601__
    if (req.rec.height > 720) {
        req.rec.IP_interval = 1;
    } else {
        req.rec.IP_interval = 0;
    }
#endif

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    /* req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16; */
    /* req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16; */
    /* req.rec.roi.roi2_xy = 0; */
    /* req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0; */
    /* req.rec.roi.roio_ratio = 256 * 70 / 100 ; */
    /* req.rec.roi.roio_ratio1 = 256 * 90 / 100; */
    /* req.rec.roi.roio_ratio2 = 0; */
    /* req.rec.roi.roio_ratio3 = 256 * 80 / 100; */


    /*
     * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     */
    text_osd.font_w = 16;
    text_osd.font_h = 32;
    max_one_line_strnum = strlen(video_rec_osd_buf);//21;

    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }
    osd_max_heigh = (req.rec.height == 1088) ? 1080 : req.rec.height ;
    text_osd.x = 0;//(req.rec.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (osd_max_heigh - text_osd.font_h * osd_line_num) / 16 * 16;
    text_osd.osd_yuv = 0xe20095;
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
    text_osd.font_matrix_base = osd_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_str_matrix);
#ifdef __CPU_AC521x__
    text_osd.direction = 1;
#else
    text_osd.direction = 0;
#endif

#ifdef CONFIG_OSD_LOGO
    graph_osd.bit_mode = 16;//2bit的osd需要配置3个color
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = icon_osd_buf;
    graph_osd.icon_size = sizeof(icon_osd_buf);
#endif
    req.rec.text_osd = NULL;
    req.rec.graph_osd = NULL;
    if (db_select("dat")) {
        req.rec.text_osd = &text_osd;
#ifdef CONFIG_OSD_LOGO
        req.rec.graph_osd = &graph_osd;
#endif
    }

    /*
    *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
    */
    req.rec.slow_motion = 0;
    req.rec.tlp_time = db_select("gap");
    /* req.rec.tlp_time = 40; // 0; */

    /*printf("\n\ntl_time : %d\n\n", req.rec.tlp_time);*/
    if (req.rec.slow_motion || req.rec.tlp_time) {
        /*         req.rec.audio.sample_rate = 0; */
        /* req.rec.audio.channel 	= 0; */
        /* req.rec.audio.volume    = 0; */
        /* req.rec.audio.buf = 0; */
        /* req.rec.audio.buf_len = 0; */
    }

    req.rec.buf = __this_net->net_v0_fbuf;
    req.rec.buf_len = __this_net->net_v0_fbuf_size;

    struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
    if (!addr) {
        addr = cdp_srv_get_cli_addr(__this_net->priv);
    }
    /*
     *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
     *                        ,inet_addr(addr->sin_addr.s_addr)
     *                                        ,_FORWARD_PORT);
      *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
     *                        ,inet_addr(addr->sin_addr.s_addr)
     *                                        ,_FORWARD_PORT);
    *
     * */
#if (defined CONFIG_NET_UDP_ENABLE)
    sprintf(req.rec.net_par.netpath, "udp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _FORWARD_PORT);
#elif (defined CONFIG_NET_TCP_ENABLE)
    sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _FORWARD_PORT);
#elif (defined CONFIG_NET_USR_ENABLE)
    sprintf(req.rec.net_par.netpath, "usr://%s", NET_USR_PATH);
#endif

    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }
    return 0;
}

static int net_video_rec0_aud_mute()
{
#ifdef CONFIG_VIDEO0_ENABLE

    if (__this_net->net_state != VIDREC_STA_START) {
        return 0;
    }

    union video_req req = {0};

    req.rec.channel = 1;

    req.rec.state = VIDEO_STATE_PKG_MUTE;

    req.rec.pkg_mute.aud_mute = !db_select("mic");
    /* req.rec.pkg_mute.aud_mute = !__this->rec_info->voice_on; */

    printf("net video rec0 aud mute %d", req.rec.pkg_mute.aud_mute);

    return server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

#else
    return 0;

#endif
}


static int net_video_rec0_stop(u8 close)
{
    union video_req req = {0};
    int err;
    puts("\nnet video rec0 stop\n");

    net_video_rec_stop_isp_scenes();
    req.rec.channel = 1;
    req.rec.state = VIDEO_STATE_STOP;
    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

    if (err != 0) {
        printf("\nstop rec err 0x%x\n", err);
        return VREC_ERR_V0_REQ_STOP;
    }

    if (close) {
        if (__this_net->net_video_rec) {
            server_close(__this_net->net_video_rec);
            __this_net->net_video_rec = NULL;
        }
    }
    __this_net->net_state = VIDREC_STA_STOP;
    net_video_rec_start_isp_scenes();
    __this_net->video_id = 0;
    return 0;
}

static int net_video_rec0_ch2_stop(u8 close)
{
    union video_req req = {0};
    int err;

    if (__this_net->net_state_ch2 == VIDREC_STA_START) {
        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }
        __this_net->net_state_ch2 = VIDREC_STA_STOP;
        puts("stop_video_rec ch2-\n");
    }
#ifdef CONFIG_VIDEO5_ENABLE
    if (close) {
        if (__this_net->net_video_rec) {
            server_close(__this_net->net_video_rec);
            __this_net->net_video_rec = NULL;
        }
    }
#endif
    __this_net->net_state_ch2 = VIDREC_STA_STOP;
    __this_net->net_video_rec = NULL;
    __this_net->video_id = 0;
    return 0;
}
static int net_video_rec1_start()
{
    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;

#ifdef CONFIG_VIDEO1_ENABLE
    err = net_video_rec_malloc_buf(1);
#endif
#ifdef CONFIG_VIDEO2_ENABLE
    err = net_video_rec_malloc_buf(2);
#endif
    if (err) {
        return err;
    }

    req.rec.width 	= __this_net->net_videoreq[1].rec.width;
    req.rec.height 	= __this_net->net_videoreq[1].rec.height;

#ifdef CONFIG_VIDEO1_ENABLE
    puts("start_video_rec1 \n");
    if (!__this_net->net_video_rec1) {
        __this_net->net_video_rec1 = server_open("video_server", "video1.1");
        if (!__this_net->net_video_rec1) {
            return VREC_ERR_V1_SERVER_OPEN;
        }
        server_register_event_handler(__this_net->net_video_rec1, (void *)1, net_rec_dev_server_event_handler);
    }
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.file    = __this->file[1];
    req.rec.IP_interval = 0;
    __this_net->video_id = 1;
#endif

#ifdef CONFIG_VIDEO2_ENABLE
    char name[12];
    void *uvc_fd;
    struct uvc_capability uvc_cap;

    puts("start_video_rec2 \n");
    if (!__this_net->net_video_rec1) {
        sprintf(name, "video2.%d", __this->uvc_id + 1);
        __this_net->net_video_rec1 = server_open("video_server", name);
        if (!__this_net->net_video_rec1) {
            return -EINVAL;
        }
        server_register_event_handler(__this_net->net_video_rec1, (void *)1, net_rec_dev_server_event_handler);
    }
    req.rec.camera_type = VIDEO_CAMERA_UVC;
    req.rec.three_way_type = 0;
    req.rec.IP_interval = 0;
    req.rec.width 	= UVC_ENC_WIDTH;
    req.rec.height 	= UVC_ENC_HEIGH;
    __this->uvc_width = req.rec.width;
    __this->uvc_height = req.rec.height;
    req.rec.uvc_id = __this->uvc_id;
    req.rec.file    = __this->file[2];
    __this_net->video_id = 2;
#endif
    req.rec.channel = __this_net->channel = 1;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fname   = "vid_***.avi";
    req.rec.format  = NET_VIDEO_FMT_AVI;
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video1_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;
    req.rec.fps = 0;
    req.rec.real_fps = net_video_rec_get_fps();
    req.rec.audio.sample_rate = VIDEO_REC_AUDIO_SAMPLE_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = AUDIO_VOLUME;
    req.rec.audio.buf = __this_net->audio_buf;
    req.rec.audio.buf_len = __this_net->audio_buf_size;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    req.rec.abr_kbps = net_video_rec_get_abr(req.rec.width);


    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    /* req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16; */
    /* req.rec.roi.roio_ratio = 256 * 70 / 100 ; */
    /* req.rec.roi.roi1_xy = 0; */
    /* req.rec.roi.roi2_xy = 0; */
    /* req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0; */
    /* req.rec.roi.roio_ratio1 = 0; */
    /* req.rec.roi.roio_ratio2 = 0; */
    /* req.rec.roi.roio_ratio3 = 256 * 80 / 100; */

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

    req.rec.buf = __this_net->net_v0_fbuf;
    req.rec.buf_len = __this_net->net_v0_fbuf_size;
    req.rec.rec_small_pic 	= 0;

    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = 0;

    struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
    if (!addr) {
        addr = cdp_srv_get_cli_addr(__this_net->priv);
    }
    /*
    *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
    *                        ,inet_addr(addr->sin_addr.s_addr)
    *                                        ,_FORWARD_PORT);
    *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
    *                        ,inet_addr(addr->sin_addr.s_addr)
    *                                        ,_FORWARD_PORT);
    *
    * */
#if (defined CONFIG_NET_UDP_ENABLE)
    sprintf(req.rec.net_par.netpath, "udp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _BEHIND_PORT);
#elif (defined CONFIG_NET_TCP_ENABLE)
    sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _BEHIND_PORT);
#elif (defined CONFIG_NET_USR_ENABLE)
    sprintf(req.rec.net_par.netpath, "usr://%s", NET_USR_PATH);
#endif

    err = server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return VREC_ERR_V1_SERVER_OPEN;
    }

    return 0;
}

static int net_video_rec1_ch2_start()
{
    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    int rec_state = (__this->state == VIDREC_STA_START ? 1 : 0);

    req.rec.width 	= __this_net->net_videoreq[1].rec.width;
    req.rec.height 	= __this_net->net_videoreq[1].rec.height;

#ifdef CONFIG_VIDEO1_ENABLE
    puts("start_video_rec1 ch2\n");
    __this_net->net_video_rec1 = __this->video_rec1;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.file    = __this->file[1];
    req.rec.IP_interval = 0;
    __this_net->video_id = 1;
#endif

#ifdef CONFIG_VIDEO2_ENABLE
    char name[12];
    void *uvc_fd;
    struct uvc_capability uvc_cap;

    puts("start_video_rec2 ch2 \n");
    __this_net->net_video_rec1 = __this->video_rec2;
    req.rec.camera_type = VIDEO_CAMERA_UVC;
#if THREE_WAY_ENABLE
    req.rec.three_way_type = VIDEO_THREE_WAY_JPEG;
    req.rec.IP_interval = 99;
#else
    req.rec.three_way_type = 0;
    req.rec.IP_interval = 0;
#endif
    req.rec.width 	= UVC_ENC_WIDTH;
    req.rec.height 	= UVC_ENC_HEIGH;
    __this->uvc_width = req.rec.width;
    __this->uvc_height = req.rec.height;
    req.rec.uvc_id = __this->uvc_id;
    req.rec.file    = __this->file[2];
    __this_net->video_id = 2;
#endif

    if (!rec_state) {
        req.rec.fname   = "vid_***.avi";
    }
    req.rec.fps = 0;
    req.rec.real_fps = net_video_rec_get_fps();

    req.rec.state 	= VIDEO_STATE_START;
#ifdef __CPU_AC521x__
    req.rec.format  = NET_VIDEO_FMT_AVI;
#else
    req.rec.format  = NET_VIDEO_FMT_MOV;
#endif
    req.rec.channel = __this_net->channel = 1;
    req.rec.quality = VIDEO_LOW_Q;
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video1_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;
    req.rec.audio.sample_rate = VIDEO_REC_AUDIO_SAMPLE_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = AUDIO_VOLUME;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");
    req.rec.abr_kbps = net_video_rec_get_abr(req.rec.width);
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

    req.rec.buf = __this->video_buf[__this_net->video_id];
    req.rec.buf_len = VREC1_FBUF_SIZE;
    req.rec.rec_small_pic 	= 0;

    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;

    struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
    if (!addr) {
        addr = cdp_srv_get_cli_addr(__this_net->priv);
    }
    /*
    *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
    *                        ,inet_addr(addr->sin_addr.s_addr)
    *                                        ,_FORWARD_PORT);
    *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
    *                        ,inet_addr(addr->sin_addr.s_addr)
    *                                        ,_FORWARD_PORT);
    *
    * */
#if (defined CONFIG_NET_UDP_ENABLE)
    sprintf(req.rec.net_par.netpath, "udp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _BEHIND_PORT);
#elif (defined CONFIG_NET_TCP_ENABLE)
    sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _BEHIND_PORT);
#elif (defined CONFIG_NET_USR_ENABLE)
    sprintf(req.rec.net_par.netpath, "usr://%s", NET_USR_PATH);
#endif

    err = server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return VREC_ERR_V1_SERVER_OPEN;
    }

    return 0;
}
static int net_video_rec1_aud_mute()
{
#ifdef CONFIG_VIDEO1_ENABLE

    if (__this_net->net_state1 != VIDREC_STA_START) {
        return	0;
    }

    union video_req req = {0};

    req.rec.channel = 1;

    req.rec.state = VIDEO_STATE_PKG_MUTE;

    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);

#else
    return 0;

#endif
}


static int net_video_rec1_stop(u8 close)
{
    union video_req req = {0};
    int err;

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    net_video_rec_stop_isp_scenes();
    if (__this_net->net_video_rec1) {
        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }
    }
    if (close) {
        if (__this_net->net_video_rec1) {
            server_close(__this_net->net_video_rec1);
            __this_net->net_video_rec1 = NULL;
        }
    }
    net_video_rec_start_isp_scenes();
    __this_net->net_state1 = VIDREC_STA_STOP;
    __this_net->video_id = 0;
#endif

    return 0;
}

static void net_video_rec_free_buf(char is_free)
{
    if (is_free) {
        if (__this_net->net_v0_fbuf && !__this_net->vbuf_share) {
            free(__this_net->net_v0_fbuf);
        }
        if (__this_net->audio_buf && !__this_net->abuf_share) {
            free(__this_net->audio_buf);
        }
        __this_net->net_v0_fbuf = NULL;
        __this_net->audio_buf = NULL;
    } else {
        if (__this_net->vbuf_share) {
            __this_net->net_v0_fbuf = NULL;
            __this_net->vbuf_share = false;
        }
        if (__this_net->abuf_share) {
            __this_net->audio_buf = NULL;
            __this_net->abuf_share = false;
        }
    }
}
static int net_video_rec_malloc_buf(char id)
{
    u32 res = db_select("res");
    int buf_size[] = {VREC0_FBUF_SIZE, VREC1_FBUF_SIZE, VREC2_FBUF_SIZE};
    int net_buf_size[] = {NET_VREC0_FBUF_SIZE, NET_VREC1_FBUF_SIZE};
    if (!__this_net->net_v0_fbuf) {
        if (__this->video_buf[id] && ((__this->state == VIDREC_STA_START && net_rec_pix_w[res] == __this_net->net_videoreq[id ? 1 : 0].rec.width) \
                                      || __this->state != VIDREC_STA_START)) {
            __this_net->vbuf_share = true;
            __this_net->net_v0_fbuf = __this->video_buf[id];
            __this_net->net_v0_fbuf_size = buf_size[id];
        } else {
            __this_net->vbuf_share = false;
            __this_net->net_v0_fbuf_size = net_buf_size[id ? 1 : 0];
            __this_net->net_v0_fbuf = malloc(__this_net->net_v0_fbuf_size);
        }
        if (!__this_net->net_v0_fbuf) {
            puts("malloc v0_buf err\n\n");
            return -ENOMEM;
        }
    }
    if (!__this_net->audio_buf) {
        if (__this->audio_buf) {
            __this_net->abuf_share = true;
            __this_net->audio_buf = __this->audio_buf;
            __this_net->audio_buf_size = AUDIO_BUF_SIZE;
        } else {
            __this_net->abuf_share = false;
            __this_net->audio_buf_size = NET_AUDIO_BUF_SIZE;
            __this_net->audio_buf = malloc(__this_net->audio_buf_size);
        }
        if (!__this_net->audio_buf) {
            free(__this_net->net_v0_fbuf);
            return -ENOMEM;
        }
    }
    return 0;
}

static int net_video0_rec_start(struct intent *it)
{

    int err = 0;
#ifdef CONFIG_VIDEO0_ENABLE


    if (__this_net->net_video0_art_on || __this_net->net_video0_vrt_on) {
        __this_net->net_state = VIDREC_STA_STARTING;
        err = net_video_rec0_start();

        if (err) {
            goto __start_err0;
        }
        if (__this->state != VIDREC_STA_START) {
            __this_net->videoram_mark = 1;
        } else {
            __this_net->videoram_mark = 0;
        }
        __this_net->net_state = VIDREC_STA_START;
        net_video_rec_start_isp_scenes();
    }

    return 0;

__start_err0:
    puts("\nstart0 err\n");
    err = net_video_rec0_stop(0);

    if (err) {
        printf("\nstart wrong0 %x\n", err);
        //while(1);
    }

#endif
    return err;
}


static int net_video1_rec_start(struct intent *it)
{
    int err = 0;

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    if (__this_net->net_video1_art_on || __this_net->net_video1_vrt_on) {
        if (__this->video_online[1] || __this->video_online[2]) {
            __this_net->net_state1 = VIDREC_STA_STARTING;
            err = net_video_rec1_start();

            if (err) {
                goto __start_err1;
            }

            __this_net->net_state1 = VIDREC_STA_START;
            net_video_rec_start_isp_scenes();
        }
    }

    return 0;

__start_err1:
    puts("\nstart1 err\n");
    err = net_video_rec1_stop(0);

    if (err) {
        printf("\nstart wrong1 %x\n", err);
        //while(1);
    }

    /* return 1; */
#endif

    return err;
}

static int net_video_rec_start(u8 mark)
{
    int err;
    if (!__this_net->is_open) {
        return 0;
    }

    puts("start net rec\n");

#ifdef CONFIG_VIDEO0_ENABLE

    printf("\n art %d, vrt %d\n", __this_net->net_video0_art_on, __this_net->net_video0_vrt_on);

    if ((__this_net->net_video0_art_on || __this_net->net_video0_vrt_on)
        && (__this_net->net_state != VIDREC_STA_START)) {
        puts("\nnet video rec0 start \n");
        err = net_video_rec0_start();
        if (err) {
            goto __start_err0;
        }
        __this_net->net_state = VIDREC_STA_START;
    }

#endif

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)

    if ((__this_net->net_video1_art_on || __this_net->net_video1_vrt_on)
        && (__this_net->net_state1 != VIDREC_STA_START))	{
        if (__this->video_online[1] || __this->video_online[2]) {
            err = net_video_rec1_start();
            if (err) {
                goto __start_err1;
            }
            __this_net->net_state1 = VIDREC_STA_START;
        }
    }

#endif

    net_video_rec_start_isp_scenes();

    return 0;

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
__start_err1:
    puts("\nstart1 err\n");
    err = net_video_rec1_stop(0);

    if (err) {
        printf("\nstart wrong1 %x\n", err);
    }

#endif

#ifdef CONFIG_VIDEO0_ENABLE
__start_err0:
    puts("\nstart0 err\n");
    err = net_video_rec0_stop(0);

    if (err) {
        printf("\nstart wrong0 %x\n", err);
    }

#endif

    return -EFAULT;
}

static int net_video0_rec_rt_mute()
{
    int err;

    if (__this_net->net_state != VIDREC_STA_START) {
        return 0;
    }

    union video_req req = {0};
    req.rec.channel = 1;
    req.rec.state 	=  VIDEO_STATE_PKG_NETVRT_MUTE;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;
    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
    req.rec.state 	=  VIDEO_STATE_PKG_NETART_MUTE;
    req.rec.net_par.net_audrt_onoff = __this_net->net_video0_art_on ;
    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
    return err;
}


static int net_video1_rec_rt_mute()
{
    int err;

    if (__this_net->net_state1 != VIDREC_STA_START) {
        return 0;
    }

    union video_req req = {0};
    req.rec.channel = 1;
    req.rec.state 	=  VIDEO_STATE_PKG_NETVRT_MUTE;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;
    err =	server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);
    req.rec.state 	=  VIDEO_STATE_PKG_NETART_MUTE;
    req.rec.net_par.net_audrt_onoff = __this_net->net_video1_art_on ;
    err = server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);
    return err;
}

static int net_video_rec_aud_mute()
{

    if (__this_net->net_state != VIDREC_STA_START) {
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
    net_video_rec0_aud_mute();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    net_video_rec1_aud_mute();
#endif
    return 0;
}


static int net_video0_rec_stop(u8 close)
{
    int err;
#ifdef CONFIG_VIDEO0_ENABLE
    __this_net->net_state = VIDREC_STA_STOPING;
    err = net_video_rec0_stop(close);

    if (err) {
        puts("\n net stop0 err\n");
    }

    __this_net->net_state = VIDREC_STA_STOP;
#endif

    return err;
}

static int net_video_rec_stop(u8 close)
{
    if (!__this_net->is_open) {
        return 0;
    }

    int err = 0;

#ifdef CONFIG_VIDEO0_ENABLE
    puts("\n net_video_rec_stop. 0.. \n");
#if !(defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    if (__this_net->videoram_mark != 1 && close == 0) {
        puts("\n video ram mark\n");
        return 0;
    }
#endif

    if (__this_net->net_state == VIDREC_STA_START) {
        __this_net->net_state = VIDREC_STA_STOPING;
        err = net_video_rec0_stop(close);

        if (err) {
            puts("\n net stop0 err\n");
        }
        __this_net->net_state = VIDREC_STA_STOP;
        printf("-----%s state stop \n", __func__);
    } else if (__this_net->net_state_ch2 == VIDREC_STA_START) {
        err = net_video_rec0_ch2_stop(close);
        if (err) {
            puts("\n net stop0 err\n");
        }
        __this_net->net_state_ch2 = VIDREC_STA_STOP;
        printf("-----%s state_ch2 stop \n", __func__);
    }

#endif

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    if (__this_net->net_state1 == VIDREC_STA_START) {
        puts("\n net video rec 1 stop\n");
        err = net_video_rec1_stop(close);
        if (err) {
            puts("\n net stop1 err\n");
        }
        printf("-----%s state1 stop \n", __func__);
    } else if (__this_net->net_state1_ch2 == VIDREC_STA_START) {
        err = net_video_rec1_ch2_stop(close);
        if (err) {
            puts("\n net stop1 err\n");
        }
        printf("-----%s state1_ch2 stop \n", __func__);
    }
#endif
    net_video_rec_free_buf(0);
    return err;
}

static int net_video_rec0_ch2_start()
{
    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;
    int buf_size[] = {VREC0_FBUF_SIZE, VREC1_FBUF_SIZE, VREC2_FBUF_SIZE};

    puts("start_video_rec ch2\n");
#ifdef CONFIG_VIDEO5_ENABLE
    __this_net->net_video_rec = server_open("video_server", "video5.0");
    if (!__this_net->net_video_rec) {
        return VREC_ERR_V0_SERVER_OPEN;
    }
    server_register_event_handler(__this_net->net_video_rec, (void *)0, net_rec_dev_server_event_handler);
#else
    __this_net->net_video_rec = __this->video_rec0;
#endif
    __this_net->net_state_ch2 = VIDREC_STA_STARTING;
    __this_net->video_id	  = 0;

    u32 res = db_select("res");
    /*
     *通道号，分辨率，封装格式，写卡的路径
     */
    req.rec.channel     = 1;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.width 		= net_rec_pix_w[res];
    req.rec.height 		= net_rec_pix_h[res];
    req.rec.file        = __this->file[0];

    /*
     *帧率为0表示使用摄像头的帧率
     */
    req.rec.quality     = VIDEO_MID_Q;
    req.rec.fps 		= 0;
    req.rec.real_fps 	= net_video_rec_get_fps();;

    /*
     *采样率，通道数，录像音量，音频使用的循环BUF,录不录声音
     */
    req.rec.audio.sample_rate = VIDEO_REC_AUDIO_SAMPLE_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = AUDIO_VOLUME;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;

    if (res == 0) {
        req.rec.pkg_mute.aud_mute = 0 ;
    } else {
        req.rec.pkg_mute.aud_mute = 1 ;
    }
    /* req.rec.pkg_mute.aud_mute = !db_select("mic"); */

    /*
     *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数
     */
    req.rec.abr_kbps = net_video_rec_get_abr(req.rec.width);

    req.rec.IP_interval = 0;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roio_ratio1 = 256 * 90 / 100;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;


    /*
     * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     */
    text_osd.font_w = 16;
    text_osd.font_h = 32;
    max_one_line_strnum = strlen(video_rec_osd_buf);//21;

    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }
    osd_max_heigh = (req.rec.height == 1088) ? 1080 : req.rec.height ;
    text_osd.x = 0;//(req.rec.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (osd_max_heigh - text_osd.font_h * osd_line_num) / 16 * 16;
    text_osd.osd_yuv = 0xe20095;
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
    text_osd.font_matrix_base = osd_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_str_matrix);
#ifdef __CPU_AC521x__
    text_osd.direction = 1;
#else
    text_osd.direction = 0;
#endif

#ifdef CONFIG_OSD_LOGO
    graph_osd.bit_mode = 16;//2bit的osd需要配置3个color
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = icon_osd_buf;
    graph_osd.icon_size = sizeof(icon_osd_buf);
#endif
    req.rec.text_osd = NULL;
    req.rec.graph_osd = NULL;
    if (db_select("dat")) {
        req.rec.text_osd = &text_osd;
#ifdef CONFIG_OSD_LOGO
        req.rec.graph_osd = &graph_osd;
#endif
    }

    /*
     *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
     */
    req.rec.slow_motion = 0;
    req.rec.tlp_time = db_select("gap");

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
#ifdef CONFIG_PSRAM_ENABLE
    req.rec.extbuf_dev.enable = 1;
    req.rec.extbuf_dev.name = "spiram";
    req.rec.extbuf_dev.size = __PSRAM_SIZE__ / 2;
    req.rec.extbuf_dev.addr = 0;
#endif
#ifdef CONFIG_VIDEO5_ENABLE
    if (!__this_net->net_v0_fbuf) {
        __this_net->net_v0_fbuf = malloc(NET_VREC0_FBUF_SIZE);
        if (!__this_net->net_v0_fbuf) {
            printf("__this_net->net_v0_fbuf\n");
            return -ENOMEM;
        }
    }
    __this_net->net_v0_fbuf_size = NET_VREC0_FBUF_SIZE;
    req.rec.buf = __this_net->net_v0_fbuf;
    req.rec.buf_len = __this_net->net_v0_fbuf_size;
#else
    req.rec.buf = __this->video_buf[__this_net->video_id];
    req.rec.buf_len = buf_size[__this_net->video_id];
#endif
    req.rec.rec_small_pic 	= 0;
    /*
     *循环录像时间，文件大小
     */
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
    req.rec.format 	= NET_VIDEO_FMT_AVI;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video0_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;

    struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
    if (!addr) {
        addr = cdp_srv_get_cli_addr(__this_net->priv);
    }

#if (defined CONFIG_NET_UDP_ENABLE)
    sprintf(req.rec.net_par.netpath, "udp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _FORWARD_PORT);
#elif (defined CONFIG_NET_TCP_ENABLE)
    sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
            , inet_ntoa(addr->sin_addr.s_addr)
            , _FORWARD_PORT);
#elif (defined CONFIG_NET_USR_ENABLE)
    sprintf(req.rec.net_par.netpath, "usr://%s", NET_USR_PATH);
#endif


    printf(">> %s \n", req.rec.net_par.netpath);
    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }
    __this_net->net_state_ch2 = VIDREC_STA_START;
    return 0;
}
static int net_video_rec0_netrt_mute()   //
{
    char buf[128];
    int ret = 0;

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    u32 res = db_select("res");
    if (__this_net->net_state_ch2 != VIDREC_STA_START) {
        ret = net_video_rec0_ch2_start();
        if (ret) {
            printf("net_video_rec0_ch2_start err \n");
            return ret;
        }
    }

    union video_req req = {0};
    req.rec.channel = 1;
    req.rec.state 	=  VIDEO_STATE_PKG_NETVRT_MUTE;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;
    ret = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
    req.rec.state 	=  VIDEO_STATE_PKG_NETART_MUTE;
    req.rec.net_par.net_audrt_onoff = __this_net->net_video0_art_on ;
    ret = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
    puts("\nvideo rec0 netrt mute \n");
    return ret;
}
static int net_video_rec1_netrt_mute()   //
{
    int ret;
    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    if (__this_net->net_state1_ch2 != VIDREC_STA_START) {
        ret = net_video_rec1_ch2_start();
        if (ret) {
            printf("net_video_rec1_ch2_start err !!!\n");
            return ret;
        }
    }

    union video_req req = {0};
    req.rec.channel = 1;
    req.rec.state   = VIDEO_STATE_PKG_NETVRT_MUTE;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;
    ret = server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);
    req.rec.channel = 1;
    req.rec.state   = VIDEO_STATE_PKG_NETART_MUTE;
    req.rec.net_par.net_audrt_onoff = __this_net->net_video1_art_on ;
    ret = server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);
    __this_net->net_state1_ch2 = VIDREC_STA_START;
    printf("video_rec1 ch2 start out\n\n");

    return ret;
}

static int  net_rt_video0_open(struct intent *it)
{
    if (!__this_net->is_open) {
        return 0;
    }

    puts("\nnet rt video0 open \n");
#ifdef CONFIG_VIDEO0_ENABLE
    int ret;

    if (it) {
        u8 mark = *((u8 *)it->data);

        if (mark == 0) {
            __this_net->net_video0_art_on = 1;
            __this_net->net_video0_vrt_on = 0 ;
        } else if (mark == 1) {
            __this_net->net_video0_vrt_on = 1;
            __this_net->net_video0_art_on = 0;
        } else {
            __this_net->net_video0_art_on = 1;
            __this_net->net_video0_vrt_on = 1;
        }

        rt_stream_cmd_analysis(0, it->exdata);
    } else {
        if (__this_net->net_video0_art_on == 0 && __this_net->net_video0_vrt_on == 0) {
            puts("\n rt not open\n");
            return 0;
        }
        if (__this_net->net_state == VIDREC_STA_START) {
            puts("\n net rt is on \n");
            return 0;
        }
    }


    u32 res = db_select("res");
    u32 sdram_size = __SDRAM_SIZE__;

    if ((__this->state == VIDREC_STA_START && res == 0 && __this_net->net_videoreq[0].rec.width > 640)
        || (__this->state == VIDREC_STA_START && (dev_online("uvc") || dev_online("video1.*")))
        || (__this->state == VIDREC_STA_START && sdram_size <= (2 * 1024 * 1024))) { //模拟后拉需要也检测
        net_video_rec_free_buf(1);
        ret = net_video_rec0_netrt_mute();
    } else {
        if (__this_net->net_state == VIDREC_STA_START) {
            ret = net_video0_rec_rt_mute();
        } else {
            ret = net_video0_rec_start(it);
        }
    }
    return ret;
#else
    return 0;
#endif

}


static int  net_rt_video0_stop(struct intent *it)
{
    int ret = 0;
    puts("\nnet rt video0 stop \n");
    if (!__this_net->is_open) {
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
    u8 mark = *((u8 *)it->data);

    if (mark == 0) {
        __this_net->net_video0_art_on = 0;
    } else if (mark == 1) {
        __this_net->net_video0_vrt_on = 0;
    } else {
        __this_net->net_video0_art_on = 0;
        __this_net->net_video0_vrt_on = 0;
    }

    u32 res = db_select("res");

    if (__this_net->net_state_ch2 == VIDREC_STA_START) {
        ret = net_video_rec0_ch2_stop(0);
    } else {
        if (__this_net->net_state == VIDREC_STA_START) {
            ret = net_video0_rec_stop(1);
        }
    }

#endif
    net_video_rec_free_buf(0);
    return ret;
}

static int net_rt_video1_open(struct intent *it)
{
    if (!__this_net->is_open) {
        return 0;
    }

    puts("\n net rt video1 open\n");
    int ret;

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    if (it) {
        u8 mark = *((u8 *)it->data);

        if (mark == 0) {
            __this_net->net_video1_art_on = 1;
            __this_net->net_video1_vrt_on = 0;
        } else if (mark == 1) {
            __this_net->net_video1_art_on = 0;
            __this_net->net_video1_vrt_on = 1;
        } else {
            __this_net->net_video1_art_on = 1;
            __this_net->net_video1_vrt_on = 1;
        }

        rt_stream_cmd_analysis(1, it->exdata);
    } else {
        if (__this_net->net_video1_art_on == 0 && __this_net->net_video1_vrt_on == 0) {
            puts("\nvideo1 rt not open \n");
            return 0;
        }
        if (__this_net->net_state1 == VIDREC_STA_START) {
            puts("\nvideo1 rt is open \n");
            return 0;
        }
    }

    if (__this->state == VIDREC_STA_START) {
        net_video_rec_free_buf(1);
        ret = net_video_rec1_netrt_mute();
    } else {
        if (__this_net->net_state1 == VIDREC_STA_START) {
            ret = net_video1_rec_rt_mute();
        } else {
            ret = net_video1_rec_start(it);
        }
    }

#endif
    if (!ret) {
        printf("net_rt_video1_open ok \n\n");
    } else {
        printf("net_rt_video1_open err \n\n");
    }
    return ret;
}
static int net_video_rec1_ch2_stop(u8 close)
{
    union video_req req = {0};
    int err;

    if (__this_net->net_state1_ch2 == VIDREC_STA_START) {
        __this_net->net_state1_ch2 = VIDREC_STA_STOPING;
        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this_net->net_video_rec1, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec2 ch2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }
        __this_net->net_state1_ch2 = VIDREC_STA_STOP;
        __this_net->net_video_rec1 = NULL;
        __this_net->video_id = 0;
    }
    return  0;
}

static int  net_rt_video1_stop(struct intent *it)
{
    if (!__this_net->is_open) {
        return 0;
    }

    int ret = 0;

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    u8 mark = *((u8 *)it->data);

    if (mark == 0) {
        __this_net->net_video1_art_on = 0;
    } else if (mark == 1) {
        __this_net->net_video1_vrt_on = 0;
    } else {
        __this_net->net_video1_art_on = 0;
        __this_net->net_video1_vrt_on = 0;
    }

    if (__this_net->net_state1_ch2 == VIDREC_STA_START) {
        ret = net_video_rec1_ch2_stop(0);
        printf("net_video_rec1_ch2_stop \n\n");
    } else {
        if (__this_net->net_state1 == VIDREC_STA_START) {
            ret = net_video_rec1_stop(1);
            printf("net_video_rec1_stop \n\n");
        }
    }

#endif
    net_video_rec_free_buf(0);
    return  ret;
}

void  net_video_rec0_close()
{
    if (__this_net->net_video_rec) {
        server_close(__this_net->net_video_rec);
        __this_net->net_video_rec = NULL;
    }

}
void  net_video_rec1_close()
{
    if (__this_net->net_video_rec1) {
        server_close(__this_net->net_video_rec1);
        __this_net->net_video_rec1 = NULL;
    }
}

static int net_video_rec_close()
{
#ifdef CONFIG_VIDEO0_ENABLE
    net_video_rec0_close();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    net_video_rec1_close();
#endif

    return 0;
}


/*
 * 录像app的录像控制入口, 根据当前状态调用相应的函数
 */
static int net_video_rec_control(void *_run_cmd)
{
    int err = 0;
    u32 clust;
    int run_cmd = (int)_run_cmd;
    struct vfs_partition *part;
    if (storage_device_ready() == 0) {
        if (!dev_online(SDX_DEV)) {
            net_video_rec_post_msg("noCard");
        } else {
            net_video_rec_post_msg("fsErr");
        }
        CTP_CMD_COMBINED(NULL, CTP_SDCARD, "VIDEO_CTRL", "NOTIFY", CTP_SDCARD_MSG);
        return 0;
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);

        __this->total_size = part->total_size;

        if (part->clust_size < 32 || (part->fs_attr & F_ATTR_RO)) {
            net_video_rec_post_msg("fsErr");
            CTP_CMD_COMBINED(NULL, CTP_SDCARD, "VIDEO_CTRL", "NOTIFY", CTP_SDCARD_MSG);
            return 0;
        }
    }
    switch (__this->state) {
    case VIDREC_STA_IDLE:
    case VIDREC_STA_STOP:
        if (run_cmd) {
            break;
        }
        __this_net->video_rec_err = FALSE;//用在录像IMC打不开情况下
        printf("--NET_VIDEO_STOP\n");
        NET_VIDEO_ERR(strm_video_rec_close(1));
        NET_VIDEO_ERR(net_video_rec_stop(1));
        __this_net->fbuf_fcnt = 0;
        __this_net->fbuf_ffil = 0;
        err = video_rec_control_start();
        if (err == 0) {
            /* net_video_rec_post_msg("onREC"); */
            if (__this->gsen_lock == 1) {
                net_video_rec_post_msg("lockREC");
            }
        }
        NET_VIDEO_ERR(err);

        NET_VIDEO_ERR(strm_video_rec_open());
        NET_VIDEO_ERR(net_rt_video0_open(NULL));
        NET_VIDEO_ERR(net_rt_video1_open(NULL));

        net_video_rec_status_notify();
        printf("--NET_VIDEO_OPEN OK\n");

        break;
    case VIDREC_STA_START:
        if (run_cmd == 0) {
            printf("---------------- S\n");
            NET_VIDEO_ERR(strm_video_rec_close(1));
            printf("--strm_video_rec_close S\n");
            NET_VIDEO_ERR(net_video_rec_stop(1));
            __this_net->fbuf_fcnt = 0;
            __this_net->fbuf_ffil = 0;

            printf("--NET_VIDEO_ERR S\n");

            err = video_rec_control_doing();
            NET_VIDEO_ERR(err);

            NET_VIDEO_ERR(strm_video_rec_open());
            NET_VIDEO_ERR(net_video_rec_start(1));
            printf("--NET_VIDEO_OPEN S OK\n");
        }
        printf("/**** VIDREC_STA_START %d\n", __LINE__);
        net_video_rec_status_notify();
        printf("/**** VIDREC_STA_START %d\n", __LINE__);
        break;
    default:
        puts("\nvrec forbid\n");
        err = 1;
        break;
    }

    return err;
}

/*
 *录像的状态机,进入录像app后就是跑这里
 */
static int net_video_rec_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int err = 0;
    int len;
    char buf[128];

    switch (state) {
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_VIDEO_DEC_MAIN:
            /*FILE_LIST_TASK_INIT();*/
            break;
        case ACTION_VIDEO_TAKE_PHOTO:
            /*printf("----ACTION_VIDEO_TAKE_PHOTO----\n\n");*/
            net_video_rec_take_photo(NULL);
            break;
        case ACTION_VIDEO_REC_CONCTRL:
            printf("----ACTION_VIDEO_REC_CONCTRL-----\n\n");
            err = net_video_rec_control(0);
            break;

        case ACTION_VIDEO_REC_GET_APP_STATUS:
            printf("----ACTION_VIDEO_REC_GET_APP_STATUS-----\n\n");
            video_rec_get_app_status(it);
            break;

        case ACTION_VIDEO_REC_GET_PATH:
            /*printf("----ACTION_VIDEO_REC_GET_PATHL-----\n\n");*/
            video_rec_get_path(it);
        case ACTION_VIDEO0_OPEN_RT_STREAM:

            printf("----ACTION_VIDEO0_OPEN_RT_STREAM-----\n\n");
            __this_net->is_open = TRUE;
            __this_net->fbuf_fcnt = 0;
            __this_net->fbuf_ffil = 0;
            err = net_rt_video0_open(it);
            sprintf(buf, "format:%d,w:%d,h:%d,fps:%d,rate:%d"
                    , __this_net->net_videoreq[0].rec.format
                    , __this_net->net_videoreq[0].rec.width
                    , __this_net->net_videoreq[0].rec.height
                    , __this_net->net_videoreq[0].rec.real_fps
                    , VIDEO_REC_AUDIO_SAMPLE_RATE);
            printf("<<<<<<< : %s\n\n\n\n\n", buf);
            if (err) {
                printf("ACTION_VIDEO0_OPEN_RT_STREAM err!!!\n\n");
                CTP_CMD_COMBINED(NULL, CTP_RT_OPEN_FAIL, "OPEN_RT_STREAM", "NOTIFY", CTP_RT_OPEN_FAIL_MSG);
            } else {
                if (CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "OPEN_RT_STREAM", "NOTIFY", buf)) {
                    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "OPEN_RT_STREAM", "NOTIFY", buf);
                }
                printf("CTP NOTIFY VIDEO0 OK\n\n");
            }
            /*app_video_rec_test();*/
            break;

        case ACTION_VIDEO1_OPEN_RT_STREAM:
            printf("----ACTION_VIDEO1_OPEN_RT_STREAM----\n\n");
            __this_net->is_open = TRUE;
            __this_net->fbuf_fcnt = 0;
            __this_net->fbuf_ffil = 0;
            err = net_rt_video1_open(it);
            sprintf(buf, "format:%d,w:%d,h:%d,fps:%d,rate:%d"
                    , __this_net->net_videoreq[1].rec.format
                    , __this_net->net_videoreq[1].rec.width
                    , __this_net->net_videoreq[1].rec.height
                    , __this_net->net_videoreq[1].rec.real_fps
                    , VIDEO_REC_AUDIO_SAMPLE_RATE);
            printf("<<<<<<< : %s\n\n\n\n\n", buf);
            if (err) {
                printf("ACTION_VIDEO1_OPEN_RT_STREAM err!!!\n\n");
                CTP_CMD_COMBINED(NULL, CTP_RT_OPEN_FAIL, "OPEN_PULL_RT_STREAM", "NOTIFY", CTP_RT_OPEN_FAIL_MSG);
            } else {
                CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "OPEN_PULL_RT_STREAM", "NOTIFY", buf);
                printf("CTP NOTIFY VIDEO1 OK\n\n");
            }
            /*app_video_rec_test();*/
            break;

        case ACTION_VIDEO0_CLOSE_RT_STREAM:
            printf("---ACTION_VIDEO0_CLOSE_RT_STREAM---\n\n");
            err = net_rt_video0_stop(it);
            if (err) {
                printf("ACTION_VIDEO_CLOE_RT_STREAM err!!!\n\n");
                strcpy(buf, "status:0");
            } else {
                strcpy(buf, "status:1");
            }
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "CLOSE_RT_STREAM", "NOTIFY", buf);
            printf("CTP NOTIFY VIDEO0 OK\n\n");
            __this_net->is_open = FALSE;
            __this_net->fbuf_fcnt = 0;
            __this_net->fbuf_ffil = 0;
            /*app_video_rec_test();*/
            break;

        case ACTION_VIDEO1_CLOSE_RT_STREAM:
            printf("---ACTION_VIDEO1_CLOSE_RT_STREAM---\n\n");
            err =  net_rt_video1_stop(it);
            if (err) {
                printf("ACTION_VIDE1_CLOE_RT_STREAM err!!!\n\n");
                strcpy(buf, "status:0");
            } else {
                strcpy(buf, "status:1");
            }

            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "CLOSE_PULL_RT_STREAM", "NOTIFY", buf);
            printf("CTP NOTIFY VIDEO1 OK\n\n");
            __this_net->is_open = FALSE;
            __this_net->fbuf_fcnt = 0;
            __this_net->fbuf_ffil = 0;
            /*app_video_rec_test();*/
            break;
        case ACTION_VIDEO_CYC_SAVEFILE:
#if 0
            video_cyc_file(0);
#if defined CONFIG_VIDEO1_ENABLE
            video_cyc_file(1);
#endif
#if defined CONFIG_VIDEO2_ENABLE
            video_cyc_file(2);
#endif
#endif
            strcpy(buf, "status:1");
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CYC_SAVEFILE", "NOTIFY", buf);
            break;

#ifdef CONFIG_NET_SCR
        case ACTION_NET_SCR_REC_OPEN:
            printf("%s ACTION_NET_SCR_REC_OPEN\n", __func__);
            extern int net_video_disp_stop(int id);
            extern int net_hide_main_ui(void);
            net_video_disp_stop(0);
            net_hide_main_ui();
            break;

        case ACTION_NET_SCR_REC_CLOSE:
            printf("%s ACTION_NET_SCR_REC_CLOSE\n", __func__);
            extern int net_video_disp_start(int id);
            extern int net_show_main_ui(void);
            net_video_disp_start(0);
            net_show_main_ui();
            break;
#endif
#ifdef CONFIG_RTSP_CLIENT_ENABLE
        case ACTION_NET_RTSP_REC_OPEN:
            printf("%s ACTION_NET_RTSP_REC_OPEN\n", __func__);
            extern int net_video_disp_stop(int id);
            extern int net_hide_main_ui(void);
            net_video_disp_stop(0);
            net_hide_main_ui();
            break;

        case ACTION_NET_RTSP_REC_CLOSE:
            printf("%s ACTION_NET_RTSP_REC_CLOSE\n", __func__);
            extern int net_video_disp_start(int id);
            extern int net_show_main_ui(void);
            net_video_disp_start(0);
            net_show_main_ui();
            break;
#endif
        }
        break;
    }

    return err;
}

static void net_video_rec_ioctl(u32 argv)
{
    char buf[128];
    u32 *pargv = (u32 *)argv;
    u32 type = (u32)pargv[0];
    char *path = (char *)pargv[1];

    /*printf("%s type : %d , %s \n\n", __func__, type, path);*/
    switch (type) {
    case NET_VIDREC_STA_STOP:
        if (__this_net->net_state == VIDREC_STA_START || __this_net->net_state1 == VIDREC_STA_START \
            || __this_net->net_state_ch2 == VIDREC_STA_START || __this_net->net_state1_ch2 == VIDREC_STA_START) {
            net_video_rec_stop(1);
        }
        if (__this_strm->state == VIDREC_STA_START || __this_strm->state_ch2 == VIDREC_STA_START) {
            strm_video_rec_close(1);
        }
        __this_net->is_open = FALSE;
        net_video_rec_free_buf(1);
        break;
    case NET_VIDREC_STATE_NOTIFY:
        sprintf(buf, "status:%d", ((__this->state == VIDREC_STA_START) ? 1 : 0));
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CTRL", "NOTIFY", buf);
        break;
    default :
        return ;
    }
}

static int net_video_rec_device_event_handler(struct sys_event *event)
{
    int err;
    struct intent it;

    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            video_rec_sd_in_notify();
            break;
        case DEVICE_EVENT_OUT:
            if (!fdir_exist(CONFIG_STORAGE_PATH)) {
                video_rec_sd_out_notify();
            }
            break;
        }
    } else if (!ASCII_StrCmp(event->arg, "sys_power", 7)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_POWER_CHARGER_IN:
            puts("---charger in\n\n");
            if ((__this->state == VIDREC_STA_IDLE) ||
                (__this->state == VIDREC_STA_STOP)) {
                video_rec_control_start();
            }
            break;
        case DEVICE_EVENT_POWER_CHARGER_OUT:
            puts("---charger out\n");
            if (__this->state == VIDREC_STA_START) {
                video_rec_control_doing();
            }
            break;
        }
    }
    return 0;
}
static void net_video_server_task(void *p)
{
    int res;
    int msg[16];

    if (os_mutex_create(&net_vdrec_mutex) != OS_NO_ERR) {
        printf("net_video_server_task , os_mutex_create err !!!\n\n");
        return;
    }
    net_video_handler_init();
    printf("net_video_server_task running\n\n");

    while (1) {

        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (os_task_del_req(OS_TASK_SELF) == OS_TASK_DEL_REQ) {
            os_task_del_res(OS_TASK_SELF);
        }

        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_EVENT:
                break;
            case Q_MSG:
                net_video_rec_ioctl((u32)msg[1]);
                break;
            default:
                break;
            }
            break;
        case OS_TIMER:
            break;
        case OS_TIMEOUT:
            break;
        }
    }
    os_mutex_del(&net_vdrec_mutex, OS_DEL_ALWAYS);
}

void net_video_server_init(void)
{
    task_create(net_video_server_task, 0, "net_video_server");
}
__initcall(net_video_server_init);

int cmd_video_rec_ctl(char start);
static int net_video_rec_key_event_handler(struct key_event *key)
{
    int err = 0;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_UP:
            cmd_video_rec_ctl(1);
            printf("--->  %s \n", __func__);
            break;
        case KEY_DOWN:
            cmd_video_rec_ctl(0);
            printf("--->  %s \n", __func__);
            break;
        }
        break;
    default:
        break;
    }
    return err;
}

/*录像app的事件总入口*/
static int net_video_rec_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_DEVICE_EVENT:
        return video_rec_device_event_action(event);//设备事件和 vidoe_rec公用一个handler，
    //case SYS_KEY_EVENT:
    //return net_video_rec_key_event_handler(&event->u.key);
    default:
        return false;
    }
}

static const struct application_operation net_video_rec_ops = {
    .state_machine  = net_video_rec_state_machine,
    .event_handler 	= net_video_rec_event_handler,
};

REGISTER_APPLICATION(app_video_rec) = {
    .name 	= "net_video_rec",
    .action	= ACTION_VIDEO_REC_MAIN,
    .ops 	= &net_video_rec_ops,
    .state  = APP_STA_DESTROY,
};


int CTP_CMD_COMBINED(void *priv, u32 err, const char *_req, const char *mothod, char *str)   //第一参数：topic
{
    return 0;
}

int ctp_cmd_analysis(const char *topic, char *content, void *priv)
{
    return 0;
}


void video_rec_set_osd(struct video_text_osd *text_osd, int width, int height)
{

}

int video0_rec_get_iframe(void)
{
    return 0;
}


int net_video_rec0_take_photo(u8 *buffer, u32 buffer_len)
{

    void *server = NULL;
    union video_req req = {0};
    int err;
    int len = 0;
    int retry;
    while (!get_video2_online_flag()) {
        msleep(100);
        retry++;
        if (retry >= 20) {
            goto exit;
        }
    }

    if (__this->video_rec2 &&  __this->state == VIDREC_STA_START) {
        server =  __this->video_rec2;
    } else {
        server = server_open("video_server", "video2.0");
    }


    if (server) {
        req.icap.width = 320;
        req.icap.height = 240;
        req.icap.buf_size = buffer_len;
        printf("\n req.icap.buf_size = %d\n", req.icap.buf_size);
        req.icap.quality = VIDEO_MID_Q;
        req.icap.buf = buffer;
        req.rec.text_osd = NULL;
        req.rec.graph_osd = NULL;
        req.icap.text_label = NULL;
        req.icap.save_cap_buf = TRUE;//保存到cap_buff写TRUE，数据格式，前4字节为数据长度，4字节后为有效数据
        err = server_request(server, VIDEO_REQ_IMAGE_CAPTURE, &req);
        if (err != 0) {
            printf("\ntake photo err\n");
            goto exit;
        }
        len = *(int *)buffer;
    }



exit:
    if (server) {
//        server_close(server);  //无需关闭，关闭反而有问题
    }
    return len;
}

int net_video_rec_get_adc_volume()
{
    return AUDIO_VOLUME;
}

int net_video_rec_get_dac_volume()
{
    return 0;
}



int ircut_set_isp_scenes(u8 on, char *buf, int size)
{

    void *server = NULL;
    struct ispt_customize_cfg cfg = {0};
    union video_req req = {0};
    int err;

    if (get_uvc_camera_device()) {
        server = get_usb_uvc_hdl();
    } else if (__this_net->net_video_rec) {
        server = __this_net->net_video_rec;
    } else if (__this->video_rec0) {
        server = __this->video_rec0;
    } else if (__this_strm->video_rec0) {
        server = __this_strm->video_rec0;
    }

    if (on) {
#ifdef CONFIG_AUTO_ISP_SCENES_ENABLE
        stop_update_isp_scenes();
#endif
        ispt_params_flush();
        ispt_params_set_smooth_step(1);
        cfg.mode = ISP_CUSTOMIZE_MODE_FILE;
        cfg.data = buf;
        cfg.len =  size;

        req.camera.cmd = SET_CUSTOMIZE_CFG;
        req.camera.customize_cfg = &cfg;
        err = server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
#ifdef CONFIG_OSD_DISPLAY_CUR_SENCE
        extern void osd_display_cur_scenec(int value);
        osd_display_cur_scenec(4);
#endif
    } else {
        stop_update_isp_scenes();
        ispt_params_flush();
        ispt_params_set_smooth_step(1);
        cfg.mode = ISP_CUSTOMIZE_MODE_FILE;
        cfg.data = buf;
        cfg.len =  size;
        req.camera.cmd = SET_CUSTOMIZE_CFG;
        req.camera.customize_cfg = &cfg;
        err = server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
        if (!err) {
#ifdef CONFIG_OSD_DISPLAY_CUR_SENCE
            extern void osd_display_cur_scenec(int value);
            osd_display_cur_scenec(2);
#endif // CONFIG_OSD_DISPLAY_CUR_SENCE
#ifdef CONFIG_AUTO_ISP_SCENES_ENABLE
            start_update_isp_scenes(server);
#endif // CONFIG_AUTO_ISP_SCENES_ENABLE
        }




    }
    return err;
}

u8 get_video_rec_state(void)
{
    rec_handler  = (struct video_rec_hdl *)get_video_rec_handler();
    return __this->state == VIDREC_STA_START;
}
int ircut_get_isp_scenes_flag(void)
{
    if (__this->video_rec0 && __this->state == VIDREC_STA_START && isp_scenes_switch_timer_done()) {

        return 0;
    }
    if (__this_net->net_video_rec && __this_net->net_state == VIDREC_STA_START && isp_scenes_switch_timer_done()) {

        return 0;
    }

    if (__this_strm->video_rec0 && __this_strm->state == VIDREC_STA_START && isp_scenes_switch_timer_done()) {

        return 0;
    }

    if (get_uvc_camera_device() && isp_scenes_switch_timer_done()) {

        return 0;
    }


    return -1;
}

//struct sockaddr_in *ctp_srv_get_cli_addr(void *cli)
//{
//    return 0;
//}
//int ctp_srv_init(u16_t port, int (*cb_func)(void *cli, enum ctp_srv_msg_type type, char *topic, char *content, void *priv), void *priv)
//{
//        return 0;
//}
//void ctp_srv_set_thread_payload_max_len(u32 max_topic_len, u32 max_content_slice_len)
//{
//
//}
//int ctp_srv_keep_alive_en(const char *recv, const char *send, const char *parm)
//{
//    return 0;
//}
//
//void ctp_cli_uninit(void)
//{
//
//}
//
//
//int cdp_srv_send(void *cli, char *topic, char *content)
//{
//    return 0;
//}
