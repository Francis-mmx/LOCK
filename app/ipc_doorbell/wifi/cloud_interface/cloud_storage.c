#define	_STDIO_H_
#include "system/includes.h"
#include "os/os_api.h"
#include "server/rt_stream_pkg.h"
#include "xciot_api.h"
#include "ut_dev_ipc_cmd.h"
#include "xc_cs_type.h"
#include "system/timer.h"
#include "generic/lbuf.h"
#include "os/os_compat.h"
#include "doorbell_event.h"
#include "generic/list.h"


#define CLOUD_STORAGE_DEBUG

/***注意：
**运存接口不支持多线程操作
**/

typedef struct cloud_media_info {
    struct list_head entry;
    int type;
    u32 len;
    u32 difftime;
    u8 is_key_frame;
    u8 data[0];
} CLOUD_MEDIA_INFO;

static LIST_HEAD(cloud_media_info_list_head);
static OS_MUTEX cloud_media_info_list_mutex;

static OS_SEM cloud_storage_sem;
static struct lbuff_head *lbuf_handle = NULL;
static int cloud_storage_timer_id;
static u8 wakeup_status;
static old_wakeup_status = INIT_WAKEUP;
static u8 cloud_storage_timer_cnt;
static u8 cloud_storage_timeout_cnt;
static u8 cloud_storage_up_state = CLOUD_STORAGE_IDLE;
static int64_t start_time = 0;
static cs_hdl_ref_t cs_hdl = NULL;
static int event_id;
static OS_MUTEX cloud_storage_mutex;
typedef struct cloud_playback_list_info {
    struct list_head entry;
    int wakeup_status;
} WAKEUP_STATUS_LIST_INFO;
static LIST_HEAD(wakeup_status_list_head);
static OS_MUTEX list_head_mutex;
static int cloud_storage_fps_cnt = 0;
static int cloud_storage_first_frame = 0;
static u8 is_wait_iframe = 1;
static u8 audio_frame_start = 0;
static int video_frame_count = 0;

typedef struct {
    int id;
    int wakeup_status;
    int uploading;
} WAKEUP_STATUS_BACKUP;


#define CLOUD_STORAGE_TIME_CNT    120
#if SDRAM_SIZE == (32 * 1024 * 1024)
#define CLOUD_STORAGE_LBUF_SIZE   (1 * 1024 * 1024)
#define AVSDK_MEM_SIZE            (1 * 1024 * 1024)
#else
#define CLOUD_STORAGE_LBUF_SIZE   (1 * 1024 * 1024 + 512 * 1024)
#define AVSDK_MEM_SIZE            (1 * 1024 * 1024)
#endif // SDRAM_SIZE




u8 is_pir_wakeup_status()
{
    WAKEUP_STATUS_LIST_INFO *p, *n;
    os_mutex_pend(&list_head_mutex, 0);
    if (list_empty(&wakeup_status_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&list_head_mutex);
        return 0;
    }
    list_for_each_entry_reverse_safe(p, n, &wakeup_status_list_head, entry) {
        if (p->wakeup_status == PIR_WAKEUP) {
            os_mutex_post(&list_head_mutex);
            return 1;
        }
    }
    os_mutex_post(&list_head_mutex);

}
int is_same_wakeup_status(int wakeup_status)
{
    WAKEUP_STATUS_LIST_INFO *p, *n;
    os_mutex_pend(&list_head_mutex, 0);
    if (list_empty(&wakeup_status_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&list_head_mutex);
        return 0;
    }
    list_for_each_entry_reverse_safe(p, n, &wakeup_status_list_head, entry) {
        if (p->wakeup_status == wakeup_status) {
            os_mutex_post(&list_head_mutex);
            return 1;
        }
    }
    os_mutex_post(&list_head_mutex);
    return 0;
}
int wakeup_status_list_head_clear(void)
{
    WAKEUP_STATUS_LIST_INFO *p, *n;
    os_mutex_pend(&list_head_mutex, 0);
    if (list_empty(&wakeup_status_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&list_head_mutex);
        return 0;
    }
    list_for_each_entry_reverse_safe(p, n, &wakeup_status_list_head, entry) {
        list_del(&p->entry);
        free(p);
    }
    os_mutex_post(&list_head_mutex);
    return 0;
}

