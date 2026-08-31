/*
 * The 8x8 tile drawers from video/newg162.asm: one routine per background
 * depth, each a gating tree (video/c_ng2gate.c) over the body and writers
 * here. The depths share every line of the body and differ only in the four
 * tables and two masks the `depth` descriptor carries.
 *
 * The writers are one family with four flags:
 *   t    OR the main-screen pixel with UnusedBit
 *   ms   write the sub screen too, at edi+75036*2, before OR-ing the main one
 *   s    write *only* the sub screen
 *   w    windowed: one pixel at a time, skipped while ngcwinmode is 1
 * plus a partial-tile form that tests each pixel against 0xFFFF first.
 *
 * Entered by jmp with one word pushed, so the asm seam ends `pop ebx / ret`.
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

extern u1 vrama[];
extern u1 tltype2b[], tltype4b[], tltype8b[];
extern u1 vidmemch2[], vidmemch4[], vidmemch8[];
extern u1 vidmemch2s[], vidmemch4s[], vidmemch8s[];
extern u4 UnusedBit[2], UnusedBitXor[2], ngwintable[];
extern u4 ng16bbgval, ng16bprval, ngptrdat2, mode0add, bgtxadd;
extern u4 taddnfy16x16, taddfy16x16, switch16x16; /* video/newgfx.c */
extern u4 yposng, flipyposng; /* video/newgfx.c: the row within the tile */
/* The offset-per-tile block: BG3's per-column scroll offsets and the walk over
   them. video/newgfx.c and video/vcache_data.c. */
extern u1* ofsmcptr;
extern u4 ofsmcptr2, ofsmtptr, ofsmtptrs, ofsmmptr, ofsmcyps;
extern u4 ofsmady, ofsmadx, ofsmval, ofsmvalh, ofshvaladd;
extern u4 yposngom, flipyposngom, bgtxadd2;
extern zreg CPalPtrng; /* video/c_ngprocbg.c: the converted palette */
extern u1 res640, SpecialLine[256];
extern u4 bg1drwng[], bg1totng[];
extern u4 ngpalcon2b[], ngpalcon4b[], ngpalcon8b[];
extern u4* ngcwinptr;
extern u4 ngcwinmode, ngcpixleft;
extern u1 tleftn, curmosaicsz;
extern u4 tleftnb;
extern u1 *vcache2b, *vcache4b, *vcache8b;
extern u1 *vcache2bs, *vcache4bs, *vcache8bs;
extern void c_cachesingle2bng(u4 ecx);
extern void c_cachesingle4bng(u4 ecx);
extern void c_cachesingle8bng(u4 ecx);

/* What the body reads that depends on bit depth; one to one with the macro
   arguments drawtileng16b took.

   `chk` (the needs-decoding map) is the one table with a scaled index - 1, 2
   or 4 bytes per tile, which is also the width of the test. `chks` and
   `tltype` are byte-indexed at every depth. mode0add applies only at 2bpp,
   because mode 0 backgrounds are 2bpp. */
typedef struct {
    u1* tltype;
    u1 const* chk;
    int chkscale; /* 1, 2 or 4 */
    u1* chks;
    u4 idxmask;
    int mode0;
    u4 const* palcon;
    u1* const* cache; /* allocated at run time, so read through the pointer */
    u1* const* caches;
    void (*cachesingle)(u4);
    /* The line drawers index the primary cache themselves and test
       transparency against the palette-block mask. */
    int lshift;
    u4 lmul;
    u4 lmask;
} depth;

static depth const d2b = { tltype2b, vidmemch2, 1, vidmemch2s, 4095u, 1,
    ngpalcon2b, &vcache2b, &vcache2bs, c_cachesingle2bng, 4, 4, 0x03u };
static depth const d4b = { tltype4b, vidmemch4, 2, vidmemch4s, 2047u, 0,
    ngpalcon4b, &vcache4b, &vcache4bs, c_cachesingle4bng, 5, 2, 0x0Fu };
static depth const d8b = { tltype8b, vidmemch8, 4, vidmemch8s, 1023u, 0,
    ngpalcon8b, &vcache8b, &vcache8bs, c_cachesingle8bng, 6, 1, 0xFFu };

/* testNba's read: chkscale bytes wide, non-zero means decode. */
static int needs_cache(depth const* const d, u4 const idx)
{
    u1 const* const p = d->chk + idx * (u4)d->chkscale;
    u4 v = 0;

    memcpy(&v, p, (size_t)d->chkscale);
    return v != 0;
}

/* Non-zero when the routine ends by tail-jumping into the mosaic pass. Every
   leaf sets it, because the seam that reads it is at the entry point. */
u4 ng2_mosaic;

/* the sub screen, in bytes from the main one */
#define SUB (75036u * 2u)

/* Writer flags. */
enum { W_T = 1,
    W_MS = 2,
    W_S = 4,
    W_W = 8 };

/* cacheloopstuff / DoCache: decode one tile into the cache, forwards and
   mirrored, taking colours from the palette ebp points at. */
static void docache(depth const* const d, u4 const tile, u1 const dl,
    u2 const* const pal)
{
    u1 const* src = *d->cache + (tile << 6);
    u1* dst = *d->caches + (tile << 8);
    u4 row;

    for (row = 0; row < 8; row++) {
        u4 n;
        for (n = 0; n < 8; n++) {
            u1 const b = src[n];
            u2 const px = b == 0 ? 0xFFFFu : pal[(u4)(u1)(b | dl)];
            *(u2*)(dst + n * 2) = px;
            *(u2*)(dst + 14 - n * 2 + 128) = px;
        }
        dst += 16;
        src += 8;
    }
}

/* A full row of eight pixels: four dwords out of the cache. */
static void w_full(u1* const edi, u1 const* const src, int const f)
{
    u4 i;
    for (i = 0; i < 16; i += 4) {
        u4 v = *(u4 const*)(src + i);
        if (f & W_S) {
            *(u4*)(edi + i + SUB) = v;
            continue;
        }
        if (f & W_MS)
            *(u4*)(edi + i + SUB) = v;
        if (f & W_T)
            v |= UnusedBit[0];
        *(u4*)(edi + i) = v;
    }
}

/* Partial tile: one pixel, skipped when the cache holds the transparent
   marker. */
static void w_part(u1* const edi, u1 const* const src, u4 const off,
    int const f)
{
    u2 v = *(u2 const*)(src + off);
    if (v == 0xFFFFu)
        return;
    if (f & W_MS)
        *(u2*)(edi + off + SUB) = v;
    if (f & W_S) {
        *(u2*)(edi + off + SUB) = v;
        return;
    }
    if (f & W_T)
        v = (u2)(v | UnusedBit[0]);
    *(u2*)(edi + off) = v;
}

/* drawtileng16b: one tile of a background line. eax indexes vrama, ecx carries
   the tile value in, edi is the output. dl is not the palette index it looks
   like: the index is masked to five bits, loads a palette constant into edx,
   and the low byte of *that* keys the cache map. */
/* One 8x8 half: decode it into the cache if the key changed, then write its
   eight rows. A map entry is one of these at 8x8 and two of them at 16x16, so
   both drawers share it. `pw` is the per-pixel writer for the windowed arms;
   without one the rows go out whole. */
static void draw_half(depth const* const d, u4 const idx, u4 const edx,
    u1* const edi, u2 const* const pal, u4 const eax, int const f)
{
    u1 const dl = (u1)edx;
    u1 const* src;
    u4 const tile = *(u4 const*)(vrama + eax);
    int const flipx = (tile & 0x4000u) != 0;
    int const flipy = (tile & 0x8000u) != 0;
    u4 k;

    if (needs_cache(d, idx)) { /* %%docache */
        d->cachesingle(idx);
        d->chks[idx] = dl;
        docache(d, idx, dl, pal);
    } else if (d->chks[idx] != dl) {
        d->chks[idx] = dl;
        docache(d, idx, dl, pal);
    }

    if (d->tltype[idx] == 2)
        return;

    /* preparetNbatile */
    src = *d->caches + (idx << 8);
    if (flipx)
        src += 128;

    if (d->tltype[idx] != 0) { /* full tile */
        for (k = 0; k < 8; k++) {
            u1* const p = edi + (flipy ? 288 * 2 * (7 - k) : 288 * 2 * k);
            w_full(p, src + 16 * k, f);
        }
    } else { /* partial tile */
        u1* p = flipy ? edi + 288 * 2 * 7 : edi;
        for (k = 0; k < 8; k++) {
            u4 n;
            for (n = 0; n < 16; n += 2)
                w_part(p, src, n, f);
            src += 16;
            p += flipy ? -288 * 2 : 288 * 2;
        }
    }
}

static void tile_body(zreg* const r, int const f, depth const* const d)
{
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    edx = d->palcon[edx];

    /* testNba */
    ecx &= d->idxmask;
    if (d->mode0)
        edx += mode0add;

    draw_half(d, ecx, edx, (u1*)(uintptr_t)r[R_EDI],
        (u2 const*)(uintptr_t)r[R_EBP], r[R_EAX], f);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
}

/* %5 - the tail every path falls into, including the priority-bit skip. */
static void finline(zreg* const r)
{
    bg1totng[ng16bbgval]++;
    r[R_EAX] = (r[R_EAX] & 0xFFFF0000u) | (u2)((u2)r[R_EAX] + 2u);
    r[R_EDI] += 16;
    if ((r[R_EAX] & 0x3Fu) == 0)
        r[R_EAX] = (r[R_EAX] & 0xFFFF0000u) | (u2)((u2)r[R_EAX] + (u2)bgtxadd);
}

/* drawtile16b: thirty-three tiles across a line. A tile whose priority bit
   does not match this pass is skipped straight to the tail. */
static void drawtile_line(zreg* const r, int const f, depth const* const d)
{
    tleftn = 33;
    for (;;) {
        u4 ecx = (r[R_ECX] & 0xFFFF0000u)
            | *(u2 const*)(vrama + r[R_EAX]);
        ecx ^= ng16bprval;
        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            tile_body(r, f, d);
        finline(r);
        if (--tleftn == 0)
            return;
    }
}

