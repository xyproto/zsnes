/* Debug 65816 core differential test. */
#define OP(n) c_dbg##n
#define ASMOP(n) asm_c##n
#define CORENAME "debug 65816"
#define CLI_RETURNS_VOID
#define OPS_IMPL "ops65816_dbg.h"

#define tablead tableadc

#include "difftest_op.c"
