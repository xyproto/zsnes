#include <string.h>

#include "asm_call.h"
#include "c_init.h"
#include "c_intrf.h"
#include "cfg.h"
#include "cpu/c_execute.h"
#include "cpu/c_regs.h"
#include "cpu/c_regsw.h"
#include "cpu/c_table.h"
#include "cpu/c_tablec.h"
#include "cpu/execute.h"
#include "cpu/regs.h"
#include "cpu/c_stable.h"
#include "debugger.h"
#include "gui/c_gui.h"
#include "gui/c_guiwindp.h"
#include "gui/gui.h"
#include "gui/guikeys.h"
#include "gui/guiwindp.h"
#include "init.h"
#include "initc.h"
#include "input.h"
#include "link.h"
#include "macros.h"
#include "ui.h"
#include "video/c_mode716.h"
#include "video/procvid.h"
#include "zmovie.h"
#include "zpath.h"
#include "zstate.h"

u1 ComboCounter;
u1 MMXSupport;
u1 ReturnFromSPCStall;
u1 SPCStallSetting;
u1 WhichSW;
u4 JoyANow;
u4 JoyAOrig;
u4 JoyBNow;
u4 JoyBOrig;
u4 JoyCNow;
u4 JoyCOrig;
u4 JoyDNow;
u4 JoyDOrig;
u4 JoyENow;
u4 JoyEOrig;
u4 numspcvblleft;
u4 spc700idle;

static u1 ComboProg[5];
static u1 ComboPtr[5];
static u1 TurboSw;
static u1* StartComb[5];
static u4 CombDelay[5];
static u4 CombDirSwap;
static u4 HoldComb[5];
static u4 PressComb[5];
static u4 const CombContDatN[] = { 0x08000000, 0x04000000, 0x02000000, 0x01000000, 0x00800000, 0x80000000, 0x00400000, 0x40000000, 0x00200000, 0x00100000, 0x10000000, 0x20000000 };
static u4 const CombContDatR[] = { 0x08000000, 0x04000000, 0x01000000, 0x02000000, 0x00800000, 0x80000000, 0x00400000, 0x40000000, 0x00200000, 0x00100000, 0x10000000, 0x20000000 };
static u4 const CombTDelN[] = { 1, 2, 3, 4, 5, 9, 30, 60, 120, 180, 240, 300 };
static u4 const CombTDelP[] = { 1, 2, 3, 4, 5, 9, 25, 50, 100, 150, 200, 250 };
static u4 const* CombCont[5];

void init(void)
{
    // Initialize snow stuff
    for (u4 i = 0; i != 400; ++i) // XXX Why only 400 of 800?
    {
        SnowData[i] <<= 8;
        SnowVelDist[i] &= 0xF7;
        if (SnowTimer == 0)
            SnowVelDist[i] |= 0x08;
    }

    BackupSystemVars();

    forceromtype = romtype;
    romtype = 0;

    /* XXX sndrot is a global variable, but is treated like the first entry of a
     * big struct */
    memcpy(regsbackup, &sndrot, sizeof(sndrot));

    clearmem();

    inittable();
    inittablec();
    SA1inittable();

    // SPC Init
    procexecloop();

    // SNES Init
    Setper2exec();

    Makemode7Table();

    if (ZCartName[0] != '\0' || romloadskip != 1) {
        romloadskip = 0;
        loadfileGUI();
        SetupROM();
        if (DisplayInfo != 0)
            showinfogui();
    }

    UpdateDevices();
    init65816();
    initregr();
    initregw();
    initsnes();

    u4 const vol = MusicRelVol * 128 * 0xA3D70A3DULL >> 38;
    MusicVol = vol < 127 ? vol : 127;

    if (AutoState == 1)
        LoadSecondState();

    // FIX STATMAT
    // Here's the auto-load ZST file stuff
    if (autoloadstate >= 1) {
        current_zst = autoloadstate - 1;
        // Load the specified state file
        loadstate2();
    }

    if (1 <= autoloadmovie && autoloadmovie <= 10) {
        CMovieExt = autoloadmovie != 1 ? '0' + (autoloadmovie - 1) : 'v';

        if (ZMVRawDump != 0) {
            MovieDumpRaw();
        } else {
            MoviePlay();
        }
    }

    if (yesoutofmemory == 1)
        outofmemfix();

#ifndef NO_DEBUGGER
    if (debugger != 0 && romloadskip != 1) {
        /* Prevent nasty hang in debugger. Likely not a good way...
         * If we don't do this, then on the SDL and win32 ports, update_ticks_pc2
         * won't be set and CheckTimers will hang. */

        /* Most likely it isn't desirable to be checking timers under the
         * debugger anyway, but this is a much simpler fix. */
#ifdef __WIN32__
        // need to get "freq" set first
        initwinvideo();
#endif
        Start60HZ();
        startdebugger();
    } else
#endif
    {
        start65816();
    }
}

