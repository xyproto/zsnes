#ifndef C_EXECLOOP_H
#define C_EXECLOOP_H

#include "../types.h"

/* What the assembly used to express by jumping back into the dispatch loop. */
enum exec_act {
    EXEC_NEXT, /* fetch the next opcode and dispatch it */
    EXEC_RELOAD, /* re-enter at the top of the loop, opcode index taken from dl */
    EXEC_SOUND, /* run the SPC catch-up burst, then carry on as EXEC_NEXT */
    EXEC_EXIT /* leave the dispatch loop */
};

enum exec_act c_cpuover(u4* r);
void exec_loop(u4* r, int at_cpuover);

#endif
