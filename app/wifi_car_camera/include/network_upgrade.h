#ifndef __NETWORK_UPGRADE_H__
#define __NETWORK_UPGRADE_H__
#include "app_config.h"

#define UPGRADE_UI_PATH		    "ram/bfu/C/"
#define UPGRADE_FILE_NAME	    "JL_AC5X.bfu"
#define UPGRADE_SFC_FILE_NAME	"JL_ACSFC.bfu"
#define UPGRADE_PATH		    CONFIG_ROOT_PATH

#define UPGRADE_FILE_PATH	    UPGRADE_PATH\
                                UPGRADE_FILE_NAME

#define UPGRADE_SFC_FILE_PATH	UPGRADE_PATH\
                                UPGRADE_SFC_FILE_NAME

//0 只是SD卡文件传输,
//1 支持升级到flash(默认升级文件名：JL_AC5X.bfu / JL_ACSFC.bfu，工具生成：wifi网络升级文件.bat / wifi网络升级文件-sfc.bat)
//注意：用升级到flash时候会删除SD卡的升级文件
//无卡或者sfc网络升级配置在:network_upgrade.h，设置NETWORK_UPGRADE_TO_FLASH 为 1 即可
#ifdef CONFIG_SFC_ENABLE
#define NETWORK_UPGRADE_TO_FLASH	1   // 跑SFC，flash升级优先
#else
#define NETWORK_UPGRADE_TO_FLASH	0   // 不跑SFC，SD卡升级为优先
#endif


void *network_upgrade_open(char *path, char *mode);
int network_upgrade_write(void *file, char *buf, int len);
int network_upgrade_read(void *file, char *buf, int len);
int network_upgrade_flen(void *file);
int network_upgrade_close(void *file);
int network_upgrade_request(char *buf, u32 len);


#endif

