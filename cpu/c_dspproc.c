#include <string.h>

#include "../c_init.h"
#include "../cfg.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../init.h"
#include "../initc.h"
#include "../macros.h"
#include "../ui.h"
#include "c_dspproc.h"
#include "dsp.h"
#include "dspproc.h"
#include "firtable.h"
#include "regs.h"
#include "spc700.h"

#ifdef __MSDOS__
#include "../dos/c_sound.h"
#endif

static eop* paramhack[4];
static u4 SBToSPC = 22050;

static void conv2speed(u4 ecx, u4* esi, u4 const* edi)
{
    do
        *esi++ = (u8)*edi++ * SBToSPC / 11025U;
    while (--ecx != 0);
}

static u2 const Gaussian[] = {
    /**/ 1305, 1305, 1304, 1304, 1304, 1304, 1304, 1303,
    /**/ 1303, 1303, 1302, 1302, 1301, 1300, 1300, 1299,
    /**/ 1298, 1297, 1297, 1296, 1295, 1294, 1293, 1292,
    /**/ 1291, 1290, 1288, 1287, 1286, 1284, 1283, 1282,
    /**/ 1280, 1279, 1277, 1275, 1274, 1272, 1270, 1269,
    /**/ 1267, 1265, 1263, 1261, 1259, 1257, 1255, 1253,
    /**/ 1251, 1248, 1246, 1244, 1241, 1239, 1237, 1234,
    /**/ 1232, 1229, 1227, 1224, 1221, 1219, 1216, 1213,
    /**/ 1210, 1207, 1205, 1202, 1199, 1196, 1193, 1190,
    /**/ 1186, 1183, 1180, 1177, 1174, 1170, 1167, 1164,
    /**/ 1160, 1157, 1153, 1150, 1146, 1143, 1139, 1136,
    /**/ 1132, 1128, 1125, 1121, 1117, 1113, 1109, 1106,
    /**/ 1102, 1098, 1094, 1090, 1086, 1082, 1078, 1074,
    /**/ 1070, 1066, 1061, 1057, 1053, 1049, 1045, 1040,
    /**/ 1036, 1032, 1027, 1023, 1019, 1014, 1010, 1005,
    /**/ 1001, 997, 992, 988, 983, 978, 974, 969,
    /**/ 965, 960, 955, 951, 946, 941, 937, 932,
    /**/ 927, 923, 918, 913, 908, 904, 899, 894,
    /**/ 889, 884, 880, 875, 870, 865, 860, 855,
    /**/ 851, 846, 841, 836, 831, 826, 821, 816,
    /**/ 811, 806, 802, 797, 792, 787, 782, 777,
    /**/ 772, 767, 762, 757, 752, 747, 742, 737,
    /**/ 732, 728, 723, 718, 713, 708, 703, 698,
    /**/ 693, 688, 683, 678, 674, 669, 664, 659,
    /**/ 654, 649, 644, 640, 635, 630, 625, 620,
    /**/ 615, 611, 606, 601, 596, 592, 587, 582,
    /**/ 577, 573, 568, 563, 559, 554, 550, 545,
    /**/ 540, 536, 531, 527, 522, 517, 513, 508,
    /**/ 504, 499, 495, 491, 486, 482, 477, 473,
    /**/ 469, 464, 460, 456, 451, 447, 443, 439,
    /**/ 434, 430, 426, 422, 418, 414, 410, 405,
    /**/ 401, 397, 393, 389, 385, 381, 378, 374,
    /**/ 370, 366, 362, 358, 354, 351, 347, 343,
    /**/ 339, 336, 332, 328, 325, 321, 318, 314,
    /**/ 311, 307, 304, 300, 297, 293, 290, 286,
    /**/ 283, 280, 276, 273, 270, 267, 263, 260,
    /**/ 257, 254, 251, 248, 245, 242, 239, 236,
    /**/ 233, 230, 227, 224, 221, 218, 215, 212,
    /**/ 210, 207, 204, 201, 199, 196, 193, 191,
    /**/ 188, 186, 183, 180, 178, 175, 173, 171,
    /**/ 168, 166, 163, 161, 159, 156, 154, 152,
    /**/ 150, 147, 145, 143, 141, 139, 137, 134,
    /**/ 132, 130, 128, 126, 124, 122, 120, 118,
    /**/ 117, 115, 113, 111, 109, 107, 106, 104,
    /**/ 102, 100, 99, 97, 95, 94, 92, 90,
    /**/ 89, 87, 86, 84, 83, 81, 80, 78,
    /**/ 77, 76, 74, 73, 71, 70, 69, 67,
    /**/ 66, 65, 64, 62, 61, 60, 59, 58,
    /**/ 56, 55, 54, 53, 52, 51, 50, 49,
    /**/ 48, 47, 46, 45, 44, 43, 42, 41,
    /**/ 40, 39, 38, 37, 36, 36, 35, 34,
    /**/ 33, 32, 32, 31, 30, 29, 29, 28,
    /**/ 27, 27, 26, 25, 24, 24, 23, 23,
    /**/ 22, 21, 21, 20, 20, 19, 19, 18,
    /**/ 17, 17, 16, 16, 15, 15, 15, 14,
    /**/ 14, 13, 13, 12, 12, 11, 11, 11,
    /**/ 10, 10, 10, 9, 9, 9, 8, 8,
    /**/ 8, 7, 7, 7, 6, 6, 6, 6,
    /**/ 5, 5, 5, 5, 4, 4, 4, 4,
    /**/ 4, 3, 3, 3, 3, 3, 2, 2,
    /**/ 2, 2, 2, 2, 2, 1, 1, 1,
    /**/ 1, 1, 1, 1, 1, 1, 1, 1,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0
};

static u2 const CubicSpline[] = {
    /**/ 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, -1, -1, -1, -2, -2, -2, -3,
    /**/ -3, -4, -4, -5, -5, -6, -6, -7,
    /**/ -8, -8, -9, -10, -10, -11, -12, -13,
    /**/ -14, -14, -15, -16, -17, -18, -19, -20,
    /**/ -21, -22, -23, -24, -25, -26, -27, -28,
    /**/ -29, -30, -31, -32, -33, -34, -35, -37,
    /**/ -38, -39, -40, -41, -43, -44, -45, -46,
    /**/ -48, -49, -50, -51, -53, -54, -55, -56,
    /**/ -58, -59, -60, -62, -63, -64, -66, -67,
    /**/ -68, -70, -71, -72, -74, -75, -76, -78,
    /**/ -79, -80, -82, -83, -84, -86, -87, -88,
    /**/ -90, -91, -92, -93, -95, -96, -97, -99,
    /**/ -100, -101, -102, -104, -105, -106, -107, -109,
    /**/ -110, -111, -112, -113, -114, -116, -117, -118,
    /**/ -119, -120, -121, -122, -123, -124, -125, -126,
    /**/ -128, -128, -129, -130, -131, -132, -133, -134,
    /**/ -135, -136, -137, -137, -138, -139, -140, -141,
    /**/ -141, -142, -143, -143, -144, -144, -145, -146,
    /**/ -146, -147, -147, -148, -148, -148, -149, -149,
    /**/ -150, -150, -150, -150, -151, -151, -151, -151,
    /**/ -151, -151, -151, -151, -151, -151, -151, -151,
    /**/ -151, -151, -150, -150, -150, -149, -149, -149,
    /**/ -148, -148, -147, -147, -146, -146, -145, -144,
    /**/ -144, -143, -142, -141, -140, -139, -138, -137,
    /**/ -136, -135, -134, -133, -132, -130, -129, -128,
    /**/ -126, -125, -123, -122, -120, -119, -117, -115,
    /**/ -113, -112, -110, -108, -106, -104, -102, -100,
    /**/ -98, -95, -93, -91, -88, -86, -83, -81,
    /**/ -78, -76, -73, -70, -67, -65, -62, -59,
    /**/ -56, -53, -50, -46, -43, -40, -36, -33,
    /**/ -30, -26, -22, -19, -15, -11, -7, -3,

    /**/ 0, 4, 8, 12, 16, 21, 26, 30,
    /**/ 35, 40, 46, 51, 56, 62, 67, 73,
    /**/ 79, 85, 91, 97, 103, 109, 116, 122,
    /**/ 129, 136, 143, 149, 156, 164, 171, 178,
    /**/ 186, 193, 201, 208, 216, 224, 232, 240,
    /**/ 248, 256, 264, 273, 281, 289, 298, 307,
    /**/ 315, 324, 333, 342, 351, 360, 369, 378,
    /**/ 387, 397, 406, 415, 425, 435, 444, 454,
    /**/ 464, 473, 483, 493, 503, 513, 523, 533,
    /**/ 543, 553, 564, 574, 584, 594, 605, 615,
    /**/ 626, 636, 647, 657, 668, 679, 689, 700,
    /**/ 711, 721, 732, 743, 754, 765, 776, 787,
    /**/ 798, 808, 819, 830, 841, 852, 863, 874,
    /**/ 886, 897, 908, 919, 930, 941, 952, 963,
    /**/ 974, 985, 996, 1008, 1019, 1030, 1041, 1052,
    /**/ 1063, 1074, 1085, 1096, 1107, 1118, 1129, 1140,
    /**/ 1152, 1162, 1173, 1184, 1195, 1206, 1217, 1228,
    /**/ 1239, 1250, 1261, 1271, 1282, 1293, 1303, 1314,
    /**/ 1325, 1335, 1346, 1356, 1367, 1377, 1388, 1398,
    /**/ 1408, 1419, 1429, 1439, 1449, 1459, 1470, 1480,
    /**/ 1490, 1499, 1509, 1519, 1529, 1539, 1548, 1558,
    /**/ 1567, 1577, 1586, 1595, 1605, 1614, 1623, 1632,
    /**/ 1641, 1650, 1659, 1668, 1677, 1685, 1694, 1702,
    /**/ 1711, 1719, 1727, 1736, 1744, 1752, 1760, 1768,
    /**/ 1776, 1783, 1791, 1798, 1806, 1813, 1820, 1828,
    /**/ 1835, 1842, 1849, 1855, 1862, 1869, 1875, 1881,
    /**/ 1888, 1894, 1900, 1906, 1912, 1918, 1923, 1929,
    /**/ 1934, 1940, 1945, 1950, 1955, 1960, 1964, 1969,
    /**/ 1974, 1978, 1982, 1986, 1990, 1994, 1998, 2002,
    /**/ 2005, 2008, 2012, 2015, 2018, 2021, 2023, 2026,
    /**/ 2028, 2031, 2033, 2035, 2037, 2038, 2040, 2041,
    /**/ 2043, 2044, 2045, 2046, 2046, 2047, 2047, 2047,

    /**/ 2048, 2047, 2047, 2047, 2046, 2046, 2045, 2044,
    /**/ 2043, 2041, 2040, 2038, 2037, 2035, 2033, 2031,
    /**/ 2028, 2026, 2023, 2021, 2018, 2015, 2012, 2008,
    /**/ 2005, 2002, 1998, 1994, 1990, 1986, 1982, 1978,
    /**/ 1974, 1969, 1964, 1960, 1955, 1950, 1945, 1940,
    /**/ 1934, 1929, 1923, 1918, 1912, 1906, 1900, 1894,
    /**/ 1888, 1881, 1875, 1869, 1862, 1855, 1849, 1842,
    /**/ 1835, 1828, 1820, 1813, 1806, 1798, 1791, 1783,
    /**/ 1776, 1768, 1760, 1752, 1744, 1736, 1727, 1719,
    /**/ 1711, 1702, 1694, 1685, 1677, 1668, 1659, 1650,
    /**/ 1641, 1632, 1623, 1614, 1605, 1595, 1586, 1577,
    /**/ 1567, 1558, 1548, 1539, 1529, 1519, 1509, 1499,
    /**/ 1490, 1480, 1470, 1459, 1449, 1439, 1429, 1419,
    /**/ 1408, 1398, 1388, 1377, 1367, 1356, 1346, 1335,
    /**/ 1325, 1314, 1303, 1293, 1282, 1271, 1261, 1250,
    /**/ 1239, 1228, 1217, 1206, 1195, 1184, 1173, 1162,
    /**/ 1152, 1140, 1129, 1118, 1107, 1096, 1085, 1074,
    /**/ 1063, 1052, 1041, 1030, 1019, 1008, 996, 985,
    /**/ 974, 963, 952, 941, 930, 919, 908, 897,
    /**/ 886, 874, 863, 852, 841, 830, 819, 808,
    /**/ 798, 787, 776, 765, 754, 743, 732, 721,
    /**/ 711, 700, 689, 679, 668, 657, 647, 636,
    /**/ 626, 615, 605, 594, 584, 574, 564, 553,
    /**/ 543, 533, 523, 513, 503, 493, 483, 473,
    /**/ 464, 454, 444, 435, 425, 415, 406, 397,
    /**/ 387, 378, 369, 360, 351, 342, 333, 324,
    /**/ 315, 307, 298, 289, 281, 273, 264, 256,
    /**/ 248, 240, 232, 224, 216, 208, 201, 193,
    /**/ 186, 178, 171, 164, 156, 149, 143, 136,
    /**/ 129, 122, 116, 109, 103, 97, 91, 85,
    /**/ 79, 73, 67, 62, 56, 51, 46, 40,
    /**/ 35, 30, 26, 21, 16, 12, 8, 4,

    /**/ 0, -3, -7, -11, -15, -19, -22, -26,
    /**/ -30, -33, -36, -40, -43, -46, -50, -53,
    /**/ -56, -59, -62, -65, -67, -70, -73, -76,
    /**/ -78, -81, -83, -86, -88, -91, -93, -95,
    /**/ -98, -100, -102, -104, -106, -108, -110, -112,
    /**/ -113, -115, -117, -119, -120, -122, -123, -125,
    /**/ -126, -128, -129, -130, -132, -133, -134, -135,
    /**/ -136, -137, -138, -139, -140, -141, -142, -143,
    /**/ -144, -144, -145, -146, -146, -147, -147, -148,
    /**/ -148, -149, -149, -149, -150, -150, -150, -151,
    /**/ -151, -151, -151, -151, -151, -151, -151, -151,
    /**/ -151, -151, -151, -151, -151, -150, -150, -150,
    /**/ -150, -149, -149, -148, -148, -148, -147, -147,
    /**/ -146, -146, -145, -144, -144, -143, -143, -142,
    /**/ -141, -141, -140, -139, -138, -137, -137, -136,
    /**/ -135, -134, -133, -132, -131, -130, -129, -128,
    /**/ -128, -126, -125, -124, -123, -122, -121, -120,
    /**/ -119, -118, -117, -116, -114, -113, -112, -111,
    /**/ -110, -109, -107, -106, -105, -104, -102, -101,
    /**/ -100, -99, -97, -96, -95, -93, -92, -91,
    /**/ -90, -88, -87, -86, -84, -83, -82, -80,
    /**/ -79, -78, -76, -75, -74, -72, -71, -70,
    /**/ -68, -67, -66, -64, -63, -62, -60, -59,
    /**/ -58, -56, -55, -54, -53, -51, -50, -49,
    /**/ -48, -46, -45, -44, -43, -41, -40, -39,
    /**/ -38, -37, -35, -34, -33, -32, -31, -30,
    /**/ -29, -28, -27, -26, -25, -24, -23, -22,
    /**/ -21, -20, -19, -18, -17, -16, -15, -14,
    /**/ -14, -13, -12, -11, -10, -10, -9, -8,
    /**/ -8, -7, -6, -6, -5, -5, -4, -4,
    /**/ -3, -3, -2, -2, -2, -1, -1, -1,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0
};

