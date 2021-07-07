#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "asm_call.h"
#include "c_init.h"
#include "c_intrf.h"
#include "c_vcache.h"
#include "cfg.h"
#include "cpu/dsp.h"
#include "cpu/dspproc.h"
#include "cpu/execute.h"
#include "cpu/regs.h"
#include "endmem.h"
#include "gblvars.h"
#include "gui/c_gui.h"
#include "gui/gui.h"
#include "gui/menu.h"
#include "initc.h"
#include "input.h"
#include "macros.h"
#include "ui.h"
#include "vcache.h"
#include "video/makevid.h"
#include "video/newgfx.h"
#include "video/newgfx16.h"
#include "video/procvid.h"
#include "zmovie.h"
#include "zstate.h"

u1 SloMo;
u1 curblank = 0x80;
u1 curcolbg[4];
u1 hiresstuff;
u1 osm2dis;
u1* colormodeofs;
u2 curbgofs[4];
u4 CSprWinPtr;
u4 sramb4save;

u1 colormodedef[][4] = {
    { 1, 1, 1, 1 },
    { 2, 2, 1, 0 },
    { 2, 2, 0, 0 },
    { 3, 2, 0, 0 },
    { 3, 1, 0, 0 },
    { 2, 1, 0, 0 },
    { 2, 0, 0, 0 },
    { 0, 0, 0, 0 }
};

static u1 FastForwardLock;
static u1 SlowDownLock;
static u1 fskipped;

static char sselm[] = "STATE SLOT  0 SELECTED";

static void SetMessage(char const* const msg)
{
    Msgptr = msg;
    MessageOn = MsgCount;
}

static void UpdateVolume(void)
{
    u4 const vol = MusicRelVol * 128 * 0xA3D70A3DULL >> 38;
    MusicVol = vol < 127 ? vol : 127;

    asm volatile("call %P0" ::"X"(WDSPReg0C), "a"(DSPMem[0x0C])
                 : "cc", "memory");
    asm volatile("call %P0" ::"X"(WDSPReg1C), "a"(DSPMem[0x1C])
                 : "cc", "memory");

    static char vollv[] = "VOLUME LEVEL :    ";
    sprintf(vollv + 15, "%-3d", MusicRelVol);
    SetMessage(vollv);
}

static inline bool TestKey2(u4 const key)
{
    bool const res = pressed[key] & 1;
    if (res)
        pressed[key] = 2;
    return res;
}

static void ToggleLayer(u4 const layer)
{
    scrndis ^= 1 << layer;
    static char msg[19];
    sprintf(msg, "BG%c LAYER %sABLED", '1' + layer, scrndis & 1 << layer ? "DIS" : "EN");
    SetMessage(msg);
}

static void stateselcomp(u4 const* const key, u1 const slot_x)
{
    if (!TestKey2(*key))
        return;

    u4 const slot = current_zst / 10 * 10 + slot_x;
    current_zst = slot;
    sprintf(sselm + 11, "%02d", slot);
    SetMessage(sselm);
}

static void soundselcomp(u4 const* const key, u1* const disable, u1* const status, char const chan_id)
{
    if (!TestKey2(*key))
        return;

    static char sndchena[] = "SOUND CH   ENABLED";
    static char sndchdis[] = "SOUND CH   DISABLED";

    *disable ^= 0x01;
    *status = 0;
    sndchena[9] = chan_id;
    sndchdis[9] = chan_id;
    SetMessage(*disable & 0x01 ? sndchena : sndchdis);
}

static void cycleinputdevicemsg(void)
{
    char const* dev1;
    switch (device1) {
    case 0:
        dev1 = "GAMEPAD";
        break;
    default:
        dev1 = "MOUSE";
        break;
    }

    char const* dev2;
    switch (device2) {
    default:
        dev2 = "GAMEPAD";
        break;
    case 1:
        dev2 = "MOUSE";
        break;
    case 2:
        dev2 = "SUPER SCOPE";
        break;
    case 3:
        dev2 = "1 JUSTIFIER";
        break;
    case 4:
        dev2 = "2 JUSTIFIERS";
        break;
    }

    static char msg[30];
    sprintf(msg, "P1: %-7.7s  P2: %-12.12s", dev1, dev2);
    SetMessage(msg);
    Get_MousePositionDisplacement();
}

