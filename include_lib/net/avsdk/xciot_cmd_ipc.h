/*****************************************************************************
 * xciot_cmd_ipc.h
 *
 *
 *
 ****************************************************************************/
/*
 * wiki
 * ====
 */
#ifndef XCIOT_CMD_IPC_H_INCLUDE
#define XCIOT_CMD_IPC_H_INCLUDE
/*****************************************************************************
 * @addtogroup xciot_cmd_ipc
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

/*****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct _ipc_AlarmGet_Req {
    /** 通道编号，如果是IPC，该值为0 */
    int32_t channel;
} ipc_AlarmGet_Req;

typedef struct _ipc_AlarmGet_Resp {
    /** 移动侦测灵敏度: 0 关闭; 1 低; 2 中; 3 高*/
    uint32_t motion;
    /** 响声侦测灵敏度: 0 关闭; 1 低; 2 中; 3 高*/
    int32_t sound;
    /** 烟雾检测:  0 关闭； 1 打开*/
    int32_t smoke;
    /** 视频遮挡: 0 关闭； 1 打开*/
    int32_t shadow;
} ipc_AlarmGet_Resp;

typedef struct _ipc_AlarmSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_AlarmSet_Resp;

typedef struct _ipc_AlarmSet_Req {
    /** 通道编号，如果是IPC，该值为0 */
    int32_t channel;
    /** 移动侦测灵敏度: 0 关闭; 1 低; 2 中; 3 高*/
    uint32_t motion;
    /** 响声侦测灵敏度: 0 关闭; 1 低; 2 中; 3 高*/
    int32_t sound;
    /** 烟雾检测:  0 关闭； 1 打开*/
    int32_t smoke;
    /** 视频遮挡: 0 关闭； 1 打开*/
    int32_t shadow;
} ipc_AlarmSet_Req;

typedef struct _ipc_AudioPause_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_AudioPause_Resp;

typedef struct _ipc_AudioPause_Req {
    /** 通道编号，如果是IPC，该值为0 */
    int32_t channel;
} ipc_AudioPause_Req;

typedef struct _ipc_AudioPlay_Req {
    /** 通道编号，如果是IPC，该值为0 */
    int32_t channel;
} ipc_AudioPlay_Req;

typedef struct _ipc_AudioPlay_Resp {
    /** 0 正常; 1 设备处于私密模式; 2 当前连接数过多*/
    int32_t code;
    /** 返回设备端支持的声音编码,iot_mediastream_type_t*/
    int32_t codec;
    /** 采样频率: 8000 Hz*/
    int32_t rate;
    /** 采样位深，默认 16 = 16bit; 8 = 8bit*/
    int32_t bit;
    /** 1: mono(单声道) 2: stereo(立体声)*/
    int32_t track;
} ipc_AudioPlay_Resp;

typedef struct _ipc_Auth_Resp {
    char dummy_field;
} ipc_Auth_Resp;

typedef struct _ipc_Auth_Req {
    char    did[65];
    int32_t auth_type;
    int32_t req_time;
    char    auth_signkey[65];
} ipc_Auth_Req;

typedef struct _ipc_ConfigGet_NotifyInfo {
    /** status: 0 disable; 1 enable;*/
    int32_t states;
    /** if status = 1 ;时间任务中的时间要素*/
    xciot_Timetask tt;
    /** if status = 1 ; 1: low; 2: middle; 3: frequent，主要是触发频率不一样。
     *  low: 		运动开始触发一条，在3分钟内如果持续有移动不再触发；如果超过3分钟后再触发
     *  middle：		触发间隔是1分钟
     *  frequent: 	触发间隔是30秒*/
    int32_t level;
} ipc_ConfigGet_NotifyInfo;

typedef struct _ipc_ConfigGet_Req {
    /** 通道编号，如果是IPC，该值为0 */
    int32_t channel;
} ipc_ConfigGet_Req;

typedef struct _ipc_ConfigGet_WifiInfo {
    /** 0 不支持WIFI；1 支持WIFI*/
    int32_t support;
    /** WIFI名称*/
    char ssid[65];
    /** WIFI网络质量: dbm*/
    int32_t qos;
} ipc_ConfigGet_WifiInfo;

typedef struct _ipc_ConfigGet_Resp {
    /** wifi信息，@ipc_ConfigGet_WifiInfo*/
    ipc_ConfigGet_WifiInfo wifi;
    /** 翻转信息: 1: Upright, 2: Flip Horizontal，3 :Flip Vertical，4: turn 180*/
    int32_t flip;
    /** LED状态: 1 常开；2 常关；*/
    int32_t led_mode;
    /** IRCut状态: 1 常开；2 常关； 3 自动*/
    int32_t ircut_mode;
    /** 私密模式: 0 不开启私有模式; 1 开始私有模式*/
    int32_t secret_mode;
    /** 通知参数，@ipc_ConfigGet_NotifyInfo*/
    ipc_ConfigGet_NotifyInfo notify;
    /** 电源频率: 0 自动; 50 50HZ; 60 60HZ*/
    uint32_t power_freq;
    /** 音量大小: 0-10; 0 静音; 1-10 数字越大音量越大*/
    uint32_t volume;
    /** 录像时长，单位秒： 5, 10, 15, 30s*/
    uint32_t duration;
} ipc_ConfigGet_Resp;

typedef XCIOT_BYTES_ARRAY_T(2048) ipc_CustomCmd_Req_arg_bytes_t;
typedef struct _ipc_CustomCmd_Req {
    /** int32参数组元素个数*/
    uint32_t arg_int32_count;
    /** int32参数组*/
    int32_t arg_int32[20];
    /** 字符串数组元素个数*/
    uint32_t arg_string_count;
    /** 字符串数组*/
    char arg_string[20][64];
    /** 字节数组*/
    ipc_CustomCmd_Req_arg_bytes_t arg_bytes;
} ipc_CustomCmd_Req;

typedef XCIOT_BYTES_ARRAY_T(2048) ipc_CustomCmd_Resp_arg_bytes_t;
typedef struct _ipc_CustomCmd_Resp {
    /** int32参数组元素个数*/
    uint32_t arg_int32_count;
    /** int32参数组*/
    int32_t arg_int32[20];
    /** 字符串数组元素个数*/
    uint32_t arg_string_count;
    /** 字符串数组*/
    char arg_string[20][64];
    /** 字节数组*/
    ipc_CustomCmd_Resp_arg_bytes_t arg_bytes;
} ipc_CustomCmd_Resp;

typedef struct _ipc_DirCreate_Resp {
    char dummy_field;
} ipc_DirCreate_Resp;

typedef struct _ipc_DirCreate_Req {
    char dir_path[64];
    char dir_name[64];
} ipc_DirCreate_Req;

typedef struct _ipc_DirDel_Resp {
    char dummy_field;
} ipc_DirDel_Resp;

typedef struct _ipc_DirDel_Req {
    char dir_path[64];
    char dir_name[64];
} ipc_DirDel_Req;

typedef struct _ipc_DirEdit_Resp {
    char dummy_field;
} ipc_DirEdit_Resp;

typedef struct _ipc_DirEdit_Req {
    char dir_path[64];
    char old_dir_name[64];
    char new_dir_name[64];
} ipc_DirEdit_Req;

typedef struct _ipc_DirList_ipc_DirList {
    /** 名字 */
    char     name[64];
    /** 类型: 1 dir; 2 file(非图片和视频文件); 3 软连接; 4 图片文件； 5 视频文件 */
    int32_t  ftype;
    /** 所属群组 */
    char     owner_group[64];
    /** 所属用户 */
    char     owner_user[64];
    /** 最后修改时间 */
    int64_t  last_timesec;
    /** 文件大小 */
    int64_t  size;
    /** 访问权限 */
    char     acl_str[64];
    /** 文件/目录属性；第一位是否隐藏; 0: 不隐藏; 1: 隐藏 */
    int32_t  attr;
    /** 录像时长 */
    int32_t  video_sec;
    /** 文件ID，用于唯一标识文件(取文件时使用); 如果是录像文件返回下面两个值 */
    uint64_t file_id;
    /** 录像文件的缩略图 */
    uint64_t thum_fid;
} ipc_DirList_ipc_DirList;

typedef struct _ipc_DirList_Req {
    /** 路径 */
    char     dir_path[64];
    /** 第几页数据，数据是按照默认排序从 (page-1) * page_size 到 page * page_size 之间的数据 */
    uint32_t page;
    /** 每页多少条数据;默认20条 */
    uint32_t page_size;
    /** 筛选的文件分类的个数 */
    uint32_t filter_ftype_count;
    /** 筛选条件; 筛选的文件分类；1 dir; 2 file(非图片和视频文件); 3 软连接; 4 图片文件； 5 视频文件; */
    int32_t filter_ftype[6];
    /** 筛选文件属性的个数 */
    uint32_t filter_attr_count;
    /** 筛选文件属性;0: 不隐藏; 1: 隐藏 */
    int32_t filter_attr[6];
} ipc_DirList_Req;

typedef struct _ipc_DirList_Resp {
    int64_t                 total;
    uint32_t                lists_count;
    ipc_DirList_ipc_DirList lists[20];
} ipc_DirList_Resp;

typedef struct _ipc_Discovery_Req {
    /** 为空则撒网搜索  否则代表查询搜索*/
    char did[64];
    /** 产品类别代码*/
    char class_code[20];
} ipc_Discovery_Req;

typedef XCIOT_BYTES_ARRAY_T(1200) ipc_Discovery_Resp_skill_t;
typedef struct _ipc_Discovery_Resp {
    /** 设备序列号，该参数内部填充，用户不用关心*/
    char did[65];
    /** 设备IP地址 */
    char ipaddr[20];
    /** 网络掩码 */
    char netmask[20];
    /** 端口，该参数内部填充，用户不用关心*/
    int32_t lis_port;
    /** sdk版本号，该参数内部填充，用户不用关心*/
    char sdk_ver[65];
    /** 当前设备模式(wifi)： 1 AP 模式; 2 STD模式;*/
    uint32_t mode;
    /** 本地签名key，用于app绑定用户，该参数内部填充，用户不用关心*/
    char lsign[65];
    /** 设备特征信息，从web后台下载的json文件内容*/
    ipc_Discovery_Resp_skill_t skill;
} ipc_Discovery_Resp;

