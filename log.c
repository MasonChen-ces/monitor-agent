#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
int debug(const char *format, ...)
{
    int ret = 0;
#ifdef DEBUG
    time_t t;
    struct tm *tm;
    char buf[80];
    char str[1024];
    va_list args;
    memset(buf, 0, 80);
    memset(str, 0, 1024);
    t = time(NULL);
    tm = localtime(&t);
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", tm);
    printf("%s [DEBUG] -> ", buf);
    va_start(args, format);
    ret = vsprintf(str, format, args);
    va_end(args);
    printf(str);
#endif
    return ret;
}

int info(const char *format, ...)
{
    int ret = 0;
    time_t t;
    struct tm *tm;
    char buf[80];
    char str[1024];
    va_list args;
    memset(buf, 0, 80);
    t = time(NULL);
    tm = localtime(&t);
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", tm);
    printf("%s [INFO] -> ", buf);
    va_start(args, format);
    ret = vsprintf(str, format, args);
    va_end(args);
    printf(str);
    return ret;
}