/* --- the 8x8 line drawers ------------------------------------------------ *
 *
 * A separate family, not a variation on the tile drawers: one scanline, read
 * from the *primary* cache (raw indices, looked up in CPalPtrng per pixel), so
 * no secondary cache and no cache key - a miss just calls cachesingleNbng.
 *
 * Transparency is the low bits of the index, which the 03h/0Fh/0FFh macro
 * argument masks. drawlineng16b's full-tile path is dead here only; the 16x16
 * and 16x8 line drawers still take theirs. */
enum { L_T = 1, /* the second palette, 512 bytes on */
    L_MS = 2, /* the sub screen as well */
    L_S = 4 }; /* the sub screen only */

static void l_pix(u1* const edi, u1 const* const src, u4 const i, u4 const ofs,
    u1 const dl, u2 const* const pal, u4 const mask, int const f)
{
    u4 const b = (u1)(src[i] + dl);
    u2 v;

    if ((b & mask) == 0)
        return;
    v = pal[(f & L_T) ? b + 256u : b];
    if (f & L_S) {
        *(u2*)(edi + ofs + SUB) = v;
        return;
    }
    *(u2*)(edi + ofs) = v;
    if (f & L_MS)
        *(u2*)(edi + ofs + SUB)
            = (f & L_T) ? (u2)(v & UnusedBitXor[0]) : v;
}

/* One tile's worth of one scanline: eight pixels. */
static void line_half(depth const* const d, u4 const idx, u1 const dl,
    u1* const edi, u4 const eax, int const f)
{
    u4 const tile = *(u4 const*)(vrama + eax);
    u2 const* const pal = (u2 const*)(uintptr_t)CPalPtrng;
    u1 const* src;
    u4 i;

    if (d->tltype[idx] == 2)
        return;

    /* preparetNba. The first shift is 16-bit in the assembly; the index is
       masked small enough that it never wraps, but keep the shape. */
    src = *d->cache + (u4)((u2)(idx << d->lshift)) * d->lmul;
    src += (tile & 0x8000u) ? flipyposng : yposng;

    for (i = 0; i < 8; i++) {
        u4 const from = (tile & 0x4000u) ? 7u - i : i;
        l_pix(edi, src, from, i * 2u, dl, pal, d->lmask, f);
    }
}

static void line_body(zreg* const r, int const f, depth const* const d)
{
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    edx = d->palcon[edx];

    ecx &= d->idxmask;
    if (d->mode0)
        edx += mode0add;

    /* No cache key and no second decode - a miss just fills the primary. */
    if (needs_cache(d, ecx))
        d->cachesingle(ecx);

    line_half(d, ecx, (u1)edx, (u1*)(uintptr_t)r[R_EDI], r[R_EAX], f);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
}

/* drawline16bmacro. Unlike the tile drawers, every exit here takes the mosaic
   tail when one is due. */
static u4 drawline_line(zreg* const r, int const f, depth const* const d)
{
    tleftn = 33;
    for (;;) {
        u4 const ecx = ((r[R_ECX] & 0xFFFF0000u)
                           | *(u2 const*)(vrama + r[R_EAX]))
            ^ ng16bprval;

        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            line_body(r, f, d);
        finline(r);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* --- the 16x16 tile drawers ---------------------------------------------- *
 *
 * A 16x16 entry is two 8x8 halves from consecutive cache slots: taddnfy16x16
 * (plus taddfy16x16 when flipped vertically) picks the row pair, and flipx
 * starts at the right-hand half.
 *
 * The half index lives in cx, so it steps 16-bit and wraps inside the low word
 * while ngptrdat2's high half stays. The halves are counted by toggling the
 * global switch16x16, so an entry drawn while it is already set draws one half
 * - the assembly's behaviour. */
static void tile_body_16x16(zreg* const r, int const f, depth const* const d)
{
    u4 const eax = r[R_EAX];
    u4 const tile = *(u4 const*)(vrama + eax);
    int const flipx = (tile & 0x4000u) != 0;
    u2 const* const pal = (u2 const*)(uintptr_t)r[R_EBP];
    u1* edi = (u1*)(uintptr_t)r[R_EDI];
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddnfy16x16);
    if (tile & 0x8000u)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddfy16x16);
    if (flipx)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);

    do {
        /* The seam pushes ecx and edx around each half, so the masked index
           and the palette constant are copies - the outer pair walks on. */
        u4 e = d->palcon[edx];

        if (d->mode0)
            e += mode0add;
        draw_half(d, ecx & d->idxmask, e, edi, pal, eax, f);

        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);
        if (flipx)
            ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx - 2u);
        edi += 16;
    } while ((switch16x16 ^= 1u) != 0);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = (zreg)(uintptr_t)edi;
}

/* %%ntile: the 16x16 tail. Unlike finline it leaves edi alone - the two halves
   advanced it by 32 between them, and the skipped path adds that itself. */
static void finline_16x16(zreg* const r)
{
    bg1totng[ng16bbgval]++;
    r[R_EAX] = (r[R_EAX] & 0xFFFF0000u) | (u2)((u2)r[R_EAX] + 2u);
    if ((r[R_EAX] & 0x3Fu) == 0)
        r[R_EAX] = (r[R_EAX] & 0xFFFF0000u) | (u2)((u2)r[R_EAX] + (u2)bgtxadd);
}

/* drawtile16b16x16: seventeen entries across a line, not thirty-three. */
static void drawtile_line_16x16(zreg* const r, int const f, depth const* const d)
{
    tleftn = 17;
    for (;;) {
        u4 const ecx = ((r[R_ECX] & 0xFFFF0000u)
                           | *(u2 const*)(vrama + r[R_EAX]))
            ^ ng16bprval;

        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            tile_body_16x16(r, f, d);
        else
            r[R_EDI] += 32;
        finline_16x16(r);
        if (--tleftn == 0)
            return;
    }
}

/* Leaf-entry counts, so a test can tell which leaves it actually reaches
   rather than inferring it from a mutation that may never run. */
u4 ng2_leafhits[4];

/* The four unwindowed leaves, one set per depth. msnt writes main before sub
   and mst the other way; the addresses differ, so the order does not. */
