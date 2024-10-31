#ifndef __NET_SCR_API_H__
#define __NET_SCR_API_H__

#include "sock_api/sock_api.h"
#include "server/rt_stream_pkg.h"



struct __JPG_INFO {
    u32 src_w;
    u32 src_h;
    u32 buf_len;
    u8  buf[];
};

struct __NET_SCR_CFG {
    struct sockaddr_in cli_addr;
    int fps;
    int src_w;
    int src_h;
};

struct __NET_SCR_INFO {
    u8 state;
    int pid;
    void *video_dec;
    void *sock_hdl;
    struct __NET_SCR_CFG  cfg;

};
int net_scr_dec(struct __JPG_INFO *info);
u8 get_net_scr_status(void);
int net_scr_init(struct __NET_SCR_CFG *cfg);
int net_scr_uninit(struct __NET_SCR_CFG *cfg);

#endif

