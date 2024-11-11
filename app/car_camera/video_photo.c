/*************************************************************************
    > File Name: video_photo.c
    > Author:
    > Mail:
    > Created Time: Thu 16 Feb 2017 09:35:08 AM HKT
 ************************************************************************/

#include "system/includes.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "server/ui_server.h"
#include "action.h"
#include "style.h"
#include "app_config.h"
#include "storage_device.h"
#include "video_photo.h"
#include "video_system.h"
#include "vrec_osd.h"
#include "user_isp_cfg.h"
#include "key_voice.h"
#include "app_database.h"
#include "asm/lcd_config.h"
#include "video_rec.h"

#define PHOTO_MODE_UNINIT   0x0
#define PHOTO_MODE_ACTIVE   0x1
#define PHOTO_MODE_STOP     0x2
#define PHOTO_MODE_QSCAN    0x4


#define CAMERA_OFFLINE          0x0
#define CAMERA_ONLINE           0x1
#define CAMERA_BUSY             0x2
#define MAX_PHOTO_NUMBER        4096


#define VIDEO_LARGE_IMAGE       0

extern int x_offset;
extern int y_offset;
struct photo_resolution
{
    u16 width;
    u16 height;
};


struct video_photo_handle tph_handler =
{
    .state = PHOTO_MODE_UNINIT,
};

const static struct photo_resolution camera0_reso[] =
{
    {640,  480},  //                VGA
    {1280, 720},  // {1024, 768}    1M
    {1920, 1088}, // {1600, 1200}   2M
    {2048, 1552}, //                3M
    {2560, 1552}, //                4M
    {2592, 1968}, //                5M
    {3072, 1968}, //                6M
    {3456, 1968}, //                7M
    {3456, 2448}, //                8M
    {3456, 2592}, //                9M
    /* {3648, 2736}, //                10M */
    {4416, 2496}, //                12M
    /* {5120, 2880}, //                16M */
    /* {6016, 4096}, //                24M */
    /* {7680, 4352}, //                32M */
    /* {10240, 5760}, //               64M */
    {4032, 2736}, //                11M
    {4032, 3024}, //                12M
};

enum
{
    CAMERA_RESO_VGA = 0x0,
    CAMERA_RESO_1M,
    CAMERA_RESO_2M,
    CAMERA_RESO_3M,
    CAMERA_RESO_4M,
    CAMERA_RESO_5M,
    CAMERA_RESO_6M,
    CAMERA_RESO_7M,
    CAMERA_RESO_8M,
    CAMERA_RESO_9M,
    CAMERA_RES0_10M,
    CAMERA_RESO_11M,
    CAMERA_RESO_12M,
};

#define __this  (&tph_handler)

//#define CAMERA0_CAP_PATH        CONFIG_REC_PATH"/C/1/"
//#define CAMERA1_CAP_PATH        CONFIG_REC_PATH"/C/2/"

/*
const u16 video_photo_reso_w[] = {800, 1280, 1600, 1920, 2208, 2560, 3072, 3712};
const u16 video_photo_reso_h[] = {600, 720, 912, 1088, 1536, 1936, 2208, 1952};
*/

static int photo_switch_camera(void);
static int video_take_photo(struct photo_camera *camera);
static void camera_close(struct photo_camera *camera);
static int photo_camera_stop_display(void);
static int photo_switch_camera_display(void);
static int photo_camera_display(void);

struct imc_presca_ctl *video_disp_get_pctl();
extern int wbl_set_function_camera0(u32 parm);
extern void face_det_onoff(u8 onoff); // 人脸检测控制 0:暂停 1:开启

#ifdef PHOTO_STICKER_ENABLE

#ifdef CONFIG_UI_ENABLE
static u8 power_on = 0;
void ani_play_end_notifice_to_ph_app(u8 end)
{
    power_on = 1;
}
#endif

static int sys_power_on(void)
{
#ifdef CONFIG_UI_ENABLE
    return power_on;
#else
    return 1;
#endif
}
#endif

void video_photo_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if(__this->ui)
    {
        req.msg.receiver = ID_WINDOW_VIDEO_TPH;
        req.msg.msg = msg;
        req.msg.exdata = argptr;
        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif

}

static u32 photo_db_select(const char *table)
{
    if(__this->camera_id == 0)
    {
        return db_select(table);
    }
    else
    {
        if((strcmp(table, "cyt") == 0) ||
        (strcmp(table, "pdat") == 0) ||
        (strcmp(table, "sca") == 0) ||
        (strcmp(table, "qua") == 0) ||
        (strcmp(table, "phm") == 0))
        {
            return db_select(table);
        }
    }
    return 0;
}


int set_camera0_reso(u8 res, int *width, int *height)
{
    *width = camera0_reso[res].width;
    *height = camera0_reso[res].height;

    return 0;
}


static int set_label_config(u16 image_width, u16 image_height, u32 font_color,
                            struct video_text_osd *label)
{
    if(!label)
    {
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
    if(image_width > 1920)
    {
        label->direction = 0;
        label->font_w = 32;
        label->font_h = 64;
        label->text_format = label_format;
        label->font_matrix_table = (char *)label_index_tbl;
        label->font_matrix_base = (u8 *)label_font_32x64;
        label->font_matrix_len = sizeof(label_font_32x64);

    }
    else
    {
        label->font_w = 16;
        label->font_h = 32;
        label->text_format = label_format;
        label->font_matrix_table = (char *)osd_str_total;
        label->font_matrix_base = (u8 *)osd_str_matrix;
        label->font_matrix_len = sizeof(osd_str_matrix);
    }
    label->osd_yuv = font_color;

    label->x = (image_width - strlen(label_format) * label->font_w) / 64 * 64;
    label->y = (image_height - label->font_h - 16) / 16 * 16;

    if(image_width == 5120 || image_width == 10240)
    {
        label->x = (1280 - strlen(label_format) * label->font_w) / 64 * 64;
        label->y = (720 - label->font_h - 16) / 16 * 16;
    }
    else if(image_width > 5120)
    {
        label->x = (image_width / 8 - strlen(label_format) * label->font_w) / 64 * 64;
        label->y = (image_height / 8 - label->font_h - 16) / 16 * 16;
    }
    else if(image_width == 4416)
    {
        label->x = (image_width / 4 - strlen(label_format) * label->font_w) / 64 * 64;
        label->y = (image_height / 4 - label->font_h - 16) / 16 * 16;
    }

    return 0;
}


#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
u8 stk_name[64];
static int get_sticker()
{
    FILE *fp;
    int size = 0;
    /* __this->sticker_num = 2; */
    /******测试使用文件, 以实际使用为准*******/
    if(__this->sticker_num == 0)
    {
        return -1;
    }
    /* if (__this->sticker_num == __this->prev_sticker) { */
    /* return &__this->sticker; */
    /* } */
    __this->sticker_name  = (u8 *)stk_name;
    sprintf((char *)__this->sticker_name, STICKER_PATH, __this->sticker_num);

    __this->prev_sticker = __this->sticker_num;
    return 0;
}


static void switch_sticker()
{
    union video_req req = {0};
    if(!__this->display)
    {
        return ;
    }

    if(__this->prev_sticker == 0)
    {
        //stop disp
        //start disp
        //stop rec
        //start rec
#ifdef CONFIG_VIDEO0_ENABLE
        get_sticker();
        photo_camera_stop_display();
        photo_camera_display();
#endif
        return;
    }
    else if(__this->sticker_num == 0)
    {
        __this->prev_sticker = __this->sticker_num;
        __this->sticker_name  = NULL;
#ifdef CONFIG_VIDEO0_ENABLE
        photo_camera_stop_display();
        photo_camera_display();
#endif
        return;
    }


    if(!get_sticker())
    {
        if(__this->sticker_name)
        {
            req.display.sticker_name      = __this->sticker_name;
        }

        server_request(__this->display, VIDEO_REQ_STICKER_SWICTH, &req);
    }


}
#endif




#ifdef PHOTO_STICKER_ENABLE
//切换贴纸
static int photo_switch_sticker(int p)
{
    static union video_dec_req dec_req;
    FILE *fp;

    if(p == 0)
    {
        if(dec_req.dec.file)
        {
            if(__this->video_dec)
            {
                server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &dec_req);
            }
            fclose(dec_req.dec.file);
            dec_req.dec.file = NULL;
        }
        if(__this->video_dec)
        {
            server_close(__this->video_dec);
            __this->video_dec = NULL;
        }
        return 0;
    }
    else
    {
        if(dec_req.dec.file)
        {
            if(__this->video_dec)
            {
                server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &dec_req);
            }
            fclose(dec_req.dec.file);
            dec_req.dec.file = NULL;
        }
        if(__this->video_dec)
        {
            server_close(__this->video_dec);
            __this->video_dec = NULL;
        }
        if(!__this->video_dec)
        {
            struct video_dec_arg arg = {0};

            arg.dev_name = "video_dec";
            arg.audio_buf_size = 128;
            arg.video_buf_size = 200 * 1024;

            __this->video_dec = server_open("video_dec_server", &arg);
            if(!__this->video_dec)
            {
                return -EFAULT;
            }
        }
        if(dec_req.dec.file)
        {
            fclose(dec_req.dec.file);
            dec_req.dec.file = NULL;
        }
        /* server_register_event_handler(__this->video_dec, NULL, dec_server_event_handler); */