#define NG2_TILE_LEAVES(bits)                   \
    void c_drawtile##bits##_nt(zreg* const r)   \
    {                                           \
        ng2_mosaic = 0;                         \
        ng2_leafhits[0]++;                      \
        drawtile_line(r, 0, &d##bits);          \
    }                                           \
    void c_drawtile##bits##_t(zreg* const r)    \
    {                                           \
        ng2_mosaic = 0;                         \
        ng2_leafhits[1]++;                      \
        drawtile_line(r, W_T, &d##bits);        \
    }                                           \
    void c_drawtile##bits##_mst(zreg* const r)  \
    {                                           \
        ng2_mosaic = 0;                         \
        ng2_leafhits[2]++;                      \
        drawtile_line(r, W_T | W_MS, &d##bits); \
    }                                           \
    void c_drawtile##bits##_msnt(zreg* const r) \
    {                                           \
        ng2_mosaic = 0;                         \
        ng2_leafhits[3]++;                      \
        drawtile_line(r, W_MS, &d##bits);       \
    }

NG2_TILE_LEAVES(2b)
NG2_TILE_LEAVES(4b)
NG2_TILE_LEAVES(8b)

/* The 16x16 plain leaves, same four kinds as the 8x8 ones. */
u4 ng2_bighits[4];

#define NG2_TILE16_LEAVES(bits)                       \
    void c_drawtile16x16##bits##_nt(zreg* const r)    \
    {                                                 \
        ng2_mosaic = 0;                               \
        ng2_bighits[0]++;                             \
        drawtile_line_16x16(r, 0, &d##bits);          \
    }                                                 \
    void c_drawtile16x16##bits##_t(zreg* const r)     \
    {                                                 \
        ng2_mosaic = 0;                               \
        ng2_bighits[1]++;                             \
        drawtile_line_16x16(r, W_T, &d##bits);        \
    }                                                 \
    void c_drawtile16x16##bits##_mst(zreg* const r)   \
    {                                                 \
        ng2_mosaic = 0;                               \
        ng2_bighits[2]++;                             \
        drawtile_line_16x16(r, W_T | W_MS, &d##bits); \
    }                                                 \
    void c_drawtile16x16##bits##_msnt(zreg* const r)  \
    {                                                 \
        ng2_mosaic = 0;                               \
        ng2_bighits[3]++;                             \
        drawtile_line_16x16(r, W_MS, &d##bits);       \
    }

NG2_TILE16_LEAVES(2b)
NG2_TILE16_LEAVES(4b)
NG2_TILE16_LEAVES(8b)

/* --- the windowed arms --------------------------------------------------- *
 *
 * ngwintable is a run-length list of dwords, alternating outside and inside
 * runs; ngcwinmode says which kind the current one is and ngcpixleft how much
 * is left. A tile with more than 8 pixels left in its run is wholly one or the
 * other and takes a plain writer (or none, in the drawtile16bw arms); one that
 * straddles draws a pixel at a time and walks the list, which is why the
 * windowed traversal goes down the columns. */
static void nextwinmode(void)
{
    u4 const* const p = ngcwinptr;

    ngcwinmode ^= 1u;
    ngcpixleft = p[1];
    ngcwinptr++;
}

/* One pixel of the run. The assembly decrements first and switches on the
   result, so a run of zero wraps rather than switching - reproduce that. */
static void winstep(void)
{
    if (--ngcpixleft == 0)
        nextwinmode();
}

/* processwinpixel, four times: a tile the windowed path does not draw still
   consumes its eight pixels of the run. */
static void skip_win_tile(void)
{
    u4 i;
    for (i = 0; i < 8; i++)
        winstep();
}

/* Windowed writer flags. Without either of the last two the write is
   suppressed while ngcwinmode is 1; with one, that side is written regardless
   and the other gated - main first for msb*, sub first for sm*. */
enum { WW_T = 1,
    WW_MS = 2,
    WW_MAIN_FIRST = 4,
    WW_SUB_FIRST = 8 };

static void w_pix_win(u1* const p, u2 v, int const f, int const part)
{
    if (f & (WW_MAIN_FIRST | WW_SUB_FIRST)) {
        if (part && v == 0xFFFFu)
            return;
        if (f & WW_MAIN_FIRST) {
            if (f & WW_T)
                v = (u2)(v | UnusedBit[0]);
            *(u2*)p = v;
            if (ngcwinmode == 1)
                return;
            /* The sub copy is the main one with the bit taken back out, not
               the value from the cache - they differ once UnusedBit was set. */
            if (f & WW_T)
                v = (u2)(v & UnusedBitXor[0]);
            *(u2*)(p + SUB) = v;
            return;
        }
        *(u2*)(p + SUB) = v;
        if (ngcwinmode == 1)
            return;
        if (f & WW_T)
            v = (u2)(v | UnusedBit[0]);
        *(u2*)p = v;
        return;
    }

    /* The gated family tests the window before the transparent marker. */
    if (ngcwinmode == 1)
        return;
    if (part && v == 0xFFFFu)
        return;
    if (f & WW_MS)
        *(u2*)(p + SUB) = v;
    if (f & WW_T)
        v = (u2)(v | UnusedBit[0]);
    *(u2*)p = v;
}

/* One 8x8 half, column by column, stepping the window run once per column
   before that column is written. A half the cache marks as fully transparent
   still consumes its eight pixels of the run. */
static void draw_half_win(depth const* const d, u4 const idx, u4 const edx,
    u1* const edi, u2 const* const pal, u4 const eax, int const f)
{
    u1 const dl = (u1)edx;
    u1 const* src;
    u4 const tile = *(u4 const*)(vrama + eax);
    int part, col;

    if (needs_cache(d, idx)) {
        d->cachesingle(idx);
        d->chks[idx] = dl;
        docache(d, idx, dl, pal);
    } else if (d->chks[idx] != dl) {
        d->chks[idx] = dl;
        docache(d, idx, dl, pal);
    }

    if (d->tltype[idx] == 2) {
        skip_win_tile();
        return;
    }

    part = d->tltype[idx] == 0;
    src = *d->caches + (idx << 8);
    if (tile & 0x4000u)
        src += 128;

    for (col = 0; col < 8; col++) {
        int row;
        winstep();
        for (row = 0; row < 8; row++) {
            u4 const from = (tile & 0x8000u) ? 16u * (7u - (u4)row)
                                             : 16u * (u4)row;
            w_pix_win(edi + 288 * 2 * row + col * 2,
                *(u2 const*)(src + col * 2 + from), f, part);
        }
    }
}

/* drawtilengwin16b. */
static void tile_body_win(zreg* const r, int const f, depth const* const d)
{
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    edx = d->palcon[edx];

    ecx &= d->idxmask;
    if (d->mode0)
        edx += mode0add;

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    draw_half_win(d, ecx, edx, (u1*)(uintptr_t)r[R_EDI],
        (u2 const*)(uintptr_t)r[R_EBP], r[R_EAX], f);
}

/* drawtileng16x16win16b: the two-half walk of tile_body_16x16 with the
   windowed half, so a map entry consumes sixteen pixels of the run whichever
   way it goes. */
static void tile_body_win_16x16(zreg* const r, int const f, depth const* const d)
{
    u4 const eax = r[R_EAX];
    u4 const tile = *(u4 const*)(vrama + eax);
    int const flipx = (tile & 0x4000u) != 0;
    u2 const* const pal = (u2 const*)(uintptr_t)r[R_EBP];
    u1* edi = (u1*)(uintptr_t)r[R_EDI];
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddnfy16x16);
    if (tile & 0x8000u)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddfy16x16);
    if (flipx)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);

    do {
        u4 e = d->palcon[edx];

        if (d->mode0)
            e += mode0add;
        draw_half_win(d, ecx & d->idxmask, e, edi, pal, eax, f);

        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);
        if (flipx)
            ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx - 2u);
        edi += 16;
    } while ((switch16x16 ^= 1u) != 0);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = (zreg)(uintptr_t)edi;
}

/* What a windowed arm draws in each of the three states a tile can be in. */
typedef struct {
    int vis; /* the run is wholly outside the window (ngcwinmode 0) */
    int clip; /* wholly inside it; NG2_NODRAW for the drawtile16bw arms */
    int win; /* the run ends inside this tile */
} wleaf;

#define NG2_NODRAW (-1)

/* Shared by both windowed loops: the tile straddles a window boundary, so it
   goes through the per-pixel writer and the run counter is handed back
   afterwards - unless this was the last tile, which returns without writing
   it back, exactly as the assembly's tleftn test does. */
static int straddle(zreg* const r, wleaf const* const lf, depth const* const d)
{
    u4* const run = ngcwinptr;

    ngcpixleft = *run;
    r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | *(u2 const*)(vrama + r[R_EAX]))
        ^ ng16bprval;
    if (!(r[R_ECX] & 0x2000u))
        tile_body_win(r, lf->win, d);
    else
        skip_win_tile();
    finline(r);
    if (--tleftn == 0)
        return 1;
    *ngcwinptr = ngcpixleft;
    return 0;
}

/* WinClipMacro + drawtile16bw. A tile wholly inside the window is skipped
   without drawing, and if the line runs out while skipping, this is the one
   path in the file that tail-jumps into the mosaic pass - which is why it
   returns a flag rather than just ending. */
static u4 drawtile_line_win(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    int clip;

    tleftn = 33;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 8u) {
            if (straddle(r, lf, d))
                return 0;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 8u;

        if (clip) {
            /* .winclipped does not xor the priority bit in - it never looks
               at it - so ecx keeps the raw tile value here. */
            r[R_ECX] = (r[R_ECX] & 0xFFFF0000u) | tile;
            finline(r);
            if (--tleftn == 0)
                return curmosaicsz != 1;
            continue;
        }

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            tile_body(r, lf->vis, d);
        finline(r);
        if (--tleftn == 0)
            return 0;
    }
}

/* drawtile16bw2: the same walk, but a tile inside the window draws with a
   second writer instead of being skipped, and there is no mosaic exit. */
static void drawtile_line_win2(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    int clip;

    tleftn = 33;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 8u) {
            if (straddle(r, lf, d))
                return;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 8u;

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            tile_body(r, clip ? lf->clip : lf->vis, d);
        finline(r);
        if (--tleftn == 0)
            return;
    }
}

/* The 16x16 windowed loops. Same walk, but seventeen entries of sixteen
   pixels each, and a clipped entry skips a double-width tile. */
static int straddle_16x16(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    u4* const run = ngcwinptr;

    ngcpixleft = *run;
    r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | *(u2 const*)(vrama + r[R_EAX]))
        ^ ng16bprval;
    if (!(r[R_ECX] & 0x2000u)) {
        tile_body_win_16x16(r, lf->win, d);
    } else {
        u4 i;
        for (i = 0; i < 16u; i++)
            winstep();
        r[R_EDI] += 32;
    }
    finline_16x16(r);
    if (--tleftn == 0)
        return 1;
    *ngcwinptr = ngcpixleft;
    return 0;
}

static u4 drawtile_line_win_16x16(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    int clip;

    tleftn = 17;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 16u) {
            if (straddle_16x16(r, lf, d))
                return 0;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 16u;

        if (clip) {
            r[R_ECX] = (r[R_ECX] & 0xFFFF0000u) | tile;
            r[R_EDI] += 32;
            finline_16x16(r);
            if (--tleftn == 0)
                return curmosaicsz != 1;
            continue;
        }

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            tile_body_16x16(r, lf->vis, d);
        else
            r[R_EDI] += 32;
        finline_16x16(r);
        if (--tleftn == 0)
            return 0;
    }
}

static void drawtile_line_win2_16x16(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    int clip;

    tleftn = 17;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 16u) {
            if (straddle_16x16(r, lf, d))
                return;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 16u;

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            tile_body_16x16(r, clip ? lf->clip : lf->vis, d);
        else
            r[R_EDI] += 32;
        finline_16x16(r);
        if (--tleftn == 0)
            return;
    }
}

/* The eight windowed leaves, one set per depth. ng2_mosaic carries the mosaic
   tail because popad puts eax back and a return value cannot survive the seam;
   only the four drawtile16bw arms set it, the rest clear it, so all eight can
   share one seam. */
u4 ng2_winhits[8];

