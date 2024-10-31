#include "generic/typedef.h"
#include "asm/imd.h"
#include "asm/imb_driver.h"
#include "asm/lcd_config.h"
#include "asm/avo_cfg.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_AVOUT


REGISTER_IMD_DEVICE(lcd_avout_dev) = {
    .info = {
        .test_mode       = false,
        .test_mode_color = 0x008080,
        .background_color = 0x000000,
        .interlaced_mode = INTERLACED_ALL,
#if   (AVOUT_MODE == NTSC_720)
        .interlaced_1st_filed = ODD_FILED,
#else
        .interlaced_1st_filed = EVEN_FILED,
#endif
#ifdef MULTI_LCD_EN
#if (AVOUT_MODE == NTSC_720)
        .xres 			 = 720,
        .yres 			 = 480,
        .target_xres     = 720,
        .target_yres     = 480,

#elif (AVOUT_MODE == PAL_720)
        .xres 			 = 720,
        .yres 			 = 576,
        .target_xres     = 720,
        .target_yres     = 576,
#elif (AVOUT_MODE == NTSC_960)
        .xres 			 = 960,
        .yres 			 = 480,
        .target_xres     = 960,
        .target_yres     = 480,
#elif (AVOUT_MODE == PAL_960)
        .xres 			 = 960,
        .yres 			 = 576,
        .target_xres     = 960,
        .target_yres     = 576,
#endif
#else
        .xres 			 = LCD_DEV_WIDTH,
        .yres 			 = LCD_DEV_HIGHT,
        .target_xres     = LCD_DEV_WIDTH,
        .target_yres     = LCD_DEV_HIGHT,
#endif


        .format          = FORMAT_YUV422,

        .len 			 = LEN_256,
        .rotate          = ROTATE_0,

        .adjust = {
#if   (AVOUT_MODE == NTSC_720)
            .y_gain = 0x100 * 0.9,
            .u_gain = 0x100 * 1.2,
            .v_gain = 0x100 * 1.2,
            .y_offs = -26,
            .u_offs = 0,
            .v_offs = 0,
            .r_gain = 0x80,
            .g_gain = 0x80,
            .b_gain = 0x80,
            .r_coe0 = 0x80,
            .g_coe1 = 0x80,
            .b_coe2 = 0x80,
            .r_gma  = 100,
            .g_gma  = 100,
            .b_gma  = 100,
#else
            .y_gain = 0x100 * 0.90,
            .u_gain = 0x100,
            .v_gain = 0x100,
            .y_offs = 16,
            .u_offs = 0,
            .v_offs = 0,
            .r_gain = 0x80,
            .g_gain = 0x80,
            .b_gain = 0x80,
            .r_coe0 = 0x80,
            .g_coe1 = 0x80,
            .b_coe2 = 0x80,
            .r_gma  = 100,
            .g_gma  = 100,
            .b_gma  = 100,
#endif
        },
    },

    .drive_mode      = MODE_AVOUT,

    /*
     *  TIPS:由于avout对时钟的要求比较高,想要比较好的效果,建议使用12M的晶振(在PG6 PG7处挂12M晶振,AVOUT_OSC选OSC1_12M)
     */
#if (AVOUT_OSC == OSC1_12M)
#if (AVOUT_MODE == NTSC_720)
    .avout_mode 	 = CVE_NTSC_720 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC1_12M | PLL0_CLK | DIVA_5 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2700,/* 2700*12M/60 = 540M */
#elif (AVOUT_MODE == PAL_720)
    .avout_mode 	 = CVE_PAL_720 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC1_12M | PLL0_CLK | DIVA_5 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2700,/* 2700*12M/60 = 540M */
#elif (AVOUT_MODE == NTSC_960)
    .avout_mode 	 = CVE_NTSC_960 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC1_12M | PLL0_CLK | DIVA_3 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2160,/* 2160*12M/60 = 432M */
#elif (AVOUT_MODE == PAL_960)
    .avout_mode 	 = CVE_PAL_960 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC1_12M | PLL0_CLK | DIVA_3 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2160,/* 2160*12M/60 = 432M */
#endif

#elif (AVOUT_OSC == OSC0_12M)
#if (AVOUT_MODE == NTSC_720)
    .avout_mode 	 = CVE_NTSC_720 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC0_12M | PLL0_CLK | DIVA_5 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2700,/* 2700*12M/60 = 540M */
#elif (AVOUT_MODE == PAL_720)
    .avout_mode 	 = CVE_PAL_720 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC0_12M | PLL0_CLK | DIVA_5 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2700,/* 2700*12M/60 = 540M */
#elif (AVOUT_MODE == NTSC_960)
    .avout_mode 	 = CVE_NTSC_960 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC0_12M | PLL0_CLK | DIVA_3 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2160,/* 2160*12M/60 = 432M */
#elif (AVOUT_MODE == PAL_960)
    .avout_mode 	 = CVE_PAL_960 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | XOSC0_12M | PLL0_CLK | DIVA_3 | DIVB_1,
    .pll0_nf         = 60,
    .pll0_nr         = 2160,/* 2160*12M/60 = 432M */
#endif

#else//(AVOUT_OSC == RTC_32K)
#if (AVOUT_MODE == NTSC_720)
    .avout_mode 	 = CVE_NTSC_720 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL0_CLK | DIVA_5 | DIVB_1,
    .pll0_nf         = 1,
    .pll0_nr         = 16480,/* 16480*0.032768M = 540M */
#elif (AVOUT_MODE == PAL_720)
    .avout_mode 	 = CVE_PAL_720 /* | CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL0_CLK | DIVA_5 | DIVB_1,
    .pll0_nf         = 1,
    .pll0_nr         = 16480,/* 16480*0.032768M = 540M */
#elif (AVOUT_MODE == NTSC_960)
    .avout_mode 	 = CVE_NTSC_960 /* |CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL0_CLK | DIVA_3 | DIVB_1,
    .pll0_nf         = 1,
    .pll0_nr         = 13184,/* 13184*0.032768M = 432M */
#elif (AVOUT_MODE == PAL_960)
    .avout_mode 	 = CVE_PAL_960 /* | CVE_COLOR_BAR */,
    .clk_cfg    	 = IMD_INTERNAL | OSC_32K | PLL0_CLK | DIVA_3 | DIVB_1,
    .pll0_nf         = 1,
    .pll0_nr         = 13184,/* 13184*0.032768M = 432 */
#endif
#endif
    .ncycle          = CYCLE_ONE,
    .dclk_cfg        = CLK_DIS | CLK_NEGATIVE,
    .sync0_cfg       = CLK_DIS | SIGNAL_TYPE_DEN | CLK_NEGATIVE,
    .sync1_cfg       = CLK_DIS | SIGNAL_TYPE_HSYNC | CLK_NEGATIVE,
    .sync2_cfg       = CLK_DIS | SIGNAL_TYPE_VSYNC | CLK_NEGATIVE,
    .data_width      = PORT_1BIT,

    .timing = {
#if (AVOUT_MODE == NTSC_720)
        //NTSC 27MHz 720x480
        .hori_total 		    = 858 * 2,
        .hori_sync 		        = 138 * 2,
        .hori_back_porth 	    = 138 * 2,
        .hori_pixel 	        = 720,

        .vert_total 		    = 263,
        .vert_sync 		        = 21,
        .vert_back_porth_odd 	= 21,//cve2_wr(51,21);
        .vert_back_porth_even 	= 22,
        .vert_pixel 	        = 240,
#elif (AVOUT_MODE == NTSC_960)
        //NTSC 36MHz 960x480
        .hori_total		 		= 1144 * 2,
        .hori_sync 				= 182 * 2,
        .hori_back_porth	 	= 182 * 2,
        .hori_pixel 			= 960,

        .vert_total 			= 263,
        .vert_sync 				= 21,
        .vert_back_porth_odd 	= 21,
        .vert_back_porth_even 	= 22,
        .vert_pixel 			= 240,
#elif (AVOUT_MODE == PAL_720)
        //PAL 27MHz 720x576
        .hori_total				= 864 * 2,
        .hori_sync              = 138 * 2,
        .hori_back_porth 	    = 138 * 2,
        .hori_pixel 	        = 720,

        .vert_total 		    = 313,
        .vert_sync 		        = 21,
        .vert_back_porth_odd 	= 23,//cve2_wr(51,21);
        .vert_back_porth_even 	= 24,
        .vert_pixel 	        = 288,
#elif (AVOUT_MODE == PAL_960)
        //PAL 36MHz 960x576
        .hori_total 		    = 1152 * 2,
        .hori_sync 	            = 192 * 2,
        .hori_back_porth 	    = 192 * 2,
        .hori_pixel 		    = 960,

        .vert_total 		    = 313,
        .vert_sync 	            = 21,
        .vert_back_porth_odd    = 23,//cve2_wr(51,21);
        .vert_back_porth_even   = 24,
        .vert_pixel 		    = 288,
#endif
    },
};


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .logo = "lcd_avout",
    .init = NULL,
    .dev  = (void *) &lcd_avout_dev,
    .type = LCD_DVP_RGB,
    .bl_ctrl = NULL,
};
#endif
