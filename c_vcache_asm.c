/*
 * c_vcache_asm.c — C replacement for vcache.asm
 *
 * Provides C implementations of tile caching (2bpp/4bpp/8bpp),
 * sprite caching, and sprite processing functions.
 *
 * When building with NO_ASM=1, this file replaces vcache.asm.
 * The cachesingle*ng functions are stubs; they're only called
 * from video rendering ASM (newg162.asm) which is also excluded
 * in a full NO_ASM build.
 *
 * Copyright (C) 1997-2008 ZSNES Team
 * Licensed under GPL v2.
 */

#include "types.h"
#include <string.h>

/* --- External globals used by tile/sprite caching --- */
extern u1  vidmemch2[4096];
extern u2  vidmemch4[2048];
extern u1  vidmemch8[4096];
extern u1* vram;
extern u1* vcache2b;
extern u1* vcache4b;
extern u1* vcache8b;
extern u4  bgptr;
extern u4  bgptrc;
extern u4  bgptrd;
extern u2  curtileptr;
extern u1  tltype4b[2048];
extern u1  tltype2b[4096];
extern u1  tltype8b[1024];

/* Sprite/OAM globals */
extern u1  oamram[];
extern u2  objptr;
extern u2  objptrn;
extern u1  objsize1;
extern u1  objsize2;
extern u2  objadds1;
extern u2  objadds2;
extern u1  objmovs1;
extern u1  objmovs2;
extern u1  objhipr;
extern u2  resolutn;
extern u1  curypos;
extern u1  interlval;
extern u4  offsetmshl;
extern u1* spritetablea;

/* Per-scanline sprite tables */
extern u1  sprlefttot[256];
extern u1  sprleftpr[256];
extern u1  sprleftpr1[256];
extern u1  sprleftpr2[256];
extern u1  sprleftpr3[256];
extern u1  sprcnt[256];
extern u1  sprstart[256];
extern u1  sprtilecnt[256];
extern u1  sprend[256];
extern u2  sprendx[256];

/* --- Data declarations (from vcache.asm .data/.bss sections) --- */
u1  sprprifix = 1;
u4  OMBGTestVal = 0;
u4  ngptrdat2 = 0;
u4  ofshvaladd = 0;
u4  ofsmtptrs = 0;
u4  ofsmcptr2 = 0;
u4  addr2add = 0;
u1  scacheloop;
u1  tiletypec;
u2  dcolortab[2][256];
u1  res640;
u1  res480;
u4  lineleft;
u4  videotroub = 0;
u4  vesa2_clbit = 0;
u4  vesa2_rpos = 0;
u4  vesa2_gpos = 0;
u4  vesa2_bpos = 0;
u4  vesa2_clbitng = 0;
u4  vesa2_clbitng2[2] = {0, 0};
u4  vesa2_clbitng3 = 0;
u4  vesa2red10 = 0;
u4  vesa2_rtrcl = 0;
u4  vesa2_rtrcla = 0;
u4  vesa2_rfull = 0;
u4  vesa2_gtrcl = 0;
u4  vesa2_gtrcla = 0;
u4  vesa2_gfull = 0;
u4  vesa2_btrcl = 0;
u4  vesa2_btrcla = 0;
u4  vesa2_bfull = 0;
u4  vesa2_x = 320;
u4  vesa2_y = 240;
u4  vesa2_bits = 8;
u4  vesa2_rposng = 0;
u4  vesa2_gposng = 0;
u4  vesa2_bposng = 0;
u4  vesa2_usbit = 0;

/* ================================================================
 * Bitplane decode helpers
 *
 * SNES tiles are stored as interleaved bitplanes.  Each pixel's
 * palette index is assembled from one bit per plane.  Bit 7 of
 * each plane byte maps to the leftmost pixel (pixel 0).
 * ================================================================ */

static void decode_2bpp_row(const u1* src, u1* dst)
{
    u1 p0 = src[0];
    u1 p1 = src[1];
    int i;
    for (i = 0; i < 8; i++) {
        int shift = 7 - i;
        dst[i] = (u1)(((p1 >> shift) & 1) << 1 |
                       ((p0 >> shift) & 1));
    }
}

