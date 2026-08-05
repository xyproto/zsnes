/* Differential test: the drawsprites16bt family in video/makev16t.asm against
 * the C port in video/c_mv16tspr.c.
 *
 * Four assembly routines behind one entry point, so one comparison covers all
 * of them. Both sides are real assembly - the oracle from the pre-port
 * revision and the current file from the working tree - so the seam thunk is
 * tested too.
 *
 * Every pixel is written twice, to the video line and to the transparency
 * buffer, and the priority form also mutates sprpriodata, csprbit and
 * csprprlft. All of that is reseeded before each run and compared after.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

typedef struct SpriteInfo {
    u2 x;
    u1* obj __attribute__((packed, aligned(2)));
    u1 pal;
    u1 status;
} SpriteInfo;

extern u1 sprprifix, cwinenabm, winonsp, csprbit, csprprlft;
extern u1 sprclprio[4], winspdata[288], sprpriodata[288];
extern u1 transpbuf[];
extern u4 sprsingle;
extern u4 pal16b[256];
extern u1* curvidoffset;
extern SpriteInfo* currentobjptr;

#define VIDSZ 1024u
#define TBSZ 1168u
#define NSPR 40u
static u1 vidbuf[VIDSZ];
static SpriteInfo objtab[NSPR];
static u1 pixels[NSPR][8];

void asm_drawsprites16bt(void);
void cur_drawsprites16bt(void);

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
    u1 bit, left;
    u1 prio[288];
    u1 vid[VIDSZ];
    u1 tb[TBSZ];
} snapshot;

static void run(void (*fn)(void), snapshot const* const in,
    snapshot* const out)
{
    memcpy(vidbuf, in->vid, VIDSZ);
    memcpy(transpbuf, in->tb, TBSZ);
    memcpy(sprpriodata, in->prio, sizeof sprpriodata);
    csprbit = in->bit;
    csprprlft = in->left;
    currentobjptr = objtab;
    curvidoffset = vidbuf;
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
    out->bit = csprbit;
    out->left = csprprlft;
    memcpy(out->prio, sprpriodata, sizeof sprpriodata);
    memcpy(out->vid, vidbuf, VIDSZ);
    memcpy(out->tb, transpbuf, TBSZ);
}

int main(void)
{
    long cov[4];
    memset(cov, 0, sizeof cov);

    DT_MAIN(20260805, 40000)
    {
        snapshot in, x, y;
        u4 objx, objy;
        u4 count = dt_mod(3) ? dt_mod(NSPR - 8) + 1 : 1;
        u4 ebp = dt_mod(4);
        int route;

        sprprifix = (u1)(dt_mod(2) ? 1 : dt_mod(3));
        cwinenabm = (u1)(dt_mod(2) ? 0x10u : dt_u32());
        winonsp = (u1)(dt_mod(3) ? dt_mod(2) : dt_u32());
        sprsingle = dt_mod(3) == 0 ? 1 : dt_u32();
        dt_fill(sprclprio, sizeof sprclprio);
        if (dt_mod(4)) {
            sprclprio[ebp] = (u1)(dt_mod(4) + 1); /* keep the prio path live */
        }
        dt_fill(winspdata, sizeof winspdata);
        for (u4 i = 0; i < 288; i++) {
            /* Mostly-open windows, so the masked arm is not the only one. */
            winspdata[i] = (u1)(dt_mod(3) ? 0 : winspdata[i]);
        }
        for (u4 i = 0; i < 256; i++) {
            pal16b[i] = dt_u32();
        }

        /* x is bounded the way the sprite engine bounds it: the pixel index
           reaches x+15 in sprpriodata and winspdata, both 288 bytes. */
        for (u4 i = 0; i < NSPR; i++) {
            /* A sprite that draws nothing at all is what leaves the scratch
               registers holding what the prologue put there, so it has to be
               common enough to happen for every sprite in a short line. */
            int const blank = dt_mod(6) == 0;

            objtab[i].x = (u2)(dt_mod(257) + 8);
            objtab[i].obj = pixels[i];
            objtab[i].pal = (u1)dt_u32();
            objtab[i].status = (u1)dt_u32();
            for (u4 j = 0; j < 8; j++) {
                /* Zero and low-nibble-only pixels are both skips, but the two
                   families skip them by different tests. */
                if (blank) {
                    objtab[i].obj[j] = 0;
                } else if (dt_mod(4) != 0) {
                    objtab[i].obj[j] = (u1)dt_u32();
                } else {
                    objtab[i].obj[j]
                        = (u1)(dt_mod(2) ? 0 : (dt_mod(15) + 1) << 4);
                }
            }
        }

        dt_fill(in.vid, VIDSZ);
        dt_fill(in.tb, TBSZ);
        dt_fill(in.prio, sizeof in.prio);
        in.bit = (u1)(dt_mod(2) ? 1u << dt_mod(8) : dt_u32());
        in.left = (u1)dt_u32();
        for (u4 i = 0; i < 7; i++) {
            in.reg[i] = dt_u32();
        }
        in.reg[2] = (in.reg[2] & 0xFFFF0000u) | (u4)(dt_u32() & 0xFFu) << 8
            | count;
        in.reg[6] = ebp;

        run(asm_drawsprites16bt, &in, &x);
        objx = (u4)(uintptr_t)currentobjptr - (u4)(uintptr_t)objtab;
        run(cur_drawsprites16bt, &in, &y);
        objy = (u4)(uintptr_t)currentobjptr - (u4)(uintptr_t)objtab;

        if (sprprifix != 1) {
            route = (cwinenabm & 0x10u) && winonsp != 0 ? 1 : 0;
        } else {
            route = sprclprio[ebp] == 0 ? 2 : 3;
        }
        cov[route]++;

        DT_EQ("eax", x.reg[0], y.reg[0]);
        DT_EQ("ebx", x.reg[1], y.reg[1]);
        DT_EQ("ecx", x.reg[2], y.reg[2]);
        DT_EQ("edx", x.reg[3], y.reg[3]);
        DT_EQ("esi", x.reg[4], y.reg[4]);
        DT_EQ("edi", x.reg[5], y.reg[5]);
        DT_EQ("ebp", x.reg[6], y.reg[6]);
        DT_EQ("csprbit", x.bit, y.bit);
        DT_EQ("csprprlft", x.left, y.left);
        DT_EQ("currentobjptr", objx, objy);
        DT_MEM("sprpriodata", x.prio, y.prio, sizeof x.prio);
        DT_MEM("video buffer", x.vid, y.vid, VIDSZ);
        DT_MEM("transpbuf", x.tb, y.tb, TBSZ);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ route=%d count=%u ebp=%u sprsingle=%u csprbit=%02x\n",
                route, count, ebp, sprsingle, in.bit);
        }
    }
    printf("  plain=%ld winon=%ld prio-off=%ld prio=%ld\n", cov[0], cov[1],
        cov[2], cov[3]);
    DT_DONE("makev16t drawsprites16bt family");
}
