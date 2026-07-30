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
extern u1 winl1, winr1, winl2, winr2, winlogica, winlogicb;
extern u1 winenabm, winenabs, scaddset, scaddtype, INTEnab, multa;
extern u2 scrnon, diva;
extern u1 bgscrolPrev, vramread;
extern u2 bg1scrolx, bg2scrolx, bg3scrolx, bg4scrolx;
extern u2 bg1scroly, bg2scroly, bg3scroly, bg4scroly;
extern u2 bg1scrolx_m7, bg1scroly_m7;
extern u2 mode7C, mode7D, mode7X0, mode7Y0;
extern u1 dmadata[129], hdmarestart, nohdmaframe, hdmadelay, SPC7110Enable;
extern u2 resolutn, curypos;
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

/* --- PPU writes ported from cpu/regsw.inc -------------------------------- *
 *
 * Same ABI the other way: al carries the value. Where the target is wider than
 * a byte the assembly stores only its low byte, so the rest has to survive.
 */
#define REG_WRITE_BYTE(reg, target)                                           \
    REGABI_REG_WRITE8(reg);                                                   \
    void c_##reg(u1 const al) { target = al; }

REG_WRITE_BYTE(reg2126w, winl1) /* window 1 left */
REG_WRITE_BYTE(reg2127w, winr1) /* window 1 right */
REG_WRITE_BYTE(reg2128w, winl2) /* window 2 left */
REG_WRITE_BYTE(reg2129w, winr2) /* window 2 right */
REG_WRITE_BYTE(reg212Aw, winlogica)
REG_WRITE_BYTE(reg212Bw, winlogicb)
REG_WRITE_BYTE(reg212Ew, winenabm)
REG_WRITE_BYTE(reg212Fw, winenabs)
REG_WRITE_BYTE(reg2130w, scaddset)
REG_WRITE_BYTE(reg2131w, scaddtype)
REG_WRITE_BYTE(reg4200w, INTEnab)
REG_WRITE_BYTE(reg4202w, multa)

#undef REG_WRITE_BYTE

REGABI_REG_WRITE8(reg212Cw); /* main screen enable; the word's high half is
                                the sub screen, written by 212D */
void c_reg212Cw(u1 const al) { scrnon = (u2)((scrnon & 0xFF00u) | al); }

REGABI_REG_WRITE8(reg2181w); /* WRAM address, low byte of a 17-bit counter */
void c_reg2181w(u1 const al) { wramrwadr = (wramrwadr & ~0xFFu) | al; }

REGABI_REG_WRITE8(reg4204w); /* dividend, low */
void c_reg4204w(u1 const al) { diva = (u2)((diva & 0xFF00u) | al); }

/* The scroll registers take two byte writes. The low byte of the pair is
   whatever the previous write to *any* scroll register left in bgscrolPrev -
   the SNES shares one latch across all of them. */
static u2 scroll_y(u1 const al)
{
    u2 const v = (u2)(((u2)al << 8) | bgscrolPrev);

    bgscrolPrev = al;
    return v;
}

/* The horizontal ones keep the low three bits of the current value instead of
   taking them from the latch. The assembly does it by shifting the whole 32-bit
   ebx left 13, patching bh, and shifting back; the caller's upper half rides
   along but lands above bit 15 and is dropped by the 16-bit store. */
static u2 scroll_x(u2 const cur, u1 const al)
{
    u4 ebx = (u4)(((u4)al << 8) | bgscrolPrev) << 13;

    ebx = (ebx & ~0xFF00u) | ((u4)(u1)((cur >> 8) << 5) << 8);
    bgscrolPrev = al;
    return (u2)(ebx >> 13);
}

#define REG_SCROLL_X(reg, target)                                             \
    REGABI_REG_WRITE8(reg);                                                   \
    void c_##reg(u1 const al) { target = scroll_x(target, al); }

#define REG_SCROLL_Y(reg, target)                                             \
    REGABI_REG_WRITE8(reg);                                                   \
    void c_##reg(u1 const al) { target = scroll_y(al); }

REGABI_REG_WRITE8(reg210Dw); /* BG1 horizontal, mirrored for mode 7 */
void c_reg210Dw(u1 const al)
{
    bg1scrolx = scroll_x(bg1scrolx, al);
    bg1scrolx_m7 = bg1scrolx;
}

REGABI_REG_WRITE8(reg210Ew); /* BG1 vertical, mirrored for mode 7 */
void c_reg210Ew(u1 const al)
{
    bg1scroly = scroll_y(al);
    bg1scroly_m7 = bg1scroly;
}

REG_SCROLL_X(reg210Fw, bg2scrolx)
REG_SCROLL_Y(reg2110w, bg2scroly)
REG_SCROLL_X(reg2111w, bg3scrolx)
REG_SCROLL_Y(reg2112w, bg3scroly)
REG_SCROLL_X(reg2113w, bg4scrolx)
REG_SCROLL_Y(reg2114w, bg4scroly)

#undef REG_SCROLL_X
#undef REG_SCROLL_Y

/* The mode 7 matrix registers are a two-write pair as well, but they keep
   their own previous byte rather than a shared latch. */
static u2 mode7_reg(u2 const cur, u1 const al)
{
    return (u2)(((u2)al << 8) | ((cur >> 8) & 0xFFu));
}

