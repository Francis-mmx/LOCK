#include "fs.h"
#include "server/server_core.h"
#include "system/includes.h"
#include "os/os_compat.h"
#include "server/net_server.h"
#include "server/ctp_server.h"
#include "http/http_server.h"
#include "storage_device.h"
#include "server/vunpkg_server.h"
#include "fs/fs.h"
#include "lwip/sockets.h"
#include "server/simple_mov_unpkg.h"
#include "time.h"
#include "sys/time.h"
#include "common/common.h"
#include "server/simple_avi_unpkg.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
//#include "rt_stream_pkg.h"
/* #include "action.h" */
#include "net_config.h"
#include "app_config.h"

extern void *memmem(const void *__haystack, size_t __haystacklen,
                    const void *__needle, size_t __needlelen);

extern int http_virfile_reg(const char *path, const char *contents, unsigned long len);



#define VIDEO_FILE_LIST_JSON_HEAD  	"{\"file_list\":["
#define VIDEO_FILE_LIST_JSON_END  	"]}"
#define VIDEO_JSON_MEM   "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\",\"c\":\"%d\",\"e\":\"%ld\"},"
#define VIDEO_JSON_APP   "{\"op\":\"NOTIFY\",\"param\":{\"status\":\"%d\",\"desc\":\"{\\\"y\\\":%d,\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"d\\\":\\\"%d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"p\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\",\\\"e\\\":\\\"%ld\\\"}\"}}"
#define PICTURE_JSON_MEM   "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\",\"c\":\"%d\",\"e\":\"%ld\"},"
#define PICTURE_JSON_APP    "{\"op\":\"NOTIFY\",\"param\":{\"desc\":\"{\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\",\\\"e\\\":\\\"%ld\\\"}\"}}"

#define INFO_LEN 160
#if (defined CONFIG_ENABLE_VLIST)
#define MAX_NUM  640
#else
#define MAX_NUM  2
#endif

//#define VF_LIST_CHECK


const char __fs_arg[3][15] = {  "-tMOVAVI -st",
                                "-tJPG  -st",
                                "-tMOVAVIJPG -st"
                             };


struct file_info {
    struct list_head entry;
    u8 addlist: 1; //1 已在链表
    u16 id: 15;
    u16 len;
    u8(*fd)[INFO_LEN];
    u32 ftime;//2^32 = 4294967296 --> 2106/2/7 14:28:16, 最多纪录到2106/2/7号
};

#define FILE_INFO_SIZE	sizeof(struct file_info)

struct media_file_info {
    FILE *fd;
    u32  vaild;
    int _attr;
    struct vfs_attr attr;
    struct __mov_unpkg_info info;
    u8 is_emf;//1 紧急文件
    u8 channel; // 前后视区分
    u8 type;// 1MOV视频  2 图片 3 AVI视频  4，，，
    u8 namebuf[64];
    u16 height; //use in jpg
    u16 width; // use in jpg
};



static struct list_head forward_file_list_head = {NULL, NULL};
static struct list_head behind_file_list_head = {NULL, NULL};
static u32 initing = 0;
static u32 mutex_init = 0;
static u8 forward_file_mem[MAX_NUM + 2][INFO_LEN] __attribute__((aligned(32)));

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
static u8 behind_file_mem[MAX_NUM + 2][INFO_LEN] __attribute__((aligned(32)));
static struct file_info file_info_tab[MAX_NUM * 2]  __attribute__((aligned(32)));
#else
static struct file_info file_info_tab[MAX_NUM]  __attribute__((aligned(32)));
#endif

static char emf_path[64] __attribute__((aligned(32)));
static OS_MUTEX file_list_mutex;
static OS_MUTEX file_list_read_mutex;

int send_json(struct media_file_info *__info, u32 status);

static void file_info_clear(void)
{
    memset(file_info_tab, 0, sizeof(file_info_tab));
#ifdef CONFIG_ENABLE_VLIST
    memset(forward_file_mem, 0, (MAX_NUM + 2) * INFO_LEN);
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    memset(behind_file_mem, 0, (MAX_NUM + 2) * INFO_LEN);
#endif
#endif
}
static char *file_is_emf(char *dir, u8 is_emf)
{
    if (is_emf) {
#ifdef CONFIG_EMR_DIR_ENABLE
        memset(emf_path, 0, sizeof(emf_path));
        strcpy((char *)emf_path, dir);
        strcat((char *)emf_path, CONFIG_EMR_REC_DIR);
        return emf_path;
#else
        return dir;
#endif
    } else {
        return dir;
    }
}
static u8 *__find_emtry_block_f(struct file_info *info)
{
    int i = info->id - 1;
    u8 *str = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    memset(forward_file_mem[i], 0, INFO_LEN);
    str = forward_file_mem[i];
    os_mutex_post(&file_list_mutex);
    return str;
}
static u8 *__find_emtry_block_b(struct file_info *info)
{
    int i = info->id - 1;
    u8 *str = NULL;
    os_mutex_pend(&file_list_mutex, 0);
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO2_ENABLE)
    memset(behind_file_mem[i], 0, INFO_LEN);
    str = behind_file_mem[i];
#endif
    os_mutex_post(&file_list_mutex);
    return str;
}
static struct file_info *file_info_find(char behind)
{
    int i;
    int num = 0;
    u32 time = 0xffffffff;
    struct file_info *info = NULL;
    int end = behind ? 2 : 1;
    int start = behind ? MAX_NUM : 0;

    for (i = start; i < end * MAX_NUM; i++) {
        info = (struct file_info *)&file_info_tab[i];
        if (info->id == 0) {
            info->id = i - start + 1;
            ASSERT((info->id > 0 && info->id <= MAX_NUM), "info->id err !!!!");
            return &file_info_tab[i];
        } else if (info->ftime < time) {
            time = info->ftime;
            num = i;
        }
    }
    info->id = num - start + 1;
    ASSERT((info->id > 0 && info->id <= MAX_NUM), "info->id err !!!!");

    return &file_info_tab[num];
}

static size_t forward_write_block(u8 *buffer, size_t len)
{
    struct file_info *info = (struct file_info *)file_info_find(0);

    if (info == NULL) {
        return -1;
    }

    const char *str = "\"e\":\"";
    char *pstr;
    pstr = strstr((const char *)buffer, str);
    if (pstr) {
        pstr += strlen(str);
        info->ftime = strtol((const char *)pstr, NULL, 10);
    } else {
        info->ftime = 0;
    }
    if (info->addlist) {
        info->len = len;
        memset(info->fd, 0, INFO_LEN);
        memcpy(info->fd, buffer, len);
        return len;
    }

    u8 *p = __find_emtry_block_f(info);
    memcpy(p, buffer, len);
    info->len = len;
    info->fd = (u8(*)[INFO_LEN])p;
    info->addlist = 1;

    /*printf("info->fd=%s   info->len=%d\n", info->fd, info->len); */
    os_mutex_pend(&file_list_mutex, 0);
    list_add_tail(&info->entry, &forward_file_list_head);
    os_mutex_post(&file_list_mutex);
    return len;
}

