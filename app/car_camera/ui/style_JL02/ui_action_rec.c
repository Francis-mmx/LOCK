
#include "ui/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "res.h"
#include "system/includes.h"
/* #include "menu_parm_api.h" */
#include "app_database.h"
#include "system/device/uart.h"
//#include <stddef.h>
#include "ui/ui_core.h"
#include "timer.h"

#define SLID_GAP  10  //每一项的间隔(滑块长度/项目数)
#define SLID_ITEM 40  //项目数
#define SECTOR_REMAIN  16

#define HARDWARE_VERSION  "xyz_abc_123_v1"
#define SOFTWARE_VERSION  "rst_abc_123_v2"
#define IDENTIFICATION_CODE  "xyz_opq_456_v1"

#define RECORD_INFOR_SIZE       sizeof(user_infor)
u32 write_user_offset = BASE_ADDRESS;            //8*1024*1024-68*1024
u32 read_flash_offset = RECORD_BASE_ADDR;

u32 write_record_offset = RECORD_BASE_ADDR;


user_visit user_global_visit[MAX_RECORD_NUM];   //全局的用户访问记录，按照从大到小的顺序存放，每个用户的每条记录都单独存放
u16 visit_global_count = 0;                                  //全局的记录数量

struct sys_time global_date;

time_to_index time_idx[MAX_RECORD_NUM];         //时间索引表
u16 time_idx_count = 0;                         //当前有效索引数量




#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

#define REC_RUNNING_TO_HOME     1  //录像时返回主界面
#define ENC_MENU_HIDE_ENABLE    1  //选定菜单后子菜单收起
#define TOUCH_R90 				1  //触摸屏XY反转

struct rec_menu_info {
    char resolution;
    char double_route;
    char mic;
    char gravity;
    char motdet;
    char park_guard;
    char wdr;
    char cycle_rec;
    char car_num;
    char dat_label;
    char exposure;
    char gap_rec;

    u8 lock_file_flag; /* 是否当前文件被锁 */

    u8 page_exit;  /*页面退出方式  1切模式退出  2返回HOME退出 */
    u8 menu_status;/*0 menu off, 1 menu on*/
    s8 enc_menu_status;
    u8 battery_val;
    u8 battery_char;

    u8 onkey_mod;
    s8 onkey_sel;
    u8 key_disable;
    u8 hlight_show_status;  /* 前照灯显示状态 */

    u8 language;
    u8 volume_lv;

    int remain_time;
};


int rec_cnt = 0;
volatile char if_in_rec; /* 是否正在录像 */
static struct rec_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct rec_menu_info))

//static u32 test_tx = 0xa533;

extern int spec_uart_send(char *buf, u32 len);
extern int spec_uart_recv(char *buf, u32 len);

extern int uart_send_package(u8 *mode,u8 *data,u8 com_len);
extern int uart_recv_retransmit();
extern void delay_hide_status();


extern int storage_device_ready();
int sys_cur_mod;
u8 av_in_statu = 0;

u8 paper_lay_flag = 0;      //壁纸界面显示标志位
u8 page_pic_flag = 0;           //设置主界面页面标志位 0--第一页  1--第二页
static const u8 num_input[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9', ' ', '.'
};

u8 user_name[26]={0};       //储存输入的用户名
int user_name_num = 0;      //当前输入字符的个数
u8 user_name_arrsy[10][26] = {0};       //保存用户名的数组
u16 user_count = 0;      //存储用户的个数
u8 list_page_num = 0;           //用户名列表页数
u8 list_cur_page = 0;
u8 user_page_flag = 0;          //用户名称设置界面标志位 0--用户管理  1--用户名称  2--用户详情 3--用户密钥
u8 now_btn_user = 0;        //当前选择进入的用户

bool user_function_array[10][2]={0};

u8 goto_facial_page_flag = 0;       //进入人脸界面标志位
u8 lock_array[8]={0};               //门锁配置项储存
static char move_num_str[10] ={0};
u8 enc_back_flag = 0;       //主界面显示标志位

u8 list_valid_array[MAX_USER_NUM] = {0};//有效用户的编号
u16 list_num_increase = 1;//用户编号，一直递增

static u8 record_now_page = 0;

static u8 erase_password_array[10] = {9,6,5,7,5,6,6,0,4,5};
u32 write_pointer = 0;  // 当前写入指针
u32 start_pointer = 0;  // 指向最早记录的指针，以便在超过500条数据后，替代最早的记录

u8 count_buffer = 0;

static u8 sys_info_flag = 0;//0--列表 1--关于设备 2--系统自检 3--老化测试 4--出厂测试 5--状态信息 6--检测详情
u8 user_key_num = 0;
u8 power_key_num = 0;
u8 auto_check_flag = 0;
u8 power_flag = UNLOCK;//指示当前密码是用于管理员权限还是开锁
u16 current_user = 0;

extern u8 lock_on;//0--未解锁 1--解锁
extern u8 ani_flag;//1--动画播放

int tim_handle = 0;//桌面闪烁定时器
static u8 net_page_flag = 0;//0--网络设置列表 1--启动配网
u8 device_status[10] = {0};
u8 aoto_check_page = 0;
u8 auto_check_status[10] = {0};
/************************************************************
				    	录像模式设置
************************************************************/
/*
 * rec分辨率设置
 */
static const u8 table_video_resolution[] = {
    VIDEO_RES_1080P,
    VIDEO_RES_720P,
    VIDEO_RES_VGA,
};


/*
 * rec循环录像设置
 */
static const u8 table_video_cycle[] = {
    0,
    3,
    5,
    10,
};


/*
 * rec曝光补偿设置
 */
static const u8 table_video_exposure[] = {
    3,
    2,
    1,
    0,
    (u8) - 1,
    (u8) - 2,
    (u8) - 3,
};


/*
 * rec重力感应设置
 */
static const u8 table_video_gravity[] = {
    GRA_SEN_OFF,
    GRA_SEN_LO,
    GRA_SEN_MD,
    GRA_SEN_HI,
};



/*
 * rec间隔录影设置, ms
 */
static const u16 table_video_gap[] = {
    0,
    100,
    200,
    500,
};

static const u16 province_gb2312[] = {
    0xA9BE, 0xFEC4, 0xA8B4, 0xA6BB, 0xF2BD, //京，宁，川，沪，津
    0xE3D5, 0xE5D3, 0xE6CF, 0xC1D4, 0xA5D4, //浙，渝，湘，粤，豫
    0xF3B9, 0xD3B8, 0xC9C1, 0xB3C2, 0xDABA, //贵，赣，辽，鲁，黑
    0xC2D0, 0xD5CB, 0xD8B2, 0xF6C3, 0xFABD, //新，苏，藏，闽，晋
    0xEDC7, 0xBDBC, 0xAABC, 0xF0B9, 0xCAB8, //琼，冀，吉，桂，甘，
    0xEECD, 0xC9C3, 0xF5B6, 0xC2C9, 0xE0C7, //皖，蒙，鄂，陕，青，
    0xC6D4                                  //云
};

static const u8 num_table[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9'
};


struct car_num {
    const char *mark;
    u32 text_id;
    u32 text_index;
};

struct car_num_str {
    u8 province;
    u8 town;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
};

/*
 * system语言设置
 */
static const u8 table_rec_language[] = {
    Chinese_Simplified,  /* 简体中文 */
    English,             /* 英文 */
};

/*
 * 音量设置
 */
static const u8 table_key_voice[] = {
    VOICE_LOW,
    VOICE_MIDDLE,
    VOICE_HIGHT,
};

enum {
    PAGE_SHOW = 0,
    MODE_SW_EXIT,
    HOME_SW_EXIT,
};


/*************************************Changed by liumenghui*************************************/
user_infor *user_data = NULL;//全局的结构体，减少内存分配

void get_user_infor_struct()
{
    if (user_data == NULL) {
        user_data = malloc(sizeof(user_infor));
        if (user_data != NULL) {
            // 初始化结构体
            memset(user_data,0,sizeof(user_infor));
        }
    }
}



/*****************************校验位 ************************************/
//u16 calculate_checksum(const Data *array,u16 command_length)
u16 calculate_checksum(void *buf,u16 data_length,u8 flag)//0--计算串口数据 1--计算记录数据
{
    u16 sum = 0;
    u8 sum_H,sum_L,i;
    if(flag == 0)
    {
        Data *array = (Data *)buf;
        //put_buf(array, 6+array->length);

//        const u8 *head_len_data = (const u8 *)&array->header;
//        for (i = 0; i < 2; i++)
//        {
//            sum += head_len_data[i];//header、length校验和
//        }
        sum += (array->length + array->tag + array->command);

        const u8 *com_data = (const u8 *)array->data;
        for(i = 0; i< data_length; i++)
        {
            sum += com_data[i];
        }
        //printf("uart checksum 0x%x\n",sum);

    }
    else if(flag == 1)
    {
        user_visit *visit = (user_visit *)buf;
        sum += (visit->record_time.year & 0xFF);
        sum += (visit->record_time.year >> 8) & 0xFF;
        sum += visit->record_time.month + visit->record_time.day +
               visit->record_time.hour + visit->record_time.min +
               visit->record_time.sec;

        sum += visit->unlock.unlock_mode;
        sum += visit->unlock.unlock_power;
        for(i = 0; i < MAX_NAME_LEN; i++)
        {
            sum += visit->name[i];
        }
        //printf("record data checksum 0x%x\n",sum);
    }
    else if(flag == 2)
    {
        u8 *recv = (u8 *)buf;
        put_buf(recv, 6+recv[3]);
        for(i = 2; i < 4+recv[3]; i++){
            sum += recv[i];
        }
        printf("check 0x%x\n",sum);
    }


    return sum;
}


/*****************************创建数据包 ************************************/
Data *create_packet_uncertain_len(u8 tag,u8 command,u8 *data,u8 com_len)
{
    Data uart_msg;
    Packet packet;
    u8 i;

    uart_msg.header = 0xCDCD;
    if(tag){
        uart_msg.tag = tag;
    }else{
        uart_msg.tag = 0x01;
    }
    uart_msg.command = command;

    uart_msg.data = (u8 *)malloc(com_len);//分配 数组长度 x 类型长度的空间
    if(uart_msg.data == NULL)
    {
        free(uart_msg.data);
        return NULL;
    }
    memcpy(uart_msg.data,data,com_len);

    uart_msg.length = sizeof(u8) + com_len;//模式和命令的长度
    memcpy(&packet.data, &uart_msg, sizeof(uart_msg));

    packet.check = calculate_checksum(&packet.data,com_len,0);//

    u8 *data_packet = malloc(sizeof(Packet));
    if (data_packet == NULL) {
        return NULL;
    }
    memcpy(data_packet, &packet.data, 5);//header,tag,length,command
    memcpy(data_packet+5, packet.data.data, com_len);//data
    data_packet[uart_msg.length+4] = packet.check >> 8;//check  2header 1length
    data_packet[uart_msg.length+5] = packet.check & 0xFF;

    return data_packet;
}



void add_new_record(time_to_index record)
{
    // Step 1: 数据前移
    for (int i = 0; i < MAX_RECORD_NUM - 1; i++) {
        time_idx[i] = time_idx[i + 1];
    }

    // Step 2: 将新数据添加到末尾
    time_idx[MAX_RECORD_NUM - 1] = record;
}



void buffer_sector_write_flash(u8 num)
{
#if 0
    u8 buff[SECTOR_SIZE] = {0};
#else
    u8 *buff = malloc(SECTOR_SIZE);
    if(buff)
    {
        memset(buff,0,SECTOR_SIZE);
    }
#endif
    u16 len = 0;

    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return ;
    }

    len = dev_ioctl(dev, IOCTL_ERASE_SECTOR, RECORD_BASE_ADDR - num * SECTOR_SIZE);     //擦除第num扇区
    if(len)
    {
        printf("erase error\n");
    }

    len = dev_bulk_read(dev, buff, BUFFER_ADDR-SECTOR_SIZE, SECTOR_SIZE);        //从buffer扇区地址读出整块数据
    if(len != SECTOR_SIZE)
    {
        printf("read error!\n");
    }

    len = dev_bulk_write(dev, buff, RECORD_BASE_ADDR - num * SECTOR_SIZE , SECTOR_SIZE);//整个扇区数据写入第num扇区
    if(len != SECTOR_SIZE)
    {
        printf("write error!\n");
    }

    len = dev_ioctl(dev, IOCTL_ERASE_SECTOR, BUFFER_ADDR - SECTOR_SIZE);     //擦除缓冲扇区
    if(len)
    {
        printf("erase error\n");
    }
    printf("erase %d sector,write %d sector\n",num,num);

    if(buff)
    {
        free(buff);
        buff = NULL;
    }
}

void write_data_to_flash(void *buf, u8 flag)//0--写用户数据 1--写记录
{
    user_infor write_data = {0};
    u8 index;
    u16 len = 0;
    u16 checksum = 0;
    u32 sector_offset;          //扇区内的偏移
    u8 sector_number;           //扇区编号
    u32 write_addr;
    user_visit max_visit = {0};
    time_to_index new_record = {0};

    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
    return ;
    }
    if (flag == 0) {
        // 写用户数据到Flash的代码
        user_infor *user_buf = ( user_infor *)buf;
        memcpy(&write_data,user_buf,sizeof(user_infor));
        index = write_data.index;
        if(index == 0)
        {
            len = dev_ioctl(dev, IOCTL_ERASE_SECTOR, write_user_offset - SECTOR_SIZE);     //擦除基地址前1个扇区
            if(len)
            {
             printf("erase error\n");
            }
            len = dev_bulk_write(dev, user_buf, write_user_offset-sizeof(user_infor), sizeof(user_infor));                   //从基地址往前面写用户数据
            printf("write flash addr 0x%x  data addr 0x%x",write_user_offset,write_user_offset-sizeof(user_infor));
            write_user_offset -= SECTOR_SIZE;          //基地址往前移4k
        }
        else
        {
            write_data.index = 0;
            dev_ioctl(dev, IOCTL_ERASE_SECTOR, BASE_ADDRESS - SECTOR_SIZE * (index - 1) - SECTOR_SIZE);
            len = dev_bulk_write(dev, user_buf, BASE_ADDRESS - SECTOR_SIZE * (index - 1) - sizeof(user_infor), sizeof(user_infor));
            printf("insert index addr 0x%x  data addr 0x%x",BASE_ADDRESS - SECTOR_SIZE * (index - 1),BASE_ADDRESS - SECTOR_SIZE * (index - 1)-sizeof(user_infor));
        }
        if(len != sizeof(user_infor))
        {
            printf("write error!\n");
        }

    } else if (flag == 1) {
        // 写记录数据到Flash的代码
        user_visit *rec_buf = ( user_visit *)buf;
        len = dev_bulk_read(dev, &max_visit, MAX_RECORD_ADDR-RECORD_SIZE, RECORD_SIZE);        //从第500条记录地址读出数据，如果没数据说明是从来没超过500条
        checksum = calculate_checksum(&max_visit, RECORD_SIZE, 1);
        if (start_pointer == 0 && write_pointer < MAX_RECORD_NUM && (checksum != max_visit.check)) //未超过500条记录，正常写前4个扇区
        {
            sector_number = write_pointer / RECORDS_PER_SECTOR;//RECORDS_PER_SECTOR
            sector_offset = (write_pointer % RECORDS_PER_SECTOR) * RECORD_SIZE;
            write_addr = RECORD_BASE_ADDR - sector_number * SECTOR_SIZE - sector_offset;

            checksum = calculate_checksum(rec_buf, RECORD_SIZE, 1);
            printf("check 0x%x\n",checksum);
            rec_buf->check = checksum;
            len = dev_bulk_write(dev, rec_buf, write_record_offset - RECORD_SIZE, RECORD_SIZE);//从基地址往前面写记录数据
            printf("record %d addr 0x%x - 0x%x\n",(RECORD_BASE_ADDR - write_record_offset) / RECORD_SIZE,write_record_offset - RECORD_SIZE,write_record_offset);
            write_record_offset -= RECORD_SIZE;          //基地址往前移一个记录长度
        }
        else                                                    //超过500条记录，先写满缓冲扇区，再拷贝到前四个扇区
        {
            checksum = calculate_checksum(rec_buf, RECORD_SIZE, 1);
            rec_buf->check = checksum;
            len = dev_bulk_write(dev, rec_buf, BUFFER_ADDR - count_buffer*RECORD_SIZE-RECORD_SIZE, RECORD_SIZE);//从缓冲扇区地址往前面写记录数据
            printf("write buffer addr 0x%x name : %s check %d\n",BUFFER_ADDR - count_buffer*RECORD_SIZE,rec_buf->name,rec_buf->check);
            count_buffer++;
            printf("count buff %d\n",count_buffer);
            if(count_buffer == RECORDS_PER_SECTOR)           //缓冲扇区写满128条
            {
                 buffer_sector_write_flash(start_pointer / RECORDS_PER_SECTOR);//擦除普通扇区，拷贝缓冲扇区，再擦除缓冲扇区
                 count_buffer = 0;
            }
        }
        if(time_idx_count < MAX_RECORD_NUM)//小于500时正常递增存放在数组
        {
            time_idx[time_idx_count].year = rec_buf->record_time.year;
            time_idx[time_idx_count].month = rec_buf->record_time.month;
            time_idx[time_idx_count].day = rec_buf->record_time.day;
            time_idx[time_idx_count].idx = time_idx_count;
            printf("visit time %d/%d/%d index = %d\n",time_idx[time_idx_count].year,time_idx[time_idx_count].month,time_idx[time_idx_count].day,time_idx[time_idx_count].idx);
            time_idx_count++;
        }
        else
        {
            time_idx_count = MAX_RECORD_NUM;
            new_record.year = rec_buf->record_time.year;
            new_record.month = rec_buf->record_time.month;
            new_record.day = rec_buf->record_time.day;
            new_record.idx = time_idx_count;
            add_new_record(new_record);//数组整体往前移，同时新增记录到末尾
        }
        db_update("spt",start_pointer);                 //存起始指针
        db_flush();
        if(start_pointer == 0){
            write_pointer = (write_pointer + 1) % MAX_RECORD_NUM;    // 更新写入指针
            if (write_pointer == start_pointer) {                    // 记录超过500条了
                start_pointer = (start_pointer + 1) % 512;// 写入超过500条时，起始指针向前移动                   4个扇区512条记录

            }
        } else {
            printf("now point %d\n",start_pointer);
            write_pointer = (write_pointer + 1) % MAX_RECORD_NUM;  // 正常更新 write_pointer
            start_pointer = (start_pointer + 1) % 512;  // 每次写入都递增 start_pointer               4个扇区512条记录
        }
     }

     dev_close(dev);
     dev = NULL;

}


 /**/
void read_data_from_flash()
{
    user_visit read_data = {0};
    user_visit buffer_data = {0};
    u32 read_idx;
    u8 len = 0;
    u16 checksum = 0;
    visit_global_count = 0;
#if 1
    write_record_offset = RECORD_BASE_ADDR;
    time_idx_count = 0;
    memset(time_idx,0,sizeof(time_idx));
    write_pointer = 0;
    count_buffer = 0;
#endif
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
     return ;
    }

    start_pointer = db_select("spt");                       //读取起始指针的值
    printf("start_pointer = %d\n",start_pointer);
    len = dev_bulk_read(dev, &buffer_data, BUFFER_ADDR - RECORD_SIZE, RECORD_SIZE);        //从缓冲地址读出第一条数据，判断缓冲扇区有无数据
    checksum = calculate_checksum(&buffer_data, RECORD_SIZE, 1);
