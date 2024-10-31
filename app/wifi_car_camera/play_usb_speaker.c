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
    u32 rlen = 0;
    running++;
    if (!state) {
        memset(buf, 0, len);
        goto __exit;
    }
#if 1
    if (b.len == 0 || b.baddr == 0) {
        b.noblock = 1;
        b.index = bindex;
        dev_ioctl(dev, AUDIOC_DQBUF, (u32)&b);
        /* printf("baddr %x, blen %d\n", b.baddr, b.len); */
        if (b.len == 0 || b.baddr == 0) {
            putchar('#');
            buf_offset = 0;
            memset(buf, 0, len);
            goto __exit;
        }
    }
    if (spk_ch == 1) {
        rlen = len > b.len - buf_offset ? b.len - buf_offset : len;
        memcpy(buf, (u8 *)(b.baddr + buf_offset), rlen);
        if (len > rlen) {
            memset(buf + rlen, 0, len - rlen);
        }
    } else if (spk_ch == 2) {
        s16 *dst_buf = (s16 *)buf;
        s16 *src_buf = (s16 *)(b.baddr + buf_offset);
        rlen = len / 2 > b.len - buf_offset ? b.len - buf_offset : len / 2;
        for (int i = rlen / 2 - 1; i >= 0; i--) {
            dst_buf[2 * i] = src_buf[i];
            dst_buf[2 * i + 1] = src_buf[i];
        }
        if (len > 2 * rlen) {
            memset(buf + 2 * rlen, 0, len - 2 * rlen);
        }
    }
    buf_offset += rlen;
    if (buf_offset >= b.len) {
        buf_offset = 0;
        dev_ioctl(dev, AUDIOC_QBUF, (u32)&b);
        b.len = 0;
    }
#else
    const s16 sin_48k[] = {
        0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
        14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
        14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
        0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
        -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
        -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
    };
    if (spk_ch == 1) {
        rlen = len > sizeof(sin_48k) ? sizeof(sin_48k) : len;
        memcpy(buf, sin_48k, rlen);
        if (len > rlen) {
            memset(buf + rlen, 0, len - rlen);
        }
    } else if (spk_ch == 2) {
        s16 *dst_buf = (s16 *)buf;
        s16 *src_buf = (s16 *)sin_48k;
        rlen = len / 2 > sizeof(sin_48k) ? sizeof(sin_48k) : len / 2;
        for (int i = rlen / 2 - 1; i >= 0; i--) {
            dst_buf[2 * i] = src_buf[i];
            dst_buf[2 * i + 1] = src_buf[i];
        }
        if (len > rlen * 2) {
            memset(buf + 2 * rlen, 0, len - 2 * rlen);
        }
    }
#endif

__exit:
    running--;
    return len;
}

int play_usb_speaker_start()
{
    int ret = 0;
    struct host_speaker_attr spk_attr = {0};
    struct host_speaker_ops spk_ops = {0};
    struct audio_format f = {0};
    struct video_reqbufs breq = {0};

    if (state) {
        return 0;
    }
    dev = dev_open("audio", (void *)AUDIO_TYPE_ENC);
    if (!dev) {
        ret = -1;
        goto __exit_fail;
    }
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
    buf_offset = 0;
    bindex = 0;
    memset(&b, 0, sizeof(struct video_buffer));

    f.format = "pcm";
    f.channel = 1;
    f.sample_rate = spk_attr.sr;
    f.bitwidth = spk_attr.bitwidth;
    //f.kbps = f.channel * f.sample_rate * (f.bitwidth / 8) / 1000 * 8;
    f.volume = 100;
    f.sample_source = "mic";
    f.frame_len = f.channel * f.sample_rate * (f.bitwidth / 8) / 100;
    ret = dev_ioctl(dev, AUDIOC_SET_FMT, (u32)&f);

    audio_buf = malloc(USB_AUDIO_BUF_SIZE);
    if (!audio_buf) {
        ret = -2;
        goto __exit_fail;
    }
    breq.buf = audio_buf;
    breq.size = USB_AUDIO_BUF_SIZE;
    ret = dev_ioctl(dev, AUDIOC_REQBUFS, (u32)&breq);
    if (ret) {
        ret = -3;
        goto __exit_fail;
    }

    ret = dev_ioctl(dev, AUDIOC_STREAM_ON, (u32)&bindex);
    if (ret) {
        ret = -4;
        goto __exit_fail;
    }
    ret = usb_host_speaker_open();
    if (ret) {
        ret = -5;
        goto __exit_fail;
    }

    state = 1;
    return 0;

__exit_fail:
    usb_host_speaker_close();
    if (dev) {
        dev_ioctl(dev, AUDIOC_STREAM_OFF, (u32)bindex);
        dev_close(dev);
        dev = NULL;
    }
    if (audio_buf) {
        free(audio_buf);
        audio_buf = NULL;
    }
    return ret;
}

int play_usb_speaker_stop()
{
    int ret;
    struct host_speaker_ops spk_ops = {0};
    u32 ot = jiffies + msecs_to_jiffies(100);

    if (!state) {
        return 0;
    }
    while (running) {
        if (time_after(jiffies, ot)) {
            printf("wait usb speaker exit running timeout!\n");
            return -1;
        }
    }
    state = 0;

    spk_ops.get_stream_data = NULL;
    usb_host_speaker_set_ops(&spk_ops);
    usb_host_speaker_close();
    dev_ioctl(dev, AUDIOC_STREAM_OFF, (u32)bindex);
    if (b.len && b.len) {
        dev_ioctl(dev, AUDIOC_QBUF, (u32)&b);
        b.baddr = 0;
        b.len = 0;
    }
    dev_close(dev);
    dev = NULL;
    free(audio_buf);
    audio_buf = NULL;
    return 0;
}


