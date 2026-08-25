/*
 * video/c_ngbg.c - the background dispatchers from video/newgfx16.asm.
 *
 * One per background per pass. Each works out where the layer's tile map and
 * palette are for this scanline, builds the window if the layer has one, and
 * hands the whole register set to the renderer in video/c_ng2tile.c.
 *
 * The assembly reached the renderer by `jmp` after a `push ebx`, which is why
 * every entry point there ended `pop ebx / ret`. Both halves are C now, so the
 * call is a plain one and the pairing is gone.
 *
 * Two things to keep from the original. The `mov eax,[BGPT1+ebx*2]` loads are
 * *dword* reads of word tables, so they pick up the next entry in the high
 * half; for eax that washes out in a later `and eax,0FFFFh`, but bgtxadd is
 * stored as a full dword and keeps it. And the second macro argument is always
 * equal to the first for all sixteen, so the two indexes collapse into one.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"

enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u1 curmosaicsz, BGMA[256], t16x161[], scadsng[256];
extern u1 vidbright, prevbrightdc;
extern u1 winbg1enval[], winlogicaval[];
extern u1 colormodedef[][4]; /* c_vcache.c: SNES mode -> depth per layer */
extern u2 BGPT1[], BGPT1X[], BGPT1Y[], BG1SXl[], BG1SYl[], BGOPT1[];
extern u2 bgtxad[];
extern u4 ng16bprval, ng16bbgval, bgtxadd, ngptrdat[], ngptrdat2;
extern u4 ngceax[], ngcedi[], mode0add, mode0ads, cpalval[256];
extern u4 taddnfy16x16, taddfy16x16, ngwinen, nglogicval, ngwintable[];
extern u4 dcolortab[];
void BuildWindow(u4 line, u4 which); /* video/c_makev16b.c */
extern u1 prdata[], prdatb[], prdatc[];

/* Which priority-flag table each layer marks - BG2 and BG4 share one. The
   entry thunk did this before it spilled the registers, and the priority-1
   pass reads it back to choose a tile row or a single line, so it has to
   happen whether or not the layer then draws anything. */
static u1* const bg_prdat[4] = { prdatb, prdata, prdatc, prdata };
void Gendcolortable(void);
extern u4 ng2_mosaic; /* video/c_ng2tile.c: the renderer wants the mosaic pass */
void c_domosaicng16b(void); /* video/c_ngmosaic.c */

/* video/c_ng2tile.c */
void c_ng_drawtileng2b16b(u4* r);
void c_ng_drawtileng4b16b(u4* r);
void c_ng_drawtileng8b16b(u4* r);
void c_ng_drawtileng16x162b16b(u4* r);
void c_ng_drawtileng16x164b16b(u4* r);
void c_ng_drawtileng16x168b16b(u4* r);

/* A dword read of a word table, which is what the assembly does. */
static u4 dword_at(u2 const* const p)
{
    u4 v;

    memcpy(&v, p, 4);
    return v;
}

/* ProcessBuildWindow. Leaves every register alone - the assembly pushes eax
   and ebx around it - and trims the first window run by how far into the line
   edi already is. */
static void build_window(u4 const* const r, u4 const bg)
{
    u4 const bx = r[R_EBX];
    u4 adj;

    ngwinen = 0;
    if (!(winbg1enval[bx + bg * 256u] & 0x0Au))
        return;

    *(u1*)&nglogicval = (u1)((winlogicaval[bx * 2u] >> (bg * 2u)) & 3u);
    BuildWindow(bx, bx + bg * 256u);

    /* (edi - esi) / 2: the pixels already behind the cursor. */
    adj = (u4)(((s4)(r[R_EDI] - r[R_ESI])) >> 1);
    if (ngwintable[0] == 0)
        ngwintable[1] -= adj;
    else
        ngwintable[0] -= adj;
}

/* The 8x8 and 16x16 forms differ only in which bits of the scroll select the
   row and column, so they are one body with the shifts and masks passed in. */
/* Everything from the depth test on down, which the two passes share: the
   direct-colour palette swap, the window, mode 0's palette block, and the
   jump into the renderer. `edx` arrives holding the colormodedef dword,
   because the renderer is entered with its upper bytes. */
