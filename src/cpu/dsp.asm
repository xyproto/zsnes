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

EXTSYM Decrease,DecreaseRateExp,EchoFB,EchoRate,EchoVL,EchoVR
EXTSYM FIRTAPVal0,FIRTAPVal1,FIRTAPVal2,FIRTAPVal3,FIRTAPVal4,FIRTAPVal5
EXTSYM FIRTAPVal6,FIRTAPVal7,GlobalVL,GlobalVR,Increase
EXTSYM MaxEcho,MusicVol,NoiseInc,NoiseSpeeds,dspPAdj,Voice0EnvInc
EXTSYM Voice0IncNumber,Voice0State,Voice0Time
EXTSYM Voice0Start,Voice1Start,Voice2Start,Voice3Start
EXTSYM Voice4Start,Voice5Start,Voice6Start,Voice7Start
EXTSYM Voice0Noise,Voice1Noise,Voice2Noise,Voice3Noise
EXTSYM Voice4Noise,Voice5Noise,Voice6Noise,Voice7Noise
EXTSYM VolumeConvTable,VolumeTableb
EXTSYM Voice0Status,Voice1Status,Voice2Status,Voice3Status
EXTSYM Voice4Status,Voice5Status,Voice6Status,Voice7Status
EXTSYM GainDecBendDataPos,GainDecBendDataTime,GainDecBendDataDat
EXTSYM AdsrSustLevLoc,AdsrBlocksLeft,AdsrNextTimeDepth
EXTSYM MuteVoiceF,VoiceStarter,DecayRate,SustainRate
EXTSYM KeyOnStA,KeyOnStB,SoundTest,keyonsn

SECTION .data
identcode db 255,1,78,78
SECTION .text

; Digital Sound Processor of the SPC700 By _Demo_

%macro initrevsthelp 1
    mov al,[DSPMem+01h+%1]
    mov ah,[DSPMem+00h+%1]
    mov [DSPMem+01h+%1],ah
    mov [DSPMem+00h+%1],al
%endmacro

SECTION .data
ALIGN32
NEWSYM DSPMem, times 256 db 0
SECTION .text

;Read DSP Registers functions

NEWSYM RDSPReg00       ; Voice  0  Volume Left
      mov al,[DSPMem+00h]
      ret

NEWSYM RDSPReg01       ; Voice  0  Volume Right
      mov al,[DSPMem+01h]
      ret

NEWSYM RDSPReg02       ; Voice  0  Pitch Low
      mov al,[DSPMem+02h]
      ret

NEWSYM RDSPReg03       ; Voice  0  Pitch High
      mov al,[DSPMem+03h]
      ret

NEWSYM RDSPReg04       ; Voice  0  SCRN
      mov al,[DSPMem+04h]
      ret

NEWSYM RDSPReg05       ; Voice  0  ADSR (1)
      mov al,[DSPMem+05h]
      ret

NEWSYM RDSPReg06       ; Voice  0  ADSR (2)
      mov al,[DSPMem+06h]
      ret

NEWSYM RDSPReg07       ; Voice  0  GAIN
      mov al,[DSPMem+07h]
      ret

NEWSYM RDSPReg08       ; Voice  0  ENVX
      mov al,[DSPMem+08h]
      and al,7Fh
      ret

NEWSYM RDSPReg09       ; Voice  0  OUTX
      mov al,[DSPMem+09h]
      ret

NEWSYM RDSPReg0A       ;
      mov al,[DSPMem+0Ah]
      ret

NEWSYM RDSPReg0B       ;
      mov al,[DSPMem+0Bh]
      ret

NEWSYM RDSPReg0C       ;
      mov al,[DSPMem+0Ch]
      ret

NEWSYM RDSPReg0D       ;
      mov al,[DSPMem+0Dh]
      ret

NEWSYM RDSPReg0E       ;
      mov al,[DSPMem+0Eh]
      ret

NEWSYM RDSPReg0F       ; Voice  0  Echo coefficient
      mov al,[DSPMem+0Fh]
      ret

NEWSYM RDSPReg10       ; Voice  1  Volume Left
      mov al,[DSPMem+10h]
      ret

NEWSYM RDSPReg11       ; Voice  1  Volume Right
      mov al,[DSPMem+11h]
      ret

NEWSYM RDSPReg12       ; Voice  1  Pitch Low
      mov al,[DSPMem+012h]
      ret

NEWSYM RDSPReg13       ; Voice  1  Pitch High
      mov al,[DSPMem+013h]
      ret

NEWSYM RDSPReg14       ; Voice  1  SCRN
      mov al,[DSPMem+014h]
      ret

NEWSYM RDSPReg15       ; Voice  1  ADSR (1)
      mov al,[DSPMem+015h]
      ret

NEWSYM RDSPReg16       ; Voice  1  ADSR (2)
      mov al,[DSPMem+016h]
      ret

NEWSYM RDSPReg17       ; Voice  1  GAIN
      mov al,[DSPMem+017h]
      ret

NEWSYM RDSPReg18       ; Voice  1  ENVX
      mov al,[DSPMem+018h]
      and al,7Fh
      ret

NEWSYM RDSPReg19       ; Voice  1  OUTX
      mov al,[DSPMem+019h]
      ret

NEWSYM RDSPReg1A       ;
      mov al,[DSPMem+01Ah]
      ret

NEWSYM RDSPReg1B       ;
      mov al,[DSPMem+01Bh]
      ret

NEWSYM RDSPReg1C       ;
      mov al,[DSPMem+01Ch]
      ret

NEWSYM RDSPReg1D       ;
      mov al,[DSPMem+01Dh]
      ret

NEWSYM RDSPReg1E       ;
      mov al,[DSPMem+01Eh]
      ret

NEWSYM RDSPReg1F       ; Voice  1  Echo coefficient
      mov al,[DSPMem+01Fh]
      ret

NEWSYM RDSPReg20       ; Voice  2  Volume Left
      mov al,[DSPMem+20h]
      ret

NEWSYM RDSPReg21       ; Voice  2  Volume Right
      mov al,[DSPMem+21h]
      ret

NEWSYM RDSPReg22       ; Voice  2  Pitch Low
      mov al,[DSPMem+022h]
      ret

NEWSYM RDSPReg23       ; Voice  2  Pitch High
      mov al,[DSPMem+023h]
      ret

NEWSYM RDSPReg24       ; Voice  2  SCRN
      mov al,[DSPMem+024h]
      ret

NEWSYM RDSPReg25       ; Voice  2  ADSR (1)
      mov al,[DSPMem+025h]
      ret

NEWSYM RDSPReg26       ; Voice  2  ADSR (2)
      mov al,[DSPMem+026h]
      ret

NEWSYM RDSPReg27       ; Voice  2  GAIN
      mov al,[DSPMem+027h]
      ret

NEWSYM RDSPReg28       ; Voice  2  ENVX
      mov al,[DSPMem+028h]
      and al,7Fh
      ret

NEWSYM RDSPReg29       ; Voice  2  OUTX
      mov al,[DSPMem+029h]
      ret

NEWSYM RDSPReg2A       ;
      mov al,[DSPMem+02Ah]
      ret

NEWSYM RDSPReg2B       ;
      mov al,[DSPMem+02Bh]
      ret

NEWSYM RDSPReg2C       ;
      mov al,[DSPMem+02Ch]
      ret

NEWSYM RDSPReg2D       ;
      mov al,[DSPMem+02Dh]
      ret

NEWSYM RDSPReg2E       ;
      mov al,[DSPMem+02Eh]
      ret

NEWSYM RDSPReg2F       ; Voice  2  Echo coefficient
      mov al,[DSPMem+02Fh]
      ret

NEWSYM RDSPReg30       ; Voice  3  Volume Left
      mov al,[DSPMem+30h]
      ret

NEWSYM RDSPReg31       ; Voice  3  Volume Right
      mov al,[DSPMem+31h]
      ret

NEWSYM RDSPReg32       ; Voice  3  Pitch Low
      mov al,[DSPMem+032h]
      ret

NEWSYM RDSPReg33       ; Voice  3  Pitch High
      mov al,[DSPMem+033h]
      ret

NEWSYM RDSPReg34       ; Voice  3  SCRN
      mov al,[DSPMem+034h]
      ret

NEWSYM RDSPReg35       ; Voice  3  ADSR (1)
      mov al,[DSPMem+035h]
      ret

NEWSYM RDSPReg36       ; Voice  3  ADSR (2)
      mov al,[DSPMem+036h]
      ret

NEWSYM RDSPReg37       ; Voice  3  GAIN
      mov al,[DSPMem+037h]
      ret

NEWSYM RDSPReg38       ; Voice  3  ENVX
      mov al,[DSPMem+038h]
      and al,7Fh
      ret

NEWSYM RDSPReg39       ; Voice  3  OUTX
      mov al,[DSPMem+039h]
      ret

NEWSYM RDSPReg3A       ;
      mov al,[DSPMem+03Ah]
      ret

