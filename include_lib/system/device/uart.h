#ifndef DEVICE_UART_H
#define DEVICE_UART_H

#include "typedef.h"
#include "device/device.h"
#include "generic/ioctl.h"
#include "system/task.h"

#define UART_DMA_SUPPORT 	0x00000001
#define UART_TX_USE_DMA 	0x00000003
#define UART_RX_USE_DMA 	0x00000005
#define UART_DEBUG 			0x00000008

struct uart_outport {
    u8  tx_pin;
    u8  rx_pin;
    u16 value;
};

extern void putbyte(char a);


enum uart_clk_src {
    LSB_CLK,
    OSC_CLK,
    PLL_48M,
};

/**************勿改***************/
#define PORT_BASE_VALUE	0x10000 //该值需要大于16 * 8

//	    PORT_TX_RX
#define PORTA_10_11		(PORT_BASE_VALUE + 0)
#define PORTA_12_13		(PORT_BASE_VALUE + 0)
#define PORTH_4_3		(PORT_BASE_VALUE + 0)

#define PORTG_6_7		(PORT_BASE_VALUE + 1)
#define PORTH_2_5		(PORT_BASE_VALUE + 1)
#define PORTF_0_1		(PORT_BASE_VALUE + 1)

#define PORTH_12_11		(PORT_BASE_VALUE + 2)
#define PORTH_9_10		(PORT_BASE_VALUE + 2)
#define PORTD_0_1		(PORT_BASE_VALUE + 2)
#define PORTH_12_13		(PORT_BASE_VALUE + 2)

#define PORTD_4_5		(PORT_BASE_VALUE + 3)
#define PORTF_2_3		(PORT_BASE_VALUE + 3)
#define PORTH_0_1		(PORT_BASE_VALUE + 3)

#define PORT_MIN_NUM	(PORT_BASE_VALUE + 0)
#define PORT_MAX_NUM	(PORT_BASE_VALUE + 4)
#define PORT_REMAP		(PORT_BASE_VALUE + 5)

#define OUTPUT_CHANNEL0	(PORT_BASE_VALUE + 0)
#define OUTPUT_CHANNEL1	(PORT_BASE_VALUE + 1)
#define OUTPUT_CHANNEL2	(PORT_BASE_VALUE + 2)
#define OUTPUT_CHANNEL3	(PORT_BASE_VALUE + 3)
#define INPUT_CHANNEL0	(PORT_BASE_VALUE + 4)
#define INPUT_CHANNEL1	(PORT_BASE_VALUE + 5)
#define INPUT_CHANNEL2	(PORT_BASE_VALUE + 6)
#define INPUT_CHANNEL3	(PORT_BASE_VALUE + 7)

#define PORT_GET_VALUE(port)	(port - PORT_BASE_VALUE)
#define CHANNEL_GET_VALUE(port)	(port - PORT_BASE_VALUE)

/*io_map_port:
  IO_PORTA_00 - IO_PORTA_13,
  IO_PORTD_00 - IO_PORTD_15,
  IO_PORTF_00 - IO_PORTF_07,
  IO_PORTG_00 - IO_PORTG_15,
  IO_PORTH_00 - IO_PORTH_13
  */
/*********************************/

/*
enum _uart_port0{
    //uart0
    PORTA_10_11 = 0,
    PORTG_6_7,
    PORTH_12_11,
    PORTD_4_5,
};
enum _uart_port1{
    //uart1
    PORTA_11_12 = 0,
    PORTH_2_5,
    PORTH_9_10,
    PORTF_2_3,
};
enum _uart_port2{
    //uart2
    PORTH_4_3 = 0,
    PORTF_0_1,
    PORTD_0_1,
    PORTH_0_1,
};

enum _uart_port3{
    //uart3
    PORTA_10_11 = 0,
    PORTG_6_7,
    PORTH_12_13,
    PORTD_4_5,
};
*/

struct uart_platform_data {
    u8 *name;

    u8  irq;
    int tx_pin;//不配置需设置-1
    int rx_pin;//不配置需设置-1
    int flags;
    u32 baudrate;

    int port;//enum _uart_port0-3的值
    int output_channel;
    int input_channel;
    u32 max_continue_recv_cnt;
    u32 idle_sys_clk_cnt;
    enum uart_clk_src clk_src;
};

enum {
    UART_CIRCULAR_BUFFER_WRITE_OVERLAY = -1,
    UART_RECV_TIMEOUT = -2,
    UART_RECV_EXIT = -3,
};