static void cloud_storage_timer_del(void)
{
    os_mutex_pend(&cloud_storage_mutex, 0);

    if ((cloud_storage_up_state == CLOUD_STORAGE_START) || (cloud_storage_up_state == CLOUD_STORAGE_STARTING)) {
        cloud_storage_timer_cnt = 0;
        cloud_storage_fps_cnt = 0;
        sys_timer_del(cloud_storage_timer_id);
        wakeup_status = INIT_WAKEUP ;
        cloud_storage_timer_id = 0;
        cloud_storage_up_state = CLOUD_STORAGE_STOPING;
    } else {
        printf("\n>>>>>>>The cloud storage timer has been deleted,cloud_storage_timer_id = %d\n", cloud_storage_timer_id);
    }
    os_mutex_post(&cloud_storage_mutex);
    //先停止录像，然后再查询PIR是否还处于触发状态
}

int get_avsdk_event_id(int wakeup_status)
{
    int event_id;
    if (wakeup_status == PIR_WAKEUP) {
        event_id = E_IOT_EVENT_SENSE_HUMAN;
    } else if (wakeup_status == KEY_WAKEUP) {
        event_id = E_IOT_EVENT_VISUAL_DOORBELL;
    } else if (wakeup_status == BREAK_WAKEUP) {
        event_id = E_IOT_EVENT_PICKPROOF_ALARM;
    } else if (wakeup_status == LOWPOWER_WAKEUP) {
        event_id = E_IOT_EVENT_LOW_BATTERY;
    }
    return event_id;
}

int64_t set_avsdk_push_event(int wakeup_status, u8 uploading)
{
    int64_t ret = -1;
    iot_event_opt_t  event = {0};


    if (!get_avsdk_connect_flag()) {
        goto __exit;
    }

    event.isCS = true;
    event.value = 0;
    event.utc_ms = uploading ? start_time : get_utc_ms();

    event.id = get_avsdk_event_id(wakeup_status);
    if (event.id == E_IOT_EVENT_LOW_BATTERY) {
        event.isCS = false;
    }
__again:
    ;
    int retry = 0;
    ret =  avsdk_push_event(&event);
    if (ret <  0) {
        if (ret == -2 && is_same_wakeup_status(wakeup_status)) {
            printf("\n >>>>>> The same event cannot be triggered repeatedly within 5S\n");
            goto __exit;
        } else if (ret != -1 && retry < 5) {
            retry++;
            msleep(1000);
            printf("\n >>>>>> set_avsdk_push_event retry\n");
            goto __again;
        }
        goto __exit;
    } else if (ret == 0) {
        printf("\nThe cloud storage function is not enabled for the current device\n");
        //即使没有开通云存，也需要保存状态，因为本地录像需要使用
        goto __exit;
    }
    if (!uploading) {
        start_time = ret;
    }
    if (event.isCS == false) {
        ret = 0;
    }
__exit:
    if (!is_same_wakeup_status(wakeup_status)) {
        WAKEUP_STATUS_LIST_INFO *info = calloc(1, sizeof(WAKEUP_STATUS_LIST_INFO));
        if (info) {
            info->wakeup_status = wakeup_status;
        }
        os_mutex_pend(&list_head_mutex, 0);
        list_add_tail(&info->entry, &wakeup_status_list_head);
        os_mutex_post(&list_head_mutex);
    }

    return ret;
}



void wakeup_status_backup_cb(void *priv)
{

    WAKEUP_STATUS_BACKUP *backup_info = (WAKEUP_STATUS_BACKUP *)priv;
    printf("\n >>>>>>>>>wakeup_status_backup_cb backup_info->wakeup_status = %d\n", backup_info->wakeup_status);
    wait_completion_del(backup_info->id);
    if (!is_same_wakeup_status(backup_info->wakeup_status)) {
        set_avsdk_push_event(backup_info->wakeup_status, backup_info->uploading);
    }
    backup_info->id = 0;
    free(backup_info);
}

int wakeup_status_backup_cb_condition(void)
{
    return (get_avsdk_connect_flag() && cloud_storage_up_state == CLOUD_STORAGE_START);
}


