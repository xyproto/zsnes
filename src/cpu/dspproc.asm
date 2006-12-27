;Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
;https://zsnes.bountysource.com
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;version 2 as published by the Free Software Foundation.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

%include "macros.mac"

EXTSYM SPCRAM,spcPCRam,spcRamDP,spcS,spcX,RevStereo
EXTSYM Invalidopcode,RSPCRegF0,RSPCRegF1,RSPCRegF2,RSPCRegF3
EXTSYM RSPCRegF4,RSPCRegF5,RSPCRegF6,RSPCRegF7,RSPCRegF8,RSPCRegF9
EXTSYM RSPCRegFA,RSPCRegFB,RSPCRegFC,RSPCRegFD,RSPCRegFE,RSPCRegFF
EXTSYM SPCRegF0,SPCRegF1,SPCRegF2,SPCRegF3,SPCRegF4,SPCRegF5,SPCRegF6
EXTSYM SPCRegF7,SPCRegF8,SPCRegF9,SPCRegFA,SPCRegFB,SPCRegFC,SPCRegFD
EXTSYM SPCRegFE,SPCRegFF,VolumeConvTable,dspRptr,dspWptr,opcjmptab
EXTSYM Op00,Op01,Op02,Op03,Op04,Op05,Op06,Op07,Op08,Op09,Op0A,Op0B,Op0C,Op0D
EXTSYM Op0E,Op0F,Op10,Op11,Op12,Op13,Op14,Op15,Op16,Op17,Op18,Op19,Op1A,Op1B
EXTSYM Op1C,Op1D,Op1E,Op1F,Op20,Op21,Op22,Op23,Op24,Op25,Op26,Op27,Op28,Op29
EXTSYM Op2A,Op2B,Op2C,Op2D,Op2E,Op2F,Op30,Op31,Op32,Op33,Op34,Op35,Op36,Op37
EXTSYM Op38,Op39,Op3A,Op3B,Op3C,Op3D,Op3E,Op3F,Op40,Op41,Op42,Op43,Op44,Op45
EXTSYM Op46,Op47,Op48,Op49,Op4A,Op4B,Op4C,Op4D,Op4E,Op4F,Op50,Op51,Op52,Op53
EXTSYM Op54,Op55,Op56,Op57,Op58,Op59,Op5A,Op5B,Op5C,Op5D,Op5E,Op5F,Op60,Op61
EXTSYM Op62,Op63,Op64,Op65,Op66,Op67,Op68,Op69,Op6A,Op6B,Op6C,Op6D,Op6E,Op6F
EXTSYM Op70,Op71,Op72,Op73,Op74,Op75,Op76,Op77,Op78,Op79,Op7A,Op7B,Op7C,Op7D
EXTSYM Op7E,Op7F,Op80,Op81,Op82,Op83,Op84,Op85,Op86,Op87,Op88,Op89,Op8A,Op8B
EXTSYM Op8C,Op8D,Op8E,Op8F,Op90,Op91,Op92,Op93,Op94,Op95,Op96,Op97,Op98,Op99
EXTSYM Op9A,Op9B,Op9C,Op9D,Op9E,Op9F,OpA0,OpA1,OpA2,OpA3,OpA4,OpA5,OpA6,OpA7
EXTSYM OpA8,OpA9,OpAA,OpAB,OpAC,OpAD,OpAE,OpAF,OpB0,OpB1,OpB2,OpB3,OpB4,OpB5
EXTSYM OpB6,OpB7,OpB8,OpB9,OpBA,OpBB,OpBC,OpBD,OpBE,OpBF,OpC0,OpC1,OpC2,OpC3
EXTSYM OpC4,OpC5,OpC6,OpC7,OpC8,OpC9,OpCA,OpCB,OpCC,OpCD,OpCE,OpCF,OpD0,OpD1
EXTSYM OpD2,OpD3,OpD4,OpD5,OpD6,OpD7,OpD8,OpD9,OpDA,OpDB,OpDC,OpDD,OpDE,OpDF
EXTSYM OpE0,OpE1,OpE2,OpE3,OpE4,OpE5,OpE6,OpE7,OpE8,OpE9,OpEA,OpEB,OpEC,OpED
EXTSYM OpEE,OpEF,OpF0,OpF1,OpF2,OpF3,OpF4,OpF5,OpF6,OpF7,OpF8,OpF9,OpFA,OpFB
EXTSYM OpFC,OpFD,OpFE,OpFF
EXTSYM RDSPReg00,RDSPReg01,RDSPReg02,RDSPReg03,RDSPReg04,RDSPReg05,RDSPReg06
EXTSYM RDSPReg07,RDSPReg08,RDSPReg09,RDSPReg0A,RDSPReg0B,RDSPReg0C,RDSPReg0D
EXTSYM RDSPReg0E,RDSPReg0F,RDSPReg10,RDSPReg11,RDSPReg12,RDSPReg13,RDSPReg14
EXTSYM RDSPReg15,RDSPReg16,RDSPReg17,RDSPReg18,RDSPReg19,RDSPReg1A,RDSPReg1B
EXTSYM RDSPReg1C,RDSPReg1D,RDSPReg1E,RDSPReg1F,RDSPReg20,RDSPReg21,RDSPReg22
EXTSYM RDSPReg23,RDSPReg24,RDSPReg25,RDSPReg26,RDSPReg27,RDSPReg28,RDSPReg29
EXTSYM RDSPReg2A,RDSPReg2B,RDSPReg2C,RDSPReg2D,RDSPReg2E,RDSPReg2F,RDSPReg30
EXTSYM RDSPReg31,RDSPReg32,RDSPReg33,RDSPReg34,RDSPReg35,RDSPReg36,RDSPReg37
EXTSYM RDSPReg38,RDSPReg39,RDSPReg3A,RDSPReg3B,RDSPReg3C,RDSPReg3D,RDSPReg3E
EXTSYM RDSPReg3F,RDSPReg40,RDSPReg41,RDSPReg42,RDSPReg43,RDSPReg44,RDSPReg45
EXTSYM RDSPReg46,RDSPReg47,RDSPReg48,RDSPReg49,RDSPReg4A,RDSPReg4B,RDSPReg4C
EXTSYM RDSPReg4D,RDSPReg4E,RDSPReg4F,RDSPReg50,RDSPReg51,RDSPReg52,RDSPReg53
EXTSYM RDSPReg54,RDSPReg55,RDSPReg56,RDSPReg57,RDSPReg58,RDSPReg59,RDSPReg5A
EXTSYM RDSPReg5B,RDSPReg5C,RDSPReg5D,RDSPReg5E,RDSPReg5F,RDSPReg60,RDSPReg61
EXTSYM RDSPReg62,RDSPReg63,RDSPReg64,RDSPReg65,RDSPReg66,RDSPReg67,RDSPReg68
EXTSYM RDSPReg69,RDSPReg6A,RDSPReg6B,RDSPReg6C,RDSPReg6D,RDSPReg6E,RDSPReg6F
EXTSYM RDSPReg70,RDSPReg71,RDSPReg72,RDSPReg73,RDSPReg74,RDSPReg75,RDSPReg76
EXTSYM RDSPReg77,RDSPReg78,RDSPReg79,RDSPReg7A,RDSPReg7B,RDSPReg7C,RDSPReg7D
EXTSYM RDSPReg7E,RDSPReg7F,RDSPReg80,RDSPReg81,RDSPReg82,RDSPReg83,RDSPReg84
EXTSYM RDSPReg85,RDSPReg86,RDSPReg87,RDSPReg88,RDSPReg89,RDSPReg8A,RDSPReg8B
EXTSYM RDSPReg8C,RDSPReg8D,RDSPReg8E,RDSPReg8F,RDSPReg90,RDSPReg91,RDSPReg92
EXTSYM RDSPReg93,RDSPReg94,RDSPReg95,RDSPReg96,RDSPReg97,RDSPReg98,RDSPReg99
EXTSYM RDSPReg9A,RDSPReg9B,RDSPReg9C,RDSPReg9D,RDSPReg9E,RDSPReg9F,RDSPRegA0
EXTSYM RDSPRegA1,RDSPRegA2,RDSPRegA3,RDSPRegA4,RDSPRegA5,RDSPRegA6,RDSPRegA7
EXTSYM RDSPRegA8,RDSPRegA9,RDSPRegAA,RDSPRegAB,RDSPRegAC,RDSPRegAD,RDSPRegAE
EXTSYM RDSPRegAF,RDSPRegB0,RDSPRegB1,RDSPRegB2,RDSPRegB3,RDSPRegB4,RDSPRegB5
EXTSYM RDSPRegB6,RDSPRegB7,RDSPRegB8,RDSPRegB9,RDSPRegBA,RDSPRegBB,RDSPRegBC
EXTSYM RDSPRegBD,RDSPRegBE,RDSPRegBF,RDSPRegC0,RDSPRegC1,RDSPRegC2,RDSPRegC3
EXTSYM RDSPRegC4,RDSPRegC5,RDSPRegC6,RDSPRegC7,RDSPRegC8,RDSPRegC9,RDSPRegCA
EXTSYM RDSPRegCB,RDSPRegCC,RDSPRegCD,RDSPRegCE,RDSPRegCF,RDSPRegD0,RDSPRegD1
EXTSYM RDSPRegD2,RDSPRegD3,RDSPRegD4,RDSPRegD5,RDSPRegD6,RDSPRegD7,RDSPRegD8
EXTSYM RDSPRegD9,RDSPRegDA,RDSPRegDB,RDSPRegDC,RDSPRegDD,RDSPRegDE,RDSPRegDF
EXTSYM RDSPRegE0,RDSPRegE1,RDSPRegE2,RDSPRegE3,RDSPRegE4,RDSPRegE5,RDSPRegE6
EXTSYM RDSPRegE7,RDSPRegE8,RDSPRegE9,RDSPRegEA,RDSPRegEB,RDSPRegEC,RDSPRegED
EXTSYM RDSPRegEE,RDSPRegEF,RDSPRegF0,RDSPRegF1,RDSPRegF2,RDSPRegF3,RDSPRegF4
EXTSYM RDSPRegF5,RDSPRegF6,RDSPRegF7,RDSPRegF8,RDSPRegF9,RDSPRegFA,RDSPRegFB
EXTSYM RDSPRegFC,RDSPRegFD,RDSPRegFE,RDSPRegFF
EXTSYM WDSPReg00,WDSPReg01,WDSPReg02,WDSPReg03,WDSPReg04,WDSPReg05,WDSPReg06
EXTSYM WDSPReg07,WDSPReg08,WDSPReg09,WDSPReg0A,WDSPReg0B,WDSPReg0C,WDSPReg0D
EXTSYM WDSPReg0E,WDSPReg0F,WDSPReg10,WDSPReg11,WDSPReg12,WDSPReg13,WDSPReg14
EXTSYM WDSPReg15,WDSPReg16,WDSPReg17,WDSPReg18,WDSPReg19,WDSPReg1A,WDSPReg1B
EXTSYM WDSPReg1C,WDSPReg1D,WDSPReg1E,WDSPReg1F,WDSPReg20,WDSPReg21,WDSPReg22
EXTSYM WDSPReg23,WDSPReg24,WDSPReg25,WDSPReg26,WDSPReg27,WDSPReg28,WDSPReg29
EXTSYM WDSPReg2A,WDSPReg2B,WDSPReg2C,WDSPReg2D,WDSPReg2E,WDSPReg2F,WDSPReg30
EXTSYM WDSPReg31,WDSPReg32,WDSPReg33,WDSPReg34,WDSPReg35,WDSPReg36,WDSPReg37
EXTSYM WDSPReg38,WDSPReg39,WDSPReg3A,WDSPReg3B,WDSPReg3C,WDSPReg3D,WDSPReg3E
EXTSYM WDSPReg3F,WDSPReg40,WDSPReg41,WDSPReg42,WDSPReg43,WDSPReg44,WDSPReg45
EXTSYM WDSPReg46,WDSPReg47,WDSPReg48,WDSPReg49,WDSPReg4A,WDSPReg4B,WDSPReg4C
EXTSYM WDSPReg4D,WDSPReg4E,WDSPReg4F,WDSPReg50,WDSPReg51,WDSPReg52,WDSPReg53
EXTSYM WDSPReg54,WDSPReg55,WDSPReg56,WDSPReg57,WDSPReg58,WDSPReg59,WDSPReg5A
EXTSYM WDSPReg5B,WDSPReg5C,WDSPReg5D,WDSPReg5E,WDSPReg5F,WDSPReg60,WDSPReg61
EXTSYM WDSPReg62,WDSPReg63,WDSPReg64,WDSPReg65,WDSPReg66,WDSPReg67,WDSPReg68
EXTSYM WDSPReg69,WDSPReg6A,WDSPReg6B,WDSPReg6C,WDSPReg6D,WDSPReg6E,WDSPReg6F
EXTSYM WDSPReg70,WDSPReg71,WDSPReg72,WDSPReg73,WDSPReg74,WDSPReg75,WDSPReg76
EXTSYM WDSPReg77,WDSPReg78,WDSPReg79,WDSPReg7A,WDSPReg7B,WDSPReg7C,WDSPReg7D
EXTSYM WDSPReg7E,WDSPReg7F,WDSPReg80,WDSPReg81,WDSPReg82,WDSPReg83,WDSPReg84
EXTSYM WDSPReg85,WDSPReg86,WDSPReg87,WDSPReg88,WDSPReg89,WDSPReg8A,WDSPReg8B
EXTSYM WDSPReg8C,WDSPReg8D,WDSPReg8E,WDSPReg8F,WDSPReg90,WDSPReg91,WDSPReg92
EXTSYM WDSPReg93,WDSPReg94,WDSPReg95,WDSPReg96,WDSPReg97,WDSPReg98,WDSPReg99
EXTSYM WDSPReg9A,WDSPReg9B,WDSPReg9C,WDSPReg9D,WDSPReg9E,WDSPReg9F,WDSPRegA0
EXTSYM WDSPRegA1,WDSPRegA2,WDSPRegA3,WDSPRegA4,WDSPRegA5,WDSPRegA6,WDSPRegA7
EXTSYM WDSPRegA8,WDSPRegA9,WDSPRegAA,WDSPRegAB,WDSPRegAC,WDSPRegAD,WDSPRegAE
EXTSYM WDSPRegAF,WDSPRegB0,WDSPRegB1,WDSPRegB2,WDSPRegB3,WDSPRegB4,WDSPRegB5
EXTSYM WDSPRegB6,WDSPRegB7,WDSPRegB8,WDSPRegB9,WDSPRegBA,WDSPRegBB,WDSPRegBC
EXTSYM WDSPRegBD,WDSPRegBE,WDSPRegBF,WDSPRegC0,WDSPRegC1,WDSPRegC2,WDSPRegC3
EXTSYM WDSPRegC4,WDSPRegC5,WDSPRegC6,WDSPRegC7,WDSPRegC8,WDSPRegC9,WDSPRegCA
EXTSYM WDSPRegCB,WDSPRegCC,WDSPRegCD,WDSPRegCE,WDSPRegCF,WDSPRegD0,WDSPRegD1
EXTSYM WDSPRegD2,WDSPRegD3,WDSPRegD4,WDSPRegD5,WDSPRegD6,WDSPRegD7,WDSPRegD8
EXTSYM WDSPRegD9,WDSPRegDA,WDSPRegDB,WDSPRegDC,WDSPRegDD,WDSPRegDE,WDSPRegDF
EXTSYM WDSPRegE0,WDSPRegE1,WDSPRegE2,WDSPRegE3,WDSPRegE4,WDSPRegE5,WDSPRegE6
EXTSYM WDSPRegE7,WDSPRegE8,WDSPRegE9,WDSPRegEA,WDSPRegEB,WDSPRegEC,WDSPRegED
EXTSYM WDSPRegEE,WDSPRegEF,WDSPRegF0,WDSPRegF1,WDSPRegF2,WDSPRegF3,WDSPRegF4
EXTSYM WDSPRegF5,WDSPRegF6,WDSPRegF7,WDSPRegF8,WDSPRegF9,WDSPRegFA,WDSPRegFB
EXTSYM WDSPRegFC,WDSPRegFD,WDSPRegFE,WDSPRegFF
EXTSYM spcBuffera,DSPMem,SoundInterpType,NoiseData,Voice0Disable,EchoDis
EXTSYM Surround,echobuf,ENVDisable,LowPassFilterType,EMUPause,AudioLogging
EXTSYM MMXSupport,StereoSound,SoundQuality

%ifdef __MSDOS__
EXTSYM SB_alloc_dma,SB_quality_limiter,vibracard
%endif

SECTION .data
NEWSYM SBHDMA, db 0         ; stupid legacy code ...

SECTION .bss

; How the sound code works :
; Clear memory at $01 to $EF
; Store $BBAA into $2140
; Wait for $CC in $2142
; Goto FFEF:
;FFD6:
; Move $2140 into Y
; Repeat until Y = 0
;FFDA:
; If $2140 <> 0, then jump to FFE9
; Move $2141 into A
; Move Y into $2140
; Move A into [$00]+Y
; Increment Y
; Jump to FFDA if Y <> 0
; Increment value at $01
;FFE9:
; If plus flag set, goto FFDA
; If (Y-$2140) <> 0, goto FFE9
;FFEF:
; Move 2142 into YA
; Move YA into Address $00
; Move A into 2140
; Move Y into A
; Move A into X
; If A is not zero, goto FFD6
; Jump to Address [0000]

DSPInterP resw 1024

section .data
ALIGN32

Gaussian:
  dw 1305,1305,1304,1304,1304,1304,1304,1303
  dw 1303,1303,1302,1302,1301,1300,1300,1299
  dw 1298,1297,1297,1296,1295,1294,1293,1292
  dw 1291,1290,1288,1287,1286,1284,1283,1282
  dw 1280,1279,1277,1275,1274,1272,1270,1269
  dw 1267,1265,1263,1261,1259,1257,1255,1253
  dw 1251,1248,1246,1244,1241,1239,1237,1234
  dw 1232,1229,1227,1224,1221,1219,1216,1213
  dw 1210,1207,1205,1202,1199,1196,1193,1190
  dw 1186,1183,1180,1177,1174,1170,1167,1164
  dw 1160,1157,1153,1150,1146,1143,1139,1136
  dw 1132,1128,1125,1121,1117,1113,1109,1106
  dw 1102,1098,1094,1090,1086,1082,1078,1074
  dw 1070,1066,1061,1057,1053,1049,1045,1040
  dw 1036,1032,1027,1023,1019,1014,1010,1005
  dw 1001, 997, 992, 988, 983, 978, 974, 969
  dw  965, 960, 955, 951, 946, 941, 937, 932
  dw  927, 923, 918, 913, 908, 904, 899, 894
  dw  889, 884, 880, 875, 870, 865, 860, 855
  dw  851, 846, 841, 836, 831, 826, 821, 816
  dw  811, 806, 802, 797, 792, 787, 782, 777
  dw  772, 767, 762, 757, 752, 747, 742, 737
  dw  732, 728, 723, 718, 713, 708, 703, 698
  dw  693, 688, 683, 678, 674, 669, 664, 659
  dw  654, 649, 644, 640, 635, 630, 625, 620
  dw  615, 611, 606, 601, 596, 592, 587, 582
  dw  577, 573, 568, 563, 559, 554, 550, 545
  dw  540, 536, 531, 527, 522, 517, 513, 508
  dw  504, 499, 495, 491, 486, 482, 477, 473
  dw  469, 464, 460, 456, 451, 447, 443, 439
  dw  434, 430, 426, 422, 418, 414, 410, 405
  dw  401, 397, 393, 389, 385, 381, 378, 374
  dw  370, 366, 362, 358, 354, 351, 347, 343
  dw  339, 336, 332, 328, 325, 321, 318, 314
  dw  311, 307, 304, 300, 297, 293, 290, 286
  dw  283, 280, 276, 273, 270, 267, 263, 260
  dw  257, 254, 251, 248, 245, 242, 239, 236
  dw  233, 230, 227, 224, 221, 218, 215, 212
  dw  210, 207, 204, 201, 199, 196, 193, 191
  dw  188, 186, 183, 180, 178, 175, 173, 171
  dw  168, 166, 163, 161, 159, 156, 154, 152
  dw  150, 147, 145, 143, 141, 139, 137, 134
  dw  132, 130, 128, 126, 124, 122, 120, 118
  dw  117, 115, 113, 111, 109, 107, 106, 104
  dw  102, 100,  99,  97,  95,  94,  92,  90
  dw   89,  87,  86,  84,  83,  81,  80,  78
  dw   77,  76,  74,  73,  71,  70,  69,  67
  dw   66,  65,  64,  62,  61,  60,  59,  58
  dw   56,  55,  54,  53,  52,  51,  50,  49
  dw   48,  47,  46,  45,  44,  43,  42,  41
  dw   40,  39,  38,  37,  36,  36,  35,  34
  dw   33,  32,  32,  31,  30,  29,  29,  28
  dw   27,  27,  26,  25,  24,  24,  23,  23
  dw   22,  21,  21,  20,  20,  19,  19,  18
  dw   17,  17,  16,  16,  15,  15,  15,  14
  dw   14,  13,  13,  12,  12,  11,  11,  11
  dw   10,  10,  10,   9,   9,   9,   8,   8
  dw    8,   7,   7,   7,   6,   6,   6,   6
  dw    5,   5,   5,   5,   4,   4,   4,   4
  dw    4,   3,   3,   3,   3,   3,   2,   2
  dw    2,   2,   2,   2,   2,   1,   1,   1
  dw    1,   1,   1,   1,   1,   1,   1,   1
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,   0,   0,   0,   0,   0,   0,   0

