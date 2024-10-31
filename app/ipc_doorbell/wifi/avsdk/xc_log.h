/*****************************************************************************
 * xc_log.h
 ****************************************************************************/

#ifndef XC_LOG_H_INCLUDE
#define XC_LOG_H_INCLUDE
/*****************************************************************************
 * Included Files
 ****************************************************************************/
// #include <stdio.h>
// #include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <stdarg.h>

/*****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/


/**
 * @brief Log level.
 */
#define EVOS_LOG_LEVEL_ALWAY 0
#define EVOS_LOG_LEVEL_NONE  1
#define EVOS_LOG_LEVEL_ERROR 2
#define EVOS_LOG_LEVEL_WARN  3
#define EVOS_LOG_LEVEL_INFO  4
#define EVOS_LOG_LEVEL_DEBUG 5
#define EVOS_LOG_LEVEL_TRACE 6

#define evos_log_run_level  EVOS_LOG_LEVEL_TRACE
/**
 * @brief Log level tag.
 */
#define EVOS_LOG_TAG_ALWAY   "ALWAY"
#define EVOS_LOG_TAG_ERROR   "ERROR"
#define EVOS_LOG_TAG_WARN    "WARN "
#define EVOS_LOG_TAG_INFO    "INFO "
#define EVOS_LOG_TAG_DEBUG   "DEBUG"
#define EVOS_LOG_TAG_TRACE   "TRACE"

/**
 * @def EVOS_ABS
 * @brief
 */
#define EVOS_ABS(x) ((x) > 0 ? (x) : -(x))

/**
 * @def EVOS_MIN
 * @brief
 */
#define EVOS_MIN(x, y) ((x) < (y) ? (x) : (y))
/**
 * @def EVOS_MAX
 * @brief
 */
#define EVOS_MAX(x, y) ((x) > (y) ? (x) : (y))

/* LOG Enable */
#define EVOS_LOG_ENABLE

#if defined(EVOS_LOG_ENABLE)

/* Log domain name */
#if !defined(EVOS_LOG_DOMAIN)
#define EVOS_LOG_DOMAIN "General"
#endif

/* Log compile level */
#if !defined(EVOS_LOG_COMPILE_LEVEL)
#define EVOS_LOG_COMPILE_LEVEL  EVOS_LOG_LEVEL_DEBUG
#endif

/* Log run level */
#if !defined(EVOS_LOG_RUN_LEVEL)
#define EVOS_LOG_RUN_LEVEL EVOS_LOG_LEVEL_DEBUG
#endif



static void logLowPrintf(const char *fmt, ...)
{

    va_list ap;
    va_start(ap, fmt);
    // vfprintf(stderr, fmt, ap);
    vprintf(fmt, ap);
    va_end(ap);
    //fflush(stderr);
}


/* Log print func */
#if 0
#if !defined(EVOS_LOG_BACKEND_FN)
extern int SEGGER_RTT_printf2(const char *sFormat, ...) ;
#define EVOS_LOG_BACKEND_FN SEGGER_RTT_printf2
#endif
#else
#if !defined(EVOS_LOG_BACKEND_FN)
#define EVOS_LOG_BACKEND_FN logLowPrintf//printf
#endif
#endif

static void *evos_MemAlloc(uint32_t size)
{
    void *mem = NULL;

    mem = malloc((size_t)size);
    if (mem) {
        memset(mem, 0, size);
    }
    return mem;
}

static void evos_MemFree(void *mem)
{

    if (mem) {
        free(mem);
    }
}

/* Log level tag */
static const char *evosLogLevelTag(uint8_t level)
{
    const char *tag = NULL;

    switch (level) {
    case EVOS_LOG_LEVEL_ALWAY:
        tag = EVOS_LOG_TAG_ALWAY;
        break;
    case EVOS_LOG_LEVEL_ERROR:
        tag = EVOS_LOG_TAG_ERROR;
        break;
    case EVOS_LOG_LEVEL_WARN:
        tag = EVOS_LOG_TAG_WARN;
        break;
    case EVOS_LOG_LEVEL_INFO:
        tag = EVOS_LOG_TAG_INFO;
        break;
    case EVOS_LOG_LEVEL_DEBUG:
        tag = EVOS_LOG_TAG_DEBUG;
        break;
    case EVOS_LOG_LEVEL_TRACE:
        tag = EVOS_LOG_TAG_TRACE;
        break;
    default:
        tag = "";
        break;
    }

    return tag;
}

