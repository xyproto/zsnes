#include <string.h>

#include "../asm.h"
#include "../asm_call.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../c_vcache.h"
#include "../cfg.h"
#include "../chips/fxemu2.h"
#include "../chips/fxtable.h"
#include "../chips/sa1regs.h"
#include "../debugger.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../gui/c_gui.h"
#include "../gui/gui.h"
#include "../gui/guimisc.h"
#include "../gui/menu.h"
#include "../init.h"
#include "../initc.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/c_mode716.h"
#include "../video/procvid.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_65816d.h"
#include "c_execute.h"
#include "c_memory.h"
#include "execute.h"
#include "memory.h"
#include "memtable.h"
#include "regs.h"
#include "regsw.h"
#include "spc700.h"
#include "table.h"

void start65816(void)
{
    initvideo();
    if (videotroub == 1)
        return;

    memset(vidbufferofsa, 0, 150072);

    if (romloadskip == 1)
        StartGUI();
    else
        continueprog();
}

static void UpdateSFX(void)
{
    UpdatePORSCMR();
    UpdatePORSCMR();
    UpdateCLSR();
}

static void reexecuteb2(void)
{
    if (NoSoundReinit != 1)
        SetupPreGame();

    UpdateDPage();
    SA1UpdateDPage();
    Makemode7Table();
    if (SFXEnable != 0)
        UpdateSFX();

    curexecstate |= 2;
    NoSoundReinit = 0;
    csounddisable = 0;
    NextNGDisplay = 0;

    u4 const pc = xpc;
    u4 const pb = xpb;
    u1* const addr = pc & 0x8000 ? snesmmap[pb] : pc < 0x4300 || memtabler8[pb] != regaccessbankr8 ? snesmap2[pb]
                                                                                                   : (u1*)dmadata - 0x4300; // XXX ugly cast
    initaddrl = addr;

    // initialize variables (Copy from variables)
    u4 edx = curcyc /* cycles */ << 8 | xp /* flags */;
    u1* ebp = spcPCRam;
    u1* esi = addr + pc; // add program counter to address
    eop** edi = tableadc[xp];

    splitflags(edx);
    execute(&edx, &ebp, &esi, &edi);
    edx = joinflags(edx);

    // de-init variables (copy to variables)
    spcPCRam = ebp;
    Curtableaddr = edi;
    xp = edx;
    curcyc = edx >> 8;
    xpc = esi - initaddrl; // subtract program counter by address

#ifdef __MSDOS__
    asm_call(ResetTripleBuf);
#endif

    if (pressed[KeySaveState] & 1 || pressed[KeyLoadState] & 1) {
        NoSoundReinit = 1;
        csounddisable = 1;
    }

    if (NoSoundReinit != 1)
        DeInitPostGame();

    // Multipass Movies
    if (MoviePassWaiting == 1) {
        MovieDumpRaw();
        continueprog();
        return;
    }

    // clear all keys
    while (Check_Key() != 0)
        Get_Key();

    if (nextmenupopup == 1) {
        showmenu();
    } else if (ReturnFromSPCStall == 1) {
        goto activatereset;
    } else if (pressed[KeySaveState] & 1) {
        pressed[1] = 0;
        pressed[KeySaveState] = 2;
        statesaver();
        reexecuteb();
    } else if (pressed[KeyLoadState] & 1) {
        loadstate();
        reexecuteb();
    } else if (pressed[KeyInsrtChap] & 1) {
        pressed[KeyInsrtChap] = 0;
        MovieInsertChapter();
        continueprognokeys();
    } else if (pressed[KeyNextChap] & 1) {
        pressed[KeyNextChap] = 0;
        multchange = 1;
        MovieSeekAhead();
        continueprognokeys();
    } else if (pressed[KeyPrevChap] & 1) {
        pressed[KeyPrevChap] = 0;
        multchange = 1;
        MovieSeekBehind();
        continueprognokeys();
    } else if (SSKeyPressed == 1 || SPCKeyPressed == 1) {
        showmenu();
    }
#ifndef NO_DEBUGGER
    else if (debugdisble == 0 && pressed[59] & 1) {
        startdebugger();
    }
#endif
    else if (pressed[59] & 1) {
        showmenu();
    } else if (pressed[KeyQuickRst] & 1) {
    activatereset:
        GUIReset = 1;
        if (MovieProcessing == 2) { // Recording
            ResetDuringMovie();
        } else {
            GUIDoReset();
        }
        ReturnFromSPCStall = 0;
        continueprog();
    } else if (guioff == 1 || pressed[KeyQuickExit] & 1) {
        endprog();
    } else {
        StartGUI();
    }
}