//    printf("buffer_data check %d name %s\n",buffer_data.check,buffer_data.name);
    if(checksum != buffer_data.check)                             //第一种情况，还未写满500条；第二种情况，写满500条后，继续写满了缓冲扇区，全部拷贝到普通扇区，缓冲扇区清零了
    {
        for(int i = 0; i < MAX_RECORD_NUM; i++)
        {
            read_idx = (start_pointer + i) % (MAX_RECORD_NUM);
            len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*read_idx-RECORD_SIZE, RECORD_SIZE);        //从基地址读出数据
            if(len != RECORD_SIZE)
            {
                printf("read error!\n");
            }
            checksum = calculate_checksum(&read_data, RECORD_SIZE, 1);
            if(read_data.check == checksum)
            {
                write_record_offset = RECORD_BASE_ADDR - read_idx * RECORD_SIZE - RECORD_SIZE;

                time_idx[time_idx_count].year = read_data.record_time.year;                 //记录时间到索引表
                time_idx[time_idx_count].month = read_data.record_time.month;
                time_idx[time_idx_count].day = read_data.record_time.day;
                time_idx[time_idx_count].idx = read_idx;

                time_idx_count++;
                if(time_idx_count > MAX_RECORD_NUM){
                    time_idx_count = MAX_RECORD_NUM;
                }
                write_pointer = (write_pointer + 1) % MAX_RECORD_NUM;
            }
            else
            {
                //break;
            }
        }
    }
    else                                                        //写满500条后，开始写缓冲扇区，但是还未写满 (start_pointer-MAX_RECORD_NUM                     0-start_pointer)
    {
        for(int i = start_pointer; i < MAX_RECORD_NUM; i++)
        {
            len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*i-RECORD_SIZE, RECORD_SIZE);        //从普通扇区地址读出数据
            if(len != RECORD_SIZE)
            {
                printf("read error!\n");
            }
            checksum = calculate_checksum(&read_data, RECORD_SIZE, 1);

            if(read_data.check == checksum)
            {
                write_record_offset = RECORD_BASE_ADDR - i * RECORD_SIZE - RECORD_SIZE;

                time_idx[time_idx_count].year = read_data.record_time.year;                 //记录时间到索引表
                time_idx[time_idx_count].month = read_data.record_time.month;
                time_idx[time_idx_count].day = read_data.record_time.day;
                time_idx[time_idx_count].idx = time_idx_count;//i;
                time_idx_count++;
                if(time_idx_count > MAX_RECORD_NUM){
                    time_idx_count = MAX_RECORD_NUM;
                }
                write_pointer = (write_pointer + 1) % MAX_RECORD_NUM;
            }
            else
            {
                //break;
            }
        }
        if(start_pointer >= RECORDS_PER_SECTOR){
            for(int i = 0; i < (start_pointer / RECORDS_PER_SECTOR * RECORDS_PER_SECTOR); i++)
            {
                len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*i-RECORD_SIZE, RECORD_SIZE);        //从copy扇区地址读出数据
                if(len != RECORD_SIZE)
                {
                    printf("read error!\n");
                }
                checksum = calculate_checksum(&read_data, RECORD_SIZE, 1);

                if(read_data.check == checksum)
                {
                    write_record_offset = RECORD_BASE_ADDR - i * RECORD_SIZE - RECORD_SIZE;

                    time_idx[time_idx_count].year = read_data.record_time.year;                 //记录时间到索引表
                    time_idx[time_idx_count].month = read_data.record_time.month;
                    time_idx[time_idx_count].day = read_data.record_time.day;
                    time_idx[time_idx_count].idx = time_idx_count;//i;

                    time_idx_count++;
                    if(time_idx_count > MAX_RECORD_NUM){
                    time_idx_count = MAX_RECORD_NUM;
                    }
                    write_pointer = (write_pointer + 1) % MAX_RECORD_NUM;
                }
                else
                {
                    //break;
                }
            }
        }
    }
    for(int i = 0; i < RECORDS_PER_SECTOR; i++)
    {
        len = dev_bulk_read(dev, &read_data, BUFFER_ADDR-RECORD_SIZE*i-RECORD_SIZE, RECORD_SIZE);        //从缓冲地址读出数据
        if(len != RECORD_SIZE)
        {
            printf("read error!\n");
        }
        checksum = calculate_checksum(&read_data, RECORD_SIZE, 1);

        if(read_data.check == checksum)
        {
            write_record_offset = RECORD_BASE_ADDR - i * RECORD_SIZE - RECORD_SIZE;

            if(time_idx_count < MAX_RECORD_NUM){
                time_idx[time_idx_count].year = read_data.record_time.year;                 //记录时间到索引表
                time_idx[time_idx_count].month = read_data.record_time.month;
                time_idx[time_idx_count].day = read_data.record_time.day;
                time_idx[time_idx_count].idx = time_idx_count;//MAX_RECORD_NUM+i;
                time_idx_count++;
            }else{
                time_idx_count = MAX_RECORD_NUM;
                for (int i = 0; i < MAX_RECORD_NUM - 1; i++) {
                    time_idx[i] = time_idx[i + 1];
                }

                // Step 2: 将新数据添加到末尾
                time_idx[MAX_RECORD_NUM - 1].year = read_data.record_time.year;
                time_idx[MAX_RECORD_NUM - 1].month = read_data.record_time.month;
                time_idx[MAX_RECORD_NUM - 1].day = read_data.record_time.day;

            }
            //printf("idx %d : year %d month %d day %d",time_idx_count,time_idx[time_idx_count-1].year,time_idx[time_idx_count-1].month,time_idx[time_idx_count-1].day);
            write_pointer = (write_pointer + 1) % MAX_RECORD_NUM;
            count_buffer++;
        }
        else
        {
            break;
        }
    }
    if(start_pointer){
        start_pointer += 1;
    }
    printf("time visit count : %d count_buff %d start_pointer %d\n",time_idx_count,count_buffer,start_pointer);
    dev_close(dev);
    dev = NULL;
}

u8 erase_flash(u8 num)
{
    u32 offset = 0;
    u8 len;
    user_infor data = {0};
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return -1;
    }
    if(num == 0xFF)
    {
        db_update("spt",0);                 //存起始指针
        db_flush();
        time_idx_count = 0;
        memset(time_idx,0,sizeof(time_idx));
        offset = BASE_ADDRESS - ((MAX_USER_NUM + 5) * 4 * 1024);
        for(int i = 0; i < (MAX_USER_NUM+5+1); i++)
        {
            len = dev_ioctl(dev, IOCTL_ERASE_SECTOR, offset - SECTOR_SIZE);     //擦除扇区
            if(len)
            {
                printf("erase error\n");
            }
            printf("erase flash addr 0x%x - 0x%x",offset - SECTOR_SIZE,offset);
            offset += SECTOR_SIZE;
            if(offset > BASE_ADDRESS)
            {
                write_user_offset  = BASE_ADDRESS;
                write_record_offset = RECORD_BASE_ADDR;
                write_pointer = 0;
                break;
            }
        }
    }
    else if(num == 0xAA)
    {
        db_update("spt",0);                 //存起始指针
        db_flush();
        time_idx_count = 0;
        write_pointer = 0;
        start_pointer = 0;
        count_buffer = 0;
        memset(time_idx,0,sizeof(time_idx));
        offset = RECORD_BASE_ADDR - 4 * SECTOR_SIZE;
        for(int i = 0; i < 5; i++)
        {
            len = dev_ioctl(dev, IOCTL_ERASE_SECTOR, offset - SECTOR_SIZE);     //擦除扇区
            if(len)
            {
                printf("erase error\n");
            }
            printf("erase flash addr 0x%x - 0x%x",offset - SECTOR_SIZE,offset);
            offset += SECTOR_SIZE;
            if(offset > RECORD_BASE_ADDR)
            {
                //write_user_offset  = BASE_ADDRESS;
                write_record_offset = RECORD_BASE_ADDR;
                break;
            }
        }
    }
    else
    {
        write_user_offset = BASE_ADDRESS - (num * 4 * 1024);

        len = dev_ioctl(dev, IOCTL_ERASE_SECTOR, write_user_offset - SECTOR_SIZE);     //擦除扇区
        if(len)
        {
            printf("erase error\n");
        }
        printf("erase flash addr 0x%x - 0x%x",write_user_offset - SECTOR_SIZE,write_user_offset);

    }
    dev_close(dev);
    dev = NULL;
}


/*通过结构体的编号，返回对应的结构体*/
user_infor *match_user_num(u16 num)
{
    u8 offset = 0;
    u32 read_size = 0;
    user_infor *match_num = malloc(sizeof(user_infor));
    if(match_num)
    {
        memset(match_num,0,sizeof(user_infor));
    }
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return NULL;
    }
    for (offset = 0; offset < MAX_USER_NUM; offset++)
    {
        read_size = dev_bulk_read(dev, match_num, BASE_ADDRESS - SECTOR_SIZE * offset - sizeof(user_infor), sizeof(user_infor));//基地址-分配每个记录的地址大小-用户数据大小
        if(read_size != sizeof(user_infor))
        {
            printf("read data error!\n");
        }
        if(match_num->user_num == num)//匹配对应的编号
        {
            match_num->index = offset + 1;
            //printf("match user num success\n");
            dev_close(dev);
            dev = NULL;

            return match_num;
        }

    }
    dev_close(dev);
    dev = NULL;
    return NULL;
}

u8 match_num_to_insert()
{
    u32 read_size = 0;
    user_infor match_index = {0};
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return NULL;
    }
    for(int i = 0; i < MAX_USER_NUM; i++)
    {
        read_size = dev_bulk_read(dev, &match_index, BASE_ADDRESS - SECTOR_SIZE * i - sizeof(user_infor), sizeof(user_infor));//基地址-分配每个记录的地址大小-用户数据大小
        if(read_size != sizeof(user_infor))
        {
            printf("read data error!\n");
        }
        if(match_index.user_num == 0xFFFF)
        {
            printf("Find insert index %d\n",i);
            dev_close(dev);
            dev = NULL;
            return i+1;
        }
    }
    return false;
}


#if 1

/*比较用户输入的密码，成功后返回结构体数据*/
user_infor *match_user_data(u8 *buf,u32 len,u32 size)
{
    u8 count,match_offset = 0;
    u32 read_size = 0;
    user_infor *match_data = malloc(sizeof(user_infor));
    if(match_data)
    {
        memset(match_data,0,sizeof(user_infor));
    }

    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        free(match_data);
        return NULL;
    }

    for (match_offset = 0; match_offset < MAX_USER_NUM; match_offset++)
    {
        read_size = dev_bulk_read(dev, match_data, BASE_ADDRESS - SECTOR_SIZE * match_offset - size, size);//基地址-分配每个记录的地址大小-用户数据大小
        if(read_size != size)
        {
            printf("read data error!\n");
        }
        for(int i = 0; i < MAX_KEY_NUM; i++)
        {
            if(memcmp(buf,match_data->key[i].key_buf,len) == 0)
            {
                printf("match password data success\n");
                dev_close(dev);
                dev = NULL;
                return match_data;
            }
        }

        if(strcmp(buf,match_data->name) == 0)
        {
            printf("match same name!\n");
            dev_close(dev);
            dev = NULL;
            return match_data;
        }

    }

    dev_close(dev);
    dev = NULL;
    return NULL;
}
#endif


u16 tim_cnt = 0;
u16 pre_page_cnt = 0;
u16 next_page_cnt = 0;
//u16 buffer_idx = 0;
u32 normal_idx = MAX_RECORD_NUM;
u16 copy_buffer_idx = 0;
u16 match_visit_time(struct sys_time *sys_time,user_visit *visit,u16 idx,u8 flag)//0--上一页  1--下一页  idx是time_idx数组下标，代表翻页进度
{

    int i = 0;
    u16 checksum = 0;
    u16 index = 0;
    u16 len = 0;
    u16 count = 0;
    u16 read_idx = 0;
    user_visit read_data;
    time_to_index Tim[MAX_RECORD_NUM];

    u8 pre_cnt = 0;

    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return 0;
    }
    start_pointer = db_select("spt");                       //读取起始指针的值
    dev_bulk_read(dev, &read_data, BUFFER_ADDR-RECORD_SIZE, RECORD_SIZE);//判断缓冲扇区有无数据
    checksum = calculate_checksum(&read_data, RECORD_SIZE, 1);
    printf("checksum %d check %d",checksum,read_data.check);
    if(checksum != read_data.check || count_buffer == 0){
        if(start_pointer == 0){
            if(flag){                                            //下一页
/*************************第一种情况：未满500条记录,从记录总数往前面读取，startpointer = 0*************************/
                for(i = tim_cnt; i < time_idx_count; i--){
                    len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*i-RECORD_SIZE, RECORD_SIZE);
                    if(len != RECORD_SIZE){
                        printf("read error!\n");
                    }
                    visit[count] = read_data;
                    count++;
                    if(time_idx[i].year == time_idx[i-1].year && time_idx[i].month == time_idx[i-1].month && time_idx[i].day == time_idx[i-1].day){
                        continue;
                    } else {
                        printf("buffer break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                        break;
                    }
                }
            } else {                                            //上一页
                for(i = tim_cnt; i < time_idx_count ; i++){
                    len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*i-RECORD_SIZE, RECORD_SIZE);
                    if(len != RECORD_SIZE){
                        printf("read error!\n");
                    }
                    visit[count] = read_data;
                    count++;
                    if(time_idx[i].year == time_idx[i+1].year && time_idx[i].month == time_idx[i+1].month && time_idx[i].day == time_idx[i+1].day){
                        continue;
                    } else {
                        break;
                    }
                }
            }
        }else{
/*************************第三种情况：缓冲扇区满，拷贝到普通扇区，清空缓冲扇区，startpointer在拷贝扇区下一个扇区起始位置*************************/
            if(flag){
                for(i = tim_cnt; i < time_idx_count; i--){
                    read_idx = (start_pointer + i) % MAX_RECORD_NUM;
                    len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*(read_idx)-RECORD_SIZE, RECORD_SIZE);//
                    if(len != RECORD_SIZE){
                        printf("read error!\n");
                    }
                    visit[count] = read_data;
                    count++;
                    if(time_idx[i].year == time_idx[i-1].year && time_idx[i].month == time_idx[i-1].month && time_idx[i].day == time_idx[i-1].day){
                        continue;
                    } else {
                        printf("normal break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                        break;
                    }
                }
            } else {
                for(i = tim_cnt; i < time_idx_count; i++){
                    read_idx = (start_pointer + i ) % MAX_RECORD_NUM;
                    len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*(read_idx)-RECORD_SIZE, RECORD_SIZE);//
                    if(len != RECORD_SIZE){
                        printf("read error!\n");
                    }
                    visit[count] = read_data;
                    count++;
                    if(time_idx[i].year == time_idx[i+1].year && time_idx[i].month == time_idx[i+1].month && time_idx[i].day == time_idx[i+1].day){
                        continue;
                    } else {
                        printf("normal break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                        break;
                    }
                }
            }
        }
    } else {
/*************************第二种情况：超过MAX_RECORD_NUM，缓冲扇区未满136，startpointer 在第一个扇区*************************/
        if(flag){       //下一页
            for(i = tim_cnt; i < time_idx_count; i--){
                if((i - MAX_RECORD_NUM + count_buffer) >= 0){
                    len = dev_bulk_read(dev, &read_data, BUFFER_ADDR-RECORD_SIZE*(i-MAX_RECORD_NUM+count_buffer)-RECORD_SIZE, RECORD_SIZE);//先读缓冲
                    if(len != RECORD_SIZE){
                        printf("read error!\n");
                    }
                    printf("1111 date %d : %d/%d/%d",i-MAX_RECORD_NUM+count_buffer,read_data.record_time.year,read_data.record_time.month,read_data.record_time.day);
                    visit[count] = read_data;
                    count++;
                    if(time_idx[i].year == time_idx[i-1].year && time_idx[i].month == time_idx[i-1].month && time_idx[i].day == time_idx[i-1].day){
                        continue;
                    } else {
                        printf("buffer break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                        break;
                    }
                } else {
                        //normal_idx = (start_pointer + normal_idx) % MAX_RECORD_NUM;
/*************************第四种情况：超过500，缓冲扇区有记录，未满136，startpointer在扇区中间某位置*************************/
                    if(count &&(time_idx[i].year != time_idx[i+1].year && time_idx[i].month != time_idx[i+1].month && time_idx[i].day != time_idx[i+1].day)){
                        printf("buffer break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                        continue;
                    }
                    copy_buffer_idx--;
                    if(start_pointer / RECORDS_PER_SECTOR && copy_buffer_idx < MAX_RECORD_NUM){             //读拷贝扇区
                        len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*copy_buffer_idx-RECORD_SIZE, RECORD_SIZE);//
                        if(len != RECORD_SIZE){
                            printf("read error!\n");
                        }
                        visit[count] = read_data;
                        count++;
                        if(time_idx[i].year == time_idx[i-1].year && time_idx[i].month == time_idx[i-1].month && time_idx[i].day == time_idx[i-1].day){
                            continue;
                        } else {
                            printf("normal break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                            break;
                        }
                    } else {
                        normal_idx--;
                        len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*normal_idx-RECORD_SIZE, RECORD_SIZE);//再读普通扇区
                        if(len != RECORD_SIZE){
                            printf("read error!\n");
                        }
                        visit[count] = read_data;
                        count++;
                        if(time_idx[i].year == time_idx[i-1].year && time_idx[i].month == time_idx[i-1].month && time_idx[i].day == time_idx[i-1].day){
                            continue;
                        } else {
                            printf("normal break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                            break;
                        }
                    }
                }
            }
        }else {     //上一页
                for(i = tim_cnt; i < time_idx_count; i++){
                    if((i - MAX_RECORD_NUM + count_buffer) >= 0){

                        len = dev_bulk_read(dev, &read_data, BUFFER_ADDR-RECORD_SIZE*(i-MAX_RECORD_NUM+count_buffer)-RECORD_SIZE, RECORD_SIZE);//先读缓冲
                        if(len != RECORD_SIZE){
                            printf("read error!\n");
                        }
                        visit[count] = read_data;
                        count++;
                        if(time_idx[i].year == time_idx[i+1].year && time_idx[i].month == time_idx[i+1].month && time_idx[i].day == time_idx[i+1].day){
                            continue;
                        } else {
                            if(pre_cnt){
                                normal_idx = (normal_idx + pre_cnt - count) % MAX_RECORD_NUM;
                            } else {
                                normal_idx = MAX_RECORD_NUM;//翻页到buffer扇区，重置normal扇区的idx
                            }
                            printf("normal %d buffer break time_idx[%d] date %d/%d/%d\n",normal_idx,i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                            break;
                        }
                    } else {
/*************************第四种情况：超过500，缓冲扇区有记录，未满136，startpointer在扇区中间某位置*************************/
                    if(count &&(time_idx[i].year != time_idx[i+1].year && time_idx[i].month != time_idx[i+1].month && time_idx[i].day != time_idx[i+1].day)){
                        printf("buffer break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                        continue;
                    }
                    copy_buffer_idx++;
                    if(start_pointer / RECORDS_PER_SECTOR && copy_buffer_idx < MAX_RECORD_NUM){             //已从缓冲扇区拷贝到普通扇区
                        len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*copy_buffer_idx-RECORD_SIZE, RECORD_SIZE);//再读普通扇区 MAX_RECORD_NUM-
                        if(len != RECORD_SIZE){
                            printf("read error!\n");
                        }
                        visit[count] = read_data;
                        count++;
                        if(time_idx[i].year == time_idx[i+1].year && time_idx[i].month == time_idx[i+1].month && time_idx[i].day == time_idx[i+1].day){

                            continue;
                        } else {
                            printf("normal break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                            break;
                        }
                    } else {
                        normal_idx++;
                        len = dev_bulk_read(dev, &read_data, RECORD_BASE_ADDR-RECORD_SIZE*normal_idx-RECORD_SIZE, RECORD_SIZE);//再读普通扇区 MAX_RECORD_NUM-
                        if(len != RECORD_SIZE){
                            printf("read error!\n");
                        }
                        visit[count] = read_data;
                        count++;
                        pre_cnt++;
                        if(time_idx[i].year == time_idx[i+1].year && time_idx[i].month == time_idx[i+1].month && time_idx[i].day == time_idx[i+1].day){
                            continue;
                        } else {
                            printf("normal break time_idx[%d] date %d/%d/%d\n",i,time_idx[i].year,time_idx[i].month,time_idx[i].day);
                            break;
                        }
                    }
                }
            }
        }

    }

/*************************第四种情况：超过500，缓冲扇区有记录，未满136，startpointer在扇区中间某位置*************************/

    dev_close(dev);
    dev = NULL;

    return count;

}


int move_posx(int x0, int x1)
{
#ifdef SLIDER_ENABLE
    s16 x_pos_down = 0;
    s16 x_pos_now = 0;
    s16 x_pos_ch = 0;
    static int tmp = 0;
    x_pos_down = x0;
    x_pos_now = x1;
    x_pos_ch = x1-x0;
    if (x_pos_ch < SLID_GAP && x_pos_ch > -SLID_GAP) {
        return false;
    }
    tmp = x_pos_ch / SLID_GAP;
    printf("============ x0 : %d x1 : %d tmp:%d\n",x_pos_down,x_pos_now,tmp);
    if (tmp > SLID_ITEM - 1) {
        tmp = SLID_ITEM - 1;
    }else if (tmp < -5) {
        return tmp;
    } else if(tmp > 5) {
        return tmp;
    }
#endif
    return 0;
}

int move_posy(int y0, int y1)
{
#ifdef SLIDER_ENABLE
    s16 y_pos_down = 0;
    s16 y_pos_now = 0;
    s16 y_pos_ch = 0;
    static int tmp = 0;
    y_pos_down = y0;
    y_pos_now = y1;
    y_pos_ch = y1-y0;
    if (y_pos_ch < SLID_GAP && y_pos_ch > -SLID_GAP) {
        return false;
    }
    tmp = y_pos_ch / SLID_GAP;
    printf("============ y0 : %d y1 : %d tmp:%d\n",y_pos_down,y_pos_now,tmp);
    if (tmp > SLID_ITEM - 1) {
        tmp = SLID_ITEM - 1;
    }else if (tmp < -5) {
        return tmp;
    } else if(tmp > 5) {
        return tmp;
    }
#endif
    return 0;
}




/*************************************Changed by liumenghui*************************************/


void reset_up_ui_func()
{
    ui_show(ENC_UP_LAY);
}

/*
 * (begin)提示框显示接口 ********************************************
 */

///*
// * (begin)UI状态变更主动请求APP函数 ***********************************
// */
//static void rec_tell_app_exit_menu(void)
//{
////    int err;
////    struct intent it;
////    init_intent(&it);
////    it.name	= "video_rec";
////    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
////    it.data = "exitMENU";
////    err = start_app(&it);
////    if (err) {
////        ASSERT(err == 0, ":rec exitMENU\n");
////    }
//}

///*
// * (begin)APP状态变更，UI响应回调 ***********************************
// */
//static int rec_on_handler(const char *type, u32 arg)
//{
//    puts("\n***rec_on_handler.***\n");
//    if_in_rec = TRUE;
//    ui_hide(ENC_TIM_REMAIN);
//    ui_show(ENC_TIM_REC);
//    ui_show(ENC_ANI_REC_HL);
//
//    return 0;
//}

/*
 * 录像模式的APP状态响应回调
 */
static const struct uimsg_handl rec_msg_handler[] = {
//    { "lockREC",        rec_lock_handler     }, /* 锁文件 */
//    { "onREC",          rec_on_handler       }, /* 开始录像 */
//    { "offREC",         rec_off_handler      }, /* 停止录像 */
//    { "saveREC",        rec_save_handler     }, /* 保存录像 */
//    { "noCard",         rec_no_card_handler  }, /* 没有SD卡 */
//    { "fsErr",          rec_fs_err_handler   },
//    { "avin",           rec_av_in_handler    },
//    { "avoff",          rec_av_off_handler   },
//    { "HlightOn",    rec_headlight_on_handler},
//    { "HlightOff",   rec_headlight_off_handler},
//    { "Remain",         rec_remain_handler  },
    // { "onMIC",          rec_on_mic_handler   },
    // { "offMIC",         rec_off_mic_handler  },
     { NULL, NULL},      /* 必须以此结尾！ */
};
/*
 * (end)
 */
/*****************************录像模式页面回调 ************************************/
static int video_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***rec mode onchange init***\n");
        ui_register_msg_handler(ID_WINDOW_VIDEO_REC, rec_msg_handler); /* 注册APP消息响应 */
        sys_cur_mod = 1;  /* 1:rec, 2:tph, 3:dec */
        memset(__this, 0, sizeof_this);
        break;
    case ON_CHANGE_RELEASE:
//        if (__this->menu_status) {
//#if DOUBLE_720
//            ui_hide(ENC_SET_LAY);
//#else
//            ui_hide(ENC_SET_WIN);
//#endif
//        }
        if (__this->page_exit == HOME_SW_EXIT) {
            ui_show(ID_WINDOW_MAIN_PAGE);
        }
//        __rec_msg_hide(0);//强制隐藏消息框

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
.onchange = video_mode_onchange,
 .ontouch = NULL,
};
static int parking_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_PARKING, rec_msg_handler); /* 注册APP消息响应 */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_PARKING)
.onchange = parking_page_onchange,
 .ontouch = NULL,
};


