/*************************************************************************
	> File Name: app/car_camera/upgrade.c
	> Author:
	> Mail:
	> Created Time: Fri 26 May 2017 03:45:26 PM HKT
 ************************************************************************/
#include "system/includes.h"
#include "app_config.h"
#include "server/ui_server.h"
#include "storage_device.h"
#include "upgrade_style.h"
#include "server/upgrade_server.h"
#include "action.h"
#include "ui/ui_slider.h"
#include "gSensor_manage.h"

#define STYLE_NAME UPGRADE

REGISTER_UI_STYLE(STYLE_NAME)

#define USE_CONFIRM_UI      0//升级确认打开/关闭

#define STORAGE_READY_TIMEOUT   (5)
#define STORAGE_RETRY_ENABLE	0	//CMD检测方式需要打开这个宏，否则不能卡升级，打开后开机速度会变慢

#define UPGRADE_UI_PATH         "ram/bfu/C/"
#define UPGRADE_PATH        CONFIG_ROOT_PATH

#if USE_CONFIRM_UI
#define FORCE_UPD_FILE      UPGRADE_PATH"RestoreA.bin"

#define UPD_CONFIRM         0x1
#define UPD_CANCEL          0x0
#endif

struct upgrade_ui_info {
    u8 ui_enable;
    u8 success;
    u8 upgrade_out;
#if USE_CONFIRM_UI
    u8 choice;
    u8 force;
    OS_SEM  sem_choose;
#endif
    FILE *file;
    OS_SEM  sem_wait;
    OS_SEM  sem_end;
    struct server *ui;
    int prev_id;
    char spercent[8];
};

static struct upgrade_ui_info ui_info;
#define __this  (&ui_info)

#if USE_CONFIRM_UI

static void remove_restore_file(const char *path)
{
    fdelete_by_name(path);
}

static bool need_force_upgrade(const char *path)
{
    FILE *fp;

    if (!path) {
        return false;
    }

    fp = fopen(path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }

    return false;
}

#endif
/*
 *升级过程进度条
 */
static int upgrade_show_progress(int percent)
{
    static u8 upgrade_success = 0;
    union uireq ui_req;
    if (upgrade_success) {
        return -EFAULT;
    }
    printf("%d\n", percent);
    if (!__this->ui || !__this->ui_enable) {
        return -EFAULT;
    }

    ui_slider_set_persent_by_id(UPD_ID_SLIDER, percent);

    if (percent < 100) {
        sprintf(__this->spercent, "%02d %%", percent);
    } else {
        sprintf(__this->spercent, "%d %%", percent);
    }
    printf("%s\n", __this->spercent);


    ui_text_set_str_by_id(UPD_ID_TEXT_PERCENT, "ascii", __this->spercent);

    if (percent == 100) {
        if (__this->prev_id) {
            ui_req.hide.id = __this->prev_id;
            server_request(__this->ui, UI_REQ_HIDE, &ui_req);
        }
        ui_req.show.id = UPD_ID_MESSAGE_6;
        server_request(__this->ui, UI_REQ_SHOW, &ui_req);
        puts("upgrade success.\n");
        upgrade_success = 1;
        os_time_dly(50);
    }
    return 0;
}

/*
 *升级信息显示
 */
static int upgrade_show_message(int msg)
{
    int id;
    union uireq ui_req;

    if (!__this->ui || !__this->ui_enable) {
        return -EINVAL;
    }
    switch (msg) {
    case 0:
        if (__this->ui) {
            ui_req.show.id = UPD_ID_PAGE_UPGRADE;
            server_request(__this->ui, UI_REQ_SHOW, &ui_req);
        }
        id = UPD_ID_MESSAGE_0;
        break;
    default:
        return -EINVAL;
    }

    if (__this->prev_id) {
        ui_req.hide.id = __this->prev_id;
        server_request(__this->ui, UI_REQ_HIDE, &ui_req);
    }
    ui_req.show.id = id;
    server_request(__this->ui, UI_REQ_SHOW, &ui_req);
    __this->prev_id = id;
    return 0;
}