static int forward_remove_block(const char *fname)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;

    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &forward_file_list_head) {
        if (pos) {
            __info = list_entry(pos, struct file_info, entry);
            if (__info) {
                if (strstr((const char *)__info->fd, fname)) {
                    memset(__info->fd, 0, __info->len);
                    list_del(&__info->entry);
                    __info->id = 0;
                    __info->addlist = 0;
                    /*free(__info);*/
                    os_mutex_post(&file_list_mutex);
                    return 0;
                }
            }
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}
static int forward_change_block(const char *fname, char attr)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    char *str = NULL;

    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &forward_file_list_head) {
        if (pos) {
            __info = list_entry(pos, struct file_info, entry);
            if (__info) {
                if (memmem(__info->fd, __info->len, fname, strlen((const char *)fname))) {
                    str = memmem(__info->fd, __info->len, "\"y\":", 4);
                    printf("attr=%c \n", attr);
                    if (str) {
                        *(str + 4) = attr;
                        os_mutex_post(&file_list_mutex);
                        return 0;
                    }
                }
            }
        }

    }
    os_mutex_post(&file_list_mutex);
    return -1;
}

static int forward_remove_block_all()
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &forward_file_list_head) {
        if (pos) {
            __info = list_entry(pos, struct file_info, entry);
            if (__info) {
                list_del(&__info->entry);
                memset(__info->fd, 0, INFO_LEN);
                __info->id = 0;
                __info->addlist = 0;
                //free(__info);
            }
        }
    }
    os_mutex_post(&file_list_mutex);
    return 0;

}

static size_t behind_write_block(u8 *buffer, size_t len)
{
    struct file_info *info = (struct file_info *)file_info_find(1);
    if (info == NULL) {
        return -1;
    }

    const char *str = "\"e\":\"";
    char *pstr;
    pstr = strstr((const char *)buffer, str);
    if (pstr) {
        pstr += strlen(str);
        info->ftime = strtol((const char *)pstr, NULL, 10);
    } else {
        info->ftime = 0;
    }
    if (info->addlist) {
        info->len = len;
        memset(info->fd, 0, INFO_LEN);
        memcpy(info->fd, buffer, len);
        return len;
    }

    u8 *p = __find_emtry_block_b(info);
    memcpy(p, buffer, len);
    info->len = len;
    info->fd = (u8(*)[INFO_LEN])p;
    info->addlist = 1;

    /* printf("info->fd=%s   info->len=%d\n", info->fd, info->len); */
    os_mutex_pend(&file_list_mutex, 0);
    list_add_tail(&info->entry, &behind_file_list_head);

    os_mutex_post(&file_list_mutex);
    return len;

}

static int behind_remove_block(const char *fname)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        if (pos) {
            __info = list_entry(pos, struct file_info, entry);
            if (__info) {
                if (strstr((const char *)__info->fd, fname)) {
                    memset(__info->fd, 0, __info->len);
                    list_del(&__info->entry);
                    __info->id = 0;
                    __info->addlist = 0;
                    //free(__info);
                    os_mutex_post(&file_list_mutex);
                    return 0;
                }
            }
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}

static int behind_change_block(const char *fname, char attr)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;

    char *str = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        if (pos) {
            __info = list_entry(pos, struct file_info, entry);
            if (__info) {
                if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
                    str = memmem(__info->fd, __info->len, "\"y\":", 4);
                    if (str) {
                        *(str + 4) = attr;
                        os_mutex_post(&file_list_mutex);
                        return 0;
                    }
                }
            }
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}




static int behind_remove_block_all()
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        if (pos) {
            __info = list_entry(pos, struct file_info, entry);
            if (__info) {
                list_del(&__info->entry);
                memset(__info->fd, 0, INFO_LEN);
                __info->id = 0;
                __info->addlist = 0;
                //free(__info);
            }
        }
    }
    os_mutex_post(&file_list_mutex);
    return 0;


}

void FILE_REMOVE_ALL()
{

    if (FILE_INITIND_CHECK()) {
        forward_remove_block_all();

#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block_all();
#endif
    }
    file_info_clear();
}


void __FILE_LIST_INIT(u8 is_forward, u32 file_num)
{

    FILE *fd = NULL;
    int ret = 0;
    u32 count = 0;
    u32 flag = 0;
    char res[32] = {0};
    char path[64];
    struct vfscan *fs = NULL;
    struct media_file_info media_info;

#ifdef CONFIG_ENABLE_VLIST
    if (is_forward) {
        strcpy((char *)res, get_rec_path_1());
        INIT_LIST_HEAD(&forward_file_list_head);
        flag = 0;
    } else {
        strcpy((char *)res, get_rec_path_2());
        INIT_LIST_HEAD(&behind_file_list_head);
        flag = 1;
    }


#ifdef CONFIG_NET_PKG_H264
    fs = fscan(res, "-tMOVJPG -st");
#else
#ifdef CONFIG_NET_PKG_JPEG
    fs = fscan(res, "-tAVIJPG -st");
#else
    fs = fscan(res, "-tJPG -st");
#endif
#endif

    if (fs == NULL) {
        return;
    }
    while (1) {
        if (fd == NULL) {
            fd = fselect(fs, FSEL_FIRST_FILE, 0);
        } else {
            fd = fselect(fs, FSEL_NEXT_FILE, 0);
        }
        if (fd == NULL || count >= file_num) {
            break;
        }
        media_info.fd = fd;
        media_info.channel = flag;
        media_info.is_emf = 0;

        send_json(&media_info, 0x2);
        if (media_info.fd) {
            fclose(fd);
            media_info.fd = NULL;
        }
        count++;
        if (count > MAX_NUM) {
            break;
        }
        if (ret <= 0) {
            continue;
        }
    }

#ifdef  CONFIG_EMR_DIR_ENABLE
    //扫描紧急文件夹
    fscan_release(fs);//先释放上一个
    memset(res, 0, sizeof(res));
    if (is_forward) {
        strcpy((char *)res, get_rec_path_1());
        strcat(res, CONFIG_EMR_REC_DIR);
        flag = 0;
    } else {
        strcpy((char *)res, get_rec_path_2());
        strcat(res, CONFIG_EMR_REC_DIR);
        flag = 1;
    }
#ifdef CONFIG_NET_PKG_H264
    fs = fscan(res, "-tMOVJPG -st");
#else
#ifdef CONFIG_NET_PKG_JPEG
    fs = fscan(res, "-tAVIJPG -st");
#else
    fs = fscan(res, "-tJPG -st");
#endif
#endif

    if (fs == NULL) {
        return;
    }
    count = 0;
    while (1) {
        if (fd == NULL) {
            fd = fselect(fs, FSEL_FIRST_FILE, 0);
        } else {
            fd = fselect(fs, FSEL_NEXT_FILE, 0);
        }
        if (fd == NULL || count >= file_num) {
            break;
        }
        media_info.fd = fd;
        media_info.channel = flag;
        media_info.is_emf = 1;

        send_json(&media_info, 0x2);
        if (media_info.fd) {
            fclose(fd);
            media_info.fd = NULL;
        }
        count++;
        if (count > MAX_NUM) {
            break;
        }
        if (ret <= 0) {
            continue;
        }
    }
#endif

close1:

    fscan_release(fs);
#endif
}


