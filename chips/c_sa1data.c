#include "../types.h"

u1 SA1Status;
u1 CurrentExecSA1;
u1 CurrentCPU;
void* prevedi; /* 65816 PC across an SA-1 switch; not saved */
u4 SA1xpc;
