#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include "types.h"

#define REGPTR(x) (regptra[(x) - 0x2000])
#define REGPTW(x) (regptwa[(x) - 0x2000])

void cycleinputdevice1(void);
bool cycleinputdevice2(void);

void DisplayBatteryStatus(void);

extern eop* regptra[0x3000];
extern eop* regptwa[0x3000];
extern u1 DSPDisable; // Disable DSP emulation
extern u1 MusicVol;
extern u1 V8Mode; // Vegetable mode! =) (Greyscale mode)
extern u1 cbitmode; // bit mode, 0=8bit, 1=16bit
extern u1 debugdisble; // debugger disable.  0 = no, 1 = yes
extern u1 gammalevel16b; // gamma level (16-bit engine)
extern u1 mode7tab[65536];
extern u1 newgfx16b;
extern u1 romispal; // 0 = NTSC, 1 = PAL
extern u1* romdata; // rom data  (4MB = 4194304)
extern u1* sfxramdata; // SuperFX Ram Data
extern u1* spcBuffera;
extern u1* spritetablea;
extern u1* sram; // sram = 65536 * 2 = 131072
extern u1* vbufdptr;
extern u1* vcache2b; // 2-bit video cache
extern u1* vcache4b; // 4-bit video cache
extern u1* vcache8b; // 8-bit video cache
extern u1* vidbuffer; // video buffer (1024x239 = 244736)
extern u1* vidbufferofsa; // offset 1
extern u1* vidbufferofsb; // offset 2
extern u1* vram; // vram = 65536
extern u1 vrama[65536];
extern u1* wramdata; // stack (64K = 65536)
extern u2 VolumeConvTable[32768];
extern u2 fulladdtab[65537];
extern u2 selcA000;

void MultiMouseProcess(void);

extern s4 MouseCount;
extern u1 mouse;
extern u2 MouseButtons[2];
extern u2 MouseMoveX[2];
extern u2 MouseMoveY[2];

/* Exact sizes. The assembly padded every buffer by a page, which is also what
   hid every overrun found so far from AddressSanitizer: a read that lands in
   your own slack is not a report. Where a tail is really used it is named. */
enum {
    BITCONV32_BYTES = 65536 * 4,
    RGBTOYUV_BYTES = 65536 * 4,
    SPCBUFFER_BYTES = 65536 * 4,
    SPRITETABLE_BYTES = 256 * 64 * 12, /* 64 SpriteInfo per line; 12 bytes is the 64-bit size */
    /* The EXTBG mode 7 writers stash a priority byte per pixel at line +
       75036*8 and hi-res mode 7 draws its second field 75036*4 further in, so
       the tail has to leave room for both at once. */
    VIDBUFFER_BYTES = 512 * 296 * 4 + 512 * 296 + 75036 * 4,
    VIDBUFFER2_BYTES = 288 * 2 * 256,
    NGWIN_BYTES = 256 * 224,
    VIDBUFFERD_BYTES = 1024 * 296,
    VCACHE2S_BYTES = 65536 * 4 * 4,
    VCACHE4S_BYTES = 65536 * 4 * 2,
    VCACHE8S_BYTES = 65536 * 4,
    VCACHE2_BYTES = 262144,
    VCACHE4_BYTES = 131072,
    VCACHE8_BYTES = 65536
};

#endif
