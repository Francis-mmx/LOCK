#include "system/includes.h"
#include "server/ui_server.h"
#include "action.h"
#include "ani_style.h"
#include "style.h"
#include "res_ver.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "asm/rtc.h"
#include "storage_device.h"

#ifdef CONFIG_WIFI_ENABLE
#include "server/ctp_server.h"
#include "server/net_server.h"
#include "net_config.h"
#include "eg_http_server.h"
#endif

#include "generic/log.h"



u32 spin_lock_cnt[2] = {0};

int upgrade_detect(const char *sdcard_name);


/*任务列表 */
const struct task_info task_info_table[] = {
#ifdef __CPU_AC521x__
    {"video_fb",            27,     1024,  1024  },
#else
    {"video_fb",            30,     1024,  1024  },
#endif
    {"ui_server",           29,     512,   1024  },
    {"ui_task",             30,     1200,  512   },
    {"init",                30,     720,   256   },
    {"sys_timer",           10,     512,   2048  },//qsize必须大于2048
    {"app_core",            22,     1500,  2048  },//提高APP响应优先级
    {"sys_event",           30,     512,   0     },
    {"video_server",        26,     1024,  256   },
    {"audio_server",        16,     512,   256   },
    {"audio_decoder",       17,     1024,  64    },
    {"audio_encoder",       17,     1024,  64    },
    {"speex_encoder",       17,     1024,  64    },
    {"amr_encoder",         17,     1024,  64    },
    {"aac_encoder",         17,     8192,  0     },
    {"video_dec_server",    27,     1024,  1024  },
    {"video0_rec0",         25,     256,   512   },
    {"video0_rec1",         19,     256,   512   },
    {"audio_rec0",          22,     256,   256   },
    {"audio_rec1",          19,     256,   256   },
    {"audio_rec2",          19,     256,   256   },
    {"audio_rec3",          19,     256,   256   },
    {"video1_rec0",         24,     256,   512   },
    {"video1_rec1",         19,     256,   512   },
    {"video2_rec0",         24,     256,   512   },
    {"video2_rec1",         19,     256,   512   },
    {"video5_rec0",         24,     256,   512   },
    {"video5_rec1",         19,     256,   512   },
    {"isp_update",          27,     256,   0     },
    {"vpkg_server",         26,     512,   512   },
    {"vunpkg_server",       23,     512,   128   },
    {"avi0",                29,     320,   64    },
    {"avi1",                29,     320,   64    },
    {"avi2",                29,     320,   64    },
    {"mov0",                28,     320,   64    },
    {"mov1",                28,     320,   64    },
    {"mov2",                28,     320,   64    },
    {"mov3",                28,     320,   64    },
    {"mp40",                28,     320,   64    },
    {"mp41",                28,     320,   64    },
    {"video_engine_server", 14,     320,   1024  },
    {"video_engine_task",   15,     256,   0     },
    {"usb_server",          20,     512,   128   },
    {"khubd",               25,     512,   32    },

    {"uvc_transceiver",     26,     320,   32    },
    {"uvc_transceiver1",    26,     1024,  32    },
    {"uvc_transceiver2",    26,     1024,  32    },
    {"custom_transceiver",  26,     2048,   32    },
    {"vimc_scale",          26,     2048,  32    },
    {"upgrade_server",      21,     1024,   32    },
    {"upgrade_core",        20,     800,   32    },
    {"dynamic_huffman0",    15,		512,   32    },
    {"dynamic_huffman1",    15,		512,   32    },
    {"video0_devbuf0",      27,     1024,  32    },
    {"video1_devbuf0",      27,     1024,  32    },
    {"video2_devbuf0",      27,     1024,  32    },
    {"video_dec_devbuf",    27,     1024,  32    },
    {"audio_dec_devbuf",    27,     1024,  32    },
    {"jpg_dec",             27,     1024,  32    },

    {"auto_test",			15,		1024,  1024  },
    {"fs_test",			    15,		1024,  0     },
    {"aac0_encoder",        16,     2028,  64    },
    {"aac1_encoder",        16,     2028,  64    },

    {"powerdet_task",       15,     1024,  1024  },
    {"sys_timer",           15,     512,   256  },

#ifdef CONFIG_WIFI_ENABLE
    {"ctp_server",          28,     600,   64    },
    {"tcpip_thread",        25,     1024,  0    },
    {"net_video_server",    16,     256,   64 	 },
    {"net_avi0",            18,     600,   600	 },
    {"net_avi1",            18,     600,   600	 },
    {"net_mov0",            18,     600,   600	 },
    {"net_mov1",            18,     600,   600	 },
    {"mov_unpkg0",          18,     600,   600	 },
    {"stream_avi0",         18,     320,   600	 },
    {"stream_avi1",         18,     320,   600	 },
    {"stream_mov0",         18,     320,   600	 },
    {"stream_mov1",         18,     320,   600	 },
#endif

    {0, 0},
};


