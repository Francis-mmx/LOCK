#include "ui/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "system/includes.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "device/av10_spi.h"
#include "asm/ldo.h"
#include "app_database.h"
#include "system/device/uart.h"


static void *uart_dev_handle;
int spec_uart_recv(char *buf, u32 len)      //接收
{
 return dev_read(uart_dev_handle, buf, len);
}
int spec_uart_send(char *buf, u32 len)      //发送
{
 return dev_write(uart_dev_handle, buf, len);
}


//void uart_send_test_func()
//{
//    printf("============= uart_send_test_func\n");
//    spec_uart_send(uart_verson_2,15);
//}

static void uart_rec_function_task(void *arg)
{
    int len;
    static  u8 yun_cnt= 0;
    static u8 buf[512];
    puts("\nuart_rec_function_task>>>>>>>>\n");
    printf("XXXXXX\n");
    while(1){
        len = spec_uart_recv(buf, 200);
        if(len > 0){
            put_buf(buf, len);
        }
        os_time_dly(1);
    }
}

void spec_uart_init(void)
{
    static u8 buf[512];
    int len;
    static char uart_circlebuf[1 * 1024] __attribute__((aligned(32))); //串口循环数据 buf,根据需要设置大小
    int parm;
    uart_dev_handle = dev_open("uart0", 0);
    /* 1 .设置接收数据地址 */
    dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_ADDR, (int)uart_circlebuf);
    parm = sizeof(uart_circlebuf);
    /* 2 .设置接收数据地址长度 */
    dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_LENTH, (int)parm);
    /* 4 .设置接收数据为阻赛方式,需要非阻赛可以去掉，建议加上超时设置 */
    #if 1
    parm = 1;
    dev_ioctl(uart_dev_handle, UART_SET_RECV_BLOCK, (int)parm);
    #endif
    /* 5 . 使能特殊串口 */
    dev_ioctl(uart_dev_handle, UART_START, (int)0);
    /*thread_fork("uart_rec_function", 8, 0x1000, 0, 0, uart_rec_function, NULL);*/
    os_task_create(uart_rec_function_task, 0, 26, 1024, 32, "uart_rec_function");
}
