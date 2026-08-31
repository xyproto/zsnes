/*
 * 65816 opcode handlers, from cpu/e65816.inc. Textual include (cpu/c_65816.c);
 * the includer supplies the typedefs and the register file, laid out as pushad
 * left it: esi = PC, dl = flags, dh = cycles left, edi = the opcode table,
 * ebp = SPC700 PC. A/X/Y/S/D are 32-bit globals written at the mode's width,
 * so every store masks - 8-bit mode must leave the high half intact for REP.
 */
#ifndef OPS65816_H
#define OPS65816_H

#include <string.h>

/* The 65816 PC is an arbitrary byte address, so operand loads are unaligned
   and would fault on a strict-alignment target. memcpy compiles to the same
   instruction wherever the direct load was legal. */
static inline u2 rd16(zreg const p)
{
    u2 v;
    memcpy(&v, (void const*)(uintptr_t)p, sizeof v);
    return v;
}

static inline u4 rd32(zreg const p)
{
    u4 v;
    memcpy(&v, (void const*)(uintptr_t)p, sizeof v);
    return v;
}

/*
 * Entry points go through OP() so the file can be included twice: once for the
 * 65816 and once for the SA-1's copy, the same core over a different register
 * file. cpu/c_ops65816_sa1.c defines OP and renames the dozen globals that
 * differ; xpc, xe, the memory tables and the stack masks really are shared.
 */
#ifndef OP
#define OP(n) c_##n
#endif

/* An instantiation that needs a handler of its own defines OPS_OWN_<opcode> and
   writes it after including this file. The SA-1 needs six; every other one of
   the 517 bodies is byte-identical between the two cores. */

#include "flags65816.h"
#include "memseam.h"

/* pushad pushes eax first and edi last, so the block reads in this order. */
enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

/* Z is the low 16 bits of flagnz, N is bit 15 (8-bit) or bit 16 (16-bit). */
static inline void setnz8(u1 const al) { flagnz = (u4)al << 8; }

/* `mov cx,ax` leaves the top half of ecx in place, and it lands in flagnz. */
static inline void setnz16(zreg* const r, u2 const ax)
{
    r[R_ECX] = (r[R_ECX] & 0xFFFF0000u) | ax;
    flagnz = r[R_ECX];
}

/* dl selects the opcode table, so clearing or setting a width flag reloads it.
   The index is the whole of ebx, as in the assembly; it stays inside tablead
   because the dispatcher only ever loads bl and so keeps the upper bits zero. */
static inline void reload_table(zreg* const r)
{
    r[R_EBX] = (r[R_EBX] & 0xFFFFFF00u) | (u1)r[R_EDX];
    r[R_EDI] = (zreg)(uintptr_t)tablead[r[R_EBX]];
}

/* Read and write the low byte or word of a 32-bit register global. */
#define GET8(v) ((u1)(v))
#define GET16(v) ((u2)(v))
#define SET8(v, b) ((v) = ((v) & 0xFFFFFF00u) | (u1)(b))
#define SET16(v, w) ((v) = ((v) & 0xFFFF0000u) | (u2)(w))

/* eax is scratch but survives the handler, so the moves through al/ax show. */
#define AL(r, b) SET8((r)[R_EAX], (b))
#define AX(r, w) SET16((r)[R_EAX], (w))

/*
 * Transfers. Every one is `mov a<w>,[src]` / `mov [dst],a<w>` / flags, so the
 * pair of macros below is the whole family; only TCS, TXS and XBA differ.
 */
#define TRANSFER8(name, src, dst)          \
    void name(zreg* const r)                 \
    {                                      \
        AL(r, GET8(src));                  \
        SET8(dst, GET8(r[R_EAX]));         \
        setnz8(GET8(r[R_EAX]));            \
    }
#define TRANSFER16(name, src, dst)         \
    void name(zreg* const r)                 \
    {                                      \
        AX(r, GET16(src));                 \
        SET16(dst, GET16(r[R_EAX]));       \
        setnz16(r, GET16(r[R_EAX]));       \
    }

/* Increments and decrements of X and Y work in place, then reload al/ax. */
#define INCDEC8(name, reg, op)             \
    void name(zreg* const r)                 \
    {                                      \
        SET8(reg, GET8(reg) op 1);         \
        AL(r, GET8(reg));                  \
        setnz8(GET8(r[R_EAX]));            \
    }
#define INCDEC16(name, reg, op)            \
    void name(zreg* const r)                 \
    {                                      \
        SET16(reg, GET16(reg) op 1);       \
        AX(r, GET16(reg));                 \
        setnz16(r, GET16(r[R_EAX]));       \
    }

/*
 * Branches. The displacement is fetched only when the branch is taken, so eax
 * keeps its old value otherwise; the PC advances past the operand either way.
 * Conditions read the split flags (flags65816.h), tested the way round the
 * assembly tests them.
 */
#define BRANCH(name, taken)                                \
    void name(zreg* const r)                                 \
    {                                                      \
        if (taken) {                                       \
            s4 const rel = *(s1 const*)(uintptr_t)r[R_ESI]; \
            r[R_EAX] = (u4)rel;                            \
            /* Sign-extend to the slot width: a backward branch relies on the \
               add wrapping, which a (u4) cast only does when zreg is 32 bits. */ \
            r[R_ESI] += (zreg)rel;                         \
        }                                                  \
        r[R_ESI]++;                                        \
    }

BRANCH(OP(COp80), 1) /* BRA r */
BRANCH(OP(COp90), (flagc & 0x01u) == 0) /* BCC r */
BRANCH(OP(COpB0), (flagc & 0x01u) != 0) /* BCS r */
BRANCH(OP(COpF0), (flagnz & 0xFFFFu) == 0) /* BEQ r */
BRANCH(OP(COpD0), (flagnz & 0xFFFFu) != 0) /* BNE r */
BRANCH(OP(COp30), (flagnz & 0x18000u) != 0) /* BMI r */
BRANCH(OP(COp10), (flagnz & 0x18000u) == 0) /* BPL r */
BRANCH(OP(COp50), (flago & 0xFFu) == 0) /* BVC r */
BRANCH(OP(COp70), (flago & 0xFFu) != 0) /* BVS r */

void OP(COp18)(zreg* const r) /* CLC i */
{
    (void)r;
    flagc = 0;
}

void OP(COp38)(zreg* const r) /* SEC i */
{
    (void)r;
    flagc = 0xFF;
}

void OP(COpB8)(zreg* const r) /* CLV i */
{
    (void)r;
    flago = 0;
}

void OP(COpD8)(zreg* const r) /* CLD i */
{
    r[R_EDX] &= ~0x08u;
    reload_table(r);
}

void OP(COpF8)(zreg* const r) /* SED i */
{
    r[R_EDX] |= 0x08u;
    reload_table(r);
}

void OP(COp78)(zreg* const r) /* SEI i */
{
    r[R_EDX] |= 0x04u;
}

void OP(COpEA)(zreg* const r) /* NOP i */
{
    (void)r;
}

void OP(COpDB)(zreg* const r) /* STP i */
{
    r[R_ESI]--;
}

void OP(COp42)(zreg* const r) /* WDM */
{
    r[R_ESI]++;
}

INCDEC8(OP(COpCAx8), xx, -) /* DEX i */
INCDEC16(OP(COpCAx16), xx, -)
INCDEC8(OP(COpE8x8), xx, +) /* INX i */
INCDEC16(OP(COpE8x16), xx, +)
INCDEC8(OP(COp88x8), xy, -) /* DEY i */
INCDEC16(OP(COp88x16), xy, -)
INCDEC8(OP(COpC8x8), xy, +) /* INY i */
INCDEC16(OP(COpC8x16), xy, +)

TRANSFER8(OP(COpAAx8), xa, xx) /* TAX i */
TRANSFER16(OP(COpAAx16), xa, xx)
TRANSFER8(OP(COpA8x8), xa, xy) /* TAY i */
TRANSFER16(OP(COpA8x16), xa, xy)
TRANSFER8(OP(COpBAx8), xs, xx) /* TSX i */
TRANSFER16(OP(COpBAx16), xs, xx)
TRANSFER8(OP(COp8Am8), xx, xa) /* TXA i */
TRANSFER16(OP(COp8Am16), xx, xa)
TRANSFER8(OP(COp98m8), xy, xa) /* TYA i */
TRANSFER16(OP(COp98m16), xy, xa)
TRANSFER8(OP(COp9Bx8), xx, xy) /* TXY i */
TRANSFER16(OP(COp9Bx16), xx, xy)
TRANSFER8(OP(COpBBx8), xy, xx) /* TYX i */
TRANSFER16(OP(COpBBx16), xy, xx)

/* TDC and TSC are 16-bit whatever the M flag says. */
TRANSFER16(OP(COp7B), xd, xa) /* TDC i */
TRANSFER16(OP(COp3B), xs, xa) /* TSC i */

#ifndef OPS_OWN_COp1B
void OP(COp1B)(zreg* const r) /* TCS i */
{
    AX(r, GET16(xa));
    if (xe & 1) {
        SET8(xs, GET8(r[R_EAX])); /* emulation mode keeps S in page one */
    } else {
        SET16(xs, GET16(r[R_EAX]));
    }
}
#endif

void OP(COp9A)(zreg* const r) /* TXS i */
{
    AX(r, GET16(xx));
    SET16(xs, GET16(r[R_EAX]));
    if (xe & 1) {
        xs = (xs & 0xFFFF00FFu) | 0x0100u;
    }
}

void OP(COpEB)(zreg* const r) /* XBA i */
{
    AX(r, (u2)((GET8(xa) << 8) | ((xa >> 8) & 0xFFu)));
    SET16(xa, GET16(r[R_EAX]));
    setnz8(GET8(r[R_EAX]));
}

/* INC A and DEC A go through the accumulator "addressing mode", which is just
   a load and a store of A at the current width. */
#define INCDECA8(name, op)                 \
    void name(zreg* const r)                 \
    {                                      \
        AL(r, GET8(xa));                   \
        AL(r, GET8(r[R_EAX]) op 1);        \
        setnz8(GET8(r[R_EAX]));            \
        SET8(xa, GET8(r[R_EAX]));          \
    }
#define INCDECA16(name, op)                \
    void name(zreg* const r)                 \
    {                                      \
        AX(r, GET16(xa));                  \
        AX(r, GET16(r[R_EAX]) op 1);       \
        setnz16(r, GET16(r[R_EAX]));       \
        SET16(xa, GET16(r[R_EAX]));        \
    }

INCDECA8(OP(COp1Am8), +) /* INC A */
INCDECA16(OP(COp1Am16), +)
INCDECA8(OP(COp3Am8), -) /* DEC A */
INCDECA16(OP(COp3Am16), -)

void OP(COp5B)(zreg* const r) /* TCD i */
{
    AX(r, GET16(xa));
    SET16(xd, GET16(r[R_EAX]));
    UpdateDPage(); /* the direct page moved; the access tables follow it */
    setnz16(r, GET16(r[R_EAX]));
}

/*
 * REP and SEP clear or set P bits named by an immediate. For M, X and D alone
 * the split flags are untouched and dl is edited in place; anything else goes
 * the long way through P. Only REP re-forces the emulation-mode bits and only
 * SEP narrows X and Y - the assembly's asymmetry, kept.
 */
void OP(COpC2)(zreg* const r) /* REP # */
{
    u1 const imm = *(u1 const*)r[R_ESI];
    int const extra = (imm & 0xC3u) != 0;

    AL(r, imm);
    r[R_ESI]++;
    AL(r, (u1)~imm);
    if (extra)
        r[R_EDX] = makedl(r[R_EDX]);
    SET8(r[R_EDX], (u1)r[R_EDX] & GET8(r[R_EAX]));
    if (extra)
        restoredl(r[R_EDX]);
    if (xe & 1)
        r[R_EDX] |= 0x30u;
    reload_table(r);
}

void OP(COpE2)(zreg* const r) /* SEP # */
{
    u1 const imm = *(u1 const*)r[R_ESI];
    int const extra = (imm & 0xC3u) != 0;

    AL(r, imm);
    r[R_ESI]++;
    if (extra)
        r[R_EDX] = makedl(r[R_EDX]);
    SET8(r[R_EDX], (u1)r[R_EDX] | imm);
    if (extra)
        restoredl(r[R_EDX]);
    reload_table(r);
    if (r[R_EDX] & 0x10u) {
        xx &= 0xFFFF00FFu; /* an 8-bit index register drops its high byte */
        xy &= 0xFFFF00FFu;
    }
}

#ifndef OPS_OWN_COpFB
void OP(COpFB)(zreg* const r) /* XCE i */
{
    AL(r, (u1)(flagc & 1u));
    flagc = 0;
    if (xe == GET8(r[R_EAX]))
        return; /* carry already matched E: nothing to swap */
    if (xe != 0)
        flagc = 0xFF;
    xe = GET8(r[R_EAX]);
    if (xe & 1) {
        r[R_EDX] |= 0x30u;
        reload_table(r);
        xx &= 0xFFFF00FFu;
        xy &= 0xFFFF00FFu;
        xs = (xs & 0xFFFF00FFu) | 0x0100u;
        stackand = 0x01FF; /* emulation mode pins the stack to page one */
        stackor = 0x0100;
    } else {
        r[R_EDX] |= 0x20u; /* native mode, and no table reload here */
        stackand = 0xFFFF;
        stackor = 0x0000;
    }
}
#endif

