#include "system/includes.h"
#include "usb_host_user.h"

static struct device *dev;
static u8 bindex;
static u8 *audio_buf;
static u8 state;
static u8 spk_ch;
static u32 buf_offset;
static struct video_buffer b;
static u8 running;

#define USB_AUDIO_BUF_SIZE          (9600)

static u32 usb_speaker_get_stream_data(u8 *buf, u32 len)
{
    int ret;
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

    memset(buf, 0, len);
    ret = user_dec_get_data(tmp_data2, tmp_data2_len);
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
        pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, buf, len);
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


    memset(buf, 0, len);
    ret = user_dec_get_data(tmp_data2, tmp_data2_len);
//    printf("\n>>>>>ret = %d\n",ret);
    if (ret) {
        s16 *tmp_buf = buf; //指向单声道数据缓存区
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

#if (CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK == 48000)
    static u8 *tmp_data1 = NULL;
    static int tmp_data1_len;
    if (!tmp_data1) {
        tmp_data1_len = 3 * DAC_DEC_LEN;
        tmp_data1 = calloc(1, tmp_data1_len);
    }
    memset(buf, 0, len);
    user_dec_get_data(tmp_data1, tmp_data1_len);
    pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, buf, len);
#else
    extern int user_dec_get_data(u8 * data, int len);
    memset(buf, 0, len);
    user_dec_get_data(buf, len);
#endif

#endif

    return len;
}

int play_usb_speaker_start()
{
    int ret = 0;
    struct host_speaker_attr spk_attr = {0};
    struct host_speaker_ops spk_ops = {0};

    usb_host_speaker_get_attr(&spk_attr);
    printf("get usb speaker attribute: %d, %d, %d, %d, %d, %d\n", spk_attr.vol_l, spk_attr.vol_r, spk_attr.ch, spk_attr.bitwidth, spk_attr.sr, spk_attr.mute);
    spk_attr.mute = 0;
    spk_attr.vol_l = 100;
    spk_attr.vol_r = 100;
    /* spk_attr.sr = 48000; */
    usb_host_speaker_set_attr(&spk_attr);
    printf("set usb speaker attribute: %d, %d, %d, %d, %d, %d\n", spk_attr.vol_l, spk_attr.vol_r, spk_attr.ch, spk_attr.bitwidth, spk_attr.sr, spk_attr.mute);
    spk_ops.get_stream_data = usb_speaker_get_stream_data;
    usb_host_speaker_set_ops(&spk_ops);

    spk_ch = spk_attr.ch;

    ret = usb_host_speaker_open();
    if (ret) {
        ret = -1;
        goto __exit;
    }

    state = 1;
    return 0;

__exit:
    usb_host_speaker_close();

    return ret;
}

int play_usb_speaker_stop()
{
    int ret;
    struct host_speaker_ops spk_ops = {0};
    spk_ops.get_stream_data = NULL;
    usb_host_speaker_set_ops(&spk_ops);
    usb_host_speaker_close();

    return 0;
}