typedef struct _ipc_EventRecordGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_EventRecordGet_Req;

typedef struct _ipc_EventRecordGet_Resp {
    /** 录像时长，单位秒： 5, 10, 15, 30*/
    uint32_t duration;
} ipc_EventRecordGet_Resp;

typedef struct _ipc_EventRecordSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_EventRecordSet_Resp;

typedef struct _ipc_EventRecordSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 录像时长，单位秒： 5, 10, 15, 30*/
    uint32_t duration;
} ipc_EventRecordSet_Req;

typedef struct _ipc_ExecIOTCMD_Req {
    /* iot普通命令 */
    iot_PpiotCmd_t    iot_cmds;

} ipc_ExecIOTCMD_Req;

typedef struct _ipc_ExecIOTCMD_Resp {
    iot_PpiotCmd_t    iot_cmds;
} ipc_ExecIOTCMD_Resp;

typedef struct _ipc_ExecTimetask_Req {
    /* iot定时命令 */
    iot_TimetaskCmd_t    iot_cmds;

} ipc_ExecTimetask_Req;

typedef struct _ipc_ExecTimetask_Resp {
    /*返回响应错误代码;0 成功*/
    int32_t result;
    /*iot定时命令*/
    iot_TimetaskCmd_t    iot_cmds;
} ipc_ExecTimetask_Resp;


typedef struct _ipc_TamperSet_Req {
    /** 状态*/
    int64_t state;
} ipc_TamperSet_Req;

typedef struct _ipc_TamperSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_TamperSet_Resp;

typedef struct _ipc_SetCS_Req {
    /** 开通状态，0未开通；1，开通*/
    int64_t code;
    /* data中数据有效长度 */
    uint16_t data_len;
    /** 需要上报云存的id */
    uint8_t data[100];
} ipc_SetCS_Req;

typedef struct _ipc_SetCS_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_SetCS_Resp;

typedef struct _ipc_SetAutoTrack_Req {
    /** 自动跟踪: 0, 关闭; 1, 开启*/
    int64_t autoTrack;
} ipc_SetAutoTrack_Req;

typedef struct _ipc_SetAutoTrack_Resp {
    /** 操作返回码: 0, 正确; 大于0，参见错误代码定义*/
    int64_t respCode;
} ipc_SetAutoTrack_Resp;

typedef struct _ipc_GetAutoTrack_Req {
    /** 忽略*/
    char dummy_field;
} ipc_GetAutoTrack_Req;

typedef struct _ipc_GetAutoTrack_Resp {
    /** 自动跟踪: 0, 关闭; 1, 开启*/
    int64_t autoTrack;
} ipc_GetAutoTrack_Resp;

typedef XCIOT_BYTES_ARRAY_T(204800) ipc_FileAdd_Req_file_body_t;
typedef struct _ipc_FileAdd_Req {
    char                        dir_path[64];
    uint64_t                    file_id;
    char                        file_name[64];
    int64_t                     create_timesec;
    int64_t                     total_size;
    char                        file_suffix[64];
    ipc_FileAdd_Req_file_body_t file_body;
} ipc_FileAdd_Req;

typedef struct _ipc_FileAdd_Resp {
    int32_t code;
} ipc_FileAdd_Resp;

typedef struct _ipc_FileAttrSet_Resp {
    char dummy_field;
} ipc_FileAttrSet_Resp;

typedef struct _ipc_FileAttrSet_Req {
    /** 上一级目录 */
    char     dir_path[64];
    /** 文件id个数 */
    uint32_t file_ids_count;
    /** 文件id */
    uint64_t file_ids[20];
    /** 文件/目录属性；第一位是否隐藏; 0: 不隐藏; 1: 隐藏 */
    int32_t  attr;
} ipc_FileAttrSet_Req;

typedef struct _ipc_FileDel_Resp {
    char dummy_field;
} ipc_FileDel_Resp;

typedef struct _ipc_FileDel_Req {
    char dir_path[64];
    uint32_t file_id_count;
    uint64_t file_id[32];
} ipc_FileDel_Req;

typedef struct _ipc_FileEdit_Resp {
    char dummy_field;
} ipc_FileEdit_Resp;

typedef struct _ipc_FileEdit_Req {
    char     dir_path[64];
    uint64_t file_id;
    char     file_name[64];
} ipc_FileEdit_Req;

typedef struct _ipc_FileRecvRate_Resp {
    char dummy_field;
} ipc_FileRecvRate_Resp;

typedef struct _ipc_FileRecvRate_Req {
    uint64_t file_id;
    int64_t  recv_size;
} ipc_FileRecvRate_Req;

typedef struct _ipc_FileStart_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 需要获取的文件ID*/
    int64_t file_id;
    /** 请求要获取文件的偏移,默认0表示从0开始*/
    int64_t offset;
    /** 请求要获取文件的长度,默认0表示所有*/
    int64_t length;
} ipc_FileStart_Req;

typedef struct _ipc_FileStart_Resp {
    /** 0表示成功, 404表示文件不存在*/
    int32_t code;
    /** 文件总大小*/
    int64_t total_size;
    /** 文件MD5或者SHA1值,通过字符串长度确认是那一种HASH*/
    char file_hash[128];
} ipc_FileStart_Resp;

typedef struct _ipc_FileStop_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_FileStop_Resp;

typedef struct _ipc_FileStop_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 需要获取的文件ID*/
    int64_t file_id;
} ipc_FileStop_Req;

typedef struct _ipc_FileThumList_Req {
    uint64_t thum_fid;
    int32_t  thum_mode;
} ipc_FileThumList_Req;

typedef XCIOT_BYTES_ARRAY_T(524288) ipc_FileThumList_Resp_thum_body_t;
typedef struct _ipc_FileThumList_Resp {
    uint64_t                          thum_fid;
    ipc_FileThumList_Resp_thum_body_t thum_body;
} ipc_FileThumList_Resp;

typedef struct _ipc_FirmwareNotify_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_FirmwareNotify_Req;

typedef struct _ipc_FirmwareNotify_Resp {
    /** 0 执行升级； 大于0表示正在升级，并且返回升级进度;
    如果是平台通知升级则不用返回进度*/
    int32_t rate;
} ipc_FirmwareNotify_Resp;

typedef struct _ipc_FirmwareRate_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_FirmwareRate_Resp;

typedef struct _ipc_FirmwareRate_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 升级进度: 0-100*/
    int32_t rate;
} ipc_FirmwareRate_Req;

/* 设备固件下载查询 */
typedef struct _ipc_FirmwareCheckByDevice_Req {
    char did[65];
    char did_signkey[65];
    /*设备当前固件版本号 eg: 1.01.01.01_180701*/
    char firmware_ver[65];
} ipc_FirmwareCheckByDevice_Req;

typedef struct _ipc_FirmwareCheckByDevice_Resp {
    /* 0: 不需要升级; 1 需要升级 */
    int32_t is_update;
    /* 固件下载地址 */
    char    firmware_url[512];
    /* 服务器上固件版本号 */
    char    firmware_ver[65];
    /* 固件的md5值 */
    char    firmware_md5[65];
    /*0：用户确认升级; 1: 设备端强制升级; 2: 临时固件 */
    int32_t firmware_type;
    /*固件大小*/
    int32_t firmware_size;
} ipc_FirmwareCheckByDevice_Resp;

/* 设备固件下载查询 */
typedef struct _ipc_FirmwareChanCheckByDevice_Req {
    /*通道0,本身*/
    uint8_t ch;
    /*设备当前固件版本号 eg: 1.01.01.01_180701*/
    char firmware_ver[65];
} ipc_FirmwareChanCheckByDevice_Req;


typedef struct _ipc_FirmwareChanCheckByDevice_Resp {
    /* 模块通道编号 */
    int32_t ch;
    /* 0: 不需要升级; 1 需要升级 */
    int32_t is_update;
    /* 固件下载地址 */
    char    firmware_url[512];
    /* 服务器上固件版本号 */
    char    firmware_ver[65];
    /* 固件的md5值 */
    char    firmware_md5[65];
    /*0：用户确认升级; 1: 设备端强制升级; 2: 临时固件 */
    int32_t firmware_type;
    /*固件大小*/
    int32_t firmware_size;
} ipc_FirmwareChanCheckByDevice_Resp;


typedef struct _ipc_ScpeGetWhitelist_Resp {
    uint32_t ipv4s_count;
    uint32_t ipv4s[128];
    uint32_t ipv6s_count;
    char ipv6s[65][40];
    /* @@protoc_insertion_point(struct:ScpeGetWhitelist_Resp) */
} ipc_ScpeGetWhitelist_Resp;

/* Struct definitions */
typedef struct {
    /* 设备id */
    char did[33];
    /* 设备在app中的名字 */
    char name[33];
    /* 0,离线； 1.在线 */
    int  state;
} mangerDid;

typedef struct {
    /*获取第几页的数据，从1开始*/
    uint32_t page;
    /* 每次返回最大个数，由resp中mdids数组决定 */
    uint32_t page_size;
} gate_MDid_Req;

typedef struct {
    /* 一共有多少设备 */
    int64_t total;
    /* 当前返回的个数 */
    uint32_t mdids_count;
    /* 信息 */
    mangerDid mdids[15];
} gate_MDid_Resp;

typedef struct _ipc_FlipGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_FlipGet_Req;

typedef struct _ipc_FlipGet_Resp {
    /** 1: Upright, 2: Flip Horizontal，
     *  3 :Flip Vertical，4: turn 180*/
    int32_t flip;
} ipc_FlipGet_Resp;

typedef struct _ipc_FlipSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_FlipSet_Resp;

typedef struct _ipc_FlipSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 1: Upright, 2: Flip Horizontal，
     *  3 :Flip Vertical，4: turn 180*/
    int32_t flip;
} ipc_FlipSet_Req;

typedef struct _ipc_HistoryDayList_HistoryInfo {
    /** 录像开始时间(ms级utc时间)*/
    int64_t start_time;
    /** 录像时长 , 单位毫秒(ms),SD卡录像(音视频)最后一帧的时间戳
     *  必须等于: start_time+length*/
    int32_t length;
    /** 文件ID，用于唯一标识文件(取文件时使用)*/
    int64_t file_id;
    /** 录像文件的缩略图ID*/
    int64_t thum_fid;
    /** 录像分类
     * 0，全部; 255, 正常录像;
     * 其他的类型参考@iot_event_id_t*/
    int32_t history_type;
} ipc_HistoryDayList_HistoryInfo;

