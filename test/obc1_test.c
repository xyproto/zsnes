/*
 * OBC1 coprocessor unit tests
 *
 * Covers all four bank-access functions ported from chips/obc1proc.asm:
 *   OBC1Read8b / OBC1Write8b / OBC1Read16b / OBC1Write16b
 *
 * Each function routes to one of three paths:
 *   - addr bit 15 set  → memaccessbank (tail-delegate)
 *   - addr < 0x6000    → regaccessbank (tail-delegate)
 *   - [0x6000, 0x7FFF] → OBC1 local logic via GetOBC1 / SetOBC1
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

/* --- Mock state ---------------------------------------------------------- */

uint16_t obc1_address;
uint8_t obc1_byte;

/* GetOBC1: sets obc1_byte from a queued response */
static uint8_t getobc1_queue[8];
static int getobc1_n;
void GetOBC1(void) { obc1_byte = getobc1_queue[getobc1_n++]; }

/* SetOBC1: records address + byte at time of call */
typedef struct {
    uint16_t addr;
    uint8_t byte;
} CallRec;
static CallRec setobc1_log[8];
static int setobc1_n;
void SetOBC1(void)
{
    setobc1_log[setobc1_n].addr = obc1_address;
    setobc1_log[setobc1_n].byte = obc1_byte;
    setobc1_n++;
}

/* Fallthrough delegates — distinctive sentinels make mis-routing obvious */
static uint32_t reg_r8_addr, mem_r8_addr;
static uint32_t reg_w8_addr, mem_w8_addr;
static uint8_t reg_w8_val, mem_w8_val;
static uint32_t reg_r16_addr, mem_r16_addr;
static uint32_t reg_w16_addr, mem_w16_addr;
static uint16_t reg_w16_val, mem_w16_val;
static int reg_r8_n, mem_r8_n, reg_w8_n, mem_w8_n;
static int reg_r16_n, mem_r16_n, reg_w16_n, mem_w16_n;

uint8_t regaccessbankr8(uint32_t a)
{
    reg_r8_addr = a;
    reg_r8_n++;
    return 0xAD;
}
uint8_t memaccessbankr8(uint32_t a)
{
    mem_r8_addr = a;
    mem_r8_n++;
    return 0xDE;
}
void regaccessbankw8(uint32_t a, uint8_t v)
{
    reg_w8_addr = a;
    reg_w8_val = v;
    reg_w8_n++;
}
void memaccessbankw8(uint32_t a, uint8_t v)
{
    mem_w8_addr = a;
    mem_w8_val = v;
    mem_w8_n++;
}
uint16_t regaccessbankr16(uint32_t a)
{
    reg_r16_addr = a;
    reg_r16_n++;
    return 0xBEEF;
}
uint16_t memaccessbankr16(uint32_t a)
{
    mem_r16_addr = a;
    mem_r16_n++;
    return 0xDEAD;
}
void regaccessbankw16(uint32_t a, uint16_t v)
{
    reg_w16_addr = a;
    reg_w16_val = v;
    reg_w16_n++;
}
void memaccessbankw16(uint32_t a, uint16_t v)
{
    mem_w16_addr = a;
    mem_w16_val = v;
    mem_w16_n++;
}

#define RESET_MOCKS()                                      \
    do {                                                   \
        getobc1_n = setobc1_n = 0;                         \
        memset(getobc1_queue, 0, sizeof(getobc1_queue));   \
        memset(setobc1_log, 0, sizeof(setobc1_log));       \
        reg_r8_n = mem_r8_n = reg_w8_n = mem_w8_n = 0;     \
        reg_r16_n = mem_r16_n = reg_w16_n = mem_w16_n = 0; \
        obc1_address = 0;                                  \
        obc1_byte = 0;                                     \
    } while (0)

/* --- Declarations (from chips/obc1proc.c) -------------------------------- */