CubicSpline:
  dw    0,   0,   0,   0,   0,   0,   0,   0
  dw    0,  -1,  -1,  -1,  -2,  -2,  -2,  -3
  dw   -3,  -4,  -4,  -5,  -5,  -6,  -6,  -7
  dw   -8,  -8,  -9, -10, -10, -11, -12, -13
  dw  -14, -14, -15, -16, -17, -18, -19, -20
  dw  -21, -22, -23, -24, -25, -26, -27, -28
  dw  -29, -30, -31, -32, -33, -34, -35, -37
  dw  -38, -39, -40, -41, -43, -44, -45, -46
  dw  -48, -49, -50, -51, -53, -54, -55, -56
  dw  -58, -59, -60, -62, -63, -64, -66, -67
  dw  -68, -70, -71, -72, -74, -75, -76, -78
  dw  -79, -80, -82, -83, -84, -86, -87, -88
  dw  -90, -91, -92, -93, -95, -96, -97, -99
  dw -100,-101,-102,-104,-105,-106,-107,-109
  dw -110,-111,-112,-113,-114,-116,-117,-118
  dw -119,-120,-121,-122,-123,-124,-125,-126
  dw -128,-128,-129,-130,-131,-132,-133,-134
  dw -135,-136,-137,-137,-138,-139,-140,-141
  dw -141,-142,-143,-143,-144,-144,-145,-146
  dw -146,-147,-147,-148,-148,-148,-149,-149
  dw -150,-150,-150,-150,-151,-151,-151,-151
  dw -151,-151,-151,-151,-151,-151,-151,-151
  dw -151,-151,-150,-150,-150,-149,-149,-149
  dw -148,-148,-147,-147,-146,-146,-145,-144
  dw -144,-143,-142,-141,-140,-139,-138,-137
  dw -136,-135,-134,-133,-132,-130,-129,-128
  dw -126,-125,-123,-122,-120,-119,-117,-115
  dw -113,-112,-110,-108,-106,-104,-102,-100
  dw  -98, -95, -93, -91, -88, -86, -83, -81
  dw  -78, -76, -73, -70, -67, -65, -62, -59
  dw  -56, -53, -50, -46, -43, -40, -36, -33
  dw  -30, -26, -22, -19, -15, -11,  -7,  -3
  dw    0,   4,   8,  12,  16,  21,  26,  30
  dw   35,  40,  46,  51,  56,  62,  67,  73
  dw   79,  85,  91,  97, 103, 109, 116, 122
  dw  129, 136, 143, 149, 156, 164, 171, 178
  dw  186, 193, 201, 208, 216, 224, 232, 240
  dw  248, 256, 264, 273, 281, 289, 298, 307
  dw  315, 324, 333, 342, 351, 360, 369, 378
  dw  387, 397, 406, 415, 425, 435, 444, 454
  dw  464, 473, 483, 493, 503, 513, 523, 533
  dw  543, 553, 564, 574, 584, 594, 605, 615
  dw  626, 636, 647, 657, 668, 679, 689, 700
  dw  711, 721, 732, 743, 754, 765, 776, 787
  dw  798, 808, 819, 830, 841, 852, 863, 874
  dw  886, 897, 908, 919, 930, 941, 952, 963
  dw  974, 985, 996,1008,1019,1030,1041,1052
  dw 1063,1074,1085,1096,1107,1118,1129,1140
  dw 1152,1162,1173,1184,1195,1206,1217,1228
  dw 1239,1250,1261,1271,1282,1293,1303,1314
  dw 1325,1335,1346,1356,1367,1377,1388,1398
  dw 1408,1419,1429,1439,1449,1459,1470,1480
  dw 1490,1499,1509,1519,1529,1539,1548,1558
  dw 1567,1577,1586,1595,1605,1614,1623,1632
  dw 1641,1650,1659,1668,1677,1685,1694,1702
  dw 1711,1719,1727,1736,1744,1752,1760,1768
  dw 1776,1783,1791,1798,1806,1813,1820,1828
  dw 1835,1842,1849,1855,1862,1869,1875,1881
  dw 1888,1894,1900,1906,1912,1918,1923,1929
  dw 1934,1940,1945,1950,1955,1960,1964,1969
  dw 1974,1978,1982,1986,1990,1994,1998,2002
  dw 2005,2008,2012,2015,2018,2021,2023,2026
  dw 2028,2031,2033,2035,2037,2038,2040,2041
  dw 2043,2044,2045,2046,2046,2047,2047,2047

  dw 2048,2047,2047,2047,2046,2046,2045,2044
  dw 2043,2041,2040,2038,2037,2035,2033,2031
  dw 2028,2026,2023,2021,2018,2015,2012,2008
  dw 2005,2002,1998,1994,1990,1986,1982,1978
  dw 1974,1969,1964,1960,1955,1950,1945,1940
  dw 1934,1929,1923,1918,1912,1906,1900,1894
  dw 1888,1881,1875,1869,1862,1855,1849,1842
  dw 1835,1828,1820,1813,1806,1798,1791,1783
  dw 1776,1768,1760,1752,1744,1736,1727,1719
  dw 1711,1702,1694,1685,1677,1668,1659,1650
  dw 1641,1632,1623,1614,1605,1595,1586,1577
  dw 1567,1558,1548,1539,1529,1519,1509,1499
  dw 1490,1480,1470,1459,1449,1439,1429,1419
  dw 1408,1398,1388,1377,1367,1356,1346,1335
  dw 1325,1314,1303,1293,1282,1271,1261,1250
  dw 1239,1228,1217,1206,1195,1184,1173,1162
  dw 1152,1140,1129,1118,1107,1096,1085,1074
  dw 1063,1052,1041,1030,1019,1008, 996, 985
  dw  974, 963, 952, 941, 930, 919, 908, 897
  dw  886, 874, 863, 852, 841, 830, 819, 808
  dw  798, 787, 776, 765, 754, 743, 732, 721
  dw  711, 700, 689, 679, 668, 657, 647, 636
  dw  626, 615, 605, 594, 584, 574, 564, 553
  dw  543, 533, 523, 513, 503, 493, 483, 473
  dw  464, 454, 444, 435, 425, 415, 406, 397
  dw  387, 378, 369, 360, 351, 342, 333, 324
  dw  315, 307, 298, 289, 281, 273, 264, 256
  dw  248, 240, 232, 224, 216, 208, 201, 193
  dw  186, 178, 171, 164, 156, 149, 143, 136
  dw  129, 122, 116, 109, 103,  97,  91,  85
  dw   79,  73,  67,  62,  56,  51,  46,  40
  dw   35,  30,  26,  21,  16,  12,   8,   4

  dw    0,  -3,  -7, -11, -15, -19, -22, -26
  dw  -30, -33, -36, -40, -43, -46, -50, -53
  dw  -56, -59, -62, -65, -67, -70, -73, -76
  dw  -78, -81, -83, -86, -88, -91, -93, -95
  dw  -98,-100,-102,-104,-106,-108,-110,-112
  dw -113,-115,-117,-119,-120,-122,-123,-125
  dw -126,-128,-129,-130,-132,-133,-134,-135
  dw -136,-137,-138,-139,-140,-141,-142,-143
  dw -144,-144,-145,-146,-146,-147,-147,-148
  dw -148,-149,-149,-149,-150,-150,-150,-151
  dw -151,-151,-151,-151,-151,-151,-151,-151
  dw -151,-151,-151,-151,-151,-150,-150,-150
  dw -150,-149,-149,-148,-148,-148,-147,-147
  dw -146,-146,-145,-144,-144,-143,-143,-142
  dw -141,-141,-140,-139,-138,-137,-137,-136
  dw -135,-134,-133,-132,-131,-130,-129,-128
  dw -128,-126,-125,-124,-123,-122,-121,-120
  dw -119,-118,-117,-116,-114,-113,-112,-111
  dw -110,-109,-107,-106,-105,-104,-102,-101
  dw -100, -99, -97, -96, -95, -93, -92, -91
  dw  -90, -88, -87, -86, -84, -83, -82, -80
  dw  -79, -78, -76, -75, -74, -72, -71, -70
  dw  -68, -67, -66, -64, -63, -62, -60, -59
  dw  -58, -56, -55, -54, -53, -51, -50, -49
  dw  -48, -46, -45, -44, -43, -41, -40, -39
  dw  -38, -37, -35, -34, -33, -32, -31, -30
  dw  -29, -28, -27, -26, -25, -24, -23, -22
  dw  -21, -20, -19, -18, -17, -16, -15, -14
  dw  -14, -13, -12, -11, -10, -10,  -9,  -8
  dw   -8,  -7,  -6,  -6,  -5,  -5,  -4,  -4
  dw   -3,  -3,  -2,  -2,  -2,  -1,  -1,  -1
  dw    0,   0,   0,   0,   0,   0,   0,   0

%include "cpu/firtable.inc"

SECTION .bss

NEWSYM spcWptr,  resd 16     ; SPC Write pointers (point to their own functions)
NEWSYM spcRptr,  resd 16     ; SPC Read pointers (point to their own functions)

SECTION .data
NEWSYM SBToSPC,        dd 22050
NEWSYM dspPAdj,        dd 0
NEWSYM NumBRRconv,     dd 0
NEWSYM BufferSizeB,    dd 320
NEWSYM BufferSizeW,    dd 640

;TIME_CONSTANT = 256 - 1000000 / frequency

NEWSYM BufferSize,  dw 320, 320, 320, 500, 320, 400, 400
NEWSYM BufferSizes, dw 320, 320, 500, 900, 400, 750, 750
NEWSYM SoundSpeeds, db 131, 165, 211, 233, 193, 225, 235 ; 8khz,11khz,22khz,44khz
NEWSYM SoundSpeedt, db 193, 210, 233                     ; 8khz,11khz,22khz
%ifdef __MSDOS__
NEWSYM SBToSPCSpeeds, dd 8000,10989,22222,43478,15874,32258,48000
NEWSYM SBToSPCSpeeds2, dd 8192,11289,22579,45158,16384,32768,48000
%else
NEWSYM SBToSPCSpeeds, dd 8000,11025,22050,44100,16000,32000,48000
%endif
NEWSYM NumofSPCBlock, dd 626,456,228,114,314,156,104
NEWSYM SPCBlockNum, dd 0
NEWSYM NoiseSpeeds, dd 1,16,21,25,31,42,50,63,83,100,125,167,200,250,333,400,500
  dd 667,800,1000,1333,1600,2000,2667,3200,4000,5333,6400,8000,10667,16000,32000

SECTION .text

NEWSYM conv2speed
.next
    mov eax,[edi]
    mov ebx,[SBToSPC]
    mul ebx
    mov ebx,11025
    div ebx
    mov [esi],eax
    add esi,4
    add edi,4
    dec ecx
    jnz .next
    ret

NEWSYM AdjustFrequency
      xor ebx,ebx
      mov ah,[MMXSupport]
      mov al,[SoundInterpType]
      or ah,ah
      jnz .mmx
      cmp byte[LowPassFilterType],3
      jb .nothq
      mov byte[LowPassFilterType],0
.nothq
      cmp al,3
      jb .mmx
      mov al,1
      mov [SoundInterpType],al
.mmx
      or al,al
      jz near .notgaussian
      cmp al,2
      je near .cubicspline
      ja near .fir_mmx
      ; Copy from Gaussian to DSPInterP
%ifndef __MSDOS__
      ; this ifndef is needed the workaround the "snow" in the DOS port
      ; used only for Gaussian though
      test ah,ah
      jne .gaussian_mmx
%endif
      mov ebx,DSPInterP+1024
      mov edx,DSPInterP+1022
      mov esi,Gaussian
      mov ecx,512
.intrploop
      movzx eax,word[esi]
      mov [edx],ax
      mov [ebx],ax
      add ebx,2
      sub edx,2
      add esi,2
      dec ecx
      jnz .intrploop
      mov ebx, DSPInterpolate_4
      jmp .notgaussian
.gaussian_mmx
      mov ebx,Gaussian
      mov edx,Gaussian+510
      mov esi,DSPInterP
      mov ecx,256
.intrploopm
      mov ax,[ebx+512]
      mov [esi],ax
      mov ax,[ebx]
      mov [esi+2],ax
      mov ax,[edx]
      mov [esi+4],ax
      mov ax,[edx+512]
      mov [esi+6],ax
      add ebx,2
      sub edx,2
      add esi,8
      dec ecx
      jnz .intrploopm
      mov ebx, DSPInterpolate_4_mmx
      jmp .notgaussian
.cubicspline
      ; Copy from CubicSpline to DSPInterP
      test ah,ah
      jne .cubic_mmx
      mov ebx,CubicSpline
      mov edx,DSPInterP
      mov ecx,1024
.intrploopb
      movzx eax,word[ebx]
      push ebx
      mov bx,ax
      sar bx,3
      sub ax,bx
      pop ebx
      mov [edx],ax
      add ebx,2
      add edx,2
      dec ecx
      jnz .intrploopb
      mov ebx, DSPInterpolate_4
      jmp .notgaussian
.cubic_mmx
      mov ebx,CubicSpline
      mov esi,DSPInterP
      mov ecx,256
.intrploopmb
      mov ax,[ebx+256*6]
      mov dx,ax
      sar dx,3
      sub ax,dx
      mov [esi],ax
      mov ax,[ebx+256*4]
      mov dx,ax
      sar dx,3
      sub ax,dx
      mov [esi+2],ax
      mov ax,[ebx+256*2]
      mov dx,ax
      sar dx,3
      sub ax,dx
      mov [esi+4],ax
      mov ax,[ebx]
      mov dx,ax
      sar dx,3
      sub ax,dx
      mov [esi+6],ax
      add ebx,2
      add esi,8
      dec ecx
      jnz .intrploopmb
      mov ebx, DSPInterpolate_4_mmx
      jmp .notgaussian

.fir_mmx
      mov ebx, DSPInterpolate_8

.notgaussian
      mov [DSPInterpolate],ebx
%ifdef __MSDOS__
      call SB_quality_limiter
%endif
      mov eax,[SoundQuality]
      mov ebx,[NumofSPCBlock+eax*4]
      cmp byte[StereoSound],1
      jne .nostereo
      shr ebx,1
.nostereo
      mov [SPCBlockNum],ebx
.next

      mov ecx,[SoundQuality]
      mov eax,[SBToSPCSpeeds+ecx*4]
%ifdef __MSDOS__
      ; code for supporting vibra cards (coded by Peter Santing)
      cmp byte[vibracard],1
      je .vibrafix

      cmp byte[SBHDMA],0
      je .not16bit
.vibrafix
      mov eax,[SBToSPCSpeeds2+ecx*4]
.not16bit
%endif
      mov [SBToSPC],eax
      mov [SBRateb],eax

      xor edx,edx
      mov ebx,eax
      mov eax,32000
      shld edx,eax,20
      shl eax,20
      div ebx
      mov [dspPAdj],eax

      ; Init all rates
      mov esi,EchoRate
      mov edi,EchoRateO
      mov ecx,16
      call conv2speed
      mov esi,AttackRate
      mov edi,AttackRateO
      mov ecx,16
      call conv2speed
      mov esi,DecayRate
      mov edi,DecayRateO
      mov ecx,8
      call conv2speed
      mov esi,SustainRate+4
      mov edi,SustainRateO+4
      mov ecx,31
      call conv2speed
      mov esi,Increase+4
      mov edi,IncreaseO+4
      mov ecx,31
      call conv2speed
      mov esi,IncreaseBent+4
      mov edi,IncreaseBentO+4
      mov ecx,31
      call conv2speed
      mov esi,Decrease+4
      mov edi,DecreaseO+4
      mov ecx,31
      call conv2speed
      mov esi,DecreaseRateExp+4
      mov edi,DecreaseRateExpO+4
      mov ecx,31
      call conv2speed
      mov dword[Voice0Pitch],0xFFFEFFFE
      mov dword[Voice0Pitch+4],0xFFFEFFFE
      mov dword[Voice0Pitch+8],0xFFFEFFFE
      mov dword[Voice0Pitch+12],0xFFFEFFFE
      ret

NEWSYM InitSPC
      push eax
      push ebx
      push ecx
      push edx

      call AdjustFrequency

      mov ecx,32768
      mov edx,32767
.nextvol
      mov al,dh
      imul dl
      shr ax,7
      movsx bx,al
      mov [VolumeConvTable+edx*2],bx
      dec edx
      dec ecx
      jnz .nextvol

      mov eax,SPCRAM
      mov ebx,0EFh