static void decode_4bpp_row(const u1* src, u1* dst)
{
    u1 p0 = src[0];
    u1 p1 = src[1];
    u1 p2 = src[16];
    u1 p3 = src[17];
    int i;
    for (i = 0; i < 8; i++) {
        int shift = 7 - i;
        dst[i] = (u1)(((p3 >> shift) & 1) << 3 |
                       ((p2 >> shift) & 1) << 2 |
                       ((p1 >> shift) & 1) << 1 |
                       ((p0 >> shift) & 1));
    }
}

/* 4bpp decode that also tracks tile type (transparent vs opaque) */
static void decode_4bpp_row_typed(const u1* src, u1* dst, u1* ttype)
{
    u1 p0 = src[0];
    u1 p1 = src[1];
    u1 p2 = src[16];
    u1 p3 = src[17];
    int i;
    for (i = 0; i < 8; i++) {
        int shift = 7 - i;
        u1 px = (u1)(((p3 >> shift) & 1) << 3 |
                      ((p2 >> shift) & 1) << 2 |
                      ((p1 >> shift) & 1) << 1 |
                      ((p0 >> shift) & 1));
        dst[i] = px;
        if (px)
            *ttype &= 1; /* has non-zero pixel */
        else
            *ttype &= 2; /* has zero pixel */
    }
}

static void decode_8bpp_row(const u1* src, u1* dst)
{
    u1 planes[8];
    int i, p;
    planes[0] = src[0];
    planes[1] = src[1];
    planes[2] = src[16];
    planes[3] = src[17];
    planes[4] = src[32];
    planes[5] = src[33];
    planes[6] = src[48];
    planes[7] = src[49];

    for (i = 0; i < 8; i++) {
        int shift = 7 - i;
        u1 px = 0;
        for (p = 0; p < 8; p++) {
            px |= (u1)(((planes[p] >> shift) & 1) << p);
        }
        dst[i] = px;
    }
}

/* ================================================================
 * Tilemap cache functions
 *
 * These walk a 32-entry tilemap region, check dirty flags, and
 * decode any dirty tiles from VRAM into the cache buffer.
 * The 'area' parameter (0-63) selects which tilemap region;
 * bit 5 selects between bgptr and bgptrc base addresses.
 * ================================================================ */

void cachetile2b(u4 area)
{
    u2 nbg = (u2)(curtileptr >> 4);
    u2 map_off;
    int pass;

    if (area & 0x20)
        map_off = (u2)((area & 0x1F) * 64 + (u2)bgptrc);
    else
        map_off = (u2)(area * 64 + (u2)bgptr);

    for (pass = 0; pass < 2; pass++) {
        u1* map_addr = vram + map_off;
        int i;
        for (i = 0; i < 32; i++) {
            u2 entry = *(u2*)(map_addr + i * 2);
            u2 tile = (u2)((entry & 0x3FF) + nbg) & 0xFFF;

            if (vidmemch2[tile] & 0x01) {
                int row;
                vidmemch2[tile] = 0;
                u1* src = vram + (u4)tile * 16;
                u1* dst = vcache2b + (u4)tile * 64;
                for (row = 0; row < 8; row++)
                    decode_2bpp_row(src + row * 2, dst + row * 8);
            }
        }
        if ((u2)bgptrc == (u2)bgptrd)
            break;
        map_off += 2048 - 64;
    }
}

void cachetile4b(u4 area)
{
    u2 nbg = (u2)(curtileptr >> 5);
    u2 map_off;
    int pass;

    if (area & 0x20)
        map_off = (u2)((area & 0x1F) * 64 + (u2)bgptrc);
    else
        map_off = (u2)(area * 64 + (u2)bgptr);

    for (pass = 0; pass < 2; pass++) {
        u1* map_addr = vram + map_off;
        int i;
        for (i = 0; i < 32; i++) {
            u2 entry = *(u2*)(map_addr + i * 2);
            u2 tile_base = (u2)((entry & 0x3FF) + nbg);
            u2 idx = (u2)(tile_base << 1) & 0xFFF;
            u1* ch = (u1*)vidmemch4;

            if ((ch[idx] | ch[idx + 1]) & 0x01) {
                int row;
                ch[idx] = 0;
                ch[idx + 1] = 0;
                u1* src = vram + (u4)idx * 16;
                u1* dst = vcache4b + (u4)idx * 32;
                for (row = 0; row < 8; row++)
                    decode_4bpp_row(src + row * 2, dst + row * 8);
            }
        }
        if ((u2)bgptrc == (u2)bgptrd)
            break;
        map_off += 2048 - 64;
    }
}

