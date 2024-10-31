#ifndef SDMMC_MODULE_H
#define SDMMC_MODULE_H


#include "generic/typedef.h"
#include "generic/ioctl.h"


#define     SD_CMD_DECT 	0
#define     SD_CLK_DECT  	1
#define     SD_IO_DECT 		2

#define    SD_CLASS_0      0
#define    SD_CLASS_2      1
#define    SD_CLASS_4      2
#define    SD_CLASS_6      3
#define    SD_CLASS_10     4

#define SD_IOCTL_GET_CLASS  _IOR('S', 0, 4)
#define SD_IOCTL_GET_CID    _IOR('S', 1, 4)

typedef struct _CID_STRUCT {
    char MID;   //制造商ID
    char OID[3];//OEM/应用ID
    char PNM[6];//产品名称
    char PRV;   //产品版本
    u8  month;  //生产日期月
    u16  year;  //生产日期年
    u32  PSN;   //产品序列号
} SD_CMD_CID ;














#endif

