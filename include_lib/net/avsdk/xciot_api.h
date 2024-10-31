/*****************************************************************************
 * xciot_api.h
 *
 *
 *
 ****************************************************************************/
/*
 * wiki
 * ====
 */
#ifndef XCIOT_API_H_INCLUDE
#define XCIOT_API_H_INCLUDE
/*****************************************************************************
 * @addtogroup xciot_api
 * @{
 */
/*****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
//#include <stdbool.h>
#include <typedef.h> //bool

#include "xciot_type.h"
#include "xciot_cmd_common.h"
#include "xciot_cmd_ipc.h"
#include "xc_cs_type.h"

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

/**
 * @brief 查询avsdk的版本号.
 *
 * @return  返回avsdk的版本号.
 */
char *avsdk_get_sdk_ver(void);

/**
 * @brief 设置SDK日志等级.
 *
 * @param[in]  level - 日志等级 see @ref iot_log_level_t.
 *
 * @return status code that indicates the execution status of the function.
 */
void avsdk_set_loglevel(iot_log_level_t level);

/**
 * @brief 注册SDK日志底层输出函数.
 *
 * @param[in]  domain     - 目前未使用该字段，填NULL.
 * @param[in]  log_output - 用户log输出函数.
 *
 * @return 无.
 */
void avsdk_log_lowPrintf(char *domain, int (*log_output)(const char *fmt, ...));

/**
 * @brief SDK 初始化.
 *
 * @param[in]  init_params - 初始化参数. 其中版本号填“”，则需要调用avsdk_report_version主动上报版本号
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_init(avsdk_init_params_t *init_params);

/**
 * @brief SDK 反初始化清除.
 */
void avsdk_fini(void);

/**
 * @brief SDK 启动.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_start(void);

/**
 * @brief SDK 断开和平台的连接.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_stop(void);

/**
 * @brief SDK 休眠.
 *
 * @param[out] sleep - 休眠参数 @ref iot_sleep_t.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_sleep(iot_sleep_t *sleep);

/**
 * @brief 查询连接平台是否成功.
 *
 *
 * @return true 连接平台成功， false失败.
 */
bool avsdk_poll_conn_platfrom(void);

/**
 * @brief SDK 休眠参数序列化.
 *
 * @param[in]  sleep  - 休眠参数,通过avsdk_sleep获取.
 *
 * @param[out] outArr - 序列化后的数据，建议512B的大小数组.
 *
 * @param[out] arrlen - 序列化后的长度.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_tools_sleep2byte(iot_sleep_t *sleep,
                           uint8_t     *outArr,
                           uint16_t    *arrlen);

/**
 * @brief SDK 快速初始化.
 *
 * @param[in] faststart_data    - 快速初始化参数内容.
 *
 * @param[in] faststart_datalen - 快速初始化参数内容长度.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_faststart(const uint8_t *faststart_data, size_t faststart_datalen);

/**
 * @brief SDK 周期性循环.
 */
void avsdk_loop(void);

/**
 * @brief 解除设备的绑定关系.
 */
int avsdk_unbind(void);

/**
 * @brief SDK 外部接管ppmq_handle
 *
 * @param[in] hdl   - 为avsdk_set_on_ppmq_handle返回的值。
 *
 * @return 返回值小于0，不再调用avsdk_ppmq_handle；
 *         大于0为多少秒后再次调用avsdk_ppmq_handle.
 */
int avsdk_ppmq_handle(void *hdl);

/**
 * @brief 返回当前用户连接数.
 *
 * @return 返回当前连接数
 */
int avsdk_poll_current_conn_num(void);

/**
 * @brief SDK errno 转化为字符串.
 */
const char *avsdk_strerror(int errnum);

/**
 * @brief 往连接写入流媒体数据.
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] iskeyframe      - 是不是关键帧(i frame set true or false).
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头.
 *
 * @param[in] enctype         - 加密类型 @ref iot_encrypt_type_t.
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] enclength       - 加密长度，默认 0.
 *
 * @param[in] mediadata       - 媒体数据.
 *
 * @param[in] mediadatalength - 媒体数据长度.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_write_media_v1(int32_t                conn_id,
                         iot_mediastream_type_t streamtype,
                         bool                   iskeyframe,
                         uint64_t               avchannel,
                         iot_encrypt_type_t     enctype,
                         uint64_t               timestamp,
                         uint64_t               enclength,
                         const uint8_t         *mediadata,
                         size_t                 mediadatalength);

/**
 * @brief 往连接写入流媒体数据V2.
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] iskeyframe      - 是不是关键帧(i frame set true or false).
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头
 *
 * @param[in] enctype         - 加密类型 @ref iot_encrypt_type_t.
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] enclength       - 加密长度默认填0
 *
 * @param[in] source		  - 媒体数据来源 @ref iot_mediasource_type_t.
 *
 * @param[in] file_id		  - 文件id.
 *
 * @param[in] exdata		  - Default NULL.
 *
 * @param[in] exdatalength    - Default 0.
 *
 * @param[in] mediadata       - 媒体数据.
 *
 * @param[in] mediadatalength - 媒体数据长度.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_write_media_v2(int32_t                conn_id,
                         iot_mediastream_type_t streamtype,
                         bool                   iskeyframe,
                         uint64_t               avchannel,
                         iot_encrypt_type_t     enctype,
                         uint64_t               timestamp,
                         uint64_t               enclength,
                         iot_mediasource_type_t source,
                         uint64_t               file_id,
                         const uint8_t         *exdata,
                         uint64_t               exdatalength,
                         const uint8_t         *mediadata,
                         size_t                 mediadatalength);

/**
 * @brief 往连接写入文件数据.
 *
 * @param[in] conn_id        - 连接ID.
 *
 * @param[in] fileid         - 文件id.
 *
 * @param[in] offset         - 文件传输偏移.
 *
 * @param[in] enctype        - 加密类型 @ref iot_encrypt_type_t,,默认E_IOT_ENCRYPT_TYPE_AES256CBC.
 *
 * @param[in] enclength      - 加密长度默认填0
 *
 * @param[in] data           - 文件数据.
 *
 * @param[in] datalength     - 数据长度.
 *
 * @return 0成功，-8表示发送队列满了，需要调用者重发，否则文件不完整.
 */
int avsdk_write_file(int32_t            conn_id,
                     uint64_t           fileid,
                     uint64_t           offset,
                     iot_encrypt_type_t enctype,
                     uint64_t           enclength,
                     const uint8_t     *data,
                     size_t             datalength);
/**
 * @brief 向连接发送自定义命令.
 *
 * @param[in] conn_id        - 连接ID.
 *
 * @param[in] req            - 命令请求.
 *
 * @param[in] rsp            - 命令回应.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_custom_cmd(int32_t             conn_id,
                     ipc_CustomCmd_Req *req,
                     ipc_CustomCmd_Resp *rsp);

/**
 * @brief 向连接发送固件升级进度（不能在回调中直接调用，需要异步调用）.
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] channel         - 通道编号，如果是IPC，该值为0.
 *
 * @param[in] rate            - 升级进度: 0-100. -1表示失败
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_firmware_rate(int32_t conn_id, int32_t channel, int32_t rate);

/**
 * @brief 向所有连接发送固件升级进度（不能在回调中直接调用，需要异步调用）.
 *
 * @param[in] channel         - 通道编号，如果是IPC，该值为0.
 *
 * @param[in] rate            - 升级进度: 0-100. -1表示失败
 *
 * @return 无.
 */
void avsdk_firmware_rate_v2(int32_t channel, int32_t rate);

/**
 * @brief 向连接发送存储格式化进度（不能在回调中直接调用，需要异步调用）.
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] channel         - 通道编号，如果是IPC，该值为0.
 *
 * @param[in] rate            - 格式化进度: 0-100; -1 格式化失败.
 *
 * @return status code that indicates the execution status of the function.
 */

int avsdk_storage_format_rate(int32_t conn_id, int32_t channel, int32_t rate);

/**
 * @brief 防撬上报，当防撬状态变化时，触发.
 *
 * @param[in] status - 设备当前防撬状态.
 */