typedef struct _ipc_HistoryDayList_HistoryTimeRange {
    /** 录像开始时间(ms级utc时间)*/
    int64_t start_time;
    /** 录像时长 , 单位毫秒(ms),SD卡录像(音视频)最后一帧的时间戳
     *  必须等于: start_time+length*/
    int32_t length;
    /** 录像分类
     * 0，全部; 255, 正常录像;
     * 其他的类型参考@iot_event_id_t*/
    int32_t history_type;
} ipc_HistoryDayList_HistoryTimeRange;

typedef struct _ipc_HistoryDayList_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 获取视频的日期: 格式: 20170801 (YYYYMMDD),0表示全部视频*/
    int32_t day;
    /** 分页信息,第几页数据，数据是按照默认排序从
     * (page-1) * page_size 到 page * page_size 之间的数据*/
    uint32_t page;
    /** 每页多少条数据;默认20条*/
    uint32_t page_size;
    /** 时间顺序: 1 时间到序(从大到小); 2 时间顺序(从小到大)*/
    int32_t order;
    /** 检索条件: 开始时间点,UTC时间，单位毫秒*/
    int64_t start_time;
    /*录像分类个数*/
    uint32_t history_type_count;
    /** 检索条件: 录像分类: 0 全部； 255, 正常录像；
     *  9999, 除了正常录像(255)以外的所有录像
     * 其他分类参考iot_event_id_t*/
    int32_t history_type[20];
    /* 返回数据的方式: 0 自动(原来处理方式维持不变);
     * 1 强制文件列表方式返回，赋值 Resp.historys*/
    int32_t rtype;
} ipc_HistoryDayList_Req;

typedef struct _ipc_HistoryDayList_Resp {
    /** 总记录数*/
    int64_t total;
    /** 当前返回的记录个数(事件录像，与连续录像互斥)*/
    uint32_t historys_count;
    /** 历史记录信息(事件录像，与连续录像互斥)*/
    ipc_HistoryDayList_HistoryInfo historys[100];
    /** 当前返回的记录个数(连续录像)*/
    uint32_t history_range_count;
    /** 历史记录信息(连续录像)*/
    ipc_HistoryDayList_HistoryTimeRange history_range[100];
} ipc_HistoryDayList_Resp;

typedef struct _ipc_HistoryDays_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 需要查询的年(2019)*/
    int32_t year;
    /** 需要查询的月(1-12)*/
    int32_t month;
} ipc_HistoryDays_Req;

typedef struct _ipc_HistoryDays_Resp {
    /** 总共拥有的记录数*/
    int64_t total;
    /** days数组元素的个数*/
    uint32_t days_count;
    /** 存在录像的日期,格式: 1-31*/
    int32_t days[50];
} ipc_HistoryDays_Resp;

typedef struct _ipc_HistoryDel_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_HistoryDel_Resp;

typedef struct _ipc_HistoryDel_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 文件ID的个数*/
    uint32_t file_id_count;
    /**文件ID数组*/
    int64_t file_id[50];
} ipc_HistoryDel_Req;

typedef struct _ipc_HistoryPause_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_HistoryPause_Resp;

typedef struct _ipc_HistoryPause_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /**文件ID;如果播放连续录像，则该值为0*/
    int64_t file_id;
} ipc_HistoryPause_Req;

typedef struct _ipc_HistoryPlanGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_HistoryPlanGet_Req;

typedef struct _ipc_HistoryPlanGet_Resp {
    /** 录像类型: 1, 连续录像; 2, 事件录像s*/
    int32_t record_type;
    /** 时间段的个数(最多3个)*/
    uint32_t tt_count;
    /** 时间段*/
    xciot_Timetask tt[10];
    /** 是否开启录像; 0: disable; 1 enable*/
    int32_t enable;
} ipc_HistoryPlanGet_Resp;

typedef struct _ipc_HistoryPlanSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_HistoryPlanSet_Resp;

typedef struct _ipc_HistoryPlanSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 录像类型: 1, 连续录像; 2, 事件录像*/
    int32_t record_type;
    /** 时间段的个数*/
    uint32_t tt_count;
    /** 时间段*/
    xciot_Timetask tt[10];
    /** 是否开启录像; 0: disable; 1 enable*/
    int32_t enable;
} ipc_HistoryPlanSet_Req;

typedef struct _ipc_HistoryPlay_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 文件ID，用于唯一标识文件(取文件时使用);如果播放连续录像，则该值为0*/
    int64_t file_id;
    /** UTC时间,单位毫秒,时间拖动: 如果为0表示从
     * 该文件开始部分开始播放;如果大于0表示从指
     * 定时间点播放*/
    int64_t start_time;
} ipc_HistoryPlay_Req;

typedef struct _ipc_HistoryPlay_Resp {
    /** 响应参数: 0 成功;  1 file_id 不存在;
     * 2 skip_time 不在file_id的录像时间范围内;*/
    int32_t code;
    /** 帧率*/
    int32_t fps;
    /** 返回设备端支持的声音编码,iot_mediastream_type_t */
    int32_t codec;
    /** 采样频率: 8000 Hz*/
    int32_t rate;
    /** 采样位深，默认 16 = 16bit; 8 = 8bit*/
    int32_t bit;
    /** 1: mono(单声道) 2: stereo(立体声) */
    int32_t track;
    /** 渲染(涉及到视频矫正算法,全景使用) 0 无矫正; 1 易瞳校准SDK;*/
    int32_t render;
} ipc_HistoryPlay_Resp;

typedef struct _ipc_HistoryThumGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 录像文件的缩略图ID*/
    int64_t thum_fid;
} ipc_HistoryThumGet_Req;

typedef XCIOT_BYTES_ARRAY_T(81920) ipc_HistoryThumGet_Resp_thum_body_t;
typedef struct _ipc_HistoryThumGet_Resp {
    /** 录像文件的缩略图ID*/
    int64_t thum_fid;
    /** 缩略图内容. 缩略图内容尽量小于40KB*/
    ipc_HistoryThumGet_Resp_thum_body_t thum_body;
} ipc_HistoryThumGet_Resp;

typedef struct _ipc_IRCutGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_IRCutGet_Req;

typedef struct _ipc_IRCutGet_Resp {
    /** 1 常开；2 常关； 3 自动*/
    int32_t mode;
} ipc_IRCutGet_Resp;

typedef struct _ipc_IRCutSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_IRCutSet_Resp;

typedef struct _ipc_IRCutSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 1 常开；2 常关； 3 自动*/
    int32_t mode;
} ipc_IRCutSet_Req;

typedef struct _ipc_LanAuth_Req {
    int32_t channel;
    char    user[65];
    char    pwd[65];
} ipc_LanAuth_Req;

typedef struct _ipc_LanAuth_Resp {
    char session_key[65];
} ipc_LanAuth_Resp;

typedef struct _ipc_LedGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_LedGet_Req;

typedef struct _ipc_LedGet_Resp {
    /** 1 常开；2 常关*/
    int32_t mode;
} ipc_LedGet_Resp;

typedef struct _ipc_LedSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_LedSet_Resp;

typedef struct _ipc_LedSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 1 常开；2 常关s*/
    int32_t mode;
} ipc_LedSet_Req;

typedef struct _ipc_LedModeSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 0(智能双光)； 1，红外灯； 2，白光灯*/
    int32_t mode;
} ipc_LedModeSet_Req;

typedef struct _ipc_LedModeSet_Resp {
    /** 操作返回码: 0, 正确; 1，失败*/
    char respCode;
} ipc_LedModeSet_Resp;

typedef struct _ipc_LedModeGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_LedModeGet_Req;

typedef struct _ipc_LedModeGet_Resp {
    /** 0(智能双光)； 1，红外灯； 2，白光灯*/
    int32_t mode;
} ipc_LedModeGet_Resp;

typedef struct _ipc_LogSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_LogSet_Resp;

typedef struct _ipc_LogSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 状态: 0 关闭日志; 1 开启日志*/
    int32_t states;
} ipc_LogSet_Req;

typedef struct _ipc_MotionzoneGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_MotionzoneGet_Req;

typedef struct _ipc_MotionzoneGet_XYPoint {
    /** 左上([0,0]开始)*/
    int32_t leftup_x;
    int32_t leftup_y;
    /** 右下([100,100]结束)*/
    int32_t rightdown_x;
    int32_t rightdown_y;
} ipc_MotionzoneGet_XYPoint;

typedef XCIOT_BYTES_ARRAY_T(50) ipc_MotionzoneGet_Resp_mz_t;
typedef struct _ipc_MotionzoneGet_Resp {
    /** 将显示区域划分为: 22x18的区块，共需要50个
     * 字节来表示，该区块被选中，则为1，反之为0*/
    ipc_MotionzoneGet_Resp_mz_t mz;
    /** 界面设置的坐标个数; 最大5个*/
    uint32_t points_count;
    /** 界面设置的坐标*/
    ipc_MotionzoneGet_XYPoint points[5];
} ipc_MotionzoneGet_Resp;

typedef struct _ipc_MotionzoneSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_MotionzoneSet_Resp;

typedef struct _ipc_MotionzoneSet_XYPoint {
    /** 左上([0,0]开始)*/
    int32_t leftup_x;
    int32_t leftup_y;
    /** 右下([100,100]结束)*/
    int32_t rightdown_x;
    int32_t rightdown_y;
} ipc_MotionzoneSet_XYPoint;

typedef XCIOT_BYTES_ARRAY_T(60) ipc_MotionzoneSet_Req_mz_t;
typedef struct _ipc_MotionzoneSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 后台可以选择4:3,8:6,12:9,16:12,20:15,24*18等6类分辨率
     *  以24*18为例，将显示区域划分为: 24x18的区块，共需要54个
     *  Bit来表示，该区块被选中，则为1，反之为0*/
    ipc_MotionzoneSet_Req_mz_t mz;
    /** 界面设置的坐标个数; 最大5个,与mz互斥*/
    uint32_t points_count;
    /** 界面设置的坐标*/
    ipc_MotionzoneSet_XYPoint points[5];
} ipc_MotionzoneSet_Req;

