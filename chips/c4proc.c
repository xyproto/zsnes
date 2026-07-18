/*
 * Capcom C4 coprocessor interface, ported from chips/c4proc.asm.
 *
 * The C4 maps into $6000-$7FFF: the four C4*8b/16b entry points route
 * addresses the same three ways as the OBC1/DSP4 ports (bit 15 set to
 * memaccessbank, below $6000 to regaccessbank, otherwise C4 RAM).  A
 * write to $7F47 triggers the ROM-to-RAM copy and a write to $7F4F runs
 * the command dispatcher.  The math helpers (C4Op*, C4TransfWireFrame*,
 * C4CalcWireFrame, Sin/CosTable) live in c4emu.c.
 *
 * The asm's per-address function-pointer tables (C4RamR/C4RamW) only
 * ever held C4ReadReg/C4WriteReg plus one C4RegFunction entry, so they
 * are replaced by direct dispatch here.  The debug-only C4Edit and
 * C4ProcessVectors routines and the unreachable DoScaleRotate2 were
 * dead code and are not ported.
 */

#include <stdint.h>
#include <string.h>

#include "c4proc.h"

extern uint8_t* romdata;
extern uint8_t* snesmmap[256];

/* c4emu.c */
extern short C4WFXVal, C4WFYVal, C4WFZVal, C4WFX2Val, C4WFY2Val;
extern short C4WFDist, C4WFScale;
extern short C41FXVal, C41FYVal, C41FAngleRes, C41FDist, C41FDistVal;
extern short SinTable[512];
extern short CosTable[512];
void C4TransfWireFrame(void);
void C4TransfWireFrame2(void);
void C4CalcWireFrame(void);
void C4Op0D(void);
void C4Op15(void);
void C4Op1F(void);
void C4Op22(void);

/* routed bank handlers (cdecl in tests, asm register-ABI in the build) */
extern uint8_t regaccessbankr8(uint32_t addr);
extern void regaccessbankw8(uint32_t addr, uint8_t val);
extern uint16_t regaccessbankr16(uint32_t addr);
extern void regaccessbankw16(uint32_t addr, uint16_t val);
extern uint8_t memaccessbankr8(uint32_t addr);
extern void memaccessbankw8(uint32_t addr, uint8_t val);
extern uint16_t memaccessbankr16(uint32_t addr);
extern void memaccessbankw16(uint32_t addr, uint16_t val);

u1* C4Ram;
uint8_t C4ObjSelec, C4SObjSelec, C4Pause;
uint32_t C4values[3];

static uint8_t* C4Data;

#define BYTE(v, n) (((uint8_t*)&(v))[n])
#define RAMW(off) (*(uint16_t*)(C4Ram + (off)))

static uint8_t rol8(uint8_t v, int n) { return (uint8_t)(v << n | v >> (8 - n)); }
static uint8_t ror8(uint8_t v, int n) { return (uint8_t)(v >> n | v << (8 - n)); }
static uint16_t ror16(uint16_t v, int n) { return (uint16_t)(v >> n | v << (16 - n)); }

/* bank byte + 16-bit address to a LoROM offset in romdata */
static uint8_t* lorom_ptr(uint32_t bank, uint32_t addr)
{
    return romdata + (bank << 15) + (addr & 0x7FFF);
}

void InitC4(void)
{
    uint8_t* base = romdata + 4096 * 1024;

    C4Data = base + 128 * 1024;
    C4Ram = base + 8192 * 8;
    memset(C4Ram, 0, 8192 * 4);
    memset(C4Data, 0, 16 * 4096 * 4);
}

/* ---- sprite to OAM conversion ---- */

static uint32_t C4count, C4sprites, C4ObjDisp;
static uint16_t C4SprX, C4SprY;
static uint8_t C4SprCnt, C4SprAttr, C4SprOAM;
static uint8_t* c4_oam_ptr; /* edi in the asm */
static uint8_t* c4_hi_ptr; /* C4usprptr */
static uint8_t c4_dl, c4_dh; /* rolling OAM high-table masks */

static void c4_add_sprite(uint16_t xy, uint16_t oamattr)
{
    if (!C4count) {
        return;
    }
    *(uint16_t*)c4_oam_ptr = xy;
    *(uint16_t*)(c4_oam_ptr + 2) = oamattr;
    *c4_hi_ptr = (uint8_t)((*c4_hi_ptr & c4_dl) | (c4_dh & (uint8_t)~c4_dl));
    c4_oam_ptr += 4;
    c4_dl = rol8(c4_dl, 2);
    c4_dh = rol8(c4_dh, 2);
    C4count--;
    if (c4_dl == 0xFC) {
        c4_hi_ptr++;
    }
}