.loop2
      mov byte[eax],0
      inc eax
      dec ebx
      jnz .loop2
      mov byte[spcX],0

      xor eax,eax
      xor ebx,ebx
      mov ebp,SPCRAM
      mov ax,0FFC0h
      add ebp,eax
      mov [spcPCRam],ebp
      mov dword[spcS],1EFh
      mov dword[spcRamDP],SPCRAM

      ; initialize all the SPC write registers
      mov dword[spcWptr+0],SPCRegF0
      mov dword[spcWptr+4],SPCRegF1
      mov dword[spcWptr+8],SPCRegF2
      mov dword[spcWptr+12],SPCRegF3
      mov dword[spcWptr+16],SPCRegF4
      mov dword[spcWptr+20],SPCRegF5
      mov dword[spcWptr+24],SPCRegF6
      mov dword[spcWptr+28],SPCRegF7
      mov dword[spcWptr+32],SPCRegF8
      mov dword[spcWptr+36],SPCRegF9
      mov dword[spcWptr+40],SPCRegFA
      mov dword[spcWptr+44],SPCRegFB
      mov dword[spcWptr+48],SPCRegFC
      mov dword[spcWptr+52],SPCRegFD
      mov dword[spcWptr+56],SPCRegFE
      mov dword[spcWptr+60],SPCRegFF
      mov dword[spcRptr+0],RSPCRegF0
      mov dword[spcRptr+4],RSPCRegF1
      mov dword[spcRptr+8],RSPCRegF2
      mov dword[spcRptr+12],RSPCRegF3
      mov dword[spcRptr+16],RSPCRegF4
      mov dword[spcRptr+20],RSPCRegF5
      mov dword[spcRptr+24],RSPCRegF6
      mov dword[spcRptr+28],RSPCRegF7
      mov dword[spcRptr+32],RSPCRegF8
      mov dword[spcRptr+36],RSPCRegF9
      mov dword[spcRptr+40],RSPCRegFA
      mov dword[spcRptr+44],RSPCRegFB
      mov dword[spcRptr+48],RSPCRegFC
      mov dword[spcRptr+52],RSPCRegFD
      mov dword[spcRptr+56],RSPCRegFE
      mov dword[spcRptr+60],RSPCRegFF
      mov dword[dspRptr+00h],RDSPReg00
      mov dword[dspRptr+04h],RDSPReg01
      mov dword[dspRptr+08h],RDSPReg02
      mov dword[dspRptr+0Ch],RDSPReg03
      mov dword[dspRptr+010h],RDSPReg04
      mov dword[dspRptr+014h],RDSPReg05
      mov dword[dspRptr+018h],RDSPReg06
      mov dword[dspRptr+01Ch],RDSPReg07
      mov dword[dspRptr+020h],RDSPReg08
      mov dword[dspRptr+024h],RDSPReg09
      mov dword[dspRptr+028h],RDSPReg0A
      mov dword[dspRptr+02Ch],RDSPReg0B
      mov dword[dspRptr+030h],RDSPReg0C
      mov dword[dspRptr+034h],RDSPReg0D
      mov dword[dspRptr+038h],RDSPReg0E
      mov dword[dspRptr+03Ch],RDSPReg0F
      mov dword[dspRptr+040h],RDSPReg10
      mov dword[dspRptr+044h],RDSPReg11
      mov dword[dspRptr+048h],RDSPReg12
      mov dword[dspRptr+04Ch],RDSPReg13
      mov dword[dspRptr+050h],RDSPReg14
      mov dword[dspRptr+054h],RDSPReg15
      mov dword[dspRptr+058h],RDSPReg16
      mov dword[dspRptr+05Ch],RDSPReg17
      mov dword[dspRptr+060h],RDSPReg18
      mov dword[dspRptr+064h],RDSPReg19
      mov dword[dspRptr+068h],RDSPReg1A
      mov dword[dspRptr+06Ch],RDSPReg1B
      mov dword[dspRptr+070h],RDSPReg1C
      mov dword[dspRptr+074h],RDSPReg1D
      mov dword[dspRptr+078h],RDSPReg1E
      mov dword[dspRptr+07Ch],RDSPReg1F
      mov dword[dspRptr+080h],RDSPReg20
      mov dword[dspRptr+084h],RDSPReg21
      mov dword[dspRptr+088h],RDSPReg22
      mov dword[dspRptr+08Ch],RDSPReg23
      mov dword[dspRptr+090h],RDSPReg24
      mov dword[dspRptr+094h],RDSPReg25
      mov dword[dspRptr+098h],RDSPReg26
      mov dword[dspRptr+09Ch],RDSPReg27
      mov dword[dspRptr+0A0h],RDSPReg28
      mov dword[dspRptr+0A4h],RDSPReg29
      mov dword[dspRptr+0A8h],RDSPReg2A
      mov dword[dspRptr+0ACh],RDSPReg2B
      mov dword[dspRptr+0B0h],RDSPReg2C
      mov dword[dspRptr+0B4h],RDSPReg2D
      mov dword[dspRptr+0B8h],RDSPReg2E
      mov dword[dspRptr+0BCh],RDSPReg2F
      mov dword[dspRptr+0C0h],RDSPReg30
      mov dword[dspRptr+0C4h],RDSPReg31
      mov dword[dspRptr+0C8h],RDSPReg32
      mov dword[dspRptr+0CCh],RDSPReg33
      mov dword[dspRptr+0D0h],RDSPReg34
      mov dword[dspRptr+0D4h],RDSPReg35
      mov dword[dspRptr+0D8h],RDSPReg36
      mov dword[dspRptr+0DCh],RDSPReg37
      mov dword[dspRptr+0E0h],RDSPReg38
      mov dword[dspRptr+0E4h],RDSPReg39
      mov dword[dspRptr+0E8h],RDSPReg3A
      mov dword[dspRptr+0ECh],RDSPReg3B
      mov dword[dspRptr+0F0h],RDSPReg3C
      mov dword[dspRptr+0F4h],RDSPReg3D
      mov dword[dspRptr+0F8h],RDSPReg3E
      mov dword[dspRptr+0FCh],RDSPReg3F
      mov dword[dspRptr+0100h],RDSPReg40
      mov dword[dspRptr+0104h],RDSPReg41
      mov dword[dspRptr+0108h],RDSPReg42
      mov dword[dspRptr+010Ch],RDSPReg43
      mov dword[dspRptr+0110h],RDSPReg44
      mov dword[dspRptr+0114h],RDSPReg45
      mov dword[dspRptr+0118h],RDSPReg46
      mov dword[dspRptr+011Ch],RDSPReg47
      mov dword[dspRptr+0120h],RDSPReg48
      mov dword[dspRptr+0124h],RDSPReg49
      mov dword[dspRptr+0128h],RDSPReg4A
      mov dword[dspRptr+012Ch],RDSPReg4B
      mov dword[dspRptr+0130h],RDSPReg4C
      mov dword[dspRptr+0134h],RDSPReg4D
      mov dword[dspRptr+0138h],RDSPReg4E
      mov dword[dspRptr+013Ch],RDSPReg4F
      mov dword[dspRptr+0140h],RDSPReg50
      mov dword[dspRptr+0144h],RDSPReg51
      mov dword[dspRptr+0148h],RDSPReg52
      mov dword[dspRptr+014Ch],RDSPReg53
      mov dword[dspRptr+0150h],RDSPReg54
      mov dword[dspRptr+0154h],RDSPReg55
      mov dword[dspRptr+0158h],RDSPReg56
      mov dword[dspRptr+015Ch],RDSPReg57
      mov dword[dspRptr+0160h],RDSPReg58
      mov dword[dspRptr+0164h],RDSPReg59
      mov dword[dspRptr+0168h],RDSPReg5A
      mov dword[dspRptr+016Ch],RDSPReg5B
      mov dword[dspRptr+0170h],RDSPReg5C
      mov dword[dspRptr+0174h],RDSPReg5D
      mov dword[dspRptr+0178h],RDSPReg5E
      mov dword[dspRptr+017Ch],RDSPReg5F
      mov dword[dspRptr+0180h],RDSPReg60
      mov dword[dspRptr+0184h],RDSPReg61
      mov dword[dspRptr+0188h],RDSPReg62
      mov dword[dspRptr+018Ch],RDSPReg63
      mov dword[dspRptr+0190h],RDSPReg64
      mov dword[dspRptr+0194h],RDSPReg65
      mov dword[dspRptr+0198h],RDSPReg66
      mov dword[dspRptr+019Ch],RDSPReg67
      mov dword[dspRptr+01A0h],RDSPReg68
      mov dword[dspRptr+01A4h],RDSPReg69
      mov dword[dspRptr+01A8h],RDSPReg6A
      mov dword[dspRptr+01ACh],RDSPReg6B
      mov dword[dspRptr+01B0h],RDSPReg6C
      mov dword[dspRptr+01B4h],RDSPReg6D
      mov dword[dspRptr+01B8h],RDSPReg6E
      mov dword[dspRptr+01BCh],RDSPReg6F
      mov dword[dspRptr+01C0h],RDSPReg70
      mov dword[dspRptr+01C4h],RDSPReg71
      mov dword[dspRptr+01C8h],RDSPReg72
      mov dword[dspRptr+01CCh],RDSPReg73
      mov dword[dspRptr+01D0h],RDSPReg74
      mov dword[dspRptr+01D4h],RDSPReg75
      mov dword[dspRptr+01D8h],RDSPReg76
      mov dword[dspRptr+01DCh],RDSPReg77
      mov dword[dspRptr+01E0h],RDSPReg78
      mov dword[dspRptr+01E4h],RDSPReg79
      mov dword[dspRptr+01E8h],RDSPReg7A
      mov dword[dspRptr+01ECh],RDSPReg7B
      mov dword[dspRptr+01F0h],RDSPReg7C
      mov dword[dspRptr+01F4h],RDSPReg7D
      mov dword[dspRptr+01F8h],RDSPReg7E
      mov dword[dspRptr+01FCh],RDSPReg7F
      mov dword[dspRptr+0200h],RDSPReg80
      mov dword[dspRptr+0204h],RDSPReg81
      mov dword[dspRptr+0208h],RDSPReg82
      mov dword[dspRptr+020Ch],RDSPReg83
      mov dword[dspRptr+0210h],RDSPReg84
      mov dword[dspRptr+0214h],RDSPReg85
      mov dword[dspRptr+0218h],RDSPReg86
      mov dword[dspRptr+021Ch],RDSPReg87
      mov dword[dspRptr+0220h],RDSPReg88
      mov dword[dspRptr+0224h],RDSPReg89
      mov dword[dspRptr+0228h],RDSPReg8A
      mov dword[dspRptr+022Ch],RDSPReg8B
      mov dword[dspRptr+0230h],RDSPReg8C
      mov dword[dspRptr+0234h],RDSPReg8D
      mov dword[dspRptr+0238h],RDSPReg8E
      mov dword[dspRptr+023Ch],RDSPReg8F
      mov dword[dspRptr+0240h],RDSPReg90
      mov dword[dspRptr+0244h],RDSPReg91
      mov dword[dspRptr+0248h],RDSPReg92
      mov dword[dspRptr+024Ch],RDSPReg93
      mov dword[dspRptr+0250h],RDSPReg94
      mov dword[dspRptr+0254h],RDSPReg95
      mov dword[dspRptr+0258h],RDSPReg96
      mov dword[dspRptr+025Ch],RDSPReg97
      mov dword[dspRptr+0260h],RDSPReg98
      mov dword[dspRptr+0264h],RDSPReg99
      mov dword[dspRptr+0268h],RDSPReg9A
      mov dword[dspRptr+026Ch],RDSPReg9B
      mov dword[dspRptr+0270h],RDSPReg9C
      mov dword[dspRptr+0274h],RDSPReg9D
      mov dword[dspRptr+0278h],RDSPReg9E
      mov dword[dspRptr+027Ch],RDSPReg9F
      mov dword[dspRptr+0280h],RDSPRegA0
      mov dword[dspRptr+0284h],RDSPRegA1
      mov dword[dspRptr+0288h],RDSPRegA2
      mov dword[dspRptr+028Ch],RDSPRegA3
      mov dword[dspRptr+0290h],RDSPRegA4
      mov dword[dspRptr+0294h],RDSPRegA5
      mov dword[dspRptr+0298h],RDSPRegA6
      mov dword[dspRptr+029Ch],RDSPRegA7
      mov dword[dspRptr+02A0h],RDSPRegA8
      mov dword[dspRptr+02A4h],RDSPRegA9
      mov dword[dspRptr+02A8h],RDSPRegAA
      mov dword[dspRptr+02ACh],RDSPRegAB
      mov dword[dspRptr+02B0h],RDSPRegAC
      mov dword[dspRptr+02B4h],RDSPRegAD
      mov dword[dspRptr+02B8h],RDSPRegAE
      mov dword[dspRptr+02BCh],RDSPRegAF
      mov dword[dspRptr+02C0h],RDSPRegB0
      mov dword[dspRptr+02C4h],RDSPRegB1
      mov dword[dspRptr+02C8h],RDSPRegB2
      mov dword[dspRptr+02CCh],RDSPRegB3
      mov dword[dspRptr+02D0h],RDSPRegB4
      mov dword[dspRptr+02D4h],RDSPRegB5
      mov dword[dspRptr+02D8h],RDSPRegB6
      mov dword[dspRptr+02DCh],RDSPRegB7
      mov dword[dspRptr+02E0h],RDSPRegB8
      mov dword[dspRptr+02E4h],RDSPRegB9
      mov dword[dspRptr+02E8h],RDSPRegBA
      mov dword[dspRptr+02ECh],RDSPRegBB
      mov dword[dspRptr+02F0h],RDSPRegBC
      mov dword[dspRptr+02F4h],RDSPRegBD
      mov dword[dspRptr+02F8h],RDSPRegBE
      mov dword[dspRptr+02FCh],RDSPRegBF
      mov dword[dspRptr+0300h],RDSPRegC0
      mov dword[dspRptr+0304h],RDSPRegC1
      mov dword[dspRptr+0308h],RDSPRegC2
      mov dword[dspRptr+030Ch],RDSPRegC3
      mov dword[dspRptr+0310h],RDSPRegC4
      mov dword[dspRptr+0314h],RDSPRegC5
      mov dword[dspRptr+0318h],RDSPRegC6
      mov dword[dspRptr+031Ch],RDSPRegC7
      mov dword[dspRptr+0320h],RDSPRegC8
      mov dword[dspRptr+0324h],RDSPRegC9
      mov dword[dspRptr+0328h],RDSPRegCA
      mov dword[dspRptr+032Ch],RDSPRegCB
      mov dword[dspRptr+0330h],RDSPRegCC
      mov dword[dspRptr+0334h],RDSPRegCD
      mov dword[dspRptr+0338h],RDSPRegCE
      mov dword[dspRptr+033Ch],RDSPRegCF
      mov dword[dspRptr+0340h],RDSPRegD0
      mov dword[dspRptr+0344h],RDSPRegD1
      mov dword[dspRptr+0348h],RDSPRegD2
      mov dword[dspRptr+034Ch],RDSPRegD3
      mov dword[dspRptr+0350h],RDSPRegD4
      mov dword[dspRptr+0354h],RDSPRegD5
      mov dword[dspRptr+0358h],RDSPRegD6
      mov dword[dspRptr+035Ch],RDSPRegD7
      mov dword[dspRptr+0360h],RDSPRegD8
      mov dword[dspRptr+0364h],RDSPRegD9
      mov dword[dspRptr+0368h],RDSPRegDA
      mov dword[dspRptr+036Ch],RDSPRegDB
      mov dword[dspRptr+0370h],RDSPRegDC
      mov dword[dspRptr+0374h],RDSPRegDD
      mov dword[dspRptr+0378h],RDSPRegDE
      mov dword[dspRptr+037Ch],RDSPRegDF
      mov dword[dspRptr+0380h],RDSPRegE0
      mov dword[dspRptr+0384h],RDSPRegE1
      mov dword[dspRptr+0388h],RDSPRegE2
      mov dword[dspRptr+038Ch],RDSPRegE3
      mov dword[dspRptr+0390h],RDSPRegE4
      mov dword[dspRptr+0394h],RDSPRegE5
      mov dword[dspRptr+0398h],RDSPRegE6
      mov dword[dspRptr+039Ch],RDSPRegE7
      mov dword[dspRptr+03A0h],RDSPRegE8
      mov dword[dspRptr+03A4h],RDSPRegE9
      mov dword[dspRptr+03A8h],RDSPRegEA
      mov dword[dspRptr+03ACh],RDSPRegEB
      mov dword[dspRptr+03B0h],RDSPRegEC
      mov dword[dspRptr+03B4h],RDSPRegED
      mov dword[dspRptr+03B8h],RDSPRegEE
      mov dword[dspRptr+03BCh],RDSPRegEF
      mov dword[dspRptr+03C0h],RDSPRegF0
      mov dword[dspRptr+03C4h],RDSPRegF1
      mov dword[dspRptr+03C8h],RDSPRegF2
      mov dword[dspRptr+03CCh],RDSPRegF3
      mov dword[dspRptr+03D0h],RDSPRegF4
      mov dword[dspRptr+03D4h],RDSPRegF5
      mov dword[dspRptr+03D8h],RDSPRegF6
      mov dword[dspRptr+03DCh],RDSPRegF7
      mov dword[dspRptr+03E0h],RDSPRegF8
      mov dword[dspRptr+03E4h],RDSPRegF9
      mov dword[dspRptr+03E8h],RDSPRegFA
      mov dword[dspRptr+03ECh],RDSPRegFB
      mov dword[dspRptr+03F0h],RDSPRegFC
      mov dword[dspRptr+03F4h],RDSPRegFD
      mov dword[dspRptr+03F8h],RDSPRegFE
      mov dword[dspRptr+03FCh],RDSPRegFF

      mov dword[dspWptr+00h],WDSPReg00
      mov dword[dspWptr+04h],WDSPReg01
      mov dword[dspWptr+08h],WDSPReg02
      mov dword[dspWptr+0Ch],WDSPReg03
      mov dword[dspWptr+010h],WDSPReg04
      mov dword[dspWptr+014h],WDSPReg05
      mov dword[dspWptr+018h],WDSPReg06
      mov dword[dspWptr+01Ch],WDSPReg07
      mov dword[dspWptr+020h],WDSPReg08
      mov dword[dspWptr+024h],WDSPReg09
      mov dword[dspWptr+028h],WDSPReg0A
      mov dword[dspWptr+02Ch],WDSPReg0B
      mov dword[dspWptr+030h],WDSPReg0C
      mov dword[dspWptr+034h],WDSPReg0D
      mov dword[dspWptr+038h],WDSPReg0E
      mov dword[dspWptr+03Ch],WDSPReg0F
      mov dword[dspWptr+040h],WDSPReg10
      mov dword[dspWptr+044h],WDSPReg11
      mov dword[dspWptr+048h],WDSPReg12
      mov dword[dspWptr+04Ch],WDSPReg13
      mov dword[dspWptr+050h],WDSPReg14
      mov dword[dspWptr+054h],WDSPReg15
      mov dword[dspWptr+058h],WDSPReg16
      mov dword[dspWptr+05Ch],WDSPReg17
      mov dword[dspWptr+060h],WDSPReg18
      mov dword[dspWptr+064h],WDSPReg19
      mov dword[dspWptr+068h],WDSPReg1A
      mov dword[dspWptr+06Ch],WDSPReg1B
      mov dword[dspWptr+070h],WDSPReg1C
      mov dword[dspWptr+074h],WDSPReg1D
      mov dword[dspWptr+078h],WDSPReg1E
      mov dword[dspWptr+07Ch],WDSPReg1F
      mov dword[dspWptr+080h],WDSPReg20
      mov dword[dspWptr+084h],WDSPReg21
      mov dword[dspWptr+088h],WDSPReg22
      mov dword[dspWptr+08Ch],WDSPReg23
      mov dword[dspWptr+090h],WDSPReg24
      mov dword[dspWptr+094h],WDSPReg25
      mov dword[dspWptr+098h],WDSPReg26
      mov dword[dspWptr+09Ch],WDSPReg27
      mov dword[dspWptr+0A0h],WDSPReg28
      mov dword[dspWptr+0A4h],WDSPReg29
      mov dword[dspWptr+0A8h],WDSPReg2A
      mov dword[dspWptr+0ACh],WDSPReg2B
      mov dword[dspWptr+0B0h],WDSPReg2C
      mov dword[dspWptr+0B4h],WDSPReg2D
      mov dword[dspWptr+0B8h],WDSPReg2E
      mov dword[dspWptr+0BCh],WDSPReg2F
      mov dword[dspWptr+0C0h],WDSPReg30
      mov dword[dspWptr+0C4h],WDSPReg31
      mov dword[dspWptr+0C8h],WDSPReg32
      mov dword[dspWptr+0CCh],WDSPReg33
      mov dword[dspWptr+0D0h],WDSPReg34
      mov dword[dspWptr+0D4h],WDSPReg35
      mov dword[dspWptr+0D8h],WDSPReg36
      mov dword[dspWptr+0DCh],WDSPReg37
      mov dword[dspWptr+0E0h],WDSPReg38
      mov dword[dspWptr+0E4h],WDSPReg39
      mov dword[dspWptr+0E8h],WDSPReg3A
      mov dword[dspWptr+0ECh],WDSPReg3B
      mov dword[dspWptr+0F0h],WDSPReg3C
      mov dword[dspWptr+0F4h],WDSPReg3D
      mov dword[dspWptr+0F8h],WDSPReg3E
      mov dword[dspWptr+0FCh],WDSPReg3F
      mov dword[dspWptr+0100h],WDSPReg40
      mov dword[dspWptr+0104h],WDSPReg41
      mov dword[dspWptr+0108h],WDSPReg42
      mov dword[dspWptr+010Ch],WDSPReg43
      mov dword[dspWptr+0110h],WDSPReg44
      mov dword[dspWptr+0114h],WDSPReg45
      mov dword[dspWptr+0118h],WDSPReg46
      mov dword[dspWptr+011Ch],WDSPReg47
      mov dword[dspWptr+0120h],WDSPReg48
      mov dword[dspWptr+0124h],WDSPReg49
      mov dword[dspWptr+0128h],WDSPReg4A
      mov dword[dspWptr+012Ch],WDSPReg4B
      mov dword[dspWptr+0130h],WDSPReg4C
      mov dword[dspWptr+0134h],WDSPReg4D
      mov dword[dspWptr+0138h],WDSPReg4E
      mov dword[dspWptr+013Ch],WDSPReg4F
      mov dword[dspWptr+0140h],WDSPReg50
      mov dword[dspWptr+0144h],WDSPReg51
      mov dword[dspWptr+0148h],WDSPReg52
      mov dword[dspWptr+014Ch],WDSPReg53
      mov dword[dspWptr+0150h],WDSPReg54
      mov dword[dspWptr+0154h],WDSPReg55
      mov dword[dspWptr+0158h],WDSPReg56
      mov dword[dspWptr+015Ch],WDSPReg57
      mov dword[dspWptr+0160h],WDSPReg58
      mov dword[dspWptr+0164h],WDSPReg59
      mov dword[dspWptr+0168h],WDSPReg5A
      mov dword[dspWptr+016Ch],WDSPReg5B
      mov dword[dspWptr+0170h],WDSPReg5C
      mov dword[dspWptr+0174h],WDSPReg5D
      mov dword[dspWptr+0178h],WDSPReg5E
      mov dword[dspWptr+017Ch],WDSPReg5F
      mov dword[dspWptr+0180h],WDSPReg60
      mov dword[dspWptr+0184h],WDSPReg61
      mov dword[dspWptr+0188h],WDSPReg62
      mov dword[dspWptr+018Ch],WDSPReg63
      mov dword[dspWptr+0190h],WDSPReg64
      mov dword[dspWptr+0194h],WDSPReg65
      mov dword[dspWptr+0198h],WDSPReg66
      mov dword[dspWptr+019Ch],WDSPReg67
      mov dword[dspWptr+01A0h],WDSPReg68
      mov dword[dspWptr+01A4h],WDSPReg69
      mov dword[dspWptr+01A8h],WDSPReg6A
      mov dword[dspWptr+01ACh],WDSPReg6B
      mov dword[dspWptr+01B0h],WDSPReg6C
      mov dword[dspWptr+01B4h],WDSPReg6D
      mov dword[dspWptr+01B8h],WDSPReg6E
      mov dword[dspWptr+01BCh],WDSPReg6F
      mov dword[dspWptr+01C0h],WDSPReg70
      mov dword[dspWptr+01C4h],WDSPReg71
      mov dword[dspWptr+01C8h],WDSPReg72
      mov dword[dspWptr+01CCh],WDSPReg73
      mov dword[dspWptr+01D0h],WDSPReg74
      mov dword[dspWptr+01D4h],WDSPReg75
      mov dword[dspWptr+01D8h],WDSPReg76
      mov dword[dspWptr+01DCh],WDSPReg77
      mov dword[dspWptr+01E0h],WDSPReg78
      mov dword[dspWptr+01E4h],WDSPReg79
      mov dword[dspWptr+01E8h],WDSPReg7A
      mov dword[dspWptr+01ECh],WDSPReg7B
      mov dword[dspWptr+01F0h],WDSPReg7C
      mov dword[dspWptr+01F4h],WDSPReg7D
      mov dword[dspWptr+01F8h],WDSPReg7E
      mov dword[dspWptr+01FCh],WDSPReg7F
      mov dword[dspWptr+0200h],WDSPReg80
      mov dword[dspWptr+0204h],WDSPReg81
      mov dword[dspWptr+0208h],WDSPReg82
      mov dword[dspWptr+020Ch],WDSPReg83
      mov dword[dspWptr+0210h],WDSPReg84
      mov dword[dspWptr+0214h],WDSPReg85
      mov dword[dspWptr+0218h],WDSPReg86
      mov dword[dspWptr+021Ch],WDSPReg87
      mov dword[dspWptr+0220h],WDSPReg88
      mov dword[dspWptr+0224h],WDSPReg89
      mov dword[dspWptr+0228h],WDSPReg8A
      mov dword[dspWptr+022Ch],WDSPReg8B
      mov dword[dspWptr+0230h],WDSPReg8C
      mov dword[dspWptr+0234h],WDSPReg8D
      mov dword[dspWptr+0238h],WDSPReg8E
      mov dword[dspWptr+023Ch],WDSPReg8F
      mov dword[dspWptr+0240h],WDSPReg90
      mov dword[dspWptr+0244h],WDSPReg91
      mov dword[dspWptr+0248h],WDSPReg92
      mov dword[dspWptr+024Ch],WDSPReg93
      mov dword[dspWptr+0250h],WDSPReg94
      mov dword[dspWptr+0254h],WDSPReg95
      mov dword[dspWptr+0258h],WDSPReg96
      mov dword[dspWptr+025Ch],WDSPReg97
      mov dword[dspWptr+0260h],WDSPReg98
      mov dword[dspWptr+0264h],WDSPReg99
      mov dword[dspWptr+0268h],WDSPReg9A
      mov dword[dspWptr+026Ch],WDSPReg9B
      mov dword[dspWptr+0270h],WDSPReg9C
      mov dword[dspWptr+0274h],WDSPReg9D
      mov dword[dspWptr+0278h],WDSPReg9E
      mov dword[dspWptr+027Ch],WDSPReg9F
      mov dword[dspWptr+0280h],WDSPRegA0
      mov dword[dspWptr+0284h],WDSPRegA1
      mov dword[dspWptr+0288h],WDSPRegA2
      mov dword[dspWptr+028Ch],WDSPRegA3
      mov dword[dspWptr+0290h],WDSPRegA4
      mov dword[dspWptr+0294h],WDSPRegA5
      mov dword[dspWptr+0298h],WDSPRegA6
      mov dword[dspWptr+029Ch],WDSPRegA7
      mov dword[dspWptr+02A0h],WDSPRegA8
      mov dword[dspWptr+02A4h],WDSPRegA9
      mov dword[dspWptr+02A8h],WDSPRegAA
      mov dword[dspWptr+02ACh],WDSPRegAB
      mov dword[dspWptr+02B0h],WDSPRegAC
      mov dword[dspWptr+02B4h],WDSPRegAD
      mov dword[dspWptr+02B8h],WDSPRegAE
      mov dword[dspWptr+02BCh],WDSPRegAF
      mov dword[dspWptr+02C0h],WDSPRegB0
      mov dword[dspWptr+02C4h],WDSPRegB1
      mov dword[dspWptr+02C8h],WDSPRegB2
      mov dword[dspWptr+02CCh],WDSPRegB3
      mov dword[dspWptr+02D0h],WDSPRegB4
      mov dword[dspWptr+02D4h],WDSPRegB5
      mov dword[dspWptr+02D8h],WDSPRegB6
      mov dword[dspWptr+02DCh],WDSPRegB7
      mov dword[dspWptr+02E0h],WDSPRegB8
      mov dword[dspWptr+02E4h],WDSPRegB9
      mov dword[dspWptr+02E8h],WDSPRegBA
      mov dword[dspWptr+02ECh],WDSPRegBB
      mov dword[dspWptr+02F0h],WDSPRegBC
      mov dword[dspWptr+02F4h],WDSPRegBD
      mov dword[dspWptr+02F8h],WDSPRegBE
      mov dword[dspWptr+02FCh],WDSPRegBF
      mov dword[dspWptr+0300h],WDSPRegC0
      mov dword[dspWptr+0304h],WDSPRegC1
      mov dword[dspWptr+0308h],WDSPRegC2
      mov dword[dspWptr+030Ch],WDSPRegC3
      mov dword[dspWptr+0310h],WDSPRegC4
      mov dword[dspWptr+0314h],WDSPRegC5
      mov dword[dspWptr+0318h],WDSPRegC6
      mov dword[dspWptr+031Ch],WDSPRegC7
      mov dword[dspWptr+0320h],WDSPRegC8
      mov dword[dspWptr+0324h],WDSPRegC9
      mov dword[dspWptr+0328h],WDSPRegCA
      mov dword[dspWptr+032Ch],WDSPRegCB
      mov dword[dspWptr+0330h],WDSPRegCC
      mov dword[dspWptr+0334h],WDSPRegCD
      mov dword[dspWptr+0338h],WDSPRegCE
      mov dword[dspWptr+033Ch],WDSPRegCF
      mov dword[dspWptr+0340h],WDSPRegD0
      mov dword[dspWptr+0344h],WDSPRegD1
      mov dword[dspWptr+0348h],WDSPRegD2
      mov dword[dspWptr+034Ch],WDSPRegD3
      mov dword[dspWptr+0350h],WDSPRegD4
      mov dword[dspWptr+0354h],WDSPRegD5
      mov dword[dspWptr+0358h],WDSPRegD6
      mov dword[dspWptr+035Ch],WDSPRegD7
      mov dword[dspWptr+0360h],WDSPRegD8
      mov dword[dspWptr+0364h],WDSPRegD9
      mov dword[dspWptr+0368h],WDSPRegDA
      mov dword[dspWptr+036Ch],WDSPRegDB
      mov dword[dspWptr+0370h],WDSPRegDC
      mov dword[dspWptr+0374h],WDSPRegDD
      mov dword[dspWptr+0378h],WDSPRegDE
      mov dword[dspWptr+037Ch],WDSPRegDF
      mov dword[dspWptr+0380h],WDSPRegE0
      mov dword[dspWptr+0384h],WDSPRegE1
      mov dword[dspWptr+0388h],WDSPRegE2
      mov dword[dspWptr+038Ch],WDSPRegE3
      mov dword[dspWptr+0390h],WDSPRegE4
      mov dword[dspWptr+0394h],WDSPRegE5
      mov dword[dspWptr+0398h],WDSPRegE6
      mov dword[dspWptr+039Ch],WDSPRegE7
      mov dword[dspWptr+03A0h],WDSPRegE8
      mov dword[dspWptr+03A4h],WDSPRegE9
      mov dword[dspWptr+03A8h],WDSPRegEA
      mov dword[dspWptr+03ACh],WDSPRegEB
      mov dword[dspWptr+03B0h],WDSPRegEC
      mov dword[dspWptr+03B4h],WDSPRegED
      mov dword[dspWptr+03B8h],WDSPRegEE
      mov dword[dspWptr+03BCh],WDSPRegEF
      mov dword[dspWptr+03C0h],WDSPRegF0
      mov dword[dspWptr+03C4h],WDSPRegF1
      mov dword[dspWptr+03C8h],WDSPRegF2
      mov dword[dspWptr+03CCh],WDSPRegF3
      mov dword[dspWptr+03D0h],WDSPRegF4
      mov dword[dspWptr+03D4h],WDSPRegF5
      mov dword[dspWptr+03D8h],WDSPRegF6
      mov dword[dspWptr+03DCh],WDSPRegF7
      mov dword[dspWptr+03E0h],WDSPRegF8
      mov dword[dspWptr+03E4h],WDSPRegF9
      mov dword[dspWptr+03E8h],WDSPRegFA
      mov dword[dspWptr+03ECh],WDSPRegFB
      mov dword[dspWptr+03F0h],WDSPRegFC
      mov dword[dspWptr+03F4h],WDSPRegFD
      mov dword[dspWptr+03F8h],WDSPRegFE
      mov dword[dspWptr+03FCh],WDSPRegFF

   ; first fill all pointer to an invalid access function
      mov ecx,256
      mov eax,Invalidopcode
      mov ebp,0
.loop
%ifdef __MSDOS__
      mov [ds:opcjmptab+ebp],eax
%else
      mov [opcjmptab+ebp],eax
