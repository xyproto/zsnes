/*
 * DSP2 coprocessor register stubs
 *
 * Ported from chips/dsp2proc.asm.
 *
 * DSP2Read8b    — validate address, read buffer byte, optional arithmetic shift
 * DSP2Read16b   — always returns 0
 * DSP2Write8b   — enforcer-queue dispatch to command handlers w00..w0B
 * DSP2Write16b  — always returns 0 (no-op)
 */

#include <stdint.h>
#include <string.h>

#define DSP2F_HALT 1u
#define DSP2F_AUTO_BUFFER_SHIFT 2u
#define DSP2F_NO_ADDR_CHK 4u

/* BSS */
uint8_t dsp2buffer[256];
uint8_t dsp2enforcerQueue[8 * 512];
uint8_t dsp2enforcer[8];

/* Data */
uint8_t dsp2f03KeyLo;
uint8_t dsp2f03KeyHi;
uint32_t dsp2enforcerReaderCursor;
uint32_t dsp2enforcerWriterCursor = 1;
uint32_t dsp2state = DSP2F_HALT;
uint32_t dsp2input;
uint32_t dsp2inputTemp;
uint32_t dsp2f0dSizeOrg;
uint32_t dsp2f0dSizeNew;

/* Command 01 — bit-plane rearrangement lookup tables */
static const uint8_t dsp2f01TblByte[256] = {
    0,
    1,
    16,
    17,
    0,
    1,
    16,
    17,
    0,
    1,
    16,
    17,
    0,
    1,
    16,
    17,
    0,
    1,
    16,
    17,
    0,
    1,
    16,
    17,
    0,
    1,
    16,
    17,
    0,
    1,
    16,
    17,
    2,
    3,
    18,
    19,
    2,
    3,
    18,
    19,
    2,
    3,
    18,
    19,
    2,
    3,
    18,
    19,
    2,
    3,
    18,
    19,
    2,
    3,
    18,
    19,
    2,
    3,
    18,
    19,
    2,
    3,
    18,
    19,
    4,
    5,
    20,
    21,
    4,
    5,
    20,
    21,
    4,
    5,
    20,
    21,
    4,
    5,
    20,
    21,
    4,
    5,
    20,
    21,
    4,
    5,
    20,
    21,
    4,
    5,
    20,
    21,
    4,
    5,
    20,
    21,
    6,
    7,
    22,
    23,
    6,
    7,
    22,
    23,
    6,
    7,
    22,
    23,
    6,
    7,
    22,
    23,
    6,
    7,
    22,
    23,
    6,
    7,
    22,
    23,
    6,
    7,
    22,
    23,
    6,
    7,
    22,
    23,
    8,
    9,
    24,
    25,
    8,
    9,
    24,
    25,
    8,
    9,
    24,
    25,
    8,
    9,
    24,
    25,
    8,
    9,
    24,
    25,
    8,
    9,
    24,
    25,
    8,
    9,
    24,
    25,
    8,
    9,
    24,
    25,
    10,
    11,
    26,
    27,
    10,
    11,
    26,
    27,
    10,
    11,
    26,
    27,
    10,
    11,
    26,
    27,
    10,
    11,
    26,
    27,
    10,
    11,
    26,
    27,
    10,
    11,
    26,
    27,
    10,
    11,
    26,
    27,
    12,
    13,
    28,
    29,
    12,
    13,
    28,
    29,
    12,
    13,
    28,
    29,
    12,
    13,
    28,
    29,
    12,
    13,
    28,
    29,
    12,
    13,
    28,
    29,
    12,
    13,
    28,
    29,
    12,
    13,
    28,
    29,
    14,
    15,
    30,
    31,
    14,
    15,
    30,
    31,
    14,
    15,
    30,
    31,
    14,
    15,
    30,
    31,
    14,
    15,
    30,
    31,
    14,
    15,
    30,
    31,
    14,
    15,
    30,
    31,
    14,
    15,
    30,
    31,
};

static const uint8_t dsp2f01TblBitMask[256] = {
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    64,
    64,
    64,
    64,
    128,
    128,
    128,
    128,
    16,
    16,
    16,
    16,
    32,
    32,
    32,
    32,
    4,
    4,
    4,
    4,
    8,
    8,
    8,
    8,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
};

/* Copies dsp2enforcer[0..7] into the next queue slot; advances writer cursor */
static void DSP2Add2Queue(void)
{
    uint32_t wc = dsp2enforcerWriterCursor;
    uint8_t* dst = dsp2enforcerQueue + wc * 8;
    dsp2enforcerWriterCursor = (wc + 1) & 511;
    memcpy(dst, dsp2enforcer, 8);
}

