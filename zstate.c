/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __UNIXSDL__
#include "gblhdr.h"
#else
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#ifdef __WIN32__
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
#include "asm_call.h"
#include "c_vcache.h"
#include "cfg.h"
#include "chips/c4proc.h"
#include "chips/msu1emu.h"
#include "chips/dsp4emu.h"
#include "chips/sa1regs.h"
#include "cpu/execute.h"
#include "cpu/regs.h"
#include "cpu/regsw.h"
#include "cpu/spc700.h"
#include "endmem.h"
#include "gblvars.h"
#include "gui/c_guiwindp.h"
#include "init.h"
#include "initc.h"
#include "input.h"
#include "ui.h"
#include "video/procvid.h"
#include "zmovie.h"
#include "zpath.h"
#include "zstate.h"
#include <stdarg.h>

#define NUMCONV_FR3
#define NUMCONV_FW3
#include "numconv.h"

#ifdef __MSDOS__
#define clim() __asm__ __volatile__("cli");
#define stim() __asm__ __volatile__("sti");
#else
#define clim()
#define stim()
#endif

void SA1UpdateDPageC(), unpackfunct(), repackfunct();
void PrepareOffset(), ResetOffset(), initpitch(), UpdateBanksSDD1();
void procexecloop();

void copy_spc7110_state_data(uint8_t**, void (*)(unsigned char**, void*, size_t), bool);

extern uint8_t intrset, cycpl, cycphb, xdbt, xpbt;
extern uint8_t xirqb, curnmi;
extern uint16_t stackand, stackor, xat, xst, xdt, xxt, xyt;

u4 Totalbyteloaded;

static void copy_snes_data(uint8_t** buffer, void (*copy_func)(uint8_t**, void*, size_t))
{
    // 65816 status, etc.
    copy_func(buffer, &curcyc, 1);
    copy_func(buffer, &curypos, 2);
    copy_func(buffer, &cacheud, 1);
    copy_func(buffer, &ccud, 1);
    copy_func(buffer, &intrset, 1);
    copy_func(buffer, &cycpl, 1);
    copy_func(buffer, &cycphb, 1);
    copy_func(buffer, &spcon, 1);
    copy_func(buffer, &stackand, 2);
    copy_func(buffer, &stackor, 2);
    copy_func(buffer, &xat, 2);
    copy_func(buffer, &xdbt, 1);
    copy_func(buffer, &xpbt, 1);
    copy_func(buffer, &xst, 2);
    copy_func(buffer, &xdt, 2);
    copy_func(buffer, &xxt, 2);
    copy_func(buffer, &xyt, 2);
    copy_func(buffer, &xp, 1);
    copy_func(buffer, &xe, 1);
    copy_func(buffer, &xpc, 2);
    copy_func(buffer, &xirqb, 1);
    copy_func(buffer, &debugger, 1);
    copy_func(buffer, &Curtableaddr, 4);
    copy_func(buffer, &curnmi, 1);
    // SPC Timers
    copy_func(buffer, &cycpbl, 4);
    copy_func(buffer, &cycpblt, 4);
    // SNES PPU Register status
    copy_func(buffer, &sndrot, 3019);
}

static void copy_spc_data(uint8_t** buffer, void (*copy_func)(uint8_t**, void*, size_t))
{
    // SPC stuff, DSP stuff
    copy_func(buffer, SPCRAM, PHspcsave);
    copy_func(buffer, BRRBuffer, PHdspsave);
    copy_func(buffer, &DSPMem, sizeof(DSPMem));
}

static void copy_extra_data(uint8_t** buffer, void (*copy_func)(uint8_t**, void*, size_t))
{
    copy_func(buffer, &soundcycleft, 4);
    copy_func(buffer, &curexecstate, 4);
    copy_func(buffer, &nmiprevaddrl, 4);
    copy_func(buffer, &nmiprevaddrh, 4);
    copy_func(buffer, &nmirept, 4);
    copy_func(buffer, &nmiprevline, 4);
    copy_func(buffer, &nmistatus, 4);
    copy_func(buffer, &joycontren, 4);
    copy_func(buffer, &NextLineCache, 1);
    copy_func(buffer, &spc700read, 10 * 4);
    copy_func(buffer, &timer2upd, 4);
    copy_func(buffer, &xa, 14 * 4);
    copy_func(buffer, &spcnumread, 1);
    copy_func(buffer, &opcd, 6 * 4);
    copy_func(buffer, &HIRQCycNext, 4);
    copy_func(buffer, &HIRQNextExe, 1);
    copy_func(buffer, &oamaddr, 14 * 4);
    copy_func(buffer, &prevoamptr, 1);
}

static size_t load_save_size;

enum copy_state_method { csm_save_zst_new,
    csm_load_zst_new,
    csm_load_zst_old,
    csm_save_rewind,
    csm_load_rewind };

