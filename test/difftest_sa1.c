/*
 * test/difftest_sa1.c - the SA-1's 65816 core (cpu/c_ops65816_sa1.c) vs
 * cpu/se65816.inc.
 *
 * The port instantiates cpu/ops65816.h a second time over the SA-1's register
 * file; this instantiates the difftest the same way, so what is being checked
 * is the wiring - that the renamed globals really are the ones the assembly
 * uses, and that the six handlers the SA-1 writes for itself are right. The
 * opcode logic itself is already covered by test/difftest_op.c.
 */
#define OP(n) c_SA1##n
#define ASMOP(n) asm_SA1##n
#define CORENAME "SA-1 65816"
#define CLI_RETURNS_VOID
#define OPS_IMPL "ops65816_sa1.h"

#define xa SA1xa
#define xx SA1xx
#define xy SA1xy
#define xs SA1xs
#define xd SA1xd
#define xdb SA1xdb
#define xpb SA1xpb
#define flagnz Sflagnz
#define flago Sflago
#define flagc Sflagc
#define tablead SA1tablead
#define DPageR8 SA1DPageR8
#define DPageR16 SA1DPageR16
#define DPageW8 SA1DPageW8
#define DPageW16 SA1DPageW16
#define UpdateDPage SA1UpdateDPage

#include "difftest_op.c"
