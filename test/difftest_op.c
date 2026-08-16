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
   what the assembly just wrote. Aliasing is harmless: both sides alias. */
#define RAMSZ 1024u
u1 fakeram[RAMSZ];

void c_membank0r8(void)
{
    u2 const a = (u2)MemSeamC;
    MemSeamA = (MemSeamA & 0xFFFFFF00u) | fakeram[a % RAMSZ];
    MemSeamB ^= 0x80u;
    MemSeamC ^= 0x00010000u;
}

void c_membank0r16(void)
{
    u2 const a = (u2)MemSeamC;
    MemSeamA = (MemSeamA & 0xFFFF0000u) | fakeram[a % RAMSZ]
        | (u4)fakeram[(u2)(a + 1u) % RAMSZ] << 8;
    MemSeamB ^= 0x80u;
    MemSeamC ^= 0x00010000u;
}

void c_membank0w8(void)
{
    u2 const a = (u2)MemSeamC;
    fakeram[a % RAMSZ] = (u1)MemSeamA;
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
eop* DPageR8;
eop* DPageR16;

extern void membank0r8(void), membank0r16(void), membank0w8(void);

/* The assembly side's entry points: the memcop spill, by hand. */
__asm__(".text\n"
        ".globl membank0r8\n"
        ".globl membank0r16\n"
        ".globl membank0w8\n"
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
        "membank0w8:  memcop c_membank0w8\n");

/* By -I, not "../cpu/...": a mutation sweep compiles this from a scratch
   tree whose test/ is a symlink, and there `../cpu` traverses back out to
   the real header, so every mutant would survive. */
#include "ops65816.h"

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
#define OPS(X)                                                                                                                                                                                                                                                                                                            \
    X(COp80, 0)                                                                                                                                                                                                                                                                                                           \
    X(COp18, 0)                                                                                                                                                                                                                                                                                                           \
    X(COp38, 0)                                                                                                                                                                                                                                                                                                           \
    X(COpB8, 0)                                                                                                                                                                                                                                                                                                           \
    X(COpD8, 1)                                                                                                                                                                                                                                                                                                           \
    X(COpF8, 1)                                                                                                                                                                                                                                                                                                           \
    X(COp78, 0)                                                                                                                                                                                                                                                                                                           \
    X(COpEA, 0) X(COpDB, 0) X(COp42, 0)                                                                                                                                                                                                                                                                                   \
        X(COpCAx8, 0) X(COpCAx16, 0) X(COpE8x8, 0) X(COpE8x16, 0)                                                                                                                                                                                                                                                         \
            X(COp88x8, 0) X(COp88x16, 0) X(COpC8x8, 0) X(COpC8x16, 0)                                                                                                                                                                                                                                                     \
                X(COpAAx8, 0) X(COpAAx16, 0) X(COpA8x8, 0) X(COpA8x16, 0)                                                                                                                                                                                                                                                 \
                    X(COpBAx8, 0) X(COpBAx16, 0) X(COp8Am8, 0) X(COp8Am16, 0)                                                                                                                                                                                                                                             \
                        X(COp98m8, 0) X(COp98m16, 0) X(COp9Bx8, 0) X(COp9Bx16, 0)                                                                                                                                                                                                                                         \
                            X(COpBBx8, 0) X(COpBBx16, 0) X(COp1B, 0) X(COp7B, 0) X(COp3B, 0)                                                                                                                                                                                                                              \
                                X(COp9A, 0) X(COpEB, 0)                                                                                                                                                                                                                                                                   \
                                    X(COp90, 0) X(COpB0, 0) X(COpF0, 0) X(COpD0, 0) X(COp30, 0)                                                                                                                                                                                                                           \
                                        X(COp10, 0) X(COp50, 0) X(COp70, 0)                                                                                                                                                                                                                                               \
                                            X(COp1Am8, 0) X(COp1Am16, 0) X(COp3Am8, 0) X(COp3Am16, 0)                                                                                                                                                                                                                     \
                                                X(COp5B, 0) X(COpC2, 1) X(COpE2, 1) X(COpFB, 1)                                                                                                                                                                                                                           \
                                                    X(COp48m8, 0) X(COp48m16, 0) X(COp8B, 0) X(COp0B, 0) X(COp4B, 0)                                                                                                                                                                                                      \
                                                        X(COpDAx8, 0) X(COpDAx16, 0) X(COp5Ax8, 0) X(COp5Ax16, 0) X(COp08, 0)                                                                                                                                                                                             \
                                                            X(COp68m8, 0) X(COp68m16, 0) X(COpAB, 0) X(COpFAx8, 0) X(COpFAx16, 0)                                                                                                                                                                                         \
                                                                X(COp7Ax8, 0) X(COp7Ax16, 0) X(COp2B, 0) X(COp28, 1)                                                                                                                                                                                                      \
                                                                    X(COpF4, 0) X(COpD4, 0) X(COp62, 1)                                                                                                                                                                                                                   \
                                                                        X(COpA9m8, 0) X(COpA9m16, 0) X(COpADm8, 1) X(COpADm16, 1)                                                                                                                                                                                         \
                                                                            X(COpBDm8, 1) X(COpBDm16, 1) X(COpB9m8, 1) X(COpB9m16, 1)                                                                                                                                                                                     \
                                                                                X(COpAFm8, 1) X(COpAFm16, 1) X(COpBFm8, 1) X(COpBFm16, 1)                                                                                                                                                                                 \
                                                                                    X(COpA5m8, 1) X(COpA5m16, 1) X(COpB5m8, 1) X(COpB5m16, 1)                                                                                                                                                                             \
                                                                                        X(COpA3m8, 1) X(COpA3m16, 1) X(COpB2m8, 1) X(COpB2m16, 1)                                                                                                                                                                         \
                                                                                            X(COpB1m8, 1) X(COpB1m16, 1) X(COpA1m8, 1) X(COpA1m16, 1)                                                                                                                                                                     \
                                                                                                X(COpB3m8, 1) X(COpB3m16, 1) X(COpA7m8, 1) X(COpA7m16, 1)                                                                                                                                                                 \
                                                                                                    X(COpB7m8, 1) X(COpB7m16, 1)                                                                                                                                                                                          \
                                                                                                        X(COp21m8, 1) X(COp21m16, 1) X(COp23m8, 1) X(COp23m16, 1)                                                                                                                                                         \
                                                                                                            X(COp25m8, 1) X(COp25m16, 1) X(COp27m8, 1) X(COp27m16, 1)                                                                                                                                                     \
                                                                                                                X(COp29m8, 1) X(COp29m16, 1) X(COp2Dm8, 1) X(COp2Dm16, 1)                                                                                                                                                 \
                                                                                                                    X(COp2Fm8, 1) X(COp2Fm16, 1) X(COp31m8, 1) X(COp31m16, 1)                                                                                                                                             \
                                                                                                                        X(COp32m8, 1) X(COp32m16, 1) X(COp33m8, 1) X(COp33m16, 1)                                                                                                                                         \
                                                                                                                            X(COp35m8, 1) X(COp35m16, 1) X(COp37m8, 1) X(COp37m16, 1)                                                                                                                                     \
                                                                                                                                X(COp39m8, 1) X(COp39m16, 1) X(COp3Dm8, 1) X(COp3Dm16, 1)                                                                                                                                 \
                                                                                                                                    X(COp3Fm8, 1) X(COp3Fm16, 1) X(COp24m8, 1) X(COp24m16, 1)                                                                                                                             \
                                                                                                                                        X(COp2Cm8, 1) X(COp2Cm16, 1) X(COp34m8, 1) X(COp34m16, 1)                                                                                                                         \
                                                                                                                                            X(COp3Cm8, 1) X(COp3Cm16, 1) X(COpC1m8, 1) X(COpC1m16, 1)                                                                                                                     \
                                                                                                                                                X(COpC3m8, 1) X(COpC3m16, 1) X(COpC5m8, 1) X(COpC5m16, 1)                                                                                                                 \
                                                                                                                                                    X(COpC7m8, 1) X(COpC7m16, 1) X(COpC9m8, 1) X(COpC9m16, 1)                                                                                                             \
                                                                                                                                                        X(COpCDm8, 1) X(COpCDm16, 1) X(COpCFm8, 1) X(COpCFm16, 1)                                                                                                         \
                                                                                                                                                            X(COpD1m8, 1) X(COpD1m16, 1) X(COpD2m8, 1) X(COpD2m16, 1)                                                                                                     \
                                                                                                                                                                X(COpD3m8, 1) X(COpD3m16, 1) X(COpD5m8, 1) X(COpD5m16, 1)                                                                                                 \
                                                                                                                                                                    X(COpD7m8, 1) X(COpD7m16, 1) X(COpD9m8, 1) X(COpD9m16, 1)                                                                                             \
                                                                                                                                                                        X(COpDDm8, 1) X(COpDDm16, 1) X(COpDFm8, 1) X(COpDFm16, 1)                                                                                         \
                                                                                                                                                                            X(COpE0x8, 1) X(COpE0x16, 1) X(COpE4x8, 1) X(COpE4x16, 1)                                                                                     \
                                                                                                                                                                                X(COpECx8, 1) X(COpECx16, 1) X(COpC0x8, 1) X(COpC0x16, 1)                                                                                 \
                                                                                                                                                                                    X(COpC4x8, 1) X(COpC4x16, 1) X(COpCCx8, 1) X(COpCCx16, 1)                                                                             \
                                                                                                                                                                                        X(COp41m8, 1) X(COp41m16, 1) X(COp43m8, 1) X(COp43m16, 1)                                                                         \
                                                                                                                                                                                            X(COp45m8, 1) X(COp45m16, 1) X(COp47m8, 1) X(COp47m16, 1)                                                                     \
                                                                                                                                                                                                X(COp49m8, 1) X(COp49m16, 1) X(COp4Dm8, 1) X(COp4Dm16, 1)                                                                 \
                                                                                                                                                                                                    X(COp4Fm8, 1) X(COp4Fm16, 1) X(COp51m8, 1) X(COp51m16, 1)                                                             \
                                                                                                                                                                                                        X(COp52m8, 1) X(COp52m16, 1) X(COp53m8, 1) X(COp53m16, 1)                                                         \
                                                                                                                                                                                                            X(COp55m8, 1) X(COp55m16, 1) X(COp57m8, 1) X(COp57m16, 1)                                                     \
                                                                                                                                                                                                                X(COp59m8, 1) X(COp59m16, 1) X(COp5Dm8, 1) X(COp5Dm16, 1)                                                 \
                                                                                                                                                                                                                    X(COp5Fm8, 1) X(COp5Fm16, 1) X(COpA2x8, 1) X(COpA2x16, 1)                                             \
                                                                                                                                                                                                                        X(COpA6x8, 1) X(COpA6x16, 1) X(COpAEx8, 1) X(COpAEx16, 1)                                         \
                                                                                                                                                                                                                            X(COpB6x8, 1) X(COpB6x16, 1) X(COpBEx8, 1) X(COpBEx16, 1)                                     \
                                                                                                                                                                                                                                X(COpA0x8, 1) X(COpA0x16, 1) X(COpA4x8, 1) X(COpA4x16, 1)                                 \
                                                                                                                                                                                                                                    X(COpACx8, 1) X(COpACx16, 1) X(COpB4x8, 1) X(COpB4x16, 1)                             \
                                                                                                                                                                                                                                        X(COpBCx8, 1) X(COpBCx16, 1) X(COp01m8, 1) X(COp01m16, 1)                         \
                                                                                                                                                                                                                                            X(COp03m8, 1) X(COp03m16, 1) X(COp05m8, 1) X(COp05m16, 1)                     \
                                                                                                                                                                                                                                                X(COp07m8, 1) X(COp07m16, 1) X(COp09m8, 1) X(COp09m16, 1)                 \
                                                                                                                                                                                                                                                    X(COp0Dm8, 1) X(COp0Dm16, 1) X(COp0Fm8, 1) X(COp0Fm16, 1)             \
                                                                                                                                                                                                                                                        X(COp11m8, 1) X(COp11m16, 1) X(COp12m8, 1) X(COp12m16, 1)         \
                                                                                                                                                                                                                                                            X(COp13m8, 1) X(COp13m16, 1) X(COp15m8, 1) X(COp15m16, 1)     \
                                                                                                                                                                                                                                                                X(COp17m8, 1) X(COp17m16, 1) X(COp19m8, 1) X(COp19m16, 1) \
                                                                                                                                                                                                                                                                    X(COp1Dm8, 1) X(COp1Dm16, 1) X(COp1Fm8, 1) X(COp1Fm16, 1)

#define DECL(n, b) extern void asm_##n(void);
OPS(DECL)
#undef DECL

static const struct {
    char const* name;
    void (*a)(void);
    void (*c)(u4*);
    int bytebx;
} ops[] = {
#define ENT(n, b) { #n, asm_##n, c_##n, b },
    OPS(ENT)
#undef ENT
};

/* Everything both sides may read or write. */
struct state {
    u4 r[8];
    u4 xa, xx, xy, xs, xd, xdb, xpb, flagnz, flago, flagc;
    u2 xpc;
    u2 stackand, stackor;
    u1 ram[RAMSZ];
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
    xe = s->xe;
    flagnz = s->flagnz;
    flago = s->flago;
    flagc = s->flagc;
    stackand = s->stackand;
    stackor = s->stackor;
    memcpy(fakeram, s->ram, RAMSZ);
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
    s->xe = xe;
    s->flagnz = flagnz;
    s->flago = flago;
    s->flagc = flagc;
    s->stackand = stackand;
    s->stackor = stackor;
    memcpy(s->ram, fakeram, RAMSZ);
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
    static u1 code[64];
    static u4 tab[1024];
    int fails = 0;

    /* Set up once, not per iteration: every entry is the same stand-in, and
       filling 1024 of them inside the loop dominated the whole run. */
    for (size_t i = 0; i < 1024; i++) {
        memtabler8[i] = membank0r8;
        memtabler16[i] = membank0r16;
    }
    DPageR8 = membank0r8;
    DPageR16 = membank0r16;

    for (size_t o = 0; o < sizeof ops / sizeof ops[0]; o++) {
        for (size_t i = 0; i < 1024; i++) {
            snesmmap[i] = code + (dt_u32() & 7u);
            snesmap2[i] = code + (dt_u32() & 7u);
        }
        DT_MAIN(12345 + (int)o, 20000)
        {
            struct state in, a, c;

            dt_fill(code, sizeof code);
            dt_fill(tab, sizeof tab);
            memcpy(tablead, tab, sizeof tablead);

            for (int i = 0; i < 8; i++)
                in.r[i] = dt_u32();
            in.r[R_ESI] = (u4)(uintptr_t)(code + 16);
            if (ops[o].bytebx)
                in.r[R_EBX] = dt_u32() & 0x3FFu;
            in.xa = dt_u32();
            in.xx = dt_u32();
            in.xy = dt_u32();
            in.xs = dt_u32();
            in.xd = dt_u32();
            in.xdb = dt_u32();
            in.xpb = dt_u32() & 0x3FFu;
            in.xpc = (u2)dt_u32();
            in.xe = dt_mod(4) == 0 ? (u1)dt_u32() : (u1)dt_mod(2);
            in.flagnz = nzval();
            in.flago = cvval();
            in.flagc = cvval();
            in.stackand = (u2)dt_u32();
            in.stackor = (u2)dt_u32();
            dt_fill(in.ram, RAMSZ);

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
            DT_EQ("xe", a.xe, c.xe);
            DT_EQ("flagnz", a.flagnz, c.flagnz);
            DT_EQ("flago", a.flago, c.flago);
            DT_EQ("flagc", a.flagc, c.flagc);
            DT_EQ("stackand", a.stackand, c.stackand);
            DT_EQ("stackor", a.stackor, c.stackor);
            DT_MEM("ram", a.ram, c.ram, RAMSZ);
        }
        if (dt_fails) {
            printf("  ^^ %s: %d/%ld mismatched\n", ops[o].name, dt_fails,
                dt_iters);
            fails++;
        }
    }

    if (fails) {
        printf("65816 opcodes: FAIL (%d of %zu opcodes mismatched)\n", fails,
            sizeof ops / sizeof ops[0]);
        return 1;
    }
    printf("65816 opcodes: PASS (%zu opcodes x %ld iterations bit-identical "
           "to asm)\n",
        sizeof ops / sizeof ops[0], dt_iters);
    return 0;
}