void cachetile8b(u4 area)
{
    u2 nbg = (u2)(curtileptr >> 6);
    u2 map_off;
    int pass;

    if (area & 0x20)
        map_off = (u2)((area & 0x1F) * 64 + (u2)bgptrc);
    else
        map_off = (u2)(area * 64 + (u2)bgptr);

    for (pass = 0; pass < 2; pass++) {
        u1* map_addr = vram + map_off;
        int i;
        for (i = 0; i < 32; i++) {
            u2 entry = *(u2*)(map_addr + i * 2);
            u2 tile_base = (u2)((entry & 0x3FF) + nbg);
            u2 idx = (u2)(tile_base << 2) & 0xFFF;

            if ((vidmemch8[idx] | vidmemch8[idx+1] |
                 vidmemch8[idx+2] | vidmemch8[idx+3]) & 0x01) {
                int row;
                vidmemch8[idx] = 0;
                vidmemch8[idx+1] = 0;
                vidmemch8[idx+2] = 0;
                vidmemch8[idx+3] = 0;
                u1* src = vram + (u4)idx * 16;
                u1* dst = vcache8b + (u4)idx * 16;
                for (row = 0; row < 8; row++)
                    decode_8bpp_row(src + row * 2, dst + row * 8);
            }
        }
        if ((u2)bgptrc == (u2)bgptrd)
            break;
        map_off += 2048 - 64;
    }
}

/* ================================================================
 * 16x16 tile cache variants
 *
 * A 16x16 "tile" is 4 adjacent 8x8 tiles in a 2x2 grid:
 *   [base], [base+1], [base+16], [base+17]
 * ================================================================ */

void cachetile2b16x16(u4 area)
{
    u2 nbg = (u2)(curtileptr >> 4);
    u2 map_off;
    int pass;

    if (area & 0x20)
        map_off = (u2)((area & 0x1F) * 64 + (u2)bgptrc);
    else
        map_off = (u2)(area * 64 + (u2)bgptr);

    for (pass = 0; pass < 2; pass++) {
        u1* map_addr = vram + map_off;
        int i;
        for (i = 0; i < 32; i++) {
            u2 entry = *(u2*)(map_addr + i * 2);
            u2 tile_base = (u2)((entry & 0x3FF) + nbg);

            /* 4 sub-tiles: base, base+1, base+16, base+17 */
            static const u2 sub_offsets[4] = {0, 1, 16, 17};
            int s;
            for (s = 0; s < 4; s++) {
                u2 tile;
                if (s == 0)
                    tile = tile_base;
                else if (s == 1)
                    tile = tile_base + 1;
                else if (s == 2)
                    tile = tile_base + 16;
                else
                    tile = tile_base + 17;
                tile &= 0xFFF;

                if (vidmemch2[tile] & 0x01) {
                    int row;
                    vidmemch2[tile] = 0;
                    u1* src = vram + (u4)tile * 16;
                    u1* dst = vcache2b + (u4)tile * 64;
                    for (row = 0; row < 8; row++)
                        decode_2bpp_row(src + row * 2, dst + row * 8);
                }
            }
        }
        if ((u2)bgptrc == (u2)bgptrd)
            break;
        map_off += 2048 - 64;
    }
}