/*****************************图标布局回调 ************************************/


static int rec_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    int index;
    int err;
    static int lock_event_flag = 0;

    switch (e) {
    case ON_CHANGE_INIT:
//        lock_event_id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, lock_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        //      unregister_sys_event_handler(lock_event_id);
        break;
    case ON_CHANGE_FIRST_SHOW: /* 在此获取默认隐藏的图标的状态并显示 */

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY)
.onchange = rec_layout_up_onchange,
};



static int enc_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("enc_onchange\n");
        sys_key_event_takeover(true, false);
        __this->onkey_mod = 0;
        __this->onkey_sel = 0;

        if(goto_facial_page_flag){
            ui_show(ENC_LAY_USER_PAGE);
            ui_show(USER_GOTO_DELETE);
            ui_show(ENC_LAY_USER_DETAILS);
            break;
        }
        ui_show(ENC_LAY_BACK);
        sys_timeout_add(NULL, delay_hide_status, 5000);
        break;
    default:
        return false;
    }

    return false;
}

extern int show_back_pic();
static int enc_onkey(void *ctr, struct element_key_event *e)
{
    return true;
}
REGISTER_UI_EVENT_HANDLER(ENC_WIN)
.onchange = enc_onchange,
 .onkey = enc_onkey,
};



/***************************** 星期文字动作 ************************************/
void get_system_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    //printf("get_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec);
    dev_close(fd);
}

static void set_system_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)time);
    printf("set_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec);
    dev_close(fd);
}


static int ReturnWeekDay(unsigned int iYear, unsigned int iMonth, unsigned int iDay)
{
    int iWeek = 0;
    unsigned int y = 0, c = 0, m = 0, d = 0;

    if (iMonth == 1 || iMonth == 2) {
        c = (iYear - 1) / 100;
        y = (iYear - 1) % 100;
        m = iMonth + 12;
        d = iDay;
    } else {
        c = iYear / 100;
        y = iYear % 100;
        m = iMonth;
        d = iDay;
    }

    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
    iWeek = iWeek >= 0 ? (iWeek % 7) : (iWeek % 7 + 7);        //iWeek为负时取模
    if (iWeek == 0) {     //星期日不作为一周的第一天
        iWeek = 7;
    }

    return iWeek;
}
static int text_week_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        printf("\nit is week %d\n", ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day));
        ui_text_set_index(text, ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day) - 1);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(REC_TXT_WEEKDAY)
.onchange = text_week_onchange,
 .ontouch = NULL,
};


struct sys_time temp_date_time;

/****************************主界面时间控件动作 ************************************/
static int timer_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        /*
        temp_date_time.hour = sys_time.hour;
        temp_date_time.min = sys_time.min;
        temp_date_time.sec = sys_time.sec;
		*/
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(REC_TIM_TIME)
.onchange = timer_sys_rec_onchange,
 .ontouch = NULL,
};
/*****************************主界面系统日期控件动作 ************************************/
static int timer_sys_date_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    int temp  = 0;
    static int last_temp = 10;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
//        puts("******************text_week_onchange");
        get_system_time(&sys_time);
        temp = ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day) - 1;
        if(temp!=last_temp){
            last_temp = temp;
            ui_text_show_index_by_id(REC_TXT_WEEKDAY, last_temp);
        }
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
		/*
        temp_date_time.year = sys_time.year;
        temp_date_time.month = sys_time.month;
        temp_date_time.day = sys_time.day;
		*/
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(REC_TIM_DATE)
.onchange = timer_sys_date_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** 进入密码界面按钮 ************************************/
static int rec_goto_password_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_password_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        printf("visit size %d",sizeof(user_visit));
        power_flag = UNLOCK;
        lock_on = 1;
        sys_timer_del(tim_handle);
        enc_back_flag = 0;
        ui_hide(ENC_LAY_BACK);
//        ui_hide(ENC_LAY_BACK_PIC);
        ui_show(ENC_PASSWORD_LAY);
        ui_show(ENC_UP_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        //uart_recv_retransmit(flag);
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(REC_PASSWORD_BTN)
.ontouch = rec_goto_password_page_ontouch,
};

/***************************** 退出密码界面返回壁纸界面按钮 ************************************/
static int rec_goto_back_page_ontouch(void *ctr, struct element_touch_event *e)
{
    user_infor read_buf = {0};
    UI_ONTOUCH_DEBUG("**rec_goto_back_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        power_flag = UNLOCK;
        lock_on = 1;
        ui_hide(ENC_PASSWORD_LAY);
        ui_show(ENC_LAY_BACK_PIC);
        ui_show(ENC_LAY_BACK);

        ui_show(ENC_LAY_HOME_PAGE);

        ui_show(ENC_UP_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {exit_admin_mode};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_RETURN_BTN)
.ontouch = rec_goto_back_page_ontouch,
};

/***************************** 密码界面进入设置界面按钮 ************************************/
static int rec_goto_set_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
#if 1
        if(user_count && power_key_num > 0){
            power_flag = ADMIN;
            ui_text_show_index_by_id(NEC_UNLOCK_POWER_TXT,1);
        }else{
            ui_hide(ENC_PASSWORD_LAY);
//            ui_show(ENC_LAY_BACK_PIC);
            ui_show(ENC_LAY_PAGE);
        }
#else
        ui_hide(ENC_PASSWORD_LAY);
        ui_show(ENC_LAY_PAGE);
#endif
        u8 command_buf = voice;
        u8 data_buf[] = {input_admin_infor};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN_SET)
.ontouch = rec_goto_set_page_ontouch,
};

/***************************** 设置界面返回密码界面 ************************************/
static int rec_set_list_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->language = index_of_table8(db_select("lag"), TABLE(table_rec_language));
        ui_text_show_index_by_id(SET_LANG_TXT,__this->language);

        __this->volume_lv = index_of_table8(db_select("kvo"), TABLE(table_key_voice));
        ui_text_show_index_by_id(SET_SOUND_TXT,__this->volume_lv);

        printf("=================== back :%d\n",db_select("back"));
        ui_text_show_index_by_id(SET_PAPER_TXT,db_select("back"));

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_LIST_LAY)
.onchange = rec_set_list_lay_onchange,
};

/***************************** 设置界面返回密码界面按钮 ************************************/
static int rec_set_goto_paw_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_goto_paw_page_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(paper_lay_flag == 0){                        //系统设置界面返回密码界面
            ui_hide(ENC_SET_LAY);
            ui_show(ENC_LAY_PAGE);
        }else{                                         //壁纸设置界面返回系统设置界面
            ui_hide(ENC_PAPER_LIST_LAY);
            ui_show(ENC_SET_LIST_LAY);
            ui_text_show_index_by_id(ENC_SET_TXT,0);
            ui_hide(ENC_PAPER_SET_PIC);
        }
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ENC_SET_RETURN)
.ontouch = rec_set_goto_paw_page_ontouch,
};


/***************************** 系统设置界面 ************************************/
static int rec_system_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_LAY)
.onchange = rec_system_set_onchange,
};



/***************************** 设置界面 日期设置按钮 ************************************/
static int rec_goto_set_date_ontouch(void *ctr, struct element_touch_event *e)
{
    struct sys_time sys_time;
    UI_ONTOUCH_DEBUG("**rec_goto_set_date_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_show(SET_DATE_LAY);

        printf("year %d : month %d : day %d\n",temp_date_time.year,temp_date_time.month,temp_date_time.day);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_DATE_BTN)
.ontouch = rec_goto_set_date_ontouch,
};

/***************************** 设置界面 时间设置按钮 ************************************/
static int rec_goto_set_time_ontouch(void *ctr, struct element_touch_event *e)
{
    struct sys_time sys_time;
    UI_ONTOUCH_DEBUG("**rec_goto_set_time_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_show(SET_TIME_LAY);

        printf("hour %d : min %d : sec %d\n",temp_date_time.hour,temp_date_time.min,temp_date_time.sec);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_TIME_BTN)
.ontouch = rec_goto_set_time_ontouch,
};


/***************************** 日期设置界面 ************************************/
static int rec_set_date_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        temp_date_time.year = sys_time.year;
        temp_date_time.month = sys_time.month;
        temp_date_time.day = sys_time.day;
        printf("date lay year %d : month %d : day %d\n",temp_date_time.year,temp_date_time.month,temp_date_time.day);
        ui_highlight_element_by_id(SET_DATE_HIGH_LIGHT);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_LAY)
.onchange = rec_set_date_onchange,
};

/***************************** 时间设置界面 ************************************/
static int rec_set_time_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        get_system_time(&sys_time);
        temp_date_time.hour = sys_time.hour;
        temp_date_time.min = sys_time.min;
        temp_date_time.sec = sys_time.sec;
        printf("hour %d : min %d : sec %d\n",temp_date_time.hour,temp_date_time.min,temp_date_time.sec);
        ui_highlight_element_by_id(SET_TIME_HIGH_LIGHT);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_TIME_LAY)
.onchange = rec_set_time_onchange,
};


/***************************** 显示当前日期 ************************************/
static int sys_show_cur_date_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    get_system_time(&sys_time);
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        break;
    case ON_CHANGE_FIRST_SHOW:
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_CURRENT)
.onchange = sys_show_cur_date_onchange,
};


/***************************** 显示当前时间 ************************************/
static int sys_show_cur_time_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    get_system_time(&sys_time);
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_FIRST_SHOW:
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_TIME_CURRENT)
.onchange = sys_show_cur_time_onchange,
};


/***************************** 判断是否闰年 ************************************/
int isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/***************************** 获取月份的天数 ************************************/
int getMonthDays(int year, int month)
{
    switch (month)
    {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return isLeapYear(year) ? 29 : 28;
        default:
            return -1;
    }
}


/*****************************设置日期控件动作 ************************************/
static int sys_set_cur_date_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    get_system_time(&sys_time);
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        time->year = temp_date_time.year;
        time->month = temp_date_time.month;
        time->day = temp_date_time.day;
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("_onchange 111 year %d\n",temp_date_time.year);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_YEAR_CUR)
.onchange = sys_set_cur_date_onchange,
};
REGISTER_UI_EVENT_HANDLER(SET_DATE_MONTH_CUR)
.onchange = sys_set_cur_date_onchange,
};
REGISTER_UI_EVENT_HANDLER(SET_DATE_DAY_CUR)
.onchange = sys_set_cur_date_onchange,
};

/*****************************设置时间控件动作 ************************************/
static int sys_set_cur_time_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    get_system_time(&sys_time);
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        time->hour = temp_date_time.hour;
        time->min = temp_date_time.min;
        time->sec = temp_date_time.sec;
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("_onchange 111 hour %d\n",temp_date_time.hour);
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;

        temp_date_time.year = sys_time.year;
        temp_date_time.month = sys_time.month;
        temp_date_time.day = sys_time.day;

        temp_date_time.hour = time->hour;
        temp_date_time.min = time->min;
        temp_date_time.sec = time->sec;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_TIME_HOUR_CUR)
.onchange = sys_set_cur_time_onchange,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_MIN_CUR)
.onchange = sys_set_cur_time_onchange,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_SEC_CUR)
.onchange = sys_set_cur_time_onchange,
};


/*****************************设置下一个日期按键，等待确认 ************************************/
static int sys_date_next_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct button *btn = (struct button *)ctr;
    UI_ONTOUCH_DEBUG("**sys_date_next_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        time->year = temp_date_time.year;
        time->month = temp_date_time.month;
        time->day = temp_date_time.day;
        switch(btn->elm.id){
        case SET_DATE_NEXT_YEAR:
            temp_date_time.year += 1;
            if(temp_date_time.year > 2099)
            {
                temp_date_time.year = 2024;
            }
            break;
        case SET_DATE_NEXT_MONTH:
            temp_date_time.month += 1;
            if(temp_date_time.month > 12)
            {
                temp_date_time.month = 1;
            }
            break;
        case SET_DATE_NEXT_DAY:
            temp_date_time.day += 1;
            if(temp_date_time.day > getMonthDays(temp_date_time.year,temp_date_time.month))
            {
                temp_date_time.day = 1;
            }
            break;
        default:
            return false;
        }
        printf("setting...%d/%d/%d\n",temp_date_time.year,temp_date_time.month,temp_date_time.day);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_DATE_NEXT_YEAR)
.ontouch = sys_date_next_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_DATE_NEXT_MONTH)
.ontouch = sys_date_next_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_DATE_NEXT_DAY)
.ontouch = sys_date_next_ontouch,
};



/*****************************设置上一个日期按键，等待确认 ************************************/
static int sys_date_pre_ontouch(void *ctr, struct element_touch_event *e)
{
    struct button *btn = (struct button *)ctr;
    UI_ONTOUCH_DEBUG("**sys_date_temp_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch(btn->elm.id){
        case SET_DATE_PRE_YEAR:
            temp_date_time.year -= 1;
            if(temp_date_time.year < 2024)
            {
                temp_date_time.year = 2099;
            }
            break;
        case SET_DATE_PRE_MONTH:
            temp_date_time.month -= 1;
            if(temp_date_time.month < 1)
            {
                temp_date_time.month = 12;
            }
            break;
        case SET_DATE_PRE_DAY:
            temp_date_time.day -= 1;
            if(temp_date_time.day < 1)
            {
                temp_date_time.day = getMonthDays(temp_date_time.year,temp_date_time.month);
            }
            break;
        default:
            return false;
        }
        printf("setting...%d/%d/%d\n",temp_date_time.year,temp_date_time.month,temp_date_time.day);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_DATE_PRE_YEAR)
.ontouch = sys_date_pre_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_DATE_PRE_MONTH)
.ontouch = sys_date_pre_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_DATE_PRE_DAY)
.ontouch = sys_date_pre_ontouch,
};


/*****************************设置时间按键，等待确认 ************************************/
static int sys_time_next_ontouch(void *ctr, struct element_touch_event *e)
{
    struct button *btn = (struct button *)ctr;
    UI_ONTOUCH_DEBUG("**sys_time_next_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch(btn->elm.id){
            case SET_TIME_NEXT_HOUR:
                temp_date_time.hour += 1;
                if(temp_date_time.hour > 23)
                {
                    temp_date_time.hour = 0;
                }
                break;
            case SET_TIME_NEXT_MIN:
                temp_date_time.min += 1;
                if(temp_date_time.min > 59)
                {
                    temp_date_time.min = 0;
                }
                break;
            case SET_TIME_NEXT_SEC:
                temp_date_time.sec += 1;
                if(temp_date_time.sec > 59)
                {
                    temp_date_time.sec = 0;
                }
                break;
            default:
                return false;
        }
        printf("setting...%d/%d/%d\n",temp_date_time.hour,temp_date_time.min,temp_date_time.sec);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_TIME_NEXT_HOUR)
.ontouch = sys_time_next_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_NEXT_MIN)
.ontouch = sys_time_next_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_NEXT_SEC)
.ontouch = sys_time_next_ontouch,
};


/*****************************设置时间按键，等待确认 ************************************/
static int sys_time_pre_ontouch(void *ctr, struct element_touch_event *e)
{
    struct button *btn = (struct button *)ctr;
    UI_ONTOUCH_DEBUG("**sys_time_pre_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch(btn->elm.id){
            case SET_TIME_PRE_HOUR:
                temp_date_time.hour -= 1;
                if(temp_date_time.hour > 23)
                {
                    temp_date_time.hour = 23;
                }
                break;
            case SET_TIME_PRE_MIN:
                temp_date_time.min -= 1;
                if(temp_date_time.min > 59)
                {
                    temp_date_time.min = 59;
                }
                break;
            case SET_TIME_PRE_SEC:
                temp_date_time.sec -= 1;
                if(temp_date_time.sec > 59)
                {
                    temp_date_time.sec = 59;
                }
                break;
            default:
                return false;
        }
        printf("setting...%d/%d/%d\n",temp_date_time.hour,temp_date_time.min,temp_date_time.sec);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_TIME_PRE_HOUR)
.ontouch = sys_time_pre_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_PRE_MIN)
.ontouch = sys_time_pre_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_PRE_SEC)
.ontouch = sys_time_pre_ontouch,
};


/*****************************日期确认按钮 ************************************/
static int sys_date_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys_date_confirm_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        printf("confirm date  %d/%d/%d\n",temp_date_time.year,temp_date_time.month,temp_date_time.day);
        set_system_time(&temp_date_time);

        ui_hide(SET_DATE_LAY);
        ui_show(ENC_SET_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_DATE_COMFIRM_BTN)
.ontouch = sys_date_confirm_ontouch,
};

/***************************** 时间确认按钮 ************************************/
static int sys_time_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys_time_confirm_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        set_system_time(&temp_date_time);
        ui_hide(SET_TIME_LAY);
        ui_show(ENC_SET_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_TIME_COMFIRM_BTN)
.ontouch = sys_time_confirm_ontouch,
};


/***************************** 设置界面 语言设置按钮 ************************************/
static int rec_goto_set_lang_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_lang_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_show(SET_LANG_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_LANG_BTN)
.ontouch = rec_goto_set_lang_ontouch,
};

/***************************** 语言设置界面 ************************************/
const static int lang_btn_id[] = {
        SET_BTN_LANG_1,
        SET_BTN_LANG_2,
};
const static int lang_pic_id[] = {
        SET_PIC_LANG_1,
        SET_PIC_LANG_2,
};
static int rec_lang_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->language = index_of_table8(db_select("lag"), TABLE(table_rec_language));
        ui_highlight_element_by_id(lang_pic_id[__this->language]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_LANG_LAY)
.onchange = rec_lang_lay_onchange,
};

static void menu_rec_language_set(int sel_item)
{
    struct intent it;

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "lag";
    it.exdata = table_rec_language[sel_item];
    start_app(&it);
    __this->language = sel_item;
}

static int rec_language_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_language_ontouch**");
    struct intent it;
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;

    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        j=sizeof(lang_pic_id)/sizeof(lang_pic_id[0]);
        for (i = 0; i < j; i++) {
            ui_no_highlight_element_by_id(lang_pic_id[i]);
        }
        j=sizeof(lang_btn_id)/sizeof(lang_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == lang_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        ui_highlight_element_by_id(lang_pic_id[sel_item]);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        j=sizeof(lang_btn_id)/sizeof(lang_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == lang_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        menu_rec_language_set(sel_item);
        ui_hide(ENC_SET_LAY);
        ui_show(ENC_SET_LAY);
        ui_hide(SET_TEXT_LANG_LAY);
        ui_show(SET_TEXT_LANG_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_BTN_LANG_1)
.ontouch = rec_language_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_BTN_LANG_2)
.ontouch = rec_language_ontouch,
};

/***************************** 设置界面 音量设置按钮 ************************************/
static int rec_goto_set_vol_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_vol_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_show(SET_VOLUME_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_SOUND_BTN)
.ontouch = rec_goto_set_vol_ontouch,
};

/***************************** 音量设置界面 ************************************/
const static int volume_btn_id[] = {
        SET_BTN_VOL_1,
        SET_BTN_VOL_2,
        SET_BTN_VOL_3,
};
const static int volume_pic_id[] = {
        SET_PIC_VOL_1,
        SET_PIC_VOL_2,
        SET_PIC_VOL_3,
};
static int rec_volume_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->volume_lv = index_of_table8(db_select("kvo"), TABLE(table_key_voice));
        ui_highlight_element_by_id(volume_pic_id[__this->volume_lv]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_VOLUME_LAY)
.onchange = rec_volume_lay_onchange,
};

static void menu_rec_volume_set(int sel_item)
{
    struct intent it;

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "kvo";
    it.exdata = table_key_voice[sel_item];
    start_app(&it);
    __this->volume_lv = sel_item;
}

static int rec_volume_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_volume_ontouch**");
    struct intent it;
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;

    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        j=sizeof(volume_pic_id)/sizeof(volume_pic_id[0]);
        for (i = 0; i < j; i++) {
            ui_no_highlight_element_by_id(volume_pic_id[i]);
        }
        j=sizeof(volume_btn_id)/sizeof(volume_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == volume_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        ui_highlight_element_by_id(volume_pic_id[sel_item]);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        j=sizeof(volume_btn_id)/sizeof(volume_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == volume_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        menu_rec_volume_set(sel_item);
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_BTN_VOL_1)
.ontouch = rec_volume_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_BTN_VOL_2)
.ontouch = rec_volume_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_BTN_VOL_3)
.ontouch = rec_volume_ontouch,
};


