#include "c_sfxproc.h"
#include "../ui.h"
#include "sfxproc.h"

void initsfxregsr(void)
{
    REGPTR(0x3000) = reg3000r;
    REGPTR(0x3001) = reg3001r;
    REGPTR(0x3002) = reg3002r;
    REGPTR(0x3003) = reg3003r;
    REGPTR(0x3004) = reg3004r;
    REGPTR(0x3005) = reg3005r;
    REGPTR(0x3006) = reg3006r;
    REGPTR(0x3007) = reg3007r;
    REGPTR(0x3008) = reg3008r;
    REGPTR(0x3009) = reg3009r;
    REGPTR(0x300A) = reg300Ar;
    REGPTR(0x300B) = reg300Br;
    REGPTR(0x300C) = reg300Cr;
    REGPTR(0x300D) = reg300Dr;
    REGPTR(0x300E) = reg300Er;
    REGPTR(0x300F) = reg300Fr;
    REGPTR(0x3010) = reg3010r;
    REGPTR(0x3011) = reg3011r;
    REGPTR(0x3012) = reg3012r;
    REGPTR(0x3013) = reg3013r;
    REGPTR(0x3014) = reg3014r;
    REGPTR(0x3015) = reg3015r;
    REGPTR(0x3016) = reg3016r;
    REGPTR(0x3017) = reg3017r;
    REGPTR(0x3018) = reg3018r;
    REGPTR(0x3019) = reg3019r;
    REGPTR(0x301A) = reg301Ar;
    REGPTR(0x301B) = reg301Br;
    REGPTR(0x301C) = reg301Cr;
    REGPTR(0x301D) = reg301Dr;
    REGPTR(0x301E) = reg301Er;
    REGPTR(0x301F) = reg301Fr;
    REGPTR(0x3030) = reg3030r;
    REGPTR(0x3031) = reg3031r;
    REGPTR(0x3032) = reg3032r;
    REGPTR(0x3033) = reg3033r;
    REGPTR(0x3034) = reg3034r;
    REGPTR(0x3035) = reg3035r;
    REGPTR(0x3036) = reg3036r;
    REGPTR(0x3037) = reg3037r;
    REGPTR(0x3038) = reg3038r;
    REGPTR(0x3039) = reg3039r;
    REGPTR(0x303A) = reg303Ar;
    REGPTR(0x303B) = reg303Br;
    REGPTR(0x303C) = reg303Cr;
    REGPTR(0x303D) = reg303Dr;
    REGPTR(0x303E) = reg303Er;
    REGPTR(0x303F) = reg303Fr;

    // set 3100-31FF to cacheregr
    eop** i = &REGPTR(0x3100);
    do
        *i = cacheregr;
    while (++i != &REGPTR(0x3300)); // XXX code and comment disagree: 0x31FF vs. 0x32FF
}

void initsfxregsw(void)
{
    REGPTW(0x3000) = reg3000w;
    REGPTW(0x3001) = reg3001w;
    REGPTW(0x3002) = reg3002w;
    REGPTW(0x3003) = reg3003w;
    REGPTW(0x3004) = reg3004w;
    REGPTW(0x3005) = reg3005w;
    REGPTW(0x3006) = reg3006w;
    REGPTW(0x3007) = reg3007w;
    REGPTW(0x3008) = reg3008w;
    REGPTW(0x3009) = reg3009w;
    REGPTW(0x300A) = reg300Aw;
    REGPTW(0x300B) = reg300Bw;
    REGPTW(0x300C) = reg300Cw;
    REGPTW(0x300D) = reg300Dw;
    REGPTW(0x300E) = reg300Ew;
    REGPTW(0x300F) = reg300Fw;
    REGPTW(0x3010) = reg3010w;
    REGPTW(0x3011) = reg3011w;
    REGPTW(0x3012) = reg3012w;
    REGPTW(0x3013) = reg3013w;
    REGPTW(0x3014) = reg3014w;
    REGPTW(0x3015) = reg3015w;
    REGPTW(0x3016) = reg3016w;
    REGPTW(0x3017) = reg3017w;
    REGPTW(0x3018) = reg3018w;
    REGPTW(0x3019) = reg3019w;
    REGPTW(0x301A) = reg301Aw;
    REGPTW(0x301B) = reg301Bw;
    REGPTW(0x301C) = reg301Cw;
    REGPTW(0x301D) = reg301Dw;
    REGPTW(0x301E) = reg301Ew;
    REGPTW(0x301F) = reg301Fw;
    REGPTW(0x3030) = reg3030w;
    REGPTW(0x3031) = reg3031w;
    REGPTW(0x3032) = reg3032w;
    REGPTW(0x3033) = reg3033w;
    REGPTW(0x3034) = reg3034w;
    REGPTW(0x3035) = reg3035w;
    REGPTW(0x3036) = reg3036w;
    REGPTW(0x3037) = reg3037w;
    REGPTW(0x3038) = reg3038w;
    REGPTW(0x3039) = reg3039w;
    REGPTW(0x303A) = reg303Aw;
    REGPTW(0x303B) = reg303Bw;
    REGPTW(0x303C) = reg303Cw;
    REGPTW(0x303D) = reg303Dw;
    REGPTW(0x303E) = reg303Ew;
    REGPTW(0x303F) = reg303Fw;

    // set 3100-31FF to cacheregw
    eop** i = &REGPTW(0x3100);
    do
        *i = cacheregw;
    while (++i != &REGPTW(0x3300)); // XXX code and comment disagree: 0x31FF vs. 0x32FF
}