static inline void PlayerDeviceHelp(u4 const key, u4* const device, u4 const bits)
{
    if (pressed[key] == 1)
        *device |= bits;
}

static void ProcSNESMouse(u4* const device)
{
    u4 d = *device;
    if (mousebuttons & 0x02)
        d |= 0x00100000;
    if (mousebuttons & 0x01)
        d |= 0x00400000;
    d = d & 0xFFFF0000 | 0x00010000 | (mouseypos & 0x7F) << 8 | (mousexpos & 0x7F);
    if (mouseydir & 0x01)
        d |= 0x00008000;
    if (mousexdir & 0x01)
        d |= 0x00000080;
    *device = d;
}

static u4 ProcessCombo(u4 const i)
{
    u4 res = 1;
    u1* eax = StartComb[i];
    if (CombDelay[i] == 0) {
        u4 KeyLPress = 0;
        for (;;) {
            u4 ebx = *eax;
            if (ebx == 0)
                goto finish;
            if (ebx < 37) {
                if (KeyLPress == 0) {
                    PressComb[i] = 0;
                    KeyLPress = 1;
                }
                if (--ebx < 12) {
                    PressComb[i] |= CombCont[i][ebx];
                } else if ((ebx -= 12) < 12) {
                    HoldComb[i] |= CombCont[i][ebx];
                } else {
                    ebx -= 12; // <- bugfix from Maxim
                    u4 const x = ~CombCont[i][ebx];
                    HoldComb[i] &= x;
                    PressComb[i] &= x; // <- buxfix from Maxim
                }
                ++eax;
                if (++ComboPtr[i] == 42)
                    goto finish;
            } else {
                if (ebx > 48)
                    goto finish;
                CombDelay[i] = (romispal != 0 ? CombTDelP : CombTDelN)[ebx - 37];
                ++eax;
                if (++ComboPtr[i] == 42)
                    goto finish;
                break;
            }
        }
    }
    --CombDelay[i];
    res = 0;
finish:
    StartComb[i] = eax;
    return res;
}

static void ProcessKeyComb(u4 const id, u4* const device)
{
    if (NumCombo == 0)
        return;
    if (ComboProg[id] == 0) {
        if (*device & 0x01000000)
            CombDirSwap = 0;
        if (*device & 0x02000000)
            CombDirSwap = 1;
        ComboData* eax = GUIComboGameSpec != 0 ? CombinDataLocl : CombinDataGlob;
        u4 n = NumCombo;
        for (;; ++eax) {
            u2 const key = eax->key;
            if (pressed[key] == 1 && eax->player == id) {
                pressed[key] = 2;
                ++ComboCounter;
                ComboProg[id] = 1;
                ComboPtr[id] = 0;
                PressComb[id] = 0;
                HoldComb[id] = 0;
                CombCont[id] = CombDirSwap != 0 && eax->ff != 0 ? CombContDatR : CombContDatN;
                StartComb[id] = eax->combo;
                break;
            }
            if (--n == 0)
                return;
        }
    }
    if (ProcessCombo(id) != 0)
        ComboProg[id] = 0;
    *device = HoldComb[id] | PressComb[id];
}

