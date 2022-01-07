#include "c_dosintrf.h"
#include "../asm.h"
#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/c_execute.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../gui/c_gui.h"
#include "../gui/gui.h"
#include "../input.h"
#include "../ui.h"
#include "c_sound.h"
#include "dosintrf.h"
#include "sound.h"

extern unsigned short _djstat_flags;

u4 ZSNESBase;

static u1 previdmode; // previous video mode

// Find Selector - DOS only
static u2 findselec(u2 const segment)
{
    u2 selector;
    u4 failed;
    asm("int $0x31;  sbb %0, %0"
        : "=r"(failed), "=a"(selector)
        : "a"(2), "b"(segment)
        : "cc");
    if (failed) {
        PrintStr("Cannot find selector!\n\r");
        DosExit();
    }
    return selector;
}

void StartUp(void)
{
    _djstat_flags = 0xFFFF; // Optimize stat() calls by not calculating data useless for ZSNES

    // enable interrupts
    u1 res;
    asm volatile("int $0x31"
                 : "=a"(res)
                 : "a"(0x0901)
                 : "cc");
    (void)res;

    asm("movw %ds, %0"
        : "=mr"(dssel));

    selcA000 = findselec(0xA000);

    // get previous video mode
    u2 const selc0040 = findselec(0x0040);
    asm("pushl %%es;  movw %1, %%es;  movb %%es:0x49, %0;  popl %%es"
        : "=r"(previdmode)
        : "mr"(selc0040));

    // Get base address
    /* These variables are used for memory allocation so they can be ignored for
     * non-DOS ports */
    u2 base_lo;
    u2 base_hi;
    u4 failed;
    asm("movw %%ds, %%bx;  int $0x31;  sbb %0, %0"
        : "=r"(failed), "=c"(base_hi), "=d"(base_lo)
        : "a"(0x0006)
        : "cc", "ebx");
    if (!failed)
        ZSNESBase = base_hi << 16 | base_lo;
}

void SystemInit(void)
{
    u2 es;
    asm volatile("movw %%es, %0"
                 : "=mr"(es)); // XXX necessary?

    // Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
    getblaster(); // get set blaster environment
    if (Force8b == 1)
        SBHDMA = 0;

    asm volatile("movw %0, %%es"
                 : "mr"(es)); // XXX necessary?
}

void PrintChar(char const c)
{
    u4 res;
    asm volatile("int $0x21"
                 : "=a"(res)
                 : "a"(0x0200), "d"(c)
                 : "cc");
    (void)res;
}

void PrintStr(char const* s)
{
    for (;;) {
        char const c = *s++;
        if (c == '\0')
            break;
        PrintChar(c);
    }
}

char WaitForKey(void)
{
    char key;
    asm volatile("int $0x21"
                 : "=a"(key)
                 : "a"(0x0700)
                 : "cc");
    return key;
}

u1 Check_Key(void)
{
    u1 res;
    asm("int $0x21"
        : "=a"(res)
        : "a"(0x0B00)
        : "cc");
    return res;
}

char Get_Key(void)
{
    char c;
    asm volatile("int $0x21"
                 : "=a"(c)
                 : "a"(0x0700)
                 : "cc");
    return c;
}

void delay(u4 n)
{
    u1 prev = inb(0x61) & 0x10;
    do {
        u1 cur;
        do
            cur = inb(0x61) & 0x10;
        while (prev == cur); // XXX busy waiting
        prev = cur;
    } while (--n != 0);
}

static void get_handler(u1 const irq, u2* const segment, IRQHandler** const handler)
{
    u4 failed;
    asm("int $0x31;  sbb %0, %0"
        : "=a"(failed), "=c"(*segment), "=d"(*handler)
        : "a"(0x204), "b"(irq)
        : "cc");
    if (failed)
        interror();
}

void InitPreGame(void)
{
    // set up interrupt handler
    // get old handler pmode mode address
    // Process stuff such as sound init, interrupt initialization
    cli();
    get_handler(0x09, &oldhand9s, &oldhand9o);
    get_handler(0x08, &oldhand8s, &oldhand8o);

    if (V8Mode != GrayscaleMode)
        V8Mode ^= 1;

    if (NoSoundReinit != 1 && soundon != 0 && DSPDisable != 1) {
        get_handler(SBInt, &oldhandSBs, &oldhandSBo);
    }
    sti();
}

