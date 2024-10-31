/*************************************************************************
	> File Name: app/car_camera/network_upgrade.c
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
#include "os/os_api.h"
#include "os/os_compat.h"
#include "network_upgrade.h"

#define NETWORK_UPGRADE_USE_MALLOC_BUFF	1	// 1 使用malloc作为缓冲区， 0 使用静态区作为缓冲区,  默认malloc

#define BFU_DATA_ADDR					(0x10)
#define UPGRADE_FILE_CHECK_PAGE_SIZE 	(4 * 1024)
#define RECV_BLOCK_SIZE    				(4 * 1024)
#define UPGRADE_FILE_SIZE 				(UPGRADE_FILE_CHECK_PAGE_SIZE * 24)//必须大于FTP里的BUFF+4KB，FTP BUFF默认44*1460 Byte

//无卡或者sfc网络升级配置在:network_upgrade.h，设置NETWORK_UPGRADE_TO_FLASH 为 1 即可

int wifi_file_upgrade_demo(void);
int wifi_file_upgrade_buff(char *pbuf, int len);

static char network_flash_upgrade_state = 0;

#if !NETWORK_UPGRADE_USE_MALLOC_BUFF
static char flash_buff[UPGRADE_FILE_SIZE] __attribute__((aligned(32)));
#endif

struct network_upgrade_info {
    int fbuff_offset;//flash升级文件校验偏移量
    int offset;
    int flen;
    struct server *up_ser;
    struct server *ui;
    FILE *file;
    int prev_id;
    union sys_upgrade_req req;
    OS_MUTEX mutex;
    u8 is_flash;
    u8 file_read;//是否是读文件
    char path[64];
    char *fbuff;//flash升级文件校验buff，4Kb
};

static void network_upgrade_success_and_reset_system(void)
{
    printf("cpu reset ....\n\n");
    cpu_reset();
}
char network_flash_upgrade_state_get(void)
{
    return network_flash_upgrade_state;
}
void *network_upgrade_open(char *path, char *mode)
{
    int is_flash = 0;
    struct server *upgrade_server;
    FILE *fd;

    struct network_upgrade_info *nwug_info = zalloc(sizeof(struct network_upgrade_info));

    if (!nwug_info) {
        printf("network_upgrade_open mallo err !!!\n");
        return NULL;
    }

    struct network_upgrade_info *__this = nwug_info;

    is_flash = NETWORK_UPGRADE_TO_FLASH;

    if (is_flash) {
        if (strstr(path, UPGRADE_FILE_NAME) || strstr(path, UPGRADE_SFC_FILE_NAME)) {
#if NETWORK_UPGRADE_USE_MALLOC_BUFF
            __this->fbuff = (char *)malloc(UPGRADE_FILE_SIZE);
            if (!__this->fbuff) {
                printf("no mem !!\n");
                free(__this);
                return NULL;
            }
#endif
            server_load(upgrade_server);
            __this->up_ser	= server_open("upgrade_server", NULL);
            if (!__this->up_ser) {
                printf("flash_upgrade server open err !!!\n");
                if (__this->fbuff) {
                    free(__this->fbuff);
                }
                free(__this);
                return NULL;
            }
            __this->is_flash = TRUE;
            network_flash_upgrade_state = TRUE;
            if (storage_device_ready()) {//flash升级必须把SD卡的删除，否则重启会升级回原来的版本
                fd = fopen(path, "r");
                if (fd) {
                    fdelete(fd);
                }
            }
            printf("\n\n system firmware update is doing now , please keep the power on !!!!!!!!!!\n\n\n\n");
        } else {
            goto file_storage;
        }
    } else {
file_storage:
        __this->is_flash = FALSE;
        if (storage_device_ready()) {
            __this->file = fopen(path, mode);
            if (strstr(mode, "r")) {
                __this->file_read = TRUE;
            }
            if (!__this->file) {
                printf("open upgrade file err !!!\n");
                free(__this);
                return NULL;
            }
        } else {
            free(__this);
            return NULL;
        }
    }
    os_mutex_create(&__this->mutex);
    memcpy(__this->path, path, strlen(path));
    return (void *)__this;
}

int network_upgrade_write(void *file, char *buf, int len)
{
    int err;
    char *pbuf = buf;
    int page = len;
    int addr = 0;
    u32 *data_addr;
    u32 *data_len;
    struct network_upgrade_info *__this = (struct network_upgrade_info *)file;

    if (!__this) {
        return -1;
    }
    os_mutex_pend(&__this->mutex, 0);
    if (__this->is_flash) {
#if NETWORK_UPGRADE_USE_MALLOC_BUFF
        if (__this->fbuff) {
            if (len <= (UPGRADE_FILE_SIZE - UPGRADE_FILE_CHECK_PAGE_SIZE)) {
                memcpy(__this->fbuff + __this->fbuff_offset, buf, len);
                __this->fbuff_offset += len;
                if (__this->fbuff_offset < UPGRADE_FILE_CHECK_PAGE_SIZE) {
                    os_mutex_post(&__this->mutex);
                    return len;
                }
            } else {
                printf("__this->fbuff no enough !!!!\n\n\n");
                os_mutex_post(&__this->mutex);
                return 0;
            }
        } else {
            printf("__this->fbuff is NULL !!!\n\n");
            os_mutex_post(&__this->mutex);
            return 0;
        }
#else
        if (len <= (UPGRADE_FILE_SIZE - UPGRADE_FILE_CHECK_PAGE_SIZE)) {
            memcpy(flash_buff + __this->fbuff_offset, buf, len);
            __this->fbuff_offset += len;
            if (__this->fbuff_offset < UPGRADE_FILE_CHECK_PAGE_SIZE) {
                os_mutex_post(&__this->mutex);
                return len;
            }
        } else {
            printf("flash_buff no enough !!!!\n\n\n");
            os_mutex_post(&__this->mutex);
            return 0;
        }
#endif
        while (1) {
            page = MIN(__this->fbuff_offset, UPGRADE_FILE_CHECK_PAGE_SIZE);
            if (page < UPGRADE_FILE_CHECK_PAGE_SIZE) {
                break;
            }

#if NETWORK_UPGRADE_USE_MALLOC_BUFF
            pbuf = __this->fbuff + addr;
#else
            pbuf = flash_buff + addr;
#endif
            __this->fbuff_offset -= page;
            addr += page;
            if (__this->offset == 0) {
                __this->req.info.type = UPGRADE_TYPE_BUF;
                __this->req.info.input.data.buf = pbuf;
                __this->req.info.input.data.size = page;
                __this->req.info.offset = 0;
                err = server_request(__this->up_ser, UPGRADE_REQ_CHECK_FILE, &__this->req);
                if (err) {
                    __this->offset = 0;
                    printf("upgrade file err : 0x%x\n", err);
                    os_mutex_post(&__this->mutex);
                    return err;
                }
                err = server_request(__this->up_ser, UPGRADE_REQ_CHECK_SYSTEM, &__this->req);
                if (err) {
                    __this->offset = 0;
                    printf("system not match to file : 0x%x \n", err);
                    os_mutex_post(&__this->mutex);
                    return err;
                }

                /*check file len */
                data_addr = (u32 *)(pbuf + BFU_DATA_ADDR);
                data_len = (u32 *)(pbuf + BFU_DATA_ADDR + sizeof(u32));
                if (*data_addr && *data_len) {
                    __this->flen = *data_len + *data_addr;
                    printf("%s flen : %d \n", __this->path, __this->flen);
                }

            }
            __this->req.core.type = UPGRADE_TYPE_BUF;
            __this->req.core.input.data.buf = pbuf;
            __this->req.core.input.data.size = page;
            __this->req.core.offset = __this->offset;
            err = server_request(__this->up_ser, UPGRADE_REQ_CORE_START, &__this->req);
            if (err) {
                __this->offset = 0;
                printf("upgrade core run err : 0x%x\n", err);
                os_mutex_post(&__this->mutex);
                return err;
            }
            __this->offset += page;
        }
        printf(">>write size: %d byte\n", __this->offset);
