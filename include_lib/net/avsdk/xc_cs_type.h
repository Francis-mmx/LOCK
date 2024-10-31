/*****************************************************************************
* xc_cs_type.h
*
* Copyright (C) 2019 %author% %author_email%
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
****************************************************************************/
#ifndef XC_CS_TYPE_H_INCLUDE
#define XC_CS_TYPE_H_INCLUDE
/*****************************************************************************
* Included Files
****************************************************************************/
// #include <stdio.h>
#include <string.h>
#include <stdint.h>
//#include <stdbool.h>
#include "typedef.h"
/*****************************************************************************
* Pre-processor Definitions
****************************************************************************/
typedef void *cs_hdl_ref_t;
/*****************************************************************************
* Public Types
****************************************************************************/
/**
 * @brief Log level.
 */
#define LOG_LEVEL_ALWAY 0
#define LOG_LEVEL_NONE  1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_INFO  4
#define LOG_LEVEL_DEBUG 5
#define LOG_LEVEL_TRACE 6

typedef enum {
    e_cs_encrypt_none        = 0,
    e_cs_encrypt_aes_128_cbc = 1,
    e_cs_encrypt_aes_192_cbc = 2,
    e_cs_encrypt_aes_256_cbc = 3,
    e_cs_encrypt_aes_128_cfb = 4,
    e_cs_encrypt_aes_192_cfb = 5,
    e_cs_encrypt_aes_256_cfb = 6,
    e_cs_encrypt_aes_128_ecb = 7,
    e_cs_encrypt_aes_192_ecb = 8,
    e_cs_encrypt_aes_256_ecb = 9,
    e_cs_encrypt_aes_128_ofb = 10,
    e_cs_encrypt_aes_192_ofb = 11,
    e_cs_encrypt_aes_256_ofb = 12,
    e_cs_encrypt_aes_128_ctr = 13,
    e_cs_encrypt_aes_192_ctr = 14,
    e_cs_encrypt_aes_256_ctr = 15,
} e_cs_encrypt_type_t;

// stream type define
typedef enum {
    e_cs_stream_invalid = 0,
    e_cs_stream_h264    = 1,
    e_cs_stream_h265    = 2,
    e_cs_stream_mpeg    = 3,
    e_cs_stream_mjpeg   = 4,

    e_cs_stream_g711a = 21,
    e_cs_stream_ulaw  = 31,
    e_cs_stream_g711u = 41,
    e_cs_stream_opus  = 51,
    e_cs_stream_adpcm = 61,
    e_cs_stream_g721  = 71,
    e_cs_stream_g723  = 81,
    e_cs_stream_g726  = 91,
    e_cs_stream_aac   = 101,
    e_cs_stream_speex = 111,
    e_cs_stream_pcm   = 121,
} e_cs_stream_type_t;

typedef enum {
    e_cs_frame_none   = 0x0,
    e_cs_frame_face   = 0x01,
    e_cs_frame_people = 0x02,
    e_cs_frame_car    = 0x04,
    e_cs_frame_animal = 0x08,
    e_cs_frame_cry    = 0x10,
    e_cs_frame_laugh  = 0x20,
} e_cs_frame_features_t;

typedef enum {
    e_cs_sound_frequeency_8k    = 1,
    e_cs_sound_frequeency_11025 = 2,
    e_cs_sound_frequeency_22050 = 3,
    e_cs_sound_frequeency_32000 = 4,
    e_cs_sound_frequeency_44100 = 5,
    e_cs_sound_frequeency_16000 = 6,
} e_cs_sound_frequency_t;

typedef enum {
    e_cs_sound_depth_8bit  = 1,
    e_cs_sound_depth_16bit = 2,

} e_cs_sound_depth_t;

typedef enum {
    e_cs_sound_channel_mono = 1,
    e_cs_sound_depth_stereo = 2,

} e_cs_sound_channel_t;

typedef enum {
    /* 缩略图jpg */
    e_cs_picture_type_thumb = 1,
    /* 高清jpg图片 */
    e_cs_picture_type_jpg = 2,

} e_cs_picture_type_t;

struct coord {
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

typedef enum {
    /* 未知参数 */
    e_cs_upload_platform_unkown = 98,
    /* 上传到阿里云 */
    e_cs_upload_platform_oss = 1,
    /* 上传到亚马逊 */
    e_cs_upload_platform_aws = 2,
    /* 上传到私有服务器 */
    e_cs_upload_platform_private = 3,

} cs_upload_platform_t;

typedef enum {
    /*SDK 未初始化，或者错误*/
    e_cs_upload_way_unkown = 0,
    /* 先存文件，再方式上传 */
    e_cs_upload_way_file = 1,
    /* 数据流方式上传 */
    e_cs_upload_way_memory = 2,

} cs_upload_way_t;

typedef struct {
    /* 当前云存事件触发时间 */
    int64_t event_time_ms;
    /* 文件大小 */
    int64_t file_size;
    /* 缩略图有没坐标,多通道只填ch0 */
    int picture_length;
    /* 录像文件总时长ms,多通道只填ch0 */
    int total_timems;
    /*图片的坐标*/
    struct coord xy;
} cs_statistic_t;

typedef struct {
    int              event_id;
    int64_t          start_timems;
    uint8_t          channel;
    uint32_t         memsize;
    cs_upload_way_t  upload_way;
    char            *file_name;
} cs_start_opt_t;

/**
 * @brief private send func，发送成功返回0.
 */
typedef int (*private_send_t)(uint64_t fileid,
                              uint32_t offset,
                              void    *payload,
                              uint32_t len);
/*****************************************************************************
* Public Data
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
/****************************************************************************/
#endif /* XC_CS_TYPE_H_INCLUDE */
/****************************************************************************/
/* */
/* End of file. */
/* */
/****************************************************************************/
/****************************************************************************/
