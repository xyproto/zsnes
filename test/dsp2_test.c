/*
 * DSP2 coprocessor unit tests
 *
 * Covers the four public functions ported from chips/dsp2proc.asm:
 *   DSP2Read8b, DSP2Read16b, DSP2Write8b, DSP2Write16b
 * and selected internal command handlers exercised via Write8b.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

/* --- Exported globals from chips/dsp2proc.c -------------------------------- */

extern uint8_t dsp2buffer[256];
extern uint8_t dsp2enforcerQueue[8 * 512];
extern uint8_t dsp2enforcer[8];
extern uint8_t dsp2f03KeyLo;
extern uint8_t dsp2f03KeyHi;
extern uint32_t dsp2enforcerReaderCursor;
extern uint32_t dsp2enforcerWriterCursor;
extern uint32_t dsp2state;
extern uint32_t dsp2f0dSizeOrg;

#define DSP2F_HALT 1u
#define DSP2F_AUTO_BUFFER_SHIFT 2u
#define DSP2F_NO_ADDR_CHK 4u

/* --- Declarations ---------------------------------------------------------- */

uint8_t DSP2Read8b(uint32_t addr);
uint16_t DSP2Read16b(uint32_t addr);
void DSP2Write8b(uint32_t addr, uint8_t val);
void DSP2Write16b(uint32_t addr, uint16_t val);

/* --- Helpers --------------------------------------------------------------- */

#define RESET_DSP2()                                             \
    do {                                                         \
        memset(dsp2buffer, 0, sizeof(dsp2buffer));               \
        memset(dsp2enforcerQueue, 0, sizeof(dsp2enforcerQueue)); \
        memset(dsp2enforcer, 0, sizeof(dsp2enforcer));           \
        dsp2f03KeyLo = 0;                                        \
        dsp2f03KeyHi = 0;                                        \
        dsp2enforcerReaderCursor = 0;                            \
        dsp2enforcerWriterCursor = 1;                            \
        dsp2state = 0;                                           \
        dsp2f0dSizeOrg = 0;                                      \
    } while (0)

/* Write an 8-byte queue entry at slot `slot` */
static void put_entry(uint32_t slot, uint8_t cmd, uint8_t param1, uint16_t addr)
{
    uint8_t* e = dsp2enforcerQueue + slot * 8;
    memset(e, 0, 8);
    e[0] = cmd;
    e[1] = param1;
    e[4] = (uint8_t)addr;
    e[5] = (uint8_t)(addr >> 8);
}

/* Queue slot field accessors */
static uint8_t qcmd(uint32_t s) { return dsp2enforcerQueue[s * 8 + 0]; }
static uint8_t qp1(uint32_t s) { return dsp2enforcerQueue[s * 8 + 1]; }
static uint16_t qaddr(uint32_t s)
{
    return (uint16_t)(dsp2enforcerQueue[s * 8 + 4] | ((uint16_t)dsp2enforcerQueue[s * 8 + 5] << 8));
}

/* ======================================================================== */
/* DSP2Read16b / DSP2Write16b                                               */
/* ======================================================================== */

static void test_read16b(void)
{
    ZT_SECTION("DSP2Read16b: always returns 0");
    ZT_CHECK(DSP2Read16b(0x0000) == 0);
    ZT_CHECK(DSP2Read16b(0x8000) == 0);
    ZT_CHECK(DSP2Read16b(0xFFFF) == 0);
}

static void test_write16b(void)
{
    ZT_SECTION("DSP2Write16b: no-op, does not crash");
    DSP2Write16b(0x8000, 0x1234);
    DSP2Write16b(0x0000, 0xABCD);
    ZT_CHECK(1);
}

/* ======================================================================== */
/* DSP2Read8b                                                               */
/* ======================================================================== */

static void test_read8b_halt(void)
{
    ZT_SECTION("DSP2Read8b: HALT flag set → returns 0");
    RESET_DSP2();
    dsp2state = DSP2F_HALT;
    dsp2buffer[0] = 0x42;
    ZT_CHECK(DSP2Read8b(0x8000) == 0);
}

static void test_read8b_addr_validation(void)
{
    ZT_SECTION("DSP2Read8b: bit-15 required, bits 14-12 must be clear → else 0");
    RESET_DSP2();
    dsp2buffer[0] = 0x55;

    /* bit 15 not set */
    ZT_CHECK(DSP2Read8b(0x0000) == 0);
    ZT_CHECK(DSP2Read8b(0x7FFF) == 0);

    /* bit 15 set but bits 14-12 not all clear */
    ZT_CHECK(DSP2Read8b(0x9000) == 0); /* bit 12 set */
    ZT_CHECK(DSP2Read8b(0xA000) == 0); /* bit 13 set */
    ZT_CHECK(DSP2Read8b(0xC000) == 0); /* bit 14 set */

    /* valid: bit 15 set, bits 14-12 clear */
    ZT_CHECK(DSP2Read8b(0x8000) == 0x55);
}

