// sprites.c - C11 port of the sprite routines (processsprites / processspritesb /
// cachesprites) from vcache.asm.  Faithful, bug-for-bug translation of the 32-bit
// x86 assembly: register widths, wrap-around and quirks are replicated exactly.

#include <stdint.h>
#include <string.h>

#include "../types.h"

// ---- shared globals (defined in endmem.asm / the test harness) ----
extern u1 oamram[1024];
extern u1 objsize1, objsize2, objhipr, interlval;
extern u4 objptr, objptrn;
extern u2 objadds1, objadds2;
extern u1 objmovs1, objmovs2;
extern u2 resolutn, curypos;

extern u1 sprlefttot[256];
extern u1 sprleftpr[]; // contiguous 1024 bytes (sprleftpr / sprleftpr1..3)
extern u1 sprcnt[256], sprstart[256], sprtilecnt[256], sprend[256];
extern u2 sprendx[256];

extern u1* spritetablea;
extern u1* vram;
extern u1* vcache4b;
extern u2 vidmemch4[2048];
extern u1 tltype4b[2048];

extern u1 sprprifix;
extern u4 addr2add;

void processsprites(void);
void processspritesb(void);
void cachesprites(void);

// ---- 16-bit helpers --------------------------------------------------------
#define S16(x) ((int16_t)((x) & 0xFFFF))
#define U16(x) ((uint16_t)((x) & 0xFFFF))
// cx is kept as a sign-extended 16-bit value in an int.
#define ADDCX(n) (cx = (int16_t)((cx + (n)) & 0xFFFF))

// Alignment-safe unaligned access (the asm does unaligned stores freely on x86).
static inline void st16(u1* p, u2 v) { memcpy(p, &v, 2); }
static inline void st32(u1* p, u4 v) { memcpy(p, &v, 4); }
static inline u2 ld16(const u1* p)
{
    u2 v;
    memcpy(&v, p, 2);
    return v;
}

// ---- registers threaded across the asm helper calls ------------------------
static int cx; // 16-bit signed/unsigned x position
static u1 dl, dh; // 8-bit y position / status byte
static u4 esi; // absolute char address into vcache4b (integer, as in asm)
static u1* ebp; // spritetablea base

// ---- .bss temporaries modelled as file-scope statics -----------------------
static u1 numleft2do;
static u1 statusbit;
static u4 cpri;
static int obj_x; // word
static u4 objloc; // esi - sprt_char (integer)
static int sprt_char;
static u1 objleft;
static int objvramloc, objvramloc2;

// ---- add_x / add_y macros --------------------------------------------------
static void add_x(void)
{
    u4 eax = (u4)sprt_char;
    eax >>= 2;
    eax = (eax & 0xFFFFFF00u) | ((eax + 16) & 0xFF); // add al,64>>2
    eax <<= 2;
    sprt_char = (int)eax;
    esi = objloc + eax;
}

static void add_y(int n)
{
    u4 eax = (u4)sprt_char;
    eax >>= 2;
    eax = (eax & 0xFFFFFF00u) | ((eax - (u4)(16 * n)) & 0xFF); // sub al,(64>>2)*n
    eax <<= 2;
    eax += 64 * 0x10;
    eax &= 0x3FFF;
    sprt_char = (int)eax;
    esi = objloc + eax;
}

// ---- pass-3 sprite emit helpers --------------------------------------------
static void reprocesssprite(void)
{
    if (S16(cx) <= -8)
        goto next;
    if (cx >= 256)
        goto next;
spec:
    ADDCX(8);
    for (;;) {
        if (!((u1)dl > (u1)resolutn)) {
            u4 y = dl;
            if (!(y < curypos)) {
                u1 ol = objleft;
                if (!(sprstart[y] > ol)) {
                    int ok;
                    if (sprtilecnt[y] <= 34)
                        ok = 1;
                    else if (sprend[y] < ol)
                        ok = 0;
                    else if (sprend[y] > ol)
                        ok = 1;
                    else
                        ok = !((u2)sprendx[y] < U16(cx));
                    if (ok) {
                        u1 oldtot = sprlefttot[y];
                        sprlefttot[y]++;
                        sprleftpr[y * 4 + cpri] = 1;
                        u4 idx = (y << 9) + ((u4)oldtot << 3);
                        st16(ebp + idx, U16(cx));
                        st32(ebp + idx + 2, esi);
                        ebp[idx + 6] = dh;
                        ebp[idx + 7] = (u1)((statusbit & 0xF8) | (u1)cpri);
                    }
                }
            }
        }
        dl = (u1)(dl + 1);
        esi += 8;
        if (--numleft2do == 0)
            break;
    }
    add_x();
    ADDCX(-8);
    return;
next:
    if (obj_x == 256)
        goto spec;
    dl = (u1)(dl + 8);
    add_x();
}

