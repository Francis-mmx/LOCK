/*************************************************************************
	> File Name: app/wifi_story_machine/upgrade.c
	> Author:
	> Mail:
	> Created Time: Fri 26 May 2017 03:45:26 PM HKT
 ************************************************************************/
#define _STDIO_H_
#include "xciot_api.h"
#include "avsdk/xciot_cmd_ipc.h"

#include "system/includes.h"
#include "app_config.h"
#include "storage_device.h"
#include "server/upgrade_server.h"
#include "os/os_compat.h"
#include "http/http_cli.h"
#include "doorbell_event.h"
#include "cloud_upgrade.h"


#define Hi3861L_PER_RECV_SIZE (1024)
#define PER_RECV_SIZE   (4096)

#define JL_OTA_FIEE_NAME "JL_AC5X.bfu"
#define HI3861L_OTA_FILE_NAME "Hi3861L_demo_ota.bin"
#define DOORBELL_OTA_FILE_NAME "doorbell_ota.bin"

struct ota_file_head {
    char name1[64];
    int len1;
    int offset1;
    char name2[64];
    int len2;
    int offset2;
};


extern void avsdk_firmware_rate_callback(void *arg, int index);
extern void cloud_upgrade_sem_pend();

static OS_SEM cloud_upgrade_sem;
static u8 sdcard_upgrade_status;
static struct cloud_upgrade_info __info;

u8 get_sdcard_upgrade_status()
{
    return sdcard_upgrade_status;
}

static int __httpcli_cb(void *ctx, void *buf, unsigned int size, void *priv, httpin_status status)
{
    return 0;
}