typedef struct _ipc_NotifyGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_NotifyGet_Req;

typedef struct _ipc_NotifyGet_Resp {
    /** status: 0 disable; 1 enable;*/
    int32_t states;
    /** if status = 1 ;时间任务中的时间要素*/
    xciot_Timetask tt;
    /** if status = 1 ; 1: low; 2: middle; 3: frequent，
     * 主要是触发频率不一样:
     * low: 在3分钟内如果持续有移动不再触发；如果超过3分钟后再触发
     * middle： 触发间隔是1分钟
     * frequent: 触发间隔是30秒*/
    int32_t level;
} ipc_NotifyGet_Resp;

typedef struct _ipc_NotifySet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_NotifySet_Resp;

typedef struct _ipc_NotifySet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 0 disable; 1 enable*/
    int32_t states;
    /** if states = 1 ;时间任务中的时间要素*/
    xciot_Timetask tt;
    /** if status = 1 ; 1: low; 2: middle; 3: frequent，
     * 主要是触发频率不一样:
     * low: 在3分钟内如果持续有移动不再触发；如果超过3分钟后再触发
     * middle： 触发间隔是1分钟
     * frequent: 触发间隔是30秒*/
    int32_t level;
} ipc_NotifySet_Req;

typedef struct _ipc_PowerFreqGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_PowerFreqGet_Req;

typedef struct _ipc_PowerFreqGet_Resp {
    /** 电源频率: 0 自动： 50=50HZ, 60=60HZ*/
    uint32_t power_freq;
} ipc_PowerFreqGet_Resp;

typedef struct _ipc_PowerFreqSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_PowerFreqSet_Resp;

typedef struct _ipc_PowerFreqSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 电源频率: 0 自动： 50=50HZ, 60=60HZ*/
    uint32_t power_freq;
} ipc_PowerFreqSet_Req;

typedef struct _ipc_PspAdd_Resp {
    /** 预置点id*/
    int32_t psp_id;
} ipc_PspAdd_Resp;

typedef struct _ipc_PspAdd_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 预置点名称; 长度16字节*/
    char psp_name[65];
    /** 是否是看守位(所有预置点只能有一个
     * 看守位,需要清除其它看守位)*/
    bool is_def;
    /** 预置点id*/
    int32_t psp_id;
} ipc_PspAdd_Req;

typedef struct _ipc_PspCall_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_PspCall_Resp;

typedef struct _ipc_PspCall_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 预置点编号*/
    int32_t psp_id;
} ipc_PspCall_Req;

typedef struct _ipc_PspDel_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_PspDel_Resp;

typedef struct _ipc_PspDel_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 预置点编号*/
    int32_t psp_id;
} ipc_PspDel_Req;

typedef struct _ipc_PspList_PspInfo {
    /** 预置点编号*/
    int32_t psp_id;
    /** 预置点名称*/
    char psp_name[65];
    /** 是否是看守位(所有预置点只能有一个
     * 看守位,需要清除其它看守位)*/
    bool is_def;
} ipc_PspList_PspInfo;

typedef struct _ipc_PspList_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_PspList_Req;

typedef struct _ipc_PspList_Resp {
    /** 已经设置的预置点的个数*/
    uint32_t psps_count;
    /** 返回已经设置的预置点*/
    ipc_PspList_PspInfo psps[18];
} ipc_PspList_Resp;

typedef struct _ipc_PtzCtrl_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_PtzCtrl_Resp;

typedef struct _ipc_PtzCtrl_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 功能代码@iot_func_code_type_t*/
    int32_t func_code;
    /** func_code 可能需要的参数	*/
    int32_t para1;
    /** func_code 可能需要的参数	*/
    int32_t para2;
} ipc_PtzCtrl_Req;

typedef struct _ipc_Reboot_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_Reboot_Resp;

typedef struct _ipc_Reboot_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_Reboot_Req;

typedef struct _ipc_Reset_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_Reset_Resp;

typedef struct _ipc_Reset_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_Reset_Req;

typedef struct _ipc_Screenshot_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_Screenshot_Req;

typedef XCIOT_BYTES_ARRAY_T(122880) ipc_Screenshot_Resp_body_t;
typedef struct _ipc_Screenshot_Resp {
    /** 1 png; 2 jpg;*/
    int32_t format;
    /** byte数组(最大120k)*/
    ipc_Screenshot_Resp_body_t body;
} ipc_Screenshot_Resp;

typedef struct _ipc_SecretGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_SecretGet_Req;

typedef struct _ipc_SecretGet_Resp {
    /** 0 不开启私有模式; 1 开始私有模式*/
    int32_t secret;
} ipc_SecretGet_Resp;

typedef struct _ipc_SecretSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_SecretSet_Resp;

typedef struct _ipc_SecretSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 0 不开启私有模式; 1 开始私有模式*/
    int32_t secret;
} ipc_SecretSet_Req;

typedef struct _ipc_StorageFormat_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_StorageFormat_Req;

typedef struct _ipc_StorageFormat_Resp {
    /** 0 成功; 1 正在执行格式化; 3: 存储卡损坏; 4: 未插卡*/
    int32_t code;
} ipc_StorageFormat_Resp;

typedef struct _ipc_StorageFormatRate_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_StorageFormatRate_Resp;

typedef struct _ipc_StorageFormatRate_Req {
    /** 存储设备格式化执行百分比 0-100(格式化进度回调，100为完成); -1 格式化失败*/
    int32_t rate;
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_StorageFormatRate_Req;

typedef struct _ipc_StorageInfo_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_StorageInfo_Req;

typedef struct _ipc_StorageInfo_Resp {
    /** 存储设备当前状态(0: 正常使用; 1: 正在格式化； 2: 未格式化; 3: 存储卡损坏; 4: 未插卡; )*/
    int32_t status;
    /*当只有 status = (0,1,2) 时返回后续两个参数*/
    /** true; 存储设备总大小(MB)*/
    int64_t total_size;
    /** 存储设备使用空间(MB)*/
    int64_t use_size;
} ipc_StorageInfo_Resp;

typedef struct _ipc_TalkbackPause_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_TalkbackPause_Resp;

typedef struct _ipc_TalkbackPause_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_TalkbackPause_Req;

typedef struct _ipc_TalkbackPlay_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_TalkbackPlay_Req;

typedef struct _ipc_TalkbackPlay_Resp {
    /** 0 正常; 1 设备处于 Private mode; 2 当前连接数过多*/
    int32_t code;
    /** 返回设备端支持的声音编码*/
    int32_t codec;
    /** 采样频率: 8000 Hz*/
    int32_t rate;
    /** 采样位深，默认 16 = 16bit; 8 = 8bit*/
    int32_t bit;
    /** 1: mono(单声道) 2: stereo(立体声)*/
    int32_t track;
} ipc_TalkbackPlay_Resp;

typedef struct _ipc_TimedcruiseGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_TimedcruiseGet_Req;

typedef struct _ipc_TimedcruiseGet_Resp {
    /** status: 0 disable; 1 enable*/
    int32_t states;
    /** 巡航模式: 1, 完整巡航; 2, 预置点巡航*/
    int32_t mode;
    /** 时间参数*/
    xciot_Timetask tt;
    /** 巡航时间间隔,单位秒；取值范围: 600-43200 之间的数据*/
    int32_t interval;
} ipc_TimedcruiseGet_Resp;

typedef struct _ipc_TimedcruiseSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_TimedcruiseSet_Resp;

typedef struct _ipc_TimedcruiseSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** status: 0 disable; 1 enable*/
    int32_t states;
    /** 巡航模式: 1, 完整巡航; 2, 预置点巡航*/
    int32_t mode;
    /** 时间参数*/
    xciot_Timetask tt;
    /** 巡航时间间隔,单位秒；取值范围: 600-43200 之间的数据*/
    int32_t interval;
} ipc_TimedcruiseSet_Req;

typedef struct _ipc_TimeGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_TimeGet_Req;

typedef struct _ipc_TimeGet_Resp {
    /** UTC时间, 单位MS*/
    int64_t now_time;
    /** 采用标准时区，时区信息携带了夏令时等信息*/
    char time_zone[65];
    /** 是否开启夏令时: 0: disable; 1: enable*/
    int32_t dst;
    /**  时间偏移量，单位分钟*/
    int32_t offset;
} ipc_TimeGet_Resp;

typedef struct _ipc_TimeSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_TimeSet_Resp;

typedef struct _ipc_TimeSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** UTC时间, 单位MS*/
    int64_t now_time;
    /** 采用标准时区，时区信息携带了夏令时等信息*/
    char time_zone[65];
    /** 是否开启夏令时: 0: disable; 1: enable*/
    int32_t dst;
    /**  时间偏移量，单位分钟*/
    int32_t offset;
} ipc_TimeSet_Req;

typedef struct _ipc_VideoPause_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_VideoPause_Resp;

typedef struct _ipc_VideoPause_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_VideoPause_Req;

typedef struct _ipc_VideoPlay_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 图像质量: 1-100; 数字越大画面质量越高.
     * 0: auto；1-5： 480； 6-10： 720；
     * 11-15： 960； 16-20： 1080； 21-25：2k*/
    int32_t qos;
    /** 建议码率: bps*/
    int32_t speed;
} ipc_VideoPlay_Req;

typedef struct _ipc_VideoPlay_Resp {
    /** 0 正常; 1 设备处于私密模式; 2 当前连接数过多*/
    int32_t code;
    /** 数字越大画面质量越高. 0: auto；
     *  1-5： 480； 6-10： 720； 11-15： 960； 16-20： 1080； 21-25：2k
     */
    int32_t qos;
    /** 帧率*/
    int32_t fps;
    /** 视频格式，参考iot_mediastream_type_t*/
    int32_t format;
    /** 渲染(涉及到视频矫正算法,全景使用) 0 无矫正; 1 易瞳校准SDK;*/
    int32_t render;
} ipc_VideoPlay_Resp;

typedef struct _ipc_VideoQosSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_VideoQosSet_Resp;