uint8_t OBC1Read8b(uint32_t addr);
void OBC1Write8b(uint32_t addr, uint8_t val);
uint16_t OBC1Read16b(uint32_t addr);
void OBC1Write16b(uint32_t addr, uint16_t val);

/* ======================================================================== */
/* OBC1Read8b                                                               */
/* ======================================================================== */

static void test_read8b_routes_mem(void)
{
    ZT_SECTION("Read8b: bit-15 set → delegates to memaccessbankr8");

    RESET_MOCKS();
    uint8_t r = OBC1Read8b(0x8100);
    ZT_CHECK(mem_r8_n == 1);
    ZT_CHECK(mem_r8_addr == 0x8100);
    ZT_CHECK(reg_r8_n == 0);
    ZT_CHECK(r == 0xDE);

    RESET_MOCKS();
    OBC1Read8b(0xFFFF);
    ZT_CHECK(mem_r8_n == 1);
    ZT_CHECK(reg_r8_n == 0);
}

static void test_read8b_routes_reg(void)
{
    ZT_SECTION("Read8b: addr < 0x6000 → delegates to regaccessbankr8");

    RESET_MOCKS();
    uint8_t r = OBC1Read8b(0x2000);
    ZT_CHECK(reg_r8_n == 1);
    ZT_CHECK(reg_r8_addr == 0x2000);
    ZT_CHECK(mem_r8_n == 0);
    ZT_CHECK(r == 0xAD);

    RESET_MOCKS();
    OBC1Read8b(0x5FFF);
    ZT_CHECK(reg_r8_n == 1);
    ZT_CHECK(mem_r8_n == 0);
}

static void test_read8b_obc1(void)
{
    ZT_SECTION("Read8b: [0x6000,0x7FFF] → sets obc1_address, calls GetOBC1");

    RESET_MOCKS();
    getobc1_queue[0] = 0x42;
    uint8_t r = OBC1Read8b(0x6000);
    ZT_CHECK(reg_r8_n == 0);
    ZT_CHECK(mem_r8_n == 0);
    ZT_CHECK(obc1_address == 0x6000);
    ZT_CHECK(getobc1_n == 1);
    ZT_CHECK(r == 0x42);

    RESET_MOCKS();
    getobc1_queue[0] = 0x99;
    OBC1Read8b(0x7FFF);
    ZT_CHECK(obc1_address == 0x7FFF);
    ZT_CHECK(getobc1_n == 1);
}

/* ======================================================================== */
/* OBC1Write8b                                                              */
/* ======================================================================== */

static void test_write8b_routes_mem(void)
{
    ZT_SECTION("Write8b: bit-15 set → delegates to memaccessbankw8");

    RESET_MOCKS();
    OBC1Write8b(0x8000, 0xAB);
    ZT_CHECK(mem_w8_n == 1);
    ZT_CHECK(mem_w8_addr == 0x8000);
    ZT_CHECK(mem_w8_val == 0xAB);
    ZT_CHECK(reg_w8_n == 0);
}

static void test_write8b_routes_reg(void)
{
    ZT_SECTION("Write8b: addr < 0x6000 → delegates to regaccessbankw8");

    RESET_MOCKS();
    OBC1Write8b(0x1000, 0xCD);
    ZT_CHECK(reg_w8_n == 1);
    ZT_CHECK(reg_w8_addr == 0x1000);
    ZT_CHECK(reg_w8_val == 0xCD);
    ZT_CHECK(mem_w8_n == 0);
}

static void test_write8b_obc1(void)
{
    ZT_SECTION("Write8b: [0x6000,0x7FFF] → sets obc1_address + obc1_byte, calls SetOBC1");

    RESET_MOCKS();
    OBC1Write8b(0x6100, 0xEF);
    ZT_CHECK(reg_w8_n == 0);
    ZT_CHECK(mem_w8_n == 0);
    ZT_CHECK(setobc1_n == 1);
    ZT_CHECK(setobc1_log[0].addr == 0x6100);
    ZT_CHECK(setobc1_log[0].byte == 0xEF);
}

