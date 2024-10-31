#define _STDIO_H_
#include "xciot_api.h"
#include "xciot_cmd_ipc.h"

#include "app_config.h"
#include "json_c/json.h"
#include "fs.h"
#include "generic/list.h"
#include "simple_mov_unpkg.h"
#include "time.h"
#include "system/init.h"
#include "os/os_compat.h"
#include "doorbell_event.h"



#define CONFIG_PLAYBACK_LIST_ENABLE  1

static const char *g_file_name_prefix[2] = {
    "KEY_",
    "PIR_",
};
#define FILE_NAME_PREFIX_LEN   4;




typedef struct cloud_playback_list_info {
    struct list_head entry;
    char name[32];//文件名
    int32_t day;//用于检索文件名
    uint64_t start_time;
    u32      length;
//    uint64_t file_id;
//    uint64_t thum_fig;

} CLOUD_PLAYBACK_LIST_INFO;

static CLOUD_PLAYBACK_LIST_INFO  cloud_playback_list_info[1024];
static LIST_HEAD(cloud_playback_file_list_head);
static OS_MUTEX cloud_playback_list_mutex;

extern uint64_t covBeijing2UnixTimeStp(struct tm *p);


static CLOUD_PLAYBACK_LIST_INFO *find_empty_cloud_playback_list_info(void)
{
    int i;
    CLOUD_PLAYBACK_LIST_INFO *info = NULL;
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    for (i = 0; i < 1024; i++) {
        if (cloud_playback_list_info[i].start_time == 0) {
            cloud_playback_list_info[i].start_time = !0;
            info = &cloud_playback_list_info[i];
            break;
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
    return info;
}

static void free_used_cloud_playback_list_info(CLOUD_PLAYBACK_LIST_INFO *info)
{
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    info->start_time = 0;
    os_mutex_post(&cloud_playback_list_mutex);
}



int get_utc_time_for_name(struct tm *p, char *name, int len)
{
    char *str = NULL;
    for (u8 i = 0; i < (sizeof(g_file_name_prefix) / sizeof(char *)); i++) {

        str = strstr(name, g_file_name_prefix[i]);
        if (str) {
            break;
        }
    }
    if (!str) {
//        printf("\n name = %s not match g_file_name_prefix\n",name);
        return -1;
    }
    str += FILE_NAME_PREFIX_LEN;
    p->tm_year = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
    str += 4;
    p->tm_mon = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2;
    p->tm_mday = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2 + 1;
    p->tm_hour = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2;
    p->tm_min = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2;
    p->tm_sec = (str[0] - '0') * 10 + (str[1] - '0');
//    printf("time is: %d/%02d/%02d-%02d:%02d:%02d \n",
//                    p->tm_year, p->tm_mon, p->tm_mday, \
//                    p->tm_hour, p->tm_min, p->tm_sec );
    return 0;
}


int cloud_playback_list_clear(void)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&cloud_playback_file_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }
    list_for_each_entry_safe(p, n, &cloud_playback_file_list_head, entry) {
        if (p) {
            list_del(&p->entry);
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}

void cloud_playback_list_get_name_for_start_time(uint64_t start_time, char *name)
{
#if CONFIG_PLAYBACK_LIST_ENABLE

    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&cloud_playback_file_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return;
    }
    list_for_each_entry_safe(p, n, &cloud_playback_file_list_head, entry) {
        if (p) {

            if (start_time == p->start_time) {
                strcpy(name, p->name);
                break;
            }

        }
    }
    os_mutex_post(&cloud_playback_list_mutex);

#endif
}




int cloud_playback_list_get_days(void *__req, void *__rsp)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    ipc_HistoryDays_Req *req = (ipc_HistoryDays_Req *)__req;
    ipc_HistoryDays_Resp *rsp = (ipc_HistoryDays_Resp *)__rsp;
    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    int i;
    int j = sizeof(rsp->days) / sizeof(int32_t);
    rsp->days_count = 0;

    u32  date =   req->year * 100 + (req->month / 10) * 10 + (req->month % 10);
    for (int i = 0; i < j; i++) {
        rsp->days[i] = 0;
    }
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&cloud_playback_file_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }
    list_for_each_entry_safe(p, n, &cloud_playback_file_list_head, entry) {
        if (p) {
//            printf("\ndate =  %d  p->day = %d \n",date,p->day / 100);
            if (date == (p->day / 100)) {
//                printf("\n day = %d\n",p->day % 100);
                for (i = 0; i <= rsp->days_count; i++) {
                    if (!rsp->days[i] || rsp->days[i] == (p->day % 100)) {
                        break;
                    }
                }
                if (!rsp->days[i] && rsp->days_count < (j - 1)) {
                    rsp->days[i] = (p->day % 100);
                    rsp->days_count++;
                    rsp->total++;
                }
            }

        }
    }
    os_mutex_post(&cloud_playback_list_mutex);

