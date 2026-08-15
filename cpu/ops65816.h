/*
 * cpu/ops65816.h - 65816 opcode handlers ported from cpu/e65816.inc.
 *
 * Textual include (cpu/c_65816.c): the includer provides the u1/u2/u4/s1
 * typedefs and the register file.
 *
 * The core runs with its state in x86 registers, so a ported handler takes the
 * whole register file: cpu/e65816.inc keeps the COpXX entry point but reduces
 * its body to the `cop` thunk, which pushads and hands the block over. Layout
 * is pushad's, so R_* below indexes it directly. Opcodes migrate one at a time.
 *
 * Of the file, esi is the 65816 program counter, dl the processor flags, dh the
 * remaining cycles, edi the opcode table for the current M/X widths and ebp the
 * SPC700 program counter. eax/ebx/ecx are scratch, but not dead: ecx's upper
 * half reaches flagnz through `mov cx,ax` (see setnz16), and ebx is the opcode
 * index the dispatcher reloads a byte at a time, so its upper bits stay zero.
 *
 * A/X/Y/S/D are 32-bit globals holding 8- or 16-bit registers, and the assembly
 * writes them at the width the current mode selects. Preserving the bytes above
 * that width is part of the behaviour - 8-bit mode leaves the high half of X
 * intact, and a later REP can bring it back - so every store here masks.
 */
#ifndef OPS65816_H
#define OPS65816_H

#include "flags65816.h"

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
static inline void setnz16(u4* const r, u2 const ax)
{
    r[R_ECX] = (r[R_ECX] & 0xFFFF0000u) | ax;
    flagnz = r[R_ECX];
}

/* dl selects the opcode table, so clearing or setting a width flag reloads it.
   The index is the whole of ebx, as in the assembly; it stays inside tablead
   because the dispatcher only ever loads bl and so keeps the upper bits zero. */
