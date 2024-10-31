#ifndef  __VM_API_H__
#define  __VM_API_H__


#include "generic/ioctl.h"
#include "device/device.h"

extern const struct device_operations vm_dev_ops;


#define DEV_VM_ERASE 			        _IO('M', 1)
#define DEV_VM_GET_STATUS 			_IOR('M', 2, sizeof(u32))
#define DEV_GET_VM_BASE_ADDR   			_IOR('M', 3, sizeof(u32))

struct vm_arg {
    bool need_defrag_when_write_full;
    bool large_mode;
    u32 vm_magic;
};

#endif