static s4 DSPInterpolate_4(u4 const edx, u4 const ebp)
{
    u4 const ebx = BRRPlace0[ebp][0] >> 16 & 0xFF;
    u4 const eax = *(u4 const*)((u1 const*)&BRRPlace0[ebp][0] + 3); // XXX ugly cast
    s4 ecx = (s4)(s2)PSampleBuf[ebp][edx + 2] * (s4)DSPInterP[ebx + 256 * 3] + (s4)(s2)PSampleBuf[ebp][eax + 3] * (s2)DSPInterP[ebx + 256 * 2] + (s4)(s2)PSampleBuf[ebp][eax + 4] * (s2)DSPInterP[ebx + 256 * 1] + (s4)(s2)PSampleBuf[ebp][eax + 5] * (s2)DSPInterP[ebx + 256 * 0];

    ecx >>= 11;

    if (ecx < -32768)
        ecx = -32768;
    if (ecx > 32767)
        ecx = 32767;

    return ecx;
}

static s4 DSPInterpolate_8(u4 const edx, u4 const ebp)
{
    u8 const* const ebx = (u8 const*)&((u1 const*)fir_lut)[((BRRPlace0[ebp][0] & 0x00FFFFFF) + 0x1000) >> 9 & 0x0000FFF0]; // XXX ugly cast
    u8 const* const xxx = (u8 const*)&PSampleBuf[ebp][BRRPlace0[ebp][0] >> 24];
    s4 res;
    asm(
        "movq     %1, %%mm0\n\t"
        "packssdw %2, %%mm0\n\t"
        "movq     %3, %%mm1\n\t"
        "packssdw %4, %%mm1\n\t"
        "movq     %5, %%mm2\n\t"
        "movq     %6, %%mm3\n\t"
        "pmaddwd  %%mm2, %%mm0\n\t"
        "pmaddwd  %%mm3, %%mm1\n\t"
        "paddd    %%mm1, %%mm0\n\t"
        "movq     %%mm0, %%mm1\n\t"
        "psrlq    $32, %%mm0\n\t"
        "paddd    %%mm1, %%mm0\n\t"
        "psrad    $14, %%mm0\n\t"
        "packssdw %%mm0, %%mm0\n\t"
        "movd     %%mm0, %0"
        : "=r"(res)
        : "m"(xxx[0]), "m"(xxx[1]), "m"(xxx[2]), "m"(xxx[3]), "m"(ebx[0]), "m"(ebx[1])
        : "mm0", "mm1", "mm2", "mm3");
    return (s2)res;
}

static s4 DSPInterpolate_4_mmx(u4 const edx, u4 const ebp)
{
    u8 const* const eax = (u8 const*)&DSPInterP[(BRRPlace0[ebp][0] >> 16 & 0xFF) * 4];
    u8 const* const xxx = (u8 const*)&PSampleBuf[ebp][edx + 2];
    s4 res;
    asm(
        "movq     %1,    %%mm0\n\t"
        "packssdw %2,    %%mm0\n\t"
        "movq     %3,    %%mm1\n\t"
        "pmaddwd  %%mm1, %%mm0\n\t"
        "movq     %%mm0, %%mm1\n\t"
        "psrlq    $32,   %%mm0\n\t"
        "paddd    %%mm1, %%mm0\n\t"
        "psrad    $11,   %%mm0\n\t"
        "packssdw %%mm0, %%mm0\n\t"
        "movd     %%mm0, %0\n\t"
        "emms"
        : "=r"(res)
        : "m"(xxx[0]), "m"(xxx[1]), "m"(*eax)
        : "mm0", "mm1");
    return res;
}

void AdjustFrequency(void)
{
    u1 const ah = MMXSupport;
    u1 al = SoundInterpType;
    if (ah == 0) {
        if (LowPassFilterType >= 3)
            LowPassFilterType = 0; // HQ
        if (al >= 3) {
            al = 1;
            SoundInterpType = al;
        }
    }

    interpolatefunc* interpolate;
    switch (al) {
    case 0:
        interpolate = 0;
        break;

    case 1: // Gaussian
        // Copy from Gaussian to DSPInterP
#ifndef __MSDOS__
        // this ifndef is needed the workaround the "snow" in the DOS port
        // used only for Gaussian though
        if (ah == 0)
#endif
        {
            u2* ebx = DSPInterP + 512;
            u2* edx = DSPInterP + 511;
            u2 const* esi = Gaussian;
            u4 ecx = 512;
            do {
                u2 const ax = *esi++;
                *edx-- = ax;
                *ebx++ = ax;
            } while (--ecx != 0);
            interpolate = DSPInterpolate_4;
        }
#ifndef __MSDOS__
        else { // Gaussian MMX
            u2 const* ebx = Gaussian;
            u2 const* edx = Gaussian + 255;
            u2* esi = DSPInterP;
            u4 ecx = 256;
            do {
                *esi++ = ebx[256];
                *esi++ = ebx[0];
                *esi++ = edx[0];
                *esi++ = edx[256];
            } while (++ebx, --edx, --ecx != 0);
            interpolate = DSPInterpolate_4_mmx;
        }
#endif
        break;

    case 2: // Cubic spline
    { // Copy from CubicSpline to DSPInterP
        u2 const* ebx = CubicSpline;
        u2* esi = DSPInterP;
        if (ah == 0) {
            u4 ecx = 1024;
            do {
                u2 const ax = *ebx++;
                *esi++ = ax - ax / 8;
            } while (--ecx != 0);
            interpolate = DSPInterpolate_4;
        } else { // Cubix MMX
            u4 ecx = 256;
            do {
                u2 const v3 = ebx[256 * 3];
                *esi++ = v3 - v3 / 8;
                u2 const v2 = ebx[256 * 2];
                *esi++ = v2 - v2 / 8;
                u2 const v1 = ebx[256 * 1];
                *esi++ = v1 - v1 / 8;
                u2 const v0 = ebx[256 * 0];
                *esi++ = v0 - v0 / 8;
            } while (++ebx, --ecx != 0);
            interpolate = DSPInterpolate_4_mmx;
        }
        break;
    }

    default: // Fir MMX
        interpolate = DSPInterpolate_8;
        break;
    }
    DSPInterpolate = interpolate;

#ifdef __MSDOS__
    SB_quality_limiter();
#endif

#ifdef __MSDOS__
    static u4 const SBToSPCSpeeds[] = { 8000, 10989, 22222, 43478, 15874, 32258, 48000 };
    static u4 const SBToSPCSpeeds2[] = { 8192, 11289, 22579, 45158, 16384, 32768, 48000 };
#else
    static u4 const SBToSPCSpeeds[] = { 8000, 11025, 22050, 44100, 16000, 32000, 48000 };
#endif
    u4 const eax =
#ifdef __MSDOS__
        // code for supporting vibra cards (coded by Peter Santing)
        vibracard == 1 || SBHDMA != 0 ? SBToSPCSpeeds2[SoundQuality] : // Vibra card or 16 bit
#endif
        SBToSPCSpeeds[SoundQuality];
    SBToSPC = eax;
    dspPAdj = ((u8)32000 << 20) / eax;

    // Original values
    static u4 const EchoRateO[] = {
        /**/ 2, 172, 344, 517, 689, 861, 1033, 1205,
        /**/ 1378, 1550, 1722, 1895, 2067, 2239, 2412, 2584
    };
    static u4 const AttackRateO[] = {
        /**/ 45202, 28665, 16537, 11025, 7056, 4189, 2866, 1764,
        /**/ 1058, 705, 441, 264, 176, 110, 66, 4
    };
    static u4 const DecayRateO[] = {
        /**/ 13230, 8158, 4851, 2697, 2284, 1212, 815, 407
    };
    static u4 const SustainRateO[] = {
        /**/ 0xFFFFFFFF, 418950, 308700, 265600, 209475, 154350, 132300, 103635,
        /**/ 78277, 65047, 51817, 38587, 31972, 26460, 19845, 16537,
        /**/ 13230, 9702, 8158, 6504, 4851, 3879, 2697, 2050,
        /**/ 1572, 1212, 1014, 815, 606, 407, 202, 125
    };
    static u4 const IncreaseO[] = {
        /**/ 0xFFFFFFFF, 45202, 34177, 28665, 22050, 16537, 14332, 11025,
        /**/ 8489, 7056, 5622, 4189, 3528, 2866, 2094, 1764,
        /**/ 1433, 1058, 882, 705, 529, 441, 352, 264,
        /**/ 220, 176, 132, 110, 88, 66, 44, 22
    };
    static u4 const IncreaseBentO[] = {
        /**/ 0xFFFFFFFF, 79100, 59535, 50160, 38580, 28665, 25000, 19250,
        /**/ 14332, 12127, 9800, 7320, 6160, 4961, 3650, 3060,
        /**/ 2425, 1845, 1540, 1212, 920, 770, 614, 460,
        /**/ 383, 306, 229, 190, 152, 113, 75, 36
    };
    static u4 const DecreaseO[] = {
        /**/ 0xFFFFFFFF, 45202, 34177, 28665, 22050, 16537, 14332, 11025,
        /**/ 8489, 7056, 5622, 4189, 3528, 2866, 2094, 1764,
        /**/ 1433, 1058, 882, 705, 529, 441, 352, 264,
        /**/ 220, 176, 132, 110, 88, 66, 44, 22
    };
    static u4 const DecreaseRateExpO[] = {
        /**/ 0xFFFFFFFF, 418950, 308700, 264600, 209470, 154350, 132300, 103635,
        /**/ 78277, 65047, 51817, 38587, 31972, 26460, 19845, 16537,
        /**/ 13230, 9702, 8158, 6504, 4851, 4079, 3197, 2425,
        /**/ 1984, 1653, 1212, 1014, 815, 606, 407, 198
    };

    // Init all rates
    conv2speed(lengthof(EchoRateO), EchoRate, EchoRateO);
    conv2speed(lengthof(AttackRateO), AttackRate, AttackRateO);
    conv2speed(lengthof(DecayRateO), DecayRate, DecayRateO);
    conv2speed(lengthof(SustainRateO) - 1, SustainRate + 1, SustainRateO + 1);
    conv2speed(lengthof(IncreaseO) - 1, Increase + 1, IncreaseO + 1);
    conv2speed(lengthof(IncreaseBentO) - 1, IncreaseBent + 1, IncreaseBentO + 1);
    conv2speed(lengthof(DecreaseO) - 1, Decrease + 1, DecreaseO + 1);
    conv2speed(lengthof(DecreaseRateExpO) - 1, DecreaseRateExp + 1, DecreaseRateExpO + 1);
    for (u2* i = Voice0Pitch; i != endof(Voice0Pitch); ++i)
        *i = 0xFFFE;
}