static void copy_state_data(uint8_t* buffer, void (*copy_func)(uint8_t**, void*, size_t), enum copy_state_method method)
{
    copy_snes_data(&buffer, copy_func);

    // WRAM (128k), VRAM (64k)
    copy_func(&buffer, wramdata, 8192 * 16);
    copy_func(&buffer, vram, 4096 * 16);

    if (spcon) {
        copy_spc_data(&buffer, copy_func);
        /*
    if (buffer) //Rewind stuff
    {
      copy_func(&buffer, &echoon0, PHdspsave2);
    }
    */
    }

    if (C4Enable) {
        copy_func(&buffer, C4Ram, 2048 * 4);
    }

    if (SFXEnable) {
        copy_func(&buffer, sfxramdata, 8192 * 16);
        copy_func(&buffer, &SfxR0, PHnum2writesfxreg);
    }

    if (SA1Enable) {
        copy_func(&buffer, &SA1Mode, PHnum2writesa1reg);
        copy_func(&buffer, SA1RAMArea, 8192 * 16);
        if (method != csm_load_zst_old) {
            copy_func(&buffer, &SA1Status, 3);
            copy_func(&buffer, &SA1xpc, 1 * 4);
            copy_func(&buffer, &sa1dmaptr, 2 * 4);
        }
    }

    if (DSP1Enable && (method != csm_load_zst_old)) {
        copy_func(&buffer, &DSP1COp, 70 + 128);
        copy_func(&buffer, &Op00Multiplicand, 3 * 4 + 128);
        copy_func(&buffer, &Op10Coefficient, 4 * 4 + 128);
        copy_func(&buffer, &Op04Angle, 4 * 4 + 128);
        copy_func(&buffer, &Op08X, 5 * 4 + 128);
        copy_func(&buffer, &Op18X, 5 * 4 + 128);
        copy_func(&buffer, &Op28X, 4 * 4 + 128);
        copy_func(&buffer, &Op0CA, 5 * 4 + 128);
        copy_func(&buffer, &Op02FX, 11 * 4 + 3 * 4 + 28 * 8 + 128);
        copy_func(&buffer, &Op0AVS, 5 * 4 + 14 * 8 + 128);
        copy_func(&buffer, &Op06X, 6 * 4 + 10 * 8 + 4 + 128);
        copy_func(&buffer, &Op01m, 4 * 4 + 128);
        copy_func(&buffer, &Op0DX, 6 * 4 + 128);
        copy_func(&buffer, &Op03F, 6 * 4 + 128);
        copy_func(&buffer, &Op14Zr, 9 * 4 + 128);
        copy_func(&buffer, &Op0EH, 4 * 4 + 128);
    }

    if (SETAEnable) {
        copy_func(&buffer, setaramdata, 256 * 16);

        // Todo: copy the SetaCmdEnable?  For completeness we should do it
        // but currently we ignore it anyway.
    }

    if (SPC7110Enable) {
        copy_func(&buffer, &SPCMultA, PHnum2writespc7110reg);
        copy_spc7110_state_data(&buffer, copy_func, (method == csm_load_zst_new) || (method == csm_load_rewind));
    }

    if (DSP4Enable) {
        copy_func(&buffer, &DSP4.waiting4command, sizeof(DSP4.waiting4command));
        copy_func(&buffer, &DSP4.half_command, sizeof(DSP4.half_command));
        copy_func(&buffer, &DSP4.command, sizeof(DSP4.command));
        copy_func(&buffer, &DSP4.in_count, sizeof(DSP4.in_count));
        copy_func(&buffer, &DSP4.in_index, sizeof(DSP4.in_index));
        copy_func(&buffer, &DSP4.out_count, sizeof(DSP4.out_count));
        copy_func(&buffer, &DSP4.out_index, sizeof(DSP4.out_index));
        copy_func(&buffer, &DSP4.parameters, sizeof(DSP4.parameters));
        copy_func(&buffer, &DSP4.output, sizeof(DSP4.output));

        copy_func(&buffer, &DSP4_vars.DSP4_Logic, sizeof(DSP4_vars.DSP4_Logic));
        copy_func(&buffer, &DSP4_vars.lcv, sizeof(DSP4_vars.lcv));
        copy_func(&buffer, &DSP4_vars.distance, sizeof(DSP4_vars.distance));
        copy_func(&buffer, &DSP4_vars.raster, sizeof(DSP4_vars.raster));
        copy_func(&buffer, &DSP4_vars.segments, sizeof(DSP4_vars.segments));
        copy_func(&buffer, &DSP4_vars.world_x, sizeof(DSP4_vars.world_x));
        copy_func(&buffer, &DSP4_vars.world_y, sizeof(DSP4_vars.world_y));
        copy_func(&buffer, &DSP4_vars.world_dx, sizeof(DSP4_vars.world_dx));
        copy_func(&buffer, &DSP4_vars.world_dy, sizeof(DSP4_vars.world_dy));
        copy_func(&buffer, &DSP4_vars.world_ddx, sizeof(DSP4_vars.world_ddx));
        copy_func(&buffer, &DSP4_vars.world_ddy, sizeof(DSP4_vars.world_ddy));
        copy_func(&buffer, &DSP4_vars.world_xenv, sizeof(DSP4_vars.world_xenv));
        copy_func(&buffer, &DSP4_vars.world_yofs, sizeof(DSP4_vars.world_yofs));
        copy_func(&buffer, &DSP4_vars.view_x1, sizeof(DSP4_vars.view_x1));
        copy_func(&buffer, &DSP4_vars.view_y1, sizeof(DSP4_vars.view_y1));
        copy_func(&buffer, &DSP4_vars.view_x2, sizeof(DSP4_vars.view_x2));
        copy_func(&buffer, &DSP4_vars.view_y2, sizeof(DSP4_vars.view_y2));
        copy_func(&buffer, &DSP4_vars.view_dx, sizeof(DSP4_vars.view_dx));
        copy_func(&buffer, &DSP4_vars.view_dy, sizeof(DSP4_vars.view_dy));
        copy_func(&buffer, &DSP4_vars.view_xofs1, sizeof(DSP4_vars.view_xofs1));
        copy_func(&buffer, &DSP4_vars.view_yofs1, sizeof(DSP4_vars.view_yofs1));
        copy_func(&buffer, &DSP4_vars.view_xofs2, sizeof(DSP4_vars.view_xofs2));
        copy_func(&buffer, &DSP4_vars.view_yofs2, sizeof(DSP4_vars.view_yofs2));
        copy_func(&buffer, &DSP4_vars.view_yofsenv, sizeof(DSP4_vars.view_yofsenv));
        copy_func(&buffer, &DSP4_vars.view_turnoff_x, sizeof(DSP4_vars.view_turnoff_x));
        copy_func(&buffer, &DSP4_vars.view_turnoff_dx, sizeof(DSP4_vars.view_turnoff_dx));
        copy_func(&buffer, &DSP4_vars.viewport_cx, sizeof(DSP4_vars.viewport_cx));
        copy_func(&buffer, &DSP4_vars.viewport_cy, sizeof(DSP4_vars.viewport_cy));
        copy_func(&buffer, &DSP4_vars.viewport_left, sizeof(DSP4_vars.viewport_left));
        copy_func(&buffer, &DSP4_vars.viewport_right, sizeof(DSP4_vars.viewport_right));
        copy_func(&buffer, &DSP4_vars.viewport_top, sizeof(DSP4_vars.viewport_top));
        copy_func(&buffer, &DSP4_vars.viewport_bottom, sizeof(DSP4_vars.viewport_bottom));
        copy_func(&buffer, &DSP4_vars.sprite_x, sizeof(DSP4_vars.sprite_x));
        copy_func(&buffer, &DSP4_vars.sprite_y, sizeof(DSP4_vars.sprite_y));
        copy_func(&buffer, &DSP4_vars.sprite_attr, sizeof(DSP4_vars.sprite_attr));
        copy_func(&buffer, &DSP4_vars.sprite_size, sizeof(DSP4_vars.sprite_size));
        copy_func(&buffer, &DSP4_vars.sprite_clipy, sizeof(DSP4_vars.sprite_clipy));
        copy_func(&buffer, &DSP4_vars.sprite_count, sizeof(DSP4_vars.sprite_count));
        copy_func(&buffer, &DSP4_vars.poly_clipLf, sizeof(DSP4_vars.poly_clipLf));
        copy_func(&buffer, &DSP4_vars.poly_clipRt, sizeof(DSP4_vars.poly_clipRt));
        copy_func(&buffer, &DSP4_vars.poly_ptr, sizeof(DSP4_vars.poly_ptr));
        copy_func(&buffer, &DSP4_vars.poly_raster, sizeof(DSP4_vars.poly_raster));
        copy_func(&buffer, &DSP4_vars.poly_top, sizeof(DSP4_vars.poly_top));
        copy_func(&buffer, &DSP4_vars.poly_bottom, sizeof(DSP4_vars.poly_bottom));
        copy_func(&buffer, &DSP4_vars.poly_cx, sizeof(DSP4_vars.poly_cx));
        copy_func(&buffer, &DSP4_vars.poly_start, sizeof(DSP4_vars.poly_start));
        copy_func(&buffer, &DSP4_vars.poly_plane, sizeof(DSP4_vars.poly_plane));
        copy_func(&buffer, &DSP4_vars.OAM_attr, sizeof(DSP4_vars.OAM_attr));
        copy_func(&buffer, &DSP4_vars.OAM_index, sizeof(DSP4_vars.OAM_index));
        copy_func(&buffer, &DSP4_vars.OAM_bits, sizeof(DSP4_vars.OAM_bits));
        copy_func(&buffer, &DSP4_vars.OAM_RowMax, sizeof(DSP4_vars.OAM_RowMax));
        copy_func(&buffer, &DSP4_vars.OAM_Row, sizeof(DSP4_vars.OAM_Row));
    }

    if(MSUEnable) {
        copy_func(&buffer, &MSU_Track_Position, sizeof(MSU_Track_Position));
        copy_func(&buffer, &MSU_StatusRead, sizeof(MSU_StatusRead));
        copy_func(&buffer, &MSU_MusicVolume, sizeof(MSU_MusicVolume));
    }

    if (method != csm_load_zst_old) {
        copy_extra_data(&buffer, copy_func);

        // We don't load SRAM from new states if box isn't checked
        if ((method != csm_load_zst_new) || SRAMState) {
            copy_func(&buffer, sram, ramsize);
        }

        if ((method == csm_save_rewind) || (method == csm_load_rewind)) {
            copy_func(&buffer, &tempesi, 4);
            copy_func(&buffer, &tempedi, 4);
            copy_func(&buffer, &tempedx, 4);
            copy_func(&buffer, &tempebp, 4);
        }
    }
}