/*
 * Stack operations. Memory access spills eax/ebx/ecx/edx through the seam
 * (cpu/memseam.h) and all four can come back changed, so cx is re-read after
 * every access. S wraps inside a page in emulation mode and across the bank in
 * native mode; stackor/stackand carry that, and XCE sets them.
 */
static inline void bank0_call(zreg* const r, void (*const fn)(void))
{
    MemSeamA = r[R_EAX];
    MemSeamB = r[R_EBX];
    MemSeamC = r[R_ECX];
    MemSeamD = r[R_EDX];
    MemSeamS = r[R_ESI];
    fn();
    r[R_EAX] = MemSeamA;
    r[R_EBX] = MemSeamB;
    r[R_ECX] = MemSeamC;
    r[R_EDX] = MemSeamD;
}

static inline void push8(zreg* const r, u1 const al)
{
    AL(r, al);
    bank0_call(r, c_membank0w8);
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
}

static inline u1 pop8(zreg* const r)
{
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + 1) & stackand);
    bank0_call(r, c_membank0r8);
    return GET8(r[R_EAX]);
}

#define PUSH8(name, src)                   \
    void name(zreg* const r)                 \
    {                                      \
        SET16(r[R_ECX], GET16(xs));        \
        push8(r, GET8(src));               \
        SET16(xs, GET16(r[R_ECX]));        \
    }
#define PUSH16(name, src)                  \
    void name(zreg* const r)                 \
    {                                      \
        SET16(r[R_ECX], GET16(xs));        \
        push8(r, (u1)((src) >> 8));        \
        push8(r, GET8(src));               \
        SET16(xs, GET16(r[R_ECX]));        \
    }

#ifndef OPS_OWN_COp48m8
PUSH8(OP(COp48m8), xa) /* PHA s */
#endif
PUSH16(OP(COp48m16), xa)
#ifndef OPS_OWN_COp8B
PUSH8(OP(COp8B), xdb) /* PHB s */
#endif
PUSH16(OP(COp0B), xd) /* PHD s */
#ifndef OPS_OWN_COp4B
PUSH8(OP(COp4B), xpb) /* PHK s */
#endif
#ifndef OPS_OWN_COpDAx8
PUSH8(OP(COpDAx8), xx) /* PHX s */
#endif
PUSH16(OP(COpDAx16), xx)
#ifndef OPS_OWN_COp5Ax8
PUSH8(OP(COp5Ax8), xy) /* PHY s */
#endif
PUSH16(OP(COp5Ax16), xy)

#ifndef OPS_OWN_COp08
void OP(COp08)(zreg* const r) /* PHP s */
{
    r[R_EDX] = makedl(r[R_EDX]);
    SET16(r[R_ECX], GET16(xs));
    push8(r, (u1)r[R_EDX]);
    SET16(xs, GET16(r[R_ECX]));
}
#endif

#define POP8(name, dst)                    \
    void name(zreg* const r)                 \
    {                                      \
        u1 v;                              \
        SET16(r[R_ECX], GET16(xs));        \
        v = pop8(r);                       \
        SET16(xs, GET16(r[R_ECX]));        \
        SET8(dst, v);                      \
        setnz8(v);                         \
    }

/* The 16-bit pull reassembles ax from the high byte still in al and the low
   byte re-read out of the register it just wrote, not from a local. */
#define POP16(name, dst)                                     \
    void name(zreg* const r)                                   \
    {                                                        \
        u1 hi;                                               \
        SET16(r[R_ECX], GET16(xs));                          \
        SET8(dst, pop8(r));                                  \
        SET16(xs, GET16(r[R_ECX]));                          \
        hi = pop8(r);                                        \
        (dst) = ((dst) & 0xFFFF00FFu) | (u4)hi << 8;         \
        SET16(xs, GET16(r[R_ECX]));                          \
        AX(r, (u2)((u2)hi << 8 | GET8(dst)));                \
        setnz16(r, GET16(r[R_EAX]));                         \
    }

#ifndef OPS_OWN_COp68m8
POP8(OP(COp68m8), xa) /* PLA s */
#endif
#ifndef OPS_OWN_COp68m16
POP16(OP(COp68m16), xa)
#endif
#ifndef OPS_OWN_COpAB
POP8(OP(COpAB), xdb) /* PLB s */
#endif
#ifndef OPS_OWN_COpFAx8
POP8(OP(COpFAx8), xx) /* PLX s */
#endif
#ifndef OPS_OWN_COpFAx16
POP16(OP(COpFAx16), xx)
#endif
#ifndef OPS_OWN_COp7Ax8
POP8(OP(COp7Ax8), xy) /* PLY s */
#endif
#ifndef OPS_OWN_COp7Ax16
POP16(OP(COp7Ax16), xy)
#endif

#ifndef OPS_OWN_COp2B
void OP(COp2B)(zreg* const r) /* PLD s */
{
    u1 hi;
    SET16(r[R_ECX], GET16(xs));
    SET8(xd, pop8(r));
    SET16(xs, GET16(r[R_ECX]));
    hi = pop8(r);
    xd = (xd & 0xFFFF00FFu) | (u4)hi << 8;
    UpdateDPage();
    SET16(xs, GET16(r[R_ECX]));
    AX(r, (u2)((u2)hi << 8 | GET8(xd)));
    setnz16(r, GET16(r[R_EAX]));
}
#endif

#ifndef OPS_OWN_COp28
void OP(COp28)(zreg* const r) /* PLP s */
{
    u1 p;
    SET16(r[R_ECX], GET16(xs));
    p = pop8(r);
    SET16(xs, GET16(r[R_ECX]));
    r[R_EBX] &= 0xFFFF00FFu; /* xor bh,bh */
    SET8(r[R_EDX], p);
    restoredl(r[R_EDX]);
    if (xe & 1) {
        r[R_EDX] |= 0x30u;
        reload_table(r);
        return;
    }
    reload_table(r);
    if (r[R_EDX] & 0x10u) {
        xx &= 0xFFFF00FFu;
        xy &= 0xFFFF00FFu;
    }
}
#endif

void OP(COpF4)(zreg* const r) /* PEA s */
{
    u1 const* const p = (u1 const*)r[R_ESI];
    SET16(r[R_ECX], GET16(xs));
    push8(r, p[1]);
    push8(r, p[0]);
    SET16(xs, GET16(r[R_ECX]));
    r[R_ESI] += 2;
}

/* PEI and PER both build a 16-bit value in ax and push it high byte first,
   saving eax around the first write because the access clobbers it. */
static void push16_ax(zreg* const r)
{
    u4 const saved = r[R_EAX];
    SET16(r[R_ECX], GET16(xs));
    AL(r, (u1)(GET16(r[R_EAX]) >> 8));
    bank0_call(r, c_membank0w8);
    r[R_EAX] = saved;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
    bank0_call(r, c_membank0w8);
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
    SET16(xs, GET16(r[R_ECX]));
}

void OP(COpD4)(zreg* const r) /* PEI s */
{
    r[R_EAX] &= 0xFFFF00FFu; /* xor ah,ah */
    AL(r, *(u1 const*)r[R_ESI]);
    SET16(r[R_ECX], GET16(xd));
    r[R_ESI]++;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EAX])));
    bank0_call(r, c_membank0r16);
    push16_ax(r);
}

void OP(COp62)(zreg* const r) /* PER s */
{
    /* The operand is relative to the 65816 PC, but esi is a host pointer, so
       the bank's base has to come back out of the memory map to recover it.
       Which map depends on where in the bank the PC currently is. */
    u1* const* map;

    SET8(r[R_EBX], GET8(xpb));
    AX(r, xpc);
    map = (r[R_EAX] & 0x8000u) ? snesmmap : snesmap2;
    r[R_EAX] = (zreg)(uintptr_t)map[r[R_EBX]];
    r[R_EBX] = r[R_ESI] - r[R_EAX];
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + rd16(r[R_ESI])));
    AX(r, GET16(r[R_EBX]));
    r[R_ESI] += 2;
    AX(r, (u2)(GET16(r[R_EAX]) + 2));
    push16_ax(r);
    r[R_EBX] = 0;
}

/*
 * Addressing modes: advance esi past the operand bytes, leave the value in al
 * or ax, reach memory through the per-bank tables (cpu/memseam.h). Two easy
 * losses: `add cx,bx` adds all of bx (safe only because bh is kept zero), and
 * the 16-bit carry out of `add cx,<index>` steps the bank - page crossing.
 */
static inline void mem_call(zreg* const r, eop* const fn)
{
    uintptr_t const b = MemSeamB, c = MemSeamC, a = MemSeamA, d = MemSeamD;

    MemSeamA = r[R_EAX];
    MemSeamB = r[R_EBX];
    MemSeamC = r[R_ECX];
    MemSeamD = r[R_EDX];
    MemSeamS = r[R_ESI];
    fn();
    r[R_EAX] = MemSeamA;
    r[R_EBX] = MemSeamB;
    r[R_ECX] = MemSeamC;
    r[R_EDX] = MemSeamD;
    /* Restore, so an access nested inside a handler leaves the outer one's
       seam alone; the register ABI this replaces got that for free. */
    MemSeamB = b;
    MemSeamC = c;
    MemSeamA = a;
    MemSeamD = d;
}

#define TABR8(r) mem_call((r), memtabler8[(r)[R_EBX]])
#define TABR16(r) mem_call((r), memtabler16[(r)[R_EBX]])
#define TABW8(r) mem_call((r), memtablew8[(r)[R_EBX]])
#define TABW16(r) mem_call((r), memtablew16[(r)[R_EBX]])

/* `add cx,idx` / `jnc .np` / `inc bl` */
static inline void idx_bank(zreg* const r, u2 const idx)
{
    u4 const sum = GET16(r[R_ECX]) + idx;
    SET16(r[R_ECX], (u2)sum);
    if (sum > 0xFFFFu)
        SET8(r[R_EBX], (u1)(GET8(r[R_EBX]) + 1));
}

/* The operand byte, and the direct page it indexes. */
static inline void dp_operand(zreg* const r)
{
    SET8(r[R_EBX], *(u1 const*)r[R_ESI]);
    r[R_ECX] = xd;
    r[R_ESI]++;
}

/* [d],l: a 24-bit pointer read out of the direct page, low word then bank. */
static inline void long_indirect(zreg* const r)
{
    u2 addr;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX])));
    addr = GET16(r[R_ECX]);
    bank0_call(r, c_membank0r16);
    SET16(r[R_ECX], (u2)(addr + 2));
    {
        /* `push ax` / `pop ax`, so only the low half is put back - the top of
           eax stays as the bank read left it. */
        u2 const saved = GET16(r[R_EAX]);
        bank0_call(r, c_membank0r8);
        SET8(r[R_EBX], GET8(r[R_EAX]));
        SET16(r[R_EAX], saved);
    }
    SET16(r[R_ECX], GET16(r[R_EAX]));
}

/* Immediate. The 16-bit form loads a full dword into eax, not a word. */
static void a_I_8(zreg* const r)
{
    AL(r, *(u1 const*)r[R_ESI]);
    r[R_ESI]++;
}
static void a_I_16(zreg* const r)
{
    r[R_EAX] = rd32(r[R_ESI]);
    r[R_ESI] += 2;
}

/* a, a,x, a,y - absolute in the data bank. */
#define ABS(name, tab, idx)                                     \
    static void name(zreg* const r)                               \
    {                                                           \
        SET16(r[R_ECX], rd16(r[R_ESI]));       \
        SET8(r[R_EBX], GET8(xdb));                              \
        r[R_ESI] += 2;                                          \
        idx;                                                    \
        tab(r);                                                 \
    }
ABS(a_a_8, TABR8, (void)0)
ABS(a_a_16, TABR16, (void)0)
ABS(a_aCx_8, TABR8, idx_bank(r, GET16(xx)))
ABS(a_aCx_16, TABR16, idx_bank(r, GET16(xx)))
ABS(a_aCy_8, TABR8, idx_bank(r, GET16(xy)))
ABS(a_aCy_16, TABR16, idx_bank(r, GET16(xy)))
ABS(a_a_8w, TABW8, (void)0)
ABS(a_a_16w, TABW16, (void)0)
ABS(a_aCx_8w, TABW8, idx_bank(r, GET16(xx)))
ABS(a_aCx_16w, TABW16, idx_bank(r, GET16(xx)))
ABS(a_aCy_8w, TABW8, idx_bank(r, GET16(xy)))
ABS(a_aCy_16w, TABW16, idx_bank(r, GET16(xy)))

/* al, al,x - absolute long, bank from the third operand byte. */
#define ABSL(name, tab, idx)                                    \
    static void name(zreg* const r)                               \
    {                                                           \
        SET16(r[R_ECX], rd16(r[R_ESI]));       \
        SET8(r[R_EBX], *(u1 const*)(uintptr_t)(r[R_ESI] + 2));  \
        r[R_ESI] += 3;                                          \
        idx;                                                    \
        tab(r);                                                 \
    }
ABSL(a_al_8, TABR8, (void)0)
ABSL(a_al_16, TABR16, (void)0)
ABSL(a_alCx_8, TABR8, idx_bank(r, GET16(xx)))
ABSL(a_alCx_16, TABR16, idx_bank(r, GET16(xx)))
ABSL(a_al_8w, TABW8, (void)0)
ABSL(a_al_16w, TABW16, (void)0)
ABSL(a_alCx_8w, TABW8, idx_bank(r, GET16(xx)))
ABSL(a_alCx_16w, TABW16, idx_bank(r, GET16(xx)))

