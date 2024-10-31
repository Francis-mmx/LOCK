#ifndef _TIME_COMPILE_H
#define _TIME_COMPILE_H

unsigned char DataStr[] = __DATE__; //获取编译的日期
unsigned char TimeStr[] = __TIME__; //获取编译的时间

#define YEAR ((((__DATE__[7]-'0')*10+(__DATE__[8]-'0'))*10 \
+(__DATE__[9]-'0'))*10+(__DATE__[10]-'0'))

#define MONTH (__DATE__[2]=='n'?1 \
:__DATE__[2]=='b'?2 \
:__DATE__[2]=='r'?(__DATE__[0]=='M'?3:4) \
:__DATE__[2]=='y'?5 \
:__DATE__[2]=='n'?6 \
:__DATE__[2]=='l'?7 \
:__DATE__[2]=='g'?8 \
:__DATE__[2]=='p'?9 \
:__DATE__[2]=='t'?10 \
:__DATE__[2]=='v'?11:12)

#define DAY ((__DATE__[4]==' '?0:__DATE__[4]-'0')*10 \
+(__DATE__[5]-'0'))

//#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)

#define HOUR ((__TIME__[0]-'0')*10+(__TIME__[1]-'0'))
#define MINUTE ((__TIME__[3]-'0')*10+(__TIME__[4]-'0'))
#define SECOND ((__TIME__[6]-'0')*10+(__TIME__[7]-'0'))
//
//#define TIME_AS_INT (((((YEAR - 2000) * 12 + MONTH) * 31 + DAY)*12+HOUR)*60+MINUTE)*60+SECOND

#endif