void FILE_DELETE(char *__fname, u8 create_file)
{
    int err;
    u8 fname[128] = {0};
    static u32 last_ms = 0;
    u32 now_ms;
    u8 one_sec = true;

    //删除全部文件会连续频繁调用函数，修改在频繁调用时候相差1秒之后才能重新更新文件
    now_ms = timer_get_ms();
    if (last_ms && now_ms > last_ms && (now_ms - last_ms) >= 1000) {
        last_ms = now_ms - (now_ms - last_ms) % 1000;
    } else if (last_ms && now_ms > last_ms) {
        one_sec = false;
    } else if (now_ms <= last_ms || !last_ms) {
        last_ms = now_ms;
    }


#ifdef CONFIG_ENABLE_VLIST
    os_mutex_pend(&file_list_read_mutex, 0);
    if (FILE_INITIND_CHECK()) {
        if (create_file && one_sec) {
            sys_timeout_add(NULL, (void (*)(void *))FILE_GEN, 1000);
        }
        os_mutex_post(&file_list_read_mutex);
        return;
    }

    if (__fname) {
        strcpy((char *)fname, __fname);
        printf("FILE_DELETE file : %s \n", fname);
        forward_remove_block((const char *)fname);
#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block((const char *)fname);
#endif
    } else {
        printf("FILE_DELETE all file \n");
        forward_remove_block_all();
#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block_all();
#endif
        file_info_clear();
    }
    if (create_file && one_sec) {
        sys_timeout_add(NULL, (void (*)(void *))FILE_GEN, 1000);
    }
    os_mutex_post(&file_list_read_mutex);
#endif
}

void FILE_CHANGE_ATTR(const char *fname, char attr)
{
    if (FILE_INITIND_CHECK()) {
        return;
    }

    if (fname == NULL) {
        printf("file name is NULL\n");
        return ;
    }

    if (strstr(fname, get_rec_path_1())) {
        forward_change_block(fname, attr);
    } else if (strstr(fname, get_rec_path_2())) {
#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_change_block(fname, attr);
#endif
    }

}