REGABI_REG_WRITE8(reg211Bw); /* A: also arms the deferred multiply */
void c_reg211Bw(u1 const al)
{
    mode7A = mode7_reg(mode7A, al);
    multchange = 1;
}

REGABI_REG_WRITE8(reg211Cw); /* B: same */
void c_reg211Cw(u1 const al)
{
    mode7B = mode7_reg(mode7B, al);
    multchange = 1;
}

REGABI_REG_WRITE8(reg211Dw);
void c_reg211Dw(u1 const al) { mode7C = mode7_reg(mode7C, al); }

REGABI_REG_WRITE8(reg211Ew);
void c_reg211Ew(u1 const al) { mode7D = mode7_reg(mode7D, al); }

REGABI_REG_WRITE8(reg211Fw);
void c_reg211Fw(u1 const al) { mode7X0 = mode7_reg(mode7X0, al); }

REGABI_REG_WRITE8(reg2120w);
void c_reg2120w(u1 const al) { mode7Y0 = mode7_reg(mode7Y0, al); }

/* CGRAM address: a palette entry index, doubled to a byte address. The 9-bit
   mask is what the assembly does and can never bite - a byte doubled is at most
   0x1FE - but keep it rather than rely on that. */
REGABI_REG_WRITE8(reg2121w);
void c_reg2121w(u1 const al) { cgaddr = (u2)(((u2)al << 1) & 0x1FFu); }

REGABI_REG_WRITE8(reg212Dw); /* sub-screen enable: the high half of scrnon */
void c_reg212Dw(u1 const al)
{
    scrnon = (u2)((scrnon & 0x00FFu) | ((u2)al << 8));
}

REGABI_REG_WRITE8(reg2182w); /* WRAM address, middle byte */
void c_reg2182w(u1 const al)
{
    wramrwadr = (wramrwadr & ~0xFF00u) | ((u4)al << 8);
}

REGABI_REG_WRITE8(reg4205w); /* dividend, high */
void c_reg4205w(u1 const al) { diva = (u2)((diva & 0x00FFu) | ((u2)al << 8)); }

/* WRAM write port; the mirror of reg2180r. */
REGABI_REG_WRITE8(reg2180w);
void c_reg2180w(u1 const al)
{
    wramdata[wramrwadr] = al;
    wramrwadr = (wramrwadr + 1) & 0x1FFFFu;
}

/* --- the $43xx DMA registers --------------------------------------------- *
 *
 * These need the address, so they use the BANK trampolines (address in ecx,
 * value in al) rather than the REG ones. The index is a 16-bit subtract, so
 * it wraps inside cx before being zero-extended.
 */
static u4 dma_index(u4 const addr)
{
    return (u2)(addr - 0x4300u);
}

#define REG_DMA_STORE(reg)                                                    \
    REGABI_BANK_WRITE8(reg);                                                  \
    void c_##reg(u4 const addr, u1 const al) { dmadata[dma_index(addr)] = al; }

REG_DMA_STORE(reg43x2w) /* source address, low */
REG_DMA_STORE(reg43x3w) /* source address, high */
REG_DMA_STORE(reg43x4w) /* source bank */
REG_DMA_STORE(reg43x5w) /* transfer size / HDMA address, low */
REG_DMA_STORE(reg43x6w) /* transfer size / HDMA address, high */
REG_DMA_STORE(reg43x7w) /* transfer size / HDMA address, bank */
REG_DMA_STORE(reg43x8w) /* A-bus table address, low */
REG_DMA_STORE(reg43x9w) /* A-bus table address, high */
REG_DMA_STORE(reg43XBw) /* unknown DMA byte */

#undef REG_DMA_STORE

/* The control and destination registers restart HDMA for the frame. */
REGABI_BANK_WRITE8(reg43X0w);
void c_reg43X0w(u4 const addr, u1 const al)
{
    dmadata[dma_index(addr)] = al;
    hdmarestart = 1;
}

REGABI_BANK_WRITE8(reg43X1w);
void c_reg43X1w(u4 const addr, u1 const al)
{
    dmadata[dma_index(addr)] = al;
    hdmarestart = 1;
}

/* Line count. Setting a non-zero count at or past the last visible line means
   this frame gets no HDMA at all, and the next one starts late. */
REGABI_BANK_WRITE8(reg43XAw);
void c_reg43XAw(u4 const addr, u1 const al)
{
    nohdmaframe = 0;
    dmadata[dma_index(addr)] = al;
    if (curypos >= resolutn && al != 0) {
        nohdmaframe = 1;
        hdmadelay++;
    }
}

REGABI_BANK_READ8(reg43XXr);
u1 c_reg43XXr(u4 const addr) { return dmadata[dma_index(addr)]; }

/* Anything with no handler. Below $2100 there is no open bus to return, and
   an SPC7110 cart clears it too - the assembly falls through into the clear
   rather than round it, which reads backwards but is what it does. */
REGABI_BANK_READ8(regINVALID);
u1 c_regINVALID(u4 const addr)
{
    return (addr & 0xFFFFu) >= 0x2100u && SPC7110Enable == 0 ? cpu_mdr : 0;
}

REGABI_REG_WRITE8(regINVALIDw);
void c_regINVALIDw(u1 const al) { (void)al; }