static void c4_conv_oam(void)
{
    uint8_t* ram = C4Ram;
    uint16_t addx = RAMW(0x621);
    uint16_t addy = RAMW(0x623);

    c4_hi_ptr = ram + 0x200 + (C4ObjDisp >> 4);
    c4_oam_ptr = ram + C4ObjDisp;
    c4_dl = rol8(0xFC, (C4sprites & 3) * 2);

    uint32_t objects = ram[0x620];
    if (objects == 0) {
        return;
    }
    C4count = 128 - C4sprites;

    uint8_t* obj = ram + 0x220;
    do {
        C4SprX = (uint16_t)(*(uint16_t*)obj - addx);
        C4SprY = (uint16_t)(*(uint16_t*)(obj + 2) - addy);
        C4SprOAM = obj[5];
        C4SprAttr = (uint8_t)(obj[4] | obj[6]);

        uint8_t* def = lorom_ptr(obj[9], *(uint16_t*)(obj + 7));
        if (*def) {
            C4SprCnt = *def++;
            do {
                int16_t x = (int8_t)def[1];
                if (C4SprAttr & 0x40) {
                    x = (int16_t)(-x - 8);
                }
                x = (int16_t)(x + (int16_t)C4SprX);
                c4_dh = 0;
                if (def[0] & 0x20) {
                    c4_dh |= 0xAA;
                    if (C4SprAttr & 0x40) {
                        x = (int16_t)(x - 8);
                    }
                }
                if (x >= -16 && x <= 272) {
                    if (x & 0x100) {
                        c4_dh |= 0x55;
                    }
                    int16_t y = (int8_t)def[2];
                    if (C4SprAttr & 0x80) {
                        y = (int16_t)(-y - 8);
                    }
                    y = (int16_t)(y + (int16_t)C4SprY);
                    if ((def[0] & 0x20) && (C4SprAttr & 0x80)) {
                        y = (int16_t)(y - 8);
                    }
                    if (y >= -16 && y <= 224) {
                        uint8_t attr = (uint8_t)(C4SprAttr ^ (def[0] & 0xC0));
                        uint8_t oam = (uint8_t)(C4SprOAM + def[3]);
                        c4_add_sprite((uint16_t)((uint8_t)x | (uint8_t)y << 8),
                            (uint16_t)(oam | attr << 8));
                    }
                }
                def += 4;
            } while (--C4SprCnt);
        } else {
            c4_dh = 0xAA;
            if (BYTE(C4SprX, 1) & 1) {
                c4_dh |= 0x55;
            }
            c4_add_sprite((uint16_t)(BYTE(C4SprX, 0) | BYTE(C4SprY, 0) << 8),
                (uint16_t)(C4SprOAM | C4SprAttr << 8));
        }
        obj += 16;
    } while (--objects);
}

void C4ProcessSprites(void)
{
    uint8_t* ram = C4Ram;

    C4sprites = ram[0x626];
    C4ObjDisp = C4sprites << 2;
    C4count = 32;

    /* clear the to-be OAM area */
    uint32_t n = 128 - C4sprites;
    uint8_t* p = ram + C4ObjDisp;
    do {
        p[1] = 0xE0;
        p += 4;
    } while (--n);

    c4_conv_oam();
}

/* ---- scale/rotate sprite rendering ---- */

static uint32_t C4SprPos;
static uint8_t* C4SprPtr;
static uint32_t C4SprPtrInc;
static uint16_t C4XXScale, C4XYScale, C4YXScale, C4YYScale;
static uint16_t C4CXPos, C4CYPos;
static uint32_t C4CXMPos, C4CYMPos, C4PCXMPos, C4PCYMPos;

/* zero width*height bytes at dst and dst+0x2000 (packed rows); row_units
   is the caller's register value, which for the rotate path differs from
   the stored C4SprPos height */
static void c4_clear_spr(uint8_t* dst, uint8_t row_units)
{
    uint8_t rows = (uint8_t)(row_units << 3);
    do {
        uint8_t n = (uint8_t)(BYTE(C4SprPos, 0) << 2);
        do {
            dst[0] = 0;
            dst[0x2000] = 0;
            dst++;
        } while (--n);
    } while (--rows);
}

/* convert packed 4-bit pixels at src into planar 4bpp tiles at dst;
   consumes BYTE(C4SprPos, 1) as its row counter */
static void c4_spr_bitplane(uint8_t* dst, uint32_t wtiles, uint8_t* src)
{
    uint32_t stride = wtiles << 2;
    do {
        uint8_t cols = BYTE(C4SprPos, 0);
        uint8_t* rowsrc = src;
        do {
            uint8_t* s = rowsrc;
            for (int row = 0; row < 8; row++) {
                uint32_t px = *(uint32_t*)s;
                uint8_t mask = 0x80;
                for (int i = 0; i < 8; i++) {
                    if (px & 1) {
                        dst[0] |= mask;
                    }
                    if (px & 2) {
                        dst[1] |= mask;
                    }
                    if (px & 4) {
                        dst[16] |= mask;
                    }
                    if (px & 8) {
                        dst[17] |= mask;
                    }
                    px >>= 4;
                    mask >>= 1;
                }
                s += stride;
                dst += 2;
            }
            dst += 16;
            rowsrc += 4;
        } while (--cols);
        src += stride * 8;
        dst += C4SprPtrInc;
    } while (--BYTE(C4SprPos, 1));
}