/***************************** 设置界面 二级设置界面-关闭按钮 ************************************/
const static int menu_off_btn_id[] = {
        SET_LANG_OFF_BTN,
        SET_VOL_OFF_BTN,
        SET_DATE_CANCEL_BTN,
        SET_TIME_CANCEL_BTN,
};
static int rec_set_two_menu_off_ontouch(void *ctr, struct element_touch_event *e)
{
    struct button *btn = (struct button *)ctr;
    struct sys_time sys_time;
    get_system_time(&sys_time);
    u8 i,j;
    int sel_item = 0;
    UI_ONTOUCH_DEBUG("**rec_set_two_menu_off_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        j=sizeof(menu_off_btn_id)/sizeof(menu_off_btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == menu_off_btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        printf("============== menu off id :%d\n",sel_item);
        switch(sel_item){
        case 0:
            __this->language = index_of_table8(db_select("lag"), TABLE(table_rec_language));
            ui_hide(SET_LANG_LAY);
            ui_text_show_index_by_id(SET_LANG_TXT,__this->language);
            break;
        case 1:
            __this->volume_lv = index_of_table8(db_select("kvo"), TABLE(table_key_voice));
            ui_hide(SET_VOLUME_LAY);
            ui_text_show_index_by_id(SET_SOUND_TXT,__this->volume_lv);
            break;
        case 2:
            temp_date_time.year = sys_time.year;
            temp_date_time.month = sys_time.month;
            temp_date_time.day = sys_time.day;
            ui_hide(SET_DATE_LAY);
            break;
        case 3:
            temp_date_time.hour = sys_time.hour;
            temp_date_time.min = sys_time.min;
            temp_date_time.sec = sys_time.sec;
            ui_hide(SET_TIME_LAY);
            break;
        default:
            break;
        }
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_LANG_OFF_BTN)
.ontouch = rec_set_two_menu_off_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_VOL_OFF_BTN)
.ontouch = rec_set_two_menu_off_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_DATE_CANCEL_BTN)
.ontouch = rec_set_two_menu_off_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_TIME_CANCEL_BTN)
.ontouch = rec_set_two_menu_off_ontouch,
};



/***************************** 设置界面 壁纸设置按钮 ************************************/
static int rec_goto_set_paper_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_set_paper_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(ENC_SET_LIST_LAY);
        ui_show(ENC_PAPER_LIST_LAY);
        ui_text_show_index_by_id(ENC_SET_TXT,1);
        ui_show(ENC_PAPER_SET_PIC);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SET_PAPER_BTN)
.ontouch = rec_goto_set_paper_ontouch,
};

/***************************** 设置界面 壁纸设置界面 ************************************/
const static int REC_SET_PAPER_PIC[4] = {
    SET_PAPER_PIC_1,
    SET_PAPER_PIC_2,
    SET_PAPER_PIC_3,
    SET_PAPER_PIC_4,
};
static int rec_paper_list_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;

    switch (e) {
    case ON_CHANGE_INIT:
        paper_lay_flag = 1;
        break;
    case ON_CHANGE_RELEASE:
        paper_lay_flag = 0;
        break;
    case ON_CHANGE_FIRST_SHOW:

        ui_pic_show_image_by_id(REC_SET_PAPER_PIC[db_select("back")],1);        //显示选中选项

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAPER_LIST_LAY)
.onchange = rec_paper_list_lay_onchange,
};

/***************************** 设置界面 壁纸选择按钮 ************************************/
static int rec_set_paper_ui_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_paper_ui_ontouch**");
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;
    const int btn_id[] = {
        SET_PAPER_1,
        SET_PAPER_2,
        SET_PAPER_3,
        SET_PAPER_4
    };
    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        j=sizeof(btn_id)/sizeof(btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == btn_id[i]) {
                sel_item = i;
                break;
            }
        }

        ui_pic_show_image_by_id(REC_SET_PAPER_PIC[db_select("back")],0);        //隐藏选中选项

        db_update("back",sel_item);
        db_flush();
        printf("================= paper num:%d\n",db_select("back"));
        ui_pic_show_image_by_id(REC_SET_PAPER_PIC[db_select("back")],1);        //显示选中选项
        ui_pic_show_image_by_id(ENC_PAPER_SET_PIC,sel_item);
        ui_pic_show_image_by_id(ENC_LAY_BACK_PIC,sel_item);
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_PAPER_1)
.ontouch = rec_set_paper_ui_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_PAPER_2)
.ontouch = rec_set_paper_ui_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_PAPER_3)
.ontouch = rec_set_paper_ui_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SET_PAPER_4)
.ontouch = rec_set_paper_ui_ontouch,
};

/***************************** 开机主界面 和 设置界面 壁纸图片UI ************************************/
static int rec_paper_set_pic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic,db_select("back"));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAPER_SET_PIC)
.onchange = rec_paper_set_pic_onchange,
};
REGISTER_UI_EVENT_HANDLER(PIC_BACK_REC)
.onchange = rec_paper_set_pic_onchange,
};
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BACK_PIC)
.onchange = rec_paper_set_pic_onchange,
};
REGISTER_UI_EVENT_HANDLER(ENC_DEVICE_STATUS_PIC)
.onchange = rec_paper_set_pic_onchange,
};

/***************************** 密码界面 密码输入按钮 ************************************/
#define  MAX_PAW_NUM  15           //密码最大个数
#define  MIN_PAW_NUM  6            //密码最小个数

u8 password_num = 0;        //输入的密码个数
u8 password_code[MAX_PAW_NUM] = {0};       //保存输入的密码
u8 asterisk_number[MAX_PAW_NUM] = {'*','*','*','*','*','*','*','*','*','*','*','*','*','*','*'};      //显示*号
static u8 input_key_flag = 0;
static int rec_password_in_ontouch(void *ctr, struct element_touch_event *e)
{
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;
    const int btn_id[] = {
        PWD_NUM_BTN0,
        PWD_NUM_BTN1,
        PWD_NUM_BTN2,
        PWD_NUM_BTN3,
        PWD_NUM_BTN4,
        PWD_NUM_BTN5,
        PWD_NUM_BTN6,
        PWD_NUM_BTN7,
        PWD_NUM_BTN8,
        PWD_NUM_BTN9
    };
    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(password_num == MAX_PAW_NUM){
            printf("======================= pwd num max\n");
            break;
        }

        j=sizeof(btn_id)/sizeof(btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        password_code[password_num] = sel_item;
        printf("============ in pwd:");
        put_buf(password_code,MAX_PAW_NUM);             //输出当前输入的密码
		password_num++;
        ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", &asterisk_number[MAX_PAW_NUM-password_num]);
//        if(password_num)
//        {
//            ui_text_show_index_by_id(ENC_PASSWORD_SYMBOL_TXT,password_num-1);
//        }

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN0)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN1)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN2)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN3)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN4)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN5)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN6)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN7)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN8)
.ontouch = rec_password_in_ontouch,
};
REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN9)
.ontouch = rec_password_in_ontouch,
};

/***************************** 密码界面 密码删除按钮 ************************************/
static int rec_password_del_ontouch(void *ctr, struct element_touch_event *e)
{
    user_infor read_buf = {0};
    UI_ONTOUCH_DEBUG("**rec_password_del_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(password_num == 0){
            ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", " ");
            break;
        }else if(password_num == 1){
            ui_hide(ENC_PASSWORD_SYMBOL_TXT);
        }
        password_num--;
        password_code[password_num] = 'a';
        printf("============== del pwd:");
        put_buf(password_code,MAX_PAW_NUM);             //输出当前输入的密码
        ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", &asterisk_number[MAX_PAW_NUM-password_num]);
//        ui_text_show_index_by_id(ENC_PASSWORD_SYMBOL_TXT,password_num-1);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_DEL_KEY)
.ontouch = rec_password_del_ontouch,
};

void hide_show_page()
{
    ui_hide(ANI_UNLOCK_LAYER);
    ui_show(ENC_LAY_BACK);
    ui_show(ENC_LAY_HOME_PAGE);
    ani_flag = 0;
}

/***************************** 开锁动画处理 ************************************/
static int unlock_animation_handler(void *_ani, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_ANIMATION_END:
        printf("2222 play END\n");
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ANI_UNLOCK_PLAY)
.onchange = unlock_animation_handler,
};


/***************************** 密码界面 密码确认按钮 ************************************/
static int rec_password_ok_ontouch(void *ctr, struct element_touch_event *e)
{
    u8 pw[MAX_PAW_NUM+1] = {0};
    u8 i,j;
    user_infor *pw_code = malloc(sizeof(user_infor));
    user_visit visit_record;
    u8 data_buf = 0;
    struct intent it;
    if(pw_code)
    {
        memset(pw_code,0,sizeof(user_infor));
    }
    struct sys_time sys_time;
    UI_ONTOUCH_DEBUG("**rec_password_ok_ontouch**");
    int tmp = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(password_num < MIN_PAW_NUM){
            printf("======================= pwd num min\n");
            break;
        }

        if(memcmp(erase_password_array,password_code,10) == 0)//清除全部flash密码
        {
            erase_flash(0xAA);
            memset(password_code,'a',MAX_PAW_NUM);          //显示时赋值为a，表示为空
            password_num = 0;
            ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", " ");
            ui_hide(ENC_PASSWORD_LAY);
//            ui_show(ENC_LAY_BACK_PIC);
            ui_show(ENC_LAY_BACK);
            ui_show(ENC_LAY_HOME_PAGE);
        }

        put_buf(password_code,MAX_PAW_NUM);             //输出输入密码
        pw_code = match_user_data(password_code, sizeof(password_code), sizeof(user_infor));
        if(pw_code != NULL)
        {
            get_system_time(&sys_time);
            memcpy(visit_record.name,pw_code->name,sizeof(visit_record.name));
            visit_record.record_time = sys_time;
            visit_record.unlock.unlock_mode = PASSWORD;

            if(power_flag)//以管理员身份进入
            {
                if(pw_code->user_power)//该用户有管理员权限
                {
                    data_buf = enter_admin_mode;
                    visit_record.unlock.unlock_power = ADMIN;
                }
                else
                {
                    printf("NO ADMIN POWER\n");
                    memset(password_code,'a',MAX_PAW_NUM);          //显示时赋值为a，表示为空
                    password_num = 0;
                    ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", " ");
                    return false;
                }
                //power_set_flag = 1;//进入权限设置界面
                page_pic_flag = 0;
                ui_hide(ENC_PASSWORD_LAY);
//                ui_show(ENC_LAY_BACK_PIC);
                ui_show(ENC_LAY_PAGE);
            }else{
                ani_flag = 1;
                data_buf = unlocked;
                ui_hide(ENC_PASSWORD_LAY);
//                ui_show(ENC_LAY_BACK_PIC);
                ui_show(ANI_UNLOCK_LAYER);//动画
                ui_show(ENC_UP_LAY);
                sys_timeout_add(NULL,hide_show_page,3000);
            }
            if (pw_code->visit_count < MAX_RECORD_NUM) {
                pw_code->visit_count++;
            } else {
                printf("MAX record num\n");
                return false;
            }

            write_data_to_flash(&visit_record,1);//写记录
            put_buf(&visit_record, sizeof(visit_record));

        }
        else
        {
            printf("password error\n");
        }
        memset(password_code,'a',MAX_PAW_NUM);          //显示时赋值为a，表示为空
        password_num = 0;
        ui_text_set_str_by_id(ENC_PASSWORD_TXT, "ascii", " ");
        if(pw_code)
        {
            free(pw_code);
            pw_code = NULL;
        }
        u8 command_buf = voice;
        uart_send_package(command_buf,&data_buf,sizeof(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PWD_NUM_BTN_OK)
.ontouch = rec_password_ok_ontouch,
};

/***************************** 密码输入界面 ************************************/
static int rec_password_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(NEC_UNLOCK_POWER_TXT,0);
        memset(password_code,'a',MAX_PAW_NUM);          //显示时赋值为a，表示为空
        password_num = 0;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PASSWORD_LAY)
.onchange = rec_password_lay_onchange,
};


/***************************** 设置主界面 ************************************/
static int rec_lay_set_pic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic,page_pic_flag);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_SET_PIC)
.onchange = rec_lay_set_pic_onchange,
};

/***************************** 设置主界面文字 ************************************/
static int rec_lay_set_txt1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT1,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT2,page_pic_flag);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_TXT1_PAGE)
.onchange = rec_lay_set_txt1_onchange,
};


static int rec_lay_set_txt2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT3,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT4,page_pic_flag);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_TXT2_PAGE)
.onchange = rec_lay_set_txt2_onchange,
};

/***************************** 主界面切换右按钮 ************************************/
static int rec_page_right_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_page_right_ontouch**");
    struct button *btn = (struct button *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        page_pic_flag = 1;
        ui_hide(ENC_PAGE_RIGHT_BTN);
//        ui_show(ENC_PAGE_LEFT_BTN);
        ui_hide(ENC_LAY_PAGE);
        ui_show(ENC_LAY_PAGE);
        /*
        ui_hide(ENC_LAY_TXT2_PAGE);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT1,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT2,page_pic_flag);
        ui_pic_show_image_by_id(ENC_LAY_SET_PIC,1);
        */
        ui_show(ENC_UP_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAGE_RIGHT_BTN)
.ontouch = rec_page_right_ontouch,
};

/***************************** 主界面切换左按钮 ************************************/
static int rec_page_left_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_page_left_ontouch**");
    struct button *btn = (struct button *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        page_pic_flag = 0;
        ui_hide(ENC_PAGE_LEFT_BTN);
//        ui_show(ENC_PAGE_RIGHT_BTN);
        ui_hide(ENC_LAY_PAGE);
        ui_show(ENC_LAY_PAGE);
        /*
        ui_show(ENC_LAY_TXT2_PAGE);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT3,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT4,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT1,page_pic_flag);
        ui_text_show_index_by_id(ENC_LAY_PAGE_TXT2,page_pic_flag);
        ui_pic_show_image_by_id(ENC_LAY_SET_PIC,0);
        */
//        ui_show(ENC_UP_LAY);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAGE_LEFT_BTN)
.ontouch = rec_page_left_ontouch,
};




/*****************************该段代码用于结构体按时间先后的重构排序************************************/
// 辅助结构体：用于存储每个访问记录及其对应的用户信息
typedef struct {
    struct sys_time visit_time;
    user_infor user_info;
} visit_with_user_info;

// 比较两个时间是否相等的函数
int is_time_equal(const struct sys_time *time1, const struct sys_time *time2) {
    return (time1->year == time2->year &&
            time1->month == time2->month &&
            time1->day == time2->day &&
            time1->hour == time2->hour &&
            time1->min == time2->min &&
            time1->sec == time2->sec);
}


// 比较两个时间的函数，用于排序
int compare_time(const void *a, const void *b) {
    visit_with_user_info *visit1 = (visit_with_user_info *)a;
    visit_with_user_info *visit2 = (visit_with_user_info *)b;

    struct sys_time *time1 = &visit1->visit_time;
    struct sys_time *time2 = &visit2->visit_time;

    if (time1->year != time2->year) return time2->year - time1->year;
    if (time1->month != time2->month) return time2->month - time1->month;
    if (time1->day != time2->day) return time2->day - time1->day;
    if (time1->hour != time2->hour) return time2->hour - time1->hour;
    if (time1->min != time2->min) return time2->min - time1->min;
    return time2->sec - time1->sec;
}
/*
void sort_all_visits_by_time(struct sys_time *time) {
    // 1. 统计指定日期的访问记录数
    int total_visits = 0;
    for (int i = 0; i < MAX_USER_NUM; i++) {
        for (int j = 0; j < user_global_visit[i].records.visit_count; j++) {
            struct sys_time visit_time = user_global_visit[i].records.visit[j].record_time;
            if (visit_time.year == time->year &&
                visit_time.month == time->month &&
                visit_time.day == time->day) {
                total_visits++;
            }
        }
    }

    // 如果没有符合条件的访问记录，直接返回
    if (total_visits == 0) {
        printf("No visits found for the specified date!\n");
        return;
    }

    // 2. 创建辅助数组来存储所有符合条件的访问记录和对应的用户信息
    visit_with_user_info *visit_array = malloc(total_visits * sizeof(visit_with_user_info));
    if (visit_array == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    int index = 0;
    for (int i = 0; i < MAX_USER_NUM; i++) {
        for (int j = 0; j < user_global_visit[i].records.visit_count; j++) {
            struct sys_time visit_time = user_global_visit[i].records.visit[j].record_time;
            if (visit_time.year == time->year &&
                visit_time.month == time->month &&
                visit_time.day == time->day) {
                visit_array[index].visit_time = visit_time;
                visit_array[index].user_info = user_global_visit[i].records;  // 复制用户数据
                index++;
            }
        }
    }

    // 3. 对访问记录进行排序，使用 qsort 提升性能
    qsort(visit_array, total_visits, sizeof(visit_with_user_info), compare_time);

    // 4. 重建 user_global_visit，只保留排序后的访问记录
    int global_idx = 0;
    for (int i = 0; i < MAX_USER_NUM; i++) {
        user_global_visit[i].records.visit_count = 0; // 重置访问记录计数
    }

    for (int i = 0; i < total_visits; i++) {
        // 跳过重复时间的记录
        if (i > 0 && is_time_equal(&visit_array[i].visit_time, &visit_array[i - 1].visit_time) &&
            visit_array[i].user_info.user_num == visit_array[i - 1].user_info.user_num) {
            continue;
        }

        // 分配到 global_visit
        user_infor *user = &visit_array[i].user_info;
        user_global_visit[global_idx].records = *user; // 复制用户信息
        user_global_visit[global_idx].records.visit[0].record_time = visit_array[i].visit_time; // 更新访问时间
        user_global_visit[global_idx].records.visit_count = 1; // 只保留一条记录
        global_idx++;
    }

    // 5. 打印排序后的结果
    for (int i = 0; i < global_idx; i++) {
        struct sys_time t = user_global_visit[i].records.visit[0].record_time;
        printf("User: %s\n", user_global_visit[i].records.name);
        printf("  Visit Time: %d-%02d-%02d %02d:%02d:%02d\n",
               t.year, t.month, t.day, t.hour, t.min, t.sec);
    }

    free(visit_array);
    visit_array = NULL;
}
*/


/*****************************该段代码用于结构体按时间先后的重构排序************************************/

//获取月份的最后一天
int get_last_day_of_month(int year, int month)
{
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // 闰年处理
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
        return 29;
    }

    return days_in_month[month - 1];
}

void reverse_array(user_visit *arr, int length) {
    user_visit temp;
    for (int i = 0; i < length / 2; i++) {
        // 交换 arr[i] 和 arr[length - 1 - i]
        temp = arr[i];
        arr[i] = arr[length - 1 - i];
        arr[length - 1 - i] = temp;
    }
}

void read_lock_configuration(void)
{
    lock_array[0] = db_select("ver");
    lock_array[1] = db_select("pol");
    lock_array[2] = db_select("body");
    lock_array[3] = db_select("open");
    lock_array[4] = db_select("lrs");
    lock_array[5] = db_select("pop");
    lock_array[6] = db_select("dblock");
    lock_array[7] = db_select("lockv");
    for(int i = 0; i < 8; i++)
    {
        printf("lock_array %d = %d\n",i,lock_array[i]);
    }
}


const int lay_user_list[]={
    ENC_LAY_USER_NAME_1,
    ENC_LAY_USER_NAME_2,
    ENC_LAY_USER_NAME_3,
    ENC_LAY_USER_NAME_4,
    ENC_LAY_USER_NAME_5,
};
const int lay_user_name_list[]={
    ENC_USER_NAME_TXT_1,
    ENC_USER_NAME_TXT_2,
    ENC_USER_NAME_TXT_3,
    ENC_USER_NAME_TXT_4,
    ENC_USER_NAME_TXT_5,
};

void icon_page_turning(int mov)
{
    if(mov > 5 && page_pic_flag) {
        page_pic_flag = 0;
        ui_hide(ENC_LAY_PAGE);
        ui_show(ENC_LAY_PAGE);
        ui_show(ENC_LAY_BTN_3);
        ui_show(ENC_LAY_BTN_4);
        ui_hide(ENC_PAGE_LEFT_BTN);
        ui_show(ENC_PAGE_RIGHT_BTN);
    } else if(mov < -5 && !page_pic_flag) {
        page_pic_flag = 1;
        ui_hide(ENC_LAY_PAGE);
        ui_show(ENC_LAY_PAGE);
        ui_hide(ENC_LAY_BTN_3);
        ui_hide(ENC_LAY_BTN_4);
        ui_hide(ENC_PAGE_RIGHT_BTN);
        ui_show(ENC_PAGE_LEFT_BTN);
    }
}


/***************************** 主界面选项按钮 1 ************************************/
static int rec_LAY_BTN_1_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    static s16 x = 0;
    int pos = 0;
    static int move_flag = 0;
    move_flag++;
    if(move_flag > 3){
        move_flag = 0;
    }
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_1_ontouch**");
    get_user_infor_struct();
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        x = e->pos.x;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        icon_page_turning(move_posx(x, e->pos.x));

        break;
    case ELM_EVENT_TOUCH_UP:
        if(move_flag <= 2)
        {
            list_cur_page = 0;
            if(page_pic_flag == 0){
                ui_hide(ENC_LAY_PAGE);
                ui_show(ENC_LAY_USER_PAGE);
            }else{
                read_lock_configuration();
                ui_hide(ENC_LAY_PAGE);
                ui_show(ENC_LAY_DOOR_LOCK_PAGE);
            }


            u8 command_buf = voice;
            u8 data_buf[] = {key_sound};
            uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        } else {
            move_flag = 0;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_1)
.ontouch = rec_LAY_BTN_1_ontouch,
};

