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

%include "macros.mac"

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

NEWSYM BRRDecode
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

; used only in dspproc.asm
SECTION .data
ALIGN32

Filter dd 0,0,240,0,488,-240,460,-208

NEWSYM prev0,              dd 0         ; previous value 1
NEWSYM prev1,              dd 0         ; previous value 2
nextsamp           dd 0         ; next sample
filter0            dd 0         ; filter 0
filter1            dd 0         ; filter 1
bshift             dd 0
sampleleft         dd 0         ; 8 bytes/sample

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
NEWSYM powhack, resd 1
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
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    ccall [DSPInterpolate], edx, ebp
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
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    ccall [DSPInterpolate], edx, ebp
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
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    ccall [DSPInterpolate], edx, ebp
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
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
;    mov ax,[edi+edx*2]
    ccall [DSPInterpolate], edx, ebp
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

    ccallv ProcessVoiceHandler16, 0
    ccallv ProcessVoiceHandler16, 1
    ccallv ProcessVoiceHandler16, 2
    ccallv ProcessVoiceHandler16, 3
    ccallv ProcessVoiceHandler16, 4
    ccallv ProcessVoiceHandler16, 5
    ccallv ProcessVoiceHandler16, 6
    ccallv ProcessVoiceHandler16, 7

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
    ccallv MixEcho2
    jmp .echowritten
.echonotokay
    ccallv MixEcho
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
    je .noLPFexit
    ccallv LPFexit
    ret
.noLPFexit
    mov esi,DSPBuffer
    cmp byte[StereoSound],1
    jne .noLPFstereo
    ccallv LPFstereo, esi
    ret
.noLPFstereo
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
    ccallv LPFexit
    ret
