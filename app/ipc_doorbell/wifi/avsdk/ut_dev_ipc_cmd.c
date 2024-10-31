/*****************************************************************************
 * ut_ipc_cmd.c
 *
 ****************************************************************************/

/*****************************************************************************
 * Included Files
 ****************************************************************************/
#define _STDIO_H_
#include "app_config.h"
#include "fs.h"
#include "app_core.h"
#include "system/database.h"
#include "hi3861l_task.h"
#include "os/os_compat.h"
#include "doorbell_event.h"
#include "action.h"

#include "xciot_api.h"
#include <stdbool.h>

#if ENABLE_DEMO_AV > 1
#include "example_ctx.h"
#endif

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>




/*****************************************************************************
 * Trace Definitions
 ****************************************************************************/

#define EVOS_LOG_COMPILE_LEVEL EVOS_LOG_LEVEL_DEBUG
#define EVOS_LOG_RUN_LEVEL     EVOS_LOG_LEVEL_DEBUG
#define EVOS_LOG_DOMAIN        "ut_dev_ipc_cmd.c"
#include "xc_log.h"



#ifdef ENABLE_OSAL
#include "osal/osal_api.h"
#endif

/*****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define OSAL_POINTER_TO_UINT(x) ((uintptr_t)(x))
#define OSAL_UINT_TO_POINTER(x) ((void *)(uintptr_t)(x))
#define OSAL_POINTER_TO_INT(x) ((intptr_t)(x))
#define OSAL_INT_TO_POINTER(x) ((void *)(intptr_t)(x))

/*****************************************************************************
 * Private Types
 ****************************************************************************/
#define USE_CONTINUOUS_VIDEO 0
/*****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/*****************************************************************************
 * Private Data
 ****************************************************************************/

/*****************************************************************************
 * Public Data
 ****************************************************************************/
#if 0
extern pthread_ctx_t thr_hv_ctx;
extern char g_history_h264_file[2048];
extern void *video_history_thread(void *);
#endif


extern void set_utc_ms(uint64_t utc_ms);
/*****************************************************************************
 * Private Functions
 ****************************************************************************/

#if 1

#if 0
struct test {
    char wifi_ssid[50];
    char wifi_key[50];
    int32_t video_qos;
    int32_t video_speed;
    int32_t flip;  //翻转信息: 1: Upright, 2: Flip Horizontal，3 :Flip Vertical，4: turn 180
    uint32_t volume_value;

    int32_t  history_plan_record_type;           // 录像类型: 1, 连续录像; 2, 事件录像
    xciot_Timetask  history_plan_tt[3];             // 时间段(最多3个)
    int32_t   history_plan_plan_count;               // 个数
    int32_t   history_plan_enable;                    //  是否开启录像；0：disable;1 enable

    int32_t  led_mode;            //  LED状态: 1 常开；2 常关；
    int32_t  ircut_mode;          // IRCut状态: 1 常开；2 常关； 3 自动
    int32_t  secret_mode;         // 私密模式: 0 不开启私有模式; 1 开始私有模式
    ipc_ConfigGet_NotifyInfo  notify;          //  通知参数
    uint32_t power_freq;

    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    int32_t time_dst;                  //  是否开启夏令时: 0: disable; 1: enable
    int32_t time_offset;			// 时间偏移，分钟
    ipc_MotionzoneSet_Req_mz_t motion_mz;              //  将显示区域划分为: 22x18的区块，共需要50个字节来表示，该区块被选中，则为1，反之为0
    ipc_MotionzoneSet_XYPoint  motion_xy_point[5];              //  界面设置的坐标
    int32_t   motion_num_point;                 //  坐标的点数, 最大5个

    ipc_PspList_PspInfo  pspinfo_arr[5];

    int32_t  Timedcruise_states;                //  status: 0 disable; 1 enable;
    int32_t  Timedcruise_mode;                  //  巡航模式: 1, 完整巡航; 2, 预置点巡航;
    xciot_Timetask Timedcruise_tt;                //  时间参数
    int32_t  Timedcruise_interval;               // 巡航时间间隔,单位秒；取值范围: 600-43200 之间的数据
    int64_t  autotrack;                         //自动跟踪: 0, 关闭; 1, 开启，

    int32_t motion_detection;
    int32_t opensound_detection;
    int32_t smoke_detection;
    int32_t shadow_detection;

    uint32_t  event_record_duration; //录像时间长度

};
#endif

#if 0
struct test g_test_t;

void cmd_test_init(void)
{

    memset(&g_test_t, 0, sizeof(g_test_t));
    strcpy(g_test_t.wifi_ssid, "ssid");
    strcpy(g_test_t.wifi_key, "wifi_key");
    g_test_t.video_qos = 15;
    g_test_t.flip = 1;
    g_test_t.volume_value = 2;
    g_test_t.history_plan_record_type = 1;

    g_test_t.history_plan_plan_count = 0;
    g_test_t.history_plan_enable = 1;
    g_test_t.led_mode = 1;
    g_test_t.ircut_mode = 1;
    g_test_t.secret_mode = 0;

    g_test_t.power_freq = 50;
    strcpy(g_test_t.time_zone, "UTC-8");
    g_test_t.time_dst = 1;
    g_test_t.notify.states = 1;
    g_test_t.notify.level = 1;
    g_test_t.motion_mz.bytes[0] = 1;

    g_test_t.motion_num_point = 3;
    for (int i = 0; i < g_test_t.motion_num_point; i++) {
        g_test_t.motion_xy_point[i].leftup_x = 20 + i;
        g_test_t.motion_xy_point[i].leftup_y = i;
        g_test_t.motion_xy_point[i].rightdown_x = 10 + i;
        g_test_t.motion_xy_point[i].rightdown_y = 5 + i;
    }

    g_test_t.Timedcruise_states = 1;
    g_test_t.motion_detection = 1;
    g_test_t.smoke_detection = 1;
    g_test_t.shadow_detection = 1;
    g_test_t.event_record_duration = 2;
}
#endif // 0