static void test_read8b_index(void)
{
    ZT_SECTION("DSP2Read8b: index = addr & 0xFF into dsp2buffer");
    RESET_DSP2();
    dsp2buffer[0x00] = 0x11;
    dsp2buffer[0x42] = 0x42;
    dsp2buffer[0xFF] = 0xFF;

    ZT_CHECK(DSP2Read8b(0x8000) == 0x11); /* 0x8000 & 0xFF = 0x00 */
    ZT_CHECK(DSP2Read8b(0x8042) == 0x42);
    ZT_CHECK(DSP2Read8b(0x80FF) == 0xFF);
    /* bits 11-8 don't affect index (only 7-0 kept) */
    ZT_CHECK(DSP2Read8b(0x8F42) == 0x42); /* 0x8F42 & 0xFF = 0x42 */
}

static void test_read8b_auto_shift(void)
{
    ZT_SECTION("DSP2Read8b: AUTO_BUFFER_SHIFT → sar dword[dsp2buffer], 8");
    RESET_DSP2();
    dsp2state = DSP2F_AUTO_BUFFER_SHIFT;
    /* buffer[0..3] as little-endian int32 = 0x00123400
     * after sar 8: 0x00001234
     * so buffer[0]=0x34, buffer[1]=0x12, buffer[2]=0x00, buffer[3]=0x00 */
    dsp2buffer[0] = 0x00;
    dsp2buffer[1] = 0x34;
    dsp2buffer[2] = 0x12;
    dsp2buffer[3] = 0x00;

    uint8_t r = DSP2Read8b(0x8000);
    ZT_CHECK(r == 0x00); /* value at [0] before shift */
    ZT_CHECK(dsp2buffer[0] == 0x34);
    ZT_CHECK(dsp2buffer[1] == 0x12);
    ZT_CHECK(dsp2buffer[2] == 0x00);
    ZT_CHECK(dsp2buffer[3] == 0x00);
}

static void test_read8b_shift_sign(void)
{
    ZT_SECTION("DSP2Read8b: AUTO_BUFFER_SHIFT is arithmetic (sign-extends)");
    RESET_DSP2();
    dsp2state = DSP2F_AUTO_BUFFER_SHIFT;
    /* buffer[0..3] = 0xFF,0xFF,0xFF,0xFF → as int32: -1
     * sar 8 of -1 → still -1 (all bits remain 1) */
    memset(dsp2buffer, 0xFF, 4);
    DSP2Read8b(0x8000);
    ZT_CHECK(dsp2buffer[0] == 0xFF);
    ZT_CHECK(dsp2buffer[1] == 0xFF);
    ZT_CHECK(dsp2buffer[2] == 0xFF);
    ZT_CHECK(dsp2buffer[3] == 0xFF);
}

/* ======================================================================== */
/* DSP2Write8b — infrastructure                                             */
/* ======================================================================== */

static void test_write8b_halt(void)
{
    ZT_SECTION("DSP2Write8b: HALT flag set → no-op");
    RESET_DSP2();
    dsp2state = DSP2F_HALT;
    put_entry(0, 0x04, 0, 0x0000);
    dsp2buffer[0] = 0xAA;
    DSP2Write8b(0x0000, 0xFF);
    ZT_CHECK(dsp2buffer[0] == 0xAA); /* unchanged */
    ZT_CHECK(dsp2enforcerReaderCursor == 0); /* not advanced */
}

static void test_write8b_addr_mismatch(void)
{
    ZT_SECTION("DSP2Write8b: addr mismatch → sets HALT flag");
    RESET_DSP2();
    put_entry(0, 0x0B, 0, 0x1234); /* queue expects addr 0x1234 */
    DSP2Write8b(0x5678, 0x42); /* write to wrong address */
    ZT_CHECK(dsp2state & DSP2F_HALT);
}

static void test_write8b_addr_match(void)
{
    ZT_SECTION("DSP2Write8b: addr match → command executes, cursor advances");
    RESET_DSP2();
    put_entry(0, 0x0B, 7, 0x0001); /* w0B: write input to buffer[7], addr=1 */
    DSP2Write8b(0x0001, 0xAB);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2buffer[7] == 0xAB);
    ZT_CHECK(dsp2enforcerReaderCursor == 1);
}

static void test_write8b_no_addr_chk(void)
{
    ZT_SECTION("DSP2Write8b: NO_ADDR_CHK bypasses address validation");
    RESET_DSP2();
    dsp2state = DSP2F_NO_ADDR_CHK;
    put_entry(0, 0x0B, 3, 0x1234); /* queue expects 0x1234 */
    DSP2Write8b(0xFFFF, 0x99); /* completely different address */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2buffer[3] == 0x99);
}

static void test_write8b_cursor_wrap(void)
{
    ZT_SECTION("DSP2Write8b: reader cursor wraps at 512");
    RESET_DSP2();
    dsp2enforcerReaderCursor = 511;
    put_entry(511, 0x0B, 0, 0x0000);
    DSP2Write8b(0x0000, 0x11);
    ZT_CHECK(dsp2enforcerReaderCursor == 0); /* (511+1) & 511 = 0 */
}

static void test_write8b_unknown_cmd(void)
{
    ZT_SECTION("DSP2Write8b: unrecognised command → sets HALT");
    RESET_DSP2();
    put_entry(0, 0x0C, 0, 0x0000); /* 0x0C not in dispatch table */
    DSP2Write8b(0x0000, 0x00);
    ZT_CHECK(dsp2state & DSP2F_HALT);
}