NEWSYM RDSPReg3B       ;
      mov al,[DSPMem+03Bh]
      ret

NEWSYM RDSPReg3C       ;
      mov al,[DSPMem+03Ch]
      ret

NEWSYM RDSPReg3D       ;
      mov al,[DSPMem+03Dh]
      ret

NEWSYM RDSPReg3E       ;
      mov al,[DSPMem+03Eh]
      ret

NEWSYM RDSPReg3F       ; Voice  3  Echo coefficient
      mov al,[DSPMem+03Fh]
      ret

NEWSYM RDSPReg40       ; Voice  4  Volume Left
      mov al,[DSPMem+40h]
      ret

NEWSYM RDSPReg41       ; Voice  4  Volume Right
      mov al,[DSPMem+41h]
      ret

NEWSYM RDSPReg42       ; Voice  4  Pitch Low
      mov al,[DSPMem+042h]
      ret

NEWSYM RDSPReg43       ; Voice  4  Pitch High
      mov al,[DSPMem+043h]
      ret

NEWSYM RDSPReg44       ; Voice  4  SCRN
      mov al,[DSPMem+044h]
      ret

NEWSYM RDSPReg45       ; Voice  4  ADSR (1)
      mov al,[DSPMem+045h]
      ret

NEWSYM RDSPReg46       ; Voice  4  ADSR (2)
      mov al,[DSPMem+046h]
      ret

NEWSYM RDSPReg47       ; Voice  4  GAIN
      mov al,[DSPMem+047h]
      ret

NEWSYM RDSPReg48       ; Voice  4  ENVX
      mov al,[DSPMem+048h]
      and al,7Fh
      ret

NEWSYM RDSPReg49       ; Voice  4  OUTX
      mov al,[DSPMem+049h]
      ret

NEWSYM RDSPReg4A       ;
      mov al,[DSPMem+04Ah]
      ret

NEWSYM RDSPReg4B       ;
      mov al,[DSPMem+04Bh]
      ret

NEWSYM RDSPReg4C       ;
      mov al,[DSPMem+04Ch]
      ret

NEWSYM RDSPReg4D       ;
      mov al,[DSPMem+04Dh]
      ret

NEWSYM RDSPReg4E       ;
      mov al,[DSPMem+04Eh]
      ret

NEWSYM RDSPReg4F       ; Voice  4  Echo coefficient
      mov al,[DSPMem+04Fh]
      ret

NEWSYM RDSPReg50       ; Voice  5  Volume Left
      mov al,[DSPMem+50h]
      ret

NEWSYM RDSPReg51       ; Voice  5  Volume Right
      mov al,[DSPMem+51h]
      ret

NEWSYM RDSPReg52       ; Voice  5  Pitch Low
      mov al,[DSPMem+052h]
      ret

NEWSYM RDSPReg53       ; Voice  5  Pitch High
      mov al,[DSPMem+053h]
      ret

NEWSYM RDSPReg54       ; Voice  5  SCRN
      mov al,[DSPMem+054h]
      ret

NEWSYM RDSPReg55       ; Voice  5  ADSR (1)
      mov al,[DSPMem+055h]
      ret

NEWSYM RDSPReg56       ; Voice  5  ADSR (2)
      mov al,[DSPMem+056h]
      ret

NEWSYM RDSPReg57       ; Voice  5  GAIN
      mov al,[DSPMem+057h]
      ret

NEWSYM RDSPReg58       ; Voice  5  ENVX
      mov al,[DSPMem+058h]
      and al,7Fh
      ret

NEWSYM RDSPReg59       ; Voice  5  OUTX
      mov al,[DSPMem+059h]
      ret

NEWSYM RDSPReg5A       ;
      mov al,[DSPMem+05Ah]
      ret

NEWSYM RDSPReg5B       ;
      mov al,[DSPMem+05Bh]
      ret

NEWSYM RDSPReg5C       ;
      mov al,[DSPMem+05Ch]
      ret

NEWSYM RDSPReg5D       ;
      mov al,[DSPMem+05Dh]
      ret

NEWSYM RDSPReg5E       ;
      mov al,[DSPMem+05Eh]
      ret

NEWSYM RDSPReg5F       ; Voice  5  Echo coefficient
      mov al,[DSPMem+05Fh]
      ret

NEWSYM RDSPReg60       ; Voice  6  Volume Left
      mov al,[DSPMem+60h]
      ret

NEWSYM RDSPReg61       ; Voice  6  Volume Right
      mov al,[DSPMem+61h]
      ret

NEWSYM RDSPReg62       ; Voice  6  Pitch Low
      mov al,[DSPMem+062h]
      ret

NEWSYM RDSPReg63       ; Voice  6  Pitch High
      mov al,[DSPMem+063h]
      ret

NEWSYM RDSPReg64       ; Voice  6  SCRN
      mov al,[DSPMem+064h]
      ret

NEWSYM RDSPReg65       ; Voice  6  ADSR (1)
      mov al,[DSPMem+065h]
      ret

NEWSYM RDSPReg66       ; Voice  6  ADSR (2)
      mov al,[DSPMem+066h]
      ret

NEWSYM RDSPReg67       ; Voice  6  GAIN
      mov al,[DSPMem+067h]
      ret

NEWSYM RDSPReg68       ; Voice  6  ENVX
      mov al,[DSPMem+068h]
      and al,7Fh
      ret

NEWSYM RDSPReg69       ; Voice  6  OUTX
      mov al,[DSPMem+069h]
      ret

NEWSYM RDSPReg6A       ;
      mov al,[DSPMem+06Ah]
      ret

NEWSYM RDSPReg6B       ;
      mov al,[DSPMem+06Bh]
      ret

NEWSYM RDSPReg6C       ;
      mov al,[DSPMem+06Ch]
      ret

NEWSYM RDSPReg6D       ;
      mov al,[DSPMem+06Dh]
      ret

NEWSYM RDSPReg6E       ;
      mov al,[DSPMem+06Eh]
      ret

NEWSYM RDSPReg6F       ; Voice  6  Echo coefficient
      mov al,[DSPMem+06Fh]
      ret

NEWSYM RDSPReg70       ; Voice  7  Volume Left
      mov al,[DSPMem+70h]
      ret

NEWSYM RDSPReg71       ; Voice  7  Volume Right
      mov al,[DSPMem+71h]
      ret

NEWSYM RDSPReg72       ; Voice  7  Pitch Low
      mov al,[DSPMem+072h]
      ret

NEWSYM RDSPReg73       ; Voice  7  Pitch High
      mov al,[DSPMem+073h]
      ret

NEWSYM RDSPReg74       ; Voice  7  SCRN
      mov al,[DSPMem+074h]
      ret

NEWSYM RDSPReg75       ; Voice  7  ADSR (1)
      mov al,[DSPMem+075h]
      ret

NEWSYM RDSPReg76       ; Voice  7  ADSR (2)
      mov al,[DSPMem+076h]
      ret

NEWSYM RDSPReg77       ; Voice  7  GAIN
      mov al,[DSPMem+077h]
      ret

NEWSYM RDSPReg78       ; Voice  7  ENVX
      mov al,[DSPMem+078h]
      and al,7Fh
      ret

NEWSYM RDSPReg79       ; Voice  7  OUTX
      mov al,[DSPMem+079h]
      ret

NEWSYM RDSPReg7A       ;
      mov al,[DSPMem+07Ah]
      ret

NEWSYM RDSPReg7B       ;
      mov al,[DSPMem+07Bh]
      ret

NEWSYM RDSPReg7C       ;
      mov al,[DSPMem+07Ch]
      ret

NEWSYM RDSPReg7D       ;
      mov al,[DSPMem+07Dh]
      ret

NEWSYM RDSPReg7E       ;
      mov al,[DSPMem+07Eh]
      ret

NEWSYM RDSPReg7F       ; Voice  7  Echo coefficient
      mov al,[DSPMem+07Fh]
      ret

NEWSYM RDSPReg80       ;
      mov al,[DSPMem+080h]
      ret

NEWSYM RDSPReg81       ;
      mov al,[DSPMem+081h]
      ret

NEWSYM RDSPReg82       ;
      mov al,[DSPMem+082h]
      ret

NEWSYM RDSPReg83       ;
      mov al,[DSPMem+083h]
      ret

NEWSYM RDSPReg84       ;
      mov al,[DSPMem+084h]
      ret

NEWSYM RDSPReg85       ;
      mov al,[DSPMem+085h]
      ret

NEWSYM RDSPReg86       ;
      mov al,[DSPMem+086h]
      ret

NEWSYM RDSPReg87       ;
      mov al,[DSPMem+087h]
      ret

NEWSYM RDSPReg88       ;
      mov al,[DSPMem+088h]
      ret

NEWSYM RDSPReg89       ;
      mov al,[DSPMem+089h]
      ret

NEWSYM RDSPReg8A       ;
      mov al,[DSPMem+08Ah]
      ret

