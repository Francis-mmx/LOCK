#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
/* #include "menu_parm_api.h" */
#include "app_config.h"

#ifdef CONFIG_UI_STYLE_JL02_ENABLE
#define STYLE_NAME  JL02

static s8 onkey_sel = 0;

/*
 * USB菜单
 */
static const char *table_usb_menu[] =
{
    "usb:msd",
    "usb:uvc",
    "usb:rec",
    "\0"
};

/*****************************USB页面回调 ************************************/
static int usb_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    struct intent it;
    int ret;
    switch(e)
    {
        case ON_CHANGE_INIT:
            sys_key_event_takeover(true, false);
            /*
             * USB页面打开后，UI任务接管key事件，APP不接收key事件
             */
            break;
        case ON_CHANGE_RELEASE:
            sys_key_event_takeover(false, false);
            /*
             * USB页面关闭后，恢复APP接收下一个key事件
             */
            ui_hide(ID_WINDOW_MAIN_PAGE);
            break;
        default:
            return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_USB_SLAVE)
.onchange = usb_page_onchange,
};

static int menu_usb_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;

    switch(e->value)
    {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            switch(sel_item)
            {
                case 0:
                    /*
                     * U盘模式
                     */
                    ui_hide(VLIST_USB);
                    ui_show(PIC_MSD_USB);
                    break;
                case 1:
                    /*
                     * USB摄像头模式
                     */
                    ui_hide(VLIST_USB);
                    ui_show(PIC_UVC_USB);
                    break;
                case 2:
                    /*
                     * 回到录像模式
                     */
                    break;
                default:
                    break;
            }

            init_intent(&it);
            it.name = "usb_app";
            it.action = ACTION_USB_SLAVE_SET_CONFIG;
            it.data = table_usb_menu[sel_item];
            start_app_async(&it, NULL, NULL);
            break;
        case KEY_DOWN:
            return false;

            break;
        case KEY_UP:
            return false;

            break;
        case KEY_MENU:

            break;
        case KEY_MODE:
            break;
        default:
            break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_USB)
.onkey = menu_usb_onkey,
.ontouch = NULL,
};


static int usb_goto_usb_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**usb_goto_usb_ontouch**");
    struct intent it;
    struct application *app;
    u8 sel_item;
    switch(e->event)
    {
        case ELM_EVENT_TOUCH_DOWN:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
            break;
        case ELM_EVENT_TOUCH_HOLD:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
            break;
        case ELM_EVENT_TOUCH_MOVE:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
            break;
        case ELM_EVENT_TOUCH_UP:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
            ui_hide(VLIST_USB);
            ui_show(PIC_MSD_USB);
            init_intent(&it);
            it.name = "usb_app";
            it.action = ACTION_USB_SLAVE_SET_CONFIG;
            it.data = table_usb_menu[0];
            start_app_async(&it, NULL, NULL);
            break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(USB_BTN)
.ontouch = usb_goto_usb_ontouch,
};

static int usb_goto_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**usb_goto_rec_ontouch**");
    struct intent it;
    struct application *app;
    u8 sel_item;
    switch(e->event)
    {
        case ELM_EVENT_TOUCH_DOWN:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
            break;
        case ELM_EVENT_TOUCH_HOLD:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
            break;
        case ELM_EVENT_TOUCH_MOVE:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
            break;
        case ELM_EVENT_TOUCH_UP:
            UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
            init_intent(&it);
            it.name = "usb_app";
            it.action = ACTION_USB_SLAVE_SET_CONFIG;
            it.data = table_usb_menu[2];
            start_app_async(&it, NULL, NULL);
            break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(REC_BTN)
.ontouch = usb_goto_rec_ontouch,
};

#endif