void FILE_GEN(void)
{
    FILE *fd = NULL;
    FILE *fd2 = NULL;
    u8 *str_ptr = NULL;
    u8 *str_ptr2 = NULL;
    char path[64];
    short enter = 0xa0d;
    int j = 0, i;
    int len;
    int readlen;
    int ret;
    char *buff = NULL;
    int redo_cnt = 0;
    json_object *obj;
    struct list_head *pos = NULL, *node = NULL;
    struct  file_info *__info = NULL;
    int last_addr = 0;

#ifdef CONFIG_ENABLE_VLIST
    if (dev_online(SDX_DEV)) {

        os_mutex_pend(&file_list_mutex, 0);

        sprintf(path, "%s%s", get_rec_path_1(), "vf_list.txt");
        fd = fopen(path, "r");
        if (fd) {
            fdelete(fd);
        }
        fd = fopen(path, "w+");
        if (!fd) {
            printf("open err %s !!!\n\n", path);
            goto exit1;
        }

        fwrite(fd, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
        fwrite(fd, &enter, sizeof(enter));
        list_for_each_safe(pos, node, &forward_file_list_head) {
            if (pos) {
                __info = list_entry(pos, struct file_info, entry);
                if (__info) {
                    fwrite(fd, __info->fd, strlen((const char *)__info->fd));
                    fwrite(fd, &enter, sizeof(enter));
                    last_addr = fpos(fd);
                }
            }
        }
        if (last_addr) {
            fseek(fd, last_addr - (sizeof(enter) + 1), SEEK_SET);
        }
        fwrite(fd, &enter, sizeof(enter));
        fwrite(fd, VIDEO_FILE_LIST_JSON_END, strlen(VIDEO_FILE_LIST_JSON_END));
        fclose(fd);
        last_addr = 0;
        pos = NULL;
        node = NULL;
        __info = NULL;

#ifdef VF_LIST_CHECK
redo1:
        //校验
        sprintf(path, "%s%s", get_rec_path_1(), "vf_list.txt");
        fd = fopen(path, "r");
        if (fd) {
            len = flen(fd);
            if (!buff) {
                buff = zalloc(len + 1);
            }
            if (!buff) {
                fclose(fd);
            } else {
                ret = fread(fd, buff, len);
                if (ret != len) {
                    printf("read list1 file err : %s \n\n", path);
                }
                fclose(fd);
                obj = json_tokener_parse(buff);
                /*printf("check list1 file : \n%s\n", buff);*/
                if (!obj) {
                    printf("check list1 file err , retry again \n");
                    redo_cnt++;
                    if (redo_cnt <= 3) {
                        msleep(100);
                        goto redo1;
                    }
                    redo_cnt = 0;
                    /*ASSERT(0,"check list1 file err !!!!!!!!! ");*/
                }
                json_object_put(obj);
            }
        }
        if (buff) {
            free(buff);
            buff = NULL;
        }
#endif
exit1:

#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
        sprintf(path, "%s%s", get_rec_path_2(), "vf_list.txt");
        fd2 = fopen(path, "r");
        if (fd2) {
            fdelete(fd2);
        }
        fd2 = fopen(path, "w+");
        if (!fd2) {
            printf("open err %s !!!\n\n", path);
            goto exit2;
        }

        fwrite(fd2, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
        fwrite(fd2, &enter, sizeof(enter));
        list_for_each_safe(pos, node, &behind_file_list_head) {
            if (pos) {
                __info = list_entry(pos, struct file_info, entry);
                if (__info) {
                    fwrite(fd2, __info->fd, strlen((const char *)__info->fd));
                    fwrite(fd2, &enter, sizeof(enter));
                    last_addr = fpos(fd2);
                }
            }
        }
        if (last_addr) {
            fseek(fd2, last_addr - (sizeof(enter) + 1), SEEK_SET);
        }
        fwrite(fd2, &enter, sizeof(enter));
        fwrite(fd2, VIDEO_FILE_LIST_JSON_END, strlen(VIDEO_FILE_LIST_JSON_END));
        fclose(fd2);
        last_addr = 0;
        pos = NULL;
        node = NULL;

#ifdef VF_LIST_CHECK
redo2:
        //校验
        sprintf(path, "%s%s", get_rec_path_2(), "vf_list.txt");
        fd = fopen(path, "r");
        if (fd) {
            len = flen(fd);
            if (!buff) {
                buff = zalloc(len + 1);
            }
            if (!buff) {
                fclose(fd);
            } else {
                ret = fread(fd, buff, len);
                if (ret != len) {
                    printf("read list2 file err : %s \n\n", path);
                }
                fclose(fd);
                obj = json_tokener_parse(buff);
                /*printf("check list2 file : \n%s\n", buff);*/
                if (!obj) {
                    printf("check list2 file err , retry again \n");
                    redo_cnt++;
                    if (redo_cnt <= 3) {
                        msleep(100);
                        goto redo2;
                    }
                    redo_cnt = 0;
                    /*ASSERT(0,"check list2 file err !!!!!!!!! ");*/
                }
                json_object_put(obj);
            }
        }
        if (buff) {
            free(buff);
            buff = NULL;
        }
#endif

#endif

exit2:
        os_mutex_post(&file_list_mutex);
        printf("----update file list\n");
    } else {
        printf("SD card no ready !!!!!!!!!!\n\n");
    }
#endif

}

void FILE_LIST_ADD(u32 status, const char *__path, u8 create_file)
{
    u32 len;
    struct media_file_info media_info = {0};
    FILE *fd = NULL;
    u8 flag = 0;
    u8 path[128] = {0};

    static u32 last_ms = 0;
    u32 now_ms;
    u8 one_sec = true;

    //删除全部文件会连续频繁调用函数，修改在频繁调用时候相差1秒之后才能重新更新文件
    now_ms = timer_get_ms();
    if (last_ms && now_ms > last_ms && (now_ms - last_ms) >= 1000) {
        last_ms = now_ms - (now_ms - last_ms) % 1000;
    } else if (last_ms && now_ms > last_ms) {
        one_sec = false;
    } else if (now_ms <= last_ms || !last_ms) {
        last_ms = now_ms;
    }

#ifdef CONFIG_ENABLE_VLIST
    printf("FILE_LIST_ADD : %s\n", __path);

    os_mutex_pend(&file_list_read_mutex, 0);

    len = strlen((const char *)__path);
    if (len < 3) {
        return;
    }
    strcpy((char *)path, __path);
    if (!storage_device_ready()) {
        printf("!!storage_device_ready err \n\n");
        return;
    }

    fd = fopen((const char *)path, "r");
    if (fd == NULL) {
        printf("err , open file :%s \n\n", path);
        return;
    }

    if (strstr((const char *)path, get_rec_path_1())) {
        flag = 0;
    } else {
        flag = 1;
    }

#ifdef CONFIG_EMR_DIR_ENABLE
    if (strstr((const char *)path, CONFIG_EMR_REC_DIR)) {
        media_info.is_emf = 1;
    } else
#endif
    {
        media_info.is_emf = 0;
    }
    media_info.fd = fd;
    media_info.channel = flag;
    send_json(&media_info, status);
    if (media_info.fd) {
        fclose(fd);
        media_info.fd = NULL;
    }
    if (create_file && one_sec) {
        sys_timeout_add(NULL, (void (*)(void *))FILE_GEN, 1000);
    }
    os_mutex_post(&file_list_read_mutex);
#endif
}

unsigned short __attribute__((weak))DUMP_PORT()
{
    return 0;
}

unsigned short __attribute__((weak))FORWARD_PORT()
{
    return 0;
}

unsigned short __attribute__((weak))BEHIND_PORT()
{
    return 0;
}

const char *__attribute__((weak))get_rec_path_1()
{
    return NULL;
}
const char *__attribute__((weak))get_rec_path_2()
{
    return NULL;
}
const char *__attribute__((weak))get_root_path()
{
    return NULL;
}





int vf_list(u8 type, u8 isforward, char *dir)
{
    struct vfscan *fs = NULL;
    FILE *vf_fd_forward = NULL;
    FILE *vf_fd_behind = NULL;
    FILE *fd = NULL;
    u32 open_count = 0;
    union vunpkg_req req;
    u8 namebuf[128];
    char *tmp_buf = NULL;
    const char *fs_arg = NULL;
    int err;
    int ret;
    int vaild;
    int file_sz = 0;
    char path[64];
    if (!storage_device_ready()) {
        return -1;
    }

    tmp_buf = (char *)malloc(512);

    if (tmp_buf == NULL) {
        printf("%s  %d malloc fail\n", __func__, __LINE__);
    }

#if 1

    if (isforward) {

        sprintf(path, "%s%s", get_rec_path_1(), "vf_list_a.txt");
        vf_fd_forward = fopen(path, "w+");

        if (vf_fd_forward == NULL) {
            printf("%s  %d fopen fail\n", __func__, __LINE__);

            return -1;
        }

        file_sz += fwrite(vf_fd_forward, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
        strcpy((char *)dir, path);
    } else {

        sprintf(path, "%s%s", get_rec_path_2(), "vf_list_a.txt");
        vf_fd_behind = fopen(path, "w+");

        if (vf_fd_behind == NULL) {
            printf("%s  %d fopen fail\n", __func__, __LINE__);

            return -1;
        }

        file_sz += fwrite(vf_fd_behind, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
        strcpy((char *)dir, path);
    }

#endif
    //copy double dir
    switch (type) {
    case VID_JPG:
        fs_arg = __fs_arg[2];
        break;
    case VIDEO:
        fs_arg = __fs_arg[0];
        break;
    case JPG:
        fs_arg = __fs_arg[1];
        break;
    default:
        printf("%s:%d type err \n\n", __func__, __LINE__);
        break;

    }

    if (isforward) {
        fs = fscan(get_rec_path_1(), fs_arg);

        while (1) {
            if (fd == NULL) {
                fd = fselect(fs, FSEL_FIRST_FILE, 0);
            } else {
                fd = fselect(fs, FSEL_NEXT_FILE, 0);
            }

            if (fd == NULL) {
                printf("open_count->%d fs_arg->%s\n", open_count, fs_arg);
                if (open_count > 0) {
                    fseek(vf_fd_forward, file_sz - 1, SEEK_SET);
                }


                fwrite(vf_fd_forward, "]}", 2);
                fclose(vf_fd_forward);
                fscan_release(fs);
                malloc_stats();
                free(tmp_buf);
                return type;
                /* break; */
            }

            fget_name(fd, namebuf, 128);
            ret = strlen((const char *)namebuf) - 3;
            struct vfs_attr attr;
            int _attr;
            fget_attrs(fd, &attr);
            fget_attr(fd, &_attr);

            if (!memcmp((namebuf + ret), "mov", 3)) {

#ifdef CONFIG_NET_PKG_H264
                struct __mov_unpkg_info info;

                if (is_vaild_mov_file(fd)) {
                    if (read_stts(fd, &info) != 0) {
                        vaild = 0;
                        fclose(fd);
                        continue;

                    }
                    if (0 != read_time_scale_dur(fd, &info)) {
                        vaild = 0;
                        fclose(fd);
                        continue;
                    }
                    if (0 != read_height_and_length(fd, &info)) {
                        vaild = 0;
                        fclose(fd);
                        continue;
                    }
                    vaild = 1;

                    if (_attr & F_ATTR_RO) {
                        vaild = 2;
                    }
                } else {
                    FILE_DELETE((char *)namebuf, 0);
                    fdelete(fd);
                    vaild = 0;
                    /*fclose(fd);*/
                    continue;
                }

                ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\"},"
                              , vaild
                              , get_rec_path_1()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , info.durition / info.scale
                              , info.height >> 16
                              , info.length >> 16
                              , info.scale / info.sample_duration
                              , flen(fd)
                             );
#endif
            } else if (!memcmp((namebuf + ret), "AVI", 3)) {
#ifdef CONFIG_NET_PKG_JPEG
                struct __mov_unpkg_info info;
                u8 state = 0;
                avi_net_preview_unpkg_init(fd, state);
                if (!is_vaild_avi_file(fd, state)) {
                    vaild = 0x0;
                    printf("err : vf_list not vaild AVI file , and delect this file!!!!\n\n");
                    FILE_DELETE((char *)namebuf, 0);
                    fdelete(fd);
                    /*fclose(fd);*/
                    continue;
                }
                if (avi_get_width_height(fd, (void *)&info, state)) {
                    vaild = 0x0;
                    printf("err : vf_list read AVI width and heigt fail !!!!\n\n");
                    fclose(fd);
                    continue;
                }
                if (_attr & F_ATTR_RO) {
                    vaild = 0x2;
                } else {
                    vaild = 0x1;
                }
                info.scale = 1;
                info.durition = avi_get_file_time(fd, state);
                info.sample_duration = avi_get_fps(fd, state);
                avi_net_unpkg_exit(fd, state);
                ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\"},"
                              , vaild
                              , get_rec_path_1()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , info.durition / info.scale
                              , info.height
                              , info.length
                              , info.sample_duration
                              , flen(fd)
                             );
#endif
            } else {
#if 1
                //  puts("picture\n");
                u16 height, width;
                fseek(fd, 0x174, SEEK_SET);
                fread(fd, &height, 2);
                fread(fd, &width, 2);
                height = lwip_htons(height);
                width  = lwip_htons(width);
                ret = sprintf(tmp_buf, "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\"},"
                              , get_rec_path_1()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , height
                              , width
                              , flen(fd)
                             );
#endif
            }

            open_count++;
#if 1
            file_sz += fwrite(vf_fd_forward, tmp_buf, ret);
#endif
            fclose(fd);
        }

    } else {

//////////////////////////////////////////////////////////////
//////////////////////////2号文件夹////////////////////////////////////
#if 1
        open_count = 0;
        fs = fscan(get_rec_path_2(), fs_arg);

        while (1) {
            if (fd == NULL) {
                fd = fselect(fs, FSEL_FIRST_FILE, 0);
            } else {
                fd = fselect(fs, FSEL_NEXT_FILE, 0);
            }


            if (fd == NULL) {
                if (open_count > 0) {
                    fseek(vf_fd_behind, file_sz - 1, SEEK_SET);
                } else {
                    return NONE;
                }

                fwrite(vf_fd_behind, "]}", 2);
                fclose(vf_fd_behind);
                fscan_release(fs);
                return type;
            }

            fget_name(fd, namebuf, 128);
            int _attr;
            struct vfs_attr attr;
            fget_attrs(fd, &attr);

            fget_attr(fd, &_attr);
            if (!memcmp((namebuf + ret), "mov", 3)) {
                struct __mov_unpkg_info info;

#ifdef CONFIG_NET_PKG_H264
                if (is_vaild_mov_file(fd)) {
                    read_stts(fd, &info);
                    read_time_scale_dur(fd, &info);
                    read_height_and_length(fd, &info);
                    vaild = 1;

                    if (_attr & F_ATTR_RO) {
                        vaild = 2;
                    }

                } else
#endif
                {

                    vaild = 0;
                    FILE_DELETE((char *)namebuf, 0);
                    fdelete(fd);
                    /*fclose(fd);*/
                    continue;
                }

                ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\"},"
                              , vaild
                              , get_rec_path_2()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , info.durition / info.scale
                              , info.height >> 16
                              , info.length >> 16
                              , info.scale / info.sample_duration
                              , flen(fd)
                             );
            } else if (!memcmp((namebuf + ret), "AVI", 3)) {
#ifdef CONFIG_NET_PKG_JPEG
                struct __mov_unpkg_info info;
                u8 state = 0;
                avi_net_preview_unpkg_init(fd, state);
                if (!is_vaild_avi_file(fd, state)) {
                    vaild = 0x0;
                    printf("err : vf_list not vaild AVI file !!!!\n\n");
                    FILE_DELETE((char *)namebuf, 0);
                    fdelete(fd);
                    /*fclose(fd);*/
                    continue;
                }
                if (avi_get_width_height(fd, (void *)&info, state)) {
                    vaild = 0x0;
                    printf("err : vf_list read AVI width and heigt fail !!!!\n\n");
                    fclose(fd);
                    continue;
                }
                if (_attr & F_ATTR_RO) {
                    vaild = 0x2;
                } else {
                    vaild = 0x1;
                }
                info.scale = 1;
                info.durition = avi_get_file_time(fd, state);
                info.sample_duration = avi_get_fps(fd, state);
                avi_net_unpkg_exit(fd, state);
                ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\"},"
                              , vaild
                              , get_rec_path_2()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , info.durition / info.scale
                              , info.height
                              , info.length
                              , info.sample_duration
                              , flen(fd)
                             );
#endif
            } else {
                //       puts("picture\n");
                u16 height, width;
                fseek(fd, 0x174, SEEK_SET);
                fread(fd, &height, 2);
                fread(fd, &width, 2);
                height = lwip_htons(height);
                width  = lwip_htons(width);
                ret = sprintf(tmp_buf, "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\"},"
                              , get_rec_path_2()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , height
                              , width
                              , flen(fd)
                             );
            }

            open_count++;
            file_sz += fwrite(vf_fd_behind, tmp_buf, ret);
            fclose(fd);
        }

#endif
    }

    return NONE;
}








