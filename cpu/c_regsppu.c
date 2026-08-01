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
extern u1 NextLineCache, prevoamptr, oamlow, nexthprior, nosprincr, objhipr;
extern u4 objptr, objptrn;
extern u1 objsize1, objsize2, objmovs1, objmovs2;
extern u2 objadds1, objadds2, oamaddrs, poamaddrs;
extern u1 reg2101w_objsize1[8], reg2101w_objsize2[8];
extern u1 reg2101w_objmovs1[8], reg2101w_objmovs2[8];
extern u2 reg2101w_objadds1[8], reg2101w_objadds2[8];
extern u1 bgmode, bg3highst, bgtilesz, mosaicon, mosaicsz;
extern u1 BG116x16t, BG216x16t, BG316x16t, BG416x16t;
extern u2 bg1ptr, bg2ptr, bg3ptr, bg4ptr;
extern u2 bg1ptrb, bg2ptrb, bg3ptrb, bg4ptrb;
extern u2 bg1ptrc, bg2ptrc, bg3ptrc, bg4ptrc;
extern u2 bg1ptrd, bg2ptrd, bg3ptrd, bg4ptrd;
extern u4 bg1ptrx, bg2ptrx, bg3ptrx, bg4ptrx;
extern u4 bg1ptry, bg2ptry, bg3ptry, bg4ptry;
extern u1 bg1scsize, bg2scsize, bg3scsize, bg4scsize;
extern u2 bg1objptr, bg2objptr, bg3objptr, bg4objptr;
extern u1 cgmod, winbg1en, winbg2en, winbg3en, winbg4en, winobjen, wincolen;
extern u1 coladdr, coladdg, coladdb, interlval;
extern u1 iohvlatch, MultiTapStat, JoyCRead;
extern u4 JoyAOrig, JoyBOrig, JoyCOrig, JoyDOrig, JoyEOrig;
extern u4 JoyANow, JoyBNow, JoyCNow, JoyDNow, JoyENow;
extern u4 vramaddr;
extern u1 vramread2, mode7set;
extern u1* vram;
extern u1 vrama[65536];
extern u1 vidmemch2[4096], vidmemch4[4096], vidmemch8[4096];
extern u1 vramincby8left, vramincby8totl, vraminctype, vramincby8on, vramincr;
extern u1 vramincby8rowl;
extern u1 nssdip1, nssdip2, nssdip3, nssdip4, nssdip5, nssdip6;
extern u2 RumbleData;
extern u1 MultiTap, device2, hblank;
extern u4 nmistatus;
extern void (*regptwa[0x3000])(void);
void reg2118(void), reg2118inc(void), reg2118inc8(void), reg2118inc8inc(void);
void reg2119(void), reg2119inc(void), reg2119inc8(void), reg2119inc8inc(void);
extern u2 vramincby8var, vramincby8ptri, addrincr;
extern u2 HIRQLoc, VIRQLoc, totlines;
extern u4 HIRQCycNext;
extern u1 HIRQNextExe;
extern u1 cycpl, cycphb, xirqb, cycpblt;
extern u1 opexec268, opexec268cph, opexec358, opexec358cph, cycpb268, cycpb358;

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

REGABI_REG_WRITE8(reg2100w); /* brightness in bits 0-3, force blank in bit 7 */
void c_reg2100w(u1 const al)
{
    vidbright = (u1)(al & 0x0Fu);
    forceblnk = (u1)(al & 0x80u);
}

/* Sprite size and name base. The six tables are indexed by the size select in
   bits 5-7; objptrn adds the name-select offset on top of the base. Repeating a
   value is skipped entirely, so the line cache is not invalidated either. */
REGABI_REG_WRITE8(reg2101w);
void c_reg2101w(u1 const al)
{
    u4 const sel = al >> 5;
    u2 const base = (u2)((u2)(al & 0x03u) << 14);
    u2 const name = (u2)((u2)((al & 0x18u) >> 3) << 13);

    if (prevoamptr != 0xFFu && prevoamptr == al) {
        return;
    }
    prevoamptr = al;

    objptr = (objptr & ~0xFFFFu) | base;
    objptrn = (objptrn & ~0xFFFFu) | (u2)(base + name);

    NextLineCache = 1;
    objsize1 = reg2101w_objsize1[sel];
    objsize2 = reg2101w_objsize2[sel];
    objmovs1 = reg2101w_objmovs1[sel];
    objmovs2 = reg2101w_objmovs2[sel];
    objadds1 = reg2101w_objadds1[sel];
    objadds2 = reg2101w_objadds2[sel];
}

