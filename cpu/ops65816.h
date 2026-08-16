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
static inline void bank0_call(u4* const r, void (*const fn)(void))
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
    bank0_call(r, c_membank0w8);
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
}

static inline u1 pop8(u4* const r)
{
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + 1) & stackand);
    bank0_call(r, c_membank0r8);
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
    bank0_call(r, c_membank0w8);
    r[R_EAX] = saved;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) - 1) | stackor);
    bank0_call(r, c_membank0w8);
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
    bank0_call(r, c_membank0r16);
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

/*
 * Addressing modes.
 *
 * Each one advances esi past its operand bytes and leaves the value in al or
 * ax. They reach memory through per-bank tables of routines that take the bank
 * in bl and the address in cx, so unlike the stack code there is no C half to
 * call directly - the table entries are the assembly thunks themselves.
 *
 * Two details are easy to lose. `add cx,bx` adds the whole of bx, not just the
 * operand byte in bl, which is only safe because the dispatcher keeps bh zero.
 * And after `add cx,<index>` a 16-bit carry steps the bank: that is the
 * page-crossing behaviour, not an optimisation to drop.
 */
static inline void mem_call(u4* const r, eop* const fn)
{
    u4 eax = r[R_EAX], ebx = r[R_EBX], ecx = r[R_ECX], edx = r[R_EDX];
    __asm__ volatile("call *%4"
                     : "+a"(eax), "+b"(ebx), "+c"(ecx), "+d"(edx)
                     : "rm"(fn)
                     : "cc", "memory");
    r[R_EAX] = eax;
    r[R_EBX] = ebx;
    r[R_ECX] = ecx;
    r[R_EDX] = edx;
}

#define TABR8(r) mem_call((r), memtabler8[(r)[R_EBX]])
#define TABR16(r) mem_call((r), memtabler16[(r)[R_EBX]])

/* `add cx,idx` / `jnc .np` / `inc bl` */
static inline void idx_bank(u4* const r, u2 const idx)
{
    u4 const sum = GET16(r[R_ECX]) + idx;
    SET16(r[R_ECX], (u2)sum);
    if (sum > 0xFFFFu)
        SET8(r[R_EBX], (u1)(GET8(r[R_EBX]) + 1));
}

/* The operand byte, and the direct page it indexes. */
static inline void dp_operand(u4* const r)
{
    SET8(r[R_EBX], *(u1 const*)(uintptr_t)r[R_ESI]);
    r[R_ECX] = xd;
    r[R_ESI]++;
}

/* [d],l: a 24-bit pointer read out of the direct page, low word then bank. */
static inline void long_indirect(u4* const r)
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
static void a_I_8(u4* const r)
{
    AL(r, *(u1 const*)(uintptr_t)r[R_ESI]);
    r[R_ESI]++;
}
static void a_I_16(u4* const r)
{
    r[R_EAX] = *(u4 const*)(uintptr_t)r[R_ESI];
    r[R_ESI] += 2;
}

/* a, a,x, a,y - absolute in the data bank. */
#define ABS(name, tab, idx)                                     \
    static void name(u4* const r)                               \
    {                                                           \
        SET16(r[R_ECX], *(u2 const*)(uintptr_t)r[R_ESI]);       \
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

/* al, al,x - absolute long, bank from the third operand byte. */
#define ABSL(name, tab, idx)                                    \
    static void name(u4* const r)                               \
    {                                                           \
        SET16(r[R_ECX], *(u2 const*)(uintptr_t)r[R_ESI]);       \
        SET8(r[R_EBX], *(u1 const*)(uintptr_t)(r[R_ESI] + 2));  \
        r[R_ESI] += 3;                                          \
        idx;                                                    \
        tab(r);                                                 \
    }
ABSL(a_al_8, TABR8, (void)0)
ABSL(a_al_16, TABR16, (void)0)
ABSL(a_alCx_8, TABR8, idx_bank(r, GET16(xx)))
ABSL(a_alCx_16, TABR16, idx_bank(r, GET16(xx)))

/* d - direct page, through the pointer the page's base selects. */
static void a_d_8(u4* const r)
{
    dp_operand(r);
    mem_call(r, DPageR8);
}
static void a_d_16(u4* const r)
{
    dp_operand(r);
    mem_call(r, DPageR16);
}

/* d,x and d,y wrap inside the bank rather than the page, so they go the long
   way round instead of through the direct-page pointer. */
#define DPIDX(name, idx)                                          \
    static void name(u4* const r)                                 \
    {                                                             \
        r[R_ECX] = xd;                                            \
        SET8(r[R_EBX], *(u1 const*)(uintptr_t)r[R_ESI]);          \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX]))); \
        r[R_ESI]++;                                               \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(idx)));      \
    }