void VoiceStart(u4 const voice)
{
    static u4 spc700temp[2];

    Voice0FirstBlock[voice] = 1;
    spc700temp[1] = 0;
    if (Voice0Status[voice] != 0) {
        spc700temp[0] = Voice0EnvInc[voice];
        spc700temp[1] = 1;
    }
    Voice0Status[voice] = 0;

    if (DSPMem[16 * voice] < 0x40 && DSPMem[16 * voice + 1] < 0x40 && *(u4 const*)&DSPMem[16 * voice + 4] == 0x0050FF07 && DSPMem[0x5D] == 6) { // Skip.
        DSPMem[16 * voice] = 15;
        DSPMem[16 * voice + 1] = 15;
        return;
    }

    // Check if adsr or gain
    if (DSPMem[16 * voice + 5] & 0x80) {
        // Calculate attack rate
        u4 const eax = DSPMem[16 * voice + 5] & 0x0F;
        if (eax != 0x0F) {
            u4 const ebx = AttackRate[eax];
            Voice0Time[voice] = ebx;
            Voice0IncNumber[voice] = 127 * 65536 / ebx;
            Voice0State[voice] = 8;
            Voice0EnvInc[voice] = 0;
            GainDecBendDataDat[voice] = 0x7F;
            Voice0Status[voice] = 1;
        } else {
            u4 const edx = DecayRate[DSPMem[16 * voice + 5] >> 4 & 0x07];
            u4 const ebx = SustainRate[DSPMem[16 * voice + 6] & 0x1F];
            if (edx < ebx) {
                // ebx = total sustain time
                /* Traverse through al entries in edx time, then through 64 - al entries
				 * in ebx - edx time. */
                u1 const al = AdsrSustLevLoc[DSPMem[16 * voice + 6] >> 5];
                u4 const eax = edx / al;
                AdsrBlocksLeft[voice] = al;
                Voice0Time[voice] = eax;
                GainDecBendDataTime[voice] = eax;
                AdsrNextTimeDepth[voice] = (ebx - edx) / (64 - al);
                Voice0EnvInc[voice] = 0x007FFFFF;
                GainDecBendDataPos[voice] = 0;
                GainDecBendDataDat[voice] = 127;
                Voice0IncNumber[voice] = -((127 - 122) * 65536 / eax);
                Voice0State[voice] = 9;
                Voice0Status[voice] = 1;
            } else { // Decay over.
                u4 const ebx_ = (ebx + (edx - ebx) * (DSPMem[18 * voice + 6] >> 5 ^ 0x07) / 7) / 32;
                Voice0EnvInc[voice] = 0x007FFFFF;
                Voice0Time[voice] = ebx_;
                GainDecBendDataTime[voice] = ebx_;
                GainDecBendDataPos[voice] = 0;
                GainDecBendDataDat[voice] = 127;
                Voice0IncNumber[voice] = -((127 - 118) * 65536 / ebx_);
                Voice0State[voice] = 7;
                Voice0Status[voice] = 1;
            }
        }
    } else { // Gain.
        if (!(DSPMem[16 * voice + 7] & 0x80)) { // Direct.
            Voice0EnvInc[voice] = (DSPMem[16 * voice + 7] & 0x7F) << 16;
            Voice0Time[voice] = 0xFFFFFFFF;
            Voice0IncNumber[voice] = 0;
            Voice0State[voice] = 4;
            Voice0Status[voice] = 1;
        } else if (DSPMem[16 * voice + 7] & 0x40) { // Increase.
            if (!(DSPMem[16 * voice + 7] & 0x20)) { // Linear Inc.
                u4 const ebx = Increase[DSPMem[16 * voice + 7] & 0x1F];
                Voice0EnvInc[voice] = 0;
                Voice0Time[voice] = ebx;
                Voice0IncNumber[voice] = (127 * 65536) / ebx;
                Voice0State[voice] = 3;
                Voice0Status[voice] = 1;
            } else {
                u4 const ebx = Increase[DSPMem[16 * voice + 7] & 0x1F];
                Voice0EnvInc[voice] = 0;
                Voice0Time[voice] = ebx - ebx / 4 - 1;
                Voice0IncNumber[voice] = (127 * 65536) / ebx;
                Voice0State[voice] = 6;
                Voice0Status[voice] = 1;
            }
        } else if (!(DSPMem[16 * voice + 7] & 0x20)) { // Linear Dec.
            u4 const ebx = Decrease[DSPMem[16 * voice + 7] & 0x1F];
            Voice0EnvInc[voice] = 0x007FFFFF;
            Voice0Time[voice] = ebx;
            Voice0IncNumber[voice] = -(127 * 65536 / ebx);
            Voice0State[voice] = 5;
            Voice0Status[voice] = 1;
        } else {
            u4 const ebx = DecreaseRateExp[DSPMem[16 * voice + 7] & 0x1F] / 32;
            Voice0EnvInc[voice] = 0x007FFFFF;
            Voice0Time[voice] = ebx;
            GainDecBendDataTime[voice] = ebx;
            GainDecBendDataPos[voice] = 0;
            GainDecBendDataDat[voice] = 127;
            Voice0IncNumber[voice] = -((127 - 118) * 65536 / ebx);
            Voice0State[voice] = 0;
            Voice0Status[voice] = 1;
        }
    }

    if (spc700temp[1] != 0) {
        TimeTemp[voice] = Voice0Time[voice];
        IncNTemp[voice] = Voice0IncNumber[voice];
        EnvITemp[voice] = Voice0EnvInc[voice];
        StatTemp[voice] = Voice0State[voice];
        u4 const eax = spc700temp[0];
        Voice0EnvInc[voice] = eax;
        Voice0Time[voice] = 127;
        Voice0IncNumber[voice] = -(eax / 128);
        Voice0State[voice] = 210;
    } else {
        u2 const ax = *(u2 const*)&DSPMem[16 * voice + 2];
        if (Voice0Pitch[voice] != ax) { // Pitchc.
            Voice0Pitch[voice] = ax;
            Voice0Freq[voice] = (u8)(ax & 0x3FFF) * dspPAdj >> 8;
            // modpitch
        }
        BRRPlace0[voice][0] = 0x10000000;
        Voice0Prev0[voice] = 0;
        Voice0Prev1[voice] = 0;
        Voice0End[voice] = 0;
        Voice0Loop[voice] = 0;
        PSampleBuf[voice][16] = 0;
        PSampleBuf[voice][17] = 0;
        PSampleBuf[voice][18] = 0;
        SoundLooped0[voice] = 0;
        echoon0[voice] = (DSPMem[0x4D] & 1U << voice) != 0; // Echo.
    }

    u2 const ax = (DSPMem[0x5D] * 64 + (*(u4 const*)&DSPMem[16 * voice + 4] & 0x000000FF)) * 4;
    Voice0Ptr[voice] = *(u2 const*)&SPCRAM[ax];
    Voice0LoopPtr[voice] = *(u2 const*)&SPCRAM[ax + 2];
}

void VoiceStarter(u1 const voice)
{
    Voice0Time[voice] = TimeTemp[voice];
    Voice0IncNumber[voice] = IncNTemp[voice];
    Voice0EnvInc[voice] = EnvITemp[voice];
    Voice0State[voice] = StatTemp[voice];

    SoundLooped0[voice] = 0;
    echoon0[voice] = (DSPMem[0x4D] & 1 << voice) != 0; // Echo.
    u2 const ax = (DSPMem[0x5D] * 64 + (*(u4 const*)&DSPMem[16 * voice + 4] & 0x000000FF)) * 4;
    Voice0Ptr[voice] = *(u2 const*)&SPCRAM[ax];
    Voice0LoopPtr[voice] = *(u2 const*)&SPCRAM[ax + 2];
    u2 const pitch = *(u2 const*)&DSPMem[16 * voice + 2];
    if (Voice0Pitch[voice] != pitch) { // Pitchc.
        Voice0Pitch[voice] = pitch;
        Voice0Freq[voice] = (u8)(pitch & 0x3FFF) * dspPAdj >> 8;
        // modpitch
    }
    BRRPlace0[voice][0] = 0x10000000;
    Voice0Prev0[voice] = 0;
    Voice0Prev1[voice] = 0;
    Voice0End[voice] = 0;
    Voice0Loop[voice] = 0;
    PSampleBuf[voice][16] = 0;
    PSampleBuf[voice][17] = 0;
    PSampleBuf[voice][18] = 0;
}