static void c4_scale_rotate(void)
{
    uint8_t* ram = C4Ram;

    /* X scaler */
    uint32_t angle = RAMW(0x1F80) & 0x1FF;
    int16_t sx = (int16_t)RAMW(0x1F8F);
    if (sx & 0x8000) {
        sx = 0x7FFF;
    }
    int32_t p = (int32_t)CosTable[angle] * sx;
    C4XXScale = (uint16_t)((uint32_t)p >> 15);
    p = (int32_t)SinTable[angle] * sx;
    C4XYScale = (uint16_t)((uint32_t)p >> 15);

    /* Y scaler; the asm drops the carry on the YY doubling */
    int16_t sy = (int16_t)RAMW(0x1F92);
    if (sy & 0x8000) {
        sy = 0x7FFF;
    }
    p = (int32_t)CosTable[angle] * sy;
    C4YYScale = (uint16_t)(((uint32_t)p >> 16) << 1);
    p = (int32_t)SinTable[angle] * sy;
    C4YXScale = (uint16_t)(-(int16_t)((uint32_t)p >> 15));
    if (RAMW(0x1F80) == 0 && RAMW(0x1F92) == 0x1000) {
        C4YYScale = 0x1000;
        C4YXScale = 0;
    }

    uint8_t w = (uint8_t)(BYTE(C4SprPos, 0) << 3);
    uint8_t h = (uint8_t)(BYTE(C4SprPos, 1) << 3);
    BYTE(C4SprPos, 2) = w;
    BYTE(C4SprPos, 3) = h;

    /* (1-scale)*(pos/2) start offsets, 4.12 fixed point */
    C4PCXMPos = (uint32_t)w << 11;
    C4PCYMPos = (uint32_t)h << 11;
    C4PCXMPos -= (uint32_t)((int32_t)(int16_t)C4XXScale * (w >> 1));
    C4PCXMPos -= (uint32_t)((int32_t)(int16_t)C4YXScale * (h >> 1));
    C4PCYMPos -= (uint32_t)((int32_t)(int16_t)C4XYScale * (w >> 1));
    C4PCYMPos -= (uint32_t)((int32_t)(int16_t)C4YYScale * (h >> 1));

    C4CYPos = 0;
    uint32_t out = 0;
    do {
        C4CXMPos = C4PCXMPos;
        C4CYMPos = C4PCYMPos;
        BYTE(C4CXPos, 0) = BYTE(C4SprPos, 2);
        do {
            uint8_t* dst = ram + (out >> 1) + 0x2000;
            if ((int32_t)C4CXMPos >> 12 >= (int32_t)w
                || (int32_t)C4CYMPos >> 12 >= (int32_t)h
                || (int32_t)C4CXMPos < 0 || (int32_t)C4CYMPos < 0) {
                /* out of source, blank the nibble */
                if (out & 1) {
                    *dst &= 0x0F;
                } else {
                    *dst &= 0xF0;
                }
            } else {
                uint32_t idx = (C4CYMPos >> 12) * w + (C4CXMPos >> 12);
                uint8_t v = C4SprPtr[idx >> 1];
                if (idx & 1) {
                    v >>= 4;
                }
                if (out & 1) {
                    *dst = (uint8_t)((*dst & 0x0F) | v << 4);
                } else {
                    *dst = (uint8_t)((*dst & 0xF0) | (v & 0x0F));
                }
            }
            C4CXMPos += (uint32_t)(int32_t)(int16_t)C4XXScale;
            C4CYMPos += (uint32_t)(int32_t)(int16_t)C4XYScale;
            out++;
        } while (--BYTE(C4CXPos, 0));
        C4PCXMPos += (uint32_t)(int32_t)(int16_t)C4YXScale;
        C4PCYMPos += (uint32_t)(int32_t)(int16_t)C4YYScale;
        C4CYPos++;
    } while (BYTE(C4CYPos, 0) != BYTE(C4SprPos, 3));
}

static void c4_spr_scale(void)
{
    uint8_t* ram = C4Ram;

    C4SprPtrInc = 0;
    C4SprPtr = lorom_ptr(ram[0x1F42], RAMW(0x1F40));
    BYTE(C4SprPos, 0) = (uint8_t)(ram[0x1F89] >> 3);
    BYTE(C4SprPos, 1) = (uint8_t)(ram[0x1F8C] >> 3);

    c4_clear_spr(ram, BYTE(C4SprPos, 1));
    c4_scale_rotate();
    c4_spr_bitplane(ram, BYTE(C4SprPos, 0), ram + 0x2000);
}

static void c4_spr_rotate(void)
{
    uint8_t* ram = C4Ram;

    C4SprPtr = ram + 0x600;
    BYTE(C4SprPos, 0) = (uint8_t)(ram[0x1F89] >> 3);
    BYTE(C4SprPos, 1) = (uint8_t)((ram[0x1F8C] >> 3) + 2);
    C4SprPtrInc = 64;
    BYTE(C4SprPos, 1) -= 2;

    /* the asm cleared with the pre-decrement height still in a register */
    c4_clear_spr(ram, (uint8_t)((ram[0x1F8C] >> 3) + 2));
    c4_scale_rotate();
    BYTE(C4SprPos, 1) += 2;
    c4_spr_bitplane(ram, BYTE(C4SprPos, 0), ram + 0x2000);
}

