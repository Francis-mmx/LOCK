/*****************************************************************************
 * iotsdk_xspp.h
 *
 * Copyright (C) 2021 lwj lwjjhy@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 ****************************************************************************/
#ifndef IOTSDK_XSPP_H_INCLUDE
#define IOTSDK_XSPP_H_INCLUDE
/*****************************************************************************
 * @addtogroup iotsdk_xspp.h
 * @{
 */
/*****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
//#include <stdbool.h>
//#include <typedef.h> //bool
#include <string.h>

/*****************************************************************************
 * Configure Definitions
 ****************************************************************************/
#define UART_BUFF_MAX     2048

#define IOTSDK_COMM_INI      1
#define IOTSDK_COMM_CFG      0
#define IOTSDK_COMM_DUMP     0
#define IOTSDK_COMM_UID      0

/* 是否使用iotsdk */
#define IOTSDK_USE_SDK       1
/*****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#if IOTSDK_COMM_UID >0
/* 用户实现，获取设备唯一识别码, 读取成功返回uid地址，失败返回NULL */
extern char *get_uid(char uid[32]);
#endif

enum xspp_errno_enum {
    E_XSPP_SUCCESS      = 0,
    E_XSPP_ENOMEM       = 2,
    E_XSPP_EINVAL       = 3,
    E_XSPP_EBUSY        = 4,
    E_XSPP_ETIMEDOUT    = 5,
    E_XSPP_ENOTSUP      = 9,
    E_XSPP_EPROTO       = 13,
    E_XSPP_CRC          = 14,
};

/* payload type */
typedef enum {
    E_DATA_TYPE_SET_INI = 0,
    E_DATA_TYPE_GET_INI = 1,
    E_DATA_TYPE_SET_CFG = 2,
    E_DATA_TYPE_GET_CFG = 3,
    E_DATA_TYPE_RAW     = 4,
    E_DATA_TYPE_GET_UID = 5, //获取设备唯一识别码

    /* add above here */
    E_DATA_TYPE_LAST
} data_type_t;

/*****************************************************************************
 * Public Types
 ****************************************************************************/
#if IOTSDK_USE_SDK  > 0
#if 0
#include "iotsdk_type.h"
#else
#include "xciot_api.h"
typedef iot_BaseCmd_t iot_BaseCmd;
typedef iot_PpiotCmd_t iot_PpiotCmd;
typedef iot_PpiotCmd_t iot_PpiotCmd;
#endif
#else
/* 是否支持float */
#define IOTSDK_ENABLE_FLOAT 0
/* 是否支持string */
#define IOTSDK_ENABLE_STRING 1
/* 是否支持byte arr */
#define IOTSDK_ENABLE_BYTE_ARR 1

/** @brief DeviceId 最大长度,PB限制不能调整 */
#define IOT_DEVICE_ID_MAXLEN  (32+1)

/** @brief DeviceKey 最大长度,PB限制不能调整 */
#define IOT_DEVICE_KEY_MAXLEN (32+1)

/** @brief signKey 最大长度 */
#define IOT_DEVICE_SIGNKEY_MAXLEN (45)

/* @brief 未定义的string最大长度 */
#define DEVICE_STRING_MAXLEN  (20)

/**
 * @brief IOT Cmd Define 基本数据类型
 * +------------|--------------+-----------+
 * |   值类型   |	支持个数   | 最大长度  |
 * +------------|--------------+-----------+
 * |   int64	|     12       |           |
 * +------------|--------------+-----------+
 * |   string	|     8        | 43  Bytes |
 * +------------|--------------+-----------+
 * |   bytes    |     1        | 321 Bytes |
 * +------------|--------------+-----------+
 */

enum {
    IOT_YCMD_INT64_MAX_COUNT   = (12),  /**< int64 类型个数 */
    IOT_YCMD_FLOAT_MAX_COUNT   = (12),  /**< float 类型个数 */
    IOT_YCMD_STRING_MAX_COUNT  = (8),   /**< string类型个数 */
    IOT_YCMD_BYTES_MAX_COUNT   = (1),   /**< bytes 类型个数  */

