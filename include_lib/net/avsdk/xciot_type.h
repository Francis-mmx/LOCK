/*****************************************************************************
 * xciot_type.h
 *
 *
 *
 ****************************************************************************/
/*
 * wiki
 * ====
 */
#ifndef XCIOT_TYPE_H_INCLUDE
#define XCIOT_TYPE_H_INCLUDE

#include <stddef.h>

typedef unsigned char   		u8, bool, BOOL, uint8_t;
typedef char            		s8;
typedef unsigned short  		u16, uint16_t;
typedef signed short    		s16;
typedef unsigned int    		u32, uint32_t;
typedef signed int      		s32, int32_t;
typedef unsigned long long 		u64, uint64_t;
typedef u32						FOURCC;
typedef long long               s64, int64_t;
typedef unsigned long long      u64;

/*****************************************************************************
 * @addtogroup xciot_type
 * @{
 */
/*****************************************************************************
 * Included Files
 ****************************************************************************/

#if !defined(XCIOT_API_H_INCLUDE)
#error "Only 'xciot_api.h' can be included directly."
#endif

/*****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define XCIOT_BYTES_ARRAY_T(n)                                               \
    struct {                                                                 \
        uint32_t size;                                                       \
        uint8_t  bytes[n];                                                   \
    }

/*****************************************************************************
 * Public Types
 ****************************************************************************/

/**
 * @brief iot service type.
 */
typedef enum {
    E_CS_SERVICE_NONE    = 0,//未开通云存
    E_CS_SERVICE_VIDEO   = 1,//事件云存储，有事件发一段视频；或者抓拍一张图片
    E_CS_SERVICE_PICTURE = 4,//只开通图片服务，即抓拍一张图片
    E_CS_SERVICE_24VIDEO = 200,//24小时全天录像
} cs_service_type_t;

/**
 * @brief iot tamper status.
 */
typedef enum {
    E_IOT_TAMPER_OFF = 0,
    E_IOT_TAMPER_ON  = 1,
} iot_tamper_status_t;

/**
 * @brief iot log level.
 */
typedef enum {
    E_IOT_LOG_LEVEL_NONE  = 1,
    E_IOT_LOG_LEVEL_ERROR = 2,
    E_IOT_LOG_LEVEL_WARN  = 3,
    E_IOT_LOG_LEVEL_INFO  = 4,
    E_IOT_LOG_LEVEL_DEBUG = 5,
    E_IOT_LOG_LEVEL_TRACE = 6
} iot_log_level_t;

/**
 * @brief iot role type.
 */
typedef enum {
    /** role type device */
    E_IOT_ROLE_TYPE_DEV = 1,

    /** role type user */
    E_IOT_ROLE_TYPE_USER = 2,

    /* add above here */
    E_IOT_ROLE_TYPE_UNK = 0
} iot_role_type_t;

/**
 * @brief iot init params.
 */
typedef struct {
    /* dev or user */
    iot_role_type_t roletype;

    /* connect max size */
    uint32_t conn_maxsz;

    /* config file buffer */
    const char *cfg_buf;
    size_t      cfg_buf_len;

    /*hardware version*/
    char hardware_version[33];

    /*firmware version*/
    char firmware_version[33];

    /* 每个设备提供的唯一代码: MAC，芯片序列号，IMEI都OK;
       下次通讯必须一致，如果不一致平台不允许登录 */
    char ucode[65];

} avsdk_init_params_t;

/**
 * @brief 平台连接状态
 */
typedef enum {
    /** 连接平台失败 */
    E_IOT_CONN_PLATFORM_STATE_DISCONN = 0,

    /** 连接平台成功 */
    E_IOT_CONN_PLATFORM_STATE_CONNECTED = 1,

    /** 连接失败，原因did和ucode重复，请更换did，或者使用上一次的ucode */
    E_IOT_CONN_PLATFORM_STATE_ERROR_UCODE = 2,

} iot_conn_platform_state_t;

/**
 * @brief iot conn 连接状态
 */
typedef enum {
    /** 连接空闲 */
    E_IOT_CONN_STATE_IDEL = 0,

    /** 连接初始化中 */
    E_IOT_CONN_STATE_INIT = 1,

    /** 连接已经建立 */
    E_IOT_CONN_STATE_OPENED = 3,

    /** 连接已经关闭 */
    E_IOT_CONN_STATE_CLOSED = 4,
} iot_conn_state_t;

/**
 * @brief 开锁方式
 */
