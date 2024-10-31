#ifndef __CLOUD_UPGRADE_H__
#define __CLOUD_UPGRADE_H__

struct cloud_upgrade_info {
    long long total_size;
    long long offset;
    long long type;
    char name[64];
    char md5[64];
    char buffer[1500];
    u32 len;
};

#endif