typedef struct _ipc_VideoQosSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 数字越大画面质量越高. 0: auto；
     *  1-5： 480； 6-10： 720； 11-15： 960； 16-20： 1080； 21-25：2k
     */
    int32_t qos;
    /** 码率: bps*/
    int32_t speed;
} ipc_VideoQosSet_Req;

typedef struct _ipc_VolumeGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_VolumeGet_Req;

typedef struct _ipc_VolumeGet_Resp {
    /** 音量大小: 0-10; 0 静音; 1-10 数字越大音量越大*/
    uint32_t volume;
} ipc_VolumeGet_Resp;

typedef struct _ipc_VolumeSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_VolumeSet_Resp;

typedef struct _ipc_VolumeSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 音量大小: 0-10; 0 静音; 1-10 数字越大音量越大*/
    uint32_t volume;
} ipc_VolumeSet_Req;

typedef struct _ipc_WifiAPGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_WifiAPGet_Req;

typedef struct _ipc_WifiAPGet_WifiInfo {
    /** WIFI名称*/
    char ssid[65];
    /** WIFI网络质量: dbm*/
    int32_t qos;
} ipc_WifiAPGet_WifiInfo;

typedef struct _ipc_WifiAPGet_Resp {
    /** 0 不支持WIFI；1 支持WIFI*/
    int32_t support;
    /** 返回WIFI的个数*/
    uint32_t wifis_count;
    /** WIFI信息*/
    ipc_WifiAPGet_WifiInfo wifis[20];
} ipc_WifiAPGet_Resp;

typedef struct _ipc_WifiGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_WifiGet_Req;

typedef struct _ipc_WifiGet_Resp {
    /** 0 不支持WIFI；1 支持WIFI*/
    int32_t support;
    /** WIFI名称*/
    char ssid[65];
    /** WIFI网络质量: dbm*/
    int32_t qos;
} ipc_WifiGet_Resp;

typedef struct _ipc_WifiSet_Resp {
    /** 忽略*/
    char dummy_field;
} ipc_WifiSet_Resp;

typedef struct _ipc_WifiSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** WIFI名称*/
    char ssid[65];
    /** WIFI密码*/
    char pwd[65];
} ipc_WifiSet_Req;

typedef struct {
    int32_t show_x;
    int32_t show_y;
} ipc_AxisInfo;

typedef struct {
    /* 0 不显示通道名; 1 显示通道名 */
    int32_t chan_name_enable;
    /* 通道名设置 */
    char chan_name[64];
    /* 通道名坐标 */
    ipc_AxisInfo name_axis;
    /* 0 不显示时间； 1 显示时间 */
    int32_t datetime_enable;
    /* 时间坐标 */
    ipc_AxisInfo datetime_axis;
    /* 0: XXXX-XX-XX 年月日 ; 1: XX-XX-XXXX 月日年 ; 2: XXXX年XX月XX日; 3: XX月XX日XXXX年; 4: XX-XX-XXXX 日月年; 5: XX日XX月XXXX年 */
    int32_t show_format;
    /* 小时制: 0, 0-24小时制; 1, 1-12小时制 */
    int32_t hour_format;
    /* 星期几，0 不显示； 1 显示 */
    int32_t show_week;
    /* 1, 透明; 2, 闪烁 (保留) */
    int32_t datetime_attr;
    /* 自定义显示字符: 0 不显示； 1 显示 */
    int32_t custom1_show;
    /* 自定义显示的字符串 ( 最大长度为44字节 ) */
    char show_custom1_str[64];
    /* 自定义显示的坐标 */
    ipc_AxisInfo custom1_axis;
    /* 通道名字体大小; 0: 系统自动; >0, 字体所占用的像素大小 */
    int32_t chan_name_fontsize;
    /* 时间字体大小; 0: 系统自动; >0, 字体所占用的像素大小 */
    int32_t datetime_fontsize;
    /* 自定义字体大小; 0: 系统自动; >0, 字体所占用的像素大小 */
    int32_t custom1_fontsize;
} ipc_OsdInfo;

typedef struct {
    int32_t channel;
} ipc_OsdGet_Req;

typedef struct {
    ipc_OsdInfo osd;
} ipc_OsdGet_Resp;

typedef struct {
    int32_t channel;
    ipc_OsdInfo osd;
} ipc_OsdSet_Req;

typedef struct {
    char dummy_field;
} ipc_OsdSet_Resp;


/* Struct definitions */
typedef struct _ipc_ObjDesc {
    /** 物体编号: 1, 人脸; 2, 人形; 3, GPS定位信息;
     *  4, 自动跟踪坐标; 5, 热成像; 256, 车
     */
    int32_t obj_type;
    /** obj_id的个数*/
    int32_t obj_id_count;
    /** obj_type == 1, 该处是人脸的人员编号(peopleid);
     *  obj_type == 2，3，256， 该值为空
     */
    char obj_id[6][45];
    /** 目标坐标的个数，一个对象4个axis,[x,y,x,y]*/
    int32_t axis_count;
    /** 目标坐标 4个数字,左下为(0,0)开始计数;
     *  左下(x,y),右上(x,y); x,y 为像素编号
     *  A.如果 obj_type == 3; 数组索引说明:
     *  0: 经度(经度 = 真实经度 * 100000 取整的值);
     *  1: 维度(纬度 = 真实纬度 * 100000 取整的值);
     *  2: 速度(m/s, 速度 = 真实速度 * 10 取整的值);
     *  3：车头朝向角度0-360(以正北为0度，顺时针增加,保留小数点后面2位，存放数据扩大100倍变为整)
     *
     *  B.如果 obj_type == 4; 表示自动跟踪坐标
     *  目标坐标 4个数字,依次为，左下(x,y)=(0，0),右上(x,y)=(360,180);
     *
     *  C.如果 obj_type == 5; axis[0]表示内容部分中的温度开始的位置(从0开始计数)，也就是热成像数据长度;
     *  视频传输内容 = 热成像数据 + 温度数据;
     *  axis[1]表示设备当前状态: 1, 在线闲置; 2, 烹饪中; 3, 干烧报警; 4, 火灾报警/高风险
     */
    int32_t axis[12];
} ipc_ObjDesc;

typedef struct _ipc_AIDesc {
    /**objs的个数*/
    int32_t objs_count;
    /** objs数组*/
    ipc_ObjDesc objs[1];

} ipc_AIDesc;

typedef struct _ipc_PeopleID {
    /**id的个数,最大6个*/
    int32_t id_count;
    /** objs数组*/
    char obj_id[6][45];
} ipc_PeopleID_t;

struct ipc_coord {
    /*isHaveCoord为false时，其他值无效*/
    bool isHaveCoord;
    /*左下x坐标*/
    uint16_t x_bl;
    /*左下y坐标*/
    uint16_t y_bl;
    /*右上x坐标*/
    uint16_t x_tr;
    /*右上y坐标*/
    uint16_t y_tr;
};

typedef struct _ipc_picture_info {
    /** 图片内容 */
    uint8_t   *p;
    /** 图片长度 */
    uint32_t len;
    /** 图片里面对象的坐标 */
    struct ipc_coord xy;
} ipc_picture_info_t;

typedef struct _ipc_picture {
    /**图片的个数,最大15个*/
    uint8_t count;
    /** objs数组*/
    ipc_picture_info_t data[15];
} ipc_picture_t;

typedef struct _ipc_PirGet_PirInfo {
    /** PIR编号，当一个设备有多个PIR的时候，从左到右。
     * 数字从1开始进行+1编号；如果为0表示所有PIR
     */
    int32_t num;
    /** 对应PIR灵敏度设置
     * 设备端支持红外测距: 该值是距离的表达，单位米
     * 设备端不支持红外测距: 该值是灵敏度表达: 0 关闭; 1, 低; 2, 中; 3, 高
     */
    int32_t level;
} ipc_PirGet_PirInfo;

typedef struct _ipc_PirGet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
} ipc_PirGet_Req;

typedef struct _ipc_PirGet_Resp {
    /** 人体感应传感器的个数*/
    uint32_t pirs_count;
    /** 传感器参数*/
    ipc_PirGet_PirInfo pirs[5];
} ipc_PirGet_Resp;

typedef struct _ipc_ChanState_Req {
    /** 忽略 */
    char dummy_field;
} ipc_ChanState_Req;

typedef struct _ipc_ChanState_ChanInfo {
    /** 通道编号，如果是IPC，该值为0 */
    int32_t channel;
    /** 0： 没有设备; 1: online; 2: offline */
    int32_t state;
} ipc_ChanInfo;

typedef struct _ipc_ChanState_Resp {
    /** chans的个数 */
    uint32_t  chans_count;
    /** 通道信息 */
    ipc_ChanInfo chans[32];
} ipc_ChanState_Resp;

typedef struct _ipc_VideoChanChange_Req {
    /** 通道编号个数 */
    uint32_t channel_count;
    /** 请求视频的通道编号: 1,2,3,4 或者 1,8,2,4 */
    int32_t channel[32];
} ipc_VideoChanChange_Req;

typedef struct _ipc_VideoChanChange_Resp {
    /** 需要矫正的个数 */
    uint32_t render_count;
    /** 渲染(涉及到视频矫正算法,全景使用) 0 无矫正; 1 易瞳校准SDK */
    int32_t render[32];
} ipc_VideoChanChange_Resp;

typedef struct _ipc_PirSet_Resp {
    char dummy_field;
} ipc_PirSet_Resp;

typedef struct _ipc_PirSet_PirInfo {
    /* PIR编号，当一个设备有多个PIR的时候，从左到右。
     * 数字从1开始进行+1编号；如果为0表示所有PIR
     */
    int32_t num;
    /* 对应PIR灵敏度设置
     * 设备端支持红外测距: 该值是距离的表达，单位米
     * 设备端不支持红外测距: 该值是灵敏度表达: 0 关闭; 1, 低; 2, 中; 3, 高
     */
    int32_t level;
} ipc_PirSet_PirInfo;

typedef struct _ipc_PirSet_Req {
    /** 通道编号，如果是IPC，该值为0*/
    int32_t channel;
    /** 传感器设置参数*/
    ipc_PirSet_PirInfo pir;
} ipc_PirSet_Req;