/* ======================================================================== */
/* DSP2Write8b — command handlers                                           */
/* ======================================================================== */

static void test_w0B(void)
{
    ZT_SECTION("w0B: input written to dsp2buffer[param1]");
    RESET_DSP2();
    put_entry(0, 0x0B, 42, 0x0000);
    DSP2Write8b(0x0000, 0xDE);
    ZT_CHECK(dsp2buffer[42] == 0xDE);

    /* param1 wraps: test at index 0 and 255 */
    RESET_DSP2();
    put_entry(0, 0x0B, 0, 0x0000);
    DSP2Write8b(0x0000, 0x01);
    ZT_CHECK(dsp2buffer[0] == 0x01);

    RESET_DSP2();
    put_entry(0, 0x0B, 255, 0x0000);
    DSP2Write8b(0x0000, 0xFF);
    ZT_CHECK(dsp2buffer[255] == 0xFF);
}

static void test_w04(void)
{
    ZT_SECTION("w04: input written to dsp2buffer[addr_low_byte_of_entry]");
    RESET_DSP2();
    /* addr low byte = 0x07 → buffer index 7 */
    put_entry(0, 0x04, 0, 0x0007);
    DSP2Write8b(0x0007, 0xCD);
    ZT_CHECK(dsp2buffer[7] == 0xCD);

    RESET_DSP2();
    put_entry(0, 0x04, 0, 0x00FF);
    DSP2Write8b(0x00FF, 0xAB);
    ZT_CHECK(dsp2buffer[0xFF] == 0xAB);
}

static void test_w02(void)
{
    ZT_SECTION("w02: sets dsp2f03KeyLo = input & 0x0F, KeyHi = KeyLo << 4");
    RESET_DSP2();
    put_entry(0, 0x02, 0, 0x0000);
    DSP2Write8b(0x0000, 0xA5); /* low nibble = 5 */
    ZT_CHECK(dsp2f03KeyLo == 0x05);
    ZT_CHECK(dsp2f03KeyHi == 0x50);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));

    RESET_DSP2();
    put_entry(0, 0x02, 0, 0x0000);
    DSP2Write8b(0x0000, 0xF0); /* low nibble = 0 */
    ZT_CHECK(dsp2f03KeyLo == 0x00);
    ZT_CHECK(dsp2f03KeyHi == 0x00);
}

static void test_w07(void)
{
    ZT_SECTION("w07: input nibble-swapped, written to buffer[param1]");
    RESET_DSP2();
    put_entry(0, 0x07, 10, 0x0000);
    DSP2Write8b(0x0000, 0xAB); /* rol 4: 0xBA */
    ZT_CHECK(dsp2buffer[10] == 0xBA);

    RESET_DSP2();
    put_entry(0, 0x07, 0, 0x0000);
    DSP2Write8b(0x0000, 0x12);
    ZT_CHECK(dsp2buffer[0] == 0x21);
}

static void test_w05(void)
{
    ZT_SECTION("w05: transparent-color apply — replaces non-key nibbles in buffer");
    RESET_DSP2();
    dsp2f03KeyLo = 0x05;
    dsp2f03KeyHi = 0x50;

    /* buffer[2] = 0x57; input = 0x35
     * high nibble of input (0x30) != KeyHi (0x50) → replace high → 0x37
     * low  nibble of input (0x05) == KeyLo (0x05) → keep low → 0x37 */
    dsp2buffer[2] = 0x57;
    put_entry(0, 0x05, 0, 0x0002); /* addr_low=2 → buffer index 2 */
    DSP2Write8b(0x0002, 0x35);
    ZT_CHECK(dsp2buffer[2] == 0x37);

    /* both nibbles match key → buffer unchanged */
    RESET_DSP2();
    dsp2f03KeyLo = 0x03;
    dsp2f03KeyHi = 0x30;
    dsp2buffer[0] = 0xAA;
    put_entry(0, 0x05, 0, 0x0000);
    DSP2Write8b(0x0000, 0x33); /* both nibbles match key */
    ZT_CHECK(dsp2buffer[0] == 0xAA);
}

static void test_w00_clears_flags(void)
{
    ZT_SECTION("w00: clears AUTO_BUFFER_SHIFT and NO_ADDR_CHK flags");
    RESET_DSP2();
    dsp2state = DSP2F_AUTO_BUFFER_SHIFT | DSP2F_NO_ADDR_CHK;
    /* Queue entry 0: w00 command at addr 0x8000 */
    put_entry(0, 0x00, 0, 0x8000);
    /* Queue entry 1 (queued by w00p0F): w00 again */
    put_entry(1, 0x00, 0, 0x8000);
    /* Write w00 with subcmd=0x0F (just queues another w00) */
    DSP2Write8b(0x8000, 0x0F);
    ZT_CHECK(!(dsp2state & DSP2F_AUTO_BUFFER_SHIFT));
    ZT_CHECK(!(dsp2state & DSP2F_NO_ADDR_CHK));
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
}