    IOT_YCMD_STRING_MAX_LENGTH = (43),  /**< string类型最大长度 */
    IOT_YCMD_BYTES_MAX_LENGTH  = (321), /**< bytes 类型最大长度 */
};



/* int64 类型 */
typedef int64_t iotYCmdInt64_t;

/* int64 数组类型 */
typedef struct {
    uint16_t size;
    uint64_t  data[IOT_YCMD_INT64_MAX_COUNT];
} iotYCmdInt64Array_t;

/* float 类型 */
typedef float iotYCmdFloat_t;

/* float 数组类型 */
typedef struct {
    uint16_t    size;
    float       data[IOT_YCMD_FLOAT_MAX_COUNT];
} iotYCmdFloatArray_t;


/* string 类型 */
typedef char    iotYCmdString_t[IOT_YCMD_STRING_MAX_LENGTH];

/* string 数组类型 */
typedef struct {
    uint16_t         size;
    iotYCmdString_t  data[IOT_YCMD_STRING_MAX_COUNT];
} iotYCmdStringArray_t;


/* bytes 类型 */
typedef struct {
    uint16_t size;
    uint8_t  bytes[IOT_YCMD_BYTES_MAX_LENGTH];
} iotYCmdBytes_t;

/** @brief IOT YCmd Id */
typedef int32_t iotYCmdId_t;

#if IOTSDK_ENABLE_BYTE_ARR > 0
#define BYTES_ARRAY_T(n) struct { uint16_t size; uint8_t bytes[n]; }
/* Struct definitions */
typedef BYTES_ARRAY_T(IOT_YCMD_BYTES_MAX_LENGTH) pbs_t;
#endif


typedef struct {

    /* 命令id */
    int32_t cid;

    /* int数组有效数据个数 */
    uint16_t pint_count;
    /* int数组 */
    int64_t pint[IOT_YCMD_INT64_MAX_COUNT];

#if IOTSDK_ENABLE_FLOAT > 0
    /* float数组有效数据个数 */
    uint16_t pfloat_count;
    /* float数组 */
    float pfloat[IOT_YCMD_FLOAT_MAX_COUNT];
#endif

#if IOTSDK_ENABLE_STRING > 0
    /* 字符串数组有效数据个数 */
    uint16_t pstr_count;
    /* 字符串数组 */
    char pstr[IOT_YCMD_STRING_MAX_COUNT][IOT_YCMD_STRING_MAX_LENGTH];
#endif

#if IOTSDK_ENABLE_BYTE_ARR > 0
    /* byte数组，@pbs_t */
    pbs_t pbs;
#endif

} iot_BaseCmd;


typedef struct {

    /* 命令类型:0 请求,1 执行应答 */
    int32_t t;
    /* 设备配置文件的did,iotsdk_cfg_get_did()获取 */
    char did[IOT_DEVICE_ID_MAXLEN];
    /* 子设备id，没有填空 */
    char sdid[IOT_DEVICE_KEY_MAXLEN];
    /* 结构体cmds的个数，仅支持1 */
    uint16_t cmds_count;
    /* 命令结构体@iot_BaseCmd */
    iot_BaseCmd cmds[1];
    /* 命令序列号，累加即可 */
    int32_t seq;
    /* 命令utc时间戳,单位ms */
    int64_t timestamp;
    /* 是否应答，默认false */
    bool ans;

} iot_PpiotCmd;

typedef iot_BaseCmd iot_BaseCmd_t;
typedef iot_PpiotCmd iot_PpiotCmd_t;
#endif

/*****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/**
 * @brief 获取毫秒级utc时间
 *
 *
 * @return  返回毫秒级utc时间.
 */
typedef uint64_t (*get_utc)(void);

/**
 * @brief 串口发送函数回调函数
 *
 * @param[in]    data    - 需要发送的数据
 *
 * @param[in]    len     - 发送的长度
 *
 * @param[in]    ud      - 用户自定义数据
 *
 * @return  返回0成功，否则失败.
 */