typedef enum {
    /** 钥匙 */
    E_IOT_OPEN_TYPE_KEY = 1,
    /** 密码 */
    E_IOT_OPEN_TYPE_PASSWORD = 2,
    /** 指纹 */
    E_IOT_OPEN_TYPE_FINGER = 3,
    /** IC卡/身份证 */
    E_IOT_OPEN_TYPE_ICCARD = 4,
    /** APP */
    E_IOT_OPEN_TYPE_APP = 5,
    /** 人脸 */
    E_IOT_OPEN_TYPE_FACE = 6,
    /** 掌纹 */
    E_IOT_OPEN_TYPE_PALM = 7,
    /** 虹膜 */
    E_IOT_OPEN_TYPE_IRIS = 8,
    /** 指静脉 */
    E_IOT_OPEN_TYPE_FINGER_VEIN = 9,
} open_type_t;

/**
 * @brief iot conn 连接方式
 */
typedef enum {
    /** 连接方式-本地连接 */
    E_IOT_CONN_TYPE_LOCAL = 0,

    /** 连接方式-P2P连接 */
    E_IOT_CONN_TYPE_P2P = 1,

    /** 连接方式-RELAY连接 */
    E_IOT_CONN_TYPE_RELAY = 2,

    /* add above here */
    E_IOT_CONN_TYPE_LAST
} iot_conn_type_t;

/** 音视频类型 */
typedef enum {
    /** 视频 H264     */
    E_IOT_MEDIASTREAM_TYPE_H264 = 1,

    /** 视频 H265     */
    E_IOT_MEDIASTREAM_TYPE_H265 = 2,

    /** 视频 MPEG     */
    E_IOT_MEDIASTREAM_TYPE_MPEG = 3,

    /** 视频 MJPEG    */
    E_IOT_MEDIASTREAM_TYPE_MJPEG = 4,

    /** 视频 热成像    */
    E_IOT_MEDIASTREAM_TYPE_TID = 10,

    /** 音频 G711A    */
    E_IOT_MEDIASTREAM_TYPE_G711A = 21,

    /** 音频 ULAW     */
    E_IOT_MEDIASTREAM_TYPE_ULAW = 31,

    /** 音频 G711U    */
    E_IOT_MEDIASTREAM_TYPE_G711U = 41,

    /** 音频 OPUS     */
    E_IOT_MEDIASTREAM_TYPE_OPUS = 51,

    /** 音频 ADPCM    */
    E_IOT_MEDIASTREAM_TYPE_ADPCM = 61,

    /** 音频 G721     */
    E_IOT_MEDIASTREAM_TYPE_G721 = 71,

    /** 音频 G723     */
    E_IOT_MEDIASTREAM_TYPE_G723 = 81,

    /** 音频 G726_16  */
    E_IOT_MEDIASTREAM_TYPE_G726 = 91,

    /** 音频 AAC      */
    E_IOT_MEDIASTREAM_TYPE_AAC = 101,

    /** 音频 Speex    */
    E_IOT_MEDIASTREAM_TYPE_SPEEX = 111,

    /** 音频 PCM      */
    E_IOT_MEDIASTREAM_TYPE_PCM = 121
} iot_mediastream_type_t;

typedef enum {
    E_IOT_ENCRYPT_TYPE_NONE = 0,

    E_IOT_ENCRYPT_TYPE_AES128CBC = 1,
    E_IOT_ENCRYPT_TYPE_AES192CBC = 2,
    E_IOT_ENCRYPT_TYPE_AES256CBC = 3,

    E_IOT_ENCRYPT_TYPE_AES128CFB = 4,
    E_IOT_ENCRYPT_TYPE_AES192CFB = 5,
    E_IOT_ENCRYPT_TYPE_AES256CFB = 6,

    E_IOT_ENCRYPT_TYPE_AES128ECB = 7,
    E_IOT_ENCRYPT_TYPE_AES192ECB = 8,
    E_IOT_ENCRYPT_TYPE_AES256ECB = 9,

    E_IOT_ENCRYPT_TYPE_AES128OFB = 10,
    E_IOT_ENCRYPT_TYPE_AES192OFB = 11,
    E_IOT_ENCRYPT_TYPE_AES256OFB = 12,

    E_IOT_ENCRYPT_TYPE_AES128CTR = 13,
    E_IOT_ENCRYPT_TYPE_AES192CTR = 14,
    E_IOT_ENCRYPT_TYPE_AES256CTR = 15
} iot_encrypt_type_t;

