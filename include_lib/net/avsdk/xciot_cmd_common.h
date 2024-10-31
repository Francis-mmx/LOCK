/*****************************************************************************
 * xciot_cmd_common.h
 *
 *
 *
 ****************************************************************************/
/*
 * wiki
 * ====
 */
#ifndef XCIOT_CMD_COMMON_H_INCLUDE
#define XCIOT_CMD_COMMON_H_INCLUDE
/*****************************************************************************
 * @addtogroup xciot_cmd_common
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
#define IOT_COMM_BYTES_ARRAY_T(n)                                          \
    struct {                                                                 \
        uint32_t size;                                                       \
        uint8_t  bytes[n];                                                   \
    }

/*****************************************************************************
 * Public Types
 ****************************************************************************/
typedef struct _xciot_Timeinfo {
    /* 开始时间，单位秒；相对于每天零点的时间差值 */
    int32_t start_sec;
    /* end_sec >= start_sec; 如果小于 start_sec 表示隔天 */
    int32_t end_sec;
} iot_Timeinfo_t;

typedef struct _xciot_Timetask {
    /* day的有效个数 */
    uint32_t       days_count;
    /* 日期构成，表示只在特定的日期触发; 与week_day互斥： YYYYMMDD=20190101 */
    int32_t        days[31];
    /* week_day的有效个数 */
    uint32_t       week_day_count;
    /* 0-6: 0 星期天 1 星期一....; */
    int32_t        week_day[7];
    /* time的个数 */
    uint32_t       time_count;
    /* 动作的时间 */
    iot_Timeinfo_t time[10];
    /* 是否使能 */
    int32_t        enable;
} xciot_Timetask;

typedef IOT_COMM_BYTES_ARRAY_T(1500) iot_BaseCmd_pbs_t;
typedef struct _iot_BaseCmd {
    /* 命令id */
    int32_t             cid;
    /* int的个数 */
    int32_t             pint_count;
    /* 命令的int64参数 */
    int64_t             pint[15];
    /* string的个数 */
    int32_t             pstr_count;
    /* 命令的string参数 */
    char                pstr[8][128];
    /* 命令的bytes数组参数 */
    iot_BaseCmd_pbs_t   pbs;
} iot_BaseCmd_t;

typedef struct _iot_PpiotCmd {
    /* 命令类型： 0 请求； 1 执行应答; */
    int32_t       t;
    /* 设备ID */
    char          did[25];
    /* 子设备ID */
    char          sdid[25];
    /* cmds的个数 */
    int32_t       cmds_count;
    /* 需要执行的命令组 */
    iot_BaseCmd_t cmds[1];
    /* 命令序列号 */
    int32_t       seq;
    /* 命令时间戳(ms) */
    int64_t       timestamp;
    /* 命令是否需要回复: true; 需要回复 */
    bool          ans;
} iot_PpiotCmd_t;

typedef struct _iot_TimetaskCmd {
    /* 定时任务唯一编号，由平台分配 */
    int32_t        ttid;
    /* days数组中有效个数 */
    int32_t        days_count;
    /* 日期构成，表示只在特定的日期触发; 与week_day互斥： YYYYMMDD=20190101 */
    int32_t        days[31];
    /* week_day数组中有效个数 */
    int32_t        week_day_count;
    /* 0-6: 0 星期天 1 星期一....; 如果为空表示单次任务 */
    int32_t        week_day[7];
    /* 开始时间，单位秒；相对于每天零点的时间差值；start_sec = end_sec = 0 表示全天 */
    int32_t        start_sec;
    /* end_sec >= start_sec; 如果小于 start_sec 表示隔天 */
    int32_t        end_sec;
    /* 定时需要执行的命令 */
    iot_PpiotCmd_t ttcmd;
    /* 是否启用 */
    bool           enable;
    /* 间隔时间, 单位秒；
     * interval_sec > 0; 表示在 start_sec<=>end_sec时间周期内，间隔interval_sec开始执行 ttcmd;
     * interval_sec ==0; 不重复start_sec<=>end_sec时间周期 */
    int32_t        interval_sec;
} iot_TimetaskCmd_t;

/**
 * @brief gps info.
 */
typedef struct {
    /* 纬度: 纬度每隔0.00001度，距离相差约1.1米 （-180 到 +180）;纬度 = 真实纬度 * 100000 取整的值 */
    int32_t   longitude;
    /* 经度: 经度每隔0.00001度，距离相差约1米（-90 到 +90）;经度 = 真实经度 * 100000 取整的值 */
    int32_t   latitude;
    /* 速度(m/s);速度 = 真实速度 * 10 取整的值 */
    int32_t   speed;
    /* 方向范围0-360，整形，以正北为0度，顺时针增加;如果值为166600，表示不支持方向功能。(保留小数点后面2位，存放数据扩大100倍变为整数) */
    int32_t   angle;
    /* 加速度: 0 不支持，或者没有急加/减速; 1 急加速; 2 急减速 */
    int32_t   acceleration;
    /* UTC时间戳，单位ms */
    uint64_t  utcms;

} gps_info_t;

typedef struct {
    /* 个数 */
    uint16_t count;

    /* gps数据信息,mtu最大1400/24 */
    gps_info_t gps[50];
} gps_t;

typedef IOT_COMM_BYTES_ARRAY_T(1200) discovery_skill_t;
typedef struct {
    char did[65];
    char ipaddr[20];
    char netmask[20];
    int32_t lis_port;
    char sdk_ver[65];
    uint32_t mode;
    char lsign[65];
    discovery_skill_t skill;
} discovery_info_t;

typedef struct {
    int count;
    discovery_info_t info[20];
} discovery_t;

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
 * @} (end addtogroup xciot_cmd_common)
 */
/****************************************************************************/
#endif /* XCIOT_CMD_COMMON_H_INCLUDE */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