static void set_handler(u1 const irq, u2 const segment, IRQHandler* const handler)
{
    u4 failed;
    asm volatile("int $0x31;  sbb %0, %0"
                 : "=a"(failed)
                 : "a"(0x205), "b"(irq), "c"(segment), "d"(handler)
                 : "cc");
    if (failed)
        interror();
}

static u2 get_cs(void)
{
    u2 cs;
    asm("movw %%cs, %0"
        : "=mr"(cs));
    return cs;
}

void SetupPreGame(void)
{
    u2 const cs = get_cs();

    // set new handler
    if (soundon != 0 && DSPDisable != 1) {
        cli();
        u2 const PIC_port = PICMaskP;
        u1 const irq_bit = 1 << (SBIrq & 0x07);
        outb(PIC_port, inb(PIC_port) | irq_bit); // Turn off IRQ through controller
        set_handler(SBInt, cs, SBHandler);
        outb(PIC_port, inb(PIC_port) & ~irq_bit); // Turn on IRQ through controller
        InitSB();
        sti();
    }
    cli();
    set_handler(0x09, cs, handler9h);
    set_handler(0x08, cs, handler8h);
    init60hz(); // Set timer to 60/50Hz
    sti();
}

void DeInitPostGame(void)
{
    // de-init interrupt handler
    cli();
    set_handler(0x09, oldhand9s, oldhand9o);
    set_handler(0x08, oldhand8s, oldhand8o);
    init18_2hz(); // Set timer to 18.2Hz
    sti();

    // DeINITSPC
    if (soundon != 0 && DSPDisable != 1) {
        DeInitSPC();
        set_handler(SBInt, oldhandSBs, oldhandSBo);
    }
}

void GUIInit(void)
{
    get_handler(0x09, &GUIoldhand9s, &GUIoldhand9o);
    get_handler(0x08, &GUIoldhand8s, &GUIoldhand8o);
    u2 const cs = get_cs();
    set_handler(0x09, cs, GUIhandler9h);
    set_handler(0x08, cs, GUIhandler8h);
    GUIinit36_4hz();
}

void GUIDeInit(void)
{
    set_handler(0x09, GUIoldhand9s, GUIoldhand9o);
    set_handler(0x08, GUIoldhand8s, GUIoldhand8o);
    GUIinit18_2hz();
}

static inline void SetPal(u1 const i, u1 const r, u1 const g, u1 const b)
{
    outb(0x03C8, i);
    outb(0x03C9, r);
    outb(0x03C9, g);
    outb(0x03C9, b);
}

void displayfpspal(void)
{
    SetPal(128, 63, 63, 63);
    SetPal(192, 0, 0, 0);
}

void superscopepal(void)
{
    SetPal(144, 63, 0, 0);
}

void saveselectpal(void)
{ // set palette of colors 128,144, and 160 to white, blue, and red
    SetPal(128, 63, 63, 63);
    SetPal(144, 0, 0, 50);
    SetPal(160, 45, 0, 0);
    SetPal(176, 47, 0, 0);
    SetPal(208, 50, 25, 0);
}

void initvideo(void)
{
    asm_call(dosinitvideo);
}

void deinitvideo(void)
{
    u4 const eax = 0x00U << 8 | previdmode;
    asm volatile("int $0x10" ::"a"(eax));
}

void DrawScreen(void)
{
    asm_call(DosDrawScreen);
}

void vidpastecopyscr(void)
{
    if (GUI16VID[cvidmode] == 1) {
        u1* const buf = vidbuffer;
        u4 n = 224 * 288 - 288;
        u4 i = 224 * 288 - 1;
        do
            ((u2*)buf)[i] = GUICPC[buf[i]];
        while (--i, --n != 0);
    }
    asm_call(DosDrawScreenB);
}

void UpdateDevices(void)
{
    asm_call(DosUpdateDevices);
}

void JoyRead(void)
{
    asm_call(DOSJoyRead);
}

#define SetDefaultKey2(player, k)           \
    player##upk = k[2], /* Up     */        \
        player##downk = k[3], /* Down   */  \
        player##leftk = k[4], /* Left   */  \
        player##rightk = k[5], /* Right  */ \
        player##startk = k[1], /* Start  */ \
        player##selk = k[0], /* Select */   \
        player##Ak = k[7], /* A      */     \
        player##Bk = k[10], /* B      */    \
        player##Xk = k[6], /* X      */     \
        player##Yk = k[9], /* Y      */     \
        player##Lk = k[8], /* L      */     \
        player##Rk = k[11] /* R      */