static void reprocessspriteflipy(void)
{
    if (S16(cx) <= -8)
        goto nextb;
    if (cx >= 256)
        goto nextb;
specb:
    ADDCX(8);
    for (;;) {
        if (!((u1)dl > (u1)resolutn)) {
            u4 y = dl;
            if (!(y < curypos)) {
                u1 ol = objleft;
                if (!(sprstart[y] > ol)) {
                    int ok;
                    if (sprtilecnt[y] <= 34)
                        ok = 1;
                    else if (sprend[y] < ol)
                        ok = 0;
                    else if (sprend[y] > ol)
                        ok = 1;
                    else
                        ok = !((u2)sprendx[y] < U16(cx));
                    if (ok) {
                        u1 oldtot = sprlefttot[y];
                        sprlefttot[y]++;
                        sprleftpr[y * 4 + cpri] = 1;
                        u4 idx = (y << 9) + ((u4)oldtot << 3);
                        st16(ebp + idx, U16(cx));
                        st32(ebp + idx + 2, esi);
                        ebp[idx + 6] = dh;
                        ebp[idx + 7] = (u1)((statusbit & 0xF8) | (u1)cpri);
                    }
                }
            }
        }
        dl = (u1)(dl + 1);
        esi -= 8;
        if (--numleft2do == 0)
            break;
    }
    ADDCX(-8);
    add_x();
    return;
nextb:
    if (obj_x == 256)
        goto specb;
    dl = (u1)(dl + 8);
    add_x();
}

// ---- increment/draw macros (pass-3) ----------------------------------------
#define NEXTSPRITE2RIGHT() \
    do {                   \
        dl = (u1)(dl - 8); \
        ADDCX(8);          \
        numleft2do = 8;    \
        reprocesssprite(); \
    } while (0)
#define NEXTSPRITE2RIGHTFLIPY() \
    do {                        \
        esi += 56;              \
        dl = (u1)(dl - 8);      \
        ADDCX(8);               \
        numleft2do = 8;         \
        reprocessspriteflipy(); \
    } while (0)
#define NEXTSPRITE2RIGHTFLIPX() \
    do {                        \
        dl = (u1)(dl - 8);      \
        ADDCX(-8);              \
        numleft2do = 8;         \
        reprocesssprite();      \
    } while (0)
#define NEXTSPRITE2RIGHTFLIPYX() \
    do {                         \
        esi += 56;               \
        dl = (u1)(dl - 8);       \
        ADDCX(-8);               \
        numleft2do = 8;          \
        reprocessspriteflipy();  \
    } while (0)

#define NEXTLINE16X16()     \
    do {                    \
        ADDCX(-8);          \
        add_y(2);           \
        numleft2do = 8;     \
        reprocesssprite();  \
        NEXTSPRITE2RIGHT(); \
    } while (0)
#define NEXTLINE16X16FY()        \
    do {                         \
        ADDCX(-8);               \
        add_y(2);                \
        esi += 56;               \
        dl = (u1)(dl - 16);      \
        numleft2do = 8;          \
        reprocessspriteflipy();  \
        NEXTSPRITE2RIGHTFLIPY(); \
    } while (0)
#define NEXTLINE16X16FX()        \
    do {                         \
        ADDCX(8);                \
        add_y(2);                \
        numleft2do = 8;          \
        reprocesssprite();       \
        NEXTSPRITE2RIGHTFLIPX(); \
    } while (0)