/* d - direct page, through the pointer the page's base selects. */
static void a_d_8(zreg* const r)
{
    dp_operand(r);
    mem_call(r, DPageR8);
}
static void a_d_16(zreg* const r)
{
    dp_operand(r);
    mem_call(r, DPageR16);
}
static void a_d_8w(zreg* const r)
{
    dp_operand(r);
    mem_call(r, DPageW8);
}
static void a_d_16w(zreg* const r)
{
    dp_operand(r);
    mem_call(r, DPageW16);
}

/* d,x and d,y wrap inside the bank rather than the page, so they go the long
   way round instead of through the direct-page pointer. */
#define DPIDX(name, idx)                                          \
    static void name(zreg* const r)                                 \
    {                                                             \
        r[R_ECX] = xd;                                            \
        SET8(r[R_EBX], *(u1 const*)r[R_ESI]);          \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX]))); \
        r[R_ESI]++;                                               \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(idx)));      \
    }
DPIDX(dpidx_x, xx)
DPIDX(dpidx_y, xy)

static void a_dCx_8(zreg* const r)
{
    dpidx_x(r);
    bank0_call(r, c_membank0r8);
}
static void a_dCx_16(zreg* const r)
{
    dpidx_x(r);
    bank0_call(r, c_membank0r16);
}
static void a_dCy_8(zreg* const r)
{
    dpidx_y(r);
    bank0_call(r, c_membank0r8);
}
static void a_dCy_16(zreg* const r)
{
    dpidx_y(r);
    bank0_call(r, c_membank0r16);
}
static void a_dCx_8w(zreg* const r)
{
    dpidx_x(r);
    bank0_call(r, c_membank0w8);
}
static void a_dCx_16w(zreg* const r)
{
    dpidx_x(r);
    bank0_call(r, c_membank0w16);
}
static void a_dCy_8w(zreg* const r)
{
    dpidx_y(r);
    bank0_call(r, c_membank0w8);
}
static void a_dCy_16w(zreg* const r)
{
    dpidx_y(r);
    bank0_call(r, c_membank0w16);
}

/* d,s - stack relative. */
static inline void sp_rel(zreg* const r)
{
    SET8(r[R_EBX], *(u1 const*)r[R_ESI]);
    SET16(r[R_ECX], GET16(xs));
    r[R_ESI]++;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX])));
}
static void a_dCs_8(zreg* const r)
{
    sp_rel(r);
    bank0_call(r, c_membank0r8);
}
static void a_dCs_16(zreg* const r)
{
    sp_rel(r);
    bank0_call(r, c_membank0r16);
}
static void a_dCs_8w(zreg* const r)
{
    sp_rel(r);
    bank0_call(r, c_membank0w8);
}
static void a_dCs_16w(zreg* const r)
{
    sp_rel(r);
    bank0_call(r, c_membank0w16);
}

/* (d) and (d),y - a 16-bit pointer from the direct page, data bank. */
/*
 * A write form is its read form with ax saved across the pointer fetch. The
 * assembly's push/pop is 16-bit, so only the low half returns and whatever the
 * fetch left above it stays. Modes with no intermediate read need no save -
 * exactly the families below that take no `save` flag.
 */
#define DIND(name, tab, idx, save)        \
    static void name(zreg* const r)         \
    {                                     \
        u2 const keep = GET16(r[R_EAX]);  \
        (void)keep;                       \
        dp_operand(r);                    \
        mem_call(r, DPageR16);            \
        SET16(r[R_ECX], GET16(r[R_EAX])); \
        SET8(r[R_EBX], GET8(xdb));        \
        idx;                              \
        if (save)                         \
            SET16(r[R_EAX], keep);        \
        tab(r);                           \
    }
DIND(a_BdB_8, TABR8, (void)0, 0)
DIND(a_BdB_16, TABR16, (void)0, 0)
DIND(a_BdBCy_8, TABR8, idx_bank(r, GET16(xy)), 0)
DIND(a_BdBCy_16, TABR16, idx_bank(r, GET16(xy)), 0)
DIND(a_BdB_8w, TABW8, (void)0, 1)
DIND(a_BdB_16w, TABW16, (void)0, 1)
DIND(a_BdBCy_8w, TABW8, idx_bank(r, GET16(xy)), 1)
DIND(a_BdBCy_16w, TABW16, idx_bank(r, GET16(xy)), 1)

/* (d,x) - the direct page is indexed before the pointer is read. */
#define DINDX(name, tab, save)                                    \
    static void name(zreg* const r)                                 \
    {                                                             \
        u2 const keep = GET16(r[R_EAX]);                          \
        (void)keep;                                               \
        r[R_ECX] = xd;                                            \
        SET8(r[R_EBX], *(u1 const*)r[R_ESI]);          \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX]))); \
        r[R_ESI]++;                                               \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(xx)));       \
        bank0_call(r, c_membank0r16);                             \
        SET16(r[R_ECX], GET16(r[R_EAX]));                         \
        SET8(r[R_EBX], GET8(xdb));                                \
        if (save)                                                 \
            SET16(r[R_EAX], keep);                                \
        tab(r);                                                   \
    }
DINDX(a_BdCxB_8, TABR8, 0)
DINDX(a_BdCxB_16, TABR16, 0)
DINDX(a_BdCxB_8w, TABW8, 1)
DINDX(a_BdCxB_16w, TABW16, 1)

/* (d,s),y - stack relative, then indirect, then indexed. */
#define SIND(name, tab, save)             \
    static void name(zreg* const r)         \
    {                                     \
        u2 const keep = GET16(r[R_EAX]);  \
        (void)keep;                       \
        sp_rel(r);                        \
        bank0_call(r, c_membank0r16);     \
        SET16(r[R_ECX], GET16(r[R_EAX])); \
        SET8(r[R_EBX], GET8(xdb));        \
        if (save)                         \
            SET16(r[R_EAX], keep);        \
        idx_bank(r, GET16(xy));           \
        tab(r);                           \
    }
SIND(a_BdCsBCy_8, TABR8, 0)
SIND(a_BdCsBCy_16, TABR16, 0)
SIND(a_BdCsBCy_8w, TABW8, 1)
SIND(a_BdCsBCy_16w, TABW16, 1)

/* [d] and [d],y - long indirect, bank from the pointer itself. */
#define LIND(name, tab, idx, save)                                \
    static void name(zreg* const r)                                 \
    {                                                             \
        u2 const keep = GET16(r[R_EAX]);                          \
        (void)keep;                                               \
        SET8(r[R_EBX], *(u1 const*)r[R_ESI]);          \
        r[R_ECX] = xd;                                            \
        r[R_ESI]++;                                               \
        long_indirect(r);                                         \
        idx;                                                      \
        if (save)                                                 \
            SET16(r[R_EAX], keep);                                \
        tab(r);                                                   \
    }
LIND(a_LdL_8, TABR8, (void)0, 0)
LIND(a_LdL_16, TABR16, (void)0, 0)
LIND(a_LdL_8w, TABW8, (void)0, 1)
LIND(a_LdL_16w, TABW16, (void)0, 1)

/* The ,y form reads its operand before the direct page, not after. */
#define LINDY(name, tab, save)                                    \
    static void name(zreg* const r)                                 \
    {                                                             \
        u2 const keep = GET16(r[R_EAX]);                          \
        (void)keep;                                               \
        r[R_ECX] = xd;                                            \
        SET8(r[R_EBX], *(u1 const*)r[R_ESI]);          \
        r[R_ESI]++;                                               \
        long_indirect(r);                                         \
        idx_bank(r, GET16(xy));                                   \
        if (save)                                                 \
            SET16(r[R_EAX], keep);                                \
        tab(r);                                                   \
    }
LINDY(a_LdLCy_8, TABR8, 0)
LINDY(a_LdLCy_16, TABR16, 0)
LINDY(a_LdLCy_8w, TABW8, 1)
LINDY(a_LdLCy_16w, TABW16, 1)

/*
 * LDA over every addressing mode. The 8-bit form writes flagnz directly rather
 * than going through setnz8 - same result, and it is what the assembly does.
 */
#define LDA8(name, mode)               \
    void name(zreg* const r)             \
    {                                  \
        mode(r);                       \
        flagnz = 0;                    \
        SET8(xa, GET8(r[R_EAX]));      \
        flagnz = (u4)GET8(r[R_EAX]) << 8; \
    }
#define LDA16(name, mode)              \
    void name(zreg* const r)             \
    {                                  \
        mode(r);                       \
        SET16(xa, GET16(r[R_EAX]));    \
        setnz16(r, GET16(r[R_EAX]));   \
    }

LDA8(OP(COpA9m8), a_I_8) /* LDA # */
LDA16(OP(COpA9m16), a_I_16)
LDA8(OP(COpADm8), a_a_8) /* LDA a */
LDA16(OP(COpADm16), a_a_16)
LDA8(OP(COpBDm8), a_aCx_8) /* LDA a,x */
LDA16(OP(COpBDm16), a_aCx_16)
LDA8(OP(COpB9m8), a_aCy_8) /* LDA a,y */
LDA16(OP(COpB9m16), a_aCy_16)
LDA8(OP(COpAFm8), a_al_8) /* LDA al */
LDA16(OP(COpAFm16), a_al_16)
LDA8(OP(COpBFm8), a_alCx_8) /* LDA al,x */
LDA16(OP(COpBFm16), a_alCx_16)
LDA8(OP(COpA5m8), a_d_8) /* LDA d */
LDA16(OP(COpA5m16), a_d_16)
LDA8(OP(COpB5m8), a_dCx_8) /* LDA d,x */
LDA16(OP(COpB5m16), a_dCx_16)
LDA8(OP(COpA3m8), a_dCs_8) /* LDA d,s */
LDA16(OP(COpA3m16), a_dCs_16)
LDA8(OP(COpB2m8), a_BdB_8) /* LDA (d) */
LDA16(OP(COpB2m16), a_BdB_16)
LDA8(OP(COpB1m8), a_BdBCy_8) /* LDA (d),y */
LDA16(OP(COpB1m16), a_BdBCy_16)
LDA8(OP(COpA1m8), a_BdCxB_8) /* LDA (d,x) */
LDA16(OP(COpA1m16), a_BdCxB_16)
LDA8(OP(COpB3m8), a_BdCsBCy_8) /* LDA (d,s),y */
LDA16(OP(COpB3m16), a_BdCsBCy_16)
LDA8(OP(COpA7m8), a_LdL_8) /* LDA [d] */
LDA16(OP(COpA7m16), a_LdL_16)
LDA8(OP(COpB7m8), a_LdLCy_8) /* LDA [d],y */
LDA16(OP(COpB7m16), a_LdLCy_16)

/*
 * Operations, composed with an addressing mode by OPMODE below. The three
 * logical ones differ: ORA is 16-bit, AND and EOR are 32-bit on eax, so those
 * two change the register's top half and ORA does not.
 */
