#include "system/includes.h"
#include "server/usb_server.h"
#include "app_config.h"
#include "action.h"
#include "storage_device.h"
#include "user_isp_cfg.h"
#include "server/video_server.h"


#define UVC_H264_BUF_SIZE  (1 * 1024 * 1024)

struct uvc_cam_h264_t {
    u16 cur_width;
    u16 cur_height;
};
struct uvc_cam_h264_t _uvc_cam_h264 = {0};

#define __this   (&_uvc_cam_h264)

static void uvc_h264_recv_task(void *arg)
{
    void *fd = NULL;
    struct uvc_reqbufs breq = {0};
    struct video_buffer b = {0};
    struct uvc_capability uvc_cap;
    int err = -1;
    int msg[32];
    u8 i = 0;
    if (dev_online("uvc")) {
        fd = dev_open("uvc", (void *)1); //指定uvc设备1
        if (fd) {
            dev_ioctl(fd, UVCIOC_QUERYCAP, (unsigned int)&uvc_cap);
            for (i = 0; i < uvc_cap.reso_num; i++) {
                printf("reso[%d] w:%d h=%d\n", i, uvc_cap.reso[i].width, uvc_cap.reso[i].height);
                if (__this->cur_width == uvc_cap.reso[i].width && __this->cur_height == uvc_cap.reso[i].height) {
                    break;
                }
            }
            if (i == uvc_cap.reso_num) {
                log_w("can not find reso,please check!!\n");
                i = 0; //传入的分辨率非法时,默认输出第一个分辨率
            }
            dev_ioctl(fd, UVCIOC_SET_CAP_SIZE, (unsigned int)i + 1);
            breq.buf = NULL;
            breq.size = UVC_H264_BUF_SIZE;

            err = dev_ioctl(fd, UVCIOC_REQBUFS, (unsigned int)&breq);
            if (!err) {
                dev_ioctl(fd, UVCIOC_STREAM_ON, 0);
            } else {
                err = -1;
                puts("uvc1 video open err1--------------------\n");
            }
        }

    }
    /* FILE *f = fopen(CONFIG_ROOT_PATH"h264***.264", "w+"); */
    /* if (!f) { */
    /* log_e("fopen faild\n"); */
    /* } */

    b.timeout = 100;

    while (1) {
        os_taskq_accept(ARRAY_SIZE(msg), msg);
        if (msg[0] == Q_MSG) {
            if (msg[1] == 0xaa) {
                if (os_task_del_req(OS_TASK_SELF) == OS_TASK_DEL_REQ) {
                    //准备退出线程
                    if (fd) {
                        dev_ioctl(fd, UVCIOC_STREAM_OFF, 0);
                        dev_close(fd);
                    }
                    /* if (f) { */
                    /* fclose(f); */
                    /* } */
                    log_d("del h264 task del self\n");
                    os_task_del_res(OS_TASK_SELF);
                }
            }
        }
        //从uvc 缓存中请求一帧H264图像
        if (fd) {
            err = dev_ioctl(fd, UVCIOC_DQBUF, (unsigned int)&b);
        }
        if (err || b.len <= 0) { //请求失败
            os_time_dly(1);
            continue;
        }
        //请求成功
        //TODO
        putchar('R');
        /* if (f) { */
        /* fwrite(f, b.baddr, b.len); */
        /* } */
        dev_ioctl(fd, UVCIOC_QBUF, (unsigned int)&b);
    }
}

int video2_uvc_h264_stream_open(u16 width, u16 height)
{
    memset(__this, 0x00, sizeof(struct uvc_cam_h264_t));
    __this->cur_width = width;
    __this->cur_height = height;
    return os_task_create(uvc_h264_recv_task, NULL, 20, 1024, 32, "uvc_h264_recv");
}

int video2_uvc_h264_stream_close()
{
    while (OS_TASK_NOT_EXIST != os_task_del_req((const char *)"uvc_h264_recv")) {
        os_taskq_post_msg((const char *)"uvc_h264_recv", 1,  0xaa);
        os_time_dly(1);
    }
}
