#ifndef APP_ACTION_H
#define APP_ACTION_H
#include "app_config.h"

#define BUFFER_SIZE 128  // 缓冲区大小
extern  u8 recv_buffer[BUFFER_SIZE];  // 环形缓冲区
extern  u8 write_index;  // 写指针
extern  u8 read_index;   // 读指针
extern  u8 data_count;     //缓冲区内数量



#define SECTOR_SIZE       0x1000          //4K
#define BASE_ADDRESS      0x7EF000        //用户数据基地址
#define MAX_USER_NUM            50        //最大用户数量
#define MAX_PASSWORD_LEN        25        //最大密码长度
#define MAX_NAME_LEN            20        //名字最长字符数
#define MAX_KEY_NUM             5         //最大密钥数量

#define RECORD_BASE_ADDR  0x7BC000        //记录基地址
#define MAX_RECORD_NUM          500       //最大记录数
#define RECORD_SIZE sizeof(user_visit)    //一条记录的大小          32
#define MAX_RECORD_ADDR   (RECORD_BASE_ADDR - RECORD_SIZE * MAX_RECORD_NUM)       //第500条记录地址

#define RECORDS_PER_SECTOR (SECTOR_SIZE / RECORD_SIZE) // 每个扇区记录数  128

#define BUFFER_ADDR       0x7B8000        //缓冲地址

#define ACTION_HOME_MAIN 				0x00001001
#define ACTION_DEVICE_OUT 				0x00001002

typedef enum{
    UNLOCK = 0,
    ADMIN,
};

typedef enum{
    FACE = 0,
    PASSWORD,
    FINGER,
    NFC,
};


typedef struct {
    u16 year;
    u8 month;
    u8 day;
    u16 idx;
}time_to_index;              //时间索引结构体，根据年月日寻找对应的记录flash地址

typedef struct{
    u8 unlock_mode ;           // 表示开锁方式及权限 face、password、finger、NFC
    u8 unlock_power ;          // 表示  管理权限、开锁
}unlock_status;


typedef struct {
    u16 check;
    struct sys_time record_time;    // 记录的时间、模式
    unlock_status unlock;
    u8 name[MAX_NAME_LEN];          // 记录的用户名
} user_visit;

typedef struct {
    u8 key_buf[MAX_PASSWORD_LEN];  // 存储密钥
    u8 key_mode;                   // 存储方式
} unlock_key;

typedef struct {
    u16 user_num;                  // 用户编号 
    u16 visit_count;               // 访问次数
    u8 name[MAX_NAME_LEN];         // 用户名字
    u8 user_power;                 // 管理员权限或开锁权限
    u8 key_num;                    // 密钥数量
    u8 index;                      // 在flash中的索引
    unlock_key key[MAX_KEY_NUM];   // 密钥结构体
} user_infor;

typedef struct {
    user_infor records;            // 开锁用户
} match_record;

    
extern  user_infor *user_data;
extern user_visit now_user;



#define ACTION_HOME_MAIN 				0x00001001
#define ACTION_DEVICE_OUT 				0x00001002



#define ACTION_VIDEO_REC_MAIN 			0x00002001
#define ACTION_VIDEO_REC_SET_CONFIG 	0x00002002
#define ACTION_VIDEO_REC_GET_CONFIG 	0x00002003
#define ACTION_VIDEO_REC_CHANGE_STATUS 	0x00002004
#define ACTION_VIDEO_REC_CONTROL        0x00002005
#define ACTION_VIDEO_REC_SWITCH_WIN     0x00002006
#define ACTION_VIDEO_REC_LOCK_FILE      0x00002007
#define ACTION_VIDEO_REC_SWITCH_WIN_OFF 0x00002008
#define ACTION_VIDEO_REC_UART_RETRANSMIT 0x00002009  //新增串口重发
#define ACTION_VIDEO_REC_SEARCH_USER    0x0000200A   //新增用户查找

#define ACTION_VIDEO_DEC_MAIN 			0x00004001
#define ACTION_VIDEO_DEC_SET_CONFIG 	0x00004002
#define ACTION_VIDEO_DEC_GET_CONFIG 	0x00004003
#define ACTION_VIDEO_DEC_CHANGE_STATUS 	0x00004004
#define ACTION_VIDEO_DEC_OPEN_FILE    	0x00004005
#define ACTION_VIDEO_DEC_CONTROL        0x00004006
#define ACTION_VIDEO_DEC_CUR_PAGE       0x00004007
#define ACTION_VIDEO_DEC_SWITCH         0x00004008

#define ACTION_PHOTO_TAKE_MAIN 			0x00008001
#define ACTION_PHOTO_TAKE_SET_CONFIG 	0x00008002
#define ACTION_PHOTO_TAKE_GET_CONFIG 	0x00008003
#define ACTION_PHOTO_TAKE_CHANGE_STATUS	0x00008004
#define ACTION_PHOTO_TAKE_GET_CAMERAID 	0x00008005
#define ACTION_PHOTO_TAKE_CONTROL       0x00008006
#define ACTION_PHOTO_TAKE_SWITCH_WIN    0x00008007

#define ACTION_MUSIC_PLAY_MAIN 			0x00009001
#define ACTION_MUSIC_PLAY_SET_CONFIG 	0x00009002
#define ACTION_MUSIC_PLAY_GET_CONFIG 	0x00009003
#define ACTION_MUSIC_PLAY_CHANGE_STATUS 0x00009004


#define ACTION_AUDIO_REC_MAIN 			0x00010001
#define ACTION_AUDIO_REC_SET_CONFIG 	0x00010002
#define ACTION_AUDIO_REC_GET_CONFIG 	0x00010003
#define ACTION_AUDIO_REC_CHANGE_STATUS  0x00010004


#define ACTION_USB_SLAVE_MAIN 			0x00020001
#define ACTION_USB_SLAVE_SET_CONFIG 	0x00020002
#define ACTION_USB_SLAVE_GET_CONFIG 	0x00020003

#define ACTION_UPGRADE_MAIN             0x00030001
#define ACTION_UPGRADE_SET_CONFIRM      0x00030002

#define ACTION_SYSTEM_MAIN 	    		0x00FFF001
#define ACTION_SYSTEM_SET_CONFIG 		0x00FFF002
#define ACTION_SYSTEM_GET_CONFIG 		0x00FFF003


#define ACTION_AUDIO_MAIN 	     		0x00011001
#define ACTION_AUDIO_SET_CONFIG     	0x00011002
#define ACTION_AUDIO_GET_CONFIG     	0x00011003
#define ACTION_AUDIO_CHANGE_STATUS      0x00011004




#endif

