/*
 * ST011 coprocessor unit tests
 *
 * Covers all eight bank-access functions ported from chips/st11proc.asm:
 *   Region 68 — setaramdata buffer (R8/W8/R16/W16)
 *   Region 60 — command/status port (R8/W8/R16/W16), via callbacks
 *
 * External dependencies are provided as test-local stubs.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

/* --- Mock state ---------------------------------------------------------- */

static uint8_t mock_ram[0x1001]; /* +1 so a 16-bit read at 0xFFF doesn't OOB */
uint8_t* setaramdata = mock_ram;

uint8_t ST011_DR;
uint16_t seta11_address;
uint8_t seta11_byte;

/* Call logs */
typedef struct {
    uint16_t addr;
    uint8_t byte;
} CallRec;

static CallRec w68_log[8];
static int w68_n;
static CallRec r60_log[8];
static int r60_n;
static uint8_t r60_ret[8];
static CallRec w60_log[8];
static int w60_n;

void ST011_MapW_68(void)
{
    w68_log[w68_n].addr = seta11_address;
    w68_log[w68_n].byte = seta11_byte;
    w68_n++;
}

void ST011_MapR_60(void)
{
    r60_log[r60_n].addr = seta11_address;
    seta11_byte = r60_ret[r60_n];
    r60_n++;
}

void ST011_MapW_60(void)
{
    w60_log[w60_n].addr = seta11_address;
    w60_log[w60_n].byte = seta11_byte;
    w60_n++;
}

#define RESET_MOCKS()                        \
    do {                                     \
        w68_n = r60_n = w60_n = 0;           \
        memset(w68_log, 0, sizeof(w68_log)); \
        memset(r60_log, 0, sizeof(r60_log)); \
        memset(r60_ret, 0, sizeof(r60_ret)); \
        memset(w60_log, 0, sizeof(w60_log)); \
        ST011_DR = 0;                        \
        seta11_address = 0;                  \
        seta11_byte = 0;                     \
    } while (0)

/* --- Declarations (from chips/st11proc.c) -------------------------------- */

uint8_t c_Seta11Read8_68(uint32_t addr);
void c_Seta11Write8_68(uint32_t addr, uint8_t val);
uint16_t c_Seta11Read16_68(uint32_t addr);
void c_Seta11Write16_68(uint32_t addr, uint16_t val);
uint8_t c_Seta11Read8_60(uint32_t addr);
void c_Seta11Write8_60(uint32_t addr, uint8_t val);
uint16_t c_Seta11Read16_60(uint32_t addr);
void c_Seta11Write16_60(uint32_t addr, uint16_t val);

/* ======================================================================== */
/* Region 68 — setaramdata buffer                                          */
/* ======================================================================== */

static void test_read8_68(void)
{
    ZT_SECTION("Read8_68: reads setaramdata[addr & 0xFFF], sets ST011_DR");

    memset(mock_ram, 0, sizeof(mock_ram));
    mock_ram[0x200] = 0xAB;
    ST011_DR = 0;

    ZT_CHECK(c_Seta11Read8_68(0x200) == 0xAB);
    ZT_CHECK(ST011_DR == 0xAB);

    /* address masked to 12 bits */
    mock_ram[0x300] = 0x77;
    ZT_CHECK(c_Seta11Read8_68(0x1300) == 0x77); /* 0x1300 & 0xFFF = 0x300 */
    ZT_CHECK(ST011_DR == 0x77);
}

static void test_write8_68_rom_guard(void)
{
    ZT_SECTION("Write8_68: bit-15 guard — addr with bit 15 set is ignored");

    RESET_MOCKS();
    c_Seta11Write8_68(0x8000, 0xFF);
    ZT_CHECK(w68_n == 0);

    c_Seta11Write8_68(0xFFFF, 0xFF);
    ZT_CHECK(w68_n == 0);
}

static void test_write8_68_normal(void)
{
    ZT_SECTION("Write8_68: sets seta11_address (raw, not masked), seta11_byte, calls MapW_68");

    RESET_MOCKS();
    c_Seta11Write8_68(0x100, 0xCD);

    ZT_CHECK(w68_n == 1);
    ZT_CHECK(w68_log[0].addr == 0x100);
    ZT_CHECK(w68_log[0].byte == 0xCD);

    /* full lower-16-bit address preserved (no 12-bit mask here) */
    RESET_MOCKS();
    c_Seta11Write8_68(0x2345, 0xEF);
    ZT_CHECK(w68_n == 1);
    ZT_CHECK(w68_log[0].addr == 0x2345);
    ZT_CHECK(w68_log[0].byte == 0xEF);
}

