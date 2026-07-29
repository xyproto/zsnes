/* Differential test: the direct-page memory handlers in cpu/memory.asm against
 * the C port in cpu/mem_ops.h.
 *
 * The handlers are leaf functions with a register ABI - ebx the direct-page
 * offset, ecx the direct page register, al/ax the value - and the caller keeps
 * whatever they leave in all three, so the test compares the registers on the
 * way out as well as the memory they wrote.
 *
 * The oracle (_memops.o, built by mkmemops.sh from the pre-port revision) is
 * driven through asm_memcall, which sets the registers up from the same seam
 * block the ported side reads. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../chips/regabi.h" /* REGABI_ENTRY/REGABI_SYM for the trampolines */
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

/* The assembly indexes wramdataa as a flat 64K array; the ROM map is a base
 * pointer the 8000-FFFF handlers add the address to. A handler can address up
 * to 0xFFFF past the base, so give the ROM window that much plus a byte of
 * slack for a 16-bit read at the very top. */
#define ROM_SIZE 0x10000
u1 wramdataa[65536];
static u1 rom[ROM_SIZE + 2];
u1* snesmmap[256];

static u1 wram_init[65536];
static u1 rom_init[ROM_SIZE + 2];

/* The seam block (normally cpu/c_memops.c). */
u4 MemSeamB, MemSeamC, MemSeamA;

/* The I/O register tables, indexed regptra[addr - 0x2000] as in ui.h, and the
   open-bus latch every register read updates. */
void (*regptra[0x3000])(void), (*regptwa[0x3000])(void);
u1 cpu_mdr;

/* What the stub handlers saw: the last four calls, since a 16-bit access makes
   two. */
u4 StubRegAddr[4], StubRegVal[4], StubRegHits;
extern void regstub_r(void), regstub_w(void); /* _memops.o */

void asm_memcall(void* fn); /* _memops.o */

#include "../cpu/mem_ops.h"

extern void asm_membank0r8ram(void), asm_membank0r8inv(void);
extern void asm_membank0r8rom(void), asm_membank0r8romram(void);
extern void asm_membank0r16ram(void), asm_membank0r16ramh(void);
extern void asm_membank0r16rom(void), asm_membank0r16romram(void);
extern void asm_membank0w8ram(void), asm_membank0w8inv(void);
extern void asm_membank0w8rom(void), asm_membank0w8romram(void);
extern void asm_membank0w16ram(void), asm_membank0w16ramh(void);
extern void asm_membank0w16inv(void), asm_membank0w16romram(void);
extern void asm_membank0r8reg(void), asm_membank0r16reg(void);
extern void asm_membank0w8reg(void), asm_membank0w16reg(void);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
    /* Which window the handler addresses, so the setup can keep it in range:
       0 = WRAM via ebx+ecx, 1 = WRAM via ecx after `add cx,bx`, 2 = the ROM
       map, 3 = the 1F00-1FFF page, 4 = neither, 5 = an I/O register. */
    int win;
} memcase;

static memcase const cases[] = {
    { "membank0r8ram", asm_membank0r8ram, c_membank0r8ram, 0 },
    { "membank0r8inv", asm_membank0r8inv, c_membank0r8inv, 4 },
    { "membank0r8rom", asm_membank0r8rom, c_membank0r8rom, 2 },
    { "membank0r8romram", asm_membank0r8romram, c_membank0r8romram, 1 },
    { "membank0r16ram", asm_membank0r16ram, c_membank0r16ram, 0 },
    { "membank0r16ramh", asm_membank0r16ramh, c_membank0r16ramh, 3 },
    { "membank0r16rom", asm_membank0r16rom, c_membank0r16rom, 2 },
    { "membank0r16romram", asm_membank0r16romram, c_membank0r16romram, 1 },
    { "membank0w8ram", asm_membank0w8ram, c_membank0w8ram, 0 },
    { "membank0w8inv", asm_membank0w8inv, c_membank0w8inv, 4 },
    { "membank0w8rom", asm_membank0w8rom, c_membank0w8rom, 4 },
    { "membank0w8romram", asm_membank0w8romram, c_membank0w8romram, 1 },
    { "membank0w16ram", asm_membank0w16ram, c_membank0w16ram, 0 },
    { "membank0w16ramh", asm_membank0w16ramh, c_membank0w16ramh, 3 },
    { "membank0w16inv", asm_membank0w16inv, c_membank0w16inv, 4 },
    { "membank0w16romram", asm_membank0w16romram, c_membank0w16romram, 1 },
    { "membank0r8reg", asm_membank0r8reg, c_membank0r8reg, 5 },
    { "membank0r16reg", asm_membank0r16reg, c_membank0r16reg, 5 },
    { "membank0w8reg", asm_membank0w8reg, c_membank0w8reg, 5 },
    { "membank0w16reg", asm_membank0w16reg, c_membank0w16reg, 5 },
};

