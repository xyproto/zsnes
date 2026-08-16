/*
 * test/difftest_op.c - 65816 opcode handlers (cpu/ops65816.h) vs cpu/e65816.inc.
 *
 * The oracle is built with `--rewrite-macro endloop=ret`, which is what makes a
 * single opcode callable: normally an opcode ends by dispatching the next one,
 * so nothing short of running a ROM exercises just one. With that rewrite each
 * COpXX is an ordinary function taking the core's register file in registers.
 *
 * Both sides get the same random register file and the same random 65816
 * register globals, and every one of them is compared afterwards - the
 * assembly writes A/X/Y at the width the current mode selects and leaves the
 * bytes above it alone, so a C port that stores the whole 32 bits looks right
 * on the emulator and wrong here.
 */
/*
 * Parameterised the same way cpu/ops65816.h is, so test/difftest_sa1.c can
 * include this file to check the SA-1 instantiation against cpu/se65816.inc.
 */
#ifndef OP
#define OP(n) c_##n
#endif
#ifndef ASMOP
#define ASMOP(n) asm_##n
#endif
#ifndef CORENAME
#define CORENAME "65816"
#endif
/* An instantiation may deliberately not reproduce the assembly - the SA-1 has
   one opcode where the original is wrong. Named here so it is reported rather
   than silently skipped. */
#ifndef KNOWN_DIVERGENCE
#define KNOWN_DIVERGENCE(name) 0
#endif

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int8_t s1;
typedef int32_t s4;
typedef void eop();

/* The 65816 register file, normally initdata.c's. */
u4 xa, xx, xy, xs, xd;
u1 xe;
u4 flagnz, flago, flagc;
u2 stackand, stackor;

/* TCD tells the memory tables the direct page moved; both sides call the
   same do-nothing stand-in, so it cancels out. */
void UpdateDPage(void) { }
/* Wider than the emulator's 256 so the test can put something in the top
   bits of ebx and still index it: that is how the port's preservation of
   those bits gets checked. */
u4 tablead[1024];

u4 xdb, xpb;
u2 xpc;
u2 brkv, brkv8, copv, copv8;
u1 xirqb, intrset, doirqnext, curnmi;
u4 nmistatus, curexecstate;
u1* initaddrl;
u1* wramdata;
/* An array, not a pointer: the assembly writes `dmadata-4300h`, which is
   address arithmetic on the symbol. Big enough that base + a 16-bit
   address stays inside it. */
u1 dmadata[0x10000];

/* The jumps land anywhere in a 64K bank, so the "ROM" has to be that big and
   the bank bases have to sit at the bottom of it. */
#define CODESZ 0x20000u
static u1 code[CODESZ];
static u1 wram[0x10000];
/* Randomised once per opcode; the per-iteration copy is what matters, not
   fresh noise every time. */
static u1 wram_seed[0x10000];

/* Stands in for the routine the core recognises by address to detect a
   register bank - JMP/JSR/RTI route $4300 and up there specially. It has to be
   a working read as well, since it is also a table entry. */
void regaccessbankr8(void);

/* The dispatch loop. Only CLI can reach it, by restarting rather than
   returning; here it just returns, which is close enough to compare against. */
void execloop(void);
__asm__(".text\n.globl execloop\nexecloop: ret\n");

/* Both sides call this one - the oracle directly, the port through c_COp58. */
void switchtovirq(u4* const pedx, u1** const pesi)
{
    *pedx ^= 0x20u;
    *pesi += 3;
}
u1* snesmmap[1024];
u1* snesmap2[1024];

/*
 * The core reaches memory with cx = address, al = data. cpu/memory.asm's
 * `memcop` thunk turns that into a call into C through MemSeam*, so the port
 * calls the C half directly while the assembly still goes via the thunk. Both
 * are pointed at the same flat 64K here: the memory system is out of scope,
 * only the address arithmetic that picks a location is being compared.
 *
 * The perturbations of MemSeamB and MemSeamC are deliberate and deliberately
 * injective. They have to change something, or a port that forgot to carry ebx
 * or ecx back out of the seam would agree by doing nothing; but an overwrite
 * would hide the bank the addressing mode just computed, which is exactly what
 * happened first time round - six bank-arithmetic mutants survived. Toggling a
 * bit keeps the computed value recoverable and still fails a port that drops
 * the register. Only the low byte of ebx is in play, because the table index is
 * taken from it and this test's tables are 1024 entries.
 */
u4 MemSeamA, MemSeamB, MemSeamC, MemSeamD;

/* Small, because it is saved and restored around both runs - without that a
   port that never writes memory would agree with the assembly by inheriting
   what the assembly just wrote. It is the per-iteration cost of that copy that
   sets how long a run takes, and there are several hundred opcodes now.
   Aliasing is harmless: both sides alias identically. */
#define RAMSZ 256u
u1 fakeram[RAMSZ];

/* The stand-in RAM is small, so an address is folded down to RAMSZ bytes - but
   the value is then mixed with the address's high byte, so two addresses that
   alias still read differently. Without that, anything that only gets the top
   half of an address wrong is invisible (PEI's `xor ah,ah` was). Read and write
   use the same mix, so a value written at an address reads back unchanged. */
