#ifndef AUDIO_DEV_H
#define AUDIO_DEV_H

#include "generic/list.h"
#include "typedef.h"
#include "device/device.h"


#define AUDIOC_QUERYCAP         _IOR('A', 0, sizeof(struct audio_capability))
#define AUDIOC_GET_FMT          _IOR('A', 1, sizeof(struct audio_format))
#define AUDIOC_SET_FMT          _IOW('A', 1, sizeof(struct audio_format))
#define AUDIOC_REQBUFS          _IOR('A', 2, sizeof(int))
#define AUDIOC_DQBUF            _IOR('A', 3, sizeof(int))
#define AUDIOC_QBUF             _IOR('A', 4, sizeof(int))
#define AUDIOC_STREAM_ON        _IOR('A', 5, sizeof(int))
#define AUDIOC_STREAM_OFF       _IOR('A', 6, sizeof(int))
//#define AUDIOC_PLAY             _IOR('A', 7, sizeof(int))



#define AUD_SUBDEV_REQ_GET_PCM_DATA      0
#define AUD_SUBDEV_REQ_PUT_PCM_DATA      0


#define    AUDIO_TYPE_DEC                  0x01
#define    AUDIO_TYPE_ENC                  0x02
#define    AUDIO_TYPE_ENC_USER             0x04

struct audio_req_data {
    u8 channel;
    u8 wait_en;
    u8 *data;
    int len;
    int sample_rate;
    u8 data_byte;//数据字节宽度
};

struct audio_endpoint;

struct audio_platform_data {
    u8 type;
    void *private_data;
};



#define AUDIO_CAP_SAMPLING      0x00000001
#define AUDIO_CAP_MP3_ENC       0x00000002

struct audio_capability {
    u32 capabilities;
};


#define AUDIO_FMT_PCM          0x01
#define AUDIO_FMT_SPEEX        0x02
#define AUDIO_FMT_AMR          0x03
#define AUDIO_FMT_AAC          0x04
#define AUDIO_FMT_OPUS         0x05

#define AUDIO_FMT_VAD          0x0f

struct audio_format {
    u8 volume;
    u8 channel;
    u8 kbps;
    u8 priority;
    u8 use_vad;
    u8 vad_auto_refresh;
    u8 bitwidth;
    volatile u8 vad_status;
    u16 vad_start_threshold;
    u16 vad_stop_threshold;
    u16 frame_len;
    u16 frame_head_reserve_len;
    int sample_rate;
    const char *format;
    const char *sample_source;
    u32(*read_input)(u8 *buf, u32 len);
};


struct audio_subdevice_ops {
    int (*init)(struct audio_platform_data *);

    int (*querycap)(struct audio_capability *cap);

    int (*get_format)(struct audio_format *);
    int (*set_format)(struct audio_format *);

    struct audio_endpoint *(*open)(struct audio_format *);

    int (*streamon)(struct audio_endpoint *);

    int (*streamoff)(struct audio_endpoint *);

    int (*response)(struct audio_endpoint *, int cmd, void *);

    int (*write)(struct audio_endpoint *, void *buf, u32 len);

    int (*close)(struct audio_endpoint *);

    int (*ioctl)(struct audio_endpoint *, u32 cmd, u32 arg);
};

struct audio_subdevice {
    u8 id;
    /*u8 inused;*/
    u8 type;
    u32 format;
    /*struct audio_subdevice *next;*/
    /*void *parent;*/
    /*void *private_data;*/
    const struct audio_subdevice_ops *ops;
};


struct audio_endpoint {
    struct list_head entry;
    struct audio_subdevice *dev;
    int inused;
    void *parent;
    void *private_data;
};


int audio_subdevice_request(struct audio_endpoint *ep, int req, void *arg);

void *audio_buf_malloc(struct audio_endpoint *ep, u32 size);


void *audio_buf_realloc(struct audio_endpoint *ep, void *buf, int size);


void audio_buf_stream_finish(struct audio_endpoint *ep, void *buf);

void audio_buf_free(struct audio_endpoint *ep, void *buf);

void audio_buf_set_time(struct audio_endpoint *ep, void *buf, u32 msec);


extern struct audio_subdevice audio_subdev_begin[];
extern struct audio_subdevice audio_subdev_end[];


#define REGISTER_AUDIO_SUBDEVICE(dev, _id) \
	const struct audio_subdevice dev sec(.audio_subdev.##_id) = { \
		.id = _id, \



extern const struct device_operations audio_dev_ops;

typedef void (*audio_output_handler_t)(s16 *data, int len, int sample);
#define REGISTER_AUDIO_OUTPUT_DATA_HANDLER(fn) \
	const audio_output_handler_t __audio_output_##fn sec(.audio_output_handler) = fn
#define do_audio_output_data_handler(data, len, sample) \
	do { \
		audio_output_handler_t *handler; \
		extern audio_output_handler_t audio_output_handler_begin[];\
        extern audio_output_handler_t audio_output_handler_end[]; \
		for (handler = audio_output_handler_begin; handler < audio_output_handler_end; \
                handler++) { \
			(*handler)(data, len, sample);	\
		} \
	}while(0)


typedef void (*audio_input_handler_t)(s16 *data, int len, int sample, s16 **out_data);
#define REGISTER_AUDIO_INPUT_DATA_HANDLER(fn) \
	const audio_input_handler_t __audio_intput_##fn sec(.audio_input_handler) = fn

#define do_audio_input_data_handler(data, len, sample, out_data) \
	do { \
		audio_input_handler_t *handler; \
		extern audio_input_handler_t audio_input_handler_begin[];\
        extern audio_input_handler_t audio_input_handler_end[]; \
		for (handler = audio_input_handler_begin; handler < audio_input_handler_end; \
                handler++) { \
			(*handler)(data, len, sample,out_data);	\
		} \
	}while(0)


#endif
