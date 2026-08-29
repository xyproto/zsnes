/* The three gating macros from video/newg162.asm.
 *
 * Every routine in that file is a decision tree that picks one of about eight
 * parameterised bodies: transparent or not, then which combination of main and
 * sub windowing. These three macros are that tree. They each end by jumping to
 * a label passed in as a macro argument, so the C cannot simply be called - it
 * reports which branch to take in ng_branch and the seam jumps on that.
 *
 * They also adjust ecx (the window pointer) and edi (the output line), and
 * those adjustments have to survive: the seam is a pushad block, so writes into
 * it are what popad restores. */
#include <stdint.h>

#include "../types.h"

enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u1 BGMS1[], FillSubScr[], curmosaicsz;
/* A dword in video/newgfx.c, though the assembly only ever tested its low byte
   - which is all it is ever set to. */
extern u4 ngwinen;
extern zreg CMainWinScr, CSubWinScr;
extern u4 mosclineval, mostranspval;

/* 0 = fall through; otherwise which of the caller's labels to jump to. */
u4 ng_branch;

/* The sub screen sits this far into the same buffer, in pixels. */
#define NG_SUB (75036u * 2u)

void c_determinetransp(zreg* const r)
{
    u4 const bx = r[R_EBX];
    u1 const dl = (u1)r[R_EDX];

    mostranspval = dl;
    mosclineval = bx;
    r[R_ECX] += CMainWinScr;
    ng_branch = 0;

    if (curmosaicsz == 1) {
        if (BGMS1[bx * 2] & dl) {
            if (FillSubScr[bx] & 1)
                ng_branch = 1; /* the transparent variant */
            return;
        }
        if (FillSubScr[bx] & 1) {
            r[R_ECX] += CSubWinScr - CMainWinScr;
            r[R_EDI] += NG_SUB;
        }
        return;
    }
    /* mosaic */
    if (BGMS1[bx * 2] & dl)
        return;
    if (!(FillSubScr[bx] & 1))
        return;
    r[R_ECX] += CSubWinScr - CMainWinScr;
}

void c_checkwindowing(zreg* const r)
{
    ng_branch = ((u1)ngwinen != 0 && *(u1 const*)(uintptr_t)r[R_ECX] != 0) ? 1 : 0;
}

/* 1 = both windows, 2 = main only, 3 = sub only, 0 = no windowing. */
void c_determinewindow(zreg* const r)
{
    if ((u1)ngwinen == 0) {
        ng_branch = 0;
        return;
    }
    if (*(u1 const*)(uintptr_t)r[R_ECX] == 0) {
        ng_branch = 3;
        return;
    }
    r[R_ECX] += CSubWinScr - CMainWinScr;
    ng_branch = *(u1 const*)(uintptr_t)r[R_ECX] != 0 ? 1 : 2;
}