/***************************** 主界面选项按钮 2 ************************************/
static int rec_LAY_BTN_2_ontouch(void *ctr, struct element_touch_event *e)
{
    s16 x_pos_down = 0;
    int pos = 0;
    static s16 x = 0;
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_2_ontouch**");
    int record_visit = 0;
    u16 count = 0;
    static int move_flag = 0;
    move_flag++;
    if(move_flag > 3){
        move_flag = 0;
    }
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        x = e->pos.x;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        icon_page_turning(move_posx(x, e->pos.x));

        break;
    case ELM_EVENT_TOUCH_UP:
        if(move_flag <= 2)
        {
            if(page_pic_flag == 0)//设置页面第一页
            {
                record_now_page = 0;
                normal_idx = MAX_RECORD_NUM;
                //read_data_from_flash();//将记录读取到全局结构体中
                //reverse_array(user_global_visit,visit_global_count);
                get_system_time(&global_date);

                if(copy_buffer_idx == 0){
                    copy_buffer_idx = start_pointer / RECORDS_PER_SECTOR * RECORDS_PER_SECTOR;
                }

                tim_cnt = time_idx_count-1;
                visit_global_count = match_visit_time(&global_date, user_global_visit,0,1);
                pre_page_cnt = 0;
                next_page_cnt = visit_global_count;
                printf("visit_global_count %d tim_cnt %d\n",visit_global_count,tim_cnt);
                //reverse_array(user_global_visit,visit_global_count);

                ui_hide(ENC_LAY_PAGE);
                ui_show(ENC_LAY_RECORD_PAGE);
            }else{
                ui_hide(ENC_LAY_PAGE);
                ui_show(ENC_LAY_SYS_INFO_PAGE);
            }
            u8 command_buf = voice;
            u8 data_buf[] = {key_sound};
            uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        }else {
            move_flag = 0;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_2)
.ontouch = rec_LAY_BTN_2_ontouch,
};

/***************************** 主界面选项按钮 3 ************************************/
static int rec_LAY_BTN_3_ontouch(void *ctr, struct element_touch_event *e)
{
    s16 x_pos_down = 0;
    static s16 x = 0;
    int pos = 0;
    static int move_flag = 0;
    move_flag++;
    if(move_flag > 3){
        move_flag = 0;
    }
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_3_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        x = e->pos.x;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        icon_page_turning(move_posx(x, e->pos.x));

        break;
    case ELM_EVENT_TOUCH_UP:
        if(move_flag <= 2)
        {
            ui_hide(ENC_LAY_PAGE);
            ui_show(ENC_SET_LAY);
            u8 command_buf = voice;
            u8 data_buf[] = {key_sound};
            uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        }else {
            move_flag = 0;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_3)
.ontouch = rec_LAY_BTN_3_ontouch,
};

/***************************** 主界面选项按钮 4 ************************************/
static int rec_LAY_BTN_4_ontouch(void *ctr, struct element_touch_event *e)
{
    s16 x_pos_down = 0;
    static s16 x = 0;
    int pos = 0;
    static int move_flag = 0;
    move_flag++;
    if(move_flag > 3){
        move_flag = 0;
    }
    UI_ONTOUCH_DEBUG("**rec_LAY_BTN_4_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        x = e->pos.x;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        icon_page_turning(move_posx(x, e->pos.x));

        break;
    case ELM_EVENT_TOUCH_UP:
        if(move_flag <= 2)
        {
            ui_hide(ENC_LAY_PAGE);
            ui_show(ENC_NETWORK_PAGE);
            u8 command_buf = voice;
            u8 data_buf[] = {key_sound};
            uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        }else {
            move_flag = 0;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BTN_4)
.ontouch = rec_LAY_BTN_4_ontouch,
};

/***************************** 设置界面返回密码界面按钮 ************************************/
static int rec_lay_page_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_lay_page_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        page_pic_flag = 0;
        power_flag = UNLOCK;
        ui_hide(ENC_LAY_PAGE);
        ui_show(ENC_LAY_BACK);
        ui_show(ENC_LAY_HOME_PAGE);
        ui_show(ENC_UP_LAY);
        lock_on = 1;
        u8 command_buf = voice;
        u8 data_buf[] = {exit_admin_mode};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PAGE_RETURN)
.ontouch = rec_lay_page_btn_ontouch,
};



/***************************** 用户管理界面 ************************************/
static int rec_lay_user_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u16 end_cnt = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        if(goto_facial_page_flag){
            ui_show(USER_GOTO_DELETE);
            ui_show(ENC_LAY_USER_DETAILS);
        }else{
            ui_show(ENC_LAY_USER_LIST);
        }
        break;
    case ON_CHANGE_SHOW:
        //sys_timeout_add(NULL,reset_up_ui_func,100);
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_PAGE)
.onchange = rec_lay_user_page_onchange,
};


/***************************** 用户设置界面 ************************************/
static int rec_lay_user_list_onchange(void *ctr, enum element_change_event e, void *arg)
{
    get_user_infor_struct();
    u8 list_num = 0;
    u16 end_cnt = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW_PROBE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 0;
        ui_hide(USER_GOTO_DELETE);
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        printf("page %d num %d",list_cur_page,user_count);
        if(user_count > 0)
        {
            end_cnt = (list_cur_page + 1) * 5;
            if(end_cnt > user_count)
            {
                end_cnt = user_count;
            }
            printf("end_cnt %d\n",end_cnt);
            for(int i = list_cur_page * 5; i < (list_cur_page + 1) * 5; i++){
                if(i < end_cnt){
                    user_data = match_user_num(list_valid_array[i]);
                    if(user_data != NULL)
                    {
                        printf("list current show number %d name %s\n",i,user_data->name);
                        ui_show(lay_user_list[i%5]);
                    }
                    else
                    {
                        i--;
                        printf("List not match any user\n");
                    }
                }else{
                    //ui_hide(lay_user_list[i%5]);
                }
                free(user_data);
                user_data = NULL;
            }
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_LIST)
.onchange = rec_lay_user_list_onchange,
};
static int rec_lay_user_input_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 1;
        ui_hide(USER_GOTO_DELETE);
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        memset(user_name,0,sizeof(user_name));
        user_name_num = 0;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_INPUT)
.onchange = rec_lay_user_input_onchange,
};


/***************************** 用户详情页面 ************************************/
static int rec_lay_user_details_onchange(void *ctr, enum element_change_event e, void *arg)
{
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 2;
        goto_facial_page_flag = 0;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        memset(user_name,0,sizeof(user_name));
        user_name_num = 0;

        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_data != NULL && user_data->user_num != 0xFFFF)
        {
            printf("details current chose user  name %s now_btn %d\n",user_data->name,now_btn_user + 5 * list_cur_page);
            current_user = user_data->user_num;
//            put_buf(user_data->key[0].key_buf,sizeof(user_data->key[0]));
//            put_buf(user_data->key[1].key_buf,sizeof(user_data->key[1]));
//            put_buf(user_data->key[2].key_buf,sizeof(user_data->key[2]));
//            put_buf(user_data->key[3].key_buf,sizeof(user_data->key[3]));
//            put_buf(user_data->key[4].key_buf,sizeof(user_data->key[4]));
            ui_text_set_str_by_id(ENC_NOW_USER_NAME, "ascii", user_data->name);

            ui_pic_show_image_by_id(ENC_USER_POWER_PIC,user_data->user_power);
            ui_text_show_index_by_id(ENC_USER_POWER_TXT,user_data->user_power);
        }

        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));

        break;
    case ON_CHANGE_SHOW:
        ani_flag = 0;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_DETAILS)
.onchange = rec_lay_user_details_onchange,
};

/***************************** 用户详情页面进入用户密钥页面按钮 ************************************/
static int rec_user_detail_goto_key_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_detail_goto_key_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(ENC_LAY_USER_DETAILS);
        ui_hide(USER_GOTO_DELETE);
        ui_show(ENC_LAY_USER_KEY);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USER_DETAIL_GOTO_KEY_BTN)
.ontouch = rec_user_detail_goto_key_ontouch,
};



/***************************** 用户密钥页面 ************************************/
const int lay_user_key_list[]={
    ENC_LAY_USER_KEY_1,
    ENC_LAY_USER_KEY_2,
    ENC_LAY_USER_KEY_3,
    ENC_LAY_USER_KEY_4,
    ENC_LAY_USER_KEY_5,
};


static int rec_lay_user_key_onchange(void *ctr, enum element_change_event e, void *arg)
{
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 3;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_data != NULL)
        {
            if(user_data->key_num)
            {
                for(int i = 0; i < user_data->key_num ; i++)
                {
                    ui_show(lay_user_key_list[i % 5]);
                }
            }

        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_KEY)
.onchange = rec_lay_user_key_onchange,
};

/***************************** 密钥列表显示 ************************************/
static int rec_lay_user_key_1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_data != NULL)
        {
            printf("user key_1 %s num %d\n",user_data->name,user_data->user_num);
            switch (user_data->key[0].key_mode){
                case FACE:
                    printf("KEY_1 FACE UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_1_PIC,0);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_1_TXT, 0);
                    break;
                case PASSWORD:
                    printf("KEY_1 PASSWORD UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_1_PIC,1);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_1_TXT, 1);
                    break;
                case FINGER:
                    printf("KEY_1 FINGER UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_1_PIC,2);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_1_TXT, 2);
                    break;
                case NFC:
                    printf("KEY_1 NFC UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_1_PIC,3);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_1_TXT, 3);
                    break;
            }
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }

        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_KEY_1)
.onchange = rec_lay_user_key_1_onchange,
};


static int rec_lay_user_key_2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 0;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_data != NULL)
        {
            printf("user key_2 %s num %d\n",user_data->name,user_data->user_num);
            switch (user_data->key[1].key_mode){
                case FACE:
                    printf("KEY_2 FACE UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_2_PIC,0);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_2_TXT, 0);
                    break;
                case PASSWORD:
                    printf("KEY_2 PASSWORD UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_2_PIC,1);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_2_TXT, 1);
                    break;
                case FINGER:
                    printf("KEY_2 FINGER UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_2_PIC,2);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_2_TXT, 2);
                    break;
                case NFC:
                    printf("KEY_2 NFC UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_2_PIC,3);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_2_TXT, 3);
                    break;
            }
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_KEY_2)
.onchange = rec_lay_user_key_2_onchange,
};

static int rec_lay_user_key_3_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 0;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_data != NULL)
        {
            printf("user key_3 %s num %d\n",user_data->name,user_data->user_num);
            switch (user_data->key[2].key_mode){
                case FACE:
                    printf("KEY_3 FACE UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_3_PIC,0);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_3_TXT, 0);
                    break;
                case PASSWORD:
                    printf("KEY_3 PASSWORD UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_3_PIC,1);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_3_TXT, 1);
                    break;
                case FINGER:
                    printf("KEY_3 FINGER UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_3_PIC,2);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_3_TXT, 2);
                    break;
                case NFC:
                    printf("KEY_3 NFC UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_3_PIC,3);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_3_TXT, 3);
                    break;
            }
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_KEY_3)
.onchange = rec_lay_user_key_3_onchange,
};

static int rec_lay_user_key_4_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 0;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_data != NULL)
        {
            printf("user key_4 %s num %d\n",user_data->name,user_data->user_num);
            switch (user_data->key[3].key_mode){
                case FACE:
                    printf("KEY_4 FACE UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_4_PIC,0);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_4_TXT, 0);
                    break;
                case PASSWORD:
                    printf("KEY_4 PASSWORD UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_4_PIC,1);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_4_TXT, 1);
                    break;
                case FINGER:
                    printf("KEY_4 FINGER UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_4_PIC,2);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_4_TXT, 2);
                    break;
                case NFC:
                    printf("KEY_4 NFC UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_4_PIC,3);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_4_TXT, 3);
                    break;
            }
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_KEY_4)
.onchange = rec_lay_user_key_4_onchange,
};

static int rec_lay_user_key_5_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 0;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_data != NULL)
        {
            printf("user key_5 %s num %d\n",user_data->name,user_data->user_num);
            switch (user_data->key[4].key_mode){
                case FACE:
                    printf("KEY_5 FACE UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_5_PIC,0);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_5_TXT, 0);
                    break;
                case PASSWORD:
                    printf("KEY_5 PASSWORD UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_5_PIC,1);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_5_TXT, 1);
                    break;
                case FINGER:
                    printf("KEY_5 FINGER UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_5_PIC,2);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_5_TXT, 2);
                    break;
                case NFC:
                    printf("KEY_5 NFC UNLOCK\n");
                    ui_pic_show_image_by_id(ENC_LAY_USER_KEY_5_PIC,3);
                    ui_text_show_index_by_id(ENC_LAY_USER_KEY_5_TXT, 3);
                    break;
            }
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_KEY_5)
.onchange = rec_lay_user_key_5_onchange,
};



/***************************** 用户管理界面返回按钮 ************************************/
static int rec_set_return_user_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_return_user_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(user_page_flag == 0){
            ui_hide(ENC_LAY_USER_PAGE);
            ui_show(ENC_LAY_PAGE);
        }else if(user_page_flag == 1){
            ui_hide(ENC_LAY_USER_INPUT);
            ui_show(ENC_LAY_USER_LIST);
        }else if(user_page_flag == 2){
            current_user = 0;
            ui_hide(ENC_LAY_USER_DETAILS);
            ui_show(ENC_LAY_USER_LIST);
        }else if(user_page_flag == 3){
            ui_hide(ENC_LAY_USER_KEY);
            ui_show(USER_GOTO_DELETE);
            ui_show(ENC_LAY_USER_DETAILS);
        }else if(user_page_flag == 4){
            ui_hide(ENC_ADD_NEW_FINGERPRINT);
            ui_show(USER_GOTO_DELETE);
            ui_show(ENC_LAY_USER_DETAILS);
        }else if(user_page_flag == 5){
            ui_hide(ENC_ADD_NEW_NFC);
            ui_show(USER_GOTO_DELETE);
            ui_show(ENC_LAY_USER_DETAILS);
        }else if(user_page_flag == 6 || user_page_flag == 7){
            ui_hide(ENC_USER_NEW_KEY);
            ui_show(USER_GOTO_DELETE);
            ui_show(ENC_LAY_USER_DETAILS);
        }
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_RETURN_USER)
.ontouch = rec_set_return_user_ontouch,
};


/***************************** 新建用户按钮 ************************************/
static int rec_set_new_user_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_new_user_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(ENC_LAY_USER_LIST);
        ui_show(ENC_LAY_USER_INPUT);
        u8 command_buf = voice;
        u8 data_buf[] = {input_user_name};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SET_NEW_USER_BTN)
.ontouch = rec_set_new_user_ontouch,
};

/***************************** 删除用户按钮 ************************************/
static int rec_set_delete_user_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_set_delete_user_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        //ui_hide(ENC_LAY_USER_DETAILS);
        ui_show(ENC_USER_DELETE_PAGE);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USER_GOTO_DELETE_BTN)
.ontouch = rec_set_delete_user_ontouch,
};


/***************************** 删除用户页面 ************************************/
static int rec_delete_user_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_DELETE_PAGE)
.onchange = rec_delete_user_onchange,
};


/***************************** 删除用户确认按钮 ************************************/
static int rec_delete_user_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_delete_user_confirm_ontouch**");
    get_user_infor_struct();
    int i;
    u8 num1,num2;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        user_key_num -= user_data->key_num;//减小密钥数量
        if(user_data->user_power)
        {
            power_key_num -= user_data->key_num;//减小权限密钥数量
        }
        printf("chose now user idx %d user_key_num %d power_key_num %d\n",user_data->index,user_key_num,power_key_num);
        num1 = user_data->user_num >> 8;
        num2 = user_data->user_num & 0xFF;
        erase_flash(user_data->index - 1);
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        if((now_btn_user + 5 * list_cur_page) < user_count)
        {
            for(i = now_btn_user + 5 * list_cur_page; i < user_count -1; i++)
            {
                list_valid_array[i] = list_valid_array[i+1];//删除用户后，数组前移
            }
            list_valid_array[user_count - 1] = 0;//列表默认显示，删除用户后需要将最后一个列表清零
            user_count--;
            list_page_num = user_count / 5;
            /*
            if(list_page_num <= list_cur_page)
            {
                list_cur_page = list_page_num ;
            }*/
            if((user_count % 5) == 0)//用户递减后如果刚好能填满整页，列表就返回上一页，否则出现空页面情况
            {
                list_cur_page = list_page_num - 1;
            }
            printf("delete user remain num %d\n",user_count);
            if(user_count == 0)
            {
                user_key_num = 0;//删除最后一个用户后,所有密钥清零
            }
        }
        ui_hide(ENC_USER_DELETE_PAGE);
        ui_hide(ENC_LAY_USER_DETAILS);
        ui_show(ENC_LAY_USER_LIST);
        u8 command_buf = delete_user;
        u8 data_buf[] = {num1,num2};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USER_DELETE_CONFIRM_BTN)
.ontouch = rec_delete_user_confirm_ontouch,
};

/***************************** 删除用户取消按钮 ************************************/
static int rec_delete_user_cancel_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_delete_user_cancel_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("delete_user_cancel\n");
        ui_hide(ENC_USER_DELETE_PAGE);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USER_DELETE_CANCEL_BTN)
.ontouch = rec_delete_user_cancel_ontouch,
};


/***************************** 用户名列表显示 ************************************/
static int rec_lay_user_name_1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 0;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        page = list_cur_page;
        user_data = match_user_num(list_valid_array[page * 5 + list_index]);
        if(user_data != NULL)
        {
            printf("user_name_1 %s num %d\n",user_data->name,user_data->user_num);
            ui_text_set_str_by_id(ENC_USER_NAME_TXT_1, "ascii", user_data->name);
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_1)
.onchange = rec_lay_user_name_1_onchange,
};
static int rec_lay_user_name_2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 1;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        page = list_cur_page;
        user_data = match_user_num(list_valid_array[page * 5 + list_index]);
        if(user_data != NULL)
        {
            printf("user_name_2 %s num %d\n",user_data->name,user_data->user_num);
            ui_text_set_str_by_id(ENC_USER_NAME_TXT_2, "ascii", user_data->name);
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_2)
.onchange = rec_lay_user_name_2_onchange,
};
static int rec_lay_user_name_3_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 2;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        page = list_cur_page;
        user_data = match_user_num(list_valid_array[page * 5 + list_index]);
        if(user_data != NULL)
        {
            printf("user_name_3 %s num %d\n",user_data->name,user_data->user_num);
            ui_text_set_str_by_id(ENC_USER_NAME_TXT_3, "ascii", user_data->name);
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_3)
.onchange = rec_lay_user_name_3_onchange,
};
static int rec_lay_user_name_4_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 3;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        page = list_cur_page;
        user_data = match_user_num(list_valid_array[page * 5 + list_index]);
        if(user_data != NULL)
        {
            printf("user_name_4 %s num %d\n",user_data->name,user_data->user_num);
            ui_text_set_str_by_id(ENC_USER_NAME_TXT_4, "ascii", user_data->name);
        }

        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_4)
.onchange = rec_lay_user_name_4_onchange,
};
static int rec_lay_user_name_5_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 list_index = 4;
    u8 page = 0;
    u8 i;
    get_user_infor_struct();
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        page = list_cur_page;
        user_data = match_user_num(list_valid_array[page * 5 + list_index]);
        if(user_data != NULL)
        {
            printf("user_name_5 %s num %d\n",user_data->name,user_data->user_num);
            ui_text_set_str_by_id(ENC_USER_NAME_TXT_5, "ascii", user_data->name);
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_USER_NAME_5)
.onchange = rec_lay_user_name_5_onchange,
};



/***************************** 用户名列表选择界面 ************************************/

static int rec_user_name_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_name_btn_ontouch**");
    struct button *btn = (struct button *)ctr;
    user_infor *record_user_name = malloc(sizeof(user_infor));
    if(record_user_name)
    {
        memset(record_user_name,0,sizeof(user_infor));
    }
    const int user_name_btn[] = {
        ENC_USER_NAME_BTN_1,
        ENC_USER_NAME_BTN_2,
        ENC_USER_NAME_BTN_3,
        ENC_USER_NAME_BTN_4,
        ENC_USER_NAME_BTN_5
    };
    u8 i,j;
    int pos;
    static u8 move_flag = 0;
    static int x = 0;
    move_flag++;
    if(move_flag > 3){
        move_flag = 0;
    }
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        x = e->pos.x;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        printf("delete user\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        pos = move_posx(x, e->pos.x);
        if(pos > 0){
            list_cur_page--;
            if(list_cur_page > 20)
            {
                list_cur_page = 0;
                return false;
            }
            ui_hide(ENC_LAY_USER_LIST);
            ui_show(ENC_LAY_USER_LIST);
        } else if(pos < 0) {
            list_page_num = (user_count - 1) / 5;
            if(list_cur_page < list_page_num)
            {
                list_cur_page++;
            }
            else
            {
                return false;
            }
            if(list_cur_page > 20)
            {
                list_cur_page = 20;
                return false;
            }
            printf("now page %d\n",list_cur_page);
            ui_hide(ENC_LAY_USER_LIST);
            ui_show(ENC_LAY_USER_LIST);
        }
        break;
    case ELM_EVENT_TOUCH_UP:
        if(move_flag <= 2){
            j=sizeof(user_name_btn)/sizeof(user_name_btn[0]);
            for (i = 0; i < j; i++) {
                if (btn->elm.id == user_name_btn[i]) {
                    now_btn_user = i;
                    break;
                }
            }

            record_user_name = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
            if(record_user_name != NULL)
            {
                printf("current chose user name %s number %d\n",record_user_name->name,record_user_name->user_num);
                ui_hide(ENC_LAY_USER_LIST);
                ui_show(USER_GOTO_DELETE);
                ui_show(ENC_LAY_USER_DETAILS);
            }
            if(record_user_name)
            {
                free(record_user_name);
                record_user_name = NULL;
            }
            u8 command_buf = voice;
            u8 data_buf[] = {key_sound};
            uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_1)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_2)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_3)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_4)
.ontouch = rec_user_name_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_USER_NAME_BTN_5)
.ontouch = rec_user_name_btn_ontouch,
};

/***************************** 用户页面上一页按钮 ************************************/
static int rec_user_previous_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_previous_page_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        list_cur_page--;
        if(list_cur_page > 20)
        {
            list_cur_page = 0;
            return false;
        }
        ui_hide(ENC_LAY_USER_LIST);
        ui_show(ENC_LAY_USER_LIST);

        printf("now page %d\n",list_cur_page);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USER_LIST_PAGE_PRE_BTN)
.ontouch = rec_user_previous_page_ontouch,
};