static int upgrade_complete(u8 success)
{
    union uireq ui_req;

    __this->success = success;
#if USE_CONFIRM_UI
    if (success && __this->force) {
        remove_restore_file(FORCE_UPD_FILE);
    }
#endif

    if (!__this->ui || !__this->ui_enable) {
        __this->upgrade_out = 1;
        return -EINVAL;
    }

    ui_req.hide.id = UPD_ID_PAGE_UPGRADE;
    server_request(__this->ui, UI_REQ_HIDE, &ui_req);
    /*
     *关闭UI
     */
#if !USE_CONFIRM_UI
    if (__this->ui) {
        server_close(__this->ui);
        __this->ui = NULL;
    }
#endif
    __this->upgrade_out = 1;
    return 0;
}

static bool upgrade_confirm(void)
{
    struct ui_style style;
    /*
     *打开UI服务
     */
#ifdef CONFIG_UI_ENABLE
    style.file = UPGRADE_UI_PATH"upgrade.sty";
    style.version = UPD_UI_VERSION;
    __this->ui = server_open("ui_server", &style);

    if (!__this->ui) {
        return true;
    }

    os_sem_post(&__this->sem_wait);
#if USE_CONFIRM_UI
    union uireq req;
    __this->success = 1;
    if (__this->force) {
        return true;
    }

    __this->upgrade_out = 1;
    sys_key_event_enable();

    req.show.id = UPD_ID_PAGE_CONFIRM;
    server_request(__this->ui, UI_REQ_SHOW, &req);

    os_sem_pend(&__this->sem_choose, 0);

    req.hide.id = UPD_ID_PAGE_CONFIRM;
    server_request(__this->ui, UI_REQ_HIDE, &req);

    if (__this->choice != UPD_CONFIRM) {
        return false;
    }
#endif

#endif

    return true;
}

static int mount_ui_fs(void)
{
    struct imount *mt;

    mt = mount(NULL, "ram/bfu", "ramfs", 0, NULL);
    if (!mt) {
        puts("mount fail\n");
        return -EFAULT;
    }

    return 0;
}


static void upgrade_event_handler(void *priv, int argc, int *argv)
{
    int err;
    int percent = 0;
    struct server *upgrade_ser = (struct server *)priv;

    switch (argv[0]) {
    case UPGRADE_EVENT_COMPLETE:
        /*升级完成*/
        if (upgrade_ser) {
            server_close(upgrade_ser);
        }

        if (__this->file) {
            fclose(__this->file);
            __this->file = NULL;
        }
        upgrade_complete(1);

        cpu_reset();
        break;
    case UPGRADE_EVENT_FAILED:
        err = argv[1];
        break;
    case UPGRADE_EVENT_PERCENT:
        /*升级进度响应*/
        percent = argv[1];
        break;
    }

}

/*
 *SD卡升级检测
 */
int start_upgrade_detect(const char *sdcard_name)
{
    int timeout = 0;
    int err = 0;
    union sys_upgrade_req req = {0};
    struct server *upgrade_ser = NULL;

    if (timeout <= STORAGE_READY_TIMEOUT) {
#if USE_CONFIRM_UI
        __this->force = need_force_upgrade(FORCE_UPD_FILE);;
        if (__this->force) {
            __this->file = fopen(FORCE_UPD_FILE, "r");
        } else {
            __this->file = fopen(UPGRADE_PATH"JL_AC5X.bfu", "r");
        }

#else
        __this->file = fopen(UPGRADE_PATH"JL_AC5X.bfu", "r");
#endif
        /*
         * 自己配置的file，可以根据需要来处理未打开的情况
         */
        if (!__this->file) {
            err = -ENFILE;
            goto __exit;
        }
        upgrade_ser = server_open("upgrade_server", NULL);
        if (!upgrade_ser) {
            log_e("open upgrade server error\n");
            err = -EINVAL;
            goto __exit;
        }
        server_register_event_handler(upgrade_ser, NULL, upgrade_event_handler);
        /*
         *校验文件
         */
        req.info.type = UPGRADE_TYPE_FILE;
        req.info.input.file = __this->file;
        err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
        if (err) {
            /*进入到这里一般升级文件数据或者其他有错误*/
            log_e("upgrade file err : 0x%x\n", err);
            goto __exit;
        }

        /*
         * 校验系统是否匹配，系统是否需要升级
         */
        err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_SYSTEM, &req);
        if (err) {
#if USE_CONFIRM_UI
            if (!__this->force && err != SYS_UPGRADE_ERR_SAME)
#endif
            {
                log_e("system not match or no need upgrade : 0x%x\n", err);
                goto __exit;
            }
        }

        err = mount_ui_fs();
        if (!err) {
            __this->ui_enable = 1;
        }
        /*
         * 加载升级UI到ramfs
         */
        req.ui.type = UPGRADE_TYPE_FILE;
        req.ui.input.file = __this->file;
        req.ui.show_progress = upgrade_show_progress;
        req.ui.show_message = upgrade_show_message;
        req.ui.path = UPGRADE_UI_PATH;
        err = server_request(upgrade_ser, UPGRADE_REQ_LOAD_UI, &req);
        if (err) {
            log_e("load ui file error : 0x%x\n", err);
        }

#ifdef CONFIG_UI_ENABLE
        if (!upgrade_confirm()) {
            err = 0;
            goto __exit;
        }
#endif
        upgrade_show_message(0);
        /*
         * 正式进入升级
         */
        req.core.type = UPGRADE_TYPE_FILE;
        req.core.input.file = __this->file;
        req.core.offset = 0;
        /* upgrade server -- UPGRADE_REQ_CORE_START :
         * server_requset 为阻塞式升级
         * server_requset_async 为非阻塞式需要处理event
         */
        err = server_request_async(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
        if (err) {
            upgrade_show_message(err);
        }

__exit:
        if (err) {
            if (upgrade_ser) {
                server_close(upgrade_ser);
            }
            if (__this->file) {
                fclose(__this->file);
                __this->file = NULL;
            }
            upgrade_complete(0);
        }
        return err;
    }

    return -EINVAL;
}