void InitSPC(void)
{
    AdjustFrequency();

    for (u4 i = 0; i != lengthof(VolumeConvTable); ++i)
        VolumeConvTable[i] = (s1)((i >> 8) * (i & 0xFFU) >> 7);

    memset(SPCRAM, 0, 0xEF);
    spcPCRam = SPCRAM + 0xFFC0;
    spcS = 0x1EF;
    spcRamDP = SPCRAM;
    spcX = 0;

    // initialize all the SPC write registers
    spcWptr[0x00] = SPCRegF0;
    spcWptr[0x01] = SPCRegF1;
    spcWptr[0x02] = SPCRegF2;
    spcWptr[0x03] = SPCRegF3;
    spcWptr[0x04] = SPCRegF4;
    spcWptr[0x05] = SPCRegF5;
    spcWptr[0x06] = SPCRegF6;
    spcWptr[0x07] = SPCRegF7;
    spcWptr[0x08] = SPCRegF8;
    spcWptr[0x09] = SPCRegF9;
    spcWptr[0x0A] = SPCRegFA;
    spcWptr[0x0B] = SPCRegFB;
    spcWptr[0x0C] = SPCRegFC;
    spcWptr[0x0D] = SPCRegFD;
    spcWptr[0x0E] = SPCRegFE;
    spcWptr[0x0F] = SPCRegFF;

    spcRptr[0x00] = RSPCRegF0;
    spcRptr[0x01] = RSPCRegF1;
    spcRptr[0x02] = RSPCRegF2;
    spcRptr[0x03] = RSPCRegF3;
    spcRptr[0x04] = RSPCRegF4;
    spcRptr[0x05] = RSPCRegF5;
    spcRptr[0x06] = RSPCRegF6;
    spcRptr[0x07] = RSPCRegF7;
    spcRptr[0x08] = RSPCRegF8;
    spcRptr[0x09] = RSPCRegF9;
    spcRptr[0x0A] = RSPCRegFA;
    spcRptr[0x0B] = RSPCRegFB;
    spcRptr[0x0C] = RSPCRegFC;
    spcRptr[0x0D] = RSPCRegFD;
    spcRptr[0x0E] = RSPCRegFE;
    spcRptr[0x0F] = RSPCRegFF;

    dspRptr[0x00] = RDSPReg00;
    dspRptr[0x01] = RDSPReg01;
    dspRptr[0x02] = RDSPReg02;
    dspRptr[0x03] = RDSPReg03;
    dspRptr[0x04] = RDSPReg04;
    dspRptr[0x05] = RDSPReg05;
    dspRptr[0x06] = RDSPReg06;
    dspRptr[0x07] = RDSPReg07;
    dspRptr[0x08] = RDSPReg08;
    dspRptr[0x09] = RDSPReg09;
    dspRptr[0x0A] = RDSPReg0A;
    dspRptr[0x0B] = RDSPReg0B;
    dspRptr[0x0C] = RDSPReg0C;
    dspRptr[0x0D] = RDSPReg0D;
    dspRptr[0x0E] = RDSPReg0E;
    dspRptr[0x0F] = RDSPReg0F;
    dspRptr[0x10] = RDSPReg10;
    dspRptr[0x11] = RDSPReg11;
    dspRptr[0x12] = RDSPReg12;
    dspRptr[0x13] = RDSPReg13;
    dspRptr[0x14] = RDSPReg14;
    dspRptr[0x15] = RDSPReg15;
    dspRptr[0x16] = RDSPReg16;
    dspRptr[0x17] = RDSPReg17;
    dspRptr[0x18] = RDSPReg18;
    dspRptr[0x19] = RDSPReg19;
    dspRptr[0x1A] = RDSPReg1A;
    dspRptr[0x1B] = RDSPReg1B;
    dspRptr[0x1C] = RDSPReg1C;
    dspRptr[0x1D] = RDSPReg1D;
    dspRptr[0x1E] = RDSPReg1E;
    dspRptr[0x1F] = RDSPReg1F;
    dspRptr[0x20] = RDSPReg20;
    dspRptr[0x21] = RDSPReg21;
    dspRptr[0x22] = RDSPReg22;
    dspRptr[0x23] = RDSPReg23;
    dspRptr[0x24] = RDSPReg24;
    dspRptr[0x25] = RDSPReg25;
    dspRptr[0x26] = RDSPReg26;
    dspRptr[0x27] = RDSPReg27;
    dspRptr[0x28] = RDSPReg28;
    dspRptr[0x29] = RDSPReg29;
    dspRptr[0x2A] = RDSPReg2A;
    dspRptr[0x2B] = RDSPReg2B;
    dspRptr[0x2C] = RDSPReg2C;
    dspRptr[0x2D] = RDSPReg2D;
    dspRptr[0x2E] = RDSPReg2E;
    dspRptr[0x2F] = RDSPReg2F;
    dspRptr[0x30] = RDSPReg30;
    dspRptr[0x31] = RDSPReg31;
    dspRptr[0x32] = RDSPReg32;
    dspRptr[0x33] = RDSPReg33;
    dspRptr[0x34] = RDSPReg34;
    dspRptr[0x35] = RDSPReg35;
    dspRptr[0x36] = RDSPReg36;
    dspRptr[0x37] = RDSPReg37;
    dspRptr[0x38] = RDSPReg38;
    dspRptr[0x39] = RDSPReg39;
    dspRptr[0x3A] = RDSPReg3A;
    dspRptr[0x3B] = RDSPReg3B;
    dspRptr[0x3C] = RDSPReg3C;
    dspRptr[0x3D] = RDSPReg3D;
    dspRptr[0x3E] = RDSPReg3E;
    dspRptr[0x3F] = RDSPReg3F;
    dspRptr[0x40] = RDSPReg40;
    dspRptr[0x41] = RDSPReg41;
    dspRptr[0x42] = RDSPReg42;
    dspRptr[0x43] = RDSPReg43;
    dspRptr[0x44] = RDSPReg44;
    dspRptr[0x45] = RDSPReg45;
    dspRptr[0x46] = RDSPReg46;
    dspRptr[0x47] = RDSPReg47;
    dspRptr[0x48] = RDSPReg48;
    dspRptr[0x49] = RDSPReg49;
    dspRptr[0x4A] = RDSPReg4A;
    dspRptr[0x4B] = RDSPReg4B;
    dspRptr[0x4C] = RDSPReg4C;
    dspRptr[0x4D] = RDSPReg4D;
    dspRptr[0x4E] = RDSPReg4E;
    dspRptr[0x4F] = RDSPReg4F;
    dspRptr[0x50] = RDSPReg50;
    dspRptr[0x51] = RDSPReg51;
    dspRptr[0x52] = RDSPReg52;
    dspRptr[0x53] = RDSPReg53;
    dspRptr[0x54] = RDSPReg54;
    dspRptr[0x55] = RDSPReg55;
    dspRptr[0x56] = RDSPReg56;
    dspRptr[0x57] = RDSPReg57;
    dspRptr[0x58] = RDSPReg58;
    dspRptr[0x59] = RDSPReg59;
    dspRptr[0x5A] = RDSPReg5A;
    dspRptr[0x5B] = RDSPReg5B;
    dspRptr[0x5C] = RDSPReg5C;
    dspRptr[0x5D] = RDSPReg5D;
    dspRptr[0x5E] = RDSPReg5E;
    dspRptr[0x5F] = RDSPReg5F;
    dspRptr[0x60] = RDSPReg60;
    dspRptr[0x61] = RDSPReg61;
    dspRptr[0x62] = RDSPReg62;
    dspRptr[0x63] = RDSPReg63;
    dspRptr[0x64] = RDSPReg64;
    dspRptr[0x65] = RDSPReg65;
    dspRptr[0x66] = RDSPReg66;
    dspRptr[0x67] = RDSPReg67;
    dspRptr[0x68] = RDSPReg68;
    dspRptr[0x69] = RDSPReg69;
    dspRptr[0x6A] = RDSPReg6A;
    dspRptr[0x6B] = RDSPReg6B;
    dspRptr[0x6C] = RDSPReg6C;
    dspRptr[0x6D] = RDSPReg6D;
    dspRptr[0x6E] = RDSPReg6E;
    dspRptr[0x6F] = RDSPReg6F;
    dspRptr[0x70] = RDSPReg70;
    dspRptr[0x71] = RDSPReg71;
    dspRptr[0x72] = RDSPReg72;
    dspRptr[0x73] = RDSPReg73;
    dspRptr[0x74] = RDSPReg74;
    dspRptr[0x75] = RDSPReg75;
    dspRptr[0x76] = RDSPReg76;
    dspRptr[0x77] = RDSPReg77;
    dspRptr[0x78] = RDSPReg78;
    dspRptr[0x79] = RDSPReg79;
    dspRptr[0x7A] = RDSPReg7A;
    dspRptr[0x7B] = RDSPReg7B;
    dspRptr[0x7C] = RDSPReg7C;
    dspRptr[0x7D] = RDSPReg7D;
    dspRptr[0x7E] = RDSPReg7E;
    dspRptr[0x7F] = RDSPReg7F;
    dspRptr[0x80] = RDSPReg80;
    dspRptr[0x81] = RDSPReg81;
    dspRptr[0x82] = RDSPReg82;
    dspRptr[0x83] = RDSPReg83;
    dspRptr[0x84] = RDSPReg84;
    dspRptr[0x85] = RDSPReg85;
    dspRptr[0x86] = RDSPReg86;
    dspRptr[0x87] = RDSPReg87;
    dspRptr[0x88] = RDSPReg88;
    dspRptr[0x89] = RDSPReg89;
    dspRptr[0x8A] = RDSPReg8A;
    dspRptr[0x8B] = RDSPReg8B;
    dspRptr[0x8C] = RDSPReg8C;
    dspRptr[0x8D] = RDSPReg8D;
    dspRptr[0x8E] = RDSPReg8E;
    dspRptr[0x8F] = RDSPReg8F;
    dspRptr[0x90] = RDSPReg90;
    dspRptr[0x91] = RDSPReg91;
    dspRptr[0x92] = RDSPReg92;
    dspRptr[0x93] = RDSPReg93;
    dspRptr[0x94] = RDSPReg94;
    dspRptr[0x95] = RDSPReg95;
    dspRptr[0x96] = RDSPReg96;
    dspRptr[0x97] = RDSPReg97;
    dspRptr[0x98] = RDSPReg98;
    dspRptr[0x99] = RDSPReg99;
    dspRptr[0x9A] = RDSPReg9A;
    dspRptr[0x9B] = RDSPReg9B;
    dspRptr[0x9C] = RDSPReg9C;
    dspRptr[0x9D] = RDSPReg9D;
    dspRptr[0x9E] = RDSPReg9E;
    dspRptr[0x9F] = RDSPReg9F;
    dspRptr[0xA0] = RDSPRegA0;
    dspRptr[0xA1] = RDSPRegA1;
    dspRptr[0xA2] = RDSPRegA2;
    dspRptr[0xA3] = RDSPRegA3;
    dspRptr[0xA4] = RDSPRegA4;
    dspRptr[0xA5] = RDSPRegA5;
    dspRptr[0xA6] = RDSPRegA6;
    dspRptr[0xA7] = RDSPRegA7;
    dspRptr[0xA8] = RDSPRegA8;
    dspRptr[0xA9] = RDSPRegA9;
    dspRptr[0xAA] = RDSPRegAA;
    dspRptr[0xAB] = RDSPRegAB;
    dspRptr[0xAC] = RDSPRegAC;
    dspRptr[0xAD] = RDSPRegAD;
    dspRptr[0xAE] = RDSPRegAE;
    dspRptr[0xAF] = RDSPRegAF;
    dspRptr[0xB0] = RDSPRegB0;
    dspRptr[0xB1] = RDSPRegB1;
    dspRptr[0xB2] = RDSPRegB2;
    dspRptr[0xB3] = RDSPRegB3;
    dspRptr[0xB4] = RDSPRegB4;
    dspRptr[0xB5] = RDSPRegB5;
    dspRptr[0xB6] = RDSPRegB6;
    dspRptr[0xB7] = RDSPRegB7;
    dspRptr[0xB8] = RDSPRegB8;
    dspRptr[0xB9] = RDSPRegB9;
    dspRptr[0xBA] = RDSPRegBA;
    dspRptr[0xBB] = RDSPRegBB;
    dspRptr[0xBC] = RDSPRegBC;
    dspRptr[0xBD] = RDSPRegBD;
    dspRptr[0xBE] = RDSPRegBE;
    dspRptr[0xBF] = RDSPRegBF;
    dspRptr[0xC0] = RDSPRegC0;
    dspRptr[0xC1] = RDSPRegC1;
    dspRptr[0xC2] = RDSPRegC2;
    dspRptr[0xC3] = RDSPRegC3;
    dspRptr[0xC4] = RDSPRegC4;
    dspRptr[0xC5] = RDSPRegC5;
    dspRptr[0xC6] = RDSPRegC6;
    dspRptr[0xC7] = RDSPRegC7;
    dspRptr[0xC8] = RDSPRegC8;
    dspRptr[0xC9] = RDSPRegC9;
    dspRptr[0xCA] = RDSPRegCA;
    dspRptr[0xCB] = RDSPRegCB;
    dspRptr[0xCC] = RDSPRegCC;
    dspRptr[0xCD] = RDSPRegCD;
    dspRptr[0xCE] = RDSPRegCE;
    dspRptr[0xCF] = RDSPRegCF;
    dspRptr[0xD0] = RDSPRegD0;
    dspRptr[0xD1] = RDSPRegD1;
    dspRptr[0xD2] = RDSPRegD2;
    dspRptr[0xD3] = RDSPRegD3;
    dspRptr[0xD4] = RDSPRegD4;
    dspRptr[0xD5] = RDSPRegD5;
    dspRptr[0xD6] = RDSPRegD6;
    dspRptr[0xD7] = RDSPRegD7;
    dspRptr[0xD8] = RDSPRegD8;
    dspRptr[0xD9] = RDSPRegD9;
    dspRptr[0xDA] = RDSPRegDA;
    dspRptr[0xDB] = RDSPRegDB;
    dspRptr[0xDC] = RDSPRegDC;
    dspRptr[0xDD] = RDSPRegDD;
    dspRptr[0xDE] = RDSPRegDE;
    dspRptr[0xDF] = RDSPRegDF;
    dspRptr[0xE0] = RDSPRegE0;
    dspRptr[0xE1] = RDSPRegE1;
    dspRptr[0xE2] = RDSPRegE2;
    dspRptr[0xE3] = RDSPRegE3;
    dspRptr[0xE4] = RDSPRegE4;
    dspRptr[0xE5] = RDSPRegE5;
    dspRptr[0xE6] = RDSPRegE6;
    dspRptr[0xE7] = RDSPRegE7;
    dspRptr[0xE8] = RDSPRegE8;
    dspRptr[0xE9] = RDSPRegE9;
    dspRptr[0xEA] = RDSPRegEA;
    dspRptr[0xEB] = RDSPRegEB;
    dspRptr[0xEC] = RDSPRegEC;
    dspRptr[0xED] = RDSPRegED;
    dspRptr[0xEE] = RDSPRegEE;
    dspRptr[0xEF] = RDSPRegEF;
    dspRptr[0xF0] = RDSPRegF0;
    dspRptr[0xF1] = RDSPRegF1;
    dspRptr[0xF2] = RDSPRegF2;
    dspRptr[0xF3] = RDSPRegF3;
    dspRptr[0xF4] = RDSPRegF4;
    dspRptr[0xF5] = RDSPRegF5;
    dspRptr[0xF6] = RDSPRegF6;
    dspRptr[0xF7] = RDSPRegF7;
    dspRptr[0xF8] = RDSPRegF8;
    dspRptr[0xF9] = RDSPRegF9;
    dspRptr[0xFA] = RDSPRegFA;
    dspRptr[0xFB] = RDSPRegFB;
    dspRptr[0xFC] = RDSPRegFC;
    dspRptr[0xFD] = RDSPRegFD;
    dspRptr[0xFE] = RDSPRegFE;
    dspRptr[0xFF] = RDSPRegFF;

    dspWptr[0x00] = WDSPReg00;
    dspWptr[0x01] = WDSPReg01;
    dspWptr[0x02] = WDSPReg02;
    dspWptr[0x03] = WDSPReg03;
    dspWptr[0x04] = WDSPReg04;
    dspWptr[0x05] = WDSPReg05;
    dspWptr[0x06] = WDSPReg06;
    dspWptr[0x07] = WDSPReg07;
    dspWptr[0x08] = WDSPReg08;
    dspWptr[0x09] = WDSPReg09;
    dspWptr[0x0A] = WDSPReg0A;
    dspWptr[0x0B] = WDSPReg0B;
    dspWptr[0x0C] = WDSPReg0C;
    dspWptr[0x0D] = WDSPReg0D;
    dspWptr[0x0E] = WDSPReg0E;
    dspWptr[0x0F] = WDSPReg0F;
    dspWptr[0x10] = WDSPReg10;
    dspWptr[0x11] = WDSPReg11;
    dspWptr[0x12] = WDSPReg12;
    dspWptr[0x13] = WDSPReg13;
    dspWptr[0x14] = WDSPReg14;
    dspWptr[0x15] = WDSPReg15;
    dspWptr[0x16] = WDSPReg16;
    dspWptr[0x17] = WDSPReg17;
    dspWptr[0x18] = WDSPReg18;
    dspWptr[0x19] = WDSPReg19;
    dspWptr[0x1A] = WDSPReg1A;
    dspWptr[0x1B] = WDSPReg1B;
    dspWptr[0x1C] = WDSPReg1C;
    dspWptr[0x1D] = WDSPReg1D;
    dspWptr[0x1E] = WDSPReg1E;
    dspWptr[0x1F] = WDSPReg1F;
    dspWptr[0x20] = WDSPReg20;
    dspWptr[0x21] = WDSPReg21;
    dspWptr[0x22] = WDSPReg22;
    dspWptr[0x23] = WDSPReg23;
    dspWptr[0x24] = WDSPReg24;
    dspWptr[0x25] = WDSPReg25;
    dspWptr[0x26] = WDSPReg26;
    dspWptr[0x27] = WDSPReg27;
    dspWptr[0x28] = WDSPReg28;
    dspWptr[0x29] = WDSPReg29;
    dspWptr[0x2A] = WDSPReg2A;
    dspWptr[0x2B] = WDSPReg2B;
    dspWptr[0x2C] = WDSPReg2C;
    dspWptr[0x2D] = WDSPReg2D;
    dspWptr[0x2E] = WDSPReg2E;
    dspWptr[0x2F] = WDSPReg2F;
    dspWptr[0x30] = WDSPReg30;
    dspWptr[0x31] = WDSPReg31;
    dspWptr[0x32] = WDSPReg32;
    dspWptr[0x33] = WDSPReg33;
    dspWptr[0x34] = WDSPReg34;
    dspWptr[0x35] = WDSPReg35;
    dspWptr[0x36] = WDSPReg36;
    dspWptr[0x37] = WDSPReg37;
    dspWptr[0x38] = WDSPReg38;
    dspWptr[0x39] = WDSPReg39;
    dspWptr[0x3A] = WDSPReg3A;
    dspWptr[0x3B] = WDSPReg3B;
    dspWptr[0x3C] = WDSPReg3C;
    dspWptr[0x3D] = WDSPReg3D;
    dspWptr[0x3E] = WDSPReg3E;
    dspWptr[0x3F] = WDSPReg3F;
    dspWptr[0x40] = WDSPReg40;
    dspWptr[0x41] = WDSPReg41;
    dspWptr[0x42] = WDSPReg42;
    dspWptr[0x43] = WDSPReg43;
    dspWptr[0x44] = WDSPReg44;
    dspWptr[0x45] = WDSPReg45;
    dspWptr[0x46] = WDSPReg46;
    dspWptr[0x47] = WDSPReg47;
    dspWptr[0x48] = WDSPReg48;
    dspWptr[0x49] = WDSPReg49;
    dspWptr[0x4A] = WDSPReg4A;
    dspWptr[0x4B] = WDSPReg4B;
    dspWptr[0x4C] = WDSPReg4C;
    dspWptr[0x4D] = WDSPReg4D;
    dspWptr[0x4E] = WDSPReg4E;
    dspWptr[0x4F] = WDSPReg4F;
    dspWptr[0x50] = WDSPReg50;
    dspWptr[0x51] = WDSPReg51;
    dspWptr[0x52] = WDSPReg52;
    dspWptr[0x53] = WDSPReg53;
    dspWptr[0x54] = WDSPReg54;
    dspWptr[0x55] = WDSPReg55;
    dspWptr[0x56] = WDSPReg56;
    dspWptr[0x57] = WDSPReg57;
    dspWptr[0x58] = WDSPReg58;
    dspWptr[0x59] = WDSPReg59;
    dspWptr[0x5A] = WDSPReg5A;
    dspWptr[0x5B] = WDSPReg5B;
    dspWptr[0x5C] = WDSPReg5C;
    dspWptr[0x5D] = WDSPReg5D;
    dspWptr[0x5E] = WDSPReg5E;
    dspWptr[0x5F] = WDSPReg5F;
    dspWptr[0x60] = WDSPReg60;
    dspWptr[0x61] = WDSPReg61;
    dspWptr[0x62] = WDSPReg62;
    dspWptr[0x63] = WDSPReg63;
    dspWptr[0x64] = WDSPReg64;
    dspWptr[0x65] = WDSPReg65;
    dspWptr[0x66] = WDSPReg66;
    dspWptr[0x67] = WDSPReg67;
    dspWptr[0x68] = WDSPReg68;
    dspWptr[0x69] = WDSPReg69;
    dspWptr[0x6A] = WDSPReg6A;
    dspWptr[0x6B] = WDSPReg6B;
    dspWptr[0x6C] = WDSPReg6C;
    dspWptr[0x6D] = WDSPReg6D;
    dspWptr[0x6E] = WDSPReg6E;
    dspWptr[0x6F] = WDSPReg6F;
    dspWptr[0x70] = WDSPReg70;
    dspWptr[0x71] = WDSPReg71;
    dspWptr[0x72] = WDSPReg72;
    dspWptr[0x73] = WDSPReg73;
    dspWptr[0x74] = WDSPReg74;
    dspWptr[0x75] = WDSPReg75;
    dspWptr[0x76] = WDSPReg76;
    dspWptr[0x77] = WDSPReg77;
    dspWptr[0x78] = WDSPReg78;
    dspWptr[0x79] = WDSPReg79;
    dspWptr[0x7A] = WDSPReg7A;
    dspWptr[0x7B] = WDSPReg7B;
    dspWptr[0x7C] = WDSPReg7C;
    dspWptr[0x7D] = WDSPReg7D;
    dspWptr[0x7E] = WDSPReg7E;
    dspWptr[0x7F] = WDSPReg7F;
    dspWptr[0x80] = WDSPReg80;
    dspWptr[0x81] = WDSPReg81;
    dspWptr[0x82] = WDSPReg82;
    dspWptr[0x83] = WDSPReg83;
    dspWptr[0x84] = WDSPReg84;
    dspWptr[0x85] = WDSPReg85;
    dspWptr[0x86] = WDSPReg86;
    dspWptr[0x87] = WDSPReg87;
    dspWptr[0x88] = WDSPReg88;
    dspWptr[0x89] = WDSPReg89;
    dspWptr[0x8A] = WDSPReg8A;
    dspWptr[0x8B] = WDSPReg8B;
    dspWptr[0x8C] = WDSPReg8C;
    dspWptr[0x8D] = WDSPReg8D;
    dspWptr[0x8E] = WDSPReg8E;
    dspWptr[0x8F] = WDSPReg8F;
    dspWptr[0x90] = WDSPReg90;
    dspWptr[0x91] = WDSPReg91;
    dspWptr[0x92] = WDSPReg92;
    dspWptr[0x93] = WDSPReg93;
    dspWptr[0x94] = WDSPReg94;
    dspWptr[0x95] = WDSPReg95;
    dspWptr[0x96] = WDSPReg96;
    dspWptr[0x97] = WDSPReg97;
    dspWptr[0x98] = WDSPReg98;
    dspWptr[0x99] = WDSPReg99;
    dspWptr[0x9A] = WDSPReg9A;
    dspWptr[0x9B] = WDSPReg9B;
    dspWptr[0x9C] = WDSPReg9C;
    dspWptr[0x9D] = WDSPReg9D;
    dspWptr[0x9E] = WDSPReg9E;
    dspWptr[0x9F] = WDSPReg9F;
    dspWptr[0xA0] = WDSPRegA0;
    dspWptr[0xA1] = WDSPRegA1;
    dspWptr[0xA2] = WDSPRegA2;
    dspWptr[0xA3] = WDSPRegA3;
    dspWptr[0xA4] = WDSPRegA4;
    dspWptr[0xA5] = WDSPRegA5;
    dspWptr[0xA6] = WDSPRegA6;
    dspWptr[0xA7] = WDSPRegA7;
    dspWptr[0xA8] = WDSPRegA8;
    dspWptr[0xA9] = WDSPRegA9;
    dspWptr[0xAA] = WDSPRegAA;
    dspWptr[0xAB] = WDSPRegAB;
    dspWptr[0xAC] = WDSPRegAC;
    dspWptr[0xAD] = WDSPRegAD;
    dspWptr[0xAE] = WDSPRegAE;
    dspWptr[0xAF] = WDSPRegAF;
    dspWptr[0xB0] = WDSPRegB0;
    dspWptr[0xB1] = WDSPRegB1;
    dspWptr[0xB2] = WDSPRegB2;
    dspWptr[0xB3] = WDSPRegB3;
    dspWptr[0xB4] = WDSPRegB4;
    dspWptr[0xB5] = WDSPRegB5;
    dspWptr[0xB6] = WDSPRegB6;
    dspWptr[0xB7] = WDSPRegB7;
    dspWptr[0xB8] = WDSPRegB8;
    dspWptr[0xB9] = WDSPRegB9;
    dspWptr[0xBA] = WDSPRegBA;
    dspWptr[0xBB] = WDSPRegBB;
    dspWptr[0xBC] = WDSPRegBC;
    dspWptr[0xBD] = WDSPRegBD;
    dspWptr[0xBE] = WDSPRegBE;
    dspWptr[0xBF] = WDSPRegBF;
    dspWptr[0xC0] = WDSPRegC0;
    dspWptr[0xC1] = WDSPRegC1;
    dspWptr[0xC2] = WDSPRegC2;
    dspWptr[0xC3] = WDSPRegC3;
    dspWptr[0xC4] = WDSPRegC4;
    dspWptr[0xC5] = WDSPRegC5;
    dspWptr[0xC6] = WDSPRegC6;
    dspWptr[0xC7] = WDSPRegC7;
    dspWptr[0xC8] = WDSPRegC8;
    dspWptr[0xC9] = WDSPRegC9;
    dspWptr[0xCA] = WDSPRegCA;
    dspWptr[0xCB] = WDSPRegCB;
    dspWptr[0xCC] = WDSPRegCC;
    dspWptr[0xCD] = WDSPRegCD;
    dspWptr[0xCE] = WDSPRegCE;
    dspWptr[0xCF] = WDSPRegCF;
    dspWptr[0xD0] = WDSPRegD0;
    dspWptr[0xD1] = WDSPRegD1;
    dspWptr[0xD2] = WDSPRegD2;
    dspWptr[0xD3] = WDSPRegD3;
    dspWptr[0xD4] = WDSPRegD4;
    dspWptr[0xD5] = WDSPRegD5;
    dspWptr[0xD6] = WDSPRegD6;
    dspWptr[0xD7] = WDSPRegD7;
    dspWptr[0xD8] = WDSPRegD8;
    dspWptr[0xD9] = WDSPRegD9;
    dspWptr[0xDA] = WDSPRegDA;
    dspWptr[0xDB] = WDSPRegDB;
    dspWptr[0xDC] = WDSPRegDC;
    dspWptr[0xDD] = WDSPRegDD;
    dspWptr[0xDE] = WDSPRegDE;
    dspWptr[0xDF] = WDSPRegDF;
    dspWptr[0xE0] = WDSPRegE0;
    dspWptr[0xE1] = WDSPRegE1;
    dspWptr[0xE2] = WDSPRegE2;
    dspWptr[0xE3] = WDSPRegE3;
    dspWptr[0xE4] = WDSPRegE4;
    dspWptr[0xE5] = WDSPRegE5;
    dspWptr[0xE6] = WDSPRegE6;
    dspWptr[0xE7] = WDSPRegE7;
    dspWptr[0xE8] = WDSPRegE8;
    dspWptr[0xE9] = WDSPRegE9;
    dspWptr[0xEA] = WDSPRegEA;
    dspWptr[0xEB] = WDSPRegEB;
    dspWptr[0xEC] = WDSPRegEC;
    dspWptr[0xED] = WDSPRegED;
    dspWptr[0xEE] = WDSPRegEE;
    dspWptr[0xEF] = WDSPRegEF;
    dspWptr[0xF0] = WDSPRegF0;
    dspWptr[0xF1] = WDSPRegF1;
    dspWptr[0xF2] = WDSPRegF2;
    dspWptr[0xF3] = WDSPRegF3;
    dspWptr[0xF4] = WDSPRegF4;
    dspWptr[0xF5] = WDSPRegF5;
    dspWptr[0xF6] = WDSPRegF6;
    dspWptr[0xF7] = WDSPRegF7;
    dspWptr[0xF8] = WDSPRegF8;
    dspWptr[0xF9] = WDSPRegF9;
    dspWptr[0xFA] = WDSPRegFA;
    dspWptr[0xFB] = WDSPRegFB;
    dspWptr[0xFC] = WDSPRegFC;
    dspWptr[0xFD] = WDSPRegFD;
    dspWptr[0xFE] = WDSPRegFE;
    dspWptr[0xFF] = WDSPRegFF;

    // first fill all pointer to an invalid access function
    // XXX seems to be redundant, all entries are overwritten below
    for (eop** i = opcjmptab; i != endof(opcjmptab); ++i)
        *i = Invalidopcode;

    // now fill the table
    opcjmptab[0x00] = Op00;
    opcjmptab[0x01] = Op01;
    opcjmptab[0x02] = Op02;
    opcjmptab[0x03] = Op03;
    opcjmptab[0x04] = Op04;
    opcjmptab[0x05] = Op05;
    opcjmptab[0x06] = Op06;
    opcjmptab[0x07] = Op07;
    opcjmptab[0x08] = Op08;
    opcjmptab[0x09] = Op09;
    opcjmptab[0x0A] = Op0A;
    opcjmptab[0x0B] = Op0B;
    opcjmptab[0x0C] = Op0C;
    opcjmptab[0x0D] = Op0D;
    opcjmptab[0x0E] = Op0E;
    opcjmptab[0x0F] = Op0F;
    opcjmptab[0x10] = Op10;
    opcjmptab[0x11] = Op11;
    opcjmptab[0x12] = Op12;
    opcjmptab[0x13] = Op13;
    opcjmptab[0x14] = Op14;
    opcjmptab[0x15] = Op15;
    opcjmptab[0x16] = Op16;
    opcjmptab[0x17] = Op17;
    opcjmptab[0x18] = Op18;
    opcjmptab[0x19] = Op19;
    opcjmptab[0x1A] = Op1A;
    opcjmptab[0x1B] = Op1B;
    opcjmptab[0x1C] = Op1C;
    opcjmptab[0x1D] = Op1D;
    opcjmptab[0x1E] = Op1E;
    opcjmptab[0x1F] = Op1F;
    opcjmptab[0x20] = Op20;
    opcjmptab[0x21] = Op21;
    opcjmptab[0x22] = Op22;
    opcjmptab[0x23] = Op23;
    opcjmptab[0x24] = Op24;
    opcjmptab[0x25] = Op25;
    opcjmptab[0x26] = Op26;
    opcjmptab[0x27] = Op27;
    opcjmptab[0x28] = Op28;
    opcjmptab[0x29] = Op29;
    opcjmptab[0x2A] = Op2A;
    opcjmptab[0x2B] = Op2B;
    opcjmptab[0x2C] = Op2C;
    opcjmptab[0x2D] = Op2D;
    opcjmptab[0x2E] = Op2E;
    opcjmptab[0x2F] = Op2F;
    opcjmptab[0x30] = Op30;
    opcjmptab[0x31] = Op31;
    opcjmptab[0x32] = Op32;
    opcjmptab[0x33] = Op33;
    opcjmptab[0x34] = Op34;
    opcjmptab[0x35] = Op35;
    opcjmptab[0x36] = Op36;
    opcjmptab[0x37] = Op37;
    opcjmptab[0x38] = Op38;
    opcjmptab[0x39] = Op39;
    opcjmptab[0x3A] = Op3A;
    opcjmptab[0x3B] = Op3B;
    opcjmptab[0x3C] = Op3C;
    opcjmptab[0x3D] = Op3D;
    opcjmptab[0x3E] = Op3E;
    opcjmptab[0x3F] = Op3F;
    opcjmptab[0x40] = Op40;
    opcjmptab[0x41] = Op41;
    opcjmptab[0x42] = Op42;
    opcjmptab[0x43] = Op43;
    opcjmptab[0x44] = Op44;
    opcjmptab[0x45] = Op45;
    opcjmptab[0x46] = Op46;
    opcjmptab[0x47] = Op47;
    opcjmptab[0x48] = Op48;
    opcjmptab[0x49] = Op49;
    opcjmptab[0x4A] = Op4A;
    opcjmptab[0x4B] = Op4B;
    opcjmptab[0x4C] = Op4C;
    opcjmptab[0x4D] = Op4D;
    opcjmptab[0x4E] = Op4E;
    opcjmptab[0x4F] = Op4F;
    opcjmptab[0x50] = Op50;
    opcjmptab[0x51] = Op51;
    opcjmptab[0x52] = Op52;
    opcjmptab[0x53] = Op53;
    opcjmptab[0x54] = Op54;
    opcjmptab[0x55] = Op55;
    opcjmptab[0x56] = Op56;
    opcjmptab[0x57] = Op57;
    opcjmptab[0x58] = Op58;
    opcjmptab[0x59] = Op59;
    opcjmptab[0x5A] = Op5A;
    opcjmptab[0x5B] = Op5B;
    opcjmptab[0x5C] = Op5C;
    opcjmptab[0x5D] = Op5D;
    opcjmptab[0x5E] = Op5E;
    opcjmptab[0x5F] = Op5F;
    opcjmptab[0x60] = Op60;
    opcjmptab[0x61] = Op61;
    opcjmptab[0x62] = Op62;
    opcjmptab[0x63] = Op63;
    opcjmptab[0x64] = Op64;
    opcjmptab[0x65] = Op65;
    opcjmptab[0x66] = Op66;
    opcjmptab[0x67] = Op67;
    opcjmptab[0x68] = Op68;
    opcjmptab[0x69] = Op69;
    opcjmptab[0x6A] = Op6A;
    opcjmptab[0x6B] = Op6B;
    opcjmptab[0x6C] = Op6C;
    opcjmptab[0x6D] = Op6D;
    opcjmptab[0x6E] = Op6E;
    opcjmptab[0x6F] = Op6F;
    opcjmptab[0x70] = Op70;
    opcjmptab[0x71] = Op71;
    opcjmptab[0x72] = Op72;
    opcjmptab[0x73] = Op73;
    opcjmptab[0x74] = Op74;
    opcjmptab[0x75] = Op75;
    opcjmptab[0x76] = Op76;
    opcjmptab[0x77] = Op77;
    opcjmptab[0x78] = Op78;
    opcjmptab[0x79] = Op79;
    opcjmptab[0x7A] = Op7A;
    opcjmptab[0x7B] = Op7B;
    opcjmptab[0x7C] = Op7C;
    opcjmptab[0x7D] = Op7D;
    opcjmptab[0x7E] = Op7E;
    opcjmptab[0x7F] = Op7F;
    opcjmptab[0x80] = Op80;
    opcjmptab[0x81] = Op81;
    opcjmptab[0x82] = Op82;
    opcjmptab[0x83] = Op83;
    opcjmptab[0x84] = Op84;
    opcjmptab[0x85] = Op85;
    opcjmptab[0x86] = Op86;
    opcjmptab[0x87] = Op87;
    opcjmptab[0x88] = Op88;
    opcjmptab[0x89] = Op89;
    opcjmptab[0x8A] = Op8A;
    opcjmptab[0x8B] = Op8B;
    opcjmptab[0x8C] = Op8C;
    opcjmptab[0x8D] = Op8D;
    opcjmptab[0x8E] = Op8E;
    opcjmptab[0x8F] = Op8F;
    opcjmptab[0x90] = Op90;
    opcjmptab[0x91] = Op91;
    opcjmptab[0x92] = Op92;
    opcjmptab[0x93] = Op93;
    opcjmptab[0x94] = Op94;
    opcjmptab[0x95] = Op95;
    opcjmptab[0x96] = Op96;
    opcjmptab[0x97] = Op97;
    opcjmptab[0x98] = Op98;
    opcjmptab[0x99] = Op99;
    opcjmptab[0x9A] = Op9A;
    opcjmptab[0x9B] = Op9B;
    opcjmptab[0x9C] = Op9C;
    opcjmptab[0x9D] = Op9D;
    opcjmptab[0x9E] = Op9E;
    opcjmptab[0x9F] = Op9F;
    opcjmptab[0xA0] = OpA0;
    opcjmptab[0xA1] = OpA1;
    opcjmptab[0xA2] = OpA2;
    opcjmptab[0xA3] = OpA3;
    opcjmptab[0xA4] = OpA4;
    opcjmptab[0xA5] = OpA5;
    opcjmptab[0xA6] = OpA6;
    opcjmptab[0xA7] = OpA7;
    opcjmptab[0xA8] = OpA8;
    opcjmptab[0xA9] = OpA9;
    opcjmptab[0xAA] = OpAA;
    opcjmptab[0xAB] = OpAB;
    opcjmptab[0xAC] = OpAC;
    opcjmptab[0xAD] = OpAD;
    opcjmptab[0xAE] = OpAE;
    opcjmptab[0xAF] = OpAF;
    opcjmptab[0xB0] = OpB0;
    opcjmptab[0xB1] = OpB1;
    opcjmptab[0xB2] = OpB2;
    opcjmptab[0xB3] = OpB3;
    opcjmptab[0xB4] = OpB4;
    opcjmptab[0xB5] = OpB5;
    opcjmptab[0xB6] = OpB6;
    opcjmptab[0xB7] = OpB7;
    opcjmptab[0xB8] = OpB8;
    opcjmptab[0xB9] = OpB9;
    opcjmptab[0xBA] = OpBA;
    opcjmptab[0xBB] = OpBB;
    opcjmptab[0xBC] = OpBC;
    opcjmptab[0xBD] = OpBD;
    opcjmptab[0xBE] = OpBE;
    opcjmptab[0xBF] = OpBF;
    opcjmptab[0xC0] = OpC0;
    opcjmptab[0xC1] = OpC1;
    opcjmptab[0xC2] = OpC2;
    opcjmptab[0xC3] = OpC3;
    opcjmptab[0xC4] = OpC4;
    opcjmptab[0xC5] = OpC5;
    opcjmptab[0xC6] = OpC6;
    opcjmptab[0xC7] = OpC7;
    opcjmptab[0xC8] = OpC8;
    opcjmptab[0xC9] = OpC9;
    opcjmptab[0xCA] = OpCA;
    opcjmptab[0xCB] = OpCB;
    opcjmptab[0xCC] = OpCC;
    opcjmptab[0xCD] = OpCD;
    opcjmptab[0xCE] = OpCE;
    opcjmptab[0xCF] = OpCF;
    opcjmptab[0xD0] = OpD0;
    opcjmptab[0xD1] = OpD1;
    opcjmptab[0xD2] = OpD2;
    opcjmptab[0xD3] = OpD3;
    opcjmptab[0xD4] = OpD4;
    opcjmptab[0xD5] = OpD5;
    opcjmptab[0xD6] = OpD6;
    opcjmptab[0xD7] = OpD7;
    opcjmptab[0xD8] = OpD8;
    opcjmptab[0xD9] = OpD9;
    opcjmptab[0xDA] = OpDA;
    opcjmptab[0xDB] = OpDB;
    opcjmptab[0xDC] = OpDC;
    opcjmptab[0xDD] = OpDD;
    opcjmptab[0xDE] = OpDE;
    opcjmptab[0xDF] = OpDF;
    opcjmptab[0xE0] = OpE0;
    opcjmptab[0xE1] = OpE1;
    opcjmptab[0xE2] = OpE2;
    opcjmptab[0xE3] = OpE3;
    opcjmptab[0xE4] = OpE4;
    opcjmptab[0xE5] = OpE5;
    opcjmptab[0xE6] = OpE6;
    opcjmptab[0xE7] = OpE7;
    opcjmptab[0xE8] = OpE8;
    opcjmptab[0xE9] = OpE9;
    opcjmptab[0xEA] = OpEA;
    opcjmptab[0xEB] = OpEB;
    opcjmptab[0xEC] = OpEC;
    opcjmptab[0xED] = OpED;
    opcjmptab[0xEE] = OpEE;
    opcjmptab[0xEF] = OpEF;
    opcjmptab[0xF0] = OpF0;
    opcjmptab[0xF1] = OpF1;
    opcjmptab[0xF2] = OpF2;
    opcjmptab[0xF3] = OpF3;
    opcjmptab[0xF4] = OpF4;
    opcjmptab[0xF5] = OpF5;
    opcjmptab[0xF6] = OpF6;
    opcjmptab[0xF7] = OpF7;
    opcjmptab[0xF8] = OpF8;
    opcjmptab[0xF9] = OpF9;
    opcjmptab[0xFA] = OpFA;
    opcjmptab[0xFB] = OpFB;
    opcjmptab[0xFC] = OpFC;
    opcjmptab[0xFD] = OpFD;
    opcjmptab[0xFE] = OpFE;
    opcjmptab[0xFF] = OpFF;

#ifdef __MSDOS__
    SB_alloc_dma();
#endif
}

