#include "app_config.h"
#include "typedef.h"



#ifndef CONFIG_DEBUG_ENABLE

int putchar(int a)
{
    return a;
}

int puts(const char *out)
{
    return 0;
}

int printf(const char *format, ...)
{
    return 0;
}


void put_buf(const u8 *buf, int len)
{

}

void put_u8hex(u8 dat)
{

}

void put_u16hex(u16 dat)
{

}

void put_u32hex(u32 dat)
{

}

void log_print(int level, const char *tag, const char *format, ...)
{

}

#endif

#if 0
void log_print_time()
{

}
#endif




