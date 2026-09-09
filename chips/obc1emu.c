#include <stdbool.h>
#include <stdint.h>

static uint8_t* OBC1_RAM = 0;

int OBC1_Address;
int OBC1_BasePtr;
int OBC1_Shift;

uint16_t obc1_address;
uint8_t obc1_byte;

void GetOBC1(void)
{
    switch (obc1_address) {
    case 0x7ff0:
        obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2)];
        break;

    case 0x7ff1:
        obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 1];
        break;

    case 0x7ff2:
        obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 2];
        break;

    case 0x7ff3:
        obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 3];
        break;

    case 0x7ff4:
        obc1_byte = OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200];
        break;

    default:
        obc1_byte = OBC1_RAM[obc1_address & 0x1fff];
    }
}

void SetOBC1(void)
{
    switch (obc1_address) {
    case 0x7ff0: {
        OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2)] = obc1_byte;
        break;
    }

    case 0x7ff1: {
        OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 1] = obc1_byte;
        break;
    }

    case 0x7ff2: {
        OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 2] = obc1_byte;
        break;
    }

    case 0x7ff3: {
        OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 3] = obc1_byte;
        break;
    }

    case 0x7ff4: {
        unsigned char Temp;

        Temp = OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200];
        Temp = (Temp & ~(3 << OBC1_Shift)) | ((obc1_byte & 3) << OBC1_Shift);
        OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200] = Temp;
        break;
    }

    case 0x7ff5: {
        if (obc1_byte & 1)
            OBC1_BasePtr = 0x1800;
        else
            OBC1_BasePtr = 0x1c00;

        break;
    }

    case 0x7ff6: {
        OBC1_Address = obc1_byte & 0x7f;
        OBC1_Shift = (obc1_byte & 3) << 1;
        break;
    }
    }

    OBC1_RAM[obc1_address & 0x1fff] = obc1_byte;
}

extern uint8_t* romdata;
void InitOBC1(void)
{
    OBC1_RAM = romdata + 0x400000;
    if (OBC1_RAM[0x1ff5] & 1)
        OBC1_BasePtr = 0x1800;
    else
        OBC1_BasePtr = 0x1c00;

    OBC1_Address = OBC1_RAM[0x1ff6] & 0x7f;
    OBC1_Shift = (OBC1_RAM[0x1ff6] & 3) << 1;
}