        char sticker_file_name[128];

        sprintf(sticker_file_name, PHOTO_STICKER_PATH, p);
        printf("sticker file = %s", sticker_file_name);

        fp = fopen(sticker_file_name, "r");
        if(!fp)
        {
            log_e("open sticker file err ");
            return -EFAULT;
        }
        dec_req.dec.file = fp;
        dec_req.dec.fb = "fb0";
        dec_req.dec.left = 0;
        dec_req.dec.top = 0;
        dec_req.dec.width = 0;
        dec_req.dec.height = 0;

        dec_req.dec.pctl = NULL;
        dec_req.dec.thm_first = 0;
        dec_req.dec.preview = 1;
        dec_req.dec.image.buf = 0;
        dec_req.dec.image.size = 0;

        server_request(__this->video_dec, VIDEO_REQ_DEC_START, &dec_req);
    }
    return 0;
}
static int show_photo_sticker(void *arg)
{
    if((int)arg == 0)
    {
        return 0;
    }
    return photo_switch_sticker((int)arg);
}
#endif

/*
 *停止快速预览
 */
static void stop_quick_scan(void *p)
{
    union video_dec_req dec_req;

    if(!(__this->state & PHOTO_MODE_QSCAN))
    {
        return;
    }

    /*
     *删除timeout超时
     */
    if(__this->qscan_timeout)
    {
        sys_timeout_del(__this->qscan_timeout);
        __this->qscan_timeout = 0;
    }

    /*
     *请求停止解码
     */
    if(__this->video_dec)
    {
        server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &dec_req);
    }

    __this->state &= ~PHOTO_MODE_QSCAN;

    if(!(__this->state & PHOTO_MODE_STOP))
    {
        /*
         *恢复摄像头显示
         */
        photo_camera_display();
    }

#ifdef PHOTO_STICKER_ENABLE
    photo_switch_sticker(__this->sticker_num);
#endif
    sys_key_event_enable();
    sys_touch_event_enable();

}


/*
 *快速预览功能
 */
static int photo_quick_scan(struct photo_camera *camera)
{
    int err = 0;
    int timeout;
    union video_req req;
    union video_dec_req dec_req;

    memset(&dec_req, 0x0, sizeof(dec_req));

    /*
     *获取延时秒数
     */
    timeout = photo_db_select("sca") * 1000;
    if(timeout == 0)
    {
        return 0;
    }


    /*
     *获取拍摄的IMAGE
     */
    err = server_request(camera->server, VIDEO_REQ_GET_IMAGE, &req);
    if(!err)
    {

        photo_camera_stop_display();

        if(camera->server)
        {
            server_close(camera->server);
            camera->server = NULL;
        }

        if(!__this->video_dec)
        {
            struct video_dec_arg arg = {0};

            arg.dev_name = "video_dec";
#ifdef PHOTO_STICKER_ENABLE
            arg.audio_buf_size = 128;
            arg.video_buf_size = 200 * 1024;
#else
            arg.audio_buf_size = 512;
            arg.video_buf_size = 512;
#endif
            __this->video_dec = server_open("video_dec_server", (void *)&arg);
            if(!__this->video_dec)
            {
                return -EFAULT;
            }
        }

        /*
         *预览参数设置
         */
        dec_req.dec.fb = "fb1";
        dec_req.dec.left = 0;
        dec_req.dec.top = 0;
        dec_req.dec.width = 0;
        dec_req.dec.height = 0;

        dec_req.dec.pctl = NULL;
        dec_req.dec.thm_first = 1;
        dec_req.dec.preview = 1;
        dec_req.dec.image.buf = req.image.buf;
        dec_req.dec.image.size = req.image.size;
        if(__this->aux_buf)
        {
            dec_req.dec.image.aux_buf = __this->aux_buf;
            dec_req.dec.image.aux_buf_size = IMAGE_AUX_BUF_SIZE;
        }

        /*
         *解码拍摄的IMAGE
         */
        err = server_request_async(__this->video_dec, VIDEO_REQ_DEC_IMAGE, &dec_req);

        __this->state |= PHOTO_MODE_QSCAN;
        __this->qscan_timeout = sys_timeout_add(NULL, stop_quick_scan, timeout);

    }

    return err;
}

#ifdef PHOTO_STICKER_ENABLE
/********************大头贴配置注意事项***********************
 *  1.大头贴文件不支持BMP、PNG，仅支持YUV数据（JPG文件解码为YUV）
 *  2.大头贴文件实际的存放应该在flash里面，防止SD卡格式化丢失
 *  3.AC521x 图像插值最大尺寸宽度为4032
 *  4.一般使用YUV420，比较省内存空间
*************************************************************/
struct image_sticker *get_photo_sticker(void)
{
    FILE *fp;
    int size = 0;
    int y_size = 0;
    u8 *image = NULL;
    struct jpeg_image_info info;
    struct jpeg_decode_req req = {0};
    char sticker_file_name[128];

