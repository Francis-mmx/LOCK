/*************************************************************************
	> File Name: usb_app.c
	> Author:
	> Mail:
	> Created Time: Tue 14 Feb 2017 09:14:45 AM HKT
 ************************************************************************/

#include "system/includes.h"
#include "server/usb_server.h"
#include "app_config.h"
#include "action.h"
#include "server/ui_server.h"
#include "style.h"
#include "storage_device.h"
#include "user_isp_cfg.h"
#include "server/video_server.h"

extern u8 get_current_disp_device();
enum {
    USB_SLAVE_MASS_STORAGE,
    USB_SLAVE_CAMERA,
    USB_SLAVE_ISP_TOOL,
    USB_SLAVE_MAX,
};

#define CUSTOM_RESOLUTION        1
#define USER_DEFINED_TOOL        1
#define UPDATE_UVC_ISP_SCENES    1

#define USB_STATE_NO_DEV         0x0
#define USB_STATE_DEVICE_MOUNT   0x1

struct usb_app_handle {
    u8 state;
    u8 mode;
#if UPDATE_UVC_ISP_SCENES
    u8 isp_scene;
#endif
    struct server *usb_slave;
    struct server *usb_host;
    struct server *ui;
    u8 *buf;
    u32 buf_size;
};

static struct usb_app_handle handler = {
    .state = USB_STATE_NO_DEV,
};

#define __this  (&handler)

extern int usb_app_set_config(struct intent *it);
extern int usb_app_get_config(struct intent *it);
/*
static int usb_app_mode_start(void)
{
    int err = 0;

    if (!__this->usb_slave){

        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave){
            return -EFAULT;
        }
    }

    return err;
}
*/

/*
 *mass storage挂载的设备列表
 */
const static char *mass_storage_dev_list[] = {
    SDX_DEV,
};


static int show_slave_list(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -EINVAL;
    }
    req.show.id = ID_WINDOW_USB_SLAVE;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif
    return 0;
}

static int hide_slave_list(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -EINVAL;
    }
    req.hide.id = ID_WINDOW_USB_SLAVE;
    server_request(__this->ui, UI_REQ_HIDE, &req);

#endif
    return 0;

}
static int usb_slave_start(void)
{
    show_slave_list();

    //usb_isp_tool_test();

    return 0;
}

static int usb_slave_stop(void)
{
    /*
     *关闭服务、释放资源
     */
#if UPDATE_UVC_ISP_SCENES
    if (__this->isp_scene) {
        stop_update_isp_scenes();
    }
#endif

    if (__this->usb_slave) {
        server_close(__this->usb_slave);
        __this->usb_slave = NULL;
        unmount_sd_to_fs(CONFIG_STORAGE_PATH);
        mount_sd_to_fs(SDX_DEV);
    }

    if (__this->buf) {
        free(__this->buf);
        __this->buf = NULL;
    }


    __this->state = USB_STATE_NO_DEV;

    hide_slave_list();
    return 0;
}


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    extern int set_usb_mass_storage();
    extern int set_usb_camera();

    switch (state) {
    case APP_STA_CREATE:
        memset(__this, 0x0, sizeof(*__this));
#ifdef CONFIG_UI_ENABLE
#ifdef MULTI_LCD_EN
        struct ui_style style = {0};
        if (get_current_disp_device()) {
            style.file = "mnt/spiflash/res/avo_LY.sty\0";
        } else {
            style.file = "mnt/spiflash/res/lcd_LY.sty\0";
        }
        __this->ui = server_open("ui_server", &style);
#else
        __this->ui = server_open("ui_server", NULL);
#endif

        if (!__this->ui) {
            return -EFAULT;
        }
#endif
#ifdef CONFIG_GSENSOR_ENABLE
        sys_power_clr_wakeup_reason("wkup_port:wkup_gsen");
#endif
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_USB_SLAVE_MAIN:
            usb_slave_start();
            __this->mode = 0;
            break;
        case ACTION_USB_SLAVE_SET_CONFIG:
            __this->mode = usb_app_set_config(it);
            break;
        case ACTION_USB_SLAVE_GET_CONFIG:
            usb_app_get_config(it);
            break;
        }
        break;
    case APP_STA_PAUSE:
    /* return -EFAULT; */
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        usb_slave_stop();
        break;
    case APP_STA_DESTROY:
        if (__this->ui) {
            server_close(__this->ui);
        }
        break;
    }

    return 0;

}
static int usb_app_key_event_handler(struct key_event *key)
{

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            //usb_app_select();
            break;
        case KEY_UP:

            break;
        case KEY_DOWN:

            break;
        default:
            break;
        }
        break;
    case KEY_EVENT_LONG:
        if (key->value == KEY_POWER) {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

static int usb_app_device_event_handler(struct sys_event *event)
{

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_ONLINE:
        break;
    case DEVICE_EVENT_OUT:
        //usb_app_pause();
        break;
    }
    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{

    switch (event->type) {
    case SYS_KEY_EVENT:
        return usb_app_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return usb_app_device_event_handler(event);
    default:
        return false;
    }
    return false;
}


static const struct application_operation usb_app_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_usb) = {
    .name 	= "usb_app",
    .action	= ACTION_USB_SLAVE_MAIN,
    .ops 	= &usb_app_ops,
    .state  = APP_STA_DESTROY,
};


