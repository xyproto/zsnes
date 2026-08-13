/* Differential test: the mosaic pass of video/newgfx16.asm against the C port
 * in video/c_ngmosaic.c.
 *
 * domosaicng16b and the sixty mosdraw* routines it used to jump into sit
 * behind one entry point, so one comparison covers all of them. Both sides are
 * real assembly - the oracle from the pre-port revision and the current file
 * from the working tree - so the seam thunk is tested too.
 *
 * The pass reads one scratch line and smears each block across the video line,
 * so the buffer it writes is compared directly; a checksum over the whole of
 * it catches a write that lands outside the two windows the pass should touch.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

extern u1 curmosaicsz;
extern u1 BGMA[256], BGMS1[], FillSubScr[256], scadtng[256];
extern u4 mosclineval, mostranspval;
extern u4 UnusedBit[2], UnusedBitXor[2];
extern u1* pesimpng;
extern u2 xtravbuf[288];

/* The sub screen sits 75036 words into the same buffer. */
#define MOS_SUB 75036u
#define VBSZ (MOS_SUB * 2u + 4096u)
#define WIN 1024u

static u1 vbuf[VBSZ];

void asm_domosaicng16b(void);
void cur_domosaicng16b(void);

/* The oracle still dispatches through the four jump tables, which cpu/table.c
   used to fill and no longer does - nothing reads them now that the pass is C.
   Filling them with the oracle's own renamed entry points is what lets the
   pre-port side run at all. */
extern void (*mosjmptab16b[15])(void);
void asm_mosdraw216b(void);
void asm_mosdraw316b(void);
void asm_mosdraw416b(void);
void asm_mosdraw516b(void);
void asm_mosdraw616b(void);
void asm_mosdraw716b(void);
void asm_mosdraw816b(void);
void asm_mosdraw916b(void);
void asm_mosdraw1016b(void);
void asm_mosdraw1116b(void);
void asm_mosdraw1216b(void);
void asm_mosdraw1316b(void);
void asm_mosdraw1416b(void);
void asm_mosdraw1516b(void);
void asm_mosdraw1616b(void);
extern void (*mosjmptab16bt[15])(void);
void asm_mosdraw216bt(void);
void asm_mosdraw316bt(void);
void asm_mosdraw416bt(void);
void asm_mosdraw516bt(void);
void asm_mosdraw616bt(void);
void asm_mosdraw716bt(void);
void asm_mosdraw816bt(void);
void asm_mosdraw916bt(void);
void asm_mosdraw1016bt(void);
void asm_mosdraw1116bt(void);
void asm_mosdraw1216bt(void);
void asm_mosdraw1316bt(void);
void asm_mosdraw1416bt(void);
void asm_mosdraw1516bt(void);
void asm_mosdraw1616bt(void);
extern void (*mosjmptab16btms[15])(void);
void asm_mosdraw216btms(void);
void asm_mosdraw316btms(void);
void asm_mosdraw416btms(void);
void asm_mosdraw516btms(void);
void asm_mosdraw616btms(void);
void asm_mosdraw716btms(void);
void asm_mosdraw816btms(void);
void asm_mosdraw916btms(void);
void asm_mosdraw1016btms(void);
void asm_mosdraw1116btms(void);
void asm_mosdraw1216btms(void);
void asm_mosdraw1316btms(void);
void asm_mosdraw1416btms(void);
void asm_mosdraw1516btms(void);
void asm_mosdraw1616btms(void);
extern void (*mosjmptab16bntms[15])(void);
void asm_mosdraw216bntms(void);
void asm_mosdraw316bntms(void);
void asm_mosdraw416bntms(void);
void asm_mosdraw516bntms(void);
void asm_mosdraw616bntms(void);
void asm_mosdraw716bntms(void);
void asm_mosdraw816bntms(void);
void asm_mosdraw916bntms(void);
void asm_mosdraw1016bntms(void);
void asm_mosdraw1116bntms(void);
void asm_mosdraw1216bntms(void);
void asm_mosdraw1316bntms(void);
void asm_mosdraw1416bntms(void);
void asm_mosdraw1516bntms(void);
void asm_mosdraw1616bntms(void);

