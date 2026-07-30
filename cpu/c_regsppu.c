/* PPU read handlers ported from cpu/regs.inc.
 *
 * Kept apart from cpu/c_regs.c, which is the table setup and pulls in the
 * whole register file, so the difftest can link these on their own.
 */
#include "../chips/regabi.h"
#include "../types.h"

/* --- PPU reads ported from cpu/regs.inc ---------------------------------- *
 *
 * Legacy ABI: no argument, the value comes back in al, and the trampoline in
 * chips/regabi.h keeps ecx, edx and the upper half of eax. It does not keep
 * ebx, and neither did the assembly - checkmultchange used bx as scratch. The
 * only callers are the trampolines in cpu/mem_ops.h, which restore their own
 * ebx around the call.
 */
extern u1 vidbright, forceblnk, multchange, compmult[3];
extern u2 mode7A, mode7B;
extern u1 rtoflags, romispal, ppustatus, cfield, extlatch, ppu2_mdr;
extern u1 latchxr, latchyr, NMIEnab, cpu_mdr, curnmi, irqon;
extern u4 wramrwadr;
extern u1 ioportval;
extern u2 divres, multres;
extern u4 JoyARead, JoyBRead, JoyCRead2, JoyDRead;
extern u1 oamram[1024];
extern u2 cgram[256];
extern u4 oamaddr;
extern u2 cgaddr, latchx, latchy;
extern u1* wramdata;

/* The mode 7 multiply is deferred until a result register is read. mode7B's
   high byte is the signed multiplier; the 32-bit product is kept as 24 bits. */
static void checkmultchange(void)
{
    if (multchange) {
        s4 const p = (s2)mode7A * (s1)(u1)(mode7B >> 8);

        compmult[0] = (u1)p;
        compmult[1] = (u1)(p >> 8);
        compmult[2] = (u1)(p >> 16);
        multchange = 0;
    }
}

REGABI_REG_READ8(reg2100r);
u1 c_reg2100r(void)
{
    return (u1)(vidbright | forceblnk); /* should be open bus */
}

REGABI_REG_READ8(reg2134r); /* multiply result, low */
u1 c_reg2134r(void)
{
    checkmultchange();
    return compmult[0];
}

REGABI_REG_READ8(reg2135r); /* multiply result, middle */
u1 c_reg2135r(void)
{
    checkmultchange();
    return compmult[1];
}

REGABI_REG_READ8(reg2136r); /* multiply result, high */
u1 c_reg2136r(void)
{
    checkmultchange();
    return compmult[2];
}

REGABI_REG_READ8(reg213Er); /* PPU status: bit 0 is always set */
u1 c_reg213Er(void)
{
    return (u1)(1 | rtoflags);
}

/* PPU status and version, plus the NTSC/PAL bit. Reading it clears the
   software latch and updates PPU2's own bus latch. */
REGABI_REG_READ8(reg213Fr);
u1 c_reg213Fr(void)
{
    u1 al = (u1)((u1)(romispal << 4) | ppustatus | cfield);

    latchxr = 0;
    latchyr = 0;
    al |= extlatch;
    ppu2_mdr = al;
    return al;
}

/* WRAM through the $2180 port; the address wraps at 128K. */
REGABI_REG_READ8(reg2180r);
u1 c_reg2180r(void)
{
    u1 const al = wramdata[wramrwadr];

    wramrwadr = (wramrwadr + 1) & 0x1FFFFu;
    return al;
}

/* Unknown register, used by a test cart. 21C2 and 21C3 are the same code. */
REGABI_REG_READ8(reg21C2r);
u1 c_reg21C2r(void) { return 0x21; }

REGABI_REG_READ8(reg21C3r);
u1 c_reg21C3r(void) { return 0x21; }

/* 420A-420F: should be open bus, but the assembly returns zero. */
REGABI_REG_READ8(reg420Ar);
u1 c_reg420Ar(void) { return 0; }

REGABI_REG_READ8(reg420Br);
u1 c_reg420Br(void) { return 0; }

REGABI_REG_READ8(reg420Cr);
u1 c_reg420Cr(void) { return 0; }

REGABI_REG_READ8(reg420Dr);
u1 c_reg420Dr(void) { return 0; }

REGABI_REG_READ8(reg420Er);
u1 c_reg420Er(void) { return 0; }

REGABI_REG_READ8(reg420Fr);
u1 c_reg420Fr(void) { return 0; }