void LPFstereo(s4* esi)
{
    u4 n = BufferSizeB / 4; // # of samples to mix / 4
    s4 ebx = LPFsample1;
    s4 edx = LPFsample2;
    do {
        s4 const eax = esi[0] >> 1;
        s4 const ecx = esi[1] >> 1;
        esi[0] = ebx + eax;
        esi[1] = edx + ecx;
        esi += 2;
        ebx = esi[0] >> 1;
        edx = esi[1] >> 1;
        esi[0] = eax + ebx;
        esi[1] = ecx + edx;
        esi += 2;
    } while (--n != 0);
    LPFsample1 = ebx;
    LPFsample2 = edx;
    LPFexit();
}

void LPFexit(void)
{
    if (Surround != 1)
        return;
    if (StereoSound != 1)
        return;

    s4* esi = DSPBuffer;
    u4 ecx = BufferSizeB / 2;
    do {
        s4 const eax = esi[0];
        s4 const ebx = esi[1];
        s4 const edx = (ebx + eax) >> 1;
        esi[0] -= ebx - edx;
        esi[1] -= eax - edx;
    } while (esi += 2, --ecx != 0);
}

void MixEcho(void)
{
    static u4 CurFiltPtr = 0;

    // Copy echobuf to DSPBuffer, EchoBuffer to echobuf
    if (StereoSound != 1) { // Mono.
        u1 const EchoT = EchoVL < EchoVR ? EchoVL : EchoVR;
        u4 esi = CEchoPtr;
        u4 edi = 0;
        do {
            // Get current echo buffer
            s4 const ebx = echobuf[esi];
            // Process FIR Filter
            u4 edx = CurFiltPtr;
            FiltLoop[edx] = ebx;
            s4 ecx = ebx * FIRTAPVal0[0] >> 7;
            for (u4 i = 1; i != 8; ++i) {
                edx = (edx + 14) % 16;
                ecx += FIRTAPVal0[i] * FiltLoop[edx] >> 7;
            }
            CurFiltPtr = (CurFiltPtr + 1) % 16;
            // Set feedback on previous echo
            s4 const eax = EchoFB * ecx >> 7;
            // Add in new echo/Store into Echo Buffer
            DSPBuffer[edi] = eax;
            echobuf[esi] = (EchoBuffer[edi] * (s4)EchoT >> 7) + eax;
            if (++esi >= MaxEcho)
                esi = 0;
        } while (++edi != BufferSizeB);
        CEchoPtr = esi;
    } else { // Stereo.
        u4 esi = CEchoPtr;
        u4 edi = 0;
        do {
            {
                // Get current echo buffer
                s4 const ebx = echobuf[esi];
                // Process FIR Filter
                u4 edx = CurFiltPtr;
                FiltLoop[edx] = ebx;
                s4 ecx = ebx * FIRTAPVal0[0] >> 7;
                for (u4 i = 1; i != 8; ++i) {
                    edx = (edx - 2) % 16;
                    ecx += FIRTAPVal0[i] * FiltLoop[edx] >> 7;
                }
                DSPBuffer[edi] += ecx;
                // Set feedback on previous echo
                s4 const eax = EchoFB * ecx >> 7;
                // Add in new echo/Store into Echo Buffer
                echobuf[esi] = (EchoBuffer[edi] * (s4)EchoVL >> 7) + eax;
            }

            ++esi;
            ++edi;

            {
                // Get current echo buffer
                s4 const ebx = echobuf[esi];
                // Process FIR Filter
                u4 edx = CurFiltPtr;
                FiltLoopR[edx] = ebx;
                s4 ecx = ebx * FIRTAPVal0[0] >> 7;
                for (u4 i = 1; i != 8; ++i) {
                    edx = (edx + 14) % 16;
                    ecx += FIRTAPVal0[i] * FiltLoopR[edx] >> 7;
                }
                DSPBuffer[edi] += ecx;
                CurFiltPtr = (CurFiltPtr + 1) % 16;
                // Set feedback on previous echo
                s4 const eax = EchoFB * ecx >> 7;
                // Add in new echo/Store into Echo Buffer
                echobuf[esi] = (EchoBuffer[edi] * (s4)EchoVR >> 7) + eax;
            }

            if (++esi >= MaxEcho * 2)
                esi = 0;
        } while (++edi != BufferSizeB);
        CEchoPtr = esi;
    }
}

