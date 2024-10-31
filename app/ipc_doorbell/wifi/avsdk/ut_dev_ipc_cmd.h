/*****************************************************************************
 * ut_dev_ipc_cmd.h
 *
 *
 *
 ****************************************************************************/
/*
 * wiki
 * ====
 */
#ifndef UT_DEV_IPC_CMD_H_INCLUDE
#define UT_DEV_IPC_CMD_H_INCLUDE
/*****************************************************************************
 * @addtogroup ut_dev_ipc_cmd
 * @{
 */
/*****************************************************************************
 * Included Files
 ****************************************************************************/

//#include <stdint.h>
//#include <stdbool.h>
#include "xciot_api.h"

/*****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/*****************************************************************************
 * Public Types
 ****************************************************************************/

/*****************************************************************************
 * Public Data
 ****************************************************************************/

/*****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#if 1
bool dev_on_ipc_Discovery(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_Discovery_Req *req,
    /* output  */ ipc_Discovery_Resp *rsp);

bool dev_on_ipc_WifiAPGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_WifiAPGet_Req *req,
    /* output  */ ipc_WifiAPGet_Resp *rsp);

bool dev_on_ipc_WifiSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiSet_Req *req,
    /* output  */ ipc_WifiSet_Resp *rsp);

bool dev_on_ipc_WifiGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiGet_Req *req,
    /* output  */ ipc_WifiGet_Resp *rsp);

bool dev_on_ipc_VideoPlay(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VideoPlay_Req *req,
    /* output  */ ipc_VideoPlay_Resp *rsp);

bool dev_on_ipc_VideoPause(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_VideoPause_Req *req,
    /* output  */ ipc_VideoPause_Resp *rsp);

bool dev_on_ipc_VideoQosSet(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_VideoQosSet_Req *req,
    /* output  */ ipc_VideoQosSet_Resp *rsp);

bool dev_on_ipc_FlipSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipSet_Req *req,
    /* output  */ ipc_FlipSet_Resp *rsp);

bool dev_on_ipc_AudioPlay(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_AudioPlay_Req *req,
    /* output  */ ipc_AudioPlay_Resp *rsp);

bool dev_on_ipc_AudioPause(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_AudioPause_Req *req,
    /* output  */ ipc_AudioPause_Resp *rsp);

bool dev_on_ipc_TalkbackPlay(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_TalkbackPlay_Req *req,
    /* output  */ ipc_TalkbackPlay_Resp *rsp);

bool dev_on_ipc_TalkbackPause(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_TalkbackPause_Req *req,
    /* output  */ ipc_TalkbackPause_Resp *rsp);

bool dev_on_ipc_HistoryPlanSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanSet_Req *req,
    /* output  */ ipc_HistoryPlanSet_Resp *rsp);

bool dev_on_ipc_HistoryPlanGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanGet_Req *req,
    /* output  */ ipc_HistoryPlanGet_Resp *rsp);

bool dev_on_ipc_HistoryDays(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryDays_Req *req,
    /* output  */ ipc_HistoryDays_Resp *rsp);

bool dev_on_ipc_HistoryDayList(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryDayList_Req *req,
    /* output  */ ipc_HistoryDayList_Resp *rsp);

bool dev_on_ipc_HistoryPlay(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryPlay_Req *req,
    /* output  */ ipc_HistoryPlay_Resp *rsp);

bool dev_on_ipc_HistoryPause(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_HistoryPause_Req *req,
    /* output  */ ipc_HistoryPause_Resp *rsp);

bool dev_on_ipc_HistoryThumGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryThumGet_Req *req,
    /* output  */ ipc_HistoryThumGet_Resp *rsp);

bool dev_on_ipc_HistoryDel(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_HistoryDel_Req *req,
    /* output  */ ipc_HistoryDel_Resp *rsp);

bool dev_on_ipc_ConfigGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_ConfigGet_Req *req,
    /* output  */ ipc_ConfigGet_Resp *rsp);

bool dev_on_ipc_TimeSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeSet_Req *req,
    /* output  */ ipc_TimeSet_Resp *rsp);

bool dev_on_ipc_TimeGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeGet_Req *req,
    /* output  */ ipc_TimeGet_Resp *rsp);

bool dev_on_ipc_LedSet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedSet_Req *req,
    /* output  */ ipc_LedSet_Resp *rsp);

bool dev_on_ipc_LedGet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedGet_Req *req,
    /* output  */ ipc_LedGet_Resp *rsp);

bool dev_on_ipc_IRCutSet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutSet_Req *req,
    /* output  */ ipc_IRCutSet_Resp *rsp);