static void c4_spr_disintegrate(void)
{
    uint8_t* ram = C4Ram;

    C4SprPtrInc = 0;
    C4SprPtr = lorom_ptr(ram[0x1F42], RAMW(0x1F40));
    BYTE(C4SprPos, 0) = (uint8_t)(ram[0x1F89] >> 3);
    BYTE(C4SprPos, 1) = (uint8_t)(ram[0x1F8C] >> 3);

    c4_clear_spr(ram, BYTE(C4SprPos, 1));

    uint32_t scalex = (uint32_t)(int32_t)(int16_t)RAMW(0x1F86);
    uint32_t scaley = (uint32_t)(int32_t)(int16_t)RAMW(0x1F8F);
    uint32_t startx = ((uint32_t)(ram[0x1F89] >> 1) << 8)
        - (ram[0x1F89] >> 1) * (uint32_t)RAMW(0x1F86);
    uint32_t starty = ((uint32_t)(ram[0x1F8C] >> 1) << 8)
        - (ram[0x1F8C] >> 1) * (uint32_t)RAMW(0x1F8F);
    BYTE(C4SprPos, 2) = ram[0x1F89];
    BYTE(C4SprPos, 3) = ram[0x1F8C];

    /* expand the 4-bit source to one byte per pixel at +0x2000 */
    {
        uint8_t* src = C4SprPtr;
        uint8_t* dst = ram + 0x2000;
        uint8_t cl = (uint8_t)(BYTE(C4SprPos, 2) >> 1);
        uint8_t ch = BYTE(C4SprPos, 3);
        for (;;) {
            /* even bytes keep the whole source byte, like the asm */
            dst[0] = *src;
            dst[1] = (uint8_t)(*src >> 4);
            src++;
            dst += 2;
            if (--cl) {
                continue;
            }
            if (!--ch) {
                break;
            }
        }
    }

    memset(ram + 0x4000, 0, 0x2000);

    /* scatter the pixels to their scaled positions */
    {
        uint8_t* src = ram + 0x2000;
        uint8_t* dst = ram + 0x4000;
        uint32_t x, y = starty;
        uint8_t cl = BYTE(C4SprPos, 2);
        uint8_t ch = BYTE(C4SprPos, 3);
        do {
            x = startx;
            cl = BYTE(C4SprPos, 2);
            do {
                if (x < (uint32_t)BYTE(C4SprPos, 2) << 8
                    && y < (uint32_t)BYTE(C4SprPos, 3) << 8) {
                    uint32_t idx = BYTE(C4SprPos, 2) * (y >> 8) + (x >> 8);
                    if (idx < 0x2000) {
                        dst[idx] = *src;
                    }
                }
                src++;
                x += scalex;
            } while (--cl);
            y += scaley;
        } while (--ch);
    }

    /* pack back to 4-bit at +0x6000 */
    {
        uint8_t* src = ram + 0x4000;
        uint8_t* dst = ram + 0x6000;
        uint8_t cl = (uint8_t)(BYTE(C4SprPos, 2) >> 1);
        uint8_t ch = BYTE(C4SprPos, 3);
        for (;;) {
            *dst++ = (uint8_t)(src[0] | src[1] << 4);
            src += 2;
            if (--cl) {
                continue;
            }
            if (!--ch) {
                break;
            }
        }
    }

    c4_spr_bitplane(ram, BYTE(C4SprPos, 0), ram + 0x6000);
}

/* ---- bitplane wave (op 00/0C) ---- */

static const uint32_t c4_bmptr[40] = {
    0x0000, 0x0002, 0x0004, 0x0006, 0x0008, 0x000A, 0x000C, 0x000E,
    0x0200, 0x0202, 0x0204, 0x0206, 0x0208, 0x020A, 0x020C, 0x020E,
    0x0400, 0x0402, 0x0404, 0x0406, 0x0408, 0x040A, 0x040C, 0x040E,
    0x0600, 0x0602, 0x0604, 0x0606, 0x0608, 0x060A, 0x060C, 0x060E,
    0x0800, 0x0802, 0x0804, 0x0806, 0x0808, 0x080A, 0x080C, 0x080E
};

static void c4_bitplane_wave_block(uint8_t* dst, uint32_t* waveptr,
    uint16_t* mask1, uint16_t* mask2, uint32_t tblofs)
{
    do {
        int16_t height = (int16_t)(-(int8_t)C4Ram[*waveptr + 0xB00] - 16);
        for (int i = 0; i < 40; i++) {
            uint16_t bits = 0;
            if (height >= 0) {
                bits = 0xFF00;
                if (height < 8) {
                    bits = *(uint16_t*)(C4Ram + 0xA00 + height * 2 + tblofs);
                }
            }
            uint16_t* w = (uint16_t*)(dst + c4_bmptr[i]);
            *w = (uint16_t)((*w & *mask2) | (bits & *mask1));
            height++;
        }
        *waveptr = (*waveptr + 1) & 0x7F;
        *mask1 = ror16(*mask1, 2);
        *mask2 = ror16(*mask2, 2);
    } while (*mask1 != 0xC0C0);
}