static void test_w00_unknown_subcmd(void)
{
    ZT_SECTION("w00: unknown sub-command → sets HALT");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    DSP2Write8b(0x8000, 0x02); /* 0x02 not in w00 dispatch */
    ZT_CHECK(dsp2state & DSP2F_HALT);
}

static void test_w00p02_chain(void)
{
    ZT_SECTION("w00+w02 chain: two writes set transparent key");
    RESET_DSP2();
    /* Slot 0: w00 command at 0x8000 (writer starts at 1 normally, but we pre-fill) */
    put_entry(0, 0x00, 0, 0x8000);
    /* Slot 2 will be used by w00p03's queue: cmd=2 at 0x8000 */
    /* Actually w00p03 queues cmd=2 and then queueincoming(cmd=0). Let's use w00p03 */
    /* Instead test just w00 + w02 directly: */
    /* w00 with subcmd=0x03 queues [cmd=2,addr=0x8000] then [cmd=0,addr=0x8000] */
    DSP2Write8b(0x8000, 0x03); /* executes slot 0 = w00, subcmd=03 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerReaderCursor == 1);

    /* Now the writer has pushed [cmd=2] and [cmd=0] into slots 1 and 2.
     * Reader is at 1. */
    DSP2Write8b(0x8000, 0xC7); /* executes slot 1 = w02, input=0xC7 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2f03KeyLo == 0x07);
    ZT_CHECK(dsp2f03KeyHi == 0x70);
}

/* ======================================================================== */
/* w01 — bit-plane rearrangement                                            */
/* ======================================================================== */

/*
 * Column `col` (= addr_low byte of the queue entry) maps to table rows
 * dsp2f01TblByte[col*8 .. col*8+7] and dsp2f01TblBitMask[col*8 .. col*8+7].
 *
 * Column 0 (ecx 0..7):
 *   i  byte  mask
 *   0   0    0x40
 *   1   1    0x40
 *   2   16   0x40
 *   3   17   0x40
 *   4   0    0x80
 *   5   1    0x80
 *   6   16   0x80
 *   7   17   0x80
 *
 * Column 4 (ecx 32..39) uses bytes 4,5,20,21 with the same masks.
 */

static void test_w01_all_bits_set(void)
{
    ZT_SECTION("w01: column 0, input 0xFF sets bits 6+7 in buffer[0,1,16,17]");
    RESET_DSP2();
    put_entry(0, 0x01, 0, 0x0000); /* col = addr_low = 0 */
    DSP2Write8b(0x0000, 0xFF);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2buffer[0] == 0xC0);
    ZT_CHECK(dsp2buffer[1] == 0xC0);
    ZT_CHECK(dsp2buffer[16] == 0xC0);
    ZT_CHECK(dsp2buffer[17] == 0xC0);
    ZT_CHECK(dsp2buffer[2] == 0x00); /* no other bytes touched */
}

static void test_w01_single_bit(void)
{
    ZT_SECTION("w01: column 0, input 0x01 (bit 0 only) → buffer[0] bit 6 set");
    RESET_DSP2();
    put_entry(0, 0x01, 0, 0x0000);
    DSP2Write8b(0x0000, 0x01);
    /* bit 0 → i=0 → byte_idx=0, mask=0x40 */
    ZT_CHECK(dsp2buffer[0] == 0x40);
    ZT_CHECK(dsp2buffer[1] == 0x00);
    ZT_CHECK(dsp2buffer[16] == 0x00);
    ZT_CHECK(dsp2buffer[17] == 0x00);
}

static void test_w01_clears_bits(void)
{
    ZT_SECTION("w01: column 0, input 0x00 clears bits 6+7 in buffer[0,1,16,17]");
    RESET_DSP2();
    dsp2buffer[0] = dsp2buffer[1] = dsp2buffer[16] = dsp2buffer[17] = 0xFF;
    put_entry(0, 0x01, 0, 0x0000);
    DSP2Write8b(0x0000, 0x00);
    /* all input bits 0 → all controlled bits cleared */
    ZT_CHECK((dsp2buffer[0] & 0xC0) == 0x00);
    ZT_CHECK((dsp2buffer[1] & 0xC0) == 0x00);
    ZT_CHECK((dsp2buffer[16] & 0xC0) == 0x00);
    ZT_CHECK((dsp2buffer[17] & 0xC0) == 0x00);
    /* bits not owned by col 0 are preserved */
    ZT_CHECK((dsp2buffer[0] & 0x3F) == 0x3F);
}

static void test_w01_column4(void)
{
    ZT_SECTION("w01: column 4 targets buffer[2,3,18,19] (cols 0-3 share [0,1,16,17]; 4-7 share [2,3,18,19])");
    RESET_DSP2();
    put_entry(0, 0x01, 0, 0x0004); /* addr_low = 4 → col 4 */
    DSP2Write8b(0x0004, 0xFF);
    ZT_CHECK(dsp2buffer[2] == 0xC0);
    ZT_CHECK(dsp2buffer[3] == 0xC0);
    ZT_CHECK(dsp2buffer[18] == 0xC0);
    ZT_CHECK(dsp2buffer[19] == 0xC0);
    ZT_CHECK(dsp2buffer[0] == 0x00); /* col 0 untouched */
}