const unsigned char utf8JLML_skill[] = {
    0x7B, 0x22, 0x62, 0x61, 0x73, 0x65, 0x22, 0x3A, 0x7B, 0x22, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x63,
    0x6F, 0x6C, 0x22, 0x3A, 0x22, 0x70, 0x70, 0x72, 0x70, 0x63, 0x22, 0x2C, 0x22, 0x65, 0x6E, 0x63,
    0x6F, 0x64, 0x65, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x65, 0x6E, 0x63, 0x72, 0x79, 0x70, 0x74, 0x22,
    0x3A, 0x33, 0x2C, 0x22, 0x73, 0x65, 0x63, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x68, 0x62, 0x5F, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x76, 0x61, 0x6C, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x6F, 0x66, 0x66, 0x6C, 0x69, 0x6E, 0x65, 0x5F, 0x63, 0x6D, 0x64, 0x22, 0x3A, 0x31, 0x2C,
    0x22, 0x6C, 0x65, 0x64, 0x22, 0x3A, 0x32, 0x2C, 0x22, 0x62, 0x61, 0x74, 0x74, 0x65, 0x72, 0x79,
    0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x65, 0x6E, 0x64, 0x5F, 0x6D, 0x6F, 0x64, 0x65, 0x22, 0x3A,
    0x30, 0x2C, 0x22, 0x61, 0x70, 0x73, 0x65, 0x63, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x65, 0x76, 0x65,
    0x6E, 0x74, 0x73, 0x22, 0x3A, 0x5B, 0x38, 0x33, 0x2C, 0x33, 0x5D, 0x2C, 0x22, 0x74, 0x61, 0x6D,
    0x70, 0x65, 0x72, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x79, 0x6E, 0x63, 0x5F, 0x6E, 0x61, 0x6D,
    0x65, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x74, 0x68, 0x72, 0x65, 0x73, 0x68, 0x6F, 0x6C, 0x64, 0x22,
    0x3A, 0x30, 0x2C, 0x22, 0x66, 0x69, 0x6C, 0x65, 0x5F, 0x66, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64,
    0x69, 0x6E, 0x67, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x72, 0x65, 0x62, 0x6F, 0x6F, 0x74, 0x5F, 0x74,
    0x69, 0x6D, 0x65, 0x22, 0x3A, 0x36, 0x30, 0x7D, 0x2C, 0x22, 0x6D, 0x63, 0x6F, 0x6E, 0x66, 0x22,
    0x3A, 0x5B, 0x7B, 0x22, 0x63, 0x6C, 0x61, 0x73, 0x73, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x22, 0x3A,
    0x22, 0x49, 0x50, 0x41, 0x56, 0x22, 0x2C, 0x22, 0x63, 0x6F, 0x6E, 0x66, 0x22, 0x3A, 0x7B, 0x22,
    0x73, 0x70, 0x65, 0x63, 0x22, 0x3A, 0x33, 0x2C, 0x22, 0x61, 0x75, 0x64, 0x69, 0x6F, 0x22, 0x3A,
    0x32, 0x2C, 0x22, 0x6D, 0x69, 0x63, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x70, 0x65, 0x61, 0x6B,
    0x65, 0x72, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x69, 0x72, 0x65, 0x6E, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x70, 0x69, 0x78, 0x65, 0x6C,
    0x22, 0x3A, 0x5B, 0x34, 0x2C, 0x32, 0x2C, 0x31, 0x5D, 0x2C, 0x22, 0x70, 0x69, 0x78, 0x65, 0x6C,
    0x5F, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x22, 0x3A, 0x5B, 0x34, 0x5D, 0x2C, 0x22, 0x73, 0x64, 0x63,
    0x61, 0x72, 0x64, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x61, 0x73, 0x70, 0x65, 0x63, 0x74, 0x5F, 0x72,
    0x61, 0x74, 0x69, 0x6F, 0x22, 0x3A, 0x22, 0x34, 0x3A, 0x33, 0x22, 0x2C, 0x22, 0x6D, 0x6F, 0x74,
    0x69, 0x6F, 0x6E, 0x7A, 0x6F, 0x6E, 0x65, 0x73, 0x22, 0x3A, 0x32, 0x2C, 0x22, 0x70, 0x74, 0x7A,
    0x22, 0x3A, 0x30, 0x2C, 0x22, 0x66, 0x61, 0x63, 0x65, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x63, 0x6C,
    0x6F, 0x75, 0x64, 0x73, 0x22, 0x3A, 0x32, 0x2C, 0x22, 0x7A, 0x6F, 0x6F, 0x6D, 0x22, 0x3A, 0x31,
    0x2C, 0x22, 0x61, 0x69, 0x5F, 0x76, 0x65, 0x6E, 0x64, 0x6F, 0x72, 0x22, 0x3A, 0x30, 0x2C, 0x22,
    0x70, 0x69, 0x72, 0x22, 0x3A, 0x7B, 0x22, 0x6E, 0x75, 0x6D, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x72,
    0x61, 0x6E, 0x67, 0x69, 0x6E, 0x67, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x76, 0x61, 0x6C, 0x75, 0x65,
    0x73, 0x22, 0x3A, 0x5B, 0x30, 0x2C, 0x31, 0x2C, 0x32, 0x2C, 0x33, 0x5D, 0x7D, 0x2C, 0x22, 0x66,
    0x6C, 0x69, 0x70, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x6F, 0x73, 0x64, 0x22, 0x3A, 0x30, 0x2C, 0x22,
    0x70, 0x73, 0x70, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x63, 0x72, 0x75, 0x69, 0x73, 0x65, 0x22, 0x3A,
    0x30, 0x2C, 0x22, 0x73, 0x6F, 0x75, 0x6E, 0x64, 0x5F, 0x64, 0x65, 0x74, 0x65, 0x63, 0x74, 0x22,
    0x3A, 0x31, 0x2C, 0x22, 0x61, 0x75, 0x64, 0x69, 0x6F, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x63, 0x22,
    0x3A, 0x30, 0x2C, 0x22, 0x76, 0x69, 0x64, 0x65, 0x6F, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x63, 0x22,
    0x3A, 0x30, 0x2C, 0x22, 0x67, 0x70, 0x73, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x6C, 0x6F, 0x63, 0x61,
    0x6C, 0x5F, 0x68, 0x74, 0x74, 0x70, 0x64, 0x6F, 0x77, 0x6E, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x72,
    0x65, 0x6D, 0x6F, 0x74, 0x65, 0x5F, 0x61, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x6E, 0x69, 0x67, 0x68, 0x74, 0x5F, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x72, 0x65, 0x63, 0x6F, 0x72, 0x64, 0x5F, 0x6D, 0x6F, 0x64, 0x65, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x70, 0x6F, 0x77, 0x65, 0x72, 0x5F, 0x66, 0x72, 0x65, 0x71, 0x22, 0x3A, 0x5B, 0x5D, 0x2C,
    0x22, 0x6D, 0x6F, 0x74, 0x69, 0x6F, 0x6E, 0x5F, 0x67, 0x72, 0x69, 0x64, 0x5F, 0x73, 0x63, 0x61,
    0x6C, 0x65, 0x22, 0x3A, 0x22, 0x34, 0x3A, 0x33, 0x22, 0x2C, 0x22, 0x72, 0x65, 0x6E, 0x64, 0x65,
    0x72, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x63, 0x68, 0x61, 0x6E, 0x73, 0x22, 0x3A, 0x31, 0x2C, 0x22,
    0x64, 0x65, 0x63, 0x5F, 0x6D, 0x69, 0x78, 0x22, 0x3A, 0x31, 0x7D, 0x7D, 0x5D, 0x7D, 0x00
};


bool dev_on_ipc_Discovery(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_Discovery_Req *req,
    /* output  */ ipc_Discovery_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("conn[%d]ipc_Discovery_Req:", conn_id);
    T_T("\t did       :%s", req->did);
    T_T("\t class_code:%s", req->class_code);

    /*注意，该接口只需填入mode、ipaddr、netmask三个参数，其他内部复写填充*/

    /*设备模式(wifi)： 1 AP 模式; 2 STA模式;*/
    rsp->mode = 2;

    /*需要用户填写真实的ip和netmask*/
    char ip[16];
    Get_IPAddress(1, ip);
    snprintf(rsp->ipaddr, sizeof(rsp->ipaddr), "%s", ip);
    snprintf(
        rsp->netmask, sizeof(rsp->netmask), "%s", "255.255.255.0");
    /* 设备特征信息，在web后台下载，每类设备唯一，
     * 需要无外网访问功能的必须填写
     */
    //从内存中读取,skill_buff使用文本工具，将"替换成\"即可
    rsp->skill.size = strlen(utf8JLML_skill);
    memcpy(rsp->skill.bytes, utf8JLML_skill, rsp->skill.size);
    T_TH_HEX("skill json", rsp->skill.bytes, rsp->skill.size);


    return retval;
}

bool dev_on_ipc_WifiAPGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_WifiAPGet_Req *req,
    /* output  */ ipc_WifiAPGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_WifiAPGet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