#if USER_DEFINED_TOOL
/*
 *user defined tool demo
 */
static int usb_tool_response(void *_tool, void *buf, int len)
{
    struct usb_tool *tool = (struct usb_tool *)_tool;
    u8 *cmd = (u8 *)buf;

    if (!tool || !buf) {
        return -EINVAL;
    }

    if (cmd[0] != USER_DEFINED_TOOL_CMD) {
        log_w("not user defined tool");
        return 0;
    }

    switch (cmd[1]) {
    case 0x0:
        break;
    default:
        break;
    }
    return 0;
}
#endif
static void set_sd_card_protect_status(u8 protect)
{
    void *sd_dev = NULL;
    sd_dev = dev_open(SDX_DEV, 0);
    if (!sd_dev) {
        log_e("%s open falid!\n");
        return;
    }
    dev_ioctl(sd_dev, IOCTL_SET_WRITE_PROTECT, protect);
    dev_close(sd_dev);
}
/*
 *设置mass storage
 */
int set_usb_mass_storage(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        //printf("state mass storage is opened, state : 0x%x.\n", __this->state);
        return 0;
    }
    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            puts("usb server open failed.\n");
            return -EFAULT;
        }
    }

    set_sd_card_protect_status(0); //0:不写保护 1:写保护
    //printf("debug isd download.\n");
    /*
     *设置请求参数：
     *类型：USB_MASS_STORAGE
     *挂载设备数量
     *挂载设备列表
     *状态：从机连接
     */
    req.type = USB_MASS_STORAGE;
    req.storage.dev_num = 1;
    req.storage.dev = mass_storage_dev_list;
    /*req.storage.name = "hunter"; //PC上显示的卷标*/
#if USER_DEFINED_TOOL
    req.storage.tool_response = usb_tool_response;
#endif
    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    __this->state = USB_STATE_DEVICE_MOUNT;
    sys_power_auto_shutdown_pause();
    return 0;
}

#if CUSTOM_RESOLUTION
static struct uvc_reso_info uvc_mjpg_reso_info = {
    .num = 5,
    .reso = {
        {1920, 1088},
        {1280, 720},
        {640, 480},
        {640, 400},
        {640, 320},
    },
};
#endif
/*
 *设置PC Camera
 */
int set_usb_camera(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }

    /*
     *设置请求参数
     *类型：USB_CAMERA (可增加isp工具，调屏工具选项)
     *设置camera的编码所需buffer、buffer长度
     *quality -- 设置图像质量
     */
    req.type = USB_CAMERA | USB_ISP_TOOL | USB_SCREEN_TOOL;
    req.camera[0].name = "video0.0";
    if (!__this->buf) {
        __this->buf = (u8 *)malloc(USB_CAMERA_BUF_SIZE);
        if (!__this->buf) {
            return -ENOMEM;
        }
    }
    __this->buf_size = USB_CAMERA_BUF_SIZE;
    req.camera[0].enable = 1;
    req.camera[0].buf = __this->buf;
    req.camera[0].buf_size = __this->buf_size;
    req.camera[0].info = NULL;
    req.camera[0].quality = 1;
#if CUSTOM_RESOLUTION
    req.camera[0].info = &uvc_mjpg_reso_info;
#endif

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

#if UPDATE_UVC_ISP_SCENES
    struct isp_generic_cfg cfg = {0};
    struct ispt_customize_cfg cus_cfg = {0};

    cfg.lv = 0;
    cfg.id = ISP_GET_LV;
    load_default_camera_config(cfg.lv, (void *)&cus_cfg);
    start_update_isp_scenes(__this->usb_slave);
    __this->isp_scene = 1;