NEWSYM RDSPReg8B       ;
      mov al,[DSPMem+08Bh]
      ret

NEWSYM RDSPReg8C       ;
      mov al,[DSPMem+08Ch]
      ret

NEWSYM RDSPReg8D       ;
      mov al,[DSPMem+08Dh]
      ret

NEWSYM RDSPReg8E       ;
      mov al,[DSPMem+08Eh]
      ret

NEWSYM RDSPReg8F       ;
      mov al,[DSPMem+08Fh]
      ret

NEWSYM RDSPReg90       ;
      mov al,[DSPMem+090h]
      ret

NEWSYM RDSPReg91       ;
      mov al,[DSPMem+091h]
      ret

NEWSYM RDSPReg92       ;
      mov al,[DSPMem+092h]
      ret

NEWSYM RDSPReg93       ;
      mov al,[DSPMem+093h]
      ret

NEWSYM RDSPReg94       ;
      mov al,[DSPMem+094h]
      ret

NEWSYM RDSPReg95       ;
      mov al,[DSPMem+095h]
      ret

NEWSYM RDSPReg96       ;
      mov al,[DSPMem+096h]
      ret

NEWSYM RDSPReg97       ;
      mov al,[DSPMem+097h]
      ret

NEWSYM RDSPReg98       ;
      mov al,[DSPMem+098h]
      ret

NEWSYM RDSPReg99       ;
      mov al,[DSPMem+099h]
      ret

NEWSYM RDSPReg9A       ;
      mov al,[DSPMem+09Ah]
      ret

NEWSYM RDSPReg9B       ;
      mov al,[DSPMem+09Bh]
      ret

NEWSYM RDSPReg9C       ;
      mov al,[DSPMem+09Ch]
      ret

NEWSYM RDSPReg9D       ;
      mov al,[DSPMem+09Dh]
      ret

NEWSYM RDSPReg9E       ;
      mov al,[DSPMem+09Eh]
      ret

NEWSYM RDSPReg9F       ;
      mov al,[DSPMem+09Fh]
      ret

NEWSYM RDSPRegA0       ;
      mov al,[DSPMem+0A0h]
      ret

NEWSYM RDSPRegA1       ;
      mov al,[DSPMem+0A1h]
      ret

NEWSYM RDSPRegA2       ;
      mov al,[DSPMem+0A2h]
      ret

NEWSYM RDSPRegA3       ;
      mov al,[DSPMem+0A3h]
      ret

NEWSYM RDSPRegA4       ;
      mov al,[DSPMem+0A4h]
      ret

NEWSYM RDSPRegA5       ;
      mov al,[DSPMem+0A5h]
      ret

NEWSYM RDSPRegA6       ;
      mov al,[DSPMem+0A6h]
      ret

NEWSYM RDSPRegA7       ;
      mov al,[DSPMem+0A7h]
      ret

NEWSYM RDSPRegA8       ;
      mov al,[DSPMem+0A8h]
      ret

NEWSYM RDSPRegA9       ;
      mov al,[DSPMem+0A9h]
      ret

NEWSYM RDSPRegAA       ;
      mov al,[DSPMem+0AAh]
      ret

NEWSYM RDSPRegAB       ;
      mov al,[DSPMem+0ABh]
      ret

NEWSYM RDSPRegAC       ;
      mov al,[DSPMem+0ACh]
      ret

NEWSYM RDSPRegAD       ;
      mov al,[DSPMem+0ADh]
      ret

NEWSYM RDSPRegAE       ;
      mov al,[DSPMem+0AEh]
      ret

NEWSYM RDSPRegAF       ;
      mov al,[DSPMem+0AFh]
      ret

NEWSYM RDSPRegB0       ;
      mov al,[DSPMem+0B0h]
      ret

NEWSYM RDSPRegB1       ;
      mov al,[DSPMem+0B1h]
      ret

NEWSYM RDSPRegB2       ;
      mov al,[DSPMem+0B2h]
      ret

NEWSYM RDSPRegB3       ;
      mov al,[DSPMem+0B3h]
      ret

NEWSYM RDSPRegB4       ;
      mov al,[DSPMem+0B4h]
      ret

NEWSYM RDSPRegB5       ;
      mov al,[DSPMem+0B5h]
      ret

NEWSYM RDSPRegB6       ;
      mov al,[DSPMem+0B6h]
      ret

NEWSYM RDSPRegB7       ;
      mov al,[DSPMem+0B7h]
      ret

NEWSYM RDSPRegB8       ;
      mov al,[DSPMem+0B8h]
      ret

NEWSYM RDSPRegB9       ;
      mov al,[DSPMem+0B9h]
      ret

NEWSYM RDSPRegBA       ;
      mov al,[DSPMem+0BAh]
      ret

NEWSYM RDSPRegBB       ;
      mov al,[DSPMem+0BBh]
      ret

NEWSYM RDSPRegBC       ;
      mov al,[DSPMem+0BCh]
      ret

NEWSYM RDSPRegBD       ;
      mov al,[DSPMem+0BDh]
      ret

NEWSYM RDSPRegBE       ;
      mov al,[DSPMem+0BEh]
      ret

NEWSYM RDSPRegBF       ;
      mov al,[DSPMem+0BFh]
      ret

NEWSYM RDSPRegC0       ;
      mov al,[DSPMem+0C0h]
      ret

NEWSYM RDSPRegC1       ;
      mov al,[DSPMem+0C1h]
      ret

NEWSYM RDSPRegC2       ;
      mov al,[DSPMem+0C2h]
      ret

NEWSYM RDSPRegC3       ;
      mov al,[DSPMem+0C3h]
      ret

NEWSYM RDSPRegC4       ;
      mov al,[DSPMem+0C4h]
      ret

NEWSYM RDSPRegC5       ;
      mov al,[DSPMem+0C5h]
      ret

NEWSYM RDSPRegC6       ;
      mov al,[DSPMem+0C6h]
      ret

NEWSYM RDSPRegC7       ;
      mov al,[DSPMem+0C7h]
      ret

NEWSYM RDSPRegC8       ;
      mov al,[DSPMem+0C8h]
      ret

NEWSYM RDSPRegC9       ;
      mov al,[DSPMem+0C9h]
      ret

NEWSYM RDSPRegCA       ;
      mov al,[DSPMem+0CAh]
      ret

NEWSYM RDSPRegCB       ;
      mov al,[DSPMem+0CBh]
      ret

NEWSYM RDSPRegCC       ;
      mov al,[DSPMem+0CCh]
      ret

NEWSYM RDSPRegCD       ;
      mov al,[DSPMem+0CDh]
      ret

NEWSYM RDSPRegCE       ;
      mov al,[DSPMem+0CEh]
      ret

NEWSYM RDSPRegCF       ;
      mov al,[DSPMem+0CFh]
      ret

NEWSYM RDSPRegD0       ;
      mov al,[DSPMem+0D0h]
      ret

NEWSYM RDSPRegD1       ;
      mov al,[DSPMem+0D1h]
      ret

NEWSYM RDSPRegD2       ;
      mov al,[DSPMem+0D2h]
      ret

NEWSYM RDSPRegD3       ;
      mov al,[DSPMem+0D3h]
      ret

NEWSYM RDSPRegD4       ;
      mov al,[DSPMem+0D4h]
      ret

NEWSYM RDSPRegD5       ;
      mov al,[DSPMem+0D5h]
      ret

NEWSYM RDSPRegD6       ;
      mov al,[DSPMem+0D6h]
      ret

NEWSYM RDSPRegD7       ;
      mov al,[DSPMem+0D7h]
      ret

NEWSYM RDSPRegD8       ;
      mov al,[DSPMem+0D8h]
      ret

NEWSYM RDSPRegD9       ;
      mov al,[DSPMem+0D9h]
      ret

NEWSYM RDSPRegDA       ;
      mov al,[DSPMem+0DAh]
      ret

NEWSYM RDSPRegDB       ;
      mov al,[DSPMem+0DBh]
      ret

NEWSYM RDSPRegDC       ;
      mov al,[DSPMem+0DCh]
      ret

NEWSYM RDSPRegDD       ;
      mov al,[DSPMem+0DDh]
      ret

NEWSYM RDSPRegDE       ;
      mov al,[DSPMem+0DEh]
      ret

NEWSYM RDSPRegDF       ;
      mov al,[DSPMem+0DFh]
      ret

NEWSYM RDSPRegE0       ;
      mov al,[DSPMem+0E0h]
      ret

NEWSYM RDSPRegE1       ;
      mov al,[DSPMem+0E1h]
      ret

NEWSYM RDSPRegE2       ;
      mov al,[DSPMem+0E2h]
      ret

NEWSYM RDSPRegE3       ;
      mov al,[DSPMem+0E3h]
      ret

NEWSYM RDSPRegE4       ;
      mov al,[DSPMem+0E4h]
      ret