static void test_w01_arithmetic_shift(void)
{
    ZT_SECTION("w01: temp is shifted arithmetically — 0x80 propagates sign to bit 7 of last iter");
    RESET_DSP2();
    /* 0x80 = 1000 0000; bit 0 is 0 for iterations 0..6, becomes 1 at iteration 7
     * because arithmetic right shift fills with sign bit:
     * 0x80→0xC0→0xE0→0xF0→0xF8→0xFC→0xFE→0xFF (bit 0 set at iteration 7)
     * iteration 7 of col 0: byte_idx=17, mask=0x80 → buffer[17] |= 0x80 */
    put_entry(0, 0x01, 0, 0x0000);
    DSP2Write8b(0x0000, 0x80);
    ZT_CHECK(dsp2buffer[17] == 0x80);
    /* all other bits/bytes untouched */
    ZT_CHECK(dsp2buffer[0] == 0x00);
    ZT_CHECK(dsp2buffer[16] == 0x00);
}

/* ======================================================================== */
/* w03 — queue N×w04 + N×w05 entries                                        */
/* ======================================================================== */

static void test_w03_halt_on_zero(void)
{
    ZT_SECTION("w03: input 0 → sets HALT");
    RESET_DSP2();
    put_entry(0, 0x03, 0, 0x0000);
    DSP2Write8b(0x0000, 0);
    ZT_CHECK(dsp2state & DSP2F_HALT);
}

static void test_w03_queue_entries(void)
{
    ZT_SECTION("w03: N=2 queues 2×w04 + 2×w05 + w00 incoming (5 slots)");
    RESET_DSP2();
    put_entry(0, 0x03, 0, 0x0000);
    uint32_t wc = dsp2enforcerWriterCursor; /* 1 after reset */
    DSP2Write8b(0x0000, 2);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 5);
    /* w04 entries: addr_low = 0, 1 */
    ZT_CHECK(qcmd(wc + 0) == 0x04);
    ZT_CHECK(qaddr(wc + 0) == (uint16_t)(0x8000 | 0));
    ZT_CHECK(qcmd(wc + 1) == 0x04);
    ZT_CHECK(qaddr(wc + 1) == (uint16_t)(0x8000 | 1));
    /* w05 entries: addr_low = 0, 1 */
    ZT_CHECK(qcmd(wc + 2) == 0x05);
    ZT_CHECK(qaddr(wc + 2) == (uint16_t)(0x8000 | 0));
    ZT_CHECK(qcmd(wc + 3) == 0x05);
    ZT_CHECK(qaddr(wc + 3) == (uint16_t)(0x8000 | 1));
    /* w00 incoming */
    ZT_CHECK(qcmd(wc + 4) == 0x00);
    ZT_CHECK(qaddr(wc + 4) == 0x8000);
}

/* ======================================================================== */
/* w06 — queue N nibble-swap entries                                        */
/* ======================================================================== */

static void test_w06_halt_on_zero(void)
{
    ZT_SECTION("w06: input 0 → sets HALT");
    RESET_DSP2();
    put_entry(0, 0x06, 0, 0x0000);
    DSP2Write8b(0x0000, 0);
    ZT_CHECK(dsp2state & DSP2F_HALT);
}

static void test_w06_queue_entries(void)
{
    ZT_SECTION("w06: N=3 queues 3×w07 (param decreasing, addr increasing) + w00");
    RESET_DSP2();
    put_entry(0, 0x06, 0, 0x0000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x0000, 3);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 4);
    /* param1 decreasing: 2,1,0; addr_low increasing: 0,1,2 */
    ZT_CHECK(qcmd(wc + 0) == 0x07);
    ZT_CHECK(qp1(wc + 0) == 2);
    ZT_CHECK(qaddr(wc + 0) == 0x8000);
    ZT_CHECK(qcmd(wc + 1) == 0x07);
    ZT_CHECK(qp1(wc + 1) == 1);
    ZT_CHECK(qaddr(wc + 1) == 0x8001);
    ZT_CHECK(qcmd(wc + 2) == 0x07);
    ZT_CHECK(qp1(wc + 2) == 0);
    ZT_CHECK(qaddr(wc + 2) == 0x8002);
    ZT_CHECK(qcmd(wc + 3) == 0x00);
    ZT_CHECK(qaddr(wc + 3) == 0x8000);
}

static void test_w06_w07_chain(void)
{
    ZT_SECTION("w06+w07 chain: 2 queued w07s execute via subsequent writes");
    RESET_DSP2();
    put_entry(0, 0x06, 0, 0x0000);
    DSP2Write8b(0x0000, 2); /* queues: w07@0x8000(p1=1), w07@0x8001(p1=0), w00@0x8000 */

    /* first queued w07: param1=1, addr=0x8000 */
    DSP2Write8b(0x8000, 0xAB); /* buffer[1] = rol(0xAB,4) = 0xBA */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2buffer[1] == 0xBA);

    /* second queued w07: param1=0, addr=0x8001 */
    DSP2Write8b(0x8001, 0x12); /* buffer[0] = rol(0x12,4) = 0x21 */
    ZT_CHECK(dsp2buffer[0] == 0x21);
}

