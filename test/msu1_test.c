/*
 * MSU-1 register stub unit tests
 *
 * Covers all functions ported from chips/msu1regs.asm:
 *   msustatusread, msudataread
 *   msuid1..msuid6
 *   msudataseek0..msudataseek3
 *   msu1track0, msu1track1
 *   msu1volume, msu1statecontrol
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

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
        get_status_calls = 0;                    \
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

uint8_t msustatusread(void);
uint8_t msudataread(void);
uint8_t msuid1(void);
uint8_t msuid2(void);
uint8_t msuid3(void);
uint8_t msuid4(void);
uint8_t msuid5(void);
uint8_t msuid6(void);
void msudataseek0(uint8_t val);
void msudataseek1(uint8_t val);
void msudataseek2(uint8_t val);
void msudataseek3(uint8_t val);
void msu1track0(uint8_t val);
void msu1track1(uint8_t val);
void msu1volume(uint8_t val);
void msu1statecontrol(uint8_t val);

/* ======================================================================== */

static void test_msustatusread(void)
{
    ZT_SECTION("msustatusread: returns MSU_StatusRead");

    RESET_MOCKS();
    MSU_StatusRead = 0xC4;
    uint8_t r = msustatusread();
    ZT_CHECK(r == 0xC4);

    /* verify call happens before the read: mock updates StatusRead */
    RESET_MOCKS();
    MSU_StatusRead = 0x00;
    /* we can't intercept mid-call, but we can confirm the value
       present after the call (set externally before calling) is returned */
    MSU_StatusRead = 0xA4;
    r = msustatusread();
    ZT_CHECK(r == 0xA4);
}

static void test_msudataread(void)
{
    ZT_SECTION("msudataread: reads MSU_DATA[Seek], increments Seek");

    RESET_MOCKS();
    mock_data[0] = 0x11;
    mock_data[1] = 0x22;
    mock_data[2] = 0x33;
    MSU_Data_SeekPort = 0;

    ZT_CHECK(msudataread() == 0x11);
    ZT_CHECK(MSU_Data_SeekPort == 1);

    ZT_CHECK(msudataread() == 0x22);
    ZT_CHECK(MSU_Data_SeekPort == 2);

    ZT_CHECK(msudataread() == 0x33);
    ZT_CHECK(MSU_Data_SeekPort == 3);

    /* seek at arbitrary offset */
    mock_data[100] = 0xAB;
    MSU_Data_SeekPort = 100;
    ZT_CHECK(msudataread() == 0xAB);
    ZT_CHECK(MSU_Data_SeekPort == 101);
}

static void test_msu_id_chars(void)
{
    ZT_SECTION("msuid1..msuid6: spell out S - M S U 1");

    ZT_CHECK(msuid1() == 'S');
    ZT_CHECK(msuid2() == '-');
    ZT_CHECK(msuid3() == 'M');
    ZT_CHECK(msuid4() == 'S');
    ZT_CHECK(msuid5() == 'U');
    ZT_CHECK(msuid6() == '1');
}

static void test_msudataseek_bytes(void)
{
    ZT_SECTION("msudataseek0..3: write individual bytes of MSU_Data_SeekPort");

    /* byte 0 — least significant byte */
    MSU_Data_SeekPort = 0x12345678u;
    msudataseek0(0xAB);
    ZT_CHECK(MSU_Data_SeekPort == 0x123456ABu);

    /* byte 1 */
    MSU_Data_SeekPort = 0x12345678u;
    msudataseek1(0xCD);
    ZT_CHECK(MSU_Data_SeekPort == 0x1234CD78u);

    /* byte 2 */
    MSU_Data_SeekPort = 0x12345678u;
    msudataseek2(0xEF);
    ZT_CHECK(MSU_Data_SeekPort == 0x12EF5678u);

    /* byte 3 — most significant byte */
    MSU_Data_SeekPort = 0x12345678u;
    msudataseek3(0x90);
    ZT_CHECK(MSU_Data_SeekPort == 0x90345678u);
}

static void test_msu1track(void)
{
    ZT_SECTION("msu1track0/1: write track bytes; only track1 triggers HandleTrackChange");

    RESET_MOCKS();

    /* track0 writes low byte, no callback */
    MSU_Track = 0x1234u;
    msu1track0(0xAB);
    ZT_CHECK(MSU_Track == 0x12ABu);
    ZT_CHECK(handle_track_calls == 0);

    /* track1 writes high byte, then calls HandleTrackChange */
    MSU_Track = 0x1234u;
    msu1track1(0xCD);
    ZT_CHECK(MSU_Track == 0xCD34u);
    ZT_CHECK(handle_track_calls == 1);

    /* sequential: set both bytes */
    MSU_Track = 0;
    msu1track0(0x56);
    msu1track1(0x00);
    ZT_CHECK(MSU_Track == 0x0056u);
    ZT_CHECK(handle_track_calls == 2);
}

static void test_msu1volume(void)
{
    ZT_SECTION("msu1volume: writes MSU_AudioVolume directly");

    MSU_AudioVolume = 0;
    msu1volume(0xFF);
    ZT_CHECK(MSU_AudioVolume == 0xFF);

    msu1volume(0x80);
    ZT_CHECK(MSU_AudioVolume == 0x80);

    msu1volume(0x00);
    ZT_CHECK(MSU_AudioVolume == 0x00);
}

static void test_msu1statecontrol(void)
{
    ZT_SECTION("msu1statecontrol: writes MSU_StateControl, calls HandleStatusBits");

    RESET_MOCKS();
    msu1statecontrol(0x03);
    ZT_CHECK(MSU_StateControl == 0x03);
    ZT_CHECK(handle_status_calls == 1);

    msu1statecontrol(0x00);
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