typedef struct _ipc_GetNetworkInfo_Req {
    /** 通道编号，如果是IPC，该值为0 */
    int32_t channel;
} ipc_GetNetworkInfo_Req;

typedef struct _ipc_GetNetworkInfo_Resp {
    /** ssid如果为空表示时rj45连接，界面不显示 */
    char ssid[64];
    /** 信号强度: 0不支持;网络质量: 1-6;数字越大网络质量越好;只有在接入是无线的情况下有效
     *  wifi信号强度跟qos转换关系如下：
     *  wifi_dbm >=-25       --> qos=6
     *  -35<= wifi_dbm < -25 --> qos=5
     *  -45<= wifi_dbm < -35 --> qos=4
     *  -50<= wifi_dbm < -45 --> qos=3
     *  -60< wifi_dbm < -50  --> qos=2
     *  wifi_dbm < -60       --> qos=1
     *  有线网                  --> qos=0
     */
    int64_t qos;
    /** IP地址 */
    char ipaddr[64];
    /** 子网掩码 */
    char netmask[64];
    /** 网关地址 */
    char gateway[64];
    /** DNS地址 */
    char dns1[64];
    /** DNS地址2 */
    char dns2[64];
    /** MAC地址 */
    char mac[64];

} ipc_GetNetworkInfo_Resp;

typedef struct _ipc_ForceIFrame_Req {
    /** 通道ID;0为ipc本身*/
    int64_t ch;
} ipc_ForceIFrame_Req;

typedef struct _ipc_ForceIFrame_Resp {
    /** 操作返回码: 0, 正确; 1，失败*/
    char respCode;
} ipc_ForceIFrame_Resp;

typedef struct _ipc_SetLowPower_Req {
    /** 通道ID;0为ipc本身*/
    int64_t ch;
    /** 1, 开启低功耗模式; 0 关闭低功耗模式*/
    int64_t state;
} ipc_SetLowPower_Req;

typedef struct _ipc_SetLowPower_Resp {
    /** 操作返回码: 0, 正确; 1，失败*/
    char respCode;
} ipc_SetLowPower_Resp;

typedef struct _ipc_GetLowPower_Req {
    /** 通道ID;0为ipc本身*/
    int64_t ch;
} ipc_GetLowPower_Req;

typedef struct _ipc_GetLowPower_Resp {
    /** 1, 开启低功耗模式; 0 关闭低功耗模式*/
    int64_t state;
} ipc_GetLowPower_Resp;

typedef IOT_COMM_BYTES_ARRAY_T(5000) SSWPMapClean_Req_data_t;
typedef struct {
    /* 设备ID,用户不用关心该字段，sdk内部填写 */
    char did[65];
    /* 设备签名Key,用户不用关心该字段，sdk内部填写 */
    char did_signkey[65];
    /* map_id */
    int64_t map_id;
    /* 清扫记录数据 */
    SSWPMapClean_Req_data_t data;
    /* 总清扫次数 */
    int64_t total_count;
    /* 总清扫面积 单位: 平方米; 倍率: 10 */
    int64_t total_area;
    /* 总清扫时间, 单位: sec */
    int64_t total_time;
    /* 开始清扫时间: utc, 单位秒 */
    int64_t start_time;
    /* 本次清扫面积 单位: 平方米; 倍率: 10 */
    int64_t clean_area;
    /* 本次清扫时间，单位秒 */
    int64_t clean_time;
    /* 是否正常清扫完成: 0： 异常; 1: 正常 */
    int64_t clean_done;
    /* 清扫类型: 1(0), 全屋清扫; 2, 选区清扫; 3, 划区清扫 */
    int64_t clean_type;
    /* 清扫触发方式: 0: App触发; 1, 设备按键触发; 2, 定时任务触发 */
    int64_t clean_trigger;

} iot_SSWPMapCleanReport_t;

typedef IOT_COMM_BYTES_ARRAY_T(700000) SSWPMapAddByDevice_data_t;
typedef IOT_COMM_BYTES_ARRAY_T(500000) SSWPMapAddByDevice_dmap_t;
typedef struct {
    char did[65];
    char did_signkey[65];
    int64_t map_id;
    SSWPMapAddByDevice_data_t data;
    SSWPMapAddByDevice_dmap_t dmap;
    /* @@protoc_insertion_point(struct:SSWPMapAddByDevice_Req) */
} iot_SSWPMapAddByDevice_t;

typedef struct {
    /*通道号*/
    int             channel;
    /*事件id*/
    iot_event_id_t  id;
    /*事件的值*/
    int64_t         value;
    /*事件发送的时间ms,填0则表示当前时间（sdk自动追加）*/
    uint64_t        utc_ms;
    /*人脸识别id,没有该功能填NULL*/
    ipc_PeopleID_t *peopleID;
    /*没有该功能填NULL，坐标的JSON数组;obj_id数组个数x4, 目标坐标 4个数字,左下为(0,0)开始计数;
     *左下(x,y),右上(x,y); x,y 为像素编号; 一张图多个目标则为多个坐标*/
    char           *obj_ext;
    /*该事件是否需要带云存,true表示带有云存(图片或视频);false表示只产生一条消息.*/
    bool            isCS;
    /*附带bytes数组的值，一般都是填NULL*/
    uint8_t        *pbs;
    /*附带bytes数组的长度*/
    int             pbs_len;
    /*该事件是不是补传*/
    bool            isretry;
    /*该事件需不需要通知局域网其他设备*/
    bool            local;
} iot_event_opt_t;

typedef struct {
    /**  网络质量: 1-6; 0 不支持;数字越大网络质量越好; 只有在接入是无线的情况下表示； 1 无信号 */
    int netQos;
    /**  当前电量: 1-100，百分比; 0 不使用电池供电 */
    int batteryLevel;
    /**  0：电池供电; 139, 设备充电中; */
    int powerModel;
    /**  时间偏移量，单位分钟；如果与平台当前的时区不一致，则触发平台下发命令:60 */
    int timeOffset;
    /**  设备当前定时时间任务的个数. */
    int ttcmdCount;
} dev_base_info_t;


/* ipc : on_ipc_PirGet */
typedef bool (*on_ipc_PirGet_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PirGet_Req *req,
    /* output  */ ipc_PirGet_Resp *rsp);

/* ipc : on_ipc_PirSet */
typedef bool (*on_ipc_PirSet_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PirSet_Req *req,
    /* output  */ ipc_PirSet_Resp *rsp);

/* ipc : on_ipc_ChanState */
typedef bool (*on_ipc_ChanState_t)(
    /* conn_id */ int32_t                   conn_id,
    /* input   */ ipc_ChanState_Req         *req,
    /* output  */ ipc_ChanState_Resp        *rsp);

/* ipc : on_ipc_VideoChanChange */
typedef bool (*on_ipc_VideoChanChange_t)(
    /* conn_id */ int32_t                   conn_id,
    /* input   */ ipc_VideoChanChange_Req  *req,
    /* output  */ ipc_VideoChanChange_Resp *rsp);

/* ipc : on_ipc_Discovery */
typedef bool (*on_ipc_Discovery_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_Discovery_Req *req,
    /* output  */ ipc_Discovery_Resp *rsp);

/* ipc : on_ipc_WifiAPGet */
typedef bool (*on_ipc_WifiAPGet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_WifiAPGet_Req *req,
    /* output  */ ipc_WifiAPGet_Resp *rsp);

/* ipc : on_ipc_WifiSet */
typedef bool (*on_ipc_WifiSet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiSet_Req *req,
    /* output  */ ipc_WifiSet_Resp *rsp);

/* ipc : on_ipc_WifiGet */
typedef bool (*on_ipc_WifiGet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiGet_Req *req,
    /* output  */ ipc_WifiGet_Resp *rsp);

typedef bool (*on_ipc_OsdSet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_OsdSet_Req   *req,
    /* output  */ ipc_OsdSet_Resp  *rsp);

/* ipc : on_ipc_WifiGet */
typedef bool (*on_ipc_OsdGet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_OsdGet_Req   *req,
    /* output  */ ipc_OsdGet_Resp  *rsp);
/* ipc : on_ipc_VideoPlay */
typedef bool (*on_ipc_VideoPlay_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VideoPlay_Req *req,
    /* output  */ ipc_VideoPlay_Resp *rsp);

/* ipc : on_ipc_VideoPause */
typedef bool (*on_ipc_VideoPause_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_VideoPause_Req *req,
    /* output  */ ipc_VideoPause_Resp *rsp);

/* ipc : on_ipc_VideoQosSet */
typedef bool (*on_ipc_VideoQosSet_t)(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_VideoQosSet_Req *req,
    /* output  */ ipc_VideoQosSet_Resp *rsp);

/* ipc : on_ipc_FlipSet */
typedef bool (*on_ipc_FlipSet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipSet_Req *req,
    /* output  */ ipc_FlipSet_Resp *rsp);

/* ipc : on_ipc_AudioPlay */
typedef bool (*on_ipc_AudioPlay_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_AudioPlay_Req *req,
    /* output  */ ipc_AudioPlay_Resp *rsp);

/* ipc : on_ipc_AudioPause */
typedef bool (*on_ipc_AudioPause_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_AudioPause_Req *req,
    /* output  */ ipc_AudioPause_Resp *rsp);

/* ipc : on_ipc_TalkbackPlay */
typedef bool (*on_ipc_TalkbackPlay_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_TalkbackPlay_Req *req,
    /* output  */ ipc_TalkbackPlay_Resp *rsp);

/* ipc : on_ipc_TalkbackPause */
typedef bool (*on_ipc_TalkbackPause_t)(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_TalkbackPause_Req *req,
    /* output  */ ipc_TalkbackPause_Resp *rsp);

/* ipc : on_ipc_HistoryPlanSet */
typedef bool (*on_ipc_HistoryPlanSet_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanSet_Req *req,
    /* output  */ ipc_HistoryPlanSet_Resp *rsp);

/* ipc : on_ipc_HistoryPlanGet */
typedef bool (*on_ipc_HistoryPlanGet_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanGet_Req *req,
    /* output  */ ipc_HistoryPlanGet_Resp *rsp);

/* ipc : on_ipc_HistoryDays */
typedef bool (*on_ipc_HistoryDays_t)(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryDays_Req *req,
    /* output  */ ipc_HistoryDays_Resp *rsp);

