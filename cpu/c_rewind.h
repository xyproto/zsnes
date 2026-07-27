#ifndef C_REWIND_H
#define C_REWIND_H

#include "../types.h"

/* Called from the pushad thunks in cpu/execute.asm; r points at the saved
   register block (see the enum in c_rewind.c). */
void ProcessRewindC(u4* r);
void UpdateRewindC(u4* r);

#endif
