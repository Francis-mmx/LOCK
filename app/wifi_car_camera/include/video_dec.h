#ifndef __VIDEO_DEC_H_
#define __VIDEO_DEC_H_


#include "system/includes.h"
#include "server/video_server.h"
#include "app_config.h"
#include "storage_device.h"


#if (THREE_WAY_ENABLE == 1)
#define DEC_PATH_NUM  3
#else
#define DEC_PATH_NUM  2
#endif

enum {
    VIDEO_DEC_STOP,
    VIDEO_DEC_PLAYING,
    VIDEO_DEC_PAUSE,
    VIDEO_DEC_FF,
    VIDEO_DEC_FR,
};

#define FILE_TYPE_MOVE      0
#define FILE_TYPE_JPG       1





struct video_dec_hdl {
    u8 status;
    char curr_dir;
    u8 file_type;
    u8 ff_fr_times;
    u8 fname[MAX_FILE_NAME_LEN];
    u8 need_move;
    char *cur_path;
    int wait;
    int timer;
    int timeout;
    struct server *ui;
    struct vfscan *fs[DEC_PATH_NUM];
    struct server *video_dec;
    union video_dec_req req;
    u8 *audio_buf;
    u8 *video_buf;
};

#ifdef CONFIG_UI_STYLE_JL02_ENABLE
enum FILE_TYPE {
    VIDEO_FILE = 0,
    PHOTO_FILE,
    LOCK_FILE,
};
#endif
extern const char *dec_path[DEC_PATH_NUM][2];

void video_dec_post_msg(const char *msg, ...);

void video_dec_post_file_info_to_ui();

int video_dec_file(int mode);

int video_dec_play_pause();

void video_dec_scan_dir();

int dec_open_file();
extern u8 get_current_disp_device();
#endif