/* ipc : on_ipc_HistoryDayList */
typedef bool (*on_ipc_HistoryDayList_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryDayList_Req *req,
    /* output  */ ipc_HistoryDayList_Resp *rsp);

/* ipc : on_ipc_HistoryPlay */
typedef bool (*on_ipc_HistoryPlay_t)(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryPlay_Req *req,
    /* output  */ ipc_HistoryPlay_Resp *rsp);

/* ipc : on_ipc_HistoryPause */
typedef bool (*on_ipc_HistoryPause_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_HistoryPause_Req *req,
    /* output  */ ipc_HistoryPause_Resp *rsp);

/* ipc : on_ipc_HistoryThumGet */
typedef bool (*on_ipc_HistoryThumGet_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryThumGet_Req *req,
    /* output  */ ipc_HistoryThumGet_Resp *rsp);

/* ipc : on_ipc_HistoryDel */
typedef bool (*on_ipc_HistoryDel_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_HistoryDel_Req *req,
    /* output  */ ipc_HistoryDel_Resp *rsp);

/* ipc : on_ipc_ConfigGet */
typedef bool (*on_ipc_ConfigGet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_ConfigGet_Req *req,
    /* output  */ ipc_ConfigGet_Resp *rsp);

/* ipc : on_ipc_TimeSet */
typedef bool (*on_ipc_TimeSet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeSet_Req *req,
    /* output  */ ipc_TimeSet_Resp *rsp);

/* ipc : on_ipc_TimeGet */
typedef bool (*on_ipc_TimeGet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeGet_Req *req,
    /* output  */ ipc_TimeGet_Resp *rsp);

/* ipc : on_ipc_LedSet */
typedef bool (*on_ipc_LedSet_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedSet_Req *req,
    /* output  */ ipc_LedSet_Resp *rsp);

/* ipc : on_ipc_LedGet */
typedef bool (*on_ipc_LedGet_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedGet_Req *req,
    /* output  */ ipc_LedGet_Resp *rsp);

typedef bool (*on_ipc_LedModeSet_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedModeSet_Req *req,
    /* output  */ ipc_LedModeSet_Resp *rsp);

typedef bool (*on_ipc_LedModeGet_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedModeGet_Req *req,
    /* output  */ ipc_LedModeGet_Resp *rsp);

typedef bool (*on_ipc_SetCS_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_SetCS_Req *req,
    /* output  */ ipc_SetCS_Resp *rsp);

/* ipc : on_ipc_IRCutSet */
typedef bool (*on_ipc_IRCutSet_t)(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutSet_Req *req,
    /* output  */ ipc_IRCutSet_Resp *rsp);

/* ipc : on_ipc_IRCutGet */
typedef bool (*on_ipc_IRCutGet_t)(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutGet_Req *req,
    /* output  */ ipc_IRCutGet_Resp *rsp);

/* ipc : on_ipc_SecretSet */
typedef bool (*on_ipc_SecretSet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretSet_Req *req,
    /* output  */ ipc_SecretSet_Resp *rsp);

/* ipc : on_ipc_SecretGet */
typedef bool (*on_ipc_SecretGet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretGet_Req *req,
    /* output  */ ipc_SecretGet_Resp *rsp);

/* ipc : on_ipc_NotifySet */
typedef bool (*on_ipc_NotifySet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifySet_Req *req,
    /* output  */ ipc_NotifySet_Resp *rsp);

/* ipc : on_ipc_NotifyGet */
typedef bool (*on_ipc_NotifyGet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifyGet_Req *req,
    /* output  */ ipc_NotifyGet_Resp *rsp);

/* ipc : on_ipc_MotionzoneSet */
typedef bool (*on_ipc_MotionzoneSet_t)(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneSet_Req *req,
    /* output  */ ipc_MotionzoneSet_Resp *rsp);

/* ipc : on_ipc_MotionzoneGet */
typedef bool (*on_ipc_MotionzoneGet_t)(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneGet_Req *req,
    /* output  */ ipc_MotionzoneGet_Resp *rsp);

/* ipc : on_ipc_PspAdd */
typedef bool (*on_ipc_PspAdd_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspAdd_Req *req,
    /* output  */ ipc_PspAdd_Resp *rsp);

/* ipc : on_ipc_PspDel */
typedef bool (*on_ipc_PspDel_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspDel_Req *req,
    /* output  */ ipc_PspDel_Resp *rsp);

/* ipc : on_ipc_PspList */
typedef bool (*on_ipc_PspList_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspList_Req *req,
    /* output  */ ipc_PspList_Resp *rsp);

/* ipc : on_ipc_PspCall */
typedef bool (*on_ipc_PspCall_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspCall_Req *req,
    /* output  */ ipc_PspCall_Resp *rsp);

/* ipc : on_ipc_Reboot */
typedef bool (*on_ipc_Reboot_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_Reboot_Req *req,
    /* output  */ ipc_Reboot_Resp *rsp);

/* ipc : on_ipc_Reset */
typedef bool (*on_ipc_Reset_t)(
    /* conn_id */ int32_t         conn_id,
    /* input   */ ipc_Reset_Req *req,
    /* output  */ ipc_Reset_Resp *rsp);

/* ipc : on_ipc_FlipGet */
typedef bool (*on_ipc_FlipGet_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipGet_Req *req,
    /* output  */ ipc_FlipGet_Resp *rsp);

/* ipc : on_ipc_LanAuth */
typedef bool (*on_ipc_LanAuth_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_LanAuth_Req *req,
    /* output  */ ipc_LanAuth_Resp *rsp);

/* ipc : on_ipc_TimedcruiseSet */
typedef bool (*on_ipc_TimedcruiseSet_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseSet_Req *req,
    /* output  */ ipc_TimedcruiseSet_Resp *rsp);

/* ipc : on_ipc_TimedcruiseGet */
typedef bool (*on_ipc_TimedcruiseGet_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseGet_Req *req,
    /* output  */ ipc_TimedcruiseGet_Resp *rsp);

/* ipc : on_ipc_StorageInfo */
typedef bool (*on_ipc_StorageInfo_t)(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_StorageInfo_Req *req,
    /* output  */ ipc_StorageInfo_Resp *rsp);

/* ipc : on_ipc_StorageFormat */
typedef bool (*on_ipc_StorageFormat_t)(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_StorageFormat_Req *req,
    /* output  */ ipc_StorageFormat_Resp *rsp);

/* ipc : on_ipc_StorageFormatRate */
typedef bool (*on_ipc_StorageFormatRate_t)(
    /* conn_id */ int32_t                     conn_id,
    /* input   */ ipc_StorageFormatRate_Req *req,
    /* output  */ ipc_StorageFormatRate_Resp *rsp);

/* ipc : on_ipc_LogSet */
typedef bool (*on_ipc_LogSet_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LogSet_Req *req,
    /* output  */ ipc_LogSet_Resp *rsp);

/* ipc : on_ipc_VolumeSet */
typedef bool (*on_ipc_VolumeSet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeSet_Req *req,
    /* output  */ ipc_VolumeSet_Resp *rsp);

/* ipc : on_ipc_PowerFreqSet */
typedef bool (*on_ipc_PowerFreqSet_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqSet_Req *req,
    /* output  */ ipc_PowerFreqSet_Resp *rsp);

/* ipc : on_ipc_PowerFreqGet */
typedef bool (*on_ipc_PowerFreqGet_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqGet_Req *req,
    /* output  */ ipc_PowerFreqGet_Resp *rsp);

/* ipc : on_ipc_VolumeGet */
typedef bool (*on_ipc_VolumeGet_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeGet_Req *req,
    /* output  */ ipc_VolumeGet_Resp *rsp);

/* ipc : on_ipc_AlarmSet */
typedef bool (*on_ipc_AlarmSet_t)(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmSet_Req *req,
    /* output  */ ipc_AlarmSet_Resp *rsp);

/* ipc : on_ipc_AlarmGet */
typedef bool (*on_ipc_AlarmGet_t)(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmGet_Req *req,
    /* output  */ ipc_AlarmGet_Resp *rsp);

/* ipc : on_ipc_Screenshot */
typedef bool (*on_ipc_Screenshot_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_Screenshot_Req *req,
    /* output  */ ipc_Screenshot_Resp *rsp);

/* ipc : on_ipc_PtzCtrl */
typedef bool (*on_ipc_PtzCtrl_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PtzCtrl_Req *req,
    /* output  */ ipc_PtzCtrl_Resp *rsp);

/* ipc : on_ipc_FirmwareNotify */
typedef bool (*on_ipc_FirmwareNotify_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_FirmwareNotify_Req *req,
    /* output  */ ipc_FirmwareNotify_Resp *rsp);

/* ipc : on_ipc_FirmwareRate */
typedef bool (*on_ipc_FirmwareRate_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_FirmwareRate_Req *req,
    /* output  */ ipc_FirmwareRate_Resp *rsp);

/* ipc : on_ipc_EventRecordGet */
typedef bool (*on_ipc_EventRecordGet_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordGet_Req *req,
    /* output  */ ipc_EventRecordGet_Resp *rsp);

/* ipc : on_ipc_EventRecordSet */
typedef bool (*on_ipc_EventRecordSet_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordSet_Req *req,
    /* output  */ ipc_EventRecordSet_Resp *rsp);

/* ipc : on_ipc_GetNetworkInfo */
typedef bool (*on_ipc_GetNetworkInfo_t)(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_GetNetworkInfo_Req *req,
    /* output  */ ipc_GetNetworkInfo_Resp *rsp);

/* ipc : on_ipc_FileStart */
typedef bool (*on_ipc_FileStart_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_FileStart_Req *req,
    /* output  */ ipc_FileStart_Resp *rsp);

/* ipc : on_ipc_FileStop */
typedef bool (*on_ipc_FileStop_t)(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_FileStop_Req *req,
    /* output  */ ipc_FileStop_Resp *rsp);

/* ipc : on_ipc_CustomCmd */
typedef bool (*on_ipc_CustomCmd_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_CustomCmd_Req *req,
    /* output  */ ipc_CustomCmd_Resp *rsp);

/* ipc : on_ipc_DirCreate */
typedef bool (*on_ipc_DirCreate_t)(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_DirCreate_Req *req,
    /* output  */ ipc_DirCreate_Resp *rsp);