static void fill_tables(void)
{
    mosjmptab16b[0] = asm_mosdraw216b;
    mosjmptab16b[1] = asm_mosdraw316b;
    mosjmptab16b[2] = asm_mosdraw416b;
    mosjmptab16b[3] = asm_mosdraw516b;
    mosjmptab16b[4] = asm_mosdraw616b;
    mosjmptab16b[5] = asm_mosdraw716b;
    mosjmptab16b[6] = asm_mosdraw816b;
    mosjmptab16b[7] = asm_mosdraw916b;
    mosjmptab16b[8] = asm_mosdraw1016b;
    mosjmptab16b[9] = asm_mosdraw1116b;
    mosjmptab16b[10] = asm_mosdraw1216b;
    mosjmptab16b[11] = asm_mosdraw1316b;
    mosjmptab16b[12] = asm_mosdraw1416b;
    mosjmptab16b[13] = asm_mosdraw1516b;
    mosjmptab16b[14] = asm_mosdraw1616b;
    mosjmptab16bt[0] = asm_mosdraw216bt;
    mosjmptab16bt[1] = asm_mosdraw316bt;
    mosjmptab16bt[2] = asm_mosdraw416bt;
    mosjmptab16bt[3] = asm_mosdraw516bt;
    mosjmptab16bt[4] = asm_mosdraw616bt;
    mosjmptab16bt[5] = asm_mosdraw716bt;
    mosjmptab16bt[6] = asm_mosdraw816bt;
    mosjmptab16bt[7] = asm_mosdraw916bt;
    mosjmptab16bt[8] = asm_mosdraw1016bt;
    mosjmptab16bt[9] = asm_mosdraw1116bt;
    mosjmptab16bt[10] = asm_mosdraw1216bt;
    mosjmptab16bt[11] = asm_mosdraw1316bt;
    mosjmptab16bt[12] = asm_mosdraw1416bt;
    mosjmptab16bt[13] = asm_mosdraw1516bt;
    mosjmptab16bt[14] = asm_mosdraw1616bt;
    mosjmptab16btms[0] = asm_mosdraw216btms;
    mosjmptab16btms[1] = asm_mosdraw316btms;
    mosjmptab16btms[2] = asm_mosdraw416btms;
    mosjmptab16btms[3] = asm_mosdraw516btms;
    mosjmptab16btms[4] = asm_mosdraw616btms;
    mosjmptab16btms[5] = asm_mosdraw716btms;
    mosjmptab16btms[6] = asm_mosdraw816btms;
    mosjmptab16btms[7] = asm_mosdraw916btms;
    mosjmptab16btms[8] = asm_mosdraw1016btms;
    mosjmptab16btms[9] = asm_mosdraw1116btms;
    mosjmptab16btms[10] = asm_mosdraw1216btms;
    mosjmptab16btms[11] = asm_mosdraw1316btms;
    mosjmptab16btms[12] = asm_mosdraw1416btms;
    mosjmptab16btms[13] = asm_mosdraw1516btms;
    mosjmptab16btms[14] = asm_mosdraw1616btms;
    mosjmptab16bntms[0] = asm_mosdraw216bntms;
    mosjmptab16bntms[1] = asm_mosdraw316bntms;
    mosjmptab16bntms[2] = asm_mosdraw416bntms;
    mosjmptab16bntms[3] = asm_mosdraw516bntms;
    mosjmptab16bntms[4] = asm_mosdraw616bntms;
    mosjmptab16bntms[5] = asm_mosdraw716bntms;
    mosjmptab16bntms[6] = asm_mosdraw816bntms;
    mosjmptab16bntms[7] = asm_mosdraw916bntms;
    mosjmptab16bntms[8] = asm_mosdraw1016bntms;
    mosjmptab16bntms[9] = asm_mosdraw1116bntms;
    mosjmptab16bntms[10] = asm_mosdraw1216bntms;
    mosjmptab16bntms[11] = asm_mosdraw1316bntms;
    mosjmptab16bntms[12] = asm_mosdraw1416bntms;
    mosjmptab16bntms[13] = asm_mosdraw1516bntms;
    mosjmptab16bntms[14] = asm_mosdraw1616bntms;
}

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
    u4 reg[7];
    u4 sum;
    u1 main[WIN];
    u1 sub[WIN];
    u1 scratch[576];
} snapshot;

static u4 checksum(void)
{
    u4 s = 0;

    for (u4 i = 0; i < VBSZ; i += 4) {
        s = s * 31u + *(u4 const*)(vbuf + i);
    }
    return s;
}

static void run(void (*fn)(void), snapshot const* const in,
    snapshot* const out)
{
    memset(vbuf, 0xCC, VBSZ);
    memcpy(vbuf, in->main, WIN);
    memcpy(vbuf + MOS_SUB * 2u, in->sub, WIN);
    memcpy(xtravbuf, in->scratch, 576);
    pesimpng = vbuf;

    rg_eax = in->reg[0];
    rg_ebx = in->reg[1];
    rg_ecx = in->reg[2];
    rg_edx = in->reg[3];
    rg_esi = in->reg[4];
    rg_edi = in->reg[5];
    rg_ebp = in->reg[6];
    rg_fn = (u4)(uintptr_t)fn;
    dt_call();

    out->reg[0] = rg_eax;
    out->reg[1] = rg_ebx;
    out->reg[2] = rg_ecx;
    out->reg[3] = rg_edx;
    out->reg[4] = rg_esi;
    out->reg[5] = rg_edi;
    out->reg[6] = rg_ebp;
    out->sum = checksum();
    memcpy(out->main, vbuf, WIN);
    memcpy(out->sub, vbuf + MOS_SUB * 2u, WIN);
    memcpy(out->scratch, xtravbuf, 576);
}

