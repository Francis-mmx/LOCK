#define _STDIO_H_


#include "printf.h"
#include "xciot_api.h"
#include "ut_dev_ipc_cmd.h"
#include "system/database.h"
#include "device/device.h"
#include "os/os_compat.h"
#include "doorbell_event.h"

#define SLEEEP_TIME_INTERVAL  500     // 判断一次500ms
#define CONNECT_TIMOUT_CNT    (CONFIG_PLATFORM_CONNECT_TIMEOUT / SLEEEP_TIME_INTERVAL)

static iot_sleep_t __s_iotsleep;
static u8  system_running = 0;
u8 get_system_running()
{
    return system_running;
}
void set_system_running(u8 value)
{
    system_running = value;
}

const char cloud_storage_status[4][32] = {
    "CLOUD_STORAGE_START",
    "CLOUD_STORAGE_STARTING",
    "CLOUD_STORAGE_STOPING",
    "CLOUD_STORAGE_STOP",
    "CLOUD_STORAGE_IDLE",
};

static u8 is_ready_to_sleep(void)
{
    static int old_conn_num = 0;
    static int delayed_sleep_count;
    static int cloud_storage_sleep_count;
    static int usb_sleep_count;


    if (avsdk_poll_current_conn_num()) {
        old_conn_num = 1;
        printf("\navsdk_poll_current_conn_num() = %d \n", avsdk_poll_current_conn_num());
        goto __exit;
    }

    if (get_cloud_storage_up_state() == CLOUD_STORAGE_IDLE && old_conn_num == 0) {
        delayed_sleep_count++;
        //不需要上传云存的，需要5s才能进入休眠
        if (delayed_sleep_count < (5000 / (SLEEEP_TIME_INTERVAL))) {
            printf("\n Cloud storage is not uploaded, and sleep needs to be delayed\n");
            goto __exit;
        }
        printf("\n >>>>>>>Cloud storage is not uploaded and sleep timeout\n");
    } else {
        delayed_sleep_count = 0;
    }

    if (get_cloud_storage_up_state() != CLOUD_STORAGE_STOP &&  get_cloud_storage_up_state() != CLOUD_STORAGE_IDLE) {

        printf("get_cloud_storage_up_state() = %s \n", cloud_storage_status[get_cloud_storage_up_state()]);
        if (get_cloud_storage_up_state() == CLOUD_STORAGE_START) {
            cloud_storage_sleep_count = 0;
            goto __exit;
        } else if (get_cloud_storage_up_state() == CLOUD_STORAGE_STOPING || get_cloud_storage_up_state() == CLOUD_STORAGE_STARTING) {
            //云存启动超时或者停止超时进入休眠
            cloud_storage_sleep_count++;
            if (cloud_storage_sleep_count < (5000 / (SLEEEP_TIME_INTERVAL))) {
                goto __exit;
            }
            printf("\n >>>>>>Cloud storage fails to start or stop, and enters sleep \n");
        }
    } else {
        cloud_storage_sleep_count = 0;
    }

    if (get_sdcard_upgrade_status()) {
        goto __exit;
    }
    if (get_system_running()) {
        goto __exit;
    }

    if (is_usb_mode()) {
        if (get_usb_uvc_hdl()) {
            puts("\n usb uvc mode does not allow hibernation \n");
            usb_sleep_count = 0;
            goto __exit;
        }
        usb_sleep_count++;
        if (usb_sleep_count < (120 * 1000 / (SLEEEP_TIME_INTERVAL))) {
            printf("\n USB disk mode allows hibernation after 2 minutes  current time = %d sec\n", usb_sleep_count);
            goto __exit;
        }
    } else {
        usb_sleep_count = 0;
    }

    return 1;

__exit:

    return 0;
}