    /******测试使用文件, 以实际使用为准*******/
    if(__this->sticker_num == 0)
    {
        return NULL;
    }

    if(__this->sticker_num == __this->prev_sticker)
    {
        return &__this->sticker;
    }
    sprintf(sticker_file_name, PHOTO_STICKER_PATH, __this->sticker_num);
    printf("sticker file = %s", sticker_file_name);

    fp = fopen(sticker_file_name, "r");
    if(!fp)
    {
        log_e("open sticker file err ");
        goto __failed;
    }

    size = flen(fp);
    image = __this->cap_buf;//和capture buffer复用，注意:只可在拍照前配置阶段使用
    if(!image)
    {
        log_e("no buffer can be put sticker file\n");
        goto __failed;
    }
    fread(fp, image, size);
    fclose(fp);

    info.input.data.buf = image;
    info.input.data.len = size;
    jpeg_decode_image_info(&info);

    y_size = info.width * info.height;
    if(!__this->sticker.addr || __this->sticker.width != info.width || __this->sticker.height != info.height)
    {
        if(__this->sticker.addr)
        {
            free(__this->sticker.addr);
        }
        __this->sticker.size = y_size * 3 / 2;
        __this->sticker.addr = (u8 *)malloc(__this->sticker.size);
        if(!__this->sticker.addr)
        {
            log_e("malloc err2");
            goto __failed;
        }
        __this->sticker.width = info.width;
        __this->sticker.height = info.height;
        __this->sticker.format = 0;
        __this->sticker.threshold = 16;
    }
    log_d("photo sticker, %d x %d\n", info.width, info.height);

    req.output_type = 0;
    req.input.data.buf = image;
    req.input.data.len = size;
    req.buf_y = __this->sticker.addr;
    req.buf_u = req.buf_y + y_size;
    req.buf_v = req.buf_u + y_size / 4;
    req.buf_width = info.width;
    req.buf_height = info.height;
    req.out_width = info.width;
    req.out_height = info.height;
    jpeg_decode_one_image(&req);

    __this->prev_sticker = __this->sticker_num;
    return &__this->sticker;
__failed:

    if(__this->sticker.addr)
    {
        free(__this->sticker.addr);
        __this->sticker.addr = NULL;
    }
    return NULL;
}

static void scan_power_on_ani_end(void *arg)
{
    if(sys_power_on())
    {
        photo_switch_sticker(__this->sticker_num);
        sys_timeout_del(__this->timer);
        __this->timer = 0;
    }
}
#endif
struct image_sticker *get_photo_sticker_by_path(const char *file_name, int id)
{
    FILE *fp;
    int size = 0;
    int y_size = 0;
    u8 *image = NULL;
    struct jpeg_image_info info;
    struct jpeg_decode_req req = {0};
    char sticker_file_name[128];

    if(file_name)
    {
        memset(sticker_file_name, 0x0, sizeof(sticker_file_name));
        strcpy(sticker_file_name, file_name);
        log_d("sticker file = %s", sticker_file_name);
    }

    fp = fopen(sticker_file_name, "r");
    if(!fp)
    {
        log_e("open sticker file err ");
        goto __failed;
    }

    size = flen(fp);
    image = __this->cap_buf;//和capture buffer复用，注意:只可在拍照前配置阶段使用
    if(!image)
    {
        log_e("no buffer can be put sticker file\n");
        goto __failed;
    }
    fread(fp, image, size);
    fclose(fp);

    info.input.data.buf = image;
    info.input.data.len = size;
    jpeg_decode_image_info(&info);

    y_size = info.width * info.height;
    if(!__this->icon_sticker[id].addr || __this->icon_sticker[id].width != info.width || __this->icon_sticker[id].height != info.height)
    {
        if(__this->icon_sticker[id].addr)
        {
            free(__this->icon_sticker[id].addr);
        }
        __this->icon_sticker[id].size = y_size * 3 / 2;
        __this->icon_sticker[id].addr = (u8 *)malloc(__this->icon_sticker[id].size);
        if(!__this->icon_sticker[id].addr)
        {
            log_e("malloc err2");
            goto __failed;
        }
        __this->icon_sticker[id].width = info.width;
        __this->icon_sticker[id].height = info.height;
        __this->icon_sticker[id].format = 0;
        __this->icon_sticker[id].threshold = 16;
    }
    log_d("icon sticker, %d x %d\n", info.width, info.height);

    req.output_type = 0;
    req.input.data.buf = image;
    req.input.data.len = size;
    req.buf_y = __this->icon_sticker[id].addr;
    req.buf_u = req.buf_y + y_size;
    req.buf_v = req.buf_u + y_size / 4;
    req.buf_width = info.width;
    req.buf_height = info.height;
    req.out_width = info.width;
    req.out_height = info.height;
    jpeg_decode_one_image(&req);

    return &__this->icon_sticker[id];
__failed:

    if(__this->icon_sticker[id].addr)
    {
        free(__this->icon_sticker[id].addr);
        __this->icon_sticker[id].addr = NULL;
    }
    return NULL;
}
/*
 *关闭camera
 */
static void camera_close(struct photo_camera *camera)
{
    if(camera->server)
    {
        server_close(camera->server);
        camera->server = NULL;
    }

    if(__this->cap_buf)
    {
        free(__this->cap_buf);
        __this->cap_buf = NULL;
    }

    if(__this->aux_buf)
    {
        free(__this->aux_buf);
        __this->aux_buf = NULL;
    }

    /*
    if (__this->thumbnails_buf) {
        free(__this->thumbnails_buf);
        __this->thumbnails_buf = NULL;
    }
    */
#ifdef PHOTO_STICKER_ENABLE
    if(__this->sticker.addr)
    {
        free(__this->sticker.addr);
        __this->sticker.addr = NULL;
        __this->prev_sticker = 0;
    }
    memset(&__this->sticker, 0x0, sizeof(__this->sticker));
#endif
}

static int storage_device_available()
{
    struct vfs_partition *part;

    if(!storage_device_ready())
    {
        if(!dev_online(SDX_DEV))
        {
            video_photo_post_msg("noCard");
        }
        else
        {
            video_photo_post_msg("fsErr");
        }
        return false;
    }
    else
    {
        part = fget_partition(CONFIG_ROOT_PATH);
        if(part->clust_size < 32 || (part->fs_attr & F_ATTR_RO))
        {
            video_photo_post_msg("fsErr");
            return false;
        }
    }

    return true;
}

