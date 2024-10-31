#include "common/common.h"
#include "server/rt_stream_pkg.h"
#include "lwip.h"
#include "stream_core.h"
#include "sock_api/sock_api.h"

/**************************************************************************************************************
1、打开实时流方法一：修改ctp_cmd.c int user_open_rt_stream(void)函数

	打开：
	前视：it.action = ACTION_VIDEO0_OPEN_RT_STREAM;或 后视：it.action = ACTION_VIDEO1_OPEN_RT_STREAM;

	分辨率：
	info.width = 640;
	info.height = 480;

	帧率：配置在net_config.h NET_VIDEO_REC_FPS0或NET_VIDEO_REC_FPS1（前后视）

	关闭：
	it.action = ACTION_VIDEO0_CLOSE_RT_STREAM; 或it.action = ACTION_VIDEO1_CLOSE_RT_STREAM;

	打开实时流方法二：调用net_video_rec.c的int user_net_video_rec_open(char forward)函数打开实时流。

2、修改自定义路径，net_config.h打开CONFIG_NET_UDP_ENABLE，改写NET_USR_PATH的自定义IP和端口号，必须有IP和端口号

3、调用 : int user_open_rt_stream(void)函数或者user_net_video_rec_open()函数，会回调到video_rt_usr.c的video_rt_usr_init()函数，
	在此函数进行所需要的socket初始化，具体可以参考video_rt_tcp.c和video_rt_udp.c。

4、当打开video_rt_usr_init()函数返回非NULL，打开完成之后，有数据会调用到video_rt_usr_send()函数，
	type的类型是JPEG_TYPE_VIDEO则为视频帧，PCM_TYPE_AUDIO则为音频。

5、关闭实时流方法一：ctp_cmd.c下调用user_close_rt_stream()函数，回调到video_rt_usr_uninit()，在此video_rt_usr_uninit()函数
	进行socket注销，具体可以参考video_rt_tcp.c和video_rt_udp.c。
	关闭实时流方法二：调用net_video_rec.c的int user_net_video_rec_close(char forward)
6、自定义拍照详情：net_video_rec.c ：int user_net_video_rec_take_photo_test(void)
**************************************************************************************************************/


static void *video_rt_usr_init(const char *path, const char *mode)
{
    //连接服务器操作
    int ret = 0;
    puts("video_rt_usr_init\n");

    struct rt_stream_info *r_info = calloc(1, sizeof(struct rt_stream_info));
    if (r_info == NULL) {
        printf("%s malloc fail\n", __FILE__);
        return NULL;
    }


    return (void *)r_info;
}

static u32 video_rt_usr_send(void *hdr, u8 *data, u32 len, u8 type)
{
    struct rt_stream_info *r_info = (struct rt_stream_info *)hdr;
    int ret;


    if (type == JPEG_TYPE_VIDEO) {

        printf("video_rt_usr_send one frame %dK \r\n", len / 1024);
    }

    return len;

}


void video_rt_usr_uninit(void *hdr)
{
    puts("video_rt_usr_uninit\n\n\n\n\n\n");
    struct rt_stream_info *r_info = (struct rt_stream_info *)hdr;
    free(r_info);

}

REGISTER_NET_VIDEO_STREAM_SUDDEV(usr_video_stream_sub) = {
    .name = "usr",
    .open =  video_rt_usr_init,
    .write = video_rt_usr_send,
    .close = video_rt_usr_uninit,
};





//int video_rt_usr_server_init(int port)
//{
//    int ret = 0;
//    puts("video_rt_usr_server_init\n");
//
//    return 0;
//
//}
//void video_rt_usr_server_uninit()
//{
//    printf("video_rt_usr_server_uninit\n");
//}