//    struct WIFI_LIST_INFO *get_wifi_list_info(void);
//    struct WIFI_LIST_INFO *wifi_list_info = get_wifi_list_info();
//    extern int get_wifi_list_info_cnt(void);
//    // todo 填充设备扫描到wifi列表
//
//    rsp->support = 1;
//    rsp->wifis_count = 0;
//    for (int i = 0; i < get_wifi_list_info_cnt(); i++) {
//        if (strlen(wifi_list_info[i].ssid)) {
//            rsp->wifis[rsp->wifis_count].qos  = wifi_list_info[i].qos;
//            strcpy(rsp->wifis[rsp->wifis_count].ssid, wifi_list_info[i].ssid);
//            //T_D("wifi_info->info_arr_[%d].ssid_ :%s ", i, rsp->wifis[i].ssid);
//            rsp->wifis_count++;
//        }
//    }
//
//    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_WIFI_LIST);

    return retval;
}

bool dev_on_ipc_WifiSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiSet_Req *req,
    /* output  */ ipc_WifiSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_WifiSet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);
    T_I("\t ssid   :%s", req->ssid);
    T_I("\t pwd    :%s", req->pwd);

    //@todo 退出AP模式，去连接下发wifi网络

    set_net_cfg_info(req->ssid, req->pwd, NULL);

    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_AP_CHANGE_STA);


    return retval;
}

bool dev_on_ipc_WifiGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiGet_Req *req,
    /* output  */ ipc_WifiGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_WifiGet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);
#if 0
    //todo填充wifi信息
    if (strlen(g_test_t.wifi_ssid)) {
        strcpy(rsp->ssid, g_test_t.wifi_ssid);
    } else {
        strcpy(rsp->ssid, "wifi_ssid");
    }
#else
    strcpy(rsp->ssid, "wifi_ssid");
#endif
    rsp->qos = -60;
    rsp->support = 1;


    return retval;
}

bool dev_on_ipc_VideoPlay(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VideoPlay_Req *req,
    /* output  */ ipc_VideoPlay_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;



    /*将连接ID添加到视频播放用户组*/
    avsdk_video_add_conn(conn_id, req->channel);


//    video0_rec_get_iframe();

    //todo填充视频信息
    rsp->qos = db_select("vqua");
    rsp->fps = 25;
    rsp->format = E_IOT_MEDIASTREAM_TYPE_H264;
    rsp->render = 0;

    T_I("conn[%d]ipc_VideoPlay_Req:", conn_id);
    T_I("\t channel:%d", req->channel);
    T_I("\t qos    :%d", req->qos);
    T_I("\t speed  :%d", req->speed);

    return retval;
}

bool dev_on_ipc_VideoPause(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_VideoPause_Req *req,
    /* output  */ ipc_VideoPause_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_VideoPause_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

    /*将连接ID从视频播放用户组删除*/
    avsdk_video_del_conn(conn_id, req->channel);

    return retval;
}

bool dev_on_ipc_VideoQosSet(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_VideoQosSet_Req *req,
    /* output  */ ipc_VideoQosSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_VideoQosSet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);
    T_I("\t qos    :%d", req->qos);
    T_I("\t speed  :%d", req->speed);

    //todo设置视频质量
    db_update("vqua", req->qos);
    db_flush();
    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_REOPEN_RT_STREAM);

    return retval;
}

bool dev_on_ipc_FlipSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipSet_Req *req,
    /* output  */ ipc_FlipSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    // todo 视频翻转
//	g_test_t.flip = req->flip;

    return retval;
}

bool dev_on_ipc_AudioPlay(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_AudioPlay_Req *req,
    /* output  */ ipc_AudioPlay_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_I("conn[%d]ipc_AudioPlay_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

    /*将连接ID添加到音频播放用户组*/
    avsdk_audio_add_conn(conn_id);

    //todo 填充音频参数
    rsp->bit = 16;
    rsp->code = 0;
#if (CONFIG_AUDIO_FORMAT_FOR_MIC == AUDIO_FORMAT_AAC)
    rsp->codec = E_IOT_MEDIASTREAM_TYPE_AAC;
#else
    rsp->codec = E_IOT_MEDIASTREAM_TYPE_PCM;
#endif
    rsp->track = 1;
    rsp->rate = net_video_rec_get_audio_rate();

    T_I("rsp->bit:%d", rsp->bit);
    T_I("rsp->code:%d", rsp->code);
    T_I("rsp->codec:%d", rsp->codec);
    T_I("rsp->track:%d", rsp->track);
    T_I("rsp->rate:%d", rsp->rate);
    return retval;
}

bool dev_on_ipc_AudioPause(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_AudioPause_Req *req,
    /* output  */ ipc_AudioPause_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_AudioPause_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

    /*将连接ID从音频播放用户组删除*/
    avsdk_audio_del_conn(conn_id);

    return retval;
}

bool dev_on_ipc_TalkbackPlay(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_TalkbackPlay_Req *req,
    /* output  */ ipc_TalkbackPlay_Resp *rsp)
{
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    /* retval - true: support, false: unsupport */
    bool retval = true;

    /* 返回设置支持的音频格式 */
#if (CONFIG_AUDIO_FORMAT_FOR_SPEARK == AUDIO_FORMAT_AAC)

    rsp->codec = E_IOT_MEDIASTREAM_TYPE_AAC;       	// codec type
#else
    rsp->codec = E_IOT_MEDIASTREAM_TYPE_PCM;
#endif
    rsp->rate  = CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK;               					// 8000? audio frequency
    rsp->bit   = 16;                 					// 16bit?
    rsp->track = 1;                  					// mono
    rsp->code  = 0;                  					// normal mode




    return retval;
}

bool dev_on_ipc_TalkbackPause(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_TalkbackPause_Req *req,
    /* output  */ ipc_TalkbackPause_Resp *rsp)
{
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    /* retval - true: support, false: unsupport */
    bool retval = true;

    return retval;
}

bool dev_on_ipc_HistoryPlanSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanSet_Req *req,
    /* output  */ ipc_HistoryPlanSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    T_D("%s called ", __func__);

    //todo 配置录像方式

    T_D("plan_count: %d ", req->tt_count);
    T_D("enable = %d", req->enable);
#if 0
    g_test_t.history_plan_record_type = req->record_type;
    g_test_t.history_plan_plan_count = req->tt_count;
    for (int32_t i = 0; i < req->tt_count; i++) {
        memcpy(&g_test_t.history_plan_tt[i], &req->tt[i], sizeof(req->tt[i]));
    }
    g_test_t.history_plan_enable = req->enable;
#endif
    return retval;
}

bool dev_on_ipc_HistoryPlanGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanGet_Req *req,
    /* output  */ ipc_HistoryPlanGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    //todo 返回录像方式
#if 0
    memset(rsp->tt, 0, sizeof(rsp->tt));
    rsp->record_type = g_test_t.history_plan_record_type;
    rsp->tt_count = g_test_t.history_plan_plan_count;

    for (int32_t i = 0; i < g_test_t.history_plan_plan_count; i++) {
        memcpy(&rsp->tt[i], &g_test_t.history_plan_tt[i], sizeof(g_test_t.history_plan_tt[i]));
    }
    rsp->enable = g_test_t.history_plan_enable;
#endif
    T_D("plan_count: %d ", rsp->tt_count);
    T_D("enable = %d", rsp->enable);


    return retval;
}

bool dev_on_ipc_HistoryDays(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryDays_Req *req,
    /* output  */ ipc_HistoryDays_Resp *rsp)
{

    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    /* retval - true: support, false: unsupport */
    bool retval = true;

    // todo 返回历史记录天数
    void cloud_playback_list_get_days(void *__req, void *__rsp);
    cloud_playback_list_get_days(req, rsp);

    return retval;
}

bool dev_on_ipc_HistoryDayList(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryDayList_Req *req,
    /* output  */ ipc_HistoryDayList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("----------dev_on_ipc_HistoryDayList--------");
    // todo 返回历史记录列表
    /* Fill output rsp, after this function, sdk will send rsp to remote */
    /* 录像分为两类，1.事件录像；2.全添录像,选择一种填写rsp */
    extern int cloud_playback_list_get(void *req, void *rsp);
    cloud_playback_list_get(req, rsp);

    return retval;
}