/* $2103 bit 7 makes sprite priority rotate from the OAM address; objhipr is
   that address as a sprite index. */
static u1 obj_hipri(void)
{
    return (u1)(((u2)oamaddr >> 2) & 0x7Fu);
}

/* OAM address, low byte. The address is kept doubled - it is a byte address
   into OAM while the register counts words - so it is halved, patched and
   doubled again. */
REGABI_REG_WRITE8(reg2102w);
void c_reg2102w(u1 const al)
{
    u2 bx;

    poamaddrs = oamaddrs;
    bx = (u2)((u2)(((oamaddrs >> 1) & 0xFF00u) | al) << 1);
    oamaddr = (oamaddr & ~0xFFFFu) | bx;
    oamaddrs = bx;
    objhipr = (u1)(nexthprior == 1 ? obj_hipri() : 0);
    NextLineCache = 1;
}

/* OAM address, high bit plus the priority-rotation flag. Writing the pair when
   the previous address was past the sprite table keeps the old address and
   stops the auto-increment - what a real PPU does to the OAM address latch. */
REGABI_REG_WRITE8(reg2103w);
void c_reg2103w(u1 const al)
{
    u2 bx = (u2)((u2)(((oamaddrs >> 1) & 0x00FFu) | ((u2)(al & 1u) << 8)) << 1);

    if (poamaddrs > 0x200u && bx == 0x200u) {
        bx = poamaddrs;
        nosprincr = 1;
    }
    oamaddr = (oamaddr & ~0xFFFFu) | bx;
    oamaddrs = bx;
    if (al & 0x80u) {
        objhipr = obj_hipri();
        nexthprior = 1;
    } else {
        nexthprior = 0; /* objhipr keeps its old value here */
    }
    NextLineCache = 1;
}

/* OAM data. The low table is written a word at a time: an even address only
   latches the byte, the odd one flushes both. The high table (bit 9) takes
   single bytes. Running past the end of OAM leaves the byte in the latch. */
REGABI_REG_WRITE8(reg2104w);
void c_reg2104w(u1 const al)
{
    u4 const ebx = oamaddr;

    NextLineCache = 1;
    if (nosprincr != 1) {
        oamaddr = ebx + 1;
        if (ebx >= 544u) {
            oamaddr = 1;
            oamlow = al;
            return;
        }
    }
    if (ebx & 0x200u) {
        oamram[ebx] = al;
    } else if (ebx & 1u) {
        oamram[ebx] = al;
        oamram[ebx - 1] = oamlow;
    } else {
        oamlow = al;
    }
}

/* Screen mode: BG mode in bits 0-2, BG3 priority in bit 3, and one 16x16 tile
   flag per BG in bits 4-7. */
REGABI_REG_WRITE8(reg2105w);
void c_reg2105w(u1 const al)
{
    bgmode = (u1)(al & 0x07u);
    bg3highst = (u1)((al >> 3) & 0x01u);
    bgtilesz = (u1)(al >> 4);
    BG116x16t = (u1)((al >> 4) & 1u);
    BG216x16t = (u1)((al >> 5) & 1u);
    BG316x16t = (u1)((al >> 6) & 1u);
    BG416x16t = (u1)((al >> 7) & 1u);
}

REGABI_REG_WRITE8(reg2106w); /* mosaic: per-BG enables low, size high */
void c_reg2106w(u1 const al)
{
    mosaicon = (u1)(al & 0x0Fu);
    mosaicsz = (u1)(al >> 4);
}

/* BG tilemap base and size. The four pointers are the map's quadrants; the
   size bits say which of them are offset, and ptrx/ptry are the same offsets
   as plain distances. */