static u1 rd(u2 const a) { return (u1)(fakeram[a % RAMSZ] ^ (u1)(a >> 8)); }
static void wr(u2 const a, u1 const v)
{
    fakeram[a % RAMSZ] = (u1)(v ^ (u1)(a >> 8));
}

void c_membank0r8(void)
{
    u2 const a = (u2)MemSeamC;
    MemSeamA = (MemSeamA & 0xFFFFFF00u) | rd(a);
    MemSeamB ^= 0x80u;
    MemSeamC ^= 0x00010000u;
}

void c_membank0r16(void)
{
    u2 const a = (u2)MemSeamC;
    MemSeamA = (MemSeamA & 0xFFFF0000u) | rd(a) | (u4)rd((u2)(a + 1u)) << 8;
    MemSeamB ^= 0x80u;
    MemSeamC ^= 0x00010000u;
}

void c_membank0w16(void)
{
    u2 const a = (u2)MemSeamC;
    wr(a, (u1)MemSeamA);
    wr((u2)(a + 1u), (u1)(MemSeamA >> 8));
    MemSeamB ^= 0x40u;
    MemSeamC ^= 0x00020000u;
}

void c_membank0w8(void)
{
    u2 const a = (u2)MemSeamC;
    wr(a, (u1)MemSeamA);
    MemSeamB ^= 0x40u;
    MemSeamC ^= 0x00020000u;
}

/*
 * The addressing modes reach memory through per-bank tables of routines with
 * the same register ABI, plus the two direct-page pointers. All of them are
 * pointed at the stand-ins above, so which bank an address lands in does not
 * matter here - what is being compared is the address arithmetic that picks it.
 */
eop* memtabler8[1024];
eop* memtabler16[1024];
eop* memtablew8[1024];
eop* memtablew16[1024];
eop* DPageR8;
eop* DPageR16;
eop* DPageW8;
eop* DPageW16;

extern void membank0r8(void), membank0r16(void);
extern void membank0w8(void), membank0w16(void);

/* The assembly side's entry points: the memcop spill, by hand. */
__asm__(".text\n"
        ".globl membank0r8\n"
        ".globl membank0r16\n"
        ".globl membank0w8\n"
        ".globl membank0w16\n"
        ".macro memcop fn\n"
        "    movl %ebx, MemSeamB\n"
        "    movl %ecx, MemSeamC\n"
        "    movl %eax, MemSeamA\n"
        "    movl %edx, MemSeamD\n"
        "    call \\fn\n"
        "    movl MemSeamB, %ebx\n"
        "    movl MemSeamC, %ecx\n"
        "    movl MemSeamA, %eax\n"
        "    movl MemSeamD, %edx\n"
        "    ret\n"
        ".endm\n"
        "membank0r8:  memcop c_membank0r8\n"
        "membank0r16: memcop c_membank0r16\n"
        "membank0w8:  memcop c_membank0w8\n"
        "membank0w16: memcop c_membank0w16\n"
        ".globl regaccessbankr8\n"
        "regaccessbankr8: memcop c_membank0r8\n");

/* By -I, not "../cpu/...": a mutation sweep compiles this from a scratch
   tree whose test/ is a symlink, and there `../cpu` traverses back out to
   the real header, so every mutant would survive. */
/* The SA-1 test swaps in its own instantiation here. */
#ifndef OPS_IMPL
#define OPS_IMPL "ops65816.h"
#endif
#include OPS_IMPL

/* Set for BRK and COP, the only opcodes that write work RAM directly. */
static int dt_wram;
static char const* dt_only;

/* pushad block, shared by both sides; op_fn is called with it loaded. */
u4 R[8];
void (*op_fn)(void);

/* Load the register file, call the assembly, read the file back. ebp is the
   SPC program counter and live across an opcode, so it has to be set too -
   hence the manual push/pop rather than an ebp clobber gcc would reject. */
static void run_asm(void)
{
    __asm__ __volatile__("pushl %%ebp\n\t"
                         "movl R+0, %%edi\n\t"
                         "movl R+4, %%esi\n\t"
                         "movl R+8, %%ebp\n\t"
                         "movl R+16, %%ebx\n\t"
                         "movl R+20, %%edx\n\t"
                         "movl R+24, %%ecx\n\t"
                         "movl R+28, %%eax\n\t"
                         "call *op_fn\n\t"
                         "movl %%edi, R+0\n\t"
                         "movl %%esi, R+4\n\t"
                         "movl %%ebp, R+8\n\t"
                         "movl %%ebx, R+16\n\t"
                         "movl %%edx, R+20\n\t"
                         "movl %%ecx, R+24\n\t"
                         "movl %%eax, R+28\n\t"
                         "popl %%ebp\n\t"
        :
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory",
        "cc");
}

/*
 * name, and whether the opcode indexes tablead with ebx. Those two get a
 * ten-bit ebx so the index stays inside the table above; the rest take a fully
 * random one. Either way the upper bits are non-zero, so a port that drops them
 * instead of preserving them shows up.
 */