static char *osal_LogTagTimestamp(char *buffer)
{
    uint16_t time_zone = 8 * 60;
    const uint8_t Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    extern uint64_t osal_clock_utctime(void);
    uint64_t stamp = osal_clock_utctime();

    uint16_t year = 0;
    uint8_t month = 0, date = 0;

    uint32_t days = 0;
    uint16_t leap_num;
    uint16_t mseconds = 0;
    uint8_t seconds, minutes, hours;

    (void)mseconds;
    (void)seconds;
    (void)minutes;
    (void)hours;

    mseconds = stamp % 1000;
    stamp /= 1000;	//seconds
    seconds = stamp % 60;
    stamp /= 60;	//minutes
    minutes = stamp % 60;
    stamp += time_zone;
    stamp /= 60;	//hours
    hours = stamp % 24;
    days = stamp / 24;
    leap_num = (days + 365) / 1461;
    //T_T("days=%d, leap_num=%d", days, leap_num);
    if (((days + 366) % 1461) == 0) {
        year = (days / 366) + 1970;
        month = 12;
        date = 31;
    } else {
        days -= leap_num;
        year = (days / 365) + 1970;
        days %= 365;
        days += 1;
        if (((year % 4) == 0) && (days == 60)) {
            month = 2;
            date = 29;
        } else {
            if (((year % 4) == 0) && (days > 60)) {
                --days;
            }
            for (month = 0; Days[month] < days; month++) {
                days -= Days[month];
            }
            ++month;
            date = days;
        }
    }

    sprintf(buffer,
            "%.04d-%.02d-%.02d %.02d:%.02d:%.02d.%.03ld",
            year,
            month,
            date,
            hours,
            minutes,
            seconds,
            mseconds);

    return buffer;

}

/* Log timestamp tag */
static const char *evosLogTimeStampTag(void)
{
    /* Tag Timestamp */
    static char timestamp[32];
    osal_LogTagTimestamp(timestamp);
    return timestamp;
}

static int evosLogHexDumpN(const void *buf, int len,
                           char *dst, int dst_len,
                           int offset)
{
    const unsigned char *p = (const unsigned char *) buf;
    char ascii[17] = "";
    int i, idx, n = 0;

    if (offset >= len) {
        dst[0] = '\0';
        return 0;
    }

    for (i = 0; i < len; i++) {
        idx = i % 16;
        if (idx == 0) {
            if (i > 0) {
                n += snprintf(dst + n, EVOS_MAX(dst_len - n, 0), " |%s|\n", ascii);
            }
            n += snprintf(dst + n, EVOS_MAX(dst_len - n, 0), "%04x ", i + offset);
        }
        if (dst_len - n < 0) {
            return n;
        }
        n += snprintf(dst + n, EVOS_MAX(dst_len - n, 0), " %02x", p[i]);
        if (i % 8 == 7) {
            n += snprintf(dst + n, EVOS_MAX(dst_len - n, 0), "%s", " ");
        }
        ascii[idx] = p[i] < 0x20 || p[i] > 0x7e ? '.' : p[i];
        ascii[idx + 1] = '\0';
    }

    while (i++ % 16) {
        n += snprintf(dst + n, EVOS_MAX(dst_len - n, 0), "%s", "   ");
        if (i % 8 == 7) {
            n += snprintf(dst + n, EVOS_MAX(dst_len - n, 0), "%s", " ");
        }
    }
    n += snprintf(dst + n, EVOS_MAX(dst_len - n, 0), " |%s|\n", ascii);

    return n;
}

/* Log hex dump */
__attribute__((used))
static void evosLogHexDump(
    const char    *domain,
    uint8_t        level,
    const char    *prefix,
    int            line,
    const uint8_t *pByte,
    uint32_t       byteCnt)
{
#if 1
    char *buffer;
    int len = byteCnt * 256;

    buffer = evos_MemAlloc(len);
    if (buffer != NULL) {
        evosLogHexDumpN(pByte, byteCnt, buffer, len, 0);

        (void)(EVOS_LOG_BACKEND_FN)(
            "[%s] [%s] [%-16.16s/%05d]: %s(%d): \n%s\r\n",
            evosLogLevelTag(level),
            evosLogTimeStampTag(),
            domain, line,
            prefix, byteCnt,
            buffer);
        evos_MemFree(buffer);
    }
#endif
}


