/*
 * video/c_ng2tile.c - the 8x8 tile drawers from video/newg162.asm.
 *
 * One routine per background depth, each a gating tree (transparent or not,
 * then which combination of main and sub windowing) over a common body. The
 * tree itself is video/c_ng2gate.c; this is the body and the pixel writers.
 *
 * The writers look like two dozen separate macros in the assembly but are one
 * family with four flags:
 *   t    OR the main-screen pixel with UnusedBit
 *   ms   write the sub screen too, at edi+75036*2, before OR-ing the main one
 *   s    write *only* the sub screen
 *   w    windowed: one pixel at a time, skipped while ngcwinmode is 1
 * and a partial-tile form that tests each pixel against 0xFFFF first.
 *
 * Entered from video/newgfx16.mac by `jmp` with one word already pushed, so
 * the assembly entry point keeps a seam that ends `pop ebx / ret`.
 */
#include <stdint.h>

#include "../types.h"

enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u1 vrama[], tltype2b[], vidmemch2[], vidmemch2s[];
extern u4 UnusedBit[2], ngwintable[];
extern u4 ng16bbgval, ng16bprval, ngptrdat2, mode0add, bgtxadd;
extern u4 bg1drwng[], bg1totng[], ngpalcon2b[];
extern u4 ngcwinptr, ngcwinmode, ngcpixleft;
extern u1 tleftn;
extern u4 tleftnb;
extern u1* vcache2b;
extern u1* vcache2bs;
extern void c_cachesingle2bng(u4 ecx);

/* the sub screen, in bytes from the main one */
#define SUB (75036u * 2u)

/* Writer flags. */
enum { W_T = 1,
    W_MS = 2,
    W_S = 4,
    W_W = 8 };

/* cacheloopstuff / DoCache: decode one tile into the cache, forwards and
   mirrored, taking colours from the palette ebp points at. */
static void docache(u4 const tile, u1 const dl, u2 const* const pal)
{
    u1 const* src = vcache2b + (tile << 6);
    u1* dst = vcache2bs + (tile << 8);
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

/* The windowed form works a pixel at a time and honours ngcwinmode. With sm
   the sub screen is written whatever the window says; with ms it is not. */
static void w_full_win(u1* const edi, u1 const* const src, int const f,
    int const submain)
{
    u4 i;
    for (i = 0; i < 16; i += 2) {
        u2 v = *(u2 const*)(src + i);
        if (submain && (f & W_MS))
            *(u2*)(edi + i + SUB) = v;
        if (ngcwinmode == 1)
            continue;
        if (!submain && (f & W_MS))
            *(u2*)(edi + i + SUB) = v;
        if (f & W_S) {
            *(u2*)(edi + i + SUB) = v;
            continue;
        }
        if (f & W_T)
            v = (u2)(v | UnusedBit[0]);
        *(u2*)(edi + i) = v;
    }
}

/* Partial tile: one pixel, skipped when the cache holds the transparent
   marker. */
static void w_part(u1* const edi, u1 const* const src, u4 const off,
    int const f, int const win, int const submain)
{
    u2 v = *(u2 const*)(src + off);
    if (v == 0xFFFFu)
        return;
    if (win && submain && (f & W_MS))
        *(u2*)(edi + off + SUB) = v;
    if (win && ngcwinmode == 1)
        return;
    if ((!win || !submain) && (f & W_MS))
        *(u2*)(edi + off + SUB) = v;
    if (f & W_S) {
        *(u2*)(edi + off + SUB) = v;
        return;
    }
    if (f & W_T)
        v = (u2)(v | UnusedBit[0]);
    *(u2*)(edi + off) = v;
}

/*
 * drawtileng16b: one tile of a background line.
 *
 * eax indexes vrama (the tile map), ecx carries the tile value in, edi is the
 * output. Note dl is *not* the palette index it looks like: the index is
 * masked to five bits and used to load a palette constant into edx, and the
 * low byte of that constant is what the cache map is keyed on.
 */
typedef struct {
    int flags; /* writer flags for the full/partial writers */
    int win; /* windowed leaf */
    int submain; /* sub written outside the window test (the sm variants) */
} leaf;

static void tile_body(u4* const r, leaf const* const lf)
{
    u1* edi = (u1*)(uintptr_t)r[R_EDI];
    u4 ecx = r[R_ECX], eax = r[R_EAX], edx;
    u1 dl;
    u4 idx;

    bg1drwng[ng16bbgval]++;
    edx = (u4)(u1)(ecx >> 8) & 0x1Fu;
    ecx &= 0x3FFu;
    ecx += ngptrdat2;
    edx = ngpalcon2b[edx];

    /* test2ba */
    ecx &= 4095u;
    edx += mode0add;
    dl = (u1)edx;
    idx = ecx;

    if ((vidmemch2[idx] & 0xFFu) != 0) { /* %%docache */
        c_cachesingle2bng(idx);
        vidmemch2s[idx] = dl;
        docache(idx, dl, (u2 const*)(uintptr_t)r[R_EBP]);
    } else if (vidmemch2s[idx] != dl) {
        vidmemch2s[idx] = dl;
        docache(idx, dl, (u2 const*)(uintptr_t)r[R_EBP]);
    }

    if (tltype2b[idx] != 2) {
        /* preparet2batile */
        u1 const* src = vcache2bs + (idx << 8);
        u4 const tile = *(u4 const*)(vrama + eax);
        int const flipx = (tile & 0x4000u) != 0;
        int const flipy = (tile & 0x8000u) != 0;
        u4 k;

        if (flipx)
            src += 128;

        if (tltype2b[idx] != 0) { /* full tile */
            for (k = 0; k < 8; k++) {
                u1* const p = edi + (flipy ? 288 * 2 * (7 - k) : 288 * 2 * k);
                if (lf->win)
                    w_full_win(p, src + 16 * k, lf->flags, lf->submain);
                else
                    w_full(p, src + 16 * k, lf->flags);
            }
        } else { /* partial tile */
            u1* p = flipy ? edi + 288 * 2 * 7 : edi;
            for (k = 0; k < 8; k++) {
                u4 n;
                for (n = 0; n < 16; n += 2)
                    w_part(p, src, n, lf->flags, lf->win, lf->submain);
                src += 16;
                p += flipy ? -288 * 2 : 288 * 2;
            }
        }
    }

    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_EDI] = (u4)(uintptr_t)edi;
}