typedef enum {
    // 方向控制
    E_IOT_FUNC_PTZ_CTRL_STOP = 10,
    E_IOT_FUNC_PTZ_CTRL_MOVE_UP,
    E_IOT_FUNC_PTZ_CTRL_MOVE_DOWN,
    E_IOT_FUNC_PTZ_CTRL_MOVE_LEFT,
    E_IOT_FUNC_PTZ_CTRL_MOVE_RIGHT,
    E_IOT_FUNC_PTZ_CTRL_MOVE_UPLEFT,
    E_IOT_FUNC_PTZ_CTRL_MOVE_DOWNLEFT,
    E_IOT_FUNC_PTZ_CTRL_MOVE_UPRIGHT,
    E_IOT_FUNC_PTZ_CTRL_MOVE_DOWNRIGHT,
    // 光圈
    E_IOT_FUNC_PTZ_CTRL_IRIS_IN,
    E_IOT_FUNC_PTZ_CTRL_IRIS_OUT,
    // 聚焦
    E_IOT_FUNC_PTZ_CTRL_FOCUS_ON,
    E_IOT_FUNC_PTZ_CTRL_FOCUS_OUT,
    // 放大
    E_IOT_FUNC_PTZ_CTRL_ZOOM_IN,
    // 缩小
    E_IOT_FUNC_PTZ_CTRL_ZOOM_OUT,

    // 巡航
    E_IOT_FUNC_PTZ_CTRL_BEGIN_CRUISE_SET,
    E_IOT_FUNC_PTZ_CTRL_SET_CRUISE,
    E_IOT_FUNC_PTZ_CTRL_END_CRUISE_SET,
    E_IOT_FUNC_PTZ_CTRL_CALL_CRUISE,
    E_IOT_FUNC_PTZ_CTRL_DELETE_CRUISE,
    E_IOT_FUNC_PTZ_CTRL_STOP_CRUISE,
    //
    E_IOT_FUNC_PTZ_CTRL_AUTO_SCAN,
    //
    E_IOT_FUNC_PTZ_CTRL_RAINBRUSH_START,
    E_IOT_FUNC_PTZ_CTRL_RAINBRUSH_STOP,

    E_IOT_FUNC_PTZ_CTRL_LIGHT_ON,
    E_IOT_FUNC_PTZ_CTRL_LIGHT_OFF,
    //控制设备转到指定的经纬度; 需要配合para1,para2两个参数赋值，分别代表经纬度
    E_IOT_FUNC_PTZ_LNG_LAT = 100,
    //设备向指定的角度前进(移动); 需要配合参数para1: 0-360;代表指定的角度，以手持设备的正前方为0角度，顺时针方西旋转.
    E_IOT_FUNC_PTZ_YAW_ANGLE = 101,
    //设备进行转圈运动; 参数para1: 0, 逆时针旋转; 1, 顺时针旋转。 参数para2(运动速度): 1-4档位；数字越大转速越快;0 不支持转速.
    E_IOT_FUNC_PTZ_TURN = 102,

    E_IOT_FUNC_PTZ_CTRL_MAX

} iot_func_code_type_t;