void SetInputDevice(u1 const device, u1 const player)
{
    static u2 const keys[][12] = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 54, 28, 72, 80, 75, 77, 31, 45, 32, 30, 44, 46 },
        { 56, 29, 36, 50, 49, 51, 82, 71, 73, 83, 79, 81 },
        { 0, 0, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0, 0, 0, 0x082, 0x083, 0 },
        { 0, 0, 0x0E8, 0x0E9, 0x0EA, 0x0EB, 0, 0, 0, 0x084, 0x085, 0 },
        { 0, 0, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0x084, 0x085, 0, 0x082, 0x083, 0 },
        { 0, 0, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0x084, 0x085, 0x086, 0x082, 0x083, 0x087 },
        { 0x081, 0x080, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0x084, 0x085, 0x087, 0x082, 0x083, 0x086 },
        { 0x0C9, 0x0C8, 0x0D4, 0x0D5, 0x0D6, 0x0D7, 0x08C, 0x089, 0x08E, 0x08B, 0x088, 0x08F },
        { 0x0C9 + 8, 0x0C8 + 8, 0x0D4 + 8, 0x0D5 + 8, 0x0D6 + 8, 0x0D7 + 8, 0x08C + 8, 0x089 + 8, 0x08E + 8, 0x08B + 8, 0x088 + 8, 0x08F + 8 },
        { 0x0C9 + 8 * 2, 0x0C8 + 8 * 2, 0x0D4 + 8 * 2, 0x0D5 + 8 * 2, 0x0D6 + 8 * 2, 0x0D7 + 8 * 2, 0x08C + 8 * 2, 0x089 + 8 * 2, 0x08E + 8 * 2, 0x08B + 8 * 2, 0x088 + 8 * 2, 0x08F + 8 * 2 },
        { 0x0C9 + 8 * 3, 0x0C8 + 8 * 3, 0x0D4 + 8 * 3, 0x0D5 + 8 * 3, 0x0D6 + 8 * 3, 0x0D7 + 8 * 3, 0x08C + 8 * 3, 0x089 + 8 * 3, 0x08E + 8 * 3, 0x08B + 8 * 3, 0x088 + 8 * 3, 0x08F + 8 * 3 },
        { 0x0CA, 0x0CB, 0x0F0, 0x0F1, 0x0F2, 0x0F3, 0x0A9, 0x0AB, 0x0AC, 0x0A8, 0x0AA, 0x0AE },
        { 0x0CA + 8, 0x0CB + 8, 0x0F0 + 4, 0x0F1 + 4, 0x0F2 + 4, 0x0F3 + 4, 0x0A9 + 8, 0x0AB + 8, 0x0AC + 8, 0x0A8 + 8, 0x0AA + 8, 0x0AE + 8 },
        { 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x189, 0x188, 0x18A, 0x181, 0x180, 0x18B },
        { 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x199, 0x198, 0x19A, 0x191, 0x190, 0x19B },
        { 0x1A2, 0x1A3, 0x1A4, 0x1A5, 0x1A6, 0x1A7, 0x1A9, 0x1A8, 0x1AA, 0x1A1, 0x1A0, 0x1AB },
        { 0x1B2, 0x1B3, 0x1B4, 0x1B5, 0x1B6, 0x1B7, 0x1B9, 0x1B8, 0x1BA, 0x1B1, 0x1B0, 0x1BB },
        { 0x1C2, 0x1C3, 0x1C4, 0x1C5, 0x1C6, 0x1C7, 0x1C9, 0x1C8, 0x1CA, 0x1C1, 0x1C0, 0x1CB }
    };

    // Sets keys according to input device selected
    u2 const* k;
    if (device == 0) {
        k = keys[0];
    } else if (device == 1) {
        switch (player) {
        case 0:
            k = keys[1];
            break;
        case 1:
            k = keys[2];
            break;
        default:
            return;
        }
    } else if (device == 2) {
        u4 n = 0;
        if (pl1contrl == 2)
            ++n;
        if (pl2contrl == 2)
            ++n;
        if (pl3contrl == 2)
            ++n;
        if (pl4contrl == 2)
            ++n;
        if (pl5contrl == 2)
            ++n;
        k = n < 2 ? keys[3] : keys[4];
    } else if (device < 17) {
        k = keys[device - 3 + 5];
    } else {
        return;
    }

    switch (player) {
    case 0:
        SetDefaultKey2(pl1, k);
        break;
    case 1:
        SetDefaultKey2(pl2, k);
        break;
    case 2:
        SetDefaultKey2(pl3, k);
        break;
    case 3:
        SetDefaultKey2(pl4, k);
        break;
    case 4:
        SetDefaultKey2(pl5, k);
        break;
    }
}