DPIDX(dpidx_x, xx)
DPIDX(dpidx_y, xy)

static void a_dCx_8(u4* const r)
{
    dpidx_x(r);
    bank0_call(r, c_membank0r8);
}
static void a_dCx_16(u4* const r)
{
    dpidx_x(r);
    bank0_call(r, c_membank0r16);
}
static void a_dCy_8(u4* const r)
{
    dpidx_y(r);
    bank0_call(r, c_membank0r8);
}
static void a_dCy_16(u4* const r)
{
    dpidx_y(r);
    bank0_call(r, c_membank0r16);
}

/* d,s - stack relative. */
static inline void sp_rel(u4* const r)
{
    SET8(r[R_EBX], *(u1 const*)(uintptr_t)r[R_ESI]);
    SET16(r[R_ECX], GET16(xs));
    r[R_ESI]++;
    SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX])));
}
static void a_dCs_8(u4* const r)
{
    sp_rel(r);
    bank0_call(r, c_membank0r8);
}
static void a_dCs_16(u4* const r)
{
    sp_rel(r);
    bank0_call(r, c_membank0r16);
}

/* (d) and (d),y - a 16-bit pointer from the direct page, data bank. */
#define DIND(name, tab, idx)             \
    static void name(u4* const r)        \
    {                                    \
        dp_operand(r);                   \
        mem_call(r, DPageR16);           \
        SET16(r[R_ECX], GET16(r[R_EAX])); \
        SET8(r[R_EBX], GET8(xdb));       \
        idx;                             \
        tab(r);                          \
    }
DIND(a_BdB_8, TABR8, (void)0)
DIND(a_BdB_16, TABR16, (void)0)
DIND(a_BdBCy_8, TABR8, idx_bank(r, GET16(xy)))
DIND(a_BdBCy_16, TABR16, idx_bank(r, GET16(xy)))

/* (d,x) - the direct page is indexed before the pointer is read. */
#define DINDX(name, tab)                                          \
    static void name(u4* const r)                                 \
    {                                                             \
        r[R_ECX] = xd;                                            \
        SET8(r[R_EBX], *(u1 const*)(uintptr_t)r[R_ESI]);          \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(r[R_EBX]))); \
        r[R_ESI]++;                                               \
        SET16(r[R_ECX], (u2)(GET16(r[R_ECX]) + GET16(xx)));       \
        bank0_call(r, c_membank0r16);                                \
        SET16(r[R_ECX], GET16(r[R_EAX]));                         \
        SET8(r[R_EBX], GET8(xdb));                                \
        tab(r);                                                   \
    }
DINDX(a_BdCxB_8, TABR8)
DINDX(a_BdCxB_16, TABR16)

/* (d,s),y - stack relative, then indirect, then indexed. */
#define SIND(name, tab)                   \
    static void name(u4* const r)         \
    {                                     \
        sp_rel(r);                        \
        bank0_call(r, c_membank0r16);        \
        SET16(r[R_ECX], GET16(r[R_EAX])); \
        SET8(r[R_EBX], GET8(xdb));        \
        idx_bank(r, GET16(xy));           \
        tab(r);                           \
    }
SIND(a_BdCsBCy_8, TABR8)
SIND(a_BdCsBCy_16, TABR16)

