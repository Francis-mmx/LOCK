#include "system/includes.h"
#include "os/os_compat.h"
#include "app_config.h"
#include "face_detect.h"

#define FACE_DET_DATA_SOURCE      "video0.4"
#define FACE_DET_IMAGE_WIDTH       320
#define FACE_DET_IMAGE_HEIGHT      240
#define FACE_DET_FPS               4     //4-5fps

#define FACE_DEBUG    1

#if (FACE_DEBUG == 1)
    #define _FACE_DEBUG     log_d
#else
    #define _FACE_DEBUG(...)
#endif

void face_det_init(void);
void face_det_uinit(void);
extern void video_rec_post_msg(const char *msg, ...);
extern void video_photo_post_msg(const char *msg, ...);


struct face_det_info
{
    int pid;
    u8 exit_state;
    u8 face_det_state;
    u8 camera_fps;
    u8 yuv_init_flag;
    u8 have_face;
    u8 no_face_cnt;
    u8 *y;
    face_detect_f e;
};

static struct face_det_info _info = {0};
#define __this  (&_info)



static int face_det_load(void)
{

    int fast_m = 1; //人脸检测的两种模式，0是比较慢的检测，1是比较快的检测
    int num_r = 6; //对应的第一层网络的捡到人脸的个数，一般可能有十几二十几个，这里选择置信度最高的6个；默认6个，可以再增加，但是检测的时间也会增加。
    int num_o = 3; //对应的第二层网络的捡到人脸的个数，一般可能有七八个，这里选择置信度最高的3个；可以再增加，但是检测的时间也会增加。

    int thresh[3] = { 30, -10, -20 };//取值范围（-255，255）第一个值是第一层网络的置信度，值越小召回率越高；
    //第二个值是第二层网络的置信度，一般不会大于第一个值；
    //第三个值是第三层网络的置信度，一般不会大于第二个值。
    //
    int smile_thresh = 0;  //笑脸预值
    memset(&__this->e, 0x0, sizeof(face_detect_f));
    face_detect_f *e = &__this->e;

    face_detect_init(fast_m, num_r, num_o, thresh, smile_thresh, e); //初始化内存和需要配置的参数

    return 0;

}
static int face_det_uninstall(void *p)
{
    struct face_detect_f *e = (struct face_detect_f *)p;

    if(e)
    {
        face_detect_free(e);//释放内存
    }

    return 0;
}


int detect_f(frame *fp)
{
    int num_box = 0;

    face_detect_f *e = &__this->e;

    num_box = face_detect_process(fp, e, e->confidence_, e->bounding_box, e->alignment_temp);
    //num_box返回检测到的人脸个数；
    //e->confidence_ 返回检测到的人脸的置信度；
    //e->bounding_box 返回捡到的框的起始点坐标和结束点坐标；
    //e->alignment_temp  返回捡到的框内的人脸的五个关键点的坐标。

    _FACE_DEBUG("\nnum_box:%d \n", num_box);
    smile_face_process(num_box, fp, e, e->confidence_, e->bounding_box, e->alignment_temp, e->smile_idx); //0是笑脸  1是不笑的脸   //num_box检测笑脸个数,越大检测时间越长
    if(num_box)
    {
        for(int i = 0; i < num_box; i++)
        {
            _FACE_DEBUG("e->bounding_box: start(%d,%d) end(%d,%d)\n",  \
                        e->bounding_box[i].x1, e->bounding_box[i].y1,  \
                        e->bounding_box[i].x2, e->bounding_box[i].y2);
            for(int j = 0; j < 5; j++)
            {
                _FACE_DEBUG("e->alignment_temp: (%d,%d) \n",  \
                            e->alignment_temp[j].x, e->alignment_temp[j].y);
            }
            _FACE_DEBUG(">>>>>>> smile face : %d \n", e->smile_idx[i]);
            if(e->smile_idx[i] == 0)
            {
                //笑脸
                //TODO
            }
            video_photo_post_msg("faceOn:d=%4 I=%4 p=%4 s=%4", i, (u32)e->bounding_box, (u32)e->alignment_temp, i); //发送消息到UI进行图片的显示
            __this->have_face = 1;
            __this->no_face_cnt = 0;

        }
    }
    else
    {
        if(__this->have_face)
        {
            if(++__this->no_face_cnt > (2000 / (1000 / FACE_DET_FPS)))    //2s未检测到人脸
            {
                video_photo_post_msg("faceOff");
                __this->have_face = 0;
                __this->no_face_cnt = 0;
            }
        }
    }
    /* face_detect_free(e);//释放内存 */

    return num_box;

}
static int image_deal_process(struct fb_map_user *map)
{
    static u32 time = 0;
    static int fps_cnt = 0;
    static int fps_cnt_time = 0;
    u32 frame_size = 0;
    frame f = {0};
    f.w = FACE_DET_IMAGE_WIDTH;
    f.h = FACE_DET_IMAGE_HEIGHT;
    f.c = 1;
    frame_size = f.w * f.h;
    u8 fps = (1000 / FACE_DET_FPS) / (1000 / __this->camera_fps);
    if(++time > fps)
    {
//            fps_cnt++;
//            if (time_after(jiffies, fps_cnt_time)) {
//                fps_cnt_time = jiffies + msecs_to_jiffies(1000);
//                printf("fps:%d\n",fps_cnt);
//                fps_cnt = 0;
//            }
        time = 0;
        if(!__this->y)
        {
            __this->y = (u8 *)malloc(frame_size);
        }
        memcpy(__this->y, map->baddr, frame_size);
        for(int i = 0; i < frame_size; i++)
        {
            __this->y[i] -= 128;
        }
        f.pixel = __this->y;

        detect_f(&f);
    }

    return 0;
}