static void memcpyinc(uint8_t** dest, void* src, size_t len)
{
    memcpy(*dest, src, len);
    *dest += len;
}

static void memcpyrinc(uint8_t** src, void* dest, size_t len)
{
    memcpy(dest, *src, len);
    *src += len;
}

extern uint32_t RewindTimer, DblRewTimer;

uint8_t* StateBackup = 0;
uint8_t AllocatedRewindStates, LatestRewindPos, EarliestRewindPos;
bool RewindPosPassed;

size_t rewind_state_size, cur_zst_size, old_zst_size;

void zmv_rewind_save(size_t, bool);
void zmv_rewind_load(size_t, bool);

void ClearCacheCheck()
{
    memset(vidmemch2, 1, sizeof(vidmemch2));
    memset(vidmemch4, 1, sizeof(vidmemch4));
    memset(vidmemch8, 1, sizeof(vidmemch8));
}

// Code to handle special frames for pausing, and desync checking
uint8_t *SpecialPauseBackup = 0, PauseFrameMode = 0;
/*
Pause frame modes

0 - no pause frame stored
1 - pause frame ready to be stored
2 - pause frame stored
3 - pause frame ready for reload
*/

void BackupPauseFrame()
{
    if (SpecialPauseBackup) {
        copy_state_data(SpecialPauseBackup, memcpyinc, csm_save_rewind);
        PauseFrameMode = 2;
    }
}

void RestorePauseFrame()
{
    if (SpecialPauseBackup) {
        copy_state_data(SpecialPauseBackup, memcpyrinc, csm_load_rewind);
        // ClearCacheCheck();
        PauseFrameMode = 0;
    }
}

void DeallocPauseFrame()
{
    if (SpecialPauseBackup) {
        free(SpecialPauseBackup);
    }
}

#define ActualRewindFrames (uint32_t)(RewindFrames * (romispal ? 10 : 12))

void BackupCVFrame()
{
    uint8_t* RewindBufferPos = StateBackup + LatestRewindPos * rewind_state_size;

    if (MovieProcessing == MOVIE_PLAYBACK) {
        zmv_rewind_save(LatestRewindPos, true);
    } else if (MovieProcessing == MOVIE_RECORD) {
        zmv_rewind_save(LatestRewindPos, false);
    }
    copy_state_data(RewindBufferPos, memcpyinc, csm_save_rewind);

    if (RewindPosPassed) {
        EarliestRewindPos = (EarliestRewindPos + 1) % AllocatedRewindStates;
        RewindPosPassed = false;
    }
    //  printf("Backing up in #%u, earliest: #%u, allocated: %u\n", LatestRewindPos, EarliestRewindPos, AllocatedRewindStates);

    LatestRewindPos = (LatestRewindPos + 1) % AllocatedRewindStates;

    if (LatestRewindPos == EarliestRewindPos) {
        RewindPosPassed = true;
    }

    RewindTimer = ActualRewindFrames;
    DblRewTimer += (DblRewTimer) ? 0 : ActualRewindFrames;
    //  printf("New backup slot: #%u, timer %u, check %u\n", LatestRewindPos, RewindTimer, DblRewTimer);
}

void RestoreCVFrame()
{
    uint8_t* RewindBufferPos;

    if (LatestRewindPos != ((EarliestRewindPos + 1) % AllocatedRewindStates)) {
        if (DblRewTimer > ActualRewindFrames) {
            if (LatestRewindPos == 1 || AllocatedRewindStates == 1) {
                LatestRewindPos = AllocatedRewindStates - 1;
            } else {
                LatestRewindPos = (LatestRewindPos) ? LatestRewindPos - 2 : AllocatedRewindStates - 2;
            }
        } else {
            LatestRewindPos = (LatestRewindPos) ? LatestRewindPos - 1 : AllocatedRewindStates - 1;
        }
    } else {
        LatestRewindPos = EarliestRewindPos;
    }

    RewindBufferPos = StateBackup + LatestRewindPos * rewind_state_size;
    // printf("Restoring from #%u, earliest: #%u\n", LatestRewindPos, EarliestRewindPos);

    if (MovieProcessing == MOVIE_RECORD) {
        zmv_rewind_load(LatestRewindPos, false);
    } else {
        if (MovieProcessing == MOVIE_PLAYBACK) {
            zmv_rewind_load(LatestRewindPos, true);
        }

        if (PauseRewind || EMUPause) {
            PauseFrameMode = EMUPause = true;
        }
    }

    copy_state_data(RewindBufferPos, memcpyrinc, csm_load_rewind);
    ClearCacheCheck();

    LatestRewindPos = (LatestRewindPos + 1) % AllocatedRewindStates;
    RewindTimer = ActualRewindFrames;
    DblRewTimer = 2 * ActualRewindFrames;
}