int avsdk_report_tamper_status(iot_tamper_status_t status);

/**
 * @brief 平台第一次连接通知,可以用于状态上报等；注意：该回调只会调用一次.
 *
 * @param[in] cb - 平台连接通知回调函数.
 */
void avsdk_set_on_connect_platform(iot_on_connect_platform_t cb);

/**
 * @brief 平台重新连接通知回调注册,可以用于联网状态灯维护等，平台后续连接状态变化都会触发该回调.
 *
 * @param[in] cb - 平台连接通知回调函数.
 */
void avsdk_set_on_reconnect_platform(iot_on_connect_platform_t cb);

/**
 * @brief 通知用户ppmq_handle入参的值.
 *
 * @param[in] cb - 连接平台utc回调函数.
 */
void avsdk_set_on_ppmq_handle(iot_on_ppmq_handle_t cb);

/**
 * @brief 平台连接UTC回调注册.
 *
 * @param[in] cb - 连接平台utc回调函数.
 */
void avsdk_set_on_platform_utc(iot_on_platform_utc_t cb);

/**
 * @brief 用户连接通知回调注册.
 *
 * @param[in] on_connect - 用户连接通知回调函数.
 */
void avsdk_set_on_connect(iot_on_connect_t on_connect);

/**
 * @brief 流媒体数据数据回调注册.
 *
 * @param[in] on_media - 用户流媒体数据回调函数.
 */
void avsdk_set_on_media(iot_on_media_t on_media);

/**
 * @brief 文件数据回调注册.
 *
 * @param[in] on_RecvFile - 用户文件数据回调函数.
 */
void avsdk_set_on_RecvFile(iot_on_RecvFile_t on_RecvFile);

/**
 * @brief 音视频丢包回调注册.
 *
 * @param[in] cb - 回调函数.
 */
void avsdk_set_on_PacketDropout(iot_on_PacketDropout_t cb);

/**
 * @brief 添加到视频播放组.
 *
 * @param[in] conn_id         - 连接id.
 *
 * @param[in] ch              - 添加哪一个通道.
 *
 * @return 无.
 */
void avsdk_video_add_conn(int conn_id, int ch);

/**
 * @brief 从视频播放组删除.
 *
 * @param[in] conn_id         - 连接id.
 *
 * @param[in] ch              - 删除哪一个通道，-1表示全部删除.
 *
 * @return 无.
 */
void avsdk_video_del_conn(int conn_id, int ch);
/**
 * @brief 添加到音频播放组.
 *
 * @param[in] conn_id         - 连接id.
 *
 * @return 无.
 */
void avsdk_audio_add_conn(int conn_id);

/**
 * @brief 从音频播放组删除.
 *
 * @param[in] conn_id         - 连接id.
 *
 * @return 无.
 */
void avsdk_audio_del_conn(int conn_id);

/**
 * @brief 添加到自定义命名用户组.
 *
 * @param[in] conn_id         - 连接id.
 *
 * @return 无.
 */
void avsdk_custom_add_conn(int conn_id);

/**
 * @brief 从自定义命名用户组删除.
 *
 * @param[in] conn_id         - 连接id.
 *
 * @return 无.
 */
void avsdk_custom_del_conn(int conn_id);

/**
 * @brief AI描述信息序列化，填充到流数据exdata段
 *
 * @param[in] AIDesc	- 对象描述信息 @ref ipc_AIDesc.
 *
 * @param[out] len		- 序列化后，数据大小
 *
 * @return 成功返回序列化后的数据，失败返回NULL；返回非NULL需要Free内存.
 */
uint8_t *avsdk_AIDesc_encode(ipc_AIDesc *AIDesc, uint32_t *len);

/**
 * @brief 发送音频媒体数据，连接内部管理（推荐使用）.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头.
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] AIDesc		  - 对数据流的描述具体参考ipc_AIDesc定义，没有填NULL.
 *
 * @param[in] payload_ptr     - 媒体数据.
 *
 * @param[in] payload_len	  - 媒体数据长度.
 *
 * @return 无.
 */
void avsdk_write_audio(iot_mediastream_type_t stream_type,
                       uint64_t               avchannel,
                       uint64_t               timestamp_ms,
                       ipc_AIDesc            *AIDesc,
                       const uint8_t         *payload_ptr,
                       uint32_t               payload_len);

/**
 * @brief 发送音频媒体数据，连接内部管理.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头.
 *
 * @param[in] enctype         - 加密类型 @ref iot_encrypt_type_t,默认E_IOT_ENCRYPT_TYPE_AES256CBC.
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] enclength       - 加密长度，默认 0.
 *
 * @param[in] exdata		  - 额外数据，对数据流的描述，没有填NULL.
 *
 * @param[in] exdatalength	  - 额外数据长度,没有填0.
 *
 * @param[in] payload_ptr     - 媒体数据.
 *
 * @param[in] payload_len	  - 媒体数据长度.
 *
 * @return 无.
 */

void avsdk_write_audio_data(iot_mediastream_type_t stream_type,
                            uint64_t               avchannel,
                            iot_encrypt_type_t     encrypt_type,
                            uint64_t               timestamp_ms,
                            uint64_t               encrypt_length,
                            const uint8_t         *exdata,
                            uint64_t               exdatalength,
                            const uint8_t         *payload_ptr,
                            uint32_t               payload_len);

/**
 * @brief 往连接connid写入音频媒体数据.
 *
 * @param[in] connid		  - 连接id.
 *
 * @param[in] streamtype	  - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] avchannel 	  - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头.
 *
 * @param[in] enctype		  - 加密类型 @ref iot_encrypt_type_t,默认E_IOT_ENCRYPT_TYPE_AES256CBC.
 *
 * @param[in] timestamp 	  - ms级时间戳.
 *
 * @param[in] enclength 	  - 加密长度，默认 0.
 *
 * @param[in] exdata		  - 额外数据，对数据流的描述，没有填NULL.
 *
 * @param[in] exdatalength	  - 额外数据长度,没有填0.
 *
 * @param[in] payload_ptr	  - 媒体数据.
 *
 * @param[in] payload_len	  - 媒体数据长度.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_write_audio_data_with_connid(int32_t                conn_id,
                                       iot_mediastream_type_t stream_type,
                                       uint64_t               avchannel,
                                       iot_encrypt_type_t     encrypt_type,
                                       uint64_t               timestamp_ms,
                                       uint64_t               encrypt_length,
                                       const uint8_t         *exdata,
                                       uint64_t               exdatalength,
                                       const uint8_t         *payload_ptr,
                                       uint32_t               payload_len);

/**
 * @brief 发送视频媒体数据.（推荐使用）
 *
 * @param[in] streamtype		- 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] iskeyframe		- 是不是关键帧(i frame set true or false).
 *
 * @param[in] avchannel			- 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头.
 *
 * @param[in] timestamp			- ms级时间戳.
 *
 * @param[in] AIDesc		  	- 对数据流的描述具体参考ipc_AIDesc定义，没有填NULL.
 *
 * @param[in] payload_ptr 		- 媒体数据.
 *
 * @param[in] payload_len 		- 媒体数据长度.
 *
 * @return 无.
 */
void avsdk_write_video(iot_mediastream_type_t stream_type,
                       bool                   is_key_frame,
                       uint64_t               avchannel,
                       uint64_t               timestamp_ms,
                       ipc_AIDesc            *AIDesc,
                       const uint8_t         *payload_ptr,
                       uint32_t               payload_len);

/**
 * @brief 发送视频媒体数据.
 *
 * @param[in] streamtype		- 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] iskeyframe		- 是不是关键帧(i frame set true or false).
 *
 * @param[in] avchannel			- 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头.
 *
 * @param[in] enctype			- 加密类型 @ref iot_encrypt_type_t,默认E_IOT_ENCRYPT_TYPE_AES256CBC.
 *
 * @param[in] timestamp			- ms级时间戳.
 *
 * @param[in] enclength			- 加密长度，默认 0.
 *
 * @param[in] exdata		  	- 额外数据，对数据流的描述，没有填NULL.
 *
 * @param[in] exdatalength	  	- 额外数据长度,没有填0.
 *
 * @param[in] payload_ptr 		- 媒体数据.
 *
 * @param[in] payload_len 		- 媒体数据长度.
 *
 * @return 无.
 */