#define NG2_WIN_LEAF(bits, name, n, v, c, w)              \
    void c_drawtile##bits##_##name(zreg* const r)         \
    {                                                     \
        static wleaf const lf = { v, c, w };              \
        ng2_winhits[n]++;                                 \
        ng2_mosaic = drawtile_line_win(r, &lf, &d##bits); \
    }

#define NG2_WIN2_LEAF(bits, name, n, v, c, w)     \
    void c_drawtile##bits##_##name(zreg* const r) \
    {                                             \
        static wleaf const lf = { v, c, w };      \
        ng2_winhits[n]++;                         \
        ng2_mosaic = 0;                           \
        drawtile_line_win2(r, &lf, &d##bits);     \
    }

#define NG2_TILE_WIN_LEAVES(bits)                                        \
    NG2_WIN_LEAF(bits, win, 0, 0, NG2_NODRAW, 0)                         \
    NG2_WIN_LEAF(bits, wint, 1, W_T, NG2_NODRAW, WW_T)                   \
    NG2_WIN_LEAF(bits, mstmsw, 2, W_T | W_MS, NG2_NODRAW, WW_T | WW_MS)  \
    NG2_WIN_LEAF(bits, msntmsw, 3, W_MS, NG2_NODRAW, WW_MS)              \
    NG2_WIN2_LEAF(bits, mstmw, 4, W_T | W_MS, W_S, WW_T | WW_SUB_FIRST)  \
    NG2_WIN2_LEAF(bits, mstsw, 5, W_T | W_MS, W_T, WW_T | WW_MAIN_FIRST) \
    NG2_WIN2_LEAF(bits, msntmw, 6, W_MS, W_S, WW_SUB_FIRST)              \
    NG2_WIN2_LEAF(bits, msntsw, 7, W_MS, 0, WW_MAIN_FIRST)

NG2_TILE_WIN_LEAVES(2b)
NG2_TILE_WIN_LEAVES(4b)
NG2_TILE_WIN_LEAVES(8b)

/* The 16x16 windowed leaves. */
u4 ng2_bigwinhits[8];

#define NG2_BWIN_LEAF(bits, name, n, v, c, w)                   \
    void c_drawtile16x16##bits##_##name(zreg* const r)          \
    {                                                           \
        static wleaf const lf = { v, c, w };                    \
        ng2_bigwinhits[n]++;                                    \
        ng2_mosaic = drawtile_line_win_16x16(r, &lf, &d##bits); \
    }

#define NG2_BWIN2_LEAF(bits, name, n, v, c, w)         \
    void c_drawtile16x16##bits##_##name(zreg* const r) \
    {                                                  \
        static wleaf const lf = { v, c, w };           \
        ng2_bigwinhits[n]++;                           \
        ng2_mosaic = 0;                                \
        drawtile_line_win2_16x16(r, &lf, &d##bits);    \
    }

#define NG2_TILE16_WIN_LEAVES(bits)                                       \
    NG2_BWIN_LEAF(bits, win, 0, 0, NG2_NODRAW, 0)                         \
    NG2_BWIN_LEAF(bits, wint, 1, W_T, NG2_NODRAW, WW_T)                   \
    NG2_BWIN_LEAF(bits, mstmsw, 2, W_T | W_MS, NG2_NODRAW, WW_T | WW_MS)  \
    NG2_BWIN_LEAF(bits, msntmsw, 3, W_MS, NG2_NODRAW, WW_MS)              \
    NG2_BWIN2_LEAF(bits, mstmw, 4, W_T | W_MS, W_S, WW_T | WW_SUB_FIRST)  \
    NG2_BWIN2_LEAF(bits, mstsw, 5, W_T | W_MS, W_T, WW_T | WW_MAIN_FIRST) \
    NG2_BWIN2_LEAF(bits, msntmw, 6, W_MS, W_S, WW_SUB_FIRST)              \
    NG2_BWIN2_LEAF(bits, msntsw, 7, W_MS, 0, WW_MAIN_FIRST)

NG2_TILE16_WIN_LEAVES(2b)
NG2_TILE16_WIN_LEAVES(4b)
NG2_TILE16_WIN_LEAVES(8b)

/* --- the 16x16 line drawers ---------------------------------------------- *
 *
 * The 8x8 line drawer inside the 16x16 two-half walk. The full-tile path is
 * live here (only drawlineng16b disabled it), so this family needs procpixels
 * as well as procpixelst: two pixels per dword, no transparency test. */
static void l_pair(u1* const edi, u1 const* const src, u4 const lo,
    u4 const hi, u4 const ofs, u1 const dl, u2 const* const pal, int const f)
{
    u4 const off = (f & L_T) ? 256u : 0u;
    u4 const b1 = (u1)(src[lo] + dl);
    u4 const b2 = (u1)(src[hi] + dl);
    u4 const v = ((u4)pal[b2 + off] << 16) | pal[b1 + off];

    if (f & L_S) {
        *(u4*)(edi + ofs + SUB) = v;
        return;
    }
    *(u4*)(edi + ofs) = v;
    if (f & L_MS) {
        /* A dword mask here, both pixels at once, where the single-pixel
           writers take the low half of the same word. */
        *(u4*)(edi + ofs + SUB) = (f & L_T) ? (v & UnusedBitXor[0]) : v;
    }
}

static void line_half_full(depth const* const d, u4 const idx, u1 const dl,
    u1* const edi, u4 const eax, int const f)
{
    u4 const tile = *(u4 const*)(vrama + eax);
    u2 const* const pal = (u2 const*)(uintptr_t)CPalPtrng;
    int const flipx = (tile & 0x4000u) != 0;
    u1 const* src;
    u4 i;

    if (d->tltype[idx] == 2)
        return;

    src = *d->cache + (u4)((u2)(idx << d->lshift)) * d->lmul;
    src += (tile & 0x8000u) ? flipyposng : yposng;

    if (d->tltype[idx] != 0) { /* full: four dwords */
        for (i = 0; i < 4; i++)
            l_pair(edi, src, flipx ? 7u - i * 2u : i * 2u,
                flipx ? 6u - i * 2u : i * 2u + 1u, i * 4u, dl, pal, f);
        return;
    }
    for (i = 0; i < 8; i++)
        l_pix(edi, src, flipx ? 7u - i : i, i * 2u, dl, pal, d->lmask, f);
}

static void line_body_16x16(zreg* const r, int const f, depth const* const d)
{
    u4 const eax = r[R_EAX];
    u4 const tile = *(u4 const*)(vrama + eax);
    int const flipx = (tile & 0x4000u) != 0;
    u1* edi = (u1*)(uintptr_t)r[R_EDI];
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddnfy16x16);
    if (tile & 0x8000u)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddfy16x16);
    if (flipx)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);

    do {
        u4 const half = ecx & d->idxmask;
        u4 e = d->palcon[edx];

        if (d->mode0)
            e += mode0add;
        if (needs_cache(d, half))
            d->cachesingle(half);
        line_half_full(d, half, (u1)e, edi, eax, f);

        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);
        if (flipx)
            ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx - 2u);
        edi += 16;
    } while ((switch16x16 ^= 1u) != 0);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = (zreg)(uintptr_t)edi;
}

static u4 drawline_line_16x16(zreg* const r, int const f, depth const* const d)
{
    tleftn = 17;
    for (;;) {
        u4 const ecx = ((r[R_ECX] & 0xFFFF0000u)
                           | *(u2 const*)(vrama + r[R_EAX]))
            ^ ng16bprval;

        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            line_body_16x16(r, f, d);
        else
            r[R_EDI] += 32;
        finline_16x16(r);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* 16x8 hi-res line drawers, 2bpp and 4bpp, no windowed arms. A 16-pixel tile
 * splits across two interlace fields (field 1 at FIELD1), so edi advances
 * eight per half. When not hi-res the partial writers keep one field and take
 * the *even* source pixel where the full path takes the odd - the assembly's
 * asymmetry. */
#define FIELD1 (75036u * 4u)

/* A pixel with no transparency test - the full paths never had one. */
static void l_write(u1* const edi, u1 const* const src, u4 const i,
    u4 const ofs, u1 const dl, u2 const* const pal, int const f)
{
    u4 const off = (f & L_T) ? 256u : 0u;
    u2 const v = pal[(u1)(src[i] + dl) + off];

    *(u2*)(edi + ofs) = v;
    if (f & L_MS)
        *(u2*)(edi + ofs + SUB)
            = (f & L_T) ? (u2)(v & UnusedBitXor[0]) : v;
}

static void line_half_16x8(depth const* const d, u4 const idx, u1 const dl,
    u1* const edi, u4 const eax, int const f, int const hires)
{
    u4 const tile = *(u4 const*)(vrama + eax);
    u2 const* const pal = (u2 const*)(uintptr_t)CPalPtrng;
    int const flipx = (tile & 0x4000u) != 0;
    u1 const* src;
    u4 i;

    if (d->tltype[idx] == 2)
        return;

    src = *d->cache + (u4)((u2)(idx << d->lshift)) * d->lmul;
    src += (tile & 0x8000u) ? flipyposng : yposng;

    if (d->tltype[idx] != 0) { /* full */
        for (i = 0; i < 4; i++) {
            u4 const even = flipx ? 7u - i * 2u : i * 2u;
            u4 const odd = flipx ? 6u - i * 2u : i * 2u + 1u;

            if (hires)
                l_write(edi, src, odd, i * 2u + FIELD1, dl, pal, f);
            l_write(edi, src, hires ? even : odd, i * 2u, dl, pal, f);
        }
        return;
    }
    for (i = 0; i < 8; i++) { /* partial */
        if (!hires && (i & 1u))
            continue;
        l_pix(edi, src, flipx ? 7u - i : i,
            (i >> 1) * 2u + ((i & 1u) ? FIELD1 : 0u), dl, pal, d->lmask, f);
    }
}

static void line_body_16x8(zreg* const r, int const f, depth const* const d,
    int const hires)
{
    u4 const eax = r[R_EAX];
    u4 const tile = *(u4 const*)(vrama + eax);
    int const flipx = (tile & 0x4000u) != 0;
    u1* edi = (u1*)(uintptr_t)r[R_EDI];
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddnfy16x16);
    if (tile & 0x8000u)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddfy16x16);
    if (flipx)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);

    do {
        u4 const half = ecx & d->idxmask;
        u4 e = d->palcon[edx];

        if (d->mode0)
            e += mode0add;
        if (needs_cache(d, half))
            d->cachesingle(half);
        line_half_16x8(d, half, (u1)e, edi, eax, f, hires);

        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);
        if (flipx)
            ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx - 2u);
        edi += 8;
    } while ((switch16x16 ^= 1u) != 0);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = (zreg)(uintptr_t)edi;
}

/* [0..3] are the leaves, [4] and [5] count hi-res versus plain entries: the
   two-field path is gated on res640, and a harness that leaves it clear never
   runs any of it. */
u4 ng2_line168hits[6];

/* %%ntile for the 16x8 drawer: the halves advanced edi by sixteen between
   them, and the skipped path adds that itself. */