int get_update_data(const char *url, int notify, void *arg)
{
    int error = 0; //升级是否出错
    int ret = 0;
    int offset = 0;//数据偏移
    int rlen = 0;  //每次读取数据长度
    u8 *buffer = NULL;   //升级的写缓存
    int data_offset = 0;
    int total_len = 0;  //当前升级文件总长度
    union sys_upgrade_req req = {0};
    static s64 __offset = 0;
    struct ota_file_head head;
    u32 upgrade_data_len = 0; //已经升级的数据长度
    void *upgrade_ser = NULL;
    struct cloud_upgrade_info *info = &__info;

    const struct net_download_ops *ops = &http_ops;
    httpcli_ctx *ctx = (httpcli_ctx *)calloc(1, sizeof(httpcli_ctx));
    if (NULL == ctx) {
        printf("\n get_update_data httpcli_ctx malloc fail\n");
        return -1;
    }
    ctx->url = url;
    ctx->connection = "close";
    ctx->timeout_millsec = 10000;
    ctx->cb = __httpcli_cb;
    error = ops->init(ctx);
    if (error != HERROR_OK) {
        printf("\n %s %d\n", __func__, __LINE__);
        goto __exit;
    }
    error = -1;

    if (ctx->content_length <= 0) {
        printf("\n %s %d\n", __func__, __LINE__);
        goto __exit;
    }

    buffer = (u8 *)malloc(PER_RECV_SIZE);
    if (!buffer) {
        printf("\n %s %d\n", __func__, __LINE__);
        goto __exit;
    }

    if (notify) {
        avsdk_firmware_rate_callback(arg, 0);
    }
    if (strstr(url, HI3861L_OTA_FILE_NAME)) {

        total_len = ctx->content_length;
        while (total_len > 0) {
            if (total_len >= Hi3861L_PER_RECV_SIZE) {
                rlen = Hi3861L_PER_RECV_SIZE;
                total_len -= Hi3861L_PER_RECV_SIZE;
            } else {
                rlen = total_len;
                total_len = 0;
            }

            do {
                ret = ops->read(ctx, (char *)buffer + offset, rlen - offset);
                if (ret < 0) {
                    printf("\n %s %d\n", __func__, __LINE__);
                    goto __exit;
                }
                offset += ret;
            } while (rlen != offset);

            info->total_size = ctx->content_length;
            info->offset = __offset;
            info->type = 0;
            strcpy(info->name, "Hi3861L_demo_ota.bin");
            strcpy(info->md5, "68d540641fc861e7ec8c058fff8d0a45");
            info->len = offset;
            memcpy(info->buffer, buffer, info->len);

            printf("\n info->total_size  = %d\n", info->total_size);
            printf("\n info->offset = %d\n", info->offset);
            printf("\n info->name = %s\n", info->name);
            printf("\n info->len = %d\n", info->len);
            printf("\n percentage = %d \n", ((__offset * 100) / ctx->content_length));
            os_taskq_post("hi_channel_task", 2, 4, info);
            cloud_upgrade_sem_pend();
            __offset += offset;
            if (notify) {
                avsdk_firmware_rate_callback(arg, ((__offset * 100) / info->total_size));
            }

            offset = 0;
        }

    } else if (strstr(url, JL_OTA_FIEE_NAME)) {
        //供电一定要稳
        error = -1;
        total_len = ctx->content_length;
        upgrade_ser = server_open("upgrade_server", NULL);
        if (!upgrade_ser) {
            goto __exit;
        }
        server_register_event_handler(upgrade_ser, NULL, NULL);
        while (total_len > 0) {
            if (total_len >= PER_RECV_SIZE) {
                rlen = PER_RECV_SIZE;
                total_len -= PER_RECV_SIZE;
            } else {
                rlen = total_len;
                total_len = 0;
            }

            do {
                ret = ops->read(ctx, (char *)buffer + offset, rlen - offset);
                if (ret < 0) {
                    goto __exit;
                }
                offset += ret;
            } while (rlen != offset);

            if (data_offset == 0) {
                req.info.type = UPGRADE_TYPE_BUF;
                req.info.input.data.buf = buffer;
                req.info.input.data.size = offset;
                req.info.offset = data_offset;
                error = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
                if (error) {
                    log_e("upgrade check file error : 0x%x\n", error);
                    goto __exit;
                }
                error = server_request(upgrade_ser, UPGRADE_REQ_CHECK_SYSTEM, &req);
                if (error) {
                    if (error != SYS_UPGRADE_ERR_SAME) {
                        log_e("upgrade check system error : 0x%x\n", error);
                        goto __exit;
                    }
                }
            }
            if (error == 0) {
                req.core.type = UPGRADE_TYPE_BUF;
                req.core.input.data.buf = buffer;
                req.core.input.data.size = offset;
                req.core.offset = data_offset;
                error = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
                if (error) {
                    log_e("upgrade core error : 0x%x\n", error);
                    goto __exit;
                }
            }
            data_offset += offset;
            if (notify) {
                avsdk_firmware_rate_callback(arg, (data_offset * 100) /  ctx->content_length);
            }
            offset = 0;
        }
        if (upgrade_ser) {
            server_close(upgrade_ser);
        }
    } else if (strstr(url, DOORBELL_OTA_FILE_NAME)) {

        u8 is_AC5713_last_packet_flag = 0;
        int is_AC5713_last_packet_len = 0;
        u8 is_HI3861L_last_packet_flag = 0;
        u8 AC5713_last_packet_data[PER_RECV_SIZE];

        error = -1;
        offset = 0;
        rlen = sizeof(head);
        do {
            ret = ops->read(ctx, (char *)&head + offset, rlen - offset);
            if (ret < 0) {
                goto __exit;
            }
            offset += ret;
        } while (rlen != offset);

        upgrade_data_len += sizeof(head);
        printf("\n name1 = %s\n", head.name1);
        printf("\n len1 = %d\n", head.len1);
        printf("\n offset1 = %d\n", head.offset1);
        printf("\n name2 = %s\n", head.name2);
        printf("\n len1 = %d\n", head.len2);
        printf("\n offset1 = %d\n", head.offset2);

        void *upgrade_ser = server_open("upgrade_server", NULL);
        if (!upgrade_ser) {
            printf("\n %s %d\n", __func__, __LINE__);
            goto __exit;
        }
        server_register_event_handler(upgrade_ser, NULL, NULL);
        offset = 0;
        total_len = head.len1;
        while (total_len > 0) {
            if (total_len >= PER_RECV_SIZE) {
                rlen = PER_RECV_SIZE;
                total_len -= PER_RECV_SIZE;
                if (total_len == 0) {
                    is_AC5713_last_packet_flag = 1;
                }
            } else {
                rlen = total_len;
                total_len = 0;
                is_AC5713_last_packet_flag = 1;
            }

            do {
                ret = ops->read(ctx, (char *)buffer + offset, rlen - offset);
                if (ret < 0) {
                    goto __exit;
                }
                offset += ret;
            } while (rlen != offset);

            if (is_AC5713_last_packet_flag) {
                is_AC5713_last_packet_len = rlen;
                printf("\n is_AC5713_last_packet_len = %d \n", is_AC5713_last_packet_len);
                break;
            }

            if (data_offset == 0) {
                req.info.type = UPGRADE_TYPE_BUF;
                req.info.input.data.buf = buffer;
                req.info.input.data.size = offset;
                req.info.offset = data_offset;
                error = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
                if (error) {
                    log_e("upgrade check file error : 0x%x\n", error);
                    goto __exit;
                }

                error = server_request(upgrade_ser, UPGRADE_REQ_CHECK_SYSTEM, &req);
                if (error) {
                    if (error != SYS_UPGRADE_ERR_SAME) {
                        log_e("upgrade check system error : 0x%x\n", error);
                        goto __exit;
                    }
                }
            }

            if (error == 0) {
                req.core.type = UPGRADE_TYPE_BUF;
                req.core.input.data.buf = buffer;
                req.core.input.data.size = offset;
                req.core.offset = data_offset;
                error = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
                if (error) {
                    log_e("upgrade core error : 0x%x\n", error);
                    goto __exit;
                }
            }
            data_offset += offset;
            upgrade_data_len += offset;
            if (notify) {
                avsdk_firmware_rate_callback(arg, (upgrade_data_len * 100) /  ctx->content_length);
            }
            offset = 0;
        }

        if (is_AC5713_last_packet_flag) {

            memcpy(AC5713_last_packet_data, buffer, is_AC5713_last_packet_len);
        }

//        if (upgrade_ser) {
//            server_close(upgrade_ser);
//        }
        error = -1;
        offset = 0;
        total_len = head.len2;
        while (total_len > 0) {
            if (total_len >= Hi3861L_PER_RECV_SIZE) {
                rlen = Hi3861L_PER_RECV_SIZE;
                total_len -= Hi3861L_PER_RECV_SIZE;
                if (total_len == 0) {
                    is_HI3861L_last_packet_flag = 1;
                }
            } else {
                rlen = total_len;
                total_len = 0;
                is_HI3861L_last_packet_flag = 1;
            }

            do {
                ret = ops->read(ctx, (char *)buffer + offset, rlen - offset);
                if (ret < 0) {
                    printf("\n %s %d\n", __func__, __LINE__);
                    goto __exit;
                }
                offset += ret;
            } while (rlen != offset);

            if (is_HI3861L_last_packet_flag &&  is_AC5713_last_packet_flag) {

                req.core.type = UPGRADE_TYPE_BUF;
                req.core.input.data.buf = AC5713_last_packet_data;
                req.core.input.data.size = is_AC5713_last_packet_len;
                req.core.offset = data_offset;
                error = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
                if (error) {
                    log_e("upgrade core error : 0x%x\n", error);
                    goto __exit;
                }
                if (upgrade_ser) {
                    server_close(upgrade_ser);
                }
                upgrade_data_len += is_AC5713_last_packet_len;
            }

            info->total_size =  head.len2;
            info->offset = __offset;
            info->type = 0;
            strcpy(info->name, "Hi3861L_demo_ota.bin");
            strcpy(info->md5, "68d540641fc861e7ec8c058fff8d0a45");
            info->len = offset;
            memcpy(info->buffer, buffer, info->len);

            printf("\n info->offset = %d\n", info->offset);
            printf("\n info->total_size  = %d\n", info->total_size);
            printf("\n info->name = %s\n", info->name);
            printf("\n info->len = %d\n", info->len);
            os_taskq_post("hi_channel_task", 2, 4, info);
            cloud_upgrade_sem_pend();
            __offset += offset;
            upgrade_data_len += offset;
            if (notify) {
                avsdk_firmware_rate_callback(arg, ((upgrade_data_len * 100) / ctx->content_length));
            }
//            data_offset += offset;
            offset = 0;
        }
    }
    puts("\n UPGRADE_SUCCESS \n");
    error = 0;
__exit:
    if (buffer) {
        free(buffer);
    }
    ops->close(ctx);
    free(ctx);

    return error;
}