/***************************** 用户页面下一页按钮 ************************************/
static int rec_user_next_page_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_next_page_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        list_page_num = (user_count - 1) / 5;
        if(list_cur_page < list_page_num)
        {
            list_cur_page++;
        }
        else
        {
            return false;
        }
        if(list_cur_page > 20)
        {
            list_cur_page = 20;
            return false;
        }
        printf("now page %d\n",list_cur_page);
        ui_hide(ENC_LAY_USER_LIST);
        ui_show(ENC_LAY_USER_LIST);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USER_LIST_PAGE_NEXT_BTN)
.ontouch = rec_user_next_page_ontouch,
};




/***************************** 用户名输入界面 ************************************/
static int rec_lay_user_scanf_ontouch(void *ctr, struct element_touch_event *e)
{
    u8 index,input_key = 0;
    u8 num1,num2;
    user_infor *name = malloc(sizeof(user_infor));
    if(name)
    {
        memset(name,0,sizeof(user_infor));
    }
    user_infor record_input = {0};
    UI_ONTOUCH_DEBUG("**rec_lay_user_scanf_ontouch**");
    struct button *btn = (struct button *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if((user_name_num == 0) && (btn->elm.id != BTN_USER_BACK)){
            ui_hide(ENC_PLASE_INPUT_TXT);
        }
        switch(btn->elm.id){
        case BTN_USER_A:
            user_name[user_name_num] = num_input[0];
            break;
        case BTN_USER_B:
            user_name[user_name_num] = num_input[1];
            break;
        case BTN_USER_C:
            user_name[user_name_num] = num_input[2];
            break;
        case BTN_USER_D:
            user_name[user_name_num] = num_input[3];
            break;
        case BTN_USER_E:
            user_name[user_name_num] = num_input[4];
            break;
        case BTN_USER_F:
            user_name[user_name_num] = num_input[5];
            break;
        case BTN_USER_G:
            user_name[user_name_num] = num_input[6];
            break;
        case BTN_USER_H:
            user_name[user_name_num] = num_input[7];
            break;
        case BTN_USER_I:
            user_name[user_name_num] = num_input[8];
            break;
        case BTN_USER_J:
            user_name[user_name_num] = num_input[9];
            break;
        case BTN_USER_K:
            user_name[user_name_num] = num_input[10];
            break;
        case BTN_USER_L:
            user_name[user_name_num] = num_input[11];
            break;
        case BTN_USER_M:
            user_name[user_name_num] = num_input[12];
            break;
        case BTN_USER_N:
            user_name[user_name_num] = num_input[13];
            break;
        case BTN_USER_O:
            user_name[user_name_num] = num_input[14];
            break;
        case BTN_USER_P:
            user_name[user_name_num] = num_input[15];
            break;
        case BTN_USER_Q:
            user_name[user_name_num] = num_input[16];
            break;
        case BTN_USER_R:
            user_name[user_name_num] = num_input[17];
            break;
        case BTN_USER_S:
            user_name[user_name_num] = num_input[18];
            break;
        case BTN_USER_T:
            user_name[user_name_num] = num_input[19];
            break;
        case BTN_USER_U:
            user_name[user_name_num] = num_input[20];
            break;
        case BTN_USER_V:
            user_name[user_name_num] = num_input[21];
            break;
        case BTN_USER_W:
            user_name[user_name_num] = num_input[22];
            break;
        case BTN_USER_X:
            user_name[user_name_num] = num_input[23];
            break;
        case BTN_USER_Y:
            user_name[user_name_num] = num_input[24];
            break;
        case BTN_USER_Z:
            user_name[user_name_num] = num_input[25];
            break;
        case BTN_USER_0:
            user_name[user_name_num] = num_input[26];
            break;
        case BTN_USER_1:
            user_name[user_name_num] = num_input[27];
            break;
        case BTN_USER_2:
            user_name[user_name_num] = num_input[28];
            break;
        case BTN_USER_3:
            user_name[user_name_num] = num_input[29];
            break;
        case BTN_USER_4:
            user_name[user_name_num] = num_input[30];
            break;
        case BTN_USER_5:
            user_name[user_name_num] = num_input[31];
            break;
        case BTN_USER_6:
            user_name[user_name_num] = num_input[32];
            break;
        case BTN_USER_7:
            user_name[user_name_num] = num_input[33];
            break;
        case BTN_USER_8:
            user_name[user_name_num] = num_input[34];
            break;
        case BTN_USER_9:
            user_name[user_name_num] = num_input[35];
            break;
        case BTN_USER_KONG:
            user_name[user_name_num] = num_input[36];
            break;
        case BTN_USER_PIONT:
            user_name[user_name_num] = num_input[37];
            break;
        case BTN_USER_BACK:
            user_name_num -= 2;
            printf("======== user_name_num:%d\n",user_name_num);

            if(user_name_num<0){
                user_name_num = 0;
                user_name[0] = '\0';
                ui_hide(ENC_USER_NAME_TXT);
                ui_show(ENC_PLASE_INPUT_TXT);
                return false;
            }

            break;
        case BTN_USER_OK:
            input_key = 1;
            if(user_count > MAX_USER_NUM){
                printf("======================= user name num max\n");
                break;
            }
            name = match_user_data(user_name, sizeof(user_name), sizeof(user_infor));
            if(name == NULL)
            {
                index = match_num_to_insert();//查找删除后留下的sector
                if((index - 1) > 0)
                {
                     record_input.index = index;//删除用户的扇区
                }
                else
                {
                    record_input.index = 0;//新的扇区
                }

                printf("======== btn ok : user name:");
                puts(user_name);
                memcpy(user_name_arrsy[user_count],user_name,sizeof(user_name));

                memcpy(record_input.name,user_name,sizeof(user_name));//将输入的用户名拷贝到结构体中
                printf("add user information  name %s\n",record_input.name);

                record_input.user_num = list_num_increase;
                num1 = list_num_increase >> 8;
                num2 = list_num_increase & 0xFF;
                printf("add user information  num %d\n",record_input.user_num);

                list_valid_array[user_count] = record_input.user_num;

                now_btn_user = user_count % 5;
                list_page_num = user_count / 5;
                list_cur_page = list_page_num;

                user_count++;
                list_num_increase++;
                if(user_count == 1){
                    record_input.user_power = ADMIN;
                } else {
                    record_input.user_power = UNLOCK;
                }
                write_data_to_flash(&record_input,0);
                if(name)
                {
                    free(name);
                    name = NULL;
                }
                u8 command_buf = add_user;
                u8 data_buf[] = {num1,num2};
                uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

                ui_hide(ENC_LAY_USER_INPUT);
                ui_show(USER_GOTO_DELETE);
                ui_show(ENC_LAY_USER_DETAILS);

            }

            return false;
        }

        if(user_name_num == (MAX_NAME_LEN )){
            printf("======================= user name num max\n");
            break;
        }

        user_name[user_name_num+1] = '\0';
        printf("============= user name:");
        puts(user_name);
        if(user_name_num == 0){
            ui_show(ENC_USER_NAME_TXT);
        }
        ui_text_set_str_by_id(ENC_USER_NAME_TXT, "ascii",&user_name);
        if(/*(btn->elm.id != BTN_USER_BACK) &&*/ (btn->elm.id != BTN_USER_OK)){
            user_name_num++;
        }
        if(!input_key)
        {
            u8 command_buf = voice;
            u8 data_buf[] = {key_sound};
            uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BTN_USER_1)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_2)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_3)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_4)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_5)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_6)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_7)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_8)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_9)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_0)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_Q)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_W)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_E)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_R)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_T)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_Y)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_U)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_I)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_O)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_P)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_A)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_S)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_D)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_F)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_G)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_H)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_J)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_K)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_L)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_Z)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_X)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_C)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_V)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_B)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_N)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_M)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_BACK)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_KONG)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_PIONT)
.ontouch = rec_lay_user_scanf_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BTN_USER_OK)
.ontouch = rec_lay_user_scanf_ontouch,
};


/***************************** 用户详情界面 管理员权限按钮 ************************************/
static int rec_user_power_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_power_btn_ontouch**");
    user_infor *user_input = malloc(sizeof(user_infor));
    if(user_input)
    {
        memset(user_input,0,sizeof(user_infor));
    }

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        user_input = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
        if(user_input != NULL)
        {
            //user_function_array[(list_page_num*5)+now_btn_user][1] = !user_function_array[(list_page_num*5)+now_btn_user][1];
            user_input->user_power = !user_input->user_power;
            if(user_input->user_power){
                power_key_num += user_input->key_num;
            } else {
                power_key_num -= user_input->key_num;
            }
            printf("==================user power limit %d index %d\n",user_input->user_power,user_input->index);
            write_data_to_flash(user_input,0);
            ui_pic_show_image_by_id(ENC_USER_POWER_PIC,user_input->user_power);
            ui_text_show_index_by_id(ENC_USER_POWER_TXT,user_input->user_power);
            if(user_input)
            {
                free(user_input);
                user_input = NULL;
            }
            u8 command_buf = voice;
            u8 data_buf[] = {key_sound};
            uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_POWER_BTN)
.ontouch = rec_user_power_btn_ontouch,
};

void other_txt(u8 idx)
{
    ui_text_show_index_by_id(ADD_NEW_FINGER_TXT_1,idx);
}

/***************************** 添加卡片界面 ************************************/
static int rec_new_nfc_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ani_flag = 1;
        user_page_flag = 5;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);

        u8 command_buf = voice;
        u8 data_buf[] = {put_card};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_ADD_NEW_NFC)
.onchange = rec_new_nfc_onchange,
};



/***************************** 添加指纹界面 ************************************/
static int rec_new_fingerprint_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ani_flag = 1;
        user_page_flag = 4;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        ui_text_show_index_by_id(ADD_NEW_FINGER_TXT_1,0);
        /*
        sys_timeout_add(1, other_txt, 1000);
        sys_timeout_add(2, other_txt, 2000);
        sys_timeout_add(3, other_txt, 3000);
        sys_timeout_add(4, other_txt, 4000);
        sys_timeout_add(5, other_txt, 5000);
        */
        u8 command_buf = voice;
        u8 data_buf[] = {press_finger};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_ADD_NEW_FINGERPRINT)
.onchange = rec_new_fingerprint_onchange,
};

void delay_show_new_key()
{
    ui_show(ENC_USER_NEW_KEY);
}


/***************************** 用户详情界面 新建指纹密码按钮 ************************************/
static int rec_goto_new_password_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_new_password_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        input_key_flag = 0;
        u8 command_buf = voice;
        u8 data_buf[] = {input_key};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        ui_hide(ENC_LAY_USER_DETAILS);
        ui_hide(USER_GOTO_DELETE);

        //sys_timeout_add(NULL, delay_show_new_key, 300);
        ui_show(ENC_USER_NEW_KEY);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_GOTO_NEW_PASSWORD_BTN)
.ontouch = rec_goto_new_password_btn_ontouch,
};

/***************************** 新建指纹密码返回详情界面按钮 ************************************/
static int rec_user_new_password_cancel_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_user_new_password_cancel_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        input_key_flag = 0;
        ui_hide(ENC_USER_NEW_KEY);
        ui_show(USER_GOTO_DELETE);
        ui_show(ENC_LAY_USER_DETAILS);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NEW_PWD_CANCEL_BTN)
.ontouch = rec_user_new_password_cancel_btn_ontouch,
};


/***************************** 密码输入界面 ************************************/
static int rec_new_key_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 num[2] = {0};
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        user_page_flag = 6;
        ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
        memset(password_code,'a',MAX_PAW_NUM);          //显示时赋值为a，表示为空
        password_num = 0;
        num[0] = current_user >> 8;
        num[1] = current_user & 0xFF;

        u8 command_buf = add_password;
        u8 data_buf[] = {num[0],num[1],other_key};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_NEW_KEY)
.onchange = rec_new_key_lay_onchange,
};



/***************************** 新建密码界面 新建密码按钮 ************************************/
static int rec_new_password_input_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_new_password_input_ontouch**");
    int sel_item = 0;
    struct button *btn = (struct button *)ctr;
    const int btn_id[] = {
        NEW_PWD_0,
        NEW_PWD_1,
        NEW_PWD_2,
        NEW_PWD_3,
        NEW_PWD_4,
        NEW_PWD_5,
        NEW_PWD_6,
        NEW_PWD_7,
        NEW_PWD_8,
        NEW_PWD_9
    };
    u8 i,j;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(password_num == MAX_PAW_NUM){
            printf("======================= pwd num max\n");
            break;
        }
        j=sizeof(btn_id)/sizeof(btn_id[0]);
        for (i = 0; i < j; i++) {
            if (btn->elm.id == btn_id[i]) {
                sel_item = i;
                break;
            }
        }
        password_code[password_num] = sel_item;
        printf("============ in pwd:");
        put_buf(password_code,MAX_PAW_NUM);             //输出当前输入的密码

        password_num++;
        ui_text_set_str_by_id(USER_NEW_PASSWORD_TXT, "ascii", &asterisk_number[MAX_PAW_NUM-password_num]);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NEW_PWD_0)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_1)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_2)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_3)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_4)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_5)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_6)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_7)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_8)
.ontouch = rec_new_password_input_ontouch,
};
REGISTER_UI_EVENT_HANDLER(NEW_PWD_9)
.ontouch = rec_new_password_input_ontouch,
};


/***************************** 新建密码界面 密码删除按钮 ************************************/
static int rec_new_password_del_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_new_password_del_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:

        if(password_num == 0){
            ui_text_set_str_by_id(USER_NEW_PASSWORD_TXT, "ascii", " ");
            break;
        }
        password_num--;
        password_code[password_num] = 'a';
        printf("============== del pwd:");
        put_buf(password_code,MAX_PAW_NUM);             //输出当前输入的密码
        ui_text_set_str_by_id(USER_NEW_PASSWORD_TXT, "ascii", &asterisk_number[MAX_PAW_NUM-password_num]);


        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(USER_NEW_PASSWORD_DEL_BTN)
.ontouch = rec_new_password_del_ontouch,
};

/***************************** 新建密码界面 密码保存按钮 ************************************/
static int rec_new_password_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_new_password_confirm_ontouch**");
    static u8 paw_buf[MAX_PAW_NUM] = {0};
    get_user_infor_struct();
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(password_num < MIN_PAW_NUM){
            printf("new pwd num min\n");
            break;
        }
        user_data = match_user_data(password_code, sizeof(password_code), sizeof(user_infor));
        if(user_data != NULL)
        {
            printf("password is same,please input again\n");
        }
        else
        {
            printf("input password is valid\n");
            input_key_flag = !input_key_flag;
            if(input_key_flag)//第一次输入密码，拷贝给临时数组
            {
                user_page_flag = 7;
                ui_text_show_index_by_id(ENC_SET_USER_TXT,user_page_flag);
                memcpy(paw_buf,password_code,sizeof(password_code));
                u8 command_buf = voice;
                u8 data_buf[] = {input_key_again};
                uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

            }
            else
            {
                if(!memcmp(paw_buf,password_code,sizeof(password_code)))//比较两次输入的密码是否相同
                {
                    printf("two password same\n");
                    user_data = match_user_num(list_valid_array[now_btn_user + 5 * list_cur_page]);
                    if(user_data != NULL)
                    {
                        printf("new_password user name  %s num %d\n",user_data->name,user_data->user_num);
                        printf("current %d key index %d\n",user_data->key_num,user_data->index);
                        if(user_data->key_num < MAX_KEY_NUM )
                        {
                            memcpy(user_data->key[user_data->key_num].key_buf,paw_buf,sizeof(paw_buf));
                            user_data->key[user_data->key_num].key_mode = PASSWORD;
                            user_data->key_num++;
                            write_data_to_flash(user_data,0);
                        }
                        else
                        {
                            printf("key num is full!\n");
                        }
                        memset(paw_buf,0,sizeof(paw_buf));
                        user_key_num++;
                        if(user_data->user_power)
                        {
                            power_key_num++;
                        }
                    }
                    u8 command_buf = voice;
                    u8 data_buf[] = {operate_success};
                    uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

                }
                else
                {
                    memset(paw_buf,0,sizeof(paw_buf));
                    printf("two password different\n");
                    input_key_flag = 0;

                    u8 command_buf = voice;
                    u8 data_buf[] = {two_paw_diff};
                    uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
                }
                ui_hide(ENC_USER_NEW_KEY);
                ui_show(USER_GOTO_DELETE);
                ui_show(ENC_LAY_USER_DETAILS);
            }
        }
        if(user_data)
        {
            free(user_data);
            user_data = NULL;
        }
        memset(user_data,0,sizeof(user_infor));

        memset(password_code,'a',MAX_PAW_NUM);          //显示时赋值为a，表示为空
        password_num = 0;
        ui_text_set_str_by_id(USER_NEW_PASSWORD_TXT, "ascii", " ");

        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(NEW_PWD_CONFIRM_BTN)
.ontouch = rec_new_password_confirm_ontouch,
};

/***************************** 新建人脸界面 ************************************/
static int rec_new_face_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 command_buf;
    u8 num[2] = {0};
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
        num[0] = current_user >> 8;
        num[1] = current_user & 0xFF;
        command_buf = add_password;
        u8 data_buf[] = {num[0],num[1],face};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        */
        break;
    case ON_CHANGE_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_FACIAL_LAY)
.onchange = rec_new_face_lay_onchange,
};


void add_face_mode(u8 command)
{
    u8 command_buf = voice;
    u8 data_buf[] = {command};
    uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
}

void time_delay_show()
{
//    ui_hide(ENC_WIN);
    ui_show(ENC_FACIAL_LAY);
}

/***************************** 用户详情界面 新建人脸按钮 ************************************/
static int rec_user_facial_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    u8 command,data;
    UI_ONTOUCH_DEBUG("**rec_user_facial_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ani_flag = 1;
        init_intent(&it);
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_SWITCH_WIN;
        start_app(&it);

        command = voice;
        data = set_face;
        uart_send_package(command,&data,1);

        sys_timeout_add(NULL, time_delay_show, 300);
/*
        sys_timeout_add(turn_left, add_face_mode, 1000);
        sys_timeout_add(turn_right, add_face_mode, 2000);
        sys_timeout_add(lower_head, add_face_mode, 3000);
        sys_timeout_add(raise_head, add_face_mode, 4000);
*/
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_USER_FACIAL_BTN)
.ontouch = rec_user_facial_btn_ontouch,
};

void exit_screen_protect()
{
    ani_flag = 0;
    goto_facial_page_flag = 0;
}

/***************************** 人脸界面 返回按钮 ************************************/
static int rec_facial_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_facial_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        init_intent(&it);
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_SWITCH_WIN_OFF;
        start_app(&it);
        goto_facial_page_flag = 1;
        ui_hide(ENC_FACIAL_LAY);
        ani_flag = 0;
        goto_facial_page_flag = 0;
//        sys_timeout_add(NULL, exit_screen_protect, 100);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FACIAL_RETURN_BTN)
.ontouch = rec_facial_return_btn_ontouch,
};


/***************************** 记录查询界面 ************************************/
const int lay_record_list[]={
    ENC_RECORD_INFOR_LIST_1,
    ENC_RECORD_INFOR_LIST_2,
    ENC_RECORD_INFOR_LIST_3,
    ENC_RECORD_INFOR_LIST_4,
    ENC_RECORD_INFOR_LIST_5,
};

static int rec_lay_record_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u16 end = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("now visit num %d\n",visit_global_count);
        if(visit_global_count > 0)
        {
            end = (record_now_page+1) * 5;
            if(end > visit_global_count)
            {
                end = visit_global_count;
            }
            printf("end %d\n",end);
            for(int i = record_now_page * 5; i < end; i++){
                ui_show(lay_record_list[i%5]);
            }

        }
    case ON_CHANGE_SHOW:
        ui_show(ENC_UP_LAY);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR)
.onchange = rec_lay_record_infor_onchange,
};


/***************************** 记录查询界面 返回按钮 ************************************/
static int rec_record_page_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_record_page_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        copy_buffer_idx = 0;
        ui_hide(ENC_LAY_RECORD_PAGE);
        ui_show(ENC_LAY_PAGE);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_RETURN)
.ontouch = rec_record_page_return_btn_ontouch,
};


/*****************************记录查询年月日 ************************************/
static int timer_record_ymd_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    int temp  = 0;
    static int last_temp = 10;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        /*
        time->year = user_global_visit[0].records.visit[0].record_time.year;
        time->month = user_global_visit[0].records.visit[0].record_time.month;
        time->day = user_global_visit[0].records.visit[0].record_time.day;
        time->hour = user_global_visit[0].records.visit[0].record_time.hour;
        time->min = user_global_visit[0].records.visit[0].record_time.min;
        time->sec = user_global_visit[0].records.visit[0].record_time.sec;
        */
        break;
    case ON_CHANGE_INIT:

        time->year = user_global_visit[0].record_time.year;
        time->month = user_global_visit[0].record_time.month;
        time->day = user_global_visit[0].record_time.day;
        time->hour = user_global_visit[0].record_time.hour;
        time->min = user_global_visit[0].record_time.min;
        time->sec = user_global_visit[0].record_time.sec;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_TIME_YMD)
.onchange = timer_record_ymd_onchange,
};