%endif
      add ebp,4
      dec ecx
      jnz .loop
   ; now fill the table
      mov dword[opcjmptab+00h],Op00
      mov dword[opcjmptab+04h],Op01
      mov dword[opcjmptab+08h],Op02
      mov dword[opcjmptab+0Ch],Op03
      mov dword[opcjmptab+010h],Op04
      mov dword[opcjmptab+014h],Op05
      mov dword[opcjmptab+018h],Op06
      mov dword[opcjmptab+01Ch],Op07
      mov dword[opcjmptab+020h],Op08
      mov dword[opcjmptab+024h],Op09
      mov dword[opcjmptab+028h],Op0A
      mov dword[opcjmptab+02Ch],Op0B
      mov dword[opcjmptab+030h],Op0C
      mov dword[opcjmptab+034h],Op0D
      mov dword[opcjmptab+038h],Op0E
      mov dword[opcjmptab+03Ch],Op0F
      mov dword[opcjmptab+040h],Op10
      mov dword[opcjmptab+044h],Op11
      mov dword[opcjmptab+048h],Op12
      mov dword[opcjmptab+04Ch],Op13
      mov dword[opcjmptab+050h],Op14
      mov dword[opcjmptab+054h],Op15
      mov dword[opcjmptab+058h],Op16
      mov dword[opcjmptab+05Ch],Op17
      mov dword[opcjmptab+060h],Op18
      mov dword[opcjmptab+064h],Op19
      mov dword[opcjmptab+068h],Op1A
      mov dword[opcjmptab+06Ch],Op1B
      mov dword[opcjmptab+070h],Op1C
      mov dword[opcjmptab+074h],Op1D
      mov dword[opcjmptab+078h],Op1E
      mov dword[opcjmptab+07Ch],Op1F
      mov dword[opcjmptab+080h],Op20
      mov dword[opcjmptab+084h],Op21
      mov dword[opcjmptab+088h],Op22
      mov dword[opcjmptab+08Ch],Op23
      mov dword[opcjmptab+090h],Op24
      mov dword[opcjmptab+094h],Op25
      mov dword[opcjmptab+098h],Op26
      mov dword[opcjmptab+09Ch],Op27
      mov dword[opcjmptab+0A0h],Op28
      mov dword[opcjmptab+0A4h],Op29
      mov dword[opcjmptab+0A8h],Op2A
      mov dword[opcjmptab+0ACh],Op2B
      mov dword[opcjmptab+0B0h],Op2C
      mov dword[opcjmptab+0B4h],Op2D
      mov dword[opcjmptab+0B8h],Op2E
      mov dword[opcjmptab+0BCh],Op2F
      mov dword[opcjmptab+0C0h],Op30
      mov dword[opcjmptab+0C4h],Op31
      mov dword[opcjmptab+0C8h],Op32
      mov dword[opcjmptab+0CCh],Op33
      mov dword[opcjmptab+0D0h],Op34
      mov dword[opcjmptab+0D4h],Op35
      mov dword[opcjmptab+0D8h],Op36
      mov dword[opcjmptab+0DCh],Op37
      mov dword[opcjmptab+0E0h],Op38
      mov dword[opcjmptab+0E4h],Op39
      mov dword[opcjmptab+0E8h],Op3A
      mov dword[opcjmptab+0ECh],Op3B
      mov dword[opcjmptab+0F0h],Op3C
      mov dword[opcjmptab+0F4h],Op3D
      mov dword[opcjmptab+0F8h],Op3E
      mov dword[opcjmptab+0FCh],Op3F
      mov dword[opcjmptab+0100h],Op40
      mov dword[opcjmptab+0104h],Op41
      mov dword[opcjmptab+0108h],Op42
      mov dword[opcjmptab+010Ch],Op43
      mov dword[opcjmptab+0110h],Op44
      mov dword[opcjmptab+0114h],Op45
      mov dword[opcjmptab+0118h],Op46
      mov dword[opcjmptab+011Ch],Op47
      mov dword[opcjmptab+0120h],Op48
      mov dword[opcjmptab+0124h],Op49
      mov dword[opcjmptab+0128h],Op4A
      mov dword[opcjmptab+012Ch],Op4B
      mov dword[opcjmptab+0130h],Op4C
      mov dword[opcjmptab+0134h],Op4D
      mov dword[opcjmptab+0138h],Op4E
      mov dword[opcjmptab+013Ch],Op4F
      mov dword[opcjmptab+0140h],Op50
      mov dword[opcjmptab+0144h],Op51
      mov dword[opcjmptab+0148h],Op52
      mov dword[opcjmptab+014Ch],Op53
      mov dword[opcjmptab+0150h],Op54
      mov dword[opcjmptab+0154h],Op55
      mov dword[opcjmptab+0158h],Op56
      mov dword[opcjmptab+015Ch],Op57
      mov dword[opcjmptab+0160h],Op58
      mov dword[opcjmptab+0164h],Op59
      mov dword[opcjmptab+0168h],Op5A
      mov dword[opcjmptab+016Ch],Op5B
      mov dword[opcjmptab+0170h],Op5C
      mov dword[opcjmptab+0174h],Op5D
      mov dword[opcjmptab+0178h],Op5E
      mov dword[opcjmptab+017Ch],Op5F
      mov dword[opcjmptab+0180h],Op60
      mov dword[opcjmptab+0184h],Op61
      mov dword[opcjmptab+0188h],Op62
      mov dword[opcjmptab+018Ch],Op63
      mov dword[opcjmptab+0190h],Op64
      mov dword[opcjmptab+0194h],Op65
      mov dword[opcjmptab+0198h],Op66
      mov dword[opcjmptab+019Ch],Op67
      mov dword[opcjmptab+01A0h],Op68
      mov dword[opcjmptab+01A4h],Op69
      mov dword[opcjmptab+01A8h],Op6A
      mov dword[opcjmptab+01ACh],Op6B
      mov dword[opcjmptab+01B0h],Op6C
      mov dword[opcjmptab+01B4h],Op6D
      mov dword[opcjmptab+01B8h],Op6E
      mov dword[opcjmptab+01BCh],Op6F
      mov dword[opcjmptab+01C0h],Op70
      mov dword[opcjmptab+01C4h],Op71
      mov dword[opcjmptab+01C8h],Op72
      mov dword[opcjmptab+01CCh],Op73
      mov dword[opcjmptab+01D0h],Op74
      mov dword[opcjmptab+01D4h],Op75
      mov dword[opcjmptab+01D8h],Op76
      mov dword[opcjmptab+01DCh],Op77
      mov dword[opcjmptab+01E0h],Op78
      mov dword[opcjmptab+01E4h],Op79
      mov dword[opcjmptab+01E8h],Op7A
      mov dword[opcjmptab+01ECh],Op7B
      mov dword[opcjmptab+01F0h],Op7C
      mov dword[opcjmptab+01F4h],Op7D
      mov dword[opcjmptab+01F8h],Op7E
      mov dword[opcjmptab+01FCh],Op7F
      mov dword[opcjmptab+0200h],Op80
      mov dword[opcjmptab+0204h],Op81
      mov dword[opcjmptab+0208h],Op82
      mov dword[opcjmptab+020Ch],Op83
      mov dword[opcjmptab+0210h],Op84
      mov dword[opcjmptab+0214h],Op85
      mov dword[opcjmptab+0218h],Op86
      mov dword[opcjmptab+021Ch],Op87
      mov dword[opcjmptab+0220h],Op88
      mov dword[opcjmptab+0224h],Op89
      mov dword[opcjmptab+0228h],Op8A
      mov dword[opcjmptab+022Ch],Op8B
      mov dword[opcjmptab+0230h],Op8C
      mov dword[opcjmptab+0234h],Op8D
      mov dword[opcjmptab+0238h],Op8E
      mov dword[opcjmptab+023Ch],Op8F
      mov dword[opcjmptab+0240h],Op90
      mov dword[opcjmptab+0244h],Op91
      mov dword[opcjmptab+0248h],Op92
      mov dword[opcjmptab+024Ch],Op93
      mov dword[opcjmptab+0250h],Op94
      mov dword[opcjmptab+0254h],Op95
      mov dword[opcjmptab+0258h],Op96
      mov dword[opcjmptab+025Ch],Op97
      mov dword[opcjmptab+0260h],Op98
      mov dword[opcjmptab+0264h],Op99
      mov dword[opcjmptab+0268h],Op9A
      mov dword[opcjmptab+026Ch],Op9B
      mov dword[opcjmptab+0270h],Op9C
      mov dword[opcjmptab+0274h],Op9D
      mov dword[opcjmptab+0278h],Op9E
      mov dword[opcjmptab+027Ch],Op9F
      mov dword[opcjmptab+0280h],OpA0
      mov dword[opcjmptab+0284h],OpA1
      mov dword[opcjmptab+0288h],OpA2
      mov dword[opcjmptab+028Ch],OpA3
      mov dword[opcjmptab+0290h],OpA4
      mov dword[opcjmptab+0294h],OpA5
      mov dword[opcjmptab+0298h],OpA6
      mov dword[opcjmptab+029Ch],OpA7
      mov dword[opcjmptab+02A0h],OpA8
      mov dword[opcjmptab+02A4h],OpA9
      mov dword[opcjmptab+02A8h],OpAA
      mov dword[opcjmptab+02ACh],OpAB
      mov dword[opcjmptab+02B0h],OpAC
      mov dword[opcjmptab+02B4h],OpAD
      mov dword[opcjmptab+02B8h],OpAE
      mov dword[opcjmptab+02BCh],OpAF
      mov dword[opcjmptab+02C0h],OpB0
      mov dword[opcjmptab+02C4h],OpB1
      mov dword[opcjmptab+02C8h],OpB2
      mov dword[opcjmptab+02CCh],OpB3
      mov dword[opcjmptab+02D0h],OpB4
      mov dword[opcjmptab+02D4h],OpB5
      mov dword[opcjmptab+02D8h],OpB6
      mov dword[opcjmptab+02DCh],OpB7
      mov dword[opcjmptab+02E0h],OpB8
      mov dword[opcjmptab+02E4h],OpB9
      mov dword[opcjmptab+02E8h],OpBA
      mov dword[opcjmptab+02ECh],OpBB
      mov dword[opcjmptab+02F0h],OpBC
      mov dword[opcjmptab+02F4h],OpBD
      mov dword[opcjmptab+02F8h],OpBE
      mov dword[opcjmptab+02FCh],OpBF
      mov dword[opcjmptab+0300h],OpC0
      mov dword[opcjmptab+0304h],OpC1
      mov dword[opcjmptab+0308h],OpC2
      mov dword[opcjmptab+030Ch],OpC3
      mov dword[opcjmptab+0310h],OpC4
      mov dword[opcjmptab+0314h],OpC5
      mov dword[opcjmptab+0318h],OpC6
      mov dword[opcjmptab+031Ch],OpC7
      mov dword[opcjmptab+0320h],OpC8
      mov dword[opcjmptab+0324h],OpC9
      mov dword[opcjmptab+0328h],OpCA
      mov dword[opcjmptab+032Ch],OpCB
      mov dword[opcjmptab+0330h],OpCC
      mov dword[opcjmptab+0334h],OpCD
      mov dword[opcjmptab+0338h],OpCE
      mov dword[opcjmptab+033Ch],OpCF
      mov dword[opcjmptab+0340h],OpD0
      mov dword[opcjmptab+0344h],OpD1
      mov dword[opcjmptab+0348h],OpD2
      mov dword[opcjmptab+034Ch],OpD3
      mov dword[opcjmptab+0350h],OpD4
      mov dword[opcjmptab+0354h],OpD5
      mov dword[opcjmptab+0358h],OpD6
      mov dword[opcjmptab+035Ch],OpD7
      mov dword[opcjmptab+0360h],OpD8
      mov dword[opcjmptab+0364h],OpD9
      mov dword[opcjmptab+0368h],OpDA
      mov dword[opcjmptab+036Ch],OpDB
      mov dword[opcjmptab+0370h],OpDC
      mov dword[opcjmptab+0374h],OpDD
      mov dword[opcjmptab+0378h],OpDE
      mov dword[opcjmptab+037Ch],OpDF
      mov dword[opcjmptab+0380h],OpE0
      mov dword[opcjmptab+0384h],OpE1
      mov dword[opcjmptab+0388h],OpE2
      mov dword[opcjmptab+038Ch],OpE3
      mov dword[opcjmptab+0390h],OpE4
      mov dword[opcjmptab+0394h],OpE5
      mov dword[opcjmptab+0398h],OpE6
      mov dword[opcjmptab+039Ch],OpE7
      mov dword[opcjmptab+03A0h],OpE8
      mov dword[opcjmptab+03A4h],OpE9
      mov dword[opcjmptab+03A8h],OpEA
      mov dword[opcjmptab+03ACh],OpEB
      mov dword[opcjmptab+03B0h],OpEC
      mov dword[opcjmptab+03B4h],OpED
      mov dword[opcjmptab+03B8h],OpEE
      mov dword[opcjmptab+03BCh],OpEF
      mov dword[opcjmptab+03C0h],OpF0
      mov dword[opcjmptab+03C4h],OpF1
      mov dword[opcjmptab+03C8h],OpF2
      mov dword[opcjmptab+03CCh],OpF3
      mov dword[opcjmptab+03D0h],OpF4
      mov dword[opcjmptab+03D4h],OpF5
      mov dword[opcjmptab+03D8h],OpF6
      mov dword[opcjmptab+03DCh],OpF7
      mov dword[opcjmptab+03E0h],OpF8
      mov dword[opcjmptab+03E4h],OpF9
      mov dword[opcjmptab+03E8h],OpFA
      mov dword[opcjmptab+03ECh],OpFB
      mov dword[opcjmptab+03F0h],OpFC
      mov dword[opcjmptab+03F4h],OpFD
      mov dword[opcjmptab+03F8h],OpFE
      mov dword[opcjmptab+03FCh],OpFF
%ifdef __MSDOS__
     call SB_alloc_dma
%endif
.nosound
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

SECTION .bss

PSampleBuf resd 24*8

NEWSYM LPFsample1, resd 1
NEWSYM LPFsample2, resd 1

NEWSYM BRRreadahead, resw 4
NEWSYM DLPFsamples, resd 8*24

section .text

%macro ProcessA 0
    shr al,4
%endmacro

%macro ProcessB 0
    and al,0Fh
%endmacro

%macro ProcessSample 1
    mov cl,[bshift]
    movzx eax,byte[esi]
    %1
    ;sign extend
    xor eax,8
    sub eax,8

    cmp cl,12
    ja %%invalid_range
    shl eax,cl
    sar eax,1
    jmp %%got_delta
%%invalid_range
    and eax,~0x7FF
%%got_delta
    mov edx,eax

    cmp dword[filter0],240
    jne %%notfilter1
    mov eax,[prev0]
    sar eax,1
    add edx,eax
    mov eax,[prev0]
    neg eax
    sar eax,5
    add edx,eax
%%notfilter1
    cmp dword[filter0],488
    jne %%notfilter2
    mov eax,[prev0]
    add edx,eax
    mov eax,[prev0]
    sar eax,1
    add eax,[prev0]
    neg eax
    sar eax,5
    add edx,eax
    mov eax,[prev1]
    sar eax,1
    sub edx,eax
    mov eax,[prev1]
    sar eax,5
    add edx,eax
%%notfilter2
    cmp dword[filter0],460
    jne %%notfilter3
    mov eax,[prev0]
    add edx,eax
    mov eax,[prev0]
    shl eax,1
    add eax,[prev0]
    shl eax,2
    add eax,[prev0]
    neg eax
    sar eax,7
    add edx,eax
    mov eax,[prev1]
    sar eax,1
    sub edx,eax
    mov eax,[prev1]
    sar eax,1
    add eax,[prev1]
    sar eax,4
    add edx,eax
%%notfilter3

    cmp edx,-32768
    jnl %%notless
    mov edx,-32768
%%notless
    cmp edx,32767
    jng %%notgreater
    mov edx,32767
%%notgreater

    mov eax,[prev0]
    mov [prev1],eax

    shl edx,1
    movsx edx,dx
    mov [prev0],edx
%endmacro

%macro ProcessDynamicLowPass 0
    mov ecx,[curvoice]
    mov edx, [Voice0Freq+ecx*4]
    cmp edx, dword 800000h
    ja %%DLPF
    ret
%%DLPF
    lea ebx,[ecx*4]
    lea ebx,[ebx*4]
    lea ecx,[ecx*4]
    lea ebx,[ecx*2+ebx]
    lea ebx,[DLPFsamples+ebx*4]
    cmp byte[LowPassFilterType],3
    je near %%DLPF_fir
;dynamic
    mov eax,[ebx+16*4]
    mov [ebx],eax
    mov eax,[ebx+17*4]
    mov [ebx+1*4],eax
    mov eax,[ebx+18*4]
    mov [ebx+2*4],eax
    mov eax,[ebx+19*4]
    mov [ebx+3*4],eax
    sub edi,32
    movsx eax,word[edi+24]
    mov [ebx+16*4],eax
    movsx eax,word[edi+26]
    mov [ebx+17*4],eax
    movsx eax,word[edi+28]
    mov [ebx+18*4],eax
    movsx eax,word[edi+30]
    mov [ebx+19*4],eax
    mov ecx,16
    shr edx,24
    cmp dl,2
    jle %%dlpf_by_2
    cmp dl,3
    jle %%dlpf_by_3
    cmp dl,4
    jle near %%dlpf_by_4
    jmp %%dlpf_by_5

%%dlpf_by_2
    mov eax,[ebx+4*4]
    jmp %%dlpf_by_2_loop

ALIGN16
%%dlpf_by_2_loop
    movsx edx,word[edi]
    add eax,edx
    sar eax,1
    mov [edi],ax
    mov eax,edx
    add edi,2
    dec ecx
    jnz %%dlpf_by_2_loop
    ret

%%dlpf_by_3
    mov eax,[ebx+3*4]
    mov ebp,[ebx+4*4]
    jmp %%dlpf_by_3_loop

ALIGN16
%%dlpf_by_3_loop
    movsx ebx,word[edi]
    add eax,ebx
    add eax,ebp
    mov edx,55555555h ; (1/3)
    imul edx
    mov [edi],dx
    add edi,2
    mov eax,ebp
    mov ebp,ebx
    dec ecx
    jnz %%dlpf_by_3_loop
    ret

%%dlpf_by_4
    mov eax,[ebx+2*4]
    mov edx,[ebx+3*4]
    mov ebp,[ebx+4*4]
    jmp %%dlpf_by_4_loop

ALIGN16
%%dlpf_by_4_loop
    movsx ebx,word[edi]
    add eax,ebx
    add eax,edx
    add eax,ebp
    sar eax,2
    mov [edi],ax
    add edi,2
    mov eax,edx
    mov edx,ebp
    mov ebp,ebx
    dec ecx
    jnz %%dlpf_by_4_loop
    ret

%%dlpf_by_5
    push ecx
    mov eax,[ebx+1*4]
    mov esi,[ebx+2*4]
    mov ebp,[ebx+3*4]
    mov ecx,[ebx+4*4]
    jmp %%dlpf_by_5_loop

ALIGN16
%%dlpf_by_5_loop
    movsx ebx,word[edi]
    add eax,ebx
    add eax,esi
    add eax,ebp
    add eax,ecx
    mov edx,33333333h   ; 1/5
    imul edx
    mov [edi],dx
    add edi,2
    mov eax,esi
    mov esi,ebp
    mov ebp,ecx
    mov ecx,ebx
    dec dword[esp]
    jnz %%dlpf_by_5_loop
    pop ecx
    ret

%%DLPF_fir
    sub edi,byte 32
    sub edx,0780000h    ;ac - address calculation (see below)
    mov eax,[ebx+16*4]
    mov ecx,[ebx+17*4]
    mov [ebx],eax
    mov [ebx+1*4],ecx
    shr edx,18          ;ac
    mov eax,[ebx+18*4]
    and edx,3FF0h       ;ac
    mov ecx,[ebx+19*4]
    mov [ebx+2*4],eax
    mov [ebx+3*4],ecx

    movq mm1,[edi]      ;u1 (U-pipe marker)
    movq mm3,[edi+8]    ;u2
    punpcklwd mm0,mm1
    movq mm5,[edi+16]   ;u3
    psrad mm0, 16
    movq mm7,[edi+24]   ;u4
    punpckhwd mm1,mm1
    movq [ebx+4*4],mm0  ;u5
    psrad mm1, 16
    movq mm4,[BRRreadahead] ;u6
    punpcklwd mm0,mm3
    movq [ebx+6*4],mm1  ;u7
    psrad mm0, 16
    punpckhwd mm3,mm3   ;u8
    movq [ebx+8*4],mm0  ;u9
    psrad mm3, 16
    punpcklwd mm0,mm5   ;u10
    movq [ebx+10*4],mm3 ;u11
    psrad mm0, 16
    punpckhwd mm5,mm5   ;u12
    movq [ebx+12*4],mm0 ;u13
    psrad mm5, 16
    punpcklwd mm0,mm7   ;u14
    movq [ebx+14*4],mm5 ;u15
    psrad mm0, 16
    punpckhwd mm7,mm7   ;u16
    movq mm2,[fir_lut_co+edx]   ;u17
    psrad mm7, 16
    punpcklwd mm6,mm4   ;u18
    movq [ebx+16*4],mm0 ;u19
    psrad mm6, 16
    movq [ebx+18*4],mm7 ;u20
    punpckhwd mm4,mm4
    movq mm3,[fir_lut_co+edx+8] ;u21
    psrad mm4, 16
    movq [ebx+20*4],mm6 ;u22
    movq [ebx+22*4],mm4 ;u23

    mov ecx,16
    jmp %%DLPF_fir_loop
    ALIGN16

; output 2 samples per iteration
%%DLPF_fir_loop
    movq mm0,[ebx]
    packssdw mm0,[ebx+8]
    movq mm4,[ebx+4]
    pmaddwd mm0,mm2
    packssdw mm4,[ebx+12]
    movq mm1,[ebx+16]
    pmaddwd mm4,mm2
    packssdw mm1,[ebx+24]
    movq mm5,[ebx+20]
    pmaddwd mm1,mm3
    packssdw mm5,[ebx+28]
    pmaddwd mm5,mm3
    paddd mm0,mm1
    add ebx,byte 8
    paddd mm4,mm5
    movq mm1,mm0
    movq mm5,mm4
    psrlq mm0,32
    psrlq mm4,32
    paddd mm0,mm1
    paddd mm4,mm5
    punpckldq mm0,mm4
    psrad mm0,14
    packssdw mm0,mm0
    sub ecx,byte 2
    movd [edi],mm0
    lea edi,[edi+4]
    jnz %%DLPF_fir_loop
    emms
    ret
%endmacro

section .bss
NEWSYM curvoice, resd 1
section .text

BRRDecode:
    mov [curvoice],ecx
    mov byte[lastbl],0
    mov byte[loopbl],0
    push ecx

    movzx eax,byte[esi]
    test al,01h
    jz .nolast
    mov byte[lastbl],1
    test al,02h
    jz .nolast
    mov byte[loopbl],1
.nolast
    mov cl,al
    and al,0Ch
    inc esi
    mov ebx,[Filter+eax*2]
    shr cl,4
    mov [filter0],ebx
    mov ebx,[Filter+eax*2+4]
    mov [bshift],cl
    mov [filter1],ebx
    mov byte[sampleleft],8
    jmp .nextsample
    ALIGN16

.nextsample
    ProcessSample ProcessA
    mov [edi],dx
    ProcessSample ProcessB
    mov [edi+2],dx
    add edi,4
    inc esi
    dec byte[sampleleft]
    jnz .nextsample

    cmp dword[DSPInterpolate],0
    jnz .BRR_decode_ahead

    cmp byte[LowPassFilterType],2
    jle near .no_dlpf

    mov eax,[curvoice]
    mov eax,[Voice0Freq+eax*4]
    cmp eax,800000h
    jb near .no_dlpf

.BRR_decode_ahead

    push esi

    cmp byte[lastbl],1
    jne .dlpf_fill

    cmp byte[loopbl],1
    jne near .dlpf_clear

    mov eax,[curvoice]
    mov esi,[Voice0LoopPtr+eax*4]
    add esi,SPCRAM

.dlpf_fill
    push dword[prev0]
    push dword[prev1]

    movzx eax,byte[esi]
    mov cl,al
    and al,0Ch
    inc esi
    mov ebx,[Filter+eax*2]
    shr cl,4
    mov [filter0],ebx
    mov ebx,[Filter+eax*2+4]
    mov [bshift],cl
    mov [filter1],ebx

    ProcessSample ProcessA
    mov [BRRreadahead],dx
    ProcessSample ProcessB
    mov [BRRreadahead+2],dx
    inc esi
    ProcessSample ProcessA
    mov [BRRreadahead+4],dx
    ProcessSample ProcessB
    mov [BRRreadahead+6],dx

    pop dword[prev1]
    pop dword[prev0]
    pop esi
    jmp .no_dlpf