#if NETWORK_UPGRADE_USE_MALLOC_BUFF
        memcpy(__this->fbuff, __this->fbuff + addr, __this->fbuff_offset);
#else
        memcpy(flash_buff, &flash_buff[addr], __this->fbuff_offset);
#endif
    } else if (__this->file) {
        err = fwrite(__this->file, buf, len);
        if (err <= 0) {
            __this->offset = 0;
            printf("Write upgrade file err\n");
            os_mutex_post(&__this->mutex);
            return err;
        }
        __this->offset += len;
    }
    os_mutex_post(&__this->mutex);
    return len;
}
int network_upgrade_read(void *file, char *buf, int len)
{
    int err = 0;
    struct network_upgrade_info *__this = (struct network_upgrade_info *)file;

    os_mutex_pend(&__this->mutex, 0);
    if (!__this->is_flash) {
        err = fread(__this->file, buf, len);
        if (err <= 0) {
            __this->offset = 0;
        }
        __this->offset += len;
    }
    os_mutex_post(&__this->mutex);
    return err;
}

int network_upgrade_flen(void *file)
{
    int err = 0;
    struct network_upgrade_info *__this = (struct network_upgrade_info *)file;

    os_mutex_pend(&__this->mutex, 0);
    if (!__this->is_flash) {
        err = flen(__this->file);
        __this->flen = err;
    }
    os_mutex_post(&__this->mutex);
    return err;
}