static void cycleinputs(bool const input1, bool const input2)
{
    if (input2) {
        if (cycleinputdevice2() && input1)
            cycleinputdevice1();
        if (device2 == 2) {
            mousexloc = 128;
            mouseyloc = 112;
        }
    } else if (input1) {
        cycleinputdevice1();
    }

    cycleinputdevicemsg();
    Get_MousePositionDisplacement();
}

static void docache(void)
{
    u1 const bg = bgmode;
    colormodeofs = colormodedef[bg];
    curcolbg[0] = colormodedef[bg][0];
    curcolbg[1] = colormodedef[bg][1];
    curcolbg[2] = colormodedef[bg][0];
    curcolbg[3] = colormodedef[bg][1];
    curbgofs[0] = bg1ptr[0];
    curbgofs[1] = bg1ptr[1];
    curbgofs[2] = bg1ptr[2];
    curbgofs[3] = bg1ptr[3];

    // clear # of sprites & bg cache
    memset(cachebg, 0, sizeof(cachebg));
    memset(sprlefttot, 0, sizeof(sprlefttot));
    memset(sprleftpr, 0, sizeof(sprleftpr));
    memset(sprleftpr1, 0, sizeof(sprleftpr1));
    memset(sprleftpr2, 0, sizeof(sprleftpr2));
    memset(sprleftpr3, 0, sizeof(sprleftpr3));
    memset(sprcnt, 0, sizeof(sprcnt));
    memset(sprstart, 0, sizeof(sprstart));
    memset(sprtilecnt, 0, sizeof(sprtilecnt));
    memset(sprend, 0, sizeof(sprend));
    memset(sprendx, 0, sizeof(sprendx));

    // do sprites
    if (!(scrndis & 0x10)) {
        asm_call(cachesprites);
        asm_call(processsprites);
    }
}