void avsdk_write_video_data(iot_mediastream_type_t stream_type,
                            bool                   is_key_frame,
                            uint64_t               avchannel,
                            iot_encrypt_type_t     encrypt_type,
                            uint64_t               timestamp_ms,
                            uint64_t               encrypt_length,
                            const uint8_t         *exdata,
                            uint64_t               exdatalength,
                            const uint8_t         *payload_ptr,
                            uint32_t               payload_len);

/**
 * @brief 往连接写入视频媒体数据.
 *
 * @param[in] connid		  	- 连接id.
 *
 * @param[in] streamtype		- 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] iskeyframe		- 是不是关键帧(i frame set true or false).
 *
 * @param[in] avchannel 		- 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头.
 *
 * @param[in] enctype			- 加密类型 @ref iot_encrypt_type_t,默认E_IOT_ENCRYPT_TYPE_AES256CBC.
 *
 * @param[in] timestamp 		- ms级时间戳.
 *
 * @param[in] enclength 		- 加密长度，默认 0.
 *
 * @param[in] exdata			- 额外数据，对数据流的描述，没有填NULL.
 *
 * @param[in] exdatalength		- 额外数据长度,没有填0.
 *
 * @param[in] payload_ptr		- 媒体数据.
 *
 * @param[in] payload_len		- 媒体数据长度.
 *
 * @return status code that indicates the execution status of the function.
 */
int avsdk_write_video_data_with_connid(int32_t                conn_id,
                                       iot_mediastream_type_t stream_type,
                                       bool                   is_key_frame,
                                       uint64_t               avchannel,
                                       iot_encrypt_type_t     encrypt_type,
                                       uint64_t               timestamp_ms,
                                       uint64_t               encrypt_length,
                                       const uint8_t         *exdata,
                                       uint64_t               exdatalength,
                                       const uint8_t         *payload_ptr,
                                       uint32_t               payload_len);

/**
 * @brief 往连接写入历史音频数据.（推荐使用）
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] file_id		  - 文件id.
 *
 * @param[in] AIDesc		  - 对数据流的描述具体参考ipc_AIDesc定义，没有填NULL.
 *
 * @param[in] payload_ptr     - 媒体数据.
 *
 * @param[in] payload_len 	  - 媒体数据长度.
 *
 * @return 0成功，-9 conn_id已断开，需要退出sd发送，其他返回值可以暂时不处理.
 */
int avsdk_write_history_audio(int32_t                conn_id,
                              iot_mediastream_type_t stream_type,
                              uint64_t               avchannel,
                              uint64_t               timestamp_ms,
                              uint64_t               file_id,
                              ipc_AIDesc            *AIDesc,
                              const uint8_t         *payload_ptr,
                              uint32_t               payload_len);

/**
 * @brief 往连接写入历史音频数据.
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头
 *
 * @param[in] enctype         - 加密类型 @ref iot_encrypt_type_t,默认E_IOT_ENCRYPT_TYPE_AES256CBC.
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] enclength       - 加密长度默认填0
 *
 * @param[in] file_id		  - 文件id.
 *
 * @param[in] exdata		  - 额外数据，对数据流的描述，没有填NULL.
 *
 * @param[in] exdatalength	  - 额外数据长度,没有填0.
 *
 * @param[in] payload_ptr     - 媒体数据.
 *
 * @param[in] payload_len 	  - 媒体数据长度.
 *
 * @return 0成功，-9 conn_id已断开，需要退出sd发送，其他返回值可以暂时不处理.
 */
int avsdk_write_history_audio_data(int32_t                conn_id,
                                   iot_mediastream_type_t stream_type,
                                   uint64_t               avchannel,
                                   iot_encrypt_type_t     encrypt_type,
                                   uint64_t               timestamp_ms,
                                   uint64_t               encrypt_length,
                                   uint64_t               file_id,
                                   const uint8_t         *exdata,
                                   uint64_t               exdatalength,
                                   const uint8_t         *payload_ptr,
                                   uint32_t               payload_len);

/**
 * @brief 往连接写入历史视频数据.（推荐使用）
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] iskeyframe      - 是不是关键帧(i frame set true or false).
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] file_id		  - 文件id.
 *
 * @param[in] AIDesc		  - 对数据流的描述具体参考ipc_AIDesc定义，没有填NULL.
 *
 * @param[in] payload_ptr     - 媒体数据.
 *
 * @param[in] payload_len 	  - 媒体数据长度.
 *
 * @return 0成功，-9 conn_id已断开，需要退出sd发送，其他返回值可以暂时不处理.
 */
int avsdk_write_history_video(int32_t                conn_id,
                              iot_mediastream_type_t stream_type,
                              bool                   is_key_frame,
                              uint64_t               avchannel,
                              uint64_t               timestamp_ms,
                              uint64_t               file_id,
                              ipc_AIDesc            *AIDesc,
                              const uint8_t         *payload_ptr,
                              uint32_t               payload_len);

/**
 * @brief 往连接写入历史视频数据.
 *
 * @param[in] conn_id         - 连接ID.
 *
 * @param[in] streamtype      - 音视频数据类型 @ref iot_mediastream_type_t.
 *
 * @param[in] iskeyframe      - 是不是关键帧(i frame set true or false).
 *
 * @param[in] avchannel       - 0, 表示设备本身；IPC都为0；如果是NVR，表示第几路摄像头
 *
 * @param[in] enctype         - 加密类型 @ref iot_encrypt_type_t,默认E_IOT_ENCRYPT_TYPE_AES256CBC.
 *
 * @param[in] timestamp       - ms级时间戳.
 *
 * @param[in] enclength       - 加密长度默认填0
 *
 * @param[in] file_id		  - 文件id.
 *
 * @param[in] exdata		  - 额外数据，对数据流的描述，没有填NULL.
 *
 * @param[in] exdatalength	  - 额外数据长度,没有填0.
 *
 * @param[in] payload_ptr     - 媒体数据.
 *
 * @param[in] payload_len 	  - 媒体数据长度.
 *
 * @return 0成功，-9 conn_id已断开，需要退出sd发送，其他返回值可以暂时不处理.
 */
int avsdk_write_history_video_data(int32_t                conn_id,
                                   iot_mediastream_type_t stream_type,
                                   bool                   is_key_frame,
                                   uint64_t               avchannel,
                                   iot_encrypt_type_t     encrypt_type,
                                   uint64_t               timestamp_ms,
                                   uint64_t               encrypt_length,
                                   uint64_t               file_id,
                                   const uint8_t         *exdata,
                                   uint64_t               exdatalength,
                                   const uint8_t         *payload_ptr,
                                   uint32_t               payload_len);
/**
 * @brief 用户自定义命令.
 *
 * @param[in] ipc_CustomCmd_Req           - 用户自定义请求.
 *
 * @param[out] ipc_CustomCmd_Resp         - 用户自定义返回.
 *
 * @return 0成功，负数失败.
 */
int avsdk_custom_cmd_send(ipc_CustomCmd_Req *req, ipc_CustomCmd_Resp *rsp);

/**
 * @brief 文件接收进度上报.
 *
 * @param[in] req          - 进度上报请求.
 *
 * @param[out] rsp         - 进度上报回应.
 *
 * @return 0成功.
 */
int avsdk_report_FileRecvRate(int32_t                conn_id,
                              ipc_FileRecvRate_Req *req,
                              ipc_FileRecvRate_Resp *rsp);

/**
 * @brief 打印PpiotCmd所有的参数.
 *
 * @param[in] perfix    - 打印前缀.
 *
 * @param[in] p         - 需要打印的结构体的指针.
 *
 * @return 0成功.
 */
void avsdk_dump_PpiotCmd(char *perfix, iot_PpiotCmd_t *p);

