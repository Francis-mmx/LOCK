#include "device/lcd_driver.h"
#include "system/includes.h"
#include "ui/includes.h"
#include "server/ui_server.h"
#include "video_rec.h"
#include "action.h"
#include "style.h"


static u8 lcd_pro_flag = 0; /* 屏保触发标志，1：已经触发 */
static u16 lcd_protect_time = 0; /* 屏保触发时间，单位秒 */
static u16 lcd_pro_cnt = 0;
static int timer = 0;

extern u8 page_pic_flag;           //设置主界面页面标志位 0--第一页  1--第二页
extern u8 ani_flag;                //1--动画或进度条
extern u8 lock_on;
extern u8 power_flag;
extern int uart_send_package(u8 command, u8 *data, u8 com_len);

void ui_lcd_light_on(void)
{
    puts("====ui_lcd_light_on====\n");
//    lcd_backlight_ctrl(true);
    pwm_duty_cycle(100);
    lcd_pro_flag = 0;
}

void ui_lcd_light_off(void)
{
    puts("====ui_lcd_light_off====\n");
//    lcd_backlight_ctrl(false);
    pwm_duty_cycle(0);
    lcd_pro_flag = 1;
}


/*
 * 屏幕保护计时器
 */
static void lcd_pro_kick(void *priv)
{
    struct intent it;
    init_intent(&it);
    if(get_parking_status() || ani_flag)
    {
        //倒车不屏保
        //puts("LCD NO OFF\n");
        lcd_pro_cnt = 0;
        return;
    }
    if(lcd_protect_time && lcd_pro_flag == 0)
    {
        lcd_pro_cnt++;
        if(lcd_pro_cnt >= lcd_protect_time)
        {
            puts("\n\n\n********lcd_pro_kick********\n\n\n");
            page_pic_flag = 0;
            lock_on = 0;
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app(&it);

            u8 command_buf = exit_sleep;
            u8 data_buf = 1;                    //进入休眠，通知锁控
            uart_send_package(command_buf, &data_buf, 1);

            lcd_pro_cnt = 0;
            lcd_pro_flag = 1;
            ui_lcd_light_off();
        }
    }
    else
    {
        lcd_pro_cnt = 0;
    }
}

void ui_lcd_light_time_set(int sec)
{
    printf("ui_lcd_light_time_set sec:%d\n", sec);

    if(sec)
    {
        lcd_protect_time = sec;
        if(!timer)
        {
            timer = sys_timer_add(NULL, lcd_pro_kick, 1000);
        }
    }
    else
    {
        lcd_protect_time = 0;
    }
}


static void backlight_event_handler(struct sys_event *event)
{
    static u8 once_flag = 0;
    struct intent it;
    init_intent(&it);
    lcd_pro_cnt = 0;
    if(lcd_pro_flag)
    {
        if(event->type == SYS_KEY_EVENT)
        {
            ui_lcd_light_on();
            lcd_pro_flag = 0;
            sys_key_event_consume(&(event->u.key)); /* 背光关闭时，按键只是打开背光 */
        }
        else if(event->type == SYS_TOUCH_EVENT)
        {
            once_flag++;
            //背光关闭后，UI退回到主页
            lock_on = 1;
            power_flag = 0;
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_MAIN;
            start_app(&it);

            sys_touch_event_consume(&(event->u.touch)); /* 背光关闭时，触摸只是打开背光 */
            if(event->u.touch.event == ELM_EVENT_TOUCH_UP)
            {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
            if(once_flag == 2)
            {
                once_flag = 0;
                u8 command_buf = exit_sleep;
                u8 data_buf = 0;                    //退出休眠，通知锁控
                uart_send_package(command_buf, &data_buf, 1);
            }
        }
    }
    else if(event->type == SYS_KEY_EVENT
    && event->u.key.event == KEY_EVENT_CLICK
    && event->u.key.value == KEY_POWER)
    {
        lcd_pro_flag = 1;
        ui_lcd_light_off();
        sys_key_event_consume(&(event->u.key));
    }
    else if(event->type == SYS_KEY_EVENT
    && event->u.key.event == KEY_EVENT_LONG
    && event->u.key.value == KEY_POWER)
    {
        sys_power_shutdown();
    }
    else if(event->type == SYS_KEY_EVENT
    && event->u.key.event == KEY_EVENT_CLICK
    && event->u.key.value == KEY_OK)
    {
        if(lcd_pro_flag)
        {
            printf("==============on");
            ui_lcd_light_on();
            lcd_pro_flag = 0;
        }
        else
        {
            printf("==============off");
            ui_lcd_light_off();
            lcd_pro_flag = 1;
        }
    }
}
SYS_EVENT_HANDLER(SYS_KEY_EVENT | SYS_TOUCH_EVENT, backlight_event_handler, 4);


static void backlight_charge_event_handler(struct sys_event *event)
{

    if(!ASCII_StrCmp(event->arg, "parking", 7))
    {
        if(event->u.dev.event == DEVICE_EVENT_IN)
        {
            if(lcd_pro_flag)
            {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
        }
        else if(event->u.dev.event == DEVICE_EVENT_OUT)
        {
            if(lcd_pro_flag)
            {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
        }
    }
    else if(!ASCII_StrCmp(event->arg, "sys_power", 9))
    {
        if(event->u.dev.event == DEVICE_EVENT_POWER_CHARGER_IN)
        {
            if(lcd_pro_flag)
            {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
        }
        else if(event->u.dev.event == DEVICE_EVENT_POWER_CHARGER_OUT)
        {
            if(lcd_pro_flag)
            {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
        }
    }

    if(!strncmp(event->arg, "parking", 7))
    {
        if(event->u.dev.event == DEVICE_EVENT_IN)
        {
            ui_lcd_light_on();
            lcd_pro_flag = 0;
        }
    }

}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, backlight_charge_event_handler, 4);