int main(void)
{
    long cov[6];
    memset(cov, 0, sizeof cov);
    fill_tables();

    DT_MAIN(20260813, 20000)
    {
        snapshot in, x, y;
        u4 line;
        int route;

        /* 2..16 draws; anything else returns at once. */
        curmosaicsz = (u1)(dt_mod(6) ? dt_mod(15) + 2 : dt_u32());
        line = dt_mod(256);
        mosclineval = line;
        /* Only the low byte is read, as the screen-enable bit for this
           layer. */
        mostranspval = dt_mod(2) ? 1u << dt_mod(8) : dt_u32();
        UnusedBit[0] = dt_mod(2) ? 0x8000u : dt_u32();
        UnusedBitXor[0] = dt_mod(2) ? 0x7FFFu : dt_u32();
        dt_fill(BGMA, 256);
        /* Mode 7 is the one value that forces the plain variant. */
        BGMA[line] = (u1)(dt_mod(4) ? dt_mod(7) : 7);
        dt_fill(BGMS1, 512);
        dt_fill(FillSubScr, 256);
        dt_fill(scadtng, 256);
        for (u4 i = 0; i < 256; i++) {
            /* The three tests below are all bit tests against mostranspval,
               so they have to come out both ways often. */
            FillSubScr[i] = (u1)(dt_mod(4) ? 1 : 0);
            BGMS1[i * 2] = (u1)(dt_mod(4) ? (u1)mostranspval : 0);
            BGMS1[i * 2 + 1] = (u1)(dt_mod(2) ? (u1)mostranspval : 0);
            scadtng[i] = (u1)(dt_mod(2) ? (u1)mostranspval : 0);
        }

        /* 0xFFFF means "nothing here", and it is the common case. */
        dt_fill(in.scratch, 576);
        for (u4 i = 0; i < 576; i += 2) {
            if (dt_mod(3) == 0) {
                in.scratch[i] = 0xFF;
                in.scratch[i + 1] = 0xFF;
            }
        }
        dt_fill(in.main, WIN);
        dt_fill(in.sub, WIN);
        for (u4 i = 0; i < 7; i++) {
            in.reg[i] = dt_u32();
        }

        run(asm_domosaicng16b, &in, &x);
        run(cur_domosaicng16b, &in, &y);

        if (curmosaicsz > 16 || curmosaicsz <= 1) {
            route = 5;
        } else if (BGMA[line] == 7) {
            route = 0;
        } else if (!(BGMS1[line * 2] & (u1)mostranspval)) {
            route = (FillSubScr[line] & 1) ? 4 : 0;
        } else if (!(FillSubScr[line] & 1)) {
            route = 0;
        } else if (!(BGMS1[line * 2 + 1] & (u1)mostranspval)) {
            route = (scadtng[line] & (u1)mostranspval) ? 1 : 0;
        } else {
            route = (scadtng[line] & (u1)mostranspval) ? 2 : 3;
        }
        cov[route]++;

        DT_EQ("eax", x.reg[0], y.reg[0]);
        DT_EQ("ebx", x.reg[1], y.reg[1]);
        DT_EQ("ecx", x.reg[2], y.reg[2]);
        DT_EQ("edx", x.reg[3], y.reg[3]);
        DT_EQ("esi", x.reg[4], y.reg[4]);
        DT_EQ("edi", x.reg[5], y.reg[5]);
        DT_EQ("ebp", x.reg[6], y.reg[6]);
        DT_EQ("buffer checksum", x.sum, y.sum);
        DT_MEM("main screen", x.main, y.main, WIN);
        DT_MEM("sub screen", x.sub, y.sub, WIN);
        DT_MEM("scratch line", x.scratch, y.scratch, 576);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ route=%d size=%u line=%u transp=%02x\n", route,
                curmosaicsz, line, (u1)mostranspval);
        }
    }
    printf("  plain=%ld t=%ld tms=%ld ntms=%ld subbias=%ld skip=%ld\n", cov[0],
        cov[1], cov[2], cov[3], cov[4], cov[5]);
    DT_DONE("newgfx16 mosaic pass");
}
