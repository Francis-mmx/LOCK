#ifndef AUDIO_SERVER_H
#define AUDIO_SERVER_H

#include "fs/fs.h"


enum {
    AUDIO_SERVER_EVENT_CURR_TIME = 0x20,
    AUDIO_SERVER_EVENT_END,
    AUDIO_SERVER_EVENT_ERR,
    AUDIO_SERVER_EVENT_SPEAK_START,
    AUDIO_SERVER_EVENT_SPEAK_STOP,
};

#define AUDIO_REQ_DEC                  0
#define AUDIO_REQ_ENC                  1
#define AUDIO_REQ_GET_INFO             2

#define AUDIO_DEC_OPEN                 0
#define AUDIO_DEC_START                1
#define AUDIO_DEC_PAUSE                2
#define AUDIO_DEC_STOP                 3
#define AUDIO_DEC_FF                   4
#define AUDIO_DEC_FR                   5
#define AUDIO_DEC_GET_BREAKPOINT       6
#define AUDIO_DEC_PP                   7
#define AUDIO_DEC_SET_VOLUME           8


#define AUDIO_ENC_OPEN      4
#define AUDIO_ENC_START     5
#define AUDIO_ENC_PAUSE     6
#define AUDIO_ENC_STOP      7
#define AUDIO_ENC_CLOSE     8
#define AUDIO_ENC_SET_VOLUME           9

struct audio_dec_breakpoint {
    int len;
    u32 fptr;
    u8 data[2048];	//ape格式断点最大2036字节
};

struct audio_finfo {
    u8 channel;
    u8 name_code; /* 0:ansi, 1:unicode_le, 2:unicode_be*/
    int sample_rate;
    int bit_rate;
    int total_time;
};


struct audio_vfs_ops {
    void *(*fopen)(const char *path, const char *mode);
    int (*fread)(void *file, void *buf, u32 len);
    int (*fwrite)(void *file, void *buf, u32 len);
    int (*fseek)(void *file, u32 offset, int seek_mode);
    int (*ftell)(void *file);
    int (*flen)(void *file);
    int (*fclose)(void *file);
};


struct audio_dec_output_ops {
    void *(*get_buf)(void *priv, u32 *len);
    void (*put_buf)(void *priv, void *buf, u32 len);
    const struct audio_vfs_ops  *vfs_ops;
};

struct audio_decoder_ops {
    const char *name;
    void *(*open)(FILE *file, void *priv,
                  const struct audio_dec_output_ops *ops, struct audio_dec_breakpoint *);
    void *(*try_open)(FILE *file, void *priv,
                      const struct audio_dec_output_ops *ops, struct audio_dec_breakpoint *);
    int (*get_audio_info)(void *, struct audio_finfo *info);
    int (*get_play_time)(void *);
    int (*fast_forward)(void *, int step_s);
    int (*fast_rewind)(void *, int step_s);
    int (*get_breakpoint)(void *, struct audio_dec_breakpoint *);
    int (*start)(void *);
    int (*close)(void *);
};



struct audio_dec_req {
    u8 cmd;
    u8 status;
    u8 channel;
    u8 volume;
    u8 priority;
    u32 output_buf_len;
    u32 sample_rate;
    u32 ff_fr_step;
    u32 total_time;
    void *output_buf;
    FILE *file;
    char *dec_type;
    const char *sample_source;
    struct audio_dec_breakpoint *bp;
    const struct audio_vfs_ops *vfs_ops;
};


struct audio_enc_req;

struct audio_package_ops {
    const char *name;
    const char *data_fmt;
    void *(*open)(void *file, const struct audio_vfs_ops *, struct audio_enc_req *);
    int (*start)(void *);
    int (*input)(void *, u8 *data, int len);
    int (*close)(void *);
};

struct audio_enc_req {
    u8 cmd;
    u8 channel;
    u8 volume;
    u8 priority;
    u8 use_vad;
    u8 vad_auto_refresh;
    u16 vad_start_threshold;	//ms
    u16 vad_stop_threshold;		//ms
    u16 kbps;
    u16 frame_size;
    u32 output_buf_len;
    u32 sample_rate;
    u32 msec;
    FILE *file;
    u8 *output_buf;
    u16 frame_head_reserve_len;
    const char *format;         //编码格式
    const char *sample_source;
    const struct audio_vfs_ops *vfs_ops;
    u32(*read_input)(u8 *buf, u32 len);
};

union audio_req {
    struct audio_dec_req dec;
    struct audio_finfo info;
    struct audio_enc_req enc;
};


#define REGISTER_AUDIO_DECODER(ops) \
        const struct audio_decoder_ops ops sec(.audio_decoder)


extern const struct audio_decoder_ops audio_decoder_begin[];
extern const struct audio_decoder_ops audio_decoder_end[];

#define list_for_each_audio_decoder(p) \
    for (p = audio_decoder_begin; p < audio_decoder_end; p++)



#define REGISTER_AUDIO_PACKAGE(ops) \
        const struct audio_package_ops ops sec(.audio_package)

extern const struct audio_package_ops audio_package_begin[];
extern const struct audio_package_ops audio_package_end[];

#define list_for_each_audio_package(p) \
    for (p = audio_package_begin; p < audio_package_end; p++)








#endif