/* ======================================================================== */
/* w08 — multiply accumulator                                               */
/* ======================================================================== */

static void test_w08_no_multiply_before_idx3(void)
{
    ZT_SECTION("w08: param1 0,1,2 write buffer byte, do not set AUTO_BUFFER_SHIFT");
    RESET_DSP2();
    put_entry(0, 0x08, 0, 0x8000);
    DSP2Write8b(0x8000, 7);
    ZT_CHECK(dsp2buffer[0] == 7);
    ZT_CHECK(!(dsp2state & DSP2F_AUTO_BUFFER_SHIFT));

    RESET_DSP2();
    put_entry(0, 0x08, 1, 0x8000);
    DSP2Write8b(0x8000, 11);
    ZT_CHECK(dsp2buffer[1] == 11);
    ZT_CHECK(!(dsp2state & DSP2F_AUTO_BUFFER_SHIFT));

    RESET_DSP2();
    put_entry(0, 0x08, 2, 0x8000);
    DSP2Write8b(0x8000, 13);
    ZT_CHECK(dsp2buffer[2] == 13);
    ZT_CHECK(!(dsp2state & DSP2F_AUTO_BUFFER_SHIFT));
}

static void test_w08_multiply_at_idx3(void)
{
    ZT_SECTION("w08: param1==3 computes buffer[0]*buffer[2], stores as LE uint32, sets AUTO_BUFFER_SHIFT");
    RESET_DSP2();
    put_entry(0, 0x08, 0, 0x8000);
    DSP2Write8b(0x8000, 3); /* buffer[0]=3 */
    put_entry(1, 0x08, 2, 0x8000);
    DSP2Write8b(0x8000, 5); /* buffer[2]=5 */
    put_entry(2, 0x08, 3, 0x8000);
    DSP2Write8b(0x8000, 0); /* triggers 3×5=15 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2state & DSP2F_AUTO_BUFFER_SHIFT);
    /* 15 = 0x0000000F stored little-endian */
    ZT_CHECK(dsp2buffer[0] == 0x0F);
    ZT_CHECK(dsp2buffer[1] == 0x00);
    ZT_CHECK(dsp2buffer[2] == 0x00);
    ZT_CHECK(dsp2buffer[3] == 0x00);
}

static void test_w08_multiply_large(void)
{
    ZT_SECTION("w08: 255×255=65025=0xFE01 stored little-endian in buffer[0..3]");
    RESET_DSP2();
    put_entry(0, 0x08, 0, 0x8000);
    DSP2Write8b(0x8000, 255);
    put_entry(1, 0x08, 2, 0x8000);
    DSP2Write8b(0x8000, 255);
    put_entry(2, 0x08, 3, 0x8000);
    DSP2Write8b(0x8000, 0);
    ZT_CHECK(dsp2buffer[0] == 0x01); /* 65025 & 0xFF */
    ZT_CHECK(dsp2buffer[1] == 0xFE); /* (65025 >> 8) & 0xFF */
    ZT_CHECK(dsp2buffer[2] == 0x00);
    ZT_CHECK(dsp2buffer[3] == 0x00);
}

/* ======================================================================== */
/* w09 — set original bitmap size                                           */
/* ======================================================================== */

static void test_w09_halt_on_zero(void)
{
    ZT_SECTION("w09: input 0 or 1 (sar to 0 signed) → sets HALT");
    RESET_DSP2();
    put_entry(0, 0x09, 0, 0x0000);
    DSP2Write8b(0x0000, 0x00); /* sar 0→0 */
    ZT_CHECK(dsp2state & DSP2F_HALT);

    RESET_DSP2();
    put_entry(0, 0x09, 0, 0x0000);
    DSP2Write8b(0x0000, 0x01); /* sar 1→0 (signed) */
    ZT_CHECK(dsp2state & DSP2F_HALT);
}