void SetupRewindBuffer()
{
    // For special rewind case to help out pauses
    DeallocPauseFrame();
    SpecialPauseBackup = malloc(rewind_state_size);

    // For standard rewinds
    if (StateBackup) {
        free(StateBackup);
    }
    for (; RewindStates; RewindStates--) {
        StateBackup = 0;
        StateBackup = (uint8_t*)malloc(rewind_state_size * RewindStates);
        if (StateBackup) {
            break;
        }
    }
    AllocatedRewindStates = RewindStates;
}

void DeallocRewindBuffer()
{
    if (StateBackup) {
        free(StateBackup);
    }
}

static size_t state_size;

static void state_size_tally(uint8_t** dest, void* src, size_t len)
{
    state_size += len;
}

void InitRewindVars()
{
    uint8_t almost_useless_array[1]; // An array is needed for copy_state_data to give the correct size
    state_size = 0;
    copy_state_data(almost_useless_array, state_size_tally, csm_save_rewind);
    rewind_state_size = state_size;

    SetupRewindBuffer();
    LatestRewindPos = 0;
    EarliestRewindPos = 0;
    RewindPosPassed = false;
    RewindTimer = 1;
    DblRewTimer = 1;
}

void InitRewindVarsForMovie()
{
    LatestRewindPos = 0;
    EarliestRewindPos = 0;
    RewindPosPassed = false;
    RewindTimer = 1;
    DblRewTimer = 1;
}

// This is used to preserve system load state between game loads
static uint8_t* BackupSystemBuffer = 0;

void BackupSystemVars(void)
{
    uint8_t* buffer;

    if (!BackupSystemBuffer) {
        state_size = 0;
        copy_snes_data(&buffer, state_size_tally);
        copy_spc_data(&buffer, state_size_tally);
        copy_extra_data(&buffer, state_size_tally);
        BackupSystemBuffer = (uint8_t*)malloc(state_size);
    }

    if (BackupSystemBuffer) {
        buffer = BackupSystemBuffer;
        copy_snes_data(&buffer, memcpyinc);
        copy_spc_data(&buffer, memcpyinc);
        copy_extra_data(&buffer, memcpyinc);
    }
}

void RestoreSystemVars(void)
{
    if (BackupSystemBuffer) {
        uint8_t* buffer = BackupSystemBuffer;
        InitRewindVars();
        copy_snes_data(&buffer, memcpyrinc);
        copy_spc_data(&buffer, memcpyrinc);
        copy_extra_data(&buffer, memcpyrinc);
    }
}

void DeallocSystemVars()
{
    if (BackupSystemBuffer) {
        free(BackupSystemBuffer);
    }
}

extern uintptr_t Voice0BufPtr, Voice1BufPtr, Voice2BufPtr, Voice3BufPtr;
extern uintptr_t Voice4BufPtr, Voice5BufPtr, Voice6BufPtr, Voice7BufPtr;

void PrepareSaveState()
{
    spcPCRam -= (uintptr_t)SPCRAM;
    spcRamDP -= (uintptr_t)SPCRAM;

    Voice0BufPtr -= (uintptr_t)spcBuffera;
    Voice1BufPtr -= (uintptr_t)spcBuffera;
    Voice2BufPtr -= (uintptr_t)spcBuffera;
    Voice3BufPtr -= (uintptr_t)spcBuffera;
    Voice4BufPtr -= (uintptr_t)spcBuffera;
    Voice5BufPtr -= (uintptr_t)spcBuffera;
    Voice6BufPtr -= (uintptr_t)spcBuffera;
    Voice7BufPtr -= (uintptr_t)spcBuffera;
}

extern uintptr_t SA1Stat;
extern uint8_t IRAM[2049], *SA1Ptr, *SA1RegPCS, *CurBWPtr, *SA1BWPtr, *SNSBWPtr;

void SaveSA1()
{
    SA1Stat &= 0xFFFFFF00;
    SA1Ptr -= (uintptr_t)SA1RegPCS;

    if (SA1RegPCS == IRAM) {
        SA1Stat = (SA1Stat & 0xFFFFFF00) + 1;
    }

    if (SA1RegPCS == IRAM - 0x3000) {
        SA1Stat = (SA1Stat & 0xFFFFFF00) + 2;
    }

    SA1RegPCS -= (uintptr_t)romdata;
    CurBWPtr -= (uintptr_t)romdata;
    SA1BWPtr -= (uintptr_t)romdata;
    SNSBWPtr -= (uintptr_t)romdata;
}

void RestoreSA1()
{
    SA1RegPCS += (uintptr_t)romdata;
    CurBWPtr += (uintptr_t)romdata;
    SA1BWPtr += (uintptr_t)romdata;
    SNSBWPtr += (uintptr_t)romdata;

    if ((SA1Stat & 0xFF) == 1) {
        SA1RegPCS = IRAM;
    }

    if ((SA1Stat & 0xFF) == 2) {
        SA1RegPCS = IRAM - 0x3000;
    }

    SA1Ptr += (uintptr_t)SA1RegPCS;
}

#define ResState(Voice_BufPtr)                               \
    Voice_BufPtr += (uintptr_t)spcBuffera;                   \
    if (Voice_BufPtr >= (uintptr_t)spcBuffera + 65536 * 4) { \
        Voice_BufPtr = (uintptr_t)spcBuffera;                \
    }

void ResetState()
{
    spcPCRam += (uintptr_t)SPCRAM;
    spcRamDP += (uintptr_t)SPCRAM;

    ResState(Voice0BufPtr);
    ResState(Voice1BufPtr);
    ResState(Voice2BufPtr);
    ResState(Voice3BufPtr);
    ResState(Voice4BufPtr);
    ResState(Voice5BufPtr);
    ResState(Voice6BufPtr);
    ResState(Voice7BufPtr);
}

extern uint32_t SfxRomBuffer, SfxCROM;
extern uint32_t SfxLastRamAdr, SfxRAMMem;

static FILE* fhandle;
void CapturePicture();

static void write_save_state_data(uint8_t** dest, void* data, size_t len)
{
    fwrite(data, 1, len, fhandle);
}

static const char zst_header_old[] = "ZSNES Save State File V0.6\x1a\x3c";
static const char zst_header_cur[] = "ZSNES Save State File V143\x1a\x8f";