bool dev_on_ipc_HistoryPlay(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryPlay_Req *req,
    /* output  */ ipc_HistoryPlay_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_I("conn[%d]dev_on_ipc_HistoryPlay:", conn_id);
    T_I("\t channel:%d", req->channel);
    T_D("\t file_id:%lld", req->file_id);
    T_D("\t start_time:%lld", req->start_time);

    //todo 播放file_id响应的历史视频

    //sprintf(g_history_h264_file, "%llu.h264", req->file_id)

    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {

        rsp->code = 0; //成功
        //本地录像无mp4封装，声音格式只能为pcm
        rsp->codec = E_IOT_MEDIASTREAM_TYPE_PCM;
        rsp->rate = net_video_rec_get_audio_rate();
        rsp->bit = 16;
        rsp->track = 1;
        extern int cloud_playback_init(int32_t conn_id,  uint64_t file_id, int64_t start_time);
        cloud_playback_init(conn_id, req->file_id, req->start_time / 1000);
    }

    return retval;
}

bool dev_on_ipc_HistoryPause(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_HistoryPause_Req *req,
    /* output  */ ipc_HistoryPause_Resp *rsp)
{
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    T_D("----------------dev_on_ipc_HistoryPause-----------------");
    /* retval - true: support, false: unsupport */
    bool retval = true;

    extern int cloud_playback_uninit(int32_t conn_id,  uint64_t file_id);
    cloud_playback_uninit(conn_id, req->file_id);

#if ENABLE_DEMO_AV > 1
    // todo 停止播放历史视频
    if (thr_hv_ctx.init == false) {
        /* 未建立线程 */
    } else {
        thr_hv_ctx.init = false;
        thr_hv_ctx.conn_id = -1;
        thr_hv_ctx.toexit = true;
        T_D("----------------close_video_history_thread-----------------");
    }
#endif
    return retval;
}

bool dev_on_ipc_HistoryThumGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryThumGet_Req *req,
    /* output  */ ipc_HistoryThumGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    // todo 获取历史视频的缩略图

    int get_file_thm(char *name, char type, char *thm_addr, int len);

    char name[64] = {0};
    char file_name[64] = {0};
    printf("\nreq->thum_fid =%llu \n", req->thum_fid);

    extern void cloud_playback_list_get_name_for_start_time(uint64_t start_time, char *name);
    cloud_playback_list_get_name_for_start_time(req->thum_fid, name);

    if (strlen(name)) {

        snprintf(file_name, sizeof(file_name), "%s%s", get_rec_path_2(), name);
        printf("\nfile_name = %s \n", file_name);
        rsp->thum_body.size  = get_file_thm(file_name, 0, rsp->thum_body.bytes, sizeof(rsp->thum_body.bytes));
    }
#if 0
    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {
        FILE *fp = fopen("thum.jpg", "rb");
        if (fp != NULL) { // 打开文件失败
            fseek(fp, 0, SEEK_END);//定位文件指针到文件尾。
            extern int flen(FILE * file);
            rsp->thum_body.size = flen(fp); //获取文件指针偏移量，即文件大小。
            T_D("resp_thum_body_length: %d ", rsp->thum_body.size);

            fseek(fp, 0, SEEK_SET);//定位文件指针到文件头。

            fread(rsp->thum_body.bytes, 1, rsp->thum_body.size, fp);

            fclose(fp);
            T_D("thum_body data[0]: 0x%02x, data[1]: 0x%02x ", rsp->thum_body.bytes[0], rsp->thum_body.bytes[1]);
        }
    }
#endif
    return retval;
}

bool dev_on_ipc_HistoryDel(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_HistoryDel_Req *req,
    /* output  */ ipc_HistoryDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    //todo 删除对应的历史视频
    extern int cloud_playback_list_remove(void *__req, void *__rsp);
    cloud_playback_list_remove(req, rsp);
    return retval;
}



bool dev_on_ipc_ConfigGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_ConfigGet_Req *req,
    /* output  */ ipc_ConfigGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------dev_on_ipc_ConfigGet-------");
    //todo 获取所有的配置信息

    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {
        T_D("dev_on_ipc_ConfigGet");
        rsp->flip = db_select("flip");
        rsp->led_mode = db_select("stled");
        rsp->ircut_mode = db_select("ircut");

        rsp->secret_mode = db_select("secret");

        //memcpy(&rsp->notify, &g_test_t.notify, sizeof(g_test_t.notify));

        rsp->power_freq = db_select("fre");
        rsp->volume = db_select("vol");
        rsp->duration = db_select("cyc");
        printf("\n >>>>>>>>>>>> rsp->volume  = %d\n", rsp->volume);
        printf("\n rsp->duration = %d\n", rsp->duration);

    }


    return retval;
}

void demo_report_timezone(void *arg)
{
    int rc = 0;
    int32_t  conn_id = OSAL_POINTER_TO_UINT(arg);
    T_D("conn_id: %d ", conn_id);
    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    db_select_buffer(TIMEZONE_VM_INDEX, time_zone, sizeof(time_zone));
    avsdk_report_TimeZone(time_zone);

}

bool dev_on_ipc_TimeSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeSet_Req *req,
    /* output  */ ipc_TimeSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_TimeSet--------");
    T_D("req->now_time = %lld", req->now_time);
    T_D("req->dst = %d", req->dst);
    T_D("req->offset = %d", req->offset);
    T_D("req->time_zone = %s", req->time_zone);
    // todo 设置设备的时间


    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    strcpy(time_zone, req->time_zone);
    db_update_buffer(TIMEZONE_VM_INDEX, time_zone, sizeof(time_zone));
    db_update("tzone", req->offset);
    db_flush();

    set_utc_ms(req->now_time);
    thread_fork("demo_report_timezone", 8, 0x1000, 0, 0, demo_report_timezone, OSAL_UINT_TO_POINTER(conn_id));

    return retval;
}

bool dev_on_ipc_TimeGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeGet_Req *req,
    /* output  */ ipc_TimeGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_TimeGet--------");
    // todo 获取设备的时间
    rsp->now_time    = time(0);

    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    if (db_select_buffer(TIMEZONE_VM_INDEX, time_zone, sizeof(time_zone) != sizeof(time_zone))) {
        strcpy(time_zone, "Asia/Shanghai");
    }
    strcpy(rsp->time_zone, time_zone);

    rsp->dst = 0;
    rsp->offset =  db_select("tzone");

    return retval;
}

bool dev_on_ipc_LedSet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedSet_Req *req,
    /* output  */ ipc_LedSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_LedSet--------");
    // todo 设置LED模式S
    db_update("stled", req->mode);
    db_flush();

    return retval;
}

bool dev_on_ipc_LedGet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedGet_Req *req,
    /* output  */ ipc_LedGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_LedGet--------");
    //todo 获取led的模式
    rsp->mode = db_select("stled");


    return retval;
}

bool dev_on_ipc_IRCutSet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutSet_Req *req,
    /* output  */ ipc_IRCutSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    puts("\n dev_on_ipc_IRCutSet \n");
    // todo 设置夜视模式
    db_update("ircut", req->mode);
    db_flush();
    return retval;
}

bool dev_on_ipc_IRCutGet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutGet_Req *req,
    /* output  */ ipc_IRCutGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    puts("\n dev_on_ipc_IRCutGet \n");
    //todo 获取夜视模式
    rsp->mode = db_select("ircut");

    return retval;
}

bool dev_on_ipc_SecretSet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretSet_Req *req,
    /* output  */ ipc_SecretSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    puts("\n dev_on_ipc_SecretSet \n");
    // todo 设置私有模式

    db_update("secret", req->secret);
    db_flush();

    return retval;
}