/**
 * @brief 异常事件上报.
 *
 * @param[in] event  - 异常事件.
 *
 * @return 返回值：为负数表示错误; 等于0事件上报成功，但是设备没开通云存; 大于0表示上报成功且设备开通了云存.
 */
int64_t avsdk_push_event(iot_event_opt_t *event);

/**
 * @brief 设备状态上报.
 *
 * @param[in] status_id         - 设备状态id.
 *
 * @param[in] timestamp         - 发生时间的时间戳ms.
 *
 * @param[in] peopleID         - 人脸识别id,没有该功能填NULL.
 *
 * @return 0正确.
 */
int avsdk_status_pulish(iot_status_id_t status_id,
                        uint64_t        timestamp,
                        ipc_PeopleID_t *peopleID);
/**
 * @brief 上报基本信息(平台和连接都会发送) (70)
 *
 * @param[in]   p  - 设备基本信息（包括初始化传入的版本号）
 *
 * @return  0成功.
 */
int avsdk_push_info(dev_base_info_t *p);

/**
 * @brief 设备时区变更上报.
 *
 * @param[in] timeZone 		- 标准时区字符串.
 *
 * @return 0正确.
 */
int avsdk_report_TimeZone(char *timeZone);

/**
 * @brief 运营网络参数上报.
 *
 * @param[in] ICCID 		- ICCID.
 *
 * @param[in] IMEI 			- IMEI.
 *
 * @param[in] IMSI		   	- IMSI.
 *
 * @return 0正确.
 */
int avsdk_report_NetworkParams(char *ICCID, char *IMEI, char *IMSI);

/**
 * @brief 网络信号质量上报.
 *
 * @param[in] QOS 		- 网络质量: 1-6数字越大网络质量越好，
 *                        0 - 不支持，1 - 无信号
 *
 * @return 0正确.
 */
int avsdk_report_NetworkQuality(uint8_t QOS);

/**
 * @brief 电池电量上报(变化5%或者低电量触发).
 *
 * @param[in] quantity 		- 当前电量: 1-100，百分比，0表示没有电池。
 *
 * @param[in] status 		- 当前是否在充电，@iot_status_power_t。
 *
 * @param[in] voltage 		- 当前电压(mv); 0：不支持电压检测。
 *
 * @param[in] use_time 		- 可用时间(单位秒): 0:不支持。
 *
 * @return 0正确.
 */
int avsdk_report_Battary_v2(uint8_t            quantity,
                            iot_status_power_t status,
                            uint32_t           voltage,
                            uint32_t           use_time);

/**
 * @brief 电池电量上报(变化5%或者低电量触发).
 *
 * @param[in] quantity 		- 当前电量: 1-100，百分比，0表示没有电池。
 *
 * @param[in] status 		- 当前是否在充电，@iot_status_power_t。
 *
 * @return 0正确.
 */
int avsdk_report_Battary(uint8_t quantity, iot_status_power_t status);

/**
 * @brief 上报版本号，如果avsdk_init的时候rom_ver填NULL,
 *         那么sdk不会自动上报版本号，需要用户调用次接口主动上报版本号
 *
 * @param[in] rom_ver    - 设备的软件版本号，格式必须为“1.01.01.03_190618”，否则上报失败
 *
 * @param[in] hdw_ver    - 设备的硬件版本号
 *
 * @return 0正确.
 */
int avsdk_report_version(char *rom_ver, char *hdw_ver);

/**
 * @brief 上报子模块版本号
 *
 * @param[in] ch    	 - 通道，从1开始
 *
 * @param[in] rom_ver    - 子设备的软件版本号，格式必须为“1.01.01.03_190618”，否则上报失败
 *
 * @return 0正确.
 */
int avsdk_report_version_ch(int ch, char *rom_ver);

/**
 * @brief 上报获取的安全码,只需要配网的时候上报一次.(扫描配网)
 *
 * @param[in] vcode    - 获取到的安全码
 *
 * @return 0正确.
 */
int avsdk_report_vcode(char *vcode);

/**
 * @brief 上报扩展字段（设备特有配置）
 *
 * @param[in] data   - 数据
 *
 * @param[in] len    - 数据长度
 *
 * @return 0正确.
 */
int avsdk_report_ExtendConfig(uint8_t *data, uint16_t len);

/**
 * @brief 设备固件版本查询.
 *
 * @param[out] ipc_FirmwareCheckByDevice_Resp    -固件信息
 *
 * @return 0正确.
 */
int avsdk_firmware_poll(ipc_FirmwareCheckByDevice_Resp *rsp);

/**
 * @brief 子模块固件版本查询.
 *
 * @param[in ] ipc_FirmwareChanCheckByDevice_Req    -子组版本信息
 * @param[out] ipc_FirmwareCheckByDevice_Resp       -子组模块待升级的固件信息
 *
 * @return 0正确.
 */
int avsdk_firmware_poll_chan(ipc_FirmwareChanCheckByDevice_Req              *req,
                             ipc_FirmwareChanCheckByDevice_Resp *rsp);


/**
 * @brief uint32形式的ipaddr转字符串的ip地址.
 *
 * @param[in]  ipaddr    - uint32形式的ipaddr
 *
 * @param[out] ip        - 字符串形式的ip地址
 *
 * @return ip.
 */
char *avsdk_ntoa(uint32_t ipaddr, char *ip);

/**
 * @brief 字符串的ip地址转uint32形式的ipaddr.
 *
 * @param[in]  ip        - 字符串形式的ip地址
 *
 * @param[out] ipaddr    - uint32形式的ipaddr
 *
 * @return 0正确.
 */
int avsdk_aton(char *ip, uint8_t ipaddr[4]);

/**
 * @brief 获取白名单IP（4G热点）.
 *
 * @param[out] rsp    - IP白名单，ipv4使用avsdk_ntoa转换
 *
 * @return 0正确.
 */
int avsdk_ScpeGetWhitelist(ipc_ScpeGetWhitelist_Resp *rsp);

/**
 * @brief 设备发送GPS数据到iotdata微服务.
 *
 * @param[in]   longitude     - 经度: 经度每隔0.00001度，距离相差约1米（-90 到 +90）;经度 = 真实经度 * 100000 取整的值
 *
 * @param[in]   latitude      - 纬度: 纬度每隔0.00001度，距离相差约1.1米 （-180 到 +180）;纬度 = 真实纬度 * 100000 取整的值
 *
 * @param[in]   speed         - 速度(m/s);速度 = 真实速度 * 10 取整的值
 *
 * @param[in]   angle         - 方向范围0-360，整形，以正北为0度，顺时针增加;如果值为166600，表示不支持方向功能。(保留小数点后面2位，存放数据扩大100倍变为整数)
 *
 * @param[in]   acceleration  - 加速度: 0 不支持，或者没有急加/减速; 1 急加速; 2 急减速
 *
 * @param[in]   utcms         - UTC时间戳，单位ms
 *
 * @return 0正确.
 */
int avsdk_put_iotdata_gps(int64_t   longitude,
                          int64_t   latitude,
                          int64_t   speed,
                          int64_t   angle,
                          int64_t   acceleration,
                          uint64_t  utcms);

/**
 * @brief 设备发送批量GPS数据到iotdata微服务.
 *
 * @param[in]   data     - gps数据
 *
 * @return 0正确.
 */
int avsdk_put_iotdata_gps_v2(gps_t *data);

/**
 * @brief 设备发送数据到iotdata微服务.
 *
 * @param[in]   p                 - 数据
 *
 * @return 0正确.
 */
int avsdk_put_iotdata(iot_PpiotCmd_t *p);

/**
 * @brief 设备上报清扫记录.
 *
 * @param[in]   p - 数据
 *
 * @return 0正确.
 */
int avsdk_report_SSWPMapClean(iot_SSWPMapCleanReport_t *p);

/**
 * @brief 设备上报地图.
 *
 * @param[in]   p - 数据
 *
 * @return 0正确.
 */
int avsdk_report_MapAdd(iot_SSWPMapAddByDevice_t *p);

