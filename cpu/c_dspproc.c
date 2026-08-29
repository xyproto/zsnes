#include <string.h>

#include "../c_init.h"
#include "../cfg.h"
#include "../endmem.h"
#include "../gblvars.h"
#include "../init.h"
#include "../initc.h"
#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof *(x))
#endif
#ifndef endof
#define endof(x) ((x) + lengthof(x))
#endif
#include "../ui.h"
#include "c_dspproc.h"
#include "dspproc.h"
#include "firtable.h"
#include "regs.h"
#include "spc700.h"

// Dispatch ABI for the eight-voice mixers: a mixer reads the voice, the
// decoded-sample buffer (edi) and the DSP-buffer index (*pesi), advances
// *pesi, and updates the increment *pebx in the pitch-modulation variants.
// paramhack[] holds the w_* wrappers, so dispatch is a plain pointer table.
typedef void mixfn(u4 voice, u4* pesi, u4* pebx, s2* edi);
static mixfn* paramhack[4];
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
    // Keep the FIR interpolation mode available with the current implementation.
    u4 const offset = ((BRRPlace0[ebp][0] & 0x00FFFFFF) + 0x1000) >> 9 & 0x0000FFF0;
    s2 const* const coeff = (s2 const*)((u1 const*)fir_lut + offset);
    u4 const base = BRRPlace0[ebp][0] >> 24;
    s4 acc = 0;
    for (u4 i = 0; i != 8; ++i) {
        acc += (s4)(s2)PSampleBuf[ebp][base + i] * coeff[i];
    }
    acc >>= 14;
    if (acc < -32768)
        acc = -32768;
    if (acc > 32767)
        acc = 32767;
    return acc;
}