#ifdef CONFIG_UI_ENABLE

#ifdef PHOTO_STICKER_ENABLE
void ani_play_end_notifice_to_ph_app(u8 end);
#endif
/*
 * 开机动画播放完毕
 */
static void animation_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;

#ifdef PHOTO_STICKER_ENABLE
    ani_play_end_notifice_to_ph_app(1);
#endif
    server_close(ui);

    /*
     * 显示完开机画面后更新配置文件,避免效果调节过度导致开机图片偏色
     */
    void *imd = dev_open("imd", NULL);

    if (imd) {
        dev_ioctl(imd, IMD_SET_COLOR_CFG, (u32)"scr_auto.bin"); /* 更新配置文件  */
        dev_close(imd);
    }

    /*
     *按键消息使能
     */
#ifdef CONFIG_PARK_ENABLE

    if (!get_parking_status())
#endif
    {
        sys_key_event_enable();
        sys_touch_event_enable();//使能触摸事件
    }
}

/*
 * 关机动画播放完毕, 关闭电源
 */
static void power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;
    u32 park_en;

    if (ui) {
        server_close(ui);
    }

#ifdef CONFIG_GSENSOR_ENABLE
    park_en = db_select("par");
    set_parking_guard_wkpu(park_en);
    sys_power_set_port_wakeup("wkup_gsen", park_en);
#endif
    sys_power_set_port_wakeup("wkup_usb", 1);
    sys_power_poweroff(0);
}
#endif

#ifdef CONFIG_WIFI_ENABLE
static int main_net_event_hander(struct sys_event *event)
{

#ifndef CONFIG_EG_HTTP_SERVER
    struct ctp_arg *event_arg = (struct ctp_arg *)event->arg;
#else
    struct eg_event_arg *event_arg = (struct eg_event_arg *)event->arg;
#endif
    struct net_event *net = &event->u.net;

    switch (net->event) {
    case NET_EVENT_CMD:
        /*printf("IN NET_EVENT_CMD\n\n\n\n");*/
#ifndef CONFIG_EG_HTTP_SERVER
        ctp_cmd_analysis(event_arg->topic, event_arg->content, event_arg->cli);
#else
        eg_cmd_analysis(event_arg->cmd, event_arg->content, event_arg->cli);
#endif

        if (event_arg->content) {
            free(event_arg->content);
        }

        event_arg->content = NULL;

        if (event_arg) {
            free(event_arg);
        }

        event_arg = NULL;

        return true;
        break;

    case NET_EVENT_DATA:
        /* printf("IN NET_EVENT_DATA\n"); */
        break;
    }

    return false;
}
#endif