NEWSYM RDSPRegE5       ;
      mov al,[DSPMem+0E5h]
      ret

NEWSYM RDSPRegE6       ;
      mov al,[DSPMem+0E6h]
      ret

NEWSYM RDSPRegE7       ;
      mov al,[DSPMem+0E7h]
      ret

NEWSYM RDSPRegE8       ;
      mov al,[DSPMem+0E8h]
      ret

NEWSYM RDSPRegE9       ;
      mov al,[DSPMem+0E9h]
      ret

NEWSYM RDSPRegEA       ;
      mov al,[DSPMem+0EAh]
      ret

NEWSYM RDSPRegEB       ;
      mov al,[DSPMem+0EBh]
      ret

NEWSYM RDSPRegEC       ;
      mov al,[DSPMem+0ECh]
      ret

NEWSYM RDSPRegED       ;
      mov al,[DSPMem+0EDh]
      ret

NEWSYM RDSPRegEE       ;
      mov al,[DSPMem+0EEh]
      ret

NEWSYM RDSPRegEF       ;
      mov al,[DSPMem+0EFh]
      ret

NEWSYM RDSPRegF0       ;
      mov al,[DSPMem+0F0h]
      ret

NEWSYM RDSPRegF1       ;
      mov al,[DSPMem+0F1h]
      ret

NEWSYM RDSPRegF2       ;
      mov al,[DSPMem+0F2h]
      ret

NEWSYM RDSPRegF3       ;
      mov al,[DSPMem+0F3h]
      ret

NEWSYM RDSPRegF4       ;
      mov al,[DSPMem+0F4h]
      ret

NEWSYM RDSPRegF5       ;
      mov al,[DSPMem+0F5h]
      ret

NEWSYM RDSPRegF6       ;
      mov al,[DSPMem+0F6h]
      ret

NEWSYM RDSPRegF7      ;
      mov al,[DSPMem+0F7h]
      ret

NEWSYM RDSPRegF8      ;
      mov al,[DSPMem+0F8h]
      ret

NEWSYM RDSPRegF9      ;
      mov al,[DSPMem+0F9h]
      ret

NEWSYM RDSPRegFA      ;
      mov al,[DSPMem+0FAh]
      ret

NEWSYM RDSPRegFB      ;
      mov al,[DSPMem+0FBh]
      ret

NEWSYM RDSPRegFC      ;
      mov al,[DSPMem+0FCh]
      ret

NEWSYM RDSPRegFD      ;
      mov al,[DSPMem+0FDh]
      ret

NEWSYM RDSPRegFE      ;
      mov al,[DSPMem+0FEh]
      ret

NEWSYM RDSPRegFF      ;
      mov al,[DSPMem+0FFh]
      ret

%macro ProcessGain 1
      push eax
      push ebx
      push edx
      test byte[DSPMem+07h+%1*10h],80h
      jz near %%Direct
      test byte[DSPMem+07h+%1*10h],40h
      jnz near %%Increase
      test byte[DSPMem+07h+%1*10h],20h
      jz %%LinearDec
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
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
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],7
      ret
%%LinearDec
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Decrease+eax*4]
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],5
      ret
%%Increase
      test byte[DSPMem+07h+%1*10h],20h
      jz %%LinearInc
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
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
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],6
      ret
%%LinearInc
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Increase+eax*4]
      mov dword[Voice0EnvInc+%1*4],0
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      mov eax,127*65536
      div ebx
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],3
      ret
%%Direct
      mov al,[DSPMem+07h+%1*10h]
      and al,7Fh
      mov dword[Voice0EnvInc+%1*4],0
      mov [Voice0EnvInc+%1*4+2],al
      mov dword[Voice0Time+%1*4],0FFFFFFFFh
      mov dword[Voice0IncNumber+%1*4],0
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],4
      ret
%endmacro

%macro ProcessGain2 1
      push eax
      push ebx
      push edx
      test byte[DSPMem+07h+%1*10h],80h
      jz near %%Direct
      test byte[DSPMem+07h+%1*10h],40h
      jnz near %%Increase
      test byte[DSPMem+07h+%1*10h],20h
      jz %%LinearDec
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[DecreaseRateExp+eax*4]
      shr ebx,5
      mov [Voice0Time+%1*4],ebx
      mov [GainDecBendDataTime+%1*4],ebx
      xor edx,edx
      mov dh,118
      mov dl,[Voice0EnvInc+%1*4+2]
      xor eax,eax
      mov al,[VolumeConvTable+edx*2]
      xor edx,edx
      shl eax,16
      mov dl,[Voice0EnvInc+%1*4+2]
      neg eax
      shl edx,16
      add eax,edx
      xor edx,edx
      mov byte[GainDecBendDataPos+%1],0
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      mov al,[Voice0EnvInc+%1*4+2]
      mov [GainDecBendDataDat+%1],al
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],7
      ret
%%LinearDec
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Decrease+eax*4]
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      xor eax,eax
      mov al,[Voice0EnvInc+%1*4+2]
      shl eax,16
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],5
      ret
%%Increase
      test byte[DSPMem+07h+%1*10h],20h
      jz %%LinearInc
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Increase+eax*4]
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      xor eax,eax
      mov al,[Voice0EnvInc+%1*4+2]
      inc al
      test al,80h
      jz %%noof
      mov al,127
%%noof
      xor al,127
      shl eax,16
      div ebx
      mov [Voice0IncNumber+%1*4],eax
      mov ebx,[Voice0Time+%1*4]
      mov eax,ebx
      shr eax,2
      sub ebx,eax
      dec ebx
      mov [Voice0Time+%1*4],ebx
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],6
      ret
%%LinearInc
      xor eax,eax
      mov al,[DSPMem+07h+%1*10h]
      and al,1Fh
      mov ebx,[Increase+eax*4]
      mov [Voice0Time+%1*4],ebx
      xor edx,edx
      xor eax,eax
      mov al,[Voice0EnvInc+%1*4+2]
      inc al
      test al,80h
      jz %%noof2
      mov al,127
%%noof2
      xor al,127
      shl eax,16
      div ebx
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],3
      ret
%%Direct
      mov al,[DSPMem+07h+%1*10h]
      and al,7Fh
      mov dword[Voice0EnvInc+%1*4],0
      mov [Voice0EnvInc+%1*4+2],al
      mov dword[Voice0Time+%1*4],0FFFFFFFFh
      mov dword[Voice0IncNumber+%1*4],0
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],4
      ret
%%end
      pop edx
      pop ebx
      pop eax
      ret
%endmacro


%macro SwitchSustain 1
      push eax
      push ebx
      push edx
      mov al,[Voice0EnvInc+%1*4+2]
      mov [GainDecBendDataDat+%1],al
      cmp byte[Voice0State+%1],8
      je %%full
      cmp byte[Voice0State+%1],2
      jae %%nofull
%%full
      mov byte[GainDecBendDataDat+%1],7Fh
%%nofull
      mov al,[DSPMem+05h+%1*10h]
      shr al,4
      and eax,07h
      mov edx,[DecayRate+eax*4]
      xor eax,eax
      mov al,[DSPMem+06h+%1*10h]
      and al,1Fh
      mov ebx,[SustainRate+eax*4]
      cmp edx,ebx
      jae near %%decayover
      ; ebx = total sustain time
      xor eax,eax
      mov al,[DSPMem+06h+%1*10h]
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
      or ebx,ebx
      jz .oopszero
      div ebx
.oopszero
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
      mov dword[Voice0EnvInc+%1*4],0
      mov al,[GainDecBendDataDat+%1]
      mov [Voice0EnvInc+%1*4+2],al
      mov ebx,[Voice0Time+%1*4]
      xor edx,edx

      mov dh,122
      mov dl,[Voice0EnvInc+%1*4+2]
      xor eax,eax
      mov al,[VolumeConvTable+edx*2]
      xor edx,edx
      shl eax,16
      mov dl,[Voice0EnvInc+%1*4+2]
      neg eax
      shl edx,16
      add eax,edx
      xor edx,edx
      mov byte[GainDecBendDataPos+%1],0
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],9
      ret
%%decayover
      sub edx,ebx
      push ebx
      mov eax,edx
      xor ebx,ebx
      mov bl,[DSPMem+06h+%1*10h]
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

      mov dh,118
      mov dl,[Voice0EnvInc+%1*4+2]
      xor eax,eax
      mov al,[VolumeConvTable+edx*2]
      xor edx,edx
      shl eax,16
      mov dl,[Voice0EnvInc+%1*4+2]
      neg eax
      shl edx,16
      add eax,edx
      xor edx,edx
      mov byte[GainDecBendDataPos+%1],0
      div ebx
      neg eax
      mov [Voice0IncNumber+%1*4],eax
      pop edx
      pop ebx
      pop eax
      mov byte[Voice0State+%1],7
      ret
%endmacro

