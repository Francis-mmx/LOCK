#include "system/includes.h"
#include "time.h"

extern u32 timer_get_ms(void);
static uint64_t old_utc_time;

#define FOURYEARDAY (365+365+365+366)  //4年一个周期内的总天数（1970~2038不存在2100这类年份，故暂不优化）
static const  uint8_t month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //平年
static const  uint8_t Leap_month_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //闰年
//const uint16_t dayPerYear[4] = {365, 365, 365, 366};

// 判断是否是闰年
// year: 需要判断的年
// return：1：闰年
//        0: 平年
static uint8_t isLeapYear(uint16_t year)
{
    uint8_t res = 0;
// 能够被4整除
    if (year % 4 == 0) { //能够被100整除，但是不能够被400整除
        if ((year % 100 == 0) && (year % 400 != 0)) {
            res = 0;
        } else {
            res = 1;
        }
    }
    return res;
}

static int dayofweek(int y, int m, int d)
{
    const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) {
        y -= 1;
    }
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

// 将北京时间转换为Unix时间戳
// year: 需要判断的年
// return：Unix时间戳（从1970/1/1 00:00:00 到现在的秒数)
// note：没对输入参数正确性做判断
uint64_t covBeijing2UnixTimeStp(struct tm *p)
{
    uint64_t daynum = 0, SecNum = 0; //保存北京时间到起始时间的天数
    uint16_t tempYear = 1970, tempMonth = 0;

    //1.年的天数
    while (tempYear < p->tm_year) {
        if (isLeapYear(tempYear)) {
            daynum += 366;
        } else {
            daynum += 365;
        }
        tempYear++;
    }
    //2.月的天数
    while (tempMonth < p->tm_mon - 1) {
        if (isLeapYear(p->tm_year)) { //闰年
            daynum += Leap_month_day[tempMonth];
        } else {
            daynum += month_day[tempMonth];
        }
        tempMonth++;
    }
    //3.天数
    daynum += (p->tm_mday - 1);

    //4.时分秒
    SecNum = daynum * 24 * 60 * 60; //s
    SecNum += p->tm_hour * 60 * 60;
    SecNum += p->tm_min * 60;
    SecNum += p->tm_sec;

    int TIMEZONE = db_select("tzone");
    //5.时区调整
    SecNum -= TIMEZONE * 60;

    return SecNum;
}
// 将Unix时间戳转换为北京时间
// unixTime: 需要判断的Unix时间戳
// *p:返回的北京时间
// return：none
// note：没对输入参数正确性做判断
void covUnixTimeStp2Beijing(uint64_t unixTime,  struct tm *p)
{
    uint32_t totleDayNum = 0, totleSecNum = 0;
    uint16_t remainDayofYear = 0, tempDay = 0, tempYear = 0;
    uint8_t *pr = NULL;


    memset(p, 0x00, sizeof(struct tm));

    totleDayNum = unixTime / (24 * 60 * 60); //总天数(注意加括号)
    totleSecNum = unixTime % (24 * 60 * 60); //当天剩余的秒速


    // 1.先计算时间 HH:MM:SS
    p->tm_hour = totleSecNum / 3600;
    p->tm_min = (totleSecNum % 3600) / 60; //error：变量搞错
    p->tm_sec = (totleSecNum % 3600) % 60;

    // 2.对时间进行时区调整（注意：这里可能造成日期 +1）
    int TIMEZONE = db_select("tzone");
    p->tm_hour += TIMEZONE / 60;
    if (p->tm_hour > 23) {
        //printf("modify day..\n");
        p->tm_hour -= 24;
        remainDayofYear++;  // 日期+1
    }

    // 3.计算哪一年
    p->tm_year = 1970 + (totleDayNum / FOURYEARDAY) * 4;   // 4年为一个周期
    remainDayofYear += totleDayNum % FOURYEARDAY;
    tempYear = isLeapYear(p->tm_year) ? 366 : 365;

    while (remainDayofYear >= tempYear) { // 计算4年整数倍外的年。
        p->tm_year++;
        remainDayofYear -= tempYear;
        tempYear = isLeapYear(p->tm_year) ? 366 : 365;
    }

    // 4.计算哪一月的哪一天
    pr = isLeapYear(p->tm_year) ? Leap_month_day : month_day;
    remainDayofYear++;          // 这里开始计算具体日期。remainDayofYear为 0 时其实是 1 号，所以这里要 +1
    p->tm_yday = remainDayofYear;

    while (remainDayofYear > *(pr + p->tm_mon)) {
        remainDayofYear -= *(pr + p->tm_mon);
        p->tm_mon++;
    }
    //printf("year:%d, day:%d.\n", p->tm_year, remainDayofYear);
    p->tm_mon++; //month
    p->tm_mday = remainDayofYear;  //day
    //printf("year:%d, day:%d.\n", p->tm_year, p->ui8DayOfMonth);
    p->tm_wday =  dayofweek(p->tm_year, p->tm_mon, p->tm_mday);
}

void set_utc_ms(uint64_t utc_ms)
{
    struct tm p = {0};;
    covUnixTimeStp2Beijing(utc_ms / 1000, &p);


    printf("%llu convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", utc_ms,
           p.tm_year, p.tm_mon, p.tm_mday, \
           p.tm_hour, p.tm_min, p.tm_sec, p.tm_wday, p.tm_yday);

    struct sys_time time;
    time.year = p.tm_year;
    time.month = p.tm_mon;
    time.day = p.tm_mday;
    time.hour = p.tm_hour;
    time.min = p.tm_min;
    time.sec = p.tm_sec;

    void *fd = dev_open("rtc", NULL);
    if (fd) {
        dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&time);
        dev_close(fd);
    }

    u32 cur_time = timer_get_ms();
    if (utc_ms < (uint64_t)cur_time) {
        return;
    }
    old_utc_time = utc_ms - (uint64_t)cur_time;
}

uint64_t get_utc_ms(void)
{
    uint64_t utc_ms = 0;
    utc_ms = old_utc_time + timer_get_ms();
////    printf("\n utc_ms =  %llu\n",utc_ms);
    return utc_ms;
}
