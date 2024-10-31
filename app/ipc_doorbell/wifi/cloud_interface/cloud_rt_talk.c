#include "system/includes.h"
#include "generic/ioctl.h"
#include "circular_buf.h"
#include "audio_dev.h"
#include "database.h"
#include "gpio.h"
#include "doorbell_event.h"

extern int net_video_rec_get_audio_rate();

#define RT_TALK_BUFFER_SIZE     64 * 1024
static cbuffer_t __cbuffer;
static u8 cloud_rt_talk_init_flag = 0;
static void *audio_dev;
static  u8 *buffer;





#include "server/audio_server.h"
static struct server *audio = NULL;
OS_SEM  r_sem;
#define INT_MAX 0x7FFFFFFF

static void audio_server_event_handler(void *priv, int argc, int *argv)
{
    printf("argc:%d agrv[0]:0x%x\n", argc, argv[0]);
    union audio_req r = {0};
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_END:
        if (audio) {
            r.dec.cmd = AUDIO_DEC_STOP;
            server_request(audio, AUDIO_REQ_DEC, &r);
            server_close(audio);
            audio = NULL;
            /* kvoice_disable = 0; */
        }
        break;
    case AUDIO_SERVER_EVENT_ERR:
        break;

    }
}
static int stream_fread(void *file, void *buf, u32 len)
{
    int ret;
//    printf("\n >>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
__again:
    if (cbuf_get_data_size(&__cbuffer) < len)  {
        ret = os_sem_pend(&r_sem, 200);
        if (ret != OS_NO_ERR) {
            doorbell_event_dac_mute(0);
            os_sem_pend(&r_sem, 0);
//            printf("\n >>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        }
        goto __again;
    }
//  printf("\n >>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    return cbuf_read(&__cbuffer, buf, len);
}

static int stream_fseek(void *file, u32 offset, int seek_mode)
{
    return 0;
}

static int stream_flen(void *file)
{
    return INT_MAX;
}

static const struct audio_vfs_ops stream_vfs = {
    .fread = stream_fread,
    .fseek = stream_fseek,
    .flen = stream_flen,
};


void aac_decode_init(void)
{
    int ret;
    union audio_req r = {0};

    audio = server_open("audio_server", "dec");
    if (!audio) {
        return;
    }
    r.dec.cmd             = AUDIO_DEC_OPEN;
    r.dec.volume          = net_video_rec_get_dac_volume();//20;
    r.dec.output_buf      = NULL;
    r.dec.output_buf_len  = 1024 * 4;
    r.dec.file            = NULL;
    r.dec.dec_type        = "m4a";
    r.dec.sample_rate     = 0;//必须写零，由文件指定,否则不会解码文件信息
    r.dec.priority        = 0;
    r.dec.sample_source = "user";//"iis0";
    r.dec.vfs_ops      = &stream_vfs;
    ret = server_request(audio, AUDIO_REQ_DEC, &r);
    printf("ret:%d line:%d\n", ret, __LINE__);
    printf("total_time= %d \n", r.dec.total_time); //获取文件播放长度，可以用于ui显示
    printf("sample_rate= %d \n", r.dec.sample_rate); //获取文件采样率，可以用于ui显示

    r.dec.cmd = AUDIO_DEC_START;
    ret = server_request(audio, AUDIO_REQ_DEC, &r);

    server_register_event_handler(audio, NULL, audio_server_event_handler);//播放结束回调函数
}
int pcm_change_sampleRate(int insr, int outsr, u8  *in, u32 in_len, u8  *out, u32 out_cap)
{

    int rate = insr / outsr;
    int out_len = in_len / rate;

    //48k采样率，降到16k,则rate=3; 48转8有问题
    for (int i = 0, j = 0; i < in_len; i += 2 * rate, j += 2) {
        out[j] = in[i];
        out[j + 1] = in[i + 1];
    }

    return (out_len);
}

static int audio_dec_irq_handler(void *priv, void *data, int len)
{

    static int DAC_DEC_LEN;
    if (!DAC_DEC_LEN) {
        DAC_DEC_LEN = len;
    }
#if (CONFIG_AUDIO_FORMAT_FOR_SPEARK == AUDIO_FORMAT_AAC)

#if (CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK == 48000)
    /**
        dac 中断每次取512字节
        AAC 解码出来的声音是双声道的
        app过来的声音是48k的
        512 对应的原始双声道的数据长度是 512 * 3 * 2，对应的单声道数据是512 * 3

    */
    static u8 *tmp_data1 = NULL;
    static u8 *tmp_data2 = NULL;
    static int tmp_data1_len;
    static int tmp_data2_len;
    if (!tmp_data1) {
        tmp_data1_len = 3 * DAC_DEC_LEN;
        tmp_data1 = calloc(1, tmp_data1_len);
    }
    if (!tmp_data2) {
        tmp_data2_len = 3 * DAC_DEC_LEN * 2;
        tmp_data2 = calloc(1, tmp_data2_len);
    }

    memset(data, 0, len);
    int ret = user_dec_get_data(tmp_data2, tmp_data2_len);
    if (ret) {
        s16 *tmp_buf = (s16 *)tmp_data1; //指向单声道数据缓存区
        s16 *tmp = (s16 *)tmp_data2; //指向双声道数据缓存区
        /**
            AAC解码出来的PCM是双声道的,需要转为单声道
            / 4 是因为双变单 / 2,s16  处理一次两个字节 / 2 所以一次是除以4
        */
        for (int i = 0; i < (tmp_data2_len / 4); i++) {
            tmp_buf[i] = tmp[i * 2];
        }
        pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, data, len);
#else

    /**
        dac 中断每次取512字节
        AAC 解码出来的声音是双声道的
        app过来的声音是16k的
        512 对应的原始双声道的数据长度是 512 * 3 * 2，对应的单声道数据是512 * 3

    */


    static u8 *tmp_data2 = NULL;
    static int tmp_data2_len;

    if (!tmp_data2) {
        tmp_data2_len =  DAC_DEC_LEN * 2;
        tmp_data2 = calloc(1, tmp_data2_len);
    }


    memset(data, 0, len);
    int ret = user_dec_get_data(tmp_data2, tmp_data2_len);
    if (ret) {
        s16 *tmp_buf = data; //指向单声道数据缓存区
        s16 *tmp = (s16 *)tmp_data2; //指向双声道数据缓存区
        /**
            AAC解码出来的PCM是双声道的,需要转为单声道
            / 4 是因为双变单 / 2,s16  处理一次两个字节 / 2 所以一次是除以4
        */
        for (int i = 0; i < (tmp_data2_len / 4); i++) {
            tmp_buf[i] = tmp[i * 2];
        }
#endif // CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK
    }
#else

    cbuffer_t *cbuffer = (cbuffer_t *)priv;
#if (CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK == 48000)
    static u8 *tmp_data1 = NULL;
    static int tmp_data1_len;
    if (!tmp_data1) {
        tmp_data1_len = 3 * DAC_DEC_LEN;
        tmp_data1 = calloc(1, tmp_data1_len);
    }
    memset(data, 0, len);
    cbuf_read(cbuffer, tmp_data1, tmp_data1_len);
    pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, data, len);
#else
    cbuf_read(cbuffer, data, len);
#endif

#endif

    return len;
}

