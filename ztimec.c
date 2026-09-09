#include <time.h>

#include "ztimec.h"

u4 GetTimeInSeconds(void)
{
    time_t current;
    struct tm* timeptr;

    time(&current);
    timeptr = localtime(&current);
    return (timeptr->tm_hour * 60 + timeptr->tm_min) * 60 + timeptr->tm_sec;
}

u4 GetTime(void)
{
    unsigned int value;
    struct tm* newtime;
    time_t long_time;

    time(&long_time);
    newtime = localtime(&long_time);

    value = ((newtime->tm_sec) % 10) + ((newtime->tm_sec) / 10) * 16
        + ((((newtime->tm_min) % 10) + ((newtime->tm_min) / 10) * 16) << 8)
        + ((((newtime->tm_hour) % 10) + ((newtime->tm_hour) / 10) * 16) << 16);
    return (value);
}

u4 GetDate(void)
{
    unsigned int value;
    struct tm* newtime;
    time_t long_time;

    time(&long_time);
    newtime = localtime(&long_time);
    value = ((newtime->tm_mday) % 10) + ((newtime->tm_mday) / 10) * 16
        + (((newtime->tm_mon) + 1) << 8)
        + ((((newtime->tm_year) % 10) + ((newtime->tm_year) / 10) * 16) << 16)
        + ((newtime->tm_wday) << 28);
    return (value);
}