#define CLOUD_STORAGE_TIME_INTERVAL     100// 判断一次100ms
void cloud_storage_timer(void *priv)
{
    static int cnt;
    if (get_avsdk_connect_flag()) {
        //云存未开始则需要启动云存
        os_mutex_pend(&cloud_storage_mutex, 0);
        if (cloud_storage_up_state == CLOUD_STORAGE_STARTING) {
            if (os_sem_valid(&cloud_storage_sem) && !os_sem_query(&cloud_storage_sem)) {
                os_sem_post(&cloud_storage_sem);
                cnt = 0;
            }
        }
        os_mutex_post(&cloud_storage_mutex);
#if 0
        //采用定时器定时方式退出云存
        cnt++;
        if ((cloud_storage_up_state == CLOUD_STORAGE_START) && (cnt > (1000) / CLOUD_STORAGE_TIME_INTERVAL)) {
            cloud_storage_timer_cnt++;
            cnt = 0;
        }

        if (cloud_storage_timer_cnt >= (cloud_storage_timeout_cnt + 2)) {
            cloud_storage_timer_del();
        }

#else
        //采用数帧数方式退出云存
        if (cloud_storage_fps_cnt >= db_select("cyc") * net_video_rec_get_fps()) {
            cloud_storage_timer_del();
        }
#endif // 0
    }
}


u8 get_cloud_storage_up_state(void)
{
    return cloud_storage_up_state;
}

u8 get_wakeup_status()
{
    return wakeup_status;
}

void set_wakeup_status(u8 status)
{
    if ((status == INIT_WAKEUP) || (status == NETWORK_WAKEUP)) {
        return;
    }
    int ret = os_mutex_pend(&cloud_storage_mutex, 300);
    if (ret != OS_NO_ERR) {
        printf("\n >>>>>>%s %d  os_mutex_pend err\n", __func__, __LINE__);
        return;
    }
    wakeup_status = status;
    //如果云存还没有启动，新建一个云存
    if (cloud_storage_up_state == CLOUD_STORAGE_STOP || cloud_storage_up_state == CLOUD_STORAGE_IDLE) {
        cloud_storage_up_state = CLOUD_STORAGE_STARTING;
        extern int video0_rec_get_iframe(void);
        video0_rec_get_iframe();
        event_id = get_avsdk_event_id(wakeup_status);
        if (!cloud_storage_timer_id) {
            cloud_storage_timer_cnt = 0;
            cloud_storage_fps_cnt = 0;
            cloud_storage_timeout_cnt = db_select("cyc");
            cloud_storage_timer_id = sys_timer_add_to_task("sys_timer", NULL, cloud_storage_timer, CLOUD_STORAGE_TIME_INTERVAL);

        }
    } else {
        /*如果云存已启动*/
        if (cloud_storage_up_state == CLOUD_STORAGE_START) {
            if (is_same_wakeup_status(wakeup_status) &&  wakeup_status == PIR_WAKEUP) {
                printf("\n >>>>>>%s %d is_same_wakeup_status(wakeup_status) = 1 &&  wakeup_status == PIR_WAKEUP \n", __func__, __LINE__);
                goto __exit;
            }
            if (set_avsdk_push_event(wakeup_status, 1) >= 0 && cloud_storage_timeout_cnt < CLOUD_STORAGE_TIME_CNT) {
                cloud_storage_timeout_cnt += db_select("cyc");
            }
        } else if (cloud_storage_up_state == CLOUD_STORAGE_STOPING) { //如果云存停止中，只发通知
            if (is_same_wakeup_status(wakeup_status) &&  wakeup_status == PIR_WAKEUP) {
                printf("\n >>>>>>%s %d is_same_wakeup_status(wakeup_status) = 1 &&  wakeup_status == PIR_WAKEUP \n", __func__, __LINE__);
                goto __exit;
            }
            set_avsdk_push_event(wakeup_status, 1);
        } else if (cloud_storage_up_state == CLOUD_STORAGE_STARTING) {
            //如果云存还没开始，不能再一次发通知,使用wait_completion等待平台连接成功再上报
            if (is_same_wakeup_status(wakeup_status) &&  wakeup_status == PIR_WAKEUP) {
                printf("\n >>>>>>%s %d is_same_wakeup_status(wakeup_status) = 1 &&  wakeup_status == PIR_WAKEUP \n", __func__, __LINE__);
                goto __exit;
            }
            printf("\n >>>>>>>>>cloud_storage_up_state == CLOUD_STORAGE_STARTING backup wakeup_status\n");
            WAKEUP_STATUS_BACKUP *backup_info = (WAKEUP_STATUS_BACKUP *)calloc(1, sizeof(WAKEUP_STATUS_BACKUP));
            backup_info->wakeup_status = wakeup_status;
            backup_info->uploading = 1;
            backup_info->id = wait_completion(wakeup_status_backup_cb_condition, wakeup_status_backup_cb, backup_info);

        }
    }

__exit:
    os_mutex_post(&cloud_storage_mutex);

}


