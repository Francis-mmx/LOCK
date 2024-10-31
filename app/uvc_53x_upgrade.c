#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "device/usb_scsi_upgrade.h"

#include "app_config.h"

#define UVC_AC53X_SLAVE_UPGRADE_EN   1   //升级AC53x从机功能


#define AC53X_BFU_FILE_PATH     CONFIG_ROOT_PATH"AC53x.bfu"

static volatile u8 enter_upgrade = 0;

/**
 * @brief uvc从机进入升级模式函数
 *
 * @param void
 *
 * @return 零表示失败
 *         非零表示成功,成功后将进入 uboot_dev_event_handler 的 DEVICE_EVENT_IN 分支，在里面读取bfu并升级
 */
int uvc_enter_upgrade_mode(void)
{
    int usb_host_slave_enter_upgrade(void);
    enter_upgrade = usb_host_slave_enter_upgrade();

    return enter_upgrade;
}

/**
 * @brief uvc从机升级过程进度回调,里面获取升级进度百分比
 *
 * @param percent: 进度百分比
 *
 * @return
 */
void usb_upgrade_progress_percent(u32 percent)
{
    printf("upgrade >> %d\n", percent);
    return;
}

/**
 * @brief uvc从机升级过程中异常处理
 *        升级过程中中断，即变砖，从机大概率会进maskrom 的uvc模式
 *        在改函数重新发命令进入升级模式
 *
 * @param void
 *
 * @return
 */
int uvc_frame_not_find(void)
{
    printf("uvc frame not find\n");
    if (enter_upgrade) {
        uvc_enter_upgrade_mode();
    }
    return 1;
}

/**
 * @brief 设置 uvc从机是否成功进入升级模式
 * @param success:
 *        0: 未进入升级模式
 *        1: 进入了升级模式
 * @return
 */
void set_uvc_upgrade_state(u8 success)
{
    enter_upgrade = success;
}
/**
 * @brief 获取 uvc从机是否进入了升级模式
 * @param void
 * @return
 *        0: 未进入升级模式
 *        1: 进入了升级模式
 */
u8 get_uvc_upgrade_state(void)
{
    return enter_upgrade;
}

static void uboot_dev_event_handler(struct sys_event *event)
{
    char *const udisk_list[] = {
        "uboot0",
        "uboot1",
    };
    int id = ((char *)event->arg)[5] - '0';
    const char *dev  = udisk_list[id];

    FILE *fp = NULL;
    u8 *bfu_stream = NULL;
    int bfu_len = 0;
    u8 usb_burn_state = 0;

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        fp = fopen(AC53X_BFU_FILE_PATH, "r");
        if (!fp) {
            printf("open bfu faild!!!\n");
            enter_upgrade = 0;
            break;
        }

        printf("\n %s %s: in\n", __func__, dev);
        bfu_len = flen(fp);
        printf("bfu_len=%d\n", bfu_len);
        bfu_stream = malloc(bfu_len);
        if (!bfu_stream) {
            fclose(fp);
            enter_upgrade = 0;
            break;
        }
        fread(fp, bfu_stream, bfu_len);
        //升级进程主函数
        u32 usb_burn_process(unsigned char *bfu_stream, int bfu_len);
        usb_burn_state = usb_burn_process(bfu_stream, bfu_len);
        enter_upgrade = 0;

        fclose(fp);
        break;
    case DEVICE_EVENT_OUT:
        /* printf("\n %s %s: out\n", __func__, dev); */
        break;
    }
}

static void uvc_upgrade_event_handler(struct sys_event *event)
{
    FILE *fp = NULL;

    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
#if 1
            //for test
            fp = fopen(AC53X_BFU_FILE_PATH, "r");
            if (fp) {
                fclose(fp);
                //根目录下有AC53升级文件，进入升级模式
                //用户也可设计其他方式触发进入升级
                uvc_enter_upgrade_mode(); //测试进入升级模式
                event->consumed = 1;
#endif
            }

            break;
        case DEVICE_EVENT_OUT:
            break;
        }
    }

    if (!ASCII_StrCmp(event->arg, "udisk*", 7)) {
        if (enter_upgrade) {
            uboot_dev_event_handler(event);
        }
    }

#ifdef CONFIG_USB_UVC_AND_UAC_ENABLE
    if (!strcmp((char *)event->arg, "usb mic") || !strcmp((char *)event->arg, "usb speaker")) {
        if (event->u.dev.event == DEVICE_EVENT_OUT) {
            if (enter_upgrade) {
                if (!strcmp((char *)event->arg, "usb mic")) {
                    extern int play_usb_mic_stop();
                    play_usb_mic_stop();
                } else if (!strcmp((char *)event->arg, "usb speaker")) {
                    extern int play_usb_speaker_stop();
                    play_usb_speaker_stop();
                }
                extern void user_usb_host_set_ops(struct usb_host_user_ops * ops);
                user_usb_host_set_ops(NULL);
            }
        }
    }
#endif
}
#if UVC_AC53X_SLAVE_UPGRADE_EN
static int uvc_upgrade_event_init(void)
{
    return register_sys_event_handler(SYS_DEVICE_EVENT, 0, NULL, uvc_upgrade_event_handler);
}
late_initcall(uvc_upgrade_event_init);
#endif