static void bg_tilemap(u1 const al, u2* const p, u4* const px, u4* const py,
                       u1* const scsize)
{
    u2 const base = (u2)((u2)(u1)(al >> 2) << 11);
    u1 const sz = (u1)(al & 0x03u);

    p[0] = base;
    p[1] = base;
    p[2] = base;
    p[3] = base;
    *px = 0;
    *py = 0;
    *scsize = sz;
    if (sz == 1) {
        p[1] = (u2)(p[1] + 0x800u);
        p[3] = (u2)(p[3] + 0x800u);
        *px = 0x800;
    }
    if (sz == 2) {
        p[2] = (u2)(p[2] + 0x800u);
        p[3] = (u2)(p[3] + 0x800u);
        *py = 0x800;
    }
    if (sz == 3) {
        p[1] = (u2)(p[1] + 0x800u);
        p[2] = (u2)(p[2] + 0x1000u);
        p[3] = (u2)(p[3] + 0x1800u);
        *px = 0x800;
        *py = 0x1000;
    }
}

/* bgNptr, bgNptrb, bgNptrc and bgNptrd are one per-BG group but are stored
   interleaved across the BGs, so the quadrants have to be gathered by hand. */
#define REG_BG_TILEMAP(reg, n)                                                \
    REGABI_REG_WRITE8(reg);                                                   \
    void c_##reg(u1 const al)                                                 \
    {                                                                         \
        u2 p[4] = { bg##n##ptr, bg##n##ptrb, bg##n##ptrc, bg##n##ptrd };      \
                                                                              \
        bg_tilemap(al, p, &bg##n##ptrx, &bg##n##ptry, &bg##n##scsize);        \
        bg##n##ptr = p[0];                                                    \
        bg##n##ptrb = p[1];                                                   \
        bg##n##ptrc = p[2];                                                   \
        bg##n##ptrd = p[3];                                                   \
    }

REG_BG_TILEMAP(reg2107w, 1)
REG_BG_TILEMAP(reg2108w, 2)
REG_BG_TILEMAP(reg2109w, 3)
REG_BG_TILEMAP(reg210Aw, 4)

#undef REG_BG_TILEMAP

/* BG character base: two 4-bit nibbles, each an 8K step. The shift is 16-bit,
   so a nibble of 8 or more wraps rather than overflowing. */
REGABI_REG_WRITE8(reg210Bw);
void c_reg210Bw(u1 const al)
{
    bg1objptr = (u2)((u2)(al & 0x0Fu) << 13);
    bg2objptr = (u2)((u2)(al >> 4) << 13);
}

REGABI_REG_WRITE8(reg210Cw);
void c_reg210Cw(u1 const al)
{
    bg3objptr = (u2)((u2)(al & 0x0Fu) << 13);
    bg4objptr = (u2)((u2)(al >> 4) << 13);
}

/* Rewriting the same value does not dirty the cache; the address still moves. */
REGABI_REG_WRITE8(reg2122w);
void c_reg2122w(u1 const al)
{
    u1* const cg = (u1*)cgram;

    if (cg[cgaddr] != al) {
        cg[cgaddr] = al;
        cgmod = 1;
    }
    cgaddr = (u2)((cgaddr + 1u) & 0x1FFu);
}

/* One nibble per layer. The asm branches on bits 1 and 3, but both arms are
   the same - the `or bl,02h` they guarded is commented out. */
#define REG_WIN_SEL(reg, lo, hi)                                               \
    REGABI_REG_WRITE8(reg);                                                    \
    void c_##reg(u1 const al)                                                  \
    {                                                                          \
        lo = (u1)(al & 0x0Fu);                                                 \
        hi = (u1)(al >> 4);                                                    \
    }

REG_WIN_SEL(reg2123w, winbg1en, winbg2en)
REG_WIN_SEL(reg2124w, winbg3en, winbg4en)
REG_WIN_SEL(reg2125w, winobjen, wincolen)

#undef REG_WIN_SEL

/* Bits 5-7 pick which channels take the intensity in bits 0-4. */
REGABI_REG_WRITE8(reg2132w);
void c_reg2132w(u1 const al)
{
    u1 const v = (u1)(al & 0x1Fu);

    if (al & 0x20u) {
        coladdr = v;
    }
    if (al & 0x40u) {
        coladdg = v;
    }
    if (al & 0x80u) {
        coladdb = v;
    }
}

/* Keeps only interlace and pseudo-hires; bit 2 is the 239-line switch. */
REGABI_REG_WRITE8(reg2133w);
void c_reg2133w(u1 const al)
{
    interlval = (u1)(al & 0x43u);
    resolutn = (u2)((al & 0x04u) ? 239 : 224);
}

REGABI_REG_WRITE8(reg2183w); /* WRAM address, bank bit - the 17th address bit */
void c_reg2183w(u1 const al)
{
    wramrwadr = (wramrwadr & ~0x00FF0000u) | ((u4)(al & 0x01u) << 16);
}

/* Bit 7 edges drive the H/V counter latch, and reach the multitap. */
REGABI_REG_WRITE8(reg4201w);
void c_reg4201w(u1 const al)
{
    if (iohvlatch == 1 && !(al & 0x80u)) {
        iohvlatch = 0;
    }
    if (!(ioportval & 0x80u) && (al & 0x80u)) {
        iohvlatch = 1;
    }
    ioportval = al;
    MultiTapStat = (u1)((MultiTapStat & 0x7Fu) | (al & 0x80u));
}

/* Writing runs the 8x8 multiply; only the low half is kept. */
REGABI_REG_WRITE8(reg4203w);
void c_reg4203w(u1 const al) { multres = (u2)((u2)al * (u2)multa); }

/* The 16/8 divide runs on the write. The quotient always fits; only the
   by-zero case is special. */
REGABI_REG_WRITE8(reg4206w);
void c_reg4206w(u1 const al)
{
    if (al == 0) {
        divres = 0xFFFFu;
        multres = diva;
    } else {
        divres = (u2)(diva / al);
        multres = (u2)(diva % al);
    }
}

/* Bit 0 picks the 3.58MHz timings; xirqb bit 7 is the IRQ start bank. The asm
   leaves its value in al, which the write ABI does not promise. */
REGABI_REG_WRITE8(reg420Dw);
void c_reg420Dw(u1 const al)
{
    if (al & 0x01u) {
        cycpl = opexec358;
        cycphb = opexec358cph;
        xirqb |= 0x80u;
        cycpblt = cycpb358;
    } else {
        cycpl = opexec268;
        cycphb = opexec268cph;
        xirqb = 0;
        cycpblt = cycpb268;
    }
}

/* The address is kept doubled, so halve it, patch the byte in, double again.
   Only the low 16 bits are the address. */
static void vram_addr_byte(u2 const mask, u2 const bits)
{
    u2 v = (u2)((u2)vramaddr >> 1);

    v = (u2)((v & mask) | bits);
    vramaddr = (vramaddr & ~0xFFFFu) | (u2)(v << 1);
    vramread = 0;
}

REGABI_REG_WRITE8(reg2116w);
void c_reg2116w(u1 const al) { vram_addr_byte(0xFF00u, al); }

/* Also prefetches into the read latches that $2139/$213A hand back. */
REGABI_REG_WRITE8(reg2117w);
void c_reg2117w(u1 const al)
{
    u2 a;

    vram_addr_byte(0x00FFu, (u2)((u2)al << 8));
    a = (u2)vramaddr;
    vramread = vram[a];
    vramread2 = vram[a + 1u];
}

REGABI_REG_WRITE8(reg211Aw); /* mode 7 settings */
void c_reg211Aw(u1 const al) { mode7set = al; }

/* VRAM data. vram points at vrama (ui.c), so the asm's two addressing routes
   are the same buffer. */
static void vram_dirty(u4 const off)
{
    u4 const i = off >> 4;

    vidmemch2[i] = 1;
    vidmemch4[i] = 1;
    vidmemch8[i] = 1;
}

static void vram_bump(void)
{
    vramaddr = (vramaddr & ~0xFFFFu) | (u2)((u2)vramaddr + addrincr);
}

/* The masks are 16-bit but the shift is 32-bit, so the top half of vramaddr
   survives the AND and takes part in the shift. Untestable: a non-zero top
   half puts the unshifted ptri term past 64K, which the asm reads OOB too. */
static u4 vram_inc8_off(void)
{
    u4 off = (vramaddr & vramincby8left) << 3;

    off += ((vramaddr & ~0xFFFFu) | (u2)(vramaddr & vramincby8var))
        >> (vramincby8totl & 31u);
    return off + ((vramaddr & ~0xFFFFu) | (u2)(vramaddr & vramincby8ptri));
}

static void vram_write(u4 const off, u4 const lohi, u1 const al)
{
    vrama[off + lohi] = al;
    vram_dirty(off);
}

#define REG_VRAM_DATA(reg, offexpr, lohi, bump)                                   REGABI_REG_WRITE8(reg);                                                       void c_##reg(u1 const al)                                                     {                                                                                 vram_write((offexpr), (lohi), al);                                            bump                                                                      }

REG_VRAM_DATA(reg2118, vramaddr, 0, )
REG_VRAM_DATA(reg2118inc, vramaddr, 0, vram_bump();)
REG_VRAM_DATA(reg2118inc8, vram_inc8_off(), 0, )
REG_VRAM_DATA(reg2118inc8inc, vram_inc8_off(), 0, vram_bump();)
REG_VRAM_DATA(reg2119, vramaddr, 1, )
REG_VRAM_DATA(reg2119inc, vramaddr, 1, vram_bump();)
REG_VRAM_DATA(reg2119inc8, vram_inc8_off(), 1, )
REG_VRAM_DATA(reg2119inc8inc, vram_inc8_off(), 1, vram_bump();)

#undef REG_VRAM_DATA

/* VRAM increment control. Bits 0-1 pick the step, bits 2-3 the address
   remapping, and bit 7 whether $2118 or $2119 is the one that increments -
   which is done by swapping the table entries rather than branching. */
REGABI_REG_WRITE8(reg2115w);
void c_reg2115w(u1 const al)
{
    static u2 const step[4] = { 2, 64, 256, 256 };
    u1 const remap = (u1)(al & 0x0Cu);

    vraminctype = al;
    addrincr = step[al & 3u];

    vramincby8on = (u1)(remap ? 1 : 0);
    if (remap == 4) {
        vramincby8left = 64 - 1;
        vramincby8totl = 5;
        vramincby8ptri = 65535 - 511;
        vramincby8var = 256 + 128 + 64;
    } else if (remap == 8) {
        vramincby8left = 128 - 1;
        vramincby8totl = 6;
        vramincby8ptri = 65535 - 1023;
        vramincby8var = 512 + 256 + 128;
    } else if (remap == 12) {
        vramincby8left = 256 - 1;
        vramincby8totl = 7;
        vramincby8ptri = 65535 - 2047;
        vramincby8var = 1024 + 512 + 256;
    }

    vramincr = (u1)((al & 0x80u) ? 0 : 1);
    if (remap) {
        regptwa[0x118] = (al & 0x80u) ? reg2118inc8 : reg2118inc8inc;
        regptwa[0x119] = (al & 0x80u) ? reg2119inc8inc : reg2119inc8;
    } else {
        regptwa[0x118] = (al & 0x80u) ? reg2118 : reg2118inc;
        regptwa[0x119] = (al & 0x80u) ? reg2119inc : reg2119;
    }
}

/* Joypad strobe. The low 16 bits are forced high - the shift register reads
   as all ones once the real data has been clocked out. */
static void joy_latch_ports(void)
{
    JoyANow = JoyAOrig | 0xFFFFu;
    JoyBNow = JoyBOrig | 0xFFFFu;
    JoyCNow = JoyCOrig | 0xFFFFu;
    JoyDNow = JoyDOrig | 0xFFFFu;
    JoyENow = JoyEOrig | 0xFFFFu;
}

/* With auto-read off the strobe latches immediately; with it on the ports are
   only re-latched once both halves of the 1-then-0 sequence have been seen. */
REGABI_REG_WRITE8(reg4016w);
void c_reg4016w(u1 const al)
{
    if (!(INTEnab & 1u)) {
        joy_latch_ports();
        MultiTapStat = (u1)(al == 1 ? (MultiTapStat | 1u) : (MultiTapStat & 0xFEu));
        return;
    }
    if (al == 1) {
        MultiTapStat |= 1u;
        JoyCRead |= 2u;
        return;
    }
    MultiTapStat &= 0xFEu;
    if (al == 0) {
        JoyCRead |= 1u;
        if (JoyCRead == 3) {
            joy_latch_ports();
        }
    }
}

/* $2139/$213A hand back the latched byte, prefetch the next, then advance -
   but only on the port that owns the increment ($2115 bit 7). */
static void vram_addr_add(u2 const d)
{
    vramaddr = (vramaddr & ~0xFFFFu) | (u2)((u2)vramaddr + d);
}

static void vram_read_advance(void)
{
    vram_addr_add(addrincr);
    if (vramincby8on != 1 || --vramincby8left != 0) {
        return;
    }
    vram_addr_add(2);
    vramincby8left = vramincby8totl;
    if (--vramincby8rowl == 0) {
        vramincby8rowl = 8;
        vram_addr_add((u2)-16);
    } else {
        vram_addr_add((u2)-vramincby8ptri);
    }
}

REGABI_REG_READ8(reg2139r);
u1 c_reg2139r(void)
{
    u1 const al = vramread;

    vramread = vram[(u2)vramaddr];
    if (vramincr != 0) {
        vram_read_advance();
    }
    return al;
}

REGABI_REG_READ8(reg213Ar);
u1 c_reg213Ar(void)
{
    u1 const al = vramread2;

    vramread2 = vram[(u2)vramaddr + 1u];
    if (vramincr != 1) {
        vram_read_advance();
    }
    return al;
}

/* NSS DIP switches: each contributes its bit only when set to exactly 1. */
REGABI_REG_READ8(reg4100r);
u1 c_reg4100r(void)
{
    return (u1)((nssdip1 == 1 ? 0x01u : 0u) | (nssdip2 == 1 ? 0x02u : 0u)
        | (nssdip3 == 1 ? 0x04u : 0u) | (nssdip4 == 1 ? 0x08u : 0u)
        | (nssdip5 == 1 ? 0x10u : 0u) | (nssdip6 == 1 ? 0x20u : 0u));
}

/* Joypad serial read: one bit per read out of the top of JoyANow, which
   rotates so 16 reads return the shift register and leave it as it was. The
   rumble sentry 0x72 in the high byte freezes the pattern. */
REGABI_REG_READ8(reg4016r);
u1 c_reg4016r(void)
{
    u1 const al = (u1)((JoyANow & 0x80000000u) ? 1u : 0u);

    if (ioportval != 0xFFu) {
        RumbleData = (u2)((RumbleData & 0xFF00u)
            | (u1)((u1)RumbleData | (u1)((ioportval & 0x40u) >> 6)));
        if ((u1)(RumbleData >> 8) != 0x72u) {
            RumbleData = (u2)((RumbleData << 1) | (RumbleData >> 15));
        }
    }
    JoyANow = (JoyANow << 1) | (JoyANow >> 31);
    return al;
}

/* Port 2 serial read. Bit 0 is the pad; with a multitap, MultiTapStat bit 7
   selects which pair of the four is being clocked out, and bit 0 means the tap
   is idle. The base 28 is the open-bus pattern the port reads back. */
REGABI_REG_READ8(reg4017r);
u1 c_reg4017r(void)
{
    u1 al = 28;

    if (device2 != 0 || MultiTap != 1) {
        al |= (u1)((JoyBNow & 0x80000000u) ? 1u : 0u);
        JoyBNow = (JoyBNow << 1) | (JoyBNow >> 31);
        return al;
    }
    if (MultiTapStat & 1u) {
        return (u1)(al | 3u);
    }
    if (MultiTapStat & 0x80u) {
        al |= (u1)((JoyBNow & 0x80000000u) ? 1u : 0u);
        al |= (u1)((JoyCNow & 0x80000000u) ? 2u : 0u);
        JoyBNow = (JoyBNow << 1) | (JoyBNow >> 31);
        JoyCNow = (JoyCNow << 1) | (JoyCNow >> 31);
    } else {
        al |= (u1)((JoyDNow & 0x80000000u) ? 1u : 0u);
        al |= (u1)((JoyENow & 0x80000000u) ? 2u : 0u);
        JoyDNow = (JoyDNow << 1) | (JoyDNow >> 31);
        JoyENow = (JoyENow << 1) | (JoyENow >> 31);
    }
    return al;
}

/* Bit 0 is the auto-joypad poll, busy for three lines from the first vblank
   line; bit 7 is vblank; bit 6 is hblank, which needs the caller's DH. */
REGABI_REG_READ8_DX(reg4212r);
u1 c_reg4212r(u4 const edx)
{
    u2 const first = (u2)(resolutn + 1u);
    u1 al = 0;

    if ((INTEnab & 1u) && curypos >= first && curypos < (u2)(first + 3u)) {
        al |= 0x01u;
    }
    if ((curypos == resolutn && (u1)nmistatus == 2)
        || (curypos >= first && curypos < (u2)(totlines - 1u))) {
        al |= 0x80u;
    }
    hblank = 0;
    if ((u1)(edx >> 8) < cycphb) {
        hblank = 1;
        al |= 0x40u;
    }
    return al;
}

/* --- the IRQ beam-position registers -------------------------------------- *
 *
 * EDX carries the cycle count in DH, hence REGABI_REG_WRITE8_DX. Only the low
 * byte of the HIRQCycNext dword is used; the rest is part of the save state.
 */
static u1 hirq_cyc(void) { return (u1)HIRQCycNext; }

static void hirq_cyc_set(u1 const v)
{
    HIRQCycNext = (HIRQCycNext & ~0xFFu) | v;
}

/* Where the H-IRQ lands, and whether it still fits in the rest of the line. */
static u1 determine_hirq_exec(u1 dh)
{
    u1 const pos = (u1)(((u4)HIRQLoc * cycpl) / 340u);
    u1 const left = (u1)(cycpl - pos);

    dh = (u1)(dh + hirq_cyc());
    hirq_cyc_set(0);
    HIRQNextExe = 0;
    if (dh > left) {
        dh = (u1)(dh - left + 30u);
        hirq_cyc_set((u1)(left + 16u));
        HIRQNextExe = 1;
    }
    return dh;
}

/* Cancel a pending H-IRQ once the beam leaves the V-IRQ line, returning its
   unspent cycles. */
static u1 cancel_hirq(u1 dh)
{
    if (HIRQNextExe == 1 && curypos != VIRQLoc) {
        dh = (u1)(dh + hirq_cyc());
        hirq_cyc_set(0);
        HIRQNextExe = 0;
    }
    return dh;
}

static u4 dx_with_dh(u4 const edx, u1 const dh)
{
    return (edx & ~0xFF00u) | ((u4)dh << 8);
}

/* Recompute only on a real change, and only on the V-IRQ line. */
REGABI_REG_WRITE8_DX(reg4207w);
u4 c_reg4207w(u1 const al, u4 const edx)
{
    if ((u1)HIRQLoc == al) {
        return edx;
    }
    HIRQLoc = (u2)((HIRQLoc & 0xFF00u) | al);
    if (curypos != VIRQLoc) {
        return edx;
    }
    return dx_with_dh(edx, determine_hirq_exec((u1)(edx >> 8)));
}

REGABI_REG_WRITE8_DX(reg4208w);
u4 c_reg4208w(u1 const al, u4 const edx)
{
    if ((u1)(HIRQLoc >> 8) == al) {
        return edx;
    }
    HIRQLoc = (u2)((HIRQLoc & 0x00FFu) | ((u2)al << 8));
    if (curypos != VIRQLoc) {
        return edx;
    }
    return dx_with_dh(edx, determine_hirq_exec((u1)(edx >> 8)));
}

REGABI_REG_WRITE8_DX(reg4209w); /* V-IRQ beam position, low */
u4 c_reg4209w(u1 const al, u4 const edx)
{
    VIRQLoc = (u2)((VIRQLoc & 0xFF00u) | al);
    return dx_with_dh(edx, cancel_hirq((u1)(edx >> 8)));
}

/* Only bit 0 is real; a position past the last line is parked out of range. */
REGABI_REG_WRITE8_DX(reg420Aw);
u4 c_reg420Aw(u1 const al, u4 const edx)
{
    VIRQLoc = (u2)((VIRQLoc & 0x00FFu) | ((u2)(al & 0x01u) << 8));
    if (VIRQLoc >= (u2)(totlines - 1u)) {
        VIRQLoc = 0x7FFFu;
    }
    return dx_with_dh(edx, cancel_hirq((u1)(edx >> 8)));
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