bool dev_on_ipc_SecretGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretGet_Req *req,
    /* output  */ ipc_SecretGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    // todo 返回私有模式
    rsp->secret = db_select("secret");


    return retval;
}

bool dev_on_ipc_NotifySet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifySet_Req *req,
    /* output  */ ipc_NotifySet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    //todo 设置事件触发方式
#if 0
    /* Analysis input req */
    {
        g_test_t.notify.states = req->states;
        T_D("days_count_: %d ", req->tt.days_count);
        for (int i = 0; i < req->tt.days_count; i++) {
            T_D("days_[%d]: %d ", i, req->tt.days[i]);
        }

        T_D("week_day_count_: %d ", req->tt.week_day_count);
        for (int i = 0; i < req->tt.week_day_count; i++) {
            T_D("week_day_[%d]: %d ", i, req->tt.week_day[i]);
        }

        T_D("time_count_: %d ", req->tt.time_count);
        for (int i = 0; i < req->tt.time_count; i++) {
            T_D("time_info_[%d].start_sec_: %d ", i, req->tt.time[i].start_sec);
            T_D("time_info_[%d].end_sec_: %d ", i, req->tt.time[i].end_sec);
        }

        memcpy(&g_test_t.notify.tt, &req->tt, sizeof(req->tt));
        g_test_t.notify.level = req->level;
    }
#endif

    return retval;
}

bool dev_on_ipc_NotifyGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifyGet_Req *req,
    /* output  */ ipc_NotifyGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    //todo 获取事件触发方式
#if 0
    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {
        memcpy(&rsp->tt, &g_test_t.notify.tt, sizeof(g_test_t.notify.tt));
        rsp->level  = g_test_t.notify.level;
        rsp->states = g_test_t.notify.states;

        T_D("resp_level: %d ", rsp->level);
        T_D("resp_states: %d ", rsp->states);

        T_D("days_count_: %d ", rsp->tt.days_count);
        for (int i = 0; i < rsp->tt.days_count; i++) {
            T_D("days_[%d]: %d ", i, rsp->tt.days[i]);
        }

        T_D("week_day_count_: %d ", rsp->tt.week_day_count);
        for (int i = 0; i < rsp->tt.week_day_count ; i++) {
            T_D("week_day_[%d]: %d ", i, rsp->tt.week_day[i]);
        }

        T_D("time_count_: %d ", rsp->tt.time_count);
        for (int i = 0; i < rsp->tt.time_count ; i++) {
            T_D("time_info_[%d].start_sec_: %d ", i, rsp->tt.time[i].start_sec);
            T_D("time_info_[%d].end_sec_: %d ", i, rsp->tt.time[i].end_sec);
        }
    }
#endif
    return retval;
}

bool dev_on_ipc_MotionzoneSet(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneSet_Req *req,
    /* output  */ ipc_MotionzoneSet_Resp *rsp)
{
    T_I("---------dev_on_ipc_MotionzoneSet--------");
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    /* Motionzone */
    {
        T_I("---------MotionzoneSet  mz.size=%d--------", req->mz.size);
        for (int i = 0; i < req->mz.size; i++) {
            T_I("[%d]=%d", i, req->mz.bytes[i]);
        }
    }
    /* Analysis XYPoint req */
    {
        for (int i = 0; i < req->points_count; i++) {
            T_D("xy_point[%d].leftup_x: %d ", i, req->points[i].leftup_x);
            T_D("xy_point[%d].leftup_y: %d ", i, req->points[i].leftup_y);
            T_D("xy_point[%d].rightdown_x: %d ", i, req->points[i].rightdown_x);
            T_D("xy_point[%d].rightdown_y: %d ", i, req->points[i].rightdown_y);

        }

        memcpy(&g_test_t.motion_mz, &req->mz, sizeof(ipc_MotionzoneSet_Req_mz_t));
        g_test_t.motion_num_point = req->points_count;
        for (int i = 0; i < req->points_count; i++) {
            g_test_t.motion_xy_point[i].leftup_x = req->points[i].leftup_x;
            g_test_t.motion_xy_point[i].leftup_y = req->points[i].leftup_y;
            g_test_t.motion_xy_point[i].rightdown_x = req->points[i].rightdown_x;
            g_test_t.motion_xy_point[i].rightdown_y = req->points[i].rightdown_y;
        }
    }
#endif
    return retval;
}

bool dev_on_ipc_MotionzoneGet(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneGet_Req *req,
    /* output  */ ipc_MotionzoneGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    memcpy(&rsp->mz, &g_test_t.motion_mz, sizeof(ipc_MotionzoneGet_Resp_mz_t));
    rsp->points_count = g_test_t.motion_num_point;
    for (int i = 0; i < rsp->points_count; i++) {
        rsp->points[i].leftup_x = g_test_t.motion_xy_point[i].leftup_x;
        rsp->points[i].leftup_y = g_test_t.motion_xy_point[i].leftup_y;
        rsp->points[i].rightdown_x = g_test_t.motion_xy_point[i].rightdown_x;
        rsp->points[i].rightdown_y = g_test_t.motion_xy_point[i].rightdown_y;
    }
#endif

    return retval;
}

bool dev_on_ipc_PspAdd(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspAdd_Req *req,
    /* output  */ ipc_PspAdd_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    for (int index = 0; index < 5; index++) {
        T_D("g_test_t.pspinfo_arr[%d].psp_id: %d ", index, g_test_t.pspinfo_arr[index].psp_id);
        if (g_test_t.pspinfo_arr[index].psp_id == 0) {
            g_test_t.pspinfo_arr[index].psp_id = index + 1;
            memcpy(g_test_t.pspinfo_arr[index].psp_name, req->psp_name, 16);
            g_test_t.pspinfo_arr[index].is_def = req->is_def;
            break;
        }
    }
#endif

    return retval;
}

bool dev_on_ipc_PspDel(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspDel_Req *req,
    /* output  */ ipc_PspDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

#if 0
    for (int index = 0; index < 5; index++) {
        if (g_test_t.pspinfo_arr[index].psp_id == req->psp_id) {
            memset(&g_test_t.pspinfo_arr[index], 0, sizeof(ipc_PspList_PspInfo));
        }
    }
#endif

    return retval;
}

bool dev_on_ipc_PspList(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspList_Req *req,
    /* output  */ ipc_PspList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

#if 0
    int num = 0;
    for (int index = 0; index < 5; index++) {
        T_D("g_test_t.pspinfo_arr[%d].psp_id_: %d ", index, g_test_t.pspinfo_arr[index].psp_id);
        if (g_test_t.pspinfo_arr[index].psp_id != 0) {
            memcpy(&rsp->psps[num], &g_test_t.pspinfo_arr[index], sizeof(ipc_PspList_PspInfo));
            num++;
        }
    }
    rsp->psps_count = num;
#endif
    T_D("resp_pspinfo_count: %d ", rsp->psps_count);

    return retval;
}

bool dev_on_ipc_PspCall(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspCall_Req *req,
    /* output  */ ipc_PspCall_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("dev_on_ipc_PspCall");
    T_I("psp_id=%d", req->psp_id);
    return retval;
}


//需要异步处理
bool dev_on_ipc_Reboot(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_Reboot_Req *req,
    /* output  */ ipc_Reboot_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d].ipc.Reboot.channel:%d", conn_id, req->channel);


    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_RESET_CMD, 1);
    return retval;
}

//需要异步
bool dev_on_ipc_Reset(
    /* conn_id */ int32_t         conn_id,
    /* input   */ ipc_Reset_Req *req,
    /* output  */ ipc_Reset_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("conn[%d].ipc.Reset.channel:%d", conn_id, req->channel);



    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_RESET_CMD, 2);

    return retval;
}

