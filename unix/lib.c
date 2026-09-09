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

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../gblhdr.h"
#include "lib.h"

#ifndef HAVE_AT_FUNCTIONS

int fstatat(int dirfd, const char* pathname, struct stat* buf, int flags)
{
    int success = -1;

    if ((!flags || (flags == AT_SYMLINK_NOFOLLOW))) {
        int cwdfd = -1;
        if ((dirfd == AT_FDCWD) || (pathname && (*pathname == '/')) || (((cwdfd = open(".", O_RDONLY)) != -1) && !fchdir(dirfd))) {
            success = (!flags) ? stat(pathname, buf) : lstat(pathname, buf);
        }

        if (cwdfd != -1) {
            IGNORE_RESULT(fchdir(cwdfd));
            close(cwdfd);
        }
    } else {
        errno = EINVAL;
    }

    return (success);
}

#endif