static void c4_bitplane_wave(void)
{
    uint8_t* dst = C4Ram;
    uint32_t waveptr = C4Ram[0x1F83];
    uint16_t mask1 = 0xC0C0;
    uint16_t mask2 = 0x3F3F;

    for (int block = 0; block < 16; block++) {
        c4_bitplane_wave_block(dst, &waveptr, &mask1, &mask2, 0);
        dst += 16;
        c4_bitplane_wave_block(dst, &waveptr, &mask1, &mask2, 16);
        dst += 16;
    }
}

/* ---- wireframe rendering ---- */

static uint32_t C4X1, C4Y1, C4Z1, C4X2, C4Y2, C4Z2, C4Col;

static void c4_draw_line(void)
{
    uint8_t* ram = C4Ram;

    /* transform both endpoints */
    C4WFXVal = (short)(uint16_t)C4X1;
    C4WFYVal = (short)(uint16_t)C4Y1;
    C4WFZVal = (short)(uint16_t)C4Z1;
    BYTE(C4WFScale, 0) = ram[0x1F90];
    BYTE(C4WFX2Val, 0) = ram[0x1F86];
    BYTE(C4WFY2Val, 0) = ram[0x1F87];
    BYTE(C4WFDist, 0) = ram[0x1F88];
    C4TransfWireFrame2();
    C4X1 = (uint16_t)C4WFXVal;
    C4Y1 = (uint16_t)C4WFYVal;

    C4WFXVal = (short)(uint16_t)C4X2;
    C4WFYVal = (short)(uint16_t)C4Y2;
    C4WFZVal = (short)(uint16_t)C4Z2;
    C4TransfWireFrame2();
    C4X2 = (uint16_t)C4WFXVal;
    C4Y2 = (uint16_t)C4WFYVal;

    /* center on the 96x96 bitmap, 8.8 fixed point */
    C4X1 = (uint16_t)(C4X1 + 48) << 8;
    C4Y1 = (uint16_t)(C4Y1 + 48) << 8;
    C4X2 = (uint16_t)(C4X2 + 48) << 8;
    C4Y2 = (uint16_t)(C4Y2 + 48) << 8;

    C4WFXVal = (short)(uint16_t)(C4X1 >> 8);
    C4WFYVal = (short)(uint16_t)(C4Y1 >> 8);
    C4WFX2Val = (short)(uint16_t)(C4X2 >> 8);
    C4WFY2Val = (short)(uint16_t)(C4Y2 >> 8);
    C4CalcWireFrame();
    uint32_t steps = (uint16_t)C4WFDist;
    if (!steps) {
        steps = 1;
    }
    C4X2 = (uint32_t)(int32_t)C4WFXVal;
    C4Y2 = (uint32_t)(int32_t)C4WFYVal;

    do {
        int16_t x = (int16_t)(uint16_t)(C4X1 >> 8);
        int16_t y = (int16_t)(uint16_t)(C4Y1 >> 8);
        if (x >= 0 && y >= 0 && x <= 95 && y <= 95) {
            /* 16-byte 2-plane tiles, 12 tiles per row */
            uint16_t ofs = (uint16_t)(((uint16_t)(y >> 3)) * 192
                + ((uint16_t)(x >> 3) << 4) + (y & 7) * 2);
            uint8_t mask = ror8(0x7F, x & 7);
            ram[0x300 + ofs] &= mask;
            ram[0x301 + ofs] &= mask;
            if (C4Col & 1) {
                ram[0x300 + ofs] |= (uint8_t)~mask;
            }
            if (C4Col & 2) {
                ram[0x301 + ofs] |= (uint8_t)~mask;
            }
        }
        C4X1 += C4X2;
        C4Y1 += C4Y2;
    } while (--steps);
}

static void c4_draw_wireframe(void)
{
    uint8_t* ram = C4Ram;
    uint8_t* line = lorom_ptr(ram[0x1F82], RAMW(0x1F80));
    uint32_t count = ram[0x295];

    while (count--) {
        /* vertex indices are stored big-endian; 0xFFFF chains backwards */
        uint8_t* p = line;
        uint32_t v1 = (uint32_t)(line[0] << 8 | line[1]);
        while (v1 == 0xFFFF) {
            p -= 5;
            v1 = (uint32_t)(p[2] << 8 | p[3]);
        }
        uint8_t* vp1 = lorom_ptr(ram[0x1F82], v1);
        uint8_t* vp2 = lorom_ptr(ram[0x1F82], (uint32_t)(line[2] << 8 | line[3]));

        C4X1 = (uint16_t)(vp1[0] << 8 | vp1[1]);
        C4Y1 = (uint16_t)(vp1[2] << 8 | vp1[3]);
        C4Z1 = (uint16_t)(vp1[4] << 8 | vp1[5]);
        C4X2 = (uint16_t)(vp2[0] << 8 | vp2[1]);
        C4Y2 = (uint16_t)(vp2[2] << 8 | vp2[3]);
        C4Z2 = (uint16_t)(vp2[4] << 8 | vp2[5]);
        C4Col = line[4];
        line += 5;
        c4_draw_line();
    }
}

static void c4_wireframe_b(void)
{
    memset(C4Ram + 0x300, 0, 16 * 12 * 3 * 4);
    c4_draw_wireframe();
}