%macro VoiceAdsr 1
      test byte[MuteVoiceF],1 << %1
      jnz near .nogain
      cmp byte[Voice0State+%1],200
      je near .nogain
      cmp [DSPMem+05h+%1*10h],al
      je near .nogain
      test al,80h
      jz near .gain
      mov [DSPMem+05h+%1*10h],al
      SwitchSustain %1
      ret
.nogain
      mov [DSPMem+05h+%1*10h],al
      ret
.gain
      cmp byte[Voice0Status+%1],1
      jne .nogain
      cmp word[DSPMem+06h+%1*10h],0A0E0h
      je .nogain
      test byte[DSPMem+05h+%1*10h],80h
      jz near .gain2
      cmp byte[Voice0State+%1],8
      je .gain1
      cmp byte[Voice0State+%1],2
      jae near .gain2
.gain1
      mov [DSPMem+05h+%1*10h],al
      ProcessGain %1   ; Normal
      ret
.gain2
      mov [DSPMem+05h+%1*10h],al
      cmp byte[Voice0State+%1],210
      jne %%noendofsamp2
      push eax
      push ebx
      mov al,%1
      call VoiceStarter
      mov dword[Voice0EnvInc+%1*4],007FFFFFh
      pop ebx
      pop eax
%%noendofsamp2
      ProcessGain2 %1
      ret
%endmacro

%macro VoiceAdsr2 1
      test byte[MuteVoiceF],1 << %1
      jnz near .noadsrswitch
      cmp byte[Voice0State+%1],200
      je near .noadsrswitch
      cmp [DSPMem+06h+%1*10h],al
      je near .noadsrswitch
      mov [DSPMem+06h+%1*10h],al
      test byte[DSPMem+05h+%1*10h],80h
      jz near .noadsrswitch
      SwitchSustain %1
      ret
.noadsrswitch
      mov [DSPMem+06h+%1*10h],al
      ret
%endmacro


%macro VoiceGain 1
      test byte[MuteVoiceF],1 << %1
      jnz .nogain
      cmp byte[Voice0State+%1],200
      je .nogain
      cmp [DSPMem+07h+%1*10h],al
      je .nogain
      mov [DSPMem+07h+%1*10h],al
      cmp byte[Voice0Status+%1],1
      jne .nogain
      test byte[DSPMem+05h+%1*10h],80h
      jz .gain
.nogain
      mov [DSPMem+07h+%1*10h],al
      ret
.gain
      cmp byte[Voice0State+%1],210
      jne %%noendofsamp
      push eax
      push ebx
      mov al,%1
      call VoiceStarter
      pop ebx
      pop eax
%%noendofsamp
      ProcessGain2 %1
%endmacro

;Write DSP Registers functions

NEWSYM WDSPReg00       ; Voice  0  Volume Left
      mov [DSPMem+00h],al
      ret
      mov [DSPMem+01h],al
      ret

NEWSYM WDSPReg01       ; Voice  0  Volume Right
      mov [DSPMem+01h],al
      ret
      mov [DSPMem+00h],al
      ret

NEWSYM WDSPReg02       ; Voice  0  Pitch Low
      mov [DSPMem+02h],al
      ret

NEWSYM WDSPReg03       ; Voice  0  Pitch High
      mov [DSPMem+03h],al
      ret

NEWSYM WDSPReg04       ; Voice  0  SCRN
      mov [DSPMem+04h],al
      ret

NEWSYM WDSPReg05       ; Voice  0  ADSR (1)
      VoiceAdsr 0
      ret

NEWSYM WDSPReg06       ; Voice  0  ADSR (2)
      VoiceAdsr2 0
      ret

NEWSYM WDSPReg07       ; Voice  0  GAIN
      VoiceGain 0
      ret

NEWSYM WDSPReg08       ; Voice  0  ENVX
      mov [DSPMem+08h],al
      ret

NEWSYM WDSPReg09       ; Voice  0  OUTX
      mov [DSPMem+09h],al
      ret

NEWSYM WDSPReg0A       ; Voice  0
      mov [DSPMem+0Ah],al
      ret

NEWSYM WDSPReg0B       ; Voice  0
      mov [DSPMem+0Bh],al
      ret

NEWSYM WDSPReg0C       ; Voice  0
      mov [DSPMem+0Ch],al
      push eax
      and eax,0FFh
      mov al,[VolumeTableb+eax]
      mov ah,[MusicVol]
      mov al,[VolumeConvTable+eax*2]
      mov [GlobalVL],al
      pop eax
      ret

NEWSYM WDSPReg0D       ; Echo Feedback
      mov [DSPMem+0Dh],al
      push eax
      and eax,0FFh
      mov al,[VolumeTableb+eax]
      mov [EchoFB],eax
      pop eax
      ret

NEWSYM WDSPReg0E       ; Voice  0
      mov [DSPMem+0Eh],al
      ret

NEWSYM WDSPReg0F       ; Voice  0  Echo coefficient
      mov [DSPMem+0Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal0],eax
      pop eax
      ret

NEWSYM WDSPReg10       ; Voice  1  Volume Left
      mov [DSPMem+10h],al
      ret

NEWSYM WDSPReg11       ; Voice  1  Volume Right
      mov [DSPMem+11h],al
      ret

NEWSYM WDSPReg12       ; Voice  1  Pitch Low
      mov [DSPMem+012h],al
      ret

NEWSYM WDSPReg13       ; Voice  1  Pitch High
      mov [DSPMem+013h],al
      ret

NEWSYM WDSPReg14       ; Voice  1  SCRN
      mov [DSPMem+14h],al
      ret

NEWSYM WDSPReg15       ; Voice  1  ADSR (1)
      VoiceAdsr 1
      ret

NEWSYM WDSPReg16       ; Voice  1  ADSR (2)
      VoiceAdsr2 1
      ret

NEWSYM WDSPReg17       ; Voice  1  GAIN
      VoiceGain 1
      ret

NEWSYM WDSPReg18       ; Voice  1  ENVX
      mov [DSPMem+018h],al
      ret

NEWSYM WDSPReg19       ; Voice  1  OUTX
      mov [DSPMem+019h],al
      ret

NEWSYM WDSPReg1A       ; Voice  1
      mov [DSPMem+01Ah],al
      ret

NEWSYM WDSPReg1B       ; Voice  1
      mov [DSPMem+01Bh],al
      ret

NEWSYM WDSPReg1C       ; Voice  1
      mov [DSPMem+01Ch],al
      push eax
      and eax,0FFh
      mov al,[VolumeTableb+eax]
      mov ah,[MusicVol]
      mov al,[VolumeConvTable+eax*2]
      mov [GlobalVR],al
      pop eax
      ret

NEWSYM WDSPReg1D       ; Voice  1
      mov [DSPMem+01Dh],al
      ret

NEWSYM WDSPReg1E       ; Voice  1
      mov [DSPMem+01Eh],al
      ret

NEWSYM WDSPReg1F       ; Voice  1  Echo coefficient
      mov [DSPMem+01Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal1],eax
      pop eax
      ret

NEWSYM WDSPReg20       ; Voice  2  Volume Left
      mov [DSPMem+20h],al
      ret

NEWSYM WDSPReg21       ; Voice  2  Volume Right
      mov [DSPMem+21h],al
      ret

NEWSYM WDSPReg22       ; Voice  2  Pitch Low
      mov [DSPMem+022h],al
      ret

NEWSYM WDSPReg23       ; Voice  2  Pitch High
      mov [DSPMem+023h],al
      ret

NEWSYM WDSPReg24       ; Voice  2  SCRN
      mov [DSPMem+24h],al
      ret

NEWSYM WDSPReg25       ; Voice  2  ADSR (1)
      VoiceAdsr 2
      ret

NEWSYM WDSPReg26       ; Voice  2  ADSR (2)
      VoiceAdsr2 2
      ret

NEWSYM WDSPReg27       ; Voice  2  GAIN
      VoiceGain 2
      ret

NEWSYM WDSPReg28       ; Voice  2  ENVX
      mov [DSPMem+028h],al
      ret

NEWSYM WDSPReg29       ; Voice  2  OUTX
      mov [DSPMem+029h],al
      ret

NEWSYM WDSPReg2A       ; Voice  2
      mov [DSPMem+02Ah],al
      ret

NEWSYM WDSPReg2B       ; Voice  2
      mov [DSPMem+02Bh],al
      ret

NEWSYM WDSPReg2C       ; Voice  2
      mov [DSPMem+02Ch],al
      push eax
      and eax,0FFh
      mov al,[VolumeTableb+eax]
      mov ah,[MusicVol]
      mov al,[VolumeConvTable+eax*2]
      mov [EchoVL],al
      pop eax
      ret

NEWSYM WDSPReg2D       ; Voice  2
      mov [DSPMem+02Dh],al
      ret

NEWSYM WDSPReg2E       ; Voice  2
      mov [DSPMem+02Eh],al
      ret

NEWSYM WDSPReg2F       ; Voice  2  Echo coefficient
      mov [DSPMem+02Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal2],eax
      pop eax
      ret

