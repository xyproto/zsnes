#include <string.h>

#include "../types.h"
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
#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof *(x))
#endif
#ifndef endof
#define endof(x) ((x) + lengthof(x))
#endif
#include "../ui.h"
#include "../vcache.h"
#include "../video/c_mode716.h"
#include "../video/procvid.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_65816d.h"
#include "c_dispatch.h"
#include "c_execloop.h"
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
    opfn** edi = tableadc[xp];

    splitflags(edx);
    execute(&edx, &ebp, &esi, &edi);
    edx = joinflags(edx);

    // de-init variables (copy to variables)
    spcPCRam = ebp;
    Curtableaddr = edi;
    xp = edx;
    curcyc = edx >> 8;
    xpc = esi - initaddrl; // subtract program counter by address

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
    reexecuteb2();
}

_Noreturn void endprog(void)
{
    deinitvideo();
    MovieStop();
    DosExit();
}

_Noreturn void interror(void)
{
    deinitvideo();
    PrintStr("Cannot process interrupt handler!\r\n");
    DosExit();
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
        processsprites();
        cachesprites();
    }
    NextLineCache = 0;
}

/* The `endloop` macro from cpu/65816dc.inc: step the SPC700 when its share of
   the cycles has run out, fetch the next opcode and charge it. Returns zero
   where the assembly returned out of the run, the scanline's budget spent. */
static int endloop(zreg* const r)
{
    u4 const dspcyc = cycpbl;

    cycpbl = dspcyc - 55;
    if (dspcyc < 55) {
        cycpbl += cycpblt;
        // 1260, 10000/12625
        u4 const sop = *(u1*)r[R_EBP];
        r[R_EBP]++;
        spc_step(r, sop);
    }

    set_bl(r, *(u1*)r[R_ESI]);
    r[R_ESI]++;

    {
        u1 const c = cpucycle[r[R_EBX]];
        u1 const dh = DH(r);
        set_dh(r, (u1)(dh - c));
        return dh >= c;
    }
}

/* A run of opcodes. In the assembly each body tail-jumped to the next through
   `endloop`, so a whole run went by without returning; the loop is the same
   shape. Every body leaves the table for the current flag state in edi, so it
   is re-read each time round. */
static void run_chain(zreg* const r)
{
    do {
        ((opfn**)r[R_EDI])[r[R_EBX]](r);
    } while (endloop(r));
}

/* The dispatch loop from cpu/execute.asm. */
void exec_loop(zreg* const r, int const at_cpuover)
{
    if (at_cpuover)
        goto cpuover;

    r[R_EBX] = (u1)r[R_EDX];
    if (curexecstate & 2)
        goto sound;

startagain:
    if (xe != 1 && r[R_EDX] & 0x01 && !(INTEnab & 0xC0))
        add_dh(r, (u1)-0x50);
    if (doirqnext != 1 && SA1IRQEnable != 0 && irqon != 0)
        add_dh(r, (u1)-12);
    run_chain(r);
    goto cpuover;

sound:
    r[R_EDI] = (zreg)tableadc[r[R_EBX]];
    {
        u4 const dspcyc = cycpbl;
        cycpbl = dspcyc - 55;
        if (dspcyc < 55) {
            cycpbl += cycpblt;
            // 1260, 10000/12625
            u4 const sop = *(u1*)r[R_EBP];
            r[R_EBP]++;
            spc_step(r, sop);
        }
    }
    set_bl(r, *(u1*)r[R_ESI]);
    r[R_ESI]++;
    {
        u1 const c = cpucycle[r[R_EBX]];
        u1 const dh = DH(r);
        set_dh(r, (u1)(dh - c));
        if (dh < c)
            goto cpuover;
    }
    run_chain(r);

cpuover:
    switch (c_cpuover(r)) {
    case EXEC_SOUND:
        // pexecs: let the SPC700 catch up in one burst.
        soundcycleft = 30;
        do {
            u4 const sop = *(u1*)r[R_EBP];
            r[R_EBP]++;
            spc_step(r, sop);
        } while (--soundcycleft != 0);
        set_dh(r, 0);
        // fall through
    case EXEC_NEXT:
        r[R_EBX] = *(u1*)r[R_ESI];
        r[R_ESI]++;
        goto startagain;
    case EXEC_RELOAD:
        r[R_EBX] = (u1)r[R_EDX];
        if (curexecstate & 2)
            goto sound;
        goto startagain;
    case EXEC_EXIT:
        break;
    }
}