static u4 drawline_line_16x8(zreg* const r, int const f, depth const* const d)
{
    int const hires = curmosaicsz <= 1 && res640 != 0;

    ng2_line168hits[hires ? 4 : 5]++;
    if (!hires) {
        /* The counter is a byte and the loop is a do-while, so a curmosaicsz
           of zero clears 256 entries rather than none. */
        u4 bx = r[R_EBX];
        u1 n = curmosaicsz;

        do {
            SpecialLine[bx++] = 0;
        } while (--n);
    }

    tleftn = 33;
    for (;;) {
        u4 const ecx = ((r[R_ECX] & 0xFFFF0000u)
                           | *(u2 const*)(vrama + r[R_EAX]))
            ^ ng16bprval;

        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            line_body_16x8(r, f, d, hires);
        else
            r[R_EDI] += 16;
        finline_16x16(r);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* Offset-per-tile line drawers: modes 2, 4 and 6, where BG3 gives a per-column
 * scroll offset recomputed per tile. ax is the map pointer's low word only, so
 * every step is a 16-bit add - wrong only once a line crosses 64KB. `mode` is
 * 4 where one word packs both offsets, 2 where they are separate; the three
 * call sites differ as the flags below say. */
enum { OM_WRAP = 1, /* step ofsmcptr by bgtxadd2 when ofsmcptr2 wraps */
    OM_HV_ALT = 2, /* the horizontal entry is tested against ofsmval */
    /* The 16x16 family only. Its three tails disagree about the width of the
       row index, and only two of the three treat it as nine bits wide. */
    OM_WIDE = 4, /* carry at 200h and mask 1FFh, not 100h and FFh */
    OM_SHR4 = 8, /* the row index shifts down four, not three */
    OM_TADD = 16 }; /* bit 3 selects a flip, into tadd{n,}fy16x16 */

static void om_advance(zreg* const r, int const mode, int const opts)
{
    u4 eax = r[R_EAX];
    u4 ebx, ecx, edx;
    zreg omp; /* ofsmcptr plus an offset: a host address */

    bg1totng[ng16bbgval]++;
    ofsmmptr = (ofsmmptr & 0xFFFF0000u) | (u2)((u2)ofsmmptr + 2u);
    ofsmtptr = (ofsmtptr & 0xFFFF0000u) | (u2)((u2)ofsmtptr + 2u);
    eax = (eax & 0xFFFF0000u) | (u2)ofsmmptr;
    yposng = yposngom;
    flipyposng = flipyposngom;
    r[R_EDI] += 16;

    if ((eax & 0x3Fu) == 0) { /* %%tileadd */
        u2 const bx = (u2)bgtxadd;

        eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + bx);
        ofsmmptr = (ofsmmptr & 0xFFFF0000u) | (u2)((u2)ofsmmptr + bx);
        ofsmtptr = (ofsmtptr & 0xFFFF0000u) | (u2)((u2)ofsmtptr + bx);
    }

    /* The vertical offset. Mode 4 reads the entry behind the cursor and wants
       the 8000h bit set; mode 2 reads the cursor itself. */
    omp = (uintptr_t)ofsmcptr + ofsmcptr2;
    ecx = ofsmval;
    ofshvaladd += 8u;
    {
        u4 const v = (mode == 4) ? *(u4 const*)(uintptr_t)(omp - 0x40u)
                                 : *(u4 const*)(uintptr_t)omp;
        int const take = (mode == 4) ? ((v & 0x8000u) && (v & ecx))
                                     : ((v & ecx) != 0);
        if (take) {
            ebx = v & 0x3FFu;
            eax = (eax & 0xFFFF0000u) | (u2)ofsmtptr;
            ebx += ofsmcyps;
            if (ebx & 0x100u)
                eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ofsmady);
            ebx &= 0xFFu;
            edx = (ebx & 7u) << 3;
            ebx = (ebx >> 3) << 6;
            eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ebx);
            yposng = edx;
            flipyposng = edx ^ 0x38u;
        }
    }

    /* The horizontal one, always from the entry behind the cursor. */
    omp = (uintptr_t)ofsmcptr + ofsmcptr2;
    ofsmcptr2 = (ofsmcptr2 + 2u) & 0x3Fu;
    ecx = (opts & OM_HV_ALT) ? ofsmval : ofsmvalh;
    if ((opts & OM_WRAP) && (ofsmcptr2 & 0x3Fu) == 0)
        ofsmcptr = (u1*)(((uintptr_t)ofsmcptr & ~(uintptr_t)0xFFFFu)
            | (u2)((u2)(uintptr_t)ofsmcptr + (u2)bgtxadd2));
    {
        u4 const v = *(u4 const*)(uintptr_t)(omp - 0x40u);

        if (!(mode == 4 && (v & 0x8000u)) && (v & ecx)) {
            eax = (eax & 0xFFFF0000u)
                | (u2)((u2)eax - (u2)ofsmtptr + (u2)ofsmtptrs);
            ebx = v + ofshvaladd;
            if (ebx & 0x100u)
                eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ofsmadx);
            ebx = (ebx & 0xF8u) >> 2;
            eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ebx);
        }
    }
    r[R_EAX] = eax;
}

/* drawlinengom16b. The drawing is drawlineng16b's, except that the full-tile
   path is live here - only drawlineng16b disabled it. */
static void line_body_om(zreg* const r, int const f, depth const* const d)
{
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    edx = d->palcon[edx];

    ecx &= d->idxmask;
    if (d->mode0)
        edx += mode0add;
    if (needs_cache(d, ecx))
        d->cachesingle(ecx);

    line_half_full(d, ecx, (u1)edx, (u1*)(uintptr_t)r[R_EDI], r[R_EAX], f);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
}

static u4 drawline_line_om(zreg* const r, int const f, depth const* const d,
    int const mode)
{
    tleftn = 33;
    for (;;) {
        u4 const ecx = ((r[R_ECX] & 0xFFFF0000u)
                           | *(u2 const*)(vrama + r[R_EAX]))
            ^ ng16bprval;

        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            line_body_om(r, f, d);
        om_advance(r, mode, OM_WRAP);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

static void line_half_win(depth const* d, u4 idx, u1 dl, u1* edi, u4 eax,
    int f); /* defined with the windowed line writers below */

/* The 16x16 offset-mode drawers draw one half per turn of the outer loop
   instead of the usual two-half walk, so switch16x16 gates three things: which
   half the prologue starts on, whether the map pointers step, and whether
   tleftn counts down. The offset walk runs on every half. */
static void om_advance_16x16(zreg* const r, int const mode, int const opts)
{
    u4 eax = r[R_EAX];
    u4 ebx, ecx, edx;
    zreg omp; /* ofsmcptr plus an offset: a host address */
    int const second = (switch16x16 & 1u) != 0;

    if (second) {
        ofsmmptr = (ofsmmptr & 0xFFFF0000u) | (u2)((u2)ofsmmptr + 2u);
        ofsmtptr = (ofsmtptr & 0xFFFF0000u) | (u2)((u2)ofsmtptr + 2u);
    }
    eax = (eax & 0xFFFF0000u) | (u2)ofsmmptr;
    bg1totng[ng16bbgval]++;
    yposng = yposngom;
    flipyposng = flipyposngom;

    if (second && (eax & 0x3Fu) == 0) {
        u2 const bx = (u2)bgtxadd;

        eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + bx);
        ofsmmptr = (ofsmmptr & 0xFFFF0000u) | (u2)((u2)ofsmmptr + bx);
        ofsmtptr = (ofsmtptr & 0xFFFF0000u) | (u2)((u2)ofsmtptr + bx);
    }

    omp = (uintptr_t)ofsmcptr + ofsmcptr2;
    ecx = ofsmval;
    ofshvaladd += 8u;
    {
        u4 const v = (mode == 4) ? *(u4 const*)(uintptr_t)(omp - 0x40u)
                                 : *(u4 const*)(uintptr_t)omp;
        int const take = (mode == 4) ? ((v & 0x8000u) && (v & ecx))
                                     : ((v & ecx) != 0);
        if (take) {
            /* Not the 8x8 walk with a wider tile: the row index spans nine
               bits, so the carry test, mask and shift all move up one, and bit
               3 picks the vertical flip within the entry - what taddnfy16x16
               and taddfy16x16 carry into the next half. */
            ebx = v & 0x3FFu;
            eax = (eax & 0xFFFF0000u) | (u2)ofsmtptr;
            ebx += ofsmcyps;
            if (ebx & ((opts & OM_WIDE) ? 0x200u : 0x100u))
                eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ofsmady);
            ebx &= (opts & OM_WIDE) ? 0x1FFu : 0xFFu;
            if (opts & OM_TADD) {
                taddnfy16x16 = 0;
                taddfy16x16 = 16;
                if (ebx & 8u) {
                    taddnfy16x16 = 16;
                    taddfy16x16 = (u4)-16;
                }
            }
            edx = (ebx & 7u) << 3;
            ebx = (ebx >> ((opts & OM_SHR4) ? 4 : 3)) << 6;
            eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ebx);
            yposng = edx;
            flipyposng = edx ^ 0x38u;
        }
    }

    omp = (uintptr_t)ofsmcptr + ofsmcptr2;
    ofsmcptr2 = (ofsmcptr2 + 2u) & 0x3Fu;
    ecx = (opts & OM_HV_ALT) ? ofsmval : ofsmvalh;
    if ((opts & OM_WRAP) && (ofsmcptr2 & 0x3Fu) == 0)
        ofsmcptr = (u1*)(((uintptr_t)ofsmcptr & ~(uintptr_t)0xFFFFu)
            | (u2)((u2)(uintptr_t)ofsmcptr + (u2)bgtxadd2));
    {
        u4 const v = *(u4 const*)(uintptr_t)(omp - 0x40u);

        if (!(mode == 4 && (v & 0x8000u)) && (v & ecx)) {
            eax = (eax & 0xFFFF0000u)
                | (u2)((u2)eax - (u2)ofsmtptr + (u2)ofsmtptrs);
            ebx = v + ofshvaladd;
            if (ebx & ((opts & OM_WIDE) ? 0x200u : 0x100u))
                eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ofsmadx);
            ebx = (ebx & 0xF8u) >> 2;
            eax = (eax & 0xFFFF0000u) | (u2)((u2)eax + (u2)ebx);
        }
    }
    r[R_EAX] = eax;
    r[R_EDI] += 16;
}

/* One half of a 16x16 offset-mode entry. The prologue picks which half from
   switch16x16, since the caller comes back round rather than looping here. */
static void line_body_om_16x16(zreg* const r, int const f, depth const* const d,
    int const win)
{
    u4 const eax = r[R_EAX];
    u4 const tile = *(u4 const*)(vrama + eax);
    int const flipx = (tile & 0x4000u) != 0;
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddnfy16x16);
    if (tile & 0x8000u)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddfy16x16);
    if (flipx)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);
    if (switch16x16 & 1u) { /* resume on the right-hand half */
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);
        if (flipx)
            ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx - 2u);
    }

    edx = d->palcon[edx];
    ecx &= d->idxmask;
    if (d->mode0)
        edx += mode0add;
    if (needs_cache(d, ecx))
        d->cachesingle(ecx);

    if (win)
        line_half_win(d, ecx, (u1)edx, (u1*)(uintptr_t)r[R_EDI], eax, f);
    else
        line_half_full(d, ecx, (u1)edx, (u1*)(uintptr_t)r[R_EDI], eax, f);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
}