#undef SetDefaultKey2

/*****************************
 * Mouse Stuff
 *****************************/

u4 Init_Mouse(void)
{
    u2 success;
    asm volatile("int $0x33"
                 : "=a"(success)
                 : "a"(0)
                 : "cc", "ebx");
    if (!success)
        return 0;
    Set_MouseXMax(0, 255);
    Set_MouseYMax(0, 223);
    asm volatile("int $0x33" ::"a"(0x0F), "c"(8), "d"(8));
    Set_MousePosition(0, 0);
    return 1;
}

u4 Get_MouseData(void)
{
    // bx: bit 0 = left button, bit 1 = right button
    // cx = Mouse X Position, dx = Mouse Y Position
    u2 buttons;
    u2 x;
    u2 y;
    asm volatile("int $0x33"
                 : "=b"(buttons), "=c"(x), "=d"(y)
                 : "a"(0x03)
                 : "cc");
    return y << 24 | x << 16 | buttons;
}

void Set_MouseXMax(u4 const min, u4 const max)
{
    asm volatile("int $0x33" ::"a"(0x07), "c"(min), "d"(max));
}

void Set_MouseYMax(u4 const min, u4 const max)
{
    asm volatile("int $0x33" ::"a"(0x08), "c"(min), "d"(max));
}

void Set_MousePosition(u4 const x, u4 const y)
{
    asm volatile("int $0x33" ::"a"(0x04), "c"(x), "d"(y));
}

u4 Get_MousePositionDisplacement(void)
{
    u2 x;
    u2 y;
    asm volatile("int $0x33"
                 : "=c"(x), "=d"(y)
                 : "a"(0x0B));
    return y << 16 | x;
}

void MouseWindow(void) { }

void StopSound(void) { }
void StartSound(void) { }

void Check60hz(void) { }

char const GUIVideoModeNames[][18] = {
    "256x224x8B  MODEQ", //  0
    "256x240x8B  MODEQ", //  1
    "256x256x8B  MODEQ", //  2
    "320x224x8B  MODEX", //  3
    "320x240x8B  MODEX", //  4
    "320x256x8B  MODEX", //  5
    "640x480x16B VESA1", //  6
    "320x240x8B  VESA2", //  7
    "320x240x16B VESA2", //  8
    "320x480x8B  VESA2", //  9
    "320x480x16B VESA2", // 10
    "512x384x8B  VESA2", // 11
    "512x384x16B VESA2", // 12
    "640x400x8B  VESA2", // 13
    "640x400x16B VESA2", // 14
    "640x480x8B  VESA2", // 15
    "640x480x16B VESA2", // 16
    "800x600x8B  VESA2", // 17
    "800x600x16B VESA2" // 18
};

u4 const NumVideoModes = lengthof(GUIVideoModeNames);

#define _ 0
#define X 1
/* Video Mode Feature Availability (X = Available, _ = Not Available)
 * Left side starts with Video Mode 0
 *                0                   1
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 */
u1 GUI16VID[] = { _, _, _, _, _, _, X, _, X, _, X, _, X, _, X, _, X, _, X };
u1 GUI2xVID[] = { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, X, _, _ };
u1 GUIEAVID[] = { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, X, _, _, _ };
u1 GUIHQ2X[] = { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ };
u1 GUIHSVID[] = { _, _, _, _, _, _, _, _, _, _, X, _, _, _, _, _, X, _, _ };
u1 GUII2VID[] = { _, _, _, _, _, _, _, _, X, _, X, _, _, _, _, _, X, _, X };
u1 GUIM7VID[] = { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, X, _, _ };
u1 GUINTVID[] = { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ };
u1 GUISLVID[] = { _, _, X, _, _, X, X, _, _, X, X, _, _, _, _, X, X, X, X };
u1 GUISSVID[] = { _, _, _, _, _, _, X, _, _, _, _, X, X, X, X, X, X, X, X };
u1 GUITBVID[] = { _, _, _, _, _, _, _, X, X, X, X, X, X, X, X, X, X, X, X };
u1 GUIWSVID[] = { _, _, _, _, _, _, _, X, X, X, X, _, _, X, X, X, X, _, _ };
#undef X
#undef _

