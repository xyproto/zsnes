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

extern uint8_t  dsp2buffer[256];
extern uint8_t  dsp2enforcerQueue[8 * 512];
extern uint8_t  dsp2enforcer[8];
extern uint8_t  dsp2f03KeyLo;
extern uint8_t  dsp2f03KeyHi;
extern uint32_t dsp2enforcerReaderCursor;
extern uint32_t dsp2enforcerWriterCursor;
extern uint32_t dsp2state;
extern uint32_t dsp2f0dSizeOrg;

#define DSP2F_HALT              1u
#define DSP2F_AUTO_BUFFER_SHIFT 2u
#define DSP2F_NO_ADDR_CHK       4u

/* --- Declarations ---------------------------------------------------------- */

uint8_t  DSP2Read8b(uint32_t addr);
uint16_t DSP2Read16b(uint32_t addr);
void     DSP2Write8b(uint32_t addr, uint8_t val);
void     DSP2Write16b(uint32_t addr, uint16_t val);

/* --- Helpers --------------------------------------------------------------- */

#define RESET_DSP2() \
    do { \
        memset(dsp2buffer, 0, sizeof(dsp2buffer)); \
        memset(dsp2enforcerQueue, 0, sizeof(dsp2enforcerQueue)); \
        memset(dsp2enforcer, 0, sizeof(dsp2enforcer)); \
        dsp2f03KeyLo = 0; \
        dsp2f03KeyHi = 0; \
        dsp2enforcerReaderCursor = 0; \
        dsp2enforcerWriterCursor = 1; \
        dsp2state = 0; \
        dsp2f0dSizeOrg = 0; \
    } while (0)

/* Write an 8-byte queue entry at slot `slot` */
static void put_entry(uint32_t slot, uint8_t cmd, uint8_t param1, uint16_t addr)
{
    uint8_t *e = dsp2enforcerQueue + slot * 8;
    memset(e, 0, 8);
    e[0] = cmd;
    e[1] = param1;
    e[4] = (uint8_t)addr;
    e[5] = (uint8_t)(addr >> 8);
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
    ZT_CHECK(r == 0x00);            /* value at [0] before shift */
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
    ZT_CHECK(dsp2buffer[0] == 0xAA);         /* unchanged */
    ZT_CHECK(dsp2enforcerReaderCursor == 0); /* not advanced */
}

static void test_write8b_addr_mismatch(void)
{
    ZT_SECTION("DSP2Write8b: addr mismatch → sets HALT flag");
    RESET_DSP2();
    put_entry(0, 0x0B, 0, 0x1234); /* queue expects addr 0x1234 */
    DSP2Write8b(0x5678, 0x42);     /* write to wrong address */
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
    DSP2Write8b(0xFFFF, 0x99);     /* completely different address */
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
    DSP2Write8b(0x8000, 0x03);    /* executes slot 0 = w00, subcmd=03 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2enforcerReaderCursor == 1);

    /* Now the writer has pushed [cmd=2] and [cmd=0] into slots 1 and 2.
     * Reader is at 1. */
    DSP2Write8b(0x8000, 0xC7);    /* executes slot 1 = w02, input=0xC7 */
    ZT_CHECK(!(dsp2state & DSP2F_HALT));
    ZT_CHECK(dsp2f03KeyLo == 0x07);
    ZT_CHECK(dsp2f03KeyHi == 0x70);
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

    ZT_RESULTS();
}