typedef int (*serial_send)(uint8_t *data, uint16_t len, void *ud);

/**
 * @brief 用户数据处理回调函数
 *
 * @param[in]    cmd    - 接收到的cmd
 *
 * @param[in]    ud     - 用户自定义数据
 *
 * @return  返回0成功，否则失败.
 */
typedef int (*recv_handle)(iot_PpiotCmd *cmd, void *ud);

/**
 * @brief 初始化通用串行协议
 *
 * @param[in]    send         - 串口底层发送函数
 *
 * @param[in]    get_time     - 获取毫秒级utc函数，必须实现
 *
 * @param[in]    handle       - 接收到串口数据解析后，用户命令处理函数
 *
 * @param[in]    ud           - 用户自定义数据
 *
 * @return  返回0成功，否则失败.
 */
int xspp_init(serial_send        send,
              get_utc            get_time,
              recv_handle        handle,
              void              *ud);

/**
 * @brief 按照串口协议序列化cmd，并通过cb发送出去
 *
 * @param[in]    cmd         - 需要发送的命令
 *
 * @param[in]    timeout     - 超时时间ms,推荐1000 填0则不阻塞等待结果(对结果不关心)
 *
 * @return  返回0成功;-5,超时; 14CRC校验错误
 */
int xspp_tx_data(iot_PpiotCmd *cmd, uint64_t timeout);


/**
 * @brief 序列化串口协议，仅支持载荷类型为2（XCPB）
 *
 * @param[in]    rxbuffer  - 传入的命令结构体
 *
 * @param[in]    rxlen     - 序列化后的数据
 *
 * @return  返回0成功，否则失败.
 */
int xspp_rx_data(const uint8_t *rxbuffer,
                 uint32_t       rxlen);

/**
 * @brief 打印ppiotcmd结构体
 *
 * @param[in]    prefix   - domian
 *
 * @param[in]    pCmd     - 打印的数据
 *
 * @return 无.
 */
void dump_ppiotcmd(const char *prefix, iot_PpiotCmd *pCmd);



#if 1 // 用户需要通过串口烧写did.ini才需要实现xspp_on_ini
/**
 * @brief 用户收到平台did.ini数据，处理函数
 *
 * @param[in]  ini    - 连接平台的did.ini信息，一般需要永久保存在flash中。
 *
 * @param[in]  len    - ini的数据长度
 *
 * @return  返回0成功，否则失败.
 */
typedef int (*write_ini)(uint8_t *ini, uint16_t len);

/**
 * @brief 获取设备的did.ini文件
 *
 * @param[out]   ini    - 用户保存的did.ini信息
 *
 * @param[out]   len    - ini数据长度
 *
 * @return  返回0成功，否则失败.
 */
typedef int (*read_ini)(uint8_t *ini, uint16_t *len);

/**
 * @brief 用户需要通过串口烧写did.ini才需要实现xspp_on_ini
 *
 * @param[in]    write    - 当串口收到did.ini数据时，回调write
 *
 * @param[in]    read     - 读取设备保存的did.ini文件
 *
 * @return  返回0成功，否则失败.
 */
int xspp_on_ini(write_ini write, read_ini read);

#endif

#if 1
/**
 * @brief 发送扩展数据，一般用户不会使用该接口
 *
 * @param[in]    type    - 数据类型
 *
 * @param[in]    data    - xxx.ini读出来的数据
 *
 * @param[in]    len     - 读取xxx.ini的数据长度
 *
 * @param[in]    timeout - 超时时间ms,推荐1000 填0则不阻塞等待结果(对结果不关心)
 *
 * @return  返回0成功，否则失败.
 */
int xspp_tx_payload(data_type_t  type,
                    void        *data,
                    uint16_t     len,
                    uint64_t     timeout);
#endif

/****************************************************************************/
#ifdef __cplusplus
}
#endif
/****************************************************************************/
/*****************************************************************************
 * @} (end addtogroup iotcmd_api)
 */
/****************************************************************************/
#endif /* IOTSDK_XSPP_H_INCLUDE */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/