/** 异常事件ID定义 */
typedef enum {
    /* 硬件故障 */
    E_IOT_EVENT_HARDWARE_FAULT = 1,
    /* 劫持报警（锁）*/
    E_IOT_EVENT_HIJACK_ALARM = 2,
    /* 防撬报警 */
    E_IOT_EVENT_PICKPROOF_ALARM = 3,
    /* 多次密码错误 */
    E_IOT_EVENT_PASSWORD_ERROR = 7,
    /* 低电量报警 */
    E_IOT_EVENT_LOW_BATTERY = 9,
    /* 恢复出厂设置 */
    E_IOT_EVENT_RESTORES_FACTORY = 10,
    /* 画面移动侦测 */
    E_IOT_EVENT_MOTION_DETECTION = 12,
    /* 声音侦测 */
    E_IOT_EVENT_SOUND_DETECTION = 13,
    /* 画面遮挡 */
    E_IOT_EVENT_SCREEN_KEEP_OUT = 14,
    /* 设备锁定 */
    E_IOT_EVENT_DEVICE_LOCK = 15,
    /* 外部人员入侵（人脸对比）*/
    E_IOT_EVENT_EXTERNAL_INTRUSION = 17,
    /* 周界入侵 */
    E_IOT_EVENT_BORDER_INTRUSION = 18,
    /* 拌线入侵 */
    E_IOT_EVENT_CROSS_LINE_INTRUSION = 19,
    /* 余粮不足(宠物喂食器) */
    E_IOT_EVENT_FOOD_FEW = 20,
    /* 未检测到宠物进食 */
    E_IOT_EVENT_NO_EAT = 21,
    /* 感应到人体PIR */
    E_IOT_EVENT_SENSE_HUMAN = 40,
    /* SOS报警 */
    E_IOT_EVENT_SOS = 47,
    /* 可视门铃触发 */
    E_IOT_EVENT_VISUAL_DOORBELL = 83,
    /* 检测到人体(AI) */
    E_IOT_EVENT_BODY_INTRUSION = 84,
    /* 检测到人脸(AI) */
    E_IOT_EVENT_FACE_INTRUSION = 85,
    /* 比对人脸，允许通行 */
    E_IOT_EVENT_COMPARE_FACE_PASS = 86,
    /* 比对人脸，拒绝通行 */
    E_IOT_EVENT_COMPARE_FACE_NOPASS = 87,
    /* 比对人脸，陌生人 */
    E_IOT_EVENT_COMPARE_FACE_STRANGER = 88,
    /* 固件升级 */
    E_IOT_EVENT_FIRMWARE_UPDATE = 119,
    /* 缩时录影（行车记录仪）*/
    E_IOT_EVENT_SHRINK_VIDEO = 121,
    /* 车辆碰撞（行车记录仪 */
    E_IOT_EVENT_CAR_CRASH = 122,
    /* 手动锁定视频（行车记录仪) */
    E_IOT_EVENT_MANUAL_TRIGGER = 123,
    /* 停车报警 */
    E_IOT_EVENT_PARK_ALARM = 124,
    /* TF卡被拔出 */
    E_IOT_EVENT_TF_PULLOUT = 125,
    /* 开门 */
    E_IOT_EVENT_OPEN_DOOR = 128,
    /* 关门 */
    E_IOT_EVENT_CLOSE_DOOR = 129,
    /* SIM卡流量不足 */
    E_IOT_EVENT_SIM_LACK = 99,
    /* 语音抓拍 */
    E_IOT_EVENT_VOICE_SNAPPED = 30,
    /* 汽车停车(汽车熄火ACC(OFF)) */
    E_IOT_EVENT_CAR_STOP = 31,
    /* 充电异常 */
    E_IOT_EVENT_CHARGE = 32,
    /* 机器被困住（比如：机器卡在坎上不能脱困） */
    E_IOT_EVENT_TRAP = 33,
    /* 高温预警 */
    E_IOT_EVENT_TEMP_HIGH = 34,
    /* 低温预警 */
    E_IOT_EVENT_TEMP_LOW = 35,

} iot_event_id_t;

/**
 * @brief 设备供电状态.
 */
typedef enum {
    E_IOT_STATUS_NOCHARGE = 0,
    E_IOT_STATUS_CHARGE   = 1
} iot_status_power_t;

/** 设备状态ID定义 */
typedef enum {
    /* 设备上线 */
    E_IOT_STATUS_ONLINE = 126,
    /* 设备离线 */
    E_IOT_STATUS_OFFILNE = 127,
    /* 锁常开打开 */
    E_IOT_LOCK_NORMAL_OPEN = 131,
    /* 锁常开关闭 */
    E_IOT_LOCK_NORMAL_OPEN_CLOSE = 132,
    /* 普通门铃 */
    E_IOT_STATUS_DOORBELL = 133,
    /* IPC云存文件上传完成 */
    E_IOT_STATUS_CSPUTOK = 140,
    /* 汽车行驶中(汽车点火状态ACC(ON)) */
    E_IOT_CAR_RUN = 145,
    /* 汽车点火状态ACC(OFF),防盗模式 */
    E_IOT_STATUS_ACC_OFF1 = 156,
    /* 汽车点火状态ACC(OFF),缩时录像模式 */
    E_IOT_STATUS_ACC_OFF2 = 157,
    /* 汽车点火状态ACC(OFF) */
    E_IOT_STATUS_ACC_OFF3 = 158,
    /* 充电完成 */
    E_IOT_STATUS_CHARGE_OK = 169,
} iot_status_id_t;

/**
 * @brief Reconnect the platform mode.
 */
typedef enum {
    /* 与平台断开时，快速重连平台，默认 */
    E_IOT_RECONNECT_LEVEL_FAST = 0,
    /* 与平台断开时，重连平台间隔较大，大约30S，适合低功耗或者网络较差的情况 */
    E_IOT_RECONNECT_LEVEL_SLOW  = 1

} iot_reconnect_mode_t;

/** 音视频来源 */
typedef enum {
    E_IOT_MEDIASOURCE_TYPE_LIVE   = 1,
    E_IOT_MEDIASOURCE_TYPE_SDCARD = 2,
    E_IOT_MEDIASOURCE_TYPE_CLOUD  = 3
} iot_mediasource_type_t;