void AdjustFrequency(void)
{
    u1 al = SoundInterpType;
    if (LowPassFilterType >= 3)
        LowPassFilterType = 0; // HQ

    interpolatefunc* interpolate;
    switch (al) {
    case 0:
        interpolate = 0;
        break;

    case 1: // Gaussian
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
        break;
    }

    case 2: // Cubic spline
    { // Copy from CubicSpline to DSPInterP
        u2 const* ebx = CubicSpline;
        u2* esi = DSPInterP;
        u4 ecx = 1024;
        do {
            u2 const ax = *ebx++;
            *esi++ = ax - ax / 8;
        } while (--ecx != 0);
        interpolate = DSPInterpolate_4;
        break;
    }

    default: // FIR
        interpolate = DSPInterpolate_8;
        break;
    }
    DSPInterpolate = interpolate;

    static u4 const SBToSPCSpeeds[] = { 8000, 11025, 22050, 44100, 16000, 32000, 48000 };
    u4 const eax =
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

    // first fill all pointer to an invalid access function
    // XXX seems to be redundant, all entries are overwritten below
    for (spcop** i = opcjmptab; i != endof(opcjmptab); ++i)
        *i = SpcOpInvalid;

    // now fill the table
    opcjmptab[0x00] = SpcOp00;
    opcjmptab[0x01] = SpcOp01;
    opcjmptab[0x02] = SpcOp02;
    opcjmptab[0x03] = SpcOp03;
    opcjmptab[0x04] = SpcOp04;
    opcjmptab[0x05] = SpcOp05;
    opcjmptab[0x06] = SpcOp06;
    opcjmptab[0x07] = SpcOp07;
    opcjmptab[0x08] = SpcOp08;
    opcjmptab[0x09] = SpcOp09;
    opcjmptab[0x0A] = SpcOp0A;
    opcjmptab[0x0B] = SpcOp0B;
    opcjmptab[0x0C] = SpcOp0C;
    opcjmptab[0x0D] = SpcOp0D;
    opcjmptab[0x0E] = SpcOp0E;
    opcjmptab[0x0F] = SpcOp0F;
    opcjmptab[0x10] = SpcOp10;
    opcjmptab[0x11] = SpcOp11;
    opcjmptab[0x12] = SpcOp12;
    opcjmptab[0x13] = SpcOp13;
    opcjmptab[0x14] = SpcOp14;
    opcjmptab[0x15] = SpcOp15;
    opcjmptab[0x16] = SpcOp16;
    opcjmptab[0x17] = SpcOp17;
    opcjmptab[0x18] = SpcOp18;
    opcjmptab[0x19] = SpcOp19;
    opcjmptab[0x1A] = SpcOp1A;
    opcjmptab[0x1B] = SpcOp1B;
    opcjmptab[0x1C] = SpcOp1C;
    opcjmptab[0x1D] = SpcOp1D;
    opcjmptab[0x1E] = SpcOp1E;
    opcjmptab[0x1F] = SpcOp1F;
    opcjmptab[0x20] = SpcOp20;
    opcjmptab[0x21] = SpcOp21;
    opcjmptab[0x22] = SpcOp22;
    opcjmptab[0x23] = SpcOp23;
    opcjmptab[0x24] = SpcOp24;
    opcjmptab[0x25] = SpcOp25;
    opcjmptab[0x26] = SpcOp26;
    opcjmptab[0x27] = SpcOp27;
    opcjmptab[0x28] = SpcOp28;
    opcjmptab[0x29] = SpcOp29;
    opcjmptab[0x2A] = SpcOp2A;
    opcjmptab[0x2B] = SpcOp2B;
    opcjmptab[0x2C] = SpcOp2C;
    opcjmptab[0x2D] = SpcOp2D;
    opcjmptab[0x2E] = SpcOp2E;
    opcjmptab[0x2F] = SpcOp2F;
    opcjmptab[0x30] = SpcOp30;
    opcjmptab[0x31] = SpcOp31;
    opcjmptab[0x32] = SpcOp32;
    opcjmptab[0x33] = SpcOp33;
    opcjmptab[0x34] = SpcOp34;
    opcjmptab[0x35] = SpcOp35;
    opcjmptab[0x36] = SpcOp36;
    opcjmptab[0x37] = SpcOp37;
    opcjmptab[0x38] = SpcOp38;
    opcjmptab[0x39] = SpcOp39;
    opcjmptab[0x3A] = SpcOp3A;
    opcjmptab[0x3B] = SpcOp3B;
    opcjmptab[0x3C] = SpcOp3C;
    opcjmptab[0x3D] = SpcOp3D;
    opcjmptab[0x3E] = SpcOp3E;
    opcjmptab[0x3F] = SpcOp3F;
    opcjmptab[0x40] = SpcOp40;
    opcjmptab[0x41] = SpcOp41;
    opcjmptab[0x42] = SpcOp42;
    opcjmptab[0x43] = SpcOp43;
    opcjmptab[0x44] = SpcOp44;
    opcjmptab[0x45] = SpcOp45;
    opcjmptab[0x46] = SpcOp46;
    opcjmptab[0x47] = SpcOp47;
    opcjmptab[0x48] = SpcOp48;
    opcjmptab[0x49] = SpcOp49;
    opcjmptab[0x4A] = SpcOp4A;
    opcjmptab[0x4B] = SpcOp4B;
    opcjmptab[0x4C] = SpcOp4C;
    opcjmptab[0x4D] = SpcOp4D;
    opcjmptab[0x4E] = SpcOp4E;
    opcjmptab[0x4F] = SpcOp4F;
    opcjmptab[0x50] = SpcOp50;
    opcjmptab[0x51] = SpcOp51;
    opcjmptab[0x52] = SpcOp52;
    opcjmptab[0x53] = SpcOp53;
    opcjmptab[0x54] = SpcOp54;
    opcjmptab[0x55] = SpcOp55;
    opcjmptab[0x56] = SpcOp56;
    opcjmptab[0x57] = SpcOp57;
    opcjmptab[0x58] = SpcOp58;
    opcjmptab[0x59] = SpcOp59;
    opcjmptab[0x5A] = SpcOp5A;
    opcjmptab[0x5B] = SpcOp5B;
    opcjmptab[0x5C] = SpcOp5C;
    opcjmptab[0x5D] = SpcOp5D;
    opcjmptab[0x5E] = SpcOp5E;
    opcjmptab[0x5F] = SpcOp5F;
    opcjmptab[0x60] = SpcOp60;
    opcjmptab[0x61] = SpcOp61;
    opcjmptab[0x62] = SpcOp62;
    opcjmptab[0x63] = SpcOp63;
    opcjmptab[0x64] = SpcOp64;
    opcjmptab[0x65] = SpcOp65;
    opcjmptab[0x66] = SpcOp66;
    opcjmptab[0x67] = SpcOp67;
    opcjmptab[0x68] = SpcOp68;
    opcjmptab[0x69] = SpcOp69;
    opcjmptab[0x6A] = SpcOp6A;
    opcjmptab[0x6B] = SpcOp6B;
    opcjmptab[0x6C] = SpcOp6C;
    opcjmptab[0x6D] = SpcOp6D;
    opcjmptab[0x6E] = SpcOp6E;
    opcjmptab[0x6F] = SpcOp6F;
    opcjmptab[0x70] = SpcOp70;
    opcjmptab[0x71] = SpcOp71;
    opcjmptab[0x72] = SpcOp72;
    opcjmptab[0x73] = SpcOp73;
    opcjmptab[0x74] = SpcOp74;
    opcjmptab[0x75] = SpcOp75;
    opcjmptab[0x76] = SpcOp76;
    opcjmptab[0x77] = SpcOp77;
    opcjmptab[0x78] = SpcOp78;
    opcjmptab[0x79] = SpcOp79;
    opcjmptab[0x7A] = SpcOp7A;
    opcjmptab[0x7B] = SpcOp7B;
    opcjmptab[0x7C] = SpcOp7C;
    opcjmptab[0x7D] = SpcOp7D;
    opcjmptab[0x7E] = SpcOp7E;
    opcjmptab[0x7F] = SpcOp7F;
    opcjmptab[0x80] = SpcOp80;
    opcjmptab[0x81] = SpcOp81;
    opcjmptab[0x82] = SpcOp82;
    opcjmptab[0x83] = SpcOp83;
    opcjmptab[0x84] = SpcOp84;
    opcjmptab[0x85] = SpcOp85;
    opcjmptab[0x86] = SpcOp86;
    opcjmptab[0x87] = SpcOp87;
    opcjmptab[0x88] = SpcOp88;
    opcjmptab[0x89] = SpcOp89;
    opcjmptab[0x8A] = SpcOp8A;
    opcjmptab[0x8B] = SpcOp8B;
    opcjmptab[0x8C] = SpcOp8C;
    opcjmptab[0x8D] = SpcOp8D;
    opcjmptab[0x8E] = SpcOp8E;
    opcjmptab[0x8F] = SpcOp8F;
    opcjmptab[0x90] = SpcOp90;
    opcjmptab[0x91] = SpcOp91;
    opcjmptab[0x92] = SpcOp92;
    opcjmptab[0x93] = SpcOp93;
    opcjmptab[0x94] = SpcOp94;
    opcjmptab[0x95] = SpcOp95;
    opcjmptab[0x96] = SpcOp96;
    opcjmptab[0x97] = SpcOp97;
    opcjmptab[0x98] = SpcOp98;
    opcjmptab[0x99] = SpcOp99;
    opcjmptab[0x9A] = SpcOp9A;
    opcjmptab[0x9B] = SpcOp9B;
    opcjmptab[0x9C] = SpcOp9C;
    opcjmptab[0x9D] = SpcOp9D;
    opcjmptab[0x9E] = SpcOp9E;
    opcjmptab[0x9F] = SpcOp9F;
    opcjmptab[0xA0] = SpcOpA0;
    opcjmptab[0xA1] = SpcOpA1;
    opcjmptab[0xA2] = SpcOpA2;
    opcjmptab[0xA3] = SpcOpA3;
    opcjmptab[0xA4] = SpcOpA4;
    opcjmptab[0xA5] = SpcOpA5;
    opcjmptab[0xA6] = SpcOpA6;
    opcjmptab[0xA7] = SpcOpA7;
    opcjmptab[0xA8] = SpcOpA8;
    opcjmptab[0xA9] = SpcOpA9;
    opcjmptab[0xAA] = SpcOpAA;
    opcjmptab[0xAB] = SpcOpAB;
    opcjmptab[0xAC] = SpcOpAC;
    opcjmptab[0xAD] = SpcOpAD;
    opcjmptab[0xAE] = SpcOpAE;
    opcjmptab[0xAF] = SpcOpAF;
    opcjmptab[0xB0] = SpcOpB0;
    opcjmptab[0xB1] = SpcOpB1;
    opcjmptab[0xB2] = SpcOpB2;
    opcjmptab[0xB3] = SpcOpB3;
    opcjmptab[0xB4] = SpcOpB4;
    opcjmptab[0xB5] = SpcOpB5;
    opcjmptab[0xB6] = SpcOpB6;
    opcjmptab[0xB7] = SpcOpB7;
    opcjmptab[0xB8] = SpcOpB8;
    opcjmptab[0xB9] = SpcOpB9;
    opcjmptab[0xBA] = SpcOpBA;
    opcjmptab[0xBB] = SpcOpBB;
    opcjmptab[0xBC] = SpcOpBC;
    opcjmptab[0xBD] = SpcOpBD;
    opcjmptab[0xBE] = SpcOpBE;
    opcjmptab[0xBF] = SpcOpBF;
    opcjmptab[0xC0] = SpcOpC0;
    opcjmptab[0xC1] = SpcOpC1;
    opcjmptab[0xC2] = SpcOpC2;
    opcjmptab[0xC3] = SpcOpC3;
    opcjmptab[0xC4] = SpcOpC4;
    opcjmptab[0xC5] = SpcOpC5;
    opcjmptab[0xC6] = SpcOpC6;
    opcjmptab[0xC7] = SpcOpC7;
    opcjmptab[0xC8] = SpcOpC8;
    opcjmptab[0xC9] = SpcOpC9;
    opcjmptab[0xCA] = SpcOpCA;
    opcjmptab[0xCB] = SpcOpCB;
    opcjmptab[0xCC] = SpcOpCC;
    opcjmptab[0xCD] = SpcOpCD;
    opcjmptab[0xCE] = SpcOpCE;
    opcjmptab[0xCF] = SpcOpCF;
    opcjmptab[0xD0] = SpcOpD0;
    opcjmptab[0xD1] = SpcOpD1;
    opcjmptab[0xD2] = SpcOpD2;
    opcjmptab[0xD3] = SpcOpD3;
    opcjmptab[0xD4] = SpcOpD4;
    opcjmptab[0xD5] = SpcOpD5;
    opcjmptab[0xD6] = SpcOpD6;
    opcjmptab[0xD7] = SpcOpD7;
    opcjmptab[0xD8] = SpcOpD8;
    opcjmptab[0xD9] = SpcOpD9;
    opcjmptab[0xDA] = SpcOpDA;
    opcjmptab[0xDB] = SpcOpDB;
    opcjmptab[0xDC] = SpcOpDC;
    opcjmptab[0xDD] = SpcOpDD;
    opcjmptab[0xDE] = SpcOpDE;
    opcjmptab[0xDF] = SpcOpDF;
    opcjmptab[0xE0] = SpcOpE0;
    opcjmptab[0xE1] = SpcOpE1;
    opcjmptab[0xE2] = SpcOpE2;
    opcjmptab[0xE3] = SpcOpE3;
    opcjmptab[0xE4] = SpcOpE4;
    opcjmptab[0xE5] = SpcOpE5;
    opcjmptab[0xE6] = SpcOpE6;
    opcjmptab[0xE7] = SpcOpE7;
    opcjmptab[0xE8] = SpcOpE8;
    opcjmptab[0xE9] = SpcOpE9;
    opcjmptab[0xEA] = SpcOpEA;
    opcjmptab[0xEB] = SpcOpEB;
    opcjmptab[0xEC] = SpcOpEC;
    opcjmptab[0xED] = SpcOpED;
    opcjmptab[0xEE] = SpcOpEE;
    opcjmptab[0xEF] = SpcOpEF;
    opcjmptab[0xF0] = SpcOpF0;
    opcjmptab[0xF1] = SpcOpF1;
    opcjmptab[0xF2] = SpcOpF2;
    opcjmptab[0xF3] = SpcOpF3;
    opcjmptab[0xF4] = SpcOpF4;
    opcjmptab[0xF5] = SpcOpF5;
    opcjmptab[0xF6] = SpcOpF6;
    opcjmptab[0xF7] = SpcOpF7;
    opcjmptab[0xF8] = SpcOpF8;
    opcjmptab[0xF9] = SpcOpF9;
    opcjmptab[0xFA] = SpcOpFA;
    opcjmptab[0xFB] = SpcOpFB;
    opcjmptab[0xFC] = SpcOpFC;
    opcjmptab[0xFD] = SpcOpFD;
    opcjmptab[0xFE] = SpcOpFE;
    opcjmptab[0xFF] = SpcOpFF;
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

