/*
 * test/difftest_dbg.c - the debug 65816 core (cpu/c_ops65816_dbg.c) vs
 * cpu/e65816c.inc.
 *
 * The third instantiation. This core shares the 65816's register file and
 * renames only the opcode table, so what is under test is the handful of
 * handlers that genuinely differ - BRK's flag byte, BRL's base, RTI's extra WAI
 * check, CLI, and the thirteen push/pull opcodes.
 */
#define OP(n) c_dbg##n
#define ASMOP(n) asm_c##n
#define CORENAME "debug 65816"
#define CLI_RETURNS_VOID
#define OPS_IMPL "ops65816_dbg.h"

#define tablead tableadc

#include "difftest_op.c"
