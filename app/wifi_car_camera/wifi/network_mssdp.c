#include "mssdp/mssdp.h"
#include "fs/fs.h"
#include "app_config.h"
#include "dev_desc.h"

#include "system/event.h"
#if defined CONFIG_NET_SERVER || defined CONFIG_NET_CLIENT
static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    if (type == MSSDP_SEARCH_MSG) {
        printf("ssdp client[0x%x] search, %s\n", dest_ipaddr, buf);
    } else if (type == MSSDP_NOTIFY_MSG) {
        printf("sssss\n");
//TODO
#if 1
        struct sys_event e;
        e.arg = "dhcp_srv";
        e.type = SYS_DEVICE_EVENT;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        sys_event_notify(&e);
#endif


        mssdp_uninit();

    }
}
#else

#define JSON_DOC "aaa"

static char json_buffer[1024];

const char *__attribute__((weak))get_dev_desc_doc()
{
    int ret;

#if 0
    FILE *fd = fopen("mnt/spiflash/res/dev_desc.txt", "r");

    if (fd == NULL) {
        printf("%s~~~~~~~~%d   open fail\n", __func__, __LINE__);
        return NULL;
    }

    memset(json_buffer, 0, 1024);
    ret = fread(fd, json_buffer, 1024);
    if (ret <= 0) {
        printf("%s~~~~~~~~%d   read fail\n", __func__, __LINE__);
        return NULL;
    }

    fclose(fd);
#else
    memset(json_buffer, 0, 1024);
    memcpy(json_buffer, DEV_DESC_CONTENT, strlen(DEV_DESC_CONTENT));
#endif

    return json_buffer;
}

char get_MassProduction(void)
{
    return 0;
}

const char *create_mssdp_notify_msg(char *msg, u16_t dev_online_flag)
{
    char ip_addr[64];
    extern u8 get_rx_signal(void);
    extern void Get_IPAddress(char is_wireless, char *ipaddr);
    extern int storage_device_ready();
    Get_IPAddress(1, ip_addr);

    sprintf(msg, "{\"RTSP\":\"rtsp://%s/avi_rt/front.sd\",\
\"SIGNAL\":\"%d\",\
\"SD_STATUS\":\"%d\"\
\"DEV_ONLINE_FLAG\":\"%d\"\
}", \
            ip_addr, \
            get_rx_signal(), \
            storage_device_ready(), \
            dev_online_flag);
    return msg;

}

static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    char msg[128];
    switch (type) {
    case MSSDP_SEARCH_MSG:
        printf("ssdp client[0x%x] search, %s\n", dest_ipaddr, buf);
        break;
    case MSSDP_BEFORE_SEND_NOTIFY_MSG:
        if (get_MassProduction()) {
            mssdp_set_notify_msg((const char *)create_mssdp_notify_msg(msg, 0), 2);
        }
        break;
    default :
        break;
    }

}
#endif

void network_mssdp_init(void)
{
    char msg[128];
    puts("mssdp run \n");
    if (get_MassProduction()) {
        mssdp_init("MSSDP_SEARCH ", "MSSDP_NOTIFY ", 3333, network_ssdp_cb, NULL);
        mssdp_set_notify_msg((const char *)create_mssdp_notify_msg(msg, 0), 2);
    } else {
        mssdp_init("MSSDP_SEARCH ", "MSSDP_NOTIFY ", 3889, network_ssdp_cb, NULL);

#if defined  CONFIG_NET_SERVER
        mssdp_set_search_msg("xyz", 30); //主动3秒
#endif
#if defined  CONFIG_NET_CLIENT
        mssdp_set_notify_msg("xy", 5); //被动30秒
#endif
#if (!defined CONFIG_NET_CLIENT) && (!defined CONFIG_NET_SERVER)
        mssdp_set_notify_msg((const char *)get_dev_desc_doc(), 60);
#endif
    }
}

void network_mssdp_uninit(void)
{
    mssdp_uninit();
}