bool dev_on_ipc_FlipGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipGet_Req *req,
    /* output  */ ipc_FlipGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

//    rsp->flip = g_test_t.flip;

    return retval;
}

bool dev_on_ipc_LanAuth(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_LanAuth_Req *req,
    /* output  */ ipc_LanAuth_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    return retval;
}

bool dev_on_ipc_TimedcruiseSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseSet_Req *req,
    /* output  */ ipc_TimedcruiseSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("dev_on_ipc_TimedcruiseSet");
#if 0
    g_test_t.Timedcruise_states = req->states;
    g_test_t.Timedcruise_mode = req->mode;
    g_test_t.Timedcruise_interval = req->interval;
    memcpy(&g_test_t.Timedcruise_tt, &req->tt, sizeof(xciot_Timetask));
#endif
    return retval;
}

bool dev_on_ipc_TimedcruiseGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseGet_Req *req,
    /* output  */ ipc_TimedcruiseGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("dev_on_ipc_TimedcruiseGet");
#if 0
    rsp->states = g_test_t.Timedcruise_states;
    rsp->mode = g_test_t.Timedcruise_mode;
    rsp->interval = g_test_t.Timedcruise_interval;
    memcpy(&rsp->tt, &g_test_t.Timedcruise_tt, sizeof(xciot_Timetask));
#endif

    return retval;
}

bool dev_on_ipc_StorageInfo(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_StorageInfo_Req *req,
    /* output  */ ipc_StorageInfo_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("------------dev_on_ipc_StorageInfo-----------");

    extern int storage_device_ready();
    if (!storage_device_ready()) {
        rsp->status = 4;
    } else {
        struct vfs_partition *part;
        part = fget_partition(CONFIG_ROOT_PATH);
        u32 cur_space;
        fget_free_space(CONFIG_ROOT_PATH, &cur_space);

        rsp->status = 0;
        rsp->total_size =  part->total_size / 1024;
        rsp->use_size = (part->total_size - cur_space) / 1024;


        T_D("total_size =%lld", rsp->total_size);
        T_D("use_size =%lld", rsp->use_size);
    }

    T_D("status =%d", rsp->status);

    return retval;
}


bool dev_on_ipc_StorageFormat(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_StorageFormat_Req *req,
    /* output  */ ipc_StorageFormat_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("--------------dev_on_ipc_StorageFormat-------------------");


    if (!storage_device_ready()) {
        return false;
    }
    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_SD_FORMAT, OSAL_UINT_TO_POINTER(conn_id));

    return retval;
}



bool dev_on_ipc_LogSet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LogSet_Req *req,
    /* output  */ ipc_LogSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("--------------dev_on_ipc_LogSet-------------------");

    T_D("channel =%s", req->channel);
    T_D("log_ipaddr =%d", req->states);

    return retval;
}

bool dev_on_ipc_VolumeSet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeSet_Req *req,
    /* output  */ ipc_VolumeSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

//	g_test_t.volume_value = req->volume;
    printf("\n >>>>>>>>>>>> req->volume  = %d\n", req->volume);
    db_update("vol", req->volume);
    db_flush();

    return retval;
}

bool dev_on_ipc_PowerFreqSet(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqSet_Req *req,
    /* output  */ ipc_PowerFreqSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("----------dev_on_ipc_PowerFreqSet-----------");
    T_D("power freq = %d", req->power_freq);

    db_update("fre", req->power_freq);
    db_flush();

    return retval;
}

bool dev_on_ipc_PowerFreqGet(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqGet_Req *req,
    /* output  */ ipc_PowerFreqGet_Resp *rsp)
{
    T_D("----------dev_on_ipc_PowerFreqGet-----------");


    /* retval - true: support, false: unsupport */
    bool retval = true;

    rsp->power_freq = db_select("fre");
    T_D("power freq = %d", rsp->power_freq);
    return retval;
}

bool dev_on_ipc_VolumeGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeGet_Req *req,
    /* output  */ ipc_VolumeGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("----------dev_on_ipc_VolumeGet-----------");
    rsp->volume = db_select("vol");

    printf("\n >>>>>>>>>>>> rsp->volume  = %d\n", rsp->volume);
    return retval;
}

bool dev_on_ipc_AlarmSet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmSet_Req *req,
    /* output  */ ipc_AlarmSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    g_test_t.motion_detection = req->motion;
    g_test_t.opensound_detection = req->sound;
    g_test_t.smoke_detection = req->smoke;
    g_test_t.shadow_detection = req->shadow;
#endif

    return retval;
}

bool dev_on_ipc_AlarmGet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmGet_Req *req,
    /* output  */ ipc_AlarmGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("dev_on_ipc_AlarmGet");
#if 0
    rsp->motion = g_test_t.motion_detection;
    rsp->sound = g_test_t.opensound_detection;
    rsp->smoke = g_test_t.smoke_detection;
    rsp->shadow = g_test_t.shadow_detection;
#endif

    return retval;
}


bool dev_on_ipc_Screenshot(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_Screenshot_Req *req,
    /* output  */ ipc_Screenshot_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("dev_on_ipc_Screenshot");
    T_D("sizeof(rsp->body.bytes) = %d", sizeof(rsp->body.bytes));

    extern int net_video_rec0_take_photo(u8 * buffer, u32 buffer_len);
    printf("\ntime1 =  %d\n", timer_get_ms());
    u32 buffer_len = net_video_rec0_take_photo(rsp->body.bytes, sizeof(rsp->body.bytes));
    printf("\ntime2 =  %d\n", timer_get_ms());
    rsp->format = 2; // 1 png; 2jpg
    rsp->body.size = buffer_len;


    return retval;
}

bool dev_on_ipc_PtzCtrl(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PtzCtrl_Req *req,
    /* output  */ ipc_PtzCtrl_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("req_func_code: %d ", req->func_code);
    T_D("req_para1: %d ", req->para1);
    T_D("req_para2: %d ", req->para2);

    return retval;
}

bool dev_on_ipc_SetAutoTrack(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_SetAutoTrack_Req  *req,
    /* output  */ ipc_SetAutoTrack_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("dev_on_ipc_SetAutoTrack");
    T_D("autoTrack: %d ", req->autoTrack);
#if 0
    g_test_t.autotrack = req->autoTrack;
#endif
    rsp->respCode = 0;

    return retval;
}

bool dev_on_ipc_GetAutoTrack(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_GetAutoTrack_Req  *req,
    /* output  */ ipc_GetAutoTrack_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("dev_on_ipc_GetAutoTrack");
#if 0
    T_D("autoTrack: %d ", g_test_t.autotrack);
    rsp->autoTrack = g_test_t.autotrack;
#endif
    return retval;
}

void avsdk_firmware_rate_callback(void *arg, int index)
{
    int32_t  conn_id = OSAL_POINTER_TO_UINT(arg);
    avsdk_firmware_rate(conn_id, 0, index);
}


void firmware_notify_thread(void *arg)
{

    ipc_FirmwareCheckByDevice_Resp rsp;
    avsdk_firmware_poll(&rsp);
    printf("\n rsp.is_update = %d     ,"
           "\n rsp.firmware_ver = %s  ,"
           "\n rsp.firmware_url = %s  ,"
           "\n rsp.firmware_type =  %d,"
           "\n rsp.firmware_size  %d   "
           ,
           rsp.is_update,
           rsp.firmware_ver,
           rsp.firmware_url,
           rsp.firmware_type,
           rsp.firmware_size);
    if (rsp.is_update == 1) {
        extern void cloud_upgrade_firmware(void *rsp, int notify, void *arg);
        cloud_upgrade_firmware(&rsp, 1, arg);
        // return;
    }
}