static void test_w09_sets_size_queues_w0a(void)
{
    ZT_SECTION("w09: input 0x04 → dsp2f0dSizeOrg=2, queues one w0A entry");
    RESET_DSP2();
    put_entry(0, 0x09, 0, 0x0000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x0000, 0x04); /* sar 4→2 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2f0dSizeOrg == 2);
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 1);
    ZT_CHECK(qcmd(wc) == 0x0A);
    ZT_CHECK(qaddr(wc) == 0x8000);
}

/* ======================================================================== */
/* w0A — set new bitmap size and build scaling map                          */
/* ======================================================================== */

static void test_w0a_halt_on_zero(void)
{
    ZT_SECTION("w0A: input 0 (sar to 0) → sets HALT");
    RESET_DSP2();
    put_entry(0, 0x0A, 0, 0x0000);
    DSP2Write8b(0x0000, 0x00);
    ZT_CHECK(dsp2state & DSP2F_HALT);
}

static void test_w0a_scaling_map(void)
{
    ZT_SECTION("w0A: origSize=4, input 0x04 (newSize=2) → 4 w0B entries + w00");
    RESET_DSP2();
    extern uint32_t dsp2f0dSizeOrg;
    dsp2f0dSizeOrg = 4;
    put_entry(0, 0x0A, 0, 0x0000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x0000, 0x04); /* sar 4→2 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 5); /* 4 w0B + 1 w00 */
    /* (cl * 2) / 4 for cl = 0,1,2,3 → 0,0,1,1 */
    ZT_CHECK(qcmd(wc + 0) == 0x0B);
    ZT_CHECK(qp1(wc + 0) == 0);
    ZT_CHECK(qaddr(wc + 0) == 0x8000);
    ZT_CHECK(qcmd(wc + 1) == 0x0B);
    ZT_CHECK(qp1(wc + 1) == 0);
    ZT_CHECK(qaddr(wc + 1) == 0x8001);
    ZT_CHECK(qcmd(wc + 2) == 0x0B);
    ZT_CHECK(qp1(wc + 2) == 1);
    ZT_CHECK(qaddr(wc + 2) == 0x8002);
    ZT_CHECK(qcmd(wc + 3) == 0x0B);
    ZT_CHECK(qp1(wc + 3) == 1);
    ZT_CHECK(qaddr(wc + 3) == 0x8003);
    ZT_CHECK(qcmd(wc + 4) == 0x00);
    ZT_CHECK(qaddr(wc + 4) == 0x8000);
}

static void test_w09_w0a_chain(void)
{
    ZT_SECTION("w09+w0A chain: two writes set sizes and build the w0B scaling map");
    RESET_DSP2();
    put_entry(0, 0x09, 0, 0x0000);
    DSP2Write8b(0x0000, 0x08); /* sar 8→4 → origSize=4; queues w0A */

    /* w0A is now at reader slot 1; the queued addr is 0x8000 */
    DSP2Write8b(0x8000, 0x04); /* sar 4→2 → newSize=2; queues 4 w0B + w00 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    /* reader advanced to 2; queued w0B entries are at slots 2..5 */
    uint32_t rc = dsp2enforcerReaderCursor;
    ZT_CHECK(qcmd(rc + 0) == 0x0B);
    ZT_CHECK(qp1(rc + 0) == 0);
    ZT_CHECK(qaddr(rc + 0) == 0x8000);
    ZT_CHECK(qcmd(rc + 1) == 0x0B);
    ZT_CHECK(qp1(rc + 1) == 0);
    ZT_CHECK(qaddr(rc + 1) == 0x8001);
    ZT_CHECK(qcmd(rc + 2) == 0x0B);
    ZT_CHECK(qp1(rc + 2) == 1);
    ZT_CHECK(qaddr(rc + 2) == 0x8002);
    ZT_CHECK(qcmd(rc + 3) == 0x0B);
    ZT_CHECK(qp1(rc + 3) == 1);
    ZT_CHECK(qaddr(rc + 3) == 0x8003);
}

/* ======================================================================== */
/* w00 sub-commands                                                         */
/* ======================================================================== */

static void test_w00p01_queues_32_w01(void)
{
    ZT_SECTION("w00 subcmd 01: queues 32 w01 entries (addr_low 0..31) + w00 incoming");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x8000, 0x01);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 33);
    /* spot-check first, a middle, and last w01 entries */
    ZT_CHECK(qcmd(wc + 0) == 0x01);
    ZT_CHECK(qaddr(wc + 0) == 0x8000);
    ZT_CHECK(qcmd(wc + 15) == 0x01);
    ZT_CHECK(qaddr(wc + 15) == 0x800F);
    ZT_CHECK(qcmd(wc + 31) == 0x01);
    ZT_CHECK(qaddr(wc + 31) == 0x801F);
    /* w00 incoming at slot wc+32 */
    ZT_CHECK(qcmd(wc + 32) == 0x00);
    ZT_CHECK(qaddr(wc + 32) == 0x8000);
}

static void test_w00p05_sets_no_addr_chk(void)
{
    ZT_SECTION("w00 subcmd 05: sets NO_ADDR_CHK, queues one w03 entry (no queueincoming)");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x8000, 0x05);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2state & DSP2F_NO_ADDR_CHK);
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 1);
    ZT_CHECK(qcmd(wc) == 0x03);
    ZT_CHECK(qaddr(wc) == 0x8000);
}

static void test_w00p06_queues_w06(void)
{
    ZT_SECTION("w00 subcmd 06: queues one w06 entry (no queueincoming)");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x8000, 0x06);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 1);
    ZT_CHECK(qcmd(wc) == 0x06);
    ZT_CHECK(qaddr(wc) == 0x8000);
}

static void test_w00p09_queues_4_w08(void)
{
    ZT_SECTION("w00 subcmd 09: queues 4 w08 entries (param 0..3, all addr 0x8000) + w00");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x8000, 0x09);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 5);
    ZT_CHECK(qcmd(wc + 0) == 0x08);
    ZT_CHECK(qp1(wc + 0) == 0);
    ZT_CHECK(qaddr(wc + 0) == 0x8000);
    ZT_CHECK(qcmd(wc + 1) == 0x08);
    ZT_CHECK(qp1(wc + 1) == 1);
    ZT_CHECK(qaddr(wc + 1) == 0x8000);
    ZT_CHECK(qcmd(wc + 2) == 0x08);
    ZT_CHECK(qp1(wc + 2) == 2);
    ZT_CHECK(qaddr(wc + 2) == 0x8000);
    ZT_CHECK(qcmd(wc + 3) == 0x08);
    ZT_CHECK(qp1(wc + 3) == 3);
    ZT_CHECK(qaddr(wc + 3) == 0x8000);
    ZT_CHECK(qcmd(wc + 4) == 0x00);
    ZT_CHECK(qaddr(wc + 4) == 0x8000);
}