void cachevideo(void)
{
    NextLineCache = 0;
    objwlrpos[0] = 0xFFFFFFFF;
    CSprWinPtr = 0;
    pressed[0] = 0;
    bgcmsung = 0;
    modeused[0] = 0;
    modeused[1] = 0;
    ngmsdraw = 0;
    ngextbg = 0;
    hiresstuff = 0;
    Mode7HiRes = 0;

    scfbl = 1;
    maxbr = vidbright;
    cgmod = 1;
    curblank = 0;

#ifndef NO_DEBUGGER
    if (debuggeron != 0)
#else
    if (SPCSave == 1)
#endif
    {
        curblank = 0x40;
        goto nofrskip;
    }

    if (sramb4save != 0) {
        if (SRAMSave5Sec == 0) {
            sramb4save = 0;
        } else if (--sramb4save == 1) {
            SaveSramData();
        }
    }

    u1 bl;
    // if emulation paused, don't alter timing
    u2 ax = 1;
    if (EMUPause != 1) {
        // fast forward goes over all other throttles
        // don't fast forward while dumping a movie
        if (RawDumpInProgress != 1) {
            if (FastFwdToggle == 0) {
                if (pressed[KeyFastFrwrd] & 1)
                    goto fastfor;
            } else {
                if (TestKey2(KeyFastFrwrd))
                    FastForwardLock ^= 1;
                if (FastForwardLock == 1) {
                fastfor:
                    bl = FFRatio + 1; // 1-29, 2x-30x fastmotion
                    goto fastforb;
                }
            }
        }
        // next up, check for slowdown
        if (FastFwdToggle == 0) {
            if (pressed[KeySlowDown] & 1)
                goto slowdwn;
        } else {
            if (TestKey2(KeySlowDown))
                SlowDownLock ^= 1;
            if (SlowDownLock == 1) {
            slowdwn:
                SloMo = SDRatio + 1; // 1-29 -> /2-/30 slowmotion
                goto throttleskip;
            }
        }
        // now we can look at emuspeed
        if (EmuSpeed >= 30) // 0-28 slow, 29 normal, 30-58 skip
        {
            bl = EmuSpeed - 29; // 30-58 -> 1-29 frames to skip, 2x-30x speed
            goto fastforb;
        }
        SloMo = 29 - EmuSpeed; // 0-29 -> repeat 29-0 times, /30-1x speed
    throttleskip:
        ax = SloMo + 1; // total times frame is drawn
    }

    if (frameskip == 0) {
        while (t1cc < ax)
            Check60hz();
        t1cc -= ax;
        if (t1cc >= ax) {
            curblank = 0x40;
            if (++fskipped <= maxskip)
                goto nofrskip;
            t1cc = 0;
            curblank = 0;
        }
        fskipped = 0;
        goto nofrskip;
    } else {
        bl = frameskip;
    }
fastforb:
    if (++frskipper < bl) {
        curblank = 0x40;
    } else {
    nofrskip:
        frskipper = 0;
    }

    if (MouseDis != 1 && GUIRClick != 0 && device1 == 0 && device2 == 0) {
        u4 buttons = Get_MouseData() & 0xFFFF;
        if (lhguimouse == 1)
            buttons = SwapMouseButtons(buttons);
        if (buttons & 0x02) {
            if (MousePRClick != 0)
                goto noclick;
            pressed[1] = 1;
        }
        MousePRClick = 0;
    noclick:;
    }

    // disable all necessary backgrounds
    if (TestKey2(KeyBGDisble0))
        ToggleLayer(0);
    if (TestKey2(KeyBGDisble1))
        ToggleLayer(1);
    if (TestKey2(KeyBGDisble2))
        ToggleLayer(2);
    if (TestKey2(KeyBGDisble3))
        ToggleLayer(3);

    if (TestKey2(KeySprDisble)) {
        scrndis ^= 0x10;
        SetMessage(scrndis & 0x10 ? "SPRITE LAYER DISABLED" : "SPRITE LAYER ENABLED");
    }

    if (TestKey2(KeyEmuSpeedDown) && EmuSpeed != 0)
        --EmuSpeed;
    if (TestKey2(KeyEmuSpeedUp) && EmuSpeed != 58)
        ++EmuSpeed;
    if (TestKey2(KeyResetSpeed))
        EmuSpeed = 29;

    if (TestKey2(KeyResetAll)) {
        memset(&Voice0Disable, 1, sizeof(Voice0Disable));
        scrndis = 0;
        disableeffects = 0;
        osm2dis = 0;
        EmuSpeed = 29;
        device1 = snesinputdefault1;
        device2 = snesinputdefault2;
        SetMessage("ALL SWITCHES NORMAL");
    }

    if (TestKey2(KeyRTRCycle)) {
        char const *const msg = ++MZTForceRTR == 3 ? MZTForceRTR = 0, "LOAD MZT MODE - OFF" : MZTForceRTR == 1 ? "LOAD MZT MODE - RECORD"
                                                                                                               : "LOAD MZT MODE - REPLAY";
        SetMessage(msg);
    }

    if (TestKey2(KeyExtraEnab1))
        cycleinputs(true, false);
    if (TestKey2(KeyExtraEnab2))
        cycleinputs(false, true);
    if (TestKey2(KeyExtraRotate))
        cycleinputs(true, true);

    if (TestKey2(KeyNewGfxSwt)) {
        prevbright = 16;
        newengen ^= 1;
        SetMessage(newengen == 1 ? "NEW GFX ENGINE ENABLED" : "NEW GFX ENGINE DISABLED");
        memset(vidmemch2, 0x01, 4096);
        memset(vidmemch4, 0x01, 4096);
        memset(vidmemch8, 0x01, 4096);
        memset(pal16b, 0, sizeof(pal16b));
        memset(prevpal, 0, sizeof(prevpal));
        u4 const val = newengen == 1 ? 0xFFFFFFFF : 0x0000FFFF;
        for (u4* i = pal16bxcl; i != endof(pal16bxcl); ++i)
            *i = val;
        genfulladdtab();
    }

    if (TestKey2(KeyWinDisble)) {
        disableeffects ^= 1;
        SetMessage(disableeffects != 1 ? "WINDOWING ENABLED" : "WINDOWING DISABLED");
    }

    if (TestKey2(KeyOffsetMSw)) {
        osm2dis ^= 1;
        SetMessage(osm2dis != 1 ? "OFFSET MODE ENABLED" : "OFFSET MODE DISABLED");
    }

    if (pressed[KeyVolUp] & 1 && MusicRelVol < 100) {
        ++MusicRelVol;
        UpdateVolume();
    }

    if (pressed[KeyVolDown] & 1 && MusicRelVol != 0) {
        --MusicRelVol;
        UpdateVolume();
    }

    if (TestKey2(KeyFRateUp) && frameskip != 10) {
        FPSOn = 0;
        ++frameskip;
        goto show_frameskip;
    }

    if (TestKey2(KeyFRateDown) && frameskip != 0) {
        char const* msg;
        if (--frameskip != 0) {
            static char frlev[] = "FRAME SKIP SET TO  ";
        show_frameskip:
            frlev[18] = '0' + frameskip - 1;
            msg = frlev;
        } else {
            t1cc = 0;
            msg = "AUTO FRAMERATE ENABLED";
        }
        SetMessage(msg);
    }

    if (TestKey2(KeyDisplayBatt))
        DisplayBatteryStatus();

    if (TestKey2(KeyIncreaseGamma) && gammalevel < 15) {
        ++gammalevel;
        goto show_gamma;
    }

    if (TestKey2(KeyDecreaseGamma)) {
        if (gammalevel != 0) {
            --gammalevel;
        show_gamma:;
            u1 const al = gammalevel;
            gammalevel16b = al / 2;
            static char gammamsg[] = "GAMMA LEVEL:   ";
            sprintf(gammamsg + 13, "%2d", al);
            SetMessage(gammamsg);
        }
    }

    if (TestKey2(KeyDisplayFPS) && frameskip == 0)
        FPSOn ^= 1;

    // do state selects
    stateselcomp(&KeyStateSlc0, 0);
    stateselcomp(&KeyStateSlc1, 1);
    stateselcomp(&KeyStateSlc2, 2);
    stateselcomp(&KeyStateSlc3, 3);
    stateselcomp(&KeyStateSlc4, 4);
    stateselcomp(&KeyStateSlc5, 5);
    stateselcomp(&KeyStateSlc6, 6);
    stateselcomp(&KeyStateSlc7, 7);
    stateselcomp(&KeyStateSlc8, 8);
    stateselcomp(&KeyStateSlc9, 9);
    if (TestKey2(KeyStateSlc0)) { // XXX huh?
        sselm[11] = '0';
        SetMessage(sselm);
    }

    if (TestKey2(KeyIncStateSlot)) {
        u4 cur = current_zst + 1;
        if (cur == 100)
            cur = 0;
        current_zst = cur;
        sprintf(sselm + 11, "%02d", cur);
        SetMessage(sselm);
    }

    if (TestKey2(KeyDecStateSlot)) {
        u4 cur = current_zst;
        if (cur == 0)
            cur = 100;
        --cur;
        current_zst = cur;
        sprintf(sselm + 11, "%02d", cur);
        SetMessage(sselm);
    }

    if (TestKey2(KeyUsePlayer1234)) {
        pl12s34 ^= 1;
        SetMessage(pl12s34 == 1 ? "USE PLAYER 1/2 with 3/4 ON" : "USE PLAYER 1/2 with 3/4 OFF");
    }

    // do sound disables
    soundselcomp(&KeyDisableSC0, &Voice0Disable[0], &Voice0Status[0], '1');
    soundselcomp(&KeyDisableSC1, &Voice0Disable[1], &Voice0Status[1], '2');
    soundselcomp(&KeyDisableSC2, &Voice0Disable[2], &Voice0Status[2], '3');
    soundselcomp(&KeyDisableSC3, &Voice0Disable[3], &Voice0Status[3], '4');
    soundselcomp(&KeyDisableSC4, &Voice0Disable[4], &Voice0Status[4], '5');
    soundselcomp(&KeyDisableSC5, &Voice0Disable[5], &Voice0Status[5], '6');
    soundselcomp(&KeyDisableSC6, &Voice0Disable[6], &Voice0Status[6], '7');
    soundselcomp(&KeyDisableSC7, &Voice0Disable[7], &Voice0Status[7], '8');

    if (curblank == 0x00) {
        vidbuffer = vidbufferofsa;
        docache();
    }
}