int cloud_storage_video_write(int type, u8 is_key_frame, int timestamp, char *buffer, int len)
{
    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;



    os_mutex_pend(&cloud_media_info_list_mutex, 0);

    if (cloud_storage_up_state == CLOUD_STORAGE_STOPING) {
        os_mutex_post(&cloud_media_info_list_mutex);
        return 0;
    }
//    if((cloud_storage_up_state == CLOUD_STORAGE_STOP || cloud_storage_up_state == CLOUD_STORAGE_IDLE) &&list_empty(&cloud_media_info_list_head)){
//        is_wait_iframe = 1;
//        audio_frame_start = 0;
//
//    }

    if (is_wait_iframe && !is_key_frame) {
        os_mutex_post(&cloud_media_info_list_mutex);
        return 0;
    }
#ifdef CLOUD_STORAGE_DEBUG
    if (is_wait_iframe && is_key_frame) {
        ASSERT(list_empty(&cloud_media_info_list_head), "cloud_media_info_list_head no empty");
    }
#endif // CLOUD_STORAGE_DEBUG

    if (cloud_storage_up_state == CLOUD_STORAGE_STOP || cloud_storage_up_state == CLOUD_STORAGE_IDLE) {
        int Iframe_cnt = 0;

        //printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>net_video_rec_get_fps() *3.5 = %d\n",(int)(net_video_rec_get_fps() * 3.5));
        if ((video_frame_count + 1) >= (int)(net_video_rec_get_fps() * 3.5)) {
            // printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>video_frame_count = %d net_video_rec_get_fps() *3.5 = %d\n",video_frame_count,(int)(net_video_rec_get_fps() * 3.5));
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head, entry) {
                if (Iframe_cnt == 0) {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        Iframe_cnt = 1;
                    }
                } else {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        break;
                    }
                }
                if (p->type == H264_TYPE_VIDEO) {
                    video_frame_count--;
                }
                list_del(&p->entry);
                free(p);
            }
#ifdef CLOUD_STORAGE_DEBUG
            ASSERT(!list_empty(&cloud_media_info_list_head), "\n cloud_media_info_list_head empty  %s %d\n ", __func__, __LINE__);
            ASSERT(Iframe_cnt, "\n lost frame Must be Iframe_cnt ==%d  video_frame_count = %d\n", Iframe_cnt, video_frame_count);
#endif // CLOUD_STORAGE_DEBUG
        }
    }

    if (cloud_storage_up_state == CLOUD_STORAGE_STARTING) {
        int Iframe_cnt = 0;

        //printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>net_video_rec_get_fps() *3.5 = %d\n",(int)(net_video_rec_get_fps() * 3.5));
        if ((video_frame_count + 1) >= (int)(net_video_rec_get_fps() * 6)) {
            // printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>video_frame_count = %d net_video_rec_get_fps() *3.5 = %d\n",video_frame_count,(int)(net_video_rec_get_fps() * 3.5));
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head, entry) {
                if (Iframe_cnt == 0) {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        Iframe_cnt = 1;
                    }
                } else {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        break;
                    }
                }
                if (p->type == H264_TYPE_VIDEO) {
                    video_frame_count--;
                }
                list_del(&p->entry);
                free(p);
            }