// --- BRR sample decoder (cpu/dspproc.asm) -----------------------------------
//
// A BRR block is 9 bytes: a header (range<<4 | filter<<2 | loop<<1 | end) and
// 8 data bytes of two 4-bit samples each, so 16 samples. The filter is a 2-tap
// IIR over prev0/prev1, the running output history; the stored sample is the
// clamped, doubled, truncated new prev0. Every shift is arithmetic.

// filter0 coefficient key selected by the header's filter field (0..3).
static s4 brr_filter0(u1 const hdr)
{
    switch ((hdr >> 2) & 0x03) {
    case 1:  return 240;
    case 2:  return 488;
    case 3:  return 460;
    default: return 0;
    }
}

static s2 brr_next_sample(u1 const nibble, u1 const bshift, s4 const filter0)
{
    s4 delta = (s4)((nibble ^ 8) - 8); // sign-extend the 4-bit nibble to -8..7

    if (bshift <= 12) {
        delta = (s4)((u4)delta << bshift);
        delta >>= 1;
    } else {
        delta &= ~0x7FF; // ranges 13..15 are treated as a fixed clamp
    }

    s4 const p0 = (s4)prev0;
    s4 const p1 = (s4)prev1;
    s4 out = delta;

    if (filter0 == 240) {
        out += (p0 >> 1) + ((-p0) >> 5);
    } else if (filter0 == 488) {
        out += p0 + ((-(p0 + (p0 >> 1))) >> 5) - (p1 >> 1) + (p1 >> 5);
    } else if (filter0 == 460) {
        out += p0 + ((-(13 * p0)) >> 7) - (p1 >> 1) + (((p1 >> 1) + p1) >> 4);
    }

    if (out < -32768) out = -32768;
    if (out > 32767) out = 32767;

    prev1 = (u4)p0;
    out = (s2)(out << 1); // double and truncate to 16 bits
    prev0 = (u4)out;
    return (s2)out;
}

