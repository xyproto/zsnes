/*
 * ST010 coprocessor unit tests
 *
 * Covers all eight bank-access functions ported from chips/st10proc.asm:
 *   Region A — setaramdata buffer (r8/w8/r16/w16)
 *   Region B — SetaCmdEnable register (r8a/w8a/r16a/w16a)
 *
 * External dependencies (setaramdata, ST010DoCommand) are provided as
 * test-local stubs so no object files from the main build are needed.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

/* --- Mock dependencies -------------------------------------------------- */

/* +1 so a 16-bit read at offset 0xFFF doesn't go out of bounds */
static uint8_t mock_ram[0x1001];
uint8_t* setaramdata = mock_ram;

static int st010_cmd_calls;
void ST010DoCommand(void) { st010_cmd_calls++; }

/* --- Declarations (implemented in chips/st10proc.c) --------------------- */

uint8_t c_setaaccessbankr8(uint32_t addr);
void c_setaaccessbankw8(uint32_t addr, uint8_t val);
uint16_t c_setaaccessbankr16(uint32_t addr);
void c_setaaccessbankw16(uint32_t addr, uint16_t val);
uint8_t c_setaaccessbankr8a(uint32_t addr);
void c_setaaccessbankw8a(uint32_t addr, uint8_t val);
uint16_t c_setaaccessbankr16a(uint32_t addr);
void c_setaaccessbankw16a(uint32_t addr, uint16_t val);

/* ======================================================================= */
/* Region A — setaramdata buffer                                           */
/* ======================================================================= */

static void test_r8_reads_from_buffer(void)
{
    ZT_SECTION("r8: reads setaramdata at addr & 0xFFF");

    memset(mock_ram, 0, sizeof(mock_ram));
    mock_ram[0x100] = 0xAB;
    ZT_CHECK(c_setaaccessbankr8(0x100) == 0xAB);

    mock_ram[0xFFF] = 0x55;
    ZT_CHECK(c_setaaccessbankr8(0xFFF) == 0x55);

    /* address masked to 12 bits: 0x1001 & 0xFFF = 0x001 */
    mock_ram[0x001] = 0x77;
    ZT_CHECK(c_setaaccessbankr8(0x1001) == 0x77);
}

static void test_w8_rom_guard(void)
{
    ZT_SECTION("w8: bit-15 guard — addr with bit 15 set is ignored");

    memset(mock_ram, 0, sizeof(mock_ram));
    st010_cmd_calls = 0;

    c_setaaccessbankw8(0x8000, 0xFF);
    ZT_CHECK(mock_ram[0x000] == 0x00);
    ZT_CHECK(st010_cmd_calls == 0);

    c_setaaccessbankw8(0xFFFF, 0xFF);
    ZT_CHECK(mock_ram[0xFFF] == 0x00);
    ZT_CHECK(st010_cmd_calls == 0);
}

static void test_w8_writes_buffer(void)
{
    ZT_SECTION("w8: writes byte to setaramdata, address masked to 12 bits");

    memset(mock_ram, 0, sizeof(mock_ram));
    st010_cmd_calls = 0;
    mock_ram[0x21] = 0x00;

    c_setaaccessbankw8(0x042, 0xCC);
    ZT_CHECK(mock_ram[0x042] == 0xCC);
    ZT_CHECK(st010_cmd_calls == 0);

    /* 0x1100 & 0xFFF = 0x100 */
    c_setaaccessbankw8(0x1100, 0xDD);
    ZT_CHECK(mock_ram[0x100] == 0xDD);
}

static void test_w8_command_trigger(void)
{
    ZT_SECTION("w8: triggers ST010DoCommand when setaramdata[0x21] == 0x80");

    memset(mock_ram, 0, sizeof(mock_ram));
    st010_cmd_calls = 0;

    mock_ram[0x21] = 0x80;
    c_setaaccessbankw8(0x10, 0x01);
    ZT_CHECK(st010_cmd_calls == 1);

    /* no trigger when [0x21] has a different value */
    mock_ram[0x21] = 0x40;
    c_setaaccessbankw8(0x10, 0x01);
    ZT_CHECK(st010_cmd_calls == 1);

    mock_ram[0x21] = 0x00;
    c_setaaccessbankw8(0x10, 0x01);
    ZT_CHECK(st010_cmd_calls == 1);
}

static void test_r16_little_endian(void)
{
    ZT_SECTION("r16: reads 16-bit little-endian from setaramdata");

    memset(mock_ram, 0, sizeof(mock_ram));
    mock_ram[0x200] = 0x34;
    mock_ram[0x201] = 0x12;
    ZT_CHECK(c_setaaccessbankr16(0x200) == 0x1234);

    /* address masked: 0x1000 & 0xFFF = 0x000 */
    mock_ram[0x000] = 0xAB;
    mock_ram[0x001] = 0xCD;
    ZT_CHECK(c_setaaccessbankr16(0x1000) == 0xCDAB);
}

