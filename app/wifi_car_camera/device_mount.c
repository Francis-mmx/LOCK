#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "net_config.h"

#include "app_config.h"

enum {
    SD_UNMOUNT,
    SD_MOUNT_SUSS,
    SD_MOUNT_FAILD,
};

static char *const sd_list[] = {
    "sd0",
    "sd1",
    "sd2",
};

static u32 class;
static u8 fs_mount = SD_UNMOUNT;
static OS_MUTEX sd_mutex;

static int __sd_mutex_init()
{
    return os_mutex_create(&sd_mutex);
}
early_initcall(__sd_mutex_init);


int mount_sd_to_fs(const char *name)
{
    extern u32 avi_rcv_time;
    if (avi_rcv_time) {
        extern u8 fs_del_unclosed_file;
        fs_del_unclosed_file = 0;
    }

    int err = 0;
    struct imount *mt;
    int id = ((char *)name)[2] - '0';
    const char *dev  = sd_list[id];

    err = os_mutex_pend(&sd_mutex, 0);
    if (err) {
        return -EFAULT;
    }


    if (fs_mount == SD_MOUNT_SUSS) {
        goto __exit;
    }
    if (fs_mount == SD_MOUNT_FAILD) {
        err = -EFAULT;
        goto __exit;
    }
    if (!dev_online(dev)) {
        err = -EFAULT;
        goto __exit;
    }

    void *fd = dev_open(dev, 0);
    if (!fd) {
        err = -EFAULT;
        goto __err;
    }
    dev_ioctl(fd, SD_IOCTL_GET_CLASS, (u32)&class);
    if (class == SD_CLASS_10) {
        puts("sd card class: 10\n");
    } else {
        log_w("sd card class: %d\n", class * 2);
    }
    dev_close(fd);

    mt = mount(dev, CONFIG_STORAGE_PATH, "fat", FAT_CACHE_NUM, NULL);
    if (!mt) {
        puts("mount fail\n");
        err = -EFAULT;
    } else {
        puts("mount sd suss\n");
#ifdef CONFIG_ENABLE_VLIST
        FILE_LIST_IN_MEM(1);
#endif
    }

__err:
    fs_mount = err ? SD_MOUNT_FAILD : SD_MOUNT_SUSS;
__exit:
    os_mutex_post(&sd_mutex);

    return err;
}

void unmount_sd_to_fs(const char *path)
{
    os_mutex_pend(&sd_mutex, 0);

    unmount(path);
    fs_mount = SD_UNMOUNT;

#ifdef CONFIG_ENABLE_VLIST
    extern int FILE_LIST_EXIT();
    FILE_LIST_EXIT();
#endif
    os_mutex_post(&sd_mutex);
}

int storage_device_ready()
{
    if (!dev_online(SDX_DEV)) {
        return false;
    }
    if (fs_mount == SD_UNMOUNT) {
        mount_sd_to_fs(SDX_DEV);
    }

    return fdir_exist(CONFIG_STORAGE_PATH);
}

int storage_device_format()
{
    int err;

    unmount_sd_to_fs(CONFIG_STORAGE_PATH);

    err = f_format(SDX_DEV, "fat", 32 * 1024);
    if (err == 0) {
        mount_sd_to_fs(SDX_DEV);
    }

    return err;
}

int storage_device_format2()
{
    int err;
    struct imount *mt = NULL;

    unmount_sd_to_fs(CONFIG_STORAGE_PATH);

    /* err = f_format(SDX_DEV, "fat", 32 * 1024); */
    void fatfs_set_device(char *dev_name);
    fatfs_set_device(SDX_DEV);
    mt = mount(SDX_DEV, CONFIG_STORAGE_PATH, "flashfs", 0, NULL);
    err = f_format(SDX_DEV, "flashfs", 32 * 1024);
    if (err == 0) {
        unmount(CONFIG_STORAGE_PATH);
        mount_sd_to_fs(SDX_DEV);
    }

    return err;
}

int fatfs_create_dir_demo(char *dir_name)
{
    void fatfs_set_device(char *dev_name);
    fatfs_set_device(SDX_DEV);
    const char *root_path = "stroage/sdcard";
    char dir_path[64];
    struct imount *mt = NULL;
    mt = mount(SDX_DEV, root_path, "flashfs", 0, NULL);
    sprintf(dir_path, "%s/C/%s/", root_path, dir_name);
    FILE *f = fopen(dir_path, "r");
    if (!f) {
        printf("%s not exist\n", dir_path);
        //不存在则创建
        f = fopen(dir_path, "w+");
        if (f) {
            printf("create %s sucess\n", dir_path);
            fclose(f);
        } else {
            printf("create %s failed\n", dir_path);
        }
    }
    if (mt) {
        unmount(root_path);
    }
}
/*
 * sd卡插拔事件处理
 */
static void sd_event_handler(struct sys_event *event)
{
    int id = ((char *)event->arg)[2] - '0';
    const char *dev  = sd_list[id];

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        mount_sd_to_fs(event->arg);
        break;
    case DEVICE_EVENT_OUT:
        printf("%s: out\n", dev);
        unmount_sd_to_fs(CONFIG_STORAGE_PATH);
        break;
    }
}


static void device_event_handler(struct sys_event *event)
{
    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        sd_event_handler(event);
    } else if (!ASCII_StrCmp(event->arg, "usb", 4)) {

    }
}
/*
 * 静态注册设备事件回调函数，优先级为0
 */
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, device_event_handler, 0);





