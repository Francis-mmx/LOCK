/*****************************************************************************
 * ut_xciot.c
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Included Files
 ****************************************************************************/

#define _STDIO_H_


#include "printf.h"
#include "xciot_api.h"
#include "ut_dev_ipc_cmd.h"
#include "system/database.h"
#include "device/device.h"
#include "os/os_compat.h"
#include "doorbell_event.h"
//#include <stddef.h>
//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <inttypes.h>


/*****************************************************************************
 * Trace Definitions
 ****************************************************************************/
#define EVOS_LOG_COMPILE_LEVEL EVOS_LOG_LEVEL_DEBUG
#define EVOS_LOG_RUN_LEVEL     EVOS_LOG_LEVEL_DEBUG
#define EVOS_LOG_DOMAIN        "ut_xciot.c"
#include "xc_log.h"


/*****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define ENABLE_CS
//#define USE_INI //使用配置文件



/*****************************************************************************
 * Private Types
 ****************************************************************************/

/*****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/*****************************************************************************
 * Private Data
 ****************************************************************************/

/*****************************************************************************
 * Public Data
 ****************************************************************************/


/*****************************************************************************
 * Private Functions
 ****************************************************************************/
extern void set_utc_ms(uint64_t utc_ms);
extern uint64_t get_utc_ms(void);
static char hardware_version[64] = DOORBELL_VERSION"1.1";
#define  FIRMWARE_VERSION  "1.02.01.01_211029"


static u8 avsdk_connect_flag = 0;
static u8 app_connect_flag = 0;
static u8 avsdk_init_flag = 0;
static u32 start_time = 0;

void set_hardware_version(char *hv, char *sv)
{
    snprintf(hardware_version, sizeof(hardware_version), "%s_%s.%s", DOORBELL_VERSION, hv, sv);
}
char *get_hardware_version(void)
{
    return hardware_version;
}
u8 get_avsdk_connect_flag(void)
{
    return avsdk_connect_flag;
}
u8 get_app_connect_flag(void)
{
    return app_connect_flag;
}
u8 get_avsdk_init_flag(void)
{
    return avsdk_init_flag;
}
static void iot_dev_on_platform_utc(uint64_t utc_ms)
{
    T_I("platform connect utc_ms = %llu", utc_ms);
    set_utc_ms(utc_ms);
}


static void iot_dev_on_connect_platform(iot_conn_platform_state_t state)
{
    int rc = 0;

    switch (state) {

    case E_IOT_CONN_PLATFORM_STATE_DISCONN:
        T_I("-----------platform connected is disconnected------------");
        {
            /* 设备与平台未连接*/
            avsdk_connect_flag = 0;
        }
        break;
    case E_IOT_CONN_PLATFORM_STATE_CONNECTED:
        T_I("------------platform connected is ok--------------------");
        static int flag = 0;
        if (flag == 0) {
            flag = 1;
            char vcode[13] = {0};
            extern int get_net_cfg_info(char *ssid, char *pwd, char *vcode);
            get_net_cfg_info(NULL, NULL, vcode);
            if (vcode[0]) {
                extern int avsdk_report_vcode(char *vcode);
                avsdk_report_vcode(vcode);
            }
            avsdk_connect_flag = 1;
#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
//            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_OPEN_RT_STREAM);
#endif
            os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_WIFI_LIST);
            printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>timer_get_ms() - start_time = %d\n", timer_get_ms() - start_time);
        }
#if 0
        {
            /* 平台连接已经建立，添加device相关代码 */


        }
#endif

#if 0
        char ssid[32] = {0};
        char passwd[32] = {0};
        avsdk_get_wifi_conf(ssid, passwd);
        T_I("ssid=%s, passwd=%s", ssid, passwd);
#endif
        break;
    case E_IOT_CONN_PLATFORM_STATE_ERROR_UCODE:
        /* 每个设备提供的唯一代码: MAC，芯片序列号，IMEI都OK; 下次通讯必须一致，如果不一致平台不允许登录 */
        T_E("ERROR: Different ucode,Please check whether DID is repeated!");
        break;
    default:
        break;
    }
}