#define NEXTLINE16X16FYX()        \
    do {                          \
        ADDCX(8);                 \
        add_y(2);                 \
        esi += 56;                \
        dl = (u1)(dl - 16);       \
        numleft2do = 8;           \
        reprocessspriteflipy();   \
        NEXTSPRITE2RIGHTFLIPYX(); \
    } while (0)

#define NEXTLINE32X32()     \
    do {                    \
        ADDCX(-24);         \
        add_y(4);           \
        numleft2do = 8;     \
        reprocesssprite();  \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
    } while (0)
#define NEXTLINE32X32FY()        \
    do {                         \
        ADDCX(-24);              \
        add_y(4);                \
        esi += 56;               \
        dl = (u1)(dl - 16);      \
        numleft2do = 8;          \
        reprocessspriteflipy();  \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
    } while (0)
#define NEXTLINE32X32FX()        \
    do {                         \
        ADDCX(24);               \
        add_y(4);                \
        numleft2do = 8;          \
        reprocesssprite();       \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
    } while (0)
#define NEXTLINE32X32FYX()        \
    do {                          \
        ADDCX(24);                \
        add_y(4);                 \
        esi += 56;                \
        dl = (u1)(dl - 16);       \
        numleft2do = 8;           \
        reprocessspriteflipy();   \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
    } while (0)

#define NEXTLINE64X64()     \
    do {                    \
        ADDCX(-56);         \
        add_y(8);           \
        numleft2do = 8;     \
        reprocesssprite();  \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
        NEXTSPRITE2RIGHT(); \
    } while (0)
#define NEXTLINE64X64FY()        \
    do {                         \
        ADDCX(-56);              \
        add_y(8);                \
        esi += 56;               \
        dl = (u1)(dl - 16);      \
        numleft2do = 8;          \
        reprocessspriteflipy();  \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
        NEXTSPRITE2RIGHTFLIPY(); \
    } while (0)
#define NEXTLINE64X64FX()        \
    do {                         \
        ADDCX(56);               \
        add_y(8);                \
        numleft2do = 8;          \
        reprocesssprite();       \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
        NEXTSPRITE2RIGHTFLIPX(); \
    } while (0)
#define NEXTLINE64X64FYX()        \
    do {                          \
        ADDCX(56);                \
        add_y(8);                 \
        esi += 56;                \
        dl = (u1)(dl - 16);       \
        numleft2do = 8;           \
        reprocessspriteflipy();   \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
        NEXTSPRITE2RIGHTFLIPYX(); \
    } while (0)

#define STATSET()            \
    do {                     \
        dh &= 7;             \
        numleft2do = 8;      \
        dh = (u1)(dh << 4);  \
        dh = (u1)(dh + 128); \
    } while (0)

// ---- pass-3 size handlers --------------------------------------------------
static void process8x8sprite(void)
{
    if (S16(cx) <= -8)
        return;
    statusbit = dh;
    if (dh & 0x40) {
        STATSET();
        esi += 56;
        reprocessspriteflipy();
        return;
    }
    STATSET();
    reprocesssprite();
}

static void process16x16sprite(void)
{
    if (S16(cx) <= -16)
        return;
    statusbit = dh;
    if (dh & 0x20) {
        if (dh & 0x40) {
            STATSET();
            ADDCX(8);
            dl = (u1)(dl + 8);
            esi += 56;
            reprocessspriteflipy();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTLINE16X16FYX();
            return;
        }
        STATSET();
        ADDCX(8);
        reprocesssprite();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTLINE16X16FX();
        return;
    }
    if (dh & 0x40) {
        STATSET();
        dl = (u1)(dl + 8);
        esi += 56;
        reprocessspriteflipy();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTLINE16X16FY();
        return;
    }
    STATSET();
    reprocesssprite();
    NEXTSPRITE2RIGHT();
    NEXTLINE16X16();
}

