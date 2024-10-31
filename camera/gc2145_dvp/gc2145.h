
#ifndef _GC2145_H
#define  _GC2145_H

#define GC2145_OUTPUT_W   1280
#define GC2145_OUTPUT_H   720
s32 GC2145_set_output_size(u16 *width, u16 *height, u8 *freq);

unsigned char rdGC2145Reg(unsigned char regID, unsigned char *regDat);
unsigned char wrGC2145Reg(unsigned char regID, unsigned char regDat);
#endif