#ifdef __MSDOS__
static void genfulladdtabred(void)
{
    // Write to buffer
    for (u4 i = 0; i != lengthof(fulladdtab); ++i) {
        u2 v = i;
        if (i & 0x4000)
            v = v & 0xBFFF | 0x3C00;
        if (i & 0x0200)
            v = v & 0xFDFF | 0x01E0;
        if (i & 0x0010)
            v = v & 0xFFEF | 0x000F;
        fulladdtab[i] = v << 1;
    }
}
#endif

void genfulladdtab(void)
{
    // Write to buffer
#ifdef __MSDOS__
    if (newengen == 1 && vesa2red10 != 0) {
        genfulladdtabred();
    } else
#endif
    {
        for (u4 i = 0; i != lengthof(fulladdtab); ++i) {
            u2 v = i;
            if (i & vesa2_rtrcl)
                v = v & vesa2_rtrcla | vesa2_rfull;
            if (i & vesa2_gtrcl)
                v = v & vesa2_gtrcla | vesa2_gfull;
            if (i & vesa2_btrcl)
                v = v & vesa2_btrcla | vesa2_bfull;
            fulladdtab[i] = v << 1;
        }
    }
}

void ConvertToAFormat(void)
{
    if (GUIOn != 1 && newengen != 0)
        return;

    u1* buf = vidbuffer + 16 * 2 + 288 * 2;
    u4 h = resolutn;
    if (MMXSupport == 1) {
        u4 w;
        u8 a;
        u8 b;
        asm volatile(
            "0:\n\t"
            "movl  $64, %2\n\t"
            "1:\n\t"
            "movq  (%0), %3\n\t"
            "movq  %3, %4\n\t"
            "pand  %5, %3\n\t"
            "pand  %6, %4\n\t"
            "psrlw $1, %3\n\t"
            "por   %4, %3\n\t"
            "movq  %3, (%0)\n\t"
            "addl  $8, %0\n\t"
            "decl  %2\n\t"
            "jnz   1b\n\t"
            "addl  $64, %0\n\t"
            "decl  %1\n\t"
            "jnz   0b\n\t"
            "emms"
            : "+r"(buf), "+r"(h), "=&r"(w), "=&y"(a), "=&y"(b)
            : "y"(0xFFC0FFC0FFC0FFC0ULL), "y"(0x001F001F001F001FULL)
            : "cc", "memory");
        (void)w;
        (void)a;
        (void)b;
    } else {
        u4* b = (u4*)buf;
        do {
            u4 w = 128;
            do {
                u4 const val = *b;
                *b++ = (val & 0xFFC0FFC0) >> 1 | val & 0x001F001F;
            } while (--w != 0);
            b += 16;
        } while (--h != 0);
    }
}