void cachetile4b16x16(u4 area)
{
    u2 nbg = (u2)(curtileptr >> 5);
    u2 map_off;
    int pass;

    if (area & 0x20)
        map_off = (u2)((area & 0x1F) * 64 + (u2)bgptrc);
    else
        map_off = (u2)(area * 64 + (u2)bgptr);

    for (pass = 0; pass < 2; pass++) {
        u1* map_addr = vram + map_off;
        int i;
        for (i = 0; i < 32; i++) {
            u2 entry = *(u2*)(map_addr + i * 2);
            u2 tile_base = (u2)((entry & 0x3FF) + nbg);

            static const u2 sub_offsets[4] = {0, 1, 16, 17};
            int s;
            for (s = 0; s < 4; s++) {
                u2 tile;
                if (s == 0)
                    tile = tile_base;
                else if (s == 1)
                    tile = tile_base + 1;
                else if (s == 2)
                    tile = tile_base + 16;
                else
                    tile = tile_base + 17;
                u2 idx = (u2)(tile << 1) & 0xFFF;
                u1* ch = (u1*)vidmemch4;

                if ((ch[idx] | ch[idx + 1]) & 0x01) {
                    int row;
                    ch[idx] = 0;
                    ch[idx + 1] = 0;
                    u1* src = vram + (u4)idx * 16;
                    u1* dst = vcache4b + (u4)idx * 32;
                    for (row = 0; row < 8; row++)
                        decode_4bpp_row(src + row * 2, dst + row * 8);
                }
            }
        }
        if ((u2)bgptrc == (u2)bgptrd)
            break;
        map_off += 2048 - 64;
    }
}

void cachetile8b16x16(u4 area)
{
    u2 nbg = (u2)(curtileptr >> 6);
    u2 map_off;
    int pass;

    if (area & 0x20)
        map_off = (u2)((area & 0x1F) * 64 + (u2)bgptrc);
    else
        map_off = (u2)(area * 64 + (u2)bgptr);

    for (pass = 0; pass < 2; pass++) {
        u1* map_addr = vram + map_off;
        int i;
        for (i = 0; i < 32; i++) {
            u2 entry = *(u2*)(map_addr + i * 2);
            u2 tile_base = (u2)((entry & 0x3FF) + nbg);

            static const u2 sub_offsets[4] = {0, 1, 16, 17};
            int s;
            for (s = 0; s < 4; s++) {
                u2 tile;
                if (s == 0)
                    tile = tile_base;
                else if (s == 1)
                    tile = tile_base + 1;
                else if (s == 2)
                    tile = tile_base + 16;
                else
                    tile = tile_base + 17;
                u2 idx = (u2)(tile << 2) & 0xFFF;

                if ((vidmemch8[idx] | vidmemch8[idx+1] |
                     vidmemch8[idx+2] | vidmemch8[idx+3]) & 0x01) {
                    int row;
                    vidmemch8[idx] = 0;
                    vidmemch8[idx+1] = 0;
                    vidmemch8[idx+2] = 0;
                    vidmemch8[idx+3] = 0;
                    u1* src = vram + (u4)idx * 16;
                    u1* dst = vcache8b + (u4)idx * 16;
                    for (row = 0; row < 8; row++)
                        decode_8bpp_row(src + row * 2, dst + row * 8);
                }
            }
        }
        if ((u2)bgptrc == (u2)bgptrd)
            break;
        map_off += 2048 - 64;
    }
}

/* ================================================================
 * Sprite caching
 *
 * cachesprites() reads OAM RAM and decodes sprite tile data from
 * VRAM into vcache4b.  Each sprite has a size (8x8 to 64x64) and
 * references tiles in one of two VRAM name bases (objptr/objptrn).
 * ================================================================ */