static void test_w16_rom_guard(void)
{
    ZT_SECTION("w16: bit-15 guard — addr with bit 15 set is ignored");

    memset(mock_ram, 0, sizeof(mock_ram));
    st010_cmd_calls = 0;

    c_setaaccessbankw16(0x8000, 0x1234);
    ZT_CHECK(mock_ram[0x000] == 0x00);
    ZT_CHECK(st010_cmd_calls == 0);

    c_setaaccessbankw16(0xFFFF, 0xABCD);
    ZT_CHECK(mock_ram[0xFFF] == 0x00);
    ZT_CHECK(st010_cmd_calls == 0);
}

static void test_w16_addr_7fff(void)
{
    ZT_SECTION("w16: addr 0x7FFF writes only low byte at [0xFFF], no trigger");

    memset(mock_ram, 0xEE, sizeof(mock_ram));
    st010_cmd_calls = 0;
    mock_ram[0x21] = 0x80; /* would trigger, but 7FFF case skips check */

    c_setaaccessbankw16(0x7FFF, 0x1234);
    ZT_CHECK(mock_ram[0xFFF] == 0x34); /* low byte written */
    ZT_CHECK(mock_ram[0xFFE] == 0xEE); /* neighbour untouched */
    ZT_CHECK(st010_cmd_calls == 0);
}

static void test_w16_wrap_at_fff(void)
{
    ZT_SECTION("w16: addr with lower 12 bits == 0xFFF wraps high byte to [0]");

    memset(mock_ram, 0, sizeof(mock_ram));
    st010_cmd_calls = 0;
    mock_ram[0x21] = 0x80; /* would trigger, but wrap case skips check */

    c_setaaccessbankw16(0xFFF, 0x1234);
    ZT_CHECK(mock_ram[0xFFF] == 0x34); /* low byte */
    ZT_CHECK(mock_ram[0x000] == 0x12); /* high byte wrapped to start */
    ZT_CHECK(st010_cmd_calls == 0);

    /* same wrap for 0x1FFF, 0x3FFF (bit 15 clear, lower 12 == 0xFFF) */
    memset(mock_ram, 0, sizeof(mock_ram));
    c_setaaccessbankw16(0x1FFF, 0xABCD);
    ZT_CHECK(mock_ram[0xFFF] == 0xCD);
    ZT_CHECK(mock_ram[0x000] == 0xAB);
    ZT_CHECK(st010_cmd_calls == 0);
}

static void test_w16_normal(void)
{
    ZT_SECTION("w16: normal write, little-endian, trigger when [0x21]==0x80");

    memset(mock_ram, 0, sizeof(mock_ram));
    st010_cmd_calls = 0;
    mock_ram[0x21] = 0x00;

    c_setaaccessbankw16(0x300, 0x5678);
    ZT_CHECK(mock_ram[0x300] == 0x78); /* low byte */
    ZT_CHECK(mock_ram[0x301] == 0x56); /* high byte */
    ZT_CHECK(st010_cmd_calls == 0);

    /* trigger when [0x21] == 0x80 */
    mock_ram[0x21] = 0x80;
    c_setaaccessbankw16(0x400, 0xABCD);
    ZT_CHECK(mock_ram[0x400] == 0xCD);
    ZT_CHECK(mock_ram[0x401] == 0xAB);
    ZT_CHECK(st010_cmd_calls == 1);

    /* address masked: 0x1300 & 0xFFF = 0x300 */
    mock_ram[0x21] = 0x00;
    c_setaaccessbankw16(0x1300, 0x1234);
    ZT_CHECK(mock_ram[0x300] == 0x34);
    ZT_CHECK(mock_ram[0x301] == 0x12);
}

/* ======================================================================= */
/* Region B — SetaCmdEnable register (4-byte array, 2-bit address mask)   */
/* ======================================================================= */

static void test_r8a_out_of_bounds(void)
{
    ZT_SECTION("r8a: returns 0 for addr >= 0x4000");

    ZT_CHECK(c_setaaccessbankr8a(0x4000) == 0x00);
    ZT_CHECK(c_setaaccessbankr8a(0xFFFF) == 0x00);
}

