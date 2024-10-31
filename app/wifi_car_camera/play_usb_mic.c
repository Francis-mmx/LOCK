#include "system/includes.h"
#include "usb_host_user.h"

static struct device *dev;
static cbuffer_t cbuf;
static u8 bindex;
static u8 *cbuf_space;
static u8 state;
static u8 restart;
static u8 running;

#define CBUFFER_SIZE            (2 * 1024)
#define CBUFFER_START_LEVEL     (CBUFFER_SIZE / 2)

static void usb_mic_recv_handler(u8 *buf, u32 len)
{
    u32 wlen;
    running++;
    if (!state) {
        running--;
        return;
    }
    wlen = cbuf_write(&cbuf, buf, len);
    if (wlen != len) {
        putchar('+');
    }
    running--;
}

static void usb_mic_dec_irq_handler(void *priv, void *data, int len)
{
    u32 rlen;
    static u32 ot;
    running++;
    if (!state) {
        running--;
        return;
    }
    if (restart) {
        if (cbuf_get_data_size(&cbuf) < CBUFFER_START_LEVEL) {
            putchar('-');
            memset(data, 0, len);
            if (time_after(jiffies, ot)) {
                usb_host_force_reset_module();
            }
            running--;
            return;
        } else {
            restart = 0;
        }
    }
    rlen = cbuf_read(&cbuf, data, len);
    if (rlen != len) {
        restart = 1;
        memset(data, 0, len);
        ot = jiffies + msecs_to_jiffies(1000);
    }
    running--;
}

int play_usb_mic_start()
{
    int ret = 0;
    struct audio_format f = {0};
    struct host_mic_attr mic_attr = {0};
    struct host_mic_ops mic_ops = {0};
    u32 arg[2];

    if (state) {
        return 0;
    }
    bindex = 0xff;
    restart = 0;
    cbuf_space = malloc(CBUFFER_SIZE);
    if (!cbuf_space) {
        return -1;
    }
    cbuf_init(&cbuf, cbuf_space, CBUFFER_SIZE);
    dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
    if (!dev) {
        ret = -2;
        goto __exit_fail;
    }
    usb_host_mic_get_attr(&mic_attr);
    printf("get usb mic attribute: %d %d %d %d %d\n", mic_attr.vol, mic_attr.ch, mic_attr.bitwidth, mic_attr.sr, mic_attr.mute);
    mic_attr.mute = 0;
    mic_attr.vol = 100;
    usb_host_mic_set_attr(&mic_attr);
    printf("set usb mic attribute: %d %d %d %d %d\n", mic_attr.vol, mic_attr.ch, mic_attr.bitwidth, mic_attr.sr, mic_attr.mute);
    mic_ops.recv_handler = usb_mic_recv_handler;
    usb_host_mic_set_ops(&mic_ops);
    f.volume = 100;
    f.channel = mic_attr.ch;
    f.bitwidth = mic_attr.bitwidth;
    f.sample_rate = mic_attr.sr;
    f.priority = 10;
    dev_ioctl(dev, AUDIOC_SET_FMT, (u32)&f);

    arg[0] = 0;
    arg[1] = (u32)usb_mic_dec_irq_handler;
    dev_ioctl(dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);

    ret = usb_host_mic_open();
    if (ret < 0) {
        ret = -3;
        goto __exit_fail;
    }

    dev_ioctl(dev, AUDIOC_STREAM_ON, (u32)&bindex);
    state = 1;

    return 0;

__exit_fail:
    usb_host_mic_close();
    if (dev) {
        dev_ioctl(dev, AUDIOC_STREAM_OFF, (u32)bindex);
        dev_close(dev);
        dev = NULL;
    }
    if (cbuf_space) {
        free(cbuf_space);
        cbuf_space = NULL;
    }
    return ret;
}

int play_usb_mic_stop()
{
    int ret = 0;
    struct host_mic_ops mic_ops = {0};
    u32 ot = jiffies + msecs_to_jiffies(100);

    if (!state) {
        return 0;
    }
    while (running) {
        if (time_after(jiffies, ot)) {
            printf("wait usb mic exit running timeout!\n");
            return -1;
        }
    }
    state = 0;

    dev_ioctl(dev, AUDIOC_STREAM_OFF, (u32)bindex);
    dev_close(dev);
    dev = NULL;
    mic_ops.recv_handler = NULL;
    usb_host_mic_set_ops(&mic_ops);
    usb_host_mic_close();
    free(cbuf_space);
    cbuf_space = NULL;
    return 0;
}