typedef struct {
    u4 b, c, a;
} setup;

typedef struct {
    u4 b, c, a;
    u4 reghits, regaddr[4], regval[4];
    u1 mdr;
    u1 wram[65536];
    u1 rom[ROM_SIZE + 2];
} snapshot;

static void run(void (*fn)(void), setup const* in, int asm_side, snapshot* out)
{
    memcpy(wramdataa, wram_init, sizeof wramdataa);
    memcpy(rom, rom_init, sizeof rom);
    snesmmap[0] = rom;
    MemSeamB = in->b;
    MemSeamC = in->c;
    MemSeamA = in->a;
    StubRegHits = 0;
    memset(StubRegAddr, 0, sizeof StubRegAddr);
    memset(StubRegVal, 0, sizeof StubRegVal);
    cpu_mdr = 0;

    if (asm_side) {
        asm_memcall((void*)fn);
    } else {
        fn();
    }

    /* The ROM base is a run-time address, so report it relative. */
    out->b = MemSeamB;
    out->c = MemSeamC;
    out->a = MemSeamA;
    out->reghits = StubRegHits;
    memcpy(out->regaddr, StubRegAddr, sizeof out->regaddr);
    memcpy(out->regval, StubRegVal, sizeof out->regval);
    out->mdr = cpu_mdr;
    memcpy(out->wram, wramdataa, sizeof out->wram);
    memcpy(out->rom, rom, sizeof out->rom);
}

int main(void)
{
    /* Filled once: the contents only have to be unpredictable, so reading the
       wrong address reads a different byte. Refilling them every iteration
       costs far more than it buys. */
    dt_fill(wram_init, sizeof wram_init);
    dt_fill(rom_init, sizeof rom_init);
    for (int i = 0; i < 0x3000; i++) {
        regptra[i] = regstub_r;
        regptwa[i] = regstub_w;
    }

    DT_MAIN(20260729, 200000)
    {
        memcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        setup in;
        snapshot a, c;

        /* Keep the upper halves live: the handlers only touch al/ax and the
           low word of ecx, so a wrong-width write shows up here. */
        in.a = dt_u32();
        in.b = dt_mod(256);
        in.c = dt_u32();

        switch (k->win) {
        case 0:
            /* ebx + ecx addresses WRAM directly, and a 16-bit read needs one
               byte of headroom. */
            in.c = dt_mod(65536 - 256 - 1);
            break;
        case 1:
            /* `add cx,bx` first, then bit 15 picks WRAM or the ROM map. Both
               windows are addressed off the full ecx, so keep it 16-bit as the
               caller (mov ecx,[xd]) always leaves it. Half the time sit just
               under 0xFFFF so the add wraps inside cx: a 32-bit add lands
               somewhere else entirely, and nothing else here would notice. */
            in.c = dt_mod(2) ? 0xFF00u + dt_mod(0x100)
                             : dt_mod(65536 - 256 - 1);
            break;
        case 2:
            /* ebx is folded into the ROM base, ecx is the offset. */
            in.c = dt_mod(ROM_SIZE - 256 - 1);
            break;
        case 3:
            /* Bias hard at the 1FFF boundary, which is the whole point of the
               ramh variants. */
            in.c = dt_mod(2) ? 0x1FFFu - dt_mod(2) - in.b : dt_mod(0x2000);
            break;
        case 5:
            /* `add ecx,ebx` is a full 32-bit add here, and the 16-bit variants
               go on to touch ecx+1, so leave room below 48FF. */
            in.c = 0x2000u + dt_mod(0x28FEu - 256u);
            break;
        default:
            in.c = dt_u32();
            break;
        }

        run(k->asm_fn, &in, 1, &a);
        run(k->c_fn, &in, 0, &c);

        DT_EQ(k->name, a.a, c.a);
        DT_EQ("ebx", a.b, c.b);
        DT_EQ("ecx", a.c, c.c);
        DT_MEM("wramdataa", a.wram, c.wram, sizeof a.wram);
        DT_MEM("ROM window", a.rom, c.rom, sizeof a.rom);
        DT_EQ("register handler calls", a.reghits, c.reghits);
        DT_MEM("register handler address", a.regaddr, c.regaddr, sizeof a.regaddr);
        DT_MEM("register handler eax", a.regval, c.regval, sizeof a.regval);
        DT_EQ("cpu_mdr", a.mdr, c.mdr);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ case %s b=%x c=%x a=%x\n", k->name, in.b, in.c, in.a);
        }
    }
    DT_DONE("direct-page memory handlers");
}