uint8_t DSP2Read8b(uint32_t addr)
{
    if (dsp2state & DSP2F_HALT)
        return 0;

    uint16_t cx = (uint16_t)addr;
    if (!(cx & 0x8000))
        return 0; /* bit 15 must be set   */
    if (cx & 0x7000)
        return 0; /* bits 14-12 must clear */

    uint8_t idx = (uint8_t)cx; /* low 8 bits */
    uint8_t result = dsp2buffer[idx];

    if (dsp2state & DSP2F_AUTO_BUFFER_SHIFT)
        *(int32_t*)dsp2buffer >>= 8; /* arithmetic shift of first dword */

    return result;
}

uint16_t DSP2Read16b(uint32_t addr)
{
    (void)addr;
    return 0;
}

void DSP2Write8b(uint32_t addr, uint8_t val)
{
    if (dsp2state & DSP2F_HALT)
        return;

    dsp2input = val;

    /* Load enforcer entry from queue at reader cursor */
    uint32_t rc = dsp2enforcerReaderCursor;
    uint8_t* slot = dsp2enforcerQueue + rc * 8;
    memcpy(dsp2enforcer, slot, 8);

    /* Address check (skipped when NO_ADDR_CHK set) */
    if (!(dsp2state & DSP2F_NO_ADDR_CHK)) {
        uint16_t ea = (uint16_t)(dsp2enforcer[4] | ((uint16_t)dsp2enforcer[5] << 8));
        if ((uint16_t)addr != ea)
            goto gohalt;
    }

    switch (dsp2enforcer[0]) {
    /* ------------------------------------------------------------------ */
    case 0x00: { /* command dispatcher */
        dsp2state &= ~(DSP2F_AUTO_BUFFER_SHIFT | DSP2F_NO_ADDR_CHK);
        switch ((uint8_t)dsp2input) {

        case 0x01: /* bit-plane rearrangement: queue 32 w01 entries */
            for (uint8_t al = 0; al < 32; al++) {
                memset(dsp2enforcer, 0, 8);
                dsp2enforcer[0] = 0x01;
                dsp2enforcer[5] = 0x80;
                dsp2enforcer[4] = al;
                DSP2Add2Queue();
            }
            goto queueincoming;

        case 0x03: /* transparent-color apply: queue w02 then entries */
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x02;
            dsp2enforcer[5] = 0x80;
            DSP2Add2Queue();
            goto queueincoming;

        case 0x05: /* same as 03 but with NO_ADDR_CHK */
            dsp2state |= DSP2F_NO_ADDR_CHK;
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x03;
            dsp2enforcer[5] = 0x80;
            DSP2Add2Queue();
            goto done;

        case 0x06: /* nibble-swap: queue w06 entry */
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x06;
            dsp2enforcer[5] = 0x80;
            DSP2Add2Queue();
            goto done;

        case 0x09: /* multiply: queue 4 w08 entries */
            for (uint8_t i = 0; i < 4; i++) {
                memset(dsp2enforcer, 0, 8);
                dsp2enforcer[0] = 0x08;
                dsp2enforcer[5] = 0x80;
                dsp2enforcer[1] = i;
                DSP2Add2Queue();
            }
            goto queueincoming;

        case 0x0D: /* bitmap scale: queue w09 (set orig size) */
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x09;
            dsp2enforcer[5] = 0x80;
            DSP2Add2Queue();
            goto done;

        case 0x0F: /* queue incoming w00 */
            goto queueincoming;

        default:
            goto gohalt;
        }
    }

    /* ------------------------------------------------------------------ */
    case 0x01: { /* bit-plane rearrangement for one column */
        uint32_t ecx = (uint32_t)dsp2enforcer[4] * 8;
        uint8_t tmp = (uint8_t)dsp2input;
        for (int i = 0; i < 8; i++, ecx++) {
            uint8_t bidx = dsp2f01TblByte[ecx];
            uint8_t mask = dsp2f01TblBitMask[ecx];
            if (tmp & 1)
                dsp2buffer[bidx] |= mask;
            else
                dsp2buffer[bidx] &= (uint8_t)~mask;
            tmp = (uint8_t)((int8_t)tmp >> 1); /* arithmetic shift */
        }
        goto done;
    }

    /* ------------------------------------------------------------------ */
    case 0x02: { /* set transparent-color key */
        uint8_t k = (uint8_t)dsp2input & 0x0F;
        dsp2f03KeyLo = k;
        dsp2f03KeyHi = (uint8_t)(k << 4);
        goto done;
    }

    /* ------------------------------------------------------------------ */
    case 0x03: { /* queue N w04 then N w05 entries */
        uint8_t n = (uint8_t)dsp2input;
        if (!n)
            goto gohalt;
        for (uint8_t al = 0; al < n; al++) {
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x04;
            dsp2enforcer[5] = 0x80;
            dsp2enforcer[4] = al;
            DSP2Add2Queue();
        }
        for (uint8_t al = 0; al < n; al++) {
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x05;
            dsp2enforcer[5] = 0x80;
            dsp2enforcer[4] = al;
            DSP2Add2Queue();
        }
        goto queueincoming;
    }

    /* ------------------------------------------------------------------ */
    case 0x04: { /* write input to buffer[addr_low] */
        dsp2buffer[dsp2enforcer[4]] = (uint8_t)dsp2input;
        goto done;
    }

    /* ------------------------------------------------------------------ */
    case 0x05: { /* transparent-color apply for one pixel */
        uint8_t idx = dsp2enforcer[4];
        uint8_t cur = dsp2buffer[idx];
        uint8_t inp = (uint8_t)dsp2input;

        uint8_t hi = inp & 0xF0;
        if (hi != dsp2f03KeyHi)
            cur = (uint8_t)((cur & 0x0F) | hi);

        uint8_t lo = inp & 0x0F;
        if (lo != dsp2f03KeyLo)
            cur = (uint8_t)((cur & 0xF0) | lo);

        dsp2buffer[idx] = cur;
        goto done;
    }

    /* ------------------------------------------------------------------ */
    case 0x06: { /* queue N nibble-swap (w07) entries */
        uint8_t count = (uint8_t)dsp2input;
        if (!count)
            goto gohalt;
        for (uint8_t i = 0; i < count; i++) {
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x07;
            dsp2enforcer[5] = 0x80;
            dsp2enforcer[1] = (uint8_t)(count - 1 - i); /* decreasing param */
            dsp2enforcer[4] = i; /* increasing addr  */
            DSP2Add2Queue();
        }
        goto queueincoming;
    }

    /* ------------------------------------------------------------------ */
    case 0x07: { /* nibble-swap input, write to buffer[param1] */
        uint8_t v = (uint8_t)dsp2input;
        dsp2buffer[dsp2enforcer[1]] = (uint8_t)((v << 4) | (v >> 4));
        goto done;
    }

    /* ------------------------------------------------------------------ */
    case 0x08: { /* accumulate multiply inputs; compute on 4th */
        uint8_t idx = dsp2enforcer[1];
        dsp2buffer[idx] = (uint8_t)dsp2input;
        if (idx == 3) {
            uint32_t product = (uint32_t)dsp2buffer[0] * (uint32_t)dsp2buffer[2];
            *(uint32_t*)dsp2buffer = product;
            dsp2state |= DSP2F_AUTO_BUFFER_SHIFT;
        }
        goto done;
    }

    /* ------------------------------------------------------------------ */
    case 0x09: { /* set original size for bitmap scaling */
        int8_t orig = (int8_t)dsp2input >> 1;
        dsp2f0dSizeOrg = (uint32_t)(uint8_t)orig;
        if (!orig)
            goto gohalt;
        memset(dsp2enforcer, 0, 8);
        dsp2enforcer[0] = 0x0A;
        dsp2enforcer[5] = 0x80;
        DSP2Add2Queue();
        goto done;
    }

    /* ------------------------------------------------------------------ */
    case 0x0A: { /* set new size for bitmap scaling; queue w0B entries */
        int8_t nw = (int8_t)dsp2input >> 1;
        dsp2f0dSizeNew = (uint32_t)(uint8_t)nw;
        if (!nw)
            goto gohalt;
        uint8_t orig = (uint8_t)dsp2f0dSizeOrg;
        uint8_t neww = (uint8_t)dsp2f0dSizeNew;
        for (uint8_t cl = 0; cl < orig; cl++) {
            uint8_t param = (uint8_t)((cl * neww) / orig);
            memset(dsp2enforcer, 0, 8);
            dsp2enforcer[0] = 0x0B;
            dsp2enforcer[5] = 0x80;
            dsp2enforcer[1] = param;
            dsp2enforcer[4] = cl;
            DSP2Add2Queue();
        }
        goto queueincoming;
    }

    /* ------------------------------------------------------------------ */
    case 0x0B: { /* write input to buffer[param1] */
        dsp2buffer[dsp2enforcer[1]] = (uint8_t)dsp2input;
        goto done;
    }

    /* ------------------------------------------------------------------ */
    default:
        goto gohalt;
    }

queueincoming:
    memset(dsp2enforcer, 0, 8);
    dsp2enforcer[0] = 0x00;
    dsp2enforcer[5] = 0x80;
    DSP2Add2Queue();
    /* fall through */

done:
    dsp2enforcerReaderCursor = (rc + 1) & 511;
    return;

gohalt:
    dsp2state |= DSP2F_HALT;
}

void DSP2Write16b(uint32_t addr, uint16_t val)
{
    (void)addr;
    (void)val;
}