#define UART_MAGIC                          'U'
#define UART_FLUSH                          _IO(UART_MAGIC,1)
#define UART_SET_RECV_ALL                   _IOW(UART_MAGIC,2,bool)
#define UART_SET_RECV_BLOCK                 _IOW(UART_MAGIC,3,bool)
#define UART_SET_RECV_TIMEOUT               _IOW(UART_MAGIC,4,u32)
#define UART_SET_RECV_TIMEOUT_CB            _IOW(UART_MAGIC,5,int (*)(void))
#define UART_GET_RECV_CNT                   _IOR(UART_MAGIC,6,u32)
#define UART_START                          _IO(UART_MAGIC,7)
#define UART_SET_CIRCULAR_BUFF_ADDR         _IOW(UART_MAGIC,8,void *)
#define UART_SET_CIRCULAR_BUFF_LENTH        _IOW(UART_MAGIC,9,u32)


#define UART_PLATFORM_DATA_BEGIN(data) \
    static const struct uart_platform_data data = {


#define UART_PLATFORM_DATA_END() \
    };


struct uart_device {
    char *name;
    const struct uart_operations *ops;
    struct device dev;
    const struct uart_platform_data *priv;
    OS_MUTEX mutex;
};


/*************************************Changed by liumenghui*************************************/

/*确认信号：报头 + OK (ascii码)   0xAA,0xBB,0x6F,0x6B  */

#define MAX_TRANSMIT 2        //最大重发次数

/*数据包：报头、长度、模式、命令、校验*/
typedef struct {
    u16 header;               // 报头，2字节        0xCDCD
    u8 tag;                   // 标识符 1字节 0x01
    u8 length;                // 指令+数据的长度
    u8 command;               // 指令，1字节
    u8 *data;                 // 命令
}Data;


typedef struct {
    Data data;                // 数据
    u16 check;                // 校验
}Packet;

#define  PACKET_OTHER_LEN  7    //2字节报头、1字节标识符、1字节长度、2字节校验,1字节指令


//指令码
typedef enum {
    voice = 0xA0,             //声音            屏幕--->锁
    add_user,                 //添加用户          屏幕--->锁
    delete_user,              //删除用户          屏幕--->锁
    add_password,             //添加密钥请求        屏幕--->锁
    device_request,           //设备请求          屏幕--->锁
    store_key,                //存储密钥          锁--->屏幕
    unlock_user,              //开锁用户          锁--->屏幕
    device_response,          //设备回复          锁--->屏幕
    
    answer_signal = 0xAF,     //应答信号          锁<--->屏幕
};

//声音数据
typedef enum {
    key_sound = 0x01,         //按键音
    door_bell,                //门铃
    powered,                  //上电
    input_admin_infor,        //请输入管理员信息
    locked,                   //已关锁
    unlocked,                 //已开锁
    enter_admin_mode,         //进入管理员模式
    exit_admin_mode,          //退出管理员模式
    operate_success,          //操作成功
    operate_fail,             //操作失败
    set_network,              //请配网
    input_key,                //请输入指纹密码卡
    input_key_again,          //请再次输入密码
    set_face,                 //进入人脸识别模式
    turn_left,                //人脸识别 向左转
    turn_right = 0x10,               //人脸识别 向右转
    lower_head,               //人脸识别 低头
    raise_head,               //人脸识别 抬头
    input_user_name,          //请输入用户名
    two_paw_diff,             //两次输入密码不同，请重新输入
    press_finger,             //请按指纹
    put_card,                 //请放卡片
    duplicate_card,           //重复卡
};


typedef enum {
    face = 0,                 //人脸
    other_key,                //其它密钥
    
};


/*************************************Changed by liumenghui*************************************/

struct uart_operations {
    int (*init)(struct uart_device *);
    int (*read)(struct uart_device *, void *buf, u32 len);
    int (*write)(struct uart_device *, void *buf, u16 len);
    int (*ioctl)(struct uart_device *, u32 cmd, u32 arg);
    int (*close)(struct uart_device *);
};



#define REGISTER_UART_DEVICE(dev) \
    static const struct uart_device dev sec(.uart)

extern struct uart_device uart_device_begin[], uart_device_end[];

#define list_for_each_uart_device(p) \
    for (p=uart_device_begin; p<uart_device_end; p++)



extern const struct device_operations uart_dev_ops;



#endif
