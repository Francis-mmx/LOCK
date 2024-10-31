
#include "os/os_compat.h"
#include "device/device.h"
#include "device/wifi_dev.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "vm_api.h"
#include "asm/crc16.h"
#include "http/http_cli.h"
#include "fs/fs.h"
#include "app_config.h"
#include "system/database.h"

struct get_mac_oauth_ctx {
    void (*callback)(void);
    char auth_key[32];
    char code[32];
    char uuid[48];
    httpcli_ctx ctx;
};
static struct get_mac_oauth_ctx *get_mac_oauth_ctx;
static int get_macaddr_task_pid;
static char server_assign_macaddr_ok, get_macaddr_task_exit_flag;

#define PROFILE_HOST "profile.jieliapp.com"
#define GET_GET_MACADDR 			"https://"PROFILE_HOST"/license/v1/device/macaddress/auth?auth_key=%s&code=%s"
#define GET_GET_MACADDR_SUCC 			"https://"PROFILE_HOST"/license/v1/device/macaddress/check?uuid=%s"


static int __https_get_mothed(const char *url, int (*cb)(char *, void *), void *priv)
{
    int error = 0;
    http_body_obj http_body_buf;

    memset(&http_body_buf, 0x0, sizeof(http_body_obj));
    memset(&get_mac_oauth_ctx->ctx, 0x0, sizeof(httpcli_ctx));

    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 5 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *) malloc(http_body_buf.buf_len * sizeof(char));

    get_mac_oauth_ctx->ctx.url = url;
    get_mac_oauth_ctx->ctx.priv = &http_body_buf;
    get_mac_oauth_ctx->ctx.connection = "close";
    get_mac_oauth_ctx->ctx.timeout_millsec = 5000;

    error = httpscli_get(&get_mac_oauth_ctx->ctx);

    if (error == HERROR_OK) {
        if (cb) {
            error = cb(http_body_buf.p, priv);
        } else {
            error = 0;
        }
    } else {
        error = -1;
    }

    if (http_body_buf.p) {
        free(http_body_buf.p);
    }

    return error;
}

static int set_flash_wifi_mac(const char *mac)
{
    void *dev;
    db_update_buffer(VM_MAC_ADDR_INDEX, mac, 6);
    dev = dev_open("wifi", NULL);
    if (!dev) {
        return -1;
    }
    dev_ioctl(dev, DEV_SET_MAC, (u32)mac);
    dev_close(dev);
    printf("set_flash_mac addr valid,[%x:%x:%x:%x:%x:%x] \n", (u8)mac[0], (u8)mac[1], (u8)mac[2], (u8)mac[3], (u8)mac[4], (u8)mac[5]);
    return 0;



    u32 mac_flash_addr;
    char mac_data[8];
    u16 *mac_crc = (u16 *)&mac_data[0];

    struct vm_arg vm_arg = {
        .need_defrag_when_write_full = 1,
        .large_mode = 0,
        .vm_magic = 0,
    };

    dev = dev_open("vm", &vm_arg);

    if (!dev) {
        return -1;
    }

    dev_ioctl(dev, DEV_GET_VM_BASE_ADDR, (u32)&mac_flash_addr);
    mac_flash_addr -= 8 * 1024; // | 4K MAC | 4K PROFILE | 68K VM | FLASH END
    dev_close(dev);

    memcpy(&mac_data[2], mac, 6);
    *mac_crc = CRC16((const void *)&mac_data[2], sizeof(mac_data) - 2);

    dev = dev_open("spiflash", NULL);

    if (!dev) {
        return -1;
    }

    dev_ioctl(dev, IOCTL_ERASE_SECTOR, mac_flash_addr);
    dev_bulk_write(dev, mac_data, mac_flash_addr, sizeof(mac_data));
    dev_close(dev);

    dev = dev_open("wifi", NULL);
    if (!dev) {
        return -1;
    }
    dev_ioctl(dev, DEV_SET_MAC, (u32)&mac_data[2]);
    dev_close(dev);
    printf("set_flash_mac addr valid,[%x:%x:%x:%x:%x:%x] ,mac_crc=0x%x \n", (u8)mac[0], (u8)mac[1], (u8)mac[2], (u8)mac[3], (u8)mac[4], (u8)mac[5], *mac_crc);

    return 0;
}