/* NMI flag in bit 7, CPU revision in bits 0-3, open bus between. Bit 7 comes
   from NMIEnab, and reading re-arms it unless we are inside the handler. */
REGABI_REG_READ8(reg4210r);
u1 c_reg4210r(void)
{
    u1 const al = (u1)((cpu_mdr & 0x70u) | 0x02u | (NMIEnab & 0x80u));

    if (curnmi == 0) {
        NMIEnab = 1;
    }
    curnmi = 0;
    return al;
}

/* IRQ flag in bit 7, open bus below; reading clears it. */
REGABI_REG_READ8(reg4211r);
u1 c_reg4211r(void)
{
    u1 const al = (u1)((cpu_mdr & 0x7Fu) | irqon);

    irqon = 0;
    return al;
}

REGABI_REG_READ8(reg4213r); /* programmable I/O port */
u1 c_reg4213r(void) { return ioportval; }

REGABI_REG_READ8(reg4214r); /* divide quotient, low */
u1 c_reg4214r(void) { return (u1)divres; }

REGABI_REG_READ8(reg4215r); /* divide quotient, high */
u1 c_reg4215r(void) { return (u1)(divres >> 8); }

REGABI_REG_READ8(reg4216r); /* multiply product / divide remainder, low */
u1 c_reg4216r(void) { return (u1)multres; }

REGABI_REG_READ8(reg4217r); /* multiply product / divide remainder, high */
u1 c_reg4217r(void) { return (u1)(multres >> 8); }

/* $4218-$421F return the joypad snapshots latched at the start of vblank, not
   live input; each pad's two bytes are the top half of its dword. */
REGABI_REG_READ8(reg4218r);
u1 c_reg4218r(void) { return (u1)(JoyARead >> 16); }

REGABI_REG_READ8(reg4219r);
u1 c_reg4219r(void) { return (u1)(JoyARead >> 24); }

REGABI_REG_READ8(reg421Ar);
u1 c_reg421Ar(void) { return (u1)(JoyBRead >> 16); }

REGABI_REG_READ8(reg421Br);
u1 c_reg421Br(void) { return (u1)(JoyBRead >> 24); }

REGABI_REG_READ8(reg421Cr);
u1 c_reg421Cr(void) { return (u1)(JoyDRead >> 16); }

REGABI_REG_READ8(reg421Dr);
u1 c_reg421Dr(void) { return (u1)(JoyDRead >> 24); }

REGABI_REG_READ8(reg421Er);
u1 c_reg421Er(void) { return (u1)(JoyCRead2 >> 16); }

REGABI_REG_READ8(reg421Fr);
u1 c_reg421Fr(void) { return (u1)(JoyCRead2 >> 24); }

/* OAM read port. The address is a word inside a dword, and OAM is 544 bytes,
   so it wraps one past the end rather than at a power of two. */
REGABI_REG_READ8(reg2138r);
u1 c_reg2138r(void)
{
    u1 const al = oamram[oamaddr & 0xFFFFu];
    u2 const next = (u2)((oamaddr & 0xFFFFu) + 1u);

    oamaddr = (oamaddr & ~0xFFFFu) | (next > 543u ? 0u : next);
    return al;
}

/* CGRAM read port; the address is 9 bits, indexing bytes of the palette. */
REGABI_REG_READ8(reg213Br);
u1 c_reg213Br(void)
{
    u1 const al = ((u1 const*)cgram)[cgaddr];

    cgaddr = (u2)((cgaddr + 1u) & 0x1FFu);
    return al;
}

/* H and V counter latches. The first read returns the low byte, the second
   bit 0 of the high byte with the other seven from PPU2's bus latch; either
   way the latch value becomes the new bus latch. */
static u1 counter_latch(u2 const v, u1* const phase)
{
    u1 al;

    if (*phase == 1) {
        al = (u1)((ppu2_mdr & 0xFEu) | ((v >> 8) & 1u));
        *phase = 0;
    } else {
        al = (u1)v;
        *phase = 1;
    }
    ppu2_mdr = al;
    return al;
}

REGABI_REG_READ8(reg213Cr);
u1 c_reg213Cr(void) { return counter_latch(latchx, &latchxr); }

REGABI_REG_READ8(reg213Dr);
u1 c_reg213Dr(void) { return counter_latch(latchy, &latchyr); }