static int take_photo_num_calc(void *p)
{
    int err;
    int file_num;
    int coef;
    u32 free_space = 0;

    puts("take_photo_num_calc\n");

    if(!storage_device_available())
    {
        video_photo_post_msg("reNum:num=%p", "00000");
        return -EINVAL;
    }

    err = fget_free_space(CONFIG_ROOT_PATH, &free_space);
    if(err)
    {
        return err;
    }
    else
    {
#ifdef __CPU_AC521x__
        /*****720P的镜头*****/
        /* coef = photo_db_select("pres") > PHOTO_RES_5M ? 96 : 36; */
        coef = photo_db_select("pres") > PHOTO_RES_5M ? 96 : 61;
#else
        coef = photo_db_select("pres") > PHOTO_RES_8M ? 10 : 30;
#endif
        coef = coef / (photo_db_select("qua") + 1);
        file_num = free_space / (__this->camera[__this->camera_id].width *
                                 __this->camera[__this->camera_id].height / coef / 1024);
        if(file_num > 99999)
        {
            file_num = 99999;
        }
    }
    if(__this->camera_id == 0)
    {
        sprintf(__this->file_str, "%05d", file_num);
    }
    else
    {
#ifdef CONFIG_VIDEO1_ENABLE
        sprintf(__this->file_str, "%05d", file_num);
#elif defined CONFIG_VIDEO2_ENABLE
        sprintf(__this->file_str, "%s", "N/A");
#endif
    }

    log_d("take_photo_num_calc: %s\n", __this->file_str);

    video_photo_post_msg("reNum:num=%p", __this->file_str);

    return 0;
}

static int video_take_photo(struct photo_camera *camera)
{
    int err = 0;
    union video_req req = {0};
    struct icap_auxiliary_mem aux_mem;
    struct jpg_thumbnail thumbnails;
    char video_name[16];
    int i = 1;

    if(!storage_device_available())
    {
        return -EINVAL;
    }

    if(!camera || (camera->state != CAMERA_ONLINE))
    {
        return -EINVAL;
    }

#ifdef CONFIG_PSRAM_ENABLE
    if(camera->width > 1920)
    {
        camera_close(camera);
        photo_camera_stop_display();
    }
#endif
    /*
     *打开相机对应的video服务
     */
    /* #ifdef CONFIG_VIDEO1_ENABLE */
    /* sprintf(video_name, "video%d", __this->camera_id); */
    /* #elif (defined CONFIG_VIDEO2_ENABLE) */
    /* sprintf(video_name, "video%d", (__this->camera_id == 1) ? 3 : 0); */
    /* #endif */
    sprintf(video_name, "video%d.0", __this->camera_id);
    if(!camera->server)
    {
        camera->server = server_open("video_server", video_name);
    }
    else
    {
        if((camera->prev_width != camera->width) || (camera->prev_height != camera->height))
        {
            server_close(camera->server);
            /******一般尺寸拍照去掉2个buffer,节省动态空间*****/
            if(camera->width < 1920)
            {
                /* if (__this->aux_buf) { */
                /* free(__this->aux_buf);    */
                /* __this->aux_buf = NULL; */
                /* } */

                /*
                if (__this->thumbnails_buf) {
                    free(__this->thumbnails_buf);
                    __this->thumbnails_buf = NULL;
                }
                */
            }
            camera->server = server_open("video_server", video_name);
        }
    }
    camera->prev_width = camera->width;
    camera->prev_height = camera->height;

    if(!camera->server)
    {
        return -EFAULT;
    }

    /*
     *设置拍照所需要的buffer
     */
    if(!__this->cap_buf)
    {
        __this->cap_buf = (u8 *)malloc(CAMERA_CAP_BUF_SIZE);
    }

#ifdef PHOTO_STICKER_ENABLE
    req.icap.sticker = get_photo_sticker();
#endif

    face_det_onoff(0);

    int num = 0;
    struct photo_icon_t *icon = NULL;
    list_for_each_face_icon(icon)
    {
        if(icon->wear_sucess)
        {
            //佩戴成功的贴纸,合入照片
            log_d(">>>>%d icon left:%d top:%d width:%d height:%d\n", num, icon->left, icon->top, icon->width, icon->height);
            req.icap.icon_sticker[num] = get_photo_sticker_by_path(icon->sticker_path, num);
            //16对齐
            int icon_width = (icon->width + 15) / 16 * 16;
            int icon_height = (icon->height + 15) / 16 * 16;
            /*下面传入比例参数*/
#if (IMAGE_270 == 1)
            req.icap.icon_sticker[num]->x_offset = (LCD_WIDTH - icon->top - icon_width) * 10000 / LCD_DEV_WIDTH;
            req.icap.icon_sticker[num]->y_offset = icon->left * 10000 / LCD_DEV_HIGHT;
#elif (IMAGE_90 == 1)
            req.icap.icon_sticker[num]->x_offset = icon->top * 10000 / LCD_DEV_WIDTH;
            req.icap.icon_sticker[num]->y_offset = (LCD_HEIGHT - icon->left - icon_height) * 10000 / LCD_DEV_HIGHT;
#endif
            req.icap.icon_sticker[num]->resize_width = icon_width * 10000 / LCD_DEV_WIDTH;
            req.icap.icon_sticker[num]->resize_height = icon_height * 10000 / LCD_DEV_HIGHT;
            req.icap.icon_sticker[num]->scale = 1;
        }
        else
        {
            //佩戴失败的图片不进行合成
            req.icap.icon_sticker[num]->scale = 0;
        }
        num++;
    }

#if (defined PHOTO_STICKER_ENABLE && __SDRAM_SIZE__ <= (8 * 1024 * 1024))
    if(camera->width >= 1920 && !__this->aux_buf)
    {
#else

#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
    if(camera->width > 1280 && !__this->aux_buf)
    {
#else
    if(camera->width > 1920 && !__this->aux_buf)
    {
#endif

#endif

        /*
         *设置尺寸较大时缩放所需要的buffer
         */
        __this->aux_buf = (u8 *)malloc(IMAGE_AUX_BUF_SIZE);
        /*__this->thumbnails_buf = (u8 *)malloc(IMAGE_THUMB_BUF_SIZE);*/
        if(!__this->aux_buf/* || !__this->thumbnails_buf*/)
        {
            err = -ENOMEM;
            goto __err;
        }
    }

    if(!__this->cap_buf)
    {
        err = -ENOMEM;
        goto __err;
    }

    aux_mem.addr = __this->aux_buf;
    aux_mem.size = IMAGE_AUX_BUF_SIZE;

    /*
     *配置拍照服务参数
     *尺寸、buffer、质量、日期标签
     */
    req.icap.width = camera->width;
    req.icap.height = camera->height;
    req.icap.quality = photo_db_select("qua");
    if(camera->id == 0)
    {
        req.icap.path = CAMERA0_CAP_PATH"img_***.jpg";
    }
    else if(camera->id == 1)
    {
        req.icap.path = CAMERA1_CAP_PATH"img_***.jpg";
    }
    else
    {
        req.icap.path = CAMERA2_CAP_PATH"img_***.jpg";
    }
    /*req.icap.path = camera->id == 0 ? CAMERA0_CAP_PATH"img_***.jpg" : CAMERA1_CAP_PATH"img_***.jpg";*/
    req.icap.buf = __this->cap_buf;
    req.icap.buf_size = CAMERA_CAP_BUF_SIZE;
    req.icap.aux_mem = &aux_mem;
    req.icap.camera_type = VIDEO_CAMERA_NORMAL;
#ifdef JPG_THUMBNAILS_ENABLE
    if(camera->width > 1920)
    {
        thumbnails.enable = 1;
        thumbnails.quality = 10;
        thumbnails.width = 480;
        thumbnails.height = 320;
        /**将缩略图buffer和拍照缓存复用**/
        thumbnails.buf = __this->cap_buf + CAMERA_CAP_BUF_SIZE - IMAGE_THUMB_BUF_SIZE;//thumbnails_buf;
        thumbnails.len = IMAGE_THUMB_BUF_SIZE;
        req.icap.thumbnails = &thumbnails;
    }
#endif

    if(camera->id == 2)
    {
        req.icap.camera_type = VIDEO_CAMERA_UVC;
        req.icap.uvc_id = __this->uvc_id;
    }

    if(photo_db_select("cyt"))
    {
        i = 3;
    }
#ifdef DIGITAL_SCALE
    req.icap.src_crop_enable = VIDEO_CROP_ENABNLE | VIDEO_LARGE_IMAGE;////支持通过配置比例对IMC源数据进行裁剪,使用数字变焦需要写1
    req.icap.sca.x = (1280 - x_offset) / 2;
    req.icap.sca.y = (720 - y_offset) / 2;
    req.icap.sca.tar_w = x_offset;
    req.icap.sca.tar_h = y_offset;
#else
    req.rec.src_crop_enable = VIDEO_LARGE_IMAGE;
#endif

    req.icap.image_original = 1;//1:原尺寸 0:剪裁尺寸

    req.icap.text_label = photo_db_select("pdat") ? &__this->label : NULL;
    req.icap.src_w = camera->src_w;
    req.icap.src_h = camera->src_h;

    set_label_config(req.icap.width, req.icap.height, 0xe20095, req.icap.text_label);

    // 传递万花筒配置到底层
#ifdef CONFIG_CAMERA_EFFECT_ENABLE
    req.icap.eft_cfg = &__this->eff_cfg;
#else
    req.icap.eft_cfg = NULL;
#endif

    sys_key_event_disable();
    sys_touch_event_disable();
    video_photo_post_msg("tphin");

    while(i--)
    {
        /*
         *发送拍照请求
         */
        err = server_request(camera->server, VIDEO_REQ_IMAGE_CAPTURE, &req);
        if(err != 0)
        {
            puts("take photo err.\n");
            goto __err;
        }
        key_voice_start(1);
        video_photo_post_msg("tphend");
    }

    extern void malloc_stats(void);
    malloc_stats();

    video_photo_post_msg("tphout");
    photo_quick_scan(camera);
#ifdef CONFIG_PSRAM_ENABLE
    if(camera->width > 1920)
    {
        camera_close(camera);
        photo_camera_display();
    }
#endif
    for(int i = 0; i < ICON_STICKER_NUM; i++)
    {
        if(__this->icon_sticker[i].addr)
        {
            free(__this->icon_sticker[i].addr);
            __this->icon_sticker[i].addr = NULL;
        }
    }
    //__this->free_file_num--;

    log_d("video take photo end\n");
    if(!photo_db_select("sca"))
    {
        sys_key_event_enable();
        sys_touch_event_enable();
    }
    take_photo_num_calc(NULL);
    face_det_onoff(1);
    return 0;
__err:

    log_w("video take photo err : %d\n", err);
    camera_close(camera);
    face_det_onoff(1);

    return err;

}



static void take_photo_delay(void *p)
{
    if(__this->delay_ms)
    {
        video_take_photo(&__this->camera[__this->camera_id]);
        __this->delay_ms = 0;
    }
}


static int video_delay_take_photo(struct photo_camera *camera)
{
    int delay_sec;
    struct vfs_partition *part;

    if(!storage_device_available())
    {
        return -EINVAL;
    }

    if(__this->delay_ms > 0)
    {
        return 0;
    }

    delay_sec = photo_db_select("phm");
    __this->delay_ms = delay_sec * 1000;


    if(__this->timeout)
    {
        sys_timeout_del(__this->timeout);
        __this->timeout = 0;
    }

    video_photo_post_msg("dlyTPH:ms=%4", __this->delay_ms);
    /*
     *设置超时函数，超过delay_ms拍照
     */
    __this->timeout = sys_timeout_add((void *)camera, take_photo_delay, __this->delay_ms);
    if(!__this->timeout)
    {
        return -EFAULT;
    }
    sys_key_event_disable();
    sys_touch_event_disable();

    return 0;
}

static int show_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if(!__this->ui)
    {
        return -EINVAL;
    }
    req.show.id = ID_WINDOW_VIDEO_TPH;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif
    return 0;
}


static void hide_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if(!__this->ui)
    {
        return;
    }


    req.hide.id = ID_WINDOW_VIDEO_TPH;
    server_request(__this->ui, UI_REQ_HIDE, &req);
#endif
}