static void reexecute(void)
{
    // clear keyboard presses
    u1* i = pressed;
    do {
        if (*i == 2)
            *i = 0;
    } while (++i != endof(pressed));
    reexecuteb2();
}

void continueprog(void)
{
    // clear keyboard presses
    memset(pressed, 0, sizeof(pressed));

    romloadskip = 0;
#ifndef NO_DEBUGGER
    debuggeron = 0;
#endif
    exiter = 0;

    InitPreGame();
    reexecute();
}

void continueprognokeys(void)
{
    romloadskip = 0;
#ifndef NO_DEBUGGER
    debuggeron = 0;
#endif
    exiter = 0;

    InitPreGame();
    reexecuteb2();
}

// Incorrect
void reexecuteb(void)
{
#ifndef __MSDOS__
    reexecuteb2();
#else
    reexecute();
#endif
}

void endprog(void)
{
    deinitvideo();
    MovieStop();
    DosExit();
}

void interror(void)
{
#ifdef __MSDOS__
    sti();
#endif
    deinitvideo();
    PrintStr("Cannot process interrupt handler!\r\n");
    DosExit();
}

static void set_timer_interval(u4 const ticks)
{
    timercount = ticks;
    outb(0x43, 0x36);
    outb(0x40, ticks);
    outb(0x40, ticks >> 8);
}

void init60hz(void)
{
    u4 const hz = romispal != 0 ? 50 : 60;
    u4 const ticks = 1193182 /* frequency of the 8253/8254 */ / hz;
    set_timer_interval(ticks);
}

void init18_2hz(void)
{
    set_timer_interval(65536);
}

void Donextlinecache(void)
{
    if (curypos != 0 && curypos < resolutn - 1 && !(scrndis & 0x10) && curblank == 0) {
        u1 ecx = curypos + 1;
        do {
            sprlefttot[ecx] = 0;
            ((u4*)sprleftpr)[ecx] = 0; // XXX ugly cast
            sprcnt[ecx] = 0;
            sprstart[ecx] = 0;
            sprtilecnt[ecx] = 0;
            sprend[ecx] = 0;
            sprendx[ecx] = 0;
        } while (++ecx != 0);
        asm_call(processsprites);
        asm_call(cachesprites);
    }
    NextLineCache = 0;
}

