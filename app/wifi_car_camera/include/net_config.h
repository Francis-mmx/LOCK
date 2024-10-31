#ifndef _NET_CONFIG_H__
#define _NET_CONFIG_H__
#include "app_config.h"

#if (defined CONFIG_WIFI_ENABLE)

/*------------WIFI模块配置----------*/
#ifdef CONFIG_USB_WIFI_ENABLE
#define USE_RTL8188
/*#define USE_RTL8192*/
#else
#define USE_RTL8189
/*#define USE_MARVEL8801*/
/*#define USE_S9082*/
#endif

//#define USE_5G2G_TOW_CHANNEL	//2.4G 5G双频模式

/*------------网络配置----------*/
#define ACCESS_NUM 				1
#define UUID 					"f2dd3cd7-b026-40aa-aaf4-"
#define WIFI_CAM_NAME_MATCH 	"wifi_camera_"
#define WIFI_CAM_WIFI_PWD    	"12345678"
#define CONFIG_ENABLE_VLIST
#define CONFIG_IPERF_ENABLE  				// iperf测试
#define CONFIG_RTSP_SERVER_ENABLE
//#define CONFIG_RTSP_CLIENT_ENABLE

#ifdef __CPU_AC521x__
#define WIFI_CAM_PREFIX    "wifi_camera_ac52_"
#else
#define WIFI_CAM_PREFIX    "wifi_camera_ac54_"
#endif
//#define CONFIG_FORCE_RESET_VM  //檫除vm所有配置
//#define CONFIG_STA_AUTO_MODE  //STA模式连接失败自动切换AP模式

/*-----------编码和网络包配置----------*/
#ifdef __CPU_AC521x__
#define CONFIG_NET_JPEG //DV15只打开JPEG
#else
#define CONFIG_NET_H264 //DV16打开JPEG和H264
#endif

// #define CONFIG_NET_TCP_ENABLE
#define CONFIG_NET_UDP_ENABLE
//#define CONFIG_NET_USR_ENABLE


#ifdef CONFIG_NET_JPEG
#define CONFIG_NET_PKG_JPEG //DV15只打开JPEG
#endif
#ifdef CONFIG_NET_H264
#define CONFIG_NET_PKG_H264 //DV16打开JPEG和H264
#endif

#ifdef CONFIG_NET_USR_ENABLE
#define NET_USR_PATH 		"192.168.1.1:8000"//需要打开CONFIG_NET_UDP_ENABLE，关闭CONFIG_NET_TCP_ENABLE和CONFIG_NET_UDP_ENABLE 有效
#endif

//#define CONFIG_NET_SCR  /*网络投屏*/
//#define CONFIG_EG_HTTP_SERVER /*enable eg_http_server*/
// #define CONFIG_EG_HTTP_CLIENT /*enable_eg_http_client only test*/

/*-------------网络端口----------------*/
#define CTP_CTRL_PORT   	3333
#define CDP_CTRL_PORT   	2228
#define NET_SCR_PORT    	2230
#define VIDEO_PREVIEW_PORT 	2226
#define VIDEO_PLAYBACK_PORT 2223
#define HTTP_PORT           8080
#define RTSP_PORT           554

/*--------视频库内使用----------------*/
#define _DUMP_PORT          2229
#define _FORWARD_PORT    	2224
#define _BEHIND_PORT     	2225


/*------------ RF模块配置-------------*/
// #define CONFIG_RTC676X_ENABLE
// #define CONFIG_A7130_ENABLE
//#define CONFIG_P2418_ENABLE
// #define CONFIG_A7196_ENABLE


/*--------录像和实时流帧率配置--------*/
//录卡和实时流的音频采样率,注意：硬件没有接咪头需要设置为0
#define VIDEO_REC_AUDIO_SAMPLE_RATE		8000

//录卡前后视帧率设置
#define  LOCAL_VIDEO_REC_FPS 			25  //不使用0作为默认值，写具体数值

#if (__SDRAM_SIZE__ <= (2 * 1024 * 1024))
#undef LOCAL_VIDEO_REC_FPS
#define  LOCAL_VIDEO_REC_FPS            20
#endif

//实时流BUFF允许缓存帧数(延时大则需要打开，例如录像中720P),写0无效,注意：该宏大于1时，会引起实时流实际帧率比配置低，但是实时性好
#define  NET_VIDEO_BUFF_FRAME_CNT		2

//实时流前后视帧率设置
#define  NET_VIDEO_REC_FPS0   			20  //不使用0作为默认值，写具体数值
#define  NET_VIDEO_REC_FPS1   			25   //不使用0作为默认值，写具体数值

//录像模式实时流丢帧配置(单路720录像且实时流720或双路录像有效),只能配置以下3个宏数值>=1其中一个,配置必须小于等于摄像头输出帧率,全0则输出录像帧率
#define  NET_VIDEO_REC_DROP_REAl_FP		15	//(实际输出帧率，单路720录像且实时流720或双路录像或SFC 2M版本录像有效,写0无效,不录像请用NET_VIDEO_REC_FPS0,NET_VIDEO_REC_FPS1)
#define  NET_VIDEO_REC_LOSE_FRAME_CNT	0	//(每隔n帧丢一帧,单路720录像且实时流720或双路录像或SFC 2M版本录像有效,写0无效)30p摄像头,需求>=15p/s则配置>=1;25p摄像头,需求>=12p/s则配置>=1.
#define  NET_VIDEO_REC_SEND_FRAME_CNT	0	//(每隔n帧发一帧,单路720录像且实时流720或双路录像或SFC 2M版本录像有效,写0无效)

//RTSP实时流帧率设置
#define  STRM_VIDEO_REC_FPS0   			20  //不使用0作为默认值，写具体数值
#define  STRM_VIDEO_REC_DROP_REAl_FP 	15  //(实际输出帧率，单路720录像且实时流720或双路录像有效或SFC 2M版本录像,写0无效)

//RTSP实时流BUFF允许缓存帧数(延时大则需要打开，例如录像中720P),写0无效,注意：该宏大于1时，会引起实时流实际帧率比配置低，但是实时性好
#define  STRM_VIDEO_BUFF_FRAME_CNT		2
/*--------------------------------------*/





#endif
#endif