void MixEcho2(void)
{
    // Copy echobuf to DSPBuffer, EchoBuffer to echobuf
    if (StereoSound != 1) { // Mono.
        u1 const EchoT = EchoVL < EchoVR ? EchoVL : EchoVR;
        u4 esi = CEchoPtr;
        u4 edi = 0;
        do {
            // Get current echo buffer
            s4 const ebx = echobuf[esi];
            DSPBuffer[edi] += ebx;
            // Add in new echo/Store into Echo Buffer
            echobuf[esi] = (EchoBuffer[edi] * (s4)EchoT >> 7) + (EchoFB * ebx >> 7);
            if (++esi >= MaxEcho)
                esi = 0; // Echo wrap.
        } while (++edi != BufferSizeB);
        CEchoPtr = esi;
    } else { // Stereo.
        u4 esi = CEchoPtr;
        u4 edi = 0;
        do {
            {
                // Get current echo buffer
                s4 const ecx = echobuf[esi];
                DSPBuffer[edi] += ecx;
                // Add in new echo/Store into Echo Buffer
                echobuf[esi] = (EchoBuffer[edi] * (s4)EchoVL >> 7) + (EchoFB * ecx >> 7);
            }

            ++esi;
            ++edi;

            {
                // Get current echo buffer
                s4 const ecx = echobuf[esi];
                DSPBuffer[edi] += ecx;
                // Add in new echo/Store into Echo Buffer
                echobuf[esi] = (EchoBuffer[edi] * (s4)EchoVR >> 7) + (EchoFB * ecx >> 7);
            }

            if (++esi >= MaxEcho * 2)
                esi = 0;
        } while (++edi != BufferSizeB);
        CEchoPtr = esi;
    }
}