/* op 00/05: transform vertices, then per-line distance and endpoints */
static void c4_wireframe(void)
{
    uint8_t* ram = C4Ram;

    C4WFX2Val = (short)ram[0x1F83];
    C4WFY2Val = (short)ram[0x1F86];
    C4WFDist = (short)ram[0x1F89];
    C4WFScale = (short)ram[0x1F8C];

    uint32_t count = RAMW(0x1F80);
    uint8_t* v = ram;
    while (count--) {
        C4WFXVal = (short)*(uint16_t*)(v + 1);
        C4WFYVal = (short)*(uint16_t*)(v + 5);
        C4WFZVal = (short)*(uint16_t*)(v + 9);
        C4TransfWireFrame();
        *(uint16_t*)(v + 1) = (uint16_t)(C4WFXVal + 0x80);
        *(uint16_t*)(v + 5) = (uint16_t)(C4WFYVal + 0x50);
        v += 16;
    }

    *(uint16_t*)(ram + 0x600) = 23;
    *(uint16_t*)(ram + 0x602) = 0x60;
    *(uint16_t*)(ram + 0x605) = 0x40;
    *(uint16_t*)(ram + 0x608) = 23;
    *(uint16_t*)(ram + 0x60A) = 0x60;
    *(uint16_t*)(ram + 0x60D) = 0x40;

    count = RAMW(0xB00);
    uint8_t* idx = ram + 0xB02;
    uint8_t* out = ram;
    while (count--) {
        uint8_t* p1 = ram + (idx[0] << 4);
        uint8_t* p2 = ram + (idx[1] << 4);
        C4WFXVal = (short)*(uint16_t*)(p1 + 1);
        C4WFYVal = (short)*(uint16_t*)(p1 + 5);
        C4WFX2Val = (short)*(uint16_t*)(p2 + 1);
        C4WFY2Val = (short)*(uint16_t*)(p2 + 5);
        C4CalcWireFrame();
        *(uint16_t*)(out + 0x600) = (uint16_t)(C4WFDist ? C4WFDist : 1);
        *(uint16_t*)(out + 0x602) = (uint16_t)C4WFXVal;
        *(uint16_t*)(out + 0x605) = (uint16_t)C4WFYVal;
        idx += 2;
        out += 8;
    }
}

static void c4_transform(void)
{
    uint8_t* ram = C4Ram;

    C4WFXVal = (short)*(uint16_t*)(ram + 0x1F81);
    C4WFYVal = (short)*(uint16_t*)(ram + 0x1F84);
    C4WFZVal = (short)*(uint16_t*)(ram + 0x1F87);
    BYTE(C4WFScale, 0) = ram[0x1F90];
    BYTE(C4WFX2Val, 0) = ram[0x1F89];
    BYTE(C4WFY2Val, 0) = ram[0x1F8A];
    BYTE(C4WFDist, 0) = ram[0x1F8B];
    C4TransfWireFrame2();
    *(uint16_t*)(ram + 0x1F80) = (uint16_t)C4WFXVal;
    *(uint16_t*)(ram + 0x1F83) = (uint16_t)C4WFYVal;
}

/* ---- command dispatch ---- */

