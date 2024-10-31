#include "generic/typedef.h"
#include "fs/fs.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "server/simple_avi_unpkg.h"
#include "os/os_compat.h"
#include "app_config.h"
#include "net_config.h"


#define UNPKG_S_ERR() do {printf("err : %s  %d\n",__func__,__LINE__);return -1;}while(0)
#define UNPKG_U_ERR() do {printf("err : %s  %d\n",__func__,__LINE__);return 0;}while(0)

#define UNPKG_AVI_ERR() do {\
        printf("\n\nmake sure avi_net_playback_unpkg_init/avi_net_preview_unpkg_init func is used before use this function ,"\
        "and use avi_net_unpkg_exit func int the end!!!!\n\n");}while(0);

#define IDX_00DC   ntohl(0x30306463)
#define IDX_01WB   ntohl(0x30317762)
#define IDX_00WB   ntohl(0x30307762)

#if (defined CONFIG_ENABLE_VLIST)
#define REC_CYC_FILE_TIME 	10	 //分钟
#define AVI_AUDIO_NUM_TABLE		(REC_CYC_FILE_TIME * 60 * 2 + 128)
#else
#define REC_CYC_FILE_TIME 		0	 //分钟
#define AVI_AUDIO_NUM_TABLE		(REC_CYC_FILE_TIME * 60 * 2 + 128)
#endif

struct avi_head_str {
    UNPKG_JL_AVI_HEAD file_head;
    FILE *fd;
    u8 flag;
    u32 flen;
    u32 vdframe_cnt;
    u32 vdframe_offset_cnt;
    u32 adframe_cnt;
    u32 adframe_offset_cnt;

    u32 audio_chunk_num;
    u32 video_chunk_num;

    u32 last_ad_num;
    u32 last_ad_sum;
    u32 last_vd_num;
    u32 last_vd_sum;

    u32 idx1_addr;
    u32 per_adframe_time;//每帧音频时间
    float video_num_coefficient;
    float audio_num_coefficient;
    u8 audio_num_buff[AVI_AUDIO_NUM_TABLE];//存储音频每帧偏移量，1s2帧，3分钟360帧
    u8 *video_num_buff;
};
static u8 avi_open_file_num = 0;
static struct avi_head_str avi_head_s[2];//0 preview , 1 playback

static int avi_get_video_audio_chunk_num(FILE *fd, u8 state);

static struct avi_head_str *avi_get_head_handler(u8 state)
{
    struct avi_head_str *avi_info = NULL;

    if (!state) {
        avi_info = &avi_head_s[0];
    } else {
        avi_info = &avi_head_s[1];
    }

    return avi_info;
}

static int avi_read_head(FILE *fd, u8 state)
{
    int len;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    memset(avi_info, 0, sizeof(struct avi_head_str));

    if (fd == NULL) {
        UNPKG_S_ERR();
    }

    if (avi_info->flag && avi_info->fd == fd) {
        return 0;
    }
    if (!fseek(fd, 0, SEEK_SET)) {
        UNPKG_S_ERR();
    }
    len = fread(fd, &avi_info->file_head, sizeof(UNPKG_JL_AVI_HEAD));
    if (len != sizeof(UNPKG_JL_AVI_HEAD)) {
        fseek(fd, 0, SEEK_SET);
        UNPKG_S_ERR();
    }
    avi_info->flen = flen(fd);
    avi_info->fd = fd;
    avi_info->flag = true;

    fseek(fd, 0, SEEK_SET);
    return 0;
}


int avi_net_playback_unpkg_init(FILE *fd, u8 state)
{
    if (fd) {
        avi_read_head(fd, state);
        avi_get_video_audio_chunk_num(fd, state);
        avi_open_file_num++;
        return 0;
    }
    return -1;
}
int avi_net_preview_unpkg_init(FILE *fd, u8 state)
{
    if (fd) {
        avi_read_head(fd, state);
        avi_open_file_num++;
        return 0;
    }
    return -1;
}
int avi_net_unpkg_exit(FILE *fd, u8 state)
{
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (fd) {
        if (avi_open_file_num == 0) {
            avi_open_file_num = 0;
        } else {
            avi_open_file_num--;
        }
        if (avi_info->video_num_buff) {
            free(avi_info->video_num_buff);
            avi_info->video_num_buff = NULL;
        }
        /*memset(avi_info,0,sizeof(struct avi_head_str));*/
        return 0;
    }
    return -1;
}