void calculate_state_sizes()
{
    state_size = 0;
    copy_state_data(0, state_size_tally, csm_save_zst_new);
    cur_zst_size = state_size + sizeof(zst_header_cur) - 1;

    state_size = 0;
    copy_state_data(0, state_size_tally, csm_load_zst_old);
    old_zst_size = state_size + sizeof(zst_header_old) - 1;
}

uint32_t current_zst = 0;
uint32_t newest_zst = 0;
time_t newestfiledate;

char* zst_name()
{
    static char buffer[7];
    if ((MovieProcessing == MOVIE_PLAYBACK) || (MovieProcessing == MOVIE_RECORD)) {
        sprintf(buffer, "%.2d.zst", (unsigned int)current_zst);
        return (buffer);
    }
    strcpy(buffer, "zst");
    if (current_zst) {
        buffer[2] = (current_zst % 10) + '0';
        if (current_zst > 9) {
            buffer[1] = (current_zst / 10) + '0';
        }
    }
    setextension(ZStateName, buffer);
    return (ZStateName);
}

void zst_determine_newest(void)
{
    struct stat filestat;
    char* zst_path;

    if (MovieInProgress()) {
        mzt_chdir_up();
        zst_path = ZMoviePath;
    } else {
        zst_path = ZSStatePath;
    }

    if (!stat_dir(zst_path, zst_name(), &filestat) && filestat.st_mtime > newestfiledate) {
        newestfiledate = filestat.st_mtime;
        newest_zst = current_zst;
    }
    if (MovieInProgress()) {
        mzt_chdir_down();
    }
}

void zst_init()
{
    newestfiledate = 0;

    if (LatestSave) {
        for (current_zst = 0; current_zst < 100; current_zst++) {
            zst_determine_newest();
        }
        current_zst = newest_zst;
        zst_name();
    }
}

int zst_exists(void)
{
    int ret;
    char* zst_path;

    if (MovieInProgress()) {
        mzt_chdir_up();
        zst_path = ZMoviePath;
    } else {
        zst_path = ZSStatePath;
    }

    ret = access_dir(zst_path, zst_name(), F_OK) ? 0 : 1;
    if (MovieInProgress()) {
        mzt_chdir_down();
    }

    return (ret);
}

static bool zst_save_compressed(FILE* fp)
{
    size_t data_size = cur_zst_size - (sizeof(zst_header_cur) - 1);
    uint8_t* buffer = 0;

    bool worked = false;

    if ((buffer = (uint8_t*)malloc(data_size))) {
        unsigned long compressed_size = compressBound(data_size);
        uint8_t* compressed_buffer = 0;

        if ((compressed_buffer = (uint8_t*)malloc(compressed_size))) {
            copy_state_data(buffer, memcpyinc, csm_save_zst_new);
            if (compress2(compressed_buffer, &compressed_size, buffer, data_size, Z_BEST_COMPRESSION) == Z_OK) {
                fwrite3(compressed_size, fp);
                fwrite(compressed_buffer, 1, compressed_size, fp);
                worked = true;
            }
            free(compressed_buffer);
        }
        free(buffer);
    }

    if (!worked) // Compression failed for whatever reason
    {
        fwrite3(cur_zst_size | 0x00800000, fp); // Uncompressed ZST will never break 8MB
    }

    return (worked);
}

void zst_save(FILE* fp, bool Thumbnail, bool Compress)
{
    PrepareOffset();
    PrepareSaveState();
    unpackfunct();

    if (SFXEnable) {
        SfxRomBuffer -= SfxCROM;
        SfxLastRamAdr -= SfxRAMMem;
    }

    if (SA1Enable) {
        SaveSA1(); // Convert SA-1 stuff to standard, non displacement format
    }

    if (!Compress || !zst_save_compressed(fp)) // If we don't want compressed or compression failed
    {
        fwrite(zst_header_cur, 1, sizeof(zst_header_cur) - 1, fp); //-1 for null

        fhandle = fp; // Set global file handle
        copy_state_data(0, write_save_state_data, csm_save_zst_new);

        if (Thumbnail) {
            CapturePicture();
            fwrite(PrevPicture, 1, sizeof(PrevPicture), fp);
        }
    }

    if (SFXEnable) {
        SfxRomBuffer += SfxCROM;
        SfxLastRamAdr += SfxRAMMem;
    }

    if (SA1Enable) {
        RestoreSA1(); // Convert back SA-1 stuff
    }

    ResetOffset();
    ResetState();
}

/*
Merges all the passed strings into buffer. Make sure to pass an extra parameter as 0 after all the strings.
Copies at most buffer_len characters. Result is always null terminated.
Returns how many bytes are needed to store all strings.
Thus if return is <= buffer_len, everything was copied.
*/
static size_t string_merge(char* buffer, size_t buffer_len, ...)
{
    char* s;
    size_t copied = 0, needed = 0;

    va_list ap;
    va_start(ap, buffer_len);

    if (buffer && buffer_len) {
        *buffer = 0;
    }

    while ((s = va_arg(ap, char*))) {
        needed += strlen(s);
        if (buffer && (copied + 1 < buffer_len)) {
            strncpy(buffer + copied, s, buffer_len - copied);
            buffer[buffer_len - 1] = 0;
            copied += strlen(buffer + copied);
        }
    }

    va_end(ap);
    return (needed + 1);
}

static char txtmsg[30];

void set_state_message(char* prefix, char* suffix)
{
    char num[3];
    sprintf(num, "%d", (unsigned int)current_zst);
    string_merge(txtmsg, sizeof(txtmsg), prefix, isextension(ZStateName, "zss") ? "AUTO" : num, suffix, 0);

    Msgptr = txtmsg;
    MessageOn = MsgCount;
}

void statesaver(void)
{
    if (MovieProcessing == MOVIE_RECORD) {
        //'Auto increment savestate slot' code
        current_zst += AutoIncSaveSlot;
        current_zst %= 100;

        if (mzt_save(current_zst, !!cbitmode, false)) {
            set_state_message("RR STATE ", " SAVED.");
        } else {
            current_zst += 100 - AutoIncSaveSlot;
            current_zst %= 100;
        }
        return;
    }

    if ((MovieProcessing == MOVIE_PLAYBACK) || (MovieProcessing == MOVIE_DUMPING_NEW)) {
        //'Auto increment savestate slot' code
        current_zst += AutoIncSaveSlot;
        current_zst %= 100;

        if (mzt_save(current_zst, !!cbitmode, true)) {
            set_state_message("RR STATE ", " SAVED.");
        } else {
            current_zst += 100 - AutoIncSaveSlot;
            current_zst %= 100;
        }
        return;
    }

    clim();

    //'Auto increment savestate slot' code
    if (!isextension(ZStateName, "zss")) {
        current_zst += (char)AutoIncSaveSlot;
        current_zst %= 100;
        zst_name();
    }

    if ((fhandle = fopen_dir(ZSStatePath, ZStateName, "wb"))) {
        zst_save(fhandle, !!cbitmode, false);
        fclose(fhandle);

        // Display message onscreen, 'STATE XX SAVED.'
        set_state_message("STATE ", " SAVED.");
    } else {
        // Display message onscreen, 'UNABLE TO SAVE.'
        Msgptr = "UNABLE TO SAVE.";
        MessageOn = MsgCount;

        if (!isextension(ZStateName, "zss")) {
            current_zst += 100 - (char)AutoIncSaveSlot;
            current_zst %= 100;
            zst_name();
        }
    }

    stim();
}

