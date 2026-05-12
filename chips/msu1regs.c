/*
 * MSU-1 register stubs
 *
 * Ported from chips/msu1regs.asm.
 *
 * msustatusread    — calls MSU1GetStatusBitsSpecial then returns MSU_StatusRead
 * msudataread      — returns MSU_DATA[MSU_Data_Seek], increments seek
 * msuid1..msuid6   — return the six ID bytes: 'S', '-', 'M', 'S', 'U', '1'
 * msudataseek0..3  — write one byte of MSU_Data_Seek (little-endian byte n)
 * msu1track0       — write low byte of MSU_Track
 * msu1track1       — write high byte of MSU_Track, then call MSU1HandleTrackChange
 * msu1volume       — write MSU_MusicVolume
 * msu1statecontrol — write MSU_CurrentStatus, then call MSU1HandleStatusBits
 */

#include <stdint.h>

extern uint8_t MSU_StatusRead;
extern uint32_t MSU_Data_Seek;
extern uint8_t* MSU_DATA;
extern uint16_t MSU_Track;
extern uint8_t MSU_MusicVolume;
extern uint8_t MSU_CurrentStatus;

extern void MSU1GetStatusBitsSpecial(void);
extern void MSU1HandleTrackChange(void);
extern void MSU1HandleStatusBits(void);

uint8_t msustatusread(void)
{
    MSU1GetStatusBitsSpecial();
    return MSU_StatusRead;
}

uint8_t msudataread(void)
{
    return MSU_DATA[MSU_Data_Seek++];
}

uint8_t msuid1(void) { return 'S'; }
uint8_t msuid2(void) { return '-'; }
uint8_t msuid3(void) { return 'M'; }
uint8_t msuid4(void) { return 'S'; }
uint8_t msuid5(void) { return 'U'; }
uint8_t msuid6(void) { return '1'; }

void msudataseek0(uint8_t val) { ((uint8_t*)&MSU_Data_Seek)[0] = val; }
void msudataseek1(uint8_t val) { ((uint8_t*)&MSU_Data_Seek)[1] = val; }
void msudataseek2(uint8_t val) { ((uint8_t*)&MSU_Data_Seek)[2] = val; }
void msudataseek3(uint8_t val) { ((uint8_t*)&MSU_Data_Seek)[3] = val; }

void msu1track0(uint8_t val)
{
    ((uint8_t*)&MSU_Track)[0] = val;
}

void msu1track1(uint8_t val)
{
    ((uint8_t*)&MSU_Track)[1] = val;
    MSU1HandleTrackChange();
}

void msu1volume(uint8_t val)
{
    MSU_MusicVolume = val;
}

void msu1statecontrol(uint8_t val)
{
    MSU_CurrentStatus = val;
    MSU1HandleStatusBits();
}