// Dynamic low-pass over the 16 decoded samples (only for LowPassFilterType > 1
// and a fast enough voice). A moving average of 2..5 taps, seeded from the
// previous block's tail kept in DLPFsamples[voice][]. The /3 and /5 use the
// same reciprocal-multiply (high 32 bits of a signed 64-bit product) the
// assembly used, so the rounding matches bit for bit.
static s4 dlpf_recip_mul(s4 const sum, s4 const recip)
{
    return (s4)(((s8)sum * (s8)recip) >> 32);
}

static void brr_dynamic_lowpass(u4 const voice, s2* edi)
{
    if (Voice0Freq[voice] <= 0x800000) {
        return;
    }

    s4* const hist = (s4*)&DLPFsamples[voice][0];

    if (LowPassFilterType != 3) {
        // "dynamic": choose the tap count from the top byte of the frequency
        hist[0] = hist[16];
        hist[1] = hist[17];
        hist[2] = hist[18];
        hist[3] = hist[19];
        edi -= 16; // rewind to the first of the 16 samples just decoded
        hist[16] = edi[12];
        hist[17] = edi[13];
        hist[18] = edi[14];
        hist[19] = edi[15];

        // The original dispatch compares the frequency's top byte as *signed*
        // (cmp dl,N / jle), so a top byte >= 128 falls into the 2-tap path.
        s1 const sel = (s1)(Voice0Freq[voice] >> 24);
        if (sel <= 2) {
            s4 t0 = hist[4];
            for (int i = 0; i < 16; i++) {
                s4 const cur = edi[i];
                edi[i] = (s2)((t0 + cur) >> 1);
                t0 = cur;
            }
            return;
        }
        if (sel <= 3) {
            s4 t0 = hist[3], t1 = hist[4];
            for (int i = 0; i < 16; i++) {
                s4 const cur = edi[i];
                edi[i] = (s2)dlpf_recip_mul(t0 + cur + t1, 0x55555555);
                t0 = t1;
                t1 = cur;
            }
            return;
        }
        if (sel <= 4) {
            s4 t0 = hist[2], t1 = hist[3], t2 = hist[4];
            for (int i = 0; i < 16; i++) {
                s4 const cur = edi[i];
                edi[i] = (s2)((t0 + cur + t1 + t2) >> 2);
                t0 = t1;
                t1 = t2;
                t2 = cur;
            }
            return;
        }
        // sel >= 5 falls through to the 5-tap path below.
    }

    // 5-tap (LowPassFilterType == 3, or the dynamic sel >= 5 case).
    s4 t0 = hist[1], t1 = hist[2], t2 = hist[3], t3 = hist[4];
    for (int i = 0; i < 16; i++) {
        s4 const cur = edi[i];
        edi[i] = (s2)dlpf_recip_mul(t0 + cur + t1 + t2 + t3, 0x33333333);
        t0 = t1;
        t1 = t2;
        t2 = t3;
        t3 = cur;
    }
}

