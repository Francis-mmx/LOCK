#include "system/includes.h"
#include "usb_host_user.h"

static struct device *dev;
static cbuffer_t cbuf;
static u8 bindex;
static u8 *cbuf_space;
static u8 state;
static u8 restart;
static u8 running;



extern int net_video_rec_get_audio_rate();
static void usb_mic_recv_handler(u8 *buf, u32 len)
{
    //只允许单通道的mic
    static int w_addr;
    static char tmpbuf[1024];

    if (w_addr + len <= 1024) {
        memcpy(tmpbuf + w_addr, buf, len);
        w_addr += len;
    }
    if (w_addr > 512) {
        static char data[512];
        memcpy(data, tmpbuf, 512);
        extern void usb_mic_data_input(u8 * buf, u32 len);
        usb_mic_data_input(data, 512);

        memcpy(data, tmpbuf + 512, w_addr - 512);
        memcpy(tmpbuf, data, w_addr - 512);
        w_addr = w_addr - 512;
    } else if (w_addr == 512) {
        extern void usb_mic_data_input(u8 * buf, u32 len);
        usb_mic_data_input(tmpbuf, 512);
        w_addr = 0;
    }
}



int play_usb_mic_start()
{
    int ret = 0;

    struct host_mic_attr mic_attr = {0};
    struct host_mic_ops mic_ops = {0};


    if (state) {
        return 0;
    }
    state = 1;

    cbuf_space = malloc(net_video_rec_get_audio_rate() * 2);
    if (!cbuf_space) {
        return -1;
    }

    void audio_enc_init(void);
    audio_enc_init();

    cbuf_init(&cbuf, cbuf_space, net_video_rec_get_audio_rate() * 2);
    usb_host_mic_get_attr(&mic_attr);
    printf("get usb mic attribute: %d %d %d %d %d\n", mic_attr.vol, mic_attr.ch, mic_attr.bitwidth, mic_attr.sr, mic_attr.mute);
    mic_attr.mute = 0;
    mic_attr.vol = 100;
    usb_host_mic_set_attr(&mic_attr);
    printf("set usb mic attribute: %d %d %d %d %d\n", mic_attr.vol, mic_attr.ch, mic_attr.bitwidth, mic_attr.sr, mic_attr.mute);
    mic_ops.recv_handler = usb_mic_recv_handler;
    usb_host_mic_set_ops(&mic_ops);

    ret = usb_host_mic_open();
    if (ret < 0) {
        ret = -1;
        goto __exit;
    }
    running = 1;
    return 0;

__exit:
    usb_host_mic_close();
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

    state = 0;

    printf("<<<< audio_enc_uninit\n");
    void audio_enc_uninit(void);
    audio_enc_uninit();

    mic_ops.recv_handler = NULL;
    usb_host_mic_set_ops(&mic_ops);
    usb_host_mic_close();
    cbuf_clear(&cbuf);
    free(cbuf_space);
    cbuf_space = NULL;
    return 0;
}


#define _STDIO_H_
#include "xciot_api.h"
#include "rt_stream_pkg.h"
#include "server/audio_server.h"

static u8 channel_bindex = 0xff;
static u8 mic_knife = 0;

static void *audio_fd = NULL;
extern uint64_t get_utc_ms(void);
void usb_mic_data_input(u8 *buf, u32 len)
{
    user_enc_input_data(buf, len);
}



void mic_task(void *priv)
{
#if 0
    int res;
    int msg[32];

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case 0:
                    if (get_avsdk_init_flag() == 2) {
                        avsdk_write_audio(E_IOT_MEDIASTREAM_TYPE_PCM, 0, get_utc_ms(), NULL, msg[2], msg[3]);
                        cloud_storage_audio_write(PCM_TYPE_AUDIO, timer_get_ms(), msg[2], msg[3]);
                    }

                    free(msg[2]);
                    break;
                }
                break;
            }
            break;
        }
    }
#else

    struct video_buffer b = {0};;
    int err;


    b.timeout = 100;
    b.noblock = 0;
    b.index = channel_bindex;

    mic_knife = 0;

    while (!mic_knife) {//i<=100
        if (audio_fd) {
            err = dev_ioctl(audio_fd, AUDIOC_DQBUF, (unsigned int)&b);
            if (err) {
                printf("audio  timeout\n");
                continue;
            }
            if (b.len) {
//                printf("b.baddr:0x%x b.len:%d\n", b.baddr, b.len);
                if (get_avsdk_init_flag() == 2) {
                    avsdk_write_audio(E_IOT_MEDIASTREAM_TYPE_PCM, 0, get_utc_ms(), NULL, b.baddr, b.len);
                    cloud_storage_audio_write(PCM_TYPE_AUDIO, timer_get_ms(), b.baddr, b.len);
                }
                err = dev_ioctl(audio_fd, AUDIOC_QBUF, (unsigned int)&b);
            }
        }
    }



#endif
}

static int audio_enc_pid = 0;
void audio_enc_init(void)
{
    int err;
    struct audio_format f = {0};
    struct video_reqbufs breq = {0};

//    audio_fd = dev_open("audio", (void *)AUDIO_TYPE_ENC);
//    audio_fd = dev_open("audio", (void *)AUDIO_TYPE_ENC_USER);
    audio_fd = dev_open("audio", (void *)((AUDIO_FMT_AAC << 4) | AUDIO_TYPE_ENC_USER));
    if (!audio_fd) {
        puts("\n open audio enc err \n");
    }

    f.format = "aac";
    f.channel       = 1;
    f.sample_rate  = 16000;
    f.volume        = -1;
    f.sample_source = "user";
    f.frame_len      = 1024;//(8000 << 1) / 2;
    f.priority      = 0;
    f.kbps = 25600;
    err = dev_ioctl(audio_fd, AUDIOC_SET_FMT, (unsigned int)&f);
    if (err) {
        puts("\n\n AUDIOC_SET_FMT err\n\n");
        goto __err;
    }
    breq.buf = NULL;
    breq.size = 80 * 1024 ; //80*1024;
    err = dev_ioctl(audio_fd, AUDIOC_REQBUFS, (unsigned int)&breq);
    if (err) {
        puts("audio req buf err \n");
        goto __err;
    }
    err = dev_ioctl(audio_fd, AUDIOC_STREAM_ON, (u32)&channel_bindex);
    thread_fork("mic_task", 17, 0x500, 64, &audio_enc_pid, mic_task, NULL);
    return;
__err:
    puts("\n stream \n");
    dev_close(audio_fd);
    audio_fd = NULL;
}

void audio_enc_uninit(void)
{
    printf("<<< audio_enc_uninit\n");
    mic_knife = 1;

    int err = 0;
    if (audio_fd) {
        err = dev_ioctl(audio_fd, AUDIOC_STREAM_OFF, (u32)channel_bindex);
        channel_bindex = 0xff;
    }

    thread_kill(&audio_enc_pid, KILL_WAIT);

    if (audio_fd) {
        dev_close(audio_fd);
        audio_fd = NULL;
    }
    return;
}



