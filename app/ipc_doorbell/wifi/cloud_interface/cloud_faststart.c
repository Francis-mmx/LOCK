#include "app_config.h"
#include "system/includes.h"
#include "doorbell_event.h"



/***
*采用52 index存放vm info
*/
struct fastdata_vm_info {
    int len;
};
static int cfg_write_addr = 0x3BB000;
static int fastdata_write_addr;

void clean_fastdata_to_flash(void)
{
    struct fastdata_vm_info info;
    info.len = 0;
    db_update_buffer(FASTSTART_VM_INDEX, &info, sizeof(struct fastdata_vm_info));
}

int write_data_to_flash(char *data, int len)
{
    void *dev;
    dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto exit;
    }
    dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, fastdata_write_addr);
    dev_bulk_write(dev, data, fastdata_write_addr, len); //此接口需要擦除flash之后才能写入
    extern int spi_flash_write_protect();
    spi_flash_write_protect();
    struct fastdata_vm_info info;
    info.len = len;

    db_update_buffer(FASTSTART_VM_INDEX, &info, sizeof(struct fastdata_vm_info));
exit:
    if (dev) {
        dev_close(dev);
    }
    return 0;
}
int read_data_for_flash(char *data, int len)
{

    void *fd;
    fd = dev_open("spiflash", NULL);
    if (!fd) {
        goto exit;
    }
    struct fastdata_vm_info info;
    int ret = db_select_buffer(FASTSTART_VM_INDEX, &info, sizeof(struct fastdata_vm_info));

    if (ret != sizeof(struct fastdata_vm_info) || info.len == 0) {
        return 0;
    }

    dev_bulk_read(fd, data, fastdata_write_addr, len);
exit:
    if (fd) {
        dev_close(fd);
    }

    return info.len;
}

void get_flash_idle_info(void)
{
    void *dev;
    u32 flash_size;
    dev = dev_open("spiflash", NULL);
    if (!dev) {
        printf("\n %s %d err\n", __func__, __LINE__);
        goto exit;
    }
    dev_ioctl(dev, IOCTL_GET_CAPACITY, (u32)&flash_size);

    flash_size -= 68 * 1024;
    printf("\n flash_size = 0x%x  \n", flash_size); //此地址为did写入地址
    flash_size -= 64 * 1024;
    printf("\n flash_size = 0x%x  \n", flash_size); //此地址为did写入地址



    //did占用4k
    //cfg_write_addr = flash_size;//此地址为did写入地址
    fastdata_write_addr = flash_size + 4 * 1024;

    printf("\n cfg_write_addr = 0x%x\n", flash_size); //此地址为did写入地址
exit:
    if (dev) {
        dev_close(dev);
    }
}

void doorbell_read_cfg_info(u8 *cfg, int len)
{
    void *dev = dev_open("spiflash", NULL);
    if (dev) {
        dev_bulk_read(dev, cfg, cfg_write_addr, len);
        dev_close(dev);
        for (int i = 0; i < len; i++) {
            if (cfg[i] == 0xff) {
                cfg[i] = 0;
                break;
            }
        }
    }
}
void doorbell_write_cfg_buf_info(void *buf, int len)
{
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto __exit;
    }
    dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, cfg_write_addr);
    dev_bulk_write(dev, buf, cfg_write_addr, len); //此接口需要擦除flash之后才能写入
    extern int spi_flash_write_protect();
    spi_flash_write_protect();
__exit:
    if (dev) {
        dev_close(dev);
    }
}


void doorbell_write_cfg_info(void)
{
#define CFG_LEN 1024
    void *fd = NULL;
    struct vfscan *fs = NULL;
    void *cfg =  NULL;

    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto __exit;
    }
    if (storage_device_ready()) {
        fs = fscan(CONFIG_ROOT_PATH, "-tINI -st");
        if (!fs) {
            goto __exit;
        }
        fd = fselect(fs, FSEL_FIRST_FILE, 0);
        if (fd) {
            cfg = calloc(1, CFG_LEN);
            if (!cfg) {
                goto __exit;
            }
            char name[128];
            fget_name(fd, name, sizeof(name));
            printf("\n name = %s\n", name);

            fread(fd, cfg, CFG_LEN);
            dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
            dev_ioctl(dev, IOCTL_ERASE_SECTOR, cfg_write_addr);
            dev_bulk_write(dev, cfg, cfg_write_addr, CFG_LEN); //此接口需要擦除flash之后才能写入
            extern int spi_flash_write_protect();
            spi_flash_write_protect();
            fdelete(fd);
        }
    }
__exit:
    if (cfg) {
        free(cfg);
    }
    if (fs) {
        fscan_release(fs);
    }
    if (dev) {
        dev_close(dev);
    }
}

int doorbell_read_cfg_did(char *buf, int buf_len)
{
    char *str = NULL;
    char *cfg = calloc(1, 1024);
    if (!cfg) {
        printf("\n %s %d\n", __func__, __LINE__);
        goto __exit;
    }
    doorbell_read_cfg_info(cfg, 1024);
    if (!strlen(cfg)) {
        printf("\n %s %d\n", __func__, __LINE__);
        free(cfg);
        goto __exit;
    }

    str = strstr(cfg, "did = ");
    if (str) {
        str += strlen("did = ");
        memset(buf, 0, buf_len);
        memcpy(buf, str, strstr(str, "\n") - str);
        printf("\n did = %s\n", buf);
    }

    free(cfg);
    return strlen(buf);
__exit:
    return 0;
}