/* EVOS_LOG */
#define EVOS_LOG(level, fmt, ...)                                            \
    do {                                                                     \
        if (EVOS_LOG_RUN_LEVEL >= level) {                                   \
            (void)(EVOS_LOG_BACKEND_FN)(                                     \
                "[%s] [%s] [%-12.12s/%05d] [%-16.16s]: " fmt "\n",              \
                evosLogLevelTag(level),                                      \
                evosLogTimeStampTag(),                                       \
                 __FUNCTION__, __LINE__, EVOS_LOG_DOMAIN,                                   \
                ##__VA_ARGS__);                                              \
        }                                                                    \
    } while (0)

/* EVOS_LOG_HEX_DUMP */
#define EVOS_LOG_HEX_DUMP(level, prefix, byte_p, byte_cnt)                   \
    do {                                                                     \
        if (EVOS_LOG_RUN_LEVEL >= level) {                                   \
            evosLogHexDump(EVOS_LOG_DOMAIN, level, prefix, __LINE__,            \
                              (const uint8_t *)byte_p, (uint32_t)byte_cnt);  \
        }                                                                    \
    } while (0)



#if 1
#define OSAL_LOG_MESSAGE(level, fmt, ...)            \
                do {                                                 \
                    if (EVOS_LOG_LEVEL_TRACE >= level) {               \
                        EVOS_LOG(                                    \
                            level, fmt, ##__VA_ARGS__);                \
                    }                                                \
                } while (0)

/* log hexdump */
#define OSAL_LOG_HEXDUMP(level, prefix, byte_p, byte_cnt)    \
                do {                                                 \
                    if (EVOS_LOG_LEVEL_TRACE >= level) {               \
                        EVOS_LOG_HEX_DUMP(                           \
                            level, prefix, byte_p, byte_cnt);                \
                    }                                                \
                } while (0)
#endif


/**
 * @def T_E/T_E_HEX/T_EH_HEX
 *
 * @brief Writes an ERROR level message to the log.
 *
 * @details Lowest logging level, these messages are logged whenever osal log is
 * active. it's meant to report severe errors, such as those from which it's
 * not possible to recover.
 *
 * @param ... A string optionally containing printf valid conversion specifier,
 * followed by as many values as specifiers.
 */

/**
 * @def T_W/T_W_HEX/T_WH_HEX
 *
 * @brief Writes a WARNING level message to the log.
 *
 * @details available if OSAL_LOG_LEVEL is OSAL_LOG_LEVEL_WARN or higher.
 * It's meant to register messages related to unusual situations that are
 * not necessarily errors.
 *
 * @param ... A string optionally containing printf valid conversion specifier,
 * followed by as many values as specifiers.
 */

/**
 * @def T_I/T_I_HEX/T_IH_HEX
 *
 * @brief Writes an INFO level message to the log.
 *
 * @details available if OSAL_LOG_LEVEL is OSAL_LOG_LEVEL_INFO or higher.
 * It's meant to write generic user oriented messages.
 *
 * @param ... A string optionally containing printf valid conversion specifier,
 * followed by as many values as specifiers.
 */

/**
 * @def T_D/T_D_HEX/T_DH_HEX
 *
 * @brief Writes a DEBUG level message to the log.
 *
 * @details highest logging level, available if OSAL_LOG_LEVEL is
 * OSAL_LOG_LEVEL_DEBUG. It's meant to write developer oriented information.
 *
 * @param ... A string optionally containing printf valid conversion specifier,
 * followed by as many values as specifiers.
 */

/**
 * @def T_T/T_T_HEX/T_TH_HEX
 *
 * @brief Writes a TRACE level message to the log.
 *
 * @details highest logging level, available if OSAL_LOG_LEVEL is
 * OSAL_LOG_LEVEL_TRACE. It's meant to write internal developer oriented information.
 *
 * @param ... A string optionally containing printf valid conversion specifier,
 * followed by as many values as specifiers.
 */
#if (EVOS_LOG_COMPILE_LEVEL >= EVOS_LOG_LEVEL_ALWAY)
/* T_A For printing very important information */
#define T_A(...)  OSAL_LOG_MESSAGE(EVOS_LOG_LEVEL_ALWAY, ##__VA_ARGS__)
#define T_A_HEX(pByte, byteCnt)                                              \
                OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_ALWAY, "",       (pByte), (byteCnt))
#define T_AH_HEX(prefix, pByte, byteCnt)                                     \
                OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_ALWAY, (prefix), (pByte), (byteCnt))
#endif