static void iot_dev_on_reconnect_platform(iot_conn_platform_state_t state)
{
    int rc = 0;
    switch (state) {
    case E_IOT_CONN_PLATFORM_STATE_DISCONN:
        T_I("-----------platform connected is disconnected------------");
        {
            avsdk_connect_flag = 0;
        }
        break;
    case E_IOT_CONN_PLATFORM_STATE_CONNECTED:
        T_I("------------platform connected is ok--------------------");
        {
            avsdk_connect_flag = 1;
        }
        break;
    case E_IOT_CONN_PLATFORM_STATE_ERROR_UCODE:
        T_E("ERROR: Different ucode,Please check whether DID is repeated!");
        break;
    default:
        break;
    }
}
static void iot_dev_on_connect(int32_t          conn_id,
                               iot_conn_state_t state,
                               iot_conn_type_t  type)
{
    int rc = 0;

    switch (state) {
    case E_IOT_CONN_STATE_IDEL:
        T_I("I'm dev conn[%d] idel", conn_id);
        {
            /* 这个连接，SDK相关资源已经被清除，可以接收新的连接 */
        }
        break;
    case E_IOT_CONN_STATE_INIT:
        T_I("I'm dev conn[%d] init", conn_id);
        {
            /* 远端向device发起的一个连接请求，SDK将尝试建立连接
             * Device实现可以初始化相关资源
             */
        }
        break;
    case E_IOT_CONN_STATE_OPENED:
        T_I("I'm dev conn[%d] opened", conn_id);
        {
            /* 连接已经建立，添加device相关代码 */
            msleep(200);
            app_connect_flag = 1;

            post_msg_doorbell_task("doorbell_block_event_task", 2, DOORBELL_EVENT_REPORT_BATTARY, 1);
        }
        break;
    case E_IOT_CONN_STATE_CLOSED:
        T_I("I'm dev conn[%d] closed", conn_id);
        {
            /* 连接已经关闭 */
            avsdk_video_del_conn(conn_id, -1);
            avsdk_audio_del_conn(conn_id);
            avsdk_custom_del_conn(conn_id);
            app_connect_flag = 0;
            /*如果用户断开连接时，没有关闭播放历史视频，则关闭线程*/
            extern int cloud_playback_uninit(int32_t conn_id,  uint64_t file_id);
            cloud_playback_uninit(conn_id, 0);

        }
        break;
    default:
        break;
    }
}

static void iot_dev_on_packet_dropout(iot_packet_state_t state)
{
    int rc = 0;

    switch (state) {

    case E_IOT_PACKET_NORMAL:
        T_A("-----------The current data transfer is normal------------");
        {
            /* 数据传输恢复正常 */
        }
        break;
    case E_IOT_PACKET_DROPOUT:
        T_A("------------packet dropout ing --------------------");
        {
            /* 数据开始丢包 */
            //todo:降低视频的码流
        }
        break;

    default:
        break;
    }
}



/*****************************************************************************
 * Public Functions
 ****************************************************************************/
