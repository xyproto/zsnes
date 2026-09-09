/* Differential test: the drawsprites16t family in video/makev16t.asm against
 * the C port in video/c_mv16tspr.c.
 *
 * Four routines - the plain and windowed forms, each with a full-add twin -
 * behind one entry point. The priority forms of this family are still
 * assembly, so the sprprifix branch at the top runs the same code on both
 * sides; it is driven anyway, to check the dispatch the thunk left behind.
 *
 * Unlike drawsprites16bt this family *reads* transpbuf and writes only the
 * video line, so a palette-12-and-up sprite is blended with whatever is
 * already there.
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

extern u1 sprprifix, cwinenabm, winonsp, csprbit, csprprlft, scaddtype;
extern u2 scrnon;
extern u1 sprclprio[4], winspdata[288], sprpriodata[288];
extern u1 transpbuf[];
extern u4 sprsingle, vesa2_clbit;
extern u4 pal16b[256], pal16bcl[256], pal16bxcl[256];
extern u1* curvidoffset;
extern SpriteInfo* currentobjptr;

/* Not in any of the emulator objects the oracle links, and a blanket stub
   would be far too small: the dword load reads one entry past the end. */
u2 fulladdtab[65537];

#define VIDSZ 1024u
#define TBSZ 1168u
#define NSPR 40u
static u1 vidbuf[VIDSZ];
static SpriteInfo objtab[NSPR];
static u1 pixels[NSPR][8];

void asm_drawsprites16t(void);
/* The port is C now (video/c_m716gate.c): it takes the registers in a
   struct, so it is driven directly rather than through dt_call. */
#include "../video/c_m716gate.h"

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
    u4 obj;
    u1 bit, left;
    u1 prio[288];
    u1 vid[VIDSZ];
    u1 tb[TBSZ];
} snapshot;

static void run_port(snapshot const* const in, snapshot* const out)
{
    m7regs r;

    memcpy(vidbuf, in->vid, VIDSZ);
    memcpy(transpbuf, in->tb, TBSZ);
    memcpy(sprpriodata, in->prio, sizeof sprpriodata);
    csprbit = in->bit;
    csprprlft = in->left;
    currentobjptr = objtab;
    curvidoffset = vidbuf;
    r.ax = in->reg[0];
    r.bx = in->reg[1];
    r.cx = in->reg[2];
    r.dx = in->reg[3];
    r.si = in->reg[4];
    r.di = in->reg[5];
    r.bp = in->reg[6];

    drawsprites16t(&r);

    rg_eax = r.ax;
    rg_ebx = r.bx;
    rg_ecx = r.cx;
    rg_edx = r.dx;
    rg_esi = r.si;
    rg_edi = r.di;
    rg_ebp = r.bp;

    out->reg[0] = rg_eax;
    out->reg[1] = rg_ebx;
    out->reg[2] = rg_ecx;
    out->reg[3] = rg_edx;
    out->reg[4] = rg_esi;
    out->reg[5] = rg_edi;
    out->reg[6] = rg_ebp;
    out->obj = (u4)(uintptr_t)currentobjptr - (u4)(uintptr_t)objtab;
    out->bit = csprbit;
    out->left = csprprlft;
    memcpy(out->prio, sprpriodata, sizeof sprpriodata);
    memcpy(out->vid, vidbuf, VIDSZ);
    memcpy(out->tb, transpbuf, TBSZ);
}

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
    out->obj = (u4)(uintptr_t)currentobjptr - (u4)(uintptr_t)objtab;
    out->bit = csprbit;
    out->left = csprprlft;
    memcpy(out->prio, sprpriodata, sizeof sprpriodata);
    memcpy(out->vid, vidbuf, VIDSZ);
    memcpy(out->tb, transpbuf, TBSZ);
}