static void test_read16_68(void)
{
    ZT_SECTION("Read16_68: reads 16-bit LE, sets ST011_DR to high byte");

    memset(mock_ram, 0, sizeof(mock_ram));
    mock_ram[0x200] = 0x34;
    mock_ram[0x201] = 0x12;
    ST011_DR = 0;

    ZT_CHECK(c_Seta11Read16_68(0x200) == 0x1234);
    ZT_CHECK(ST011_DR == 0x12); /* high byte of the 16-bit result */

    /* address masked to 12 bits */
    mock_ram[0x000] = 0xCD;
    mock_ram[0x001] = 0xAB;
    ZT_CHECK(c_Seta11Read16_68(0x1000) == 0xABCD); /* 0x1000 & 0xFFF = 0x000 */
    ZT_CHECK(ST011_DR == 0xAB);
}

static void test_write16_68_rom_guard(void)
{
    ZT_SECTION("Write16_68: bit-15 guard — addr with bit 15 set is ignored");

    RESET_MOCKS();
    c_Seta11Write16_68(0x8000, 0x1234);
    ZT_CHECK(w68_n == 0);
}

static void test_write16_68_normal(void)
{
    ZT_SECTION("Write16_68: two MapW_68 calls — low byte then high byte (asm bug fixed)");

    RESET_MOCKS();
    c_Seta11Write16_68(0x100, 0xABCD);

    ZT_CHECK(w68_n == 2);
    /* first call: low byte at base address */
    ZT_CHECK(w68_log[0].addr == 0x100);
    ZT_CHECK(w68_log[0].byte == 0xCD); /* low byte */
    /* second call: high byte at address + 1 */
    ZT_CHECK(w68_log[1].addr == 0x101);
    ZT_CHECK(w68_log[1].byte == 0xAB); /* high byte */
}

/* ======================================================================== */
/* Region 60 — command/status port via callbacks                           */
/* ======================================================================== */

static void test_read8_60_guard(void)
{
    ZT_SECTION("Read8_60: addr >= 0x4000 returns 0, no callback");

    RESET_MOCKS();
    ZT_CHECK(c_Seta11Read8_60(0x4000) == 0x00);
    ZT_CHECK(c_Seta11Read8_60(0xFFFF) == 0x00);
    ZT_CHECK(r60_n == 0);
}

static void test_read8_60_normal(void)
{
    ZT_SECTION("Read8_60: 2-bit addr mask, one MapR_60 call, returns seta11_byte");

    RESET_MOCKS();
    r60_ret[0] = 0x5A;
    ZT_CHECK(c_Seta11Read8_60(0x01) == 0x5A);
    ZT_CHECK(r60_n == 1);
    ZT_CHECK(r60_log[0].addr == 0x01);

    /* addr masked to 2 bits: 0x05 → index 1 */
    RESET_MOCKS();
    r60_ret[0] = 0x3C;
    ZT_CHECK(c_Seta11Read8_60(0x05) == 0x3C); /* 5 & 3 = 1 */
    ZT_CHECK(r60_log[0].addr == 0x01);
}

static void test_write8_60_guard(void)
{
    ZT_SECTION("Write8_60: addr >= 0x4000 is ignored, no callback");

    RESET_MOCKS();
    c_Seta11Write8_60(0x4000, 0xFF);
    ZT_CHECK(w60_n == 0);
}

static void test_write8_60_normal(void)
{
    ZT_SECTION("Write8_60: 2-bit addr mask, sets seta11_byte, one MapW_60 call");

    RESET_MOCKS();
    c_Seta11Write8_60(0x02, 0xBB);
    ZT_CHECK(w60_n == 1);
    ZT_CHECK(w60_log[0].addr == 0x02);
    ZT_CHECK(w60_log[0].byte == 0xBB);

    /* addr masked: 0x06 → index 2 */
    RESET_MOCKS();
    c_Seta11Write8_60(0x06, 0xCC);
    ZT_CHECK(w60_log[0].addr == 0x02); /* 6 & 3 = 2 */
    ZT_CHECK(w60_log[0].byte == 0xCC);
}