static int photo_camera_display(void)
{
    union video_req req = {0};
    char video_name[16];

#ifdef CONFIG_DISPLAY_ENABLE
    if(__this->display)
    {
        return 0;
    }

    if(__this->camera[__this->camera_id].state == CAMERA_ONLINE)
    {

        /*
         *打开对应摄像头的服务
         */
        sprintf(video_name, "video%d.0", __this->camera_id);

        __this->display = server_open("video_server", video_name);

        if(!__this->display)
        {
            puts("server_open:faild\n");
            return -EFAULT;
        }

        /*
         *设置显示参数
         */
        memset(&req, 0x0, sizeof(req));
        req.display.fb      = "fb1";
        req.display.left    = 0;//dc->rect.left;
        req.display.top     = 0;//dc->rect.top;
        req.display.state   = VIDEO_STATE_START;
        req.display.pctl = video_disp_get_pctl();
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
        /*
         * 切割尺寸
         * */
#if (LCD_DEV_WIDTH == 1280)
        //1280x480
        req.display.width   = 1280;
        req.display.height  = 720;
        req.display.border_left  = 0;
        req.display.border_top   = 112;//(req.display.height - LCD_DEV_HIGHT) / 2 / 16 * 16; // 0;
        req.display.border_right = 0;
        req.display.border_bottom = 128;//(req.display.height - LCD_DEV_HIGHT - req.display.border_top) / 16 * 16; // 0;
#elif (LCD_DEV_WIDTH == 1152)
        //1152x432
        req.display.width   = 1152;
        req.display.height  = 640;//720 * LCD_DEV_WIDTH / 1280 / 16 * 16;// height;
        req.display.border_left  = 0;
        req.display.border_top   = 96;//(req.display.height - LCD_DEV_HIGHT) / 2 / 16 * 16; // 0;
        req.display.border_right = 0;
        req.display.border_bottom = 112;//(req.display.height - LCD_DEV_HIGHT - req.display.border_top) / 16 * 16; // 0;
#else
        //不切割
        req.display.width   = LCD_DEV_WIDTH;//dc->rect.width;
        req.display.height  = LCD_DEV_HIGHT;//dc->rect.height;
        req.display.border_left = 0;
        req.display.border_top  = 0;
        req.display.border_right = 0;
        req.display.border_bottom = 0;
#endif

#elif (defined CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE || defined CONFIG_UI_STYLE_LY_ENABLE)
        req.display.width   = LCD_DEV_WIDTH;//dc->rect.width;
        req.display.height  = LCD_DEV_HIGHT;//dc->rect.height;
        req.display.border_left = 0;
        req.display.border_top  = 0;
        req.display.border_right = 0;
        req.display.border_bottom = 0;
        req.display.camera_type = VIDEO_CAMERA_NORMAL;
#endif

        req.display.camera_type = VIDEO_CAMERA_NORMAL;
        if(__this->camera_id == 0)
        {
            req.display.display_mirror = 0;
            req.display.win_type    = DISP_FRONT_WIN;
#ifdef DIGITAL_SCALE
            req.display.src_crop_enable = VIDEO_CROP_ENABNLE | VIDEO_LARGE_IMAGE;////支持通过配置比例对IMC源数据进行裁剪,使用数字变焦需要写1
            req.display.sca.x = (1280 - x_offset) / 2;
            req.display.sca.y = (720 - y_offset) / 2;
            req.display.sca.tar_w = x_offset;
            req.display.sca.tar_h = y_offset;
#else
            req.display.src_crop_enable = VIDEO_LARGE_IMAGE;
#endif
        }
        else
        {
            req.display.display_mirror = 0;
            req.display.win_type    = DISP_BACK_WIN;
        }
#if (DOUBLE_720 == 1)
        req.display.double720       = true;
#endif

#ifdef CONFIG_VIDEO2_ENABLE
        if(__this->camera_id == 2)
        {
            req.display.camera_type = VIDEO_CAMERA_UVC;
            req.display.uvc_id = __this->uvc_id;
        }
#endif

#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
        get_sticker();

        if(__this->sticker_name)
        {
            req.display.sticker_name      = __this->sticker_name;
        }
#endif
        req.display.src_w = __this->camera[__this->camera_id].src_w;
        req.display.src_h = __this->camera[__this->camera_id].src_h;

        server_request(__this->display, VIDEO_REQ_DISPLAY, &req);
        if(__this->camera_id == 0)
        {
            int color = photo_db_select("col");
            if(color != PHOTO_COLOR_NORMAL)
            {
                set_isp_special_effect(__this->display, color);
            }
            else
            {
                start_update_isp_scenes(__this->display);
            }
        }
        /*
         *设置当前模式下摄像头的参数 : 曝光补偿/白平衡/锐化
         */
        if(__this->camera_id == 0)
        {
            set_camera_config(&__this->camera[__this->camera_id]);
        }
    }
#endif
    return 0;
}