void execute(u4* const pedx, u1** const pebp, u1** const pesi, eop*** const pedi)
{
    u4 edx = *pedx;
    u1* ebp = *pebp;
    u1* esi = *pesi;
    eop** edi = *pedi;

    u1 p = edx;
    if (!(curexecstate & 0x02)) {
        // startagain: // XXX from asm
        if (xe != 1 && edx & 0x01 && !(INTEnab & 0xC0)) {
            edx = edx & 0xFFFF00FF | (edx - (0x50 << 8)) & 0x0000FF00;
        }
        if (doirqnext != 1 && SA1IRQEnable != 0 && irqon != 0) {
            edx = edx & 0xFFFF00FF | (edx - (12 << 8)) & 0x0000FF00;
        }
    } else {
        edi = tableadc[p];

        u4 const dspcyc = cycpbl;
        cycpbl = dspcyc - 55;
        if (dspcyc < 55) {
            cycpbl += cycpblt;
            // 1260, 10000/12625
            // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
            u4 ecx;
            u4 ebx = 0;
            u4 const op = *ebp++;
            asm volatile("push %%ebp;  mov %0, %%ebp;  call *%5;  mov %%ebp, %0;  pop %%ebp"
                         : "+a"(ebp), "=c"(ecx), "+b"(ebx), "+S"(esi), "+D"(edi)
                         : "c"(opcjmptab[op])
                         : "cc", "memory");
        }

        p = *esi++;
        u1 const c = cpucycle[p];
        u1 const cpucyc = edx >> 8;
        edx = edx & 0xFFFF00FF | (cpucyc - c) << 8 & 0x0000FF00;
        if (cpucyc < c)
            goto cpuover;
    }

    {
        u4 ecx = 0;
        u4 ebx = p; // XXX HACK: We run out of registers.  p is guaranteed to have only the lower 8 bits set, which is sufficient
        // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
        asm volatile("push %%ebp;  mov %0, %%ebp;  call *(%5, %3, %c6);  mov %%ebp, %0;  pop %%ebp"
                     : "+a"(ebp), "+c"(ecx), "+d"(edx), "+b"(ebx), "+S"(esi), "+D"(edi)
                     : "n"(sizeof(*edi))
                     : "cc", "memory");
    }
cpuover :

{
    u4 ecx = 0;
    u4 ebx = 0;
    // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
    asm volatile("push %%ebp;  mov %0, %%ebp;  call %P6;  mov %%ebp, %0;  pop %%ebp"
                 : "+a"(ebp), "+c"(ecx), "+d"(edx), "+b"(ebx), "+S"(esi), "+D"(edi)
                 : "X"(cpuover)
                 : "cc", "memory");
}

    *pedx = edx;
    *pebp = ebp;
    *pesi = esi;
    *pedi = edi;
}

void StartSFXdebugb(void)
{
    UpdatePORSCMR();
    UpdateSCBRCOLR();

    if (SfxSCMR & ((SfxPBR & 0x7F) < 0x70 ? /* noram */ 0x10 : /* ram */ 0x08)) {
        NumberOfOpcodes = SFXCounter == 1 ? 0x0FFFFFFF : SfxCLSR & 0x01 ? 800
                                                                        : // 678*2
            420; // 678
        asm_call(MainLoop);
    }
}

void UpdatePORSCMR(void)
{
    {
        u4 eax;
        if (SfxPOR & 0x10)
            goto objmode;
        switch (SfxSCMR & 0x24) // 4 + 32
        {
        default:
            eax = sfx128lineloc;
            break;
        case 0x04:
            eax = sfx160lineloc;
            break;
        case 0x20:
            eax = sfx192lineloc;
            break;
        objmode:
        case 0x24:
            eax = sfxobjlineloc;
            break;
        }
        sfxclineloc = eax;
    }

    u4 const eax_ = (SfxPOR & 0x0F) << 2 | SfxSCMR & 0x03;
    u4 const ebx = PLOTJmpb[eax_];
    u4 const eax = PLOTJmpb[eax_];
    FxTable[0x4C] = eax;
    FxTableb[0x4C] = eax;
    FxTablec[0x4C] = eax;
    FxTabled[0x4C] = ebx;
}

void UpdateSCBRCOLR(void)
{
    SCBRrel = sfxramdata + SfxSCBR * 1024;
    u4 const eax = SfxCOLR;
    fxbit01pcal = fxbit01[eax];
    fxbit23pcal = fxbit23[eax];
    fxbit45pcal = fxbit45[eax];
    fxbit67pcal = fxbit67[eax];
}

void UpdateCLSR(void)
{
    NumberOfOpcodes2 = SFXCounter != 1 ? 0x0FFFFFFF : SfxCLSR & 0x01 ? 700
                                                                     : 350;
}

void StartSFX(void)
{
    if (SfxSCMR & ((SfxPBR & 0x7F) < 0x70 ? /* noram */ 0x10 : /* ram */ 0x08)) {
        NumberOfOpcodes = NumberOfOpcodes2;
        asm_call(MainLoop);
    }
}