#ifdef CLOUD_STORAGE_DEBUG
            ASSERT(!list_empty(&cloud_media_info_list_head), "\n cloud_media_info_list_head empty  %s %d\n ", __func__, __LINE__);
            ASSERT(Iframe_cnt, "\n lost frame Must be Iframe_cnt ==%d  video_frame_count = %d\n", Iframe_cnt, video_frame_count);
#endif // CLOUD_STORAGE_DEBUG
        }
    }

    info = (CLOUD_MEDIA_INFO *)calloc(1, sizeof(CLOUD_MEDIA_INFO) + len); //lbuf内申请一块空间
    if (info) {
        info->type = type;
        info->is_key_frame = is_key_frame;
        info->difftime = timestamp;
        info-> len = len;
        memcpy(&info->data[0], buffer, len);
        list_add_tail(&info->entry, &cloud_media_info_list_head);
    } else {
#ifdef CLOUD_STORAGE_DEBUG
        ASSERT(0, "\n %s %d no mem \n", __func__, __LINE__);
#endif // CLOUD_STORAGE_DEBUG
    }

#ifdef CLOUD_STORAGE_DEBUG
    if (cloud_storage_up_state != CLOUD_STORAGE_STOPING && cloud_storage_up_state != CLOUD_STORAGE_START) {
        p = list_first_entry(&cloud_media_info_list_head, CLOUD_MEDIA_INFO, entry);
        ASSERT(p->is_key_frame, "\n list_first_entry no I frame %s %d\n", __func__, __LINE__);
    }
#endif // CLOUD_STORAGE_DEBUG

    video_frame_count++;
    is_wait_iframe = 0;
    audio_frame_start = 1;
    os_mutex_post(&cloud_media_info_list_mutex);
    return len;
}