.dlpf_clear
    xor eax,eax
    mov [BRRreadahead],eax
    mov [BRRreadahead+4],eax
    pop esi

.no_dlpf

    pop ecx

    cmp byte[LowPassFilterType],1
    ja .dlpf
    ret

.dlpf
    ProcessDynamicLowPass

section .data
ALIGN32

; Original Values
NEWSYM EchoRateO
  dd 2,172,344,517,689,861,1033,1205,1378,1550,1722,1895,2067,2239,2412,2584
NEWSYM AttackRateO
  dd 45202,28665,16537,11025,7056,4189,2866,1764,1058,705,441,264,176,110,66,4
NEWSYM DecayRateO
  dd 13230,8158,4851,2697,2284,1212,815,407
NEWSYM SustainRateO
  dd 0FFFFFFFFh,418950,308700,265600,209475,154350,132300,103635,78277,65047
  dd 51817,38587,31972,26460,19845,16537,13230,9702,8158,6504,4851,3879,2697
  dd 2050,1572,1212,1014,815,606,407,202,125
NEWSYM SustainValueO
  db 15,31,47,63,79,95,111,127
NEWSYM IncreaseO
  dd 0FFFFFFFFh,45202,34177,28665,22050,16537,14332,11025,8489,7056,5622,4189
  dd 3528,2866,2094,1764,1433,1058,882,705,529,441,352,264,220,176,132,110,88
  dd 66,44,22
NEWSYM IncreaseBentO
  dd 0FFFFFFFFh,79100,59535,50160,38580,28665,25000,19250,14332,12127,9800,7320
  dd 6160,4961,3650,3060,2425,1845,1540,1212,920,770,614,460,383,306,229,190,152
  dd 113,75,36
NEWSYM DecreaseO
  dd 0FFFFFFFFh,45202,34177,28665,22050,16537,14332,11025,8489,7056,5622,4189
  dd 3528,2866,2094,1764,1433,1058,882,705,529,441,352,264,220,176,132,110,88,66
  dd 44,22
NEWSYM DecreaseRateExpO
  dd 0FFFFFFFFh,418950,308700,264600,209470,154350,132300,103635,78277,65047
  dd 51817,38587,31972,26460,19845,16537,13230,9702,8158,6504,4851,4079,3197
  dd 2425,1984,1653,1212,1014,815,606,407,198

SECTION .bss
NoisePtr           resd 1

; used only in dspproc.asm
SECTION .data
ALIGN32

Filter dd 0,0,240,0,488,-240,460,-208

prev0              dd 0         ; previous value 1
prev1              dd 0         ; previous value 2
nextsamp           dd 0         ; next sample
filter0            dd 0         ; filter 0
filter1            dd 0         ; filter 1
bshift             dd 0
sampleleft         dd 0         ; 8 bytes/sample

lastbl             dd 0         ; Last block if = 1
loopbl             dd 0         ; Loop if = 1
usenoisedata       dd 0

VolumeTableD:
db 0,3,6,9,12,15,17,18,19,21,22,23,24,24,26,28,30,31,33,35,36,38,40,41,43,45,46,48,49
db 51,52,54,56,57,58,60,61,63,64,66,67,68,70,71,72,74,75,76,78,79,80,81,82,84,85,86
db 87,88,89,90,91,92,93,94,96,96,97,98,99,100,101,102,103,104,105,106,106,107,108
db 109,110,110,111,112,112,113,114,114,115,116,116,117,117,118,118,119,120,120,120
db 121,121,122,122,123,123,123,124,124,124,125,125,125,126,126,126,126,126,127,127
db 127,127,127,127,127,127,127,127,127,128,128,128,128,128,128,128,128,128,128
db 128,129,129,129,129,129,130,130,130,131,131,131,132,132,132,133,133,134,134,135
db 135,135,136,137,137,138,138,139,139,140,141,141,142,143,143,144,145,145,146,147
db 148,149,149,150,151,152,153,154,155,156,157,158,159,159,161,162,163,164,165,166
db 167,168,169,170,171,173,174,175,176,177,179,180,181,183,184,185,187,188,189,191
db 192,194,195,197,198,199,201,203,204,206,207,209,210,212,214,215,217,219,220,222
db 224,225,227,229,231,231,232,233,234,236,237,238,240,243,246,249,252,255

db 0,1,3,5,7,9,11,13,15,17,19,21,22,24,26,28,30,31,33,35,36,38,40,41,43,45,46,48,49
db 51,52,54,56,57,58,60,61,63,64,66,67,68,70,71,72,74,75,76,78,79,80,81,82,84,85,86
db 87,88,89,90,91,92,93,94,96,96,97,98,99,100,101,102,103,104,105,106,106,107,108
db 109,110,110,111,112,112,113,114,114,115,116,116,117,117,118,118,119,120,120,120
db 121,121,122,122,123,123,123,124,124,124,125,125,125,126,126,126,126,126,127,127
db 127,127,127,127,127,127,127,127,127,128,128,128,128,128,128,128,128,128,128
db 128,129,129,129,129,129,130,130,130,131,131,131,132,132,132,133,133,134,134,135
db 135,135,136,137,137,138,138,139,139,140,141,141,142,143,143,144,145,145,146,147
db 148,149,149,150,151,152,153,154,155,156,157,158,159,159,161,162,163,164,165,166
db 167,168,169,170,171,173,174,175,176,177,179,180,181,183,184,185,187,188,189,191
db 192,194,195,197,198,199,201,203,204,206,207,209,210,212,214,215,217,219,220,222
db 224,225,227,229,231,233,234,236,238,240,242,244,246,248,250,252,254,255

NEWSYM VolumeTableb
               db 00h,01h,02h,03h,04h,05h,06h,07h,08h,09h,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh
               db 10h,11h,12h,13h,14h,15h,16h,17h,18h,19h,1Ah,1Bh,1Ch,1Dh,1Eh,1Fh
               db 20h,21h,22h,23h,24h,25h,26h,27h,28h,29h,2Ah,2Bh,2Ch,2Dh,2Eh,2Fh
               db 30h,31h,32h,33h,34h,35h,36h,37h,38h,39h,3Ah,3Bh,3Ch,3Dh,3Eh,3Fh
               db 40h,41h,42h,43h,44h,45h,46h,47h,48h,49h,4Ah,4Bh,4Ch,4Dh,4Eh,4Fh
               db 50h,51h,52h,53h,54h,55h,56h,57h,58h,59h,5Ah,5Bh,5Ch,5Dh,5Eh,5Fh
               db 60h,61h,62h,63h,64h,65h,66h,67h,68h,69h,6Ah,6Bh,6Ch,6Dh,6Eh,6Fh
               db 70h,71h,72h,73h,74h,75h,76h,77h,78h,79h,7Ah,7Bh,7Ch,7Dh,7Eh,7Fh
               db 7Fh,7Eh,7Dh,7Ch,7Bh,7Ah,79h,78h,77h,76h,75h,74h,73h,72h,71h,70h
               db 6Fh,6Eh,6Dh,6Ch,6Bh,6Ah,69h,68h,67h,66h,65h,64h,63h,62h,61h,60h
               db 5Fh,5Eh,5Dh,5Ch,5Bh,5Ah,59h,58h,57h,56h,55h,54h,53h,52h,51h,50h
               db 4Fh,4Eh,4Dh,4Ch,4Bh,4Ah,49h,48h,47h,46h,45h,44h,43h,42h,41h,40h
               db 3Fh,3Eh,3Dh,3Ch,3Bh,3Ah,39h,38h,37h,36h,35h,34h,33h,32h,31h,30h
               db 2Fh,2Eh,2Dh,2Ch,2Bh,2Ah,29h,28h,27h,26h,25h,24h,23h,22h,21h,20h
               db 1Fh,1Eh,1Dh,1Ch,1Bh,1Ah,19h,18h,17h,16h,15h,14h,13h,12h,11h,10h
               db 0Fh,0Eh,0Dh,0Ch,0Bh,0Ah,09h,08h,07h,06h,05h,04h,03h,02h,01h,00h

; appears to only be used in dspproc.asm

;VolumeTable:   db 0,2,4,6,8,10,12,14,16,18
;               db 20,22,24,26,28,30,32,34,36,38
;               db 40,42,44,46,48,50,52,54,56,58
;               db 60,62,64,66,68,70,72,74,76,78
;               db 80,82,84,86,88,90,92,94,96,98
;               db 100,102,104,106,108,110,112,114,116,118
;               db 120,122,124,126,127,127,127,127,127,127
;VolumeTable:   db 127,127,127,127,127,127,127,127,127,127
;               db 127,127,127,127,127,127,127,127,127,127
;               db 127,127,127,127,127,127,127,127,127,127
;               db 127,127,127,127,127,127,127,127,127,127
;               db 127,127,127,127,127,127,127,127,127,127
;               db 127,127,127,127,127,127,127,127
;               db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127,127,127
;                db 127,127,127,127,127,127,127,127
;VolumeTable     db 1,1,2,3,4,5,6,7,8,9
;                db 10,11,12,13,14,15,16,17,18,19
;                db 20,21,22,23,24,25,26,27,28,29
;                db 30,31,32,33,34,35,36,37,38,39
;                db 40,41,42,43,44,45,46,47,48,49
;                db 50,51,52,53,54,55,46,57,58,59
;                db 60,61,62,63,64,65,56,67,68,69
;                db 70,71,72,73,74,75,66,77,78,79
;                db 80,81,82,83,84,85,76,87,88,89
;                db 90,91,92,93,94,95,86,97,98,99
;                db 100,101,102,103,104,105,106,107,108,109
;                db 110,111,112,113,114,115,116,117,118,119
;                db 120,121,122,123,124,125,126,127
;VolumeTable     db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1,1,1
;                db 1,1,1,1,1,1,1,1

SECTION .bss
NEWSYM DSPBuffer, resd 320*4 ; The play buffer...
NEWSYM EchoBuffer, resd 320*4 ; The play buffer...
NEWSYM PModBuffer, resd 320*4 ; The play buffer...
NEWSYM BRRBuffer, resb 32   ; The BRR Decode Buffer

NEWSYM BRRPlace0, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp0,  resd 1             ; Keep this 0
NEWSYM BRRPlace1, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp1,  resd 1             ; Keep this 0
NEWSYM BRRPlace2, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp2,  resd 1             ; Keep this 0
NEWSYM BRRPlace3, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp3,  resd 1             ; Keep this 0
NEWSYM BRRPlace4, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp4,  resd 1             ; Keep this 0
NEWSYM BRRPlace5, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp5,  resd 1             ; Keep this 0
NEWSYM BRRPlace6, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp6,  resd 1             ; Keep this 0
NEWSYM BRRPlace7, resd 1             ; Place in the BRRBuffer for Voice 0
NEWSYM BRRTemp7,  resd 1             ; Keep this 0

NEWSYM Voice0Freq, resd 1            ; Frequency of Voice 0 (Delta Freq)
NEWSYM Voice1Freq, resd 1            ; Frequency of Voice 1 (Delta Freq)
NEWSYM Voice2Freq, resd 1            ; Frequency of Voice 2 (Delta Freq)
NEWSYM Voice3Freq, resd 1            ; Frequency of Voice 3 (Delta Freq)
NEWSYM Voice4Freq, resd 1            ; Frequency of Voice 4 (Delta Freq)
NEWSYM Voice5Freq, resd 1            ; Frequency of Voice 5 (Delta Freq)
NEWSYM Voice6Freq, resd 1            ; Frequency of Voice 6 (Delta Freq)
NEWSYM Voice7Freq, resd 1            ; Frequency of Voice 7 (Delta Freq)

NEWSYM Voice0Pitch, resw 1            ; Previous Pitch for Voice 0
NEWSYM Voice1Pitch, resw 1            ; Previous Pitch for Voice 1
NEWSYM Voice2Pitch, resw 1            ; Previous Pitch for Voice 2
NEWSYM Voice3Pitch, resw 1            ; Previous Pitch for Voice 3
NEWSYM Voice4Pitch, resw 1            ; Previous Pitch for Voice 4
NEWSYM Voice5Pitch, resw 1            ; Previous Pitch for Voice 5
NEWSYM Voice6Pitch, resw 1            ; Previous Pitch for Voice 6
NEWSYM Voice7Pitch, resw 1            ; Previous Pitch for Voice 7

NEWSYM Voice0Status,   resb 1 ; 0=Not Playing 1=Playing
NEWSYM Voice1Status,   resb 1
NEWSYM Voice2Status,   resb 1
NEWSYM Voice3Status,   resb 1
NEWSYM Voice4Status,   resb 1
NEWSYM Voice5Status,   resb 1
NEWSYM Voice6Status,   resb 1
NEWSYM Voice7Status,   resb 1

NEWSYM Voice0Ptr,      resd 1 ; Ptr to Next BRR Block to be played
NEWSYM Voice1Ptr,      resd 1
NEWSYM Voice2Ptr,      resd 1
NEWSYM Voice3Ptr,      resd 1
NEWSYM Voice4Ptr,      resd 1
NEWSYM Voice5Ptr,      resd 1
NEWSYM Voice6Ptr,      resd 1
NEWSYM Voice7Ptr,      resd 1
NEWSYM Voice0LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice1LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice2LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice3LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice4LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice5LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice6LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice7LoopPtr,  resd 1 ; Ptr to Loop BRR Block to be played

NEWSYM Voice0BufPtr,  resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice1BufPtr,  resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice2BufPtr,  resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice3BufPtr,  resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice4BufPtr,  resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice5BufPtr,  resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice6BufPtr,  resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice7BufPtr,  resd 1 ; Ptr to Buffer Block to be played

NEWSYM SoundCounter,   resd 1 ; Counter used for sound generation
NEWSYM SoundCounter2,  resd 1 ; Counter used for sound generation
NEWSYM Voice0Prev0,    resd 1
NEWSYM Voice1Prev0,    resd 1
NEWSYM Voice2Prev0,    resd 1
NEWSYM Voice3Prev0,    resd 1
NEWSYM Voice4Prev0,    resd 1
NEWSYM Voice5Prev0,    resd 1
NEWSYM Voice6Prev0,    resd 1
NEWSYM Voice7Prev0,    resd 1
NEWSYM Voice0Prev1,    resd 1
NEWSYM Voice1Prev1,    resd 1
NEWSYM Voice2Prev1,    resd 1
NEWSYM Voice3Prev1,    resd 1
NEWSYM Voice4Prev1,    resd 1
NEWSYM Voice5Prev1,    resd 1
NEWSYM Voice6Prev1,    resd 1
NEWSYM Voice7Prev1,    resd 1

NEWSYM Voice0Loop,     resb 1
NEWSYM Voice1Loop,     resb 1
NEWSYM Voice2Loop,     resb 1
NEWSYM Voice3Loop,     resb 1
NEWSYM Voice4Loop,     resb 1
NEWSYM Voice5Loop,     resb 1
NEWSYM Voice6Loop,     resb 1
NEWSYM Voice7Loop,     resb 1

NEWSYM Voice0End,      resb 1
NEWSYM Voice1End,      resb 1
NEWSYM Voice2End,      resb 1
NEWSYM Voice3End,      resb 1
NEWSYM Voice4End,      resb 1
NEWSYM Voice5End,      resb 1
NEWSYM Voice6End,      resb 1
NEWSYM Voice7End,      resb 1

NEWSYM Voice0Noise,    resb 1
NEWSYM Voice1Noise,    resb 1
NEWSYM Voice2Noise,    resb 1
NEWSYM Voice3Noise,    resb 1
NEWSYM Voice4Noise,    resb 1
NEWSYM Voice5Noise,    resb 1
NEWSYM Voice6Noise,    resb 1
NEWSYM Voice7Noise,    resb 1

NEWSYM Voice0Volume,   resb 1
NEWSYM Voice1Volume,   resb 1
NEWSYM Voice2Volume,   resb 1
NEWSYM Voice3Volume,   resb 1
NEWSYM Voice4Volume,   resb 1
NEWSYM Voice5Volume,   resb 1
NEWSYM Voice6Volume,   resb 1
NEWSYM Voice7Volume,   resb 1

NEWSYM Voice0VolumeR,   resb 1
NEWSYM Voice1VolumeR,   resb 1
NEWSYM Voice2VolumeR,   resb 1
NEWSYM Voice3VolumeR,   resb 1
NEWSYM Voice4VolumeR,   resb 1
NEWSYM Voice5VolumeR,   resb 1
NEWSYM Voice6VolumeR,   resb 1
NEWSYM Voice7VolumeR,   resb 1

NEWSYM Voice0VolumeL,   resb 1
NEWSYM Voice1VolumeL,   resb 1
NEWSYM Voice2VolumeL,   resb 1
NEWSYM Voice3VolumeL,   resb 1
NEWSYM Voice4VolumeL,   resb 1
NEWSYM Voice5VolumeL,   resb 1
NEWSYM Voice6VolumeL,   resb 1
NEWSYM Voice7VolumeL,   resb 1

NEWSYM Voice0Env,      resb 1
NEWSYM Voice1Env,      resb 1
NEWSYM Voice2Env,      resb 1
NEWSYM Voice3Env,      resb 1
NEWSYM Voice4Env,      resb 1
NEWSYM Voice5Env,      resb 1
NEWSYM Voice6Env,      resb 1
NEWSYM Voice7Env,      resb 1

NEWSYM Voice0Out,      resb 1
NEWSYM Voice1Out,      resb 1
NEWSYM Voice2Out,      resb 1
NEWSYM Voice3Out,      resb 1
NEWSYM Voice4Out,      resb 1
NEWSYM Voice5Out,      resb 1
NEWSYM Voice6Out,      resb 1
NEWSYM Voice7Out,      resb 1

; 1 Attack, 2 Decrease,3 Sustain, 0 Gain
NEWSYM Voice0State,   resb 1
NEWSYM Voice1State,   resb 1
NEWSYM Voice2State,   resb 1
NEWSYM Voice3State,   resb 1
NEWSYM Voice4State,   resb 1
NEWSYM Voice5State,   resb 1
NEWSYM Voice6State,   resb 1
NEWSYM Voice7State,   resb 1

NEWSYM Voice0Time,     resd 1
NEWSYM Voice1Time,     resd 1
NEWSYM Voice2Time,     resd 1
NEWSYM Voice3Time,     resd 1
NEWSYM Voice4Time,     resd 1
NEWSYM Voice5Time,     resd 1
NEWSYM Voice6Time,     resd 1
NEWSYM Voice7Time,     resd 1

NEWSYM Voice0Attack,   resd 1
NEWSYM Voice1Attack,   resd 1
NEWSYM Voice2Attack,   resd 1
NEWSYM Voice3Attack,   resd 1
NEWSYM Voice4Attack,   resd 1
NEWSYM Voice5Attack,   resd 1
NEWSYM Voice6Attack,   resd 1
NEWSYM Voice7Attack,   resd 1

NEWSYM Voice0Decay, resd 1
NEWSYM Voice1Decay, resd 1
NEWSYM Voice2Decay, resd 1
NEWSYM Voice3Decay, resd 1
NEWSYM Voice4Decay, resd 1
NEWSYM Voice5Decay, resd 1
NEWSYM Voice6Decay, resd 1
NEWSYM Voice7Decay, resd 1

NEWSYM Voice0SustainL, resb 1
NEWSYM Voice1SustainL, resb 1
NEWSYM Voice2SustainL, resb 1
NEWSYM Voice3SustainL, resb 1
NEWSYM Voice4SustainL, resb 1
NEWSYM Voice5SustainL, resb 1
NEWSYM Voice6SustainL, resb 1
NEWSYM Voice7SustainL, resb 1

NEWSYM Voice0SustainL2, resb 1
NEWSYM Voice1SustainL2, resb 1
NEWSYM Voice2SustainL2, resb 1
NEWSYM Voice3SustainL2, resb 1
NEWSYM Voice4SustainL2, resb 1
NEWSYM Voice5SustainL2, resb 1
NEWSYM Voice6SustainL2, resb 1
NEWSYM Voice7SustainL2, resb 1

NEWSYM Voice0SustainR, resd 1
NEWSYM Voice1SustainR, resd 1
NEWSYM Voice2SustainR, resd 1
NEWSYM Voice3SustainR, resd 1
NEWSYM Voice4SustainR, resd 1
NEWSYM Voice5SustainR, resd 1
NEWSYM Voice6SustainR, resd 1
NEWSYM Voice7SustainR, resd 1

NEWSYM Voice0SustainR2, resd 1
NEWSYM Voice1SustainR2, resd 1
NEWSYM Voice2SustainR2, resd 1
NEWSYM Voice3SustainR2, resd 1
NEWSYM Voice4SustainR2, resd 1
NEWSYM Voice5SustainR2, resd 1
NEWSYM Voice6SustainR2, resd 1
NEWSYM Voice7SustainR2, resd 1

NEWSYM Voice0IncNumber,  resd 1
NEWSYM Voice1IncNumber,  resd 1
NEWSYM Voice2IncNumber,  resd 1
NEWSYM Voice3IncNumber,  resd 1
NEWSYM Voice4IncNumber,  resd 1
NEWSYM Voice5IncNumber,  resd 1
NEWSYM Voice6IncNumber,  resd 1
NEWSYM Voice7IncNumber,  resd 1

; END formerly initialized to 1 junk

NEWSYM Voice0SLenNumber,  resd 1
NEWSYM Voice1SLenNumber,  resd 1
NEWSYM Voice2SLenNumber,  resd 1
NEWSYM Voice3SLenNumber,  resd 1
NEWSYM Voice4SLenNumber,  resd 1
NEWSYM Voice5SLenNumber,  resd 1
NEWSYM Voice6SLenNumber,  resd 1
NEWSYM Voice7SLenNumber,  resd 1

NEWSYM Voice0SEndNumber,  resd 1
NEWSYM Voice1SEndNumber,  resd 1
NEWSYM Voice2SEndNumber,  resd 1
NEWSYM Voice3SEndNumber,  resd 1
NEWSYM Voice4SEndNumber,  resd 1
NEWSYM Voice5SEndNumber,  resd 1
NEWSYM Voice6SEndNumber,  resd 1
NEWSYM Voice7SEndNumber,  resd 1

NEWSYM Voice0SEndLNumber,  resd 1
NEWSYM Voice1SEndLNumber,  resd 1
NEWSYM Voice2SEndLNumber,  resd 1
NEWSYM Voice3SEndLNumber,  resd 1
NEWSYM Voice4SEndLNumber,  resd 1
NEWSYM Voice5SEndLNumber,  resd 1
NEWSYM Voice6SEndLNumber,  resd 1
NEWSYM Voice7SEndLNumber,  resd 1

; MORE junk that was initialized to 1

NEWSYM Voice0DecreaseNumber,  resd 1
NEWSYM Voice1DecreaseNumber,  resd 1
NEWSYM Voice2DecreaseNumber,  resd 1
NEWSYM Voice3DecreaseNumber,  resd 1
NEWSYM Voice4DecreaseNumber,  resd 1
NEWSYM Voice5DecreaseNumber,  resd 1
NEWSYM Voice6DecreaseNumber,  resd 1
NEWSYM Voice7DecreaseNumber,  resd 1

NEWSYM Voice0EnvInc,          resd 1
NEWSYM Voice1EnvInc,          resd 1
NEWSYM Voice2EnvInc,          resd 1
NEWSYM Voice3EnvInc,          resd 1
NEWSYM Voice4EnvInc,          resd 1
NEWSYM Voice5EnvInc,          resd 1
NEWSYM Voice6EnvInc,          resd 1
NEWSYM Voice7EnvInc,          resd 1

; END initialized to 1 junk

; 0 = Direct, 1 = Increase, 2 = Increase2, 3 = Decrease, 4 = Decrease2
NEWSYM Voice0GainType,       resb 1
NEWSYM Voice1GainType,       resb 1
NEWSYM Voice2GainType,       resb 1
NEWSYM Voice3GainType,       resb 1
NEWSYM Voice4GainType,       resb 1
NEWSYM Voice5GainType,       resb 1
NEWSYM Voice6GainType,       resb 1
NEWSYM Voice7GainType,       resb 1