static void switch_to_other_app(void)
{
    struct intent it;
    struct application *app;

    init_intent(&it);

    app = get_current_app();
    if (app) {
        it.action = ACTION_BACK;
        start_app(&it);
    }
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);

#ifdef CONFIG_UI_ENABLE
    if (dev_online("usb0")) {
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_MAIN;
        start_app(&it);
        return;
    }
#endif
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}

static void wait_upgrade_stop(void)
{
    os_sem_pend(&__this->sem_end, 0);

    task_kill("upgrade_core");
}

#if USE_CONFIRM_UI
static int set_upgrade_confirm(int confirm)
{
    if (confirm) {
        __this->choice = UPD_CONFIRM;
    } else {
        __this->choice = UPD_CANCEL;
    }

    os_sem_post(&__this->sem_choose);

    wait_upgrade_stop();

    if (!confirm) {
        switch_to_other_app();
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
    }
    return 0;
}
#endif

static void upgrade_task(void *arg)
{
    int msg[32];
    int err = 0;

    os_sem_create(&__this->sem_wait, 0);
    os_sem_create(&__this->sem_end, 0);
#if USE_CONFIRM_UI
    os_sem_create(&__this->sem_choose, 0);
#endif

    err = start_upgrade_detect((char *)arg);
    if (err) {
        os_sem_post(&__this->sem_end);
    }
    while (1) {
        os_time_dly(2);
        os_taskq_accept(ARRAY_SIZE(msg), msg);
        /*os_sem_pend(&__this->sem_wait, 0);*/
    }
}


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    struct intent mit;

    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_UPGRADE_MAIN:
            task_create(upgrade_task, (void *)it->data, "upgrade_core");
            break;
#if USE_CONFIRM_UI
        case ACTION_UPGRADE_SET_CONFIRM:
            set_upgrade_confirm((int)it->data);
            break;
#endif
        }
        break;
    case APP_STA_PAUSE:
        return -EFAULT;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
        break;
    }

    return 0;

}
static int upgrade_key_event_handler(struct key_event *key)
{
    struct intent it;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            break;
        case KEY_UP:

            break;
        case KEY_DOWN:

            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

static int upgrade_device_event_handler(struct sys_event *event)
{
    int err;

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_ONLINE:
        break;
    case DEVICE_EVENT_OUT:
        //upgrade_pause();
        break;
    }
    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{

    switch (event->type) {
    case SYS_KEY_EVENT:
        return upgrade_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return upgrade_device_event_handler(event);
    default:
        return false;
    }
    return false;
}


static const struct application_operation upgrade_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_upgrade) = {
    .name 	= "sdcard_upgrade",
    .action	= ACTION_UPGRADE_MAIN,
    .ops 	= &upgrade_ops,
    .state  = APP_STA_DESTROY,
};