static void ProcessVoiceStuff(u4 const p1)
{
    static u1 const AdsrBendData[] = {
        /**/ 122, 118, 114, 110, 106, 102, 99, 95, 92, 89, 86, 83, 80, 77, 74, 72,
        /**/ 69, 67, 64, 62, 60, 58, 56, 54, 52, 50, 48, 47, 45, 44, 42, 41,
        /**/ 39, 38, 36, 35, 34, 33, 32, 30, 29, 28, 27, 26, 25, 24, 24, 23,
        /**/ 22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13, 13,
        /**/ 12, 12, 11, 11, 11, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7,
        /**/ 7, 6, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4,
        /**/ 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,
        /**/ 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 255
    };

    static u1 const GainDecBendData[] = {
        /**/ 118, 110, 102, 95, 89, 83, 77, 72, 67, 62, 58, 54, 50, 47, 44, 41,
        /**/ 38, 35, 33, 30, 28, 26, 24, 23, 21, 20, 18, 17, 16, 15, 14, 13,
        /**/ 12, 11, 10, 9, 9, 8, 7, 7, 6, 6, 5, 5, 5, 4, 4, 4,
        /**/ 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 255
    };

    static u1 const VolumeTableD[] = {
        /**/ 0, 3, 6, 9, 12, 15, 17, 18, 19, 21, 22, 23, 24, 24, 26, 28,
        /**/ 30, 31, 33, 35, 36, 38, 40, 41, 43, 45, 46, 48, 49, 51, 52, 54,
        /**/ 56, 57, 58, 60, 61, 63, 64, 66, 67, 68, 70, 71, 72, 74, 75, 76,
        /**/ 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
        /**/ 96, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 106, 107, 108, 109,
        /**/ 110, 110, 111, 112, 112, 113, 114, 114, 115, 116, 116, 117, 117, 118, 118, 119,
        /**/ 120, 120, 120, 121, 121, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125,
        /**/ 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
        /**/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 129, 129, 129, 129, 129,
        /**/ 130, 130, 130, 131, 131, 131, 132, 132, 132, 133, 133, 134, 134, 135, 135, 135,
        /**/ 136, 137, 137, 138, 138, 139, 139, 140, 141, 141, 142, 143, 143, 144, 145, 145,
        /**/ 146, 147, 148, 149, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 159,
        /**/ 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 173, 174, 175, 176, 177,
        /**/ 179, 180, 181, 183, 184, 185, 187, 188, 189, 191, 192, 194, 195, 197, 198, 199,
        /**/ 201, 203, 204, 206, 207, 209, 210, 212, 214, 215, 217, 219, 220, 222, 224, 225,
        /**/ 227, 229, 231, 231, 232, 233, 234, 236, 237, 238, 240, 243, 246, 249, 252, 255,

        /**/ 0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 22, 24, 26, 28,
        /**/ 30, 31, 33, 35, 36, 38, 40, 41, 43, 45, 46, 48, 49, 51, 52, 54,
        /**/ 56, 57, 58, 60, 61, 63, 64, 66, 67, 68, 70, 71, 72, 74, 75, 76,
        /**/ 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
        /**/ 96, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 106, 107, 108, 109,
        /**/ 110, 110, 111, 112, 112, 113, 114, 114, 115, 116, 116, 117, 117, 118, 118, 119,
        /**/ 120, 120, 120, 121, 121, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125,
        /**/ 126, 126, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
        /**/ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 129, 129, 129, 129, 129,
        /**/ 130, 130, 130, 131, 131, 131, 132, 132, 132, 133, 133, 134, 134, 135, 135, 135,
        /**/ 136, 137, 137, 138, 138, 139, 139, 140, 141, 141, 142, 143, 143, 144, 145, 145,
        /**/ 146, 147, 148, 149, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 159,
        /**/ 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 173, 174, 175, 176, 177,
        /**/ 179, 180, 181, 183, 184, 185, 187, 188, 189, 191, 192, 194, 195, 197, 198, 199,
        /**/ 201, 203, 204, 206, 207, 209, 210, 212, 214, 215, 217, 219, 220, 222, 224, 225,
        /**/ 227, 229, 231, 233, 234, 236, 238, 240, 242, 244, 246, 248, 250, 252, 254, 255
    };

    {
        u2 const ax = *(u2 const*)&DSPMem[16 * p1 + 2];
        if (Voice0Pitch[p1] != ax) { // Pitchc.
            Voice0Pitch[p1] = ax;
            // modpitch
            Voice0Freq[p1] = (u8)(ax & 0x3FFF) * dspPAdj >> 8;
        }
    }

    u4 esi = 0;

SkipProcess2 : {
    u1 const al = VolumeTableD[DSPMem[16 * p1 + 0]];
    u1 const bl = VolumeTableD[DSPMem[16 * p1 + 1]];
    Voice0VolumeRe[p1] = al;
    Voice0VolumeLe[p1] = bl;
    u1 ah = al;
    u1 bh = bl;
    if (ah & 0x80)
        ah = -ah;
    if (bh & 0x80)
        bh = -bh;
    ah = (u1)(ah + bh) >> 1;
    if (al & 0x80 || bl & 0x80)
        ah = -ah;
    Voice0Volumee[p1] = ah;
}

    {
        u2 const ax = GlobalVL << 8 | VolumeTableD[DSPMem[16 * p1 + 0]];
        u2 const bx = GlobalVR << 8 | VolumeTableD[DSPMem[16 * p1 + 1]];
        u1 const al = VolumeConvTable[ax];
        u1 const bl = VolumeConvTable[bx];
        Voice0VolumeR[p1] = al;
        Voice0VolumeL[p1] = bl;
        u1 ah = al;
        u1 bh = bl;
        if (ah & 0x80)
            ah = -ah;
        if (bh & 0x80)
            bh = -bh;
        ah = (u1)(ah + bh) >> 1;
        if (al & 0x80 || bl & 0x80)
            ah = -ah;
        Voice0Volume[p1] = ah;
    }

    lastbl = 0;
    loopbl = 0;

    UniqueSoundv = DSPMem[0x3D] & (1U << p1) || (p1 < 7 && DSPMem[0x2D] & (1U << (p1 + 1)));

    s2* edi = Voice0BufPtr[p1];
    u4 ebx;
    for (;;) {
        ebx = Voice0Freq[p1];
        if (DSPInterpolate != 0) {
            if (StereoSound == 1) { // NextSampleSi.
                do {
                    if (BRRPlace0[p1][0] >= 0x10000000)
                        goto ProcessBRR;
                    Voice0EnvInc[p1] += Voice0IncNumber[p1];
                    if (--Voice0Time[p1] == 0)
                        goto ProcessNextEnvelope;
                EndofProcessNEnvsi:;
                    u4 eax = p1; // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
                    asm volatile("push %%ebp;  mov %0, %%ebp;  call %A4;  pop %%ebp"
                                 : "+a"(eax), "+b"(ebx), "+S"(esi), "+D"(edi)
                                 : "m"(paramhack[3])
                                 : "cc", "memory", "ecx", "edx");
                } while (esi != BufferSizeB);
            } else { // NextSamplei.
                do {
                    if (BRRPlace0[p1][0] >= 0x10000000)
                        goto ProcessBRR;
                    Voice0EnvInc[p1] += Voice0IncNumber[p1];
                    if (--Voice0Time[p1] == 0)
                        goto ProcessNextEnvelope;
                EndofProcessNEnvi:;
                    u4 eax = p1; // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
                    asm volatile("push %%ebp;  mov %0, %%ebp;  call %A4;  pop %%ebp"
                                 : "+a"(eax), "+b"(ebx), "+S"(esi), "+D"(edi)
                                 : "m"(paramhack[2])
                                 : "cc", "memory", "ecx", "edx");
                } while (esi != BufferSizeW);
            }
        } else {
            if (StereoSound == 1) { // NextSampleS.
                do {
                    if (BRRPlace0[p1][0] >= 0x10000000)
                        goto ProcessBRR;
                    Voice0EnvInc[p1] += Voice0IncNumber[p1];
                    if (--Voice0Time[p1] == 0)
                        goto ProcessNextEnvelope;
                EndofProcessNEnvs:;
                    u4 eax = p1; // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
                    asm volatile("push %%ebp;  mov %0, %%ebp;  call %A4;  pop %%ebp"
                                 : "+a"(eax), "+b"(ebx), "+S"(esi), "+D"(edi)
                                 : "m"(paramhack[1])
                                 : "cc", "memory", "ecx", "edx");
                } while (esi != BufferSizeB);
            } else { // NextSample.
                do {
                    if (BRRPlace0[p1][0] >= 0x10000000)
                        goto ProcessBRR;
                    Voice0EnvInc[p1] += Voice0IncNumber[p1];
                    if (--Voice0Time[p1] == 0)
                        goto ProcessNextEnvelope;
                EndofProcessNEnv:;
                    u4 eax = p1; // XXX hack: GCC cannot handle ebp as input/output, so take the detour over eax
                    asm volatile("push %%ebp;  mov %0, %%ebp;  call %A4;  pop %%ebp"
                                 : "+a"(eax), "+b"(ebx), "+S"(esi), "+D"(edi)
                                 : "m"(paramhack[0])
                                 : "cc", "memory", "ecx", "edx");
                } while (esi != BufferSizeW);
            }
        }
        DSPMem[16 * p1 + 8] = ENVDisable == 1 ? 0 : Voice0EnvInc[p1] >> 16;
        return;

    ProcessBRR:
        if (Voice0End[p1] == 1) { // No decode 1 block.
#if 0 // XXX was commented out
			DSPMem[0x5C]    &= ~(1U << p1);
			DSPMem[0x4C]    &= ~(1U << p1);
			Voice0Looped[p1] = 0;
#endif
            if (Voice0Loop[p1] != 1) { // End sample.
                DSPMem[0x7C] |= 1U << p1;
                DSPMem[16 * p1 + 8] = 0;
                DLPFsamples[p1][16] = 0;
                DLPFsamples[p1][17] = 0;
                DLPFsamples[p1][18] = 0;
                DLPFsamples[p1][19] = 0;
#if 0 // XXX was commented out
				DSPMem[0x5C]        &= ~(1U << p1);
#endif
                Voice0EnvInc[p1] = 0;
                Voice0IncNumber[p1] = 0;
                Voice0Status[p1] = 0;
#if 0 // XXX was commented out
				DSPMem[16 * p1 + 9]  = 0;
#endif
                return;
            }
#if 0 // XXX was commented out
			Voice0Looped[p1] = 1;
#endif
            SoundLooped0[p1] = 1;
            DSPMem[0x7C] |= 1U << p1;
#if 0 // XXX was commented out
			Voice0Prev0[p1] = 0;
			Voice0Prev1[p1] = 0;
#endif

#if 0 // XXX was commented out
			{
				u2 const ax = DSPMem[0x5D] * 256 + DSPMem[16 * p1 + 4] * 4;
				Voice0Ptr[p1]     = *(u2 const*)&SPCRAM[ax];
				Voice0LoopPtr[p1] = *(u2 const*)&SPCRAM[ax + 2];
			}
#endif

            Voice0Ptr[p1] = Voice0LoopPtr[p1];
#if 0 // XXX was commented out
			Voice0Prev1[p1] = Voice0Prev0[p1];
#endif
        }

        // Decode 1 block.
        BRRPlace0[p1][0] -= 0x10000000;
        {
            u4 const esi_ = Voice0Ptr[p1];

            PSampleBuf[p1][0] = PSampleBuf[p1][16];
            PSampleBuf[p1][1] = PSampleBuf[p1][17];
            PSampleBuf[p1][2] = PSampleBuf[p1][18];

            s2* edi = (s2*)spcBuffera + (esi_ + 1) * 2;
            Voice0BufPtr[p1] = edi;
            u1* esi = SPCRAM + esi_;
            prev0 = Voice0Prev0[p1];
            prev1 = Voice0Prev1[p1];
            u4 eax;
            u4 ecx;
            u4 edx;
            u4 ebx;
            asm volatile("push %%ebp;  call %P6;  pop %%ebp"
                         : "=a"(eax), "=c"(ecx), "=d"(edx), "=b"(ebx), "+S"(esi), "+D"(edi)
                         : "X"(BRRDecode), "c"(p1)
                         : "cc", "memory");
        }

        edi = Voice0BufPtr[p1];
        PSampleBuf[p1][3] = edi[0];
        PSampleBuf[p1][4] = edi[1];
        PSampleBuf[p1][5] = edi[2];
        PSampleBuf[p1][6] = edi[3];
        PSampleBuf[p1][7] = edi[4];
        PSampleBuf[p1][8] = edi[5];
        PSampleBuf[p1][9] = edi[6];
        PSampleBuf[p1][10] = edi[7];
        PSampleBuf[p1][11] = edi[8];
        PSampleBuf[p1][12] = edi[9];
        PSampleBuf[p1][13] = edi[10];
        PSampleBuf[p1][14] = edi[11];
        PSampleBuf[p1][15] = edi[12];
        PSampleBuf[p1][16] = edi[13];
        PSampleBuf[p1][17] = edi[14];
        PSampleBuf[p1][18] = edi[15];

        PSampleBuf[p1][19] = BRRreadahead[0];
        PSampleBuf[p1][20] = BRRreadahead[1];
        PSampleBuf[p1][21] = BRRreadahead[2];
        PSampleBuf[p1][22] = BRRreadahead[3];

        Voice0Prev0[p1] = prev0;
        Voice0Prev1[p1] = prev1;
        Voice0Loop[p1] = loopbl;
        Voice0End[p1] = lastbl;
        Voice0Ptr[p1] += 9;
    }

ProcessNextEnvelope:
    switch (Voice0State[p1]) {
    case 10: // ADSRSustain.
    {
        u4 const bl = GainDecBendDataPos[p1];
        u1 const dh = AdsrBendData[bl + 1];
        u1 const al = (u1)VolumeConvTable[AdsrBendData[bl] << 8 | GainDecBendDataDat[p1]];
        Voice0EnvInc[p1] = al << 16;
        if (dh != 255) { // More ADSR.
            u4 const ebx = AdsrNextTimeDepth[p1];
            Voice0Time[p1] = ebx;
            Voice0IncNumber[p1] = -((u1)(al - (u1)VolumeConvTable[dh << 8 | GainDecBendDataDat[p1]]) * 65536 / ebx);
            ++GainDecBendDataPos[p1];
            goto ContinueGain;
        } else {
            Voice0State[p1] = 5;
            goto MuteGain;
        }
    }

    case 9: // ADSRDecayProc.
    {
        u4 const bl = GainDecBendDataPos[p1]++;
        u1 const al = (u1)VolumeConvTable[AdsrBendData[bl] << 8 | GainDecBendDataDat[p1]];
        u4 const ebx = GainDecBendDataTime[p1];
        Voice0EnvInc[p1] = al << 16;
        Voice0Time[p1] = ebx;
        Voice0IncNumber[p1] = -((u1)(al - (u1)VolumeConvTable[AdsrBendData[bl + 1] << 8 | GainDecBendDataDat[p1]]) * 65536 / ebx);
        if (--AdsrBlocksLeft[p1] != 0)
            Voice0State[p1] = 10;
        goto ContinueGain;
    }

    case 7: // DecreaseBent.
    {
        u1 const bl = GainDecBendDataPos[p1];
        u1 const dl = GainDecBendDataDat[p1];
        u1 const al = (u1)VolumeConvTable[GainDecBendData[bl] << 8 | dl];
        Voice0EnvInc[p1] = al << 16;
        u1 const dh = GainDecBendData[bl + 1];
        if (dh != 255) { // More.
            u4 const ebx = GainDecBendDataTime[p1];
            Voice0Time[p1] = ebx;
            Voice0IncNumber[p1] = -((u1)(al - (u1)VolumeConvTable[dh << 8 | dl]) * 65536 / ebx);
            ++GainDecBendDataPos[p1];
            goto ContinueGain;
        } else {
            Voice0State[p1] = 5;
            goto MuteGain;
        }
    }

    case 8: // ADSRDecay.
    {
        u4 const edx = DecayRate[DSPMem[16 * p1 + 5] >> 4 & 0x07];
        u4 const ebx = SustainRate[DSPMem[16 * p1 + 6] & 0x1F];
        if (edx >= ebx) { // Decay over.
            u4 const ebx_ = (ebx + (u4)((u8)(edx - ebx) * (DSPMem[16 * p1 + 6] >> 5 ^ 0x07) / 7)) >> 5;
            Voice0EnvInc[p1] = 0x007FFFFF;
            Voice0Time[p1] = ebx_;
            GainDecBendDataTime[p1] = ebx_;
            GainDecBendDataPos[p1] = 0;
            GainDecBendDataDat[p1] = 127;
            Voice0IncNumber[p1] = -((127 - 118) * 65536 / ebx_);
            Voice0State[p1] = 7;
        } else {
            // ebx = total sustain time
            u1 const al = AdsrSustLevLoc[DSPMem[16 * p1 + 6] >> 5];
            /* Traverse through al entries in edx time, then through 64-al entries in ebx-edx time. */
            AdsrBlocksLeft[p1] = al;
            Voice0Time[p1] = edx / al;
            GainDecBendDataTime[p1] = edx / al;
            AdsrNextTimeDepth[p1] = (ebx - edx) / (64 - al);
            Voice0EnvInc[p1] = 0x007FFFFF;
            GainDecBendDataPos[p1] = 0;
            GainDecBendDataDat[p1] = 127;
            Voice0IncNumber[p1] = -((127 - 122) * 65536 / Voice0Time[p1]);
            Voice0State[p1] = 9;
        }
        goto ContinueGain;
    }

        u4 ebx_;

    case 1: // Decay.
    {
        // Calculate Decay Value
        Voice0EnvInc[p1] = 0x007FFFFF;
        u1 const al = DSPMem[16 * p1 + 5] >> 4 & 0x07;
        u1 const dl = DSPMem[16 * p1 + 6] & 0x1F;
        u4 ebx = DecayRate[al];
        if (dl != 0x1F && ebx > SustainRate[dl]) { // Decay fix.
            if (al == 0 && (DSPMem[16 * p1 + 6] & 0xE0) == 0xE0) { // Decay skip.
                ebx_ = SustainRate[dl];
                goto continuesust;
            }
            ebx = DecayRate[DSPMem[16 * p1 + 5] >> 4 & 0x07] - SustainRate[dl];
            if (ebx < SustainRate[dl])
                ebx = SustainRate[dl];
        }
        if (ebx == 0)
            ebx = 1;
        Voice0Time[p1] = ebx;
        static u1 const SustainValue[] = { 15, 31, 47, 63, 79, 95, 111, 127 };
        Voice0IncNumber[p1] = -((SustainValue[DSPMem[16 * p1 + 6] >> 5 & 0x07] ^ 0x7F) * 65536 / ebx);
        Voice0State[p1] = 2;
        goto ContinueGain;
    }

    case 2: // Sustain.
    {
        // Calculate Decay Value
        ebx_ = SustainRate[DSPMem[16 * p1 + 6] & 0x1F];
        if (!(ebx_ & 0x80000000)) { // Sustain not okay.
            ebx_ -= DecayRate[DSPMem[16 * p1 + 5] >> 4 & 0x07];
        continuesust:
            if (ebx_ <= 100)
                ebx_ = 100;
        }
        Voice0Time[p1] = ebx_;
        Voice0IncNumber[p1] = -((Voice0EnvInc[p1] & 0x00FF0000) / ebx_);
        Voice0State[p1] = 4;
        goto ContinueGain;
    }

    case 3: // Blank.
        Voice0EnvInc[p1] = 0x007F0000;
        Voice0IncNumber[p1] = 0;
        Voice0Time[p1] = 0xFFFFFFFF;
        goto ContinueGain;

    case 4:
    case 200:
    default: // EndofSamp.
        DLPFsamples[p1][16] = 0;
        DLPFsamples[p1][17] = 0;
        DLPFsamples[p1][18] = 0;
        DLPFsamples[p1][19] = 0;
        Voice0EnvInc[p1] = 0;
        Voice0IncNumber[p1] = 0;
        Voice0Status[p1] = 0;
        Voice0State[p1] = 0;
        DSPMem[16 * p1 + 8] = 0;
        DSPMem[16 * p1 + 9] = 0;
        DSPMem[0x7C] |= 1U << p1;
        return;

    case 210: // EndofSamp2.
        Voice0EnvInc[p1] = 0;
        Voice0IncNumber[p1] = 0;
        Voice0State[p1] = 0;
        DSPMem[16 * p1 + 8] = 0;
        DSPMem[16 * p1 + 9] = 0;
        VoiceStarter(p1);
        goto SkipProcess2;

    case 5: // MuteGain.
    MuteGain:
        Voice0EnvInc[p1] = 0;
        Voice0IncNumber[p1] = 0;
        Voice0Time[p1] = 0xFFFFFFFF;
        goto ContinueGain;

    case 6: // IncreaseBent.
        Voice0Time[p1] = Increase[DSPMem[16 * p1 + 7] & 0x1F];
        Voice0IncNumber[p1] >>= 2;
        Voice0State[p1] = 3;
        goto ContinueGain;

    ContinueGain:
        ebx = Voice0Freq[p1];
        if (DSPInterpolate != 0) {
            if (StereoSound == 1)
                goto EndofProcessNEnvsi;
            goto EndofProcessNEnvi;
        } else {
            if (StereoSound == 1)
                goto EndofProcessNEnvs;
            goto EndofProcessNEnv;
        }
    }
}