static void test_w8a_r8a_roundtrip(void)
{
    ZT_SECTION("w8a/r8a: write then read, address masked to 2 bits");

    c_setaaccessbankw8a(0, 0x11);
    c_setaaccessbankw8a(1, 0x22);
    c_setaaccessbankw8a(2, 0x33);
    c_setaaccessbankw8a(3, 0x44);

    ZT_CHECK(c_setaaccessbankr8a(0) == 0x11);
    ZT_CHECK(c_setaaccessbankr8a(1) == 0x22);
    ZT_CHECK(c_setaaccessbankr8a(2) == 0x33);
    ZT_CHECK(c_setaaccessbankr8a(3) == 0x44);

    /* addr masked to 2 bits: 4 → 0, 7 → 3 */
    ZT_CHECK(c_setaaccessbankr8a(4) == 0x11);
    ZT_CHECK(c_setaaccessbankr8a(7) == 0x44);
}

static void test_w8a_out_of_bounds(void)
{
    ZT_SECTION("w8a: addr >= 0x4000 is ignored");

    c_setaaccessbankw8a(0, 0xAA);
    c_setaaccessbankw8a(0x4000, 0xFF); /* should be ignored */
    ZT_CHECK(c_setaaccessbankr8a(0) == 0xAA);
}

static void test_r16a_out_of_bounds(void)
{
    ZT_SECTION("r16a: returns 0 for addr >= 0x4000");

    ZT_CHECK(c_setaaccessbankr16a(0x4000) == 0x0000);
    ZT_CHECK(c_setaaccessbankr16a(0xFFFF) == 0x0000);
}

static void test_r16a_byte_order_and_wrap(void)
{
    ZT_SECTION("r16a: big-endian pair from SetaCmdEnable, wraps at index 3->0");

    c_setaaccessbankw8a(0, 0x12);
    c_setaaccessbankw8a(1, 0x34);
    c_setaaccessbankw8a(2, 0x56);
    c_setaaccessbankw8a(3, 0x78);

    /* addr 0: high=[0]=0x12, low=[1]=0x34 → 0x1234 */
    ZT_CHECK(c_setaaccessbankr16a(0) == 0x1234);

    /* addr 2: high=[2]=0x56, low=[3]=0x78 → 0x5678 */
    ZT_CHECK(c_setaaccessbankr16a(2) == 0x5678);

    /* addr 3: high=[3]=0x78, low=[0]=0x12 (wrap) → 0x7812 */
    ZT_CHECK(c_setaaccessbankr16a(3) == 0x7812);

    /* addr masked: addr 4 → same as addr 0 */
    ZT_CHECK(c_setaaccessbankr16a(4) == 0x1234);
}

static void test_w16a_out_of_bounds(void)
{
    ZT_SECTION("w16a: addr >= 0x4000 is ignored");

    memset(mock_ram, 0xEE, sizeof(mock_ram));
    uint8_t saved = mock_ram[0];
    c_setaaccessbankw16a(0x4000, 0x1234);
    ZT_CHECK(mock_ram[0] == saved);
}

static void test_w16a_roundtrip(void)
{
    ZT_SECTION("w16a/r16a: symmetric big-endian roundtrip via SetaCmdEnable");

    /* clear */
    c_setaaccessbankw8a(0, 0);
    c_setaaccessbankw8a(1, 0);
    c_setaaccessbankw8a(2, 0);
    c_setaaccessbankw8a(3, 0);

    c_setaaccessbankw16a(0, 0x1234);
    ZT_CHECK(c_setaaccessbankr16a(0) == 0x1234);
    ZT_CHECK(c_setaaccessbankr8a(0) == 0x12); /* high byte at addr 0 */
    ZT_CHECK(c_setaaccessbankr8a(1) == 0x34); /* low  byte at addr 1 */

    /* wrap: addr 3 → [3]=high byte, [0]=low byte */
    c_setaaccessbankw16a(3, 0xABCD);
    ZT_CHECK(c_setaaccessbankr16a(3) == 0xABCD);
    ZT_CHECK(c_setaaccessbankr8a(3) == 0xAB);
    ZT_CHECK(c_setaaccessbankr8a(0) == 0xCD);

    /* address masked: addr 4 same as addr 0 */
    c_setaaccessbankw16a(4, 0x5678);
    ZT_CHECK(c_setaaccessbankr16a(0) == 0x5678);
}

/* ======================================================================= */

int main(void)
{
    printf("ZSNES2 ST010 coprocessor tests\n");

    test_r8_reads_from_buffer();
    test_w8_rom_guard();
    test_w8_writes_buffer();
    test_w8_command_trigger();
    test_r16_little_endian();
    test_w16_rom_guard();
    test_w16_addr_7fff();
    test_w16_wrap_at_fff();
    test_w16_normal();

    test_r8a_out_of_bounds();
    test_w8a_r8a_roundtrip();
    test_w8a_out_of_bounds();
    test_r16a_out_of_bounds();
    test_r16a_byte_order_and_wrap();
    test_w16a_out_of_bounds();
    test_w16a_roundtrip();

    ZT_RESULTS();
}
