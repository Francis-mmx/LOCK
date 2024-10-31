#include "system/includes.h"
#include "generic/ioctl.h"
#include "circular_buf.h"
#include "audio_dev.h"
#include "database.h"
#include "gpio.h"

#include "server/audio_server.h"
static struct server *audio = NULL;
FILE *file_aud;

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
        }
        break;

    }
}
static int stream_fread(void *file, void *buf, u32 len)
{
    if (file_aud) {
        return fread(file_aud, buf, len);
    }
    return 0;
}

static int stream_fseek(void *file, u32 offset, int seek_mode)
{
    if (file_aud) {
        return fseek(file_aud, offset, seek_mode);
    }
    return 0;
}

static int stream_flen(void *file)
{
    if (file_aud) {
        return flen(file_aud);
    }
    return 0;
}

static const struct audio_vfs_ops stream_vfs = {
    .fread = stream_fread,
    .fseek = stream_fseek,
    .flen = stream_flen,
};


void aac_decode_open(void)
{
    int ret;
    union audio_req r = {0};

    file_aud = fopen(CONFIG_ROOT_PATH"testdac.aac", "r");
    if (!file_aud) {
        return ;
    }

    audio = server_open("audio_server", "dec");
    if (!audio) {
        return;
    }
    r.dec.cmd             = AUDIO_DEC_OPEN;
    r.dec.volume          = -1;
    r.dec.output_buf      = NULL;
    r.dec.output_buf_len  = 1024 * 4;
    r.dec.file            = 0;
    r.dec.dec_type        = "m4a";
    r.dec.sample_rate     = 0;//必须写零，由文件指定,否则不会解码文件信息
    r.dec.priority        = 0;
    /* r.dec.sample_source = "user"; */
    r.dec.sample_source = "dac";
    r.dec.vfs_ops      = &stream_vfs;
    ret = server_request(audio, AUDIO_REQ_DEC, &r);
    printf("ret:%d line:%d\n", ret, __LINE__);
    printf("total_time= %d \n", r.dec.total_time); //获取文件播放长度，可以用于ui显示
    printf("sample_rate= %d \n", r.dec.sample_rate); //获取文件采样率，可以用于ui显示

    r.dec.cmd = AUDIO_DEC_START;
    ret = server_request(audio, AUDIO_REQ_DEC, &r);
    printf("ret:%d line:%d\n", ret, __LINE__);
    server_register_event_handler(audio, NULL, audio_server_event_handler);//播放结束回调函数
}