//    printf("\n rsp->days_count= %d rsp->total = %d \n",rsp->days_count,rsp->total);
#endif
    return 0;
}

int cloud_playback_list_get(void *__req, void *__rsp)
{
#if CONFIG_PLAYBACK_LIST_ENABLE

    ipc_HistoryDayList_Req *req = (ipc_HistoryDayList_Req *)__req;
    ipc_HistoryDayList_Resp *rsp = (ipc_HistoryDayList_Resp *)__rsp;
    CLOUD_PLAYBACK_LIST_INFO *p, *n;

    int i = 0;
    int j = 0;
    int page_offset = (req->page - 1) * req->page_size;



    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&cloud_playback_file_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }


    if (req->order == 2) {
        list_for_each_entry_reverse_safe(p, n, &cloud_playback_file_list_head, entry) {

            if (p->day == req->day || req->day == 0) {
                if (i == page_offset && page_offset < ((req->page - 1) * req->page_size + req->page_size)) {
                    //printf("\n>>>> p->name = %s\n",p->name);
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].start_time = p->start_time;  //01:05:03
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].length = p->length;
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].file_id = p->start_time;
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].thum_fid = p->start_time;

                    u8 type = 255;
                    if (strstr(p->name, g_file_name_prefix[0])) {
                        type = E_IOT_EVENT_VISUAL_DOORBELL;
                    } else if (strstr(p->name, g_file_name_prefix[1])) {
                        type = E_IOT_EVENT_SENSE_HUMAN;
                    }
                    for (j = 0; i < req->history_type_count; j++) {
                        printf("\ntype = %d req->history_type[%d] = %d\n", type, j, req->history_type[j]);
                        if (type == req->history_type[j] ||  req->history_type[j] == 0) {
                            break;
                        }
                    }
                    if (j == req->history_type_count) {
                        continue;
                    }

                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].history_type = type;
                    page_offset++;
                }
                i++;
            }
        }
    } else {

        list_for_each_entry_safe(p, n, &cloud_playback_file_list_head, entry) {
#if 1
            printf("\n %s %d\n", __func__, __LINE__);
            printf("\n p->day = %d req->day = %d\n", p->day, req->day);
            printf("\n page_offset = %d ((req->page - 1 ) * 20 + 20) = %d\n", page_offset, ((req->page - 1) * 20 + 20));
            printf("\n>>>> p->name = %s\n", p->name);
#endif // 0
            if (p->day == req->day || req->day == 0) {
                if (i == page_offset && page_offset < ((req->page - 1) * req->page_size + req->page_size)) {

                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].start_time = p->start_time * 1000;  //01:05:03
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].length = p->length;
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].file_id = p->start_time;
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].thum_fid = p->start_time;

                    u8 type = 255;
                    if (strstr(p->name, g_file_name_prefix[0])) {
                        type = E_IOT_EVENT_VISUAL_DOORBELL;
                    } else if (strstr(p->name, g_file_name_prefix[1])) {
                        type = E_IOT_EVENT_SENSE_HUMAN;
                    }
                    for (j = 0; j < req->history_type_count; j++) {
                        printf("\ntype = %d req->history_type[%d] = %d\n", type, j, req->history_type[j]);
                        if (type == req->history_type[j] ||  req->history_type[j] == 0) {
                            break;
                        }
                    }
                    if (j == req->history_type_count) {
                        continue;
                    }
                    rsp->historys[page_offset - ((req->page - 1) * req->page_size)].history_type = type; //视频类型
                    page_offset++;
                }
                i++;
            }


        }
    }
    rsp->total = i;
    rsp->historys_count = page_offset - ((req->page - 1) * req->page_size);

    printf("\n req->page = %d\n", req->page);
    printf("\n  rsp->total = %d \n",  rsp->total);
    printf("\n rsp->historys_count = %d \n",  rsp->historys_count);
    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE

    return 0;
}