/* [d] and [d],y - long indirect, bank from the pointer itself. */
#define LIND(name, tab, idx)                                      \
    static void name(u4* const r)                                 \
    {                                                             \
        SET8(r[R_EBX], *(u1 const*)(uintptr_t)r[R_ESI]);          \
        r[R_ECX] = xd;                                            \
        r[R_ESI]++;                                               \
        long_indirect(r);                                         \
        idx;                                                      \
        tab(r);                                                   \
    }
LIND(a_LdL_8, TABR8, (void)0)
LIND(a_LdL_16, TABR16, (void)0)

/* The ,y form reads its operand before the direct page, not after. */
#define LINDY(name, tab)                                          \
    static void name(u4* const r)                                 \
    {                                                             \
        r[R_ECX] = xd;                                            \
        SET8(r[R_EBX], *(u1 const*)(uintptr_t)r[R_ESI]);          \
        r[R_ESI]++;                                               \
        long_indirect(r);                                         \
        idx_bank(r, GET16(xy));                                   \
        tab(r);                                                   \
    }
LINDY(a_LdLCy_8, TABR8)
LINDY(a_LdLCy_16, TABR16)

/*
 * LDA over every addressing mode. The 8-bit form writes flagnz directly rather
 * than going through setnz8 - same result, and it is what the assembly does.
 */
#define LDA8(name, mode)               \
    void name(u4* const r)             \
    {                                  \
        mode(r);                       \
        flagnz = 0;                    \
        SET8(xa, GET8(r[R_EAX]));      \
        flagnz = (u4)GET8(r[R_EAX]) << 8; \
    }
#define LDA16(name, mode)              \
    void name(u4* const r)             \
    {                                  \
        mode(r);                       \
        SET16(xa, GET16(r[R_EAX]));    \
        setnz16(r, GET16(r[R_EAX]));   \
    }

LDA8(c_COpA9m8, a_I_8) /* LDA # */
LDA16(c_COpA9m16, a_I_16)
LDA8(c_COpADm8, a_a_8) /* LDA a */
LDA16(c_COpADm16, a_a_16)
LDA8(c_COpBDm8, a_aCx_8) /* LDA a,x */
LDA16(c_COpBDm16, a_aCx_16)
LDA8(c_COpB9m8, a_aCy_8) /* LDA a,y */
LDA16(c_COpB9m16, a_aCy_16)
LDA8(c_COpAFm8, a_al_8) /* LDA al */
LDA16(c_COpAFm16, a_al_16)
LDA8(c_COpBFm8, a_alCx_8) /* LDA al,x */
LDA16(c_COpBFm16, a_alCx_16)
LDA8(c_COpA5m8, a_d_8) /* LDA d */
LDA16(c_COpA5m16, a_d_16)
LDA8(c_COpB5m8, a_dCx_8) /* LDA d,x */
LDA16(c_COpB5m16, a_dCx_16)
LDA8(c_COpA3m8, a_dCs_8) /* LDA d,s */
LDA16(c_COpA3m16, a_dCs_16)
LDA8(c_COpB2m8, a_BdB_8) /* LDA (d) */
LDA16(c_COpB2m16, a_BdB_16)
LDA8(c_COpB1m8, a_BdBCy_8) /* LDA (d),y */
LDA16(c_COpB1m16, a_BdBCy_16)
LDA8(c_COpA1m8, a_BdCxB_8) /* LDA (d,x) */
LDA16(c_COpA1m16, a_BdCxB_16)
LDA8(c_COpB3m8, a_BdCsBCy_8) /* LDA (d,s),y */
LDA16(c_COpB3m16, a_BdCsBCy_16)
LDA8(c_COpA7m8, a_LdL_8) /* LDA [d] */
LDA16(c_COpA7m16, a_LdL_16)
LDA8(c_COpB7m8, a_LdLCy_8) /* LDA [d],y */
LDA16(c_COpB7m16, a_LdLCy_16)

/*
 * Operations. Each takes the value an addressing mode left in al or ax and is
 * composed with one by OPMODE below.
 *
 * The three logical operations are not written alike: ORA is a 16-bit `or ax`
 * but AND and EOR are 32-bit on eax, so those two carry the top half of the
 * register out of the handler changed and ORA does not.
 */