#ifdef MULTI_LCD_EN
extern int ui_platform_init();
extern void *lcd_get_cur_hdl();
extern void lcd_set_cur_hdl(void *dev);
static u8 sw = 0;
u8 get_current_disp_device()
{
    return sw;
}
static void switch_lcd()
{
    struct intent it;
    struct application *app;

    void *lcd_dev = lcd_get_cur_hdl();
    app = get_current_app();
    init_intent(&it);
    if (app) {
        it.name = app->name;
        it.action = ACTION_BACK;
        start_app(&it);
    }
    if (lcd_dev) {
        dev_close(lcd_dev);
    }

    if (sw == 0) {
        lcd_dev = dev_open("lcd", "lcd_avout");
    } else {
        gpio_direction_output(IO_PORTF_07, 1);
        lcd_dev = dev_open("lcd", "bst40");
    }
    lcd_set_cur_hdl(lcd_dev);
    sw = !sw;

    ui_platform_init();
    if (app) {
        it.name = app->name;
        if (!strcmp(app->name, "video_rec")) {
            it.action = ACTION_VIDEO_REC_MAIN;
        } else if (!strcmp(app->name, "video_dec")) {
            it.action = ACTION_VIDEO_DEC_MAIN;
        } else if (!strcmp(app->name, "video_photo")) {
            it.action = ACTION_PHOTO_TAKE_MAIN;
        }
        start_app(&it);
    }
}
#endif
static int main_key_event_handler(struct key_event *key)
{
    struct intent it;
    struct application *app;

#ifdef CONFIG_UI_USE_TALK
    extern void adkey_crl_talk(void);
#endif

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
#ifdef MULTI_LCD_EN
        case KEY_F1:
            switch_lcd();/*双屏切换*/
            break;
#endif
        case KEY_MODE:
            init_intent(&it);
            app = get_current_app();

            if (app) {
                if (!strcmp(app->name, "usb_app")) {
                    break;
                }

                it.action = ACTION_BACK;
                start_app(&it);

                if (!strcmp(app->name, "video_rec")) {
                    it.name = "video_photo";
                    it.action = ACTION_PHOTO_TAKE_MAIN;
                } else if (!strcmp(app->name, "video_photo")) {
                    it.name = "video_dec";
                    it.action = ACTION_VIDEO_DEC_MAIN;
                } else if (!strcmp(app->name, "video_dec")) {
                    it.name = "video_rec";
                    it.action = ACTION_VIDEO_REC_MAIN;
                }

                start_app(&it);
            }

            break;

        case KEY_TALK:
#ifdef CONFIG_UI_USE_TALK
            adkey_crl_talk();
#endif
            break;

        default:
            return false;
        }

        break;


    case KEY_EVENT_LONG:
        if (key->value == KEY_POWER) {
            puts("---key_power\n");
            static u8 power_fi = 0;

            if (power_fi) {
                puts("re enter power off\n");
                break;
            }

            power_fi = 1;
            sys_key_event_disable();

            struct sys_event e;
            e.type = SYS_DEVICE_EVENT;
            e.arg = "sys_power";
            e.u.dev.event = DEVICE_EVENT_POWER_SHUTDOWN;
            sys_event_notify(&e);
        }

        break;

    default:
        return false;
    }

    return true;
}

extern u8 get_usb_in_status();
static int main_dev_event_handler(struct sys_event *event)
{
    struct intent it;
    struct application *app;

    init_intent(&it);
    app = get_current_app();

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
#ifdef CONFIG_UI_ENABLE
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
            if (app && strcmp(app->name, "usb_app") && strcmp(app->name, "sdcard_upgrade")) {
                it.action = ACTION_BACK;
                start_app(&it);

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }

#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            else if (!app) { //主界面进入usb界面
                union uireq req;
                struct server *ui;
                /* ui = server_open("ui_server", NULL); */
#ifdef MULTI_LCD_EN
                struct ui_style style = {0};
                if (get_current_disp_device()) {
                    style.file = "mnt/spiflash/res/avo_LY.sty\0";
                } else {
                    style.file = "mnt/spiflash/res/lcd_LY.sty\0";
                }
                ui = server_open("ui_server", &style);
#else
                ui = server_open("ui_server", NULL);
#endif

                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }

#endif
#ifdef CONFIG_PARK_ENABLE

            if (get_parking_status()) {
                sys_key_event_enable();
                sys_touch_event_enable();
            }

#endif
        }

#endif

#ifdef CONFIG_PARK_ENABLE

        if (!ASCII_StrCmp(event->arg, "parking", 7)) {
            if (app) {
                if (!strcmp(app->name, "video_rec")) {
                    break;
                }

                if ((!strcmp(app->name, "video_photo"))
                    || (!strcmp(app->name, "video_dec"))
                    || (!strcmp(app->name, "video_system"))
                    || (!strcmp(app->name, "usb_app"))) {

                    if (!strcmp(app->name, "usb_app")) {
                        if (get_usb_in_status()) {
                            puts("usb in status\n");
                            break;
                        }
                    }

                    it.action = ACTION_BACK;
                    start_app(&it);
                }

                puts("\n =============parking on eee video rec=========\n");
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }

#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            else if (!app) { //主界面进入倒车界面
                union uireq req;
                struct server *ui;
                ui = server_open("ui_server", NULL);
                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }

#endif

        }

#endif
        break;

    case DEVICE_EVENT_OUT:
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
            if (app && !strcmp(app->name, "usb_app")) {
                it.action = ACTION_BACK;
                start_app(&it);

#ifdef PHOTO_STICKER_ENABLE
                it.name	= "video_photo";
                it.action = ACTION_PHOTO_TAKE_MAIN;
#else
                it.name	= "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
#endif

                start_app(&it);
#ifdef CONFIG_PARK_ENABLE

                if (get_parking_status()) {
                    sys_key_event_disable();
                    sys_touch_event_disable();
                }

#endif
            }
        }

        break;

    case DEVICE_EVENT_CHANGE:
        break;

    case DEVICE_EVENT_POWER_SHUTDOWN:
        if (!strcmp(event->arg, "sys_power")) {
            u32 park_en;

            init_intent(&it);
            while (1) {
                //退出所有app
                app = get_current_app();
                if (app) {
#ifdef CONFIG_NET_ENABLE
                    if (!strcmp(app->name, "net_video_rec")) {
                        //断开所有客户端
                        ctp_srv_disconnect_all_cli();
                        cdp_srv_disconnect_all_cli();
                    }
#endif
                    it.action = ACTION_BACK;
                    start_app(&it);
                } else {
                    break;
                }
                os_time_dly(1);

            }

#ifdef CONFIG_UI_ENABLE
            struct ui_style style;
            style.file = "mnt/spiflash/audlogo/ani.sty";
            style.version = ANI_UI_VERSION;
            struct server *ui = server_open("ui_server", &style);

            if (ui) {
                union uireq req;

                if (get_parking_status()) {
                    puts("hide_park_ui\n");
                    req.hide.id = ID_WINDOW_PARKING;
                    server_request(ui, UI_REQ_HIDE, &req);
                }

                req.show.id = ANI_ID_PAGE_POWER_OFF;
                server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                                     power_off_play_end, ui);
            }

#else
#ifdef CONFIG_GSENSOR_ENABLE
            park_en = db_select("par");
            set_parking_guard_wkpu(park_en);  //gsensor parking guard */
            sys_power_set_port_wakeup("wkup_gsen", park_en);
#endif
            sys_power_set_port_wakeup("wkup_usb", 1);
            sys_power_poweroff(0);
#endif
        }

        break;
    }

    return 0;
}


/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        main_key_event_handler(&event->u.key);
        break;

    case SYS_TOUCH_EVENT:
        break;

    case SYS_DEVICE_EVENT:
        main_dev_event_handler(event);
        break;

#ifdef CONFIG_WIFI_ENABLE
    case SYS_NET_EVENT:
        main_net_event_hander(event);
        break;
#endif

    default:
        ASSERT(0, "unknow event type: %s\n", __func__);
        break;
    }
}