#endif
    __this->state = USB_STATE_DEVICE_MOUNT;
    sys_power_auto_shutdown_pause();
    return 0;
}
int set_usb_microphone(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }

    req.type = USB_MIC;
    req.microphone.name = "mic";  //数据源
    req.microphone.format = "pcm";//编码格式
    req.microphone.buf = NULL;
    req.microphone.buf_size = 0;
    req.microphone.info = NULL;

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    __this->state = USB_STATE_DEVICE_MOUNT;
    sys_power_auto_shutdown_pause();

    return 0;
}
/*
 *设置PC COMPOSITEN DEVICE
 */
int set_usb_composite_device(void)
{
#define USB_CD_MIC_EN 1
#define USB_CD_VIDEO_EN 1
#define USB_CD_MSD_EN 0

    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }



#if USB_CD_VIDEO_EN
    /*
     *设置请求参数
     *类型：USB_CAMERA (可增加isp工具，调屏工具选项)
     *设置camera的编码所需buffer、buffer长度
     *quality -- 设置图像质量
     */
    req.type = (USB_CAMERA | USB_ISP_TOOL);
    req.camera[0].name = "video0.0";
    if (!__this->buf) {
        __this->buf = (u8 *)malloc(USB_CAMERA_BUF_SIZE);
        if (!__this->buf) {
            return -ENOMEM;
        }
    }
    __this->buf_size = USB_CAMERA_BUF_SIZE;
    req.camera[0].enable = 1;
    req.camera[0].buf = __this->buf;
    req.camera[0].buf_size = __this->buf_size;
    req.camera[0].info = NULL;
    req.camera[0].quality = 1;
#if CUSTOM_RESOLUTION
    req.camera[0].info = &uvc_mjpg_reso_info;
#endif

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

#if UPDATE_UVC_ISP_SCENES
    start_update_isp_scenes(__this->usb_slave);
    __this->isp_scene = 1;
#endif
#endif

#if USB_CD_MIC_EN
    req.type = USB_MIC;
    req.microphone.name = "mic";  //数据源
    req.microphone.format = "pcm";//编码格式
    req.microphone.buf = NULL;
    req.microphone.buf_size = 0;
    req.microphone.info = NULL;

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }
#endif

#if USB_CD_MSD_EN
    /*
     *设置请求参数：
     *类型：USB_MASS_STORAGE
     *挂载设备数量
     *挂载设备列表
     *状态：从机连接
     */
    req.type = USB_MASS_STORAGE;
    req.storage.dev_num = 1;
    req.storage.dev = mass_storage_dev_list;
    /*req.storage.name = "hunter"; //PC上显示的卷标*/
#if USER_DEFINED_TOOL
    req.storage.tool_response = usb_tool_response;
#endif
    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }
#endif

    __this->state = USB_STATE_DEVICE_MOUNT;
    sys_power_auto_shutdown_pause();

    return 0;
}
/*
 *返回录像模式
 */
int back_to_video_rec(void)
{
    struct intent it;
    struct application *app;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }
    init_intent(&it);
    app = get_current_app();
    if (app) {
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
    }

    return 0;
}

u8 get_usb_in_status()
{
    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 1;
    }
    return 0;
}



#if 0
#define mbe32buf_to_cpu(x)   (((u32)(((u8 *)(x))[0]) << 24) | \
                        	((u32)(((u8 *)(x))[1]) << 16) | \
                        	((u32)(((u8 *)(x))[2]) << 8) | \
                        	((u32)((u8 *)(x))[3]))
#define mbe16buf_to_cpu(x)   (((u16)(((u8 *)(x))[0]) << 8) | (u16)(((u8 *)(x))[1]))

int user_cmd_read(struct scsi_private_request *req)
{
    u32 cmd;
    u16 len;
    u8 buf[16];//每次传输数据少于512byte

    cmd = mbe32buf_to_cpu(req->cmd + 2);
    len = mbe16buf_to_cpu(req->cmd + 6);

    switch (cmd) {
    case 0x12345678:
        //fill data to buf
        strcpy((char *)buf, "demo");
        //send data to pc
        req->ops.send(req->dev, (void *)buf, len);
        break;
    default:
        break;
    }
    return 0;
}


int user_cmd_write(struct scsi_private_request *req)
{
    u32 cmd;
    u16 len;
    u32 get_time;
    u8 buf[16];//每次传输数据少于512byte
    cmd = mbe32buf_to_cpu(req->cmd + 2);
    len = mbe16buf_to_cpu(req->cmd + 6);

    switch (cmd) {
    case 0x12345678:
        //get data form pc
        req->ops.read(req->dev, (void *)buf, len);
        /* printf("\n %s\n",buf); */
        break;
    default:
        break;
    }
    return 0;
}

#endif

