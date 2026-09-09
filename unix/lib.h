#ifndef LIB_H
#define LIB_H

#include <sys/stat.h>

/* usleep was obsolescent in POSIX.1-2001 and removed in POSIX.1-2008, so it
   is invisible under _POSIX_C_SOURCE=200809L. nanosleep replaced it. */
void zsleep_us(unsigned int usec);

#ifndef HAVE_AT_FUNCTIONS

#ifndef AT_FDCWD
#define AT_FDCWD -2
#endif

#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 1
#endif

int fstatat(int dirfd, const char* pathname, struct stat* buf, int flags);

#endif

#endif