void __FILE_LIST_INIT(u8 is_forward, u32 file_num);

extern int send_ctp_string(int cmd_type, char *buf, char *_req, void *priv);
#if 0
static int get_and_write_file_attribute(u32 status, FILE *file_list_fd, FILE *fd, u8 flag)
{
    char namebuf[20];
    char tmp_buf[128] = {[0 ... 127] = 0};
    char tmp_buf2[128] = {[0 ... 127] = 0};
    int ret;
    int _attr;
    struct vfs_attr attr;
    struct __mov_unpkg_info info;
    u32 vaild = 0;
    fget_name(fd, namebuf, 20);
    ret = strlen((const char *)namebuf) - 3;
    fget_attrs(fd, &attr);
    fget_attr(fd, &_attr);
    char *buf = malloc(256);
    if (buf == NULL) {
        return -1;
    }
    /* printf("fname -> %s\n", namebuf); */
    if (!memcmp((namebuf + ret), "MOV", 3)) {
        /* time1 = timer_get_ms(); */
        if (is_vaild_mov_file(fd)) {
            if (read_stts(fd, &info) != 0) {
                vaild = 0;
                fclose(fd);
                continue;

            }
            if (0 != read_time_scale_dur(fd, &info)) {
                vaild = 0;
                fclose(fd);
                continue;
            }
            if (0 != read_height_and_length(fd, &info)) {
                vaild = 0;
                fclose(fd);
                continue;
            }


            vaild = 1;

            if (_attr & F_ATTR_RO) {
                vaild = 2;
            }

            /* time2 = timer_get_ms(); */
            /* printf("use in time:%dms\n",time2 - time1); */
        } else {

            vaild = 0;
            printf("Invalid file\n");
            return -1;
        }

        ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\",\"c\":\"%d\"},"
                      , vaild
                      , flag ? get_rec_path_2() : get_rec_path_1()
                      , namebuf
                      , attr.crt_time.year
                      , attr.crt_time.month
                      , attr.crt_time.day
                      , attr.crt_time.hour
                      , attr.crt_time.min
                      , attr.crt_time.sec
                      , info.durition / info.scale
                      , info.height >> 16
                      , info.length >> 16
                      , info.scale / info.sample_duration
                      , flen(fd)
                      , flag
                     );

        if (!flag) {
            forward_write_block(tmp_buf, ret);
        } else {
            behind_write_block(tmp_buf, ret);
        }

        if (status <= 1 && !initing) {
            ret = sprintf(buf, "{\"op\":\"NOTIFY\",\"param\":{\"status\":\"%d\",\"desc\":\"{\\\"y\\\":%d,\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"d\\\":\\\"%d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"p\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\"}\"}}"
                          , status
                          , vaild
                          , flag ? get_rec_path_2() : get_rec_path_1()
                          , namebuf
                          , attr.crt_time.year
                          , attr.crt_time.month
                          , attr.crt_time.day
                          , attr.crt_time.hour
                          , attr.crt_time.min
                          , attr.crt_time.sec
                          , info.durition / info.scale
                          , info.height >> 16
                          , info.length >> 16
                          , info.scale / info.sample_duration
                          , flen(fd)
                          , flag
                         );
            send_ctp_string(CTP_NOTIFY_COMMAND, buf, "VIDEO_FINISH", NULL);
        } else {
            free(buffer);
        }
    } else {
        //  puts("picture\n");
        u16 height, width;
        fseek(fd, 0x174, SEEK_SET);
        fread(fd, &height, 2);
        fread(fd, &width, 2);
        height = lwip_htons(height);
        width  = lwip_htons(width);
        ret = sprintf(tmp_buf, "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\",\"c\":\"%d\"},"
                      , flag ? get_rec_path_2() : get_rec_path_1()
                      , namebuf
                      , attr.crt_time.year
                      , attr.crt_time.month
                      , attr.crt_time.day
                      , attr.crt_time.hour
                      , attr.crt_time.min
                      , attr.crt_time.sec
                      , height
                      , width
                      , flen(fd)
                      , flag
                     );

        /* ret = fwrite(file_list_fd, tmp_buf, ret); */
        if (!flag) {
            forward_write_block(tmp_buf, ret);
        } else {
            behind_write_block(tmp_buf, ret);
        }

        if (status <= 1 && !initing) {
            sprintf(buf, "{\"op\":\"NOTIFY\",\"param\":{\"desc\":\"{\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\"}\"}}"
                    , flag ? get_rec_path_2() : get_rec_path_1()
                    , namebuf
                    , attr.crt_time.year
                    , attr.crt_time.month
                    , attr.crt_time.day
                    , attr.crt_time.hour
                    , attr.crt_time.min
                    , attr.crt_time.sec
                    , height
                    , width
                    , flen(fd)
                    , flag
                   );

            send_ctp_string(CTP_NOTIFY_COMMAND, buf, "PHOTO_CTRL", NULL);
        } else {
            free(buffer);
        }
    } else {
        free(buffer);
    }
    return ret;

}
#endif