int get_flash_mac_addr(char mac[6])
{
    char mac_data[8];
    if (db_select_buffer(VM_MAC_ADDR_INDEX, mac_data, 6) > 0) {
        if (mac) {
            memcpy(mac, &mac_data[0], 6);
        }

        server_assign_macaddr_ok = 1;

        printf("get_flash_mac_addr valid [%x:%x:%x:%x:%x:%x] \r\n", (unsigned char)mac[0], (unsigned char)mac[1], (unsigned char)mac[2], (unsigned char)mac[3], (unsigned char)mac[4], (unsigned char)mac[5]);
    }
    return -1;


    void *dev;
    u32 mac_flash_addr;
    u16 *mac_crc = (u16 *)&mac_data[0];

    struct vm_arg vm_arg = {
        .need_defrag_when_write_full = 1,
        .large_mode = 0,
        .vm_magic = 0,
    };

    dev = dev_open("vm", &vm_arg);

    if (!dev) {
        return -1;
    }

    dev_ioctl(dev, DEV_GET_VM_BASE_ADDR, (u32)&mac_flash_addr);
    mac_flash_addr -= 8 * 1024;
    dev_close(dev);
    printf("get_mac_addr_flash_addr = 0x%x\n", mac_flash_addr);

    dev = dev_open("spiflash", NULL);

    if (!dev) {
        return -1;
    }

    dev_ioctl(dev, IOCTL_SET_READ_MODE, 0);
    dev_bulk_read(dev, mac_data, mac_flash_addr, sizeof(mac_data));
    dev_ioctl(dev, IOCTL_CLR_READ_MODE, 0);
    dev_close(dev);

    if (CRC16((const void *)&mac_data[2], sizeof(mac_data) - 2) != *mac_crc) {
        printf("flash mac addr crc error=0x%x \n", *mac_crc);

        if (*mac_crc != 0xffff) {
            /* put_buf((const u8 *)mac_data, sizeof(mac_data)); */
        }

        return -1;
    }

    if (mac) {
        memcpy(mac, &mac_data[2], 6);
    }

    server_assign_macaddr_ok = 1;

    printf("get_flash_mac_addr valid [%x:%x:%x:%x:%x:%x] \r\n", (unsigned char)mac[0], (unsigned char)mac[1], (unsigned char)mac[2], (unsigned char)mac[3], (unsigned char)mac[4], (unsigned char)mac[5]);

    return 0;
}

static int oauth_get_macaddr(char *resp_buf, void *priv)
{
    json_object *new_obj;
    unsigned char *json_str, *mac, *uuid;

    json_str = strstr(resp_buf, "{\"");
    if (!json_str) {
        return -1;
    }

    new_obj = json_tokener_parse(json_str);
    if (!new_obj) {
        return -1;
    }

    mac = json_object_get_string(json_object_object_get(json_object_object_get(new_obj, "data"), "mac"));
    uuid = json_object_get_string(json_object_object_get(json_object_object_get(new_obj, "data"), "uuid"));
    if (!mac || mac[0] == '\0' || !uuid || uuid[0] == '\0') {
        printf("get server macaddr  error : %s \n", resp_buf);
        goto EXIT;
    }
    strcpy(get_mac_oauth_ctx->uuid, uuid);

    u8 mac_data[6];
    u32 tmp;

    //bug fix
    sscanf(&mac[10], "%x", &tmp);
    mac_data[5] = tmp;
    mac[10] = '\0';
    sscanf(&mac[8], "%x", &tmp);
    mac_data[4] = tmp;
    mac[8] = '\0';
    sscanf(&mac[6], "%x", &tmp);
    mac_data[3] = tmp;
    mac[6] = '\0';
    sscanf(&mac[4], "%x", &tmp);
    mac_data[2] = tmp;
    mac[4] = '\0';
    sscanf(&mac[2], "%x", &tmp);
    mac_data[1] = tmp;
    mac[2] = '\0';
    sscanf(&mac[0], "%x", &tmp);
    mac_data[0] = tmp;

    set_flash_wifi_mac(mac_data);

    json_object_put(new_obj);

    return 0;

EXIT:
    json_object_put(new_obj);
    return -1;
}

