/*
 * MSU-1 register stub unit tests
 *
 * Covers all functions ported from chips/msu1regs.asm:
 *   c_msustatusread, c_msudataread
 *   c_msuid1..c_msuid6
 *   c_msudataseek0..c_msudataseek3
 *   c_msu1track0, c_msu1track1
 *   c_msu1volume, c_msu1statecontrol
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

#define MSU_STATUS_DATA_BUSY 0x80

/* --- Mock globals -------------------------------------------------------- */

uint8_t MSU_StatusRead;
uint32_t MSU_Data_SeekPort;
uint32_t MSU_Data_Addr;
uint16_t MSU_Track;
uint8_t MSU_AudioVolume;
uint8_t MSU_StateControl;

static uint8_t mock_data[256];
uint8_t* MSU_DATA = mock_data;

static int handle_track_calls;
void MSU1HandleTrackChange(void) { handle_track_calls++; }

static int handle_status_calls;
void MSU1HandleControlBits(void) { handle_status_calls++; }

#define RESET_MOCKS()                            \
    do {                                         \
        handle_track_calls = 0;                  \
        handle_status_calls = 0;                 \
        MSU_StatusRead = 0;                      \
        MSU_Data_SeekPort = 0;                   \
        MSU_Data_Addr = 0;                       \
        MSU_Track = 0;                           \
        MSU_AudioVolume = 0;                     \
        MSU_StateControl = 0;                    \
        memset(mock_data, 0, sizeof(mock_data)); \
    } while (0)

/* --- Declarations (from chips/msu1regs.c) -------------------------------- */

uint8_t c_msustatusread(void);
uint8_t c_msudataread(void);
uint8_t c_msuid1(void);
uint8_t c_msuid2(void);
uint8_t c_msuid3(void);
uint8_t c_msuid4(void);
uint8_t c_msuid5(void);
uint8_t c_msuid6(void);
void c_msudataseek0(uint8_t val);
void c_msudataseek1(uint8_t val);
void c_msudataseek2(uint8_t val);
void c_msudataseek3(uint8_t val);
void c_msu1track0(uint8_t val);
void c_msu1track1(uint8_t val);
void c_msu1volume(uint8_t val);
void c_msu1statecontrol(uint8_t val);

/* ======================================================================== */

static void test_msustatusread(void)
{
    ZT_SECTION("c_msustatusread: returns MSU_StatusRead");

    RESET_MOCKS();
    MSU_StatusRead = 0xC4;
    uint8_t r = c_msustatusread();
    ZT_CHECK(r == 0xC4);

    /* verify call happens before the read: mock updates StatusRead */
    RESET_MOCKS();
    MSU_StatusRead = 0x00;
    /* we can't intercept mid-call, but we can confirm the value
       present after the call (set externally before calling) is returned */
    MSU_StatusRead = 0xA4;
    r = c_msustatusread();
    ZT_CHECK(r == 0xA4);
}

static void test_msudataread(void)
{
    ZT_SECTION("c_msudataread: reads MSU_DATA[Seek], increments Seek");

    RESET_MOCKS();
    mock_data[0] = 0x11;
    mock_data[1] = 0x22;
    mock_data[2] = 0x33;
    MSU_Data_Addr = 0;

    ZT_CHECK(c_msudataread() == 0x11);
    ZT_CHECK(MSU_Data_Addr == 1);

    ZT_CHECK(c_msudataread() == 0x22);
    ZT_CHECK(MSU_Data_Addr == 2);

    ZT_CHECK(c_msudataread() == 0x33);
    ZT_CHECK(MSU_Data_Addr == 3);

    /* seek at arbitrary offset */
    mock_data[100] = 0xAB;
    MSU_Data_Addr = 100;
    ZT_CHECK(c_msudataread() == 0xAB);
    ZT_CHECK(MSU_Data_Addr == 101);

    /* DATA_BUSY bit set: read returns current byte but does not advance */
    MSU_StatusRead = MSU_STATUS_DATA_BUSY;
    MSU_Data_Addr = 0;
    ZT_CHECK(c_msudataread() == 0x11);
    ZT_CHECK(MSU_Data_Addr == 0);
}