/**
 * @brief 设备发送数据到app.
 *
 * @param[in]   req                 - 发送的数据
 *
 * @param[in]   rsp                 - 应答的数据
 *
 * @return 0正确.
 */
int avsdk_push_ppiotcmd(int32_t             conn_id,
                        iot_PpiotCmd_t     *req,
                        iot_PpiotCmd_t     *rsp);

/**
 * @brief 设备发送数据到app(所有的连接都会发送一份).
 *
 * @param[in]   req                 - 发送的数据
 *
 * @return 0正确.
 */
int avsdk_push_ppiotcmd2(ipc_ExecIOTCMD_Req *req);

/**
 * @brief 查询当前设备云存支持的服务类型.
 *
 * @return 枚举服务类型 0,无服务 1.事件云存储; 2.人脸识别; 3.短信服务; 4. 事件图片(jpg); 200全天云存.
 */
cs_service_type_t avsdk_cs_get_service_type(void);

/**
 * @brief 查询当前设备云存支持的服务类型以及支持云存的事件id.
 *
 * @param[out]   type       - 0,无服务 1.事件云存储; 2.人脸识别; 3.短信服务; 4. 事件图片(jpg); 200全天云存.
 * @param[out]   event_arr  - 需要上报云存的事件列表，长度最大100Byte
 *
 * @return 0,成功
 */
int avsdk_cs_info(cs_service_type_t *type, char *event_arr);

/**
* @brief 设置云存能传输的最大图片.
*
* @param[in] maxsize     -设置最大能上传图片的大小，sdk内部默认512*1024,单位Byte
*
* @return 0正确; -1 maxsieze值大于1M.
*/
int avsdk_cs_set_picture_maxsize(uint32_t maxsize);

/**
 * @brief 云存图片(jpg)传输.
 *
 * @param[in] event_id		-异常事件id,@iot_event_id_t
 *
 * @param[in] channel		-通道，IPC为0
 *
 * @param[in] data          -需要上传的图片数据
 *
 * @param[in] data_len      -图片数据长度
 *
 * @param[in] peopleID      -人脸识别id,没有该功能填NULL.
 *
 * @param[in] xy            -识别到物体的坐标,没有该功能，应使用memset清零.
 *
 * @return 0正确.
 */
int avsdk_cs_picture_pulish(iot_event_id_t  event_id,
                            int64_t         channel,
                            uint8_t        *data,
                            uint32_t        data_len,
                            ipc_PeopleID_t *peopleID,
                            struct coord   *xy);

/**
 * @brief 云存图片(jpg)多张传输.
 *
 * @param[in] event_id		-异常事件id,@iot_event_id_t
 *
 * @param[in] channel		-通道，IPC为0
 *
 * @param[in] picture       -需要上传的图片数据
 *
 * @param[in] peopleID      -人脸识别id,没有该功能填NULL.
 *
 * @return 0正确.
 */
int avsdk_cs_picture_pulish_v2(iot_event_id_t  event_id,
                               int64_t         channel,
                               ipc_picture_t  *picture,
                               ipc_PeopleID_t *peopleID);

/**
 * @brief 云存图片(小图片)传输，适合内存很小的设备.
 *
 * @param[in] event_id      -异常事件id,@iot_event_id_t
 *
 * @param[in] channel       -通道，IPC为0
 *
 * @param[in] obj_ext       -没有该功能填NULL，坐标的JSON数组;obj_id数组个数x4, 目标坐标 4个数字,左下为(0,0)开始计数; 左下(x,y),右上(x,y); x,y 为像素编号; 一张图多个目标则为多个坐标
 *
 * @param[in] data          -需要上传的图片数据
 *
 * @param[in] data_len      -图片数据长度
 *
 * @return 0正确.
 */
int avsdk_cs_picture_pulish_v3(iot_event_id_t  event_id,
                               int64_t         channel,
                               char           *obj_ext,
                               uint8_t        *data,
                               uint32_t        data_len);


/**
 * @brief 设置云存日志等级
 *
 * @param[in]   level        -#define LOG_LEVEL_NONE  1
 *                            #define LOG_LEVEL_ERROR 2
 *                            #define LOG_LEVEL_WARN  3
 *                            #define LOG_LEVEL_INFO  4
 *                            #define LOG_LEVEL_DEBUG 5
 *                            #define LOG_LEVEL_TRACE 6
 *
 *
 * @return  设置日志等级
 */
void avsdk_cs_set_log_level(int level);

/**
 * @brief  云存开始
 *
 * @param[out]  cs_hdl      - 操作句柄
 * @param[in]   opt         - 云存配置文件
 *
 * @return  成功返回0
 */
int avsdk_cs_start(cs_hdl_ref_t       *cs_hdl,
                   cs_start_opt_t     *opt);

#if 1 //云存v2接口

/**
 * @brief 设置存入的音视频格式v2
 *
 * @param[in]   encrypt        		- 文件加密方式,none或者e_cs_encrypt_aes_256_cbc
 *
 * @param[in]   video_format        - 视频格式
 *
 * @param[in]   fps                 - 帧率
 *
 * @param[in]   audio_format        - 音频格式
 *
 * @param[in]   sound_frequency     - 声音采样频率
 *
 * @param[in]   sound_depth         - 声音采样深度
 *
 * @param[in]   sound_channel       - 声音声道
 *
 * @param[in]   picture_num         - 0无图片，有几张图片调用几次avsdk_cs_append_thumbnail_v2
 *
 * @param[in]   picture_type        - 图片类型，参考e_cs_picture_type_t
 *
 * @return  返回0成功，返回-1表示已经设置过音视频格式
 */
int avsdk_cs_set_format_v2(cs_hdl_ref_t           cs_hdl,
                           e_cs_encrypt_type_t    encrypt,
                           e_cs_stream_type_t     video_format,
                           uint8_t                fps,
                           e_cs_stream_type_t     audio_format,
                           e_cs_sound_frequency_t sound_frequency,
                           e_cs_sound_depth_t     sound_depth,
                           e_cs_sound_channel_t   sound_channel,
                           uint8_t                picture_num,
                           e_cs_picture_type_t    picture_type);

/**
 * @brief 存入缩略图v2
 *
 * @param[in]   payload        - 缩略图的内容
 *
 * @param[in]   payload_len    - 内容的长度
 *
 * @param[in]   xy             - 图片中对象的坐标，没有填NULL
 *
 * @return  返回0成功，返回 -1表示没有写入音视频格式
 */
int avsdk_cs_append_thumbnail_v2(cs_hdl_ref_t  cs_hdl,
                                 uint8_t      *payload,
                                 uint32_t      payload_len,
                                 struct coord *xy);

/**
 * @brief 存入video数据v2
 *
 * @param[in]   ch                  - AV通道，ipc只支持一个通道填0；如果配置了多路视频通道，那么从1开始编号；例如：设备配置的两个视频通道，视频通道ch只能是1或者2;
 *
 * @param[in]   is_key_frame        - 关键帧填1，其他填0
 *
 * @param[in]   frame_features      - 帧特征@e_frame_features_t
 *
 * @param[in]   timestamp_ms        - 相对于第一个视频帧时间的间隔ms
 *
 * @param[in]   xy                 	- 物体的坐标，参考struct coord
 *
 * @param[in]   payload_ptr         - 视频内容
 *
 * @param[in]   payload_len                 - 视频内容长度
 *
 * @return  返回0成功，返回-1表示没有写入音视频格式，如果返回 -4 表示文件大小超过16M.
 */
int avsdk_cs_append_video_v2(cs_hdl_ref_t          cs_hdl,
                             uint8_t               ch,
                             char                  is_key_frame,
                             e_cs_frame_features_t frame_features,
                             uint32_t              timestamp_ms,
                             struct coord         *xy,
                             uint8_t              *payload_ptr,
                             uint32_t              payload_len);

/**
 * @brief 存入audio数据v2
 *
 * @param[in]   ch                  - AV通道，ipc只支持一个通道填0；如果配置了多路视频通道，那么从1开始编号
 *
 * @param[in]   frame_features      - 帧特征@e_cs_frame_features_t
 *
 * @param[in]   timestamp_ms        - 相对于第一个视频帧时间的间隔ms
 *
 * @param[in]   payload_ptr         - 视频内容
 *
 * @param[in]   payload_len         - 视频内容长度
 *
 * @return  返回0成功，返回-1表示没有写入音视频格式，如果返回 -4 表示文件大小超过16M.
 */