int get_media_file_info(struct media_file_info *__info)
{
    int ret = 0;
    fget_name(__info->fd, __info->namebuf, sizeof(__info->namebuf));
    ret = strlen((const char *)__info->namebuf) - 3;
    fget_attrs(__info->fd, &__info->attr);
    __info->attr.crt_time.sec += 1; //BUG BUG BUG
    fget_attr(__info->fd, &__info->_attr);
    if (!memcmp((__info->namebuf + ret), "MOV", 3)) {
#ifdef CONFIG_NET_PKG_H264
        if (is_vaild_mov_file(__info->fd)) {
            if (0 != read_stts(__info->fd, &__info->info)) {
                __info->vaild = 0x0;
                printf("Invalid file\n");
                return -1;
            }
            if (0 != read_time_scale_dur(__info->fd, &__info->info)) {
                __info->vaild = 0x0;
                printf("Invalid file\n");
                return -1;
            }
            if (0 != read_height_and_length(__info->fd, &__info->info)) {
                __info->vaild = 0x0;
                printf("Invalid file\n");
                return -1;
            }

            if (__info->_attr & F_ATTR_RO) {
                __info->vaild = 0x2;
            } else {
                __info->vaild = 0x1;
            }
            __info->type = 0x1;

        } else
#endif
        {
            printf("warning : %s , not vaild MOV file , delect file..... !!!!\n\n", __info->namebuf);
            __info->vaild = 0x0;
            fdelete(__info->fd);
            __info->fd = NULL;
            printf("Invalid file\n");
            return -1;
        }

    } else if (!memcmp((__info->namebuf + ret), "JPG", 3)) {
        __info->type = 0x2;
        fseek(__info->fd, 0x174, SEEK_SET);
        fread(__info->fd, &__info->height, 2);
        fread(__info->fd, &__info->width, 2);
        __info->height = lwip_htons(__info->height);
        __info->width  = lwip_htons(__info->width);
    } else if (!memcmp((__info->namebuf + ret), "AVI", 3)) {
#ifdef CONFIG_NET_PKG_JPEG
        __info->type = 0x3;
        u8 state = 0;
        avi_net_preview_unpkg_init(__info->fd, state);
        if (!is_vaild_avi_file(__info->fd, state)) {
            __info->vaild = 0x0;
            printf("warning : %s , not vaild AVI file , delect file..... !!!!\n\n", __info->namebuf);
            FILE_DELETE((char *)__info->namebuf, 0);
            fdelete(__info->fd);
            __info->fd = NULL;
            return -1;
        }
        if (avi_get_width_height(__info->fd, (void *)&__info->info, state)) {
            __info->vaild = 0x0;
            printf("err : read AVI width and heigt fail !!!!\n\n");
            return -1;
        }
        if (__info->_attr & F_ATTR_RO) {
            __info->vaild = 0x2;
        } else {
            __info->vaild = 0x1;
        }
        __info->info.scale = 1;
        __info->info.durition = avi_get_file_time(__info->fd, state);
        __info->info.sample_duration = avi_get_fps(__info->fd, state);
        avi_net_unpkg_exit(__info->fd, state);
#endif
    } else {
        printf("FILE NAME:%s err!!!!!!\n", __info->namebuf);
        return -1;

    }
    return 0;
}