static void test_read16_60_guard(void)
{
    ZT_SECTION("Read16_60: addr >= 0x4000 returns 0, no callbacks");

    RESET_MOCKS();
    ZT_CHECK(c_Seta11Read16_60(0x4000) == 0x0000);
    ZT_CHECK(r60_n == 0);
}

static void test_read16_60_normal(void)
{
    ZT_SECTION("Read16_60: two MapR_60 calls, first result=low byte, second=high");

    RESET_MOCKS();
    r60_ret[0] = 0x34; /* first read → low byte */
    r60_ret[1] = 0x12; /* second read → high byte */
    uint16_t result = c_Seta11Read16_60(0x00);
    ZT_CHECK(result == 0x1234);
    ZT_CHECK(r60_n == 2);
    ZT_CHECK(r60_log[0].addr == 0);
    ZT_CHECK(r60_log[1].addr == 1);

    /* addr wrap: starting at 3, second call wraps to addr 0 */
    RESET_MOCKS();
    r60_ret[0] = 0x78;
    r60_ret[1] = 0x56;
    result = c_Seta11Read16_60(0x03);
    ZT_CHECK(result == 0x5678);
    ZT_CHECK(r60_log[0].addr == 3);
    ZT_CHECK(r60_log[1].addr == 0); /* (3+1) & 3 = 0 */

    /* addr masked then two reads: 0x07 → index 3, wraps to 0 */
    RESET_MOCKS();
    r60_ret[0] = 0xCD;
    r60_ret[1] = 0xAB;
    result = c_Seta11Read16_60(0x07); /* 7 & 3 = 3 */
    ZT_CHECK(result == 0xABCD);
    ZT_CHECK(r60_log[0].addr == 3);
    ZT_CHECK(r60_log[1].addr == 0);
}

static void test_write16_60_guard(void)
{
    ZT_SECTION("Write16_60: addr >= 0x4000 is ignored, no callbacks");

    RESET_MOCKS();
    c_Seta11Write16_60(0x4000, 0x1234);
    ZT_CHECK(w60_n == 0);
}

static void test_write16_60_normal(void)
{
    ZT_SECTION("Write16_60: two MapW_60 calls — low byte then high byte, addr wraps");

    RESET_MOCKS();
    c_Seta11Write16_60(0x00, 0x1234);
    ZT_CHECK(w60_n == 2);
    ZT_CHECK(w60_log[0].addr == 0);
    ZT_CHECK(w60_log[0].byte == 0x34); /* low byte first */
    ZT_CHECK(w60_log[1].addr == 1);
    ZT_CHECK(w60_log[1].byte == 0x12); /* high byte second */

    /* wrap: start at addr 3, second write goes to addr 0 */
    RESET_MOCKS();
    c_Seta11Write16_60(0x03, 0xABCD);
    ZT_CHECK(w60_log[0].addr == 3);
    ZT_CHECK(w60_log[0].byte == 0xCD);
    ZT_CHECK(w60_log[1].addr == 0); /* (3+1) & 3 = 0 */
    ZT_CHECK(w60_log[1].byte == 0xAB);

    /* addr masked: 0x07 → index 3 */
    RESET_MOCKS();
    c_Seta11Write16_60(0x07, 0x5678);
    ZT_CHECK(w60_log[0].addr == 3); /* 7 & 3 = 3 */
    ZT_CHECK(w60_log[0].byte == 0x78);
    ZT_CHECK(w60_log[1].addr == 0);
    ZT_CHECK(w60_log[1].byte == 0x56);
}

/* ======================================================================== */

int main(void)
{
    printf("ZSNES2 ST011 coprocessor tests\n");

    test_read8_68();
    test_write8_68_rom_guard();
    test_write8_68_normal();
    test_read16_68();
    test_write16_68_rom_guard();
    test_write16_68_normal();

    test_read8_60_guard();
    test_read8_60_normal();
    test_write8_60_guard();
    test_write8_60_normal();
    test_read16_60_guard();
    test_read16_60_normal();
    test_write16_60_guard();
    test_write16_60_normal();

    ZT_RESULTS();
}