/** 音视频流头 */
typedef struct {
    /** AVFormat(uint8_t)
     *  - bit7  : iskeyframe 1关键帧 0非关键帧
     *  - bit6-0: streamtype see @ref iot_mediastream_type_t
     */
    struct {
        bool                   iskeyframe;
        iot_mediastream_type_t streamtype;
    } avformat;

    /** Encrypt Type */
    iot_encrypt_type_t enctype;

    /** AV Channel */
    uint64_t avchannel;

    /** AV Seq */
    uint64_t avseq;

    /** Timestamp(ms) */
    uint64_t timestamp;

    /** EncryptLength, 0表示全部加密 */
    uint64_t enclength;
} iot_exhdr_media_t;

/**
 * @brief 休眠函数返回参数.
 */
typedef struct {
    /** 用户发送心跳和接受唤醒报文的fd */
    int fd;

    /** 心跳维持的方式:0,sdk内部建立tcp连接；1,外部建立tcp连接 */
    uint8_t way;

    /** 外部连接tcp的服务器 */
    uint8_t *server;

    /** 外部连接tcp的服务器端口的个数 */
    uint8_t port_count;

    /** 外部连接tcp的服务器端口 */
    uint16_t port[4];

    /** 连接请求报文内容 */
    uint8_t *conn_req;

    /** 连接请求报文长度 */
    size_t conn_req_len;

    /** 连接应答报文内容 */
    uint8_t *conn_rsp;

    /** 连接应答报文长度 */
    size_t conn_rsp_len;

    /** 心跳报文内容 */
    uint8_t *hb_data;

    /** 心跳报文长度 */
    size_t hb_datalen;

    /** 发送心跳报文时间间隔(单位秒) */
    uint32_t hb_interval;

    /** 唤醒报文内容 */
    uint8_t *wakeup_data;

    /** 唤醒报文长度 */
    size_t wakeup_datalen;

    /** 快速启动参数内容 */
    uint8_t *faststart_data;

    /** 快速启动参数内容长度 */
    size_t faststart_datalen;
} iot_sleep_t;

/**
 * @brief SDK 错误码列表.
 */
enum iot_errno_enum {
    E_IOT_SUCCESS      = 0,
    E_IOT_EINTR        = 1,
    E_IOT_ENOMEM       = 2,
    E_IOT_EINVAL       = 3,
    E_IOT_EBUSY        = 4,
    E_IOT_ETIMEDOUT    = 5,
    E_IOT_ECONNREFUSED = 6,
    E_IOT_ECLOSED      = 7,
    E_IOT_EAGAIN       = 8,
    E_IOT_ENOTSUP      = 9,
    E_IOT_EADDRINUSE   = 10,
    E_IOT_ESTATE       = 11,
    E_IOT_ENOENT       = 12,
    E_IOT_EPROTO       = 13,
    E_IOT_EUNREACHABLE = 14,
    E_IOT_EADDRINVAL   = 15,
    E_IOT_EPERM        = 16,
    E_IOT_EMSGSIZE     = 17,
    E_IOT_ECONNABORTED = 18,
    E_IOT_ECONNRESET   = 19,
    E_IOT_ECANCELED    = 20,
    E_IOT_ENOFILES     = 21,
    E_IOT_ENOSPC       = 22,
    E_IOT_EEXIST       = 23,
    E_IOT_EREADONLY    = 24,
    E_IOT_EWRITEONLY   = 25,
    E_IOT_ECRYPTO      = 26,
    E_IOT_EPEERAUTH    = 27,
    E_IOT_ENOARG       = 28,
    E_IOT_EAMBIGUOUS   = 29,
    E_IOT_EBADTYPE     = 30,
    E_IOT_EINTERNAL    = 1000,
    E_IOT_ESYSERR      = 0x10000000,
    E_IOT_ETRANERR     = 0x20000000,

    /* yun errno */
    E_IOT_EYUN_BASE = 10000,

    /** 命令ID没有注册，不支持 */
    E_IOT_EYUN_CmdIDNotReg = (E_IOT_EYUN_BASE + 1),

    /** 调用错误 */
    E_IOT_EYUN_INVOKEError = (E_IOT_EYUN_BASE + 2),

    /** 超过限制 */
    E_IOT_EYUN_RecordOverLimit = (E_IOT_EYUN_BASE + 10),

    /** RPC调用错误 */
    E_IOT_EYUN_RPCCALLERROR = (E_IOT_EYUN_BASE + 11),

    /** 设备ID不合法 */
    E_IOT_EYUN_DeviceIDIllegal = (E_IOT_EYUN_BASE + 97),

    /** 不支持的命令 */
    E_IOT_EYUN_NOTSUPPORTCMD = (E_IOT_EYUN_BASE + 98),

    /** 未知错误 */
    E_IOT_EYUN_UNKNOWNERROR = (E_IOT_EYUN_BASE + 99),