static inline void reload_table(u4* const r)
{
    r[R_EBX] = (r[R_EBX] & 0xFFFFFF00u) | (u1)r[R_EDX];
    r[R_EDI] = (u4)(uintptr_t)tablead[r[R_EBX]];
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
    void name(u4* const r)                 \
    {                                      \
        AL(r, GET8(src));                  \
        SET8(dst, GET8(r[R_EAX]));         \
        setnz8(GET8(r[R_EAX]));            \
    }
#define TRANSFER16(name, src, dst)         \
    void name(u4* const r)                 \
    {                                      \
        AX(r, GET16(src));                 \
        SET16(dst, GET16(r[R_EAX]));       \
        setnz16(r, GET16(r[R_EAX]));       \
    }

/* Increments and decrements of X and Y work in place, then reload al/ax. */
#define INCDEC8(name, reg, op)             \
    void name(u4* const r)                 \
    {                                      \
        SET8(reg, GET8(reg) op 1);         \
        AL(r, GET8(reg));                  \
        setnz8(GET8(r[R_EAX]));            \
    }
#define INCDEC16(name, reg, op)            \
    void name(u4* const r)                 \
    {                                      \
        SET16(reg, GET16(reg) op 1);       \
        AX(r, GET16(reg));                 \
        setnz16(r, GET16(r[R_EAX]));       \
    }

/*
 * Branches. The displacement is only fetched when the branch is taken, so eax
 * keeps its old value on the untaken path; the PC advances past the operand
 * either way. Conditions read the flags in their split form - see
 * flags65816.h - and are written here the way round the assembly tests them.
 */
#define BRANCH(name, taken)                                \
    void name(u4* const r)                                 \
    {                                                      \
        if (taken) {                                       \
            s4 const rel = *(s1 const*)(uintptr_t)r[R_ESI]; \
            r[R_EAX] = (u4)rel;                            \
            r[R_ESI] += (u4)rel;                           \
        }                                                  \
        r[R_ESI]++;                                        \
    }

BRANCH(c_COp80, 1) /* BRA r */
BRANCH(c_COp90, (flagc & 0x01u) == 0) /* BCC r */
BRANCH(c_COpB0, (flagc & 0x01u) != 0) /* BCS r */
BRANCH(c_COpF0, (flagnz & 0xFFFFu) == 0) /* BEQ r */
BRANCH(c_COpD0, (flagnz & 0xFFFFu) != 0) /* BNE r */
BRANCH(c_COp30, (flagnz & 0x18000u) != 0) /* BMI r */
BRANCH(c_COp10, (flagnz & 0x18000u) == 0) /* BPL r */
BRANCH(c_COp50, (flago & 0xFFu) == 0) /* BVC r */
BRANCH(c_COp70, (flago & 0xFFu) != 0) /* BVS r */

void c_COp18(u4* const r) /* CLC i */
{
    (void)r;
    flagc = 0;
}

void c_COp38(u4* const r) /* SEC i */
{
    (void)r;
    flagc = 0xFF;
}

void c_COpB8(u4* const r) /* CLV i */
{
    (void)r;
    flago = 0;
}

void c_COpD8(u4* const r) /* CLD i */
{
    r[R_EDX] &= ~0x08u;
    reload_table(r);
}

void c_COpF8(u4* const r) /* SED i */
{
    r[R_EDX] |= 0x08u;
    reload_table(r);
}

void c_COp78(u4* const r) /* SEI i */
{
    r[R_EDX] |= 0x04u;
}

void c_COpEA(u4* const r) /* NOP i */
{
    (void)r;
}

void c_COpDB(u4* const r) /* STP i */
{
    r[R_ESI]--;
}

void c_COp42(u4* const r) /* WDM */
{
    r[R_ESI]++;
}

INCDEC8(c_COpCAx8, xx, -) /* DEX i */
INCDEC16(c_COpCAx16, xx, -)
INCDEC8(c_COpE8x8, xx, +) /* INX i */
INCDEC16(c_COpE8x16, xx, +)
INCDEC8(c_COp88x8, xy, -) /* DEY i */
INCDEC16(c_COp88x16, xy, -)
INCDEC8(c_COpC8x8, xy, +) /* INY i */
INCDEC16(c_COpC8x16, xy, +)

TRANSFER8(c_COpAAx8, xa, xx) /* TAX i */
TRANSFER16(c_COpAAx16, xa, xx)
TRANSFER8(c_COpA8x8, xa, xy) /* TAY i */
TRANSFER16(c_COpA8x16, xa, xy)
TRANSFER8(c_COpBAx8, xs, xx) /* TSX i */
TRANSFER16(c_COpBAx16, xs, xx)
TRANSFER8(c_COp8Am8, xx, xa) /* TXA i */
TRANSFER16(c_COp8Am16, xx, xa)
TRANSFER8(c_COp98m8, xy, xa) /* TYA i */
TRANSFER16(c_COp98m16, xy, xa)
TRANSFER8(c_COp9Bx8, xx, xy) /* TXY i */
TRANSFER16(c_COp9Bx16, xx, xy)
TRANSFER8(c_COpBBx8, xy, xx) /* TYX i */
TRANSFER16(c_COpBBx16, xy, xx)

/* TDC and TSC are 16-bit whatever the M flag says. */
TRANSFER16(c_COp7B, xd, xa) /* TDC i */
TRANSFER16(c_COp3B, xs, xa) /* TSC i */

void c_COp1B(u4* const r) /* TCS i */
{
    AX(r, GET16(xa));
    if (xe & 1) {
        SET8(xs, GET8(r[R_EAX])); /* emulation mode keeps S in page one */
    } else {
        SET16(xs, GET16(r[R_EAX]));
    }
}

void c_COp9A(u4* const r) /* TXS i */
{
    AX(r, GET16(xx));
    SET16(xs, GET16(r[R_EAX]));
    if (xe & 1) {
        xs = (xs & 0xFFFF00FFu) | 0x0100u;
    }
}

void c_COpEB(u4* const r) /* XBA i */
{
    AX(r, (u2)((GET8(xa) << 8) | ((xa >> 8) & 0xFFu)));
    SET16(xa, GET16(r[R_EAX]));
    setnz8(GET8(r[R_EAX]));
}

/* INC A and DEC A go through the accumulator "addressing mode", which is just
   a load and a store of A at the current width. */
#define INCDECA8(name, op)                 \
    void name(u4* const r)                 \
    {                                      \
        AL(r, GET8(xa));                   \
        AL(r, GET8(r[R_EAX]) op 1);        \
        setnz8(GET8(r[R_EAX]));            \
        SET8(xa, GET8(r[R_EAX]));          \
    }
#define INCDECA16(name, op)                \
    void name(u4* const r)                 \
    {                                      \
        AX(r, GET16(xa));                  \
        AX(r, GET16(r[R_EAX]) op 1);       \
        setnz16(r, GET16(r[R_EAX]));       \
        SET16(xa, GET16(r[R_EAX]));        \
    }

INCDECA8(c_COp1Am8, +) /* INC A */
INCDECA16(c_COp1Am16, +)
INCDECA8(c_COp3Am8, -) /* DEC A */
INCDECA16(c_COp3Am16, -)

void c_COp5B(u4* const r) /* TCD i */
{
    AX(r, GET16(xa));
    SET16(xd, GET16(r[R_EAX]));
    UpdateDPage(); /* the direct page moved; the access tables follow it */
    setnz16(r, GET16(r[R_EAX]));
}

/*
 * REP and SEP. Both clear or set P bits named by an immediate. When only M, X
 * and D are involved the split flags are untouched and dl can be edited in
 * place; a bit outside those has to go the long way round through P, hence the
 * join / edit / split. Only REP re-forces the emulation-mode bits, and only SEP
 * narrows X and Y - the assembly is asymmetric here and the port keeps it.
 */
void c_COpC2(u4* const r) /* REP # */
{
    u1 const imm = *(u1 const*)(uintptr_t)r[R_ESI];
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

void c_COpE2(u4* const r) /* SEP # */
{
    u1 const imm = *(u1 const*)(uintptr_t)r[R_ESI];
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

void c_COpFB(u4* const r) /* XCE i */
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

/*
 * Stack operations.
 *
 * The core reaches memory through cpu/memory.asm's `memcop` thunk, which spills
 * eax/ebx/ecx/edx to MemSeam* around a call into C. Calling that C directly
 * from here has the same effect and skips the trip through assembly - but it
 * means all four registers come back possibly changed, which is why cx is
 * always re-read after an access rather than kept in a local.
 *
 * S wraps inside a page in emulation mode and across the bank in native mode;
 * stackor / stackand carry that, and XCE sets them.
 */
static inline void membank(u4* const r, void (*const fn)(void))
{
    MemSeamA = r[R_EAX];
    MemSeamB = r[R_EBX];
    MemSeamC = r[R_ECX];
    MemSeamD = r[R_EDX];
    fn();
    r[R_EAX] = MemSeamA;
    r[R_EBX] = MemSeamB;
    r[R_ECX] = MemSeamC;
    r[R_EDX] = MemSeamD;
}

static inline void push8(u4* const r, u1 const al)
{
    AL(r, al);
    membank(r, c_membank0w8);
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
}

static inline u1 pop8(u4* const r)
{
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + 1) & stackand);
    membank(r, c_membank0r8);
    return GET8(r[R_EAX]);
}

#define PUSH8(name, src)                   \
    void name(u4* const r)                 \
    {                                      \
        SET16(r[R_ECX], GET16(xs));        \
        push8(r, GET8(src));               \
        SET16(xs, GET16(r[R_ECX]));        \
    }
#define PUSH16(name, src)                  \
    void name(u4* const r)                 \
    {                                      \
        SET16(r[R_ECX], GET16(xs));        \
        push8(r, (u1)((src) >> 8));        \
        push8(r, GET8(src));               \
        SET16(xs, GET16(r[R_ECX]));        \
    }

PUSH8(c_COp48m8, xa) /* PHA s */
PUSH16(c_COp48m16, xa)
PUSH8(c_COp8B, xdb) /* PHB s */
PUSH16(c_COp0B, xd) /* PHD s */
PUSH8(c_COp4B, xpb) /* PHK s */
PUSH8(c_COpDAx8, xx) /* PHX s */
PUSH16(c_COpDAx16, xx)
PUSH8(c_COp5Ax8, xy) /* PHY s */
PUSH16(c_COp5Ax16, xy)

void c_COp08(u4* const r) /* PHP s */
{
    r[R_EDX] = makedl(r[R_EDX]);
    SET16(r[R_ECX], GET16(xs));
    push8(r, (u1)r[R_EDX]);
    SET16(xs, GET16(r[R_ECX]));
}

#define POP8(name, dst)                    \
    void name(u4* const r)                 \
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
    void name(u4* const r)                                   \
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

POP8(c_COp68m8, xa) /* PLA s */
POP16(c_COp68m16, xa)
POP8(c_COpAB, xdb) /* PLB s */
POP8(c_COpFAx8, xx) /* PLX s */
POP16(c_COpFAx16, xx)
POP8(c_COp7Ax8, xy) /* PLY s */
POP16(c_COp7Ax16, xy)

void c_COp2B(u4* const r) /* PLD s */
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

void c_COp28(u4* const r) /* PLP s */
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

void c_COpF4(u4* const r) /* PEA s */
{
    u1 const* const p = (u1 const*)(uintptr_t)r[R_ESI];
    SET16(r[R_ECX], GET16(xs));
    push8(r, p[1]);
    push8(r, p[0]);
    SET16(xs, GET16(r[R_ECX]));
    r[R_ESI] += 2;
}

/* PEI and PER both build a 16-bit value in ax and push it high byte first,
   saving eax around the first write because the access clobbers it. */
static void push16_ax(u4* const r)
{
    u4 const saved = r[R_EAX];
    SET16(r[R_ECX], GET16(xs));
    AL(r, (u1)(GET16(r[R_EAX]) >> 8));
    membank(r, c_membank0w8);
    r[R_EAX] = saved;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
    membank(r, c_membank0w8);
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
    SET16(xs, GET16(r[R_ECX]));
}

void c_COpD4(u4* const r) /* PEI s */
{
    r[R_EAX] &= 0xFFFF00FFu; /* xor ah,ah */
    AL(r, *(u1 const*)(uintptr_t)r[R_ESI]);
    SET16(r[R_ECX], GET16(xd));
    r[R_ESI]++;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EAX])));
    membank(r, c_membank0r16);
    push16_ax(r);
}

void c_COp62(u4* const r) /* PER s */
{
    /* The operand is relative to the 65816 PC, but esi is a host pointer, so
       the bank's base has to come back out of the memory map to recover it.
       Which map depends on where in the bank the PC currently is. */
    u1* const* map;

    SET8(r[R_EBX], GET8(xpb));
    AX(r, xpc);
    map = (r[R_EAX] & 0x8000u) ? snesmmap : snesmap2;
    r[R_EAX] = (u4)(uintptr_t)map[r[R_EBX]];
    r[R_EBX] = r[R_ESI] - r[R_EAX];
    SET16(r[R_EBX], (u2)(GET16(r[R_EBX]) + *(u2 const*)(uintptr_t)r[R_ESI]));
    AX(r, GET16(r[R_EBX]));
    r[R_ESI] += 2;
    AX(r, (u2)(GET16(r[R_EAX]) + 2));
    push16_ax(r);
    r[R_EBX] = 0;
}

#endif /* OPS65816_H */
