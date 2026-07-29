/*
 * cpu/mem_ops.h - direct-page memory accessors ported from cpu/memory.asm.
 *
 * Textual include (cpu/c_memops.c): the includer provides the u1/u2/u4
 * typedefs and the seam block declared below.
 *
 * These are the Bank0dat* handlers, reached through DPageR8/DPageR16/DPageW8/
 * DPageW16 (cpu/memtable.c picks one per direct-page high byte). The assembly
 * calls them with:
 *
 *     ebx  the direct-page offset byte just fetched from the opcode stream
 *     ecx  the direct page register, xd
 *     eax  al/ax carries the value on a write, and takes it on a read
 *
 * and the caller keeps whatever the handler leaves in ebx and ecx, so those
 * are outputs too - the "inv" and "romram" ones deliberately advance ecx and
 * some zero ebx. cpu/memory.asm spills all three to the seam around the call
 * (the memcop macro), so a body just reads and writes MemSeam*.
 *
 * The reg variants call an I/O register handler, which still wants the legacy
 * ABI, so they go through the trampolines below; the includer must have
 * included chips/regabi.h for REGABI_ENTRY/REGABI_SYM. The chip and SA-1
 * variants need BWCheck and the DSP1 entry points and are still assembly.
 */
#ifndef MEM_OPS_H
#define MEM_OPS_H

/* wramdataa is the 64K WRAM window the assembly indexes as a flat array. */
static inline u1* mem_wram(u4 const off)
{
    return (u1*)wramdataa + off;
}

/* The ROM map base the 8000-FFFF handlers add to the address. The assembly
   writes `[snesmmap]`, i.e. entry 0; the per-bank entries belong to the
   regaccessbank* handlers, which are still assembly. */
static inline u1* mem_rom(void)
{
    return snesmmap[0];
}

/* Reads return in al/ax, leaving the rest of eax alone: the 65816 core keeps
   live values in the upper half. */
static inline void mem_set_al(u1 const v)
{
    MemSeamA = (MemSeamA & ~0xFFu) | v;
}

static inline void mem_set_ax(u2 const v)
{
    MemSeamA = (MemSeamA & ~0xFFFFu) | v;
}

/* `add cx,bx`: 16-bit add, so it wraps inside cx and leaves ecx's top half. */
static inline void mem_add_cx_bx(void)
{
    MemSeamC = (MemSeamC & ~0xFFFFu)
        | ((MemSeamC + MemSeamB) & 0xFFFFu);
}

/* Call one I/O register handler: address in ecx, value in al, and it keeps
   ecx and edx. Naked like the trampolines in chips/regabi.h - a constrained
   asm cannot promise to preserve everything a legacy handler may touch, and
   the tables are indexed regptra[addr - 0x2000], i.e. base - 0x8000 + ecx*4
   exactly as cpu/regs.mac writes it. */
#define MEM_REG_TRAMPOLINE(name, table)                                       \
    __asm__(REGABI_ENTRY(name) "pushl %ebx\n"                                 \
                               "pushl %esi\n"                                 \
                               "pushl %edi\n"                                 \
                               "movl " REGABI_SYM(MemSeamC) ", %ecx\n"        \
                               "movl " REGABI_SYM(MemSeamA) ", %eax\n"        \
                               "call *" REGABI_SYM(table) "-0x8000(,%ecx,4)\n" \
                               "movl %eax, " REGABI_SYM(MemSeamA) "\n"        \
                               "popl %edi\n"                                  \
                               "popl %esi\n"                                  \
                               "popl %ebx\n"                                  \
                               "ret\n");                                      \
    void name(void)

MEM_REG_TRAMPOLINE(MemRegRead, regptra);
MEM_REG_TRAMPOLINE(MemRegWrite, regptwa);

/* --- 8-bit reads --------------------------------------------------------- */

void c_membank0r8ram(void) /* 0000-1FFF */
{
    mem_set_al(*mem_wram(MemSeamB + MemSeamC));
}

/* Open bus: the value is the high byte of the address the access landed on,
   and the assembly folds ebx into ecx first, which the caller then sees. */
/* Every register read also latches the open-bus value. */
void c_membank0r8reg(void) /* 2000-48FF */
{
    MemSeamC += MemSeamB;
    MemRegRead();
    cpu_mdr = (u1)(MemSeamA & 0xFFu);
    MemSeamB = 0;
}

void c_membank0r8inv(void) /* 4800-5FFF */
{
    MemSeamC += MemSeamB;
    mem_set_al((u1)((MemSeamC >> 8) & 0xFFu));
}