NEWSYM WDSPReg30       ; Voice  3  Volume Left
      mov [DSPMem+30h],al
      ret

NEWSYM WDSPReg31       ; Voice  3  Volume Right
      mov [DSPMem+31h],al
      ret

NEWSYM WDSPReg32       ; Voice  3  Pitch Low
      mov [DSPMem+032h],al
      ret

NEWSYM WDSPReg33       ; Voice  3  Pitch High
      mov [DSPMem+033h],al
      ret

NEWSYM WDSPReg34       ; Voice  3  SCRN
      mov [DSPMem+34h],al
      ret

NEWSYM WDSPReg35       ; Voice  3  ADSR (1)
      VoiceAdsr 3
      ret

NEWSYM WDSPReg36       ; Voice  3  ADSR (2)
      VoiceAdsr2 3
      ret

NEWSYM WDSPReg37       ; Voice  3  GAIN
      VoiceGain 3
      ret

NEWSYM WDSPReg38       ; Voice  3  ENVX
      mov [DSPMem+038h],al
      ret

NEWSYM WDSPReg39       ; Voice  3  OUTX
      mov [DSPMem+039h],al
      ret

NEWSYM WDSPReg3A       ; Voice  3
      mov [DSPMem+03Ah],al
      ret

NEWSYM WDSPReg3B       ; Voice  3
      mov [DSPMem+03Bh],al
      ret

NEWSYM WDSPReg3C       ; Voice  3
      mov [DSPMem+03Ch],al
      push eax
      and eax,0FFh
      mov al,[VolumeTableb+eax]
      mov ah,[MusicVol]
      mov al,[VolumeConvTable+eax*2]
      mov [EchoVR],al
      pop eax
      ret

NEWSYM WDSPReg3D       ; Voice  3
      mov byte[Voice0Noise],0
      mov byte[Voice1Noise],0
      mov byte[Voice2Noise],0
      mov byte[Voice3Noise],0
      mov byte[Voice4Noise],0
      mov byte[Voice5Noise],0
      mov byte[Voice6Noise],0
      mov byte[Voice7Noise],0
      test al,1
      jz .TestVoice1
      mov byte[Voice0Noise],1
.TestVoice1
      test al,2
      jz .TestVoice2
      mov byte[Voice1Noise],1
.TestVoice2
      test al,4
      jz .TestVoice3
      mov byte[Voice2Noise],1
.TestVoice3
      test al,8
      jz .TestVoice4
      mov byte[Voice3Noise],1
.TestVoice4
      test al,16
      jz .TestVoice5
      mov byte[Voice4Noise],1
.TestVoice5
      test al,32
      jz .TestVoice6
      mov byte[Voice5Noise],1
.TestVoice6
      test al,64
      jz .TestVoice7
      mov byte[Voice6Noise],1
.TestVoice7
      test al,128
      jz .TestVoice8
      mov byte[Voice7Noise],1
.TestVoice8
      mov [DSPMem+03Dh],al
      ret

NEWSYM WDSPReg3E       ; Voice  3
      mov [DSPMem+03Eh],al
      ret

NEWSYM WDSPReg3F       ; Voice  3  Echo coefficient
      mov [DSPMem+03Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal3],eax
      pop eax
      ret

NEWSYM WDSPReg40       ; Voice  4  Volume Left
      mov [DSPMem+40h],al
      ret

NEWSYM WDSPReg41       ; Voice  4  Volume Right
      mov [DSPMem+41h],al
      ret

NEWSYM WDSPReg42       ; Voice  4  Pitch Low
      mov [DSPMem+042h],al
      ret

NEWSYM WDSPReg43       ; Voice  4  Pitch High
      mov [DSPMem+043h],al
      ret

NEWSYM WDSPReg44       ; Voice  4  SCRN
      mov [DSPMem+44h],al
      ret

NEWSYM WDSPReg45       ; Voice  4  ADSR (1)
      VoiceAdsr 4
      ret

NEWSYM WDSPReg46       ; Voice  4  ADSR (2)
      VoiceAdsr2 4
      ret

NEWSYM WDSPReg47       ; Voice  4  GAIN
      VoiceGain 4
      ret

NEWSYM WDSPReg48       ; Voice  4  ENVX
      mov [DSPMem+048h],al
      ret

NEWSYM WDSPReg49       ; Voice  4  OUTX
      mov [DSPMem+049h],al
      ret

NEWSYM WDSPReg4A       ; Voice  4
      mov [DSPMem+04Ah],al
      ret

NEWSYM WDSPReg4B       ; Voice  4
      mov [DSPMem+04Bh],al
      ret

NEWSYM WDSPReg4C       ; Key On
      push ebx
      mov bl,[MuteVoiceF]
      xor bl,0FFh
      and bl,al

      xor byte[DSPMem+05Ch],0FFh
      jnz .notzero
      and bl,[DSPMem+05Ch]
.notzero
      xor byte[DSPMem+05Ch],0FFh

      or byte[KeyOnStA],bl
      pop ebx
      test al,80h
      jz .nokon
      inc byte[SoundTest]
.nokon
      mov [DSPMem+04Ch],al
      push eax
      xor al,0FFh
      and byte[DSPMem+07Ch],al
      pop eax
      ret

NEWSYM ProcessKeyOn
      test al,1
      jz .TestVoice1
      push edx
      call Voice0Start
      pop edx
.TestVoice1
      test al,2
      jz .TestVoice2
      push edx
      call Voice1Start
      pop edx
.TestVoice2
      test al,4
      jz .TestVoice3
      push edx
      call Voice2Start
      pop edx
.TestVoice3
      test al,8
      jz .TestVoice4
      push edx
      call Voice3Start
      pop edx
.TestVoice4
      test al,16
      jz .TestVoice5
      push edx
      call Voice4Start
      pop edx
.TestVoice5
      test al,32
      jz .TestVoice6
      push edx
      call Voice5Start
      pop edx
.TestVoice6
      test al,64
      jz .TestVoice7
      push edx
      call Voice6Start
      pop edx
.TestVoice7
      test al,128
      jz .TestVoice8
      push edx
      call Voice7Start
      pop edx
.TestVoice8
      test al,0FFh
      jz .novoice
      mov byte[keyonsn],1
.novoice
      ret

NEWSYM WDSPReg4D       ; Voice  4
      mov [DSPMem+04Dh],al
      ret

NEWSYM WDSPReg4E       ; Voice  4
      mov [DSPMem+04Eh],al
      ret

NEWSYM WDSPReg4F       ; Voice  4  Echo coefficient
      mov [DSPMem+04Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal4],eax
      pop eax
      ret

NEWSYM WDSPReg50       ; Voice  5  Volume Left
      mov [DSPMem+50h],al
      ret

NEWSYM WDSPReg51       ; Voice  5  Volume Right
      mov [DSPMem+51h],al
      ret

NEWSYM WDSPReg52       ; Voice  5  Pitch Low
      mov [DSPMem+052h],al
      ret

NEWSYM WDSPReg53       ; Voice  5  Pitch High
      mov [DSPMem+053h],al
      ret

NEWSYM WDSPReg54       ; Voice  5  SCRN
      mov [DSPMem+54h],al
      ret

NEWSYM WDSPReg55       ; Voice  5  ADSR (1)
      VoiceAdsr 5
      ret

NEWSYM WDSPReg56       ; Voice  5  ADSR (2)
      VoiceAdsr2 5
      ret

NEWSYM WDSPReg57       ; Voice  5  GAIN
      VoiceGain 5
      ret

NEWSYM WDSPReg58       ; Voice  5  ENVX
      mov [DSPMem+058h],al
      ret

NEWSYM WDSPReg59       ; Voice  5  OUTX
      mov [DSPMem+059h],al
      ret

NEWSYM WDSPReg5A       ; Voice  5
      mov [DSPMem+05Ah],al
      ret

NEWSYM WDSPReg5B       ; Voice  5
      mov [DSPMem+05Bh],al
      ret

%macro keyoffm 1
    test byte[MuteVoiceF],1 << %1
    jnz %%nokeyoff
    push eax
    push edx
    push ebx
    mov dword[Voice0Time+%1*4],255
    mov eax,[Voice0EnvInc+%1*4]
    shr eax,8
    neg eax
    mov [Voice0IncNumber+%1*4],eax
    mov byte[Voice0State+%1],200
    mov byte[DSPMem+08h+%1*10h],0
    or byte[DSPMem+7Ch],1 << %1
    pop ebx
    pop edx
    pop eax
%%nokeyoff
%endmacro

NEWSYM WDSPReg5C       ; Key Off
      push eax
      xor al,0FFh
      and byte[KeyOnStA],al
      and byte[KeyOnStB],al
      pop eax
      test al,1
      jz .TestVoice1
      keyoffm 0
.TestVoice1
      test al,2
      jz .TestVoice2
      keyoffm 1