#define  RECV_BLOCK_SIZE    (4 * 1024)
/*
 * flash升级文件测试代码，wifi可以参照这个写文件
 */
int sfc_mode_file_upgrade(void)
{
    void *sys_ufd = NULL;
    struct server *upgrade_ser;
    union sys_upgrade_req req = {0};
    u8 *buf = NULL;
    int err = 0;
    int block_size = 0;
    int size = 0;
    int offset = 0;

    server_load(upgrade_server);
    upgrade_ser = server_open("upgrade_server", NULL);
    if (!upgrade_ser) {
        log_e("open upgrade server error\n");
        return -EINVAL;
    }

    FILE *fsdc = fopen(CONFIG_ROOT_PATH"JL_ACSFC.bfu", "r");
    if (!fsdc) {
        log_e("open JL_AC5X.bfu error\n");
        return -ENOENT;
    }

    buf = malloc(RECV_BLOCK_SIZE);
    if (!buf) {
        log_e("malloc recv buffer error\n");
        goto __ota_exit;
    }
    block_size = RECV_BLOCK_SIZE;
    fread(fsdc, buf, block_size);
    /*
     * 校验文件，必须从0开始校验
     * 如果传入的长度 < 文件长度，则只校验头的部分
     */
    req.info.type = UPGRADE_TYPE_BUF;
    req.info.input.data.buf = buf;
    req.info.input.data.size = block_size;
    req.info.offset = 0;
    err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
    if (err) {
        log_e("upgrade file err : 0x%x\n", err);
        goto __ota_exit;
    }

    /*
     * 升级信息校验，确定系统是否需要升级
     */
    req.info.type = UPGRADE_TYPE_BUF;
    req.info.input.data.buf = buf;
    req.info.input.data.size = block_size;
    req.info.offset = 0;
    err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_SYSTEM, &req);
    if (err) {
        log_e("system not match to file : 0x%x\n", err);
        goto __ota_exit;
    }
    /*
     * 这里已经确定可以升级
     * 可以在这里进行升级确认，以及插电提醒
     */

    /*
     * 开始升级
     */
    size = flen(fsdc);
    do {
        req.core.type = UPGRADE_TYPE_BUF;
        req.core.input.data.buf = buf;
        req.core.input.data.size = block_size;
        req.core.offset = offset;
        err = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
        if (err) {
            log_e("upgrade core run err : 0x%x\n", err);
            goto __ota_exit;
        }

        offset += block_size;
        size -= block_size;
        block_size = size > RECV_BLOCK_SIZE ? RECV_BLOCK_SIZE : size;
        fread(fsdc, buf, block_size);
    } while (block_size);


    server_close(upgrade_ser);
    fdelete(fsdc);//SFC sd卡升级完成删除源文件
    cpu_reset();
__ota_exit:
    server_close(upgrade_ser);
    if (fsdc) {
        fclose(fsdc);
    }

    if (buf) {
        free(buf);
    }

    return 0;
}
int upgrade_detect(const char *sdcard_name)
{
    int err;
    int msg[32];
    int retry = 0;
    memset(__this, 0x0, sizeof(*__this));

    int mount_count = 0;

    server_load(upgrade_server);
    puts("upgrade_normal_mode\n");
    mount_sd_to_fs(SDX_DEV);

#if STORAGE_RETRY_ENABLE
    while (!storage_device_ready() && retry++ < 20) {
        log_d("sd not ready, retry %d", retry);
        if (retry >= 20) {
            log_e("sd not ready, exit");
            return -ENODEV;
        }
        os_time_dly(10);
    }
#else
    if (!storage_device_ready()) {
        return -ENODEV;
    }
#endif

#ifdef CONFIG_SFC_ENABLE
    return sfc_mode_file_upgrade();
#else
    struct intent it;

    init_intent(&it);
    it.name = "sdcard_upgrade";
    it.action = ACTION_UPGRADE_MAIN;
    it.data = SDX_DEV;
    start_app(&it);
    while (1) {
        os_time_dly(2);
        os_taskq_accept(ARRAY_SIZE(msg), msg);
        if (__this->upgrade_out) {
            break;
        }
    }

    if (!__this->success) {
        it.action = ACTION_BACK;
        start_app(&it);
        return -EFAULT;
    }
    return 0;
#endif
    /*return start_upgrade_detect(sdcard_name);*/
}