static void process32x32sprite(void)
{
    if (S16(cx) <= -32)
        return;
    statusbit = dh;
    if (dh & 0x20) {
        if (dh & 0x40) {
            STATSET();
            ADDCX(24);
            dl = (u1)(dl + 24);
            esi += 56;
            reprocessspriteflipy();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTLINE32X32FYX();
            NEXTLINE32X32FYX();
            NEXTLINE32X32FYX();
            return;
        }
        STATSET();
        ADDCX(24);
        reprocesssprite();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        return;
    }
    if (dh & 0x40) {
        STATSET();
        dl = (u1)(dl + 24);
        esi += 56;
        reprocessspriteflipy();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTLINE32X32FY();
        NEXTLINE32X32FY();
        NEXTLINE32X32FY();
        return;
    }
    STATSET();
    reprocesssprite();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTLINE32X32();
    NEXTLINE32X32();
    NEXTLINE32X32();
}

static void process64x64sprite(void)
{
    if (S16(cx) <= -64)
        return;
    statusbit = dh;
    if (dh & 0x20) {
        if (dh & 0x40) {
            STATSET();
            ADDCX(56);
            dl = (u1)(dl + 56);
            esi += 56;
            reprocessspriteflipy();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTLINE64X64FYX();
            NEXTLINE64X64FYX();
            NEXTLINE64X64FYX();
            NEXTLINE64X64FYX();
            NEXTLINE64X64FYX();
            NEXTLINE64X64FYX();
            NEXTLINE64X64FYX();
            return;
        }
        STATSET();
        ADDCX(56);
        reprocesssprite();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTLINE64X64FX();
        NEXTLINE64X64FX();
        NEXTLINE64X64FX();
        NEXTLINE64X64FX();
        NEXTLINE64X64FX();
        NEXTLINE64X64FX();
        NEXTLINE64X64FX();
        return;
    }
    if (dh & 0x40) {
        STATSET();
        dl = (u1)(dl + 56);
        esi += 56;
        reprocessspriteflipy();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTLINE64X64FY();
        NEXTLINE64X64FY();
        NEXTLINE64X64FY();
        NEXTLINE64X64FY();
        NEXTLINE64X64FY();
        NEXTLINE64X64FY();
        NEXTLINE64X64FY();
        return;
    }
    STATSET();
    reprocesssprite();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTLINE64X64();
    NEXTLINE64X64();
    NEXTLINE64X64();
    NEXTLINE64X64();
    NEXTLINE64X64();
    NEXTLINE64X64();
    NEXTLINE64X64();
}

static void process16x32sprite(void)
{
    if (S16(cx) <= -16)
        return;
    statusbit = dh;
    if (dh & 0x20) {
        if (dh & 0x40) {
            STATSET();
            ADDCX(8);
            dl = (u1)(dl + 8);
            esi += 56;
            reprocessspriteflipy();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTLINE16X16FYX();
            dl = (u1)(dl + 32);
            NEXTLINE16X16FYX();
            NEXTLINE16X16FYX();
            return;
        }
        STATSET();
        ADDCX(8);
        reprocesssprite();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTLINE16X16FX();
        NEXTLINE16X16FX();
        NEXTLINE16X16FX();
        return;
    }
    if (dh & 0x40) {
        STATSET();
        dl = (u1)(dl + 8);
        esi += 56;
        reprocessspriteflipy();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTLINE16X16FY();
        dl = (u1)(dl + 32);
        NEXTLINE16X16FY();
        NEXTLINE16X16FY();
        return;
    }
    STATSET();
    reprocesssprite();
    NEXTSPRITE2RIGHT();
    NEXTLINE16X16();
    NEXTLINE16X16();
    NEXTLINE16X16();
}