/* ipc : on_ipc_DirDel */
typedef bool (*on_ipc_DirDel_t)(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_DirDel_Req *req,
    /* output  */ ipc_DirDel_Resp *rsp);

/* ipc : on_ipc_DirEdit */
typedef bool (*on_ipc_DirEdit_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_DirEdit_Req *req,
    /* output  */ ipc_DirEdit_Resp *rsp);

/* ipc : on_ipc_DirList */
typedef bool (*on_ipc_DirList_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_DirList_Req *req,
    /* output  */ ipc_DirList_Resp *rsp);

/* ipc : on_ipc_FileAdd */
typedef bool (*on_ipc_FileAdd_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FileAdd_Req *req,
    /* output  */ ipc_FileAdd_Resp *rsp);

/* ipc : on_ipc_FileDel */
typedef bool (*on_ipc_FileDel_t)(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FileDel_Req *req,
    /* output  */ ipc_FileDel_Resp *rsp);

/* ipc : on_ipc_FileEdit */
typedef bool (*on_ipc_FileEdit_t)(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_FileEdit_Req *req,
    /* output  */ ipc_FileEdit_Resp *rsp);

/* ipc : on_ipc_FileRecvRate */
typedef bool (*on_ipc_FileRecvRate_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_FileRecvRate_Req *req,
    /* output  */ ipc_FileRecvRate_Resp *rsp);

/* ipc : on_ipc_FileThumList */
typedef bool (*on_ipc_FileThumList_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_FileThumList_Req *req,
    /* output  */ ipc_FileThumList_Resp *rsp);

/* ipc : on_ipc_FileAttrSet */
typedef bool (*on_ipc_FileAttrSet_t)(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_FileAttrSet_Req *req,
    /* output  */ ipc_FileAttrSet_Resp *rsp);

/* ipc : on_ipc_ExecIOTCMD */
typedef bool (*on_ipc_ExecIOTCMD_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_ExecIOTCMD_Req *req,
    /* output  */ ipc_ExecIOTCMD_Resp *rsp);

/* ipc : on_ipc_ExecTimetask */
typedef bool (*on_ipc_ExecTimetask_t)(
    /* conn_id */ int32_t			     conn_id,
    /* input   */ ipc_ExecTimetask_Req  *req,
    /* output  */ ipc_ExecTimetask_Resp *rsp);

/* ipc : on_ipc_TamperSet */
typedef bool (*on_ipc_TamperSet_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_TamperSet_Req *req,
    /* output  */ ipc_TamperSet_Resp *rsp);

/* ipc : on_ipc_SetAutoTrack */
typedef bool (*on_ipc_SetAutoTrack_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_SetAutoTrack_Req  *req,
    /* output  */ ipc_SetAutoTrack_Resp *rsp);

/* ipc : on_ipc_GetAutoTrack */
typedef bool (*on_ipc_GetAutoTrack_t)(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_GetAutoTrack_Req  *req,
    /* output  */ ipc_GetAutoTrack_Resp *rsp);

/* ipc : on_ipc_ForceIFrame */
typedef bool (*on_ipc_ForceIFrame_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_ForceIFrame_Req  *req,
    /* output  */ ipc_ForceIFrame_Resp *rsp);

/* ipc : on_ipc_SetLowPower */
typedef bool (*on_ipc_SetLowPower_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_SetLowPower_Req  *req,
    /* output  */ ipc_SetLowPower_Resp *rsp);

/* ipc : on_ipc_GetLowPower */
typedef bool (*on_ipc_GetLowPower_t)(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_GetLowPower_Req  *req,
    /* output  */ ipc_GetLowPower_Resp *rsp);


/* iot on ipc cmd */
typedef struct {
    on_ipc_Discovery_t         ipc_Discovery;
    on_ipc_WifiAPGet_t         ipc_WifiAPGet;
    on_ipc_WifiSet_t           ipc_WifiSet;
    on_ipc_WifiGet_t           ipc_WifiGet;
    on_ipc_OsdSet_t            ipc_OsdSet;
    on_ipc_OsdGet_t            ipc_OsdGet;
    on_ipc_VideoPlay_t         ipc_VideoPlay;
    on_ipc_VideoPause_t        ipc_VideoPause;
    on_ipc_VideoQosSet_t       ipc_VideoQosSet;
    on_ipc_FlipSet_t           ipc_FlipSet;
    on_ipc_AudioPlay_t         ipc_AudioPlay;
    on_ipc_AudioPause_t        ipc_AudioPause;
    on_ipc_TalkbackPlay_t      ipc_TalkbackPlay;
    on_ipc_TalkbackPause_t     ipc_TalkbackPause;
    on_ipc_HistoryPlanSet_t    ipc_HistoryPlanSet;
    on_ipc_HistoryPlanGet_t    ipc_HistoryPlanGet;
    on_ipc_HistoryDays_t       ipc_HistoryDays;
    on_ipc_HistoryDayList_t    ipc_HistoryDayList;
    on_ipc_HistoryPlay_t       ipc_HistoryPlay;
    on_ipc_HistoryPause_t      ipc_HistoryPause;
    on_ipc_HistoryThumGet_t    ipc_HistoryThumGet;
    on_ipc_HistoryDel_t        ipc_HistoryDel;
    on_ipc_ConfigGet_t         ipc_ConfigGet;
    on_ipc_TimeSet_t           ipc_TimeSet;
    on_ipc_TimeGet_t           ipc_TimeGet;
    on_ipc_LedSet_t            ipc_LedSet;
    on_ipc_LedGet_t            ipc_LedGet;
    on_ipc_LedModeSet_t        ipc_LedModeSet;
    on_ipc_LedModeGet_t        ipc_LedModeGet;
    on_ipc_SetCS_t             ipc_SetCS;
    on_ipc_IRCutSet_t          ipc_IRCutSet;
    on_ipc_IRCutGet_t          ipc_IRCutGet;
    on_ipc_SecretSet_t         ipc_SecretSet;
    on_ipc_SecretGet_t         ipc_SecretGet;
    on_ipc_NotifySet_t         ipc_NotifySet;
    on_ipc_NotifyGet_t         ipc_NotifyGet;
    on_ipc_MotionzoneSet_t     ipc_MotionzoneSet;
    on_ipc_MotionzoneGet_t     ipc_MotionzoneGet;
    on_ipc_PspAdd_t            ipc_PspAdd;
    on_ipc_PspDel_t            ipc_PspDel;
    on_ipc_PspList_t           ipc_PspList;
    on_ipc_PspCall_t           ipc_PspCall;
    on_ipc_Reboot_t            ipc_Reboot;
    on_ipc_Reset_t             ipc_Reset;
    on_ipc_FlipGet_t           ipc_FlipGet;
    on_ipc_LanAuth_t           ipc_LanAuth;
    on_ipc_TimedcruiseSet_t    ipc_TimedcruiseSet;
    on_ipc_TimedcruiseGet_t    ipc_TimedcruiseGet;
    on_ipc_StorageInfo_t       ipc_StorageInfo;
    on_ipc_StorageFormat_t     ipc_StorageFormat;
    on_ipc_StorageFormatRate_t ipc_StorageFormatRate;
    on_ipc_LogSet_t            ipc_LogSet;
    on_ipc_VolumeSet_t         ipc_VolumeSet;
    on_ipc_PowerFreqSet_t      ipc_PowerFreqSet;
    on_ipc_PowerFreqGet_t      ipc_PowerFreqGet;
    on_ipc_VolumeGet_t         ipc_VolumeGet;
    on_ipc_AlarmSet_t          ipc_AlarmSet;
    on_ipc_AlarmGet_t          ipc_AlarmGet;
    on_ipc_Screenshot_t        ipc_Screenshot;
    on_ipc_PtzCtrl_t           ipc_PtzCtrl;
    on_ipc_PirSet_t            ipc_PirSet;
    on_ipc_PirGet_t            ipc_PirGet;
    on_ipc_ChanState_t         ipc_ChanState;
    on_ipc_VideoChanChange_t   ipc_VideoChanChange;
    on_ipc_FirmwareNotify_t    ipc_FirmwareNotify;
    on_ipc_FirmwareRate_t      ipc_FirmwareRate;
    on_ipc_EventRecordGet_t    ipc_EventRecordGet;
    on_ipc_EventRecordSet_t    ipc_EventRecordSet;
    on_ipc_GetNetworkInfo_t    ipc_GetNetworkInfo;
    on_ipc_FileStart_t         ipc_FileStart;
    on_ipc_FileStop_t          ipc_FileStop;
    on_ipc_CustomCmd_t         ipc_CustomCmd;
    on_ipc_DirCreate_t         ipc_DirCreate;
    on_ipc_DirDel_t            ipc_DirDel;
    on_ipc_DirEdit_t           ipc_DirEdit;
    on_ipc_DirList_t           ipc_DirList;
    on_ipc_FileAdd_t           ipc_FileAdd;
    on_ipc_FileDel_t           ipc_FileDel;
    on_ipc_FileEdit_t          ipc_FileEdit;
    on_ipc_FileRecvRate_t      ipc_FileRecvRate;
    on_ipc_FileAttrSet_t       ipc_FileAttrSet;
    on_ipc_FileThumList_t      ipc_FileThumList;
    on_ipc_ExecIOTCMD_t        ipc_ExecIOTCMD;
    on_ipc_ExecTimetask_t      ipc_ExecTimetask;
    on_ipc_TamperSet_t         ipc_TamperSet;
    on_ipc_SetAutoTrack_t      ipc_SetAutoTrack;
    on_ipc_GetAutoTrack_t      ipc_GetAutoTrack;
    on_ipc_ForceIFrame_t       ipc_ForceIFrame;
    on_ipc_SetLowPower_t       ipc_SetLowPower;
    on_ipc_GetLowPower_t       ipc_GetLowPower;
} iot_on_ipc_cmd_t;

/* iot on platform iotCmd */
typedef void (*iot_on_platform_iotcmd_t)(iot_PpiotCmd_t *cmd);

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
 * @} (end addtogroup xciot_cmd_ipc)
 */
/****************************************************************************/
#endif /* XCIOT_CMD_IPC_H_INCLUDE */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