static int photo_camera_stop_display(void)
{
    union video_req req;

    if(__this->display)
    {
        /*
        req.camera.mode = ISP_MODE_REC;
        req.camera.cmd = SET_CAMERA_MODE;
        server_request(__this->display, VIDEO_REQ_CAMERA_EFFECT, &req);
        */
        if(__this->camera_id == 0)
        {
            stop_update_isp_scenes();
        }

        req.display.state = VIDEO_STATE_STOP;
        /* req.display.state = VIDEO_STATE_SWITCH_CAMERA; */

        server_request(__this->display, VIDEO_REQ_DISPLAY, &req);
        server_close(__this->display);
        __this->display = NULL;

    }

    return 0;
}

static int photo_switch_camera_display(void)
{
    union video_req req;

    if(__this->display)
    {
        /*
        req.camera.mode = ISP_MODE_REC;
        req.camera.cmd = SET_CAMERA_MODE;
        server_request(__this->display, VIDEO_REQ_CAMERA_EFFECT, &req);
        */
        if(__this->camera_id == 0)
        {
            stop_update_isp_scenes();
        }

        // 不黑屏切换前后摄，仅用于拍照APP下切换前后显示窗口大小一致使用
        req.display.state = VIDEO_STATE_SWITCH_CAMERA;

        server_request(__this->display, VIDEO_REQ_DISPLAY, &req);
        server_close(__this->display);
        __this->display = NULL;

    }

    return 0;
}




static int video_photo_start(void)
{
    int err;
    int res = db_select("pres");
    __this->sticker_num = db_select("stk");
    __this->prev_sticker = -1;//__this->sticker_num;
    bool online;

    /*
     *相机初始化，后拉默认VGA分辨率
     */
    __this->camera[0].id = 0;
    __this->camera[0].state = CAMERA_ONLINE;
    __this->camera[0].width = camera0_reso[res].width;
    __this->camera[0].height = camera0_reso[res].height;

#ifdef CONFIG_VIDEO1_ENABLE
    online = dev_online("video1.*");
    __this->camera[1].id = 1;
    __this->camera[1].state = online ? CAMERA_ONLINE : CAMERA_OFFLINE;
#endif
#ifdef CONFIG_VIDEO2_ENABLE
    online = dev_online("uvc");
    __this->camera[2].id = 2;
    __this->camera[2].state = online ? CAMERA_ONLINE : CAMERA_OFFLINE;
#endif

#ifdef CONFIG_VIDEO1_ENABLE
#if (DOUBLE_720 == 1)
    __this->camera[1].width = 1280;
    __this->camera[1].height = 720;
#else
    __this->camera[1].width = 640;
    __this->camera[1].height = 480;
#endif
#endif

#ifdef CONFIG_VIDEO2_ENABLE
    __this->camera[2].width = 640;
    __this->camera[2].height = 480;
#endif

    __this->camera_id = 0;
    puts("video_photo start.\n");
    err = photo_camera_display();
    if(err)
    {
        return err;
    }

#ifdef PHOTO_STICKER_ENABLE
    if(__this->sticker_num)
    {
        if(sys_power_on())
        {
            photo_switch_sticker(__this->sticker_num);
        }
        else
        {
            __this->timer = sys_timer_add(NULL, scan_power_on_ani_end, 20);
        }
    }
#endif

    show_main_ui();

    __this->sd_wait = wait_completion(storage_device_ready, take_photo_num_calc, NULL);
    __this->state = PHOTO_MODE_ACTIVE;

    return 0;
}

static int video_photo_stop(void)
{
    __this->state |= PHOTO_MODE_STOP;

    /*
     *停止快速预览
     */
    stop_quick_scan(NULL);
    /*
     *关闭相机0，相机1，关闭显示
     */
    camera_close(&__this->camera[0]);
    camera_close(&__this->camera[1]);
    camera_close(&__this->camera[2]);


    photo_camera_stop_display();

#ifdef PHOTO_STICKER_ENABLE
    photo_switch_sticker(0);
#endif
    if(__this->video_dec)
    {
        server_close(__this->video_dec);
        __this->video_dec = NULL;
    }

    if(__this->timeout)
    {
        __this->delay_ms = 0;
        sys_timeout_del(__this->timeout);
        __this->timeout = 0;
        //防止拍照计时插USB，按键和touch失效
    }

    hide_main_ui();

    return 0;
}



static void photo_mode_init(void)
{
    if(__this->state == PHOTO_MODE_UNINIT)
    {
        memset(__this, 0, sizeof(*__this));
    }

    if(!__this->cap_buf)
    {
        __this->cap_buf = (u8 *)malloc(CAMERA_CAP_BUF_SIZE);
    }
#ifndef PHOTO_STICKER_ENABLE_SMALL_MEM
    if(!__this->aux_buf)
    {
        __this->aux_buf = (u8 *)malloc(IMAGE_AUX_BUF_SIZE);
    }
#endif

    __this->state = PHOTO_MODE_ACTIVE;
}