int avsdk_cs_append_audio_v2(cs_hdl_ref_t          cs_hdl,
                             uint8_t               ch,
                             e_cs_frame_features_t frame_features,
                             uint32_t              timestamp_ms,
                             uint8_t              *payload_ptr,
                             uint32_t              payload_len);
#endif

/**
 * @brief  获取云文件的大小（流）
 *
 * @return  成功返回0
 */
uint32_t avsdk_cs_get_file_size(cs_hdl_ref_t cs_hdl);

/**
 * @brief 云存结束.
 *
 * @param[in] peopleID         - 人脸识别id,没有该功能填NULL.
 *
 * @return 0正确.*/
int avsdk_cs_end(cs_hdl_ref_t *cs_hdl, ipc_PeopleID_t *peopleID);



#if 1 //v1版接口后期停止新功能开发，请优先使用v2接口
/**
 * @brief 设置存入的音视频格式（v1版接口，请优先使用v2接口）
 *
 * @param[in]   encrypt        		- 文件加密方式,none或者e_cs_encrypt_aes_256_cbc
 *
 * @param[in]   video_format        - 视频格式
 *
 * @param[in]   fps                 - 帧率
 *
 * @param[in]   audio_format        - 音频格式
 *
 * @param[in]   sound_frequency     - 声音采样频率
 *
 * @param[in]   sound_depth         - 声音采样深度
 *
 * @param[in]   sound_channel       - 声音声道
 *
 * @param[in]   is_have_thumbnail   - 0无图片，1缩略图，2图片
 *
 *
 * @return  返回0成功，返回-1表示已经设置过音视频格式
 */
int avsdk_cs_set_format(cs_hdl_ref_t           cs_hdl,
                        e_cs_encrypt_type_t    encrypt,
                        e_cs_stream_type_t     video_format,
                        uint8_t                fps,
                        e_cs_stream_type_t     audio_format,
                        e_cs_sound_frequency_t sound_frequency,
                        e_cs_sound_depth_t     sound_depth,
                        e_cs_sound_channel_t   sound_channel,
                        char                   is_have_thumbnail);

/**
 * @brief 存入缩略图(v1版接口，请优先使用v2接口)
 *
 * @param[in]   payload             - 缩略图的内容
 *
 * @param[in]   payload_len         - 内容的长度
 *
 * @param[in]   xy                  - 物体的坐标，参考struct coord
 *
 * @return  返回0成功，返回 -1表示没有写入音视频格式
 */
int avsdk_cs_append_thumbnail(cs_hdl_ref_t  cs_hdl,
                              uint8_t      *payload,
                              uint32_t      payload_len,
                              struct coord *xy);

/**
 * @brief 存入video数据（v1版接口，请优先使用v2接口）
 *
 * @param[in]   is_key_frame                - 关键帧填1，其他填0
 *
 * @param[in]   frame_features              - 帧特征@e_frame_features_t
 *
 * @param[in]   timestamp_ms                - 相对于第一个视频帧时间的间隔ms
 *
 * @param[in]   xy                 			- 物体的坐标，参考struct coord
 *
 * @param[in]   payload_ptr                 - 视频内容
 *
 * @param[in]   payload_len                 - 视频内容长度
 *
 * @return  返回0成功，返回-1表示没有写入音视频格式，如果返回 -4 表示文件大小超过16M;-9本次cs已退出或者未初始化.
 */
int avsdk_cs_append_video(cs_hdl_ref_t          cs_hdl,
                          char                  is_key_frame,
                          e_cs_frame_features_t frame_features,
                          uint32_t              timestamp_ms,
                          struct coord         *xy,
                          uint8_t              *payload_ptr,
                          uint32_t              payload_len);

/**
 * @brief 存入audio数据（v1版接口，请优先使用v2接口）
 *
 * @param[in]   frame_features              - 帧特征@e_cs_frame_features_t
 *
 * @param[in]   timestamp_ms                - 相对于第一个视频帧时间的间隔ms
 *
 * @param[in]   payload_ptr                 - 视频内容
 *
 * @param[in]   payload_len                 - 视频内容长度
 *
 * @return  返回0成功，返回-1表示没有写入音视频格式，如果返回 -4 表示文件大小超过16M;-9本次cs已退出或者未初始化.
 */
int avsdk_cs_append_audio(cs_hdl_ref_t          cs_hdl,
                          e_cs_frame_features_t frame_features,
                          uint32_t              timestamp_ms,
                          uint8_t              *payload_ptr,
                          uint32_t              payload_len);
#endif
/**
 * @brief 指定云存上报的格式；格式无变化调用一次即可；
 * 一般用户不需要关注此接口.
 *
 * @param[in] format         - 0使用服务器下发的普通格式；101.需要易瞳校准SDK;5,热成像
 *
 * @return 无.*/
void avsdk_cs_extend_format(int format);

/**
 * @brief 获取wifi热点的ssid和password,需要sdk初始化后调用.
 *
 * @param[out]   ssid                 - wifi热点ssid
 *
 * @param[out]   pass                 - wifi热点password
 *
 * @return 无.
 */
void avsdk_get_wifi_conf(char *ssid, char *pass);

/**
 * @brief 获取wifi热点的ssid和password，可随时调用.
 *
 * @param[in]    did                  - 配置文件中的did
 *
 * @param[out]   ssid                 - wifi热点ssid
 *
 * @param[out]   pass                 - wifi热点password
 *
 * @return 无.
 */

void avsdk_get_wifi_info(char *did, char *ssid, char *pass);

/**
 * @brief 设备发送iot命令到平台.
 *
 * @param[in]   topic             - 发送的topic，从文档获取
 *
 * @param[in]   p                 - iot命令
 *
 * @return 无.
 */
int avsdk_send_iotcmd(char *topic, iot_PpiotCmd_t *p);

/**
 * @brief 与平台断开时，重连平台模式，一般不需要调用.
 *
 * @param[in]   mode  - 平台重连运行模式，默认快速模式
 *
 * @return 无.
 */
void avsdk_set_platform_mode(iot_reconnect_mode_t mode);

/**
 * @brief 设置kcp窗口，一般不需要调用.
 *
 * @param[in]   snd_wnd             - 发送窗口
 *
 * @param[in]   rcv_wnd             - 接收窗口
 *
 * @return 无.
 */
void avsdk_set_wnd(int32_t snd_wnd, int32_t rcv_wnd);

/**
 * @brief 设置发送队列容量，一般不需要调用.
 *
 * @param[in]   cmd_cap             - 发送窗口
 *
 * @param[in]   file_cap            - 接收窗口
 *
 * @param[in]   audio_cap           - 发送窗口
 *
 * @param[in]   video_cap           - 接收窗口
 *
 * @return 无.
 */
void avsdk_set_sendcap(uint16_t cmd_cap,
                       uint16_t file_cap,
                       uint16_t audio_cap,
                       uint16_t video_cap);
#if 1
int avsdk_export_apigw(iot_server_t *server);
char *avsdk_cfg_lslat(void);
char *avsdk_cfg_did(void);
char *avsdk_cfg_dkey(void);
void avsdk_get_signKey(char *deviceId, char *deviceKey, char *signKey);
void avsdk_get_localSignKey(char *deviceId, char *deviceKey, char *signKey);
#endif