void ProcessVoiceHandler16(u4 const p1)
{
    if (Voice0Disable[p1] != 1)
        return;
    if (Voice0Status[p1] != 1)
        return;

    powhack = 1U << p1;

    if (p1 == 0 || Voice0Disable[p1 - 1] != 1 || Voice0Status[p1 - 1] != 1 || !(DSPMem[0x2D] & 1U << p1) || DSPMem[16 * p1 + 4] == DSPMem[16 * (p1 - 1) + 4]) { // No pitch mod.
        if (DSPMem[0x3D] & 1U << p1 || echoon0[p1] != 1) { // No echo.
            paramhack[0] = NonEchoMono;
            paramhack[1] = NonEchoStereo;
            paramhack[2] = NonEchoMonoInterpolated;
            paramhack[3] = NonEchoStereoInterpolated;
        } else { // Process Echo.
            paramhack[0] = EchoMono;
            paramhack[1] = EchoStereo;
            paramhack[2] = EchoMonoInterpolated;
            paramhack[3] = EchoStereoInterpolated;
        }
    } else { // Pitch mod.
        if (DSPMem[0x3D] & 1U << p1 || echoon0[p1] != 1) { // No Echo PM.
            paramhack[0] = NonEchoMonoPM;
            paramhack[1] = NonEchoStereoPM;
            paramhack[2] = NonEchoMonoPM;
            paramhack[3] = NonEchoStereoPM;
        } else { // Echo PM
            paramhack[0] = EchoMonoPM;
            paramhack[1] = EchoStereoPM;
            paramhack[2] = EchoMonoPM;
            paramhack[3] = EchoStereoPM;
        }
    }

    ProcessVoiceStuff(p1);
}
