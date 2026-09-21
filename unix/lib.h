#ifndef LIB_H
#define LIB_H

/* usleep was obsolescent in POSIX.1-2001 and removed in POSIX.1-2008, so it
   is invisible under _POSIX_C_SOURCE=200809L. nanosleep replaced it. */
void zsleep_us(unsigned int usec);

#endif