int cloud_storage_audio_write(int type, int timestamp, char *buffer, int len)
{
    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;
    os_mutex_pend(&cloud_media_info_list_mutex, 0);
    if (cloud_storage_up_state == CLOUD_STORAGE_STOPING) {
        os_mutex_post(&cloud_media_info_list_mutex);
        return 0;
    }
    if (!audio_frame_start) {
        os_mutex_post(&cloud_media_info_list_mutex);
        return 0;
    }
#ifdef CLOUD_STORAGE_DEBUG
    if (cloud_storage_up_state != CLOUD_STORAGE_STOPING && cloud_storage_up_state != CLOUD_STORAGE_START) {
        ASSERT(!list_empty(&cloud_media_info_list_head), "cloud_storage_audio_write cloud_media_info_list_head  empty");
        p = list_first_entry(&cloud_media_info_list_head, CLOUD_MEDIA_INFO, entry);
        ASSERT(p->is_key_frame, "\n list_first_entry no I frame %s %d  \n", __func__, __LINE__);
    }
#endif // CLOUD_STORAGE_DEBUG

    info = (CLOUD_MEDIA_INFO *)calloc(1, sizeof(CLOUD_MEDIA_INFO) + len); //lbuf内申请一块空间
    if (info) {
        info->type = type;
        info->is_key_frame = 0;
        info->difftime = timestamp;
        info-> len = len;
        memcpy(&info->data[0], buffer, len);
        list_add_tail(&info->entry, &cloud_media_info_list_head);
    } else {
#ifdef CLOUD_STORAGE_DEBUG
        ASSERT(0, "\n %s %d no mem \n", __func__, __LINE__);
#endif // CLOUD_STORAGE_DEBUG
    }
    os_mutex_post(&cloud_media_info_list_mutex);
    return len;
}
void cloud_storage_uploading(int uploading)
{
    int timestamp = 0;
    int64_t ret = 0;
    CLOUD_MEDIA_INFO *p = NULL, *n = NULL;

    if (!uploading) {
        do {
            os_mutex_pend(&cloud_media_info_list_mutex, 0);
            if (get_cloud_storage_up_state() != CLOUD_STORAGE_START) {
                printf("\n 111 time3 =  %d\n", timer_get_ms());
                os_mutex_post(&cloud_media_info_list_mutex);
                break;
            }
            if (list_empty(&cloud_media_info_list_head)) {
                os_mutex_post(&cloud_media_info_list_mutex);
                msleep(10);
                continue;
            }
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head, entry) {
                list_del(&p->entry);
                break;
            }
            if (p) {
                if (p->type == H264_TYPE_VIDEO) {
                    cloud_storage_fps_cnt++;
                }
                free(p);
            }

            os_mutex_post(&cloud_media_info_list_mutex);
        } while (1);
    } else {

        do {
            os_mutex_pend(&cloud_media_info_list_mutex, 0);
            if (get_cloud_storage_up_state() != CLOUD_STORAGE_START) {
                printf("\n222 time3 =  %d\n", timer_get_ms());
                os_mutex_post(&cloud_media_info_list_mutex);
                break;
            }
            if (list_empty(&cloud_media_info_list_head)) { //查询LBUF内是否有数据帧
                os_mutex_post(&cloud_media_info_list_mutex);
                msleep(10);
                continue;
            }
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head, entry) {
                list_del(&p->entry);
                break;
            }
            os_mutex_post(&cloud_media_info_list_mutex);
            if (p) {
                if (!cloud_storage_first_frame) {
                    cloud_storage_first_frame = 1;
#ifdef CLOUD_STORAGE_DEBUG
                    if (p->type == H264_TYPE_VIDEO && !p->is_key_frame) {
                        //防止出现第一帧是音频帧或者不是I帧的情况出现，测试打开，生产可以关闭
                        ASSERT(0, "\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>cloud_storage_first_frame err H264 NO I\n");
                    } else if (p->type != H264_TYPE_VIDEO) {
                        ASSERT(0, "\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>cloud_storage_first_frame err  AUDIO frame\n");

                    }
#endif
                    timestamp = p->difftime;
                }
                if (get_cloud_storage_up_state() == CLOUD_STORAGE_START) {
#ifdef CLOUD_STORAGE_DEBUG
                    printf("\n p->difftime - timestamp = %d\n ", p->difftime - timestamp);
#endif
                    if (p->type == H264_TYPE_VIDEO) {

                        ret = avsdk_cs_append_video_v2(cs_hdl,
                                                       0,
                                                       p->is_key_frame,
                                                       e_cs_frame_none,
                                                       p->difftime - timestamp,
                                                       NULL,
                                                       &p->data[0],
                                                       p->len
                                                      );

                        if (ret == -3 || ret == -2 || ret == -6) {
                            printf("\n %s %d ret = %d\n", __func__, __LINE__, ret);
                        }
                        cloud_storage_fps_cnt++;
                    } else {
                        ret =  avsdk_cs_append_audio_v2(cs_hdl,
                                                        0,
                                                        e_cs_frame_none,
                                                        p->difftime - timestamp,
                                                        &p->data[0],
                                                        p->len
                                                       );
                        if (ret == -3 || ret == -2 || ret == -6) {
                            printf("\n %s %d ret = %d\n", __func__, __LINE__, ret);
                        }
                    }
                }
                free(p);
            }

        } while (1);
    }
}

void set_cloud_cloud_storage_end(int uploading)
{
    int64_t ret = 0;
    CLOUD_MEDIA_INFO *p = NULL, *n = NULL;

    if (!list_empty(&cloud_media_info_list_head)) {

        os_mutex_pend(&cloud_media_info_list_mutex, 0);
        list_for_each_entry_safe(p, n, &cloud_media_info_list_head, entry) {
            list_del(&p->entry);
            free(p);
        }
        os_mutex_post(&cloud_media_info_list_mutex);
    }

    if (!uploading) {
        os_mutex_pend(&cloud_storage_mutex, 0);
        video_frame_count = 0;
        is_wait_iframe = 1;
        audio_frame_start = 0;
        cloud_storage_up_state = CLOUD_STORAGE_STOP;
        os_sem_set(&cloud_storage_sem, 0);
        wakeup_status_list_head_clear();
        os_mutex_post(&cloud_storage_mutex);
    } else {
        os_mutex_pend(&cloud_storage_mutex, 0);
        puts("\n avsdk_cs_end 1\n");
        ret  = avsdk_cs_end(&cs_hdl, NULL);
        puts("\n avsdk_cs_end 2\n");
        video_frame_count = 0;
        is_wait_iframe = 1;
        audio_frame_start = 0;
        cloud_storage_up_state = CLOUD_STORAGE_STOP;
        os_sem_set(&cloud_storage_sem, 0);

        wakeup_status_list_head_clear();
        cloud_storage_first_frame = 0;
        os_mutex_post(&cloud_storage_mutex);
    }
}