.TestVoice2
      test al,4
      jz .TestVoice3
      keyoffm 2
.TestVoice3
      test al,8
      jz .TestVoice4
      keyoffm 3
.TestVoice4
      test al,16
      jz .TestVoice5
      keyoffm 4
.TestVoice5
      test al,32
      jz .TestVoice6
      keyoffm 5
.TestVoice6
      test al,64
      jz .TestVoice7
      keyoffm 6
.TestVoice7
      test al,128
      jz .TestVoice8
      keyoffm 7
.TestVoice8
      mov [DSPMem+05Ch],al
      ret

NEWSYM WDSPReg5D       ; Voice  5
      mov [DSPMem+05Dh],al
      ret

NEWSYM WDSPReg5E       ; Voice  5
      mov [DSPMem+05Eh],al
      ret

NEWSYM WDSPReg5F       ; Voice  5  Echo coefficient
      mov [DSPMem+05Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal5],eax
      pop eax
      ret

NEWSYM WDSPReg60       ; Voice  6  Volume Left
      mov [DSPMem+60h],al
      ret

NEWSYM WDSPReg61       ; Voice  6  Volume Right
      mov [DSPMem+61h],al
      ret

NEWSYM WDSPReg62       ; Voice  6  Pitch Low
      mov [DSPMem+062h],al
      ret

NEWSYM WDSPReg63       ; Voice  6  Pitch High
      mov [DSPMem+063h],al
      ret

NEWSYM WDSPReg64       ; Voice  6  SCRN
      mov [DSPMem+64h],al
      ret

NEWSYM WDSPReg65       ; Voice  6  ADSR (1)
      VoiceAdsr 6
      ret

NEWSYM WDSPReg66       ; Voice  6  ADSR (2)
      VoiceAdsr2 6
      ret

NEWSYM WDSPReg67       ; Voice  6  GAIN
      VoiceGain 6
      ret

NEWSYM WDSPReg68       ; Voice  6  ENVX
      mov [DSPMem+068h],al
      ret

NEWSYM WDSPReg69       ; Voice  6  OUTX
      mov [DSPMem+069h],al
      ret

NEWSYM WDSPReg6A       ; Voice  6
      mov [DSPMem+06Ah],al
      ret

NEWSYM WDSPReg6B       ; Voice  6
      mov [DSPMem+06Bh],al
      ret

NEWSYM WDSPReg6C       ; Voice  6
      mov [DSPMem+06Ch],al
      and byte[DSPMem+06Ch],7Fh
      test al,0C0h
      jz .NoRes2
      mov byte[Voice0Status],0
      mov byte[Voice1Status],0
      mov byte[Voice2Status],0
      mov byte[Voice3Status],0
      mov byte[Voice4Status],0
      mov byte[Voice5Status],0
      mov byte[Voice6Status],0
      mov byte[Voice7Status],0
.NoRes2
      push eax
      push ebx
      push ecx
      push edx
      and eax,1Fh
      xor edx,edx
      mov eax,[NoiseSpeeds+eax*4]
      Mul dword[dspPAdj]
      ShrD EAX, EDX, 17
      mov [NoiseInc],eax
      pop edx
      pop ecx
      pop ebx
      pop eax
      ret

NEWSYM WDSPReg6D       ; Voice  6
      mov [DSPMem+06Dh],al
      ret

NEWSYM WDSPReg6E       ; Voice  6
      mov [DSPMem+06Eh],al
      ret

NEWSYM WDSPReg6F       ; Voice  6  Echo coefficient
      mov [DSPMem+06Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal6],eax
      pop eax
      ret

NEWSYM WDSPReg70       ; Voice  7  Volume Left
      mov [DSPMem+70h],al
      ret

NEWSYM WDSPReg71       ; Voice  7  Volume Right
      mov [DSPMem+71h],al
      ret

NEWSYM WDSPReg72       ; Voice  7  Pitch Low
      mov [DSPMem+072h],al
      ret

NEWSYM WDSPReg73       ; Voice  7  Pitch High
      mov [DSPMem+073h],al
      ret

NEWSYM WDSPReg74       ; Voice  7  SCRN
      mov [DSPMem+74h],al
      ret

NEWSYM WDSPReg75       ; Voice  7  ADSR (1)
      VoiceAdsr 7
      ret

NEWSYM WDSPReg76       ; Voice  7  ADSR (2)
      VoiceAdsr2 7
      ret

NEWSYM WDSPReg77       ; Voice  7  GAIN
      VoiceGain 7
      ret

NEWSYM WDSPReg78       ; Voice  7  ENVX
      mov [DSPMem+078h],al
      ret

NEWSYM WDSPReg79       ; Voice  7  OUTX
      mov [DSPMem+079h],al
      ret

NEWSYM WDSPReg7A       ; Voice  7
      mov [DSPMem+07Ah],al
      ret

NEWSYM WDSPReg7B       ; Voice  7
      mov [DSPMem+07Bh],al
      ret

NEWSYM WDSPReg7C       ; ENDX
      mov byte[DSPMem+07Ch],0
      ret

NEWSYM WDSPReg7D       ; Echo Delay
      mov [DSPMem+07Dh],al
      push ebx
      mov ebx,eax
      and ebx,0Fh
      mov ebx,[EchoRate+ebx*4]
      mov [MaxEcho],ebx
      pop ebx
      ret

NEWSYM WDSPReg7E       ; Voice  7
      mov [DSPMem+07Eh],al
      ret

NEWSYM WDSPReg7F       ; Voice  7  Echo coefficient
      mov [DSPMem+07Fh],al
      push eax
      movsx eax,al
      mov [FIRTAPVal7],eax
      pop eax
      ret

NEWSYM WDSPReg80       ;
      mov [DSPMem+080h],al
      ret

NEWSYM WDSPReg81       ;
      mov [DSPMem+081h],al
      ret

NEWSYM WDSPReg82       ;
      mov [DSPMem+082h],al
      ret

NEWSYM WDSPReg83       ;
      mov [DSPMem+083h],al
      ret

NEWSYM WDSPReg84       ;
      mov [DSPMem+084h],al
      ret

NEWSYM WDSPReg85       ;
      mov [DSPMem+085h],al
      ret

NEWSYM WDSPReg86       ;
      mov [DSPMem+086h],al
      ret

NEWSYM WDSPReg87       ;
      mov [DSPMem+087h],al
      ret

NEWSYM WDSPReg88       ;
      mov [DSPMem+088h],al
      ret

NEWSYM WDSPReg89       ;
      mov [DSPMem+089h],al
      ret

NEWSYM WDSPReg8A       ;
      mov [DSPMem+08Ah],al
      ret

NEWSYM WDSPReg8B       ;
      mov [DSPMem+08Bh],al
      ret

NEWSYM WDSPReg8C       ;
      mov [DSPMem+08Ch],al
      ret

NEWSYM WDSPReg8D       ;
      mov [DSPMem+08Dh],al
      ret

NEWSYM WDSPReg8E       ;
      mov [DSPMem+08Eh],al
      ret

NEWSYM WDSPReg8F       ;
      mov [DSPMem+08Fh],al
      ret

NEWSYM WDSPReg90       ;
      mov [DSPMem+090h],al
      ret

NEWSYM WDSPReg91       ;
      mov [DSPMem+091h],al
      ret

NEWSYM WDSPReg92       ;
      mov [DSPMem+092h],al
      ret

NEWSYM WDSPReg93       ;
      mov [DSPMem+093h],al
      ret

NEWSYM WDSPReg94       ;
      mov [DSPMem+094h],al
      ret

NEWSYM WDSPReg95       ;
      mov [DSPMem+095h],al
      ret

NEWSYM WDSPReg96       ;
      mov [DSPMem+096h],al
      ret

NEWSYM WDSPReg97       ;
      mov [DSPMem+097h],al
      ret

NEWSYM WDSPReg98       ;
      mov [DSPMem+098h],al
      ret

NEWSYM WDSPReg99       ;
      mov [DSPMem+099h],al
      ret

NEWSYM WDSPReg9A       ;
      mov [DSPMem+09Ah],al
      ret

NEWSYM WDSPReg9B       ;
      mov [DSPMem+09Bh],al
      ret

NEWSYM WDSPReg9C       ;
      mov [DSPMem+09Ch],al
      ret

NEWSYM WDSPReg9D       ;
      mov [DSPMem+09Dh],al
      ret

NEWSYM WDSPReg9E       ;
      mov [DSPMem+09Eh],al
      ret

NEWSYM WDSPReg9F       ;
      mov [DSPMem+09Fh],al
      ret

NEWSYM WDSPRegA0       ;
      mov [DSPMem+0A0h],al
      ret

NEWSYM WDSPRegA1       ;
      mov [DSPMem+0A1h],al
      ret

NEWSYM WDSPRegA2       ;
      mov [DSPMem+0A2h],al
      ret

NEWSYM WDSPRegA3       ;
      mov [DSPMem+0A3h],al
      ret

