#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "difftest.h"

typedef void LineFilter(u2*, u1*, u4, u4, u1*, u4);

LineFilter _2xSaILine;
LineFilter _2xSaISuper2xSaILine;
LineFilter _2xSaISuperEagleLine;
LineFilter asm_2xSaILine;
LineFilter asm_2xSaISuper2xSaILine;
LineFilter asm_2xSaISuperEagleLine;

#define MAX_WIDTH 64
#define MARGIN 4
#define STRIDE (MAX_WIDTH + 2 * MARGIN)
#define ROWS 4
#define SRC_PITCH (STRIDE * sizeof(u2))
#define DST_PITCH (MAX_WIDTH * 2 * sizeof(u2))

static u2 source[ROWS * STRIDE];
static u2 delta[ROWS * STRIDE];
static u1 initial[DST_PITCH * 2];
static u1 asm_dst[DST_PITCH * 2];
static u1 c_dst[DST_PITCH * 2];

static int check_filter(
    char const* const name, LineFilter* const assembly, LineFilter* const c)
{
    DT_MAIN(20260903, 30000)
    {
        u4 const width = 4 * (1 + dt_mod(MAX_WIDTH / 4));

        for (size_t i = 0; i < ROWS * STRIDE; i++) {
            do {
                source[i] = (u2)dt_u32();
            } while (source[i] == 0);
        }
        memset(delta, 0, sizeof delta);
        dt_fill(initial, sizeof initial);
        memcpy(asm_dst, initial, sizeof asm_dst);
        memcpy(c_dst, initial, sizeof c_dst);

        assembly(source + STRIDE + MARGIN,
            (u1*)(delta + STRIDE + MARGIN), SRC_PITCH, width, asm_dst,
            DST_PITCH);
        c(source + STRIDE + MARGIN, (u1*)(delta + STRIDE + MARGIN), SRC_PITCH,
            width, c_dst, DST_PITCH);

        DT_MEM(name, asm_dst, c_dst, sizeof asm_dst);
    }
    DT_DONE(name);
}

int main(void)
{
    if (check_filter("_2xSaILine", asm_2xSaILine, _2xSaILine))
        return 1;
    if (check_filter("_2xSaISuper2xSaILine", asm_2xSaISuper2xSaILine,
            _2xSaISuper2xSaILine))
        return 1;
    return check_filter("_2xSaISuperEagleLine", asm_2xSaISuperEagleLine,
        _2xSaISuperEagleLine);
}
