#include "system/includes.h"
#include "hi3861l_task.h"
#include "lwip.h"

extern int Hi3861L_init(void) ;//主要是create wifi 线程的
extern int avsdk_cmd_init(void);
extern s32 hi3861l_hichannel_init();

#define		WIFI_SSID	"GJ1"
#define		WIFI_PWD    "8888888899"


void get_wifi_ssid_pwd(u8 *ssid, u8 *pwd)
{
    memcpy(ssid, WIFI_SSID, strlen(WIFI_SSID));
    memcpy(pwd, WIFI_PWD, strlen(WIFI_PWD));
}


static void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    printf("%s, name = %s\n, ip = %s", __func__, name, ipaddr_ntoa(ipaddr));
}


void Hi3861L_network_test(void)
{
    ip_addr_t host_ip;
    dns_gethostbyname("www.taobao.com", &host_ip, dns_found, NULL);

    //ping_init("120.24.247.138", 1000, 1000, NULL, NULL);
}


static void Hi3861L_demo_task(void)
{
    /*初始化与Hi3861L命令传输协议*/
    avsdk_cmd_init();

    /*建立与Hi3861L 通信的任务，所有命令的发送和接受统一由此函数处理*/
    Hi3861L_init();

    /*初始化hichannel通道，建立跟Hi3861L的通信*/
    if (hi3861l_hichannel_init()) {
        hi3861l_reset();
        return;
    }

    //os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_ALL_CFG);
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_NETWORK_CONFIG);
}


void Hi3861L_demo_test(void)
{
    thread_fork("Hi3861L_demo_task", 10, 1024, 0, NULL, Hi3861L_demo_task, NULL);
}



