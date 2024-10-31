#include "server/audio_server.h"
#include "app_config.h"
#include "printf.h"
#include "cpu.h"
#include "event.h"
#include "system/includes.h"
#include "generic/gpio.h"
#include "generic/typedef.h"
#include "os/os_compat.h"

/* #define AAC_ENC_EN */

#ifdef AAC_ENC_EN

//#define ENC_PCM   [>录制PCM音频<]     //否则录制AAC

extern int storage_device_ready();

struct _audio_config {
    void *aud_dev;
    void *audio_fd;
    void *audio_data;
    u8 audio_enc_status;
    u8 cbuf_rw_flag;	 // cbuf读写标志，写完置1，读完置0
    u32 cbuf_data_len;
    cbuffer_t cbuf_aud_data;
    u8 knife;           //为1杀线程
};
static struct _audio_config dev_cfg;
#define __this  (&dev_cfg)

static u8 channel_bindex = 0xff;



static void audio_enc_init(void)
{
    int err;
    struct audio_format f = {0};
    struct video_reqbufs breq = {0};

    __this->audio_fd = dev_open("audio", (void *)AUDIO_TYPE_ENC);
    if (!__this->audio_fd) {
        puts("\n open audio enc err \n");
    }
#ifdef ENC_PCM
    /* f.type          = AUDIO_FMT_PCM; */
    f.format = "pcm";
#else
    /* f.type          = AUDIO_FMT_AAC; */
    f.format = "aac";
#endif // ENC_PCM
    f.channel       = 1;
    f.sample_rate  = 8000;
    f.volume        = -1;
    f.sample_source = "mic";
    f.frame_len      = 1024;//(8000 << 1) / 2;
    f.priority      = 0;
    f.kbps = 25600;
    err = dev_ioctl(__this->audio_fd, AUDIOC_SET_FMT, (unsigned int)&f);
    if (err) {
        puts("\n\n AUDIOC_SET_FMT err\n\n");
        goto __err;
    }
    breq.buf = NULL;
    breq.size = 2 * 80 * 1024 ; //80*1024;
    err = dev_ioctl(__this->audio_fd, AUDIOC_REQBUFS, (unsigned int)&breq);
    if (err) {
        puts("audio req buf err \n");
        goto __err;
    }
    return;
__err:
    puts("\n stream \n");
    dev_close(__this->audio_fd);
    __this->audio_fd = NULL;
}



static void audio_enc_start(void)
{
    int err = 0;
    if (__this->audio_fd) {
        err = dev_ioctl(__this->audio_fd, AUDIOC_STREAM_ON, (u32)&channel_bindex);
        printf("channel_bindex =%d\n", channel_bindex);
        __this->audio_enc_status = 1;
        if (err) {
            puts("audio stream on err \n");
            dev_close(__this->audio_fd);
            __this->audio_fd = NULL;
        }
    }
}


static void audio_enc_stop(void)
{
    puts("audio_enc_stop>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    int err = 0;
    if (__this->audio_fd) {
        err = dev_ioctl(__this->audio_fd, AUDIOC_STREAM_OFF, (u32)channel_bindex);
        channel_bindex = 0xff;
        __this->audio_enc_status = 0;
    }
}


static void audio_enc_close(void)
{
    puts("audio_enc_close>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    if (__this->audio_fd) {
        if (channel_bindex != 0xff) {
            dev_ioctl(__this->audio_fd, AUDIOC_STREAM_OFF, (u32)channel_bindex);
            __this->audio_enc_status = 0;
            channel_bindex = 0xff;
        }
        dev_close(__this->audio_fd);
        __this->audio_fd = NULL;
    }

}


static void audio_enc_open(void)
{
    if (__this->audio_enc_status == 0) {
        audio_enc_init();
        audio_enc_start();
    }
}


#define WIRTE_CARD  //[>写卡测试<]
#define FILE_LEN    50*1024

static void audio_enc_task(void *p)
{
    printf("audio_enc_task\n");


    struct video_buffer b;
    int err;

    u32 data_len = 0;
    u8 buf_status = 0;
    char *audio_buf = calloc(1, FILE_LEN);

    audio_enc_init();
    audio_enc_start();
    os_time_dly(200);
    b.timeout = 100;
    b.noblock = 0;
    b.index = channel_bindex;


    while (!__this->knife) {//i<=100
        if (__this->audio_fd) {
            err = dev_ioctl(__this->audio_fd, AUDIOC_DQBUF, (unsigned int)&b);
            if (err) {
                printf("audio  timeout >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
                continue;
            }
            printf("\n data_len:%d,b.len:%d,buf_status:%d\n", data_len, b.len, buf_status);
            if (b.len) {
                if (data_len + b.len <= FILE_LEN) {

                    data_len += b.len;
                    if (data_len < 0) {
                        printf("\n data_len <<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 0\n");
                        continue;
                    }
                    memcpy(&audio_buf[data_len - b.len], (u8 *)b.baddr, b.len);
//                    printf("\n LOADING... \n");
                } else {
                    buf_status = 1;

                }
                err = dev_ioctl(__this->audio_fd, AUDIOC_QBUF, (unsigned int)&b);
            }
#ifdef WIRTE_CARD
            if (buf_status) {
#ifdef ENC_PCM
                printf("\n  write_aac_file\n");
                FILE *f = fopen(CONFIG_ROOT_PATH"audio***.PCM", "w+");
#else
                FILE *f = fopen(CONFIG_ROOT_PATH"audio***.aac", "w+");
#endif // ENC_PCM
                if (f) {
                    fwrite(f, audio_buf, data_len - 256);
                    fclose(f);
                    buf_status = 0;
                    data_len = 0;
                }
            }
#endif
        } else {
            printf("__this->audio_fd is Null\n");
        }

        os_time_dly(5);
    }
    puts("say bye >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    audio_enc_stop();
    audio_enc_close();
    free(audio_buf);
}


static int task_pid;
void stop_audio_enc()
{


    printf("\n  yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy \n");
    __this->knife = 1;
    thread_kill(&task_pid, KILL_WAIT);

}

#define TASK_STK_SIZE 2048
#define TASK_Q_SIZE 128
#define TASK_PRIO 21
void start_audio_enc(void)
{
    int err;
    memset(__this, 0x0, sizeof(*__this));
    __this->audio_enc_status = 0;
    puts("\n audio_ENC_init \n\n\n\n\n\n");
    thread_fork("audio_ENC_task", TASK_PRIO, TASK_STK_SIZE, TASK_Q_SIZE, &task_pid, audio_enc_task, NULL);
//	err = task_create(audio_ENC_task, __this, "audio_enc_task");
}

#endif