static u4 drawline_line_om_16x16(zreg* const r, int const f,
    depth const* const d, int const mode)
{
    tleftn = 17;
    for (;;) {
        u4 const ecx = ((r[R_ECX] & 0xFFFF0000u)
                           | *(u2 const*)(vrama + r[R_EAX]))
            ^ ng16bprval;

        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            line_body_om_16x16(r, f, d, 0);
        om_advance_16x16(r, mode,
            OM_WRAP | OM_WIDE | OM_SHR4 | OM_TADD);
        if ((switch16x16 ^= 1u) != 0)
            continue; /* the other half of the same entry */
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* The windowed line writers: same four families and WW_* flags as the tile
   ones, but "transparent" means the second palette, not an UnusedBit OR. The
   sub-first family reads the plain palette and ORs the bit on for the main
   copy; the main-first one reads the second palette and masks it back out for
   the sub. Not mirror images.

   The window run steps in the writer, not the loop: a windowed line tile has
   no per-column pass to hang it on. */
static void l_pix_win(u1* const edi, u1 const* const src, u4 const i,
    u4 const ofs, u1 const dl, u2 const* const pal, u4 const mask,
    int const f)
{
    u4 b;
    u2 v;

    winstep();

    if (!(f & (WW_MAIN_FIRST | WW_SUB_FIRST)) && ngcwinmode == 1)
        return;

    b = (u1)(src[i] + dl);
    if ((b & mask) == 0)
        return;

    if (f & WW_SUB_FIRST) {
        v = pal[b];
        *(u2*)(edi + ofs + SUB) = v;
        if (ngcwinmode == 1)
            return;
        if (f & WW_T)
            v = (u2)(v | UnusedBit[0]);
        *(u2*)(edi + ofs) = v;
        return;
    }

    v = pal[(f & WW_T) ? b + 256u : b];
    if (f & WW_MAIN_FIRST) {
        *(u2*)(edi + ofs) = v;
        if (ngcwinmode == 1)
            return;
        if (f & WW_T)
            v = (u2)(v & UnusedBitXor[0]);
        *(u2*)(edi + ofs + SUB) = v;
        return;
    }

    *(u2*)(edi + ofs) = v;
    if (f & WW_MS)
        *(u2*)(edi + ofs + SUB)
            = (f & WW_T) ? (u2)(v & UnusedBitXor[0]) : v;
}

/* One tile's eight pixels of a windowed scanline. No tltype test at all - a
   windowed line tile draws whatever the cache says about it, unlike every
   other body in the file - and the window step is inside l_pix_win. */
static void line_half_win(depth const* const d, u4 const idx, u1 const dl,
    u1* const edi, u4 const eax, int const f)
{
    u4 const tile = *(u4 const*)(vrama + eax);
    u2 const* const pal = (u2 const*)(uintptr_t)CPalPtrng;
    u1 const* src;
    u4 i;

    src = *d->cache + (u4)((u2)(idx << d->lshift)) * d->lmul;
    src += (tile & 0x8000u) ? flipyposng : yposng;

    for (i = 0; i < 8; i++)
        l_pix_win(edi, src, (tile & 0x4000u) ? 7u - i : i, i * 2u, dl, pal,
            d->lmask, f);
}

/* drawlinengwin16b. */
static void line_body_win(zreg* const r, int const f, depth const* const d)
{
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    edx = d->palcon[edx];

    ecx &= d->idxmask;
    if (d->mode0)
        edx += mode0add;
    if (needs_cache(d, ecx))
        d->cachesingle(ecx);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    line_half_win(d, ecx, (u1)edx, (u1*)(uintptr_t)r[R_EDI], r[R_EAX], f);
}

/* drawlineng16x16win16b: the two-half walk with the windowed half, so a map
   entry consumes sixteen pixels of the run either way. */
static void line_body_win_16x16(zreg* const r, int const f, depth const* const d)
{
    u4 const eax = r[R_EAX];
    u4 const tile = *(u4 const*)(vrama + eax);
    int const flipx = (tile & 0x4000u) != 0;
    u1* edi = (u1*)(uintptr_t)r[R_EDI];
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddnfy16x16);
    if (tile & 0x8000u)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + (u2)taddfy16x16);
    if (flipx)
        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);

    do {
        u4 const half = ecx & d->idxmask;
        u4 e = d->palcon[edx];

        if (d->mode0)
            e += mode0add;
        if (needs_cache(d, half))
            d->cachesingle(half);
        line_half_win(d, half, (u1)e, edi, eax, f);

        ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx + 1u);
        if (flipx)
            ecx = (ecx & 0xFFFF0000u) | (u2)((u2)ecx - 2u);
        edi += 16;
    } while ((switch16x16 ^= 1u) != 0);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = (zreg)(uintptr_t)edi;
}

/* The straddling tile, shared by both windowed line loops. Every exit in the
   line drawers takes the mosaic tail, so unlike the tile version this one
   reports it too. */
static int straddle_line(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    u4* const run = ngcwinptr;

    ngcpixleft = *run;
    r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | *(u2 const*)(vrama + r[R_EAX]))
        ^ ng16bprval;
    if (!(r[R_ECX] & 0x2000u))
        line_body_win(r, lf->win, d);
    else
        skip_win_tile();
    finline(r);
    if (--tleftn == 0)
        return 1;
    *ngcwinptr = ngcpixleft;
    return 0;
}