void cloud_upgrade_firmware(void *rsp, int notify, void *arg)
{
    sdcard_upgrade_status = 1;
    ipc_FirmwareCheckByDevice_Resp *__rsp = (ipc_FirmwareCheckByDevice_Resp *)rsp;
    int error = get_update_data(__rsp->firmware_url, notify, arg);
    if (error == 0) {
        char firmware_version[64] = {0};
        memcpy(firmware_version, __rsp->firmware_ver, strlen(__rsp->firmware_ver));
        db_update_buffer(FIRMWARE_VM_INDEX, firmware_version, sizeof(firmware_version));
        //发消息让3861L重启
        if (strstr(__rsp->firmware_url, JL_OTA_FIEE_NAME)) {
            os_taskq_post("hi_channel_task", 2, HI_CHANNEL_CMD_RESET_SET, 1);
        } else {
            os_taskq_post("hi_channel_task", 2, HI_CHANNEL_CMD_RESET_SET, 3);
        }

    }
    msleep(1000);
    sdcard_upgrade_status = 0;
}

void cloud_upgrade_sem_pend()
{
    int ret;
    ret = os_sem_pend(&cloud_upgrade_sem, 2000);
    if (ret != OS_NO_ERR) {
        extern void hi3861l_reset(void);
        hi3861l_reset();//如果3861L超时不回复消息,复位3861L
    }
}

