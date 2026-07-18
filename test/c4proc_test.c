/*
 * C4 coprocessor interface unit tests (chips/c4proc.c).
 *
 * Covers the address routing of the four bank handlers, the $7F47
 * ROM-to-RAM copy trigger, and the pure-math commands dispatched by a
 * $7F4F write (multiply, sum, square, propulsion, polar conversions,
 * immediate values).  The rendering commands need ROM sprite data and
 * are covered by the ROM smoke test.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

extern uint8_t* C4Ram;
extern uint32_t C4values[3];
void InitC4(void);

uint8_t c_C4Read8b(uint32_t addr);
uint16_t c_C4Read16b(uint32_t addr);
void c_C4Write8b(uint32_t addr, uint8_t val);
void c_C4Write16b(uint32_t addr, uint16_t val);

/* externs c4proc.c references; the test owns them here */
uint8_t* romdata;
uint8_t* snesmmap[256];
static uint8_t rombuf[8 * 1024 * 1024];

/* routed handler stubs recording the last delegated access */
static uint32_t last_addr;
static uint8_t last_val8;
static uint16_t last_val16;
static const char* last_route;

uint8_t regaccessbankr8(uint32_t a)
{
    last_route = "regr8";
    last_addr = a;
    return 0x11;
}
uint8_t memaccessbankr8(uint32_t a)
{
    last_route = "memr8";
    last_addr = a;
    return 0x22;
}
void regaccessbankw8(uint32_t a, uint8_t v)
{
    last_route = "regw8";
    last_addr = a;
    last_val8 = v;
}
void memaccessbankw8(uint32_t a, uint8_t v)
{
    last_route = "memw8";
    last_addr = a;
    last_val8 = v;
}
uint16_t regaccessbankr16(uint32_t a)
{
    last_route = "regr16";
    last_addr = a;
    return 0x3344;
}
uint16_t memaccessbankr16(uint32_t a)
{
    last_route = "memr16";
    last_addr = a;
    return 0x5566;
}
void regaccessbankw16(uint32_t a, uint16_t v)
{
    last_route = "regw16";
    last_addr = a;
    last_val16 = v;
}
void memaccessbankw16(uint32_t a, uint16_t v)
{
    last_route = "memw16";
    last_addr = a;
    last_val16 = v;
}

static uint16_t ramw(uint32_t off)
{
    return (uint16_t)(C4Ram[off] | C4Ram[off + 1] << 8);
}

static void set_ramw(uint32_t off, uint16_t v)
{
    C4Ram[off] = (uint8_t)v;
    C4Ram[off + 1] = (uint8_t)(v >> 8);
}

static uint32_t raml(uint32_t off)
{
    return (uint32_t)(C4Ram[off] | C4Ram[off + 1] << 8 | C4Ram[off + 2] << 16
        | (uint32_t)C4Ram[off + 3] << 24);
}

/* run a command: set $7F4D mode, then write the command to $7F4F */
static void run_cmd(uint8_t mode, uint8_t cmd)
{
    C4Ram[0x1F4D] = mode;
    c_C4Write8b(0x7F4F, cmd);
}

static void test_routing(void)
{
    ZT_SECTION("bank handlers route mem/reg/C4 windows");

    ZT_CHECK_INT(c_C4Read8b(0x8123), 0x22);
    ZT_CHECK_STR(last_route, "memr8");
    ZT_CHECK_INT(last_addr, 0x8123);

    ZT_CHECK_INT(c_C4Read8b(0x2100), 0x11);
    ZT_CHECK_STR(last_route, "regr8");

    c_C4Write8b(0x9000, 0x77);
    ZT_CHECK_STR(last_route, "memw8");
    ZT_CHECK_INT(last_val8, 0x77);

    c_C4Write16b(0x4200, 0xBEEF);
    ZT_CHECK_STR(last_route, "regw16");
    ZT_CHECK_INT(last_val16, 0xBEEF);

    ZT_CHECK_INT(c_C4Read16b(0xC000), 0x5566);
    ZT_CHECK_INT(c_C4Read16b(0x1000), 0x3344);

    /* C4 RAM window, mirrored every 0x2000 */
    c_C4Write8b(0x6012, 0xAB);
    ZT_CHECK_INT(C4Ram[0x12], 0xAB);
    ZT_CHECK_INT(c_C4Read8b(0x6012), 0xAB);
    c_C4Write16b(0x6100, 0x1234);
    ZT_CHECK_INT(ramw(0x100), 0x1234);
    ZT_CHECK_INT(c_C4Read16b(0x6100), 0x1234);
}