void BRRDecode(u4 const voice, u1* esi, s2* edi)
{
    lastbl = 0;
    loopbl = 0;

    u1 const hdr = *esi++;
    if (hdr & 0x01) {
        lastbl = 1;
        if (hdr & 0x02) {
            loopbl = 1;
        }
    }
    u1 const bshift = hdr >> 4;
    s4 const filter0 = brr_filter0(hdr);

    for (int i = 0; i < 8; i++) {
        edi[0] = brr_next_sample(*esi >> 4, bshift, filter0);
        edi[1] = brr_next_sample(*esi & 0x0F, bshift, filter0);
        edi += 2;
        esi++;
    }

    // Decode the next block's first 4 samples ahead of time when the output
    // needs them (interpolation, or the fast-voice dynamic low pass).
    int do_readahead = (DSPInterpolate != 0);
    if (!do_readahead && LowPassFilterType > 2 && Voice0Freq[voice] >= 0x800000) {
        do_readahead = 1;
    }

    if (do_readahead) {
        if (lastbl == 1 && loopbl != 1) {
            BRRreadahead[0] = 0;
            BRRreadahead[1] = 0;
            BRRreadahead[2] = 0;
            BRRreadahead[3] = 0;
        } else {
            u1* rsrc = (lastbl == 1) ? SPCRAM + Voice0LoopPtr[voice] : esi;
            u4 const save0 = prev0, save1 = prev1;

            u1 const h2 = *rsrc++;
            u1 const bs2 = h2 >> 4;
            s4 const f2 = brr_filter0(h2);
            BRRreadahead[0] = brr_next_sample(*rsrc >> 4, bs2, f2);
            BRRreadahead[1] = brr_next_sample(*rsrc & 0x0F, bs2, f2);
            rsrc++;
            BRRreadahead[2] = brr_next_sample(*rsrc >> 4, bs2, f2);
            BRRreadahead[3] = brr_next_sample(*rsrc & 0x0F, bs2, f2);

            prev1 = save1;
            prev0 = save0;
        }
    }

    if (LowPassFilterType > 1) {
        brr_dynamic_lowpass(voice, edi);
    }
}