bool dev_on_ipc_FirmwareNotify(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_FirmwareNotify_Req  *req,
    /* output  */ ipc_FirmwareNotify_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    /** TODO:异步操作
     * 1.调用avsdk_firmware_poll查询固件信息和url。
     * 2.使用http下载固件。
     * 3.调用avsdk_firmware_rate上报升级进度
     * 4.更新固件重启。
     */
    thread_fork("firmware_notify_thread", 18, 0x1000, 0, 0, firmware_notify_thread, OSAL_UINT_TO_POINTER(conn_id));
    rsp->rate = 0;
    return retval;
}


bool dev_on_ipc_EventRecordGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordGet_Req *req,
    /* output  */ ipc_EventRecordGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_EventRecordGet------------------");
    rsp->duration = db_select("cyc");
    printf("\nrsp->duration = %d \n", rsp->duration);

    return retval;
}



bool dev_on_ipc_EventRecordSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordSet_Req *req,
    /* output  */ ipc_EventRecordSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_EventRecordSet------------------");
    printf("\nrsp->duration = %d \n", req->duration);

    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_CHANGE_CYC_TIME, req->duration);

    return retval;
}

bool dev_on_ipc_GetNetworkInfo(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_GetNetworkInfo_Req *req,
    /* output  */ ipc_GetNetworkInfo_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------dev_on_ipc_GetNetworkInfo------------------");

    struct NETWORK_INFO *network_info = sys_get_network_info();
    rsp->qos = system_get_network_info_qos();
    snprintf(rsp->ssid, sizeof(rsp->ssid), "%s", network_info->ssid);
    snprintf(rsp->ipaddr, sizeof(rsp->ipaddr), "%s",  network_info->ip);
    snprintf(rsp->netmask, sizeof(rsp->netmask), "%s",  network_info->nm);
    snprintf(rsp->gateway, sizeof(rsp->gateway), "%s",  network_info->gw);
    snprintf(rsp->dns1, sizeof(rsp->dns1), "%s",  network_info->dns1);
    snprintf(rsp->dns2, sizeof(rsp->dns2), "%s",  network_info->dns2);
    snprintf(rsp->mac, sizeof(rsp->mac), "%s",  network_info->mac);

    T_D("channel: %d ", req->channel);
    T_D("ssid: %s ", rsp->ssid);
    T_D("qos: %d ", rsp->qos);
    T_D("ipaddr: %s ", rsp->ipaddr);
    T_D("netmask: %s ", rsp->netmask);
    T_D("gateway: %s ", rsp->gateway);
    T_D("dns1: %s ", rsp->dns1);
    T_D("dns2: %s ", rsp->dns2);
    T_D("mac: %s ", rsp->mac);


    return retval;
}



bool dev_on_ipc_FileStart(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_FileStart_Req *req,
    /* output  */ ipc_FileStart_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_FileStart------------------");

    T_D("req_func_code: %lld ", req->file_id);
    T_D("req_para1: %lld ", req->length);
    T_D("req_para2: %lld ", req->offset);

    return retval;
}

bool dev_on_ipc_FileStop(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_FileStop_Req *req,
    /* output  */ ipc_FileStop_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------dev_on_ipc_FileStop------------------");
    T_D("req_func_code: %lld ", req->file_id);

    return retval;
}

bool dev_on_ipc_CustomCmd(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_CustomCmd_Req *req,
    /* output  */ ipc_CustomCmd_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    static char time = 0;
    avsdk_custom_add_conn(conn_id);
    T_D("req_byte: len = %d", req->arg_int32_count);

    for (char i = 0; i < req->arg_int32_count; i++) {
        T_D("req_int [%d]=%d", i, req->arg_int32[i]);
    }

    T_D("req_byte: len = %d", req->arg_bytes.size);
    for (int i = 0; i < req->arg_bytes.size; i++) {
        T_D("req_byte[%d] = %02x", i, req->arg_bytes.bytes[i]);
    }

    for (int index = 0; index < req->arg_string_count; index++) {
        T_D("req_str_[%d]: %s, len = %d", index, req->arg_string[index], strlen(req->arg_string[index]));

    }


    //if 收到什么，将connid加入custom用户组
    //avsdk_custom_add_conn(conn_id);
    //if 收到什么，将connid从custom用户组删除
    //avsdk_custom_del_conn(conn_id);

    return retval;
}

bool dev_on_ipc_PirSet(
    /* conn_id */ int32_t			 conn_id,
    /* input   */ ipc_PirSet_Req *req,
    /* output  */ ipc_PirSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_PirSet------------------");

    T_D("req->pir.num: %d ", req->pir.num);
    T_D("req->pir.level: %d ", req->pir.level);

    extern void set_pir_state(int state);
    set_pir_state(req->pir.level);
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_SET_PIR);
    return retval;
}


bool dev_on_ipc_PirGet(
    /* conn_id */ int32_t			 conn_id,
    /* input   */ ipc_PirGet_Req *req,
    /* output  */ ipc_PirGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_PirGet------------------");


    rsp->pirs_count = 1;
    rsp->pirs[0].num = 1;
    extern int get_pir_state();
    rsp->pirs[0].level = get_pir_state();
    T_D("rsp->pirs_count: %d ", rsp->pirs_count);
    T_D("rsp->pirs[0].num: %d ", rsp->pirs[0].num);
    T_D("rsp->pirs[0].level: %d ", rsp->pirs[0].level);



    return retval;
}

bool dev_on_ipc_ChanState(
    /* conn_id */ int32_t			  conn_id,
    /* input   */ ipc_ChanState_Req  *req,
    /* output  */ ipc_ChanState_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_ChanState------------------");



    rsp->chans_count = 1;
    rsp->chans[0].channel = 0;
    rsp->chans[0].state = 1;

    return retval;
}

bool dev_on_ipc_VideoChanChange(
    /* conn_id */ int32_t			 conn_id,
    /* input   */ ipc_VideoChanChange_Req *req,
    /* output  */ ipc_VideoChanChange_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_VideoChanChange------------------");

    T_D("rsp->channel_count: %d ", req->channel_count);
    for (int i = 0; i < req->channel_count; i++) {
        T_D("rsp->channel[%d]: %d ", i, req->channel[i]);
    }

    rsp->render_count = 1;
    rsp->render[0] = 5;

    return retval;
}

//所有的发送api都不能在回调里面调用
static int tamper_set_thread_pid;
void tamper_set_thread(void *priv)
{
    int state = (u32) priv;
    extern void set_tamper_state(int state);
    set_tamper_state(state);
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_SET_TAMPER);
    if (state) {
        avsdk_report_tamper_status(E_IOT_TAMPER_ON);
    } else {
        avsdk_report_tamper_status(E_IOT_TAMPER_OFF);
    }
}

//强拆报警
bool dev_on_ipc_TamperSet(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_TamperSet_Req *req,
    /* output  */ ipc_TamperSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    //参考iot_tamper_status_t
    T_D("TamperSet state: %lld", req->state);

    int state = req->state;


    thread_kill(&tamper_set_thread_pid, 0);

    thread_fork("tamper_set_thread", 8, 0x1000, 0, &tamper_set_thread_pid, tamper_set_thread, (void *)state);

    return retval;
}


bool dev_on_ipc_DirCreate(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirCreate_Req *req,
    /* output  */ ipc_DirCreate_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------%s------------------", OSAL_STRFUNC);
    T_D("dir_name:%s", req->dir_name);
    T_D("dir_name:%s", req->dir_path);
    return retval;
}

bool dev_on_ipc_DirDel(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirDel_Req *req,
    /* output  */ ipc_DirDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------%s------------------", OSAL_STRFUNC);

    T_D("dir_name:%s", req->dir_name);
    T_D("dir_name:%s", req->dir_path);

    return retval;
}