char const GUIInputNames[][17] = {
    "NONE            ",
    "KEYBOARD        ",
    "2BUTTONJOYSTICK ",
    "4BUTTONJOYSTICK ",
    "6BUTTONJOYSTICK ",
    "8BUTTONJOYSTICK ",
    "SIDEWINDERPAD1  ",
    "SIDEWINDERPAD2  ",
    "SIDEWINDERPAD3  ",
    "SIDEWINDERPAD4  ",
    "GAMEPAD PRO P0  ",
    "GAMEPAD PRO P1  ",
    "PARALLEL LPT1 P1",
    "PARALLEL LPT1 P2",
    "PARALLEL LPT1 P3",
    "PARALLEL LPT1 P4",
    "PARALLEL LPT1 P5"
};

u4 const NumInputDevices = lengthof(GUIInputNames);

char const ScanCodeListing[] = {
    "---"
    "ESC"
    " 1 "
    " 2 "
    " 3 "
    " 4 "
    " 5 "
    " 6 "
    " 7 "
    " 8 "
    " 9 "
    " 0 "
    " - "
    " = "
    "BKS"
    "TAB"
    " Q "
    " W "
    " E "
    " R "
    " T "
    " Y "
    " U "
    " I "
    " O "
    " P "
    " [ "
    " ] "
    "RET"
    "CTL"
    " A "
    " S "
    " D "
    " F "
    " G "
    " H "
    " J "
    " K "
    " L "
    " : "
    " \" "
    " ~ "
    "LSH"
    " \\ "
    " Z "
    " X "
    " C "
    " V "
    " B "
    " N "
    " M "
    " , "
    " . "
    " / "
    "RSH"
    " * "
    "ALT"
    "SPC"
    "CAP"
    "F1 "
    "F2 "
    "F3 "
    "F4 "
    "F5 "
    "F6 "
    "F7 "
    "F8 "
    "F9 "
    "F10"
    "NUM"
    "SCR"
    "HOM"
    "UP "
    "PGU"
    " - "
    "LFT"
    " 5 "
    "RGT"
    " + "
    "END"
    "DWN"
    "PGD"
    "INS"
    "DEL"
    "   "
    "   "
    "   "
    "F11"
    "F12"
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    "   "
    // Joystick Stuff, Port 201h (80h)
    "JB7"
    "JB8"
    "JB1"
    "JB2"
    "JB3"
    "JB4"
    "JB5"
    "JB6"
    "SWA"
    "SWB"
    "SWC"
    "SWX"
    "SWY"
    "SWZ"
    "SWL"
    "SWR"
    "S2A"
    "S2B"
    "S2C"
    "S2X"
    "S2Y"
    "S2Z"
    "S2L"
    "S2R"
    "S3A"
    "S3B"
    "S3C"
    "S3X"
    "S3Y"
    "S3Z"
    "S3L"
    "S3R"
    "S4A"
    "S4B"
    "S4C"
    "S4X"
    "S4Y"
    "S4Z"
    "S4L"
    "S4R"
    "GRR"
    "GRB"
    "GRY"
    "GRG"
    "GL1"
    "GL2"
    "GR1"
    "GR2"
    "G2R"
    "G2B"
    "G2Y"
    "G2G"
    "2L1"
    "2L2"
    "2R1"
    "2R2"
    "G3R"
    "G3B"
    "G3Y"
    "G3G"
    "3L1"
    "3L2"
    "3R1"
    "3R2"
    "G4R"
    "G4B"
    "G4Y"
    "G4G"
    "4L1"
    "4L2"
    "4R1"
    "4R2"
    "SWS"
    "SWM"
    "GSL"
    "GST"
    "JUP"
    "JDN"
    "JLF"
    "JRG"
    "S2S"
    "S2M"
    "2SL"
    "2ST"
    "SWU"
    "SWD"
    "SWL"
    "SWR"
    "S3S"
    "S3M"
    "3SL"
    "3ST"
    "S2U"
    "S2D"
    "S2L"
    "S2R"
    "S4S"
    "S4M"
    "4SL"
    "4ST"
    "S3U"
    "S3D"
    "S3L"
    "S3R"
    "J2U"
    "J2D"
    "J2L"
    "J2R"
    "S4U"
    "S4D"
    "S4L"
    "S4R"
    "GRU"
    "GRD"
    "GRL"
    "GRR"
    "G2U"
    "G2D"
    "G2L"
    "G2R"
    "G3U"
    "G3D"
    "G3L"
    "G3R"
    "G4U"
    "G4D"
    "G4L"
    "G4R"
    // Joystick Stuff, Port 209h (100h)
    "JB7"
    "JB8"
    "JB1"
    "JB2"
    "JB3"
    "JB4"
    "JB5"
    "JB6"
    "SWA"
    "SWB"
    "SWC"
    "SWX"
    "SWY"
    "SWZ"
    "SWL"
    "SWR"
    "S2A"
    "S2B"
    "S2C"
    "S2X"
    "S2Y"
    "S2Z"
    "S2L"
    "S2R"
    "S3A"
    "S3B"
    "S3C"
    "S3X"
    "S3Y"
    "S3Z"
    "S3L"
    "S3R"
    "S4A"
    "S4B"
    "S4C"
    "S4X"
    "S4Y"
    "S4Z"
    "S4L"
    "S4R"
    "GRR"
    "GRB"
    "GRY"
    "GRG"
    "GL1"
    "GL2"
    "GR1"
    "GR2"
    "G2R"
    "G2B"
    "G2Y"
    "G2G"
    "2L1"
    "2L2"
    "2R1"
    "2R2"
    "G3R"
    "G3B"
    "G3Y"
    "G3G"
    "3L1"
    "3L2"
    "3R1"
    "3R2"
    "G4R"
    "G4B"
    "G4Y"
    "G4G"
    "4L1"
    "4L2"
    "4R1"
    "4R2"
    "SWS"
    "SWM"
    "GSL"
    "GST"
    "JUP"
    "JDN"
    "JLF"
    "JRG"
    "S2S"
    "S2M"
    "2SL"
    "2ST"
    "SWU"
    "SWD"
    "SWL"
    "SWR"
    "S3S"
    "S3M"
    "3SL"
    "3ST"
    "S2U"
    "S2D"
    "S2L"
    "S2R"
    "S4S"
    "S4M"
    "4SL"
    "4ST"
    "S3U"
    "S3D"
    "S3L"
    "S3R"
    "J2U"
    "J2D"
    "J2L"
    "J2R"
    "S4U"
    "S4D"
    "S4L"
    "S4R"
    "GRU"
    "GRD"
    "GRL"
    "GRR"
    "G2U"
    "G2D"
    "G2L"
    "G2R"
    "G3U"
    "G3D"
    "G3L"
    "G3R"
    "G4U"
    "G4D"
    "G4L"
    "G4R"
    // Extra Stuff (180h) (Parallel Port)
    "PPB"
    "PPY"
    "PSL"
    "PST"
    "PUP"
    "PDN"
    "PLT"
    "PRT"
    "PPA"
    "PPX"
    "PPL"
    "PPR"
    "   "
    "   "
    "   "
    "   "
    "P2B"
    "P2Y"
    "P2S"
    "P2T"
    "P2U"
    "P2D"
    "P2L"
    "P2R"
    "P2A"
    "P2X"
    "P2L"
    "P2R"
    "   "
    "   "
    "   "
    "   "
    "P3B"
    "P3Y"
    "P3S"
    "P3T"
    "P3U"
    "P3D"
    "P3L"
    "P3R"
    "P3A"
    "P3X"
    "P3L"
    "P3R"
    "   "
    "   "
    "   "
    "   "
    "P4B"
    "P4Y"
    "P4S"
    "P4T"
    "P4U"
    "P4D"
    "P4L"
    "P4R"
    "P4A"
    "P4X"
    "P4L"
    "P4R"
    "   "
    "   "
    "   "
    "   "
    "P5B"
    "P5Y"
    "P5S"
    "P5T"
    "P5U"
    "P5D"
    "P5L"
    "P5R"
    "P5A"
    "P5X"
    "P5L"
    "P5R"
    "   "
    "   "
    "   "
    "   "
};
