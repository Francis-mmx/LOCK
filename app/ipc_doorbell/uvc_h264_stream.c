#define _STDIO_H_
#include "xciot_api.h"
#include "system/includes.h"
#include "server/usb_server.h"
#include "app_config.h"
#include "action.h"
#include "storage_device.h"
#include "user_isp_cfg.h"
#include "server/video_server.h"
#include "sock_api/sock_api.h"
#include "rt_stream_pkg.h"

#define UVC_H264_BUF_SIZE  ( 512 * 1024 + 256 * 1024)
static const char Iframe[5] = {0x0, 0x0, 0x0, 0x1, 0x67};
static const char Pframe[5] = {0x0, 0x0, 0x0, 0x1, 0x41};
struct uvc_cam_h264_t {
    u16 cur_width;
    u16 cur_height;
};
struct uvc_cam_h264_t _uvc_cam_h264 = {0};

#define __this   (&_uvc_cam_h264)
u8 is_key_frame;
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
//            os_time_dly(1);
            extern void vPortYield(void);
            vPortYield();
//            putbyte('G');
            continue;
        }
        //请求成功
        //TODO
        // putchar('R');
//        static u8 vframe[512 * 1024];
//

//        if(b.len > sizeof(vframe)){
//           ASSERT(0,"\n vframe len = %d\n",b.len / 1024);
//        }
//
//        put_buf(b.baddr,64);
//        printf("\n>>>>>>>>>>>>>> %s %d\n",__func__,__LINE__);
//        memcpy(vframe,b.baddr,b.len);
//        u32 start_code = 0x01000000;
//        u32 tmp2;
//        u32 tmp;
//          printf("\n>>>>>>>>>>>>>> %s %d\n",__func__,__LINE__);
//        if (*((char *)(vframe + 4)) == 0x67) {
//            memcpy(&tmp, vframe, 4);
//            tmp = htonl(tmp);
//            memcpy(vframe, &start_code, 4);
//            memcpy(&tmp2, vframe + tmp + 4, 4);
//            tmp2 = htonl(tmp2);
//            memcpy(vframe + tmp + 4, &start_code, 4);
//            memcpy(vframe + tmp + tmp2 + 8, &start_code, 4);
//            is_key_frame = 1;
//
//        } else {
//            memcpy(vframe, &start_code, 4);
//            is_key_frame = 0;
//        }

        static int t = 0, fps = 0;
        if (t == 0) {
            t = timer_get_ms();
        }

        fps++;
        if ((timer_get_ms() - t) >= 1000) {
            printf("\n fps = %d \n", fps);
            t = timer_get_ms();
            fps = 0;
        }

        if (!memcmp(b.baddr, Iframe, 5)) {
            is_key_frame = 1;
        } else if (!memcmp(b.baddr, Pframe, 5)) {
            is_key_frame = 0;
        }

        if (get_avsdk_init_flag() == 2) {

            static int timer;
            extern uint64_t get_utc_ms(void);
            if (timer == 0) {
                timer = get_utc_ms();
            }


            // printf("\navsdk_poll_current_conn_num() = %d \n", avsdk_poll_current_conn_num());

            avsdk_write_video(E_IOT_MEDIASTREAM_TYPE_H264, is_key_frame, 0, get_utc_ms(), NULL, b.baddr, b.len);

            cloud_storage_video_write(H264_TYPE_VIDEO, is_key_frame, timer_get_ms(), b.baddr, b.len);
//        printf("\n >>>>>>>>>>>timer = %d\n",get_utc_ms() - timer);
            timer = get_utc_ms();

        }

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
    return os_task_create(uvc_h264_recv_task, NULL, 15, 1024, 32, "uvc_h264_recv");
}

int video2_uvc_h264_stream_close()
{
    while (OS_TASK_NOT_EXIST != os_task_del_req((const char *)"uvc_h264_recv")) {
        os_taskq_post_msg((const char *)"uvc_h264_recv", 1,  0xaa);
        os_time_dly(1);
    }
}