static u4 drawline_line_win(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    int clip;

    tleftn = 33;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 8u) {
            if (straddle_line(r, lf, d))
                return curmosaicsz != 1;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 8u;

        if (clip) {
            r[R_ECX] = (r[R_ECX] & 0xFFFF0000u) | tile;
            finline(r);
            if (--tleftn == 0)
                return curmosaicsz != 1;
            continue;
        }

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            line_body(r, lf->vis, d);
        finline(r);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

static u4 drawline_line_win2(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    int clip;

    tleftn = 33;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 8u) {
            if (straddle_line(r, lf, d))
                return curmosaicsz != 1;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 8u;

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            line_body(r, clip ? lf->clip : lf->vis, d);
        finline(r);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* The four plain line leaves, one set per depth. */
u4 ng2_linehits[4];

#define NG2_LINE_LEAVES(bits)                                \
    void c_drawline##bits##_nt(zreg* const r)                \
    {                                                        \
        ng2_linehits[0]++;                                   \
        ng2_mosaic = drawline_line(r, 0, &d##bits);          \
    }                                                        \
    void c_drawline##bits##_t(zreg* const r)                 \
    {                                                        \
        ng2_linehits[1]++;                                   \
        ng2_mosaic = drawline_line(r, L_T, &d##bits);        \
    }                                                        \
    void c_drawline##bits##_mst(zreg* const r)               \
    {                                                        \
        ng2_linehits[2]++;                                   \
        ng2_mosaic = drawline_line(r, L_T | L_MS, &d##bits); \
    }                                                        \
    void c_drawline##bits##_msnt(zreg* const r)              \
    {                                                        \
        ng2_linehits[3]++;                                   \
        ng2_mosaic = drawline_line(r, L_MS, &d##bits);       \
    }

NG2_LINE_LEAVES(2b)
NG2_LINE_LEAVES(4b)
NG2_LINE_LEAVES(8b)

/* The eight windowed line leaves, one set per depth. */
u4 ng2_linewinhits[8];

#define NG2_LWIN_LEAF(bits, name, n, v, c, w)             \
    void c_drawline##bits##_##name(zreg* const r)         \
    {                                                     \
        static wleaf const lf = { v, c, w };              \
        ng2_linewinhits[n]++;                             \
        ng2_mosaic = drawline_line_win(r, &lf, &d##bits); \
    }

#define NG2_LWIN2_LEAF(bits, name, n, v, c, w)             \
    void c_drawline##bits##_##name(zreg* const r)          \
    {                                                      \
        static wleaf const lf = { v, c, w };               \
        ng2_linewinhits[n]++;                              \
        ng2_mosaic = drawline_line_win2(r, &lf, &d##bits); \
    }

#define NG2_LINE_WIN_LEAVES(bits)                                         \
    NG2_LWIN_LEAF(bits, win, 0, 0, NG2_NODRAW, 0)                         \
    NG2_LWIN_LEAF(bits, wint, 1, L_T, NG2_NODRAW, WW_T)                   \
    NG2_LWIN_LEAF(bits, mstmsw, 2, L_T | L_MS, NG2_NODRAW, WW_T | WW_MS)  \
    NG2_LWIN_LEAF(bits, msntmsw, 3, L_MS, NG2_NODRAW, WW_MS)              \
    NG2_LWIN2_LEAF(bits, mstmw, 4, L_T | L_MS, L_S, WW_T | WW_SUB_FIRST)  \
    NG2_LWIN2_LEAF(bits, mstsw, 5, L_T | L_MS, L_T, WW_T | WW_MAIN_FIRST) \
    NG2_LWIN2_LEAF(bits, msntmw, 6, L_MS, L_S, WW_SUB_FIRST)              \
    NG2_LWIN2_LEAF(bits, msntsw, 7, L_MS, 0, WW_MAIN_FIRST)

NG2_LINE_WIN_LEAVES(2b)
NG2_LINE_WIN_LEAVES(4b)
NG2_LINE_WIN_LEAVES(8b)

/* The four plain 16x16 line leaves. */
u4 ng2_line16hits[4];

#define NG2_LINE16_LEAVES(bits)                                    \
    void c_drawline16x16##bits##_nt(zreg* const r)                 \
    {                                                              \
        ng2_line16hits[0]++;                                       \
        ng2_mosaic = drawline_line_16x16(r, 0, &d##bits);          \
    }                                                              \
    void c_drawline16x16##bits##_t(zreg* const r)                  \
    {                                                              \
        ng2_line16hits[1]++;                                       \
        ng2_mosaic = drawline_line_16x16(r, L_T, &d##bits);        \
    }                                                              \
    void c_drawline16x16##bits##_mst(zreg* const r)                \
    {                                                              \
        ng2_line16hits[2]++;                                       \
        ng2_mosaic = drawline_line_16x16(r, L_T | L_MS, &d##bits); \
    }                                                              \
    void c_drawline16x16##bits##_msnt(zreg* const r)               \
    {                                                              \
        ng2_line16hits[3]++;                                       \
        ng2_mosaic = drawline_line_16x16(r, L_MS, &d##bits);       \
    }

NG2_LINE16_LEAVES(2b)
NG2_LINE16_LEAVES(4b)
NG2_LINE16_LEAVES(8b)

/* The 16x16 windowed line loops: seventeen entries of sixteen pixels, and
   every exit takes the mosaic tail the way the 8x8 line drawers do. */
static int straddle_line_16(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    u4* const run = ngcwinptr;

    ngcpixleft = *run;
    r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | *(u2 const*)(vrama + r[R_EAX]))
        ^ ng16bprval;
    if (!(r[R_ECX] & 0x2000u)) {
        line_body_win_16x16(r, lf->win, d);
    } else {
        u4 i;
        for (i = 0; i < 16u; i++)
            winstep();
        r[R_EDI] += 32;
    }
    finline_16x16(r);
    if (--tleftn == 0)
        return 1;
    *ngcwinptr = ngcpixleft;
    return 0;
}

static u4 drawline_line_win_16x16(zreg* const r, wleaf const* const lf,
    depth const* const d)
{
    int clip;

    tleftn = 17;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 16u) {
            if (straddle_line_16(r, lf, d))
                return curmosaicsz != 1;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 16u;

        if (clip && lf->clip == NG2_NODRAW) {
            r[R_ECX] = (r[R_ECX] & 0xFFFF0000u) | tile;
            r[R_EDI] += 32;
            finline_16x16(r);
            if (--tleftn == 0)
                return curmosaicsz != 1;
            continue;
        }

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            line_body_16x16(r, clip ? lf->clip : lf->vis, d);
        else
            r[R_EDI] += 32;
        finline_16x16(r);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* The eight windowed 16x16 line leaves. */
u4 ng2_line16winhits[8];

#define NG2_L16WIN_LEAF(bits, name, n, v, c, w)                 \
    void c_drawline16x16##bits##_##name(zreg* const r)          \
    {                                                           \
        static wleaf const lf = { v, c, w };                    \
        ng2_line16winhits[n]++;                                 \
        ng2_mosaic = drawline_line_win_16x16(r, &lf, &d##bits); \
    }

#define NG2_LINE16_WIN_LEAVES(bits)                                        \
    NG2_L16WIN_LEAF(bits, win, 0, 0, NG2_NODRAW, 0)                        \
    NG2_L16WIN_LEAF(bits, wint, 1, L_T, NG2_NODRAW, WW_T)                  \
    NG2_L16WIN_LEAF(bits, mstmsw, 2, L_T | L_MS, NG2_NODRAW, WW_T | WW_MS) \
    NG2_L16WIN_LEAF(bits, msntmsw, 3, L_MS, NG2_NODRAW, WW_MS)             \
    NG2_L16WIN_LEAF(bits, mstmw, 4, L_T | L_MS, L_S, WW_T | WW_SUB_FIRST)  \
    NG2_L16WIN_LEAF(bits, mstsw, 5, L_T | L_MS, L_T, WW_T | WW_MAIN_FIRST) \
    NG2_L16WIN_LEAF(bits, msntmw, 6, L_MS, L_S, WW_SUB_FIRST)              \
    NG2_L16WIN_LEAF(bits, msntsw, 7, L_MS, 0, WW_MAIN_FIRST)

NG2_LINE16_WIN_LEAVES(2b)
NG2_LINE16_WIN_LEAVES(4b)
NG2_LINE16_WIN_LEAVES(8b)

/* The four 16x8 leaves, 2bpp and 4bpp only - there is no 8bpp hi-res drawer
   and no windowed arm in this family. */

#define NG2_LINE168_LEAVES(bits)                                  \
    void c_drawline16x8##bits##_nt(zreg* const r)                 \
    {                                                             \
        ng2_line168hits[0]++;                                     \
        ng2_mosaic = drawline_line_16x8(r, 0, &d##bits);          \
    }                                                             \
    void c_drawline16x8##bits##_t(zreg* const r)                  \
    {                                                             \
        ng2_line168hits[1]++;                                     \
        ng2_mosaic = drawline_line_16x8(r, L_T, &d##bits);        \
    }                                                             \
    void c_drawline16x8##bits##_mst(zreg* const r)                \
    {                                                             \
        ng2_line168hits[2]++;                                     \
        ng2_mosaic = drawline_line_16x8(r, L_T | L_MS, &d##bits); \
    }                                                             \
    void c_drawline16x8##bits##_msnt(zreg* const r)               \
    {                                                             \
        ng2_line168hits[3]++;                                     \
        ng2_mosaic = drawline_line_16x8(r, L_MS, &d##bits);       \
    }

NG2_LINE168_LEAVES(2b)
NG2_LINE168_LEAVES(4b)

/* The four plain offset-mode leaves. The mode flag rides with the depth. */
u4 ng2_lineomhits[4];

#define NG2_LINEOM_LEAVES(bits, mode)                           \
    void c_drawlineom##bits##_nt(zreg* const r)                 \
    {                                                           \
        ng2_lineomhits[0]++;                                    \
        ng2_mosaic = drawline_line_om(r, 0, &d##bits, mode);    \
    }                                                           \
    void c_drawlineom##bits##_t(zreg* const r)                  \
    {                                                           \
        ng2_lineomhits[1]++;                                    \
        ng2_mosaic = drawline_line_om(r, L_T, &d##bits, mode);  \
    }                                                           \
    void c_drawlineom##bits##_mst(zreg* const r)                \
    {                                                           \
        ng2_lineomhits[2]++;                                    \
        ng2_mosaic                                              \
            = drawline_line_om(r, L_T | L_MS, &d##bits, mode);  \
    }                                                           \
    void c_drawlineom##bits##_msnt(zreg* const r)               \
    {                                                           \
        ng2_lineomhits[3]++;                                    \
        ng2_mosaic = drawline_line_om(r, L_MS, &d##bits, mode); \
    }

NG2_LINEOM_LEAVES(2b, 4)
NG2_LINEOM_LEAVES(4b, 2)
NG2_LINEOM_LEAVES(8b, 4)

/* The windowed offset-mode loops. The straddling body is drawlinengomwin16b,
   whose tail is the offset walk with OM_HV_ALT; the clipped path is
   WinClipMacroom's, which takes neither option. */
static void line_body_win_om(zreg* const r, int const f, depth const* const d)
{
    u4 ecx = r[R_ECX], edx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    edx = d->palcon[edx];

    ecx &= d->idxmask;
    if (d->mode0)
        edx += mode0add;
    if (needs_cache(d, ecx))
        d->cachesingle(ecx);

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    line_half_win(d, ecx, (u1)edx, (u1*)(uintptr_t)r[R_EDI], r[R_EAX], f);
}

static int straddle_om(zreg* const r, wleaf const* const lf,
    depth const* const d, int const mode)
{
    u4* const run = ngcwinptr;

    ngcpixleft = *run;
    r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | *(u2 const*)(vrama + r[R_EAX]))
        ^ ng16bprval;
    if (!(r[R_ECX] & 0x2000u))
        line_body_win_om(r, lf->win, d);
    else
        skip_win_tile();
    om_advance(r, mode, OM_HV_ALT);
    if (--tleftn == 0)
        return 1;
    *ngcwinptr = ngcpixleft;
    return 0;
}

static u4 drawline_line_win_om(zreg* const r, wleaf const* const lf,
    depth const* const d, int const mode)
{
    int clip;

    tleftn = 33;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;
        u4 const tile = *(u2 const*)(vrama + r[R_EAX]);

        if (*run <= 8u) {
            if (straddle_om(r, lf, d, mode))
                return curmosaicsz != 1;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 8u;

        if (clip && lf->clip == NG2_NODRAW) {
            r[R_ECX] = (r[R_ECX] & 0xFFFF0000u) | tile;
            om_advance(r, mode, 0);
            if (--tleftn == 0)
                return curmosaicsz != 1;
            continue;
        }

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u) | tile) ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            line_body_om(r, clip ? lf->clip : lf->vis, d);
        om_advance(r, mode, OM_WRAP);
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* The eight windowed offset-mode leaves. */
u4 ng2_lineomwinhits[8];

#define NG2_LOMWIN_LEAF(bits, mode, name, n, v, c, w)              \
    void c_drawlineom##bits##_##name(zreg* const r)                \
    {                                                              \
        static wleaf const lf = { v, c, w };                       \
        ng2_lineomwinhits[n]++;                                    \
        ng2_mosaic = drawline_line_win_om(r, &lf, &d##bits, mode); \
    }

#define NG2_LINEOM_WIN_LEAVES(bits, mode)                            \
    NG2_LOMWIN_LEAF(bits, mode, win, 0, 0, NG2_NODRAW, 0)            \
    NG2_LOMWIN_LEAF(bits, mode, wint, 1, L_T, NG2_NODRAW, WW_T)      \
    NG2_LOMWIN_LEAF(bits, mode, mstmsw, 2, L_T | L_MS, NG2_NODRAW,   \
        WW_T | WW_MS)                                                \
    NG2_LOMWIN_LEAF(bits, mode, msntmsw, 3, L_MS, NG2_NODRAW, WW_MS) \
    NG2_LOMWIN_LEAF(bits, mode, mstmw, 4, L_T | L_MS, L_S,           \
        WW_T | WW_SUB_FIRST)                                         \
    NG2_LOMWIN_LEAF(bits, mode, mstsw, 5, L_T | L_MS, L_T,           \
        WW_T | WW_MAIN_FIRST)                                        \
    NG2_LOMWIN_LEAF(bits, mode, msntmw, 6, L_MS, L_S, WW_SUB_FIRST)  \
    NG2_LOMWIN_LEAF(bits, mode, msntsw, 7, L_MS, 0, WW_MAIN_FIRST)

NG2_LINEOM_WIN_LEAVES(2b, 4)
NG2_LINEOM_WIN_LEAVES(4b, 2)
NG2_LINEOM_WIN_LEAVES(8b, 4)

/* The four plain 16x16 offset-mode leaves. */
u4 ng2_lineom16hits[4];

#define NG2_LINEOM16_LEAVES(bits, mode)                               \
    void c_drawlineom16x16##bits##_nt(zreg* const r)                  \
    {                                                                 \
        ng2_lineom16hits[0]++;                                        \
        ng2_mosaic = drawline_line_om_16x16(r, 0, &d##bits, mode);    \
    }                                                                 \
    void c_drawlineom16x16##bits##_t(zreg* const r)                   \
    {                                                                 \
        ng2_lineom16hits[1]++;                                        \
        ng2_mosaic = drawline_line_om_16x16(r, L_T, &d##bits, mode);  \
    }                                                                 \
    void c_drawlineom16x16##bits##_mst(zreg* const r)                 \
    {                                                                 \
        ng2_lineom16hits[2]++;                                        \
        ng2_mosaic                                                    \
            = drawline_line_om_16x16(r, L_T | L_MS, &d##bits, mode);  \
    }                                                                 \
    void c_drawlineom16x16##bits##_msnt(zreg* const r)                \
    {                                                                 \
        ng2_lineom16hits[3]++;                                        \
        ng2_mosaic = drawline_line_om_16x16(r, L_MS, &d##bits, mode); \
    }

NG2_LINEOM16_LEAVES(2b, 4)
NG2_LINEOM16_LEAVES(4b, 2)
NG2_LINEOM16_LEAVES(8b, 4)

/* The windowed 16x16 offset-mode loop. The run threshold is eight, not
   sixteen: this family advances the window one half at a time, as it counts
   tleftn. Each of the three tails needs its own OM_WIDE/OM_SHR4/OM_TADD flags;
   they do not follow from one another. */
static u4 drawline_line_win_om_16x16(zreg* const r, wleaf const* const lf,
    depth const* const d, int const mode)
{
    int clip;

    tleftn = 17;
    ngcwinptr = ngwintable;
    ngcwinmode = 0;
    clip = 0;
    if (ngwintable[0] == 0) {
        ngcwinptr++;
        ngcwinmode = 1;
        clip = 1;
    }

    for (;;) {
        u4* const run = ngcwinptr;

        if (*run <= 8u) { /* straddles a window boundary */
            ngcpixleft = *run;
            r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u)
                           | *(u2 const*)(vrama + r[R_EAX]))
                ^ ng16bprval;
            if (!(r[R_ECX] & 0x2000u))
                line_body_om_16x16(r, lf->win, d, 1);
            else
                skip_win_tile();
            om_advance_16x16(r, mode, OM_HV_ALT);
            if ((switch16x16 ^= 1u) == 0 && --tleftn == 0)
                return curmosaicsz != 1;
            *ngcwinptr = ngcpixleft;
            clip = ngcwinmode == 1;
            continue;
        }
        *run -= 8u;

        if (clip && lf->clip == NG2_NODRAW) {
            /* WinClipMacroom16x16 draws nothing and never loads the tile. */
            om_advance_16x16(r, mode, OM_WIDE);
            if ((switch16x16 ^= 1u) != 0)
                continue;
            if (--tleftn == 0)
                return curmosaicsz != 1;
            continue;
        }

        r[R_ECX] = ((r[R_ECX] & 0xFFFF0000u)
                       | *(u2 const*)(vrama + r[R_EAX]))
            ^ ng16bprval;
        if (!(r[R_ECX] & 0x2000u))
            line_body_om_16x16(r, clip ? lf->clip : lf->vis, d, 0);
        om_advance_16x16(r, mode, OM_WRAP | OM_WIDE | OM_SHR4 | OM_TADD);
        if ((switch16x16 ^= 1u) != 0)
            continue;
        if (--tleftn == 0)
            return curmosaicsz != 1;
    }
}

/* The eight windowed 16x16 offset-mode leaves. */
u4 ng2_lineom16winhits[8];

#define NG2_LOM16WIN_LEAF(bits, mode, name, n, v, c, w)           \
    void c_drawlineom16x16##bits##_##name(zreg* const r)          \
    {                                                             \
        static wleaf const lf = { v, c, w };                      \
        ng2_lineom16winhits[n]++;                                 \
        ng2_mosaic                                                \
            = drawline_line_win_om_16x16(r, &lf, &d##bits, mode); \
    }

#define NG2_LINEOM16_WIN_LEAVES(bits, mode)                            \
    NG2_LOM16WIN_LEAF(bits, mode, win, 0, 0, NG2_NODRAW, 0)            \
    NG2_LOM16WIN_LEAF(bits, mode, wint, 1, L_T, NG2_NODRAW, WW_T)      \
    NG2_LOM16WIN_LEAF(bits, mode, mstmsw, 2, L_T | L_MS, NG2_NODRAW,   \
        WW_T | WW_MS)                                                  \
    NG2_LOM16WIN_LEAF(bits, mode, msntmsw, 3, L_MS, NG2_NODRAW, WW_MS) \
    NG2_LOM16WIN_LEAF(bits, mode, mstmw, 4, L_T | L_MS, L_S,           \
        WW_T | WW_SUB_FIRST)                                           \
    NG2_LOM16WIN_LEAF(bits, mode, mstsw, 5, L_T | L_MS, L_T,           \
        WW_T | WW_MAIN_FIRST)                                          \
    NG2_LOM16WIN_LEAF(bits, mode, msntmw, 6, L_MS, L_S, WW_SUB_FIRST)  \
    NG2_LOM16WIN_LEAF(bits, mode, msntsw, 7, L_MS, 0, WW_MAIN_FIRST)

NG2_LINEOM16_WIN_LEAVES(2b, 4)
NG2_LINEOM16_WIN_LEAVES(4b, 2)
NG2_LINEOM16_WIN_LEAVES(8b, 4)

/* The gating trees: what newg162.asm's twenty entry points became. The tree
 * picks one of twelve leaves from transparency and window settings (four for
 * the 16x8 pair). It works on the register block because the decisions are not
 * pure; both determine steps move ecx, and the leaf reads it. */
extern u1 BGMS1[], scadtng[];
extern u4 ng_branch;
void c_determinetransp(zreg* r); /* video/c_ng2gate.c */
void c_checkwindowing(zreg* r);
void c_determinewindow(zreg* r);

typedef void (*ng_leaf)(zreg*);

typedef struct {
    ng_leaf nt, t, mst, msnt; /* no window on this layer */
    ng_leaf win, wint; /* CheckWindowing said yes */
    ng_leaf mstmsw, mstmw, mstsw; /* DetermineWindow 1, 2, 3 */
    ng_leaf msntmsw, msntmw, msntsw;
} ng_tree;

/* DetermineWindow answers 1, 2 or 3, and 0 for "no window here". */
static ng_leaf pick_win(ng_leaf const both, ng_leaf const main,
    ng_leaf const sub, ng_leaf const none)
{
    switch (ng_branch) {
    case 1:
        return both;
    case 2:
        return main;
    case 3:
        return sub;
    default:
        return none;
    }
}

static void ng_gate(zreg* const r, ng_tree const* const t)
{
    u4 const bx = r[R_EBX];
    u1 const dl = (u1)r[R_EDX];

    c_determinetransp(r);
    if (ng_branch == 1) {
        if (BGMS1[bx * 2 + 1] & dl) {
            if (scadtng[bx] & dl) {
                c_determinewindow(r);
                pick_win(t->mstmsw, t->mstmw, t->mstsw, t->mst)(r);
            } else {
                c_determinewindow(r);
                pick_win(t->msntmsw, t->msntmw, t->msntsw, t->msnt)(r);
            }
            return;
        }
        if (scadtng[bx] & dl) {
            c_checkwindowing(r);
            (ng_branch == 1 ? t->wint : t->t)(r);
            return;
        }
        /* falls back to the opaque arm */
    }
    c_checkwindowing(r);
    (ng_branch == 1 ? t->win : t->nt)(r);
}

/* The 16x8 pair: same tree with the window decisions taken out. */
static void ng_gate_16x8(zreg* const r, ng_tree const* const t)
{
    u4 const bx = r[R_EBX];
    u1 const dl = (u1)r[R_EDX];

    c_determinetransp(r);
    if (ng_branch == 1) {
        if (BGMS1[bx * 2 + 1] & dl) {
            ((scadtng[bx] & dl) ? t->mst : t->msnt)(r);
            return;
        }
        if (scadtng[bx] & dl) {
            t->t(r);
            return;
        }
    }
    t->nt(r);
}

#define NG_TREE(entry, pfx)                                           \
    static ng_tree const entry##_tree = { c_##pfx##_nt, c_##pfx##_t,  \
        c_##pfx##_mst, c_##pfx##_msnt, c_##pfx##_win, c_##pfx##_wint, \
        c_##pfx##_mstmsw, c_##pfx##_mstmw, c_##pfx##_mstsw,           \
        c_##pfx##_msntmsw, c_##pfx##_msntmw, c_##pfx##_msntsw };      \
    void c_ng_##entry(zreg* const r) { ng_gate(r, &entry##_tree); }

#define NG_TREE_16X8(entry, pfx)                                     \
    static ng_tree const entry##_tree = { c_##pfx##_nt, c_##pfx##_t, \
        c_##pfx##_mst, c_##pfx##_msnt, 0, 0, 0, 0, 0, 0, 0, 0 };     \
    void c_ng_##entry(zreg* const r) { ng_gate_16x8(r, &entry##_tree); }

NG_TREE(drawtileng2b16b, drawtile2b)
NG_TREE(drawtileng4b16b, drawtile4b)
NG_TREE(drawtileng8b16b, drawtile8b)
NG_TREE(drawtileng16x162b16b, drawtile16x162b)
NG_TREE(drawtileng16x164b16b, drawtile16x164b)
NG_TREE(drawtileng16x168b16b, drawtile16x168b)
NG_TREE(drawlineng2b16b, drawline2b)
NG_TREE(drawlineng4b16b, drawline4b)
NG_TREE(drawlineng8b16b, drawline8b)
NG_TREE(drawlineng16x162b16b, drawline16x162b)
NG_TREE(drawlineng16x164b16b, drawline16x164b)
NG_TREE(drawlineng16x168b16b, drawline16x168b)
NG_TREE(drawlinengom2b16b, drawlineom2b)
NG_TREE(drawlinengom4b16b, drawlineom4b)
NG_TREE(drawlinengom8b16b, drawlineom8b)
NG_TREE(drawlinengom16x162b16b, drawlineom16x162b)
NG_TREE(drawlinengom16x164b16b, drawlineom16x164b)
NG_TREE(drawlinengom16x168b16b, drawlineom16x168b)
NG_TREE_16X8(drawlineng16x82b16b, drawline16x82b)
NG_TREE_16X8(drawlineng16x84b16b, drawline16x84b)