extern uint32_t SfxMemTable[256], SfxCPB;
extern uint32_t SfxPBR, SfxROMBR, SfxRAMBR, SCBRrel, SfxSCBR;
extern uint8_t ioportval;
extern u1 nexthdma;

static void read_save_state_data(uint8_t** dest, void* data, size_t len)
{
    load_save_size += fread(data, 1, len, fhandle);
}

static bool zst_load_compressed(FILE* fp, size_t compressed_size)
{
    unsigned long data_size = cur_zst_size - (sizeof(zst_header_cur) - 1);
    uint8_t* buffer = 0;
    bool worked = false;

    if ((buffer = (uint8_t*)malloc(data_size))) {
        uint8_t* compressed_buffer = 0;

        if ((compressed_buffer = (uint8_t*)malloc(compressed_size))) {
            fread(compressed_buffer, 1, compressed_size, fp);
            if (uncompress(buffer, &data_size, compressed_buffer, compressed_size) == Z_OK) {
                copy_state_data(buffer, memcpyrinc, csm_load_zst_new);
                worked = true;
            }
            free(compressed_buffer);
        }
        free(buffer);
    }
    return (worked);
}

bool zst_load(FILE* fp, size_t Compressed)
{
    size_t zst_version = 0;

    if (Compressed) {
        if (!zst_load_compressed(fp, Compressed)) {
            return (false);
        }
    } else {
        char zst_header_check[sizeof(zst_header_cur) - 1];

        Totalbyteloaded += fread(zst_header_check, 1, sizeof(zst_header_check), fp);

        if (!memcmp(zst_header_check, zst_header_cur, sizeof(zst_header_check) - 2)) {
            zst_version = 143; // v1.43+
        }

        if (!memcmp(zst_header_check, zst_header_old, sizeof(zst_header_check) - 2)) {
            zst_version = 60; // v0.60 - v1.42
        }

        if (!zst_version) {
            return (false);
        } // Pre v0.60 saves are no longer loaded

        load_save_size = 0;
        fhandle = fp; // Set global file handle
        copy_state_data(0, read_save_state_data, (zst_version == 143) ? csm_load_zst_new : csm_load_zst_old);
        Totalbyteloaded += load_save_size;
    }

    if (SFXEnable) {
        SfxCPB = SfxMemTable[(SfxPBR & 0xFF)];
        SfxCROM = SfxMemTable[(SfxROMBR & 0xFF)];
        SfxRAMMem = (uintptr_t)sfxramdata + ((SfxRAMBR & 0xFF) << 16);
        SfxRomBuffer += SfxCROM;
        SfxLastRamAdr += SfxRAMMem;
        SCBRrel = (SfxSCBR << 10) + (uintptr_t)sfxramdata;
    }

    if (SA1Enable) {
        RestoreSA1(); // Convert back SA-1 stuff
        SA1UpdateDPageC();
    }

    if (SDD1Enable) {
        UpdateBanksSDD1();
    }

    // Clear cache check if state loaded
    ClearCacheCheck();

    if (zst_version < 143) // Set new vars which old states did not have
    {
        prevoamptr = 0xFF;
        ioportval = 0xFF;
        spcnumread = 0;
    }

    if (MovieProcessing != MOVIE_RECORD) {
        nexthdma = 0;
    }

    repackfunct();
    initpitch();
    ResetOffset();
    ResetState();
    procexecloop();

    return (true);
}

// Wrapper for above
bool zst_compressed_loader(FILE* fp)
{
    size_t data_size = fread3(fp);
    return ((data_size & 0x00800000) ? zst_load(fp, 0) : zst_load(fp, data_size));
}

#define PH65816regsize 36
void zst_sram_load(FILE* fp)
{
    fseek(fp, sizeof(zst_header_cur) - 1 + PH65816regsize + 199635, SEEK_CUR);
    if (spcon) {
        fseek(fp, PHspcsave + PHdspsave + sizeof(DSPMem), SEEK_CUR);
    }
    if (C4Enable) {
        fseek(fp, 8192, SEEK_CUR);
    }
    if (SFXEnable) {
        fseek(fp, PHnum2writesfxreg + 131072, SEEK_CUR);
    }
    if (SA1Enable) {
        fseek(fp, PHnum2writesa1reg, SEEK_CUR);
        fread(SA1RAMArea, 1, 131072, fp); // SA-1 sram
        fseek(fp, 15, SEEK_CUR);
    }
    if (DSP1Enable) {
        fseek(fp, 2874, SEEK_CUR);
    }
    if (SETAEnable) {
        fread(setaramdata, 1, 4096, fp);
    } // SETA sram
    if (SPC7110Enable) {
        fseek(fp, PHnum2writespc7110reg + 6, SEEK_CUR);
    }
    if (DSP4Enable) {
        fseek(fp, 1294, SEEK_CUR);
    }
    if(MSUEnable) {
        fseek(fp, 6, SEEK_CUR);
    }
    fseek(fp, 220, SEEK_CUR);
    if (ramsize) {
        fread(sram, 1, ramsize, fp);
    } // normal sram
}

void zst_sram_load_compressed(FILE* fp)
{
    size_t compressed_size = fread3(fp);

    if (compressed_size & 0x00800000) {
        zst_sram_load(fp);
    } else {
        unsigned long data_size = cur_zst_size - (sizeof(zst_header_cur) - 1);
        uint8_t* buffer = 0;

        if ((buffer = (uint8_t*)malloc(data_size))) {
            uint8_t* compressed_buffer = 0;
            if ((compressed_buffer = (uint8_t*)malloc(compressed_size))) {
                fread(compressed_buffer, 1, compressed_size, fp);
                if (uncompress(buffer, &data_size, compressed_buffer, compressed_size) == Z_OK) {
                    uint8_t* data = buffer + PH65816regsize + 199635;
                    if (spcon) {
                        data += PHspcsave + PHdspsave + sizeof(DSPMem);
                    }
                    if (C4Enable) {
                        data += 8192;
                    }
                    if (SFXEnable) {
                        data += PHnum2writesfxreg + 131072;
                    }
                    if (SA1Enable) {
                        data += PHnum2writesa1reg;
                        memcpyrinc(&data, SA1RAMArea, 131072); // SA-1 sram
                        data += 15;
                    }
                    if (DSP1Enable) {
                        data += 2874;
                    }
                    if (SETAEnable) {
                        memcpyrinc(&data, setaramdata, 4096);
                    } // SETA sram
                    if (SPC7110Enable) {
                        data += PHnum2writespc7110reg + 6;
                    }
                    if (DSP4Enable) {
                        data += 1294;
                    }
                    if(MSUEnable) {
                        data += 6;
                    }
                    data += 220;
                    if (ramsize) {
                        memcpyrinc(&data, sram, ramsize);
                    } // normal sram
                }
                free(compressed_buffer);
            }
            free(buffer);
        }
    }
}