    /** 没有初始化数据对象 */
    E_IOT_EYUN_NOTINIT = (E_IOT_EYUN_BASE + 100),

    /** 数据库操作返回错误 */
    E_IOT_EYUN_DBERROR = (E_IOT_EYUN_BASE + 101),

    /** 参数不完整 */
    E_IOT_EYUN_ParameterError = (E_IOT_EYUN_BASE + 102),

    /** 数据记录不存在 */
    E_IOT_EYUN_NOTEXISTRECORD = (E_IOT_EYUN_BASE + 103),

    /** 参数不合法 */
    E_IOT_EYUN_ParameterIllegal = (E_IOT_EYUN_BASE + 104),

    /** 编码数据错误 */
    E_IOT_EYUN_MarshalError = (E_IOT_EYUN_BASE + 105),

    /** 没有权限访问 */
    E_IOT_EYUN_ACLDENY = (E_IOT_EYUN_BASE + 106),

    /** access_token过期 */
    E_IOT_EYUN_ACCESSTOKENExpired = (E_IOT_EYUN_BASE + 107),

    /** refresh_token过期 */
    E_IOT_EYUN_REFRESHTOKENExpired = (E_IOT_EYUN_BASE + 108),

    /** AccessKey不存在 */
    E_IOT_EYUN_ACCESSKEYNOTEXIST = (E_IOT_EYUN_BASE + 120),

    /** 用户ID不存在 */
    E_IOT_EYUN_UIDNOTEXIST = (E_IOT_EYUN_BASE + 121),

    /** 签名不正确 */
    E_IOT_EYUN_ClientSignkeyNotMatch = (E_IOT_EYUN_BASE + 122),

    /** 签名过期 */
    E_IOT_EYUN_ClientSignkeyExpired = (E_IOT_EYUN_BASE + 123),

    /** 认证不匹配 */
    E_IOT_EYUN_PUBLICAUTHDENY = (E_IOT_EYUN_BASE + 150),

    /** 账号密码不匹配 */
    E_IOT_EYUN_ACCOUNTNOTMATCH = (E_IOT_EYUN_BASE + 201),

    /** 没有执行NOTRUNPPMQCONNECT */
    E_IOT_EYUN_NOTRUNPPMQCONNECT = (E_IOT_EYUN_BASE + 202),

    /** 访问设备拒绝 */
    E_IOT_EYUN_ACCESSDEVICEDENY = (E_IOT_EYUN_BASE + 250),

    /** 数据记录已经存在 */
    E_IOT_EYUN_ExistRecord = (E_IOT_EYUN_BASE + 251),

    /** 数据记录存在关联记录 */
    E_IOT_EYUN_ExistDependentRecord = (E_IOT_EYUN_BASE + 252),

    /** 连接分配的SessionKey不匹配 */
    E_IOT_EYUN_CONNSESSIONKEYNOTMATCH = (E_IOT_EYUN_BASE + 500),

    /** 连接Session超时 */
    E_IOT_EYUN_SESSIONKEYTIMEOUT = (E_IOT_EYUN_BASE + 501),

    /** 接口访问太频繁,1分钟后重新发送验证码 */
    E_IOT_EYUN_SENDVCODETOOOFTEN = (E_IOT_EYUN_BASE + 801),

    /** vcode不存在 */
    E_IOT_EYUN_VCODENOTEXIST = (E_IOT_EYUN_BASE + 802),

    /** vcode不匹配 */
    E_IOT_EYUN_VCODENOTMATCH = (E_IOT_EYUN_BASE + 803),

    /** APP应该重新调用SendVcode用于重新发送注册验证码 */
    E_IOT_EYUN_VCODEEXPIRED = (E_IOT_EYUN_BASE + 804),

    /** 用户已经存在,APP提示进入密码找回或者登陆 */
    E_IOT_EYUN_USEREXIST = (E_IOT_EYUN_BASE + 805),

    /** 用户账号被锁定(30分钟解锁) */
    E_IOT_EYUN_USERLOCK = (E_IOT_EYUN_BASE + 806),

    /** 用户账号被禁用 */
    E_IOT_EYUN_USERDISABLE = (E_IOT_EYUN_BASE + 807),

    /** 用户名密码不匹配 */
    E_IOT_EYUN_USERPASSWORDError = (E_IOT_EYUN_BASE + 808),

    /** 用户手机号码已经绑定 */
    E_IOT_EYUN_USERMOBILEEXIST = (E_IOT_EYUN_BASE + 809),

    /** 存储个人头像错误 */
    E_IOT_EYUN_USERHEADPICSAVEError = (E_IOT_EYUN_BASE + 810),