#define OPS(X)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
    X(COp80, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COp18, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COp38, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COpB8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COpD8, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COpF8, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COp78, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COpEA, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COpDB, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COp42, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    X(COpCAx8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COpCAx16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    X(COpE8x8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COpE8x16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    X(COp88x8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COp88x16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    X(COpC8x8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COpC8x16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    X(COpAAx8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COpAAx16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    X(COpA8x8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COpA8x16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    X(COpBAx8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COpBAx16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    X(COp8Am8, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    X(COp8Am16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
        X(COp98m8, 0) X(COp98m16, 0) X(COp9Bx8, 0) X(COp9Bx16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
            X(COpBBx8, 0) X(COpBBx16, 0) X(COp1B, 0) X(COp7B, 0) X(COp3B, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     \
                X(COp9A, 0) X(COpEB, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
                    X(COp90, 0) X(COpB0, 0) X(COpF0, 0) X(COpD0, 0) X(COp30, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                        X(COp10, 0) X(COp50, 0) X(COp70, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
                            X(COp1Am8, 0) X(COp1Am16, 0) X(COp3Am8, 0) X(COp3Am16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
                                X(COp5B, 0) X(COpC2, 1) X(COpE2, 1) X(COpFB, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                    X(COp48m8, 0) X(COp48m16, 0) X(COp8B, 0) X(COp0B, 0) X(COp4B, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
                                        X(COpDAx8, 0) X(COpDAx16, 0) X(COp5Ax8, 0) X(COp5Ax16, 0) X(COp08, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
                                            X(COp68m8, 0) X(COp68m16, 0) X(COpAB, 0) X(COpFAx8, 0) X(COpFAx16, 0)                                                                                                                                                                                                                                                                                                                                                                                                                                                \
                                                X(COp7Ax8, 0) X(COp7Ax16, 0) X(COp2B, 0) X(COp28, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
                                                    X(COpF4, 0) X(COpD4, 0) X(COp62, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
                                                        X(COpA9m8, 0) X(COpA9m16, 0) X(COpADm8, 1) X(COpADm16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                \
                                                            X(COpBDm8, 1) X(COpBDm16, 1) X(COpB9m8, 1) X(COpB9m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                            \
                                                                X(COpAFm8, 1) X(COpAFm16, 1) X(COpBFm8, 1) X(COpBFm16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                    X(COpA5m8, 1) X(COpA5m16, 1) X(COpB5m8, 1) X(COpB5m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                    \
                                                                        X(COpA3m8, 1) X(COpA3m16, 1) X(COpB2m8, 1) X(COpB2m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                \
                                                                            X(COpB1m8, 1) X(COpB1m16, 1) X(COpA1m8, 1) X(COpA1m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                            \
                                                                                X(COpB3m8, 1) X(COpB3m16, 1) X(COpA7m8, 1) X(COpA7m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                    X(COpB7m8, 1) X(COpB7m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
                                                                                        X(COp21m8, 1) X(COp21m16, 1) X(COp23m8, 1) X(COp23m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                                \
                                                                                            X(COp25m8, 1) X(COp25m16, 1) X(COp27m8, 1) X(COp27m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                X(COp29m8, 1) X(COp29m16, 1) X(COp2Dm8, 1) X(COp2Dm16, 1)                                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                    X(COp2Fm8, 1) X(COp2Fm16, 1) X(COp31m8, 1) X(COp31m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                        X(COp32m8, 1) X(COp32m16, 1) X(COp33m8, 1) X(COp33m16, 1)                                                                                                                                                                                                                                                                                                                                                                                                \
                                                                                                            X(COp35m8, 1) X(COp35m16, 1) X(COp37m8, 1) X(COp37m16, 1)                                                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                X(COp39m8, 1) X(COp39m16, 1) X(COp3Dm8, 1) X(COp3Dm16, 1)                                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                    X(COp3Fm8, 1) X(COp3Fm16, 1) X(COp24m8, 1) X(COp24m16, 1)                                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                        X(COp2Cm8, 1) X(COp2Cm16, 1) X(COp34m8, 1) X(COp34m16, 1)                                                                                                                                                                                                                                                                                                                                                                                \
                                                                                                                            X(COp3Cm8, 1) X(COp3Cm16, 1) X(COpC1m8, 1) X(COpC1m16, 1)                                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                X(COpC3m8, 1) X(COpC3m16, 1) X(COpC5m8, 1) X(COpC5m16, 1)                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                    X(COpC7m8, 1) X(COpC7m16, 1) X(COpC9m8, 1) X(COpC9m16, 1)                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                        X(COpCDm8, 1) X(COpCDm16, 1) X(COpCFm8, 1) X(COpCFm16, 1)                                                                                                                                                                                                                                                                                                                                                                \
                                                                                                                                            X(COpD1m8, 1) X(COpD1m16, 1) X(COpD2m8, 1) X(COpD2m16, 1)                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                X(COpD3m8, 1) X(COpD3m16, 1) X(COpD5m8, 1) X(COpD5m16, 1)                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                    X(COpD7m8, 1) X(COpD7m16, 1) X(COpD9m8, 1) X(COpD9m16, 1)                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                                        X(COpDDm8, 1) X(COpDDm16, 1) X(COpDFm8, 1) X(COpDFm16, 1)                                                                                                                                                                                                                                                                                                                                                \
                                                                                                                                                            X(COpE0x8, 1) X(COpE0x16, 1) X(COpE4x8, 1) X(COpE4x16, 1)                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                                X(COpECx8, 1) X(COpECx16, 1) X(COpC0x8, 1) X(COpC0x16, 1)                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                    X(COpC4x8, 1) X(COpC4x16, 1) X(COpCCx8, 1) X(COpCCx16, 1)                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                        X(COp41m8, 1) X(COp41m16, 1) X(COp43m8, 1) X(COp43m16, 1)                                                                                                                                                                                                                                                                                                                                \
                                                                                                                                                                            X(COp45m8, 1) X(COp45m16, 1) X(COp47m8, 1) X(COp47m16, 1)                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                                                X(COp49m8, 1) X(COp49m16, 1) X(COp4Dm8, 1) X(COp4Dm16, 1)                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                    X(COp4Fm8, 1) X(COp4Fm16, 1) X(COp51m8, 1) X(COp51m16, 1)                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                        X(COp52m8, 1) X(COp52m16, 1) X(COp53m8, 1) X(COp53m16, 1)                                                                                                                                                                                                                                                                                                                \
                                                                                                                                                                                            X(COp55m8, 1) X(COp55m16, 1) X(COp57m8, 1) X(COp57m16, 1)                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                                                                X(COp59m8, 1) X(COp59m16, 1) X(COp5Dm8, 1) X(COp5Dm16, 1)                                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                    X(COp5Fm8, 1) X(COp5Fm16, 1) X(COpA2x8, 1) X(COpA2x16, 1)                                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                        X(COpA6x8, 1) X(COpA6x16, 1) X(COpAEx8, 1) X(COpAEx16, 1)                                                                                                                                                                                                                                                                                                \
                                                                                                                                                                                                            X(COpB6x8, 1) X(COpB6x16, 1) X(COpBEx8, 1) X(COpBEx16, 1)                                                                                                                                                                                                                                                                                            \
                                                                                                                                                                                                                X(COpA0x8, 1) X(COpA0x16, 1) X(COpA4x8, 1) X(COpA4x16, 1)                                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                    X(COpACx8, 1) X(COpACx16, 1) X(COpB4x8, 1) X(COpB4x16, 1)                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                        X(COpBCx8, 1) X(COpBCx16, 1) X(COp01m8, 1) X(COp01m16, 1)                                                                                                                                                                                                                                                                                \
                                                                                                                                                                                                                            X(COp03m8, 1) X(COp03m16, 1) X(COp05m8, 1) X(COp05m16, 1)                                                                                                                                                                                                                                                                            \
                                                                                                                                                                                                                                X(COp07m8, 1) X(COp07m16, 1) X(COp09m8, 1) X(COp09m16, 1)                                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                    X(COp0Dm8, 1) X(COp0Dm16, 1) X(COp0Fm8, 1) X(COp0Fm16, 1)                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                        X(COp11m8, 1) X(COp11m16, 1) X(COp12m8, 1) X(COp12m16, 1)                                                                                                                                                                                                                                                                \
                                                                                                                                                                                                                                            X(COp13m8, 1) X(COp13m16, 1) X(COp15m8, 1) X(COp15m16, 1)                                                                                                                                                                                                                                                            \
                                                                                                                                                                                                                                                X(COp17m8, 1) X(COp17m16, 1) X(COp19m8, 1) X(COp19m16, 1)                                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                    X(COp1Dm8, 1) X(COp1Dm16, 1) X(COp1Fm8, 1) X(COp1Fm16, 1)                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                        X(COp81m8, 1) X(COp81m16, 1) X(COp83m8, 1) X(COp83m16, 1)                                                                                                                                                                                                                                                \
                                                                                                                                                                                                                                                            X(COp85m8, 1) X(COp85m16, 1) X(COp87m8, 1) X(COp87m16, 1)                                                                                                                                                                                                                                            \
                                                                                                                                                                                                                                                                X(COp8Dm8, 1) X(COp8Dm16, 1) X(COp8Fm8, 1) X(COp8Fm16, 1)                                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                    X(COp91m8, 1) X(COp91m16, 1) X(COp92m8, 1) X(COp92m16, 1)                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                        X(COp93m8, 1) X(COp93m16, 1) X(COp95m8, 1) X(COp95m16, 1)                                                                                                                                                                                                                                \
                                                                                                                                                                                                                                                                            X(COp97m8, 1) X(COp97m16, 1) X(COp99m8, 1) X(COp99m16, 1)                                                                                                                                                                                                                            \
                                                                                                                                                                                                                                                                                X(COp9Dm8, 1) X(COp9Dm16, 1) X(COp9Fm8, 1) X(COp9Fm16, 1)                                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                    X(COp86x8, 1) X(COp86x16, 1) X(COp8Ex8, 1) X(COp8Ex16, 1)                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                                        X(COp96x8, 1) X(COp96x16, 1) X(COp84x8, 1) X(COp84x16, 1)                                                                                                                                                                                                                \
                                                                                                                                                                                                                                                                                            X(COp8Cx8, 1) X(COp8Cx16, 1) X(COp94x8, 1) X(COp94x16, 1)                                                                                                                                                                                                            \
                                                                                                                                                                                                                                                                                                X(COp64m8, 1) X(COp64m16, 1) X(COp74m8, 1) X(COp74m16, 1)                                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                    X(COp9Cm8, 1) X(COp9Cm16, 1) X(COp9Em8, 1) X(COp9Em16, 1)                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                                                        X(COp06m8, 1) X(COp06m16, 1) X(COp0Am8, 1) X(COp0Am16, 1)                                                                                                                                                                                                \
                                                                                                                                                                                                                                                                                                            X(COp0Em8, 1) X(COp0Em16, 1) X(COp16m8, 1) X(COp16m16, 1)                                                                                                                                                                                            \
                                                                                                                                                                                                                                                                                                                X(COp1Em8, 1) X(COp1Em16, 1) X(COpCEm8, 1) X(COpCEm16, 1)                                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                    X(COpC6m8, 1) X(COpC6m16, 1) X(COpD6m8, 1) X(COpD6m16, 1)                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                                                                        X(COpDEm8, 1) X(COpDEm16, 1) X(COpEEm8, 1) X(COpEEm16, 1)                                                                                                                                                                                \
                                                                                                                                                                                                                                                                                                                            X(COpE6m8, 1) X(COpE6m16, 1) X(COpF6m8, 1) X(COpF6m16, 1)                                                                                                                                                                            \
                                                                                                                                                                                                                                                                                                                                X(COpFEm8, 1) X(COpFEm16, 1) X(COp46m8, 1) X(COp46m16, 1)                                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                                    X(COp4Am8, 1) X(COp4Am16, 1) X(COp4Em8, 1) X(COp4Em16, 1)                                                                                                                                                                    \
                                                                                                                                                                                                                                                                                                                                        X(COp56m8, 1) X(COp56m16, 1) X(COp5Em8, 1) X(COp5Em16, 1)                                                                                                                                                                \
                                                                                                                                                                                                                                                                                                                                            X(COp26m8, 1) X(COp26m16, 1) X(COp2Am8, 1) X(COp2Am16, 1)                                                                                                                                                            \
                                                                                                                                                                                                                                                                                                                                                X(COp2Em8, 1) X(COp2Em16, 1) X(COp36m8, 1) X(COp36m16, 1)                                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                                                    X(COp3Em8, 1) X(COp3Em16, 1) X(COp66m8, 1) X(COp66m16, 1)                                                                                                                                                    \
                                                                                                                                                                                                                                                                                                                                                        X(COp6Am8, 1) X(COp6Am16, 1) X(COp6Em8, 1) X(COp6Em16, 1)                                                                                                                                                \
                                                                                                                                                                                                                                                                                                                                                            X(COp76m8, 1) X(COp76m16, 1) X(COp7Em8, 1) X(COp7Em16, 1)                                                                                                                                            \
                                                                                                                                                                                                                                                                                                                                                                X(COp14m8, 1) X(COp14m16, 1) X(COp1Cm8, 1) X(COp1Cm16, 1)                                                                                                                                        \
                                                                                                                                                                                                                                                                                                                                                                    X(COp04m8, 1) X(COp04m16, 1) X(COp0Cm8, 1) X(COp0Cm16, 1)                                                                                                                                    \
                                                                                                                                                                                                                                                                                                                                                                        X(COp61m8nd, 1) X(COp61m16nd, 1) X(COp61m8d, 1) X(COp61m16d, 1)                                                                                                                          \
                                                                                                                                                                                                                                                                                                                                                                            X(COp63m8nd, 1) X(COp63m16nd, 1) X(COp63m8d, 1) X(COp63m16d, 1)                                                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                X(COp65m8nd, 1) X(COp65m16nd, 1) X(COp65m8d, 1) X(COp65m16d, 1)                                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                    X(COp67m8nd, 1) X(COp67m16nd, 1) X(COp67m8d, 1) X(COp67m16d, 1)                                                                                                              \
                                                                                                                                                                                                                                                                                                                                                                                        X(COp69m8nd, 1) X(COp69m16nd, 1) X(COp69m8d, 1) X(COp69m16d, 1)                                                                                                          \
                                                                                                                                                                                                                                                                                                                                                                                            X(COp6Dm8nd, 1) X(COp6Dm16nd, 1) X(COp6Dm8d, 1) X(COp6Dm16d, 1)                                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                X(COp6Fm8nd, 1) X(COp6Fm16nd, 1) X(COp6Fm8d, 1) X(COp6Fm16d, 1)                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                    X(COp71m8nd, 1) X(COp71m16nd, 1) X(COp71m8d, 1) X(COp71m16d, 1)                                                                                              \
                                                                                                                                                                                                                                                                                                                                                                                                        X(COp72m8nd, 1) X(COp72m16nd, 1) X(COp72m8d, 1) X(COp72m16d, 1)                                                                                          \
                                                                                                                                                                                                                                                                                                                                                                                                            X(COp73m8nd, 1) X(COp73m16nd, 1) X(COp73m8d, 1) X(COp73m16d, 1)                                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                X(COp75m8nd, 1) X(COp75m16nd, 1) X(COp75m8d, 1) X(COp75m16d, 1)                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                    X(COp77m8nd, 1) X(COp77m16nd, 1) X(COp77m8d, 1) X(COp77m16d, 1)                                                                              \
                                                                                                                                                                                                                                                                                                                                                                                                                        X(COp79m8nd, 1) X(COp79m16nd, 1) X(COp79m8d, 1) X(COp79m16d, 1)                                                                          \
                                                                                                                                                                                                                                                                                                                                                                                                                            X(COp7Dm8nd, 1) X(COp7Dm16nd, 1) X(COp7Dm8d, 1) X(COp7Dm16d, 1)                                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                X(COp7Fm8nd, 1) X(COp7Fm16nd, 1) X(COp7Fm8d, 1) X(COp7Fm16d, 1)                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                    X(COpE1m8nd, 1) X(COpE1m16nd, 1) X(COpE1m8d, 1) X(COpE1m16d, 1)                                                              \
                                                                                                                                                                                                                                                                                                                                                                                                                                        X(COpE3m8nd, 1) X(COpE3m16nd, 1) X(COpE3m8d, 1) X(COpE3m16d, 1)                                                          \
                                                                                                                                                                                                                                                                                                                                                                                                                                            X(COpE5m8nd, 1) X(COpE5m16nd, 1) X(COpE5m8d, 1) X(COpE5m16d, 1)                                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                X(COpE7m8nd, 1) X(COpE7m16nd, 1) X(COpE7m8d, 1) X(COpE7m16d, 1)                                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                    X(COpE9m8nd, 1) X(COpE9m16nd, 1) X(COpE9m8d, 1) X(COpE9m16d, 1)                                              \
                                                                                                                                                                                                                                                                                                                                                                                                                                                        X(COpEDm8nd, 1) X(COpEDm16nd, 1) X(COpEDm8d, 1) X(COpEDm16d, 1)                                          \
                                                                                                                                                                                                                                                                                                                                                                                                                                                            X(COpEFm8nd, 1) X(COpEFm16nd, 1) X(COpEFm8d, 1) X(COpEFm16d, 1)                                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                X(COpF1m8nd, 1) X(COpF1m16nd, 1) X(COpF1m8d, 1) X(COpF1m16d, 1)                                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                    X(COpF2m8nd, 1) X(COpF2m16nd, 1) X(COpF2m8d, 1) X(COpF2m16d, 1)                              \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                        X(COpF3m8nd, 1) X(COpF3m16nd, 1) X(COpF3m8d, 1) X(COpF3m16d, 1)                          \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                            X(COpF5m8nd, 1) X(COpF5m16nd, 1) X(COpF5m8d, 1) X(COpF5m16d, 1)                      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                X(COpF7m8nd, 1) X(COpF7m16nd, 1) X(COpF7m8d, 1) X(COpF7m16d, 1)                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    X(COpF9m8nd, 1) X(COpF9m16nd, 1) X(COpF9m8d, 1) X(COpF9m16d, 1)              \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        X(COpFDm8nd, 1) X(COpFDm16nd, 1) X(COpFDm8d, 1) X(COpFDm16d, 1)          \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            X(COpFFm8nd, 1) X(COpFFm16nd, 1) X(COpFFm8d, 1) X(COpFFm16d, 1)      \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                X(COp4C, 1) X(COp6C, 1) X(COp7C, 1) X(COp5C, 1)                  \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    X(COpDC, 1) X(COp82, 1) X(COp60, 1) X(COp6B, 1)              \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        X(COp20, 1) X(COpFC, 1) X(COp22, 1) X(COp54, 1)          \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            X(COp44, 1) X(COpCB, 1) X(COp89m8, 1) X(COp89m16, 1) \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                X(COp00, 1) X(COp02, 1) X(COp40, 1)

#define DECL(n, b) extern void ASMOP(n)(void);
OPS(DECL)
#undef DECL

/* CLI's C half returns "restart the dispatch loop", which its thunk in
   cpu/e65816.inc turns into `xor ebx,ebx; jmp execloop`. The oracle's opcode
   contains that branch, so the test has to model the thunk too. */
extern void ASMOP(COp58)(void);
static void cli_c(u4* const r)
{
#ifdef CLI_RETURNS_VOID
    OP(COp58)(r); /* the SA-1 has no IRQ to switch to, so no restart path */
#else
    if (OP(COp58)(r))
        r[R_EBX] = 0;
#endif
}

static const struct {
    char const* name;
    void (*a)(void);
    void (*c)(u4*);
    int bytebx;
} ops[] = {
#define ENT(n, b) { #n, ASMOP(n), OP(n), b },
    OPS(ENT)
#undef ENT
        { "COp58", ASMOP(COp58), cli_c, 1 },
};

/* Everything both sides may read or write. */
struct state {
    u4 r[8];
    u2 brkv_, brkv8_, copv_, copv8_;
    u1 xirqb_, intrset_, doirqnext_, curnmi_;
    u4 nmistatus_, curexecstate_;
    u4 initaddrl_;
    u4 xa, xx, xy, xs, xd, xdb, xpb, flagnz, flago, flagc;
    u2 xpc;
    u2 stackand, stackor;
    u1 ram[RAMSZ];
    u1 wram[0x10000];
    u1 xe;
};

static void load(struct state const* const s)
{
    memcpy(R, s->r, sizeof R);
    xa = s->xa;
    xx = s->xx;
    xy = s->xy;
    xs = s->xs;
    xd = s->xd;
    xdb = s->xdb;
    xpb = s->xpb;
    xpc = s->xpc;
    brkv = s->brkv_;
    brkv8 = s->brkv8_;
    copv = s->copv_;
    copv8 = s->copv8_;
    xirqb = s->xirqb_;
    intrset = s->intrset_;
    doirqnext = s->doirqnext_;
    curnmi = s->curnmi_;
    nmistatus = s->nmistatus_;
    curexecstate = s->curexecstate_;
    initaddrl = (u1*)(uintptr_t)s->initaddrl_;
    xe = s->xe;
    flagnz = s->flagnz;
    flago = s->flago;
    flagc = s->flagc;
    stackand = s->stackand;
    stackor = s->stackor;
    memcpy(fakeram, s->ram, RAMSZ);
    if (dt_wram)
        memcpy(wram, s->wram, sizeof wram);
}

static void save(struct state* const s)
{
    memcpy(s->r, R, sizeof R);
    s->xa = xa;
    s->xx = xx;
    s->xy = xy;
    s->xs = xs;
    s->xd = xd;
    s->xdb = xdb;
    s->xpb = xpb;
    s->xpc = xpc;
    s->brkv_ = brkv;
    s->brkv8_ = brkv8;
    s->copv_ = copv;
    s->copv8_ = copv8;
    s->xirqb_ = xirqb;
    s->intrset_ = intrset;
    s->doirqnext_ = doirqnext;
    s->curnmi_ = curnmi;
    s->nmistatus_ = nmistatus;
    s->curexecstate_ = curexecstate;
    s->initaddrl_ = (u4)(uintptr_t)initaddrl;
    s->xe = xe;
    s->flagnz = flagnz;
    s->flago = flago;
    s->flagc = flagc;
    s->stackand = stackand;
    s->stackor = stackor;
    memcpy(s->ram, fakeram, RAMSZ);
    if (dt_wram)
        memcpy(s->wram, wram, sizeof wram);
}

/*
 * Flags need shaping, not just randomising. A uniform flagnz has its low 16
 * bits clear once in 65536, so BEQ would take its branch a handful of times in
 * a whole run and a wrong Z mask would sail through; likewise flagc and flago
 * are only ever 0 or 0xFF in practice, and E is 0 or 1.
 */
static u4 nzval(void)
{
    switch (dt_mod(4)) {
    case 0:
        return 0; /* Z set, N clear */
    case 1:
        return dt_u32() & 0xFFFF0000u; /* Z set, N from bit 16 */
    case 2:
        return dt_u32() | 0x00008000u; /* N set */
    default:
        return dt_u32();
    }
}

/* A register's 16-bit half, biased towards the values that make loop and
   wrap conditions fire: block moves end when A underflows past zero, and a
   uniform 32-bit value hits that about once in 65536 tries. */
static u4 regval(void)
{
    switch (dt_mod(8)) {
    case 0:
        return (dt_u32() & 0xFFFF0000u) | 0x0000u;
    case 1:
        return (dt_u32() & 0xFFFF0000u) | 0x0001u;
    case 2:
        return (dt_u32() & 0xFFFF0000u) | 0xFFFFu;
    case 3:
        return (dt_u32() & 0xFFFF0000u) | (dt_u32() & 0xFFu);
    default:
        return dt_u32();
    }
}

static u4 cvval(void)
{
    switch (dt_mod(4)) {
    case 0:
        return 0;
    case 1:
        return 0xFF;
    case 2:
        return dt_u32() & 0xFFFFFF00u; /* set, but not in the low byte */
    default:
        return dt_u32();
    }
}

static char const* const rname[8]
    = { "edi", "esi", "ebp", "esp", "ebx", "edx", "ecx", "eax" };

int main(void)
{
    static u4 tab[1024];
    int fails = 0;

    /* Set up once, not per iteration: every entry is the same stand-in, and
       filling 1024 of them inside the loop dominated the whole run. */
    for (size_t i = 0; i < 1024; i++) {
        memtabler8[i] = membank0r8;
        memtabler16[i] = membank0r16;
        memtablew8[i] = membank0w8;
        memtablew16[i] = membank0w16;
    }
    DPageR8 = membank0r8;
    DPageR16 = membank0r16;
    DPageW8 = membank0w8;
    DPageW16 = membank0w16;

    /* DT_ONLY=COp40 runs one opcode, which is how you find the one that hangs
       or crashes without waiting for the other five hundred. */
    {
        char const* only = getenv("DT_ONLY");
        dt_only = only && *only ? only : 0;
    }
    for (size_t o = 0; o < sizeof ops / sizeof ops[0]; o++) {
        if (dt_only && strcmp(dt_only, ops[o].name) != 0)
            continue;
        if (dt_only)
            printf("running %s\n", ops[o].name), fflush(stdout);
        dt_wram = strcmp(ops[o].name, "COp00") == 0
            || strcmp(ops[o].name, "COp02") == 0;
        dt_fill(code, CODESZ);
        dt_fill(tab, sizeof tab);
        memcpy(tablead, tab, sizeof tablead);
        if (dt_wram)
            dt_fill(wram_seed, sizeof wram_seed);
        /* Bases near the bottom of the buffer, so base + a 16-bit address
           stays inside it. Some banks read as register banks so the $4300
           routing in JMP/JSR/RTI is exercised. */
        for (size_t i = 0; i < 1024; i++) {
            snesmmap[i] = code + (dt_u32() & 7u);
            snesmap2[i] = code + (dt_u32() & 7u);
            memtabler8[i] = (i % 3u == 0) ? regaccessbankr8 : membank0r8;
        }
        wramdata = wram;
        DT_MAIN(12345 + (int)o, 20000)
        {
            struct state in, a, c;

            /* Only the operand bytes need to change per iteration. The rest
               of the buffer is randomised once per opcode: it is 128K, and
               refilling it every time dominated the whole run. */
            dt_fill(code + 0x100u, 16);

            for (int i = 0; i < 8; i++)
                in.r[i] = dt_u32();
            in.r[R_ESI] = (u4)(uintptr_t)(code + 0x100u);
            if (ops[o].bytebx)
                in.r[R_EBX] = dt_u32() & 0x3FFu;
            in.xa = regval();
            in.xx = regval();
            in.xy = regval();
            in.xs = dt_u32();
            in.xd = dt_u32();
            in.xdb = dt_u32();
            in.xpb = dt_u32() & 0x3FFu;
            in.xpc = (u2)dt_u32();
            in.brkv_ = (u2)dt_u32();
            in.brkv8_ = (u2)dt_u32();
            in.copv_ = (u2)dt_u32();
            in.copv8_ = (u2)dt_u32();
            in.xirqb_ = (u1)(dt_u32() & 0x3Fu);
            in.intrset_ = (u1)dt_mod(4);
            in.doirqnext_ = (u1)dt_mod(2);
            in.curnmi_ = (u1)dt_mod(2);
            in.nmistatus_ = dt_mod(5);
            in.curexecstate_ = dt_mod(4);
            in.initaddrl_ = (u4)(uintptr_t)(code + (dt_u32() & 7u));
            in.xe = dt_mod(4) == 0 ? (u1)dt_u32() : (u1)dt_mod(2);
            in.flagnz = nzval();
            in.flago = cvval();
            in.flagc = cvval();
            in.stackand = (u2)dt_u32();
            in.stackor = (u2)dt_u32();
            dt_fill(in.ram, RAMSZ);
            if (dt_wram)
                memcpy(in.wram, wram_seed, sizeof in.wram);

            load(&in);
            op_fn = ops[o].a;
            run_asm();
            save(&a);

            load(&in);
            ops[o].c(R);
            save(&c);

            for (int i = 0; i < 8; i++)
                DT_EQ(rname[i], a.r[i], c.r[i]);
            DT_EQ("xa", a.xa, c.xa);
            DT_EQ("xx", a.xx, c.xx);
            DT_EQ("xy", a.xy, c.xy);
            DT_EQ("xs", a.xs, c.xs);
            DT_EQ("xd", a.xd, c.xd);
            DT_EQ("xdb", a.xdb, c.xdb);
            DT_EQ("xpb", a.xpb, c.xpb);
            DT_EQ("xpc", a.xpc, c.xpc);
            DT_EQ("intrset", a.intrset_, c.intrset_);
            DT_EQ("doirqnext", a.doirqnext_, c.doirqnext_);
            DT_EQ("curnmi", a.curnmi_, c.curnmi_);
            DT_EQ("curexecstate", a.curexecstate_, c.curexecstate_);
            DT_EQ("initaddrl", a.initaddrl_, c.initaddrl_);
            DT_EQ("xe", a.xe, c.xe);
            DT_EQ("flagnz", a.flagnz, c.flagnz);
            DT_EQ("flago", a.flago, c.flago);
            DT_EQ("flagc", a.flagc, c.flagc);
            DT_EQ("stackand", a.stackand, c.stackand);
            DT_EQ("stackor", a.stackor, c.stackor);
            DT_MEM("ram", a.ram, c.ram, RAMSZ);
            if (dt_wram)
                DT_MEM("wram", a.wram, c.wram, sizeof a.wram);
        }
        if (dt_fails) {
            int const known = KNOWN_DIVERGENCE(ops[o].name);
            printf("  %s %s: %d/%ld mismatched\n", known ? "-- known:" : "  ^^",
                ops[o].name, dt_fails, dt_iters);
            if (!known)
                fails++;
        }
    }

    if (fails) {
        printf(CORENAME " opcodes: FAIL (%d of %zu opcodes mismatched)\n", fails,
            sizeof ops / sizeof ops[0]);
        return 1;
    }
    printf(CORENAME " opcodes: PASS (%zu opcodes x %ld iterations bit-identical "
                    "to asm)\n",
        sizeof ops / sizeof ops[0], dt_iters);
    return 0;
}