static void PlayerDeviceFix(u4* const device)
{
    if (AllowUDLR == 1)
        return;
    u4 d = *device;
    if (d & 0x0C000000 == 0x0C000000)
        d &= 0xF3FFFFFF;
    if (d & 0x03000000 == 0x03000000)
        d &= 0xFCFFFFFF;
    *device = d;
}

// Reads from Keyboard, etc.
void ReadInputDevice(void)
{
    WhichSW = 1;
    ++TurboSw;
    u1 const TurboCB = Turbo30hz != 0 ? 0x02 : 0x01;
    // Read External Devices (Joystick, PPort, etc.)
    JoyRead();
    // Process Data
    JoyAOrig = 0;
    JoyBOrig = 0;

    // Get Player1 input device
    if (device1 == 1) {
        processmouse1();
        ProcSNESMouse(&JoyAOrig);
    } else {
        PlayerDeviceHelp(pl1Bk, &JoyAOrig, 0x80000000);
        PlayerDeviceHelp(pl1Yk, &JoyAOrig, 0x40000000);
        PlayerDeviceHelp(pl1selk, &JoyAOrig, 0x20000000);
        PlayerDeviceHelp(pl1startk, &JoyAOrig, 0x10000000);
        PlayerDeviceHelp(pl1upk, &JoyAOrig, 0x08000000);
        PlayerDeviceHelp(pl1downk, &JoyAOrig, 0x04000000);
        PlayerDeviceHelp(pl1leftk, &JoyAOrig, 0x02000000);
        PlayerDeviceHelp(pl1rightk, &JoyAOrig, 0x01000000);
        PlayerDeviceHelp(pl1Ak, &JoyAOrig, 0x00800000);
        PlayerDeviceHelp(pl1Xk, &JoyAOrig, 0x00400000);
        PlayerDeviceHelp(pl1Lk, &JoyAOrig, 0x00200000);
        PlayerDeviceHelp(pl1Rk, &JoyAOrig, 0x00100000);
        PlayerDeviceHelp(pl1ULk, &JoyAOrig, 0x0A000000);
        PlayerDeviceHelp(pl1URk, &JoyAOrig, 0x09000000);
        PlayerDeviceHelp(pl1DLk, &JoyAOrig, 0x06000000);
        PlayerDeviceHelp(pl1DRk, &JoyAOrig, 0x05000000);
        PlayerDeviceFix(&JoyAOrig);
        if (!(TurboSw & TurboCB)) {
            PlayerDeviceHelp(pl1Xtk, &JoyAOrig, 0x00400000);
            PlayerDeviceHelp(pl1Ytk, &JoyAOrig, 0x40000000);
            PlayerDeviceHelp(pl1Atk, &JoyAOrig, 0x00800000);
            PlayerDeviceHelp(pl1Btk, &JoyAOrig, 0x80000000);
            PlayerDeviceHelp(pl1Ltk, &JoyAOrig, 0x00200000);
            PlayerDeviceHelp(pl1Rtk, &JoyAOrig, 0x00100000);
        }
        ComboCounter = 0;
        ProcessKeyComb(0, &JoyAOrig);
        JoyAOrig |= 0x00008000; // Joystick Enable
        if (GUIDelayB == 1) {
            if (JoyAOrig & 0x80000000)
                goto inputbdcb;
            --GUIDelayB;
        } else if (GUIDelayB != 0) {
            --GUIDelayB;
        inputbdcb:
            JoyAOrig &= 0x7FFFFFFF;
        }
    }

    if (device2 == 1) {
        processmouse2();
        ProcSNESMouse(&JoyBOrig);
    } else if (device2 == 2) {
        processmouse2();
        u4 j = JoyBOrig & 0x0000FFFF | 0x00FF0000 | ssautosw << 24;
        if (mousebuttons & 0x01)
            j |= 0x80000000;
        if (pressed[SSPause] != 0)
            j |= 0x10000000;
        if (mousebuttons & 0x02)
            j |= 0x40000000;
        JoyBOrig = j;
    } else if (device2 == 3) {
        processmouse2();
        if (*(u4*)(romdata + 0x1000) == 0xAD20C203) {
            u1* eax = wramdata;
            if (eax[0] != 26) {
                eax[0x040A] = mousexloc;
                eax[0x040E] = mouseyloc;
            }
        }
        // JoyBOrig = JoyBOrig & 0x0000FFFF | 0x000E0000;
        if (mousebuttons & 0x01)
            JoyAOrig |= 0x80000000;
        if (mousebuttons & 0x02)
            JoyAOrig |= 0x00800000;
    } else if (pl2contrl != 0) { // Get Player2 input device
        PlayerDeviceHelp(pl2Bk, &JoyBOrig, 0x80000000);
        PlayerDeviceHelp(pl2Yk, &JoyBOrig, 0x40000000);
        PlayerDeviceHelp(pl2selk, &JoyBOrig, 0x20000000);
        PlayerDeviceHelp(pl2startk, &JoyBOrig, 0x10000000);
        PlayerDeviceHelp(pl2upk, &JoyBOrig, 0x08000000);
        PlayerDeviceHelp(pl2downk, &JoyBOrig, 0x04000000);
        PlayerDeviceHelp(pl2leftk, &JoyBOrig, 0x02000000);
        PlayerDeviceHelp(pl2rightk, &JoyBOrig, 0x01000000);
        PlayerDeviceHelp(pl2Ak, &JoyBOrig, 0x00800000);
        PlayerDeviceHelp(pl2Xk, &JoyBOrig, 0x00400000);
        PlayerDeviceHelp(pl2Lk, &JoyBOrig, 0x00200000);
        PlayerDeviceHelp(pl2Rk, &JoyBOrig, 0x00100000);
        PlayerDeviceHelp(pl2ULk, &JoyBOrig, 0x0A000000);
        PlayerDeviceHelp(pl2URk, &JoyBOrig, 0x09000000);
        PlayerDeviceHelp(pl2DLk, &JoyBOrig, 0x06000000);
        PlayerDeviceHelp(pl2DRk, &JoyBOrig, 0x05000000);
        PlayerDeviceFix(&JoyBOrig);
        if (!(TurboSw & TurboCB)) {
            PlayerDeviceHelp(pl2Xtk, &JoyBOrig, 0x00400000);
            PlayerDeviceHelp(pl2Ytk, &JoyBOrig, 0x40000000);
            PlayerDeviceHelp(pl2Atk, &JoyBOrig, 0x00800000);
            PlayerDeviceHelp(pl2Btk, &JoyBOrig, 0x80000000);
            PlayerDeviceHelp(pl2Ltk, &JoyBOrig, 0x00200000);
            PlayerDeviceHelp(pl2Rtk, &JoyBOrig, 0x00100000);
        }
        ProcessKeyComb(1, &JoyBOrig);
        JoyBOrig |= 0x00008000; // Joystick Enable
    }

    JoyCOrig = 0;
    if (pl3contrl != 0) { // Get Player3 input device
        PlayerDeviceHelp(pl3Bk, &JoyCOrig, 0x80000000);
        PlayerDeviceHelp(pl3Yk, &JoyCOrig, 0x40000000);
        PlayerDeviceHelp(pl3selk, &JoyCOrig, 0x20000000);
        PlayerDeviceHelp(pl3startk, &JoyCOrig, 0x10000000);
        PlayerDeviceHelp(pl3upk, &JoyCOrig, 0x08000000);
        PlayerDeviceHelp(pl3downk, &JoyCOrig, 0x04000000);
        PlayerDeviceHelp(pl3leftk, &JoyCOrig, 0x02000000);
        PlayerDeviceHelp(pl3rightk, &JoyCOrig, 0x01000000);
        PlayerDeviceHelp(pl3Ak, &JoyCOrig, 0x00800000);
        PlayerDeviceHelp(pl3Xk, &JoyCOrig, 0x00400000);
        PlayerDeviceHelp(pl3Lk, &JoyCOrig, 0x00200000);
        PlayerDeviceHelp(pl3Rk, &JoyCOrig, 0x00100000);
        PlayerDeviceHelp(pl3ULk, &JoyCOrig, 0x0A000000);
        PlayerDeviceHelp(pl3URk, &JoyCOrig, 0x09000000);
        PlayerDeviceHelp(pl3DLk, &JoyCOrig, 0x06000000);
        PlayerDeviceHelp(pl3DRk, &JoyCOrig, 0x05000000);
        PlayerDeviceFix(&JoyCOrig);
        if (!(TurboSw & TurboCB)) {
            PlayerDeviceHelp(pl3Xtk, &JoyCOrig, 0x00400000);
            PlayerDeviceHelp(pl3Ytk, &JoyCOrig, 0x40000000);
            PlayerDeviceHelp(pl3Atk, &JoyCOrig, 0x00800000);
            PlayerDeviceHelp(pl3Btk, &JoyCOrig, 0x80000000);
            PlayerDeviceHelp(pl3Ltk, &JoyCOrig, 0x00200000);
            PlayerDeviceHelp(pl3Rtk, &JoyCOrig, 0x00100000);
        }
        ProcessKeyComb(2, &JoyCOrig);
        JoyCOrig |= 0x00008000; // Joystick Enable
    }

    JoyDOrig = 0;
    if (pl4contrl != 0) { // Get Player4 input device
        PlayerDeviceHelp(pl4Bk, &JoyDOrig, 0x80000000);
        PlayerDeviceHelp(pl4Yk, &JoyDOrig, 0x40000000);
        PlayerDeviceHelp(pl4selk, &JoyDOrig, 0x20000000);
        PlayerDeviceHelp(pl4startk, &JoyDOrig, 0x10000000);
        PlayerDeviceHelp(pl4upk, &JoyDOrig, 0x08000000);
        PlayerDeviceHelp(pl4downk, &JoyDOrig, 0x04000000);
        PlayerDeviceHelp(pl4leftk, &JoyDOrig, 0x02000000);
        PlayerDeviceHelp(pl4rightk, &JoyDOrig, 0x01000000);
        PlayerDeviceHelp(pl4Ak, &JoyDOrig, 0x00800000);
        PlayerDeviceHelp(pl4Xk, &JoyDOrig, 0x00400000);
        PlayerDeviceHelp(pl4Lk, &JoyDOrig, 0x00200000);
        PlayerDeviceHelp(pl4Rk, &JoyDOrig, 0x00100000);
        PlayerDeviceHelp(pl4ULk, &JoyDOrig, 0x0A000000);
        PlayerDeviceHelp(pl4URk, &JoyDOrig, 0x09000000);
        PlayerDeviceHelp(pl4DLk, &JoyDOrig, 0x06000000);
        PlayerDeviceHelp(pl4DRk, &JoyDOrig, 0x05000000);
        PlayerDeviceFix(&JoyDOrig);
        if (!(TurboSw & TurboCB)) {
            PlayerDeviceHelp(pl4Xtk, &JoyDOrig, 0x00400000);
            PlayerDeviceHelp(pl4Ytk, &JoyDOrig, 0x40000000);
            PlayerDeviceHelp(pl4Atk, &JoyDOrig, 0x00800000);
            PlayerDeviceHelp(pl4Btk, &JoyDOrig, 0x80000000);
            PlayerDeviceHelp(pl4Ltk, &JoyDOrig, 0x00200000);
            PlayerDeviceHelp(pl4Rtk, &JoyDOrig, 0x00100000);
        }
        ProcessKeyComb(3, &JoyDOrig);
        JoyDOrig |= 0x00008000; // Joystick Enable
    }

    JoyEOrig = 0;
    if (pl5contrl != 0) { // Get Player5 input device
        PlayerDeviceHelp(pl5Bk, &JoyEOrig, 0x80000000);
        PlayerDeviceHelp(pl5Yk, &JoyEOrig, 0x40000000);
        PlayerDeviceHelp(pl5selk, &JoyEOrig, 0x20000000);
        PlayerDeviceHelp(pl5startk, &JoyEOrig, 0x10000000);
        PlayerDeviceHelp(pl5upk, &JoyEOrig, 0x08000000);
        PlayerDeviceHelp(pl5downk, &JoyEOrig, 0x04000000);
        PlayerDeviceHelp(pl5leftk, &JoyEOrig, 0x02000000);
        PlayerDeviceHelp(pl5rightk, &JoyEOrig, 0x01000000);
        PlayerDeviceHelp(pl5Ak, &JoyEOrig, 0x00800000);
        PlayerDeviceHelp(pl5Xk, &JoyEOrig, 0x00400000);
        PlayerDeviceHelp(pl5Lk, &JoyEOrig, 0x00200000);
        PlayerDeviceHelp(pl5Rk, &JoyEOrig, 0x00100000);
        PlayerDeviceHelp(pl5ULk, &JoyEOrig, 0x0A000000);
        PlayerDeviceHelp(pl5URk, &JoyEOrig, 0x09000000);
        PlayerDeviceHelp(pl5DLk, &JoyEOrig, 0x06000000);
        PlayerDeviceHelp(pl5DRk, &JoyEOrig, 0x05000000);
        PlayerDeviceFix(&JoyEOrig);
        if (!(TurboSw & TurboCB)) {
            PlayerDeviceHelp(pl5Xtk, &JoyEOrig, 0x00400000);
            PlayerDeviceHelp(pl5Ytk, &JoyEOrig, 0x40000000);
            PlayerDeviceHelp(pl5Atk, &JoyEOrig, 0x00800000);
            PlayerDeviceHelp(pl5Btk, &JoyEOrig, 0x80000000);
            PlayerDeviceHelp(pl5Ltk, &JoyEOrig, 0x00200000);
            PlayerDeviceHelp(pl5Rtk, &JoyEOrig, 0x00100000);
        }
        ProcessKeyComb(4, &JoyEOrig);
        JoyEOrig |= 0x00008000; // Joystick Enable
    }

    if (pl12s34 == 1) {
        if (device1 == 0)
            JoyAOrig = JoyCOrig;
        if (device2 == 0)
            JoyBOrig = JoyDOrig;
    }
}