/*
 * 摄像头切换
 */
static int photo_switch_camera(void)
{
    int err;
    u8 switch_id;

    switch_id = __this->camera_id;
    do
    {
        if(++switch_id >= PHOTO_CAMERA_NUM)
        {
            switch_id = 0;
        }

        if(switch_id == __this->camera_id)
        {
            return 0;
        }

        if(__this->camera[switch_id].state == CAMERA_ONLINE)
        {
            break;
        }

    }
    while(switch_id != __this->camera_id);

    stop_quick_scan(NULL);
    camera_close(&__this->camera[__this->camera_id]);
    video_photo_cfg_reset();
    /* photo_camera_stop_display(); */
    photo_switch_camera_display();

    __this->camera_id = switch_id;

    take_photo_num_calc(NULL);

    err = photo_camera_display();
    if(err)
    {
        return err;
    }
    printf("camera_id = %d\n",  __this->camera_id);
    video_photo_post_msg("swCAM:id=%1", __this->camera_id);

    return 0;
}


static int video_photo_change_status(struct intent *it)
{
    int err;
    u32 free_space = 0;
    int file_num;
    int coef;
#ifdef CONFIG_UI_ENABLE
    struct key_event org = { KEY_EVENT_CLICK, KEY_OK };
    struct key_event new = { KEY_EVENT_CLICK, KEY_PHOTO };

    if(!strcmp(it->data, "opMENU:"))    /* ui要求打开rec菜单 */
    {
        puts("ui ask me to opMENU:.\n");
#ifdef PHOTO_STICKER_ENABLE
        photo_switch_sticker(0);
#endif
        if(1)    /* 允许ui打开菜单 */
        {
            it->data = "opMENU:en";
            sys_key_event_unmap(&org, &new);
        }
        else     /* 禁止ui打开菜单 */
        {

            it->data = "opMENU:dis";
        }

    }
    else if(!strcmp(it->data, "exitMENU"))      /* ui已经关闭rec菜单 */
    {
#ifdef PHOTO_STICKER_ENABLE
        photo_switch_sticker(__this->sticker_num);
#endif
        sys_key_event_map(&org, &new);
        puts("ui tell me exitMENU.\n");
        video_photo_restore();
        take_photo_num_calc(NULL);
    }
    else if(!strcmp(it->data, "idCAM:"))      /* 获取摄像头id */
    {
        if(__this->camera_id == 0)
        {
            it->data = "idCAM:0";
        }
        else
        {
            it->data = "idCAM:1";
        }
    }
    else
    {
        puts("unknow status ask by ui.\n");
    }
#endif
    return 0;
}

static int video_photo_change_source_reso(int dev_id, u16 width, u16 height)
{
    __this->camera[dev_id].src_w = width;
    __this->camera[dev_id].src_h = height;
    if(__this->camera[__this->camera_id].state == CAMERA_ONLINE)
    {
        log_d("video%d.* change source reso to %d x %d\n", dev_id, width, height);
        if(__this->camera_id == dev_id)
        {
            if((__this->state & PHOTO_MODE_ACTIVE) &&
                    !(__this->state & PHOTO_MODE_STOP) &&
                    !(__this->state & PHOTO_MODE_QSCAN))
            {
                photo_camera_stop_display();
                if(__this->timeout)
                {
                    sys_timeout_del(__this->timeout);
                }
                camera_close(&__this->camera[__this->camera_id]);
                //video2.* photo taking and display must be closed before source reso changing
                photo_camera_display();
                if(__this->timeout)
                {
                    __this->timeout = sys_timeout_add((void *)&__this->camera[__this->camera_id], take_photo_delay, __this->delay_ms);
                }
            }
        }
    }
    return 0;
}

