;Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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
%ifdef __AMD64__
bits 64
%else
bits 32
%endif

section .text

%ifdef MACHO
section .text align=16
section .data align=4
section .bss  align=4
%endif

%ifdef ELF
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

%ifdef ELF
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%else
%imacro newsym 1
  GLOBAL _%1
  _%1:
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL _%1
  _%1:
  %1: %2
%endmacro
%endif

%ifdef ELF
%define EXTSYM EXTERN
%else
%imacro EXTSYM 1-*
%rep %0
  EXTERN _%1
  %define %1 _%1
%rotate 1
%endrep
%endmacro
%endif

%macro ALIGN32 0
  times ($$-$) & 1Fh nop    ; Long word alignment
%endmacro

%macro ALIGN16 0
  times ($$-$) & 1Fh nop    ; Long word alignment
%endmacro

%macro ccall 1-*
	push ecx
	push edx
%ifdef MACHO
	mov edx, esp
	sub esp, %0 * 4
	and esp, 0xFFFFFFF0 ; Align the stack pointer
%if %0 != 1
	add esp, %0 * 4
	push edx
	mov edx, [edx]
%else
	mov [esp], edx
%endif
%endif
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%ifdef MACHO
	mov esp, [esp + (%0 - 1) * 4]
%elif %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro

%macro ccallv 1+
	push eax
	ccall %1
	pop eax
%endmacro
EXTSYM SPCRAM,RevStereo,VolumeConvTable
EXTSYM DSPMem,NoiseData,EchoDis
EXTSYM echobuf,LowPassFilterType,EMUPause,AudioLogging
EXTSYM StereoSound
EXTSYM LPFexit
EXTSYM LPFstereo
EXTSYM MixEcho
EXTSYM MixEcho2
EXTSYM ProcessVoiceHandler16

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

NEWSYM DSPInterP, resw 1024

section .data
ALIGN32

%include "cpu/firtable.inc"

SECTION .bss

NEWSYM spcWptr,  resd 16     ; SPC Write pointers (point to their own functions)
NEWSYM spcRptr,  resd 16     ; SPC Read pointers (point to their own functions)

SECTION .data
NEWSYM dspPAdj,        dd 0
NEWSYM NumBRRconv,     dd 0
NEWSYM BufferSizeB,    dd 320
NEWSYM BufferSizeW,    dd 640

;TIME_CONSTANT = 256 - 1000000 / frequency

NEWSYM NoiseSpeeds, dd 1,16,21,25,31,42,50,63,83,100,125,167,200,250,333,400,500
  dd 667,800,1000,1333,1600,2000,2667,3200,4000,5333,6400,8000,10667,16000,32000

SECTION .bss

NEWSYM PSampleBuf, resd 24*8

NEWSYM LPFsample1, resd 1
NEWSYM LPFsample2, resd 1

NEWSYM BRRreadahead, resw 4
NEWSYM DLPFsamples, resd 8*24

section .text


; BRRDecode and its ProcessSample/ProcessDynamicLowPass helpers have been
; ported to C (cpu/c_dspproc.c); prev0/prev1/lastbl/loopbl remain shared.
SECTION .data
ALIGN32

NEWSYM prev0,              dd 0         ; previous value 1
NEWSYM prev1,              dd 0         ; previous value 2

NEWSYM lastbl,             dd 0
NEWSYM loopbl,             dd 0
usenoisedata       dd 0

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
NEWSYM DSPBuffer, resd 320*4
NEWSYM EchoBuffer, resd 320*4
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

NEWSYM Voice0Freq, resd 1
NEWSYM Voice1Freq, resd 1
NEWSYM Voice2Freq, resd 1
NEWSYM Voice3Freq, resd 1
NEWSYM Voice4Freq, resd 1
NEWSYM Voice5Freq, resd 1
NEWSYM Voice6Freq, resd 1
NEWSYM Voice7Freq, resd 1

NEWSYM Voice0Pitch, resw 1            ; Previous Pitch for Voice 0
NEWSYM Voice1Pitch, resw 1            ; Previous Pitch for Voice 1
NEWSYM Voice2Pitch, resw 1            ; Previous Pitch for Voice 2
NEWSYM Voice3Pitch, resw 1            ; Previous Pitch for Voice 3
NEWSYM Voice4Pitch, resw 1            ; Previous Pitch for Voice 4
NEWSYM Voice5Pitch, resw 1            ; Previous Pitch for Voice 5
NEWSYM Voice6Pitch, resw 1            ; Previous Pitch for Voice 6
NEWSYM Voice7Pitch, resw 1            ; Previous Pitch for Voice 7

NEWSYM Voice0Status,   resb 1
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

NEWSYM Voice0BufPtr,  resd 1
NEWSYM Voice1BufPtr,  resd 1
NEWSYM Voice2BufPtr,  resd 1
NEWSYM Voice3BufPtr,  resd 1
NEWSYM Voice4BufPtr,  resd 1
NEWSYM Voice5BufPtr,  resd 1
NEWSYM Voice6BufPtr,  resd 1
NEWSYM Voice7BufPtr,  resd 1

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

NEWSYM useless, resd 4    ;This is needed because of the stupid
                          ;alignment dependency in the savestates.

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

NEWSYM TimeTemp,   resd 8   ; 104 bytes
NEWSYM IncNTemp,   resd 8
NEWSYM EnvITemp,   resd 8
NEWSYM StatTemp,   resd 2

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

NEWSYM AdsrSustLevLoc, db 58,39,27,19,13,8,3,1

dspsave equ marksave-BRRBuffer
dspconvb equ marksave-Voice0Freq
dspsave2 equ marksave2-echoon0
NEWSYM PHdspsave, dd dspsave
NEWSYM PHdspconvb, dd dspconvb
NEWSYM PHdspsave2, dd dspsave2

section .bss
NEWSYM NoiseInc, resd 1
NEWSYM NoisePointer, resd 1
section .text

section .bss
NEWSYM powhack, resd 1
section .text

SECTION .data
ALIGN16
NEWSYM DSPInterpolate, dd 0


; ProcessSoundBuffer and LPFmonoloop have been ported to C (cpu/c_dspproc.c).