static void o_ORA8(zreg* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) | GET8(xa)));
    flagnz = 0;
    SET8(xa, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_ORA16(zreg* const r)
{
    AX(r, (u2)(GET16(r[R_EAX]) | GET16(xa)));
    SET16(xa, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}
static void o_AND8(zreg* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) & GET8(xa)));
    flagnz = 0;
    SET8(xa, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_AND16(zreg* const r)
{
    r[R_EAX] &= xa;
    SET16(xa, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}
static void o_EOR8(zreg* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) ^ GET8(xa)));
    flagnz = 0;
    SET8(xa, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_EOR16(zreg* const r)
{
    r[R_EAX] ^= xa;
    SET16(xa, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}

static void o_LDX8(zreg* const r)
{
    flagnz = 0;
    SET8(xx, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_LDX16(zreg* const r)
{
    SET16(xx, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}
static void o_LDY8(zreg* const r)
{
    flagnz = 0;
    SET8(xy, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_LDY16(zreg* const r)
{
    SET16(xy, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}

/* The comparisons subtract into cl/cx and leave the result there. The x86 carry
   out of a subtract is a borrow, so C is its inverse; and the 16-bit form
   writes the whole of ecx into flagnz rather than zeroing it first. */
static inline void cmp8(zreg* const r, u4 const reg)
{
    u4 const lhs = GET8(reg), rhs = GET8(r[R_EAX]);
    SET8(r[R_ECX], (u1)(lhs - rhs));
    flagnz = (u4)GET8(r[R_ECX]) << 8;
    flagc = lhs < rhs ? 0 : 0xFF;
}
static inline void cmp16(zreg* const r, u4 const reg)
{
    u4 const lhs = GET16(reg), rhs = GET16(r[R_EAX]);
    SET16(r[R_ECX], (u2)(lhs - rhs));
    flagnz = r[R_ECX];
    flagc = lhs < rhs ? 0 : 0xFF;
}

static void o_CMP8(zreg* const r) { cmp8(r, xa); }
static void o_CMP16(zreg* const r) { cmp16(r, xa); }
static void o_CPX8(zreg* const r) { cmp8(r, xx); }
static void o_CPX16(zreg* const r) { cmp16(r, xx); }
static void o_CPY8(zreg* const r) { cmp8(r, xy); }
static void o_CPY16(zreg* const r) { cmp16(r, xy); }

/* BIT takes N and V straight from the operand's top two bits and Z from the
   test against A. The Z store is a word, so the N bit above it survives. */
static void o_BIT8(zreg* const r)
{
    u1 const v = GET8(r[R_EAX]);
    flagnz = (v & 0x80u) ? 0x10000u : 0;
    flago = (v & 0x40u) ? 1u : 0;
    flagnz = (flagnz & 0xFFFF0000u) | ((GET8(xa) & v) ? 1u : 0u);
}
static void o_BIT16(zreg* const r)
{
    u2 const v = GET16(r[R_EAX]);
    flagnz = (v & 0x8000u) ? 0x10000u : 0;
    flago = (v & 0x4000u) ? 1u : 0;
    flagnz = (flagnz & 0xFFFF0000u) | ((GET16(xa) & v) ? 1u : 0u);
}

/* An opcode is an addressing mode followed by an operation. */
#define OPMODE(name, mode, op) \
    void name(zreg* const r)     \
    {                          \
        mode(r);               \
        op(r);                 \
    }

/* AND */
OPMODE(OP(COp21m8), a_BdCxB_8, o_AND8)
OPMODE(OP(COp21m16), a_BdCxB_16, o_AND16)
OPMODE(OP(COp23m8), a_dCs_8, o_AND8)
OPMODE(OP(COp23m16), a_dCs_16, o_AND16)
OPMODE(OP(COp25m8), a_d_8, o_AND8)
OPMODE(OP(COp25m16), a_d_16, o_AND16)
OPMODE(OP(COp27m8), a_LdL_8, o_AND8)
OPMODE(OP(COp27m16), a_LdL_16, o_AND16)
OPMODE(OP(COp29m8), a_I_8, o_AND8)
OPMODE(OP(COp29m16), a_I_16, o_AND16)
OPMODE(OP(COp2Dm8), a_a_8, o_AND8)
OPMODE(OP(COp2Dm16), a_a_16, o_AND16)
OPMODE(OP(COp2Fm8), a_al_8, o_AND8)
OPMODE(OP(COp2Fm16), a_al_16, o_AND16)
OPMODE(OP(COp31m8), a_BdBCy_8, o_AND8)
OPMODE(OP(COp31m16), a_BdBCy_16, o_AND16)
OPMODE(OP(COp32m8), a_BdB_8, o_AND8)
OPMODE(OP(COp32m16), a_BdB_16, o_AND16)
OPMODE(OP(COp33m8), a_BdCsBCy_8, o_AND8)
OPMODE(OP(COp33m16), a_BdCsBCy_16, o_AND16)
OPMODE(OP(COp35m8), a_dCx_8, o_AND8)
OPMODE(OP(COp35m16), a_dCx_16, o_AND16)
OPMODE(OP(COp37m8), a_LdLCy_8, o_AND8)
OPMODE(OP(COp37m16), a_LdLCy_16, o_AND16)
OPMODE(OP(COp39m8), a_aCy_8, o_AND8)
OPMODE(OP(COp39m16), a_aCy_16, o_AND16)
OPMODE(OP(COp3Dm8), a_aCx_8, o_AND8)
OPMODE(OP(COp3Dm16), a_aCx_16, o_AND16)
OPMODE(OP(COp3Fm8), a_alCx_8, o_AND8)
OPMODE(OP(COp3Fm16), a_alCx_16, o_AND16)

/* BIT */
OPMODE(OP(COp24m8), a_d_8, o_BIT8)
OPMODE(OP(COp24m16), a_d_16, o_BIT16)
OPMODE(OP(COp2Cm8), a_a_8, o_BIT8)
OPMODE(OP(COp2Cm16), a_a_16, o_BIT16)
OPMODE(OP(COp34m8), a_dCx_8, o_BIT8)
OPMODE(OP(COp34m16), a_dCx_16, o_BIT16)
OPMODE(OP(COp3Cm8), a_aCx_8, o_BIT8)
OPMODE(OP(COp3Cm16), a_aCx_16, o_BIT16)

/* CMP */
OPMODE(OP(COpC1m8), a_BdCxB_8, o_CMP8)
OPMODE(OP(COpC1m16), a_BdCxB_16, o_CMP16)
OPMODE(OP(COpC3m8), a_dCs_8, o_CMP8)
OPMODE(OP(COpC3m16), a_dCs_16, o_CMP16)
OPMODE(OP(COpC5m8), a_d_8, o_CMP8)
OPMODE(OP(COpC5m16), a_d_16, o_CMP16)
OPMODE(OP(COpC7m8), a_LdL_8, o_CMP8)
OPMODE(OP(COpC7m16), a_LdL_16, o_CMP16)
OPMODE(OP(COpC9m8), a_I_8, o_CMP8)
OPMODE(OP(COpC9m16), a_I_16, o_CMP16)
OPMODE(OP(COpCDm8), a_a_8, o_CMP8)
OPMODE(OP(COpCDm16), a_a_16, o_CMP16)
OPMODE(OP(COpCFm8), a_al_8, o_CMP8)
OPMODE(OP(COpCFm16), a_al_16, o_CMP16)
OPMODE(OP(COpD1m8), a_BdBCy_8, o_CMP8)
OPMODE(OP(COpD1m16), a_BdBCy_16, o_CMP16)
OPMODE(OP(COpD2m8), a_BdB_8, o_CMP8)
OPMODE(OP(COpD2m16), a_BdB_16, o_CMP16)
OPMODE(OP(COpD3m8), a_BdCsBCy_8, o_CMP8)
OPMODE(OP(COpD3m16), a_BdCsBCy_16, o_CMP16)
OPMODE(OP(COpD5m8), a_dCx_8, o_CMP8)
OPMODE(OP(COpD5m16), a_dCx_16, o_CMP16)
OPMODE(OP(COpD7m8), a_LdLCy_8, o_CMP8)
OPMODE(OP(COpD7m16), a_LdLCy_16, o_CMP16)
OPMODE(OP(COpD9m8), a_aCy_8, o_CMP8)
OPMODE(OP(COpD9m16), a_aCy_16, o_CMP16)
OPMODE(OP(COpDDm8), a_aCx_8, o_CMP8)
OPMODE(OP(COpDDm16), a_aCx_16, o_CMP16)
OPMODE(OP(COpDFm8), a_alCx_8, o_CMP8)
OPMODE(OP(COpDFm16), a_alCx_16, o_CMP16)

/* CPX */
OPMODE(OP(COpE0x8), a_I_8, o_CPX8)
OPMODE(OP(COpE0x16), a_I_16, o_CPX16)
OPMODE(OP(COpE4x8), a_d_8, o_CPX8)
OPMODE(OP(COpE4x16), a_d_16, o_CPX16)
OPMODE(OP(COpECx8), a_a_8, o_CPX8)
OPMODE(OP(COpECx16), a_a_16, o_CPX16)

/* CPY */
OPMODE(OP(COpC0x8), a_I_8, o_CPY8)
OPMODE(OP(COpC0x16), a_I_16, o_CPY16)
OPMODE(OP(COpC4x8), a_d_8, o_CPY8)
OPMODE(OP(COpC4x16), a_d_16, o_CPY16)
OPMODE(OP(COpCCx8), a_a_8, o_CPY8)
OPMODE(OP(COpCCx16), a_a_16, o_CPY16)

/* EOR */
OPMODE(OP(COp41m8), a_BdCxB_8, o_EOR8)
OPMODE(OP(COp41m16), a_BdCxB_16, o_EOR16)
OPMODE(OP(COp43m8), a_dCs_8, o_EOR8)
OPMODE(OP(COp43m16), a_dCs_16, o_EOR16)
OPMODE(OP(COp45m8), a_d_8, o_EOR8)
OPMODE(OP(COp45m16), a_d_16, o_EOR16)
OPMODE(OP(COp47m8), a_LdL_8, o_EOR8)
OPMODE(OP(COp47m16), a_LdL_16, o_EOR16)
OPMODE(OP(COp49m8), a_I_8, o_EOR8)
OPMODE(OP(COp49m16), a_I_16, o_EOR16)
OPMODE(OP(COp4Dm8), a_a_8, o_EOR8)
OPMODE(OP(COp4Dm16), a_a_16, o_EOR16)
OPMODE(OP(COp4Fm8), a_al_8, o_EOR8)
OPMODE(OP(COp4Fm16), a_al_16, o_EOR16)
OPMODE(OP(COp51m8), a_BdBCy_8, o_EOR8)
OPMODE(OP(COp51m16), a_BdBCy_16, o_EOR16)
OPMODE(OP(COp52m8), a_BdB_8, o_EOR8)
OPMODE(OP(COp52m16), a_BdB_16, o_EOR16)
OPMODE(OP(COp53m8), a_BdCsBCy_8, o_EOR8)
OPMODE(OP(COp53m16), a_BdCsBCy_16, o_EOR16)
OPMODE(OP(COp55m8), a_dCx_8, o_EOR8)
OPMODE(OP(COp55m16), a_dCx_16, o_EOR16)
OPMODE(OP(COp57m8), a_LdLCy_8, o_EOR8)
OPMODE(OP(COp57m16), a_LdLCy_16, o_EOR16)
OPMODE(OP(COp59m8), a_aCy_8, o_EOR8)
OPMODE(OP(COp59m16), a_aCy_16, o_EOR16)
OPMODE(OP(COp5Dm8), a_aCx_8, o_EOR8)
OPMODE(OP(COp5Dm16), a_aCx_16, o_EOR16)
OPMODE(OP(COp5Fm8), a_alCx_8, o_EOR8)
OPMODE(OP(COp5Fm16), a_alCx_16, o_EOR16)

/* LDX */
OPMODE(OP(COpA2x8), a_I_8, o_LDX8)
OPMODE(OP(COpA2x16), a_I_16, o_LDX16)
OPMODE(OP(COpA6x8), a_d_8, o_LDX8)
OPMODE(OP(COpA6x16), a_d_16, o_LDX16)
OPMODE(OP(COpAEx8), a_a_8, o_LDX8)
OPMODE(OP(COpAEx16), a_a_16, o_LDX16)
OPMODE(OP(COpB6x8), a_dCy_8, o_LDX8)
OPMODE(OP(COpB6x16), a_dCy_16, o_LDX16)
OPMODE(OP(COpBEx8), a_aCy_8, o_LDX8)
OPMODE(OP(COpBEx16), a_aCy_16, o_LDX16)

/* LDY */
OPMODE(OP(COpA0x8), a_I_8, o_LDY8)
OPMODE(OP(COpA0x16), a_I_16, o_LDY16)
OPMODE(OP(COpA4x8), a_d_8, o_LDY8)
OPMODE(OP(COpA4x16), a_d_16, o_LDY16)
OPMODE(OP(COpACx8), a_a_8, o_LDY8)
OPMODE(OP(COpACx16), a_a_16, o_LDY16)
OPMODE(OP(COpB4x8), a_dCx_8, o_LDY8)
OPMODE(OP(COpB4x16), a_dCx_16, o_LDY16)
OPMODE(OP(COpBCx8), a_aCx_8, o_LDY8)
OPMODE(OP(COpBCx16), a_aCx_16, o_LDY16)

/* ORA */
OPMODE(OP(COp01m8), a_BdCxB_8, o_ORA8)
OPMODE(OP(COp01m16), a_BdCxB_16, o_ORA16)
OPMODE(OP(COp03m8), a_dCs_8, o_ORA8)
OPMODE(OP(COp03m16), a_dCs_16, o_ORA16)
OPMODE(OP(COp05m8), a_d_8, o_ORA8)
OPMODE(OP(COp05m16), a_d_16, o_ORA16)
OPMODE(OP(COp07m8), a_LdL_8, o_ORA8)
OPMODE(OP(COp07m16), a_LdL_16, o_ORA16)
OPMODE(OP(COp09m8), a_I_8, o_ORA8)
OPMODE(OP(COp09m16), a_I_16, o_ORA16)
OPMODE(OP(COp0Dm8), a_a_8, o_ORA8)
OPMODE(OP(COp0Dm16), a_a_16, o_ORA16)
OPMODE(OP(COp0Fm8), a_al_8, o_ORA8)
OPMODE(OP(COp0Fm16), a_al_16, o_ORA16)
OPMODE(OP(COp11m8), a_BdBCy_8, o_ORA8)
OPMODE(OP(COp11m16), a_BdBCy_16, o_ORA16)
OPMODE(OP(COp12m8), a_BdB_8, o_ORA8)
OPMODE(OP(COp12m16), a_BdB_16, o_ORA16)
OPMODE(OP(COp13m8), a_BdCsBCy_8, o_ORA8)
OPMODE(OP(COp13m16), a_BdCsBCy_16, o_ORA16)
OPMODE(OP(COp15m8), a_dCx_8, o_ORA8)
OPMODE(OP(COp15m16), a_dCx_16, o_ORA16)
OPMODE(OP(COp17m8), a_LdLCy_8, o_ORA8)
OPMODE(OP(COp17m16), a_LdLCy_16, o_ORA16)
OPMODE(OP(COp19m8), a_aCy_8, o_ORA8)
OPMODE(OP(COp19m16), a_aCy_16, o_ORA16)
OPMODE(OP(COp1Dm8), a_aCx_8, o_ORA8)
OPMODE(OP(COp1Dm16), a_aCx_16, o_ORA16)
OPMODE(OP(COp1Fm8), a_alCx_8, o_ORA8)
OPMODE(OP(COp1Fm16), a_alCx_16, o_ORA16)


/*
 * Stores run the other way round: the operation loads al/ax and the addressing
 * mode writes it. The 16-bit forms load all of eax, and 16-bit STZ clears all
 * of it, so the upper half differs between the widths.
 */
static void o_STA8(zreg* const r) { AL(r, GET8(xa)); }
static void o_STA16(zreg* const r) { r[R_EAX] = xa; }
static void o_STX8(zreg* const r) { AL(r, GET8(xx)); }
static void o_STX16(zreg* const r) { r[R_EAX] = xx; }
static void o_STY8(zreg* const r) { AL(r, GET8(xy)); }
static void o_STY16(zreg* const r) { r[R_EAX] = xy; }
static void o_STZ8(zreg* const r) { AL(r, 0); }
static void o_STZ16(zreg* const r) { r[R_EAX] = 0; }

/* A store is an operation followed by an addressing mode, not the reverse. */
#define STMODE(name, op, mode) \
    void name(zreg* const r)     \
    {                          \
        op(r);                 \
        mode(r);               \
    }

/* STA */
STMODE(OP(COp81m8), o_STA8, a_BdCxB_8w)
STMODE(OP(COp81m16), o_STA16, a_BdCxB_16w)
STMODE(OP(COp83m8), o_STA8, a_dCs_8w)
STMODE(OP(COp83m16), o_STA16, a_dCs_16w)
STMODE(OP(COp85m8), o_STA8, a_d_8w)
STMODE(OP(COp85m16), o_STA16, a_d_16w)
STMODE(OP(COp87m8), o_STA8, a_LdL_8w)
STMODE(OP(COp87m16), o_STA16, a_LdL_16w)
STMODE(OP(COp8Dm8), o_STA8, a_a_8w)
STMODE(OP(COp8Dm16), o_STA16, a_a_16w)
STMODE(OP(COp8Fm8), o_STA8, a_al_8w)
STMODE(OP(COp8Fm16), o_STA16, a_al_16w)
STMODE(OP(COp91m8), o_STA8, a_BdBCy_8w)
STMODE(OP(COp91m16), o_STA16, a_BdBCy_16w)
STMODE(OP(COp92m8), o_STA8, a_BdB_8w)
STMODE(OP(COp92m16), o_STA16, a_BdB_16w)
STMODE(OP(COp93m8), o_STA8, a_BdCsBCy_8w)
STMODE(OP(COp93m16), o_STA16, a_BdCsBCy_16w)
STMODE(OP(COp95m8), o_STA8, a_dCx_8w)
STMODE(OP(COp95m16), o_STA16, a_dCx_16w)
STMODE(OP(COp97m8), o_STA8, a_LdLCy_8w)
STMODE(OP(COp97m16), o_STA16, a_LdLCy_16w)
STMODE(OP(COp99m8), o_STA8, a_aCy_8w)
STMODE(OP(COp99m16), o_STA16, a_aCy_16w)
STMODE(OP(COp9Dm8), o_STA8, a_aCx_8w)
STMODE(OP(COp9Dm16), o_STA16, a_aCx_16w)
STMODE(OP(COp9Fm8), o_STA8, a_alCx_8w)
STMODE(OP(COp9Fm16), o_STA16, a_alCx_16w)

/* STX */
STMODE(OP(COp86x8), o_STX8, a_d_8w)
STMODE(OP(COp86x16), o_STX16, a_d_16w)
STMODE(OP(COp8Ex8), o_STX8, a_a_8w)
STMODE(OP(COp8Ex16), o_STX16, a_a_16w)
STMODE(OP(COp96x8), o_STX8, a_dCy_8w)
STMODE(OP(COp96x16), o_STX16, a_dCy_16w)

/* STY */
STMODE(OP(COp84x8), o_STY8, a_d_8w)
STMODE(OP(COp84x16), o_STY16, a_d_16w)
STMODE(OP(COp8Cx8), o_STY8, a_a_8w)
STMODE(OP(COp8Cx16), o_STY16, a_a_16w)
STMODE(OP(COp94x8), o_STY8, a_dCx_8w)
STMODE(OP(COp94x16), o_STY16, a_dCx_16w)

/* STZ */
STMODE(OP(COp64m8), o_STZ8, a_d_8w)
STMODE(OP(COp64m16), o_STZ16, a_d_16w)
STMODE(OP(COp74m8), o_STZ8, a_dCx_8w)
STMODE(OP(COp74m16), o_STZ16, a_dCx_16w)
STMODE(OP(COp9Cm8), o_STZ8, a_a_8w)
STMODE(OP(COp9Cm16), o_STZ16, a_a_16w)
STMODE(OP(COp9Em8), o_STZ8, a_aCx_8w)
STMODE(OP(COp9Em16), o_STZ16, a_aCx_16w)


/*
 * Read-modify-write. The read half leaves esi alone - all `brni` means -
 * because the write half re-reads the operand bytes and advances. Only four
 * modes are used this way.
 */
/* The no-advance form of A is `mov ax,[xa]`, a word, where the plain read form
   is `mov eax,[xa]`, a dword. The widths really do differ. */
static void a_A_8ni(zreg* const r) { AL(r, GET8(xa)); }
static void a_A_16ni(zreg* const r) { AX(r, GET16(xa)); }
static void a_A_8w(zreg* const r) { SET8(xa, GET8(r[R_EAX])); }
static void a_A_16w(zreg* const r) { SET16(xa, GET16(r[R_EAX])); }

#define ABSNI(name, tab, idx)                             \
    static void name(zreg* const r)                         \
    {                                                     \
        SET16(r[R_ECX], rd16(r[R_ESI])); \
        SET8(r[R_EBX], GET8(xdb));                        \
        idx;                                              \
        tab(r);                                           \
    }
ABSNI(a_a_8ni, TABR8, (void)0)
ABSNI(a_a_16ni, TABR16, (void)0)
ABSNI(a_aCx_8ni, TABR8, idx_bank(r, GET16(xx)))
ABSNI(a_aCx_16ni, TABR16, idx_bank(r, GET16(xx)))

static void a_d_8ni(zreg* const r)
{
    SET8(r[R_EBX], *(u1 const*)r[R_ESI]);
    r[R_ECX] = xd;
    mem_call(r, DPageR8);
}
static void a_d_16ni(zreg* const r)
{
    SET8(r[R_EBX], *(u1 const*)r[R_ESI]);
    r[R_ECX] = xd;
    mem_call(r, DPageR16);
}

#define DPIDXNI(name, fn)                                         \
    static void name(zreg* const r)                                 \
    {                                                             \
        r[R_ECX] = xd;                                            \
        SET8(r[R_EBX], *(u1 const*)r[R_ESI]);          \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX]))); \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(xx)));       \
        bank0_call(r, fn);                                        \
    }
DPIDXNI(a_dCx_8ni, c_membank0r8)
DPIDXNI(a_dCx_16ni, c_membank0r16)

/* `mov cl,[flagc]` / `add cl,cl` puts bit 7 of flagc into the carry and leaves
   the doubled byte in cl, which the 8-bit flag store does not overwrite. */
static inline int carry_in(zreg* const r)
{
    u1 const cl = GET8(flagc);
    SET8(r[R_ECX], (u1)(cl + cl));
    return (cl & 0x80u) != 0;
}
static inline void setnzc8(u1 const al, int const cf)
{
    flagnz = (u4)al << 8;
    flagc = cf ? 0xFF : 0;
}
static inline void setnzc16(zreg* const r, u2 const ax, int const cf)
{
    SET16(r[R_ECX], ax);
    flagnz = r[R_ECX];
    flagc = cf ? 0xFF : 0;
}

static void o_ASL8(zreg* const r)
{
    u1 const v = GET8(r[R_EAX]);
    AL(r, (u1)(v << 1));
    setnzc8(GET8(r[R_EAX]), (v & 0x80u) != 0);
}
static void o_ASL16(zreg* const r)
{
    u2 const v = GET16(r[R_EAX]);
    AX(r, (u2)(v << 1));
    setnzc16(r, GET16(r[R_EAX]), (v & 0x8000u) != 0);
}
static void o_LSR8(zreg* const r)
{
    u1 const v = GET8(r[R_EAX]);
    AL(r, (u1)(v >> 1));
    setnzc8(GET8(r[R_EAX]), (v & 1u) != 0);
}
static void o_LSR16(zreg* const r)
{
    u2 const v = GET16(r[R_EAX]);
    AX(r, (u2)(v >> 1));
    setnzc16(r, GET16(r[R_EAX]), (v & 1u) != 0);
}
static void o_ROL8(zreg* const r)
{
    int const ci = carry_in(r);
    u1 const v = GET8(r[R_EAX]);
    AL(r, (u1)((u1)(v << 1) | (u1)ci));
    setnzc8(GET8(r[R_EAX]), (v & 0x80u) != 0);
}
static void o_ROL16(zreg* const r)
{
    int const ci = carry_in(r);
    u2 const v = GET16(r[R_EAX]);
    AX(r, (u2)((u2)(v << 1) | (u2)ci));
    setnzc16(r, GET16(r[R_EAX]), (v & 0x8000u) != 0);
}
static void o_ROR8(zreg* const r)
{
    int const ci = carry_in(r);
    u1 const v = GET8(r[R_EAX]);
    AL(r, (u1)((v >> 1) | (u1)(ci << 7)));
    setnzc8(GET8(r[R_EAX]), (v & 1u) != 0);
}
static void o_ROR16(zreg* const r)
{
    int const ci = carry_in(r);
    u2 const v = GET16(r[R_EAX]);
    AX(r, (u2)((v >> 1) | (u2)(ci << 15)));
    setnzc16(r, GET16(r[R_EAX]), (v & 1u) != 0);
}

/* INC and DEC of memory set N and Z only. */
static void o_INCm8(zreg* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) + 1));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_INCm16(zreg* const r)
{
    AX(r, (u2)(GET16(r[R_EAX]) + 1));
    setnz16(r, GET16(r[R_EAX]));
}
static void o_DECm8(zreg* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) - 1));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_DECm16(zreg* const r)
{
    AX(r, (u2)(GET16(r[R_EAX]) - 1));
    setnz16(r, GET16(r[R_EAX]));
}