static void process32x64sprite(void)
{
    if (S16(cx) <= -32)
        return;
    statusbit = dh;
    if (dh & 0x20) {
        if (dh & 0x40) {
            STATSET();
            ADDCX(24);
            dl = (u1)(dl + 24);
            esi += 56;
            reprocessspriteflipy();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTSPRITE2RIGHTFLIPYX();
            NEXTLINE32X32FYX();
            NEXTLINE32X32FYX();
            NEXTLINE32X32FYX();
            dl = (u1)(dl + 64);
            NEXTLINE32X32FYX();
            NEXTLINE32X32FYX();
            NEXTLINE32X32FYX();
            NEXTLINE32X32FYX();
            return;
        }
        STATSET();
        ADDCX(24);
        reprocesssprite();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTSPRITE2RIGHTFLIPX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        NEXTLINE32X32FX();
        return;
    }
    if (dh & 0x40) {
        STATSET();
        dl = (u1)(dl + 24);
        esi += 56;
        reprocessspriteflipy();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTSPRITE2RIGHTFLIPY();
        NEXTLINE32X32FY();
        NEXTLINE32X32FY();
        NEXTLINE32X32FY();
        dl = (u1)(dl + 64);
        NEXTLINE32X32FY();
        NEXTLINE32X32FY();
        NEXTLINE32X32FY();
        NEXTLINE32X32FY();
        return;
    }
    STATSET();
    reprocesssprite();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTSPRITE2RIGHT();
    NEXTLINE32X32();
    NEXTLINE32X32();
    NEXTLINE32X32();
    NEXTLINE32X32();
    NEXTLINE32X32();
    NEXTLINE32X32();
    NEXTLINE32X32();
}

// ---- pass-1 / pass-2 RTO handlers (do_RTO / do_RTO2) -----------------------
static void do_rto(int W, int H)
{
    if (S16(cx) <= -W)
        return;
    u1 bl = dl;
    for (int i = 0; i < H; i++) {
        u4 bx = bl;
        if (!(bx > resolutn) && !(bx < curypos)) {
            sprcnt[bx]++;
            if (sprcnt[bx] == 32)
                sprstart[bx] = objleft;
        }
        bl = (u1)(bl + 1);
    }
}

static void do_rto2(int W, int H)
{
    if (S16(cx) <= -W)
        return;
    u1 bl = dl;
    for (int yi = 0; yi < H; yi++) {
        u1 dhl = (u1)(129 - objleft);
        u4 bx = bl;
        if (!(sprstart[bx] > dhl) && !(bx > resolutn) && !(bx < curypos)) {
            int cols = W >> 3;
            int saved = cx;
            for (int c = 0; c < cols; c++) {
                int doit;
                if (obj_x == 256)
                    doit = 1;
                else if (S16(cx) <= -8)
                    doit = 0;
                else if (cx >= 256)
                    doit = 0;
                else
                    doit = 1;
                if (doit) {
                    sprtilecnt[bx]++;
                    if (sprtilecnt[bx] == 34) {
                        sprend[bx] = dhl;
                        u2 v = U16(cx);
                        v = (u2)(v + 8);
                        sprendx[bx] = v;
                    }
                }
                ADDCX(8);
            }
            cx = saved;
        }
        bl = (u1)(bl + 1);
    }
}

// ---- size-table selection --------------------------------------------------
typedef void (*sizefn)(void);

static void select_size(u1 osize, int is_size1, sizefn* fn, int* w, int* h)
{
    switch (osize) {
    case 1:
        *fn = process8x8sprite;
        *w = 8;
        *h = 8;
        return;
    case 4:
        *fn = process16x16sprite;
        *w = 16;
        *h = 16;
        return;
    case 16:
        *fn = process32x32sprite;
        *w = 32;
        *h = 32;
        return;
    case 64:
        *fn = process64x64sprite;
        *w = 64;
        *h = 64;
        return;
    default:
        if (is_size1) {
            if (interlval & 2) {
                *fn = process16x16sprite;
                *w = 16;
                *h = 16;
            } else {
                *fn = process16x32sprite;
                *w = 16;
                *h = 32;
            }
        } else {
            *fn = process32x64sprite;
            *w = 32;
            *h = 64;
        }
    }
}