; YET ANOTHER block that was initialized to 1

NEWSYM Voice0GainTime,       resd 1
NEWSYM Voice1GainTime,       resd 1
NEWSYM Voice2GainTime,       resd 1
NEWSYM Voice3GainTime,       resd 1
NEWSYM Voice4GainTime,       resd 1
NEWSYM Voice5GainTime,       resd 1
NEWSYM Voice6GainTime,       resd 1
NEWSYM Voice7GainTime,       resd 1

NEWSYM useless, resd 2    ;This is needed because of the stupid
                          ;alignment dependency in the savestates.

NEWSYM Freqdisp,        resd 1
NEWSYM SBRateb,         resd 1

NEWSYM Voice0Looped,            resb 1
NEWSYM Voice1Looped,            resb 1
NEWSYM Voice2Looped,            resb 1
NEWSYM Voice3Looped,            resb 1
NEWSYM Voice4Looped,            resb 1
NEWSYM Voice5Looped,            resb 1
NEWSYM Voice6Looped,            resb 1
NEWSYM Voice7Looped,            resb 1

VoiceNoiseEn resb 8
NEWSYM GainDecBendDataPos, resb 8
NEWSYM GainDecBendDataTime, resd 8
NEWSYM GainDecBendDataDat, resb 8

NEWSYM AdsrBlocksLeft, resb 8
NEWSYM AdsrNextTimeDepth, resd 8

TimeTemp   resd 8   ; 104 bytes
IncNTemp   resd 8
EnvITemp   resd 8
StatTemp   resd 2

NEWSYM FutureExpand,   resb 44
; pharos equ hack *sigh*
marksave:

NEWSYM echoon0,   resb 1
NEWSYM echoon1,   resb 1
NEWSYM echoon2,   resb 1
NEWSYM echoon3,   resb 1
NEWSYM echoon4,   resb 1
NEWSYM echoon5,   resb 1
NEWSYM echoon6,   resb 1
NEWSYM echoon7,   resb 1

NEWSYM GlobalVL,   resd 1
NEWSYM GlobalVR,   resd 1
NEWSYM EchoVL,   resd 1
NEWSYM EchoVR,   resd 1
NEWSYM EchoT,    resd 1

NEWSYM Voice0Volumee,  resb 1
NEWSYM Voice1Volumee,  resb 1
NEWSYM Voice2Volumee,  resb 1
NEWSYM Voice3Volumee,  resb 1
NEWSYM Voice4Volumee,  resb 1
NEWSYM Voice5Volumee,  resb 1
NEWSYM Voice6Volumee,  resb 1
NEWSYM Voice7Volumee,  resb 1

NEWSYM Voice0VolumeRe,  resb 1
NEWSYM Voice1VolumeRe,  resb 1
NEWSYM Voice2VolumeRe,  resb 1
NEWSYM Voice3VolumeRe,  resb 1
NEWSYM Voice4VolumeRe,  resb 1
NEWSYM Voice5VolumeRe,  resb 1
NEWSYM Voice6VolumeRe,  resb 1
NEWSYM Voice7VolumeRe,  resb 1

NEWSYM Voice0VolumeLe,  resb 1
NEWSYM Voice1VolumeLe,  resb 1
NEWSYM Voice2VolumeLe,  resb 1
NEWSYM Voice3VolumeLe,  resb 1
NEWSYM Voice4VolumeLe,  resb 1
NEWSYM Voice5VolumeLe,  resb 1
NEWSYM Voice6VolumeLe,  resb 1
NEWSYM Voice7VolumeLe,  resb 1

NEWSYM FIRTAPVal0,      resd 1
NEWSYM FIRTAPVal1,      resd 1
NEWSYM FIRTAPVal2,      resd 1
NEWSYM FIRTAPVal3,      resd 1
NEWSYM FIRTAPVal4,      resd 1
NEWSYM FIRTAPVal5,      resd 1
NEWSYM FIRTAPVal6,      resd 1
NEWSYM FIRTAPVal7,      resd 1

NEWSYM CEchoPtr,        resd 1
NEWSYM EchoFB,          resd 1

NEWSYM Voice0Ptre,     resd 1 ; Ptr to Next BRR Block to be played
NEWSYM Voice1Ptre,     resd 1
NEWSYM Voice2Ptre,     resd 1
NEWSYM Voice3Ptre,     resd 1
NEWSYM Voice4Ptre,     resd 1
NEWSYM Voice5Ptre,     resd 1
NEWSYM Voice6Ptre,     resd 1
NEWSYM Voice7Ptre,     resd 1
NEWSYM Voice0LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice1LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice2LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice3LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice4LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice5LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice6LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played
NEWSYM Voice7LoopPtre, resd 1 ; Ptr to Loop BRR Block to be played

NEWSYM Voice0BufPtre, resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice1BufPtre, resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice2BufPtre, resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice3BufPtre, resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice4BufPtre, resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice5BufPtre, resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice6BufPtre, resd 1 ; Ptr to Buffer Block to be played
NEWSYM Voice7BufPtre, resd 1 ; Ptr to Buffer Block to be played

NEWSYM CurFiltPtr, resd 1
NEWSYM FiltLoop, resd 16
NEWSYM FiltLoopR, resd 16

NEWSYM SoundLooped0, resb 1
NEWSYM SoundLooped1, resb 1
NEWSYM SoundLooped2, resb 1
NEWSYM SoundLooped3, resb 1
NEWSYM SoundLooped4, resb 1
NEWSYM SoundLooped5, resb 1
NEWSYM SoundLooped6, resb 1
NEWSYM SoundLooped7, resb 1

NEWSYM UniqueSoundv, resd 1

NEWSYM Voice0FirstBlock,  resb 1
NEWSYM Voice1FirstBlock,  resb 1
NEWSYM Voice2FirstBlock,  resb 1
NEWSYM Voice3FirstBlock,  resb 1
NEWSYM Voice4FirstBlock,  resb 1
NEWSYM Voice5FirstBlock,  resb 1
NEWSYM Voice6FirstBlock,  resb 1
NEWSYM Voice7FirstBlock,  resb 1

marksave2:

SECTION .data
ALIGN32

NEWSYM MaxEcho,         dd 172

;    |AR Time 0 to 1|DR|Time 1 to SL|SL|Ratio| SR Time 1to 1/10|
;---------------------------------------------------------------------
;    |0 |  4.1 sec  | 0|   1.2 sec  | 0| 1/8 |  0| INF         |10|1.2 sec
;    |1 |  2.6      | 1| 740  msec  | 1| 2/8 |  1| 38          |11|880 msec
;    |2 |  1.5      | 2| 440        | 2| 3/8 |  2| 28          |12|740
;    |3 |  1.0      | 3| 290        | 3| 4/8 |  3| 24          |13|590
;    |4 |640 msec   | 4| 180        | 4| 5/8 |  4| 19          |14|440
;    |5 |380        | 5| 110        | 5| 6/8 |  5| 14          |15|370
;    |6 |260        | 6|  74        | 6| 7/8 |  6| 12          |16|290
;    |7 |160        | 7|  37        | 7| 1   |  7|  9.4        |17|220
;    |8 | 96        --------------------------  8|  7.1        |18|180
;    |9 | 64        |                        |  9|  5.9        |19|150
;    |A | 40        |                        |  A|  4.7        |1A|110
;    |B | 24        |                        |  B|  3.5        |1B| 92
;    |C | 16        |                        |  C|  2.9        |1C| 74
;    |D | 10        |                        |  D|  2.4        |1D| 55
;    |E |  6        |                        |  E|  1.8        |1E| 37
;    |F |  0        |                        |  F|  1.5        |1F| 28
;    ---------------                         ---------------------------

; All the values are in 1/11025

NEWSYM EchoRate
               dd 2,172,344,517,689,861,1033,1205,1378,1550,1722,1895,
               dd 2067,2239,2412,2584

NEWSYM AttackRate
               dd 45202,28665,16537,11025,7056,4189,2866,1764,1058,705,441
               dd 264,176,110,66,4

NEWSYM DecayRate
               dd 13230,8158,4851,2697,1984,815,407,125
NEWSYM SustainRate
               dd 0FFFFFFFFh,418950,308700,265600,209475,154350,132300
               dd 103635,78277,65047,51817,38587,31972,26460,19845,16537
               dd 13230,9702,8158,6504,4851,3879,2697,1450
               dd 1212,1014,815,606,407,202,125,70

NEWSYM SustainValue
               db 15,31,47,63,79,95,111,127

NEWSYM Increase
               dd 0FFFFFFFFh,45202,34177,28665,22050,16537,14332,11025
               dd 8489,7056,5622,4189,3528,2866,2094,1764
               dd 1433,1058,882,705,529,441,352,264
               dd 220,176,132,110,88,66,44,22

NEWSYM IncreaseBent
               dd 0FFFFFFFFh,79100,59535,50160,38580,28665,25000,19250
               dd 14332,12127,9800,7320,6160,4961,3650,3060
               dd 2425,1845,1540,1212,920,770,614,460
               dd 383,306,229,190,152,113,75,36

NEWSYM Decrease
               dd 0FFFFFFFFh,45202,34177,28665,22050,16537,14332,11025
               dd 8489,7056,5622,4189,3528,2866,2094,1764
               dd 1433,1058,882,705,529,441,352,264
               dd 220,176,132,110,88,66,44,22

NEWSYM DecreaseRateExp
               dd 0FFFFFFFFh,418950,308700,264600,209470,154350,132300,103635
               dd 78277,65047,51817,38587,31972,26460,19845,16537
               dd 13230,9702,8158,6504,4851,4079,3197,2425
               dd 1984,1653,1212,1014,815,606,407,198

GainDecBendData db 118,110,102,95,89,83,77,72,67,62,58,54,50,47,44,41,38,35
                db 33,30,28,26,24,23,21,20,18,17,16,15,14,13
                db 12,11,10,9,9,8,7,7,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,2,2,1,1,1,1
                db 255

AdsrBendData db 122,118,114,110,106,102,99,95,92,89,86,83,80,77,74,72,69,67
             db 64,62,60,58,56,54,52,50,48,47,45,44,42,41,39,38,36,35,34,33
             db 32,30,29,28,27,26,25,24,24,23,22,21,20,20,19,18,18,17,16,16
             db 15,15,14,14,13,13,12,12,11,11,11,10,10,9,9,9,8,8,8,7,7,7,7,6
             db 6,6,6,5,5,5,5,5,4,4,4,4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2
             db 2,2,1,1,1,1,1,1,1,1,1,1,1,1,255

NEWSYM AdsrSustLevLoc, db 58,39,27,19,13,8,3,1

dspsave equ marksave-BRRBuffer
dspconvb equ marksave-Voice0Freq
dspsave2 equ marksave2-echoon0
NEWSYM PHdspsave, dd dspsave
NEWSYM PHdspconvb, dd dspconvb
NEWSYM PHdspsave2, dd dspsave2

SECTION .bss
spc700temp resd 2
SECTION .text

%macro VoiceStart 2
      mov byte[Voice0FirstBlock+%1],1
      mov dword[spc700temp+4],0
      cmp byte[Voice0Status+%1],0
      je .moveon
      push eax
      mov eax,[Voice0EnvInc+%1*4]
      mov [spc700temp],eax
      mov dword[spc700temp+4],1
      pop eax
.moveon
      mov byte[Voice0Status+%1],0
      push eax
      push ebx
      push edx
      mov eax,[DSPMem+%1*10h]
      cmp al,40h
      jae .noskip
      cmp ah,40h
      jae .noskip
      mov eax,[DSPMem+%1*10h+4]
      cmp eax,0050FF07h
      jne .noskip
      cmp byte[DSPMem+5Dh],6
      jne .noskip
      mov byte[DSPMem+%1*10h],15
      mov byte[DSPMem+%1*10h+1],15
      pop edx
      pop ebx
      pop eax
      ret
.noskip

      ; Check if adsr or gain
      test byte[DSPMem+05h+%1*10h],80h
      jz near .gain

      ; Calculate attack rate
      movzx eax,byte[DSPMem+05h+%1*10h]
      and al,0Fh
      cmp eax,0Fh
      je .skipattack
      mov ebx,[AttackRate+eax*4]
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      div ebx
      mov [Voice0IncNumber+%1*4],eax
      mov byte[Voice0State+%1],8
      mov dword[Voice0EnvInc+%1*4],0
      mov byte[GainDecBendDataDat+%1],7Fh
      mov byte[Voice0Status+%1],1
      jmp .finproc
.skipattack
      mov al,[DSPMem+05h+%1*10h]
      shr al,4
      and eax,07h
      mov edx,[DecayRate+eax*4]
      movzx eax,byte[DSPMem+06h+%1*10h]
      and al,1Fh
      mov ebx,[SustainRate+eax*4]
      cmp edx,ebx
      jae near .decayover

      ; ebx = total sustain time
      movzx eax,byte[DSPMem+06h+%1*10h]
      shr al,5
      mov al,[AdsrSustLevLoc+eax]
      ; traverse through al entries in edx time
      ; then through 64-al entries in ebx-edx time
      mov [AdsrBlocksLeft+%1],al
      sub ebx,edx
      push ebx
      push eax
      mov ebx,eax
      mov eax,edx
      xor edx,edx
      div ebx
      mov [Voice0Time+%1*4],eax
      mov [GainDecBendDataTime+%1*4],eax
      pop eax
      pop ebx
      mov edx,ebx
      mov ebx,64
      sub bl,al
      mov eax,edx
      xor edx,edx
      div ebx
      mov [AdsrNextTimeDepth+%1*4],eax
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      mov ebx,[Voice0Time+%1*4]
      xor edx,edx
      mov eax,127*65536
      sub eax,122*65536
      mov byte[GainDecBendDataPos+%1],0
      mov byte[GainDecBendDataDat+%1],127
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      mov byte[Voice0State+%1],9
      mov byte[Voice0Status+%1],1
      jmp .finproc
.decayover
      sub edx,ebx
      push ebx
      mov eax,edx
      movzx ebx,byte[DSPMem+06h+%1*10h]
      shr bl,5
      xor bl,07h
      mul ebx
      mov ebx,7
      div ebx
      pop ebx
      add ebx,eax
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      shr ebx,5
      mov [Voice0Time+%1*4],ebx
      mov [GainDecBendDataTime+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      sub eax,118*65536
      mov byte[GainDecBendDataPos+%1],0
      mov byte[GainDecBendDataDat+%1],127
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      mov byte[Voice0State+%1],7
      mov byte[Voice0Status+%1],1
      jmp .finproc
.gain
      test byte[DSPMem+07h+%1*10h],80h
      jz near .Direct
      test byte[DSPMem+07h+%1*10h],40h
      jnz near .Increase
      test byte[DSPMem+07h+%1*10h],20h
      jz .LinearDec
      movzx eax,byte[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[DecreaseRateExp+eax*4]
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      shr ebx,5
      mov [Voice0Time+%1*4],ebx
      mov [GainDecBendDataTime+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      sub eax,118*65536
      mov byte[GainDecBendDataPos+%1],0
      mov byte[GainDecBendDataDat+%1],127
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      mov byte[Voice0State+%1],7
      mov byte[Voice0Status+%1],1
      jmp .finproc
.LinearDec
      movzx eax,byte[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Decrease+eax*4]
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      mov byte[Voice0State+%1],5
      mov byte[Voice0Status+%1],1
      jmp .finproc
.Increase
      test byte[DSPMem+07h+%1*10h],20h
      jz .LinearInc
      movzx eax,byte[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Increase+eax*4]
      mov dword[Voice0EnvInc+%1*4],0
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      div ebx
      mov [Voice0IncNumber+%1*4],eax
      mov ebx,[Voice0Time+%1*4]
      mov eax,ebx
      shr eax,2
      sub ebx,eax
      dec ebx
      mov [Voice0Time+%1*4],ebx
      mov byte[Voice0State+%1],6
      mov byte[Voice0Status+%1],1
      jmp .finproc
.LinearInc
      movzx eax,byte[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Increase+eax*4]
      mov dword[Voice0EnvInc+%1*4],0
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      div ebx
      mov [Voice0IncNumber+%1*4],eax
      mov byte[Voice0State+%1],3
      mov byte[Voice0Status+%1],1
      jmp .finproc
.Direct
      mov al,[DSPMem+07h+%1*10h]
      and al,7Fh
      mov dword[Voice0EnvInc+%1*4],0
      mov [Voice0EnvInc+%1*4+2],al
      mov dword[Voice0Time+%1*4],0FFFFFFFFh
      mov dword[Voice0IncNumber+%1*4],0
      mov byte[Voice0State+%1],4
      mov byte[Voice0Status+%1],1
      jmp .finproc
.finproc
      cmp dword[spc700temp+4],0
      je .skipall
      mov eax,[Voice0Time+%1*4]
      mov [TimeTemp+%1*4],eax
      mov eax,[Voice0IncNumber+%1*4]
      mov [IncNTemp+%1*4],eax
      mov eax,[Voice0EnvInc+%1*4]
      mov [EnvITemp+%1*4],eax
      mov al,[Voice0State+%1]
      mov [StatTemp+%1],al
      mov eax,[spc700temp]
      mov [Voice0EnvInc+%1*4],eax
      mov dword[Voice0Time+%1*4],127
      shr eax,7
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      mov byte[Voice0State+%1],210
      jmp .novoice
.skipall
      mov ax,[DSPMem+02h+%1*10h]
      cmp word[Voice0Pitch+%1*2],ax
      je .nopitchc
      mov [Voice0Pitch+%1*2],ax
      And EAX, 03FFFh
      Mul dword[dspPAdj]
      ShRD EAX,EDX,8
      mov [Voice0Freq+%1*4],eax
      ; modpitch
.nopitchc
      mov dword[BRRPlace0+%1*8],10000000h
      mov dword[Voice0Prev0+%1*4],0
      mov dword[Voice0Prev1+%1*4],0
      mov byte[Voice0End+%1],0
      mov byte[Voice0Loop+%1],0
      mov dword[PSampleBuf+%1*24*4+16*4],0
      mov dword[PSampleBuf+%1*24*4+17*4],0
      mov dword[PSampleBuf+%1*24*4+18*4],0
      mov byte[SoundLooped0+%1],0
      mov byte[echoon0+%1],0
      test byte[DSPMem+4Dh],%2
      jz .noecho
      mov byte[echoon0+%1],1
.noecho
.novoice
      mov edx,[DSPMem+04h+%1*10h]
      and edx,0ffh
      shl edx,2
      xor eax,eax
      mov ah,[DSPMem+5Dh]
      add ax,dx
      movzx ebx,word[SPCRAM+eax]
      mov [Voice0Ptr+%1*4],ebx
      movzx ebx,word[SPCRAM+eax+2]
      mov [Voice0LoopPtr+%1*4],ebx
      pop edx
      pop ebx
      pop eax
      ret
%endmacro

NEWSYM Voice0Start
      VoiceStart 0,1
      ret

NEWSYM Voice1Start
      VoiceStart 1,2
      ret

NEWSYM Voice2Start
      VoiceStart 2,4
      ret

NEWSYM Voice3Start
      VoiceStart 3,8
      ret

NEWSYM Voice4Start
      VoiceStart 4,16
      ret

NEWSYM Voice5Start
      VoiceStart 5,32
      ret

NEWSYM Voice6Start
      VoiceStart 6,64
      ret

NEWSYM Voice7Start
      VoiceStart 7,128
      ret

;    mov byte[Voice0Env+%1],3Fh
;    jmp .Adsr
; Pass both voice # and next function name

%macro VoiceStarterM 1
      cmp al,%1
      jne near %%nope
      push edx
      mov eax,[TimeTemp+%1*4]
      mov [Voice0Time+%1*4],eax
      mov eax,[IncNTemp+%1*4]
      mov [Voice0IncNumber+%1*4],eax
      mov eax,[EnvITemp+%1*4]
      mov [Voice0EnvInc+%1*4],eax
      mov al,[StatTemp+%1]
      mov [Voice0State+%1],al

      mov byte[SoundLooped0+%1],0
      mov byte[echoon0+%1],0
      test byte[DSPMem+4Dh],1 << %1
      jz %%noecho
      mov byte[echoon0+%1],1
%%noecho
      mov edx,[DSPMem+04h+%1*10h]
      and edx,0ffh
      shl edx,2
      xor eax,eax
      mov ah,[DSPMem+5Dh]
      add ax,dx
      movzx ebx,word[SPCRAM+eax]
      mov [Voice0Ptr+%1*4],ebx
      movzx ebx,word[SPCRAM+eax+2]
      mov [Voice0LoopPtr+%1*4],ebx
      mov ax,[DSPMem+02h+%1*10h]
      cmp word[Voice0Pitch+%1*2],ax
      je %%nopitchc
      mov [Voice0Pitch+%1*2],ax
      And EAX, 03FFFh
      Mul dword[dspPAdj]
      ShRD EAX,EDX,8
      mov [Voice0Freq+%1*4],eax
      ; modpitch
%%nopitchc
      mov dword[BRRPlace0+%1*8],10000000h
      mov dword[Voice0Prev0+%1*4],0
      mov dword[Voice0Prev1+%1*4],0
      mov byte[Voice0End+%1],0
      mov byte[Voice0Loop+%1],0
      mov dword[PSampleBuf+%1*24*4+16*4],0
      mov dword[PSampleBuf+%1*24*4+17*4],0
      mov dword[PSampleBuf+%1*24*4+18*4],0
      pop edx
      ret
%%nope
%endmacro

NEWSYM VoiceStarter
   VoiceStarterM 0
   VoiceStarterM 1
   VoiceStarterM 2
   VoiceStarterM 3
   VoiceStarterM 4
   VoiceStarterM 5
   VoiceStarterM 6
   VoiceStarterM 7
   ret

section .bss
NEWSYM NoiseInc, resd 1
NEWSYM NoisePointer, resd 1
NEWSYM LastNoise, resd 1
section .text

%macro CalculatePMod 1
    movzx eax,byte[PModBuffer+esi]
    mov ebx,[Voice0Freq+%1*4]
    add al,80h
    mul ebx
    shr eax,7
    shl edx,25
    or eax,edx
    mov ebx,eax
%endmacro

%macro ProcessPMod 1
    push ecx
    push edx
    mov cl,[Voice0EnvInc+%1*4+2]
    mov ax,[edi+edx*2]
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    mov [PModBuffer+esi],ah
    pop edx
    pop ecx
%endmacro

section .bss
powhack resd 1
paramhack resd 4
section .text

ALIGN16
NEWSYM NonEchoMonoPM
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0Volume+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax
    add esi,2
    CalculatePMod ebp
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM NonEchoStereoPM
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeR+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeL+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    add esi,2
    mov cx,[VolumeConvTable+eax*2]
    pop eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4-4],eax
    CalculatePMod ebp
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM EchoMonoPM
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0Volume+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0Volumee+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0Volumee+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]
    pop eax

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*2],eax

    add esi,2
    CalculatePMod ebp
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM EchoStereoPM
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeR+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    mov ebx,eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeRe+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov cx,[ds:VolumeConvTable+eax*2]
%else
    movzx eax,byte[Voice0VolumeRe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]
%endif
    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeL+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]

    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4+4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeLe+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeLe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
%ifdef __MSDOS__
    mov ebx,[ds:Voice0Freq+ebp*4]
%else
    mov ebx,[Voice0Freq+ebp*4]
%endif
    add [EchoBuffer+esi*4+4],eax

    add esi,2
    CalculatePMod ebp
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

; interpolation is only done when the rate < mixRate, so ebx always contains
; less than 100000h.

SECTION .data
ALIGN16
NEWSYM DSPInterpolate, dd 0

SECTION .text

ALIGN16
DSPInterpolate_4
    push edi
%ifdef __MSDOS__
    lea edi,[ds:ebp*2+ebp]
%else
    lea edi,[ebp*2+ebp]
%endif
    shl edi,5