static void incr_date_time(struct tm *tm_time, int incr)
{
    do {
        if (incr >= 60) {
            tm_time->tm_sec += 60;
            incr -= 60;
        } else {
            tm_time->tm_sec += incr;
            incr = 0;
        }

        if (tm_time->tm_sec > 59) {
            tm_time->tm_sec -= 60;
            tm_time->tm_min += 1;

            if (tm_time->tm_min > 59) {
                tm_time->tm_min -= 60;
                tm_time->tm_hour += 1;

                if (tm_time->tm_hour > 23) {
                    tm_time->tm_hour -= 24;
                    tm_time->tm_mday += 1;

                    if (tm_time->tm_mday > cal_days(tm_time->tm_year, tm_time->tm_mon)) {
                        tm_time->tm_mday = 1;
                        tm_time->tm_mon += 1;

                        if (tm_time->tm_mon > 12) {
                            tm_time->tm_mon = 1;
                            ++tm_time->tm_year;
                        }
                    }
                }
            }
        }
    } while (incr);
}
int send_json(struct media_file_info *__info, u32 status)
{
    int ret = 0;

    struct tm _tm;
    char *buffer = malloc(256);
    if (buffer == NULL) {
        printf("malloc fail \n");
        return -1;

    }

    if (get_media_file_info(__info)) {
        printf("get_media_file_info fail\n");
        free(buffer);
        return -1;
    }

    int file_len = flen(__info->fd);
    _tm.tm_year  = __info->attr.crt_time.year - 1900;
    _tm.tm_mon  = __info->attr.crt_time.month - 1;
    _tm.tm_mday  = __info->attr.crt_time.day;
    _tm.tm_hour  = __info->attr.crt_time.hour;
    _tm.tm_min  = __info->attr.crt_time.min;
    _tm.tm_sec  = __info->attr.crt_time.sec;
    /*
    printf("start=%d-%d-%d-%d-%d-%d\n"
           , _tm.tm_year
           , _tm.tm_mon
           , _tm.tm_mday
           , _tm.tm_hour
           , _tm.tm_min
           , _tm.tm_sec);
    	   */
    switch (__info->type) {
    case 0x1:
        incr_date_time(&_tm, __info->info.durition / __info->info.scale);
        ret = sprintf(buffer, VIDEO_JSON_MEM
                      , __info->vaild
                      , __info->channel ? file_is_emf((char *)get_rec_path_2(), __info->is_emf) : file_is_emf((char *)get_rec_path_1(), __info->is_emf)
                      , __info->namebuf
                      , __info->attr.crt_time.year
                      , __info->attr.crt_time.month
                      , __info->attr.crt_time.day
                      , __info->attr.crt_time.hour
                      , __info->attr.crt_time.min
                      , __info->attr.crt_time.sec
                      , __info->info.durition / __info->info.scale
                      , __info->info.height
                      , __info->info.length
                      , __info->info.scale / __info->info.sample_duration
                      , file_len
                      , __info->channel
                      , mktime(&_tm)
                     );

        if (!__info->channel) {
            forward_write_block((u8 *)buffer, ret);
        } else {
            behind_write_block((u8 *)buffer, ret);
        }

        if (status <= 1 && !initing) {
            ret = sprintf(buffer, VIDEO_JSON_APP
                          , status
                          , __info->vaild
                          , __info->channel ? file_is_emf((char *)get_rec_path_2(), __info->is_emf) : file_is_emf((char *)get_rec_path_1(), __info->is_emf)
                          , __info->namebuf
                          , __info->attr.crt_time.year
                          , __info->attr.crt_time.month
                          , __info->attr.crt_time.day
                          , __info->attr.crt_time.hour
                          , __info->attr.crt_time.min
                          , __info->attr.crt_time.sec
                          , __info->info.durition / __info->info.scale
                          , __info->info.height >> 16
                          , __info->info.length >> 16
                          , __info->info.scale / __info->info.sample_duration
                          , file_len
                          , __info->channel
                          , mktime(&_tm)
                         );
            send_ctp_string(CTP_NOTIFY_COMMAND, buffer, "VIDEO_FINISH", NULL);
        } else {
            free(buffer);
        }
        break;
    case 0x2:
        ret = sprintf(buffer, PICTURE_JSON_MEM
                      , __info->channel ? file_is_emf((char *)get_rec_path_2(), __info->is_emf) : file_is_emf((char *)get_rec_path_1(), __info->is_emf)
                      , __info->namebuf
                      , __info->attr.crt_time.year
                      , __info->attr.crt_time.month
                      , __info->attr.crt_time.day
                      , __info->attr.crt_time.hour
                      , __info->attr.crt_time.min
                      , __info->attr.crt_time.sec
                      , __info->height
                      , __info->width
                      , file_len
                      , __info->channel
                      , mktime(&_tm)
                     );

        if (!__info->channel) {
            forward_write_block((u8 *)buffer, ret);
        } else {
            behind_write_block((u8 *)buffer, ret);
        }

        if (status <= 1 && !initing) {
            sprintf(buffer, PICTURE_JSON_APP
                    , __info->channel ? file_is_emf((char *)get_rec_path_2(), __info->is_emf) : file_is_emf((char *)get_rec_path_1(), __info->is_emf)
                    , __info->namebuf
                    , __info->attr.crt_time.year
                    , __info->attr.crt_time.month
                    , __info->attr.crt_time.day
                    , __info->attr.crt_time.hour
                    , __info->attr.crt_time.min
                    , __info->attr.crt_time.sec
                    , __info->height
                    , __info->width
                    , file_len
                    , __info->channel
                    , mktime(&_tm)
                   );
            send_ctp_string(CTP_NOTIFY_COMMAND, buffer, "PHOTO_CTRL", NULL);
        } else {
            free(buffer);
        }
        break;
    case 0x3:
        incr_date_time(&_tm, __info->info.durition / __info->info.scale);
        ret = sprintf(buffer, VIDEO_JSON_MEM
                      , __info->vaild
                      , __info->channel ? file_is_emf((char *)get_rec_path_2(), __info->is_emf) : file_is_emf((char *)get_rec_path_1(), __info->is_emf)
                      , __info->namebuf
                      , __info->attr.crt_time.year
                      , __info->attr.crt_time.month
                      , __info->attr.crt_time.day
                      , __info->attr.crt_time.hour
                      , __info->attr.crt_time.min
                      , __info->attr.crt_time.sec
                      , __info->info.durition / __info->info.scale
                      , __info->info.height
                      , __info->info.length
                      , __info->info.sample_duration
                      , file_len
                      , __info->channel
                      , mktime(&_tm)
                     );

        if (!__info->channel) {
            forward_write_block((u8 *)buffer, ret);
        } else {
            behind_write_block((u8 *)buffer, ret);
        }

        if (status <= 1 && !initing) {
            ret = sprintf(buffer, VIDEO_JSON_APP
                          , status
                          , __info->vaild
                          , __info->channel ? file_is_emf((char *)get_rec_path_2(), __info->is_emf) : file_is_emf((char *)get_rec_path_1(), __info->is_emf)
                          , __info->namebuf
                          , __info->attr.crt_time.year
                          , __info->attr.crt_time.month
                          , __info->attr.crt_time.day
                          , __info->attr.crt_time.hour
                          , __info->attr.crt_time.min
                          , __info->attr.crt_time.sec
                          , __info->info.durition / __info->info.scale
                          , __info->info.height
                          , __info->info.length
                          , __info->info.sample_duration
                          , file_len
                          , __info->channel
                          , mktime(&_tm)
                         );
            send_ctp_string(CTP_NOTIFY_COMMAND, buffer, "VIDEO_FINISH", NULL);//内部已free掉buffer
        } else {
            free(buffer);
        }
        break;
    default:
        free(buffer);
        break;
    }
    return 0;
}