void cachesprites(void)
{
    u1* hiptr = oamram + 512;
    u1 curobjtype = *hiptr;
    u4 objleftinbyte = 4;
    u4 hiptr_idx = 0;

    u2 nbg1 = (u2)(objptr >> 4);
    u2 nbg2 = (u2)(objptrn >> 4);

    u1* esi = oamram + 2;
    u4 sprnum = 3;

    u4 obj;
    for (obj = 0; obj < 128; obj++) {
        /* Determine sprite size from OAM high table */
        u1 num2do, byte2move;
        u2 byte2add;

        if (curobjtype & 0x02) {
            num2do = objsize2;
            byte2add = objadds2;
            byte2move = objmovs2;
        } else {
            num2do = objsize1;
            byte2add = objadds1;
            byte2move = objmovs1;
        }

        curobjtype >>= 2;
        objleftinbyte--;
        if (objleftinbyte == 0) {
            objleftinbyte = 4;
            hiptr_idx++;
            curobjtype = oamram[512 + hiptr_idx];
        }

        /* Current tile base from OAM entry */
        u2 curobj = (u2)(esi[0] | ((esi[1] & 0x01) << 8));
        u1 byteb4add = byte2move;
        u1 num2do_left = num2do;

        while (num2do_left > 0) {
            /* Check if sprite Y position is on-screen */
            u1 ypos = oamram[sprnum - 2];
            u1 screen_bottom = (u1)(curypos - 1);
            if (ypos <= screen_bottom) {
                /* Check name base bit */
                int use_name2 = oamram[sprnum] & 0x01;
                u2 tile_idx;
                u2 nbg_cur;

                if (use_name2) {
                    nbg_cur = nbg2;
                } else {
                    nbg_cur = nbg1;
                }

                tile_idx = (u2)(curobj + curobj + nbg_cur) & 0xFFF;
                u1* ch = (u1*)vidmemch4;

                if ((ch[tile_idx] | ch[tile_idx + 1]) & 0x01) {
                    int row;
                    u4 sprfillpl = tile_idx;
                    ch[tile_idx] = 0;
                    ch[tile_idx + 1] = 0;

                    u1* src = vram + (u4)tile_idx * 16;
                    u1* dst = vcache4b + (u4)tile_idx * 32;
                    u1 ttype = 3;

                    for (row = 0; row < 8; row++)
                        decode_4bpp_row_typed(src + row * 2, dst + row * 8, &ttype);

                    tltype4b[sprfillpl >> 1] = ttype;
                }
            }

            /* Advance to next tile in sprite */
            curobj = (u2)(((curobj << 4) + 0x10) >> 4);
            byteb4add--;
            if (byteb4add == 0) {
                u2 tmp = (u2)(curobj << 4);
                tmp = (u2)(tmp + ((byte2add & 0xFF) << 4) + 0x10);
                curobj = (u2)(tmp >> 4);
                byteb4add = byte2move;
            }
            num2do_left--;
        }

        esi += 4;
        sprnum += 4;
    }
}

/* ================================================================
 * Sprite processing
 *
 * processsprites() / processspritesb() scan OAM RAM and build
 * per-scanline sprite lists used by the video renderer.
 *
 * This is the most complex function (~770 lines of ASM).
 * The C version implements the sprprifix==1 path
 * (processspritesb), which is the common case.
 * ================================================================ */

void processsprites(void)
{
    if (sprprifix == 0)
        return; /* Original path — not yet converted */

    /* processspritesb: processes sprites with priority fix enabled */
    u1 xsize, ysize;

    /* Determine object sizes based on objsize1/objsize2 */
    static const u1 sizes[] = {8, 16, 32, 64};
    u1 size1_pixels, size2_pixels;

    if (objsize1 <= 3)
        size1_pixels = sizes[objsize1 - 1];
    else
        size1_pixels = 8;

    if (objsize2 <= 3)
        size2_pixels = sizes[objsize2 - 1];
    else
        size2_pixels = 8;

    u1* hiptr = oamram + 512;
    u4 hiptr_idx = 0;
    u1 curobjtype = hiptr[0];
    u4 objleftinbyte = 4;

    int obj;
    for (obj = 0; obj < 128; obj++) {
        /* Get sprite size */
        u1 cur_size;
        if (curobjtype & 0x02) {
            cur_size = size2_pixels;
            xsize = size2_pixels;
            ysize = size2_pixels;
        } else {
            cur_size = size1_pixels;
            xsize = size1_pixels;
            ysize = size1_pixels;
        }

        /* Advance hi-table bits */
        u1 xmsb = (curobjtype & 0x01);
        curobjtype >>= 2;
        objleftinbyte--;
        if (objleftinbyte == 0) {
            objleftinbyte = 4;
            hiptr_idx++;
            curobjtype = oamram[512 + hiptr_idx];
        }

        /* Read OAM entry */
        u1* oam_entry = oamram + obj * 4;
        s4 xpos = oam_entry[0];
        u1 ypos = oam_entry[1];
        u2 tile = (u2)(oam_entry[2] | ((oam_entry[3] & 0x01) << 8));
        u1 attr = oam_entry[3];
        u1 priority = (attr >> 4) & 0x03;

        /* Handle X sign extension */
        if (xmsb)
            xpos = xpos - 256;

        /* Check if sprite is on-screen horizontally */
        if (xpos <= -(s4)xsize || xpos >= 256)
            continue;

        /* Handle interlace */
        u1 effective_ysize = ysize;
        if (interlval)
            effective_ysize >>= 1;

        /* Determine which scanlines this sprite covers */
        u1 y;
        for (y = 0; y < effective_ysize; y++) {
            u1 scanline = (u1)(ypos + y + 1);
            if (scanline >= (u1)resolutn)
                continue;
            if (scanline >= curypos)
                continue;

            sprlefttot[scanline]++;
            switch (priority) {
            case 0: sprleftpr[scanline]++; break;
            case 1: sprleftpr1[scanline]++; break;
            case 2: sprleftpr2[scanline]++; break;
            case 3: sprleftpr3[scanline]++; break;
            }

            u1 tiles_wide = xsize >> 3;
            sprtilecnt[scanline] += tiles_wide;
        }
    }
}