#if 0
void malloc_debug_start(void);
void malloc_debug_show(void);
void malloc_stats(void);
static void rtos_stack_check_func(void *p)
{
    char pWriteBuffer[2048];
    extern void vTaskList(char *pcWriteBuffer);
    vTaskList((char *)&pWriteBuffer);
    printf(" \n\ntask_name          task_state priority stack task_num\n%s\n", pWriteBuffer);
    malloc_stats();
    /* extern unsigned char ucGetCpuUsage(void); */
    /* printf("cpu use:%d%%\n",ucGetCpuUsage()); */
    //extern void malloc_debug_start(void);
    //malloc_debug_start();
    //malloc_debug_show();
    extern void get_task_state(void *parm);
    get_task_state(NULL);//1分钟以内调用一次才准确
}

void malloc_st(void *p)
{
    rtos_stack_check_func(NULL);
}
#endif
/*
 * 应用程序主函数
 */
void app_main()
{
    struct intent it;
    int err;

#ifdef MULTI_LCD_EN
    void *lcd_dev;
#if  0
    sw = 1;
    lcd_dev = dev_open("lcd", "lcd_avout");
#else
    sw = 0;
    lcd_dev = dev_open("lcd", "bst40");
#endif
    lcd_set_cur_hdl(lcd_dev);
#endif

    if (!fdir_exist("mnt/spiflash")) {
        mount("spiflash", "mnt/spiflash", "sdfile", SDFILE_RESERVE_SPACE, NULL);
    }

    mount_sd_to_fs(SDX_DEV);
    err = upgrade_detect(SDX_DEV);

    if (!err) {
        return;
    }

    extern int sys_power_init(void);
    sys_power_init();
    extern int user_isp_cfg_init(void);
    user_isp_cfg_init();

    /* sys_timer_add(NULL, malloc_st, 5000); */
    /*
     * 播放开机动画
     */
#ifdef CONFIG_UI_ENABLE
    struct ui_style style;

    style.file = "mnt/spiflash/audlogo/ani.sty";
    style.version = ANI_UI_VERSION;

    struct server *ui = server_open("ui_server", &style);

    if (ui) {
        union uireq req;

        req.show.id = ANI_ID_PAGE_POWER_ON;
        /* #if ((LCD_DEV_WIDTH > 480) && (__SDRAM_SIZE__ <= 8*1024*1024))  */
        server_request(ui, UI_REQ_SHOW_SYNC, &req);
        animation_play_end(ui);

        /* #else */
        /* server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req, */
        /* animation_play_end, ui); */
        /* #endif */
    }

#else
    sys_key_event_enable();
    sys_touch_event_enable();
#endif

    sys_power_auto_shutdown_start(db_select("aff") * 60);
    sys_power_low_voltage_shutdown(370, 3);
    sys_power_charger_off_shutdown(10, 1);

    init_intent(&it);
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);

    //sys_timer_add(NULL,malloc_st,5000);
#if 0	//USB PC摄像模式调试镜头
    it.name = "usb_app";
    it.data = "usb:uvc";
    it.action = ACTION_USB_SLAVE_MAIN;
    start_app(&it);

    os_time_dly(200);
    it.action = ACTION_USB_SLAVE_SET_CONFIG;
    start_app(&it);
    return;
#endif


#if 0
    int flow_init(void);
    flow_init();
    os_time_dly(250);
    printf("video start ...\n\n");
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
    return;
#endif

#if 0
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
    int user_open_rt_stream(void);
    user_open_rt_stream();

    return;
#endif

#ifdef CONFIG_UI_ENABLE

    if (dev_online("usb0")) {
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_MAIN;
        start_app(&it);
        return;
    }

#endif

#ifdef PHOTO_STICKER_ENABLE
    it.name	= "video_photo";
    it.action = ACTION_PHOTO_TAKE_MAIN;
#else
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
#endif
    start_app(&it);
}