void c_membank0r8rom(void) /* 8000-FFFF */
{
    MemSeamB += (u4)(uintptr_t)mem_rom();
    mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
    MemSeamB = 0;
}

void c_membank0r8romram(void) /* 0000-1FFF */
{
    mem_add_cx_bx();
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_rom();
        mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
        MemSeamB = 0;
    } else {
        mem_set_al(*mem_wram(MemSeamC));
    }
}

/* --- 16-bit reads -------------------------------------------------------- */

void c_membank0r16ram(void) /* 0000-1EFF */
{
    mem_set_ax((u2)(mem_wram(MemSeamB + MemSeamC)[0]
        | (mem_wram(MemSeamB + MemSeamC)[1] << 8)));
}

/* The last word of the page cannot straddle the boundary, so the high byte
   reads back as the low one rather than wrapping. */
void c_membank0r16ramh(void) /* 1F00-1FFF */
{
    MemSeamC += MemSeamB;
    if (MemSeamC == 0x1FFFu) {
        u1 const v = *mem_wram(MemSeamC);
        mem_set_ax((u2)(v | (v << 8)));
    } else {
        mem_set_ax((u2)(mem_wram(MemSeamC)[0] | (mem_wram(MemSeamC)[1] << 8)));
    }
}

/* Two byte reads, low then high. The assembly stashes the low byte in ah
   before the second call, so the second handler sees it there - reproduce that
   intermediate eax, not just the final one. cpu_mdr is written after each
   read, so it ends up holding the high byte. */
void c_membank0r16reg(void) /* 2000-48FF */
{
    u1 lo, hi;

    MemSeamC += MemSeamB;
    MemRegRead();
    lo = (u1)(MemSeamA & 0xFFu);
    cpu_mdr = lo;
    MemSeamA = (MemSeamA & ~0xFF00u) | ((u4)lo << 8);
    MemSeamC++;
    MemRegRead();
    hi = (u1)(MemSeamA & 0xFFu);
    cpu_mdr = hi;
    MemSeamC--;
    MemSeamA = (MemSeamA & ~0xFFFFu) | lo | ((u4)hi << 8);
    MemSeamB = 0;
}

void c_membank0r16rom(void) /* 8000-FFFF */
{
    MemSeamB += (u4)(uintptr_t)mem_rom();
    mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
        | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
    MemSeamB = 0;
}

void c_membank0r16romram(void) /* 0000-1FFF */
{
    mem_add_cx_bx();
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_rom();
        mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
            | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
        MemSeamB = 0;
    } else {
        mem_set_ax((u2)(mem_wram(MemSeamC)[0] | (mem_wram(MemSeamC)[1] << 8)));
    }
}

/* --- writes -------------------------------------------------------------- */

void c_membank0w8ram(void) /* 0000-1FFF */
{
    *mem_wram(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
}

void c_membank0w8reg(void) /* 2000-48FF */
{
    MemSeamC += MemSeamB;
    MemRegWrite();
    MemSeamB = 0;
}

/* The high byte is moved into al for the second write, and whatever the first
   handler left in eax carries into that move. */
void c_membank0w16reg(void) /* 2000-48FF */
{
    MemSeamC += MemSeamB;
    MemRegWrite();
    MemSeamA = (MemSeamA & ~0xFFu) | ((MemSeamA >> 8) & 0xFFu);
    MemSeamC++;
    MemRegWrite();
    MemSeamC--;
    MemSeamB = 0;
}

void c_membank0w8inv(void) /* 4800-5FFF */
{
}

void c_membank0w8rom(void) /* 8000-FFFF */
{
}

void c_membank0w8romram(void) /* 0000-1FFF */
{
    mem_add_cx_bx();
    if (!(MemSeamC & 0x8000u)) {
        *mem_wram(MemSeamC) = (u1)(MemSeamA & 0xFFu);
    }
}

void c_membank0w16ram(void) /* 0000-1EFF */
{
    mem_wram(MemSeamB + MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
    mem_wram(MemSeamB + MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
}

/* Same boundary case as the read: only the low byte lands. */
void c_membank0w16ramh(void) /* 1F00-1FFF */
{
    MemSeamC += MemSeamB;
    mem_wram(MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
    if (MemSeamC != 0x1FFFu) {
        mem_wram(MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
    }
}

void c_membank0w16inv(void) /* 4800-5FFF */
{
}

void c_membank0w16romram(void) /* 0000-1FFF */
{
    mem_add_cx_bx();
    if (!(MemSeamC & 0x8000u)) {
        mem_wram(MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
        mem_wram(MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
    }
}

#endif