void cloud_storage_task(void *priv)
{
    const char *file_name = "./xxx/cloud_file";//?éò??áD′μ????t??
    int64_t ret = 0;
    CLOUD_MEDIA_INFO *p = NULL, *n = NULL;


    avsdk_cs_set_log_level(LOG_LEVEL_NONE);

    while (1) {

        ret = os_sem_pend(&cloud_storage_sem, 0);
        if (ret != OS_NO_ERR) {
            msleep(10);
            continue;
        }

        ret = set_avsdk_push_event(wakeup_status, 0);
        os_mutex_pend(&cloud_storage_mutex, 0);
        cloud_storage_up_state = CLOUD_STORAGE_START;
        os_mutex_post(&cloud_storage_mutex);

        if (ret <= 0) {
            cloud_storage_uploading(0);
            set_cloud_cloud_storage_end(0);
        } else {
            puts("\n avsdk_cs_start 1\n");
            cs_start_opt_t opt = {
                .event_id = event_id,
                .start_timems = start_time,
                .channel = 0,
                .memsize = 512 * 1024 + 256 * 1024,
                .upload_way = e_cs_upload_way_memory,
                .file_name = "./cloud_file",
            };
            /*开始云储存*/
            int rc = avsdk_cs_start(&cs_hdl, &opt);
            printf("avsdk_cs_start, rc=%d", rc);
            if (!rc) {
                puts("\n avsdk_cs_start 2\n");
                uint8_t picture_num = 1;
#if (CONFIG_AUDIO_FORMAT_FOR_MIC == AUDIO_FORMAT_AAC)

                avsdk_cs_set_format_v2(cs_hdl, e_cs_encrypt_none, e_cs_stream_h264, net_video_rec_get_fps(),
                                       e_cs_stream_aac,
                                       e_cs_sound_frequeency_16000,
                                       e_cs_sound_depth_16bit,
                                       e_cs_sound_channel_mono,
                                       picture_num,
                                       e_cs_picture_type_thumb);

#else
                avsdk_cs_set_format_v2(cs_hdl, e_cs_encrypt_none, e_cs_stream_h264, net_video_rec_get_fps(),
                                       e_cs_stream_pcm,
                                       e_cs_sound_frequeency_16000,
                                       e_cs_sound_depth_16bit,
                                       e_cs_sound_channel_mono,
                                       picture_num,
                                       e_cs_picture_type_thumb);
#endif



                extern int net_video_rec0_take_photo(u8 * buffer, u32 buffer_len);
                u8 *buffer = calloc(1, 32 * 1024);
                u32 buffer_len = 0;
                if (buffer) {
                    printf("\ntime1 =  %d\n", timer_get_ms());
                    buffer_len = net_video_rec0_take_photo(buffer, 32 * 1024);
                    printf("\ntime2 =  %d\n", timer_get_ms());
                }
                printf("\n buffer_len = %d\n", buffer_len);
                if (buffer_len) {
                    avsdk_cs_append_thumbnail_v2(cs_hdl, buffer + 4, buffer_len, NULL);
                    free(buffer);
                } else {
                    //在这里启动拍照，将照片数据传递
                    avsdk_cs_append_thumbnail_v2(cs_hdl, NULL, 0, NULL);
                }
                cloud_storage_uploading(1);
                set_cloud_cloud_storage_end(1);

            } else {
                puts("\n avsdk_cs_start fail\n");
                cloud_storage_uploading(0);
                set_cloud_cloud_storage_end(0);
            }

        }
        extern void set_system_running(u8 value);
        set_system_running(1);
        post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_PIR_STATUS_GET);
    }
}

int cloud_storage_init()
{
    os_mutex_create(&cloud_storage_mutex);
    os_mutex_create(&list_head_mutex);
    os_mutex_create(&cloud_media_info_list_mutex);
    os_sem_create(&cloud_storage_sem, 0);


    return thread_fork("cloud_storage_task", 8, 0x1000, 0, 0, cloud_storage_task, NULL);
}
//late_initcall(cloud_storage_init);