bool dev_on_ipc_DirEdit(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirEdit_Req *req,
    /* output  */ ipc_DirEdit_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------%s------------------", OSAL_STRFUNC);


    T_D("dir_name:%s", req->dir_path);
    T_D("old_dir_name:%s", req->old_dir_name);
    T_D("new_dir_name:%s", req->new_dir_name);

    return retval;
}

bool dev_on_ipc_DirList(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirList_Req *req,
    /* output  */ ipc_DirList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);
    T_D("dir_name:%s", req->dir_path);
    T_D("page:%d", req->page);
    T_D("page_size:%d", req->page_size);

    return retval;
}

#if 1 //文件传输

typedef struct {
    int32_t conn_id;
    bool tosend;
    bool recv_end;
    bool init;
    uint64_t total_size;
    uint64_t last_offset;
    ipc_FileRecvRate_Req    req;

} frr_ctx_t;


frr_ctx_t frr;

bool dev_on_ipc_FileAdd(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileAdd_Req *req,
    /* output  */ ipc_FileAdd_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------%s------------------", OSAL_STRFUNC);

    T_I("req->dir_path:%s", req->dir_path);

    T_I("req->file_name:%s", req->file_name);
    T_I("req->file_id:%lld", req->file_id);
    T_I("req->total_size:%lld", req->total_size);

    frr.total_size = req->total_size;

    return retval;
}


void *report_recvfile_thread(void *arg)
{

    frr_ctx_t *frrp = &frr;
    int32_t  conn_id;
    int rc = 0;
    ipc_FileRecvRate_Req    req;
    ipc_FileRecvRate_Resp   rsp;
#if 0
    while (1) {

        /* diff > 40K */
        if (!frrp->tosend && (frrp->req.recv_size != 0) && (frrp->req.recv_size - frrp->last_offset > 40000)) {
            T_I("set tosend =1, (receive,last)  (%lld-%lld)", frrp->req.recv_size, frrp->last_offset);
            frrp->tosend = true;
        }

        /* 判定是不是文件结束 */
        if (!frrp->tosend && (frrp->total_size == frrp->req.recv_size)) {
            if (frrp->last_offset != frrp->total_size) {

                frrp->tosend = true;
                T_I("receive file end %lld", req.recv_size);
            } else {
                /* 已经发送了最后一包 */
                frr.last_offset = 0;
                frr.recv_end = false;
                frr.tosend = false;
                frr.total_size = 0;
                frr.conn_id = -1;
                memset(&frr.req, 0, sizeof(ipc_FileRecvRate_Req));
                memset(&req, 0, sizeof(ipc_FileRecvRate_Req));
            }

        }

        if (frrp->tosend) {
            if (frrp->req.recv_size - frrp->last_offset == 0) {
                frrp->tosend = false;
                continue ;
            }

            if (frrp->conn_id < 0) {
                continue;
            }
            conn_id = frrp->conn_id;
            memcpy(&req, &frrp->req, sizeof(req));
            rc = avsdk_report_FileRecvRate(conn_id,
                                           &req,
                                           &rsp);
            if (rc != 0 && rc != 9 && rc != 5) {
                T_E("error: avsdk_report_FileRecvRate, rc=%d", rc);
            } else {
                T_W("report diff = %lld", (req.recv_size - frrp->last_offset));

                frrp->last_offset = req.recv_size;

                T_W("report_recvfile filed == %lld,(total-recv-last) (%lld-%lld-%lld)",
                    frrp->req.file_id,
                    frrp->total_size,
                    req.recv_size,
                    frrp->last_offset);

                frrp->tosend = false;

            }
        } else {
            usleep(10);
        }
    }
#endif

    return NULL;
}


void dev_on_RecvFile(int32_t            conn_id,
                     ipc_RecvFile_Req   *req,
                     ipc_RecvFile_Resp  *rsp)
{
    if (req->offset % 100 * 1024 == 0) {
        T_I("dev_on_RecvFile");
        T_D("conn_id=%d", conn_id);
        T_I("file_id=%"PRIi64, req->file_id);
        T_I("offset=%"PRIi64, req->offset);
        T_D("payload_len=%"PRIi64, req->payload_len);
        T_T("msg:%s", req->payload);
    }

    frr.req.file_id = req->file_id;
    frr.req.recv_size = req->offset + req->payload_len;
    frr.conn_id = conn_id;
#if 0
    if (!frr.init) {
        pthread_t frr_tidp;
        pthread_create(&frr_tidp, NULL, report_recvfile_thread, (void *)&frr);
        frr.init = true;
    }
#endif
    rsp->recode = 0;
}

#endif //文件传输demo

bool dev_on_ipc_FileDel(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileDel_Req *req,
    /* output  */ ipc_FileDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}

bool dev_on_ipc_FileEdit(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileEdit_Req *req,
    /* output  */ ipc_FileEdit_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}

bool dev_on_ipc_FileRecvRate(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileRecvRate_Req *req,
    /* output  */ ipc_FileRecvRate_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}

bool dev_on_ipc_FileThumList(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileThumList_Req *req,
    /* output  */ ipc_FileThumList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}


bool dev_on_ipc_ExecIOTCMD(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_ExecIOTCMD_Req *req,
    /* output  */ ipc_ExecIOTCMD_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    /* 注意：
     * req中iot_cmds与ttcmd为互斥，每次命令只会有其中一个赋值，
     * 可以通过cmds_count是否为0，来判定命令为iot_cmds还是ttcmd。
     */
    T_D("---------------%s------------------", OSAL_STRFUNC);
    if (req->iot_cmds.cmds_count) {
        /* 打印iot_cmds所有的参数 */
        avsdk_dump_PpiotCmd("req->iot_cmds", &req->iot_cmds);
        for (int i = 0; i < req->iot_cmds.cmds_count; i++) {
            if (req->iot_cmds.cmds[i].cid == 590) {
//                iot_PpiotCmd_t *ppiot = &req->iot_cmds;
                video0_rec_get_iframe();
            }
        }

    } else {
        //todo:无效命令
        printf("\n %s %d\n", __func__, __LINE__);
    }
    return retval;
}


bool dev_on_ipc_SetLowPower(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_SetLowPower_Req *req,
    /* output  */ ipc_SetLowPower_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------%s------------------", OSAL_STRFUNC);
    db_update("lowpower", req->state);
    db_flush();
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_SET_POWER_MODE);
    return retval;
}

bool dev_on_ipc_GetLowPower(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_GetLowPower_Req *req,
    /* output  */ ipc_GetLowPower_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------%s------------------", OSAL_STRFUNC);
    rsp->state = db_select("lowpower");
    return retval;
}

bool dev_on_ipc_ForceIFrame(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_ForceIFrame_Req *req,
    /* output  */ ipc_ForceIFrame_Resp *rsp)
{
    T_D("---------------%s------------------", OSAL_STRFUNC);
    /* retval - true: support, false: unsupport */
    bool retval = true;
    video0_rec_get_iframe();
    return retval;
}

void dev_on_ipc_talkbackcb(int32_t              conn_id,
                           iot_exhdr_media_t    *exhdr,
                           const uint8_t        *media_data,
                           size_t               media_len)
{
//   T_I("---------Speak Date[%d] avformat = %d\n", media_len,exhdr->avformat.streamtype);

    if (exhdr->avformat.streamtype == E_IOT_MEDIASTREAM_TYPE_AAC || exhdr->avformat.streamtype == E_IOT_MEDIASTREAM_TYPE_PCM) {
        char *data = malloc(media_len);
        memcpy(data, media_data, media_len);
        os_taskq_post("cloud_rt_talk_task", 3, RT_AUDIO_CTRL_WRITE, data, media_len);
    }

}



#endif

/*****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