/* ======================================================================== */
/* OBC1Read16b                                                              */
/* ======================================================================== */

static void test_read16b_routes(void)
{
    ZT_SECTION("Read16b: routing — mem for bit-15, reg for < 0x6000");

    RESET_MOCKS();
    uint16_t r = OBC1Read16b(0x9000);
    ZT_CHECK(mem_r16_n == 1);
    ZT_CHECK(mem_r16_addr == 0x9000);
    ZT_CHECK(r == 0xDEAD);

    RESET_MOCKS();
    r = OBC1Read16b(0x3000);
    ZT_CHECK(reg_r16_n == 1);
    ZT_CHECK(reg_r16_addr == 0x3000);
    ZT_CHECK(r == 0xBEEF);
}

static void test_read16b_obc1(void)
{
    ZT_SECTION("Read16b: two GetOBC1 calls, addr increments, little-endian result");

    RESET_MOCKS();
    getobc1_queue[0] = 0x34; /* first  read → low  byte */
    getobc1_queue[1] = 0x12; /* second read → high byte */

    uint16_t r = OBC1Read16b(0x6000);
    ZT_CHECK(getobc1_n == 2);
    ZT_CHECK(r == 0x1234);

    /* verify address progressed: first at 0x6000, second at 0x6001 */
    ZT_CHECK(obc1_address == 0x6001);

    /* check second call actually used incremented address */
    RESET_MOCKS();
    getobc1_queue[0] = 0xCD;
    getobc1_queue[1] = 0xAB;
    r = OBC1Read16b(0x7FFE);
    ZT_CHECK(r == 0xABCD);
    ZT_CHECK(obc1_address == 0x7FFF);
}

/* ======================================================================== */
/* OBC1Write16b                                                             */
/* ======================================================================== */

static void test_write16b_routes(void)
{
    ZT_SECTION("Write16b: routing — mem for bit-15, reg for < 0x6000");

    RESET_MOCKS();
    OBC1Write16b(0xA000, 0x1234);
    ZT_CHECK(mem_w16_n == 1);
    ZT_CHECK(mem_w16_addr == 0xA000);
    ZT_CHECK(mem_w16_val == 0x1234);

    RESET_MOCKS();
    OBC1Write16b(0x0100, 0x5678);
    ZT_CHECK(reg_w16_n == 1);
    ZT_CHECK(reg_w16_addr == 0x0100);
    ZT_CHECK(reg_w16_val == 0x5678);
}

static void test_write16b_obc1(void)
{
    ZT_SECTION("Write16b: two SetOBC1 calls — low byte first, high byte second");

    RESET_MOCKS();
    OBC1Write16b(0x6000, 0xABCD);

    ZT_CHECK(setobc1_n == 2);
    ZT_CHECK(setobc1_log[0].addr == 0x6000);
    ZT_CHECK(setobc1_log[0].byte == 0xCD); /* low byte */
    ZT_CHECK(setobc1_log[1].addr == 0x6001);
    ZT_CHECK(setobc1_log[1].byte == 0xAB); /* high byte */

    /* check with a different address */
    RESET_MOCKS();
    OBC1Write16b(0x7FFE, 0x1234);
    ZT_CHECK(setobc1_log[0].addr == 0x7FFE);
    ZT_CHECK(setobc1_log[0].byte == 0x34);
    ZT_CHECK(setobc1_log[1].addr == 0x7FFF);
    ZT_CHECK(setobc1_log[1].byte == 0x12);
}

/* ======================================================================== */

int main(void)
{
    printf("ZSNES2 OBC1 coprocessor tests\n");

    test_read8b_routes_mem();
    test_read8b_routes_reg();
    test_read8b_obc1();

    test_write8b_routes_mem();
    test_write8b_routes_reg();
    test_write8b_obc1();

    test_read16b_routes();
    test_read16b_obc1();

    test_write16b_routes();
    test_write16b_obc1();

    ZT_RESULTS();
}