/* TSB and TRB fold an existing bit-15 N up into bit 16 first, so that the word
   store of Z below it cannot wipe N out. */
static inline void tsb_flags(u4 const nz)
{
    if (flagnz & 0x18000u)
        flagnz |= 0x10000u;
    flagnz = (flagnz & 0xFFFF0000u) | (nz ? 1u : 0u);
}
static void o_TSB8(zreg* const r)
{
    u1 const cl = GET8(xa);
    SET8(r[R_ECX], cl);
    tsb_flags(GET8(r[R_EAX]) & cl);
    AL(r, (u1)(GET8(r[R_EAX]) | cl));
}
static void o_TSB16(zreg* const r)
{
    u2 const cx = GET16(xa);
    SET16(r[R_ECX], cx);
    tsb_flags(GET16(r[R_EAX]) & cx);
    AX(r, (u2)(GET16(r[R_EAX]) | cx));
}
static void o_TRB8(zreg* const r)
{
    u1 const cl = GET8(xa);
    SET8(r[R_ECX], cl);
    tsb_flags(cl & GET8(r[R_EAX]));
    SET8(r[R_ECX], (u1)~cl);
    AL(r, (u1)(GET8(r[R_EAX]) & GET8(r[R_ECX])));
}
static void o_TRB16(zreg* const r)
{
    u2 const cx = GET16(xa);
    SET16(r[R_ECX], cx);
    tsb_flags(cx & GET16(r[R_EAX]));
    SET16(r[R_ECX], (u2)~cx);
    AX(r, (u2)(GET16(r[R_EAX]) & GET16(r[R_ECX])));
}

/* Read without advancing, operate, then write - the write re-reads the operand
   and moves esi on. */
#define RMW(name, mode_ni, op, mode_w) \
    void name(zreg* const r)             \
    {                                  \
        mode_ni(r);                    \
        op(r);                         \
        mode_w(r);                     \
    }

/* ASL */
RMW(OP(COp06m8), a_d_8ni, o_ASL8, a_d_8w)
RMW(OP(COp06m16), a_d_16ni, o_ASL16, a_d_16w)
RMW(OP(COp0Am8), a_A_8ni, o_ASL8, a_A_8w)
RMW(OP(COp0Am16), a_A_16ni, o_ASL16, a_A_16w)
RMW(OP(COp0Em8), a_a_8ni, o_ASL8, a_a_8w)
RMW(OP(COp0Em16), a_a_16ni, o_ASL16, a_a_16w)
RMW(OP(COp16m8), a_dCx_8ni, o_ASL8, a_dCx_8w)
RMW(OP(COp16m16), a_dCx_16ni, o_ASL16, a_dCx_16w)
RMW(OP(COp1Em8), a_aCx_8ni, o_ASL8, a_aCx_8w)
RMW(OP(COp1Em16), a_aCx_16ni, o_ASL16, a_aCx_16w)

/* DECm */
RMW(OP(COpCEm8), a_a_8ni, o_DECm8, a_a_8w)
RMW(OP(COpCEm16), a_a_16ni, o_DECm16, a_a_16w)
RMW(OP(COpC6m8), a_d_8ni, o_DECm8, a_d_8w)
RMW(OP(COpC6m16), a_d_16ni, o_DECm16, a_d_16w)
#ifndef OPS_OWN_COpD6m8
RMW(OP(COpD6m8), a_dCx_8ni, o_DECm8, a_dCx_8w)
#endif
RMW(OP(COpD6m16), a_dCx_16ni, o_DECm16, a_dCx_16w)
RMW(OP(COpDEm8), a_aCx_8ni, o_DECm8, a_aCx_8w)
RMW(OP(COpDEm16), a_aCx_16ni, o_DECm16, a_aCx_16w)