void avsdk_sleep_thread(void *priv)
{
    int64_t ret;
    int retry = 0;

    static iot_sleep_t *s_iotsleep = &__s_iotsleep;
    uint64_t upgrade_check_time = 0;
    int network_status, old_network_status = 0;

    //此时已开始上报云存
    while (1) {

        //如果不是低功耗模式就不休眠
        if (!db_select("lowpower")) {
            puts("\n lowpower mode is off \n");
            msleep(SLEEEP_TIME_INTERVAL);
            continue;
        }

        network_status = get_network_status();
        printf("\n network_status = %d\n", network_status);
        if (network_status != old_network_status) {
            retry = 0;
        }
        old_network_status = network_status;
        //dhcp 成功但是连接平台没有成功
        if ((network_status == CONNECT_SUC || network_status == DHCP_SUC) && !get_avsdk_connect_flag()) {
            msleep(SLEEEP_TIME_INTERVAL);
            if (++retry >=  CONNECT_TIMOUT_CNT) {
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_ENTER_SLEEP, NULL);
                break;
            }
        } else if ((network_status == CONNECT_SUC || network_status == DHCP_SUC) && get_avsdk_connect_flag()) {
            msleep(SLEEEP_TIME_INTERVAL);
            if (is_ready_to_sleep()) {
                ipc_FirmwareCheckByDevice_Resp rsp = {0};
                ret = db_select_buffer(CHECKTIME_VM_INDEX, &upgrade_check_time, sizeof(upgrade_check_time));
                printf("\n read upgrade_check_time = %llu\n", upgrade_check_time);
                if (ret != sizeof(upgrade_check_time) || (ret == sizeof(upgrade_check_time) && get_utc_ms() - upgrade_check_time >= (24 * 3600 * 1000))) {
                    avsdk_firmware_poll(&rsp);
                    upgrade_check_time = 0;
                    upgrade_check_time = get_utc_ms();
                    printf("\n write upgrade_check_time = %llu \n", upgrade_check_time);
                    db_update_buffer(CHECKTIME_VM_INDEX, &upgrade_check_time, sizeof(upgrade_check_time));
                }
                if (rsp.is_update == 1 && rsp.firmware_type == 1) {
                    printf("\n rsp.is_update = %d     ,"
                           "\n rsp.firmware_ver = %s  ,"
                           "\n rsp.firmware_url = %s  ,"
                           "\n rsp.firmware_type =  %d,"
                           "\n rsp.firmware_size  %d   "
                           ,
                           rsp.is_update,
                           rsp.firmware_ver,
                           rsp.firmware_url,
                           rsp.firmware_type,
                           rsp.firmware_size);

                    extern void cloud_upgrade_firmware(void *rsp, int notify, void *arg);
                    cloud_upgrade_firmware(&rsp, 0, NULL);
                }
                s_iotsleep->way = 1;
                ret =  avsdk_sleep(s_iotsleep);
                if (ret) {
                    puts("\n avsdk_sleep fail\n");
                    s_iotsleep = NULL;
                }

                if (s_iotsleep) {
                    /* put_buf(s_iotsleep->faststart_data,s_iotsleep->faststart_datalen); */
                    /* printf("\n s_iotsleep.faststart_datalen = %d\n",s_iotsleep->faststart_datalen); */
                    write_data_to_flash(s_iotsleep->faststart_data, s_iotsleep->faststart_datalen);
                }

                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_ENTER_SLEEP, s_iotsleep);
                break;

            }
        } else if (network_status == NETWORK_CONFIG) {
            msleep(SLEEEP_TIME_INTERVAL);
            if (++retry >=  CONFIG_NETCONFIG_TIMEOUT / SLEEEP_TIME_INTERVAL) {
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_ENTER_SLEEP, NULL);
                break;
            }
        } else {
            msleep(SLEEEP_TIME_INTERVAL);
            printf("\n Network exception \n");
            if (++retry >=  CONFIG_STA_CONNECT_TIMEOUT / SLEEEP_TIME_INTERVAL) {
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_ENTER_SLEEP, NULL);
                break;
            }
        }
    }
__exit:
    printf("\n avsdk_sleep_thread exit\n");
}




void doorbell_sleep_thread_init(void)
{
    thread_fork("avsdk_sleep_thread", 18, 0x2000, 0, 0, avsdk_sleep_thread, NULL);
}
