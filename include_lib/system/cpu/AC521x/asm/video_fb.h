#ifndef __ASM_FBDEV_H
#define __ASM_FBDEV_H

#include "device/video.h"


#define FBDEV_FULL_SCREEN_FIRST 1
#define FBDEV_FULL_IMAGE_FIRST  2


struct video_fb_s_attr {
    u8  rotate;
    u8  imr_id;
    u16 width;
    u16 height;

    u16 src_left;
    u16 src_top;
    u16 src_width;
    u16 src_height;
};

struct fb_layer {
    int num;
    void *fb;
} now_fb;
void *video_fb_open(struct video_format *, int, int);
int video_fb_get_map(void *, struct fb_map_user *map);
int video_fb_put_map(void *, struct fb_map_user *map);
int video_fb_close(void *fb);
int video_fb_get_pingpong_buffer(void *_hdl, int id, struct fb_map_user *map);
int video_fb_get_s_attr(void *_hdl, struct video_fb_s_attr *attr);

int video_fb_set_event_handler(void *_hdl,
                               int (*handler)(void *, enum fb_event), void *priv);

int video_fb_map_num(void *_hdl);
int video_fb_release_map(void *_hdl);
int video_fb_set_map(void *_hdl);

int video_fb_get_pingpong_buffer_man(void *_hdl, struct fb_map_user *map, u8 flag);
void video_fb_set_standstill(void *_hdl, u32 standstill);

int video_fb_set_disp_win(void *_hdl, struct video_window *win);
#endif