void stateloader(char* statename, bool keycheck, bool xfercheck)
{
    if (keycheck) {
        pressed[1] = 0;
        pressed[KeyLoadState] = 2;
        multchange = 1;
        MessageOn = MsgCount;
    }

    if (MZTForceRTR == RTR_REPLAY_TO_RECORD && (MovieProcessing == MOVIE_PLAYBACK)) {
        MovieRecord();
    } else if (MZTForceRTR == RTR_RECORD_TO_REPLAY && (MovieProcessing == MOVIE_RECORD)) {
        MovieStop();
        MoviePlay();
    }

    switch (MovieProcessing) {
    case MOVIE_PLAYBACK:
        if (mzt_load(current_zst, true)) {
            Msgptr = "CHAPTER LOADED.";
            MessageOn = MsgCount;
        } else {
            set_state_message("UNABLE TO LOAD STATE ", ".");
        }
        return;
    case MOVIE_RECORD:
        if (mzt_load(current_zst, false)) {
            set_state_message("RR STATE ", " LOADED.");

            if (PauseLoad || EMUPause) {
                PauseFrameMode = EMUPause = true;
            }
        } else {
            set_state_message("UNABLE TO LOAD STATE ", ".");
        }
        return;
    case MOVIE_OLD_PLAY: {
        size_t fname_len = strlen(statename);
        setextension(statename, "zmv");
        if (isdigit(CMovieExt)) {
            statename[fname_len - 1] = CMovieExt;
        }
    }
    case MOVIE_ENDING_DUMPING:
    case MOVIE_DUMPING_NEW:
    case MOVIE_DUMPING_OLD:
        return;
        break;
    }

    clim();

    if (!isextension(ZStateName, "zss")) {
        zst_name();
    }

    // Actual state loading code
    if ((fhandle = fopen_dir(ZSStatePath, statename, "rb"))) {
        if (xfercheck) {
            Totalbyteloaded = 0;
        }

        if (zst_load(fhandle, 0)) {
            set_state_message("STATE ", " LOADED."); // 'STATE XX LOADED.'

            if (PauseLoad || EMUPause) {
                PauseFrameMode = EMUPause = true;
            }
        } else {
            set_state_message("STATE ", " TOO OLD."); // 'STATE X TOO OLD.' - I don't think this is always accurate -Nach
        }
        fclose(fhandle);
    } else {
        set_state_message("UNABLE TO LOAD STATE ", "."); // 'UNABLE TO LOAD STATE XX.'
    }

    memset(&Voice0Disable, 1, sizeof(Voice0Disable));

    stim();
}

void debugloadstate()
{
    stateloader(ZStateName, 0, 0);
}

void loadstate(void)
{
    stateloader(ZStateName, 1, 0);
}

void loadstate2(void)
{
    stateloader(ZStateName, 0, 1);
}

void LoadSecondState(void)
{
    setextension(ZStateName, "zss");
    loadstate2();
    zst_name();
}

void SaveSecondState(void)
{
    setextension(ZStateName, "zss");
    statesaver();
    zst_name();
}

extern uint8_t CHIPBATT, *sram2;
void SaveCombFile();

// Sram saving
void SaveSramData(void)
{
    if (*ZSaveName && (!SRAMSave5Sec || sramb4save)) {
        FILE* fp = 0;
        uint8_t special = 0;
        uint8_t* data_to_save;

        setextension(ZSaveName, "srm");

        if (ramsize && !sramsavedis) {
            if (SFXEnable) {
                data_to_save = (uint8_t*)sfxramdata;
                special = 1;
            } else if (SA1Enable) {
                data_to_save = (uint8_t*)SA1RAMArea;
                special = 1;
            } else if (SETAEnable) {
                data_to_save = (uint8_t*)setaramdata;
                special = 1;
            } else {
                data_to_save = sram;
            }

            if (!special || CHIPBATT) {
                clim();
                if (!nosaveSRAM && (fp = fopen_dir(ZSramPath, ZSaveName, "wb"))) {
                    fwrite(data_to_save, 1, ramsize, fp);
                    fclose(fp);
                }
                if (!nosaveSRAM && *ZSaveST2Name && (fp = fopen_dir(ZSramPath, ZSaveST2Name, "wb"))) {
                    fwrite(sram2, 1, ramsize, fp);
                    fclose(fp);
                }
                stim();
            }
        }
        sramb4save = 0;
    }
    SaveCombFile();
}

extern bool SramExists;
void OpenSramFile()
{
    FILE* fp;

    setextension(ZSaveName, "srm");
    if ((fp = fopen_dir(ZSramPath, ZSaveName, "rb"))) {
        fread(sram, 1, ramsize, fp);
        fclose(fp);

        SramExists = true;

        if (*ZSaveST2Name && (fp = fopen_dir(ZSramPath, ZSaveST2Name, "rb"))) {
            fread(sram2, 1, ramsize, fp);
            fclose(fp);
        }
    } else {
        SramExists = false;
    }
}

/*
SPC File Format - Invented by _Demo_ & zsKnight
Cleaned up by Nach

00000h-00020h - File Header : SNES-SPC700 Sound File Data v0.00 (33 bytes)
00021h-00023h - 0x1a,0x1a,0x1a (3 bytes)
00024h        - 10 (1 byte)
00025h        - PC Register value (1 Word)
00027h        - A Register Value (1 byte)
00028h        - X Register Value (1 byte)
00029h        - Y Register Value (1 byte)
0002Ah        - Status Flags Value (1 byte)
0002Bh        - Stack Register Value (1 byte)
0002Ch-0002Dh - Reserved (2 bytes)
0002Eh-0004Dh - SubTitle/Song Name (32 bytes)
0004Eh-0006Dh - Title of Game (32 bytes)
0006Eh-0007Dh - Name of Dumper (16 bytes)
0007Eh-0009Dh - Comments (32 bytes)
0009Eh-000A1h - Date the SPC was Dumped (4 bytes)
000A2h-000A8h - Reserved (7 bytes)
000A9h-000ACh - Length of SPC in seconds (4 bytes)
000ADh-000AFh - Fade out length in milliseconds (3 bytes)
000B0h-000CFh - Author of Song (32 bytes)
000D0h        - Default Channel Disables (0 = enable, 1 = disable) (1 byte)
000D1h        - Emulator used to dump .spc file (1 byte)
                (0 = UNKNOWN, 1 = ZSNES, 2 = SNES9X)
                (Note : Contact the authors if you're an snes emu author with
                 an .spc capture in order to assign you a number)
000D2h-000FFh - Reserved (46 bytes)
00100h-100FFh - SPCRam (64 KB)
10100h-101FFh - DSPRam (256 bytes)
*/