void face_yuv_task(void *priv)
{
    int len;
    int ret;
    void *video_dev_fd = NULL, *fb2 = NULL;
    struct fb_map_user map1;

    _FACE_DEBUG(">>>>>>>>>>>>>> Enter face yuv_task\n");
    malloc_stats();

    //msleep(5000);
    //数据来源video，输出到fb2
    struct video_format f = {0};
    struct fb_var_screeninfo fb2_info = {0};
    const char *dev_name = FACE_DET_DATA_SOURCE;
    const char *fb_name = "fb2";
    f.type  = VIDEO_BUF_TYPE_VIDEO_OVERLAY;
    f.win.left = 0;
    f.win.top    = 0;
    f.win.width  = FACE_DET_IMAGE_WIDTH;
    f.win.height = FACE_DET_IMAGE_HEIGHT;
    f.win.border_left   = 0;
    f.win.border_top    = 0;
    f.win.border_right  = 0;
    f.win.border_bottom = 0;
    f.private_data   = fb_name;

    video_dev_fd = dev_open(dev_name, NULL);
    if(video_dev_fd == NULL)
    {
        _FACE_DEBUG("%s test open device %s faild\n", fb_name, dev_name);
        return;
    }

    ret = dev_ioctl(video_dev_fd, VIDIOC_SET_FMT, (u32)&f);
    if(ret)
    {
        _FACE_DEBUG("VIDIOC_SET_FMT faild\n");
        dev_close(video_dev_fd);
        return;
    }

    ret = dev_ioctl(video_dev_fd, VIDIOC_OVERLAY, 1);
    if(ret)
    {
        _FACE_DEBUG("VIDIOC_OVERLAY faild\n");
        dev_close(video_dev_fd);
        return;
    }

    ret = dev_ioctl(video_dev_fd, VIDIOC_GET_FPS, (u32)&__this->camera_fps);

    //打开显示通道
    fb2 = dev_open(f.private_data, (void *)FB_COLOR_FORMAT_YUV420);
    if(!fb2)
    {
        _FACE_DEBUG(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);
        dev_close(video_dev_fd);
        return;
    }

    face_det_load(); /*初始化加载人脸识别模块*/

    malloc_stats();
    while(1)
    {

        if(__this->exit_state)
        {
            __this->exit_state = 0;
            break;
        }
        if(!__this->face_det_state)
        {
            os_time_dly(2);
            continue;
        }

        dev_ioctl(fb2, FBIOGET_FBUFFER_INUSED, (int)&map1);
        if((u32)map1.baddr)
        {
            image_deal_process(&map1);
            //获取到才释放
            dev_ioctl(fb2, FBIOPUT_FBUFFER_INUSED, (int)&map1);
        }
        else
        {
            //获取不到重试
            continue;
        }

    }
    face_det_uninstall(&__this->e); /*人脸识别模块释放与卸载*/
    dev_close(fb2);
    dev_ioctl(video_dev_fd, VIDIOC_OVERLAY, 0);
    dev_close(video_dev_fd);
    _FACE_DEBUG("face_yuv_task exit\n\n ");

}

void face_det_init()
{
    if(__this->yuv_init_flag == 1)
    {
        return;
    }
    memset(__this, 0, sizeof(struct face_det_info));
    __this->yuv_init_flag = 1;
    __this->exit_state = 0;
    __this->face_det_state = 1;
    __this->y = NULL;
    thread_fork("GET_YUV_TASK", 15, 0x2000, 0, &__this->pid, face_yuv_task, NULL);
}

#ifdef FACE_DECT_ENABLE
    late_initcall(face_det_init);
#endif

void face_det_uinit(void)
{
    if(__this->yuv_init_flag == 0)
    {
        return;
    }
    if(__this->y)
    {
        free(__this->y);
        __this->y = NULL;
    }
    __this->yuv_init_flag = 0;
    __this->face_det_state = 0;
    __this->exit_state = 1;
    thread_kill(&__this->pid, KILL_WAIT);
    face_det_uninstall(&__this->e); /*人脸识别模块释放与卸载*/
}
void face_det_onoff(u8 onoff) // 人脸检测控制 0:暂停 1:开启
{
    if(__this->yuv_init_flag)
    {
        __this->face_det_state = onoff;
    }
}