static void bg_finish(u4* const r, u4 const bg, int const big, u4 const edx,
    u4 eax, u4 edi, u4 ebp)
{
    u4 const bx = r[R_EBX];
    u4 const depth = (u1)edx;

    if (depth == 0 || depth > 3)
        return; /* .no8bit: the layer draws nothing at this mode */

    if (depth == 1) {
        mode0add = 0;
    } else if (depth == 3 && (scadsng[bx] & 1u)) {
        /* Direct colour: the palette is the generated table, not the layer's,
           and it is rebuilt when the brightness moved. */
        if (vidbright != prevbrightdc) {
            prevbrightdc = vidbright;
            Gendcolortable();
        }
        ebp = (u4)(uintptr_t)dcolortab;
    }

    r[R_EAX] = eax;
    r[R_EDI] = edi;
    r[R_EBP] = ebp;
    build_window(r, bg);
    if (depth == 1 && BGMA[bx] == 0)
        mode0add = mode0ads; /* mode 0 gives each layer its own palette block */

    r[R_ECX] = bx + bg * 256u;
    r[R_EDX] = (edx & 0xFFFFFF00u) | (1u << bg);

    if (depth == 2)
        (big ? c_ng_drawtileng16x164b16b : c_ng_drawtileng4b16b)(r);
    else if (depth == 1)
        (big ? c_ng_drawtileng16x162b16b : c_ng_drawtileng2b16b)(r);
    else
        (big ? c_ng_drawtileng16x168b16b : c_ng_drawtileng8b16b)(r);

    /* The renderer used to reach the mosaic pass by a tail-jump out of the
       seam in video/newg162.asm. Coming from here there is no seam, so this
       takes it - and c_domosaicng16b reads none of the registers, only the
       mosaic state, so nothing has to be handed over. */
    if (ng2_mosaic)
        c_domosaicng16b();
}

/* The priority-1 pass. It does not work the addresses out again: the pass-0
   dispatcher cached them per scanline in ngceax/ngcedi/ngptrdat/bgtxad and
   this reads them back. Note bgtxadd is restored by a *word* store, so its
   upper half is whatever the earlier pass left there. */
static void bg_tile_pr1(u4* const r, u4 const bg, int const big)
{
    u4 const bx = r[R_EBX];
    u4 const i = bx + bg * 256u;
    u4 edx;

    if (big) {
        u4 const ecx = (dword_at(&BG1SYl[i]) & 0xFFFFu) + bx;

        taddnfy16x16 = 0;
        taddfy16x16 = 16;
        if (ecx & 8u) {
            taddnfy16x16 = 16;
            taddfy16x16 = (u4)-16;
        }
    }

    bgtxadd = (bgtxadd & 0xFFFF0000u) | bgtxad[i];
    ngptrdat2 = ngptrdat[i];
    memcpy(&edx, (u1 const*)colormodedef + (BGMA[bx] & 7u) * 4u + bg, 4);
    bg_finish(r, bg, big, edx, ngceax[i], ngcedi[i], cpalval[bx]);
}