int is_vaild_avi_file(FILE *fd, u8 state)
{
    int riff = ntohl(0x52494646);//"riff"
    int avi = ntohl(0x41564920);//"avi "
    int err;
    int fsize;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (!fd) {
        UNPKG_S_ERR();
    }
    if (avi_info->flag && avi_info->fd == fd) {
        goto get_data;
    } else {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

get_data:
    if (avi_info->file_head.avih.dwTotalFrames == 0 || avi_info->file_head.file_size == 0) {
        UNPKG_U_ERR();
    }
    fsize = flen(fd);
    if (avi_info->file_head.file_size > fsize) {
        printf("avi file is destroy !!!\n\n");
        return 0;
    }
    if (avi_info->file_head.riff == riff || avi_info->file_head.file_type == avi) {
        return 1;
    }
    return 0;
}

int avi_is_has_audio(FILE *fd, u8 state)
{
    int err;
    int auds = ntohl(0x61756473);//"auds"
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (!fd) {
        UNPKG_S_ERR();
    }
    if (avi_info->flag && avi_info->fd == fd) {
        goto get_data;
    } else {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

get_data:
    if (avi_info->file_head.aud_strh.fccType == auds && avi_info->file_head.aud_strh.dwLength > 0) {
        return 0;
    }
    return -1;
}

int avi_get_fps(FILE *fd, u8 state)
{
    int fps;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (!fd) {
        UNPKG_S_ERR();
    }
    if (avi_info->flag && avi_info->fd == fd) {
        goto get_data;
    } else {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

get_data:
    fps = avi_info->file_head.vid_strh.dwRate;
//    printf("file fps : %d  \n",fps);
    return fps;
}

int avi_get_file_time(FILE *fd, u8 state)
{
    int time;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (!fd) {
        UNPKG_S_ERR();
    }
    if (avi_info->flag && avi_info->fd == fd) {
        goto get_data;
    } else {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

get_data:
    time = 1000000 / avi_info->file_head.avih.dwMicroSecPerFrame;
    time = avi_info->file_head.avih.dwTotalFrames / (time > 0 ? time : avi_info->file_head.avih.dwTotalFrames);
    time = time > 0 ? time : 1;
    /*printf("file time : %d \n", time);*/
    return time;
}

int avi_get_width_height(FILE *fd, void *__info, u8 state)
{
    AVI_UNPKG_INFO *info = (AVI_UNPKG_INFO *)__info;
    int err;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (!fd) {
        UNPKG_S_ERR();
    }
    if (avi_info->flag && avi_info->fd == fd) {
        goto get_data;
    } else {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

get_data:
    info->height = avi_info->file_head.avih.dwHeight;
    info->length = avi_info->file_head.avih.dwWidth;
    /*printf("height : %d , length : %d  \n",info->height,info->length);*/
    return 0;
}

int avi_get_audio_sample_rate(FILE *fd, u8 state)
{
    int err;
    int rate;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (!fd) {
        UNPKG_S_ERR();
    }
    if (avi_info->flag && avi_info->fd == fd) {
        goto get_data;
    } else {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

get_data:
    rate = avi_info->file_head.aud_strh.dwRate;
    /*if(!avi_info->audio_chunk_num)*/
    /*return 0;*/
//    printf("rate : %d  \n",rate);
    return rate;

}

int avi_audio_base_to_get_video_frame(u32 audio_num, u8 state) //线性补偿,audio_num:0,1....n
{
    double vd_num;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (avi_info->video_num_coefficient < 0.000001) {
        return 0;
    }
    audio_num += 1;
    vd_num = (double)(avi_info->video_num_coefficient * audio_num);
    return (int)vd_num;
}
int avi_video_base_to_get_audio_frame(int vd_num, u8 state) //线性补偿,vd_num:0,1.....n
{
    double ad_num;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (avi_info->audio_num_coefficient < 0.000001) {
        return 0;
    }
    vd_num += 1;
    ad_num = (double)(avi_info->audio_num_coefficient * vd_num);
    return (int)ad_num;
}

static int avi_get_audio_perfram_size(FILE *fd, int addr, u8 state)
{
    AVI_INDEX_INFO index_info = {0};
    int index_info_len = sizeof(AVI_INDEX_INFO);
    int i;
    int cnt = 0;
    int len;

    if (fd == NULL) {
        UNPKG_S_ERR();
    }

read_vd:
    if (!fseek(fd, addr + cnt * index_info_len, SEEK_SET)) {
        UNPKG_S_ERR();
    }
    len = fread(fd, &index_info, index_info_len);
    if (len != index_info_len) {
        UNPKG_S_ERR();
    }
    if (index_info.dwChunkId == 0 || index_info.dwOffset == 0 || index_info.dwSize == 0
        || (index_info.dwChunkId != IDX_00DC && index_info.dwChunkId != IDX_01WB && index_info.dwChunkId != IDX_00WB)) {
        return 0;
    }

    if (index_info.dwChunkId == IDX_01WB || index_info.dwChunkId == IDX_00WB) {
        return index_info.dwSize;
    }
    cnt++;
    goto read_vd;
    return -1;
}



static u32 avi_find_index_addr(FILE *fd, u8 state)
{
    int offset;
    int index_addr;
    int idx1 = ntohl(0x69647831);//"index1"
    int len;
    int err;
    int addr;
    float vd_presec_ms;
    float ad_presec_ms;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    struct idex1_str {
        int str;
        int len;
    } idex1_head = {0};

    u8 try_find_idx = 0; //只在文件末尾找一次idx1

    if (fd == NULL) {
        UNPKG_U_ERR();
    }

    if (avi_info->flag && avi_info->fd == fd) {
        goto get_data;
    } else {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

get_data:
    if (avi_info->idx1_addr) {
        return avi_info->idx1_addr;
    }

    // 2 . 读取AVI文件头
    if (!avi_info->flag || avi_info->fd != fd) {
        UNPKG_U_ERR();
    }

    index_addr = avi_info->file_head.len - 4;//音视频数据内容大小
    addr  = sizeof(UNPKG_JL_AVI_HEAD) + index_addr;
    printf("avi_find addr=0x%x\n", addr);

get_idx_data:
    //3 . 跳转到索引表处
    if (!fseek(fd, addr, SEEK_SET)) {
        fseek(fd, 0, SEEK_SET);
        UNPKG_U_ERR();
    }

    //4 . 读取"inde1"块的头和整个索引表的长度
    len = fread(fd, &idex1_head, sizeof(struct idex1_str));
    addr += sizeof(struct idex1_str);
    if (len != sizeof(struct idex1_str)) {
        fseek(fd, 0, SEEK_SET);
        UNPKG_U_ERR();
    }

    if (idex1_head.str != idx1) {
        if (!try_find_idx) {
            /* 兼容avi修复的文件解码 */
            addr = flen(fd) - 512 * 1024 + 8;
            try_find_idx = 1;
            goto get_idx_data;
        }
        fseek(fd, 0, SEEK_SET);
        UNPKG_U_ERR();
    }

    if (!avi_info->per_adframe_time) {
        len = avi_get_audio_perfram_size(fd, addr, state);
        if (len > 0) {
            avi_info->per_adframe_time = ((float)len / (avi_info->file_head.aud_strh.dwRate * avi_info->file_head.aud_strf.nBlockAlign)) * 1000;
            ad_presec_ms = (float)((float)len / (avi_info->file_head.aud_strh.dwRate * avi_info->file_head.aud_strf.nBlockAlign)) * 1000;
            vd_presec_ms = (float)avi_info->file_head.avih.dwMicroSecPerFrame / 1000;
            avi_info->video_num_coefficient = (float)ad_presec_ms / vd_presec_ms;
            avi_info->audio_num_coefficient = (float)vd_presec_ms / ad_presec_ms;
            /*printf("ad_presec_ms : %f , %f , %f \n",ad_presec_ms,vd_presec_ms,avi_info->video_num_coefficient);*/
        } else {
            printf("get AVI audio frame err!!!!!  \n");
            avi_info->per_adframe_time = ((float)8184 / (8000 * 2)) * 1000; //ms
        }
    }
    //5 . 返回第一个索引表地址
    avi_info->idx1_addr = addr;
    printf("\n[ debug ]--func=%s line=%d idx1_addr=%x\n", __func__, __LINE__, avi_info->idx1_addr);

    //6 . 恢复文件指针
    fseek(fd, 0, SEEK_SET);
    return avi_info->idx1_addr;
}

static int avi_audio_get_offset_cnt(FILE *fd, int num, u8 state) //第一帧从1开始，...
{
    int i;
    int addr = 0;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (!avi_info->flag || avi_info->fd != fd) {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }
    if (num > avi_info->audio_chunk_num) {
        return -1;
    }
    if (num <= 1) {
        avi_info->last_ad_num = 0;
        avi_info->last_ad_sum = 0;
        num = 1;
    }
    if (num < avi_info->last_ad_num) {
        for (i = 0; i < num; i++) { //任意帧
            addr += avi_info->audio_num_buff[i];
        }
    } else {
        addr += avi_info->last_ad_sum;
        for (i = avi_info->last_ad_num; i < num; i++) { //升序帧，第一帧从1开始，...
            addr += avi_info->audio_num_buff[i];
        }
    }
    avi_info->last_ad_num = num;
    avi_info->last_ad_sum = addr;
    return addr;
}
static int avi_video_get_offset_cnt(FILE *fd, int num, u8 state) //第一帧从1开始，...
{
    int i;
    int addr = 0;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (!avi_info->flag || avi_info->fd != fd) {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }

    if (num <= 1) {
        avi_info->last_vd_num = 0;
        avi_info->last_vd_sum = 0;
        num = 1;
        return 0;
    }

    if (num > avi_info->video_chunk_num && !state) {  //回放模式
        return -1;
    }

    if (num < avi_info->last_vd_num) {
        for (i = 0; i < num; i++) { //任意帧
            addr += avi_info->video_num_buff[i];
        }
    } else {
        addr += avi_info->last_vd_sum;
        for (i = avi_info->last_vd_num; i < num; i++) { //升序帧，第一帧从1开始，...
            addr += avi_info->video_num_buff[i];
        }
    }
    avi_info->last_vd_num = num;
    avi_info->last_vd_sum = addr;
    return addr;
}

static int avi_get_video_audio_chunk_num(FILE *fd, u8 state) //获取整个视频的音视频帧数
{
    int num = 0;
    int cnt = 0;
    AVI_INDEX_INFO index_info = {0};
    int index_info_len = sizeof(AVI_INDEX_INFO);
    int addr;
    int len;
    int start = 0, end = 0;
    int vdstart = 0, vdend = 0;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (avi_info->audio_chunk_num) {
        return avi_info->audio_chunk_num;
    }

    addr = (int)avi_find_index_addr(fd, state);
    if (addr <= 0) {
        UNPKG_S_ERR();
    }
    if (!fseek(fd, addr + cnt * index_info_len, SEEK_SET)) {
        UNPKG_S_ERR();
    }

    if (!avi_info->video_num_buff) {
        avi_info->video_num_buff = zalloc(avi_info->file_head.avih.dwTotalFrames);//回放模式
    }
    if (!avi_info->video_num_buff) {
        printf("%s no mem err !!!!!\n", __func__);
        UNPKG_S_ERR();
    }

    for (;;) {
        len = fread(fd, &index_info, index_info_len);
        if (len != index_info_len) {
            UNPKG_S_ERR();
        }
        if (index_info.dwChunkId == 0 || index_info.dwOffset == 0 || index_info.dwSize == 0
            || (index_info.dwChunkId != IDX_00DC && index_info.dwChunkId != IDX_01WB && index_info.dwChunkId != IDX_00WB)) {
            break;
        }

        if (index_info.dwChunkId == IDX_01WB || index_info.dwChunkId == IDX_00WB) {
            start = cnt + num;
            if (num < sizeof(avi_info->audio_num_buff)) {
                avi_info->audio_num_buff[num] =  start - end;
            }
            end = start;
            num++;
            avi_info->audio_chunk_num = num;
        } else {
            vdstart = cnt + num;
            if (cnt < avi_info->file_head.avih.dwTotalFrames) {
                avi_info->video_num_buff[cnt] = vdstart - vdend;
            }
            vdend = vdstart;
            cnt++;
        }
    }
    avi_info->video_chunk_num = cnt;
    /*printf("audio_chunk_num num : %d \n",num);*/
    return num;
}
int avi_get_video_num(FILE *fd, int offset_ms, u8 state)
{
    int cnt;
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    if (!avi_info->flag || avi_info->fd != fd) {
        UNPKG_AVI_ERR();
        UNPKG_S_ERR();
    }
    cnt = offset_ms / (avi_info->file_head.avih.dwMicroSecPerFrame / 1000); //计算对应帧数
    return cnt;
}
int avi_get_audio_chunk_num(FILE *fd, u8 state) //获取整个视频的音频帧数
{
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    return avi_info->audio_chunk_num;
}
int avi_get_video_chunk_num(FILE *fd, u8 state) //获取整个视频的音频帧数
{
    struct avi_head_str *avi_info = avi_get_head_handler(state);
    return avi_info->video_chunk_num;
}
//读对应视频帧数
int avi_video_get_frame(FILE *fd, int offset_num, u8 *buf, u32 buf_len, u8 state)
{
    u32 addr;
    int len;
    int all_len;
    int idx_send_cnt;
    int idx_offset;
    AVI_INDEX_INFO index_info = {0};
    int index_info_len = sizeof(AVI_INDEX_INFO);
    int i;
    int cnt = 0;
    int offset_addr;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (fd == NULL) {
        UNPKG_S_ERR();
    }
    addr = avi_find_index_addr(fd, state);
    if (!addr) {
        UNPKG_S_ERR();
    }
    avi_info->vdframe_offset_cnt = offset_num;//计算对应帧数
    if (avi_info->vdframe_offset_cnt > avi_info->file_head.avih.dwTotalFrames) {
        avi_info->vdframe_offset_cnt = avi_info->file_head.avih.dwTotalFrames;
    }
    cnt = avi_video_get_offset_cnt(avi_info->fd, avi_info->vdframe_offset_cnt, state);
    /*printf("video cnt : %d , offset_num : %d \n",cnt,offset_num);*/
    if (cnt < 0) {
        return -1;
    }
read_vd:
    offset_addr = addr + cnt * index_info_len;
    if (offset_addr > avi_info->flen) {
        return 0;
    }
    if (!fseek(fd, offset_addr, SEEK_SET)) {
        UNPKG_S_ERR();
    }
    len = fread(fd, &index_info, index_info_len);
    if (len != index_info_len) {
        UNPKG_S_ERR();
    }
    if (index_info.dwChunkId == 0 || index_info.dwOffset == 0 || index_info.dwSize == 0
        || (index_info.dwChunkId != IDX_00DC && index_info.dwChunkId != IDX_01WB && index_info.dwChunkId != IDX_00WB)) {
        printf("end of video AVI file ... \n");
        return 0;
    }

    if (index_info.dwChunkId == IDX_00DC) {
//        idx_offset = index_info.dwOffset + 0x204;
        idx_offset = index_info.dwOffset + sizeof(UNPKG_JL_AVI_HEAD) + 4;
        if (offset_addr > avi_info->flen || offset_addr < 0) {
            UNPKG_S_ERR();
        }
        if (!fseek(fd, idx_offset, SEEK_SET)) {
            printf("idx_offset : %d , index_info.dwOffset : %d \n", idx_offset, index_info.dwOffset);
            UNPKG_S_ERR();
        }
        all_len  = 0;
    } else {
        cnt++;
        goto read_vd;
    }
    /*printf("video frame len : %d \n", index_info.dwSize);*/
    if (index_info.dwSize > buf_len) {
        printf("\n\n\n!!!!!err video data buff len not enough , need : %d ,but buflen is %d !!!!!!!!!!!\n\n", index_info.dwSize, buf_len);
        /*ASSERT(0, "!err video data buff len not enough");*/
        return 0;
    }
    all_len = fread(fd, buf, index_info.dwSize);
    /*printf("video frame all_len : %d \n", all_len);*/
    if (all_len != index_info.dwSize) {
        UNPKG_S_ERR();
    }
    avi_info->vdframe_cnt = offset_num;
    return all_len;
}
int avi_video_set_frame_addr(FILE *fd, int offset_num, u8 state)
{
    u32 addr;
    int len;
    int all_len;
    int idx_send_cnt;
    int idx_offset;
    AVI_INDEX_INFO index_info = {0};
    int index_info_len = sizeof(AVI_INDEX_INFO);
    int i;
    int cnt = 0;
    int offset_addr;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (fd == NULL) {
        UNPKG_S_ERR();
    }
    addr = avi_find_index_addr(fd, state);
    if (!addr) {
        UNPKG_S_ERR();
    }
    avi_info->vdframe_offset_cnt = offset_num;//计算对应帧数
    if (avi_info->vdframe_offset_cnt > avi_info->file_head.avih.dwTotalFrames) {
        avi_info->vdframe_offset_cnt = avi_info->file_head.avih.dwTotalFrames;
        /*UNPKG_S_ERR();*/
    }
    cnt = avi_video_get_offset_cnt(avi_info->fd, offset_num, state);
    if (cnt < 0) {
        return -1;
    }
//    printf("cnt : %d ,vdframe_offset_cnt : %d\n",cnt,avi_info->vdframe_offset_cnt);
read_vd:
    offset_addr = addr + cnt * index_info_len;
    if (offset_addr > avi_info->flen) {
        return 0;
    }
    if (!fseek(fd, offset_addr, SEEK_SET)) {
        UNPKG_S_ERR();
    }
    len = fread(fd, &index_info, index_info_len);
    if (len != index_info_len) {
        UNPKG_S_ERR();
    }
    if (index_info.dwChunkId == 0 || index_info.dwOffset == 0 || index_info.dwSize == 0
        || (index_info.dwChunkId != IDX_00DC && index_info.dwChunkId != IDX_01WB && index_info.dwChunkId != IDX_00WB)) {
        printf("end of video AVI file ... \n");
        return 0;
    }

    if (index_info.dwChunkId == IDX_00DC) {
        idx_offset = index_info.dwOffset + 0x204;
        if (offset_addr > avi_info->flen || offset_addr < 0) {
            UNPKG_S_ERR();
        }
        if (!fseek(fd, idx_offset, SEEK_SET)) {
            UNPKG_S_ERR();
        }
    } else {
        cnt++;
        /*avi_info->adframe_cnt++;*/
        goto read_vd;
    }
    return index_info.dwSize;
}


//读对应音频帧数
int avi_audio_get_frame(FILE *fd, int offset_num, u8 *buf, u32 buf_len, u8 state)  //offset_num : 1....n
{
    u32 addr;
    int len;
    int all_len;

    int idx_send_cnt;
    int idx_offset;
    AVI_INDEX_INFO index_info = {0};
    int index_info_len = sizeof(AVI_INDEX_INFO);
    int i;
    int cnt = 0;
    int offset_addr;
    int last_addr = 0;
    struct avi_head_str *avi_info = avi_get_head_handler(state);

    if (fd == NULL) {
        UNPKG_S_ERR();
    }
    addr = avi_find_index_addr(fd, state);
    if (!addr) {
        UNPKG_S_ERR();
    }
    if (offset_num > avi_info->audio_chunk_num) {
        /*printf("Warnning : offset_num > avi_info->audio_chunk_num \n");*/
        return 0;
    }
    last_addr = fpos(fd);
    avi_info->adframe_offset_cnt = 0;
    cnt = avi_audio_get_offset_cnt(fd, offset_num, state);
    if (cnt < 0) {
        return -1;
    }
    /*printf("audio cnt : %d \n",cnt);*/
read_vd:
    offset_addr = addr + cnt * index_info_len;
    if (offset_addr > avi_info->flen) {
        fseek(fd, last_addr, SEEK_SET);
        return 0;
    }
    if (!fseek(fd, offset_addr, SEEK_SET)) {
        fseek(fd, last_addr, SEEK_SET);
        UNPKG_S_ERR();
    }
    len = fread(fd, &index_info, index_info_len);
    if (len != index_info_len) {
        fseek(fd, last_addr, SEEK_SET);
        UNPKG_S_ERR();
    }
    if (index_info.dwChunkId == 0 || index_info.dwOffset == 0 || index_info.dwSize == 0
        || (index_info.dwChunkId != IDX_00DC && index_info.dwChunkId != IDX_01WB && index_info.dwChunkId != IDX_00WB)) {
        fseek(fd, last_addr, SEEK_SET);
        /*printf("end of audio AVI file ... \n");*/
        return 0;
    }

    if (index_info.dwChunkId == IDX_01WB || index_info.dwChunkId == IDX_00WB) {
//        idx_offset = index_info.dwOffset + 0x204;
        idx_offset = index_info.dwOffset + sizeof(UNPKG_JL_AVI_HEAD) + 4;
        if (offset_addr > avi_info->flen || offset_addr < 0) {
            UNPKG_S_ERR();
        }
        if (!fseek(fd, idx_offset, SEEK_SET)) {
            fseek(fd, last_addr, SEEK_SET);
            UNPKG_S_ERR();
        }
        all_len  = 0;
    } else {
        cnt++;
        goto read_vd;
    }
    if (index_info.dwSize > buf_len) {
        printf("\n\n\n!!!!!err audio data buff len not enough , need : %d ,but buflen is %d !!!!!!!!!!!\n\n", index_info.dwSize, buf_len);
        /*ASSERT(0, "!err audio data buff len not enough");*/
        return 0;
    }
    all_len = fread(fd, buf, index_info.dwSize);
    if (all_len != index_info.dwSize) {
        fseek(fd, last_addr, SEEK_SET);
        UNPKG_S_ERR();
    }
    avi_info->adframe_cnt++;
    fseek(fd, last_addr, SEEK_SET);
    return all_len;
}