extern u1 NoiseData[];   // defined in ui.c
extern u4 NoiseInc;      // defined in cpu/dspproc.c
extern u4 NoisePointer;
extern u1 PModBuffer[];

// All twelve voice mixers (the eight base variants plus the four interpolated
// ones) are now C in dsp_mixers.h, shared verbatim with the diff-test.
#include "dsp_mixers.h"

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
                    paramhack[3](p1, &esi, &ebx, edi);
                } while (esi != BufferSizeB);
            } else { // NextSamplei.
                do {
                    if (BRRPlace0[p1][0] >= 0x10000000)
                        goto ProcessBRR;
                    Voice0EnvInc[p1] += Voice0IncNumber[p1];
                    if (--Voice0Time[p1] == 0)
                        goto ProcessNextEnvelope;
                EndofProcessNEnvi:;
                    paramhack[2](p1, &esi, &ebx, edi);
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
                    paramhack[1](p1, &esi, &ebx, edi);
                } while (esi != BufferSizeB);
            } else { // NextSample.
                do {
                    if (BRRPlace0[p1][0] >= 0x10000000)
                        goto ProcessBRR;
                    Voice0EnvInc[p1] += Voice0IncNumber[p1];
                    if (--Voice0Time[p1] == 0)
                        goto ProcessNextEnvelope;
                EndofProcessNEnv:;
                    paramhack[0](p1, &esi, &ebx, edi);
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
            BRRDecode(p1, esi, edi);
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
            paramhack[0] = w_NonEchoMono;
            paramhack[1] = w_NonEchoStereo;
            paramhack[2] = w_NonEchoMonoInterpolated;
            paramhack[3] = w_NonEchoStereoInterpolated;
        } else { // Process Echo.
            paramhack[0] = w_EchoMono;
            paramhack[1] = w_EchoStereo;
            paramhack[2] = w_EchoMonoInterpolated;
            paramhack[3] = w_EchoStereoInterpolated;
        }
    } else { // Pitch mod.
        if (DSPMem[0x3D] & 1U << p1 || echoon0[p1] != 1) { // No Echo PM.
            paramhack[0] = w_NonEchoMonoPM;
            paramhack[1] = w_NonEchoStereoPM;
            paramhack[2] = w_NonEchoMonoPM;
            paramhack[3] = w_NonEchoStereoPM;
        } else { // Echo PM
            paramhack[0] = w_EchoMonoPM;
            paramhack[1] = w_EchoStereoPM;
            paramhack[2] = w_EchoMonoPM;
            paramhack[3] = w_EchoStereoPM;
        }
    }

    ProcessVoiceStuff(p1);
}