static void bg_tile(u4* const r, u4 const bg, int const big)
{
    u4 const bx = r[R_EBX];
    u4 const i = bx + bg * 256u;
    u4 ecx = r[R_ECX];
    u4 eax, edi, edx, ebp, depth;

    if (big) {
        taddnfy16x16 = 0;
        taddfy16x16 = 16;
        if (ecx & 8u) {
            taddnfy16x16 = 16;
            taddfy16x16 = (u4)-16;
        }
    }

    eax = dword_at(&BGPT1[i]);
    if (ecx & (big ? 0x200u : 0x100u))
        eax += dword_at(&BGPT1Y[i]);
    eax += (ecx * (big ? 4u : 8u)) & 0x7C0u;

    ecx = dword_at(&BG1SXl[i]);
    edx = dword_at(&BGPT1X[i]);
    if (ecx & (big ? 0x200u : 0x100u)) {
        eax += edx;
        /* neg dx: the low word only, the high half rides along untouched. */
        edx = (edx & 0xFFFF0000u) | (u2)(-(s2)(u2)edx);
    }
    edi = ecx;
    edx = (edx & 0xFFFF0000u) | (u2)((u2)edx - 64u);
    ecx &= big ? 0x1F0u : 0xF8u;
    eax &= 0xFFFFu;
    ecx >>= big ? 3 : 2;
    bgtxadd = edx;
    bgtxad[i] = (u2)edx;
    eax += ecx;

    ecx = dword_at(&BGOPT1[i]) & 0xFFFFu;
    edi &= big ? 0xFu : 7u;
    /* A dword read at colormodedef + mode*4 + bg, so edx keeps the three
       neighbouring layers' depths in its upper bytes - and `mov dl,1<<n`
       later replaces only the low one, which is what reaches the renderer. */
    memcpy(&edx, (u1 const*)colormodedef + (BGMA[bx] & 7u) * 4u + bg, 4);
    depth = (u1)edx;
    edi = (u4)(-(s4)edi) * 2u;
    ebp = cpalval[bx];

    switch (depth) {
    case 2: /* 4bpp */
        ecx >>= 5;
        break;
    case 1: /* 2bpp */
        ecx >>= 4;
        break;
    case 3: /* 8bpp */
        ecx >>= 6;
        break;
    default:
        return; /* the layer draws nothing at this mode */
    }

    edi += r[R_ESI];
    ngptrdat[i] = ecx;
    ngptrdat2 = ecx;
    ngceax[i] = eax;
    ngcedi[i] = edi;

    bg_finish(r, bg, big, edx, eax, edi, ebp);
}

#define NG_BG_TILE(n)                                           \
    void c_drawbg##n##tile16b(u4* const r)                      \
    {                                                           \
        u4 const bg = (n) - 1u;                                 \
                                                                \
        bg_prdat[bg][r[R_EBX]] = 1;                             \
        curmosaicsz = 1;                                        \
        ng16bprval = 0;                                         \
        ng16bbgval = bg;                                        \
        bg_tile(r, bg, t16x161[r[R_EBX] + bg * 256u] == 1);     \
    }                                                           \
    void c_drawbg##n##tilepr116b(u4* const r)                   \
    {                                                           \
        u4 const bg = (n) - 1u;                                 \
                                                                \
        curmosaicsz = 1;                                        \
        ng16bprval = 0x2000u;                                   \
        ng16bbgval = bg;                                        \
        bg_tile_pr1(r, bg, t16x161[r[R_EBX] + bg * 256u] == 1); \
    }

NG_BG_TILE(1)
NG_BG_TILE(2)
NG_BG_TILE(3)
NG_BG_TILE(4)

/* --- the line dispatchers ------------------------------------------------ *
 *
 * The same job as the tile ones for a single scanline, with three extra
 * branches on top: hi-res (BGMA >= 5), offset-per-tile (BGMA 2 or 4, unless
 * osm2dis or bgmode 4 rules it out), and 16x16 tiles. yposng/flipyposng carry
 * the row inside the tile, which the tile dispatchers never needed.
 *
 * Two things the tile version does not do. Mosaic replaces the output pointer:
 * the line is drawn into xtravbuf instead, so *esi changes* and the later
 * `add edi,esi` picks the new one up. And the palette for direct colour goes
 * into CPalPtrng rather than ebp, because a line drawer looks it up per pixel.
 */
extern u1 bgmode, intrlng[], mosenng[], mosszng[], osm2dis, xtravbuf[];
extern u1* pesimpng;
extern u1* vram;
extern u2 BG3SXl[], BG3SYl[], BGPT3[], BGPT3X[];
extern u4 cfieldad, mosstart[4], yposng, flipyposng, yposngom, flipyposngom;
extern u4 ofsmcptr, ofsmcptr2, ofsmady, ofsmadx, ofsmtptr, ofsmtptrs;
extern u4 ofsmmptr, ofsmcyps, ofshvaladd, ofsmval, ofsmvalh, bgtxadd2;
extern u4 CPalPtrng;