    /** 用户ID不匹配 */
    E_IOT_EYUN_REFRESHTOKENUSERIDNOTMATCH = (E_IOT_EYUN_BASE + 811),

    /** access_tokennotexist */
    E_IOT_EYUN_ACCESSTOKENNOTEXIST = (E_IOT_EYUN_BASE + 812),

    /** access_tokenlogout */
    E_IOT_EYUN_ACCESSTOKENLOGOUT = (E_IOT_EYUN_BASE + 813),

    /** oldmobilenotmatch */
    E_IOT_EYUN_OLDMOBILENOTMATCH = (E_IOT_EYUN_BASE + 815),

    /** 用户不存在 */
    E_IOT_EYUN_USERNOTEXIST = (E_IOT_EYUN_BASE + 816),

    /** qrcode不匹配 */
    E_IOT_EYUN_QRVCODENOTMATCH = (E_IOT_EYUN_BASE + 817),

    /** qrcode过期 */
    E_IOT_EYUN_QRVCODEEXPIRED = (E_IOT_EYUN_BASE + 818),

    /** 解锁参数不匹配 */
    E_IOT_EYUN_UNLOCKNOTMATCH = (E_IOT_EYUN_BASE + 819),

    /** 用户解锁参数未设置 */
    E_IOT_EYUN_UNLOCKNOTSET = (E_IOT_EYUN_BASE + 820),

    /** 用户手机号码不匹配 */
    E_IOT_EYUN_MOBILENOTMATCH = (E_IOT_EYUN_BASE + 821),

    /** 不能修改用户的国别 */
    E_IOT_EYUN_NOTMODIFYCC = (E_IOT_EYUN_BASE + 822),

    /** 设备已经被添加 */
    E_IOT_EYUN_DeviceAdded = (E_IOT_EYUN_BASE + 901),

    /** 设备不属于该用户 */
    E_IOT_EYUN_DeviceNotBelong = (E_IOT_EYUN_BASE + 902),

    /** 分享记录已经存在 */
    E_IOT_EYUN_SHAREEXIST = (E_IOT_EYUN_BASE + 903),

    /** 设备不存在 */
    E_IOT_EYUN_DeviceNotExist = (E_IOT_EYUN_BASE + 904),

    /** 设备离线 */
    E_IOT_EYUN_DeviceOffline = (E_IOT_EYUN_BASE + 905),

    /** 没有发现设备配置信息 */
    E_IOT_EYUN_DeviceConfigNotFind = (E_IOT_EYUN_BASE + 906),

    /** 必须设置设备安全添加码 */
    E_IOT_EYUN_DeviceNotSetSeccode = (E_IOT_EYUN_BASE + 907),

    /** 没有发现设备的安全添加码 */
    E_IOT_EYUN_DeviceNotFindSeccode = (E_IOT_EYUN_BASE + 908),

    /** 设备安全添加码UID不匹配 */
    E_IOT_EYUN_DeviceSeccodeNotMatchUid = (E_IOT_EYUN_BASE + 909),

    /** 安全添加码已经过期 */
    E_IOT_EYUN_SeccodeExpired = (E_IOT_EYUN_BASE + 910),

    /** 安全添加码不匹配 */
    E_IOT_EYUN_SeccodeNotMatch = (E_IOT_EYUN_BASE + 911),

    /** 设备不需要安全添加码 */
    E_IOT_EYUN_DeviceNotNeedSeccode = (E_IOT_EYUN_BASE + 912),

    /** 不存在需要确认的记录 */
    E_IOT_EYUN_ComfirmNOTFind = (E_IOT_EYUN_BASE + 913),

    /** 分享需要对方用户确认 */
    E_IOT_EYUN_ShareConfirmBYSUID = (E_IOT_EYUN_BASE + 914),

    /** 设备已经被解绑，没有拥有者 */
    E_IOT_EYUN_DeviceUnbind = (E_IOT_EYUN_BASE + 1001),

    /** 不支持的记录类型 */
    E_IOT_EYUN_UnsupportRecTyp = (E_IOT_EYUN_BASE + 1002),

    /** 未找到设备的云储存配置 */
    E_IOT_EYUN_NoDCSConfig = (E_IOT_EYUN_BASE + 1003),

    /** OssMaxFidxIll */
    E_IOT_EYUN_OssMaxFidxIll = (E_IOT_EYUN_BASE + 1004),

    /** AssumeRoleFault */
    E_IOT_EYUN_AssumeRoleFault = (E_IOT_EYUN_BASE + 1005),

    /** devcloudsservicehasexpired */
    E_IOT_EYUN_DCSConfigExpired = (E_IOT_EYUN_BASE + 1006),