int network_upgrade_close(void *file)
{
    struct network_upgrade_info *__this = (struct network_upgrade_info *)file;
    int err;
    int addr = 0;
    int page;
    char *pbuf;

    if (!__this) {
        return -1;
    }
    os_mutex_pend(&__this->mutex, 0);
    if (__this->is_flash) {
        if (!__this->up_ser) {
            goto exit;
        }
        if (__this->fbuff_offset > 0 && __this->offset > 0) {
            while (__this->fbuff_offset) {
                page = MIN(__this->fbuff_offset, UPGRADE_FILE_CHECK_PAGE_SIZE);
#if NETWORK_UPGRADE_USE_MALLOC_BUFF
                pbuf = __this->fbuff + addr;
#else
                pbuf = flash_buff + addr;
#endif
                addr += page;

                __this->req.core.type = UPGRADE_TYPE_BUF;
                __this->req.core.input.data.buf = pbuf;
                __this->req.core.input.data.size = page;
                __this->req.core.offset = __this->offset;
                err = server_request(__this->up_ser, UPGRADE_REQ_CORE_START, &__this->req);
                if (err) {
                    __this->offset = 0;
                    printf("upgrade core run err : 0x%x\n", err);
                    break;;
                }
                __this->offset += __this->fbuff_offset;
                __this->fbuff_offset -= page;
                printf(">>>write size: %d byte\n", __this->offset);
            }
        }
        if (__this->offset > 0 && __this->offset == __this->flen) {
            printf(">>>>>>>>>>>network_upgrade successfuly , system will reset after 2s ...\n\n");
            sys_timeout_add_to_task("sys_timer", NULL, network_upgrade_success_and_reset_system, 2000);
        } else {
            printf(">>>>>>>>>>>network_upgrade error\n\n");
        }
        server_close(__this->up_ser);
exit:
        if (__this->fbuff) {
            free(__this->fbuff);
        }
        os_mutex_post(&__this->mutex);
        os_mutex_del(&__this->mutex, 0);
        free(__this);
        network_flash_upgrade_state = FALSE;
    } else if (__this->file) {
        fclose(__this->file);
        if (__this->offset > 0 && !__this->file_read) {
            if (strstr(__this->path, UPGRADE_FILE_NAME)) {
                printf("network_upgrade successfuly , please reset system to upgrede firmware \n\n");
            }
        }
        if (__this->fbuff) {
            free(__this->fbuff);
        }
        os_mutex_post(&__this->mutex);
        os_mutex_del(&__this->mutex, 0);
        free(__this);
    }
    printf("file close \n");
    return 0;
}