static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int len;
    struct intent mit;
    struct key_event org = { KEY_EVENT_CLICK, KEY_OK };
    struct key_event new = { KEY_EVENT_CLICK, KEY_PHOTO };

    switch(state)
    {
        case APP_STA_CREATE:
            log_d("\n>>>>> video_photo: create\n");
            photo_mode_init();
            server_load(video_server);
#ifdef CONFIG_UI_ENABLE
#ifdef MULTI_LCD_EN
            struct ui_style style = {0};
            if(get_current_disp_device())
            {
                style.file = "mnt/spiflash/res/avo_LY.sty\0";
            }
            else
            {
                style.file = "mnt/spiflash/res/lcd_LY.sty\0";
            }
            __this->ui = server_open("ui_server", &style);
#else
            __this->ui = server_open("ui_server", NULL);
#endif

            if(!__this->ui)
            {
                return -EINVAL;
            }
#endif
            sys_key_event_map(&org, &new);
            break;
        case APP_STA_START:
            if(!it)
            {
                break;
            }
            switch(it->action)
            {
                case ACTION_PHOTO_TAKE_MAIN:
                    video_photo_start();
                    break;
                case ACTION_PHOTO_TAKE_SET_CONFIG:
                    if(!__this->ui)
                    {
                        return -EINVAL;
                    }
                    video_photo_set_config(it);
                    db_flush();
                    if(it->data && !strcmp(it->data, "pres"))
                    {
                        take_photo_num_calc(NULL);
                    }
                    break;
                case ACTION_PHOTO_TAKE_CHANGE_STATUS:
                    video_photo_change_status(it);
                    break;
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
                case ACTION_PHOTO_TAKE_CONTROL:
                    puts("---------to take photo----------.\n");
                    if(__this->state & PHOTO_MODE_QSCAN)
                    {
                        break;
                    }

                    if(photo_db_select("phm") == 0)
                    {
                        /*
                         *正常拍照
                         */
                        video_take_photo(&__this->camera[__this->camera_id]);
                    }
                    else
                    {
                        /*
                         *延时拍照
                         */
                        video_delay_take_photo(&__this->camera[__this->camera_id]);
                    }
                    break;
                case ACTION_PHOTO_TAKE_SWITCH_WIN:
                    photo_switch_camera();
                    break;
#endif
            }
            break;
        case APP_STA_PAUSE:
            break;
        case APP_STA_RESUME:
            break;
        case APP_STA_STOP:
            video_photo_cfg_reset();
            video_photo_stop();
            if(__this->sd_wait)
            {
                wait_completion_del(__this->sd_wait);
                __this->sd_wait = 0;
            }
            break;
        case APP_STA_DESTROY:
            if(__this->ui)
            {
                server_close(__this->ui);
                __this->ui = NULL;
            }

            sys_key_event_unmap(&org, &new);
            f_free_cache(CONFIG_ROOT_PATH);
            malloc_stats();
            log_d("<<<<<< video_photo: destroy\n");
            break;
    }

    return 0;

}
/* #define UVC_CMA_GRAY */
static int video_photo_key_event_handler(struct key_event *key)
{
    struct intent it;
    static u8 mode = 0;

#ifdef UVC_CMA_GRAY
    // VGA UVC黑白切换功能
    void *uvc_test;
    static u32 flag = 0;
#endif

    switch(key->event)
    {
        case KEY_EVENT_CLICK:
            switch(key->value)
            {
                case KEY_PHOTO:
                    if(__this->state & PHOTO_MODE_QSCAN)
                    {
                        break;
                    }

                    if(photo_db_select("phm") == 0)
                    {
                        /*
                         *正常拍照
                         */
                        video_take_photo(&__this->camera[__this->camera_id]);
                    }
                    else
                    {
                        /*
                         *延时拍照
                         */
                        video_delay_take_photo(&__this->camera[__this->camera_id]);
                    }
                    break;
                case KEY_MODE:
#ifdef CONFIG_CAMERA_EFFECT_ENABLE
                    if(__this->eff_cfg.enable)
                    {
                        mode = 0;
                        camera_effect_stop((u32 *)&__this->eff_cfg);
                        camera_effect_uninit((u32 *)&__this->eff_cfg);
                    }
#endif
                    break;
                case KEY_MENU:
                    break;
                case KEY_UP:
#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
                    __this->sticker_num++;
                    if(__this->sticker_num > STICKER_NUM)
                    {
                        __this->sticker_num = 0;
                    }
                    switch_sticker();
                    db_update("stk", __this->sticker_num);
                    db_flush();
                    break;
#endif

#if (defined PHOTO_STICKER_ENABLE)
                    __this->sticker_num++;
                    if(__this->sticker_num > PHOTO_STICKER_NUM)
                    {
                        __this->sticker_num = 0;
                    }
                    photo_switch_sticker(__this->sticker_num);
                    db_update("stk", __this->sticker_num);
                    db_flush();

#elif (defined CONFIG_CAMERA_EFFECT_ENABLE)
                    // 启动万花筒特效
                    if(!__this->eff_cfg.enable)
                    {
                        puts("camera effect init\n");

                        // 320*240测试
#if (defined LCD_DSI_VDO_2LANE_MIPI_ST7701S)        // 竖屏
                        __this->eff_cfg.enable = 0;
                        __this->eff_cfg.mode   = mode;
                        __this->eff_cfg.eff_t.w = 320;
                        __this->eff_cfg.eff_t.h = 240;
                        /* __this->eff_cfg.eff_t.w = 480; */
                        /* __this->eff_cfg.eff_t.h = 272; */
                        __this->eff_cfg.eff_t.wh_mode = 1;
                        __this->eff_cfg.resulotion = 0;
                        __this->eff_cfg.eff_t.mode_full = 0;

#elif (defined LCD_480x272_8BITS)   // 横屏显示测试
                        __this->eff_cfg.enable = 0;
                        __this->eff_cfg.mode   = mode;
                        __this->eff_cfg.eff_t.w = 480;
                        __this->eff_cfg.eff_t.h = 272;
                        __this->eff_cfg.eff_t.wh_mode = 0;
                        __this->eff_cfg.resulotion = 0;
                        __this->eff_cfg.eff_t.mode_full = 0;
#endif
                        __this->eff_cfg.eff_t.buffer = (u8 *)malloc(__this->eff_cfg.eff_t.w * __this->eff_cfg.eff_t.h * 3 / 2);
                        __this->eff_cfg.dst_buf = (u8 *)malloc(__this->eff_cfg.eff_t.w * __this->eff_cfg.eff_t.h * 3 / 2);

                        camera_effect_init((u32 *)&__this->eff_cfg);
                        camera_effect_start((u32 *)&__this->eff_cfg);

                    }
                    else
                    {
                        // 切换模式
                        if(++mode < EM_END)
                        {
                            printf("camera effect mode switch\n");
                            camera_effect_switch_mode((u32 *)&__this->eff_cfg, mode);
                        }
                        else
                        {
                            puts("stop camera effect\n");
                            mode = 0;
                            camera_effect_stop((u32 *)&__this->eff_cfg);
                            camera_effect_uninit((u32 *)&__this->eff_cfg);
                        }
                    }
#else
                    photo_switch_camera();
#endif
                    break;
                case KEY_DOWN:

#ifdef UVC_CMA_GRAY
                    uvc_test = dev_open("uvc", 0);
                    dev_ioctl(uvc_test, UVCIOC_SET_CUR_GRAY, flag);
                    dev_close(uvc_test);
                    flag = ~flag;
#endif

#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
                    __this->sticker_num--;
                    if(__this->sticker_num < 0)
                    {
                        __this->sticker_num = STICKER_NUM;
                    }
                    switch_sticker();
                    db_update("stk", __this->sticker_num);
                    db_flush();
                    break;
#endif

#ifdef PHOTO_STICKER_ENABLE
                    __this->sticker_num--;
                    if(__this->sticker_num < 0)
                    {
                        __this->sticker_num = PHOTO_STICKER_NUM;
                    }
                    photo_switch_sticker(__this->sticker_num);
                    db_update("stk", __this->sticker_num);
                    db_flush();
#endif
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return false;
}

static int video_photo_device_event_handler(struct sys_event *e)
{
    int err;

    if(!strcmp(e->arg, "video1"))
    {
        switch(e->u.dev.event)
        {
            case DEVICE_EVENT_IN:
                if(__this->camera_id == 1 && __this->camera[1].state == CAMERA_OFFLINE)
                {
                    err = photo_camera_display();
                    if(err)
                    {
                        return err;
                    }
                }
                __this->camera[1].state = CAMERA_ONLINE;
                break;
            case DEVICE_EVENT_OUT:
                if(__this->camera_id == 1)
                {
                    photo_switch_camera();
                }
                __this->camera[1].state = CAMERA_OFFLINE;
                break;
            default:
                break;
        }
    }

    if(!strncmp((char *)e->arg, "uvc", 3))
    {
        switch(e->u.dev.event)
        {
            case DEVICE_EVENT_IN:
                __this->uvc_id = ((char *)e->arg)[3] - '0';
                if(__this->camera_id == 2 && __this->camera[2].state == CAMERA_OFFLINE)
                {
                    err = photo_camera_display();
                    if(err)
                    {
                        return err;
                    }
                }
                __this->camera[2].state = CAMERA_ONLINE;
                break;
            case DEVICE_EVENT_OUT:
                if(__this->camera_id == 2)
                {
                    photo_switch_camera();
                }
                __this->camera[2].state = CAMERA_OFFLINE;
                break;

        }
    }

    if(!strcmp((char *)e->arg, "camera0_err"))
    {
        photo_camera_stop_display();
        photo_camera_display();
    }

    if(!ASCII_StrCmp(e->arg, "sd*", 4))
    {
        take_photo_num_calc(NULL);
    }

    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{
    switch(event->type)
    {
        case SYS_KEY_EVENT:
            return video_photo_key_event_handler(&event->u.key);
        case SYS_DEVICE_EVENT:
            return video_photo_device_event_handler(event);
        default:
            return false;
    }
    return false;
}

static const struct application_operation video_photo_ops =
{
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_video_photo) =
{
    .name   = "video_photo",
    .action = ACTION_PHOTO_TAKE_MAIN,
    .ops    = &video_photo_ops,
    .state  = APP_STA_DESTROY,
};