NEWSYM WDSPRegA4       ;
      mov [DSPMem+0A4h],al
      ret

NEWSYM WDSPRegA5       ;
      mov [DSPMem+0A5h],al
      ret

NEWSYM WDSPRegA6       ;
      mov [DSPMem+0A6h],al
      ret

NEWSYM WDSPRegA7       ;
      mov [DSPMem+0A7h],al
      ret

NEWSYM WDSPRegA8       ;
      mov [DSPMem+0A8h],al
      ret

NEWSYM WDSPRegA9       ;
      mov [DSPMem+0A9h],al
      ret

NEWSYM WDSPRegAA       ;
      mov [DSPMem+0AAh],al
      ret

NEWSYM WDSPRegAB       ;
      mov [DSPMem+0ABh],al
      ret

NEWSYM WDSPRegAC       ;
      mov [DSPMem+0ACh],al
      ret

NEWSYM WDSPRegAD       ;
      mov [DSPMem+0ADh],al
      ret

NEWSYM WDSPRegAE       ;
      mov [DSPMem+0AEh],al
      ret

NEWSYM WDSPRegAF       ;
      mov [DSPMem+0AFh],al
      ret

NEWSYM WDSPRegB0       ;
      mov [DSPMem+0B0h],al
      ret

NEWSYM WDSPRegB1       ;
      mov [DSPMem+0B1h],al
      ret

NEWSYM WDSPRegB2       ;
      mov [DSPMem+0B2h],al
      ret

NEWSYM WDSPRegB3       ;
      mov [DSPMem+0B3h],al
      ret

NEWSYM WDSPRegB4       ;
      mov [DSPMem+0B4h],al
      ret

NEWSYM WDSPRegB5       ;
      mov [DSPMem+0B5h],al
      ret

NEWSYM WDSPRegB6       ;
      mov [DSPMem+0B6h],al
      ret

NEWSYM WDSPRegB7       ;
      mov [DSPMem+0B7h],al
      ret

NEWSYM WDSPRegB8       ;
      mov [DSPMem+0B8h],al
      ret

NEWSYM WDSPRegB9       ;
      mov [DSPMem+0B9h],al
      ret

NEWSYM WDSPRegBA       ;
      mov [DSPMem+0BAh],al
      ret

NEWSYM WDSPRegBB       ;
      mov [DSPMem+0BBh],al
      ret

NEWSYM WDSPRegBC       ;
      mov [DSPMem+0BCh],al
      ret

NEWSYM WDSPRegBD       ;
      mov [DSPMem+0BDh],al
      ret

NEWSYM WDSPRegBE       ;
      mov [DSPMem+0BEh],al
      ret

NEWSYM WDSPRegBF       ;
      mov [DSPMem+0BFh],al
      ret

NEWSYM WDSPRegC0       ;
      mov [DSPMem+0C0h],al
      ret

NEWSYM WDSPRegC1       ;
      mov [DSPMem+0C1h],al
      ret

NEWSYM WDSPRegC2       ;
      mov [DSPMem+0C2h],al
      ret

NEWSYM WDSPRegC3       ;
      mov [DSPMem+0C3h],al
      ret

NEWSYM WDSPRegC4       ;
      mov [DSPMem+0C4h],al
      ret

NEWSYM WDSPRegC5       ;
      mov [DSPMem+0C5h],al
      ret

NEWSYM WDSPRegC6       ;
      mov [DSPMem+0C6h],al
      ret

NEWSYM WDSPRegC7       ;
      mov [DSPMem+0C7h],al
      ret

NEWSYM WDSPRegC8       ;
      mov [DSPMem+0C8h],al
      ret

NEWSYM WDSPRegC9       ;
      mov [DSPMem+0C9h],al
      ret

NEWSYM WDSPRegCA       ;
      mov [DSPMem+0CAh],al
      ret

NEWSYM WDSPRegCB       ;
      mov [DSPMem+0CBh],al
      ret

NEWSYM WDSPRegCC       ;
      mov [DSPMem+0CCh],al
      ret

NEWSYM WDSPRegCD       ;
      mov [DSPMem+0CDh],al
      ret

NEWSYM WDSPRegCE       ;
      mov [DSPMem+0CEh],al
      ret

NEWSYM WDSPRegCF       ;
      mov [DSPMem+0CFh],al
      ret

NEWSYM WDSPRegD0       ;
      mov [DSPMem+0D0h],al
      ret

NEWSYM WDSPRegD1       ;
      mov [DSPMem+0D1h],al
      ret

NEWSYM WDSPRegD2       ;
      mov [DSPMem+0D2h],al
      ret

NEWSYM WDSPRegD3       ;
      mov [DSPMem+0D3h],al
      ret

NEWSYM WDSPRegD4       ;
      mov [DSPMem+0D4h],al
      ret

NEWSYM WDSPRegD5       ;
      mov [DSPMem+0D5h],al
      ret

NEWSYM WDSPRegD6       ;
      mov [DSPMem+0D6h],al
      ret

NEWSYM WDSPRegD7       ;
      mov [DSPMem+0D7h],al
      ret

NEWSYM WDSPRegD8       ;
      mov [DSPMem+0D8h],al
      ret

NEWSYM WDSPRegD9       ;
      mov [DSPMem+0D9h],al
      ret

NEWSYM WDSPRegDA       ;
      mov [DSPMem+0DAh],al
      ret

NEWSYM WDSPRegDB       ;
      mov [DSPMem+0DBh],al
      ret

NEWSYM WDSPRegDC       ;
      mov [DSPMem+0DCh],al
      ret

NEWSYM WDSPRegDD       ;
      mov [DSPMem+0DDh],al
      ret

NEWSYM WDSPRegDE       ;
      mov [DSPMem+0DEh],al
      ret

NEWSYM WDSPRegDF       ;
      mov [DSPMem+0DFh],al
      ret

NEWSYM WDSPRegE0       ;
      mov [DSPMem+0E0h],al
      ret

NEWSYM WDSPRegE1       ;
      mov [DSPMem+0E1h],al
      ret

NEWSYM WDSPRegE2       ;
      mov [DSPMem+0E2h],al
      ret

NEWSYM WDSPRegE3       ;
      mov [DSPMem+0E3h],al
      ret

NEWSYM WDSPRegE4       ;
      mov [DSPMem+0E4h],al
      ret

NEWSYM WDSPRegE5       ;
      mov [DSPMem+0E5h],al
      ret

NEWSYM WDSPRegE6       ;
      mov [DSPMem+0E6h],al
      ret

NEWSYM WDSPRegE7       ;
      mov [DSPMem+0E7h],al
      ret

NEWSYM WDSPRegE8       ;
      mov [DSPMem+0E8h],al
      ret

NEWSYM WDSPRegE9       ;
      mov [DSPMem+0E9h],al
      ret

NEWSYM WDSPRegEA       ;
      mov [DSPMem+0EAh],al
      ret

NEWSYM WDSPRegEB       ;
      mov [DSPMem+0EBh],al
      ret

NEWSYM WDSPRegEC       ;
      mov [DSPMem+0ECh],al
      ret

NEWSYM WDSPRegED       ;
      mov [DSPMem+0EDh],al
      ret

NEWSYM WDSPRegEE       ;
      mov [DSPMem+0EEh],al
      ret

NEWSYM WDSPRegEF       ;
      mov [DSPMem+0EFh],al
      ret

NEWSYM WDSPRegF0       ;
      mov [DSPMem+0F0h],al
      ret

NEWSYM WDSPRegF1       ;
      mov [DSPMem+0F1h],al
      ret

NEWSYM WDSPRegF2       ;
      mov [DSPMem+0F2h],al
      ret

NEWSYM WDSPRegF3       ;
      mov [DSPMem+0F3h],al
      ret

NEWSYM WDSPRegF4       ;
      mov [DSPMem+0F4h],al
      ret

NEWSYM WDSPRegF5       ;
      mov [DSPMem+0F5h],al
      ret

NEWSYM WDSPRegF6       ;
      mov [DSPMem+0F6h],al
      ret

NEWSYM WDSPRegF7       ;
      mov [DSPMem+0F7h],al
      ret

NEWSYM WDSPRegF8       ;
      mov [DSPMem+0F8h],al
      ret

NEWSYM WDSPRegF9       ;
      mov [DSPMem+0F9h],al
      ret

NEWSYM WDSPRegFA       ;
      mov [DSPMem+0FAh],al
      ret

NEWSYM WDSPRegFB       ;
      mov [DSPMem+0FBh],al
      ret

NEWSYM WDSPRegFC       ;
      mov [DSPMem+0FCh],al
      ret

NEWSYM WDSPRegFD       ;
      mov [DSPMem+0FDh],al
      ret

NEWSYM WDSPRegFE       ;
      mov [DSPMem+0FEh],al
      ret

NEWSYM WDSPRegFF       ;
      mov [DSPMem+0FFh],al
      ret