void c_ng_drawlineng2b16b(u4* r), c_ng_drawlineng4b16b(u4* r);
void c_ng_drawlineng8b16b(u4* r);
void c_ng_drawlineng16x162b16b(u4* r), c_ng_drawlineng16x164b16b(u4* r);
void c_ng_drawlineng16x168b16b(u4* r);
void c_ng_drawlineng16x82b16b(u4* r), c_ng_drawlineng16x84b16b(u4* r);
void c_ng_drawlinengom2b16b(u4* r), c_ng_drawlinengom4b16b(u4* r);
void c_ng_drawlinengom8b16b(u4* r);
void c_ng_drawlinengom16x162b16b(u4* r), c_ng_drawlinengom16x164b16b(u4* r);
void c_ng_drawlinengom16x168b16b(u4* r);

enum { LK_PLAIN, /* .nooffsetm, 8x8 and 16x16 */
    LK_OM, /* .offsetm */
    LK_HR }; /* .tiles16x8 */

/* The direct-colour swap, which for a line drawer replaces CPalPtrng. */
static void line_direct(u4 const bx)
{
    if (!(scadsng[bx] & 1u))
        return;
    if (vidbright != prevbrightdc) {
        prevbrightdc = vidbright;
        Gendcolortable();
    }
    CPalPtrng = (u4)(uintptr_t)dcolortab;
}

/* Everything from the direct-colour swap on down, which both line passes
   share: the window, mode 0's palette block, and the call into the renderer. */
static void line_tail(u4* const r, u4 const bg, int const kind, int const big,
    u4 const edx, u4 const eax, u4 const edi)
{
    u4 const bx = r[R_EBX];
    u4 const depth = (u1)edx;

    if (depth == 3)
        line_direct(bx);
    if (depth == 1 && kind != LK_OM)
        mode0add = 0;

    r[R_EAX] = eax;
    r[R_EDI] = edi;

    if (kind == LK_OM) {
        ofsmval = 0x2000u << bg;
        ofsmvalh = 0x2000u << bg;
    }
    if (kind != LK_HR) /* the hi-res branch builds no window */
        build_window(r, bg);
    if (depth == 1) {
        if (kind == LK_OM)
            mode0add = 0; /* after the window here, unlike the others */
        else if (kind == LK_PLAIN && BGMA[bx] == 0)
            mode0add = mode0ads;
    }

    r[R_ECX] = bx + bg * 256u;
    r[R_EDX] = (edx & 0xFFFFFF00u) | (1u << bg);

    if (kind == LK_HR) {
        (depth == 2 ? c_ng_drawlineng16x84b16b : c_ng_drawlineng16x82b16b)(r);
    } else if (kind == LK_OM) {
        if (depth == 2)
            (big ? c_ng_drawlinengom16x164b16b : c_ng_drawlinengom4b16b)(r);
        else if (depth == 1)
            (big ? c_ng_drawlinengom16x162b16b : c_ng_drawlinengom2b16b)(r);
        else
            (big ? c_ng_drawlinengom16x168b16b : c_ng_drawlinengom8b16b)(r);
    } else {
        if (depth == 2)
            (big ? c_ng_drawlineng16x164b16b : c_ng_drawlineng4b16b)(r);
        else if (depth == 1)
            (big ? c_ng_drawlineng16x162b16b : c_ng_drawlineng2b16b)(r);
        else
            (big ? c_ng_drawlineng16x168b16b : c_ng_drawlineng8b16b)(r);
    }

    if (ng2_mosaic)
        c_domosaicng16b();
}

/* The priority-0 prologue: work the addresses out and cache them for pass 1. */
static void line_finish(u4* const r, u4 const bg, int const kind, int const big,
    u4 const edx, u4 const eax, u4 edi, u4 ecx)
{
    u4 const i = r[R_EBX] + bg * 256u;
    u4 const depth = (u1)edx;

    switch (depth) {
    case 2:
        ecx >>= 5;
        break;
    case 1:
        ecx >>= 4;
        break;
    case 3:
        if (kind == LK_HR)
            return; /* no 8bpp hi-res drawer */
        ecx >>= 6;
        break;
    default:
        return;
    }

    edi += r[R_ESI];
    ngptrdat[i] = ecx;
    ngptrdat2 = ecx;
    ngceax[i] = eax;
    ngcedi[i] = edi;
    line_tail(r, bg, kind, big, edx, eax, edi);
}

/* The 16x16 vertical flip for the coming line: which half of the tile the row
   falls in decides which way the second half steps. */