extern u1 AudioLogging; // defined in zmovie.c
extern u1 EMUPause; // defined in cpu/execute.asm

// Whether the echo buffer currently holds data that must be cleared once echo
// is switched off (kept across calls, so it stays here rather than as a local).
static u1 echowrittento;

// Top-level sound generation for one buffer: clears the mix/echo buffers, runs
// the eight voices, mixes echo, optionally reverses stereo, and applies the
// low-pass filter. Ported from cpu/dspproc.asm; the heavy lifting still lives
// in the C helpers it calls (ProcessVoiceHandler16, MixEcho/MixEcho2, LPF*).
void ProcessSoundBuffer(void)
{
    memset(DSPBuffer, 0, (size_t)BufferSizeB * 4);

    int const echo_off = (EchoDis == 1) || (DSPMem[0x6C] & 0x20);
    if (!echo_off) {
        memset(EchoBuffer, 0, (size_t)BufferSizeB * 4);
    }

    if (EMUPause == 1) {
        return;
    }
    if (AudioLogging == 1) { // logging enabled but skipping this pass
        return;
    }

    for (u4 v = 0; v < 8; v++) {
        ProcessVoiceHandler16(v);
    }

    if (EchoDis != 1) {
        if (DSPMem[0x6C] & 0x20) { // echo writes disabled: flush any stale echo
            if (echowrittento != 0) {
                u4 n = MaxEcho;
                if (StereoSound == 1) {
                    n += n;
                }
                memset(echobuf, 0, (size_t)n * 4);
                echowrittento = 0;
            }
        } else {
            echowrittento = 1;
            if (FIRTAPVal0[0] == 0x7F && FIRTAPVal0[1] == 0 && FIRTAPVal0[2] == 0
                && FIRTAPVal0[3] == 0 && FIRTAPVal0[4] == 0 && FIRTAPVal0[5] == 0
                && FIRTAPVal0[6] == 0 && FIRTAPVal0[7] == 0) {
                MixEcho2(); // trivial FIR: identity echo
            } else {
                MixEcho();
            }
        }
    }

    if (RevStereo != 0) {
        s4* p = DSPBuffer;
        for (u4 i = BufferSizeB >> 1; i != 0; i--) {
            s4 const l = p[0];
            p[0] = p[1];
            p[1] = l;
            p += 2;
        }
    }

    if (LowPassFilterType != 1) {
        LPFexit();
        return;
    }

    if (StereoSound == 1) {
        LPFstereo(DSPBuffer);
        return;
    }

    // Mono low-pass: a running half-sum filter over the buffer, two samples at
    // a time (the accumulator alternates between the two locals, as in the asm).
    s4* esi = DSPBuffer;
    s4 ebx = LPFsample1;
    for (u4 i = BufferSizeB >> 1; i != 0; i--) {
        s4 eax = *esi >> 1;
        ebx += eax;
        *esi++ = ebx;
        ebx = *esi >> 1;
        eax += ebx;
        *esi++ = eax;
    }
    LPFsample1 = ebx;
    LPFexit();
}
