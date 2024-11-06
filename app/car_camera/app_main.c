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
#include "generic/log.h"


#define ANSWER_TAG 0x21

u32 spin_lock_cnt[2] = {0};

u32 uart_timer_handle[10] = {0};
extern u8 device_status[10];
extern u8 auto_check_status[10];
extern u8 auto_check_flag;
u8 ani_flag = 0;//1--动画或进度条
u8 sys_lock_time;
extern u8 on_homepage;
int upgrade_detect(const char *sdcard_name);
u8 answer_flag = 0;//0--未收到回复 1--收到回复

/*任务列表 */
const struct task_info task_info_table[] = {
#ifdef __CPU_AC521x__
    {"video_fb",            27,     1024,   1024  },
#else
    {"video_fb",            30,     1024,   1024  },
#endif
    {"ui_server",           29,     1024,   1024  },
    {"ui_task",             30,     4096,   512   },
    {"sys_timer",           10,     512,   2048  },//qsize必须大于2048
    {"init",                30,     1024,   256   },
    {"app_core",            15,     2048,   3072  },
    {"sys_event",           30,     1024,   0     },
    {"video_server",        16,     2048,   256   },
    {"audio_server",        16,     1024,   256   },
    {"audio_decoder",       17,     5120,   64    },
    {"audio_encoder",       17,     1024,   64    },
    {"speex_encoder",       17,     1024,   64    },
    {"amr_encoder",         17,     1024,   64    },
    {"aac_encoder",         17,     8192,   0     },
    {"video_dec_server",    27,     1024,   1024  },
    {"video0_rec0",         22,     2048,   512   },
    {"video0_rec1",         21,     2048,   512   },
    {"audio_rec0",          24,     2048,   256   },
    {"audio_rec1",          24,     2048,   256   },
    {"audio_rec2",          24,     2048,   256   },
    {"video1_rec0",         19,     2048,   512   },
    {"video2_rec0",         19,     2048,   512   },
    {"isp_update",          27,     1024,   0     },
    {"vpkg_server",         26,     2048,   512   },
    {"vunpkg_server",       23,     1024,   128   },
    {"avi0",                29,     2048,   64    },
    {"avi1",                29,     2048,   64    },
    {"avi2",                29,     2048,   64    },
    {"mov0",                28,     2048,   64    },
    {"mov1",                28,     2048,   64    },
    {"mov2",                28,     2048,   64    },
    {"mov3",                28,     2048,   64    },
    {"mp40",                28,     2048,   64    },
    {"mp41",                28,     2048,   64    },
    {"video_engine_server", 14,     1024,   1024  },
    {"video_engine_task",   15,     2048,   0     },
    {"usb_server",          20,     2048,   128   },
    {"khubd",               25,     1024,   32    },

    {"uvc_transceiver",     26,     2048,   32    },
    {"uvc_transceiver1",    26,     2048,   32    },
    {"uvc_transceiver2",    26,     2048,   32    },
    {"vimc_scale",          26,     2048,   32    },
    {"upgrade_server",      21,     1024,   32    },
    {"upgrade_core",        20,     1024,   32    },
    {"dynamic_huffman0",    15,		1024,	32    },
    {"dynamic_huffman1",    15,		1024,	32    },
    {"video0_devbuf0",      27,     1024,   32    },
    {"video1_devbuf0",      27,     1024,   32    },
    {"video2_devbuf0",      27,     1024,   32    },
    {"video_dec_devbuf",    27,     1024,   32    },
    {"audio_dec_devbuf",    27,     1024,   32    },
    {"jpg_dec",             27,     1024,   32    },

    {"yuv_enc0",            30,     3072,   256   },
    {"imr_rt0",             28,     3072,   256   },
    {"imr_rt1",             27,     3072,   256   },
    {"imr_enc_task",        28,     2048,   32    },
    /*  {"yuv_enc0",            12,     3072,   256   }, */
    /* {"imr_rt0",             30,     3072,   256   }, */
    /* {"imr_rt1",             30,     3072,   256   }, */

    {"auto_test",			15,		1024,	1024  },
    {"fs_test",			    15,		1024,	0     },

    {"powerdet_task",       15,     1024,   1024  },

    {"sys_timer",            15,     512,   256   },
#ifdef CONFIG_CT_AUDIO_TEST
    {"audio_test",            15,     512,   256   },
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
    sys_power_set_port_wakeup("wkup_tp", 1);
    sys_power_poweroff(0);
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
            app = get_current_app();
            if (app) {
                it.action = ACTION_BACK;
                start_app(&it);
            }

#ifdef CONFIG_UI_ENABLE
            struct ui_style style;
            style.file = "mnt/spiflash/audlogo/ani.sty";
            style.version = UI_VERSION;
            struct server *ui = server_open("ui_server", &style);

            if (ui) {
                union uireq req;

                if (get_parking_status()) {
                    puts("hide_park_ui\n");
                    //req.hide.id = ID_WINDOW_PARKING;
                    server_request(ui, UI_REQ_HIDE, &req);
                }

                req.show.id = PAGE_1;
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
            sys_power_set_port_wakeup("wkup_tp", 1);
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
    malloc_debug_show();

    /* extern unsigned char ucGetCpuUsage(void); */
    /* printf("cpu use:%d%%\n",ucGetCpuUsage()); */
}

void malloc_st(void *p)
{
    rtos_stack_check_func(NULL);
}

#endif

/*************************************Changed by liumenghui*************************************/

void animation_play()
{
    struct ui_style style;

    style.file = "mnt/spiflash/audlogo/ani.sty";
    style.version = UI_VERSION;

    struct server *ui = server_open("ui_server", &style);
    if (ui)
    {
        union uireq req;

        req.show.id = PAGE_0;
        /* #if ((LCD_DEV_WIDTH > 480) && (__SDRAM_SIZE__ <= 8*1024*1024))  */
        server_request(ui, UI_REQ_SHOW_SYNC, &req);

        animation_play_end(ui);

        /* #else */
        /* server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req, */
        /* animation_play_end, ui); */
        /* #endif */
    }
}


/*************************************串口接收数据*************************************/

extern u8 tx_flag;
extern u8 power_flag;
extern void hide_show_page();
extern user_infor *match_user_num(u8 num);
extern void get_system_time(struct sys_time *time);
extern void write_data_to_flash(void *buf, u8 flag);//0--写用户数据 1--写记录
extern void get_user_infor_struct();

int spec_uart_send(char *buf, u32 len) ;//串口发送
u8 lock_on = 1;
user_visit now_user;//当前的访问用户
static struct intent uart_recv;

u8 recv_buffer[BUFFER_SIZE];  // 接收缓冲区
u8 write_index = 0;  // 写指针
u8 read_index = 0;   // 读指针
u8 data_count = 0;     //缓冲区内数量

/*************************************发送数据包和重发机制*************************************/
static struct intent uart_buf;
int uart_send_package(u8 command,u8 *data,u8 com_len)
{
    u8 wake_command[15] = {0};
    u8 total_length = com_len + PACKET_OTHER_LEN;
    const char *packet_buf = create_packet_uncertain_len(0,command,data,com_len);
    spec_uart_send(wake_command,15);
    spec_uart_send(packet_buf,total_length);//首次发送

    //启用新的任务调度
    init_intent(&uart_buf);
    uart_buf.name	= "video_rec";
    uart_buf.action = ACTION_VIDEO_REC_UART_RETRANSMIT;
    uart_buf.data = packet_buf;
    uart_buf.exdata = total_length;
    start_app(&uart_buf);

    return 0;
}

void transmit_callback(struct intent *it)
{
    spec_uart_send(it->data,it->exdata);//重发数据包

    //再次调度任务
    uart_buf.name	= "video_rec";
    uart_buf.action = ACTION_VIDEO_REC_UART_RETRANSMIT;
    start_app(&uart_buf);
}

void transmit_overtime(void)//超时处理函数，后续可新增功能
{
    printf("uart transmit overtime\n");
}


int uart_recv_retransmit()
{
    u8 com = (u8)uart_buf.data[4];
    if(!answer_flag){
        if(tx_flag < MAX_TRANSMIT)
        {
            tx_flag++;
            switch(com){
                case 0xA0:
                    uart_timer_handle[0] = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
                    break;
                case 0xA1:
                    uart_timer_handle[1] = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
                    break;
                case 0xA2:
                    uart_timer_handle[2] = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
                    break;
                case 0xA3:
                    uart_timer_handle[3] = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
                    break;
                case 0xA4:
                    uart_timer_handle[4] = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
                    break;
                case 0x10:
                    uart_timer_handle[5] = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
                    break;
                case 0x11:
                    uart_timer_handle[6] = sys_timeout_add(&uart_buf,transmit_callback,100);//定时    重发数据包       100ms后删除
                    break;
            }
        }
        else
        {
            tx_flag = 0;
            switch(com){
                case 0xA0:
                    uart_timer_handle[0] = sys_timeout_add(0,transmit_overtime,100);
                    break;
                case 0xA1:
                    uart_timer_handle[1] = sys_timeout_add(0,transmit_overtime,100);
                    break;
                case 0xA2:
                    uart_timer_handle[2] = sys_timeout_add(0,transmit_overtime,100);
                    break;
                case 0xA3:
                    uart_timer_handle[3] = sys_timeout_add(0,transmit_overtime,100);
                    break;
                case 0xA4:
                    uart_timer_handle[4] = sys_timeout_add(0,transmit_overtime,100);
                    break;
                case 0x10:
                    uart_timer_handle[5] = sys_timeout_add(0,transmit_overtime,100);
                    break;
                case 0x11:
                    uart_timer_handle[6] = sys_timeout_add(0,transmit_overtime,100);
                    break;
            }
        }
    }
}


void ani_show()
{
    ui_hide(ANI_UNLOCK_LAYER);
    ui_show(ENC_LAY_BACK);
    ui_show(ENC_LAY_HOME_PAGE);
    lock_on = 1;
    ani_flag = 0;
    on_homepage = 1;
}
void uart_send_unlock(u8 *buf)
{
    printf("uart_send_unlock\n");
    u16 user = 0;
    struct sys_time sys_time;
    get_user_infor_struct();

    if(lock_on){
        //解锁用户
        user = buf[5];
        user = (user << 8) + buf[6];
        user_data = match_user_num(user);
        if(user_data != NULL){
            get_system_time(&sys_time);
            memcpy(now_user.name,user_data->name,sizeof(now_user.name));
            now_user.record_time = sys_time;
            switch(buf[7]){                     //解锁方式
                case 0:
                    printf("face \n");
                    break;
                case 1:
                    printf("password \n");
                    break;
                case 2:
                    printf("fingerprint \n");
                    break;
                case 3:
                    printf("nfc \n");
                    break;
                default :
                    printf("undefined\n");
                    return;
            }
            for(int i = 0; i < user_data->key_num; i++){
                if(user_data->key[i].key_mode == buf[7]){
                    now_user.unlock.unlock_mode = buf[7];
                    break;
                }
                if(i == (user_data->key_num - 1)){
                    printf("user not the key\n");
                    return;
                }
            }
            printf("current chose user name %s number %d\n",user_data->name,user_data->user_num);
            if(power_flag){                 //进入管理员模式
                if(user_data->user_power){  //该用户有管理员权限
                    printf("admin power\n");
                    now_user.unlock.unlock_power = ADMIN;
                    ui_hide(ENC_LAY_BACK);
                    ui_hide(ENC_PASSWORD_LAY);
                    ui_show(ENC_LAY_BACK_PIC);
                    ui_show(ENC_LAY_PAGE);
                } else {
                    printf("not power user \n");
                    return ;
                }
            } else {
                //开锁动画
                printf("unlock\n");
                ani_flag = 1;
                lock_on = 0;
                on_homepage = 0;
                now_user.unlock.unlock_power = UNLOCK;
                ui_hide(ENC_LAY_BACK);
                ui_hide(ENC_PASSWORD_LAY);
                ui_show(ENC_LAY_BACK_PIC);
                ui_show(ANI_UNLOCK_LAYER);
                ui_show(ENC_UP_LAY);
                sys_timeout_add(NULL, ani_show, 3000);
            }
            if(now_user.name != NULL){
                write_data_to_flash(&now_user, 1);//添加访问记录
                memset(&now_user,0,sizeof(now_user));
            }
        }
    }

    if(user_data)
    {
        free(user_data);
        user_data = NULL;
    }
}

void delay_hide_status()
{
    on_homepage = 1;
    device_status[0] = 0x0F;
    ui_hide(ENC_DEVICE_STATUS);
    ui_show(ENC_LAY_HOME_PAGE);
}

void get_device_infor(u8 *buf)
{
    printf("get_device_infor\n");

    for(int i = 0; i < 8; i++)
    {
        device_status[i] = buf[5+i];
    }
    device_status[5] = !get_tp_init_state_func();
    put_buf(device_status, 7);

    if(device_status[0] || device_status[1] || device_status[2] || device_status[3] || device_status[4] || device_status[5] || device_status[6] || device_status[7])
    {
        printf("get device error");

        ui_show(ENC_DEVICE_STATUS);
        sys_timeout_add(NULL, delay_hide_status, 5000);
    } else {
        delay_hide_status();
    }
}

void cancel_retransmit(u8 *buf)
{
    if(buf[2] == 0x11){              //取消重发时，锁->屏的标识符为0x11
        if(buf[4] == 0xA3){
            if(buf[5] == 0x01){                         //指纹模块响应，跳转指纹界面
                ui_hide(ENC_USER_NEW_KEY);
                ui_show(ENC_ADD_NEW_FINGERPRINT);
            }else if(buf[5] == 0x02){                   //卡片模块响应，跳转卡片界面
                ui_hide(ENC_USER_NEW_KEY);
                ui_show(ENC_ADD_NEW_NFC);
            }
        }else if(buf[4] == 0xA4){
            if(auto_check_flag)
            {
                printf("auto check");
                for(int i = 0; i < 8; i++)
                {
                    auto_check_status[i] = buf[5+i];
                }
                auto_check_status[5] = !get_tp_init_state_func();
                put_buf(auto_check_status, 7);
            }
            for(int i=0;i<(tx_flag+1);i++){
                sys_timeout_del(uart_timer_handle[4]);//删除添加的 设备查询指令 超时回调
            }
        }
        /*取消重发*/
        if(buf[5] == 0){
            switch(buf[4]){
                case 0xA0:
                    for(int i=0;i<(tx_flag+1);i++){//在重发第三次的时候同样可以取消，否则进入超时处理
                        sys_timeout_del(uart_timer_handle[0]);//删除添加的 语音指令 超时回调
                    }
                    break;
                case 0xA1:
                    for(int i=0;i<(tx_flag+1);i++){
                        sys_timeout_del(uart_timer_handle[1]);//删除添加的 添加用户指令 超时回调
                    }
                    break;
                case 0xA2:
                    for(int i=0;i<(tx_flag+1);i++){
                        sys_timeout_del(uart_timer_handle[2]);//删除添加的 删除用户指令 超时回调
                    }
                    break;
                case 0xA3:
                    for(int i=0;i<(tx_flag+1);i++){
                        sys_timeout_del(uart_timer_handle[3]);//删除添加的 添加密钥指令 超时回调
                    }
                    break;
                case 0xA4:
//                    for(int i=0;i<(tx_flag+1);i++){
//                        sys_timeout_del(uart_timer_handle[4]);//删除添加的 设备查询指令 超时回调
//                    }
                    break;
                case 0x10:
                    for(int i=0;i<(tx_flag+1);i++){
                        sys_timeout_del(uart_timer_handle[5]);//删除添加的 休眠指令 超时回调
                    }
                    break;
                case 0x11:
                    for(int i=0;i<(tx_flag+1);i++){
                        sys_timeout_del(uart_timer_handle[6]);//删除添加的 启动人脸控件指令 超时回调
                    }
                    break;
            }
        }
        tx_flag = 0;
    }
}


extern u8 goto_facial_page_flag;       //进入人脸界面标志位
void add_user_new_key(u8 *buf)
{
    struct intent it;
    u16 user = 0;
    printf("add_user_new_key\n");
    get_user_infor_struct();
    if(buf[3] == 4){//传递用户编号和新增密钥类型
        user = buf[5];
        user = (user << 8) + buf[6];
        user_data = match_user_num(user);
        if(user_data != NULL){
            if(user_data->key_num < MAX_KEY_NUM ){
                memcpy(user_data->key[user_data->key_num].key_buf, 1, MAX_KEY_NUM);
                user_data->key[user_data->key_num].key_mode = buf[7];
                user_data->key_num++;
                printf("add %d user name %s new key %d\n",user_data->key_num,user_data->name,user_data->key[user_data->key_num].key_mode);
                write_data_to_flash(user_data,0);
                if(buf[7] == 0){
                    init_intent(&it);
                    it.name = "video_rec";
                    it.action = ACTION_VIDEO_REC_SWITCH_WIN_OFF;
                    start_app(&it);
                    goto_facial_page_flag = 1;
                    ui_hide(ENC_FACIAL_LAY);
                    ui_show(ENC_WIN);
                }
                else if(buf[7] == 2){
                    ui_hide(ENC_ADD_NEW_FINGERPRINT);
                    ui_show(ENC_LAY_USER_DETAILS);
                }else if(buf[7] == 3){
                    ui_hide(ENC_ADD_NEW_NFC);
                    ui_show(ENC_LAY_USER_DETAILS);
                }

            } else {
                printf("current user key is full\n");
                return;
            }
        }

        if(user_data){
            free(user_data);
            user_data = NULL;
        }
    }else if(buf[3] == 2) {//传递新增密钥，以便UI进行页面跳转
        if(buf[5] == 2){
            ui_hide(ENC_USER_NEW_KEY);
            ui_show(ENC_ADD_NEW_FINGERPRINT);
        } else if(buf[5] == 3) {
            ui_hide(ENC_USER_NEW_KEY);
            ui_show(ENC_ADD_NEW_NFC);
        }
    }
}


void get_fingerprint_confirm(u8 *buf)
{
    static u8 idx = 1;
    u8 command_buf,data_buf;
    printf("get_fingerprint_confirm\n");
    command_buf = voice;
    if(buf[5]){
        ui_text_show_index_by_id(ADD_NEW_FINGER_TXT_1,6);//指纹录入失败
        data_buf = operate_fail;
        ui_hide(ENC_ADD_NEW_FINGERPRINT);
        ui_hide(ENC_LAY_USER_DETAILS);
    } else {
        ui_text_show_index_by_id(ADD_NEW_FINGER_TXT_1,idx);
        idx++;
        if(idx == 5){
            idx = 0;
            data_buf = operate_success;
        } else {
            data_buf = press_finger;
        }
    }
    uart_send_package(command_buf,&data_buf,1);
}


extern int tim_handle;
extern void time_blink();

void exit_system_lock()
{
    ui_hide(ENC_SYSTEM_LOCK);
    ui_show(ENC_LAY_BACK);
    ui_show(ENC_LAY_HOME_PAGE);
    if(!tim_handle){
        tim_handle = sys_timer_add(NULL, time_blink, 1000);
    }
}


void system_locktime_cal(u8 time)
{
    u8 cal_time;
    cal_time = sys_lock_time - time;
    if(cal_time == 0){
        exit_system_lock();
    }
    ui_pic_show_image_by_id(SYSTEM_LOCK_TIME_1,cal_time/100);
    ui_pic_show_image_by_id(SYSTEM_LOCK_TIME_2,cal_time/10%10);
    ui_pic_show_image_by_id(SYSTEM_LOCK_TIME_3,cal_time%10);
    ui_show(ENC_UP_LAY);
}

void get_system_lock_status(u8 *buf)
{
    static int locktime_handle[256];
    u8 command_buf,data_buf;
    printf("get_system_lock_status");
    if(buf[5] == 0x01){
        if(tim_handle){
            sys_timer_del(tim_handle);
            tim_handle = 0;
        }
        ui_hide(ENC_PASSWORD_LAY);
        ui_hide(ENC_LAY_BACK);
        ui_show(ENC_SYSTEM_LOCK);

        sys_lock_time = buf[6];
        for(u16 i = 0; i < sys_lock_time + 1; i++)
        {
            locktime_handle[i] = sys_timeout_add(i, system_locktime_cal, i*1000);
        }
    }else if(buf[5] == 0x02){
        command_buf = voice;
        data_buf = system_locked;
        uart_send_package(command_buf,&data_buf,1);
    }else if(buf[5] == 0){
        for(u16 i = 0; i < sys_lock_time + 1; i++)
        {
            sys_timeout_del(locktime_handle[i]);
        }
        exit_system_lock();
    }
}


extern u8 goto_facial_page_flag;
void goto_face_identification(u8 *buf)
{
    printf("goto_face_identification");
    struct intent it;
    if(on_homepage){
        if(buf[5]){
            //退出人脸控件
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_SWITCH_WIN_OFF;
            start_app(&it);
            goto_facial_page_flag = 1;
            ui_hide(ENC_FACIAL_LAY);
        }else{
            //进入人脸控件
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_SWITCH_WIN;
            start_app(&it);
            ui_show(ENC_FACIAL_LAY);
        }
    }
}


void get_face_confirm(u8 *buf)
{
    static u8 idx = 0;
    u8 command_buf,data_buf;
    command_buf = voice;
    printf("get_face_confirm\n");
    if(buf[5]){
        data_buf = operate_fail;
    } else {
        if(idx == 4){
            idx = 0;
            data_buf = operate_success;
        } else {
            data_buf = turn_left+idx;
        }
        idx++;
    }
    uart_send_package(command_buf,&data_buf,1);
}

void uart_recv_handle()
{
    u8 i,idx;
    u8 command,data;
    u8 recv_data[BUFFER_SIZE];
    u16 len = recv_buffer[(read_index + 3) % BUFFER_SIZE];

    if(data_count >= (len + 2)){
        for (i = 0; i < len + 6; i++) {
            recv_data[i] = recv_buffer[read_index];
            read_index = (read_index + 1) % BUFFER_SIZE;
            data_count--;
        }
        idx = 4 + recv_data[3];
        u16 check = (recv_data[idx] << 8) + recv_data[idx + 1];

        if(recv_data[0] == 0xAA && recv_data[1] == 0xBB ){
            if(check == calculate_checksum(recv_data,recv_data[3],2)){  //校验成功
                data = 0;
                if((recv_data[4] == 0xA5 || recv_data[4] == 0xA6 || recv_data[4] == 0xA7 || recv_data[4] == 0xA8 || recv_data[4] == 0xA9) && recv_data[2] == 0x00){
                    const char *packet_buf = create_packet_uncertain_len(ANSWER_TAG,recv_data[4],&data,1);
                    spec_uart_send(packet_buf,8);//回复信号
                }
                switch(recv_data[4]){
                    case 0xA0:case 0xA1:case 0xA2:case 0xA3:case 0xA4:case 0x10:case 0x11:
                        answer_flag = 1;
                        cancel_retransmit(recv_data);                               //应答信号,取消重发
                        return;
                    case 0xA5:
                        add_user_new_key(recv_data);                                //添加新的用户密钥
                        break;
                    case 0xA6:
                        uart_send_unlock(recv_data);                                //解锁信号
                        break;
                    case 0xA7:
                        get_device_infor(recv_data);                                //获取自检设备信息
                        break;
                    case 0xA8:
                        get_system_lock_status(recv_data);                          //获取系统锁定事件
                        break;
                    case 0xA9:
                        goto_face_identification(recv_data);                        //进入人脸识别控件
                        break;
                    default:
                        return ;
                }
            } else {                           //校验失败
                    data = 1;
        }

        }
    }
}

int uart_receive_package(u8 *buf, int len)  //串口接收   最大接收长度512字节  接收时间580ms左右
{
    if((len == (buf[3] + 6)) && buf[0] == 0xAA && buf[1] == 0xBB){
        for (u8 i = 0; i < len; i++) {
            recv_buffer[write_index] = buf[i];
            write_index = (write_index + 1) % BUFFER_SIZE;
            data_count++;
        }
    }
}




/*************************************Changed by liumenghui*************************************/



/*
 * 应用程序主函数
 */
void app_main()
{
    struct intent it;
    int err;

    puts("app_main\n");
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
        mount("spiflash", "mnt/spiflash", "sdfile", 0, NULL);
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
    uart_receive_callback(uart_receive_package); //串口回调函数


#if 0
    // mode 有三种
    // 0 直接缩放
    // 1 直接剪切
    // 2 先剪切，再缩放
    // 其它模式无效，自己根据需要选择使用模式
    // 其它参数根据名称
    /* extern int manual_scale_and_encode_jpeg(u8 *src_yuv, u8 *jpg_path, int src_width, int src_height, int dst_width, int dst_height, u8 mode); */
#define SRC_W	1280
#define SRC_H	720
#define YUV_SIZE (SRC_W * SRC_H * 3 / 2)

    u8 *yuv = (u8 *)malloc(YUV_SIZE);
    FILE *fp = fopen(CONFIG_ROOT_PATH"I420.yuv", "r");
    fread(fp, yuv, YUV_SIZE);
    fclose(fp);

    for (int i = 0; i < 10; i++) {
        // 手动缩放YUV并保存成保存jpg
        int jpg_size = manual_scale_and_encode_jpeg(yuv, CONFIG_ROOT_PATH"mm***.jpg", SRC_W, SRC_H, 2592, 1968, 1);
        /* os_time_dly(10); */
    }
#endif
    /*
     * 播放开机动画
     */
#ifdef CONFIG_UI_ENABLE
    struct ui_style style;

    style.file = "mnt/spiflash/audlogo/ani.sty";
    style.version = UI_VERSION;

    struct server *ui = server_open("ui_server", &style);
    if (ui)
    {
        union uireq req;

        req.show.id = PAGE_0;
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

/*******************************************上电*******************************************/
//    u8 command_buf = voice;
//    u8 data_buf[] = {powered};
//    uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
/*******************************************上电*******************************************/
    sys_power_auto_shutdown_start(db_select("aff") * 60);
    sys_power_low_voltage_shutdown(320, PWR_DELAY_INFINITE);
//    sys_power_charger_off_shutdown(10, 1);


    init_intent(&it);
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);

    /* sys_timer_add(NULL,malloc_st,5000);  */
#ifdef CONFIG_UI_ENABLE
    if (dev_online("usb0")) {
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_MAIN;
        start_app(&it);
        return;
    }
#endif

    printf("touch panel : %d",get_tp_init_state_func());

#ifdef PHOTO_STICKER_ENABLE
    it.name	= "video_photo";
    it.action = ACTION_PHOTO_TAKE_MAIN;
#else
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
#endif
    start_app(&it);

    //init_intent(&it);
    //it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SEARCH_USER;
    start_app(&it);

}







