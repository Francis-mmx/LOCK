#ifndef __LCD_CONFIG_H__
#define __LCD_CONFIG_H__

#include "app_config.h"
#include "device/lcd_driver.h"

#if (defined LCD_AVOUT)

#define NTSC_720		0
#define PAL_720			1
#define NTSC_960        2
#define PAL_960         3
#define AVOUT_MODE      PAL_720

#define RTC_32K		    0	/*RTC挂32K晶振*/
#define OSC0_12M	    1   /*PD0 PD1挂12M晶振*/
#define OSC1_12M	    2   /*PG6 PG7挂12M晶振*/
#define RTC_12M		    3   /*RTC挂12M晶振*/
#define AVOUT_OSC	    RTC_32K
#endif

/*
 *TIPS:由于avout对时钟的要求比较高,想要比较好的效果,建议使用12M的晶振
 * */
#ifndef MULTI_LCD_EN

#if (defined LCD_AVOUT)

#if (AVOUT_MODE == NTSC_720)
#define LCD_DEV_WIDTH	720
#define LCD_DEV_HIGHT	480
#elif (AVOUT_MODE == PAL_720)
#define LCD_DEV_WIDTH	720
#define LCD_DEV_HIGHT	576
#elif (AVOUT_MODE == NTSC_960)
#define LCD_DEV_WIDTH	960
#define LCD_DEV_HIGHT	480
#elif (AVOUT_MODE == PAL_960)
#define LCD_DEV_WIDTH	960
#define LCD_DEV_HIGHT	576
#endif

#endif

#if (defined LCD_AVOUT_HD)
#define LCD_DEV_WIDTH 1280
#define LCD_DEV_HIGHT 720
#endif


#if (defined LCD_DSI_DCS_1LANE_16BIT) ||\
    (defined LCD_DSI_DCS_1LANE_24BIT)
#define LCD_DEV_WIDTH	640
#define LCD_DEV_HIGHT	360
#endif


#if (defined LCD_DSI_DCS_2LANE_24BIT)
#define LCD_DEV_WIDTH	480
#define LCD_DEV_HIGHT	854
#endif


#if (defined LCD_DSI_DCS_4LANE_1080P)
/*插值*/
#define LCD_DEV_WIDTH	1280//1920
#define LCD_DEV_HIGHT	720//1080
#endif

#if (defined LCD_DSI_DCS_4LANE_720P)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif

#if (defined LCD_DSI_VDO_4LANE_720P_RM68200GA1)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif

#if (defined LCD_480x272) || \
    (defined LCD_480x272_8BITS)
#define LCD_DEV_WIDTH	480
#define LCD_DEV_HIGHT	272
#endif

#if (defined LCD_DSI_VDO_4LANE_MIPI) || \
    (defined LCD_DSI_VDO_2LANE_MIPI_EK79030) || \
    (defined LCD_DSI_VDO_4LANE_MIPI_EK79030) || \
    (defined LCD_DSI_VDO_4LANE_MIPI_ICN9706)
#if (__SDRAM_SIZE__ >= 16 * 1024 * 1024)
#define LCD_DEV_WIDTH	1152 /* 1280 */
//#if (DOUBLE_720 == 1)
//#define LCD_DEV_HIGHT   720
//#else//
#define LCD_DEV_HIGHT	432 /* 480 */
//#endif
// #define LCD_DEV_WIDTH	1280
// #define LCD_DEV_HIGHT	480
#else
#define LCD_DEV_WIDTH	480
#define LCD_DEV_HIGHT	272
#endif
#endif

#if (defined LCD_DSI_VDO_2LANE_MIPI_ST7701S)
#if (defined CONFIG_CAMERA_EFFECT_ENABLE)
#define LCD_DEV_WIDTH	320
#define LCD_DEV_HIGHT	240
#else
#if (__SDRAM_SIZE__ >= 16 * 1024 * 1024)
// #if (__SDRAM_SIZE__ > 16 * 1024 * 1024)
#define LCD_DEV_WIDTH	800
#define LCD_DEV_HIGHT	480
#else
#define LCD_DEV_WIDTH	480
#define LCD_DEV_HIGHT	272
// #define LCD_DEV_WIDTH	320
// #define LCD_DEV_HIGHT	240
#endif
#endif
#endif

#ifdef LCD_DSI_VDO_VISUAL_LCD_FOR_PIP
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif

#if (defined LCD_HX8369A_MCU)|| \
    (defined LCD_HX8369A_SPI_RGB)
#define LCD_DEV_WIDTH	800
#define LCD_DEV_HIGHT	480
#endif

#if (defined LCD_ST7789S_MCU)
#define LCD_DEV_WIDTH	320
#define LCD_DEV_HIGHT	240
#endif


#if (defined LCD_MIPI_4LANE_S8500L0)
/*插值*/
#define LCD_DEV_WIDTH	864//1296(被16整除)
#define LCD_DEV_HIGHT	224//336（被16整除）
#endif


#if (defined LCD_DSI_VDO_4LANE_1280x400)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	400
#endif

#if (defined LCD_LX50FWB4001_V2)
#define LCD_DEV_WIDTH	480
#define LCD_DEV_HIGHT	856
#endif

#if (defined LCD_DSI_VDO_4LANE_1600x400_WTL098802G01_1)
#define LCD_DEV_WIDTH	1600
#define LCD_DEV_HIGHT	400
#endif

#ifdef MIPI_LCD_800X480_BST40_L5
/* #define LCD_DEV_WIDTH   480 */
/* #define LCD_DEV_HIGHT   272 */
#define LCD_DEV_WIDTH   800
#define LCD_DEV_HIGHT   480

#endif

#ifdef CONFIG_DISPLAY_ENABLE
#ifndef LCD_DEV_WIDTH
#error lcd isn't definition!!!
#endif
#else
#define LCD_DEV_WIDTH	0
#define LCD_DEV_HIGHT	0
#endif

#else

extern u16 lcd_get_width();
extern u16 lcd_get_height();
#define LCD_DEV_WIDTH lcd_get_width()
#define LCD_DEV_HIGHT lcd_get_height()

#endif //MULTI_LCD_EN

#endif // __LCD_NFIG_HH