/* INCm */
RMW(OP(COpEEm8), a_a_8ni, o_INCm8, a_a_8w)
RMW(OP(COpEEm16), a_a_16ni, o_INCm16, a_a_16w)
RMW(OP(COpE6m8), a_d_8ni, o_INCm8, a_d_8w)
RMW(OP(COpE6m16), a_d_16ni, o_INCm16, a_d_16w)
RMW(OP(COpF6m8), a_dCx_8ni, o_INCm8, a_dCx_8w)
RMW(OP(COpF6m16), a_dCx_16ni, o_INCm16, a_dCx_16w)
RMW(OP(COpFEm8), a_aCx_8ni, o_INCm8, a_aCx_8w)
RMW(OP(COpFEm16), a_aCx_16ni, o_INCm16, a_aCx_16w)

/* LSR */
RMW(OP(COp46m8), a_d_8ni, o_LSR8, a_d_8w)
RMW(OP(COp46m16), a_d_16ni, o_LSR16, a_d_16w)
RMW(OP(COp4Am8), a_A_8ni, o_LSR8, a_A_8w)
RMW(OP(COp4Am16), a_A_16ni, o_LSR16, a_A_16w)
RMW(OP(COp4Em8), a_a_8ni, o_LSR8, a_a_8w)
RMW(OP(COp4Em16), a_a_16ni, o_LSR16, a_a_16w)
RMW(OP(COp56m8), a_dCx_8ni, o_LSR8, a_dCx_8w)
RMW(OP(COp56m16), a_dCx_16ni, o_LSR16, a_dCx_16w)
RMW(OP(COp5Em8), a_aCx_8ni, o_LSR8, a_aCx_8w)
RMW(OP(COp5Em16), a_aCx_16ni, o_LSR16, a_aCx_16w)

/* ROL */
RMW(OP(COp26m8), a_d_8ni, o_ROL8, a_d_8w)
RMW(OP(COp26m16), a_d_16ni, o_ROL16, a_d_16w)
RMW(OP(COp2Am8), a_A_8ni, o_ROL8, a_A_8w)
RMW(OP(COp2Am16), a_A_16ni, o_ROL16, a_A_16w)
RMW(OP(COp2Em8), a_a_8ni, o_ROL8, a_a_8w)
RMW(OP(COp2Em16), a_a_16ni, o_ROL16, a_a_16w)
RMW(OP(COp36m8), a_dCx_8ni, o_ROL8, a_dCx_8w)
RMW(OP(COp36m16), a_dCx_16ni, o_ROL16, a_dCx_16w)
RMW(OP(COp3Em8), a_aCx_8ni, o_ROL8, a_aCx_8w)
RMW(OP(COp3Em16), a_aCx_16ni, o_ROL16, a_aCx_16w)

/* ROR */
RMW(OP(COp66m8), a_d_8ni, o_ROR8, a_d_8w)
RMW(OP(COp66m16), a_d_16ni, o_ROR16, a_d_16w)
RMW(OP(COp6Am8), a_A_8ni, o_ROR8, a_A_8w)
RMW(OP(COp6Am16), a_A_16ni, o_ROR16, a_A_16w)
RMW(OP(COp6Em8), a_a_8ni, o_ROR8, a_a_8w)
RMW(OP(COp6Em16), a_a_16ni, o_ROR16, a_a_16w)
RMW(OP(COp76m8), a_dCx_8ni, o_ROR8, a_dCx_8w)
RMW(OP(COp76m16), a_dCx_16ni, o_ROR16, a_dCx_16w)
RMW(OP(COp7Em8), a_aCx_8ni, o_ROR8, a_aCx_8w)
RMW(OP(COp7Em16), a_aCx_16ni, o_ROR16, a_aCx_16w)

/* TRB */
RMW(OP(COp14m8), a_d_8ni, o_TRB8, a_d_8w)
RMW(OP(COp14m16), a_d_16ni, o_TRB16, a_d_16w)
RMW(OP(COp1Cm8), a_a_8ni, o_TRB8, a_a_8w)
RMW(OP(COp1Cm16), a_a_16ni, o_TRB16, a_a_16w)

/* TSB */
RMW(OP(COp04m8), a_d_8ni, o_TSB8, a_d_8w)
RMW(OP(COp04m16), a_d_16ni, o_TSB16, a_d_16w)
RMW(OP(COp0Cm8), a_a_8ni, o_TSB8, a_a_8w)
RMW(OP(COp0Cm16), a_a_16ni, o_TSB16, a_a_16w)

/*
 * ADC and SBC. Binary forms are a plain x86 add/subtract-with-carry; decimal
 * forms follow with DAA or DAS, whose OF the core reads via `seto byte[flago]`.
 * Carry in differs: ADC does `add cl,cl` and takes bit 7 of flagc, SBC does
 * `sub cl,1` and borrows when flagc's low byte is zero. Both leave cl modified.
 */
static inline int borrow_in(zreg* const r)
{
    u1 const cl = GET8(flagc);
    SET8(r[R_ECX], (u1)(cl - 1));
    return cl == 0;
}

/* `seto` writes one byte, so flago's upper three survive. */
static inline void nvzc8(zreg* const r, int const of, int const cf)
{
    flagnz = 0;
    AL(r, GET8(xa));
    SET8(flago, of ? 1 : 0);
    flagnz = (u4)GET8(r[R_EAX]) << 8;
    flagc = cf ? 0xFF : 0;
}
static inline void nvzc16(zreg* const r, int const of, int const cf)
{
    SET16(r[R_ECX], GET16(xa));
    SET8(flago, of ? 1 : 0);
    flagnz = r[R_ECX];
    flagc = cf ? 0xFF : 0;
}

/*
 * Intel's DAA and DAS, verbatim: the second adjustment tests the values from
 * before the first, and DAS leaves CF alone when not taken. OF too, because
 * the assembly stored it: on x86 it is the signed overflow of the combined
 * adjustment on the entering AL, checked over all 1024 (AL, CF, AF) states.
 */
static inline int decimal_of(u1 const old, u4 const adj, u1 const res,
    int const sub)
{
    if (adj == 0)
        return 0;
    if (sub)
        return ((old ^ (u1)adj) & (old ^ res) & 0x80u) != 0;
    return ((old ^ res) & ((u1)adj ^ res) & 0x80u) != 0;
}

static inline u4 daa_adjust(u1 const al, int const cf, int const af)
{
    u4 adj = 0;

    if ((al & 0x0Fu) > 9 || af)
        adj += 6u;
    if (al > 0x99u || cf)
        adj += 0x60u;
    return adj;
}

static inline u1 daa_adj(u1 al, int* const cf, int const af, int* const of)
{
    u1 const old = al;
    int const oldcf = *cf;
    u4 const adj = daa_adjust(old, oldcf, af);

    *of = decimal_of(old, adj, (u1)(old + adj), 0);
    if ((al & 0x0Fu) > 9 || af) {
        *cf = oldcf || ((u4)al + 6u > 0xFFu);
        al = (u1)(al + 6u);
    }
    if (old > 0x99u || oldcf) {
        al = (u1)(al + 0x60u);
        *cf = 1;
    } else {
        *cf = 0;
    }
    return al;
}
static inline u1 das_adj(u1 al, int* const cf, int const af, int* const of)
{
    u1 const old = al;
    int const oldcf = *cf;
    u4 const adj = daa_adjust(old, oldcf, af);

    *of = decimal_of(old, adj, (u1)(old - adj), 1);
    if ((al & 0x0Fu) > 9 || af) {
        *cf = oldcf || (al < 6u);
        al = (u1)(al - 6u);
    }
    if (old > 0x99u || oldcf) {
        al = (u1)(al - 0x60u);
        *cf = 1;
    }
    return al;
}

static void o_ADC8nd(zreg* const r)
{
    int const ci = carry_in(r);
    u1 const a = GET8(xa), v = GET8(r[R_EAX]);
    u4 const sum = (u4)a + v + (u4)ci;
    SET8(xa, (u1)sum);
    nvzc8(r, ((a ^ (u1)sum) & (v ^ (u1)sum) & 0x80u) != 0, sum > 0xFFu);
}
static void o_ADC16nd(zreg* const r)
{
    int const ci = carry_in(r);
    u2 const a = GET16(xa), v = GET16(r[R_EAX]);
    u4 const sum = (u4)a + v + (u4)ci;
    SET16(xa, (u2)sum);
    nvzc16(r, ((a ^ (u2)sum) & (v ^ (u2)sum) & 0x8000u) != 0, sum > 0xFFFFu);
}
static void o_SBC8nd(zreg* const r)
{
    int const bi = borrow_in(r);
    u1 const a = GET8(xa), v = GET8(r[R_EAX]);
    u4 const dif = (u4)a - v - (u4)bi;
    SET8(xa, (u1)dif);
    nvzc8(r, ((a ^ v) & (a ^ (u1)dif) & 0x80u) != 0, (dif & 0x100u) == 0);
}
static void o_SBC16nd(zreg* const r)
{
    int const bi = borrow_in(r);
    u2 const a = GET16(xa), v = GET16(r[R_EAX]);
    u4 const dif = (u4)a - v - (u4)bi;
    SET16(xa, (u2)dif);
    nvzc16(r, ((a ^ v) & (a ^ (u2)dif) & 0x8000u) != 0, (dif & 0x10000u) == 0);
}

static void o_ADC8d(zreg* const r)
{
    int const ci = carry_in(r);
    u1 const v = GET8(r[R_EAX]);
    u1 a, s;
    u4 sum;
    int cf, af, of;
    SET8(r[R_ECX], v); /* mov cl,al */
    AL(r, GET8(xa));
    a = GET8(r[R_EAX]);
    sum = (u4)a + v + (u4)ci;
    s = (u1)sum;
    cf = sum > 0xFFu;
    af = ((a ^ v ^ s) & 0x10u) != 0;
    AL(r, daa_adj(s, &cf, af, &of));
    SET8(xa, GET8(r[R_EAX]));
    nvzc8(r, of, cf);
}
static void o_SBC8d(zreg* const r)
{
    int const bi = borrow_in(r);
    u1 const v = GET8(r[R_EAX]);
    u1 a, s;
    u4 dif;
    int cf, af, of;
    SET8(r[R_ECX], v);
    AL(r, GET8(xa));
    a = GET8(r[R_EAX]);
    dif = (u4)a - v - (u4)bi;
    s = (u1)dif;
    cf = (dif & 0x100u) != 0;
    af = ((a ^ v ^ s) & 0x10u) != 0;
    AL(r, das_adj(s, &cf, af, &of));
    SET8(xa, GET8(r[R_EAX]));
    nvzc8(r, of, !cf);
}

/* The 16-bit decimal forms work a byte at a time, adjusting after each, with
   the carry running from one into the next. */
static void o_ADC16d(zreg* const r)
{
    int cf = carry_in(r);
    u2 const v = GET16(r[R_EAX]);
    int i, of = 0;
    SET16(r[R_ECX], v); /* mov cx,ax */
    for (i = 0; i < 2; i++) {
        u1 const a = (u1)(xa >> (8 * i));
        u1 const b = (u1)(GET16(r[R_ECX]) >> (8 * i));
        u4 const sum = (u4)a + b + (u4)cf;
        u1 const s = (u1)sum;
        int const af = ((a ^ b ^ s) & 0x10u) != 0;
        cf = sum > 0xFFu;
        AL(r, daa_adj(s, &cf, af, &of));
        xa = (xa & ~(0xFFu << (8 * i))) | (u4)GET8(r[R_EAX]) << (8 * i);
    }
    nvzc16(r, of, cf);
}
static void o_SBC16d(zreg* const r)
{
    int cf = borrow_in(r);
    u2 const v = GET16(r[R_EAX]);
    int i, of = 0;
    SET16(r[R_ECX], v);
    for (i = 0; i < 2; i++) {
        u1 const a = (u1)(xa >> (8 * i));
        u1 const b = (u1)(GET16(r[R_ECX]) >> (8 * i));
        u4 const dif = (u4)a - b - (u4)cf;
        u1 const s = (u1)dif;
        int const af = ((a ^ b ^ s) & 0x10u) != 0;
        cf = (dif & 0x100u) != 0;
        AL(r, das_adj(s, &cf, af, &of));
        xa = (xa & ~(0xFFu << (8 * i))) | (u4)GET8(r[R_EAX]) << (8 * i);
    }
    nvzc16(r, of, !cf); /* the `cmc` before the flag store */
}

/* ADC8nd */
OPMODE(OP(COp61m8nd), a_BdCxB_8, o_ADC8nd)
OPMODE(OP(COp63m8nd), a_dCs_8, o_ADC8nd)
OPMODE(OP(COp65m8nd), a_d_8, o_ADC8nd)
OPMODE(OP(COp67m8nd), a_LdL_8, o_ADC8nd)
OPMODE(OP(COp69m8nd), a_I_8, o_ADC8nd)
OPMODE(OP(COp6Dm8nd), a_a_8, o_ADC8nd)
OPMODE(OP(COp6Fm8nd), a_al_8, o_ADC8nd)
OPMODE(OP(COp71m8nd), a_BdBCy_8, o_ADC8nd)
OPMODE(OP(COp72m8nd), a_BdB_8, o_ADC8nd)
OPMODE(OP(COp73m8nd), a_BdCsBCy_8, o_ADC8nd)
OPMODE(OP(COp75m8nd), a_dCx_8, o_ADC8nd)
OPMODE(OP(COp77m8nd), a_LdLCy_8, o_ADC8nd)
OPMODE(OP(COp79m8nd), a_aCy_8, o_ADC8nd)
OPMODE(OP(COp7Dm8nd), a_aCx_8, o_ADC8nd)
OPMODE(OP(COp7Fm8nd), a_alCx_8, o_ADC8nd)