#if (EVOS_LOG_COMPILE_LEVEL >= EVOS_LOG_LEVEL_ERROR)
#define T_E(...)  OSAL_LOG_MESSAGE(EVOS_LOG_LEVEL_ERROR, ##__VA_ARGS__)
#define T_E_HEX(pByte, byteCnt)                                              \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_ERROR, "",       (pByte), (byteCnt))
#define T_EH_HEX(prefix, pByte, byteCnt)                                     \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_ERROR, (prefix), (pByte), (byteCnt))
#endif

#if (EVOS_LOG_COMPILE_LEVEL >= EVOS_LOG_LEVEL_WARN)
#define T_W(...)  OSAL_LOG_MESSAGE(EVOS_LOG_LEVEL_WARN, ##__VA_ARGS__)
#define T_W_HEX(pByte, byteCnt)                                              \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_WARN, "",       (pByte), (byteCnt))
#define T_WH_HEX(prefix, pByte, byteCnt)                                     \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_WARN, (prefix), (pByte), (byteCnt))
#endif

#if (EVOS_LOG_COMPILE_LEVEL >= EVOS_LOG_LEVEL_INFO)
#define T_I(...)  OSAL_LOG_MESSAGE(EVOS_LOG_LEVEL_INFO, ##__VA_ARGS__)
#define T_I_HEX(pByte, byteCnt)                                              \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_INFO, "",       (pByte), (byteCnt))
#define T_IH_HEX(prefix, pByte, byteCnt)                                     \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_INFO, (prefix), (pByte), (byteCnt))
#endif

#if (EVOS_LOG_COMPILE_LEVEL >= EVOS_LOG_LEVEL_DEBUG)
#define T_D(...)  OSAL_LOG_MESSAGE(EVOS_LOG_LEVEL_DEBUG, ##__VA_ARGS__)
#define T_D_HEX(pByte, byteCnt)                                              \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_DEBUG, "",       (pByte), (byteCnt))
#define T_DH_HEX(prefix, pByte, byteCnt)                                     \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_DEBUG, (prefix), (pByte), (byteCnt))
#endif

#if (EVOS_LOG_COMPILE_LEVEL >= EVOS_LOG_LEVEL_TRACE)
#define T_T(...)  OSAL_LOG_MESSAGE(EVOS_LOG_LEVEL_TRACE, ##__VA_ARGS__)
#define T_T_HEX(pByte, byteCnt)                                              \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_TRACE, "",       (pByte), (byteCnt))
#define T_TH_HEX(prefix, pByte, byteCnt)                                     \
    OSAL_LOG_HEXDUMP(EVOS_LOG_LEVEL_TRACE, (prefix), (pByte), (byteCnt))
#endif
#endif /* defined(OSAL_LOG_ENABLE) */

/* create dummy macros */
#ifndef T_E
#define T_E(...)
#endif

#ifndef T_W
#define T_W(...)
#endif

#ifndef T_I
#define T_I(...)
#endif

#ifndef T_D
#define T_D(...)
#endif

#ifndef T_T
#define T_T(...)
#endif

#ifndef T_E_HEX
#define T_E_HEX(...)
#endif

#ifndef T_W_HEX
#define T_W_HEX(...)
#endif

#ifndef T_I_HEX
#define T_I_HEX(...)
#endif

#ifndef T_D_HEX
#define T_D_HEX(...)
#endif

#ifndef T_T_HEX
#define T_T_HEX(...)
#endif

#ifndef T_EH_HEX
#define T_EH_HEX(...)
#endif

#ifndef T_WH_HEX
#define T_WH_HEX(...)
#endif

#ifndef T_IH_HEX
#define T_IH_HEX(...)
#endif

#ifndef T_DH_HEX
#define T_DH_HEX(...)
#endif

#ifndef T_TH_HEX
#define T_TH_HEX(...)
#endif

#if 1

#define OSAL_ASSERT_ENABLE

#ifdef OSAL_ASSERT_ENABLE

#define OSAL_STRFUNC ((const char *)(__FUNCTION__))

#define OSAL_ASSERT(exp)                                                     \
    do {                                                                     \
        if (!(exp)) {                                                        \
            T_E("Assert to %s[%s:%d]", #exp, OSAL_STRFUNC, __LINE__);        \
        }                                                                    \
    } while (0)
#else
/* create dummy macros */
#define OSAL_ASSERT(exp)
#endif
#endif

/*****************************************************************************
 * Public Types
 ****************************************************************************/

/*****************************************************************************
 * Public Data
 ****************************************************************************/

/*****************************************************************************
 * Inline Functions
 ****************************************************************************/



/****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************/
#ifdef __cplusplus
}
#endif
/****************************************************************************/
#endif /* EVOS_LOG_H_INCLUDE */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