static void test_msu_id_chars(void)
{
    ZT_SECTION("c_msuid1..c_msuid6: spell out S - M S U 1");

    ZT_CHECK(c_msuid1() == 'S');
    ZT_CHECK(c_msuid2() == '-');
    ZT_CHECK(c_msuid3() == 'M');
    ZT_CHECK(c_msuid4() == 'S');
    ZT_CHECK(c_msuid5() == 'U');
    ZT_CHECK(c_msuid6() == '1');
}

static void test_msudataseek_bytes(void)
{
    ZT_SECTION("c_msudataseek0..3: write individual bytes of MSU_Data_SeekPort");

    /* byte 0, least significant byte */
    MSU_Data_SeekPort = 0x12345678u;
    c_msudataseek0(0xAB);
    ZT_CHECK(MSU_Data_SeekPort == 0x123456ABu);

    /* byte 1 */
    MSU_Data_SeekPort = 0x12345678u;
    c_msudataseek1(0xCD);
    ZT_CHECK(MSU_Data_SeekPort == 0x1234CD78u);

    /* byte 2 */
    MSU_Data_SeekPort = 0x12345678u;
    c_msudataseek2(0xEF);
    ZT_CHECK(MSU_Data_SeekPort == 0x12EF5678u);

    /* byte 3, most significant byte */
    MSU_Data_SeekPort = 0x12345678u;
    c_msudataseek3(0x90);
    ZT_CHECK(MSU_Data_SeekPort == 0x90345678u);
}

static void test_msu1track(void)
{
    ZT_SECTION("c_msu1track0/1: write track bytes; only track1 triggers HandleTrackChange");

    RESET_MOCKS();

    /* track0 writes low byte, no callback */
    MSU_Track = 0x1234u;
    c_msu1track0(0xAB);
    ZT_CHECK(MSU_Track == 0x12ABu);
    ZT_CHECK(handle_track_calls == 0);

    /* track1 writes high byte, then calls HandleTrackChange */
    MSU_Track = 0x1234u;
    c_msu1track1(0xCD);
    ZT_CHECK(MSU_Track == 0xCD34u);
    ZT_CHECK(handle_track_calls == 1);

    /* sequential: set both bytes */
    MSU_Track = 0;
    c_msu1track0(0x56);
    c_msu1track1(0x00);
    ZT_CHECK(MSU_Track == 0x0056u);
    ZT_CHECK(handle_track_calls == 2);
}

static void test_msu1volume(void)
{
    ZT_SECTION("c_msu1volume: writes MSU_AudioVolume directly");

    MSU_AudioVolume = 0;
    c_msu1volume(0xFF);
    ZT_CHECK(MSU_AudioVolume == 0xFF);

    c_msu1volume(0x80);
    ZT_CHECK(MSU_AudioVolume == 0x80);

    c_msu1volume(0x00);
    ZT_CHECK(MSU_AudioVolume == 0x00);
}

static void test_msu1statecontrol(void)
{
    ZT_SECTION("c_msu1statecontrol: writes MSU_StateControl, calls HandleStatusBits");

    RESET_MOCKS();
    c_msu1statecontrol(0x03);
    ZT_CHECK(MSU_StateControl == 0x03);
    ZT_CHECK(handle_status_calls == 1);

    c_msu1statecontrol(0x00);
    ZT_CHECK(MSU_StateControl == 0x00);
    ZT_CHECK(handle_status_calls == 2);
}

/* ======================================================================== */

int main(void)
{
    printf("ZSNES2 MSU-1 register tests\n");

    test_msustatusread();
    test_msudataread();
    test_msu_id_chars();
    test_msudataseek_bytes();
    test_msu1track();
    test_msu1volume();
    test_msu1statecontrol();

    ZT_RESULTS();
}