// ---- processspritesb (live path) -------------------------------------------
void processspritesb(void)
{
    sizefn s1fn, s2fn;
    int s1w, s1h, s2w, s2h;
    select_size(objsize1, 1, &s1fn, &s1w, &s1h);
    select_size(objsize2, 0, &s2fn, &s2w, &s2h);

    addr2add = 0;
    int base = (((int)objhipr << 2) & 0x1FC);
    objvramloc = (int)(objptr << 1);
    objvramloc2 = (int)((objptrn - objptr) << 1);
    ebp = spritetablea;

    int bx = base;

    // pass 1: .objloop_rto (forward)
    objleft = 128;
    do {
        int savebx = bx;
        dl = (u1)(oamram[bx + 1] + 1);
        u1 alx = oamram[bx];
        u1 cl = (u1)bx;
        int ebxs = bx >> 4;
        cl = (u1)((cl >> 1) & 6);
        u1 ah = oamram[ebxs + 512];
        ah = (u1)((ah >> cl) & 3);
        u1 ch = (u1)(ah & 1);
        cx = (ch << 8) | alx;
        if (U16(cx) >= 384)
            ADDCX(65535 - 511);
        if (!(S16(cx) > 256)) {
            if (ah & 2)
                do_rto(s2w, s2h);
            else
                do_rto(s1w, s1h);
        }
        bx = savebx;
        bx = (bx + 4) & 0x1FC;
    } while (--objleft);
    bx = (bx - 4) & 0x1FC;

    // pass 2: .objloop_rto2 (backward)
    objleft = 128;
    do {
        int savebx = bx;
        dl = (u1)(oamram[bx + 1] + 1);
        u1 alx = oamram[bx];
        u1 cl = (u1)bx;
        int ebxs = bx >> 4;
        cl = (u1)((cl >> 1) & 6);
        u1 ah = oamram[ebxs + 512];
        ah = (u1)((ah >> cl) & 3);
        u1 ch = (u1)(ah & 1);
        cx = (ch << 8) | alx;
        if (U16(cx) >= 384)
            ADDCX(65535 - 511);
        obj_x = U16(cx);
        if (!(S16(cx) > 256)) {
            if (ah & 2)
                do_rto2(s2w, s2h);
            else
                do_rto2(s1w, s1h);
        }
        bx = savebx;
        bx = (bx - 4) & 0x1FC;
    } while (--objleft);
    bx = (bx + 4) & 0x1FC;

    // pass 3: .objloop (forward, emit sprite table)
    objleft = 128;
    do {
        u1 attr_lo = oamram[bx + 2];
        u1 attr_hi = oamram[bx + 3];
        cpri = (attr_hi >> 4) & 3;
        int savebx = bx;
        dl = (u1)(oamram[bx + 1] + 1);
        dh = (u1)(attr_hi >> 1);
        u1 ch1 = (u1)(attr_hi & 1);
        int tilenum = (ch1 << 8) | attr_lo;
        int sc = (tilenum << 6);
        sprt_char = sc & 0x3FFF;
        int ecx0 = sc + objvramloc;
        if (attr_hi & 1)
            ecx0 += objvramloc2;
        ecx0 &= 0x1FFFF;
        esi = (u4)(uintptr_t)vcache4b + (u4)ecx0;
        objloc = esi - (u4)sprt_char;
        u1 alx = oamram[bx];
        u1 cl = (u1)bx;
        int ebxs = bx >> 4;
        cl = (u1)((cl >> 1) & 6);
        u1 ah = oamram[ebxs + 512];
        ah = (u1)((ah >> cl) & 3);
        u1 ch = (u1)(ah & 1);
        cx = (ch << 8) | alx;
        if (U16(cx) >= 384)
            ADDCX(65535 - 511);
        obj_x = U16(cx);
        if (!(S16(cx) > 256)) {
            if (ah & 2)
                s2fn();
            else
                s1fn();
        }
        bx = savebx;
        bx = (bx + 4) & 0x1FC;
    } while (--objleft);
}

// ---- processsprites: legacy dispatch (sprprifix is hardcoded 1) ------------
void processsprites(void)
{
    if (sprprifix != 0) {
        processspritesb();
        return;
    }
    // Legacy path is dead code (sprprifix is never zero in the emulator).
}