//一次性buffer升级
int network_upgrade_request(char *buf, u32 len)
{
    int err;
    err = wifi_file_upgrade_buff(buf, len);
    if (err) {
        printf("network_upgrade_request err !!\n");
    }
    return err;
}



/*
 * wifi 升级配置流程：
 *
 * 1.打开upgrade_server 服务，可以配置参数struct sys_upgrade_param
 *   param.buf = 申请固定长度的buffer 4K对齐
 *   param.buf_size = 长度
 *   param.dev_name = "spiflash"
 * 2.请求 UPGRADE_REQ_CHECK_FILE *进行数据校验*
 * 3.请求 UPGRADE_REQ_CHECK_SYSTEM  *进行系统升级检测*
 * 4.从offset为0开始进行一块一块数据升级请求 UPGRADE_REQ_CORE_START
 * 5.升级完成关闭服务，复位重启
 */

/*
 * flash升级文件测试代码，wifi可以参照这个写文件
 */
int wifi_file_upgrade_demo(void)
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
        printf("open upgrade server error\n");
        return -EINVAL;
    }

    /*FILE *fsdc = fopen(UPGRADE_FILE_PATH, "r");*/
    FILE *fsdc = fopen(UPGRADE_PATH"test.bin", "r");
    if (!fsdc) {
        printf("open test ota_ver2.bfu error\n");
        return -ENOENT;
    }

    buf = malloc(RECV_BLOCK_SIZE);
    if (!buf) {
        printf("malloc recv buffer error\n");
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
        printf("upgrade file err : 0x%x\n", err);
        return err;
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
        printf("system not match to file : 0x%x\n", err);
        return err;
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
            printf("upgrade core run err : 0x%x\n", err);
        }

        offset += block_size;
        size -= block_size;
        block_size = size > RECV_BLOCK_SIZE ? RECV_BLOCK_SIZE : size;
        fread(fsdc, buf, block_size);
    } while (block_size);


    server_close(upgrade_ser);
    printf("upgrade ok !!!\n");
    cpu_reset();
__ota_exit:
    if (fsdc) {
        fclose(fsdc);
    }

    if (buf) {
        free(buf);
    }

    return 0;
}

int wifi_file_upgrade_buff(char *pbuf, int len)
{
    void *sys_ufd = NULL;
    struct server *upgrade_ser;
    union sys_upgrade_req req = {0};
    char *buf = NULL;
    int err = 0;
    int block_size = 0;
    int size = 0;
    int offset = 0;

    server_load(upgrade_server);
    upgrade_ser = server_open("upgrade_server", NULL);
    if (!upgrade_ser) {
        printf("open upgrade server error\n");
        return -EINVAL;
    }

    /*
     * 校验文件，必须从0开始校验
     * 如果传入的长度 < 文件长度，则只校验头的部分
     */
    buf = pbuf;
    block_size = UPGRADE_FILE_CHECK_PAGE_SIZE;

    req.info.type = UPGRADE_TYPE_BUF;
    req.info.input.data.buf = buf;
    req.info.input.data.size = block_size;
    req.info.offset = 0;
    err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
    if (err) {
        printf("upgrade file err : 0x%x\n", err);
        return err;
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
        printf("system not match to file : 0x%x\n", err);
        return err;
    }
    /*
     * 这里已经确定可以升级
     * 可以在这里进行升级确认，以及插电提醒
     */

    /*
     * 开始升级
     */
    size = len;
    block_size = UPGRADE_FILE_CHECK_PAGE_SIZE;
    do {
        req.core.type = UPGRADE_TYPE_BUF;
        req.core.input.data.buf = buf;
        req.core.input.data.size = block_size;
        req.core.offset = offset;
        err = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
        if (err) {
            printf("upgrade core run err : 0x%x\n", err);
        }

        offset += block_size;
        size -= block_size;
        buf += block_size;
        block_size = size > UPGRADE_FILE_CHECK_PAGE_SIZE ? UPGRADE_FILE_CHECK_PAGE_SIZE : size;
    } while (block_size);


    server_close(upgrade_ser);
    printf("upgrade ok !!!\n");
    cpu_reset();
__ota_exit:
    return 0;
}



