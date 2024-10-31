#include "app_config.h"
#include "ftpserver/stupid-ftpd.h"
#include "upgrade/upgrade_detect.h"
#include "network_upgrade.h"



static void *stupid_vfs_open(char *path,  char *mode)
{
    printf("\n stupid_vfs_open : path is %s , mode is %s\n", path, mode);
    return network_upgrade_open(path, mode);
}

static int stupid_vfs_write(void  *file, void  *buf, u32 len)
{
    return network_upgrade_write(file, buf, len);
}

static int stupid_vfs_read(void  *file, void  *buf, u32 len)
{
    return network_upgrade_read(file, buf, len);
}

static int stupid_vfs_flen(void  *file)
{
    return network_upgrade_flen(file);
}

static int stupid_vfs_close(void *file)
{
    return network_upgrade_close(file);
}

//注册一个ftpd的vfs接口

void ftpd_vfs_interface_cfg(void)
{
    struct ftpd_vfs_cfg info;
    info.fopen = stupid_vfs_open;
    info.fwrite = stupid_vfs_write;
    info.fread = stupid_vfs_read;
    info.flen = stupid_vfs_flen;
    info.fclose = stupid_vfs_close;
//注册接口到ftp中
    stupid_vfs_reg(&info);
}