bool dev_on_ipc_IRCutGet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutGet_Req *req,
    /* output  */ ipc_IRCutGet_Resp *rsp);

bool dev_on_ipc_SecretSet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretSet_Req *req,
    /* output  */ ipc_SecretSet_Resp *rsp);

bool dev_on_ipc_SecretGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretGet_Req *req,
    /* output  */ ipc_SecretGet_Resp *rsp);

bool dev_on_ipc_NotifySet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifySet_Req *req,
    /* output  */ ipc_NotifySet_Resp *rsp);

bool dev_on_ipc_NotifyGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifyGet_Req *req,
    /* output  */ ipc_NotifyGet_Resp *rsp);

bool dev_on_ipc_MotionzoneSet(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneSet_Req *req,
    /* output  */ ipc_MotionzoneSet_Resp *rsp);

bool dev_on_ipc_MotionzoneGet(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneGet_Req *req,
    /* output  */ ipc_MotionzoneGet_Resp *rsp);

bool dev_on_ipc_PspAdd(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspAdd_Req *req,
    /* output  */ ipc_PspAdd_Resp *rsp);

bool dev_on_ipc_PspDel(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspDel_Req *req,
    /* output  */ ipc_PspDel_Resp *rsp);

bool dev_on_ipc_PspList(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspList_Req *req,
    /* output  */ ipc_PspList_Resp *rsp);

bool dev_on_ipc_PspCall(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspCall_Req *req,
    /* output  */ ipc_PspCall_Resp *rsp);

bool dev_on_ipc_Reboot(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_Reboot_Req *req,
    /* output  */ ipc_Reboot_Resp *rsp);

bool dev_on_ipc_Reset(
    /* conn_id */ int32_t         conn_id,
    /* input   */ ipc_Reset_Req *req,
    /* output  */ ipc_Reset_Resp *rsp);

bool dev_on_ipc_FlipGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipGet_Req *req,
    /* output  */ ipc_FlipGet_Resp *rsp);

bool dev_on_ipc_LanAuth(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_LanAuth_Req *req,
    /* output  */ ipc_LanAuth_Resp *rsp);

bool dev_on_ipc_TimedcruiseSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseSet_Req *req,
    /* output  */ ipc_TimedcruiseSet_Resp *rsp);

bool dev_on_ipc_TimedcruiseGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseGet_Req *req,
    /* output  */ ipc_TimedcruiseGet_Resp *rsp);

bool dev_on_ipc_StorageInfo(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_StorageInfo_Req *req,
    /* output  */ ipc_StorageInfo_Resp *rsp);

bool dev_on_ipc_StorageFormat(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_StorageFormat_Req *req,
    /* output  */ ipc_StorageFormat_Resp *rsp);

bool dev_on_ipc_StorageFormatRate(
    /* conn_id */ int32_t                     conn_id,
    /* input   */ ipc_StorageFormatRate_Req *req,
    /* output  */ ipc_StorageFormatRate_Resp *rsp);

bool dev_on_ipc_LogSet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LogSet_Req *req,
    /* output  */ ipc_LogSet_Resp *rsp);

bool dev_on_ipc_VolumeSet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeSet_Req *req,
    /* output  */ ipc_VolumeSet_Resp *rsp);

bool dev_on_ipc_PowerFreqSet(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqSet_Req *req,
    /* output  */ ipc_PowerFreqSet_Resp *rsp);

bool dev_on_ipc_PowerFreqGet(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqGet_Req *req,
    /* output  */ ipc_PowerFreqGet_Resp *rsp);

bool dev_on_ipc_VolumeGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeGet_Req *req,
    /* output  */ ipc_VolumeGet_Resp *rsp);

bool dev_on_ipc_AlarmSet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmSet_Req *req,
    /* output  */ ipc_AlarmSet_Resp *rsp);

bool dev_on_ipc_AlarmGet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmGet_Req *req,
    /* output  */ ipc_AlarmGet_Resp *rsp);

bool dev_on_ipc_Screenshot(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_Screenshot_Req *req,
    /* output  */ ipc_Screenshot_Resp *rsp);

bool dev_on_ipc_PtzCtrl(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PtzCtrl_Req *req,
    /* output  */ ipc_PtzCtrl_Resp *rsp);

bool dev_on_ipc_SetAutoTrack(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_SetAutoTrack_Req  *req,
    /* output  */ ipc_SetAutoTrack_Resp *rsp);

bool dev_on_ipc_GetAutoTrack(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_GetAutoTrack_Req  *req,
    /* output  */ ipc_GetAutoTrack_Resp *rsp);