#if 1 //for 蓝牙配网加解密
/*********************************************************************************************************************
蓝牙配网需要使用指定的128位UUID,入下：

#define DEV_BLE_SERVER_UUID     "865300b1-43e6-47b7-9cb0-5fc21d4ae340"
#define DEV_BLE_RX_UUID         "865300b3-43e6-47b7-9cb0-5fc21d4ae340" //read 从app收数据
#define DEV_BLE_TX_NTF_UUID     "865300b2-43e6-47b7-9cb0-5fc21d4ae340" //send ntf发数据给app
#define DEV_BLE_TX_IND_UUID     "865300b4-43e6-47b7-9cb0-5fc21d4ae340" //send ind发数据给app

#define BLE_SERVER_UUID       {0x40,0xE3,0x4A,0x1D,0xC2,0x5F,0xB0,0x9C,0xB7,0x47,0xE6,0x43,0xB1,0x00,0x53,0x86}
#define BLE_TX_NTF_UUID       {0x40,0xE3,0x4A,0x1D,0xC2,0x5F,0xB0,0x9C,0xB7,0x47,0xE6,0x43,0xB2,0x00,0x53,0x86}
#define BLE_RX_UUID           {0x40,0xE3,0x4A,0x1D,0xC2,0x5F,0xB0,0x9C,0xB7,0x47,0xE6,0x43,0xB3,0x00,0x53,0x86}
#define BLE_TX_IND_UUID       {0x40,0xE3,0x4A,0x1D,0xC2,0x5F,0xB0,0x9C,0xB7,0x47,0xE6,0x43,0xB4,0x00,0x53,0x86}
*********************************************************************************************************************/
int avsdk_local_decode(uint8_t *payload, size_t len, iot_PpiotCmd_t *cmd);
int avsdk_local_encode(iot_PpiotCmd_t *cmd, uint8_t *payload, size_t *len);

/*
 * @brief:  蓝牙配网初始化
 *
 * @param[out]: hdl     -   句柄
 * @param[in]:  write   -   写数据回调
 * @param[in]： cfg     -    配置WiFi回调
 *
 * @return: 0成功
 **/
int avsdk_bdn_init(iot_bdn_ref_t *hdl,
                   int (*ble_send)(uint8_t *data, uint16_t len),
                   int (*wifi_set)(char *ssid, char *pwd));

/*
 * @brief:  蓝牙配网反初始化，资源释放
 *
 * @param[in]:  hdl     -   句柄,由avsdk_bdn_init获取
 *
 * @return: 0成功
 **/
int avsdk_bdn_fini(iot_bdn_ref_t *hdl);

/*
 * @brief:  蓝牙数据接收
 *
 * @param[out]: hdl     -   句柄,由avsdk_bdn_init获取
 * @param[in]:  data    -   待处理数据指针
 * @param[in]:  len     -   待处理数据长度
 *
 * @return:
 **/
void avsdk_bdn_recv(iot_bdn_ref_t hdl, uint8_t *data, uint16_t len);
#endif

#if 1
/*
 * @brief:  启动本地udp服务器，可用于ap配网，只有资源紧张，默认没开启udp服务的设备才需要调用，常规sdk都不需要调用
 *
 *
 * @return: 0成功，-4已经有服务运行。
 **/
int avsdk_localsrv_udp_start(void);

/*
 * @brief:  关闭本地udp服务器，节约资源；一般不需要调用
 *
 *
 * @return: 无
 **/
void avsdk_localsrv_udp_stop(void);

#endif

/*
 * @brief:  发现局域网内的设备
 *
 * @param[out]:  p     -   局域网类设备的信息
 *
 * @return: 0成功
 **/
int avsdk_broadcast_discovery(discovery_t *p);

#if 1 //设备拉流
/*
 * @brief: 获取属于该用户的设备
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回设备列表信息
 *
 * @return: 0成功
 **/
int avsdk_user_dev_list(gate_MDid_Req  *req,
                        gate_MDid_Resp *rsp);
/*
 * @brief: 连接指定设备
 *
 * @param[in]:   did     - avsdk_user_dev_list返回在线的设备中的一个
 *
 * @return: 小于0失败，大于0表示conn_id,用于其他命令发送
 **/
int avsdk_user_conn_dev(char *did);

/*
 * @brief: 播放视频
 *
 * @param[in]:   conn_id - 由avsdk_user_conn_dev获得
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回视频的参数
 *
 * @return: 0成功
 **/
int avsdk_user_VideoPlay(int conn_id, ipc_VideoPlay_Req *req, ipc_VideoPlay_Resp *rsp);

/*
 * @brief: 停止播放视频
 *
 * @param[in]:   conn_id - 由avsdk_user_conn_dev获得
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回音频的参数
 *
 * @return: 0成功
 **/
int avsdk_user_VideoPause(int conn_id, ipc_VideoPause_Req *req, ipc_VideoPause_Resp *rsp);

/*
 * @brief: 播放音频
 *
 * @param[in]:   conn_id - 由avsdk_user_conn_dev获得
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回音频的参数
 *
 * @return: 0成功
 **/
int avsdk_user_AudioPlay(int conn_id, ipc_AudioPlay_Req *req, ipc_AudioPlay_Resp *rsp);

/*
 * @brief: 停止播放音频
 *
 * @param[in]:   conn_id - 由avsdk_user_conn_dev获得
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回音频的参数
 *
 * @return: 0成功
 **/

int avsdk_user_AudioPause(int conn_id, ipc_AudioPause_Req *req, ipc_AudioPause_Resp *rsp);

/*
 * @brief: 发起对讲
 *
 * @param[in]:   conn_id - 由avsdk_user_conn_dev获得
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回设备支持播放的音频参数
 *
 * @return: 0成功
 **/

int avsdk_user_TalkbackPlay(int conn_id, ipc_TalkbackPlay_Req *req, ipc_TalkbackPlay_Resp *rsp);

/*
 * @brief: 停止对讲
 *
 * @param[in]:   conn_id - 由avsdk_user_conn_dev获得
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回音频的参数
 *
 * @return: 0成功
 **/

int avsdk_user_TalkbackPause(int conn_id, ipc_TalkbackPause_Req *req, ipc_TalkbackPause_Resp *rsp);

/*
 * @brief: 获取一张缩略图
 *
 * @param[in]:   conn_id - 由avsdk_user_conn_dev获得
 *
 * @param[in]:   req     - 请求参数
 *
 * @param[out]:  rsp     - 返回缩略图
 *
 * @return: 0成功
 **/
int avsdk_user_Screenshot(int conn_id, ipc_Screenshot_Req *req, ipc_Screenshot_Resp *rsp);


#endif

#if 1 //不更新的接口,不推荐使用

/**
 * @brief 异常事件上报,建议使用avsdk_push_event
 *
 * @param[in] event_id  - 异常事件id.
 *
 * @param[in] avchannel - IPC都为0；如果是NVR，表示第几路摄像头
 *
 * @param[in] hw_errno  - 硬件错误码，event_id为E_IOT_EVENT_HARDWARE_FAULT和E_IOT_EVENT_FIRMWARE_UPDATE有效.
 *
 * @param[in] peopleID  - 人脸识别id,没有该功能填NULL.
 *
 * @param[in] isCS      - 该事件是否带云存,true表示带有云存类型的事件(与开通云存状态没关系);false表示只产生一条消息.
 *
 * @return 返回值：为负数表示错误; 等于0事件上报成功，但是设备没开通云存; 大于0表示上报成功且设备开通了云存.
 */
int64_t avsdk_event_pulish(iot_event_id_t  event_id,
                           int64_t         avchannel,
                           int64_t         hw_errno,
                           ipc_PeopleID_t *peopleID,
                           bool            isCS);

#endif //不更新的接口