static void __get_macaddr_task(void *priv)
{
    int ret;
    char msg[1024];

    sprintf(msg, GET_GET_MACADDR, get_mac_oauth_ctx->auth_key, get_mac_oauth_ctx->code);

    while (!get_macaddr_task_exit_flag) {
        ret = __https_get_mothed(msg, oauth_get_macaddr, NULL);

        if (!ret) {
            sprintf(msg, GET_GET_MACADDR_SUCC, get_mac_oauth_ctx->uuid);
            __https_get_mothed(msg, NULL, NULL);
            get_mac_oauth_ctx->callback();
            break;
        }
    }
exit:
    free(get_mac_oauth_ctx);
    get_mac_oauth_ctx = NULL;
    os_time_dly(100);	//此处要加延时是因为避免杀线程比创建线程跑得还要快，导致任务句柄释放了还在使用
}


char is_server_assign_macaddr_ok(void)
{
    return server_assign_macaddr_ok;
}

static int dev_get_manufacture_code(void)
{
    FILE *fd;
    void *dev;
    json_object *new_obj;
    const char *auth_key, *code;
    char buf[512] = {0};
    u8 mac[6];

    if (get_mac_oauth_ctx->auth_key[0] != '\0' && get_mac_oauth_ctx->code[0] != '\0') {
        return 0;
    }

    fd = fopen("mnt/spiflash/res/manufacture.txt", "r");
    if (!fd) {
        return -1;
    }

    fread(fd, buf, sizeof(buf));
    fclose(fd);

    new_obj = json_tokener_parse((const char *)buf);
    if (!new_obj) {
        return -1;
    }

    auth_key =  json_object_get_string(json_object_object_get(new_obj, "auth_key"));
    code =  json_object_get_string(json_object_object_get(new_obj, "code"));

    if (auth_key && code) {
        strcpy(get_mac_oauth_ctx->auth_key, auth_key);
        strcpy(get_mac_oauth_ctx->code, code);
    }

    printf("dev_get_manufacture_code auth_key=%s , code=%s \n", get_mac_oauth_ctx->auth_key, get_mac_oauth_ctx->code);

    json_object_put(new_obj);

    return 0;
}

int server_assign_macaddr(void *(callback)(void))
{
    int ret;

    if (get_macaddr_task_pid) {
        return 0;
    }

    get_mac_oauth_ctx = calloc(1, sizeof(struct get_mac_oauth_ctx));
    if (!get_mac_oauth_ctx) {
        printf("server_assign_macaddr malloc fail\n");
        return -1;
    }
    get_mac_oauth_ctx->callback = callback;

    ret = dev_get_manufacture_code();
    if (ret) {
        printf("server_assign_macaddr fail\n");
        return -1;
    }

    ret = get_flash_mac_addr(NULL);
    if (ret == 0) {
        printf("macaddr in flash return\n");
        return 0;
    }

    thread_fork("get_macaddr_task", 22, 1024 * 4, 0, &get_macaddr_task_pid, __get_macaddr_task, NULL);

    return 0;
}

void cancel_server_assign_macaddr(void)
{
    if (get_mac_oauth_ctx) {
        http_cancel_dns(&get_mac_oauth_ctx->ctx);
        get_macaddr_task_exit_flag = 1;
        thread_kill(&get_macaddr_task_pid, KILL_WAIT);
        get_macaddr_task_exit_flag = 0;
    }
}


