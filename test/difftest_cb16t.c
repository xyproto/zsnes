/* Differential test: clearback16t and clearback16ts in video/makev16t.asm
 * against the C port in video/c_mv16tclr.c.
 *
 * Pure pixel code, so unlike the scanline gates there is a real buffer to
 * compare rather than a branch choice. Both sides are the real assembly - the
 * oracle from the pre-port revision and the current file from the working tree
 * (tools/mkoracle.py --worktree) - so the seam thunk is covered too.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

extern u1 scaddtype;
extern u2 scrnon;
extern u4 pal16b[256];
extern u4 vesa2_clbit;
extern u1* curvidoffset;

/* transpbuf is read from +32 for 512 bytes, and the fulladd loops step two
   bytes while reading four, so the last read runs two past the end. */
#define TBSZ 1024u
#define VIDSZ 1024u
u1 transpbuf[TBSZ];
static u1 vidbuf[VIDSZ];
u2 fulladdtab[65537];

void asm_clearback16t(void), asm_clearback16ts(void);
void cur_clearback16t(void), cur_clearback16ts(void);

u4 rg_eax, rg_ebx, rg_ecx, rg_edx, rg_esi, rg_edi, rg_ebp, rg_fn;
__asm__(".pushsection .text\n"
        ".globl dt_call\n"
        "dt_call:\n"
        "  pushl %ebx\n  pushl %esi\n  pushl %edi\n  pushl %ebp\n"
        "  movl rg_eax, %eax\n"
        "  movl rg_ebx, %ebx\n"
        "  movl rg_ecx, %ecx\n"
        "  movl rg_edx, %edx\n"
        "  movl rg_esi, %esi\n"
        "  movl rg_edi, %edi\n"
        "  movl rg_ebp, %ebp\n"
        "  call *rg_fn\n"
        "  movl %eax, rg_eax\n"
        "  movl %ebx, rg_ebx\n"
        "  movl %ecx, rg_ecx\n"
        "  movl %edx, rg_edx\n"
        "  movl %esi, rg_esi\n"
        "  movl %edi, rg_edi\n"
        "  movl %ebp, rg_ebp\n"
        "  popl %ebp\n  popl %edi\n  popl %esi\n  popl %ebx\n"
        "  ret\n"
        ".popsection\n");
void dt_call(void);

typedef struct {
    u4 ax, bx, cx, dx, si, di, bp;
    u1 vid[VIDSZ];
} snapshot;

static void run(void (*fn)(void), u4 const* const in, u1 const* const vseed,
    snapshot* const out)
{
    memcpy(vidbuf, vseed, VIDSZ);
    rg_eax = in[0];
    rg_ebx = in[1];
    rg_ecx = in[2];
    rg_edx = in[3];
    rg_esi = in[4];
    rg_edi = in[5];
    rg_ebp = in[6];
    rg_fn = (u4)(uintptr_t)fn;
    dt_call();

    out->ax = rg_eax;
    out->bx = rg_ebx;
    out->cx = rg_ecx;
    out->dx = rg_edx;
    out->si = rg_esi;
    out->di = rg_edi;
    out->bp = rg_ebp;
    memcpy(out->vid, vidbuf, VIDSZ);
}

int main(void)
{
    long cov[2][5];
    memset(cov, 0, sizeof cov);
    vesa2_clbit = 0xF7DEu;
    for (u4 i = 0; i < 65536; i++) {
        fulladdtab[i] = (u2)(i * 5u + (i >> 3));
    }

    DT_MAIN(20260803, 60000)
    {
        snapshot x, y;
        u4 in[7];
        u1 vseed[VIDSZ];
        int const ts = (int)dt_mod(2);
        int route;

        scaddtype = (u1)(dt_mod(4) ? (u1)(0x20u | (dt_mod(2) ? 0x40u : 0)
                                         | (dt_mod(4) == 0 ? 0x80u : 0))
                                   : (u1)dt_u32());
        scrnon = (u2)(dt_mod(2) ? 0 : dt_u32());
        /* pal16b[0] is the backdrop; zero picks the straight-copy route, so
           make it common enough to reach. */
        pal16b[0] = dt_mod(3) == 0 ? 0 : (dt_u32() & 0xFFFFu);
        /* Transparent pixels take the other arm of the averaging loop. */
        for (u4 i = 0; i < TBSZ; i++) {
            transpbuf[i] = (u1)(dt_mod(3) ? dt_u32() : 0);
        }
        dt_fill(vseed, VIDSZ);
        /* Leave room at both ends: the loops walk 512 bytes forward. */
        curvidoffset = vidbuf + 128;

        for (u4 i = 0; i < 7; i++) {
            in[i] = dt_u32();
        }

        run(ts ? asm_clearback16ts : asm_clearback16t, in, vseed, &x);
        run(ts ? cur_clearback16ts : cur_clearback16t, in, vseed, &y);

        route = ts                                        ? 4
            : !(scaddtype & 0x20u)                        ? 0
            : (scaddtype & 0x80u)                         ? 4
            : ((scaddtype & 0x40u) && (scrnon >> 8) != 0) ? 1
            : pal16b[0] == 0                              ? 2
                                                          : 3;
        cov[ts][route < 5 ? route : 0]++;

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebp", x.bp, y.bp);
        DT_MEM("video buffer", x.vid, y.vid, VIDSZ);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s scaddtype=%02x scrnon=%04x pal0=%08x route=%d\n",
                ts ? "clearback16ts" : "clearback16t", scaddtype, scrnon,
                pal16b[0], route);
        }
    }
    printf("  clearback16t  backcopy=%ld avg=%ld subcopy=%ld fulladd=%ld\n",
        cov[0][0], cov[0][1], cov[0][2], cov[0][3]);
    printf("  via 16ts      from16t=%ld direct=%ld\n", cov[0][4], cov[1][4]);
    DT_DONE("makev16t clearback16t / clearback16ts");
}