/* %5 - the tail every path falls into, including the priority-bit skip. */
static void finline(u4* const r)
{
    bg1totng[ng16bbgval]++;
    r[R_EAX] = (r[R_EAX] & 0xFFFF0000u) | (u2)((u2)r[R_EAX] + 2u);
    r[R_EDI] += 16;
    if ((r[R_EAX] & 0x3Fu) == 0)
        r[R_EAX] = (r[R_EAX] & 0xFFFF0000u) | (u2)((u2)r[R_EAX] + (u2)bgtxadd);
}

/* drawtile16b: thirty-three tiles across a line. A tile whose priority bit
   does not match this pass is skipped straight to the tail. */
static void drawtile_line(u4* const r, leaf const* const lf)
{
    tleftn = 33;
    for (;;) {
        u4 ecx = (r[R_ECX] & 0xFFFF0000u)
            | *(u2 const*)(vrama + r[R_EAX]);
        ecx ^= ng16bprval;
        r[R_ECX] = ecx;
        if (!(ecx & 0x2000u))
            tile_body(r, lf);
        finline(r);
        if (--tleftn == 0)
            return;
    }
}

/* Leaf-entry counts, so a test can tell which leaves it actually reaches
   rather than inferring it from a mutation that may never run. */
u4 ng2_leafhits[4];

/* The plain leaf: no transparency, no windowing. Entered by jmp with one word
   pushed, so the assembly seam ends `pop ebx / ret`. */
void c_drawtile2b_nt(u4* const r)
{
    static leaf const lf = { 0, 0, 0 };
    ng2_leafhits[0]++;
    drawtile_line(r, &lf);
}

/* The other three leaves that need no window handling. msnt writes the main
   screen before the sub one and mst the other way round; the addresses differ,
   so the order does not. */
void c_drawtile2b_t(u4* const r)
{
    static leaf const lf = { W_T, 0, 0 };
    ng2_leafhits[1]++;
    drawtile_line(r, &lf);
}

void c_drawtile2b_mst(u4* const r)
{
    static leaf const lf = { W_T | W_MS, 0, 0 };
    ng2_leafhits[2]++;
    drawtile_line(r, &lf);
}

void c_drawtile2b_msnt(u4* const r)
{
    static leaf const lf = { W_MS, 0, 0 };
    ng2_leafhits[3]++;
    drawtile_line(r, &lf);
}