bool dev_on_ipc_FirmwareNotify(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_FirmwareNotify_Req *req,
    /* output  */ ipc_FirmwareNotify_Resp *rsp);

bool dev_on_ipc_FirmwareRate(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_FirmwareRate_Req *req,
    /* output  */ ipc_FirmwareRate_Resp *rsp);

bool dev_on_ipc_EventRecordGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordGet_Req *req,
    /* output  */ ipc_EventRecordGet_Resp *rsp);

bool dev_on_ipc_EventRecordSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordSet_Req *req,
    /* output  */ ipc_EventRecordSet_Resp *rsp);

bool dev_on_ipc_GetNetworkInfo(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_GetNetworkInfo_Req *req,
    /* output  */ ipc_GetNetworkInfo_Resp *rsp);

bool dev_on_ipc_FileStart(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_FileStart_Req *req,
    /* output  */ ipc_FileStart_Resp *rsp);

bool dev_on_ipc_FileStop(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_FileStop_Req *req,
    /* output  */ ipc_FileStop_Resp *rsp);

bool dev_on_ipc_CustomCmd(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_CustomCmd_Req *req,
    /* output  */ ipc_CustomCmd_Resp *rsp);

bool dev_on_ipc_PirSet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_PirSet_Req *req,
    /* output  */ ipc_PirSet_Resp *rsp);

bool dev_on_ipc_PirGet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_PirGet_Req *req,
    /* output  */ ipc_PirGet_Resp *rsp);

bool dev_on_ipc_ChanState(
    /* conn_id */ int32_t			  conn_id,
    /* input   */ ipc_ChanState_Req  *req,
    /* output  */ ipc_ChanState_Resp *rsp);

bool dev_on_ipc_VideoChanChange(
    /* conn_id */ int32_t			 conn_id,
    /* input   */ ipc_VideoChanChange_Req *req,
    /* output  */ ipc_VideoChanChange_Resp *rsp);

bool dev_on_ipc_TamperSet(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_TamperSet_Req *req,
    /* output  */ ipc_TamperSet_Resp *rsp);


bool dev_on_ipc_DirCreate(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirCreate_Req *req,
    /* output  */ ipc_DirCreate_Resp *rsp);
bool dev_on_ipc_DirDel(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirDel_Req *req,
    /* output  */ ipc_DirDel_Resp *rsp);
bool dev_on_ipc_DirEdit(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirEdit_Req *req,
    /* output  */ ipc_DirEdit_Resp *rsp);
bool dev_on_ipc_DirList(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirList_Req *req,
    /* output  */ ipc_DirList_Resp *rsp);
bool dev_on_ipc_FileAdd(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileAdd_Req *req,
    /* output  */ ipc_FileAdd_Resp *rsp);
bool dev_on_ipc_FileDel(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileDel_Req *req,
    /* output  */ ipc_FileDel_Resp *rsp);
bool dev_on_ipc_FileEdit(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileEdit_Req *req,
    /* output  */ ipc_FileEdit_Resp *rsp);
bool dev_on_ipc_FileRecvRate(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileRecvRate_Req *req,
    /* output  */ ipc_FileRecvRate_Resp *rsp);
bool dev_on_ipc_FileThumList(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileThumList_Req *req,
    /* output  */ ipc_FileThumList_Resp *rsp);
bool dev_on_ipc_ExecIOTCMD(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_ExecIOTCMD_Req *req,
    /* output  */ ipc_ExecIOTCMD_Resp *rsp);

void dev_on_RecvFile(int32_t            conn_id,
                     ipc_RecvFile_Req   *req,
                     ipc_RecvFile_Resp  *rsp);

void dev_on_ipc_talkbackcb(int32_t              conn_id,
                           iot_exhdr_media_t    *exhdr,
                           const uint8_t        *media_data,
                           size_t               media_len);

bool dev_on_ipc_SetLowPower(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_SetLowPower_Req *req,
    /* output  */ ipc_SetLowPower_Resp *rsp);

bool dev_on_ipc_GetLowPower(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_GetLowPower_Req *req,
    /* output  */ ipc_GetLowPower_Resp *rsp);

bool dev_on_ipc_ForceIFrame(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_ForceIFrame_Req *req,
    /* output  */ ipc_ForceIFrame_Resp *rsp);

void custom_thread(void *arg);


#endif
/****************************************************************************/
#ifdef __cplusplus
}
#endif
/*****************************************************************************
 * @} (end addtogroup ut_dev_ipc_cmd)
 */
/****************************************************************************/
#endif /* UT_DEV_IPC_CMD_H_INCLUDE */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