void pcm_decode_init(void)
{

    struct audio_format f = {0};
    audio_dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
    f.volume        = net_video_rec_get_dac_volume();
    f.channel       = 1;
    f.sample_rate   = net_video_rec_get_audio_rate();
    f.priority      = 0;
    f.format = "pcm";
    f.frame_len     = 1024;
#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
    f.sample_source     = "dac";
#else
    f.sample_source     = "user";
#endif
    int err = dev_ioctl(audio_dev, AUDIOC_SET_FMT, (u32)&f);

    u32 arg[2];
    arg[0] = (u32)&__cbuffer;
    arg[1] = (u32)audio_dec_irq_handler;
    dev_ioctl(audio_dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);
    int bindex = 0;
    dev_ioctl(audio_dev, AUDIOC_STREAM_ON, (u32)&bindex);

}


int cloud_rt_talk_write(char *data, int len)
{
#if (CONFIG_AUDIO_FORMAT_FOR_SPEARK == AUDIO_FORMAT_AAC)

    if (cloud_rt_talk_init_flag) {
        doorbell_event_dac_mute(1);
        len = cbuf_write(&__cbuffer, data, len);
        if (len == 0) {
            putchar('B');
            cbuf_clear(&__cbuffer);
        } else {
            if (!os_sem_query(&r_sem)) {
                os_sem_post(&r_sem);
            }
        }

        if (!audio && cbuf_get_data_size(&__cbuffer) >= 512) {
#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
            if (!audio_dev) {
                pcm_decode_init();
            }
#endif
            extern void aac_decode_init(void);
            aac_decode_init();
        }
    }
#else

    if (cloud_rt_talk_init_flag) {
        if (!audio_dev) {
            pcm_decode_init();
        }
        doorbell_event_dac_mute(1);
        len = cbuf_write(&__cbuffer, data, len);
        if (len == 0) {
            putchar('B');
            cbuf_clear(&__cbuffer);
        }
    }
#endif // CONFIG_AUDIO_FORMAT_FOR_SPEARK
    return len;
}

void cloud_rt_talk_task(void *priv)
{
    int res;
    int msg[32];
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case RT_AUDIO_CTRL_START:
                    break;
                case RT_AUDIO_CTRL_WRITE:
                    cloud_rt_talk_write((void *)msg[2], msg[3]);
                    free((void *)msg[2]);
                    break;
                case RT_AUDIO_CTRL_STOP:
                    break;
                }
            }
        }
    }
}

int cloud_rt_talk_init(void)
{

    if (cloud_rt_talk_init_flag) {
        cbuf_clear(&__cbuffer);
        return -1;
    }
    buffer  = calloc(1, RT_TALK_BUFFER_SIZE);
    if (!buffer) {
        return -1;
    }

    os_sem_create(&r_sem, 0);
    cbuf_init(&__cbuffer, buffer, RT_TALK_BUFFER_SIZE);
    cloud_rt_talk_init_flag = 1;
    thread_fork("cloud_rt_talk_task", 16, 0x2E00, 64, 0, cloud_rt_talk_task, NULL);
    return 0;

}
//late_initcall(cloud_rt_talk_init);