void record_pre_page()
{
    u16 record_tmp = 0;
    u16 record_visit = 0;
    struct sys_time sys_time = {0};
    static struct sys_time time_tmp = {0};//静态的时间变量，用来存放最后一个记录的时间

    if (visit_global_count > 0 && record_now_page > 0) {
        record_now_page--;
        ui_hide(ENC_RECORD_INFOR);
        ui_show(ENC_RECORD_INFOR);

    }else{
        if(memcmp(&time_tmp,&global_date,sizeof(struct sys_time)) != 0){//与全局的时间比较，不一样，则说明有新的时间记录，赋值存放
            time_tmp = global_date;
        }
        get_system_time(&sys_time);
        record_tmp = tim_cnt + pre_page_cnt;
        if(record_tmp < time_idx_count && tim_cnt != (time_idx_count-1)){
            record_now_page = 0;
            memset(user_global_visit,0,sizeof(user_global_visit));
            do {
                normal_idx += (next_page_cnt-1);
                copy_buffer_idx += (next_page_cnt-1);
                next_page_cnt = 1;
                tim_cnt += pre_page_cnt;

                pre_page_cnt = match_visit_time(&global_date, user_global_visit,visit_global_count,0);
                visit_global_count = pre_page_cnt;
                printf("visit_global_count %d tim_cnt %d normal_idx %d\n",visit_global_count,tim_cnt,normal_idx);
                reverse_array(user_global_visit,visit_global_count);
                record_now_page = (visit_global_count-1) / 5;//更新记录页数。从每个时间的最后记录往前翻

            } while (visit_global_count == 0 && (tim_cnt + 1 > time_idx_count));

            ui_hide(ENC_RECORD_TIME_YMD);
            ui_show(ENC_RECORD_TIME_YMD);
            ui_hide(ENC_RECORD_INFOR);
            ui_show(ENC_RECORD_INFOR);
        }

    }

    printf("visit_global_count %d record page now %d\n",visit_global_count,record_now_page);
    u8 command_buf = voice;
    u8 data_buf[] = {key_sound};
    uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
}


void record_next_page()
{
    u16 record_tmp = 0;
    static struct sys_time time_tmp = {0};
    static u16 count_tmp = 0;

    if(visit_global_count > 5 * (record_now_page + 1)){             //访问记录大于当前页数
        record_now_page++;
        ui_hide(ENC_RECORD_INFOR);
        ui_show(ENC_RECORD_INFOR);

    } else {
        if(memcmp(&time_tmp,&global_date,sizeof(struct sys_time)) != 0){//保护最后读到的访问记录
            time_tmp = global_date;
        }

        record_tmp = tim_cnt-next_page_cnt;
        if(record_tmp < MAX_RECORD_NUM){
            memset(user_global_visit,0,sizeof(user_global_visit));
            record_now_page = 0;
            do {
                pre_page_cnt = 1;
                tim_cnt -= next_page_cnt;
                next_page_cnt = match_visit_time(&global_date, user_global_visit,visit_global_count,1);//查找前一天的访问
                visit_global_count = next_page_cnt;
                printf("next_page_cnt %d tim_cnt %d normal_idx %d\n",visit_global_count,tim_cnt,normal_idx);

            } while (visit_global_count == 0 && tim_cnt > MAX_RECORD_NUM);

            ui_hide(ENC_RECORD_TIME_YMD);
            ui_show(ENC_RECORD_TIME_YMD);
            ui_hide(ENC_RECORD_INFOR);
            ui_show(ENC_RECORD_INFOR);
        }
    }

    printf("visit_global_count %d record page now %d\n",visit_global_count,record_now_page);
    u8 command_buf = voice;
    u8 data_buf[] = {key_sound};
    uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
}


/***************************** 记录页面滑动 按钮 ************************************/
static int rec_record_infor_move_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_record_infor_move_btn_ontouch**");
    int pos;
    static u8 move_flag = 0;
    static int x = 0;
    move_flag++;
    if(move_flag > 3){
        move_flag = 0;
    }
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        x = e->pos.x;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        pos = move_posx(x, e->pos.x);
        if(pos < 0){
            record_next_page();
        } else if(pos > 0) {
            record_pre_page();
        }
        break;
    case ELM_EVENT_TOUCH_UP:

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_MOVE_BTN)
.ontouch = rec_record_infor_move_btn_ontouch,
};



/***************************** 记录页面 上一页 按钮 ************************************/
static int rec_record_pre_page_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_record_pre_page_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        record_pre_page();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_LEFT_BTN)
.ontouch = rec_record_pre_page_btn_ontouch,
};

/***************************** 记录页面 下一页 按钮 ************************************/
static int rec_record_next_page_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_record_next_page_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        record_next_page();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_RIGHT_BTN)
.ontouch = rec_record_next_page_btn_ontouch,
};




/****************************记录时间控件动作 ************************************/
static int timer_1_user_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 0;
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        time->year = user_global_visit[record_now_page * 5 + idx].record_time.year;
        time->month = user_global_visit[record_now_page * 5 + idx].record_time.month;
        time->day = user_global_visit[record_now_page * 5 + idx].record_time.day;
        time->hour = user_global_visit[record_now_page * 5 + idx].record_time.hour;
        time->min = user_global_visit[record_now_page * 5 + idx].record_time.min;
        time->sec = user_global_visit[record_now_page * 5 + idx].record_time.sec;

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_1)
.onchange = timer_1_user_infor_onchange,
};



static int timer_2_user_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 1;
    u16 cnt;
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        time->year = user_global_visit[record_now_page * 5 + idx].record_time.year;
        time->month = user_global_visit[record_now_page * 5 + idx].record_time.month;
        time->day = user_global_visit[record_now_page * 5 + idx].record_time.day;
        time->hour = user_global_visit[record_now_page * 5 + idx].record_time.hour;
        time->min = user_global_visit[record_now_page * 5 + idx].record_time.min;
        time->sec = user_global_visit[record_now_page * 5 + idx].record_time.sec;

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_2)
.onchange = timer_2_user_infor_onchange,
};



static int timer_3_user_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 2;
    u16 cnt;
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        time->year = user_global_visit[record_now_page * 5 + idx].record_time.year;
        time->month = user_global_visit[record_now_page * 5 + idx].record_time.month;
        time->day = user_global_visit[record_now_page * 5 + idx].record_time.day;
        time->hour = user_global_visit[record_now_page * 5 + idx].record_time.hour;
        time->min = user_global_visit[record_now_page * 5 + idx].record_time.min;
        time->sec = user_global_visit[record_now_page * 5 + idx].record_time.sec;

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_3)
.onchange = timer_3_user_infor_onchange,
};


static int timer_4_user_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 3;
    u16 cnt;
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        time->year = user_global_visit[record_now_page * 5 + idx].record_time.year;
        time->month = user_global_visit[record_now_page * 5 + idx].record_time.month;
        time->day = user_global_visit[record_now_page * 5 + idx].record_time.day;
        time->hour = user_global_visit[record_now_page * 5 + idx].record_time.hour;
        time->min = user_global_visit[record_now_page * 5 + idx].record_time.min;
        time->sec = user_global_visit[record_now_page * 5 + idx].record_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_4)
.onchange = timer_4_user_infor_onchange,
};



static int timer_5_user_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 4;
    u16 cnt;
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        time->year = user_global_visit[record_now_page * 5 + idx].record_time.year;
        time->month = user_global_visit[record_now_page * 5 + idx].record_time.month;
        time->day = user_global_visit[record_now_page * 5 + idx].record_time.day;
        time->hour = user_global_visit[record_now_page * 5 + idx].record_time.hour;
        time->min = user_global_visit[record_now_page * 5 + idx].record_time.min;
        time->sec = user_global_visit[record_now_page * 5 + idx].record_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_TIME_5)
.onchange = timer_5_user_infor_onchange,
};




/***************************** 记录列表显示 ************************************/
static int rec_user_infor_list_1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 0;

    struct sys_time time_visit;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        printf("Record name list 1 : %s\n",user_global_visit[record_now_page * 5 + idx].name);
        ui_text_set_str_by_id(ENC_RECORD_INFOR_NAME_1, "ascii", user_global_visit[record_now_page * 5 + idx].name);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_MODE_1, user_global_visit[record_now_page * 5 + idx].unlock.unlock_power);
        ui_pic_show_image_by_id(ENC_RECORD_INFOR_PIC_1,user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_TXT_1, user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_1)
.onchange = rec_user_infor_list_1_onchange,
};
static int rec_user_infor_list_2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 1;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("Record name list 2 : %s\n",user_global_visit[record_now_page * 5 + idx].name);
        ui_text_set_str_by_id(ENC_RECORD_INFOR_NAME_2, "ascii", user_global_visit[record_now_page * 5 + idx].name);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_MODE_2, user_global_visit[record_now_page * 5 + idx].unlock.unlock_power);
        ui_pic_show_image_by_id(ENC_RECORD_INFOR_PIC_2,user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_TXT_2, user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_2)
.onchange = rec_user_infor_list_2_onchange,
};
static int rec_user_infor_list_3_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 2;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("Record name list 3 : %s\n",user_global_visit[record_now_page * 5 + idx].name);
        ui_text_set_str_by_id(ENC_RECORD_INFOR_NAME_3, "ascii", user_global_visit[record_now_page * 5 + idx].name);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_MODE_3, user_global_visit[record_now_page * 5 + idx].unlock.unlock_power);
        ui_pic_show_image_by_id(ENC_RECORD_INFOR_PIC_3,user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_TXT_3, user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_3)
.onchange = rec_user_infor_list_3_onchange,
};
static int rec_user_infor_list_4_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 3;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("Record name list 4 : %s\n",user_global_visit[record_now_page * 5 + idx].name);
        ui_text_set_str_by_id(ENC_RECORD_INFOR_NAME_4, "ascii", user_global_visit[record_now_page * 5 + idx].name);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_MODE_4, user_global_visit[record_now_page * 5 + idx].unlock.unlock_power);
        ui_pic_show_image_by_id(ENC_RECORD_INFOR_PIC_4,user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_TXT_4, user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_4)
.onchange = rec_user_infor_list_4_onchange,
};
static int rec_user_infor_list_5_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 idx = 4;

    struct sys_time time_visit;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("Record name list 5 : %s\n",user_global_visit[record_now_page * 5 + idx].name);
        ui_text_set_str_by_id(ENC_RECORD_INFOR_NAME_5, "ascii", user_global_visit[record_now_page * 5 + idx].name);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_MODE_5, user_global_visit[record_now_page * 5 + idx].unlock.unlock_power);
        ui_pic_show_image_by_id(ENC_RECORD_INFOR_PIC_5,user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        ui_text_show_index_by_id(ENC_RECORD_INFOR_TXT_5, user_global_visit[record_now_page * 5 + idx].unlock.unlock_mode);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_RECORD_INFOR_LIST_5)
.onchange = rec_user_infor_list_5_onchange,
};



u8 start_btn = 0;
/***************************** 网络设置页面 ************************************/
static int network_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        net_page_flag = 0;
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_LIST_LAY)
.onchange = network_set_onchange,
};


/***************************** 网络设置界面 返回按钮 ************************************/
static int network_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**network_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        if(net_page_flag){
            start_btn = 0;
            ui_hide(NETWORK_DISTRIBUTION_START_ANI);

            ui_hide(NETWORK_SET_LAY);
            ui_show(NETWORK_LIST_LAY);
        }else{
            ui_hide(ENC_NETWORK_PAGE);
            ui_show(ENC_LAY_PAGE);
        }
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_RETURN_BTN)
.ontouch = network_return_btn_ontouch,
};

/***************************** 进入配网按钮 ************************************/
static int network_enter_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**network_start_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        start_btn = 0;
        ui_hide(NETWORK_LIST_LAY);
        ui_show(NETWORK_SET_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_SET_BTN)
.ontouch = network_enter_btn_ontouch,
};


/***************************** 开始配网页面 ************************************/
static int network_start_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        //ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        //ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        net_page_flag = 1;
        ui_text_show_index_by_id(NETWORK_DISTRIBUTION_START_TXT,start_btn);
        ui_pic_show_image_by_id(NETWORK_DISTRIBUTION_START_PIC,start_btn);
        break;
    case ON_CHANGE_SHOW:
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_SET_LAY)
.onchange = network_start_lay_onchange,
};


void network_ani_playend_handle()
{
    ui_hide(NETWORK_DISTRIBUTION_START_ANI);
    ui_text_show_index_by_id(NETWORK_DISTRIBUTION_START_TXT,2);
    ui_pic_show_image_by_id(NETWORK_DISTRIBUTION_START_PIC,1);
    ani_flag = 0;
}

/***************************** 配网界面 开始配网按钮 ************************************/
static int network_distribute_start_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**network_distribute_start_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        start_btn = !start_btn;
        ui_text_show_index_by_id(NETWORK_DISTRIBUTION_START_TXT,start_btn);
        if(start_btn)
        {
            ani_flag = 1;
            ui_show(NETWORK_DISTRIBUTION_START_ANI);
            ui_hide(NETWORK_DISTRIBUTION_START_PIC);
            sys_timeout_add(NULL, network_ani_playend_handle, 30000);
        }else{
            ui_hide(NETWORK_DISTRIBUTION_START_ANI);
            ui_pic_show_image_by_id(NETWORK_DISTRIBUTION_START_PIC,start_btn);
        }

        u8 command_buf = voice;
        u8 data_buf[] = {set_network};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_DISTRIBUTION_START_BTN)
.ontouch = network_distribute_start_btn_ontouch,
};


#if 0
/***************************** 配网界面 返回按钮 ************************************/
static int network_set_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**network_set_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        start_btn = 0;
        ui_hide(NETWORK_DISTRIBUTION_START_ANI);

        ui_hide(NETWORK_SET_LAY);
        ui_show(NETWORK_LIST_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_SET_RETURN_BTN)
.ontouch = network_set_return_btn_ontouch,
};
#endif

/***************************** 重置wifi模块按钮 ************************************/
static int network_reset_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**network_reset_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_show(NETWORK_RESET_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_RESET_BTN)
.ontouch = network_reset_btn_ontouch,
};

/***************************** 重置wifi模块页面 ************************************/
static int network_reset_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_SHOW:
        ui_show(ENC_UP_LAY);
        printf("start reset netwaork...\n");
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_RESET_LAY)
.onchange = network_reset_lay_onchange,
};


/***************************** 重置wifi 取消按钮 ************************************/
static int network_reset_cancel_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**network_reset_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(NETWORK_RESET_LAY);
        ui_show(NETWORK_LIST_LAY);
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NET_WORK_RESET_CANCEL_BTN)
.ontouch = network_reset_cancel_btn_ontouch,
};


/***************************** 重置wifi 确认按钮 ************************************/
static int network_reset_confirm_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**network_reset_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("Reset Wifi Confirm\n");
        ui_hide(NETWORK_RESET_LAY);
        ui_show(NETWORK_RESET_PROG_BAR_LAY);

        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_RESET_CONFIRM_BTN)
.ontouch = network_reset_confirm_btn_ontouch,
};



void net_reset_progress()
{
    ani_flag = 1;
    static u8 persent = 0;
    persent += 1;
    ui_slider_set_persent_by_id(NETWORK_RESET_PROGRESS_BAR,persent);
    if(persent == 100)
    {
        persent = 0;
        ui_hide(NETWORK_RESET_PROG_BAR_LAY);
        ui_show(NETWORK_LIST_LAY);
        ani_flag = 0;
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
    }
}

/***************************** 重置wifi模块进度条页面 ************************************/
static int network_reset_lay_progress_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("start reset netwaork...\n");
        for(int i = 0; i < 100; i++)
        {
            sys_timeout_add(NULL,net_reset_progress,10);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(NETWORK_RESET_PROG_BAR_LAY)
.onchange = network_reset_lay_progress_onchange,
};

/***************************** 网络设置界面 ************************************/
static int rec_network_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
//        ani_flag = 0;
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_NETWORK_PAGE)
.onchange = rec_network_set_onchange,
};




/***************************** 系统信息界面 返回按钮 ************************************/
static int rec_sys_info_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_sys_info_return_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:

        switch(sys_info_flag) {
        case 0:
            ui_hide(ENC_LAY_SYS_INFO_PAGE);
            ui_show(ENC_LAY_PAGE);
//            ui_show(ENC_PAGE_LEFT_BTN);
            break;
        case 1:
            sys_info_flag = 0;
            ui_hide(SYS_ABOUT_DEVICE_PAGE);
            ui_show(SYS_INFO_LIST);
            break;
        case 2:
            sys_info_flag = 0;
            auto_check_flag = 0;
            ui_hide(SYS_AUTO_CHECK_PAGE);
            ui_show(SYS_INFO_LIST);
            break;
        case 3:
            sys_info_flag = 0;
            ui_hide(SYS_AGAIN_TEST_PAGE);
            ui_show(SYS_INFO_LIST);
            break;
        case 4:
            sys_info_flag = 0;
            //ui_hide(SYS_FINAL_TEST_PAGE);
            ui_show(SYS_INFO_LIST);
            break;
        case 5:
            sys_info_flag = 0;
            ui_hide(SYS_STATUS_INFOR_PAGE);
            ui_show(SYS_INFO_LIST);
            break;
        case 6:
            sys_info_flag = 0;
            ui_hide(SYS_CHECK_DETAIL_PAGE);
            ui_show(SYS_INFO_LIST);
            break;
        }



        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SYS_INFO_RETURN)
.ontouch = rec_sys_info_return_btn_ontouch,
};

/***************************** 门锁配置界面 开关按钮 ************************************/
static int rec_door_lock_onoff_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_door_lock_onoff_btn_ontouch**");
    struct button *btn = (struct button *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch(btn->elm.id){
        case ENC_LOCK_LIST_BTN_1:
            lock_array[0] = !lock_array[0];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_1,lock_array[0]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_1,lock_array[0]);

            db_update("ver", lock_array[0]);//验证模式 存入数据库
            db_flush();
            break;
        case ENC_LOCK_LIST_BTN_2:
            lock_array[1] = !lock_array[1];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_2,lock_array[1]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_2,lock_array[1]);

            db_update("pol", lock_array[1]);//防撬报警 存入数据库
            db_flush();
            break;
        case ENC_LOCK_LIST_BTN_3:
            lock_array[2] = !lock_array[2];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_3,lock_array[2]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_3,lock_array[2]);

            db_update("body", lock_array[2]);//人体感应 存入数据库
            db_flush();
            break;
        case ENC_LOCK_LIST_BTN_4:
            lock_array[3] = !lock_array[3];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_4,lock_array[3]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_4,lock_array[3]);

            db_update("open", lock_array[3]);//常开模式 存入数据库
            db_flush();
            break;
        case ENC_LOCK_LIST_BTN_5:
            lock_array[4] = !lock_array[4];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_5,lock_array[4]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_5,lock_array[4]);

            db_update("lrs", lock_array[4]);//左开右开选择 存入数据库
            db_flush();
            break;
        case ENC_LOCK_LIST_BTN_6:
            lock_array[5] = !lock_array[5];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_6,lock_array[5]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_6,lock_array[5]);

            db_update("pop", lock_array[5]);// 自弹锁体选择 存入数据库
            db_flush();
            break;
        case ENC_LOCK_LIST_BTN_7:
            lock_array[6] = !lock_array[6];
            ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_7,lock_array[6]);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_7,lock_array[6]);

            db_update("dblock", lock_array[6]);//自动反锁选择 存入数据库
            db_flush();
            break;
        }
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_1)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_2)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_3)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_4)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_5)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_6)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_BTN_7)
.ontouch = rec_door_lock_onoff_btn_ontouch,
};


/***************************** 门锁配置界面初始化 ************************************/
static int rec_lay_lock_list_1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_1,lock_array[0]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_1,lock_array[0]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_2,lock_array[1]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_2,lock_array[1]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_3,lock_array[2]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_3,lock_array[2]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_4,lock_array[3]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_4,lock_array[3]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_5,lock_array[4]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_5,lock_array[4]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_LOCK_LIST_1)
.onchange = rec_lay_lock_list_1_onchange,
};

void move_pic_timeout_func()
{
    ui_opt_ctr_move(ENC_LOCK_LEV_PIC,lock_array[7]*10,0);
}
static int rec_lay_lock_list_2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:

        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_6,lock_array[5]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_6,lock_array[5]);
        ui_text_show_index_by_id(ENC_LOCK_LIST_TXT_7,lock_array[6]);
        ui_pic_show_image_by_id(ENC_LOCK_LIST_PIC_7,lock_array[6]);

//        ui_show(ENC_LOCK_LEV_PIC);
        printf("========= %d , %s\n",lock_array[7],move_num_str);
        if(lock_array[7] == 0){
            ui_text_set_str_by_id(ENC_LOCK_LEV_NUM_TXT,"ascii","0");
        }else{
            sprintf(move_num_str,"%d",lock_array[7]);
            ui_text_set_str_by_id(ENC_LOCK_LEV_NUM_TXT,"ascii",move_num_str);
        }
//        ui_opt_ctr_move(ENC_LOCK_LEV_PIC,lock_array[7]*10,0);
        sys_timeout_add(NULL,move_pic_timeout_func,10);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_LOCK_LIST_2)
.onchange = rec_lay_lock_list_2_onchange,
};

/***************************** 系统信息界面 翻页按钮 ************************************/
u8 door_lock_page_flag = 0;         //记录现在在哪个设置页面
static int rec_lock_list_page_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_lock_list_page_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        door_lock_page_flag = !door_lock_page_flag;
        if(door_lock_page_flag == 0){
            ui_hide(ENC_LAY_LOCK_LIST_1);
            ui_show(ENC_LAY_LOCK_LIST_2);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PAGE_PIC,1);
        }else{
            ui_hide(ENC_LAY_LOCK_LIST_2);
            ui_show(ENC_LAY_LOCK_LIST_1);
            ui_pic_show_image_by_id(ENC_LOCK_LIST_PAGE_PIC,0);
        }
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LIST_PAGE_BTN)
.ontouch = rec_lock_list_page_btn_ontouch,
};


/***************************** 门锁配置界面 返回按钮 ************************************/
static int rec_door_lock_return_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_door_lock_return_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:

        ui_hide(ENC_LAY_DOOR_LOCK_PAGE);
        ui_show(ENC_LAY_PAGE);
        //ui_show(ENC_PAGE_LEFT_BTN);
        door_lock_page_flag = 0;
        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_DOOR_LOCK_RETURN_BTN)
.ontouch = rec_door_lock_return_btn_ontouch,
};


/***************************** 系统信息界面 自动反锁阈值调整 ************************************/

static int rec_door_lock_lev_move_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_door_lock_lev_move_btn_ontouch**");
    static s16 x_pos_down = 0;
    static int i = 0;
    static int tmp = 0;
    int move_point = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        x_pos_down = e->pos.x;
        i = lock_array[7];
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        memset(move_num_str,' ',sizeof(move_num_str));
        s16 x_pos_now = e->pos.x;
        s16 x_pos_ch = x_pos_now-x_pos_down;
        if (x_pos_ch < SLID_GAP && x_pos_ch > -SLID_GAP) {
            return false;
        }