#if 1
/* 注册回调函数 */
void avsdk_set_on_ipc_PirSet(on_ipc_PirSet_t cb);
void avsdk_set_on_ipc_PirGet(on_ipc_PirGet_t cb);
void avsdk_set_on_ipc_ChanState(on_ipc_ChanState_t cb);
void avsdk_set_on_ipc_VideoChanChange(on_ipc_VideoChanChange_t cb);
void avsdk_set_on_ipc_Discovery(on_ipc_Discovery_t cb);
void avsdk_set_on_ipc_WifiAPGet(on_ipc_WifiAPGet_t cb);
void avsdk_set_on_ipc_WifiSet(on_ipc_WifiSet_t cb);
void avsdk_set_on_ipc_WifiGet(on_ipc_WifiGet_t cb);
void avsdk_set_on_ipc_OsdSet(on_ipc_OsdSet_t cb);
void avsdk_set_on_ipc_OsdGet(on_ipc_OsdGet_t cb);
void avsdk_set_on_ipc_VideoPlay(on_ipc_VideoPlay_t cb);
void avsdk_set_on_ipc_VideoPause(on_ipc_VideoPause_t cb);
void avsdk_set_on_ipc_VideoQosSet(on_ipc_VideoQosSet_t cb);
void avsdk_set_on_ipc_FlipSet(on_ipc_FlipSet_t cb);
void avsdk_set_on_ipc_AudioPlay(on_ipc_AudioPlay_t cb);
void avsdk_set_on_ipc_AudioPause(on_ipc_AudioPause_t cb);
void avsdk_set_on_ipc_TalkbackPlay(on_ipc_TalkbackPlay_t cb);
void avsdk_set_on_ipc_TalkbackPause(on_ipc_TalkbackPause_t cb);
void avsdk_set_on_ipc_HistoryPlanSet(on_ipc_HistoryPlanSet_t cb);
void avsdk_set_on_ipc_HistoryPlanGet(on_ipc_HistoryPlanGet_t cb);
void avsdk_set_on_ipc_HistoryDays(on_ipc_HistoryDays_t cb);
void avsdk_set_on_ipc_HistoryDayList(on_ipc_HistoryDayList_t cb);
void avsdk_set_on_ipc_HistoryPlay(on_ipc_HistoryPlay_t cb);
void avsdk_set_on_ipc_HistoryPause(on_ipc_HistoryPause_t cb);
void avsdk_set_on_ipc_HistoryThumGet(on_ipc_HistoryThumGet_t cb);
void avsdk_set_on_ipc_HistoryDel(on_ipc_HistoryDel_t cb);
void avsdk_set_on_ipc_ConfigGet(on_ipc_ConfigGet_t cb);
void avsdk_set_on_ipc_TimeSet(on_ipc_TimeSet_t cb);
void avsdk_set_on_ipc_TimeGet(on_ipc_TimeGet_t cb);
void avsdk_set_on_ipc_LedSet(on_ipc_LedSet_t cb);
void avsdk_set_on_ipc_LedGet(on_ipc_LedGet_t cb);
void avsdk_set_on_ipc_LedModeSet(on_ipc_LedModeSet_t cb);
void avsdk_set_on_ipc_LedModeGet(on_ipc_LedModeGet_t cb);
void avsdk_set_on_ipc_IRCutSet(on_ipc_IRCutSet_t cb);
void avsdk_set_on_ipc_IRCutGet(on_ipc_IRCutGet_t cb);
void avsdk_set_on_ipc_SecretSet(on_ipc_SecretSet_t cb);
void avsdk_set_on_ipc_SecretGet(on_ipc_SecretGet_t cb);
void avsdk_set_on_ipc_NotifySet(on_ipc_NotifySet_t cb);
void avsdk_set_on_ipc_NotifyGet(on_ipc_NotifyGet_t cb);
void avsdk_set_on_ipc_MotionzoneSet(on_ipc_MotionzoneSet_t cb);
void avsdk_set_on_ipc_MotionzoneGet(on_ipc_MotionzoneGet_t cb);
void avsdk_set_on_ipc_PspAdd(on_ipc_PspAdd_t cb);
void avsdk_set_on_ipc_PspDel(on_ipc_PspDel_t cb);
void avsdk_set_on_ipc_PspList(on_ipc_PspList_t cb);
void avsdk_set_on_ipc_PspCall(on_ipc_PspCall_t cb);
void avsdk_set_on_ipc_Reboot(on_ipc_Reboot_t cb);
void avsdk_set_on_ipc_Reset(on_ipc_Reset_t cb);
void avsdk_set_on_ipc_FlipGet(on_ipc_FlipGet_t cb);
void avsdk_set_on_ipc_LanAuth(on_ipc_LanAuth_t cb);
void avsdk_set_on_ipc_TimedcruiseSet(on_ipc_TimedcruiseSet_t cb);
void avsdk_set_on_ipc_TimedcruiseGet(on_ipc_TimedcruiseGet_t cb);
void avsdk_set_on_ipc_StorageInfo(on_ipc_StorageInfo_t cb);
void avsdk_set_on_ipc_StorageFormat(on_ipc_StorageFormat_t cb);
void avsdk_set_on_ipc_StorageFormatRate(on_ipc_StorageFormatRate_t cb);
void avsdk_set_on_ipc_LogSet(on_ipc_LogSet_t cb);
void avsdk_set_on_ipc_VolumeSet(on_ipc_VolumeSet_t cb);
void avsdk_set_on_ipc_PowerFreqSet(on_ipc_PowerFreqSet_t cb);
void avsdk_set_on_ipc_PowerFreqGet(on_ipc_PowerFreqGet_t cb);
void avsdk_set_on_ipc_VolumeGet(on_ipc_VolumeGet_t cb);
void avsdk_set_on_ipc_AlarmSet(on_ipc_AlarmSet_t cb);
void avsdk_set_on_ipc_AlarmGet(on_ipc_AlarmGet_t cb);
void avsdk_set_on_ipc_Screenshot(on_ipc_Screenshot_t cb);
void avsdk_set_on_ipc_PtzCtrl(on_ipc_PtzCtrl_t cb);
void avsdk_set_on_ipc_FirmwareNotify(on_ipc_FirmwareNotify_t cb);
void avsdk_set_on_ipc_FirmwareRate(on_ipc_FirmwareRate_t cb);
void avsdk_set_on_ipc_EventRecordGet(on_ipc_EventRecordGet_t cb);
void avsdk_set_on_ipc_EventRecordSet(on_ipc_EventRecordSet_t cb);
void avsdk_set_on_ipc_GetNetworkInfo(on_ipc_GetNetworkInfo_t cb);
void avsdk_set_on_ipc_FileStart(on_ipc_FileStart_t cb);
void avsdk_set_on_ipc_FileStop(on_ipc_FileStop_t cb);
void avsdk_set_on_ipc_CustomCmd(on_ipc_CustomCmd_t cb);
void avsdk_set_on_ipc_DirCreate(on_ipc_DirCreate_t cb);
void avsdk_set_on_ipc_DirDel(on_ipc_DirDel_t cb);
void avsdk_set_on_ipc_DirEdit(on_ipc_DirEdit_t cb);
void avsdk_set_on_ipc_DirList(on_ipc_DirList_t cb);
void avsdk_set_on_ipc_FileAdd(on_ipc_FileAdd_t cb);
void avsdk_set_on_ipc_FileDel(on_ipc_FileDel_t cb);
void avsdk_set_on_ipc_FileEdit(on_ipc_FileEdit_t cb);
void avsdk_set_on_ipc_FileRecvRate(on_ipc_FileRecvRate_t cb);
void avsdk_set_on_ipc_FileThumList(on_ipc_FileThumList_t cb);
void avsdk_set_on_ipc_FileAttrSet(on_ipc_FileAttrSet_t cb);
void avsdk_set_on_ipc_ExecIOTCMD(on_ipc_ExecIOTCMD_t cb);
void avsdk_set_on_ipc_ExecTimetask(on_ipc_ExecTimetask_t cb);
void avsdk_set_on_ipc_TamperSet(on_ipc_TamperSet_t cb);
void avsdk_set_on_ipc_SetAutoTrack(on_ipc_SetAutoTrack_t cb);
void avsdk_set_on_ipc_GetAutoTrack(on_ipc_GetAutoTrack_t cb);
void avsdk_set_on_ipc_ForceIFrame(on_ipc_ForceIFrame_t cb);
void avsdk_set_on_ipc_SetLowPower(on_ipc_SetLowPower_t cb);
void avsdk_set_on_ipc_GetLowPower(on_ipc_GetLowPower_t cb);
#endif

/****************************************************************************/
#ifdef __cplusplus
}
#endif
/*****************************************************************************
 * @} (end addtogroup xciot_api)
 */
/****************************************************************************/
#endif /* XCIOT_API_H_INCLUDE */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