static void c4_activate(uint8_t cmd)
{
    uint8_t* ram = C4Ram;

    if (ram[0x1F4D] == 0x0E && !(cmd & 0xC3)) {
        ram[0x1F80] = (uint8_t)(cmd >> 2);
        return;
    }

    switch (cmd) {
    case 0x00: /* sprite functions, selected by $7F4D */
        switch (ram[0x1F4D]) {
        case 0x00:
            C4ProcessSprites();
            break;
        case 0x03:
            c4_spr_scale();
            break;
        case 0x05:
            c4_wireframe();
            break;
        case 0x07:
            c4_spr_rotate();
            break;
        case 0x08:
            c4_draw_wireframe();
            break;
        case 0x0B:
            c4_spr_disintegrate();
            break;
        case 0x0C:
            c4_bitplane_wave();
            break;
        default:
            break;
        }
        break;
    case 0x01: /* draw wireframe with cleared bitmap */
        c4_wireframe_b();
        break;
    case 0x05: /* propulsion */
    {
        uint16_t* vals = (uint16_t*)C4values;
        vals[1] = RAMW(0x1F83);
        vals[0] = RAMW(0x1F81);
        int16_t div = (int16_t)RAMW(0x1F83);
        uint16_t result = 1;
        if (div) { /* the asm faulted on zero */
            int32_t q = 65536 / div;
            vals[3] = (uint16_t)q;
            int32_t prod = (int32_t)(int16_t)(uint16_t)q * (int16_t)RAMW(0x1F81);
            result = (uint16_t)(prod >> 8);
        }
        RAMW(0x1F80) = result;
        vals[2] = result;
        break;
    }
    case 0x0D: /* set vector length */
        C41FXVal = (short)RAMW(0x1F80);
        C41FYVal = (short)RAMW(0x1F83);
        C41FDistVal = (short)RAMW(0x1F86);
        C4Op0D();
        RAMW(0x1F89) = (uint16_t)C41FXVal;
        RAMW(0x1F8C) = (uint16_t)C41FYVal;
        break;
    case 0x10: /* polar to rectangular */
    {
        uint32_t angle = RAMW(0x1F80) & 0x1FF;
        int32_t d = (int16_t)RAMW(0x1F83);
        int32_t v = (int16_t)(uint16_t)(((uint32_t)(CosTable[angle] * d)) >> 15);
        *(uint32_t*)(ram + 0x1F86) = (uint32_t)v;
        v = (int16_t)(uint16_t)(((uint32_t)(SinTable[angle] * d)) >> 15);
        v -= v >> 6;
        *(uint32_t*)(ram + 0x1F89) = (uint32_t)v;
        break;
    }
    case 0x13: /* polar to rectangular, rounded */
    {
        uint32_t angle = RAMW(0x1F80) & 0x1FF;
        int32_t d = (int32_t)(int16_t)RAMW(0x1F83) * 2;
        int32_t v = CosTable[angle] * d;
        v = (v >> 8) + ((v >> 7) & 1);
        *(uint32_t*)(ram + 0x1F86) = (uint32_t)v;
        v = SinTable[angle] * d;
        v = (v >> 8) + ((v >> 7) & 1);
        RAMW(0x1F89) = (uint16_t)v;
        ram[0x1F8B] = (uint8_t)(v >> 16);
        break;
    }
    case 0x15: /* calculate distance */
        C41FXVal = (short)RAMW(0x1F80);
        C41FYVal = (short)RAMW(0x1F83);
        C4Op15();
        RAMW(0x1F80) = (uint16_t)C41FDist;
        break;
    case 0x1F: /* calculate angle */
        C41FXVal = (short)RAMW(0x1F80);
        C41FYVal = (short)RAMW(0x1F83);
        C4Op1F();
        RAMW(0x1F86) = (uint16_t)C41FAngleRes;
        break;
    case 0x22: /* two-dimensional line drawing */
        C4Op22();
        break;
    case 0x25: /* 24-bit multiply */
    {
        uint32_t a = *(uint32_t*)(ram + 0x1F80) & 0xFFFFFF;
        uint32_t b = *(uint32_t*)(ram + 0x1F83) & 0xFFFFFF;
        *(uint32_t*)(ram + 0x1F80) = a * b;
        break;
    }
    case 0x2D: /* coordinate transform */
        c4_transform();
        break;
    case 0x40: /* byte sum of the first 0x800 bytes */
    {
        uint16_t sum = 0;
        for (uint32_t i = 0; i < 0x800; i++) {
            sum = (uint16_t)(sum + ram[i]);
        }
        RAMW(0x1F80) = sum;
        break;
    }
    case 0x54: /* 24-bit square */
    {
        int32_t v = (int32_t)(*(uint32_t*)(ram + 0x1F80) << 8) >> 8;
        int64_t sq = (int64_t)v * v;
        *(uint32_t*)(ram + 0x1F83) = (uint32_t)sq;
        RAMW(0x1F87) = (uint16_t)((uint64_t)sq >> 32);
        break;
    }
    case 0x5C: /* immediate register values */
        *(uint32_t*)(ram + 0) = 0xFF000000;
        *(uint32_t*)(ram + 4) = 0xFF00FFFF;
        *(uint32_t*)(ram + 8) = 0xFF000000;
        *(uint32_t*)(ram + 12) = 0x0000FFFF;
        *(uint32_t*)(ram + 16) = 0x0000FFFF;
        *(uint32_t*)(ram + 20) = 0x7FFFFF80;
        *(uint32_t*)(ram + 24) = 0xFF008000;
        *(uint32_t*)(ram + 28) = 0x7FFF007F;
        *(uint32_t*)(ram + 32) = 0xFFFF7FFF;
        *(uint32_t*)(ram + 36) = 0xFF010000;
        *(uint32_t*)(ram + 40) = 0x0100FEFF;
        *(uint32_t*)(ram + 44) = 0x00FEFF00;
        break;
    case 0x89: /* immediate ROM values */
        ram[0x1F80] = 0x36;
        ram[0x1F81] = 0x43;
        ram[0x1F82] = 0x05;
        break;
    default:
        break;
    }
}

/* write to $7F47: copy from the SNES address at $7F40-42 into C4 RAM */
static void c4_memcpy(void)
{
    uint8_t* ram = C4Ram;
    uint32_t len = RAMW(0x1F43);
    uint8_t* src = snesmmap[ram[0x1F42]] + RAMW(0x1F40);
    uint8_t* dst = ram + (RAMW(0x1F45) & 0x1FFF);

    /* the asm looped 4G times on len 0; copy nothing instead */
    while (len--) {
        *dst++ = *src++;
    }
}

static void c4ram_write(uint32_t off, uint8_t val)
{
    C4Ram[off] = val;
    if (off == 0x1F47) {
        c4_memcpy();
    } else if (off == 0x1F4F) {
        c4_activate(val);
    }
}

/* ---- bank access entry points ---- */

uint8_t c_C4Read8b(uint32_t addr)
{
    if (addr & 0x8000) {
        return memaccessbankr8(addr);
    }
    if (addr < 0x6000) {
        return regaccessbankr8(addr);
    }
    return C4Ram[(addr - 0x6000) & 0x1FFF];
}