    /** 密码长度必须是6-12位 */
    E_IOT_EYUN_PwdLengthError = (E_IOT_EYUN_BASE + 1101),

    /** 未确认的密码不能超过15个 */
    E_IOT_EYUN_PwdNumberError = (E_IOT_EYUN_BASE + 1102),

    /** 用户超过最大限制 */
    E_IOT_EYUN_PwdOverLimit = (E_IOT_EYUN_BASE + 1103),

    /** 用户过期时间不合法(小于当前系统UTC时间) */
    E_IOT_EYUN_PwdExpiredTimeError = (E_IOT_EYUN_BASE + 1104),

    /** 设备不是智能门锁 */
    E_IOT_EYUN_DeviceISNOTSMLK = (E_IOT_EYUN_BASE + 1105),

    /** 房间已经被绑定 */
    E_IOT_EYUN_RoomBinded = (E_IOT_EYUN_BASE + 1201),

    /** 身份证重复 */
    E_IOT_EYUN_IdnumberRepeated = (E_IOT_EYUN_BASE + 1202),

    /** 小区重复 */
    E_IOT_EYUN_BuildingRepeated = (E_IOT_EYUN_BASE + 1203),

    /** 云存储初始化故障 */
    E_IOT_EYUN_ReformerInitFault = (E_IOT_EYUN_BASE + 1301),

    /** 签名key不匹配 */
    E_IOT_EYUN_SignkeyNotMatch = (E_IOT_EYUN_BASE + 5100),

    /** 该用户下存在用户 */
    E_IOT_EYUN_ExistChildUID = (E_IOT_EYUN_BASE + 5000),

    /** regionid存在用户 */
    E_IOT_EYUN_RegionIDExistUser = (E_IOT_EYUN_BASE + 5001),

    /** regionid存在设备 */
    E_IOT_EYUN_RegionIDExistDevice = (E_IOT_EYUN_BASE + 5002),

    /** regionid存在下级节点 */
    E_IOT_EYUN_RegionExistNextNode = (E_IOT_EYUN_BASE + 5003),

    /** 用户不存在上级用户 */
    E_IOT_EYUN_UserNotExistParentUID = (E_IOT_EYUN_BASE + 5004),
};

/* 告知用户ppmq_handle，供外部控制休眠使用；一般模式不注册该回调,填NULL */
typedef void (*iot_on_ppmq_handle_t)(void *handle);

/* 连接平台状态回调函数类型 */
typedef void (*iot_on_connect_platform_t)(iot_conn_platform_state_t state);

/* 连接平台返回utc时间回调函数类型 */
typedef void (*iot_on_platform_utc_t)(uint64_t utc_ms);

/* 连接状态回调函数类型 */
typedef void (*iot_on_connect_t)(int32_t          conn_id,
                                 iot_conn_state_t state,
                                 iot_conn_type_t  type);

/* 流媒体数据回调函数类型 */
typedef void (*iot_on_media_t)(int32_t            conn_id,
                               iot_exhdr_media_t *exhdr,
                               const uint8_t     *media_data,
                               size_t             media_len);

typedef struct _ipc_RecvFile_Req {
    /* 文件id */
    int64_t file_id;
    /* 数据偏移量 */
    int64_t offset;
    /* 数据内容 */
    uint8_t *payload;

    /* 数据长度 */
    uint64_t payload_len;
} ipc_RecvFile_Req;

typedef struct _ipc_RecvFile_Resp {
    /* 0表示成功 */
    int32_t recode;
} ipc_RecvFile_Resp;

/* 文件接收回调函数类型 */
typedef void (*iot_on_RecvFile_t)(int32_t            conn_id,
                                  ipc_RecvFile_Req *req,
                                  ipc_RecvFile_Resp *rsp);

/**
 * @brief 数据是否丢包
 */
typedef enum {
    /** 没有丢包 */
    E_IOT_PACKET_NORMAL = 0,

    /** 处于丢包中 */
    E_IOT_PACKET_DROPOUT = 1,

} iot_packet_state_t;

/* 连接平台状态回调函数类型 */
typedef void (*iot_on_PacketDropout_t)(iot_packet_state_t state);

typedef struct {
    char     ipaddr[30];
    uint32_t ports_count;
    int32_t  ports[10];
} iot_server_t;

//蓝牙配网句柄
typedef void *iot_bdn_ref_t;

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

/****************************************************************************/
#ifdef __cplusplus
}
#endif
/*****************************************************************************
 * @} (end addtogroup xciot_type)
 */
/****************************************************************************/
#endif /* XCIOT_TYPE_H_INCLUDE */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