/* ADC16nd */
OPMODE(OP(COp61m16nd), a_BdCxB_16, o_ADC16nd)
OPMODE(OP(COp63m16nd), a_dCs_16, o_ADC16nd)
OPMODE(OP(COp65m16nd), a_d_16, o_ADC16nd)
OPMODE(OP(COp67m16nd), a_LdL_16, o_ADC16nd)
OPMODE(OP(COp69m16nd), a_I_16, o_ADC16nd)
OPMODE(OP(COp6Dm16nd), a_a_16, o_ADC16nd)
OPMODE(OP(COp6Fm16nd), a_al_16, o_ADC16nd)
OPMODE(OP(COp71m16nd), a_BdBCy_16, o_ADC16nd)
OPMODE(OP(COp72m16nd), a_BdB_16, o_ADC16nd)
OPMODE(OP(COp73m16nd), a_BdCsBCy_16, o_ADC16nd)
OPMODE(OP(COp75m16nd), a_dCx_16, o_ADC16nd)
OPMODE(OP(COp77m16nd), a_LdLCy_16, o_ADC16nd)
OPMODE(OP(COp79m16nd), a_aCy_16, o_ADC16nd)
OPMODE(OP(COp7Dm16nd), a_aCx_16, o_ADC16nd)
OPMODE(OP(COp7Fm16nd), a_alCx_16, o_ADC16nd)

/* ADC8d */
OPMODE(OP(COp61m8d), a_BdCxB_8, o_ADC8d)
OPMODE(OP(COp63m8d), a_dCs_8, o_ADC8d)
OPMODE(OP(COp65m8d), a_d_8, o_ADC8d)
OPMODE(OP(COp67m8d), a_LdL_8, o_ADC8d)
OPMODE(OP(COp69m8d), a_I_8, o_ADC8d)
OPMODE(OP(COp6Dm8d), a_a_8, o_ADC8d)
OPMODE(OP(COp6Fm8d), a_al_8, o_ADC8d)
OPMODE(OP(COp71m8d), a_BdBCy_8, o_ADC8d)
OPMODE(OP(COp72m8d), a_BdB_8, o_ADC8d)
OPMODE(OP(COp73m8d), a_BdCsBCy_8, o_ADC8d)
OPMODE(OP(COp75m8d), a_dCx_8, o_ADC8d)
OPMODE(OP(COp77m8d), a_LdLCy_8, o_ADC8d)
OPMODE(OP(COp79m8d), a_aCy_8, o_ADC8d)
OPMODE(OP(COp7Dm8d), a_aCx_8, o_ADC8d)
OPMODE(OP(COp7Fm8d), a_alCx_8, o_ADC8d)

/* ADC16d */
OPMODE(OP(COp61m16d), a_BdCxB_16, o_ADC16d)
OPMODE(OP(COp63m16d), a_dCs_16, o_ADC16d)
OPMODE(OP(COp65m16d), a_d_16, o_ADC16d)
OPMODE(OP(COp67m16d), a_LdL_16, o_ADC16d)
OPMODE(OP(COp69m16d), a_I_16, o_ADC16d)
OPMODE(OP(COp6Dm16d), a_a_16, o_ADC16d)
OPMODE(OP(COp6Fm16d), a_al_16, o_ADC16d)
OPMODE(OP(COp71m16d), a_BdBCy_16, o_ADC16d)
OPMODE(OP(COp72m16d), a_BdB_16, o_ADC16d)
OPMODE(OP(COp73m16d), a_BdCsBCy_16, o_ADC16d)
OPMODE(OP(COp75m16d), a_dCx_16, o_ADC16d)
OPMODE(OP(COp77m16d), a_LdLCy_16, o_ADC16d)
OPMODE(OP(COp79m16d), a_aCy_16, o_ADC16d)
OPMODE(OP(COp7Dm16d), a_aCx_16, o_ADC16d)
OPMODE(OP(COp7Fm16d), a_alCx_16, o_ADC16d)

/* SBC8nd */
OPMODE(OP(COpE1m8nd), a_BdCxB_8, o_SBC8nd)
OPMODE(OP(COpE3m8nd), a_dCs_8, o_SBC8nd)
OPMODE(OP(COpE5m8nd), a_d_8, o_SBC8nd)
OPMODE(OP(COpE7m8nd), a_LdL_8, o_SBC8nd)
OPMODE(OP(COpE9m8nd), a_I_8, o_SBC8nd)
OPMODE(OP(COpEDm8nd), a_a_8, o_SBC8nd)
OPMODE(OP(COpEFm8nd), a_al_8, o_SBC8nd)
OPMODE(OP(COpF1m8nd), a_BdBCy_8, o_SBC8nd)
OPMODE(OP(COpF2m8nd), a_BdB_8, o_SBC8nd)
OPMODE(OP(COpF3m8nd), a_BdCsBCy_8, o_SBC8nd)
OPMODE(OP(COpF5m8nd), a_dCx_8, o_SBC8nd)
OPMODE(OP(COpF7m8nd), a_LdLCy_8, o_SBC8nd)
OPMODE(OP(COpF9m8nd), a_aCy_8, o_SBC8nd)
OPMODE(OP(COpFDm8nd), a_aCx_8, o_SBC8nd)
OPMODE(OP(COpFFm8nd), a_alCx_8, o_SBC8nd)

/* SBC16nd */
OPMODE(OP(COpE1m16nd), a_BdCxB_16, o_SBC16nd)
OPMODE(OP(COpE3m16nd), a_dCs_16, o_SBC16nd)
OPMODE(OP(COpE5m16nd), a_d_16, o_SBC16nd)
OPMODE(OP(COpE7m16nd), a_LdL_16, o_SBC16nd)
OPMODE(OP(COpE9m16nd), a_I_16, o_SBC16nd)
OPMODE(OP(COpEDm16nd), a_a_16, o_SBC16nd)
OPMODE(OP(COpEFm16nd), a_al_16, o_SBC16nd)
OPMODE(OP(COpF1m16nd), a_BdBCy_16, o_SBC16nd)
OPMODE(OP(COpF2m16nd), a_BdB_16, o_SBC16nd)
OPMODE(OP(COpF3m16nd), a_BdCsBCy_16, o_SBC16nd)
OPMODE(OP(COpF5m16nd), a_dCx_16, o_SBC16nd)
OPMODE(OP(COpF7m16nd), a_LdLCy_16, o_SBC16nd)
OPMODE(OP(COpF9m16nd), a_aCy_16, o_SBC16nd)
OPMODE(OP(COpFDm16nd), a_aCx_16, o_SBC16nd)
OPMODE(OP(COpFFm16nd), a_alCx_16, o_SBC16nd)

/* SBC8d */
OPMODE(OP(COpE1m8d), a_BdCxB_8, o_SBC8d)
OPMODE(OP(COpE3m8d), a_dCs_8, o_SBC8d)
OPMODE(OP(COpE5m8d), a_d_8, o_SBC8d)
OPMODE(OP(COpE7m8d), a_LdL_8, o_SBC8d)
OPMODE(OP(COpE9m8d), a_I_8, o_SBC8d)
OPMODE(OP(COpEDm8d), a_a_8, o_SBC8d)
OPMODE(OP(COpEFm8d), a_al_8, o_SBC8d)
OPMODE(OP(COpF1m8d), a_BdBCy_8, o_SBC8d)
OPMODE(OP(COpF2m8d), a_BdB_8, o_SBC8d)
OPMODE(OP(COpF3m8d), a_BdCsBCy_8, o_SBC8d)
OPMODE(OP(COpF5m8d), a_dCx_8, o_SBC8d)
OPMODE(OP(COpF7m8d), a_LdLCy_8, o_SBC8d)
OPMODE(OP(COpF9m8d), a_aCy_8, o_SBC8d)
OPMODE(OP(COpFDm8d), a_aCx_8, o_SBC8d)
OPMODE(OP(COpFFm8d), a_alCx_8, o_SBC8d)

/* SBC16d */
OPMODE(OP(COpE1m16d), a_BdCxB_16, o_SBC16d)
OPMODE(OP(COpE3m16d), a_dCs_16, o_SBC16d)
OPMODE(OP(COpE5m16d), a_d_16, o_SBC16d)
OPMODE(OP(COpE7m16d), a_LdL_16, o_SBC16d)
OPMODE(OP(COpE9m16d), a_I_16, o_SBC16d)
OPMODE(OP(COpEDm16d), a_a_16, o_SBC16d)
OPMODE(OP(COpEFm16d), a_al_16, o_SBC16d)
OPMODE(OP(COpF1m16d), a_BdBCy_16, o_SBC16d)
OPMODE(OP(COpF2m16d), a_BdB_16, o_SBC16d)
OPMODE(OP(COpF3m16d), a_BdCsBCy_16, o_SBC16d)
OPMODE(OP(COpF5m16d), a_dCx_16, o_SBC16d)
OPMODE(OP(COpF7m16d), a_LdLCy_16, o_SBC16d)
OPMODE(OP(COpF9m16d), a_aCy_16, o_SBC16d)
OPMODE(OP(COpFDm16d), a_aCx_16, o_SBC16d)
OPMODE(OP(COpFFm16d), a_alCx_16, o_SBC16d)

/*
 * Control flow. esi is a host pointer into the mapped bank, not a 65816
 * address, so every jump goes back through the memory map: pick snesmmap or
 * snesmap2 by where in the bank the target is, keep that base in initaddrl,
 * add the offset. The `dma` flag: absolute jumps and JSR also route $4300 and
 * up in a register bank's low half at dmadata; the indirect ones do not.
 */
static inline u1* bank_base(u4 const eax, u4 const ebx, int const dma)
{
    if (eax & 0x8000u)
        return snesmmap[ebx];
    if (dma && eax >= 0x4300u && memtabler8[ebx] == regaccessbankr8)
        return dmadata - 0x4300;
    return snesmap2[ebx];
}
static inline void jump_to(zreg* const r, int const dma)
{
    u1* const base = bank_base(r[R_EAX], r[R_EBX], dma);
    initaddrl = base;
    r[R_ESI] = (zreg)(uintptr_t)base + r[R_EAX];
}

/* The 65816 PC that esi currently stands for. */
static inline u2 pc_now(u4 const esi) { return (u2)(esi - (zreg)(uintptr_t)initaddrl); }

#ifndef OPS_OWN_COp4C
void OP(COp4C)(zreg* const r) /* JMP a */
{
    r[R_EAX] = 0;
    AX(r, rd16(r[R_ESI]));
    SET8(r[R_EBX], GET8(xpb));
    xpc = GET16(r[R_EAX]);
    jump_to(r, 1);
}
#endif

void OP(COp6C)(zreg* const r) /* JMP (a) */
{
    SET16(r[R_ECX], rd16(r[R_ESI]));
    r[R_EAX] = 0;
    bank0_call(r, c_membank0r16);
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}

void OP(COp7C)(zreg* const r) /* JMP (a,x) */
{
    SET16(r[R_ECX], rd16(r[R_ESI]));
    r[R_EAX] = 0;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(xx)));
    SET8(r[R_EBX], GET8(xpb));
    TABR16(r);
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}

void OP(COp5C)(zreg* const r) /* JMP al */
{
    r[R_EAX] = 0;
    SET8(r[R_EBX], *(u1 const*)(uintptr_t)(r[R_ESI] + 2));
    AX(r, rd16(r[R_ESI]));
    SET8(xpb, GET8(r[R_EBX]));
    xpc = GET16(r[R_EAX]);
    jump_to(r, 0);
}

void OP(COpDC)(zreg* const r) /* JML (a) */
{
    u4 saved;
    SET16(r[R_ECX], rd16(r[R_ESI]));
    r[R_EAX] = 0;
    bank0_call(r, c_membank0r16);
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + 2));
    saved = r[R_EAX]; /* `push eax` here really is 32-bit */
    bank0_call(r, c_membank0r8);
    SET8(r[R_EBX], GET8(r[R_EAX]));
    r[R_EAX] = saved;
    xpc = GET16(r[R_EAX]);
    SET8(xpb, GET8(r[R_EBX]));
    jump_to(r, 0);
}

#ifndef OPS_OWN_COp82
void OP(COp82)(zreg* const r) /* BRL rl */
{
    r[R_EBX] = r[R_ESI] - (zreg)(uintptr_t)initaddrl;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 2));
    r[R_EAX] = 0;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + rd16(r[R_ESI])));
    AX(r, GET16(r[R_EBX]));
    r[R_EBX] = 0;
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}
#endif

void OP(COp60)(zreg* const r) /* RTS s */
{
    SET16(r[R_ECX], GET16(xs));
    xpc = (u2)((xpc & 0xFF00u) | pop8(r));
    xpc = (u2)((xpc & 0x00FFu) | (u2)pop8(r) << 8);
    SET16(xs, GET16(r[R_ECX]));
    r[R_EBX] &= 0xFFFF00FFu;
    r[R_EAX] = 0;
    AX(r, xpc);
    AX(r, (u2)(GET16(r[R_EAX]) + 1));
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}