static void tadd_from_row(u4 const row)
{
    taddnfy16x16 = 0;
    taddfy16x16 = 16;
    if (row & 8u) {
        taddnfy16x16 = 16;
        taddfy16x16 = (u4)-16;
    }
}

/* The offset-per-tile setup: where BG3's offset table is for this line, and
   the eight variables the om drawers walk it with. */
static void om_setup(u4* const r, u4 const bg, int const big, int const mask16)
{
    u4 const bx = r[R_EBX];
    u4 eax, edx;

    edx = BG3SYl[bx];
    if (!big && (u2)edx != 0xFFFFu)
        edx &= 0x1FFu; /* the 8x8 form clips the row; the 16x16 one does not */
    if (big)
        edx &= 0x3FFu;
    edx = (edx >> 3) << 6;
    eax = (u2)((u2)BGPT3[bx] + (u2)edx);

    edx = BG3SXl[bx] & 0xF8u;
    ofsmcyps = bx;
    if (curmosaicsz != 1)
        ofsmcyps = mosstart[bg];
    edx = (edx >> 3) << 1;
    eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)edx);
    if (BG3SYl[bx] > 0xFFF7u)
        eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + 0x780u);
    eax += 0x40u;

    /* mask16: the priority-1 8x8 form masks the cursor with 0FFC0h, so the
       carry out of the low word is dropped there and kept everywhere else. */
    ofsmcptr = (eax & (mask16 ? 0xFFC0u : 0xFFFFFFC0u)) + (u4)(uintptr_t)vram;
    ofsmcptr2 = eax & 0x3Fu;
    ofsmady = dword_at(&BGPT1Y[bx + bg * 256u]);
    ofsmadx = dword_at(&BGPT1X[bx + bg * 256u]);
    ofsmtptr = dword_at(&BGPT1[bx + bg * 256u]);
    ofsmtptrs = ofsmtptr;
}

/* The BG3 horizontal overflow, which decides whether the offset cursor starts
   a screen on and which way bgtxadd2 steps. A *word* store, so bgtxadd2 keeps
   its upper half. */
static void om_overflow(u4 const bx)
{
    bgtxadd2 &= 0xFFFF0000u;
    if (BGPT3X[bx] == 0)
        return;
    if ((u2)((u2)BG3SXl[bx] + 16u) & 0x100u) {
        ofsmcptr += 0x800u;
        bgtxadd2 = (bgtxadd2 & 0xFFFF0000u) | (u2)((u2)bgtxadd2 - 0x800u);
    } else {
        bgtxadd2 = (bgtxadd2 & 0xFFFF0000u) | (u2)((u2)bgtxadd2 + 0x800u);
    }
}