int cloud_playback_list_add(char *path)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    if (!storage_device_ready()) {
        return 0;
    }
    void *fd = fopen(path, "r");
    if (!fd) {
        return 0;
    }

    if (!is_vaild_mov_file(fd)) {
        fclose(fd);
        return 0;
    }

    extern int read_time_scale_dur(FILE * file_fp, struct __mov_unpkg_info * info);
    struct __mov_unpkg_info unpkg_info = {0};;
    if (read_time_scale_dur(fd, &unpkg_info)) {
        fclose(fd);
        return 0;
    }
    CLOUD_PLAYBACK_LIST_INFO *info = find_empty_cloud_playback_list_info();
    if (!info) {
        printf("\n %s %d\n", __func__, __LINE__);
        fclose(fd);
        return 0;
    }
    info->length = unpkg_info.durition / unpkg_info.scale;

    char name[128];
    int len = fget_name(fd, name, sizeof(name));
    snprintf(info->name, "%s", name);

    struct tm p;
    get_utc_time_for_name(&p, info->name, strlen(info->name));
    char day[32];
    snprintf(day, sizeof(day), "%d%02d%02d", p.tm_year, p.tm_mon, p.tm_mday);
    info->day = atol(day);
    info->start_time = covBeijing2UnixTimeStp(&p);
    //info->file_id = info->thum_fig =  info->start_time;
#if 0
    printf("\n>>>>>>>>>>>>>>>>>> info->name = %s\n", info->name);
    printf("\n>>>>>>>>>>>>>>>>>> info->length = %d \n", info->length);
    printf("\n>>>>>>>>>>>>>>>>>> info->start_time = %llu \n", info->start_time);
    printf("\n>>>>>>>>>>>>>>>>>> info->day = %llu \n", info->day);
#endif
    fclose(fd);

    os_mutex_pend(&cloud_playback_list_mutex, 0);
    list_add(&info->entry, &cloud_playback_file_list_head);
    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE

    return 0;
}



