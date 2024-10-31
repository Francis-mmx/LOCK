#ifndef  __USB_TYPES_H__
#define  __USB_TYPES_H__
#include "system/includes.h"
typedef u8 __u8;
typedef u16 __le16;
typedef u32 __le32;
typedef u16 __u16;
typedef u32 __u32;

#define ___ntohl(X)     ((((u16)(X) & 0xff00) >> 8) |(((u16)(X) & 0x00ff) << 8))

#define ___ntohs(X)     ((((u32)(X) & 0xff000000) >> 24) | \
                         (((u32)(X) & 0x00ff0000) >> 8) | \
                         (((u32)(X) & 0x0000ff00) << 8) | \
                         (((u32)(X) & 0x000000ff) << 24))

#if defined(cpu_to_be16) || defined(cpu_to_be32) || defined(be16_to_cpu) || defined(be32_to_cpu)
#error #define cpu_to_be16
#endif


//#ifdef USB_LITTLE_ENDIAN
//#elif	defined(USB_BIG_ENDIAN)
//#error not support bigendian
//#else
//#error not define endian
//#endif

#define cpu_to_be16(v16) ___ntohl(v16)
#define cpu_to_be32(v32) ___ntohs(v32)

#define be16_to_cpu(v16) cpu_to_be16(v16)
#define be32_to_cpu(v32) cpu_to_be32(v32)
#define __le16_to_cpu(v16)     (v16)
#define __le32_to_cpu(v32)     (v32)

#if defined(cpu_to_le16) || defined(cpu_to_le32) || defined(le16_to_cpu) || defined(le32_to_cpu)
#error #define cpu_to_be16
#endif

#define cpu_to_le16(v16) (v16)
#define cpu_to_le32(v32) (v32)

#define le16_to_cpu(v16) cpu_to_le16(v16)
#define le32_to_cpu(v32) cpu_to_le32(v32)

#define LOWORD(l)           ((u16)(l))
#define HIWORD(l)           ((u16)(((u32)(l) >> 16) & 0xFFFF))


#define LOBYTE(w)           ((u8)((w) & 0xff))
#define HIBYTE(w)           ((u8)(((u16)(w) >> 8) & 0xFF))

#define DW1BYTE(dw)         (LOBYTE(LOWORD(dw)))
#define DW2BYTE(dw)         (HIBYTE(LOWORD(dw)))
#define DW3BYTE(dw)         (LOBYTE(HIWORD(dw)))
#define DW4BYTE(dw)         (HIBYTE(HIWORD(dw)))





#endif  /*USB_TYPES_H*/