// ---- cachesprites ----------------------------------------------------------
void cachesprites(void)
{
    u1* objptr_p = oamram + 512;
    u1 curobjtype = *objptr_p;
    int objleftinbyte = 4;
    u1* esi_oam = oamram + 2;

    u2 nbg = (u2)((objptr & 0xFFFF) >> 4);
    u2 nbg2 = (u2)((objptrn & 0xFFFF) >> 4);

    int sprnum = 3;

    for (int objremain = 128; objremain > 0; objremain--) {
        u2 byte2add;
        u1 byte2move, byteb4add, num2do_count, num2do_size;
        if (curobjtype & 2) {
            num2do_count = objsize2;
            num2do_size = objsize2;
            byte2add = objadds2;
            byte2move = objmovs2;
            byteb4add = objmovs2;
        } else {
            num2do_count = objsize1;
            num2do_size = objsize1;
            byte2add = objadds1;
            byte2move = objmovs1;
            byteb4add = objmovs1;
        }
        curobjtype = (u1)(curobjtype >> 2);
        if (--objleftinbyte == 0) {
            objleftinbyte = 4;
            objptr_p++;
            curobjtype = *objptr_p;
        }

        u2 curobj = ld16(esi_oam);
        curobj = (u2)(curobj & 0x01FF); // and bh,1

        do {
            u1 cl = oamram[sprnum - 2];
            u1 chk = (u1)(curypos - 1);
            int skip = 0;
            if (cl < chk) {
                skip = 1;
            } else {
                u1 rch = (u1)(resolutn - 1);
                if (cl <= rch) {
                    /* okayres */
                } else if (num2do_size >= 8) {
                    /* okayres */
                } else if (num2do_size == 1) {
                    if (cl + 8 <= 0xFF)
                        skip = 1;
                } else {
                    if (cl + 16 <= 0xFF)
                        skip = 1;
                }
            }

            if (!skip) {
                int namebase = (oamram[sprnum] & 1);
                u2 bx2 = curobj;
                bx2 = (u2)(bx2 * 2);
                bx2 = (u2)(bx2 + (namebase ? nbg2 : nbg));
                bx2 &= 4095;
                u4 eb = bx2;
                if (ld16((u1*)vidmemch4 + eb) & 0x0101) {
                    st16((u1*)vidmemch4 + eb, 0);
                    u4 sprfillpl = eb;
                    u2 srcbx = (u2)(eb << 4);
                    u1* src = vram + srcbx;
                    u4 dstoff = 2u * (u4)srcbx;
                    u1* dst = vcache4b + dstoff;
                    u1 tiletypec = 3;
                    for (int row = 0; row < 8; row++) {
                        u1 cl2 = src[0], ch2 = src[1];
                        u1 dl2 = src[16], dh2 = src[17];
                        for (int px = 0; px < 8; px++) {
                            u1 al = 0;
                            al = (u1)((al << 1) | (dh2 >> 7));
                            dh2 = (u1)(dh2 << 1);
                            al = (u1)((al << 1) | (dl2 >> 7));
                            dl2 = (u1)(dl2 << 1);
                            al = (u1)((al << 1) | (ch2 >> 7));
                            ch2 = (u1)(ch2 << 1);
                            al = (u1)((al << 1) | (cl2 >> 7));
                            cl2 = (u1)(cl2 << 1);
                            dst[px] = al;
                            if (al)
                                tiletypec &= 1;
                            else
                                tiletypec &= 2;
                        }
                        dst += 8;
                        src += 2;
                    }
                    tltype4b[sprfillpl >> 1] = tiletypec;
                }
            }

            // .nocache: advance curobj
            {
                u2 b = curobj;
                b = (u2)(b << 4);
                b = (u2)((b & 0xFF00) | ((b + 0x10) & 0xFF));
                b = (u2)(b >> 4);
                curobj = b;
            }
            if (--byteb4add == 0) {
                u2 b = curobj;
                b = (u2)(b << 4);
                u1 al = (u1)((byte2add & 0xFF) << 4);
                b = (u2)((b & 0xFF00) | ((b + al) & 0xFF));
                b = (u2)(b >> 4);
                b = (u2)((b & 0xFF00) | ((b + 0x10) & 0xFF));
                curobj = b;
                byteb4add = byte2move;
            }
        } while (--num2do_count);

        esi_oam += 4;
        sprnum += 4;
    }
}
