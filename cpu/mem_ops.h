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
 * included chips/regabi.h for REGABI_ENTRY/REGABI_SYM.
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
   exactly as cpu/regs.mac writes it.

   The seam is restored around the call. A register write can start a DMA, and
   the transfer runs through these same handlers, so without this the nested
   access would overwrite the outer one's address - the assembly had no such
   problem because it kept the address in ecx, which the callee preserves. */
#define MEM_REG_TRAMPOLINE(name, table)                                       \
    __asm__(REGABI_ENTRY(name) "pushl %ebx\n"                                 \
                               "pushl %esi\n"                                 \
                               "pushl %edi\n"                                 \
                               "movl " REGABI_SYM(MemSeamC) ", %ecx\n"        \
                               "movl " REGABI_SYM(MemSeamA) ", %eax\n"        \
                               "movl " REGABI_SYM(MemSeamD) ", %edx\n"        \
                               "pushl " REGABI_SYM(MemSeamB) "\n"            \
                               "pushl %ecx\n"                                 \
                               "call *" REGABI_SYM(table) "-0x8000(,%ecx,4)\n" \
                               "movl %eax, " REGABI_SYM(MemSeamA) "\n"        \
                               "movl %edx, " REGABI_SYM(MemSeamD) "\n"        \
                               "popl %ecx\n"                                  \
                               "movl %ecx, " REGABI_SYM(MemSeamC) "\n"        \
                               "popl %ecx\n"                                  \
                               "movl %ecx, " REGABI_SYM(MemSeamB) "\n"        \
                               "popl %edi\n"                                  \
                               "popl %esi\n"                                  \
                               "popl %ebx\n"                                  \
                               "ret\n");                                      \
    void name(void)

/* Call a handler that is still assembly, with the register ABI live. Lets a
   ported handler keep a tail-call into one that has not moved yet; the seam
   goes in and comes back out, so the caller sees exactly what the assembly
   would have left in the registers. */
__asm__(REGABI_ENTRY(MemCallAsm) "pushl %ebx\n"
                                 "pushl %esi\n"
                                 "pushl %edi\n"
                                 "movl 16(%esp), %eax\n"
                                 "pushl %eax\n"
                                 "movl " REGABI_SYM(MemSeamB) ", %ebx\n"
                                 "movl " REGABI_SYM(MemSeamC) ", %ecx\n"
                                 "movl " REGABI_SYM(MemSeamD) ", %edx\n"
                                 "movl " REGABI_SYM(MemSeamA) ", %eax\n"
                                 "call *(%esp)\n"
                                 "movl %eax, " REGABI_SYM(MemSeamA) "\n"
                                 "addl $4, %esp\n"
                                 "movl %ebx, " REGABI_SYM(MemSeamB) "\n"
                                 "movl %ecx, " REGABI_SYM(MemSeamC) "\n"
                                 "movl %edx, " REGABI_SYM(MemSeamD) "\n"
                                 "popl %edi\n"
                                 "popl %esi\n"
                                 "popl %ebx\n"
                                 "ret\n");
void MemCallAsm(void* fn);

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
static inline void mem_reg_read16(void)
{
    u1 lo, hi;

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
}

void c_membank0r16reg(void) /* 2000-48FF */
{
    MemSeamC += MemSeamB;
    mem_reg_read16();
    MemSeamB = 0;
}

/* Open bus, but the 16-bit read hands back a fixed 8080h rather than the
   address high byte - the two moves that build one in ax are overwritten
   before the return. */
