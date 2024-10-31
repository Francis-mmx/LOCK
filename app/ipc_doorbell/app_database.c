#include "system/includes.h"

#include "res.h"
#include "app_database.h"
#include "app_config.h"
#include "vm_api.h"

#define CN_PA   ((0xA9BE << 16) | ('A' << 8)  | ('B' << 0))
#define CN_PB   (('C'    << 24) | ('D' << 16) | ('E' << 8) | ('F' << 0))


/*
 * app配置项表
 * 参数1: 配置项名字
 * 参数2: 配置项需要多少个bit存储
 * 参数3: 配置项的默认值
 */
static const struct db_table app_config_table[] = {
    /*
     *  录像模式配置项
     */
    {"mic",     1,      1},                         // 录音开关
    {"mot",     1,      0},                         // 移动侦测开关
    {"par",     1,      0},                         // 停车守卫开关
    {"wdr",     1,      0},                         // 夜视增强开关
    {"num",     1,      0},                         // 车牌开关
    {"dat",     1,      1},                         // 时间标签开关
    {"two",     1,      1},                         // 双路开关
    {"gra",     2,      GRA_SEN_MD},               // 重力感应灵敏度
#if __SDRAM_SIZE__ <= (2 * 1024 * 1024)
    {"res",     3,      VIDEO_RES_720P},           // 录像分辨率
#else
    {"res",     3,      VIDEO_RES_1080P},           // 录像分辨率
#endif
    {"cyc",     4,      3},                         // 循环录像时间，单位分钟
    {"exp",     8,      0},                         // 曝光, 范围-3到+3
    {"gap",     16,     0},                         // 间隔录影设置
    {"cna",     32,     CN_PA},                     // 车牌号码前3个字符
    {"cnb",     32,     CN_PB},                     // 车牌号码后4个字符

    /*
     *  系统模式配置项
     */
    {"kvo",     1,      1},                         // 按键音开关
    {"lag",     5,      Chinese_Simplified},        // 语言设置
    {"fre",     8,      50},                        // 灯光频率
    {"aff",     8,      0},                         // 自动关机时间, 单位分钟
    {"pro",     8,      0},                         // 屏幕保护时间, 单位秒
    {"tvm",     8,      TVM_PAL},                   // 电视制式
    {"lan",     32,     0},                         // 轨道偏移
    {"hlw",     1,      0},                         // 前照灯提醒开关

    /*
     *  拍照模式配置项
     */
    {"sok",     1,      0},                         // 防手抖开关
    {"pdat",    1,      1},                         // 图片日期标签开关
    {"cyt",     1,      0},                         // 连拍开关
    {"qua",     3,      PHOTO_QUA_HI},              // 图片质量
    {"acu",     3,      PHOTO_ACU_HI},              // 图片锐度
    {"phm",     4,      0},                         // 延时拍照， 单位秒
    {"pres",    4,      PHOTO_RES_1M},              // 图片分辨率
    {"wbl",     4,      PHOTO_WBL_AUTO},            // 白平衡
    {"col",     4,      PHOTO_COLOR_NORMAL},        // 颜色模式
    {"sca",     4,      0},                         // 快速预览时间，单位s
    {"pexp",    8,      0},                         // 曝光设置，范围-3到+3
    {"iso",     16,     0},                         // iso
    {"stk",     8,      0},                         // 大头贴索引

    {"dac",     16,     0x55aa},                         // dac_trim

#ifdef CONFIG_WIFI_ENABLE
    {"res2",    3,      VIDEO_RES_VGA},           // 录像分辨率
    {"rtf",     3,      VIDEO_RES_VGA},            //预览前视分辨率
    {"rtb",     3,      VIDEO_RES_VGA},             //预览后视分辨率
    {"wfo",     1,      1},                         //wifi开关
    {"wfmode",  1,      0},                        // wifi运行模式
    {"bvo",     1,      1},                         // 开机音开关
#endif


    {"vqua",     32,     16},                     // 实时流码率


    {"ircut",   2,      3 },                   //IRCUT状态
    {"flip",    3,      0 },                     //翻转
    {"stled",   1,      1 },                     //状态指示灯
    {"secret",  1,      0 },                   //私密模式
    {"vol",     4,      7 },                     //音量
    {"tzone",   32,     480 },                  //时区

    {"lowpower",  1,      1},


};



int app_set_config(struct intent *it, const struct app_cfg *cfg, int size)
{
    int i;

    printf("app_set_config: %s, %d\n", it->data, it->exdata);

    for (i = 0; i < size; i++) {
        if (!strcmp(it->data, cfg[i].table)) {
            if (cfg[i].set) {
                int err = cfg[i].set(it->exdata);
                if (err) {
                    return err;
                }
            }
            db_update(cfg[i].table, it->exdata);
            return 0;
        }
    }

    return -EINVAL;
}





static int app_config_init()
{
    int err;

#if defined CONFIG_DATABASE_2_RTC
    err = db_create("rtc", NULL);
    ASSERT(err == 0, "open device rtc faild\n");
#elif defined CONFIG_DATABASE_2_FLASH
    struct vm_arg vm_arg = {
        .need_defrag_when_write_full = 1,
        .large_mode = VM_STORAGE_MODE,
        .vm_magic = VM_MAGIC_VERSION,
    };
    err = db_create("vm", &vm_arg);
    ASSERT(err == 0, "open device vm faild\n");
#if defined CONFIG_FORCE_RESET_VM
    db_erase();
#endif
#else
#error "undefine database device"
#endif

    return db_create_table(app_config_table, ARRAY_SIZE(app_config_table));
}
__initcall(app_config_init);


