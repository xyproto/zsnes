#include <errno.h>
#include <time.h>

#include "lib.h"

/* Resumes after a signal rather than returning short, which is what the
   callers assumed of usleep. */
void zsleep_us(unsigned int const usec)
{
    struct timespec ts;

    ts.tv_sec = (time_t)(usec / 1000000u);
    ts.tv_nsec = (long)(usec % 1000000u) * 1000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
    }
}