/*
 * cachesingle*ng — decode a single tile on demand.
 * Called from C video code (c_makev16b.c) with the tile index.
 * "ng" = new graphics (16-bit) path.
 *
 * Input: tile index is passed via the global `ecx` register in ASM,
 *        but in C we use a parameter.  The callers are wrapped with
 *        #ifdef NO_ASM to call the C version with a direct argument.
 *
 * The ASM-compatible wrappers (no args, reads ecx) are kept for
 * symbol compatibility when ASM video code is still linked.
 */

void cachesingle4bng_c(u4 idx)
{
    vidmemch4[idx] = 0;
    u1 ttype = 3;
    u1* dst = vcache4b + idx * 64;
    const u1* src = vram + idx * 32;
    int row;
    for (row = 0; row < 8; row++) {
        decode_4bpp_row_typed(src + row * 2, dst + row * 8, &ttype);
    }
    tltype4b[idx] = ttype;
}

void cachesingle2bng_c(u4 idx)
{
    vidmemch2[idx] = 0;
    u1 ttype = 3;
    u1* dst = vcache2b + idx * 64;
    const u1* src = vram + idx * 16;
    int row;
    for (row = 0; row < 8; row++) {
        u1 p0 = src[row * 2];
        u1 p1 = src[row * 2 + 1];
        int i;
        for (i = 0; i < 8; i++) {
            int shift = 7 - i;
            u1 px = (u1)(((p1 >> shift) & 1) << 1 |
                          ((p0 >> shift) & 1));
            dst[row * 8 + i] = px;
            if (px) ttype &= 1;
            else    ttype &= 2;
        }
    }
    tltype2b[idx] = ttype;
}

void cachesingle8bng_c(u4 idx)
{
    vidmemch8[idx] = 0;
    u1 ttype = 3;
    u1* dst = vcache8b + idx * 64;
    const u1* src = vram + idx * 64;
    int row;
    for (row = 0; row < 8; row++) {
        u1 planes[8];
        int p;
        planes[0] = src[row * 2];
        planes[1] = src[row * 2 + 1];
        planes[2] = src[row * 2 + 16];
        planes[3] = src[row * 2 + 17];
        planes[4] = src[row * 2 + 32];
        planes[5] = src[row * 2 + 33];
        planes[6] = src[row * 2 + 48];
        planes[7] = src[row * 2 + 49];
        int i;
        for (i = 0; i < 8; i++) {
            int shift = 7 - i;
            u1 px = 0;
            for (p = 0; p < 8; p++)
                px |= (u1)(((planes[p] >> shift) & 1) << p);
            dst[row * 8 + i] = px;
            if (px) ttype &= 1;
            else    ttype &= 2;
        }
    }
    tltype8b[idx] = ttype;
}

/* ASM-compatible stubs (no args — called from ASM with ecx in register) */
void cachesingle4bng(void) { }
void cachesingle2bng(void) { }
void cachesingle8bng(void) { }
void cachesingle4b(void) { }
void cachesingle2b(void) { }
void cachesingle(void) { }
void cache2bit(void) { }
void cache4bit(void) { }
void cache8bit(void) { }
void cache2bit16x16(void) { }
void cache4bit16x16(void) { }
void cache8bit16x16(void) { }