void OP(COp6B)(zreg* const r) /* RTL s */
{
    SET16(r[R_ECX], GET16(xs));
    r[R_EAX] = 0;
    xpc = (u2)((xpc & 0xFF00u) | pop8(r));
    r[R_EAX] = 0;
    xpc = (u2)((xpc & 0x00FFu) | (u2)pop8(r) << 8);
    r[R_EAX] = 0;
    SET8(xpb, pop8(r));
    SET16(xs, GET16(r[R_ECX]));
    r[R_EBX] &= 0xFFFF00FFu;
    r[R_EAX] = 0;
    AX(r, xpc);
    AX(r, (u2)(GET16(r[R_EAX]) + 1));
    SET8(r[R_EBX], GET8(xpb));
    xpc = GET16(r[R_EAX]);
    jump_to(r, 0);
}

/* The return address a JSR pushes is the last byte of the instruction, not the
   next one, which is why RTS adds one on the way back. */
static inline void push_pc(zreg* const r)
{
    SET16(r[R_ECX], GET16(xs));
    push8(r, (u1)(xpc >> 8));
    push8(r, (u1)xpc);
}

#ifndef OPS_OWN_COp20
void OP(COp20)(zreg* const r) /* JSR a */
{
    r[R_EBX] = r[R_ESI] - (zreg)(uintptr_t)initaddrl;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 1));
    xpc = GET16(r[R_EBX]);
    push_pc(r);
    r[R_EAX] = 0;
    SET16(xs, GET16(r[R_ECX]));
    AX(r, rd16(r[R_ESI]));
    r[R_EBX] &= 0xFFFF00FFu;
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 1);
}
#endif

#ifndef OPS_OWN_COpFC
void OP(COpFC)(zreg* const r) /* JSR (a,x) */
{
    r[R_EBX] = r[R_ESI] - (zreg)(uintptr_t)initaddrl;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 1));
    xpc = GET16(r[R_EBX]);
    push_pc(r);
    r[R_EAX] = 0;
    SET16(xs, GET16(r[R_ECX]));
    r[R_EAX] = 0;
    r[R_EBX] &= 0xFFFF00FFu;
    SET16(r[R_ECX], rd16(r[R_ESI]));
    SET8(r[R_EBX], GET8(xpb));
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(xx)));
    TABR16(r);
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EBX], GET8(xpb));
    jump_to(r, 0);
}
#endif

#ifndef OPS_OWN_COp22
void OP(COp22)(zreg* const r) /* JSL al */
{
    r[R_EBX] = r[R_ESI] - (zreg)(uintptr_t)initaddrl;
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + 2));
    xpc = GET16(r[R_EBX]);
    SET16(r[R_ECX], GET16(xs));
    push8(r, GET8(xpb));
    push8(r, (u1)(xpc >> 8));
    push8(r, (u1)xpc);
    SET16(xs, GET16(r[R_ECX]));
    r[R_EAX] = 0;
    r[R_EBX] &= 0xFFFF00FFu;
    AX(r, rd16(r[R_ESI]));
    SET8(r[R_EBX], *(u1 const*)(uintptr_t)(r[R_ESI] + 2));
    xpc = GET16(r[R_EAX]);
    SET8(xpb, GET8(r[R_EBX]));
    jump_to(r, 0);
}
#endif

/* Block move. One byte per execution: the opcode backs esi up over itself and
   runs again until A underflows, so the loop lives in the dispatcher. */
static void block_move(zreg* const r, int const dir)
{
    AX(r, rd16(r[R_ESI]));
    SET8(xdb, GET8(r[R_EAX]));
    SET8(r[R_EBX], (u1)(GET16(r[R_EAX]) >> 8));
    SET16(r[R_ECX], GET16(xx));
    TABR8(r);
    SET8(r[R_EBX], GET8(xdb));
    SET16(r[R_ECX], GET16(xy));
    TABW8(r);
    if (r[R_EDX] & 0x10u) { /* 8-bit index: only the low byte steps */
        SET8(xx, (u1)(GET8(xx) + dir));
        SET8(xy, (u1)(GET8(xy) + dir));
    } else {
        SET16(xx, (u2)(GET16(xx) + dir));
        SET16(xy, (u2)(GET16(xy) + dir));
    }
    SET16(xa, (u2)(GET16(xa) - 1));
    if (GET16(xa) == 0xFFFFu)
        r[R_ESI] += 2;
    else
        r[R_ESI]--;
}

void OP(COp54)(zreg* const r) { block_move(r, +1); } /* MVN xyc */
void OP(COp44)(zreg* const r) { block_move(r, -1); } /* MVP xyc */

#ifndef OPS_OWN_COpCB
void OP(COpCB)(zreg* const r) /* WAI i */
{
    if (intrset == 1) {
        r[R_ESI]--;
        return;
    }
    if (intrset != 0) {
        if (intrset == 2) {
            intrset = 0;
            doirqnext = 0;
            return;
        }
        r[R_ESI]--;
        return;
    }
    intrset = 1;
    r[R_ESI]--;
}
#endif

void OP(COp89m8)(zreg* const r) /* BIT # - immediate does not touch N or V */
{
    AL(r, *(u1 const*)r[R_ESI]);
    if (flagnz & 0x18000u)
        flagnz |= 0x10000u;
    r[R_ESI]++;
    flagnz = (flagnz & 0xFFFF0000u) | ((GET8(xa) & GET8(r[R_EAX])) ? 1u : 0u);
}

void OP(COp89m16)(zreg* const r) /* BIT # */
{
    AX(r, rd16(r[R_ESI]));
    if (flagnz & 0x18000u)
        flagnz |= 0x10000u;
    r[R_ESI] += 2;
    flagnz = (flagnz & 0xFFFF0000u) | ((GET16(xa) & GET16(r[R_EAX])) ? 1u : 0u);
}

/*
 * BRK and COP push the return context straight into work RAM, not through the
 * memory tables, then vector through brkv/copv. Emulation mode pushes no bank
 * and uses the 8-bit vectors.
 */
static inline void brk_cop(zreg* const r, u2 const vec, u2 const vec8, u4 const setbits8)
{
    u1* ram;
    u2 sp;
    int const emul = (xe & 1) != 0;

    r[R_ESI]++;
    SET8(r[R_EBX], GET8(xpb));
    AX(r, xpc);
    r[R_EAX] = (zreg)(uintptr_t)((r[R_EAX] & 0x8000u) ? snesmmap[r[R_EBX]]
                                                    : snesmap2[r[R_EBX]]);
    r[R_EBX] = r[R_ESI] - r[R_EAX];
    xpc = GET16(r[R_EBX]);

    ram = wramdata;
    r[R_EBX] = 0;
    SET16(r[R_EBX], GET16(xs));
    sp = GET16(r[R_EBX]);
#define PUSHRAM(v)                                    \
    do {                                              \
        SET8(r[R_ECX], (v));                          \
        ram[sp] = GET8(r[R_ECX]);                     \
        sp = (u2)(((sp - 1) & stackand) | stackor);   \
    } while (0)
    if (!emul)
        PUSHRAM(GET8(xpb));
    PUSHRAM((u1)(xpc >> 8));
    PUSHRAM((u1)xpc);
#undef PUSHRAM
    /* The flags go straight out of dl - unlike the address bytes above, they do
       not pass through cl, so cl is left holding the low byte of the PC. */
    r[R_EDX] = makedl(r[R_EDX]);
    ram[sp] = (u1)r[R_EDX];
    sp = (u2)(((sp - 1) & stackand) | stackor);
    SET16(r[R_EBX], sp);
    SET16(xs, sp);
    r[R_EBX] &= 0xFFFF00FFu;

    SET8(r[R_EBX], emul ? GET8(xpb) : GET8(xirqb));
    if (!emul)
        SET8(xpb, GET8(r[R_EBX]));
    r[R_EAX] = 0;
    AX(r, emul ? vec8 : vec);
    xpc = GET16(r[R_EAX]);
    SET8(r[R_EDX], (u1)((r[R_EDX] & 0xF3u) | (emul ? setbits8 : 0x04u)));
    jump_to(r, 0);
}

#ifndef OPS_OWN_COp00
void OP(COp00)(zreg* const r) { brk_cop(r, brkv, brkv8, 0x0Cu); } /* BRK s */
#endif
#ifndef OPS_OWN_COp02
void OP(COp02)(zreg* const r) { brk_cop(r, copv, copv8, 0x04u); } /* COP s */

/*
 * RTI pulls P, then the return address, then re-enters there. The opcode table
 * is reloaded from the *pulled* flags, landing on a WAI ($CB) re-arms intrset,
 * and emulation mode returns to bank zero whatever was pushed.
 */
static inline void rti_body(zreg* const r)
{
    int const emul = (xe & 1) != 0;

    if (nmistatus == 3) {
        if (curexecstate & 0x01u)
            curexecstate &= 0xFEu;
        if (curexecstate == 0)
            r[R_EDX] &= 0xFFFF00FFu; /* xor dh,dh - the cycle count */
    }
    curnmi = 0;

    SET16(r[R_ECX], GET16(xs));
    SET8(r[R_EDX], pop8(r));
    SET16(xs, GET16(r[R_ECX]));
    if (emul)
        r[R_EDX] |= 0x30u;
    restoredl(r[R_EDX]);

    SET16(r[R_ECX], GET16(xs));
    r[R_EAX] = 0;
    xpc = (u2)((xpc & 0xFF00u) | pop8(r));
    r[R_EAX] = 0;
    xpc = (u2)((xpc & 0x00FFu) | (u2)pop8(r) << 8);
    if (!emul) {
        r[R_EAX] = 0;
        SET8(xpb, pop8(r));
    }
    SET16(xs, GET16(r[R_ECX]));

    r[R_EBX] &= 0xFFFF00FFu;
    r[R_EAX] = 0;
    AX(r, xpc);
    SET8(r[R_EBX], (u1)r[R_EDX]);
    r[R_EDI] = (zreg)(uintptr_t)tablead[r[R_EBX]];
    SET8(r[R_EBX], emul ? 0 : GET8(xpb));
    xpc = GET16(r[R_EAX]);

    if (emul) {
        jump_to(r, 0);
        return;
    }
    {
        u1* const base = bank_base(r[R_EAX], r[R_EBX], 1);
        int const low = (r[R_EAX] & 0x8000u) == 0;
        int const dma = low && r[R_EAX] >= 0x4300u;
        if (dma && memtabler8[r[R_EBX]] != regaccessbankr8)
            doirqnext = 0;
        initaddrl = base;
        r[R_ESI] = (zreg)(uintptr_t)base + r[R_EAX];
        /* Returning onto a WAI means the wait is still in force. */
        if (low && !(dma && memtabler8[r[R_EBX]] == regaccessbankr8)
            && *(u1 const*)r[R_ESI] == 0xCBu)
            intrset = 2;
    }
    if (r[R_EDX] & 0x10u) {
        xx &= 0xFFFF00FFu;
        xy &= 0xFFFF00FFu;
    }
}
#endif

#ifndef OPS_OWN_COp40
void OP(COp40)(zreg* const r) { rti_body(r); } /* RTI s */
#endif

#ifndef OPS_OWN_COp58
void OP(COp58)(zreg* const r) /* CLI i */
{
    r[R_EDX] &= ~0x04u;
    if (doirqnext == 0)
        return;
    doirqnext = 0;
    {
        zreg edx = r[R_EDX];
        zreg esi = r[R_ESI];
        switchtovirq(&edx, &esi);
        r[R_EDX] = edx;
        r[R_ESI] = esi;
    }
}
#endif

/*
 * The SA-1 and debug cores start their stack macros with `mov eax,[wramdata]`,
 * so that pointer's upper three bytes reach the handler's result; the 65816
 * itself does not. Hence macros here rather than in either instantiation.
 */
#define WRAM_PUSH8(name, src)                    \
    void OP(name)(zreg* const r)                  \
    {                                           \
        r[R_EAX] = (zreg)(uintptr_t)wramdata;     \
        SET16(r[R_ECX], GET16(xs));             \
        push8(r, GET8(src));                    \
        SET16(xs, GET16(r[R_ECX]));             \
    }
#define WRAM_POP8(name, dst)                     \
    void OP(name)(zreg* const r)                  \
    {                                           \
        u1 v;                                   \
        r[R_EAX] = (zreg)(uintptr_t)wramdata;     \
        SET16(r[R_ECX], GET16(xs));             \
        v = pop8(r);                            \
        SET16(xs, GET16(r[R_ECX]));             \
        SET8(dst, v);                           \
        setnz8(v);                              \
    }
#define WRAM_POP16(name, dst)                        \
    void OP(name)(zreg* const r)                      \
    {                                               \
        u1 hi;                                      \
        r[R_EAX] = (zreg)(uintptr_t)wramdata;         \
        SET16(r[R_ECX], GET16(xs));                 \
        SET8(dst, pop8(r));                         \
        SET16(xs, GET16(r[R_ECX]));                 \
        hi = pop8(r);                               \
        (dst) = ((dst) & 0xFFFF00FFu) | (u4)hi << 8; \
        SET16(xs, GET16(r[R_ECX]));                 \
        AX(r, (u2)((u2)hi << 8 | GET8(dst)));       \
        setnz16(r, GET16(r[R_EAX]));                \
    }

#endif /* OPS65816_H */