void c_membank0r16inv(void) /* 4800-5FFF */
{
    MemSeamC += MemSeamB;
    mem_set_ax(0x8080u);
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
static inline void mem_reg_write16(void)
{
    MemRegWrite();
    MemSeamA = (MemSeamA & ~0xFFu) | ((MemSeamA >> 8) & 0xFFu);
    MemSeamC++;
    MemRegWrite();
    MemSeamC--;
}

void c_membank0w16reg(void) /* 2000-48FF */
{
    MemSeamC += MemSeamB;
    mem_reg_write16();
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

/* --- whole-bank ROM access ----------------------------------------------- */

/* Here ebx selects a ROM map entry rather than being an offset into one. The
   index is deliberately unmasked: the assembly writes [snesmmap+ebx*4] and
   some callers leave more than a bank number in ebx, so masking it to 8 bits
   silently reads the wrong entry. */
static inline u1* mem_bank(void)
{
    return snesmmap[MemSeamB];
}

void c_memaccessbankr8(void)
{
    MemSeamB = (u4)(uintptr_t)mem_bank();
    mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
    MemSeamB = 0;
}

void c_memaccessbankr16(void)
{
    MemSeamB = (u4)(uintptr_t)mem_bank();
    mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
        | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
    MemSeamB = 0;
}

/* ROM is read-only unless the user turned patching on, and then ebx is left
   alone - the early return does not clear it. */
void c_memaccessbankw8(void)
{
    if (!writeon) {
        return;
    }
    MemSeamB = (u4)(uintptr_t)mem_bank();
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    MemSeamB = 0;
}

void c_memaccessbankw16(void)
{
    if (!writeon) {
        return;
    }
    MemSeamB = (u4)(uintptr_t)mem_bank();
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) = (u1)((MemSeamA >> 8) & 0xFFu);
    MemSeamB = 0;
}

/* --- WRAM and expansion-RAM banks ---------------------------------------- */

/* Banks 7E and 7F: a flat move, with ebx left exactly as the caller set it. */
void c_wramaccessbankr8(void)
{
    mem_set_al(*mem_wram(MemSeamC));
}

void c_wramaccessbankr16(void)
{
    mem_set_ax((u2)(mem_wram(MemSeamC)[0] | (mem_wram(MemSeamC)[1] << 8)));
}

void c_wramaccessbankw8(void)
{
    *mem_wram(MemSeamC) = (u1)(MemSeamA & 0xFFu);
}

void c_wramaccessbankw16(void)
{
    mem_wram(MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
    mem_wram(MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
}

void c_eramaccessbankr8(void)
{
    mem_set_al(ram7fa[MemSeamC]);
}

void c_eramaccessbankr16(void)
{
    mem_set_ax((u2)(ram7fa[MemSeamC] | (ram7fa[MemSeamC + 1] << 8)));
}

void c_eramaccessbankw8(void)
{
    ram7fa[MemSeamC] = (u1)(MemSeamA & 0xFFu);
}

void c_eramaccessbankw16(void)
{
    ram7fa[MemSeamC] = (u1)(MemSeamA & 0xFFu);
    ram7fa[MemSeamC + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
}

/* --- cartridge SRAM ------------------------------------------------------ */

/* Every SRAM access masks with ramsizeand, so a cart smaller than the window
   mirrors; a cart with no SRAM at all reads back zero. Writes arm the
   save-to-disk countdown. */
void c_sramaccessbankr8b(void)
{
    if (ramsize == 0) {
        mem_set_al(0);
        MemSeamB = 0;
        return;
    }
    mem_set_al(sram[MemSeamC & ramsizeand]);
    MemSeamB = 0;
}

/* The second byte is masked again after the increment, so a read at the top of
   the cart wraps to its start rather than running past it. */
void c_sramaccessbankr16b(void)
{
    u4 a;

    if (ramsize == 0) {
        mem_set_ax(0);
        MemSeamB = 0;
        return;
    }
    a = MemSeamC & ramsizeand;
    mem_set_ax((u2)(sram[a] | (sram[(a + 1) & ramsizeand] << 8)));
    MemSeamB = 0;
}

void c_sramaccessbankw8b(void)
{
    if (ramsize != 0) {
        sram[MemSeamC & ramsizeand] = (u1)(MemSeamA & 0xFFu);
        sramb4save = 5 * 60;
    }
    MemSeamB = 0;
}

void c_sramaccessbankw16b(void)
{
    if (ramsize != 0) {
        u4 const a = MemSeamC & ramsizeand;

        sram[a] = (u1)(MemSeamA & 0xFFu);
        sram[(a + 1) & ramsizeand] = (u1)((MemSeamA >> 8) & 0xFFu);
        sramb4save = 5 * 60;
    }
    MemSeamB = 0;
}

/* Banks 78-7D map SRAM in 32K slices: turn the bank into a slice offset, run
   the access, then put the caller's address back. `sub bl,78h` is a byte
   subtract, so it wraps inside bl rather than borrowing. Kept faithful even
   though nothing can currently observe it: after the shift the difference sits
   above bit 22, and every SRAM access masks with ramsizeand, which a 128K
   cartridge never takes past 0x1FFFF. */
static inline void mem_sram_slice(u4 const base, void (*body)(void))
{
    u4 const saved = MemSeamC;

    MemSeamB = (MemSeamB & ~0xFFu) | ((MemSeamB - base) & 0xFFu);
    MemSeamB <<= 15;
    MemSeamC += MemSeamB;
    body();
    MemSeamC = saved;
}

/* Banks 70-7D. `and bl,7Fh` masks the low byte only, so anything the caller
   left above it survives into the shift. Like the 78-7D form above, none of
   this is observable today: initc.c caps ramsize at 0x20000, so ramsizeand
   never reaches past bit 16, while the bank only contributes bits 18 and up.
   Kept faithful anyway. */
static inline void mem_sram_bank70(void (*body)(void))
{
    MemSeamB &= ~0x80u;
    mem_sram_slice(0x70, body);
}

/* On a cart with more than 2Mb of ROM or more than 32K of SRAM the top half of
   these banks is ROM, not the SRAM mirror. */
static inline int mem_sram_large(void)
{
    return curromspace > 0x200000u || ramsize > 0x8000u;
}

void c_sramaccessbankr8(void)
{
    if (mem_sram_large() && (MemSeamC & 0x8000u)) {
        c_memaccessbankr8();
        return;
    }
    mem_sram_bank70(c_sramaccessbankr8b);
}

void c_sramaccessbankr16(void)
{
    if (mem_sram_large() && (MemSeamC & 0x8000u)) {
        c_memaccessbankr16();
        return;
    }
    mem_sram_bank70(c_sramaccessbankr16b);
}

void c_sramaccessbankw8(void)
{
    if (mem_sram_large() && (MemSeamC & 0x8000u)) {
        c_memaccessbankw8();
        return;
    }
    mem_sram_bank70(c_sramaccessbankw8b);
}

void c_sramaccessbankw16(void)
{
    if (mem_sram_large() && (MemSeamC & 0x8000u)) {
        c_memaccessbankw16();
        return;
    }
    mem_sram_bank70(c_sramaccessbankw16b);
}

void c_sramaccessbankr8s(void) { mem_sram_slice(0x78, c_sramaccessbankr8b); }
void c_sramaccessbankr16s(void) { mem_sram_slice(0x78, c_sramaccessbankr16b); }
void c_sramaccessbankw8s(void) { mem_sram_slice(0x78, c_sramaccessbankw8b); }
void c_sramaccessbankw16s(void) { mem_sram_slice(0x78, c_sramaccessbankw16b); }

/* --- Sufami Turbo / S-RTC style SRAM banks ------------------------------- */

/* These sit at 60-6F and 70-7F with ROM in the low half of each bank: a clear
   bit 15 tail-jumps to the plain ROM accessor instead. Unlike
   sramaccessbank*b there is no ramsize == 0 guard here. */
static inline int mem_st_is_rom(void)
{
    return (MemSeamC & 0x8000u) == 0;
}

/* Bank into a 32K slice, then mask. Only the low bits of the slice survive the
   mask on any real cartridge - 128K of SRAM masks to 0x1FFFF, and the shift
   puts anything above (bank - base) = 3 out of its reach - so most of the base
   constant is unobservable. Kept as the assembly writes it. */
static inline u4 mem_st_addr(u4 const base)
{
    MemSeamB = (MemSeamB & ~0xFFu) | ((MemSeamB - base) & 0xFFu);
    MemSeamB <<= 15;
    return (MemSeamC + MemSeamB) & ramsizeand;
}

#define MEM_ST_READ8(name, base, buf)                                        \
    void name(void)                                                          \
    {                                                                        \
        if (mem_st_is_rom()) {                                               \
            c_memaccessbankr8();                                             \
            return;                                                          \
        }                                                                    \
        mem_set_al((buf)[mem_st_addr(base)]);                                \
        MemSeamB = 0;                                                        \
    }

#define MEM_ST_READ16(name, base, buf)                                       \
    void name(void)                                                          \
    {                                                                        \
        u4 a;                                                                \
                                                                             \
        if (mem_st_is_rom()) {                                               \
            c_memaccessbankr16();                                            \
            return;                                                          \
        }                                                                    \
        a = mem_st_addr(base);                                               \
        mem_set_ax((u2)((buf)[a] | ((buf)[(a + 1) & ramsizeand] << 8)));      \
        MemSeamB = 0;                                                        \
    }

#define MEM_ST_WRITE8(name, base, buf)                                       \
    void name(void)                                                          \
    {                                                                        \
        if (mem_st_is_rom()) {                                               \
            c_memaccessbankw8();                                             \
            return;                                                          \
        }                                                                    \
        (buf)[mem_st_addr(base)] = (u1)(MemSeamA & 0xFFu);                   \
        sramb4save = 5 * 60;                                                 \
        MemSeamB = 0;                                                        \
    }

#define MEM_ST_WRITE16(name, base, buf)                                      \
    void name(void)                                                          \
    {                                                                        \
        u4 a;                                                                \
                                                                             \
        if (mem_st_is_rom()) {                                               \
            c_memaccessbankw16();                                            \
            return;                                                          \
        }                                                                    \
        a = mem_st_addr(base);                                               \
        (buf)[a] = (u1)(MemSeamA & 0xFFu);                                   \
        (buf)[(a + 1) & ramsizeand] = (u1)((MemSeamA >> 8) & 0xFFu);         \
        sramb4save = 5 * 60;                                                 \
        MemSeamB = 0;                                                        \
    }

MEM_ST_READ8(c_stsramr8, 0x60, sram)
MEM_ST_READ16(c_stsramr16, 0x60, sram)
MEM_ST_WRITE8(c_stsramw8, 0x60, sram)
MEM_ST_WRITE16(c_stsramw16, 0x60, sram)
MEM_ST_READ8(c_stsramr8b, 0x70, sram2)
MEM_ST_READ16(c_stsramr16b, 0x70, sram2)
MEM_ST_WRITE8(c_stsramw8b, 0x70, sram2)
MEM_ST_WRITE16(c_stsramw16b, 0x70, sram2)

/* --- banks 00-3F / 80-BF: the mixed ROM, WRAM, I/O and cartridge window ---
 *
 * ebx is the bank number. The address decides everything: bit 15 set is ROM,
 * below 2000 is the WRAM mirror, 2000-48FF the I/O registers, and 6000-7FFF
 * whatever the cartridge put there - SuperFX RAM, DSP1, or HiROM SRAM in 8K
 * slices - with plain open bus in between.
 */
void c_regaccessbankr8(void)
{
    if (MemSeamC & 0x8000u) { /* ROM */
        MemSeamB = (u4)(uintptr_t)mem_bank();
        mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) { /* WRAM mirror */
        mem_set_al(*mem_wram(MemSeamC));
        return;
    }
    if (MemSeamC <= 0x48FFu) { /* I/O register */
        MemRegRead();
        cpu_mdr = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) { /* open bus: the address's high byte */
        mem_set_al((u1)((MemSeamC >> 8) & 0xFFu));
        return;
    }
    /* SuperFX RAM. The 8K mirror mask is redundant on this path - bit 15 was
       already routed to ROM, so the address cannot exceed 7FFF - but the same
       shape is shared with membank0*chip, where it is not. Kept as written. */
    if (SFXEnable) {
        u4 const saved = MemSeamC;

        mem_set_al(sfxramdata[(MemSeamC - 0x6000u) & 0x1FFFu]);
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    MemSeamB &= 0x7Fu;
    if (MemSeamB < 0x10u) { /* DSP1 lives in the low banks */
        mem_set_al(0);
        if (DSP1Type == 2) {
            mem_set_al(c_DSP1Read8b(MemSeamC));
        }
        MemSeamB = 0;
        return;
    }
    if (MemSeamB < 0x30u) { /* nothing mapped */
        mem_set_al(0);
        MemSeamB = 0;
        return;
    }
    /* HiROM SRAM: banks 30 and up carry an 8K slice each. */
    {
        u4 const saved = MemSeamC;

        MemSeamB = (MemSeamB & ~0xFFu) | ((MemSeamB - 0x30u) & 0xFFu);
        MemSeamB <<= 13;
        MemSeamC = (((MemSeamC - 0x6000u) & 0x1FFFu) + MemSeamB) & 0xFFFFu;
        c_sramaccessbankr8b();
        MemSeamC = saved;
    }
}

void c_regaccessbankw8(void)
{
    if (MemSeamC & 0x8000u) { /* ROM, only writable with patching on */
        if (!writeon) {
            return;
        }
        MemSeamB = (u4)(uintptr_t)mem_bank();
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        *mem_wram(MemSeamC) = (u1)(MemSeamA & 0xFFu);
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        MemRegWrite();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) { /* open bus swallows the write */
        return;
    }
    if (SFXEnable) { /* same redundant mirror mask as the read side */
        u4 const saved = MemSeamC;

        sfxramdata[(MemSeamC - 0x6000u) & 0x1FFFu] = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    MemSeamB &= 0x7Fu;
    if (MemSeamB < 0x10u) {
        if (DSP1Type == 2) {
            c_DSP1Write8b(MemSeamC, (u1)(MemSeamA & 0xFFu));
        }
        MemSeamB = 0;
        return;
    }
    if (MemSeamB < 0x30u) {
        MemSeamB = 0;
        return;
    }
    {
        u4 const saved = MemSeamC;

        MemSeamB = (MemSeamB & ~0xFFu) | ((MemSeamB - 0x30u) & 0xFFu);
        MemSeamB <<= 13;
        MemSeamC = (((MemSeamC - 0x6000u) & 0x1FFFu) + MemSeamB) & 0xFFFFu;
        c_sramaccessbankw8b();
        MemSeamC = saved;
    }
}

/* The 16-bit twins. They are not simply the 8-bit ones widened: the last word
   of the WRAM mirror cannot straddle 2000, the write side compares only cx
   where the read side compares ecx, and the DSP1 write path is the one branch
   here that does not clear ebx. */
void c_regaccessbankr16(void)
{
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_bank();
        mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
            | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        /* The word is read either way; at 1FFF the high half is open bus. */
        u2 v = (u2)(mem_wram(MemSeamC)[0] | (mem_wram(MemSeamC)[1] << 8));

        if (MemSeamC == 0x1FFFu) {
            v = (u2)((v & 0xFFu) | ((v & 0xFFu) << 8));
        }
        mem_set_ax(v);
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        mem_reg_read16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        u1 const bus = (u1)((MemSeamC >> 8) & 0xFFu);

        mem_set_ax((u2)(bus | (bus << 8)));
        return;
    }
    if (SFXEnable) {
        u4 const saved = MemSeamC;
        u4 const a = (MemSeamC - 0x6000u) & 0x1FFFu;

        mem_set_ax((u2)(sfxramdata[a] | (sfxramdata[a + 1] << 8)));
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    MemSeamB &= 0x7Fu;
    if (MemSeamB < 0x10u) {
        mem_set_ax(0);
        if (DSP1Type == 2) {
            mem_set_ax(c_DSP1Read16b(MemSeamC));
        }
        MemSeamB = 0;
        return;
    }
    if (MemSeamB < 0x30u) {
        mem_set_ax(0);
        MemSeamB = 0;
        return;
    }
    {
        u4 const saved = MemSeamC;

        MemSeamB = (MemSeamB & ~0xFFu) | ((MemSeamB - 0x30u) & 0xFFu);
        MemSeamB <<= 13;
        MemSeamC = (((MemSeamC - 0x6000u) & 0x1FFFu) + MemSeamB) & 0xFFFFu;
        c_sramaccessbankr16b();
        MemSeamC = saved;
    }
}

void c_regaccessbankw16(void)
{
    if (MemSeamC & 0x8000u) {
        if (!writeon) {
            return;
        }
        MemSeamB = (u4)(uintptr_t)mem_bank();
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) = (u1)((MemSeamA >> 8) & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        mem_wram(MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
        if (MemSeamC != 0x1FFFu) {
            mem_wram(MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
        }
        return;
    }
    /* Note cx, not ecx - the read side compares the full register. */
    if ((MemSeamC & 0xFFFFu) <= 0x48FFu) {
        mem_reg_write16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        return;
    }
    if (SFXEnable) {
        u4 const saved = MemSeamC;
        u4 const a = (MemSeamC - 0x6000u) & 0x1FFFu;

        sfxramdata[a] = (u1)(MemSeamA & 0xFFu);
        sfxramdata[a + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    MemSeamB &= 0x7Fu;
    if (MemSeamB < 0x10u) {
        if (DSP1Type == 2) {
            c_DSP1Write16b(MemSeamC, (u2)(MemSeamA & 0xFFFFu));
        }
        return; /* the one branch here that leaves ebx alone */
    }
    if (MemSeamB < 0x30u) {
        mem_set_al(0);
        MemSeamB = 0;
        return;
    }
    {
        u4 const saved = MemSeamC;

        MemSeamB = (MemSeamB & ~0xFFu) | ((MemSeamB - 0x30u) & 0xFFu);
        MemSeamB <<= 13;
        MemSeamC = (((MemSeamC - 0x6000u) & 0x1FFFu) + MemSeamB) & 0xFFFFu;
        c_sramaccessbankw16b();
        MemSeamC = saved;
    }
}

/* --- the general bank 00-3F / 80-BF dispatchers -------------------------- *
 *
 * Same windows as regaccessbank*, with three differences worth keeping in
 * sight: the address is masked to 16 bits first, ROM comes from map entry 0
 * rather than one picked by ebx, and an SA-1 cart hands the whole access to
 * the still-assembly SA-1 variant. The open-bus value also differs between the
 * 8- and 16-bit reads, and none of the cartridge paths here clear ebx.
 */
void c_membank0r8SA1(void);
void c_membank0r16SA1(void);
void c_membank0w8SA1(void);
void c_membank0w16SA1(void);

void c_membank0r8(void)
{
    MemSeamC &= 0xFFFFu;
    if (SA1Enable) {
        c_membank0r8SA1();
        return;
    }
    if (MemSeamC < 0x2000u) {
        mem_set_al(*mem_wram(MemSeamC));
        return;
    }
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_rom();
        mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        MemRegRead();
        cpu_mdr = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        mem_set_al((u1)((MemSeamC >> 8) & 0xFFu));
        return;
    }
    if (SFXEnable) {
        u4 const saved = MemSeamC;

        mem_set_al(sfxramdata[(MemSeamC - 0x6000u) & 0x1FFFu]);
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    mem_set_al(0);
    if (DSP1Type == 2) {
        mem_set_al(c_DSP1Read8b(MemSeamC));
    }
}

void c_membank0r16(void)
{
    MemSeamC &= 0xFFFFu;
    if (SA1Enable) {
        c_membank0r16SA1();
        return;
    }
    if (MemSeamC < 0x2000u) {
        mem_set_ax((u2)(mem_wram(MemSeamC)[0] | (mem_wram(MemSeamC)[1] << 8)));
        return;
    }
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_rom();
        mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
            | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        mem_reg_read16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        mem_set_ax(0); /* not the address high byte, unlike the 8-bit read */
        return;
    }
    if (SFXEnable) {
        u4 const saved = MemSeamC;
        u4 const a = (MemSeamC - 0x6000u) & 0x1FFFu;

        mem_set_ax((u2)(sfxramdata[a] | (sfxramdata[a + 1] << 8)));
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    mem_set_ax(0);
    if (DSP1Type == 2) {
        mem_set_ax(c_DSP1Read16b(MemSeamC));
    }
}

void c_membank0w8(void)
{
    MemSeamC &= 0xFFFFu;
    if (SA1Enable) {
        c_membank0w8SA1();
        return;
    }
    if (MemSeamC < 0x2000u) {
        *mem_wram(MemSeamC) = (u1)(MemSeamA & 0xFFu);
        return;
    }
    if (MemSeamC & 0x8000u) {
        if (!writeon) {
            return;
        }
        MemSeamB = (u4)(uintptr_t)mem_rom();
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        MemRegWrite();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        return;
    }
    if (SFXEnable) {
        u4 const saved = MemSeamC;

        sfxramdata[(MemSeamC - 0x6000u) & 0x1FFFu] = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    if (DSP1Type == 2) {
        c_DSP1Write8b(MemSeamC, (u1)(MemSeamA & 0xFFu));
    }
}

void c_membank0w16(void)
{
    MemSeamC &= 0xFFFFu;
    if (SA1Enable) {
        c_membank0w16SA1();
        return;
    }
    if (MemSeamC < 0x2000u) {
        mem_wram(MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
        mem_wram(MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
        return;
    }
    if (MemSeamC & 0x8000u) {
        if (!writeon) {
            return;
        }
        MemSeamB = (u4)(uintptr_t)mem_rom();
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) = (u1)((MemSeamA >> 8) & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        mem_reg_write16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        return;
    }
    if (SFXEnable) {
        u4 const saved = MemSeamC;
        u4 const a = (MemSeamC - 0x6000u) & 0x1FFFu;

        sfxramdata[a] = (u1)(MemSeamA & 0xFFu);
        sfxramdata[a + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
        MemSeamB = 0;
        MemSeamC = saved;
        return;
    }
    if (DSP1Type == 2) {
        c_DSP1Write16b(MemSeamC, (u2)(MemSeamA & 0xFFFFu));
    }
}

/* --- bank 00-3F low RAM on an SA-1 cart ---------------------------------- *
 *
 * While the 65816 has the bus this is the ordinary WRAM mirror; while the SA-1
 * has it, the same window is the SA-1's own 2K of IRAM, and anything above it
 * reads back zero. Only ecx is range-checked, so ebx can carry the index a
 * little past 800h - IRAM has room for that.
 */
void c_membank0r8ramSA1(void)
{
    if (SA1Status == 0) {
        mem_set_al(*mem_wram(MemSeamC + MemSeamB));
        return;
    }
    if (MemSeamC >= 0x800u) {
        mem_set_al(0);
        return;
    }
    mem_set_al(IRAM[MemSeamC + MemSeamB]);
}

void c_membank0r16ramSA1(void)
{
    if (SA1Status == 0) {
        u4 const a = MemSeamC + MemSeamB;

        mem_set_ax((u2)(mem_wram(a)[0] | (mem_wram(a)[1] << 8)));
        return;
    }
    if (MemSeamC >= 0x800u) {
        mem_set_ax(0);
        return;
    }
    mem_set_ax((u2)(IRAM[MemSeamC + MemSeamB]
        | (IRAM[MemSeamC + MemSeamB + 1] << 8)));
}

void c_membank0w8ramSA1(void)
{
    if (SA1Status == 0) {
        *mem_wram(MemSeamC + MemSeamB) = (u1)(MemSeamA & 0xFFu);
        return;
    }
    if (MemSeamC >= 0x800u) {
        return;
    }
    IRAM[MemSeamC + MemSeamB] = (u1)(MemSeamA & 0xFFu);
}

void c_membank0w16ramSA1(void)
{
    if (SA1Status == 0) {
        u4 const a = MemSeamC + MemSeamB;

        mem_wram(a)[0] = (u1)(MemSeamA & 0xFFu);
        mem_wram(a)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
        return;
    }
    if (MemSeamC >= 0x800u) {
        return;
    }
    IRAM[MemSeamC + MemSeamB] = (u1)(MemSeamA & 0xFFu);
    IRAM[MemSeamC + MemSeamB + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
}

/* --- the SA-1's view of its own RAM -------------------------------------- *
 *
 * Banks 40-4F, four 64K slices of SA1RAMArea. While a character-conversion
 * DMA is in flight the reads come from the converter instead, one byte at a
 * time - writes never take that path.
 */
void c_SA1RAMaccessbankr8(void)
{
    if (SA1_in_cc1_dma != 0) {
        SA1_DMA_ADDR = (SA1_DMA_ADDR & ~0xFFFFu) | (MemSeamC & 0xFFFFu);
        SA1_DMA_CC1();
        mem_set_al(SA1_DMA_VALUE);
        return;
    }
    MemSeamB = ((MemSeamB & 3u) << 16) + (u4)(uintptr_t)SA1RAMArea;
    mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
    MemSeamB = 0;
}

void c_SA1RAMaccessbankr16(void)
{
    if (SA1_in_cc1_dma != 0) {
        u1 lo, hi;

        SA1_DMA_ADDR = (SA1_DMA_ADDR & ~0xFFFFu) | (MemSeamC & 0xFFFFu);
        SA1_DMA_CC1();
        lo = SA1_DMA_VALUE;
        /* The address advances as a word, so it wraps inside the bank. */
        SA1_DMA_ADDR = (SA1_DMA_ADDR & ~0xFFFFu)
            | ((SA1_DMA_ADDR + 1) & 0xFFFFu);
        SA1_DMA_CC1();
        hi = SA1_DMA_VALUE;
        mem_set_ax((u2)(lo | (hi << 8)));
        return;
    }
    MemSeamB = ((MemSeamB & 3u) << 16) + (u4)(uintptr_t)SA1RAMArea;
    mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
        | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
    MemSeamB = 0;
}

void c_SA1RAMaccessbankw8(void)
{
    MemSeamB = ((MemSeamB & 3u) << 16) + (u4)(uintptr_t)SA1RAMArea;
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    MemSeamB = 0;
}

void c_SA1RAMaccessbankw16(void)
{
    MemSeamB = ((MemSeamB & 3u) << 16) + (u4)(uintptr_t)SA1RAMArea;
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) = (u1)((MemSeamA >> 8) & 0xFFu);
    MemSeamB = 0;
}

/* --- the SA-1's RAM seen as a bit map ------------------------------------ *
 *
 * Banks 60-6F: the same RAM as above, but addressed one pixel at a time -
 * 4 bits each, or 2 when SA1Overflow's bit 15 is set, which also widens the
 * bank field from 3 to 4 bits because a slice then covers half as much. The
 * address is a pixel index, so it is shifted down to a byte index and left
 * that way: the caller sees the shifted ecx, and ebx comes back zero.
 */
static inline u4 mem_bm_2bit(void)
{
    return SA1Overflow & 0x8000u;
}

/* Byte index of the pixel ecx addresses, folding ebx's slice in. Shifts ecx
   down as the assembly does, so call it once per access. */
static inline u1* mem_bm_byte(void)
{
    u4 const slice = mem_bm_2bit() ? ((MemSeamB & 0x0Fu) << 14)
                                   : ((MemSeamB & 0x07u) << 15);
    u4 const idx = MemSeamC >> (mem_bm_2bit() ? 2 : 1);

    MemSeamC = idx;
    return SA1RAMArea + slice + idx;
}

static inline u4 mem_bm_shift(u4 const addr)
{
    return mem_bm_2bit() ? ((addr & 3u) << 1) : ((addr & 1u) << 2);
}

static inline u4 mem_bm_mask(void)
{
    return mem_bm_2bit() ? 3u : 0x0Fu;
}

void c_SA1RAMaccessbankr8b(void)
{
    u4 const sh = mem_bm_shift(MemSeamC);
    u1 const* const p = mem_bm_byte();

    mem_set_al((u1)((*p >> sh) & mem_bm_mask()));
    MemSeamB = 0;
}

/* Two pixels: the addressed field and the next one along, which is in the
   following byte only when the first was the last field of this one. */
void c_SA1RAMaccessbankr16b(void)
{
    u4 const addr = MemSeamC;
    u4 const sh = mem_bm_shift(addr);
    u4 const step = mem_bm_2bit() ? 2u : 4u;
    u4 const mask = mem_bm_mask();
    u1 const* const p = mem_bm_byte();
    u1 lo = (u1)((*p >> sh) & mask);
    u1 const hi = sh + step > 7u ? (u1)(p[1] & mask)
                                 : (u1)((*p >> (sh + step)) & mask);

    /* At 2 bits per pixel the second field masks with 2 rather than 3, so a
       colour read from there loses its bottom bit. That is what the assembly
       does; do not "fix" it without a reason to. */
    if (mem_bm_2bit() && (addr & 3u) == 1u) {
        lo = (u1)(lo & 2u);
    }
    mem_set_ax((u2)(lo | (hi << 8)));
    MemSeamB = 0;
}

/* The write leaves the shifted field in al, not the caller's value. */
void c_SA1RAMaccessbankw8b(void)
{
    u4 const sh = mem_bm_shift(MemSeamC);
    u4 const mask = mem_bm_mask();
    u1* const p = mem_bm_byte();
    u1 const field = (u1)(((MemSeamA & mask) << sh) & 0xFFu);

    *p = (u1)((*p & (u1)~(u1)(mask << sh)) | field);
    mem_set_al(field);
    MemSeamB = 0;
}

/* Two byte writes at consecutive pixels; ebx and ecx are restored in between,
   so the second one indexes from the original address plus one, and ah moves
   into al for it. */
void c_SA1RAMaccessbankw16b(void)
{
    u4 const b = MemSeamB;
    u4 const c = MemSeamC;

    c_SA1RAMaccessbankw8b();
    MemSeamB = b;
    MemSeamC = c + 1;
    mem_set_al((u1)((MemSeamA >> 8) & 0xFFu));
    c_SA1RAMaccessbankw8b();
}

/* --- SA-1 BW-RAM, byte view or bit map ----------------------------------- *
 *
 * With BWShift set and the SA-1 holding the bus, the 6000-7FFF window is a
 * packed view of BW-RAM: two pixels per byte, or four when SA1Overflow's bit
 * 15 selects the 2-bit mode. Otherwise it is a plain byte window through
 * CurBWPtr.
 */
static inline int mem_bw_mapped(void)
{
    return (BWShift & 0xFFu) != 0 && SA1Status != 0;
}

static inline u4 mem_bw_shift(u4 const off)
{
    return (SA1Overflow & 0x8000u) ? ((off & 3u) << 1) : ((off & 1u) << 2);
}

static inline u4 mem_bw_index(u4 const off)
{
    return (SA1Overflow & 0x8000u) ? (off >> 2) : (off >> 1);
}

static inline u4 mem_bw_mask(void)
{
    return (SA1Overflow & 0x8000u) ? 0x03u : 0x0Fu;
}

static inline u1 mem_bw_get(u4 const off)
{
    u4 const sh = mem_bw_shift(off);

    return (u1)((SA1BWPtr[mem_bw_index(off)] & (u1)(mem_bw_mask() << sh)) >> sh);
}

/* Returns the field it stored: the assembly leaves it in al (or ah for the
   high half of a 16-bit write) rather than putting the caller's byte back, and
   the caller can see that. */
static inline u1 mem_bw_put(u4 const off, u1 const val)
{
    u4 const sh = mem_bw_shift(off);
    u4 const mask = mem_bw_mask();
    u4 const idx = mem_bw_index(off);
    u1 const field = (u1)(((val & mask) << sh) & 0xFFu);

    SA1BWPtr[idx] = (u1)((SA1BWPtr[idx] & (u1)~(u1)(mask << sh)) | field);
    return field;
}

static inline u1 mem_bw_read8(void)
{
    return mem_bw_get(MemSeamC - 0x6000u);
}

static inline void mem_bw_write8(void)
{
    u1 const field = mem_bw_put(MemSeamC - 0x6000u, (u1)(MemSeamA & 0xFFu));

    MemSeamA = (MemSeamA & ~0xFFu) | field;
}

static inline void mem_bw_read16(void)
{
    u4 const off = MemSeamC - 0x6000u;
    u1 const lo = mem_bw_get(off);
    u1 const hi = mem_bw_get(off + 1);

    mem_set_ax((u2)(lo | (hi << 8)));
}

/* Low half first, then the high half one pixel along. Unlike the 8-bit write,
   the assembly brackets the low half with push/pop eax, so al comes back
   intact and only ah is left holding its shifted field. */
static inline void mem_bw_write16(void)
{
    u4 const off = MemSeamC - 0x6000u;
    u1 hi;

    (void)mem_bw_put(off, (u1)(MemSeamA & 0xFFu));
    hi = mem_bw_put(off + 1, (u1)((MemSeamA >> 8) & 0xFFu));
    MemSeamA = (MemSeamA & ~0xFF00u) | ((u4)hi << 8);
}

/* --- the 6000-FFFF cartridge window -------------------------------------- *
 *
 * Whatever the cart puts there: an 8K SuperFX RAM mirror, SA-1 BW-RAM (a byte
 * window through CurBWPtr, or the bit map once BWShift is set) or the DSP1,
 * and nothing at all otherwise - a read is then zero, not open bus. Note the
 * address add is a full 32-bit one, unlike the `add cx,bx` the ram and romram
 * handlers use, and that only the two cartridge RAM paths clear ebx.
 */
static inline u4 mem_sfx_off(void)
{
    return (MemSeamC - 0x6000u) & 0x1FFFu;
}

void c_membank0r8chip(void) /* 6000-7FFF */
{
    MemSeamC += MemSeamB;
    if (SFXEnable) {
        mem_set_al(sfxramdata[mem_sfx_off()]);
        MemSeamB = 0;
        return;
    }
    if (SA1Enable) {
        if (mem_bw_mapped()) {
            mem_set_al(mem_bw_read8());
            return;
        }
        mem_set_al(CurBWPtr[MemSeamC]);
        MemSeamB = 0;
        return;
    }
    mem_set_al(0);
    if (DSP1Type == 2) {
        mem_set_al(c_DSP1Read8b(MemSeamC));
    }
}

void c_membank0r16chip(void) /* 6000-FFFF */
{
    MemSeamC += MemSeamB;
    if (SFXEnable) {
        u4 const a = mem_sfx_off();

        mem_set_ax((u2)(sfxramdata[a] | (sfxramdata[a + 1] << 8)));
        MemSeamB = 0;
        return;
    }
    if (SA1Enable) {
        if (mem_bw_mapped()) {
            mem_bw_read16();
            return;
        }
        mem_set_ax((u2)(CurBWPtr[MemSeamC] | (CurBWPtr[MemSeamC + 1] << 8)));
        MemSeamB = 0;
        return;
    }
    mem_set_ax(0);
    if (DSP1Type == 2) {
        mem_set_ax(c_DSP1Read16b(MemSeamC));
    }
}

void c_membank0w8chip(void) /* 6000-FFFF */
{
    MemSeamC += MemSeamB;
    if (SFXEnable) {
        sfxramdata[mem_sfx_off()] = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (SA1Enable) {
        if (mem_bw_mapped()) {
            mem_bw_write8();
            return;
        }
        CurBWPtr[MemSeamC] = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (DSP1Type == 2) {
        c_DSP1Write8b(MemSeamC, (u1)(MemSeamA & 0xFFu));
    }
}

/* The 16-bit write reaches the same body twice over: membank0w16chip folds ebx
   into the address and falls through into membank0w16rom, which is installed
   for 8000-FFFF and so uses the direct page register on its own. */
void c_membank0w16rom(void) /* 8000-FFFF */
{
    if (SFXEnable) {
        u4 const a = mem_sfx_off();

        sfxramdata[a] = (u1)(MemSeamA & 0xFFu);
        sfxramdata[a + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (SA1Enable) {
        if (mem_bw_mapped()) {
            mem_bw_write16();
            return;
        }
        CurBWPtr[MemSeamC] = (u1)(MemSeamA & 0xFFu);
        CurBWPtr[MemSeamC + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (DSP1Type == 2) {
        c_DSP1Write16b(MemSeamC, (u2)(MemSeamA & 0xFFFFu));
    }
}

void c_membank0w16chip(void) /* 6000-FFFF */
{
    MemSeamC += MemSeamB;
    c_membank0w16rom();
}

void c_regaccessbankr8SA1(void)
{
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_bank();
        mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            mem_set_al(*mem_wram(MemSeamC));
            return;
        }
        if (MemSeamC >= 0x800u) {
            mem_set_al(0);
            return;
        }
        mem_set_al(IRAM[MemSeamC]);
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        MemRegRead();
        cpu_mdr = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        mem_set_al(0); /* zero here, not the address byte */
        return;
    }
    if (mem_bw_mapped()) {
        mem_set_al(mem_bw_read8());
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
    MemSeamB = 0;
}

void c_regaccessbankw8SA1(void)
{
    if (MemSeamC & 0x8000u) {
        if (!writeon) {
            return;
        }
        MemSeamB = (u4)(uintptr_t)mem_bank();
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            *mem_wram(MemSeamC) = (u1)(MemSeamA & 0xFFu);
            return;
        }
        if (MemSeamC >= 0x800u) {
            return;
        }
        IRAM[MemSeamC] = (u1)(MemSeamA & 0xFFu);
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        MemRegWrite();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        return;
    }
    if (mem_bw_mapped()) {
        mem_bw_write8();
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    MemSeamB = 0;
}

void c_regaccessbankr16SA1(void)
{
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_bank();
        mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
            | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            mem_set_ax((u2)(mem_wram(MemSeamC)[0] | (mem_wram(MemSeamC)[1] << 8)));
            return;
        }
        if (MemSeamC >= 0x800u) {
            mem_set_ax(0);
            return;
        }
        mem_set_ax((u2)(IRAM[MemSeamC] | (IRAM[MemSeamC + 1] << 8)));
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        mem_reg_read16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        mem_set_ax(0);
        return;
    }
    if (mem_bw_mapped()) {
        mem_bw_read16();
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
        | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
    MemSeamB = 0;
}

void c_regaccessbankw16SA1(void)
{
    if (MemSeamC & 0x8000u) {
        if (!writeon) {
            return;
        }
        MemSeamB = (u4)(uintptr_t)mem_bank();
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
        *(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) = (u1)((MemSeamA >> 8) & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            mem_wram(MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
            mem_wram(MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
            return;
        }
        if (MemSeamC >= 0x800u) {
            return;
        }
        IRAM[MemSeamC] = (u1)(MemSeamA & 0xFFu);
        IRAM[MemSeamC + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
        return;
    }
    /* cx, not ecx - matching the assembly. Nothing can tell the difference:
       a >16-bit address that got past this would index the register table out
       of bounds, in the original too, so it never reaches here. */
    if ((MemSeamC & 0xFFFFu) <= 0x48FFu) {
        mem_reg_write16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        return;
    }
    if (mem_bw_mapped()) {
        mem_bw_write16();
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) = (u1)((MemSeamA >> 8) & 0xFFu);
    MemSeamB = 0;
}

/* --- the SA-1 cart's general dispatchers --------------------------------- *
 *
 * The same windows as regaccessbank*SA1, with two differences: ROM comes from
 * map entry 0 rather than a bank-indexed one, and a write to ROM is simply
 * dropped - there is no writeon check here at all.
 */
void c_membank0r8SA1(void)
{
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_rom();
        mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            mem_set_al(*mem_wram(MemSeamC));
            return;
        }
        if (MemSeamC >= 0x800u) {
            mem_set_al(0);
            return;
        }
        mem_set_al(IRAM[MemSeamC]);
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        MemRegRead();
        cpu_mdr = (u1)(MemSeamA & 0xFFu);
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        mem_set_al(0);
        return;
    }
    if (mem_bw_mapped()) {
        mem_set_al(mem_bw_read8());
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    mem_set_al(*(u1*)(uintptr_t)(MemSeamB + MemSeamC));
    MemSeamB = 0;
}

void c_membank0r16SA1(void)
{
    if (MemSeamC & 0x8000u) {
        MemSeamB = (u4)(uintptr_t)mem_rom();
        mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
            | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            mem_set_ax((u2)(mem_wram(MemSeamC)[0] | (mem_wram(MemSeamC)[1] << 8)));
            return;
        }
        if (MemSeamC >= 0x800u) {
            mem_set_ax(0);
            return;
        }
        mem_set_ax((u2)(IRAM[MemSeamC] | (IRAM[MemSeamC + 1] << 8)));
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        mem_reg_read16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        mem_set_ax(0);
        return;
    }
    if (mem_bw_mapped()) {
        mem_bw_read16();
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    mem_set_ax((u2)(*(u1*)(uintptr_t)(MemSeamB + MemSeamC)
        | (*(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) << 8)));
    MemSeamB = 0;
}

void c_membank0w8SA1(void)
{
    if (MemSeamC & 0x8000u) {
        return; /* ROM writes go nowhere here, patching or not */
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            *mem_wram(MemSeamC) = (u1)(MemSeamA & 0xFFu);
            return;
        }
        if (MemSeamC >= 0x800u) {
            return;
        }
        IRAM[MemSeamC] = (u1)(MemSeamA & 0xFFu);
        return;
    }
    if (MemSeamC <= 0x48FFu) {
        MemRegWrite();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        return;
    }
    if (mem_bw_mapped()) {
        mem_bw_write8();
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    MemSeamB = 0;
}

void c_membank0w16SA1(void)
{
    if (MemSeamC & 0x8000u) {
        return;
    }
    if (MemSeamC < 0x2000u) {
        if (SA1Status == 0) {
            mem_wram(MemSeamC)[0] = (u1)(MemSeamA & 0xFFu);
            mem_wram(MemSeamC)[1] = (u1)((MemSeamA >> 8) & 0xFFu);
            return;
        }
        if (MemSeamC >= 0x800u) {
            return;
        }
        IRAM[MemSeamC] = (u1)(MemSeamA & 0xFFu);
        IRAM[MemSeamC + 1] = (u1)((MemSeamA >> 8) & 0xFFu);
        return;
    }
    if ((MemSeamC & 0xFFFFu) <= 0x48FFu) { /* cx, as in the SA-1 twin */
        mem_reg_write16();
        MemSeamB = 0;
        return;
    }
    if (MemSeamC < 0x6000u) {
        return;
    }
    if (mem_bw_mapped()) {
        mem_bw_write16();
        return;
    }
    MemSeamB = (u4)(uintptr_t)CurBWPtr;
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC) = (u1)(MemSeamA & 0xFFu);
    *(u1*)(uintptr_t)(MemSeamB + MemSeamC + 1) = (u1)((MemSeamA >> 8) & 0xFFu);
    MemSeamB = 0;
}

#endif