static void bg_line(u4* const r, u4 const bg)
{
    u4 const bx = r[R_EBX];
    u4 const i = bx + bg * 256u;
    u4 ecx = r[R_ECX];
    u4 eax, edi, edx;
    int big, kind;

    ng16bprval = 0;
    pesimpng = (u1*)(uintptr_t)r[R_ESI];
    ng16bbgval = bg;
    if (bgmode >= 5 && (intrlng[bx] & 1u))
        ecx += bx + cfieldad;

    curmosaicsz = 1;
    if ((mosenng[bx] & (1u << bg)) && mosszng[bx] != 0) {
        u4 q;

        curmosaicsz = (u1)(mosszng[bx] + 1u);
        /* The line goes to the scratch buffer instead, and esi follows it. */
        for (q = 0; q < 128u; q++)
            memcpy(xtravbuf + 32 + q * 4, "\xFF\xFF\xFF\xFF", 4);
        r[R_ESI] = (u4)(uintptr_t)(xtravbuf + 32);
        ecx = (dword_at(&BG1SYl[i]) & 0xFFFFu) + mosstart[bg];
    }

    if (BGMA[bx] >= 5) {
        kind = LK_HR;
        big = t16x161[i] == 1;
    } else if (osm2dis != 1
        && (BGMA[bx] == 2 || (bgmode != 4 && BGMA[bx] == 4))) {
        kind = LK_OM;
        big = t16x161[i] == 1;
    } else {
        kind = LK_PLAIN;
        big = t16x161[i] == 1;
    }

    if (kind == LK_OM)
        om_setup(r, bg, big, 0);

    if (kind == LK_HR && !big) {
        taddnfy16x16 = 0;
        taddfy16x16 = 0;
    } else if (big) {
        tadd_from_row(ecx);
    }

    eax = (kind == LK_OM) ? ofsmtptr : dword_at(&BGPT1[i]);
    if (ecx & (big ? 0x200u : 0x100u))
        eax += dword_at(&BGPT1Y[i]);

    /* The row inside the tile: bits 3..5 of the scaled scroll. */
    edx = (ecx * 8u) & 0x38u;
    yposng = edx;
    flipyposng = edx ^ 0x38u;
    if (kind == LK_OM) {
        yposngom = edx;
        flipyposngom = flipyposng;
    }
    eax += (ecx * (big ? 4u : 8u)) & 0x7C0u;

    ecx = dword_at(&BG1SXl[i]);
    if (kind == LK_HR)
        ecx += ecx;
    edx = dword_at(&BGPT1X[i]);
    if (ecx & ((big || kind == LK_HR) ? 0x200u : 0x100u)) {
        eax += edx;
        if (kind == LK_OM)
            ofsmtptr += edx;
        edx = (edx & 0xFFFF0000u) | (u2)(-(s2)(u2)edx);
    }
    edi = ecx;
    edx = (edx & 0xFFFF0000u) | (u2)((u2)edx - 64u);
    ecx &= (big || kind == LK_HR) ? 0x1F0u : 0xF8u;
    eax &= 0xFFFFu;
    ecx >>= (big || kind == LK_HR) ? 3 : 2;
    bgtxadd = edx;
    bgtxad[i] = (u2)edx;
    eax += ecx;
    if (kind == LK_OM) {
        ofsmtptr += ecx;
        ofsmmptr = eax;
        om_overflow(bx);
        ofshvaladd = 0;
    }

    ecx = dword_at(&BGOPT1[i]) & 0xFFFFu;
    edi &= (big || kind == LK_HR) ? 0xFu : 7u;
    if (kind == LK_HR)
        edi >>= 1;
    memcpy(&edx, (u1 const*)colormodedef + (BGMA[bx] & 7u) * 4u + bg, 4);
    edi = (u4)(-(s4)edi) * 2u;

    line_finish(r, bg, kind, big, edx, eax, edi, ecx);
}

#define NG_BG_LINE(n)                      \
    void c_drawbg##n##line16b(u4* const r) \
    {                                      \
        bg_prdat[(n) - 1u][r[R_EBX]] = 0;  \
        bg_line(r, (n) - 1u);              \
    }

NG_BG_LINE(1)
NG_BG_LINE(2)
NG_BG_LINE(3)
NG_BG_LINE(4)

/* --- the priority-1 line dispatchers ------------------------------------- *
 *
 * Like the priority-1 tile pass, these read back what pass 0 cached in
 * ngceax/ngcedi/ngptrdat/bgtxad rather than working the addresses out again.
 * The row inside the tile is not cached, so every branch recomputes it - and
 * from BG1SYl with the mosaic and interlace adjustments folded in, not from
 * the caller's ecx the way pass 0 does.
 *
 * Three places where this is not pass 0 with the arithmetic removed, all of
 * them the assembly's rather than mistakes to tidy away:
 *
 * - offset-per-tile is taken on BGMA 2 only. The test for 4 is commented out,
 *   so mode 4 draws through the plain path here and through the offset one on
 *   pass 0;
 * - its 8x8 form masks the map cursor with 0FFC0h, sixteen bits, where pass 0
 *   and the 16x16 form use 0FFFFFFC0h;
 * - on an interlaced mosaic line the mosaic start lands in the row twice,
 *   because the same subtract-and-add is applied on both sides of the field
 *   offset.
 */

/* The scanline's row: the layer's vertical scroll, replaced by the mosaic
   block's start when mosaic is on, plus the interlace field. */