static void test_w00p0d_queues_w09(void)
{
    ZT_SECTION("w00 subcmd 0D: queues one w09 entry (no queueincoming)");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    uint32_t wc = dsp2enforcerWriterCursor;
    DSP2Write8b(0x8000, 0x0D);
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerWriterCursor == wc + 1);
    ZT_CHECK(qcmd(wc) == 0x09);
    ZT_CHECK(qaddr(wc) == 0x8000);
}

/* ======================================================================== */
/* Integration / multi-write chains                                         */
/* ======================================================================== */

static void test_multiply_chain_via_w00p09(void)
{
    ZT_SECTION("full multiply chain: w00p09 + 4 w08 writes → product in buffer");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    DSP2Write8b(0x8000, 0x09); /* queues 4×w08 + w00 at slots 1..5 */
    DSP2Write8b(0x8000, 4); /* w08 param1=0 → buffer[0]=4        */
    DSP2Write8b(0x8000, 0); /* w08 param1=1 → buffer[1]=0        */
    DSP2Write8b(0x8000, 7); /* w08 param1=2 → buffer[2]=7        */
    DSP2Write8b(0x8000, 0); /* w08 param1=3 → 4×7=28             */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2state & DSP2F_AUTO_BUFFER_SHIFT);
    ZT_CHECK(dsp2buffer[0] == 28);
    ZT_CHECK(dsp2buffer[1] == 0);
    ZT_CHECK(dsp2buffer[2] == 0);
    ZT_CHECK(dsp2buffer[3] == 0);
}

static void test_multiply_then_read(void)
{
    ZT_SECTION("AUTO_BUFFER_SHIFT drains product 8 bits at a time via Read8b");
    RESET_DSP2();
    put_entry(0, 0x00, 0, 0x8000);
    DSP2Write8b(0x8000, 0x09);
    DSP2Write8b(0x8000, 4); /* buffer[0]=4  */
    DSP2Write8b(0x8000, 0); /* buffer[1]=0  */
    DSP2Write8b(0x8000, 255); /* buffer[2]=255 */
    DSP2Write8b(0x8000, 0); /* 4×255=1020=0x000003FC */
    ZT_CHECK(dsp2buffer[0] == 0xFC);
    ZT_CHECK(dsp2buffer[1] == 0x03);
    /* Read8b consumes via arithmetic shift: 0x000003FC→0x000003, etc. */
    ZT_CHECK(DSP2Read8b(0x8000) == 0xFC); /* buffer[0] before shift */
    ZT_CHECK(dsp2buffer[0] == 0x03); /* 0x3FC >> 8 = 0x3       */
    ZT_CHECK(dsp2buffer[1] == 0x00);
    ZT_CHECK(DSP2Read8b(0x8000) == 0x03);
    ZT_CHECK(dsp2buffer[0] == 0x00);
    ZT_CHECK(DSP2Read8b(0x8000) == 0x00); /* product fully drained   */
}

/* ======================================================================== */

int main(void)
{
    printf("ZSNES2 DSP2 coprocessor tests\n");

    test_read16b();
    test_write16b();

    test_read8b_halt();
    test_read8b_addr_validation();
    test_read8b_index();
    test_read8b_auto_shift();
    test_read8b_shift_sign();

    test_write8b_halt();
    test_write8b_addr_mismatch();
    test_write8b_addr_match();
    test_write8b_no_addr_chk();
    test_write8b_cursor_wrap();
    test_write8b_unknown_cmd();

    test_w0B();
    test_w04();
    test_w02();
    test_w07();
    test_w05();

    test_w00_clears_flags();
    test_w00_unknown_subcmd();
    test_w00p02_chain();

    test_w01_all_bits_set();
    test_w01_single_bit();
    test_w01_clears_bits();
    test_w01_column4();
    test_w01_arithmetic_shift();

    test_w03_halt_on_zero();
    test_w03_queue_entries();

    test_w06_halt_on_zero();
    test_w06_queue_entries();
    test_w06_w07_chain();

    test_w08_no_multiply_before_idx3();
    test_w08_multiply_at_idx3();
    test_w08_multiply_large();

    test_w09_halt_on_zero();
    test_w09_sets_size_queues_w0a();

    test_w0a_halt_on_zero();
    test_w0a_scaling_map();
    test_w09_w0a_chain();

    test_w00p01_queues_32_w01();
    test_w00p05_sets_no_addr_chk();
    test_w00p06_queues_w06();
    test_w00p09_queues_4_w08();
    test_w00p0d_queues_w09();

    test_multiply_chain_via_w00p09();
    test_multiply_then_read();

    ZT_RESULTS();
}