int FILE_LIST_EXIT(void)
{
    FILE_DELETE(NULL, 1);
    os_mutex_del(&file_list_mutex, OS_DEL_ALWAYS);
    os_mutex_del(&file_list_read_mutex, OS_DEL_ALWAYS);
    mutex_init = 0;
    return 0;
}

int FILE_LIST_INIT(u32 flag)
{
    char buf[128];
    u32 err = 0;
#ifdef CONFIG_ENABLE_VLIST
    if (initing) {
        printf("File list in doing, Please waiting\n");
        return -1;
    }
    initing = 1;
    if (!mutex_init) {

        err = os_mutex_create(&file_list_mutex);
        if (err != OS_NO_ERR) {
            printf("os mutex create fail \n");
            initing = 0;
            return -1;
        }
        err = os_mutex_create(&file_list_read_mutex);
        if (err != OS_NO_ERR) {
            printf("os fiile mutex create fail \n");
            initing = 0;
            return -1;
        }
        mutex_init = 1;

    }
    os_mutex_pend(&file_list_read_mutex, 0);

    __FILE_LIST_INIT(1, MAX_NUM);

#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
    __FILE_LIST_INIT(0, MAX_NUM);

#endif

    if (flag) {
        FILE_GEN();
        sprintf(buf, "type:1,path:%s%s", get_rec_path_1(), "vf_list.txt");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "FORWARD_MEDIA_FILES_LIST", "NOTIFY", buf);

#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
        sprintf(buf, "type:1,path:%s%s", get_rec_path_2(), "vf_list.txt");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "BEHIND_MEDIA_FILES_LIST", "NOTIFY", buf);
#endif
    }
    initing = 0;
    os_mutex_post(&file_list_read_mutex);
#endif
    return 0;
}



void FILE_TEST()
{
    puts("FILE_TEST\n\n\n\n");
    __FILE_LIST_INIT(1, MAX_NUM);
    __FILE_LIST_INIT(0, MAX_NUM);
    puts("FILE_TEST\n\n\n\n");
}


void file_list_thread(void *arg)
{
    u32 *flag = (u32 *)arg;
    u32 cnt = 0;
    /*msleep(200);//等待SD卡稳定 */
    /*生成文件列表*/
    if (storage_device_ready()) {
#if defined CONFIG_ENABLE_VLIST
        FILE_LIST_INIT(*flag);
#endif
    }
}





void FILE_LIST_INIT_SMALL(u32 file_num)
{
    __FILE_LIST_INIT(2, file_num);
#if defined CONFIG_VIDEO2_ENABLE || defined CONFIG_VIDEO1_ENABLE
    __FILE_LIST_INIT(3, file_num);
#endif
}




int FILE_INITIND_CHECK()
{
    return initing;
}


void FILE_LIST_IN_MEM(u32 flag)
{
    static u32 count = 0;
    char buf[32];
    u32 space = 0;

#ifdef CONFIG_ENABLE_VLIST
    sprintf(buf, "file_list_thread%d", count++);
    thread_fork(buf, 10, 0x1000, 0, 0, file_list_thread, (void *)&flag);
#endif

}

void FILE_LIST_SCAN(void)
{
    u32 flag = 1;
    file_list_thread(&flag);
}

void FILE_LIST_TASK_INIT(void)
{
    FILE_LIST_IN_MEM(1);
}