static void o_ORA8(u4* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) | GET8(xa)));
    flagnz = 0;
    SET8(xa, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_ORA16(u4* const r)
{
    AX(r, (u2)(GET16(r[R_EAX]) | GET16(xa)));
    SET16(xa, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}
static void o_AND8(u4* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) & GET8(xa)));
    flagnz = 0;
    SET8(xa, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_AND16(u4* const r)
{
    r[R_EAX] &= xa;
    SET16(xa, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}
static void o_EOR8(u4* const r)
{
    AL(r, (u1)(GET8(r[R_EAX]) ^ GET8(xa)));
    flagnz = 0;
    SET8(xa, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_EOR16(u4* const r)
{
    r[R_EAX] ^= xa;
    SET16(xa, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}

static void o_LDX8(u4* const r)
{
    flagnz = 0;
    SET8(xx, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_LDX16(u4* const r)
{
    SET16(xx, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}
static void o_LDY8(u4* const r)
{
    flagnz = 0;
    SET8(xy, GET8(r[R_EAX]));
    flagnz = (u4)GET8(r[R_EAX]) << 8;
}
static void o_LDY16(u4* const r)
{
    SET16(xy, GET16(r[R_EAX]));
    setnz16(r, GET16(r[R_EAX]));
}

/* The comparisons subtract into cl/cx and leave the result there. The x86 carry
   out of a subtract is a borrow, so C is its inverse; and the 16-bit form
   writes the whole of ecx into flagnz rather than zeroing it first. */
static inline void cmp8(u4* const r, u4 const reg)
{
    u4 const lhs = GET8(reg), rhs = GET8(r[R_EAX]);
    SET8(r[R_ECX], (u1)(lhs - rhs));
    flagnz = (u4)GET8(r[R_ECX]) << 8;
    flagc = lhs < rhs ? 0 : 0xFF;
}
static inline void cmp16(u4* const r, u4 const reg)
{
    u4 const lhs = GET16(reg), rhs = GET16(r[R_EAX]);
    SET16(r[R_ECX], (u2)(lhs - rhs));
    flagnz = r[R_ECX];
    flagc = lhs < rhs ? 0 : 0xFF;
}

static void o_CMP8(u4* const r) { cmp8(r, xa); }
static void o_CMP16(u4* const r) { cmp16(r, xa); }
static void o_CPX8(u4* const r) { cmp8(r, xx); }
static void o_CPX16(u4* const r) { cmp16(r, xx); }
static void o_CPY8(u4* const r) { cmp8(r, xy); }
static void o_CPY16(u4* const r) { cmp16(r, xy); }

/* BIT takes N and V straight from the operand's top two bits and Z from the
   test against A. The Z store is a word, so the N bit above it survives. */
static void o_BIT8(u4* const r)
{
    u1 const v = GET8(r[R_EAX]);
    flagnz = (v & 0x80u) ? 0x10000u : 0;
    flago = (v & 0x40u) ? 1u : 0;
    flagnz = (flagnz & 0xFFFF0000u) | ((GET8(xa) & v) ? 1u : 0u);
}
static void o_BIT16(u4* const r)
{
    u2 const v = GET16(r[R_EAX]);
    flagnz = (v & 0x8000u) ? 0x10000u : 0;
    flago = (v & 0x4000u) ? 1u : 0;
    flagnz = (flagnz & 0xFFFF0000u) | ((GET16(xa) & v) ? 1u : 0u);
}

/* An opcode is an addressing mode followed by an operation. */
#define OPMODE(name, mode, op) \
    void name(u4* const r)     \
    {                          \
        mode(r);               \
        op(r);                 \
    }

/* AND */
OPMODE(c_COp21m8, a_BdCxB_8, o_AND8)
OPMODE(c_COp21m16, a_BdCxB_16, o_AND16)
OPMODE(c_COp23m8, a_dCs_8, o_AND8)
OPMODE(c_COp23m16, a_dCs_16, o_AND16)
OPMODE(c_COp25m8, a_d_8, o_AND8)
OPMODE(c_COp25m16, a_d_16, o_AND16)
OPMODE(c_COp27m8, a_LdL_8, o_AND8)
OPMODE(c_COp27m16, a_LdL_16, o_AND16)
OPMODE(c_COp29m8, a_I_8, o_AND8)
OPMODE(c_COp29m16, a_I_16, o_AND16)
OPMODE(c_COp2Dm8, a_a_8, o_AND8)
OPMODE(c_COp2Dm16, a_a_16, o_AND16)
OPMODE(c_COp2Fm8, a_al_8, o_AND8)
OPMODE(c_COp2Fm16, a_al_16, o_AND16)
OPMODE(c_COp31m8, a_BdBCy_8, o_AND8)
OPMODE(c_COp31m16, a_BdBCy_16, o_AND16)
OPMODE(c_COp32m8, a_BdB_8, o_AND8)
OPMODE(c_COp32m16, a_BdB_16, o_AND16)
OPMODE(c_COp33m8, a_BdCsBCy_8, o_AND8)
OPMODE(c_COp33m16, a_BdCsBCy_16, o_AND16)
OPMODE(c_COp35m8, a_dCx_8, o_AND8)
OPMODE(c_COp35m16, a_dCx_16, o_AND16)
OPMODE(c_COp37m8, a_LdLCy_8, o_AND8)
OPMODE(c_COp37m16, a_LdLCy_16, o_AND16)
OPMODE(c_COp39m8, a_aCy_8, o_AND8)
OPMODE(c_COp39m16, a_aCy_16, o_AND16)
OPMODE(c_COp3Dm8, a_aCx_8, o_AND8)
OPMODE(c_COp3Dm16, a_aCx_16, o_AND16)
OPMODE(c_COp3Fm8, a_alCx_8, o_AND8)
OPMODE(c_COp3Fm16, a_alCx_16, o_AND16)

/* BIT */
OPMODE(c_COp24m8, a_d_8, o_BIT8)
OPMODE(c_COp24m16, a_d_16, o_BIT16)
OPMODE(c_COp2Cm8, a_a_8, o_BIT8)
OPMODE(c_COp2Cm16, a_a_16, o_BIT16)
OPMODE(c_COp34m8, a_dCx_8, o_BIT8)
OPMODE(c_COp34m16, a_dCx_16, o_BIT16)
OPMODE(c_COp3Cm8, a_aCx_8, o_BIT8)
OPMODE(c_COp3Cm16, a_aCx_16, o_BIT16)

/* CMP */
OPMODE(c_COpC1m8, a_BdCxB_8, o_CMP8)
OPMODE(c_COpC1m16, a_BdCxB_16, o_CMP16)
OPMODE(c_COpC3m8, a_dCs_8, o_CMP8)
OPMODE(c_COpC3m16, a_dCs_16, o_CMP16)
OPMODE(c_COpC5m8, a_d_8, o_CMP8)
OPMODE(c_COpC5m16, a_d_16, o_CMP16)
OPMODE(c_COpC7m8, a_LdL_8, o_CMP8)
OPMODE(c_COpC7m16, a_LdL_16, o_CMP16)
OPMODE(c_COpC9m8, a_I_8, o_CMP8)
OPMODE(c_COpC9m16, a_I_16, o_CMP16)
OPMODE(c_COpCDm8, a_a_8, o_CMP8)
OPMODE(c_COpCDm16, a_a_16, o_CMP16)
OPMODE(c_COpCFm8, a_al_8, o_CMP8)
OPMODE(c_COpCFm16, a_al_16, o_CMP16)
OPMODE(c_COpD1m8, a_BdBCy_8, o_CMP8)
OPMODE(c_COpD1m16, a_BdBCy_16, o_CMP16)
OPMODE(c_COpD2m8, a_BdB_8, o_CMP8)
OPMODE(c_COpD2m16, a_BdB_16, o_CMP16)
OPMODE(c_COpD3m8, a_BdCsBCy_8, o_CMP8)
OPMODE(c_COpD3m16, a_BdCsBCy_16, o_CMP16)
OPMODE(c_COpD5m8, a_dCx_8, o_CMP8)
OPMODE(c_COpD5m16, a_dCx_16, o_CMP16)
OPMODE(c_COpD7m8, a_LdLCy_8, o_CMP8)
OPMODE(c_COpD7m16, a_LdLCy_16, o_CMP16)
OPMODE(c_COpD9m8, a_aCy_8, o_CMP8)
OPMODE(c_COpD9m16, a_aCy_16, o_CMP16)
OPMODE(c_COpDDm8, a_aCx_8, o_CMP8)
OPMODE(c_COpDDm16, a_aCx_16, o_CMP16)
OPMODE(c_COpDFm8, a_alCx_8, o_CMP8)
OPMODE(c_COpDFm16, a_alCx_16, o_CMP16)

/* CPX */
OPMODE(c_COpE0x8, a_I_8, o_CPX8)
OPMODE(c_COpE0x16, a_I_16, o_CPX16)
OPMODE(c_COpE4x8, a_d_8, o_CPX8)
OPMODE(c_COpE4x16, a_d_16, o_CPX16)
OPMODE(c_COpECx8, a_a_8, o_CPX8)
OPMODE(c_COpECx16, a_a_16, o_CPX16)

/* CPY */
OPMODE(c_COpC0x8, a_I_8, o_CPY8)
OPMODE(c_COpC0x16, a_I_16, o_CPY16)
OPMODE(c_COpC4x8, a_d_8, o_CPY8)
OPMODE(c_COpC4x16, a_d_16, o_CPY16)
OPMODE(c_COpCCx8, a_a_8, o_CPY8)
OPMODE(c_COpCCx16, a_a_16, o_CPY16)

/* EOR */
OPMODE(c_COp41m8, a_BdCxB_8, o_EOR8)
OPMODE(c_COp41m16, a_BdCxB_16, o_EOR16)
OPMODE(c_COp43m8, a_dCs_8, o_EOR8)
OPMODE(c_COp43m16, a_dCs_16, o_EOR16)
OPMODE(c_COp45m8, a_d_8, o_EOR8)
OPMODE(c_COp45m16, a_d_16, o_EOR16)
OPMODE(c_COp47m8, a_LdL_8, o_EOR8)
OPMODE(c_COp47m16, a_LdL_16, o_EOR16)
OPMODE(c_COp49m8, a_I_8, o_EOR8)
OPMODE(c_COp49m16, a_I_16, o_EOR16)
OPMODE(c_COp4Dm8, a_a_8, o_EOR8)
OPMODE(c_COp4Dm16, a_a_16, o_EOR16)
OPMODE(c_COp4Fm8, a_al_8, o_EOR8)
OPMODE(c_COp4Fm16, a_al_16, o_EOR16)
OPMODE(c_COp51m8, a_BdBCy_8, o_EOR8)
OPMODE(c_COp51m16, a_BdBCy_16, o_EOR16)
OPMODE(c_COp52m8, a_BdB_8, o_EOR8)
OPMODE(c_COp52m16, a_BdB_16, o_EOR16)
OPMODE(c_COp53m8, a_BdCsBCy_8, o_EOR8)
OPMODE(c_COp53m16, a_BdCsBCy_16, o_EOR16)
OPMODE(c_COp55m8, a_dCx_8, o_EOR8)
OPMODE(c_COp55m16, a_dCx_16, o_EOR16)
OPMODE(c_COp57m8, a_LdLCy_8, o_EOR8)
OPMODE(c_COp57m16, a_LdLCy_16, o_EOR16)
OPMODE(c_COp59m8, a_aCy_8, o_EOR8)
OPMODE(c_COp59m16, a_aCy_16, o_EOR16)
OPMODE(c_COp5Dm8, a_aCx_8, o_EOR8)
OPMODE(c_COp5Dm16, a_aCx_16, o_EOR16)
OPMODE(c_COp5Fm8, a_alCx_8, o_EOR8)
OPMODE(c_COp5Fm16, a_alCx_16, o_EOR16)

/* LDX */
OPMODE(c_COpA2x8, a_I_8, o_LDX8)
OPMODE(c_COpA2x16, a_I_16, o_LDX16)
OPMODE(c_COpA6x8, a_d_8, o_LDX8)
OPMODE(c_COpA6x16, a_d_16, o_LDX16)
OPMODE(c_COpAEx8, a_a_8, o_LDX8)
OPMODE(c_COpAEx16, a_a_16, o_LDX16)
OPMODE(c_COpB6x8, a_dCy_8, o_LDX8)
OPMODE(c_COpB6x16, a_dCy_16, o_LDX16)
OPMODE(c_COpBEx8, a_aCy_8, o_LDX8)
OPMODE(c_COpBEx16, a_aCy_16, o_LDX16)

/* LDY */
OPMODE(c_COpA0x8, a_I_8, o_LDY8)
OPMODE(c_COpA0x16, a_I_16, o_LDY16)
OPMODE(c_COpA4x8, a_d_8, o_LDY8)
OPMODE(c_COpA4x16, a_d_16, o_LDY16)
OPMODE(c_COpACx8, a_a_8, o_LDY8)
OPMODE(c_COpACx16, a_a_16, o_LDY16)
OPMODE(c_COpB4x8, a_dCx_8, o_LDY8)
OPMODE(c_COpB4x16, a_dCx_16, o_LDY16)
OPMODE(c_COpBCx8, a_aCx_8, o_LDY8)
OPMODE(c_COpBCx16, a_aCx_16, o_LDY16)

/* ORA */
OPMODE(c_COp01m8, a_BdCxB_8, o_ORA8)
OPMODE(c_COp01m16, a_BdCxB_16, o_ORA16)
OPMODE(c_COp03m8, a_dCs_8, o_ORA8)
OPMODE(c_COp03m16, a_dCs_16, o_ORA16)
OPMODE(c_COp05m8, a_d_8, o_ORA8)
OPMODE(c_COp05m16, a_d_16, o_ORA16)
OPMODE(c_COp07m8, a_LdL_8, o_ORA8)
OPMODE(c_COp07m16, a_LdL_16, o_ORA16)
OPMODE(c_COp09m8, a_I_8, o_ORA8)
OPMODE(c_COp09m16, a_I_16, o_ORA16)
OPMODE(c_COp0Dm8, a_a_8, o_ORA8)
OPMODE(c_COp0Dm16, a_a_16, o_ORA16)
OPMODE(c_COp0Fm8, a_al_8, o_ORA8)
OPMODE(c_COp0Fm16, a_al_16, o_ORA16)
OPMODE(c_COp11m8, a_BdBCy_8, o_ORA8)
OPMODE(c_COp11m16, a_BdBCy_16, o_ORA16)
OPMODE(c_COp12m8, a_BdB_8, o_ORA8)
OPMODE(c_COp12m16, a_BdB_16, o_ORA16)
OPMODE(c_COp13m8, a_BdCsBCy_8, o_ORA8)
OPMODE(c_COp13m16, a_BdCsBCy_16, o_ORA16)
OPMODE(c_COp15m8, a_dCx_8, o_ORA8)
OPMODE(c_COp15m16, a_dCx_16, o_ORA16)
OPMODE(c_COp17m8, a_LdLCy_8, o_ORA8)
OPMODE(c_COp17m16, a_LdLCy_16, o_ORA16)
OPMODE(c_COp19m8, a_aCy_8, o_ORA8)
OPMODE(c_COp19m16, a_aCy_16, o_ORA16)
OPMODE(c_COp1Dm8, a_aCx_8, o_ORA8)
OPMODE(c_COp1Dm16, a_aCx_16, o_ORA16)
OPMODE(c_COp1Fm8, a_alCx_8, o_ORA8)
OPMODE(c_COp1Fm16, a_alCx_16, o_ORA16)

#endif /* OPS65816_H */
