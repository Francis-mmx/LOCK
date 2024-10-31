#ifndef __H62_DVP_H__
#define __H62_DVP_H__

#include "typedef.h"
#include "asm/isp_dev.h"

#define H62_DVP_OUTPUT_W     1280
#define H62_DVP_OUTPUT_H      720


#define H62_VARIABLE_FPS    0

s32 H62_DVP_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 H62_DVP_power_ctl(u8 isp_dev, u8 is_work);

s32 H62_DVP_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio);
s32 H62_DVP_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

void H62_DVP_sleep();
void H62_DVP_wakeup();
void H62_DVP_W_Reg(u16 addr, u16 val);
u16 H62_DVP_R_Reg(u16 addr);


#endif


