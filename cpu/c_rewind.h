#ifndef C_REWIND_H
#define C_REWIND_H

#include "../types.h"

/* Called from the pushad thunks in cpu/execute.asm; r points at the saved
   register block (see the enum in c_rewind.c). */
void ProcessRewindC(zreg* r);
void UpdateRewindC(zreg* r);

#endif