%ifdef __MSDOS__
    movzx ebx,byte[ds:BRRPlace0+ebp*8+2]
%else
    movzx ebx,byte[BRRPlace0+ebp*8+2]
%endif

    mov ax,[PSampleBuf+edx*4+8+edi]
    mov dx,[DSPInterP+ebx*2+256*6]
    imul dx
    shl edx,16
    mov dx,ax
%ifdef __MSDOS__
    mov eax,[ds:BRRPlace0+ebp*8+3]
%else
    mov eax,[BRRPlace0+ebp*8+3]
%endif
    mov ecx,edx

    mov ax,[PSampleBuf+eax*4+12+edi]
    mov dx,[DSPInterP+ebx*2+256*4]
    imul dx
    shl edx,16
    mov dx,ax
%ifdef __MSDOS__
    mov eax,[ds:BRRPlace0+ebp*8+3]
%else
    mov eax,[BRRPlace0+ebp*8+3]
%endif
    add ecx,edx

    mov ax,[PSampleBuf+eax*4+16+edi]
    mov dx,[DSPInterP+ebx*2+256*2]
    imul dx
    shl edx,16
    mov dx,ax
%ifdef __MSDOS__
    mov eax,[ds:BRRPlace0+ebp*8+3]
%else
    mov eax,[BRRPlace0+ebp*8+3]
%endif
    add ecx,edx

    mov ax,[PSampleBuf+eax*4+20+edi]
    mov dx,[DSPInterP+ebx*2]
    imul dx
    shl edx,16
    mov dx,ax
    add ecx,edx

    sar ecx,11

    cmp ecx, -32768
    jge .sat1
    mov ecx, -32768
.sat1
    cmp ecx, 32767
    jle .sat2
    mov ecx, 32767
.sat2

    mov ax,cx
    pop edi
    ret

ALIGN16
DSPInterpolate_8:

    push edi
%ifdef __MSDOS__
    lea edi,[ds:ebp*2+ebp]
%else
    lea edi,[ebp*2+ebp]
%endif
    shl edi,5
%ifdef __MSDOS__
    mov ebx,[ds:BRRPlace0+ebp*8]
    movzx eax,byte[ds:BRRPlace0+ebp*8+3]
%else
    mov ebx,[BRRPlace0+ebp*8]
    movzx eax,byte[BRRPlace0+ebp*8+3]
%endif
    shl eax,2
    and ebx,0FFFFFFh
    add ebx,1000h
    shr ebx,9
    and ebx,0FFF0h
    add ebx,fir_lut
    movq mm0,[eax+PSampleBuf+edi]
    packssdw mm0,[eax+PSampleBuf+edi+8]
    movq mm1,[eax+PSampleBuf+edi+16]
    packssdw mm1,[eax+PSampleBuf+edi+24]
    movq mm2,[ebx]
    movq mm3,[ebx+8]
    pmaddwd mm0,mm2
    pmaddwd mm1,mm3
    paddd mm0,mm1
    movq mm1,mm0
    psrlq mm0, 32
    paddd mm0, mm1
    psrad mm0, 14
    packssdw mm0, mm0
    movd eax, mm0
    movsx eax, ax
.end
    pop edi
    ret

ALIGN16
DSPInterpolate_4_mmx:

    push edi
%ifdef __MSDOS__
    lea edi,[ds:ebp*2+ebp]
%else
    lea edi,[ebp*2+ebp]
%endif
    shl edi,5
%ifdef __MSDOS__
    movzx eax,byte[ds:BRRPlace0+ebp*8+2]
%else
    movzx eax,byte[BRRPlace0+ebp*8+2]
%endif
    movq mm0,[edx*4+PSampleBuf+edi+8]
    packssdw mm0,[edx*4+PSampleBuf+edi+16]
    movq mm1,[DSPInterP+eax*8]
    pmaddwd mm0,mm1
    movq mm1, mm0
    psrlq mm0, 32
    paddd mm0, mm1
    psrad mm0, 11
    packssdw mm0, mm0
    movd eax, mm0
    emms
    pop edi
    ret

ALIGN16
NEWSYM NonEchoMonoInterpolated
%ifdef __MSDOS__
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    call [DSPInterpolate] ;ebp
;    mov edx,[BRRPlace0+ebp*8+3]
;    mov ax,[edi+edx*2]
    ; DSPInterP (Samp*i+Samp2*i2+Samp3*i3+Samp4*i3)>>11
.AfterNoise1
%ifdef __MSDOS__
    movzx edx,byte[ds:Voice0Volume+ebp]
    mov dh,[ds:Voice0EnvInc+ebp*4+2]
    mov ebx,[ds:Voice0Freq+ebp*4]
%else
    movzx edx,byte[Voice0Volume+ebp]
    mov dh,[Voice0EnvInc+ebp*4+2]
    mov ebx,[Voice0Freq+ebp*4]
%endif
    mov cx,[VolumeConvTable+edx*2]
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax
    add esi,2
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM EchoMonoInterpolated
%ifdef __MSDOS__
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    call [DSPInterpolate] ;ebp
;    mov edx,[BRRPlace0+ebp*8+3]
;    mov ax,[edi+edx*2]
.AfterNoise1
%ifdef __MSDOS__
    movzx edx,byte[ds:Voice0Volume+ebp]
    mov dh,[ds:Voice0EnvInc+ebp*4+2]
    mov ebx,[ds:Voice0Freq+ebp*4]
%else
    movzx edx,byte[Voice0Volume+ebp]
    mov dh,[Voice0EnvInc+ebp*4+2]
    mov ebx,[Voice0Freq+ebp*4]
%endif
    mov cx,[VolumeConvTable+edx*2]
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx    ; ***
%else
    add [BRRPlace0+ebp*8],ebx    ; ***
%endif
    add esi,2                   ; ***
    mov ebx,eax

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2-4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0Volumee+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0Volumee+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]

    mov eax,ebx

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*2-4],eax
    ret

ALIGN16
NEWSYM NonEchoStereoInterpolated
%ifdef __MSDOS__
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    call [DSPInterpolate]; %1
.AfterNoise1b
%ifdef __MSDOS__
    movzx edx,byte[ds:Voice0VolumeR+ebp]
    mov dh,[ds:Voice0EnvInc+ebp*4+2]
    mov ebx,[ds:Voice0Freq+ebp*4]
%else
    movzx edx,byte[Voice0VolumeR+ebp]
    mov dh,[Voice0EnvInc+ebp*4+2]
    mov ebx,[Voice0Freq+ebp*4]
%endif
    mov cx,[VolumeConvTable+edx*2]

    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeL+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    add esi,2
    mov cx,[VolumeConvTable+eax*2]
    pop eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4-4],eax
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM EchoStereoInterpolated
%ifdef __MSDOS__
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
;    mov ax,[edi+edx*2]
    call [DSPInterpolate] ;%1
.AfterNoise1b
%ifdef __MSDOS__
    movzx edx,byte[ds:Voice0VolumeR+ebp]
    mov dh,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx edx,byte[Voice0VolumeR+ebp]
    mov dh,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+edx*2]

    movsx eax,ax
    mov ebx,eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeRe+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeRe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeL+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]

    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4+4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeLe+ebp]
    mov ah,[dS:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeLe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
%ifdef __MSDOS__
    mov ebx,[ds:Voice0Freq+ebp*4]
%else
    mov ebx,[Voice0Freq+ebp*4]
%endif
    add [EchoBuffer+esi*4+4],eax

    add esi,2
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM NonEchoMono
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0Volume+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax
    add esi,2
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM NonEchoStereo
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeR+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeL+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    add esi,2
    mov cx,[VolumeConvTable+eax*2]
    pop eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4-4],eax
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM EchoMono
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0Volume+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax

%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0Volumee+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0Volumee+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]
    pop eax

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*2],eax

    add esi,2
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

ALIGN16
NEWSYM EchoStereo
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeR+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
    mov edx,[ds:BRRPlace0+ebp*8+3]
%else
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
%endif
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    mov [LastNoise],ax
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    mov ebx,eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeRe+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeRe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*4],eax

%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeL+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]

    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4+4],eax
%ifdef __MSDOS__
    movzx eax,byte[ds:Voice0VolumeLe+ebp]
    mov ah,[ds:Voice0EnvInc+ebp*4+2]