int avsdk_main(void)
{

    int rc;
    uint8_t *faststart_data = NULL;
    size_t faststart_datalen = 0;
    char *avsdk_cfg = NULL;
    if (avsdk_init_flag != 0) {
        return 0;
    }
    avsdk_init_flag = 1;

    start_time = timer_get_ms();
    /* step 0: 设置SDK打印日志等级 */
    /* step 1: 设置初始化参数 */
    avsdk_init_params_t init_params;
    memset(&init_params, 0, sizeof(init_params));
    /* ipc dev */
    init_params.roletype = E_IOT_ROLE_TYPE_DEV;
    /* conn maxsize */
    init_params.conn_maxsz = 5;


#define AVSDK_CFG_LEN 1024
    avsdk_cfg = calloc(1, AVSDK_CFG_LEN);
    if (!avsdk_cfg) {
        puts("\n avsdk_cfg buff malloc fail\n");
        return -1;
    }
//    extern void doorbell_read_cfg_info(u8 * cfg, int len);
//    doorbell_read_cfg_info(avsdk_cfg, AVSDK_CFG_LEN);
    unsigned char  data[] = {
        0x5B, 0x69, 0x6F, 0x74, 0x5D, 0x0A, 0x64, 0x69, 0x64, 0x20, 0x3D, 0x20, 0x50, 0x50, 0x4A, 0x4C,
        0x4D, 0x4C, 0x30, 0x30, 0x31, 0x33, 0x45, 0x42, 0x30, 0x44, 0x33, 0x44, 0x32, 0x46, 0x0A, 0x73,
        0x69, 0x67, 0x6E, 0x6B, 0x65, 0x79, 0x20, 0x3D, 0x20, 0x71, 0x6C, 0x44, 0x7A, 0x73, 0x52, 0x56,
        0x73, 0x4B, 0x76, 0x51, 0x4A, 0x39, 0x51, 0x6A, 0x77, 0x4C, 0x59, 0x65, 0x35, 0x36, 0x51, 0x3D,
        0x3D, 0x0A, 0x6C, 0x73, 0x6C, 0x61, 0x74, 0x20, 0x3D, 0x20, 0x69, 0x4D, 0x59, 0x48, 0x6F, 0x35,
        0x54, 0x6A, 0x30, 0x7A, 0x47, 0x4B, 0x50, 0x52, 0x2F, 0x32, 0x64, 0x77, 0x56, 0x50, 0x41, 0x41,
        0x3D, 0x3D, 0x0A, 0x73, 0x63, 0x6F, 0x64, 0x65, 0x20, 0x3D, 0x20, 0x33, 0x38, 0x30, 0x37, 0x38,
        0x37, 0x0A, 0x67, 0x64, 0x6F, 0x6D, 0x61, 0x69, 0x6E, 0x20, 0x3D, 0x20, 0x70, 0x72, 0x6F, 0x64,
        0x2E, 0x67, 0x6C, 0x62, 0x73, 0x2E, 0x78, 0x63, 0x74, 0x68, 0x69, 0x6E, 0x67, 0x73, 0x2E, 0x63,
        0x6F, 0x6D, 0x0A, 0x67, 0x69, 0x70, 0x61, 0x64, 0x64, 0x72, 0x20, 0x3D, 0x20, 0x33, 0x39, 0x2E,
        0x31, 0x30, 0x38, 0x2E, 0x35, 0x39, 0x2E, 0x36, 0x30, 0x2C, 0x38, 0x2E, 0x32, 0x30, 0x39, 0x2E,
        0x37, 0x37, 0x2E, 0x34, 0x39, 0x2C, 0x34, 0x37, 0x2E, 0x32, 0x35, 0x32, 0x2E, 0x35, 0x2E, 0x32,
        0x32, 0x35, 0x2C, 0x34, 0x37, 0x2E, 0x32, 0x34, 0x30, 0x2E, 0x31, 0x2E, 0x32, 0x34, 0x34, 0x0A
    };



    memcpy(avsdk_cfg, data, sizeof(data));
    printf("\nbuffer =  %s\n", avsdk_cfg);
    //put_buf(avsdk_cfg,sizeof(avsdk_cfg ));

    init_params.cfg_buf = avsdk_cfg;
    init_params.cfg_buf_len = strlen(avsdk_cfg);




    char firmware_version[64] = {0};




    printf("\n hardware_version = %s\n", hardware_version);

    /*hardware version*/
    strcpy(init_params.hardware_version, hardware_version);

    /*firmware version*/
    int ret = db_select_buffer(FIRMWARE_VM_INDEX, firmware_version, sizeof(firmware_version));
    if (ret != sizeof(firmware_version)) {
        memcpy(firmware_version, FIRMWARE_VERSION, strlen(FIRMWARE_VERSION));
        db_update_buffer(FIRMWARE_VM_INDEX, firmware_version, sizeof(firmware_version));
    }
    printf("\nfirmware_version = %s\n", firmware_version);
    strcpy(init_params.firmware_version, firmware_version);

    /*每个设备提供的唯一代码: MAC，芯片序列号，IMEI都OK;
     *下次通讯必须一致，如果不一致平台不允许登录;
     *填""则平台连接不检查ucode字段。
     */
    strcpy(init_params.ucode, "");



    /* step2: sdk 初始化 */
    rc = avsdk_init(&init_params);
    if (rc) {
        printf("Failed to avsdk_init, rc:%d(%s)\n", rc, avsdk_strerror(rc));
        goto __exit;
    }

    //avsdk_set_wnd(1024,1024);

    avsdk_set_sendcap(30, 10, 10, 10);
    /* step3: 注册平台回调函数 */

    /*平台连接utc回调，可用于同步时间*/
    avsdk_set_on_platform_utc(iot_dev_on_platform_utc);
    /*平台连接状态回调*/
    avsdk_set_on_connect_platform(iot_dev_on_connect_platform);
    avsdk_set_on_reconnect_platform(iot_dev_on_reconnect_platform);
    /*APP连接状态回调*/
    avsdk_set_on_connect(iot_dev_on_connect);
    /*音视频丢包通知回调*/
    avsdk_set_on_PacketDropout(iot_dev_on_packet_dropout);

    /* step4: 注册业务回调函数 */

    /* 实时音视频 */
    avsdk_set_on_ipc_VideoPlay(dev_on_ipc_VideoPlay);
    avsdk_set_on_ipc_VideoPause(dev_on_ipc_VideoPause);
    avsdk_set_on_ipc_AudioPlay(dev_on_ipc_AudioPlay);
    avsdk_set_on_ipc_AudioPause(dev_on_ipc_AudioPause);
    avsdk_set_on_ipc_ChanState(dev_on_ipc_ChanState);

    /* 音视频设置参数 */
    avsdk_set_on_ipc_VideoQosSet(dev_on_ipc_VideoQosSet);
    avsdk_set_on_ipc_FlipSet(dev_on_ipc_FlipSet);
    avsdk_set_on_ipc_FlipGet(dev_on_ipc_FlipGet);
    avsdk_set_on_ipc_VolumeGet(dev_on_ipc_VolumeGet);
    avsdk_set_on_ipc_VolumeSet(dev_on_ipc_VolumeSet);

    /* 对讲 */
    avsdk_set_on_media(dev_on_ipc_talkbackcb);
    avsdk_set_on_ipc_TalkbackPlay(dev_on_ipc_TalkbackPlay);
    avsdk_set_on_ipc_TalkbackPause(dev_on_ipc_TalkbackPause);

    /* 历史播放 */
    avsdk_set_on_ipc_HistoryPlay(dev_on_ipc_HistoryPlay);
    avsdk_set_on_ipc_HistoryPause(dev_on_ipc_HistoryPause);

    /* 历史播放设置 */
    avsdk_set_on_ipc_HistoryPlanSet(dev_on_ipc_HistoryPlanSet);
    avsdk_set_on_ipc_HistoryPlanGet(dev_on_ipc_HistoryPlanGet);
    avsdk_set_on_ipc_HistoryDays(dev_on_ipc_HistoryDays);
    avsdk_set_on_ipc_HistoryDayList(dev_on_ipc_HistoryDayList);
    avsdk_set_on_ipc_HistoryThumGet(dev_on_ipc_HistoryThumGet);
    avsdk_set_on_ipc_HistoryDel(dev_on_ipc_HistoryDel);

    /* 平台命令,防撬 */
    avsdk_set_on_ipc_TamperSet(dev_on_ipc_TamperSet);

    /* 配置网络 */
    avsdk_set_on_ipc_Discovery(dev_on_ipc_Discovery);
    avsdk_set_on_ipc_WifiAPGet(dev_on_ipc_WifiAPGet);
    avsdk_set_on_ipc_WifiSet(dev_on_ipc_WifiSet);
    avsdk_set_on_ipc_WifiGet(dev_on_ipc_WifiGet);
    avsdk_set_on_ipc_GetNetworkInfo(dev_on_ipc_GetNetworkInfo);

    /*时间设置*/
    avsdk_set_on_ipc_TimeSet(dev_on_ipc_TimeSet);
    avsdk_set_on_ipc_TimeGet(dev_on_ipc_TimeGet);

    /* LED */
    avsdk_set_on_ipc_LedSet(dev_on_ipc_LedSet);
    avsdk_set_on_ipc_LedGet(dev_on_ipc_LedGet);

    /* 夜视功能 */
    avsdk_set_on_ipc_IRCutSet(dev_on_ipc_IRCutSet);
    avsdk_set_on_ipc_IRCutGet(dev_on_ipc_IRCutGet);

    /* 私有模式 */
    avsdk_set_on_ipc_SecretSet(dev_on_ipc_SecretSet);
    avsdk_set_on_ipc_SecretGet(dev_on_ipc_SecretGet);
#if 0
    /* 运动设置 */
    avsdk_set_on_ipc_MotionzoneSet(dev_on_ipc_MotionzoneSet);
    avsdk_set_on_ipc_MotionzoneGet(dev_on_ipc_MotionzoneGet);
    avsdk_set_on_ipc_PspAdd(dev_on_ipc_PspAdd);
    avsdk_set_on_ipc_PspDel(dev_on_ipc_PspDel);
    avsdk_set_on_ipc_PspList(dev_on_ipc_PspList);
    avsdk_set_on_ipc_PspCall(dev_on_ipc_PspCall);
    avsdk_set_on_ipc_TimedcruiseSet(dev_on_ipc_TimedcruiseSet);
    avsdk_set_on_ipc_TimedcruiseGet(dev_on_ipc_TimedcruiseGet);
    avsdk_set_on_ipc_PtzCtrl(dev_on_ipc_PtzCtrl);
    avsdk_set_on_ipc_SetAutoTrack(dev_on_ipc_SetAutoTrack);
    avsdk_set_on_ipc_GetAutoTrack(dev_on_ipc_GetAutoTrack);
#endif

    /*电源频率*/
    avsdk_set_on_ipc_PowerFreqSet(dev_on_ipc_PowerFreqSet);
    avsdk_set_on_ipc_PowerFreqGet(dev_on_ipc_PowerFreqGet);

    /* 异常事件 */
    avsdk_set_on_ipc_EventRecordGet(dev_on_ipc_EventRecordGet);
    avsdk_set_on_ipc_EventRecordSet(dev_on_ipc_EventRecordSet);
    avsdk_set_on_ipc_NotifySet(dev_on_ipc_NotifySet);
    avsdk_set_on_ipc_NotifyGet(dev_on_ipc_NotifyGet);
    avsdk_set_on_ipc_AlarmSet(dev_on_ipc_AlarmSet);
    avsdk_set_on_ipc_AlarmGet(dev_on_ipc_AlarmGet);
    avsdk_set_on_ipc_PirSet(dev_on_ipc_PirSet);
    avsdk_set_on_ipc_PirGet(dev_on_ipc_PirGet);

    /* 文件传输 */
    avsdk_set_on_ipc_FileStart(dev_on_ipc_FileStart);
    avsdk_set_on_ipc_FileStop(dev_on_ipc_FileStop);

    /* 用户自定义命令 */
    avsdk_set_on_ipc_CustomCmd(dev_on_ipc_CustomCmd);

    /*获取所有配置信息*/
    avsdk_set_on_ipc_ConfigGet(dev_on_ipc_ConfigGet);

    /* 低功耗设置 */
    avsdk_set_on_ipc_SetLowPower(dev_on_ipc_SetLowPower);
    avsdk_set_on_ipc_GetLowPower(dev_on_ipc_GetLowPower);

    /* 强制I帧信号*/
    avsdk_set_on_ipc_ForceIFrame(dev_on_ipc_ForceIFrame);


    /* 其他接口 */
    avsdk_set_on_ipc_Screenshot(dev_on_ipc_Screenshot);
    avsdk_set_on_ipc_FirmwareNotify(dev_on_ipc_FirmwareNotify);
    avsdk_set_on_ipc_Reboot(dev_on_ipc_Reboot);
    avsdk_set_on_ipc_Reset(dev_on_ipc_Reset);
    avsdk_set_on_ipc_LanAuth(dev_on_ipc_LanAuth);
    avsdk_set_on_ipc_StorageInfo(dev_on_ipc_StorageInfo);
    avsdk_set_on_ipc_StorageFormat(dev_on_ipc_StorageFormat);
    avsdk_set_on_ipc_LogSet(dev_on_ipc_LogSet);
    avsdk_set_on_ipc_VideoChanChange(dev_on_ipc_VideoChanChange);

#if 0
    /* 文件接收 */
    avsdk_set_on_RecvFile(dev_on_RecvFile);

    /* 相册相关命令 */
    avsdk_set_on_ipc_DirCreate(dev_on_ipc_DirCreate);

    avsdk_set_on_ipc_DirDel(dev_on_ipc_DirDel);
    avsdk_set_on_ipc_DirEdit(dev_on_ipc_DirEdit);
    avsdk_set_on_ipc_DirList(dev_on_ipc_DirList);

    avsdk_set_on_ipc_FileAdd(dev_on_ipc_FileAdd);
    avsdk_set_on_ipc_FileDel(dev_on_ipc_FileDel);
    avsdk_set_on_ipc_FileEdit(dev_on_ipc_FileEdit);
    avsdk_set_on_ipc_FileRecvRate(dev_on_ipc_FileRecvRate);
    avsdk_set_on_ipc_FileThumList(dev_on_ipc_FileThumList);
#endif
    avsdk_set_on_ipc_ExecIOTCMD(dev_on_ipc_ExecIOTCMD);


    /* step5: 开始运行 sdk */



    printf("start sdk....\n");

    faststart_data = calloc(1, 1024);
    if (!faststart_data) {
        puts("\n faststart_data buff malloc fail\n");
        goto __exit;
    }


    faststart_datalen = read_data_for_flash(faststart_data, 1024);

    if (faststart_datalen) {
        rc = avsdk_faststart(faststart_data, faststart_datalen);
        printf("avsdk_faststart, rc:%d(%s)\n", rc, avsdk_strerror(rc));
        if (rc != 0) {
            rc = avsdk_start();
            printf("avsdk_start, rc:%d(%s)\n", rc, avsdk_strerror(rc));
        }
    } else {
        /* 正常启动SDK */
        rc = avsdk_start();
        printf("avsdk_start, rc:%d(%s)\n", rc, avsdk_strerror(rc));
    }

    avsdk_init_flag = 2;


    /* step6: 创建音视频发送线程 */

__exit:
    if (faststart_data) {
        free(faststart_data);
    }
    if (avsdk_cfg) {
        free(avsdk_cfg);
    }
    T_W("----->avsdk init success");
    return 0;
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