void execute(u4* const pedx, u1** const pebp, u1** const pesi, opfn*** const pedi)
{
    zreg r[8] = { 0 };

    r[R_EDX] = *pedx;
    r[R_EBP] = (zreg)*pebp;
    r[R_ESI] = (zreg)*pesi;
    r[R_EDI] = (zreg)*pedi;

    exec_loop(r, 0);

    *pedx = r[R_EDX];
    *pebp = (u1*)r[R_EBP];
    *pesi = (u1*)r[R_ESI];
    *pedi = (opfn**)r[R_EDI];
}

// Raise a 65816 IRQ only when the GSU has stopped (Go clear, IRQ flag
// set, IRQs unmasked); an IRQ while it is merely suspended mid-task
// makes games flip buffers before rendering is done.
static inline void SfxIRQpoll(void)
{
    extern u4 SfxSFR, SfxCFGR;
    if ((SfxSFR & 0x8020) == 0x8000 && !(SfxCFGR & 0x80)) {
        doirqnext = 1;
    }
}

// Per-scanline GSU opcode budget: ~5.82M instructions/s at 10.7 MHz,
// 2.5x that at 21.4 MHz (CLSR bit 0). Unlimited runs games too fast.
static u4 SfxOpcodesPerLine(void)
{
    u4 const perLine = 5823405u / (romispal ? 50u * 312u : 60u * 262u);
    u4 const budget = SfxCLSR & 0x01 ? perLine * 5 / 2 : perLine;
    u4 percent = SuperFXClockMultiplier;

    if (percent < 50 || percent > 800) {
        percent = 100;
    }
    return budget * percent / 100;
}

void StartSFXdebugb(void)
{
    UpdatePORSCMR();
    UpdateSCBRCOLR();

    if (SfxSCMR & ((SfxPBR & 0x7F) < 0x70 ? /* noram */ 0x10 : /* ram */ 0x08)) {
        NumberOfOpcodes = SfxOpcodesPerLine();
        MainLoop();
        SfxIRQpoll();
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
    u4 const eax = PLOTJmpa[eax_];
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
    NumberOfOpcodes2 = SfxOpcodesPerLine();
}

static u1 SfxRanThisLine; // GSU already got its slice this scanline
static u4 SfxOwedOps; // budget banked while the bus is assigned to the 65816

void StartSFX(void)
{
    if (SfxRanThisLine) {
        SfxRanThisLine = 0;
        return;
    }
    if (SfxSCMR & ((SfxPBR & 0x7F) < 0x70 ? /* noram */ 0x10 : /* ram */ 0x08)) {
        NumberOfOpcodes = NumberOfOpcodes2 + SfxOwedOps;
        SfxOwedOps = 0;
        MainLoop();
        SfxIRQpoll();
    } else {
        // The gate only pauses the GSU briefly on hardware, not for whole
        // scanlines; bank the budget (up to one frame) and pay it back.
        SfxOwedOps += NumberOfOpcodes2;
        if (SfxOwedOps > NumberOfOpcodes2 * 240) {
            SfxOwedOps = NumberOfOpcodes2 * 240;
        }
    }
}

// Run a slice as soon as the game starts the GSU, so short tasks finish
// before the game polls the Go flag.
void SfxExecOnStart(void)
{
    if (SfxSCMR & ((SfxPBR & 0x7F) < 0x70 ? /* noram */ 0x10 : /* ram */ 0x08)) {
        NumberOfOpcodes = NumberOfOpcodes2;
        MainLoop();
        SfxIRQpoll();
        SfxRanThisLine = 1;
    }
}

// Finish the in-flight GSU task when vblank starts (bounded to about a
// frame of work), so the NMI handler never copies a half-drawn buffer.
void SfxVblankCatchup(void)
{
    extern u4 SfxSFR;

    if ((SfxSFR & 0x20) && (SfxSCMR & ((SfxPBR & 0x7F) < 0x70 ? /* noram */ 0x10 : /* ram */ 0x08))) {
        NumberOfOpcodes = NumberOfOpcodes2 * 240;
        MainLoop();
        SfxIRQpoll();
    }
}