int cloud_playback_list_remove(void *__req, void *__rsp)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    ipc_HistoryDel_Req *req = (ipc_HistoryDel_Req *)__req;
    ipc_HistoryDel_Resp *rsp = (ipc_HistoryDel_Resp *)__rsp;
    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    uint64_t file_id = 0;
    char name[128];
    int i;
    int total_id = req->file_id_count;
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&cloud_playback_file_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }
    list_for_each_entry_safe(p, n, &cloud_playback_file_list_head, entry) {
        if (p) {
            file_id = p->start_time;

            for (i = 0; i < req->file_id_count ; i++) {

                if (file_id == req->file_id[i]) {
                    list_del(&p->entry);
                    snprintf(name, sizeof(name), "%s%s", get_rec_path_1(), p->name);
                    void *fd = fopen(name, "r");
                    if (fd) {
                        fdelete(fd);
                    }
                    free_used_cloud_playback_list_info(p);
                    total_id--;
                    break;
                }
            }
            if (0 == total_id) {
                break;
            }
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}




int cloud_playback_list_local_remove(char *path)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    uint64_t file_id = 0;
    char name[128];
    int i;
    if (!storage_device_ready()) {
        return 0;
    }
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&cloud_playback_file_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }
    list_for_each_entry_safe(p, n, &cloud_playback_file_list_head, entry) {
        if (p) {
            if (strstr(path, p->name)) {
                list_del(&p->entry);
                void *fd = fopen(name, "r");
                if (fd) {
                    fdelete(path);
                }
                free_used_cloud_playback_list_info(p);
            }
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}




void cloud_playback_list_task(void *priv)
{
    int len;
    struct tm p;
    struct vfscan *fs = NULL;
    void *fd = NULL;
    int length = 0;



    while (!storage_device_ready()) {
        msleep(10);
    }

    fs = fscan(CONFIG_REC_PATH_1, "-tAVI -st");
    if (!fs) {
        printf("\n %s %d\n", __func__, __LINE__);
        return;
    }
    fd = fselect(fs, FSEL_LAST_FILE, 0);


    while (1) {
        if (!fd) {
            break;
        }
#if 0
        struct __mov_unpkg_info unpkg_info = {0};
        if (!flen(fd) || !is_vaild_mov_file(fd)) {
            fclose(fd);
            fd = fselect(fs, FSEL_PREV_FILE, 0);
            continue;
        }


        if (read_time_scale_dur(fd, &unpkg_info) ||  read_time_scale_dur(fd, &unpkg_info) || read_height_and_length(fd, &unpkg_info)) {
            fclose(fd);
            fd = fselect(fs, FSEL_PREV_FILE, 0);
            continue;
        }
        if (unpkg_info.scale) {
            length =  unpkg_info.durition / unpkg_info.scale;
        }
#else
        printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>1\n");



        int ret =  avi_net_playback_unpkg_init(fd, 1);
        if (ret) {
            ASSERT(0, "\n >>>>>>>>>>>>>>>>%s %d\n", __func__, __LINE__);
        }
        if (!is_vaild_avi_file(fd, 1)) {
            fd = fselect(fs, FSEL_PREV_FILE, 0);
            fclose(fd);
            avi_net_unpkg_exit(fd, 1);
            continue;
        }


        length = avi_get_file_time(fd, 1);

        avi_net_unpkg_exit(fd, 0);

#endif
        CLOUD_PLAYBACK_LIST_INFO *info = find_empty_cloud_playback_list_info();
        if (!info) {
            printf("\n %s %d\n", __func__, __LINE__);
            fclose(fd);
            break;
        }


        info->length = length;



        char name[128];
        len = fget_name(fd, name, sizeof(name));
        snprintf(info->name, "%s", name);
        fclose(fd);

        if (get_utc_time_for_name(&p, info->name, strlen(info->name))) {
            free_used_cloud_playback_list_info(info);
            fd = fselect(fs, FSEL_PREV_FILE, 0);
            continue;
        }




        char day[32];
        snprintf(day, sizeof(day), "%d%02d%02d", p.tm_year, p.tm_mon, p.tm_mday);
        info->day = atol(day);
        info->start_time = covBeijing2UnixTimeStp(&p);
        //info->file_id = info->thum_fig =  info->start_time;
#if 1
        printf("\n>>>>>>>>>>>>>>>>>> info->name = %s\n", info->name);
        printf("\n>>>>>>>>>>>>>>>>>> info->length = %d \n", info->length);
        printf("\n>>>>>>>>>>>>>>>>>> info->start_time = %llu \n", info->start_time);
        printf("\n>>>>>>>>>>>>>>>>>> info->day = %llu \n", info->day);
#endif
        os_mutex_pend(&cloud_playback_list_mutex, 0);
        list_add_tail(&info->entry, &cloud_playback_file_list_head);
        os_mutex_post(&cloud_playback_list_mutex);

        fd = fselect(fs, FSEL_PREV_FILE, 0);

        extern void vPortYield(void);
        vPortYield();

    }

    fscan_release(fs);
}

int cloud_playback_list_init()
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    os_mutex_create(&cloud_playback_list_mutex);
    thread_fork("cloud_playback_list_task", 8, 0x1000, 0, 0, cloud_playback_list_task, NULL);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}

//late_initcall(cloud_playback_list_init);