void cloud_upgrade_sem_post()
{
    os_sem_post(&cloud_upgrade_sem);
}

void sdcard_upgrade_thread(void *priv)
{
    void *fd = NULL;
    char *buf;
    struct server *upgrade_ser = NULL;
    union sys_upgrade_req req = {0};
    int err = 0;
    int block_size = 0;
    int size = 0;
    int offset = 0;
    struct cloud_upgrade_info *info = &__info;
    static s64 __offset = 0;
    int rlen;
    int total_len;
    int upgrade_mode = -1;
    char name[128];


    buf = (u8 *)malloc(PER_RECV_SIZE);
    if (!buf) {
        log_e("malloc recv buffer error\n");
        goto __exit;
    }
    snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, JL_OTA_FIEE_NAME);
    fd = fopen(name, "r");
    if (fd) {
        sdcard_upgrade_status = 1;
        upgrade_mode = 0;
        server_load(upgrade_server);
        upgrade_ser = server_open("upgrade_server", NULL);
        if (!upgrade_ser) {
            goto __exit;
        }
        server_register_event_handler(upgrade_ser, NULL, NULL);
        block_size = PER_RECV_SIZE;
        fread(fd, buf, block_size);
        req.info.type = UPGRADE_TYPE_BUF;
        req.info.input.data.buf = buf;
        req.info.input.data.size = block_size;
        req.info.offset = 0;
        err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
        if (err) {
            log_e("upgrade file err : 0x%x\n", err);
            goto __exit;
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
            goto __exit;
        }
        fseek(fd, block_size, SEEK_SET);
        /*
         * 这里已经确定可以升级
         * 可以在这里进行升级确认，以及插电提醒
         */

        /*
         * 开始升级
         */
        size = flen(fd);
        do {
            req.core.type = UPGRADE_TYPE_BUF;
            req.core.input.data.buf = buf;
            req.core.input.data.size = block_size;
            req.core.offset = offset;
            err = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
            if (err) {
                log_e("upgrade core run err : 0x%x\n", err);
                goto __exit;
            }

            offset += block_size;
            size -= block_size;
            block_size = size > PER_RECV_SIZE ? PER_RECV_SIZE : size;
            if (!block_size) {
                break;
            }
            err = fread(fd, buf, block_size);
            if (err <= 0) {
                goto __exit;
            }
        } while (block_size);
        fdelete(fd);

    }
    snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, HI3861L_OTA_FILE_NAME);
    fd = fopen(name, "r");
    if (fd) {
        upgrade_mode = 1;
        sdcard_upgrade_status = 1;
        /*
         * 开始升级
         */
        int rlen;
        int total_len;
        total_len = size = flen(fd);
        while (total_len) {
            if (total_len >= Hi3861L_PER_RECV_SIZE) {
                rlen = Hi3861L_PER_RECV_SIZE;
                total_len -= Hi3861L_PER_RECV_SIZE;
            } else {
                rlen = total_len;
                total_len = 0;
            }
            err = fread(fd, buf, rlen);
            if (err <= 0) {
                goto __exit;
            }



            info->total_size = size;
            info->offset = __offset;

            info->type = 0;
            strcpy(info->name, "Hi3861L_demo_ota.bin");
            strcpy(info->md5, "68d540641fc861e7ec8c058fff8d0a45");
            info->len = rlen;
            memcpy(info->buffer, buf, info->len);

            printf("\n info->total_size  = %d\n", info->total_size);
            printf("\n info->offset = %d\n", info->offset);
            printf("\n info->name = %s\n", info->name);
            printf("\n info->len = %d\n", info->len);
            os_taskq_post("hi_channel_task", 2, 4, info);
            cloud_upgrade_sem_pend();
            __offset += rlen;
        }
        fdelete(fd);
    }
    snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, DOORBELL_OTA_FILE_NAME);
    fd = fopen(name, "r");
    if (fd) {
        upgrade_mode = 2;
        sdcard_upgrade_status = 1;
        struct ota_file_head head;
        fread(fd, &head, sizeof(head));

        printf("\n name1 = %s\n", head.name1);
        printf("\n len1 = %d\n", head.len1);
        printf("\n offset1 = %d\n", head.offset1);
        printf("\n name1 = %s\n", head.name2);
        printf("\n len1 = %d\n", head.len2);
        printf("\n offset1 = %d\n", head.offset2);

        server_load(upgrade_server);
        upgrade_ser = server_open("upgrade_server", NULL);
        if (!upgrade_ser) {
            goto __exit;
        }
        server_register_event_handler(upgrade_ser, NULL, NULL);
        block_size = PER_RECV_SIZE;
        fseek(fd, head.offset1, SEEK_SET);
        fread(fd, buf, block_size);
        req.info.type = UPGRADE_TYPE_BUF;
        req.info.input.data.buf = buf;
        req.info.input.data.size = block_size;
        req.info.offset = 0;
        err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
        if (err) {
            log_e("upgrade file err : 0x%x\n", err);
            goto __exit;
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
            goto __exit;
        }
        fseek(fd, head.offset1 + block_size, SEEK_SET);
        /*
         * 这里已经确定可以升级
         * 可以在这里进行升级确认，以及插电提醒
         */

        /*
         * 开始升级
         */
        size = head.len1;
        do {
            req.core.type = UPGRADE_TYPE_BUF;
            req.core.input.data.buf = buf;
            req.core.input.data.size = block_size;
            req.core.offset = offset;
            err = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
            if (err) {
                log_e("upgrade core run err : 0x%x\n", err);
                goto __exit;
            }

            offset += block_size;
            size -= block_size;
            block_size = size > PER_RECV_SIZE ? PER_RECV_SIZE : size;
            if (!block_size) {
                break;
            }
            err = fread(fd, buf, block_size);
            if (err <= 0) {
                goto __exit;
            }
        } while (block_size);
        fseek(fd, head.offset2, SEEK_SET);
        /*升级3861L*/

        total_len = size = head.len2;
        while (total_len) {
            if (total_len >= Hi3861L_PER_RECV_SIZE) {
                rlen = Hi3861L_PER_RECV_SIZE;
                total_len -= Hi3861L_PER_RECV_SIZE;
            } else {
                rlen = total_len;
                total_len = 0;
            }
            err = fread(fd, buf, rlen);
            if (err <= 0) {
                goto __exit;
            }

            info->total_size = size;
            info->offset = __offset;
            info->type = 0;
            strcpy(info->name, "Hi3861L_demo_ota.bin");
            strcpy(info->md5, "68d540641fc861e7ec8c058fff8d0a45");
            info->len = rlen;
            memcpy(info->buffer, buf, info->len);
            printf("\n info->total_size  = %d\n", info->total_size);
            printf("\n info->offset = %d\n", info->offset);
            printf("\n info->name = %s\n", info->name);
            printf("\n info->len = %d\n", info->len);
            os_taskq_post("hi_channel_task", 2, 4, info);

            cloud_upgrade_sem_pend();
            __offset += rlen;
        }
        fdelete(fd);
    }
    if (upgrade_mode != -1) {
        printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>HI_CHANNEL_CMD_RESET_SET\n");
        //发消息让3861L重启
        os_taskq_post("hi_channel_task", 2, HI_CHANNEL_CMD_RESET_SET, upgrade_mode == 0 ? 1 : 3);
    }
__exit:

    if (upgrade_ser) {
        server_close(upgrade_ser);
    }

    if (buf) {
        free(buf);
    }
}

int sdcard_upgrade_init(void)
{
    void *fd = NULL;
    char name[128];
    snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, JL_OTA_FIEE_NAME);
    fd = fopen(name, "r");
    if (fd) {
        goto __exit;
    }
    snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, HI3861L_OTA_FILE_NAME);
    fd = fopen(name, "r");
    if (fd) {
        goto __exit;
    }
    snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, DOORBELL_OTA_FILE_NAME);
    fd = fopen(name, "r");
    if (fd) {
        goto __exit;
    }
    return 0;
__exit:
    fclose(fd);
    thread_fork("sdcard_upgrade_thread", 18, 0x1000, 0, 0, sdcard_upgrade_thread, NULL);
    return -1;
}

int cloud_upgrade_init()
{
    os_sem_create(&cloud_upgrade_sem, 0);
    return 0;
}



//late_initcall(cloud_upgrade_init);