static void test_memcpy_trigger(void)
{
    ZT_SECTION("$7F47 write copies from the SNES map into C4 RAM");

    memcpy(romdata + 0x1234, "\x01\x02\x03\x04\x05\x06\x07\x08", 8);
    snesmmap[0x02] = romdata;

    C4Ram[0x1F42] = 0x02; /* source bank */
    set_ramw(0x1F40, 0x1234); /* source address */
    set_ramw(0x1F43, 8); /* length */
    set_ramw(0x1F45, 0x6040); /* destination, masked to 0x0040 */
    c_C4Write8b(0x7F47, 0);

    ZT_CHECK_INT(C4Ram[0x40], 0x01);
    ZT_CHECK_INT(C4Ram[0x47], 0x08);

    /* length 0 copies nothing (the asm hung) */
    C4Ram[0x50] = 0xEE;
    set_ramw(0x1F43, 0);
    set_ramw(0x1F45, 0x6050);
    c_C4Write8b(0x7F47, 0);
    ZT_CHECK_INT(C4Ram[0x50], 0xEE);
}

static void test_math_commands(void)
{
    ZT_SECTION("dispatched math commands");

    /* 24-bit multiply */
    set_ramw(0x1F80, 0x0300);
    C4Ram[0x1F82] = 0;
    set_ramw(0x1F83, 0x0004);
    C4Ram[0x1F85] = 0;
    run_cmd(0x02, 0x25);
    ZT_CHECK_INT(raml(0x1F80), 0x0300u * 4);

    /* byte sum over the first 0x800 bytes */
    memset(C4Ram, 0, 0x800);
    C4Ram[0] = 200;
    C4Ram[0x7FF] = 100;
    run_cmd(0x02, 0x40);
    ZT_CHECK_INT(ramw(0x1F80), 300);

    /* 24-bit signed square: (-2)^2 = 4 */
    C4Ram[0x1F80] = 0xFE;
    C4Ram[0x1F81] = 0xFF;
    C4Ram[0x1F82] = 0xFF;
    run_cmd(0x02, 0x54);
    ZT_CHECK_INT(raml(0x1F83), 4);
    ZT_CHECK_INT(ramw(0x1F87), 0);

    /* propulsion: (65536 / 0x100) * 0x80 >> 8 = 0x80 */
    set_ramw(0x1F81, 0x0080);
    set_ramw(0x1F83, 0x0100);
    run_cmd(0x02, 0x05);
    ZT_CHECK_INT(ramw(0x1F80), 0x80);
    ZT_CHECK_INT(((uint16_t*)C4values)[3], 0x100);

    /* immediate ROM values */
    run_cmd(0x02, 0x89);
    ZT_CHECK_INT(C4Ram[0x1F80], 0x36);
    ZT_CHECK_INT(C4Ram[0x1F81], 0x43);
    ZT_CHECK_INT(C4Ram[0x1F82], 0x05);

    /* immediate register values */
    run_cmd(0x02, 0x5C);
    ZT_CHECK_INT(raml(0), 0xFF000000u);
    ZT_CHECK_INT(raml(20), 0x7FFFFF80u);
    ZT_CHECK_INT(raml(44), 0x00FEFF00u);

    /* polar to rectangular: angle 0x80 (90 degrees), distance 0x100:
       cos 90 = 0 and sin 90 = 0x7FFF, so x = 0, y = d - d/64.
       The two dword stores overlap at $1F89, like the asm's did. */
    set_ramw(0x1F80, 0x0080);
    set_ramw(0x1F83, 0x0100);
    run_cmd(0x02, 0x10);
    ZT_CHECK_INT(C4Ram[0x1F86], 0);
    ZT_CHECK_INT(C4Ram[0x1F87], 0);
    ZT_CHECK_INT(C4Ram[0x1F88], 0);
    ZT_CHECK_INT(raml(0x1F89), 0xFF - (0xFF >> 6));

    /* mode 0x0E short-circuit stores cmd >> 2 */
    run_cmd(0x0E, 0x04);
    ZT_CHECK_INT(C4Ram[0x1F80], 0x01);
}

int main(void)
{
    romdata = rombuf;
    InitC4();

    ZT_CHECK(C4Ram == romdata + 4096 * 1024 + 8192 * 8);

    test_routing();
    test_memcpy_trigger();
    test_math_commands();

    printf("C4 interface port tests\n");
    ZT_RESULTS();
}
