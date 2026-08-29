#ifndef C_PROCWIN_H
#define C_PROCWIN_H

#include "../types.h"

/* The four registers procwindowback16t changes, which clearback16bts reads
   straight afterwards. See video/c_procwin.c. */
typedef struct
{
    zreg ax, bx, cx, si;
} pwregs;

void c_procwindowback16t(pwregs* r);

#endif
