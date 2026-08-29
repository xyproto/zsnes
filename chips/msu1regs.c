/*
 * MSU-1 register stubs, from chips/msu1regs.asm.
 *
 * msustatusread    MSU_StatusRead
 * msudataread      MSU_DATA[MSU_Data_Addr], then step the address
 * msuid1..6        the ID bytes 'S', '-', 'M', 'S', 'U', '1'
 * msudataseek0..3  one little-endian byte of MSU_Data_SeekPort
 * msu1track0/1     the two halves of MSU_Track; the high one also runs
 *                  MSU1HandleTrackChange
 * msu1volume       MSU_AudioVolume
 * msu1statecontrol MSU_StateControl, then MSU1HandleControlBits
 */

#include <stdint.h>

#include "regabi.h"

#define MSU_STATUS_DATA_BUSY 0x80

extern uint8_t MSU_StatusRead;
extern uint32_t MSU_Data_SeekPort;
extern uint32_t MSU_Data_Addr;
extern uint8_t* MSU_DATA;
extern uint16_t MSU_Track;
extern uint8_t MSU_AudioVolume;
extern uint8_t MSU_StateControl;

extern void MSU1HandleTrackChange(void);
extern void MSU1HandleControlBits(void);

uint8_t c_msustatusread(void)
{
    return MSU_StatusRead;
}

uint8_t c_msudataread(void)
{
    // Reads have no effect when data busy bit set
    if (MSU_StatusRead & MSU_STATUS_DATA_BUSY) {
        return MSU_DATA[MSU_Data_Addr];
    } else {
        return MSU_DATA[MSU_Data_Addr++];
    }
}

uint8_t c_msuid1(void) { return 'S'; }
uint8_t c_msuid2(void) { return '-'; }
uint8_t c_msuid3(void) { return 'M'; }
uint8_t c_msuid4(void) { return 'S'; }
uint8_t c_msuid5(void) { return 'U'; }
uint8_t c_msuid6(void) { return '1'; }

void c_msudataseek0(uint8_t val) { ((uint8_t*)&MSU_Data_SeekPort)[0] = val; }
void c_msudataseek1(uint8_t val) { ((uint8_t*)&MSU_Data_SeekPort)[1] = val; }
void c_msudataseek2(uint8_t val) { ((uint8_t*)&MSU_Data_SeekPort)[2] = val; }
void c_msudataseek3(uint8_t val)
{
    // Writing to $2003 triggers seek
    ((uint8_t*)&MSU_Data_SeekPort)[3] = val;
    // Writes have no effect if data busy bit set
    if (!(MSU_StatusRead & MSU_STATUS_DATA_BUSY)) {
        MSU_StatusRead |= MSU_STATUS_DATA_BUSY; // Start seek, set data busy bit
        MSU_Data_Addr = MSU_Data_SeekPort; // Set data address to seek port
        MSU_StatusRead &= ~MSU_STATUS_DATA_BUSY; // Seek finished, clear data busy bit
    }
}

void c_msu1track0(uint8_t val)
{
    ((uint8_t*)&MSU_Track)[0] = val;
}

void c_msu1track1(uint8_t val)
{
    ((uint8_t*)&MSU_Track)[1] = val;
    MSU1HandleTrackChange();
}

void c_msu1volume(uint8_t val)
{
    MSU_AudioVolume = val;
}

void c_msu1statecontrol(uint8_t val)
{
    MSU_StateControl = val;
    MSU1HandleControlBits();
}

REGABI_REG_READ8(msustatusread);
REGABI_REG_READ8(msudataread);
REGABI_REG_READ8(msuid1);
REGABI_REG_READ8(msuid2);
REGABI_REG_READ8(msuid3);
REGABI_REG_READ8(msuid4);
REGABI_REG_READ8(msuid5);
REGABI_REG_READ8(msuid6);
REGABI_REG_WRITE8(msudataseek0);
REGABI_REG_WRITE8(msudataseek1);
REGABI_REG_WRITE8(msudataseek2);
REGABI_REG_WRITE8(msudataseek3);
REGABI_REG_WRITE8(msu1track0);
REGABI_REG_WRITE8(msu1track1);
REGABI_REG_WRITE8(msu1volume);
REGABI_REG_WRITE8(msu1statecontrol);
