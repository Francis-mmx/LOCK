#ifndef APP_CONFIG_H
#define APP_CONFIG_H



#define CONFIG_BOARD_5211B_DEV_20181116



#ifdef CONFIG_BOARD_5211B_DEV_20181116
#define __CPU_AC521x__
#define __SDRAM_SIZE__    (8 * 1024 * 1024)
#define __FLASH_SIZE__    (2 * 1024 * 1024)
// #define __PSRAM_SIZE__    (8 * 1024 * 1024)
// #define CONFIG_PAP_ENABLE                       [> PAP模块使能  <]
#define CONFIG_SD0_ENABLE                       /* SD卡选择 */
//#define CONFIG_SFC_ENABLE
// #define LCD_AVOUT
// #define LCD_DSI_VDO_4LANE_MIPI_EK79030
// #define LCD_DSI_VDO_4LANE_YZN101IBGG2001
#define LCD_DSI_DCS_2LANE_24BIT
#define CONFIG_UI_ENABLE                        /* UI开关 */

//#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
// #define CONFIG_UI_STYLE_JL02_ENABLE
// #define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
 #define CONFIG_UI_STYLE_JL02_ENABLE
 #define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关  <]
 #define CONFIG_GT911                          [> 触摸屏选择 <]
 #define CONFIG_VIDEO2_ENABLE 		            [> usb后摄像头使能 <]
// #define CONFIG_PSRAM_ENABLE                     [>外挂psram存放vidoebuf<]
// #define CONFIG_CAMERA_EFFECT_ENABLE				// 屏显分辨率最大320*240
//#define SCREEN_PROTECTION                       /*屏保开关*/
#endif



//#define CONFIG_DATABASE_2_RTC                   /* 系统配置存RTC */
#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
// #define CONFIG_PAP_ENABLE                       [> PAP模块使能  <]
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
#define CONFIG_VIDEO_DEC_ENABLE                 /* 视频解码显示  */
#define CONFIG_ADKEY_ENABLE                     /* AD按键开关  */
#define CONFIG_IOKEY_ENABLE                     /* IO按键开关  */
//#define CONFIG_GSENSOR_ENABLE                   /* 重力感应开关  */
//#define KEY_MODE_EXIT_MENU                      /* UI菜单MODE键退出  */
// #define CONFIG_EMR_DIR_ENABLE                   [> 紧急录像文件放在单独文件夹 <]
#define CONFIG_DISPLAY_ENABLE                   /* 摄像头显示使能 */


/*CHIP TEST FUNCTIONS*/
// #define CONFIG_CT_AUDIO_TEST					//ADC DAC测试


#define CONFIG_VE_MOTION_DETECT_MODE_ISP		/* 移动侦测使用MODE_ISP, 否则使用MODE_NORMAL */
#if __SDRAM_SIZE__ >= (16 * 1024 * 1024)
/**
 *有关双720p宏定义选择:
 *DOUBLE_720、 DOUBLE_720_SMALL_SCR 两个宏，二选一，根据当前屏幕使用分辨率决定使用对应宏.
 *屏幕分辨率720p则选择宏DOUBLE_720,  屏幕分辨率小于720P  则选择 DOUBLE_720_SMALL_SCR.
 *
 * ***/
// #define DOUBLE_720 1							//双路720使能 目前仅适用于前路dvp720, 后拉AHD720 ,屏幕分辨率 720*1280
#define DOUBLE_720 0							//关闭双路720使能

#define DOUBLE_720_SMALL_SCR   0            //关闭双路720使能
//#define DOUBLE_720_SMALL_SCR   1               //双路720使能 目前仅适用于前路dvp720, 后拉AHD720 ,屏幕分辨率 小于  720*1280


// #define CONFIG_LANE_DETECT_ENABLE               [> 车道检测使能 <]
#endif

/**
 * PHOTO_STICKER_ENABLE 该宏定义仅支持拍照大头贴
 * PHOTO_STICKER_ENABLE_SMALL_MEM 该宏定义支持拍照大头贴,录像大头贴(如使用2M内存版本芯片，需要跑 sfc模式，使能CONFIG_SFC_ENABLE即可)
 * 两个宏二选一
 * */
// #define PHOTO_STICKER_ENABLE    				[> 大头贴功能，默认关闭 <]
// #define PHOTO_STICKER_ENABLE_SMALL_MEM  		[> 录像与拍照功能添加大头贴功能，默认关闭 <]

//#define FACE_DECT_ENABLE                       /*人脸检测使能*/ //使能后需要修改UI工程

#ifdef PHOTO_STICKER_ENABLE_SMALL_MEM
#define STICKER_NUM   3
#define STICKER_NAME		"st%06d.jpg"
#define STICKER_PATH      "mnt/spiflash/audlogo/"STICKER_NAME
#endif