static u4 line_pr1_row(u4 const bx, u4 const bg, int const interl)
{
    u4 row = (dword_at(&BG1SYl[bx + bg * 256u]) & 0xFFFFu) + bx;

    if (curmosaicsz != 1)
        row = row - bx + mosstart[bg];
    if (interl && (intrlng[bx] & 1u)) {
        row += bx + cfieldad;
        if (curmosaicsz != 1)
            row = row - bx + mosstart[bg];
    }
    return row;
}

static void line_pr1_ypos(u4 const row, int const om)
{
    u4 const y = (row * 8u) & 0x38u;

    yposng = y;
    flipyposng = y ^ 0x38u;
    if (om) {
        yposngom = y;
        flipyposngom = flipyposng;
    }
}

/* The x half of the offset-mode setup, which pass 0 does inline because it is
   working the map pointer out at the same time. */
static void om_setup_pr1_x(u4 const bx, u4 const bg, int const big)
{
    u4 const i = bx + bg * 256u;
    u4 ecx = dword_at(&BG1SXl[i]);

    if (ecx & (big ? 0x200u : 0x100u))
        ofsmtptr += dword_at(&BGPT1X[i]);
    ecx &= big ? 0x1F0u : 0xF8u;
    ofsmtptr += ecx >> (big ? 3 : 2);
}

static void line_pr1_finish(u4* const r, u4 const bg, int const kind,
    int const big)
{
    u4 const bx = r[R_EBX];
    u4 const i = bx + bg * 256u;
    u4 edx, depth;

    /* Written before the depth test, so a layer that draws nothing still
       leaves these behind - as the assembly does. */
    bgtxadd = (bgtxadd & 0xFFFF0000u) | bgtxad[i];
    ngptrdat2 = ngptrdat[i];
    memcpy(&edx, (u1 const*)colormodedef + (BGMA[bx] & 7u) * 4u + bg, 4);
    if (kind == LK_OM) {
        ofsmmptr = ngceax[i];
        ofshvaladd = 0;
    }

    depth = (u1)edx;
    if (depth == 0 || depth > 3)
        return;
    if (depth == 3 && kind == LK_HR)
        return; /* no 8bpp hi-res drawer */

    line_tail(r, bg, kind, big, edx, ngceax[i], ngcedi[i]);
}

static void bg_line_pr1(u4* const r, u4 const bg)
{
    u4 const bx = r[R_EBX];
    int const big = t16x161[bx + bg * 256u] == 1;
    int kind;
    u4 row;

    ng16bprval = 0x2000u;
    pesimpng = (u1*)(uintptr_t)r[R_ESI];
    ng16bbgval = bg;

    curmosaicsz = 1;
    if ((mosenng[bx] & (1u << bg)) && mosszng[bx] != 0) {
        curmosaicsz = (u1)(mosszng[bx] + 1u);
        memset(xtravbuf + 32, 0xFF, 512);
        r[R_ESI] = (u4)(uintptr_t)(xtravbuf + 32);
    }

    if (BGMA[bx] >= 5)
        kind = LK_HR;
    else if (osm2dis != 1 && BGMA[bx] == 2)
        kind = LK_OM;
    else
        kind = LK_PLAIN;

    if (kind == LK_OM) {
        om_setup(r, bg, big, !big);
        if (!big) /* the 16x16 form takes the overflow at the far end */
            om_overflow(bx);
        om_setup_pr1_x(bx, bg, big);
        row = line_pr1_row(bx, bg, 0); /* no interlace on this branch */
        if (big)
            tadd_from_row(row);
        line_pr1_ypos(row, 1);
    } else {
        row = line_pr1_row(bx, bg, 1);
        if (kind == LK_HR) {
            tadd_from_row(row);
            if (!big) {
                taddnfy16x16 = 0;
                taddfy16x16 = 0;
            }
        } else if (big) {
            tadd_from_row(row);
        }
        line_pr1_ypos(row, 0);
    }

    if (kind == LK_OM && big)
        om_overflow(bx);

    line_pr1_finish(r, bg, kind, big);
}

#define NG_BG_LINE_PR1(n)                     \
    void c_drawbg##n##linepr116b(u4* const r) \
    {                                         \
        bg_line_pr1(r, (n) - 1u);             \
    }

NG_BG_LINE_PR1(1)
NG_BG_LINE_PR1(2)
NG_BG_LINE_PR1(3)
NG_BG_LINE_PR1(4)