extern uint8_t spcextraram[64];
extern uint32_t infoloc;

char spcsaved[16];
void savespcdata(void)
{
    size_t fname_len;
    unsigned int i = 0;

    setextension(ZSaveName, "spc");
    fname_len = strlen(ZSaveName);

    while (i < 100) {
        if (i) {
            sprintf(ZSaveName - 1 + fname_len - ((i < 10) ? 0 : 1), "%d", i);
        }
        if (access_dir(ZSpcPath, ZSaveName, F_OK)) {
            break;
        }
        i++;
    }
    if (i < 100) {
        FILE* fp = fopen_dir(ZSpcPath, ZSaveName, "wb");
        if (fp) {
            uint8_t ssdatst[256];
            time_t t = time(0);
            struct tm* lt = localtime(&t);

            // Assemble N/Z flags into P
            spcP &= 0xFD;
            if (!spcNZ) {
                spcP |= 2;
            }
            spcP &= 0x7F;
            if (spcNZ & 0x80) {
                spcP |= 0x80;
            }

            strcpy((char*)ssdatst, "SNES-SPC700 Sound File Data v0.30"); // 00000h - File Header : SNES-SPC700 Sound File Data v0.00
            ssdatst[0x21] = ssdatst[0x22] = ssdatst[0x23] = 0x1a; // 00021h - 0x1a,0x1a,0x1a
            ssdatst[0x24] = 10; // 00024h - 10
            *(uint16_t*)(ssdatst + 0x25) = spcPCRam - SPCRAM; // 00025h - PC Register value (1 Word)
            ssdatst[0x27] = spcA; // 00027h - A Register Value (1 byte)
            ssdatst[0x28] = spcX; // 00028h - X Register Value (1 byte)
            ssdatst[0x29] = spcY; // 00029h - Y Register Value (1 byte)
            ssdatst[0x2A] = spcP; // 0002Ah - Status Flags Value (1 byte)
            ssdatst[0x2B] = spcS; // 0002Bh - Stack Register Value (1 byte)

            ssdatst[0x2C] = 0; // 0002Ch - Reserved
            ssdatst[0x2D] = 0; // 0002Dh - Reserved

            PrepareSaveState();

            memset(ssdatst + 0x2E, 0, 32); // 0002Eh-0004Dh - SubTitle/Song Name
            memset(ssdatst + 0x4E, 0, 32); // 0004Eh-0006Dh - Title of Game
            memcpy(ssdatst + 0x4E, ((uint8_t*)romdata) + infoloc, 21);
            memset(ssdatst + 0x6E, 0, 16); // 0006Eh-0007Dh - Name of Dumper
            memset(ssdatst + 0x7E, 0, 32); // 0007Eh-0009Dh - Comments

            // 0009Eh-000A1h - Date the SPC was Dumped
            ssdatst[0x9E] = lt->tm_mday;
            ssdatst[0x9F] = lt->tm_mon + 1;
            ssdatst[0xA0] = (lt->tm_year + 1900) & 0xFF;
            ssdatst[0xA1] = ((lt->tm_year + 1900) >> 8) & 0xFF;

            memset(ssdatst + 0xA2, 0, 7); // 000A2h-000A8h - Reserved
            memset(ssdatst + 0xA9, 0, 4); // 000A9h-000ACh - Length of SPC in seconds
            memset(ssdatst + 0xAD, 0, 3); // 000ADh-000AFh - Fade out time in milliseconds
            memset(ssdatst + 0xB0, 0, 32); // 000B0h-000CFh - Author of Song

            // Set Channel Disables
            ssdatst[0xD0] = 0; // 000D0h - Default Channel Disables (0 = enable, 1 = disable)
            if (Voice0Disable[0]) {
                ssdatst[0xD0] |= BIT(0);
            }
            if (Voice0Disable[1]) {
                ssdatst[0xD0] |= BIT(1);
            }
            if (Voice0Disable[2]) {
                ssdatst[0xD0] |= BIT(2);
            }
            if (Voice0Disable[3]) {
                ssdatst[0xD0] |= BIT(3);
            }
            if (Voice0Disable[4]) {
                ssdatst[0xD0] |= BIT(4);
            }
            if (Voice0Disable[5]) {
                ssdatst[0xD0] |= BIT(5);
            }
            if (Voice0Disable[6]) {
                ssdatst[0xD0] |= BIT(6);
            }
            if (Voice0Disable[7]) {
                ssdatst[0xD0] |= BIT(7);
            }

            ssdatst[0xD1] = 1; // 000D1h - Emulator used to dump .spc file
            memset(ssdatst + 0xD2, 0, 46); // 000D2h-000FFh - Reserved

            fwrite(ssdatst, 1, sizeof(ssdatst), fp);
            fwrite(SPCRAM, 1, 65536, fp); // 00100h-100FFh - SPCRam
            fwrite(DSPMem, 1, 192, fp); // 10100h-101FFh - DSPRam
            fwrite(spcextraram, 1, 64, fp); // Seems DSPRam is split in two, but I don't get what's going on here
            fclose(fp);

            ResetState();

            sprintf(spcsaved, "%s FILE SAVED.", ZSaveName + fname_len - 3);
        }
    }
}

void SaveGameSpecificInput()
{
    if (!*ZSaveName) {
        psr_cfg_run(write_input_vars, ZCfgPath, "zinput.cfg");
    }

    if (GameSpecificInput && *ZSaveName) {
        setextension(ZSaveName, "inp");
        psr_cfg_run(write_input_vars, ZInpPath, ZSaveName);
    }
}

void LoadGameSpecificInput()
{
    if (GameSpecificInput && *ZSaveName) {
        psr_cfg_run(read_input_vars, ZCfgPath, "zinput.cfg");

        setextension(ZSaveName, "inp");
        psr_cfg_run(read_input_vars, ZInpPath, ZSaveName);
    }
}
