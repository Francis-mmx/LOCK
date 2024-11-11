#include "system/includes.h"

#define LOG_TAG "flash_wp"
#include "generic/log.h"

void test_wirte_protect()
{
    void *dev = dev_open("spiflash", NULL);
    if(!dev)
    {
        return ;
    }
    /*dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);*/
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, 0x7EE00);        //8*1024*1024-68*1024-4*1024
    u8 buf[512];
    memset(buf, 0xaa, sizeof(buf));
    dev_bulk_write(dev, buf, 0x7EEE00, sizeof(buf));
    memset(buf, 0xbb, sizeof(buf));
    dev_bulk_read(dev, buf, 0x7EEE00, sizeof(buf));
    printf("write flash ...\n");
    put_buf(buf, sizeof(buf));
    dev_close(dev);
    dev = NULL;
    //cpu_reset();
}
int spi_flash_write_protect()
{
    int i;
    int id;

    /* return 0; */

    if(!fdir_exist("mnt/spiflash"))
    {
        if(!mount("spiflash", "mnt/spiflash", "sdfile", 0, NULL))
        {
            return -EFAULT;
        }
    }

    FILE *file = fopen("mnt/spiflash/res/flash_cfg.bin", "r");
    if(!file)
    {
        return 0;
    }

    int len = flen(file);
    u32 *data = (u32 *)malloc(len);
    if(!data)
    {
        return -ENOMEM;
    }
    if(len != fread(file, data, len))
    {
        return -EFAULT;
    }
    fclose(file);

    void *dev = dev_open("spiflash", NULL);
    if(!dev)
    {
        return -EFAULT;
    }
    dev_ioctl(dev, IOCTL_GET_ID, (u32)&id);

    for(i = 1; i < len; i += 6)
    {
        if(data[i] == id)
        {
            u32 cmd = data[i + 1];
            log_v("flash_id = %x, write_protect\n", id);
            dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, cmd);
            goto __exit;
        }
    }
    log_w("flash_not_write_protect\n");

__exit:
    free(data);
    dev_close(dev);
    /* test_wirte_protect(); */

    return 0;
}
__initcall(spi_flash_write_protect);