#ifdef  CONFIG_UI_ENABLE
#ifndef PHOTO_STICKER_ENABLE_SMALL_MEM
// #define CONFIG_FILE_PREVIEW_ENABLE              [> 预览图使能 <]
#endif
#endif


// #define CONFIG_PWM_ENABLE						[> 电机PWM功能 <]
#if __SDRAM_SIZE__ >= (8 * 1024 * 1024)
#define CONFIG_OSD_LOGO							/* 录像OSD彩色水印功能开关 */
#endif

/* #define DIGITAL_SCALE */                          //数字变焦使能

//#define MP4_PACKAGE_ENABLE						/* MP4封装使能, 默认关闭 */

#if defined CONFIG_VIDEO1_ENABLE
#define CONFIG_PARK_ENABLE
#ifdef CONFIG_VIDEO2_ENABLE
#define	CONFIG_VIDEO_PARK_DECT		3
#else
#define	CONFIG_VIDEO_PARK_DECT		1
#endif
#else
#if defined CONFIG_VIDEO2_ENABLE
#define CONFIG_PARK_ENABLE
#define	CONFIG_VIDEO_PARK_DECT		3
#else
#define	CONFIG_VIDEO_PARK_DECT		0
#endif
#endif


#ifdef CONFIG_NET_ENABLE
#define CONFIG_WIFI_ENABLE  					/* 无线WIFI */
#endif

#ifdef CONFIG_VIDEO1_ENABLE
//#define CONFIG_AV10_SPI_ENABLE                  /* AV10 SPI开关  */
#endif


#ifdef CONFIG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd0"
#define SDX_DEV					"sd0"
#endif

#ifdef CONFIG_SD1_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd1"
#define SDX_DEV					"sd1"
#endif

#ifdef CONFIG_SD2_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd2"
#define SDX_DEV					"sd2"
#endif

#define CONFIG_REC_DIR_0       "DCIM/1/"
#define CONFIG_REC_DIR_1       "DCIM/2/"
#ifndef CONFIG_VIDEO1_ENABLE
#define CONFIG_REC_DIR_2       "DCIM/2/"
#else
#define CONFIG_REC_DIR_2       "DCIM/3/"
#endif


#define CONFIG_ROOT_PATH     	CONFIG_STORAGE_PATH"/C/"
#define CONFIG_REC_PATH_0       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_0
#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_1
#define CONFIG_REC_PATH_2       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_2

#ifdef CONFIG_EMR_DIR_ENABLE

#define CONFIG_EMR_REC_DIR         "EMR/"
#define CONFIG_EMR_REC_DIR_0       "DCIM/1/"CONFIG_EMR_REC_DIR
#define CONFIG_EMR_REC_DIR_1       "DCIM/2/"CONFIG_EMR_REC_DIR
#ifndef CONFIG_VIDEO1_ENABLE
#define CONFIG_EMR_REC_DIR_2       "DCIM/2/"CONFIG_EMR_REC_DIR
#else
#define CONFIG_EMR_REC_DIR_2       "DCIM/3/"CONFIG_EMR_REC_DIR
#endif
#define CONFIG_EMR_REC_PATH_0   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_0
#define CONFIG_EMR_REC_PATH_1   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_1
#define CONFIG_EMR_REC_PATH_2   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_2

#endif


#define CAMERA0_CAP_PATH        CONFIG_REC_PATH_0
#define CAMERA1_CAP_PATH        CONFIG_REC_PATH_1
#define CAMERA2_CAP_PATH        CONFIG_REC_PATH_2

#define CONFIG_DEC_ROOT_PATH 	CONFIG_ROOT_PATH"DCIM/"
#define CONFIG_DEC_PATH_1 	    CONFIG_REC_PATH_0
#define CONFIG_DEC_PATH_2 	    CONFIG_REC_PATH_1
#define CONFIG_DEC_PATH_3 	    CONFIG_REC_PATH_2

#define CONFIG_MUSIC_PATH      CONFIG_ROOT_PATH"MUSIC/"

#define MAX_FILE_NAME_LEN       64
#define FILE_SHOW_NUM           12  /* 一页显示文件数 */

#if (defined CONFIG_VIDEO1_ENABLE) && (defined CONFIG_VIDEO2_ENABLE)
#define THREE_WAY_ENABLE		1
#define CONFIG_VIDEO_REC_NUM    3
#else
#define THREE_WAY_ENABLE		0
#define CONFIG_VIDEO_REC_NUM    3
#endif




#ifndef __LD__
#include "cpu_config.h"
#include "board.h"
#endif







#endif