uint16_t c_C4Read16b(uint32_t addr)
{
    if (addr & 0x8000) {
        return memaccessbankr16(addr);
    }
    if (addr < 0x6000) {
        return regaccessbankr16(addr);
    }
    uint32_t off = (addr - 0x6000) & 0x1FFF;
    return (uint16_t)(C4Ram[off] | C4Ram[off + 1] << 8);
}

void c_C4Write8b(uint32_t addr, uint8_t val)
{
    if (addr & 0x8000) {
        memaccessbankw8(addr, val);
        return;
    }
    if (addr < 0x6000) {
        regaccessbankw8(addr, val);
        return;
    }
    c4ram_write((addr - 0x6000) & 0x1FFF, val);
}

void c_C4Write16b(uint32_t addr, uint16_t val)
{
    if (addr & 0x8000) {
        memaccessbankw16(addr, val);
        return;
    }
    if (addr < 0x6000) {
        regaccessbankw16(addr, val);
        return;
    }
    uint32_t off = (addr - 0x6000) & 0x1FFF;
    c4ram_write(off, (uint8_t)val);
    c4ram_write(off + 1, (uint8_t)(val >> 8));
}

#if defined(__GNUC__) && defined(__i386__)

#if defined(__APPLE__) || defined(__MINGW32__)
#define CSYM(x) "_" #x
#else
#define CSYM(x) #x
#endif

__asm__(
    ".globl " CSYM(C4Read8b) "\n" CSYM(C4Read8b) ":\n"
                                                 "testw $0x8000, %cx\n"
                                                 "jnz " CSYM(memaccessbankr8) "\n"
                                                                              "cmpl $0x6000, %ecx\n"
                                                                              "jb " CSYM(regaccessbankr8) "\n"
                                                                                                          "pushl %ecx\n"
                                                                                                          "pushl %edx\n"
                                                                                                          "pushl %eax\n"
                                                                                                          "pushl %ecx\n"
                                                                                                          "call " CSYM(c_C4Read8b) "\n"
                                                                                                                                   "addl $4, %esp\n"
                                                                                                                                   "movb %al, (%esp)\n"
                                                                                                                                   "popl %eax\n"
                                                                                                                                   "popl %edx\n"
                                                                                                                                   "popl %ecx\n"
                                                                                                                                   "ret\n");

__asm__(
    ".globl " CSYM(C4Write8b) "\n" CSYM(C4Write8b) ":\n"
                                                   "testw $0x8000, %cx\n"
                                                   "jnz " CSYM(memaccessbankw8) "\n"
                                                                                "cmpl $0x6000, %ecx\n"
                                                                                "jb " CSYM(regaccessbankw8) "\n"
                                                                                                            "pushl %eax\n"
                                                                                                            "pushl %ecx\n"
                                                                                                            "pushl %edx\n"
                                                                                                            "pushl %eax\n"
                                                                                                            "pushl %ecx\n"
                                                                                                            "call " CSYM(c_C4Write8b) "\n"
                                                                                                                                      "addl $8, %esp\n"
                                                                                                                                      "popl %edx\n"
                                                                                                                                      "popl %ecx\n"
                                                                                                                                      "popl %eax\n"
                                                                                                                                      "ret\n");

__asm__(
    ".globl " CSYM(C4Read16b) "\n" CSYM(C4Read16b) ":\n"
                                                   "testw $0x8000, %cx\n"
                                                   "jnz " CSYM(memaccessbankr16) "\n"
                                                                                 "cmpl $0x6000, %ecx\n"
                                                                                 "jb " CSYM(regaccessbankr16) "\n"
                                                                                                              "pushl %ecx\n"
                                                                                                              "pushl %edx\n"
                                                                                                              "pushl %eax\n"
                                                                                                              "pushl %ecx\n"
                                                                                                              "call " CSYM(c_C4Read16b) "\n"
                                                                                                                                        "addl $4, %esp\n"
                                                                                                                                        "movw %ax, (%esp)\n"
                                                                                                                                        "popl %eax\n"
                                                                                                                                        "popl %edx\n"
                                                                                                                                        "popl %ecx\n"
                                                                                                                                        "ret\n");

__asm__(
    ".globl " CSYM(C4Write16b) "\n" CSYM(C4Write16b) ":\n"
                                                     "testw $0x8000, %cx\n"
                                                     "jnz " CSYM(memaccessbankw16) "\n"
                                                                                   "cmpl $0x6000, %ecx\n"
                                                                                   "jb " CSYM(regaccessbankw16) "\n"
                                                                                                                "pushl %eax\n"
                                                                                                                "pushl %ecx\n"
                                                                                                                "pushl %edx\n"
                                                                                                                "pushl %eax\n"
                                                                                                                "pushl %ecx\n"
                                                                                                                "call " CSYM(c_C4Write16b) "\n"
                                                                                                                                           "addl $8, %esp\n"
                                                                                                                                           "popl %edx\n"
                                                                                                                                           "popl %ecx\n"
                                                                                                                                           "popl %eax\n"
                                                                                                                                           "ret\n");

#endif