// Terminate Program
void DosExit(void)
{
    if (AutoState == 1)
        SaveSecondState();
    zexit();
}

void MMXCheck(void)
{ // Check for cpu that doesn't support CPUID
    ShowMMXSupport = 0;
    MMXSupport = 0;

    // Real way to check for presence of CPUID instruction  -kode54
    u4 eflags_before;
    u4 eflags_after;
    asm(
        "pushf\n\t"
        "popl  %0\n\t"
        "movl  %0, %1\n\t"
        "xorl  $0x00200000, %1\n\t"
        "pushl %1\n\t"
        "popf\n\t"
        "pushf\n\t"
        "popl  %1\n\t"
        : "=r"(eflags_before), "=r"(eflags_after)::"cc");
    if (eflags_before == eflags_after)
        return; // No CPUID, so no MMX either

    u4 eax, ebx, ecx, edx;

    asm("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1));
    if (!(edx & 0x00800000))
        return; // No MMX

    ShowMMXSupport = 1;
    MMXSupport = AllowMMX;
}

void outofmemfix(void)
{
    u1* rom = romdata;
    if (romtype == 2)
        rom += 0x8000; // hirom
    rom[0] = 0x58;
    rom[1] = 0x80;
    rom[2] = 0xFE;
    resetv = 0x8000;
    xpc = 0x8000;

    Msgptr = newgfx16b != 1 ? "OUT OF MEMORY." : "ROM IS TOO BIG.";
    MessageOn = 0xFFFFFFFF;
}

void idledetectspc(void)
{
    ++numspcvblleft;
    if (SPCStallSetting < 2) {
        ++SPCStallSetting;
        ReturnFromSPCStall = 1;
    } else {
        spc700idle = 29;
        Msgptr = "SPC700 STALL DETECTED.";
        MessageOn = MsgCount;
    }
}

void printhex(u2 ax)
{
    u4 n = 4;
    do
        PrintChar("0123456789ABCDEF"[ax >> 12]);
    while (ax <<= 4, --n != 0);
}