%else
    movzx eax,byte[Voice0VolumeLe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
%endif
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
%ifdef __MSDOS__
    mov ebx,[ds:Voice0Freq+ebp*4]
%else
    mov ebx,[Voice0Freq+ebp*4]
%endif
    add [EchoBuffer+esi*4+4],eax

    add esi,2
%ifdef __MSDOS__
    add [ds:BRRPlace0+ebp*8],ebx
%else
    add [BRRPlace0+ebp*8],ebx
%endif
    ret

%macro ProcessVoiceStuff 4
    mov ax,[DSPMem+02h+%1*10h]
    cmp word[Voice0Pitch+%1*2],ax
    je %%nopitchc
    mov [Voice0Pitch+%1*2],ax
    And EAX, 03FFFh
    Mul dword[dspPAdj]
    ShRD EAX,EDX,8
    ; modpitch
    mov [Voice0Freq+%1*4],eax
%%nopitchc
    jmp %%SkipProcess
%%ProcessNextEnvelope
    cmp byte[Voice0State+%1],10
    je near %%ADSRSustain
    cmp byte[Voice0State+%1],9
    je near %%ADSRDecayProc
    cmp byte[Voice0State+%1],7
    je near %%DecreaseBent
    cmp byte[Voice0State+%1],8
    je near %%ADSRDecay
    cmp byte[Voice0State+%1],1
    je near %%Decay
    cmp byte[Voice0State+%1],2
    je near %%Sustain
    cmp byte[Voice0State+%1],3
    je near %%Blank
    cmp byte[Voice0State+%1],4
    je near %%EndofSamp
    cmp byte[Voice0State+%1],200
    je near %%EndofSamp
    cmp byte[Voice0State+%1],210
    je near %%EndofSamp2
    cmp byte[Voice0State+%1],5
    je %%MuteGain
    cmp byte[Voice0State+%1],6
    je %%IncreaseBent
    jmp %%EndofSamp
%%MuteGain
    mov dword[Voice0EnvInc+%1*4],0
    mov dword[Voice0IncNumber+%1*4],0
    mov dword[Voice0Time+%1*4],0FFFFFFFFh
    jmp %%ContinueGain
%%IncreaseBent
    push ebx
    movzx eax,byte[DSPMem+07h+%1*10h]
    and al,1Fh
    mov ebx,[Increase+eax*4]
    mov [Voice0Time+%1*4],ebx
    shr dword[Voice0IncNumber+%1*4],2
    mov byte[Voice0State+%1],3
    pop ebx
    jmp %%ContinueGain
%%ADSRDecay
      push ebx
      push edx
      mov al,[DSPMem+05h+%1*10h]
      shr al,4
      and eax,07h
      mov edx,[DecayRate+eax*4]
      movzx eax,byte[DSPMem+06h+%1*10h]
      and al,1Fh
      mov ebx,[SustainRate+eax*4]
      cmp edx,ebx
      jae %%decayover
      ; ebx = total sustain time
      movzx eax,byte[DSPMem+06h+%1*10h]
      shr al,5
      mov al,[AdsrSustLevLoc+eax]
      ; traverse through al entries in edx time
      ; then through 64-al entries in ebx-edx time
      mov [AdsrBlocksLeft+%1],al
      sub ebx,edx
      push ebx
      push eax
      mov ebx,eax
      mov eax,edx
      xor edx,edx
      div ebx
      mov [Voice0Time+%1*4],eax
      mov [GainDecBendDataTime+%1*4],eax
      pop eax
      pop ebx
      mov edx,ebx
      mov ebx,64
      sub bl,al
      mov eax,edx
      xor edx,edx
      div ebx
      mov [AdsrNextTimeDepth+%1*4],eax
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      mov ebx,[Voice0Time+%1*4]
      xor edx,edx
      mov eax,127*65536-122*65536
      mov byte[GainDecBendDataPos+%1],0
      mov byte[GainDecBendDataDat+%1],127
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      mov byte[Voice0State+%1],9
    jmp %%ContinueGain
%%decayover
      sub edx,ebx
      push ebx
      mov eax,edx
      movzx ebx,byte[DSPMem+06h+%1*10h]
      shr bl,5
      xor bl,07h
      mul ebx
      mov ebx,7
      div ebx
      pop ebx
      add ebx,eax
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      shr ebx,5
      mov [Voice0Time+%1*4],ebx
      mov [GainDecBendDataTime+%1*4],ebx
      xor edx,edx
      mov eax,127*65536-118*65536
      mov byte[GainDecBendDataPos+%1],0
      mov byte[GainDecBendDataDat+%1],127
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      mov byte[Voice0State+%1],7
    jmp %%ContinueGain
%%ADSRDecayProc
    push ebx
    push edx
    mov dword[Voice0EnvInc+%1*4],0
    movzx ebx,byte[GainDecBendDataPos+%1]
    movzx edx,byte[GainDecBendDataDat+%1]
    mov dh,[AdsrBendData+ebx]
    movzx eax,byte[VolumeConvTable+edx*2]
    mov [Voice0EnvInc+%1*4+2],al
    mov dl,[GainDecBendDataDat+%1]
    mov dh,[AdsrBendData+ebx+1]
    sub al,[VolumeConvTable+edx*2]
    mov ebx,[GainDecBendDataTime+%1*4]
    mov [Voice0Time+%1*4],ebx
    xor edx,edx
    shl eax,16
    inc byte[GainDecBendDataPos+%1]
    div ebx
    neg eax
    mov [Voice0IncNumber+%1*4],eax
    pop edx
    pop ebx
    dec byte[AdsrBlocksLeft+%1]
    jz %%nomoredecay
    jmp %%ContinueGain
%%nomoredecay
    mov byte[Voice0State+%1],10
    jmp %%ContinueGain
%%ADSRSustain
    push ebx
    push edx
    mov dword[Voice0EnvInc+%1*4],0
    movzx ebx,byte[GainDecBendDataPos+%1]
    movzx edx,byte[GainDecBendDataDat+%1]
    mov dh,[AdsrBendData+ebx]
    movzx eax,byte[VolumeConvTable+edx*2]
    mov [Voice0EnvInc+%1*4+2],al
    mov dl,[GainDecBendDataDat+%1]
    mov dh,[AdsrBendData+ebx+1]
    cmp dh,255
    je %%nomoreadsr
    mov dl,[VolumeConvTable+edx*2]
    mov ebx,[AdsrNextTimeDepth+%1*4]
    sub al,dl
    mov [Voice0Time+%1*4],ebx
    xor edx,edx
    shl eax,16
    inc byte[GainDecBendDataPos+%1]
    div ebx
    neg eax
    mov [Voice0IncNumber+%1*4],eax
    pop edx
    pop ebx
    jmp %%ContinueGain
%%nomoreadsr
    pop edx
    pop ebx
    mov byte[Voice0State+%1],5
    jmp %%MuteGain
%%DecreaseBent
    push ebx
    push edx
    movzx ebx,byte[GainDecBendDataPos+%1]
    movzx edx,byte[GainDecBendDataDat+%1]
    mov dh,[GainDecBendData+ebx]
    mov dword[Voice0EnvInc+%1*4],0
    movzx eax,byte[VolumeConvTable+edx*2]
    mov [Voice0EnvInc+%1*4+2],al
    mov dh,[GainDecBendData+ebx+1]
    cmp dh,255
    je %%nomore
    mov ebx,[GainDecBendDataTime+%1*4]
    sub al,[VolumeConvTable+edx*2]
    mov [Voice0Time+%1*4],ebx
    xor edx,edx
    shl eax,16
    inc byte[GainDecBendDataPos+%1]
    div ebx
    neg eax
    mov [Voice0IncNumber+%1*4],eax
    pop edx
    pop ebx
    jmp %%ContinueGain
%%nomore
    pop edx
    pop ebx
    mov byte[Voice0State+%1],5
    jmp %%MuteGain
%%Decay
    ; Calculate Decay Value
    push ebx
    push edx
    mov dword[Voice0EnvInc+%1*4],07FFFFFh
    mov al,[DSPMem+05h+%1*10h]
    mov dl,[DSPMem+06h+%1*10h]
    shr al,4
    and eax,07h
    and edx,1Fh
    mov ebx,[DecayRate+eax*4]
    cmp edx,1Fh
    je %%nodecayfix
    cmp ebx,[SustainRate+edx*4]
    jbe %%nodecayfix
    cmp al,0
    jne %%nodecayskip
    mov al,[DSPMem+06h+%1*10h]
    and al,0E0h
    cmp al,0E0h
    je near %%Sustain2
%%nodecayskip
    mov al,[DSPMem+05h+%1*10h]
    shr al,4
    and eax,07h
    mov ebx,[DecayRate+eax*4]
    sub ebx,[SustainRate+edx*4]
    cmp ebx,[SustainRate+edx*4]
    jae %%nodecayfix
    mov ebx,[SustainRate+edx*4]
%%nodecayfix
    or ebx,ebx
    jnz %%nozero
    inc ebx
%%nozero
    mov dl,[DSPMem+06h+%1*10h]
    shr dl,5
    and dl,07h
    mov [Voice0Time+%1*4],ebx
    mov al,[SustainValue+edx]
    xor al,7Fh
    shl eax,16
    xor edx,edx
    div ebx
    neg eax
    mov [Voice0IncNumber+%1*4],eax
    mov byte[Voice0State+%1],2
    pop edx
    pop ebx
    mov ebx,[Voice0Freq+%1*4]
    cmp dword[DSPInterpolate],0
    je %%notinterpsound
    cmp byte[StereoSound],1
    je near %%EndofProcessNEnvsi
    jmp %%EndofProcessNEnvi
%%notinterpsound
    cmp byte[StereoSound],1
    je near %%EndofProcessNEnvs
    jmp %%EndofProcessNEnv
%%Sustain2
    mov ebx,[SustainRate+edx*4]
    jmp %%continuesust
%%Sustain
    ; Calculate Decay Value
    push ebx
    push edx
    mov al,[DSPMem+05h+%1*10h]
    mov dl,[DSPMem+06h+%1*10h]
    and edx,1Fh
    shr al,4
    and eax,07h
    mov ebx,[SustainRate+edx*4]
    test ebx,80000000h
    jnz %%sustainokay
    sub ebx,[DecayRate+eax*4]
%%continuesust
    cmp ebx,100
    jg %%sustainokay
    mov ebx,100
%%sustainokay
    mov [Voice0Time+%1*4],ebx
    mov al,[Voice0EnvInc+%1*4+2]
    shl eax,16
    xor edx,edx
    div ebx
    neg eax
    mov [Voice0IncNumber+%1*4],eax
    mov byte[Voice0State+%1],4
    pop edx
    pop ebx
    mov ebx,[Voice0Freq+%1*4]
    cmp dword[DSPInterpolate],0
    je %%notinterpsound2
    cmp byte[StereoSound],1
    je near %%EndofProcessNEnvsi
    jmp %%EndofProcessNEnvi
%%notinterpsound2
    cmp byte[StereoSound],1
    je near %%EndofProcessNEnvs
    jmp %%EndofProcessNEnv
%%Blank
    mov dword[Voice0EnvInc+%1*4],007F0000h
    mov dword[Voice0IncNumber+%1*4],0
    mov dword[Voice0Time+%1*4],0FFFFFFFFh
%%ContinueGain
    mov ebx,[Voice0Freq+%1*4]
    cmp dword[DSPInterpolate],0
    je %%notinterpsound3
    cmp byte[StereoSound],1
    je near %%EndofProcessNEnvsi
    jmp %%EndofProcessNEnvi
%%notinterpsound3
    cmp byte[StereoSound],1
    je near %%EndofProcessNEnvs
    jmp %%EndofProcessNEnv
%%EndofSamp
    mov dword[DLPFsamples+%1*24*4+16*4],0
    mov dword[DLPFsamples+%1*24*4+17*4],0
    mov dword[DLPFsamples+%1*24*4+18*4],0
    mov dword[DLPFsamples+%1*24*4+19*4],0
    mov dword[Voice0EnvInc+%1*4],0
    mov dword[Voice0IncNumber+%1*4],0
    mov byte[Voice0Status+%1],0
    mov byte[Voice0State+%1],0
    mov byte[DSPMem+08h+%1*10h],0
    mov byte[DSPMem+09h+%1*10h],0
    or byte[DSPMem+7Ch],%3
    jmp %2
%%EndofSamp2
    mov dword[Voice0EnvInc+%1*4],0
    mov dword[Voice0IncNumber+%1*4],0
    mov byte[Voice0State+%1],0
    mov byte[DSPMem+08h+%1*10h],0
    mov byte[DSPMem+09h+%1*10h],0
    mov al,%1
call VoiceStarter
    jmp %%SkipProcess2
%%SkipProcess
    xor esi,esi
%%SkipProcess2

    movzx eax,byte[DSPMem+00h+%1*10h]
    movzx ebx,byte[DSPMem+01h+%1*10h]
    mov al,[VolumeTableD+eax]
    mov bl,[VolumeTableD+ebx]
    mov [Voice0VolumeRe+%1],al
    mov [Voice0VolumeLe+%1],bl
    mov ah,al
    mov bh,bl
    test ah,80h
    jz %%notnegc
    neg ah
%%notnegc
    test bh,80h
    jz %%notnegd
    neg bh
%%notnegd
    add ah,bh
    shr ah,1
    test al,80h
    jnz %%neg2
    test bl,80h
    jz %%notneg2
%%neg2
    neg ah
%%notneg2
    mov [Voice0Volumee+%1],ah

    movzx eax,byte[DSPMem+00h+%1*10h]
    movzx ebx,byte[DSPMem+01h+%1*10h]
    mov al,[VolumeTableD+eax]
    mov bl,[VolumeTableD+ebx]
    mov ah,[GlobalVL]
    mov bh,[GlobalVR]
    mov al,[VolumeConvTable+eax*2]
    mov bl,[VolumeConvTable+ebx*2]
    mov [Voice0VolumeR+%1],al
    mov [Voice0VolumeL+%1],bl
    mov ah,al
    mov bh,bl
    test ah,80h
    jz %%notneg
    neg ah
%%notneg
    test bh,80h
    jz %%notnegb
    neg bh
%%notnegb
    add ah,bh
    shr ah,1
    test al,80h
    jnz %%neg3
    test bl,80h
    jz %%notneg3
%%neg3
    neg ah
%%notneg3
    mov [Voice0Volume+%1],ah

;    cmp byte[Voice0Volume+%1],0
;    je %%volskip
    mov byte[lastbl],0
    mov byte[loopbl],0
    mov ebx,[Voice0Freq+%1*4]
    mov edi,[Voice0BufPtr+%1*4]

    mov byte[UniqueSoundv],0
    test byte[DSPMem+3Dh],%3
    jnz %%Unique
%if %1<7                                        ; added
    test byte[DSPMem+2Dh],%3 << 1
    jnz %%Uniquepm
%endif                                          ; added
    jmp %%NotUnique
%%Uniquepm
%%Unique
    mov byte[UniqueSoundv],1
%%NotUnique

    cmp dword[DSPInterpolate],0
    je %%notinterpsound4
    cmp byte[StereoSound],1
    je %%NextSampleSi
    jmp %%NextSamplei
%%notinterpsound4
    cmp byte[StereoSound],1
    je near %%NextSampleS
%%NextSample
    cmp dword[BRRPlace0+%1*8],10000000h
    jae near %%ProcessBRR
    mov eax,[Voice0IncNumber+%1*4]
    add [Voice0EnvInc+%1*4],eax
    dec dword[Voice0Time+%1*4]
    jz %%ProcessNextEnvelope
%%EndofProcessNEnv
    ;%5 %1, %2, %3, %4
    call dword[paramhack];%5
    cmp esi,[BufferSizeW]
    jne %%NextSample
;    mov [DSPMem+09h+%1*10h],ah
    mov al,[Voice0EnvInc+%1*4+2]
    mov [DSPMem+08h+%1*10h],al
    cmp byte[ENVDisable],1
    jne %%skipenvclear
    mov byte[DSPMem+08h+%1*10h],0
%%skipenvclear
    jmp %2
%%NextSampleSi
    cmp dword[BRRPlace0+%1*8],10000000h
    jae near %%ProcessBRR
    mov eax,[Voice0IncNumber+%1*4]
    add [Voice0EnvInc+%1*4],eax
    dec dword[Voice0Time+%1*4]
    jz %%ProcessNextEnvelope
%%EndofProcessNEnvsi
    ;%8 %1, %2, %3, %4
    call dword[paramhack+12];%8
    cmp esi,[BufferSizeB]
    jne %%NextSampleSi
;    mov [DSPMem+09h+%1*10h],ah
    mov al,[Voice0EnvInc+%1*4+2]
    mov [DSPMem+08h+%1*10h],al
    cmp byte[ENVDisable],1
    jne %%skipenvclearsi
    mov byte[DSPMem+08h+%1*10h],0
%%skipenvclearsi
    jmp %2
%%NextSamplei
    cmp dword[BRRPlace0+%1*8],10000000h
    jae near %%ProcessBRR
    mov eax,[Voice0IncNumber+%1*4]
    add [Voice0EnvInc+%1*4],eax
    dec dword[Voice0Time+%1*4]
    jz %%ProcessNextEnvelope
%%EndofProcessNEnvi
    ;%7 %1, %2, %3, %4
    call dword[paramhack+8];%7
    cmp esi,[BufferSizeW]
    jne %%NextSamplei
;    mov [DSPMem+09h+%1*10h],ah
    mov al,[Voice0EnvInc+%1*4+2]
    mov [DSPMem+08h+%1*10h],al
    cmp byte[ENVDisable],1
    jne %%skipenvcleari
    mov byte[DSPMem+08h+%1*10h],0
%%skipenvcleari
    jmp %2
%%NextSampleS
    cmp dword[BRRPlace0+%1*8],10000000h
    jae %%ProcessBRR
    mov eax,[Voice0IncNumber+%1*4]
    add [Voice0EnvInc+%1*4],eax
    dec dword[Voice0Time+%1*4]
    jz %%ProcessNextEnvelope
%%EndofProcessNEnvs
    ;%6 %1, %2, %3, %4
    call dword[paramhack+4];%6
    cmp esi,[BufferSizeB]
    jne %%NextSampleS
;    mov [DSPMem+09h+%1*10h],ah
    mov al,[Voice0EnvInc+%1*4+2]
    mov [DSPMem+08h+%1*10h],al
    cmp byte[ENVDisable],1
    jne %%skipenvclear2
    mov byte[DSPMem+08h+%1*10h],0
%%skipenvclear2
%%noclearenv
    jmp %2
%%ProcessBRR
    cmp byte[Voice0End+%1],1
    je near %%noDecode1Block
%%Decode1Block
    sub dword[BRRPlace0+%1*8],10000000h
    push esi
    mov esi, [Voice0Ptr+%1*4]
;    cmp byte[Voice0Looped+%1],0
;    je %%nobrrcheck

    mov eax,[PSampleBuf+16*4+%1*24*4]
    mov [PSampleBuf+0*4+%1*24*4],eax
    mov eax,[PSampleBuf+17*4+%1*24*4]
    mov [PSampleBuf+1*4+%1*24*4],eax
    mov eax,[PSampleBuf+18*4+%1*24*4]
    mov [PSampleBuf+2*4+%1*24*4],eax

    mov edi,esi
    inc edi
    shl edi,2
    add esi,SPCRAM
    add edi,[spcBuffera]
    mov eax,[Voice0Prev0+%1*4]
    mov [Voice0BufPtr+%1*4],edi
    mov [prev0],eax
    mov eax,[Voice0Prev1+%1*4]
    mov [prev1],eax
    mov ecx,%1
    push ebp
    call BRRDecode
    pop ebp
    pop esi
    mov edi,[Voice0BufPtr+%1*4]

    movsx eax,word[edi]
    mov [PSampleBuf+3*4+%1*24*4],eax
    movsx eax,word[edi+2*1]
    mov [PSampleBuf+4*4+%1*24*4],eax
    movsx eax,word[edi+2*2]
    mov [PSampleBuf+5*4+%1*24*4],eax
    movsx eax,word[edi+2*3]
    mov [PSampleBuf+6*4+%1*24*4],eax
    movsx eax,word[edi+2*4]
    mov [PSampleBuf+7*4+%1*24*4],eax
    movsx eax,word[edi+2*5]
    mov [PSampleBuf+8*4+%1*24*4],eax
    movsx eax,word[edi+2*6]
    mov [PSampleBuf+9*4+%1*24*4],eax
    movsx eax,word[edi+2*7]
    mov [PSampleBuf+10*4+%1*24*4],eax
    movsx eax,word[edi+2*8]
    mov [PSampleBuf+11*4+%1*24*4],eax
    movsx eax,word[edi+2*9]
    mov [PSampleBuf+12*4+%1*24*4],eax
    movsx eax,word[edi+2*10]
    mov [PSampleBuf+13*4+%1*24*4],eax
    movsx eax,word[edi+2*11]
    mov [PSampleBuf+14*4+%1*24*4],eax
    movsx eax,word[edi+2*12]
    mov [PSampleBuf+15*4+%1*24*4],eax
    movsx eax,word[edi+2*13]
    mov [PSampleBuf+16*4+%1*24*4],eax
    movsx eax,word[edi+2*14]
    mov [PSampleBuf+17*4+%1*24*4],eax
    movsx eax,word[edi+2*15]
    mov [PSampleBuf+18*4+%1*24*4],eax

    movsx eax,word[BRRreadahead]
    mov [PSampleBuf+19*4+%1*24*4],eax
    movsx eax,word[BRRreadahead+2]
    mov [PSampleBuf+20*4+%1*24*4],eax
    movsx eax,word[BRRreadahead+4]
    mov [PSampleBuf+21*4+%1*24*4],eax
    movsx eax,word[BRRreadahead+6]
    mov [PSampleBuf+22*4+%1*24*4],eax

    mov eax,[prev0]
    mov [Voice0Prev0+%1*4],eax
    mov eax,[prev1]
    mov [Voice0Prev1+%1*4],eax
    mov al,[loopbl]
    mov [Voice0Loop+%1],al
    mov al,[lastbl]
    mov [Voice0End+%1],al
    mov ebx,[Voice0Freq+%1*4]
    add dword[Voice0Ptr+%1*4],9
    cmp dword[DSPInterpolate],0
    je %%notinterpsound6
    cmp byte[StereoSound],1
    je near %%NextSampleSi
    jmp %%NextSamplei
%%notinterpsound6
    cmp byte[StereoSound],1
    je %%NextSampleS
    jmp %%NextSample
%%noDecode1Block
;    and byte[DSPMem+5Ch],%4
;    and byte[DSPMem+4Ch],%4
;    mov byte[Voice0Looped+%1],0
    cmp byte[Voice0Loop+%1],1
    jne %%EndSample
;    mov byte[Voice0Looped+%1],1
    mov byte[SoundLooped0+%1],1
    or byte[DSPMem+7Ch],%3
%%SkipStuff3
;    mov dword[Voice0Prev0+%1*4],0
;    mov dword[Voice0Prev1+%1*4],0

;      push eax
;      push edx
;      push ebx
;      mov edx,[DSPMem+04h+%1*10h]
;      and edx,0ffh
;      shl edx,2
;      xor eax,eax
;      mov ah,[DSPMem+5Dh]
;      add ax,dx
;      xor ebx,ebx
;      mov bx,[SPCRAM+eax]
;      mov dword[Voice0Ptr+%1*4],ebx
;      xor ebx,ebx
;      mov bx,[SPCRAM+eax+2]
;      mov dword[Voice0LoopPtr+%1*4],ebx
;      pop ebx
;      pop edx
;      pop eax

    mov eax,[Voice0LoopPtr+%1*4]
    mov [Voice0Ptr+%1*4],eax
;    mov eax,[Voice0Prev0+%1*4]
;    mov [Voice0Prev1+%1*4],eax
    jmp %%Decode1Block
%%EndSample
;    cmp byte[ENVDisable],1
;    je %%noSkipStuff4
%%noSkipStuff4
    or byte[DSPMem+7Ch],%3
    mov byte[DSPMem+08h+%1*10h],0
%%SkipStuff4
    mov dword[DLPFsamples+%1*24*4+16*4],0
    mov dword[DLPFsamples+%1*24*4+17*4],0
    mov dword[DLPFsamples+%1*24*4+18*4],0
    mov dword[DLPFsamples+%1*24*4+19*4],0
;    and byte[DSPMem+5Ch],%4
    mov dword[Voice0EnvInc+%1*4],0
    mov dword[Voice0IncNumber+%1*4],0
    mov byte[Voice0Status+%1],0
;    mov byte[DSPMem+09h+%1*10h],0h
;    jmp %2
;%%ProcessVoice1
;    jmp %2
%endmacro

%macro ProcessVoiceHandler16 4
    cmp byte[Voice0Disable+%1],1
    jne near %2
    cmp byte[Voice0Status+%1],1
    jne near %2
    mov ebp,%1
    mov dword[powhack],1<<%1
    mov eax,%1
    dec al
    cmp al,0FFh
    je .nopitchmod

    cmp byte[Voice0Disable+eax],1
    jne .nopitchmod
    cmp byte[Voice0Status+eax],1
    jne .nopitchmod

    test byte[DSPMem+2Dh],%3
    jnz .pitchmod
.nopitchmod
    test byte[DSPMem+3Dh],%3
    jnz .NoEcho
    cmp byte[echoon0+%1],1
    je .echostuff
.NoEcho
    mov dword[paramhack],NonEchoMono
    mov dword[paramhack+4],NonEchoStereo
    mov dword[paramhack+8],NonEchoMonoInterpolated
    mov dword[paramhack+12],NonEchoStereoInterpolated
    jmp .pvs
    ;ProcessVoiceStuff %1, %2, %3, %4, NonEchoMono, NonEchoStereo, NonEchoMonoInterpolated, NonEchoStereoInterpolated
    ; Process Echo
.echostuff
    mov dword[paramhack],EchoMono
    mov dword[paramhack+4],EchoStereo
    mov dword[paramhack+8],EchoMonoInterpolated
    mov dword[paramhack+12],EchoStereoInterpolated
    jmp .pvs
    ;ProcessVoiceStuff %1, %2, %3, %4, EchoMono, EchoStereo, EchoMonoInterpolated, EchoStereoInterpolated
.pitchmod
    mov al,[DSPMem+4+%1*10h]
    cmp al,[DSPMem+4+%1*10h-10h]
    je .nopitchmod
    test byte[DSPMem+3Dh],%3
    jnz .NoEchopm
    cmp byte[echoon0+%1],1
    je .echopm
.NoEchopm
    mov dword[paramhack],NonEchoMonoPM
    mov dword[paramhack+4],NonEchoStereoPM
    mov dword[paramhack+8],NonEchoMonoPM
    mov dword[paramhack+12],NonEchoStereoPM
    jmp .pvs
    ;ProcessVoiceStuff %1, %2, %3, %4, NonEchoMonoPM, NonEchoStereoPM, NonEchoMonoPM, NonEchoStereoPM
.echopm
    mov dword[paramhack],EchoMonoPM
    mov dword[paramhack+4],EchoStereoPM
    mov dword[paramhack+8],EchoMonoPM
    mov dword[paramhack+12],EchoStereoPM

    ;ProcessVoiceStuff %1, %2, %3, %4, EchoMonoPM, EchoStereoPM, EchoMonoPM, EchoStereoPM
.pvs
    ProcessVoiceStuff %1, %2, %3, %4
%endmacro

%macro FiltTapProc 1
    sub edx,2
    mov eax,[FIRTAPVal0+%1*4]
    and edx,0Fh
    mov ebx,[FiltLoop+edx*4]
    imul eax,ebx
    sar eax,7
    add ecx,eax
%endmacro

%macro FiltTapProcR 1
    sub edx,2
    mov eax,[FIRTAPVal0+%1*4]
    and edx,0Fh
    mov ebx,[FiltLoopR+edx*4]
    imul eax,ebx
    sar eax,7
    add ecx,eax
%endmacro

%macro MixEcho 0
    mov al,[EchoVL]
    mov bl,[EchoVR]
    cmp bl,al
    ja .novol
    mov bl,al
.novol
    mov [EchoT],bl

    ; Copy echobuf to DSPBuffer, EchoBuffer to echobuf
    cmp byte[StereoSound],1
    je near .Stereo
    mov esi,[CEchoPtr]
    xor edi,edi
.next
    ; Get current echo buffer
    mov ebx,[echobuf+esi*4]
    ; Process FIR Filter
    mov edx,[CurFiltPtr]
    mov eax,ebx
    mov [FiltLoop+edx*4],ebx
    mov ecx,[FIRTAPVal0]
    imul eax,ecx
    sar eax,7
    mov ecx,eax
    FiltTapProc 1
    FiltTapProc 2
    FiltTapProc 3
    FiltTapProc 4
    FiltTapProc 5
    FiltTapProc 6
    FiltTapProc 7
    inc dword[CurFiltPtr]
    and byte[CurFiltPtr],0Fh
    ; Set feedback on previous echo
    mov eax,[EchoFB]
    imul eax,ecx
    sar eax,7
    ; Add in new echo/Store into Echo Buffer
    mov ecx,eax
    xor ebx,ebx
    add [DSPBuffer+edi*4],ecx
    mov eax,[EchoBuffer+edi*4]
    mov bl,[EchoT]
    mul ebx
    sar eax,7
    add eax,ecx
    mov [echobuf+esi*4],eax
    inc esi
    cmp esi,[MaxEcho]
    jb .nexte
    xor esi,esi
.nexte
    inc edi
    cmp edi,[BufferSizeB]
    jne .next
    mov [CEchoPtr],esi
    jmp .Mono

.Stereo
    mov esi,[CEchoPtr]
    xor edi,edi
.nexts
    ; Get current echo buffer
    mov ebx,[echobuf+esi*4]
    ; Process FIR Filter
    mov edx,[CurFiltPtr]
    mov eax,ebx
    mov [FiltLoop+edx*4],ebx
    mov ecx,[FIRTAPVal0]
    imul eax,ecx
    sar eax,7
    mov ecx,eax
    FiltTapProc 1
    FiltTapProc 2
    FiltTapProc 3
    FiltTapProc 4
    FiltTapProc 5
    FiltTapProc 6
    FiltTapProc 7
    add [DSPBuffer+edi*4],ecx
    ; Set feedback on previous echo
    mov eax,[EchoFB]
    imul eax,ecx
    sar eax,7

    ; Add in new echo/Store into Echo Buffer

    mov ecx,eax
    mov eax,[EchoBuffer+edi*4]
    movzx ebx,byte[EchoVL]
    mul ebx
    sar eax,7
    add eax,ecx

    mov [echobuf+esi*4],eax
    inc esi
    inc edi

    ; Get current echo buffer
    mov ebx,[echobuf+esi*4]
    ; Process FIR Filter
    mov edx,[CurFiltPtr]
    mov eax,ebx
    mov [FiltLoopR+edx*4],ebx
    mov ecx,[FIRTAPVal0]
    imul eax,ecx
    sar eax,7
    mov ecx,eax
    FiltTapProcR 1
    FiltTapProcR 2
    FiltTapProcR 3
    FiltTapProcR 4
    FiltTapProcR 5
    FiltTapProcR 6
    FiltTapProcR 7
    add [DSPBuffer+edi*4],ecx
    inc dword[CurFiltPtr]
    and byte[CurFiltPtr],0Fh
    ; Set feedback on previous echo
    mov eax,[EchoFB]
    imul eax,ecx
    sar eax,7
    ; Add in new echo/Store into Echo Buffer
    mov ecx,eax
    mov eax,[EchoBuffer+edi*4]
    movzx ebx,byte[EchoVR]
    mul ebx
    sar eax,7
    add eax,ecx
    mov [echobuf+esi*4],eax

    mov eax,[MaxEcho]
    inc esi
    shl eax,1
    cmp esi,eax
    jb .nextes
    xor esi,esi
.nextes
    inc edi
    cmp edi,[BufferSizeB]
    jne .nexts
    mov [CEchoPtr],esi
.Mono
%endmacro

%macro MixEcho2 0
    mov al,[EchoVL]
    mov bl,[EchoVR]
    cmp bl,al
    ja %%novol
    mov bl,al
%%novol
    mov [EchoT],bl

    ; Copy echobuf to DSPBuffer, EchoBuffer to echobuf
    cmp byte[StereoSound],1
    je %%Stereo
    mov esi,[CEchoPtr]
    xor edi,edi
%%next
    ; Get current echo buffer
    mov ebx,[echobuf+esi*4]
    mov eax,[EchoFB]
    add [DSPBuffer+edi*4],ebx
    imul eax,ebx
    sar eax,7
    ; Add in new echo/Store into Echo Buffer
    mov ecx,eax
    mov eax,[EchoBuffer+edi*4]
    movzx ebx,byte[EchoT]
    mul ebx
    sar eax,7
    add eax,ecx
    mov [echobuf+esi*4],eax
    inc esi
    cmp esi,[MaxEcho]
    jae %%echowrap
%%nexte
    inc edi
    cmp edi,[BufferSizeB]
    jne %%next
    mov [CEchoPtr],esi
    jmp %%Mono
%%echowrap
    xor esi,esi
    jmp %%nexte

%%Stereo
    mov esi,[CEchoPtr]
    xor edi,edi
%%nexts
    ; Get current echo buffer
    mov ecx,[echobuf+esi*4]
    mov eax,[EchoFB]
    add [DSPBuffer+edi*4],ecx
    imul eax,ecx
    sar eax,7
    ; Add in new echo/Store into Echo Buffer
    mov ecx,eax
    mov eax,[EchoBuffer+edi*4]
    movzx ebx,byte[EchoVL]
    mul ebx
    sar eax,7
    add eax,ecx
    mov [echobuf+esi*4],eax
    inc esi
    inc edi

    ; Get current echo buffer
    mov ecx,[echobuf+esi*4]
    mov eax,[EchoFB]
    add [DSPBuffer+edi*4],ecx
    imul eax,ecx
    sar eax,7
    ; Add in new echo/Store into Echo Buffer
    mov ecx,eax
    mov eax,[EchoBuffer+edi*4]
    movzx ebx,byte[EchoVR]
    mul ebx
    sar eax,7
    add eax,ecx
    mov [echobuf+esi*4],eax

    mov eax,[MaxEcho]
    inc esi
    shl eax,1
    cmp esi,eax
    jae %%echowrap2
%%nextes
    inc edi
    cmp edi,[BufferSizeB]
    jne %%nexts
    mov [CEchoPtr],esi
    jmp %%Mono
%%echowrap2
    xor esi,esi
    jmp %%nextes
%%Mono
%endmacro

section .bss
echowrittento resb 1
section .text

NEWSYM ProcessSoundBuffer
    ; Clear the DSP Buffer
    mov edi,DSPBuffer
    mov ax,ds
    mov es,ax
    xor eax,eax
    mov ecx,[BufferSizeB]
    rep stosd

    ; Clear Echo Buffer
    cmp byte[EchoDis],1
    je .nowriteecho
    test byte[DSPMem+6Ch],20h
    jnz .nowriteecho
    mov edi,EchoBuffer
    mov ecx,[BufferSizeB]
    rep stosd
.nowriteecho

    cmp byte[EMUPause],1
    jne .nopause
    ret
.nopause

    ;When logging is enabled but don't want logging this pass, return
    cmp byte[AudioLogging],1
    jne .rawdump
    ret
.rawdump

    ; Process the sound :I

    ProcessVoiceHandler16 0,ProcessVoice116,1,254
NEWSYM ProcessVoice116
    ProcessVoiceHandler16 1,ProcessVoice216,2,253
NEWSYM ProcessVoice216
    ProcessVoiceHandler16 2,ProcessVoice316,4,251
NEWSYM ProcessVoice316
    ProcessVoiceHandler16 3,ProcessVoice416,8,247
NEWSYM ProcessVoice416
    ProcessVoiceHandler16 4,ProcessVoice516,16,239
NEWSYM ProcessVoice516
    ProcessVoiceHandler16 5,ProcessVoice616,32,223
NEWSYM ProcessVoice616
    ProcessVoiceHandler16 6,ProcessVoice716,64,191
NEWSYM ProcessVoice716
    ProcessVoiceHandler16 7,ProcessVoice816,128,127
NEWSYM ProcessVoice816

    cmp byte[EchoDis],1
    je near .echowritten
    test byte[DSPMem+6Ch],20h
    jnz near .nowriteecho2
    mov byte[echowrittento],1
    ; Mix Echo with DSP Buffer
    cmp dword[FIRTAPVal0],7Fh
    jne near .echonotokay
    cmp dword[FIRTAPVal1],0
    jne near .echonotokay
    cmp dword[FIRTAPVal2],0
    jne near .echonotokay
    cmp dword[FIRTAPVal3],0
    jne near .echonotokay
    cmp dword[FIRTAPVal4],0
    jne near .echonotokay
    cmp dword[FIRTAPVal5],0
    jne near .echonotokay
    cmp dword[FIRTAPVal6],0
    jne near .echonotokay
    cmp dword[FIRTAPVal7],0
    jne near .echonotokay
    MixEcho2
    jmp .echowritten
.echonotokay
    MixEcho
    jmp .echowritten
.nowriteecho2
    cmp byte[echowrittento],0
    je .echowritten
    mov edi,echobuf
    mov ecx,[MaxEcho]
    cmp byte[StereoSound],1
    jne .noechostereo
    add ecx,ecx
.noechostereo
    xor eax,eax
    rep stosd
    mov byte[echowrittento],0
.echowritten

    cmp byte[RevStereo],0
    je .norevstereo
    mov edi,DSPBuffer
    mov ax,ds
    mov es,ax
    xor eax,eax
    mov ecx,[BufferSizeB]
    shr ecx,1
.revstloop
    mov eax,[edi]
    mov ebx,[edi+4]
    mov [edi],ebx
    mov [edi+4],eax
    add edi,8
    dec ecx
    jnz .revstloop
.norevstereo

    cmp byte[LowPassFilterType],1
    jne near LPFexit
    mov esi,DSPBuffer
    cmp byte[StereoSound],1
    jz  LPFstereo
    mov ecx, [BufferSizeB]
    shr ecx,1
    mov ebx,[LPFsample1]
NEWSYM LPFmonoloop
    mov eax,[esi]
    sar eax,1
    add ebx,eax
    mov [esi],ebx
    add esi,4
    mov ebx,[esi]
    sar ebx,1
    add eax,ebx
    mov [esi],eax
    add esi,4
    dec ecx
    jnz LPFmonoloop
    mov [LPFsample1],ebx
    jmp LPFexit

NEWSYM LPFstereo
    mov ecx, [BufferSizeB]
    shr ecx,2
;    mov ecx, <------------------- # of samples to mix / 4
    mov ebx,[LPFsample1]
    mov edx,[LPFsample2]

NEWSYM LPFstereoloop
    push ecx
    mov eax,[esi]
    sar eax,1
    add ebx,eax
    mov [esi],ebx
    mov ecx,[esi+4]
    sar ecx,1
    add edx,ecx
    mov [esi+4],edx
    add esi,8
    mov ebx,[esi]
    sar ebx,1
    add eax,ebx
    mov [esi],eax
    mov edx,[esi+4]
    sar edx,1
    add ecx,edx
    mov [esi+4],ecx
    add esi,8
    pop ecx
    dec ecx
    jnz LPFstereoloop
    mov [LPFsample1],ebx
    mov [LPFsample2],edx

NEWSYM LPFexit
    cmp byte[Surround],1
    jnz .nosurround
    cmp byte[StereoSound],1
    jnz .nosurround
    mov esi,DSPBuffer
    mov ecx,[BufferSizeB]
    shr ecx,1
.loop
    mov eax,[esi]
    mov edx,[esi+4]
    add edx,eax
    sar edx,1
    sub eax,edx
    mov ebx,[esi+4]
    sub [esi+4],eax
    sub ebx,edx
    sub [esi],ebx
    add esi,8
    dec ecx
    jnz .loop
.nosurround
    ret
