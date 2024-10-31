/*****************************************************************
>file name : uvc_device.h
>author : lichao
>create time : Sat 02 Sep 2017 03:22:12 PM HKT
*****************************************************************/
#ifndef _UVC_DEVICE_H_
#define _UVC_DEVICE_H_
#include "device/video.h"
#include "device/camera.h"

#define UVC_CMD_BASE        0x00010000

#define UVC_GET_CUR_FPS         (UVC_CMD_BASE + 1)
#define UVC_GET_CUR_BITS_RATE   (UVC_CMD_BASE + 2)

enum trans_mode {
    UVC_PUSH_PHY_MODE,
    UVC_PUSH_VIRTUAL_MODE,
};

struct usb_camera_info {
    u16 width;
    u16 height;
    int fps;
    int sample_fmt;
    enum trans_mode  mode;
};

int uvc_get_src_pixformat(void *fh);
int uvc_set_output_buf(void *fh, void *buf, int size);

void *uvc_output_open(u8 mijor, struct camera_device_info *info);
int uvc_output_set_fmt(void *fh, struct video_format *f);
int uvc_get_real_fps(void *fh);
int uvc_output_one_frame(void *fh);
int uvc_output_start(void *fh);
int uvc_output_stop(void *fh);
int uvc_output_close(void *fh);
int uvc_set_output_buffer(void *fh, void *buf, int num);
int uvc_set_scaler_handler(void *fh, void *priv, int (*handler)(void *, struct YUV_frame_data *));
int uvc_output_set_reso(void *fh, struct video_format *f, struct camera_device_info *info);
#endif