int main(void)
{
    long cov[5];
    memset(cov, 0, sizeof cov);
    vesa2_clbit = 0xF7DEu;
    for (u4 i = 0; i < 65537; i++) {
        fulladdtab[i] = (u2)(i * 5u + (i >> 3));
    }

    DT_MAIN(20260805, 40000)
    {
        snapshot in, x, y;
        u4 count = dt_mod(3) ? dt_mod(NSPR - 8) + 1 : 1;
        int win, half, route;

        /* 1 takes the priority branch, which is still assembly on both sides;
           worth a few iterations to check the thunk left it reachable. */
        sprprifix = (u1)(dt_mod(8) == 0 ? 1 : dt_mod(2) * 2);
        cwinenabm = (u1)(dt_mod(2) ? 0x10u : dt_u32());
        winonsp = (u1)(dt_mod(3) ? dt_mod(2) : dt_u32());
        scaddtype = (u1)(dt_mod(4) ? dt_mod(4) << 6 : dt_u32());
        scrnon = (u2)(dt_mod(3) ? dt_mod(2) << 12 : dt_u32());
        sprsingle = dt_mod(3) == 0 ? 1 : dt_u32();
        dt_fill(sprclprio, sizeof sprclprio);
        dt_fill(winspdata, sizeof winspdata);
        for (u4 i = 0; i < 288; i++) {
            winspdata[i] = (u1)(dt_mod(3) ? 0 : winspdata[i]);
        }
        /* A palette entry is 16 bits, and the whole of it indexes fulladdtab
           after the add - wider values read far outside the table. The clipped
           tables are left unclipped on purpose: the full-add path does not
           mask them, and only an unmasked value can show that. */
        for (u4 i = 0; i < 256; i++) {
            pal16b[i] = dt_u32() & 0xFFFFu;
            pal16bcl[i] = dt_u32() & 0xFFFFu;
            pal16bxcl[i] = dt_u32() & 0xFFFFu;
        }

        for (u4 i = 0; i < NSPR; i++) {
            int const blank = dt_mod(6) == 0;

            objtab[i].x = (u2)(dt_mod(257) + 8);
            objtab[i].obj = pixels[i];
            /* Palette 12 and up is what selects the blending arms, so it has
               to be common; the assembly compares ch against 12*16. */
            objtab[i].pal = (u1)(dt_mod(2) ? dt_mod(64) + 192 : dt_u32());
            objtab[i].status = (u1)dt_u32();
            for (u4 j = 0; j < 8; j++) {
                objtab[i].obj[j] = blank ? 0 : (u1)(dt_mod(4) ? dt_u32() : 0);
            }
        }

        dt_fill(in.vid, VIDSZ);
        /* A transparent pixel underneath is the case the half-add path
           branches on, and it is the common one on real hardware - uniform
           random bytes produce a zero word about once in 65536. */
        dt_fill(in.tb, TBSZ);
        for (u4 i = 0; i < TBSZ; i += 2) {
            if (dt_mod(3) != 0) {
                in.tb[i] = 0;
                in.tb[i + 1] = 0;
            }
        }
        dt_fill(in.prio, sizeof in.prio);
        in.bit = (u1)(dt_mod(2) ? 1u << dt_mod(8) : dt_u32());
        in.left = (u1)dt_u32();
        for (u4 i = 0; i < 7; i++) {
            in.reg[i] = dt_u32();
        }
        in.reg[2] = (in.reg[2] & 0xFFFF0000u) | (u4)(dt_u32() & 0xFFu) << 8
            | count;
        in.reg[6] = dt_mod(4);

        run(asm_drawsprites16t, &in, &x);
        run_port(&in, &y);

        win = (cwinenabm & 0x10u) && winonsp != 0;
        half = (scaddtype & 0x40u) && (scrnon >> 8) != 0
            && !(scaddtype & 0x80u);
        if (sprprifix == 1) {
            route = 4;
        } else {
            route = win * 2 + !half;
        }
        cov[route]++;

        DT_EQ("eax", x.reg[0], y.reg[0]);
        DT_EQ("ebx", x.reg[1], y.reg[1]);
        DT_EQ("ecx", x.reg[2], y.reg[2]);
        DT_EQ("edx", x.reg[3], y.reg[3]);
        DT_EQ("esi", x.reg[4], y.reg[4]);
        DT_EQ("edi", x.reg[5], y.reg[5]);
        DT_EQ("ebp", x.reg[6], y.reg[6]);
        DT_EQ("currentobjptr", x.obj, y.obj);
        DT_EQ("csprbit", x.bit, y.bit);
        DT_EQ("csprprlft", x.left, y.left);
        DT_MEM("sprpriodata", x.prio, y.prio, sizeof x.prio);
        DT_MEM("video buffer", x.vid, y.vid, VIDSZ);
        DT_MEM("transpbuf", x.tb, y.tb, TBSZ);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ route=%d count=%u scaddtype=%02x scrnon=%04x\n", route,
                count, scaddtype, scrnon);
        }
    }
    printf("  half=%ld fulladd=%ld winhalf=%ld winfulladd=%ld prio=%ld\n",
        cov[0], cov[1], cov[2], cov[3], cov[4]);
    DT_DONE("makev16t drawsprites16t family");
}
