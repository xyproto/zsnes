#include "c_mode716.h"
#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof *(x))
#endif
#include "../types.h"
#include "../ui.h"

void Makemode7Table(void)
{
    for (u4 i = 0; i != lengthof(mode7tab); ++i) {
        mode7tab[i] = ((i & 0x07) << 4) + ((i >> 8 & 0x07) << 1) + 1;
    }
}