//        printf("============ x_pos_now:%d , x_pos_ch:%d",x_pos_now,x_pos_now);
        tmp = i + x_pos_ch / SLID_GAP;
//        printf("============ tmp:%d\n",tmp);
        if (tmp > SLID_ITEM - 1) {
            tmp = SLID_ITEM - 1;
        }else if (tmp < 0) {
            tmp = 0;
        }
        if(tmp==i){
            return false;
        }
        move_point = tmp - lock_array[7];
        lock_array[7] = tmp;
        sprintf(move_num_str,"%d",lock_array[7]);
        printf("============ move: %d,  num: %d \n",move_point,lock_array[7]);

        db_update("lockv", lock_array[7]);//自动反锁阈值 存入数据库
        db_flush();
//        puts(move_num_str);
        ui_text_set_str_by_id(ENC_LOCK_LEV_NUM_TXT,"ascii",move_num_str);
        ui_opt_ctr_move(ENC_LOCK_LEV_PIC,move_point*10,0);

        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    case ELM_EVENT_TOUCH_UP:

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LOCK_LEV_BTN)
.ontouch = rec_door_lock_lev_move_btn_ontouch,
};

#if 0
/***************************** 关机按键 ************************************/
static int rec_rec_power_up_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_rec_power_up_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:

        sys_power_shutdown();
        //spec_uart_send(create_packet(voice,key_sound),PACKET_HLC_LEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(REC_POWER_UP_BTN)
.ontouch = rec_rec_power_up_ontouch,
};
#endif

void hide_show_lock()
{
    ui_hide(ANI_LOCK_LAYER);
    ui_show(ENC_LAY_BACK);
    ui_show(ENC_LAY_HOME_PAGE);
    ani_flag = 0;
}

/***************************** 关锁按键 ************************************/
static int rec_rec_lock_off_ontouch(void *ctr, struct element_touch_event *e)
{
    u8 command_buf,data_buf;
    UI_ONTOUCH_DEBUG("**rec_rec_lock_off_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ani_flag = 1;
        ui_hide(ENC_LAY_BACK);
        ui_show(ANI_LOCK_LAYER);
        sys_timeout_add(NULL, hide_show_lock, 3000);
        command_buf = voice;
        data_buf = locked;
        uart_send_package(command_buf,&data_buf,1);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(RES_LOCK_BTN)
.ontouch = rec_rec_lock_off_ontouch,
};

/***************************** 铃声按键 ************************************/
static int rec_rec_ling_up_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_rec_ling_up_ontouch**");
    static u8 tmp = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        printf("========= tmp:%d\n",tmp);

        ui_pic_show_image_by_id(PIC_BAT_REC,tmp);
        tmp++;
        if(tmp == 20){
            tmp = 0;
        }
        u8 command_buf = voice;
        u8 data_buf[] = {door_bell};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(RES_LING_BTN)
.ontouch = rec_rec_ling_up_ontouch,
};


extern u8 sys_lock_time;
/***************************** 系统锁定页面 ************************************/
static int rec_enc_system_lock_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
//        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
//        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_SHOW:
        
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SYSTEM_LOCK)
.onchange = rec_enc_system_lock_onchange,
};

/***************************** 系统锁定按键 ************************************/
static int rec_system_lock_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_system_lock_ontouch**");
    u8 command_buf,data_buf;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:

        command_buf = voice;
        data_buf = system_locked;
        uart_send_package(command_buf,&data_buf,1);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYSTEM_LOCK_BTN)
.ontouch = rec_system_lock_ontouch,
};

void time_blink()
{
    static u8 ti = 0;
    ti = !ti;
    if(ti){
        ui_hide(PIC_TIME_MM);
    }else{
        ui_show(PIC_TIME_MM);
    }
    ui_show(ENC_UP_LAY);
}

/***************************** 主页 ************************************/
static int rec_enc_back_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        enc_back_flag = 1;
        break;
    case ON_CHANGE_RELEASE:
        enc_back_flag = 0;
        break;
    case ON_CHANGE_FIRST_SHOW:

        if(tim_handle){
            sys_timer_del(tim_handle);
        }
        tim_handle = sys_timer_add(NULL, time_blink, 1000);
        break;
    case ON_CHANGE_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_BACK)
.onchange = rec_enc_back_lay_onchange,
};

/***************************** 顶部UI界面 ************************************/
static int rec_enc_up_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
        if(enc_back_flag){
            //ui_show(REC_POWER_UP_BTN);
            //ui_show(REC_POWER_UP_PIC);
        }else{
            //ui_hide(REC_POWER_UP_BTN);
            //ui_hide(REC_POWER_UP_PIC);
        }

        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_UP_LAY)
.onchange = rec_enc_up_lay_onchange,
};


/***************************** 门锁配置界面 ************************************/
static int rec_enc_door_lock_configuration_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:

        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_DOOR_LOCK_PAGE)
.onchange = rec_enc_door_lock_configuration_onchange,
};

void erase_and_reset()
{
    erase_flash(0xFF);
    db_reset();
}

void factory_reset_progressbar()
{
    ani_flag = 1;
    static u8 persent = 0;
    persent += 1;
    ui_slider_set_persent_by_id(SYS_FACTORY_RESET_PROBAR,persent);
    if(persent == 100)
    {
        persent = 0;
        ui_hide(SYS_FACTORY_RESET_LAY);
        ui_show(SYS_INFO_LIST);
        ani_flag = 0;
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
    }
}

/***************************** 恢复出厂进度条页面 ************************************/
static int factory_progressbar_increase_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("start reset factory...\n");
        user_count = 0;
        user_key_num = 0;
        visit_global_count = 0;
        sys_timeout_add(NULL,erase_and_reset,10);

        for(int i = 0; i < 100; i++)
        {
            sys_timeout_add(NULL,factory_reset_progressbar,10);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_FACTORY_RESET_LAY)
.onchange = factory_progressbar_increase_onchange,
};


/***************************** 恢复出厂弹窗 ************************************/
static int rec_enc_factory_reset_win_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_FACTORY_RESET_WIN)
.onchange = rec_enc_factory_reset_win_onchange,
};

/***************************** 恢复出厂 取消按键 ************************************/
static int rec_factory_reset_cancel_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_factory_reset_cancel_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(SYS_FACTORY_RESET_WIN);
        ui_show(SYS_INFO_LIST);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_RESET_CANCEL_BTN)
.ontouch = rec_factory_reset_cancel_ontouch,
};


/***************************** 恢复出厂 确认按键 ************************************/
static int rec_factory_reset_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_factory_reset_confirm_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:

        ui_hide(SYS_FACTORY_RESET_WIN);
        ui_show(SYS_FACTORY_RESET_LAY);

        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_RESET_CONFIRM_BTN)
.ontouch = rec_factory_reset_confirm_ontouch,
};




/***************************** 进入恢复出厂按键 ************************************/
static int rec_goto_factory_reset_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_factory_reset_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
        ui_show(SYS_FACTORY_RESET_WIN);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_GOTO_FACTORY_RESET_BTN)
.ontouch = rec_goto_factory_reset_ontouch,
};

/***************************** 关于设备页面 ************************************/
static int rec_about_device_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
        ui_show(ENC_UP_LAY);
        ui_text_set_str_by_id(DEVICE_HARDWARE_VERSION,"ascii",HARDWARE_VERSION);
        ui_text_set_str_by_id(DEVICE_SOFTWARE_VERSION,"ascii",SOFTWARE_VERSION);
        ui_text_set_str_by_id(DEVICE_IDENTIFICATION_CODE,"ascii",IDENTIFICATION_CODE);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_ABOUT_DEVICE_PAGE)
.onchange = rec_about_device_onchange,
};


/***************************** 进入关于设备按键 ************************************/
static int rec_goto_about_device_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_about_device_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        sys_info_flag = 1;
        ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
        ui_hide(SYS_INFO_LIST);
        ui_show(SYS_ABOUT_DEVICE_PAGE);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_GOTO_ABOUT_DEVICE_BTN)
.ontouch = rec_goto_about_device_ontouch,
};


/***************************** 自检页面 ************************************/
static int rec_auto_check_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    case ON_CHANGE_SHOW:
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_AUTO_CHECK_PAGE)
.onchange = rec_auto_check_onchange,
};


/***************************** 进入系统自检按键 ************************************/
static int rec_goto_auto_check_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_auto_check_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        memset(auto_check_status,0,sizeof(auto_check_status));
        sys_info_flag = 2;
        auto_check_flag = 0;
        ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
        ui_hide(SYS_INFO_LIST);
        ui_show(SYS_AUTO_CHECK_PAGE);


        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_GOTO_AUTO_CHECK_BTN)
.ontouch = rec_goto_auto_check_ontouch,
};

void auto_check_progressbar()
{
    ani_flag = 1;
    static u8 persent = 0;
    persent += 1;
    ui_slider_set_persent_by_id(SYS_CHECK_PROGRESS_BAR,persent);

    ui_pic_show_image_by_id(SYS_CHECK_PROBAR_PERSENT_1,persent/100);
    ui_pic_show_image_by_id(SYS_CHECK_PROBAR_PERSENT_2,persent/10%10);
    ui_pic_show_image_by_id(SYS_CHECK_PROBAR_PERSENT_3,persent%10);
    if(persent == 100)
    {
        persent = 0;
        ui_text_show_index_by_id(SYS_AUTO_CHECK_TXT_1,auto_check_flag);
        ani_flag = 0;
        u8 command_buf = voice;
        u8 data_buf[] = {operate_success};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
    }
}

/***************************** 开始自检按钮 ************************************/
static int sys_check_start_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys_check_start_btn_ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        ani_flag = 1;
        auto_check_flag = !auto_check_flag;
        if(auto_check_flag)
        {
            for(int i = 0; i < 100; i++)
            {
                sys_timeout_add(NULL,auto_check_progressbar,10);
            }
        }
        else
        {
            sys_info_flag = 6;
            ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
            ui_text_show_index_by_id(SYS_AUTO_CHECK_TXT_1,auto_check_flag);
            ui_hide(SYS_AUTO_CHECK_PAGE);
            ui_show(SYS_CHECK_DETAIL_PAGE);
        }
        u8 command_buf = device_request;
        u8 data_buf[] = {0x01};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_CHECK_START_BTN)
.ontouch = sys_check_start_btn_ontouch,
};


/***************************** 开机自检详情界面 ************************************/

static int power_on_check_detail_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("power_on_check_detail_onchange");
        put_buf(device_status, 7);
        if(!device_status[0]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_1_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_1_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_1,0);
        } else if(device_status[0] == 1){
            printf("fingerprint check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_1_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_1_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_1,1);
        }else if(device_status[0] == 2){
            printf("fingerprint not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_1_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_1_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_1,2);
        }

        if(!device_status[1]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_2_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_2_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_2,0);
        } else if(device_status[1] == 1){
            printf("nfc check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_2_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_2_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_2,1);
        }else if(device_status[1] == 2){
            printf("nfc not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_2_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_2_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_2,2);
        }

        if(!device_status[2]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_3_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_3_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_3,0);
        } else if(device_status[2] == 1){
            printf("face check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_3_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_3_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_3,1);
        }else if(device_status[2] == 2){
            printf("face not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_3_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_3_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_3,2);
        }

        if(!device_status[3]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_4_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_4_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_4,0);
        } else if(device_status[3] == 1) {
            printf("radar check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_4_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_4_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_4,1);
        }else if(device_status[3] == 2){
            printf("radar not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_4_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_4_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_4,2);
        }

        if(!device_status[4]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_5_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_5_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_5,0);
        } else if(device_status[4] == 1){
            printf("electrical machinery check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_5_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_5_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_5,1);
        }else if(device_status[4] == 2){
            printf("electrical machinery not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_5_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_5_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_5,2);
        }

        if(!device_status[5]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_11_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_11_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_11,0);
        } else if(device_status[5] == 1){
            printf("touch check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_11_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_11_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_11,1);
        }else if(device_status[5] == 2){
            printf("touch not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_11_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_11_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_11,2);
        }

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_DEVICE_STATUS)
.onchange = power_on_check_detail_onchange,
};



/***************************** 系统信息 自检详情界面 ************************************/

static int rec_sys_check_detail_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
        printf("rec_sys_check_detail_onchange");
        ui_show(ENC_UP_LAY);
        if(!auto_check_status[0]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_6_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_6_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_6,0);
        } else if(auto_check_status[0] == 1){
            printf("fingerprint check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_6_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_6_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_6,1);
        } else if(auto_check_status[0] == 2){
            printf("fingerprint not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_6_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_6_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_6,2);
        }

        if(!auto_check_status[1]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_7_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_7_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_7,0);
        } else if(auto_check_status[1] == 1){
            printf("nfc check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_7_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_7_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_7,1);
        }else if(auto_check_status[1] == 2){
            printf("nfc not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_7_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_7_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_7,2);
        }

        if(!auto_check_status[2]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_8_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_8_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_8,0);
        } else if(auto_check_status[2] == 1){
            printf("face check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_8_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_8_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_8,1);
        }else if(auto_check_status[2] == 2){
            printf("face not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_8_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_8_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_8,2);
        }

        if(!auto_check_status[3]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_9_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_9_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_9,0);
        } else if(auto_check_status[3] == 1) {
            printf("radar check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_9_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_9_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_9,1);
        }else if(auto_check_status[3] == 2){
            printf("radar not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_9_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_9_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_9,2);
        }

        if(!auto_check_status[4]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_10_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_10_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_10,0);
        } else if(auto_check_status[4] == 1){
            printf("electrical machinery check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_10_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_10_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_10,1);
        }else if(auto_check_status[4] == 2){
            printf("electrical machinery not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_10_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_10_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_10,2);
        }

        if(!auto_check_status[5]) {
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_12_N);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_12_Y);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_12,0);
        } else if(auto_check_status[5] == 1){
            printf("touch check fail");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_12_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_12_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_12,1);
        }else if(auto_check_status[5] == 2){
            printf("touch not suport");
            ui_hide(SYS_CHECK_DETAIL_SYMBOL_12_Y);
            ui_show(SYS_CHECK_DETAIL_SYMBOL_12_N);
            ui_text_show_index_by_id(SYS_CHECK_DETAIL_TXT_12,2);
        }

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_CHECK_DETAIL_PAGE)
.onchange = rec_sys_check_detail_onchange,
};



/***************************** 开始老化测试按钮 ************************************/
static int rec_again_test_start_btn_ontouch(void *ctr, struct element_touch_event *e)
{
    static u8 test_btn = 0;
    UI_ONTOUCH_DEBUG("**rec_again_test_start_btn_ontouch**");

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        test_btn = !test_btn;
        ui_text_show_index_by_id(SYS_AGAIN_TEST_TXT_1,test_btn);
        ui_text_show_index_by_id(SYS_AGAIN_TEST_TXT_2,test_btn);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_AGAIN_TEST_BTN)
.ontouch = rec_again_test_start_btn_ontouch,
};



/***************************** 老化测试界面 ************************************/
static int rec_sys_again_test_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_show(ENC_UP_LAY);
        ui_pic_show_image_by_id(SYS_AGAIN_TEST_NUM_4,0);
        ui_pic_show_image_by_id(SYS_AGAIN_TEST_NUM_3,0);
        ui_pic_show_image_by_id(SYS_AGAIN_TEST_NUM_2,0);
        ui_pic_show_image_by_id(SYS_AGAIN_TEST_NUM_1,0);
        ui_text_set_str_by_id(SYS_AGAIN_TEST_UNLOCK_TXT,"ascii","0");
        ui_text_set_str_by_id(SYS_AGAIN_TEST_LOCK_TXT,"ascii","0");
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_AGAIN_TEST_PAGE)
.onchange = rec_sys_again_test_onchange,
};



/***************************** 进入老化测试按键 ************************************/
static int rec_goto_again_test_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_again_test_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        sys_info_flag = 3;
        ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
        ui_hide(SYS_INFO_LIST);
        ui_show(SYS_AGAIN_TEST_PAGE);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_GOTO_AGAIN_TEST_BTN)
.ontouch = rec_goto_again_test_ontouch,
};

#if 0
/***************************** 进入出厂测试按键 ************************************/
static int rec_goto_final_test_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_final_test_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        sys_info_flag = 4;
        ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
        ui_hide(SYS_INFO_LIST);
        //ui_show(SYS_FINAL_TEST_PAGE);

    /*
    u8 command_buf = voice;
    u8 data_buf[] = {key_sound};
    uart_send_package(command_buf,command_buf,ARRAY_SIZE(command_buf));
    */
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_GOTO_FINAL_TEST_BTN)
.ontouch = rec_goto_final_test_ontouch,
};
#endif

/***************************** 进入状态信息按键 ************************************/
static int rec_goto_status_infor_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec_goto_status_infor_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        sys_info_flag = 5;
        ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
        ui_hide(SYS_INFO_LIST);
        ui_show(SYS_STATUS_INFOR_PAGE);

        u8 command_buf = voice;
        u8 data_buf[] = {key_sound};
        uart_send_package(command_buf,data_buf,ARRAY_SIZE(data_buf));

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_GOTO_STATUS_INFOR_BTN)
.ontouch = rec_goto_status_infor_ontouch,
};

char num_to_str[4][3] = {0};
/***************************** 系统信息 状态信息界面 ************************************/
static int rec_sys_status_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 num = 0;
    switch (e) {
    case ON_CHANGE_INIT:

        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_show(ENC_UP_LAY);
        memset(num_to_str[0],0,sizeof(num_to_str[0]));
        sprintf(num_to_str[0],"%d",user_count);
        printf("SYS_INFOR_USER_NUM_TXT_1 %s\n",num_to_str[0]);
        ui_text_set_str_by_id(SYS_INFOR_USER_NUM_TXT_1, "ascii", num_to_str[0]);

        ui_text_set_str_by_id(SYS_INFOR_USER_NUM_TXT_2, "ascii", "50");

        memset(num_to_str[1],0,sizeof(num_to_str[1]));
        num = MAX_USER_NUM - user_count;
        sprintf(num_to_str[1],"%d",num);
        printf("SYS_INFOR_USER_NUM_TXT_3 %s\n",num_to_str[1]);
        ui_text_set_str_by_id(SYS_INFOR_USER_NUM_TXT_3, "ascii", num_to_str[1]);

        memset(num_to_str[2],0,sizeof(num_to_str[2]));
        sprintf(num_to_str[2],"%d",user_key_num);
        printf("SYS_INFOR_USER_NUM_TXT_4 %s\n",num_to_str[2]);
        ui_text_set_str_by_id(SYS_INFOR_USER_NUM_TXT_4, "ascii", num_to_str[2]);


        ui_text_set_str_by_id(SYS_INFOR_USER_NUM_TXT_5, "ascii", "250");

        memset(num_to_str[3],0,sizeof(num_to_str[3]));
        num = MAX_USER_NUM*MAX_KEY_NUM - user_key_num;
        sprintf(num_to_str[3],"%d",num);
        printf("SYS_INFOR_USER_NUM_TXT_6 %s\n",num_to_str[3]);
        ui_text_set_str_by_id(SYS_INFOR_USER_NUM_TXT_6, "ascii", num_to_str[3]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_STATUS_INFOR_PAGE)
.onchange = rec_sys_status_infor_onchange,
};


/***************************** 系统信息 列表界面 ************************************/
static int rec_sys_infor_list_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:

        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        ui_text_show_index_by_id(SYS_INFO_TXT,sys_info_flag);
        break;
    case ON_CHANGE_SHOW:
        ani_flag = 0;
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_INFO_LIST)
.onchange = rec_sys_infor_list_onchange,
};




/***************************** 系统信息界面 ************************************/
static int rec_enc_sys_infor_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:

        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_SHOW:
        ui_show(ENC_UP_LAY);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_SYS_INFO_PAGE)
.onchange = rec_enc_sys_infor_onchange,
};


static int rec_enc_lay_set_move_ontouch(void *ctr, struct element_touch_event *e)
{
    static s16 x_pos_down = 0;
    s16 x_pos_now = 0;
    s16 x_pos_ch = 0;
    static int pos = 0;
    static int tmp = 0;
    int move_point = 0;
    u8 command_buf,data_buf;
    UI_ONTOUCH_DEBUG("**rec_enc_lay_set_move_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        pos = e->pos.x;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        icon_page_turning(move_posx(pos, e->pos.x));

        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("**ELM_EVENT_TOUCH_UP**");
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_SET_MOVE_BTN)
.ontouch = rec_enc_lay_set_move_ontouch,
};


/***************************** 设置主界面 ************************************/
static int rec_enc_set_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:

        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        lock_on = 0;
        ui_show(ENC_UP_LAY);
//        ui_paint_brush_set(BRUSH_CIRCLE,0x05,0);
//        ui_paint_circle(ENC_LAY_PAGE,200,350,100);
        if(page_pic_flag)
        {
            ui_show(ENC_PAGE_LEFT_BTN);
            ui_hide(ENC_PAGE_RIGHT_BTN);
            ui_hide(ENC_LAY_BTN_3);
            ui_hide(ENC_LAY_BTN_4);
            ui_show(ENC_LAY_TXT1_PAGE);
            //ui_text_show_index_by_id(ENC_LAY_PAGE_TXT1,page_pic_flag);
            //ui_text_show_index_by_id(ENC_LAY_PAGE_TXT2,page_pic_flag);
        }
        else
        {
            ui_show(ENC_PAGE_RIGHT_BTN);
            ui_hide(ENC_PAGE_LEFT_BTN);
            ui_show(ENC_LAY_BTN_3);
            ui_show(ENC_LAY_BTN_4);
        }
        break;
    case ON_CHANGE_SHOW:
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ENC_LAY_PAGE)
.onchange = rec_enc_set_lay_onchange,
};


#endif
