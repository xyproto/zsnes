;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
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

EXTSYM regaccessbankr8,regaccessbankr16,regaccessbankw8,regaccessbankw16
EXTSYM DSPOp0A,Op0AA,Op0AB,Op0AC,Op0AD,Op0AVS,DSPOp10
EXTSYM debstop
EXTSYM DSPOp00,Op00Multiplicand,Op00Multiplier
EXTSYM Op00Result
;EXTSYM DSPOp10,Op10a,Op10b,Op10A,Op10B
EXTSYM DSPOp04,Op04Angle,Op04Cos,Op04Radius,Op04Sin
EXTSYM DSPOp28,Op28R,Op28X,Op28Y,Op28Z
EXTSYM DSPOp0C,Op0CA,Op0CX1,Op0CX2,Op0CY1,Op0CY2
EXTSYM DSPOp02,Op02AAS,Op02AZS,Op02CX,Op02CY,Op02FX,Op02FY
EXTSYM Op02FZ,Op02LES,Op02LFE,Op02VOF,Op02VVA
EXTSYM DSPOp06,Op06X,Op06Y,Op06Z,Op06H,Op06V,Op06S
EXTSYM DSPOp0E,Op0EH,Op0EV,Op0EX,Op0EY
EXTSYM Op01m, Op01Zr, Op01Xr, Op01Yr, DSPOp01
EXTSYM Op0DX, Op0DY, Op0DZ, Op0DF, Op0DL, Op0DU, DSPOp0D
EXTSYM Op03X, Op03Y, Op03Z, Op03F, Op03L, Op03U, DSPOp03
EXTSYM Op14Zr, Op14Xr, Op14Yr, Op14U, Op14F, Op14L
EXTSYM Op14Zrr,Op14Xrr,Op14Yrr, DSPOp14

NEWSYM Dsp1ProcAsmStart





NEWSYM dsp1ptr, dd 0
NEWSYM dsp1array, times 4096 db 0

;*******************************************************
; DSP1 Read Functions
;*******************************************************

NEWSYM DSP1Read8b3F
    test ecx,8000h
    jnz .dsp1area
    jmp regaccessbankr8
.dsp1area
    cmp ecx,0C000h
    jae .doC000
    mov al,080h
    ret
.doC000
    mov al,80h
    ret


NEWSYM DSP1Read16b3F
    test ecx,8000h
    jnz .dsp1area
    jmp regaccessbankr16
.dsp1area
    cmp ecx,0C000h
    jae .doC000
    cmp byte[DSP1RLeft],0
    jne .movestuff
    mov ax,08000h
    ret
.doC000
    mov ax,08000h
    ret
.movestuff
    push ebx
    xor ebx,ebx
    mov bl,[DSP1CPtrR]
    mov ax,[DSP1RET+ebx*2]
    pop ebx
    inc byte[DSP1CPtrR]
    dec byte[DSP1RLeft]
    jz .nomore
.goback
    ret
.nomore
    cmp byte[DSP1COp],0Ah
    jne .goback
    push eax
    pushad
    call DSPOp0A
    popad
    mov ax,[Op0AA]
    mov [DSP1RET],ax
    mov ax,[Op0AB]
    mov [DSP1RET+2],ax
    mov ax,[Op0AC]
    mov [DSP1RET+4],ax
    mov ax,[Op0AD]
    mov [DSP1RET+6],ax
    mov byte[DSP1RLeft],4
    mov byte[DSP1CPtrR],0
    pop eax
    ret

NEWSYM DSP1Read8b
;    mov byte[debstop],1
    cmp ecx,7000h
    jae .do7000
    xor al,al
    ret
.do7000
    mov al,80h
    test ecx,1
    jz .no1
    mov al,80h
.no1
    ret

NEWSYM DSP1Read16b
;    mov byte[debstop],1
    cmp ecx,7000h
    jae .do7000
    cmp byte[DSP1RLeft],0
    jne .movestuff
    mov ax,0FFFFh
    ret
.do7000
    mov ax,8000h
    test ecx,01h
    jz .norev
    mov ax,0080h
.norev
    ret
.movestuff
    push ebx
    xor ebx,ebx
    mov bl,[DSP1CPtrR]
    mov ax,[DSP1RET+ebx*2]
    pop ebx
    inc byte[DSP1CPtrR]
    dec byte[DSP1RLeft]
    jz .nomore
.goback
    ret
.nomore
    cmp byte[DSP1COp],0Ah
    jne .goback
    push eax
    pushad
    call DSPOp0A
    popad
    mov ax,[Op0AA]
    mov [DSP1RET],ax
    mov ax,[Op0AB]
    mov [DSP1RET+2],ax
    mov ax,[Op0AC]
    mov [DSP1RET+4],ax
    mov ax,[Op0AD]
    mov [DSP1RET+6],ax
    mov byte[DSP1RLeft],4
    mov byte[DSP1CPtrR],0
    pop eax
    ret

%macro DSP1WriteInit 2
    cmp al,%1
    jne %%no
    mov byte[DSP1WLeft],%2
%%no
%endmacro

NEWSYM DSP1Write8b3F
    test ecx,8000h
    jnz .dsp1area
    jmp regaccessbankw8
.dsp1area
    call DSP1Write8b
    ret

NEWSYM DSP1Write16b3F
    test ecx,8000h
    jnz .dsp1area
    jmp regaccessbankw16
.dsp1area
    call DSP1Write16b
    ret

NEWSYM DSP1Write8b
    push ebx
    xor ebx,ebx
    mov bl,al
    mov byte[DSPFuncUsed+ebx],1
    pop ebx
    mov byte[DSP1COp],al
    mov byte[DSP1CPtrW],0
    DSP1WriteInit 00h, 2  ; 16-bit multiply
    DSP1WriteInit 10h, 2  ; Inverse
    DSP1WriteInit 04h, 2  ; Trigonometric
    DSP1WriteInit 08h, 3  ; Vector Size
    DSP1WriteInit 18h, 4  ; Vector Size Comparison
    DSP1WriteInit 28h, 3  ; Vector Absolute Value
    DSP1WriteInit 0Ch, 3  ; Coordinate Rotation
    DSP1WriteInit 1Ch, 4  ; 3D Coordinate Rotation
    DSP1WriteInit 02h, 7  ; Vector Size
    DSP1WriteInit 0Ah, 1  ; Raster Data Calculation via DMA
    DSP1WriteInit 1Ah, 1  ; Raster Data Calculation w/o DMA
    DSP1WriteInit 06h, 3  ; Object Projection Calculation
    DSP1WriteInit 0Eh, 2  ; Coordinate Calculation of a point onscreen
    DSP1WriteInit 01h, 4  ; Set Attitude Matrix A
    DSP1WriteInit 11h, 4  ; Set Attitude Matrix B
    DSP1WriteInit 21h, 4  ; Set Attitude Matrix C
    DSP1WriteInit 0Dh, 3  ; Convert from global to object coords Matrix A
    DSP1WriteInit 1Dh, 3  ; Convert from global to object coords Matrix B
    DSP1WriteInit 2Dh, 3  ; Convert from global to object coords Matrix C
    DSP1WriteInit 03h, 3  ; Convert from object to global coords Matrix A
    DSP1WriteInit 13h, 3  ; Convert from object to global coords Matrix B
    DSP1WriteInit 23h, 3  ; Convert from object to global coords Matrix C
    DSP1WriteInit 0Bh, 3  ; Calculation of inner product Matrix A
    DSP1WriteInit 1Bh, 3  ; Calculation of inner product Matrix B
    DSP1WriteInit 2Bh, 3  ; Calculation of inner product Matrix C
    DSP1WriteInit 14h, 6  ; 3D angle rotation
    ret

%macro DSP1WriteProc 2
    cmp byte[DSP1COp],%1
    jne %%no
    pushad
    call %2
    popad
%%no
%endmacro

NEWSYM DSP1Write16b
;    mov byte[debstop],1
    cmp byte[DSP1WLeft],0
    jne .yesleft
    ret
.yesleft
    push ebx
    xor ebx,ebx
    mov bl,[DSP1CPtrW]
    mov [DSP1VARS+ebx*2],ax
    pop ebx
    inc byte[DSP1CPtrW]
    dec byte[DSP1WLeft]
    jz .ProcessDSP1
    ret
.ProcessDSP1
    mov byte[DSP1CPtrR],0
    mov byte[DSP1RLeft],0
    DSP1WriteProc 00h, DSP1_00  ; 16-bit multiply
    DSP1WriteProc 10h, DSP1_10  ; Inverse
    DSP1WriteProc 04h, DSP1_04  ; Trigonometric
    DSP1WriteProc 08h, DSP1_08  ; Vector Size
    DSP1WriteProc 18h, DSP1_18  ; Vector Size Comparison
    DSP1WriteProc 28h, DSP1_28  ; Vector Absolute Value
    DSP1WriteProc 0Ch, DSP1_0C  ; Coordinate Rotation
    DSP1WriteProc 1Ch, DSP1_1C  ; 3D Coordinate Rotation
    DSP1WriteProc 02h, DSP1_02  ; Vector Size
    DSP1WriteProc 0Ah, DSP1_0A  ; Raster Data Calculation via DMA
    DSP1WriteProc 1Ah, DSP1_0A  ; Raster Data Calculation w/o DMA
    DSP1WriteProc 06h, DSP1_06  ; Object Projection Calculation
    DSP1WriteProc 0Eh, DSP1_0E  ; Coordinate Calculation of a point onscreen
    DSP1WriteProc 01h, DSP1_01  ; Set Attitude Matrix A
    DSP1WriteProc 11h, DSP1_11  ; Set Attitude Matrix B
    DSP1WriteProc 21h, DSP1_21  ; Set Attitude Matrix C
    DSP1WriteProc 0Dh, DSP1_0D  ; Convert from global to object coords Matrix A
    DSP1WriteProc 1Dh, DSP1_1D  ; Convert from global to object coords Matrix B
    DSP1WriteProc 2Dh, DSP1_2D  ; Convert from global to object coords Matrix C
    DSP1WriteProc 03h, DSP1_03  ; Convert from object to global coords Matrix A
    DSP1WriteProc 13h, DSP1_13  ; Convert from object to global coords Matrix B
    DSP1WriteProc 23h, DSP1_23  ; Convert from object to global coords Matrix C
    DSP1WriteProc 0Bh, DSP1_0B  ; Calculation of inner product Matrix A
    DSP1WriteProc 1Bh, DSP1_1B  ; Calculation of inner product Matrix B
    DSP1WriteProc 2Bh, DSP1_2B  ; Calculation of inner product Matrix C
    DSP1WriteProc 14h, DSP1_14  ; 3D angle rotation
    ret

NEWSYM DSP1COp,   db 0
NEWSYM DSP1RLeft, db 0
NEWSYM DSP1WLeft, db 0
NEWSYM DSP1CPtrW, db 0
NEWSYM DSP1CPtrR, db 0
NEWSYM DSP1VARS,  times 16 dw 0
NEWSYM DSP1RET,   times 16 dw 0
NEWSYM DSPDet,    db 0

NEWSYM DSPFuncUsed, times 256 db 0

;*******************************************************
; DSP1 Conversion Functions
;*******************************************************
DSP1_00:  ; 16-bit multiply
    or byte[DSPDet],01h
    push eax
    mov ax,[DSP1VARS]
    mov [Op00Multiplicand],ax
    mov ax,[DSP1VARS+2]
    mov [Op00Multiplier],ax
    pushad
    call DSPOp00
    popad
    mov ax,[Op00Result]
    mov [DSP1RET],ax
    mov byte[DSP1RLeft],1
    pop eax
    ret

EXTSYM Op10Exponent, Op10ExponentR
EXTSYM Op10Coefficient, Op10CoefficientR
DSP1_10:  ; Inverse
    push eax
    mov ax,[DSP1VARS]
    mov [Op10Coefficient],ax
    mov ax,[DSP1VARS+2]
    mov [Op10Exponent],ax
    pushad
    call DSPOp10
    popad
    mov ax,[Op10CoefficientR]
    mov [DSP1RET],ax
    mov ax,[Op10ExponentR]
    mov [DSP1RET+2],ax
    mov byte[DSP1RLeft],2
    pop eax
    ret

DSP1_04:  ; Trigonometric
    or byte[DSPDet],02h
    push eax
    mov ax,[DSP1VARS]
    mov [Op04Angle],ax
    mov ax,[DSP1VARS+2]
    mov [Op04Radius],ax
    pushad
    call DSPOp04
    popad
    mov ax,[Op04Sin]
    mov [DSP1RET],ax
    mov ax,[Op04Cos]
    mov [DSP1RET+2],ax
    mov byte[DSP1RLeft],2
    pop eax
    ret
DSP1_08:  ; Vector Size
    ret
DSP1_18:  ; Vector Size Comparison
    ret

DSP1_28:  ; Vector Absolute Value
    or byte[DSPDet],04h
    push eax
    mov ax,[DSP1VARS]
    mov [Op28X],ax
    mov ax,[DSP1VARS+2]
    mov [Op28Y],ax
    mov ax,[DSP1VARS+4]
    mov [Op28Z],ax
    pushad
    call DSPOp28
    popad
    mov ax,[Op28R]
    mov [DSP1RET],ax
    mov byte[DSP1RLeft],1
    pop eax
    ret


DSP1_0C:  ; Coordinate Rotation
    or byte[DSPDet],08h
    push eax
    mov ax,[DSP1VARS]
    mov [Op0CA],ax
    mov ax,[DSP1VARS+2]
    mov [Op0CX1],ax
    mov ax,[DSP1VARS+4]
    mov [Op0CY1],ax
    pushad
    call DSPOp0C
    popad
    mov ax,[Op0CX2]
    mov [DSP1RET],ax
    mov ax,[Op0CY2]
    mov [DSP1RET+2],ax
    mov byte[DSP1RLeft],2
    pop eax
    ret
DSP1_1C:  ; 3D Coordinate Rotation
    ret


DSP1_02:  ; Vector Size
    or byte[DSPDet],10h
    push eax
;Op02FX           dw 0
;Op02FY           dw 0
;Op02FZ           dw 0
;Op02LFE          dw 0
;Op02LES          dw 0
;Op02AAS          dw 0
;Op02AZS          dw 0
    mov ax,[DSP1VARS]
    mov [Op02FX],ax
    mov ax,[DSP1VARS+2]
    mov [Op02FY],ax
    mov ax,[DSP1VARS+4]
    mov [Op02FZ],ax
    mov ax,[DSP1VARS+6]
    mov [Op02LFE],ax
    mov ax,[DSP1VARS+8]
    mov [Op02LES],ax
    mov ax,[DSP1VARS+10]
    mov [Op02AAS],ax
    mov ax,[DSP1VARS+12]
    mov [Op02AZS],ax
    pushad
    call DSPOp02
    popad
;Op02VOF          dw 0
;Op02VVA          dw 0
;Op02CX           dw 0
;Op02CY           dw 0
    mov ax,[Op02VOF]
    mov [DSP1RET],ax
    mov ax,[Op02VVA]
    mov [DSP1RET+2],ax
    mov ax,[Op02CX]
    mov [DSP1RET+4],ax
    mov ax,[Op02CY]
    mov [DSP1RET+6],ax
    mov byte[DSP1RLeft],4
    pop eax
    ret
    mov eax,dsp1array
    add eax,[dsp1ptr]
    push ebx
    mov byte[eax],02h
    mov bx,[Op02FX]
    mov [eax+1],bx
    mov bx,[Op02FY]
    mov [eax+3],bx
    mov bx,[Op02FZ]
    mov [eax+5],bx
    mov bx,[Op02LFE]
    mov [eax+7],bx
    mov bx,[Op02LES]
    mov [eax+9],bx
    mov bx,[Op02AAS]
    mov [eax+11],bx
    mov bx,[Op02AZS]
    mov [eax+13],bx
    mov bx,[Op02VOF]
    mov [eax+15],bx
    mov bx,[Op02VVA]
    mov [eax+17],bx
    mov bx,[Op02CX]
    mov [eax+19],bx
    mov bx,[Op02CY]
    mov [eax+21],bx
    pop ebx
    add dword[dsp1ptr],23
DSP1_0A:  ; Raster Data Calculation via DMA
    mov byte[DSP1COp],0Ah
    or byte[DSPDet],20h
    push eax
    mov ax,[DSP1VARS]
    mov [Op0AVS],ax
    pushad
    call DSPOp0A
    popad
    mov ax,[Op0AA]
    mov [DSP1RET],ax
    mov ax,[Op0AB]
    mov [DSP1RET+2],ax
    mov ax,[Op0AC]
    mov [DSP1RET+4],ax
    mov ax,[Op0AD]
    mov [DSP1RET+6],ax
    mov byte[DSP1RLeft],4
    pop eax
    ret
DSP1_06:  ; Object Projection Calculation
    or byte[DSPDet],40h
    push eax
    mov ax,[DSP1VARS]
    mov [Op06X],ax
    mov ax,[DSP1VARS+2]
    mov [Op06Y],ax
    mov ax,[DSP1VARS+4]
    mov [Op06Z],ax
    pushad
    call DSPOp06
    popad
    mov ax,[Op06H]
    mov word[DSP1RET],ax
    mov ax,[Op06V]
    mov word[DSP1RET+2],ax
    mov ax,[Op06S]
    mov word[DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret
    mov eax,dsp1array
    add eax,[dsp1ptr]
    push ebx
    mov byte[eax],06h
    mov bx,[Op06X]
    mov [eax+1],bx
    mov bx,[Op06Y]
    mov [eax+3],bx
    mov bx,[Op06Z]
    mov [eax+5],bx
    mov bx,[Op06H]
    mov [eax+7],bx
    mov bx,[Op06V]
    mov [eax+9],bx
    mov bx,[Op06S]
    mov [eax+11],bx
    pop ebx
    add dword[dsp1ptr],13
DSP1_0E:  ; Coordinate Calculation of a point onscreen
    push eax
    mov ax,[DSP1VARS]
    mov [Op0EH],ax
    mov ax,[DSP1VARS+2]
    mov [Op0EV],ax
    pushad
    call DSPOp0E
    popad
    mov ax,[Op0EX]
    mov word[DSP1RET],ax
    mov ax,[Op0EY]
    mov word[DSP1RET+2],ax
    mov byte[DSP1RLeft],2
    pop eax
    ret
DSP1_01:  ; Set Attitude Matrix A
    push eax
    mov ax,[DSP1VARS]
    mov [Op01m],ax
    mov ax,[DSP1VARS+2]
    mov [Op01Zr],ax
    mov ax,[DSP1VARS+4]
    mov [Op01Xr],ax
    mov ax,[DSP1VARS+6]
    mov [Op01Yr],ax
    pushad
    call DSPOp01
    popad
    pop eax
    ret
DSP1_11:  ; Set Attitude Matrix B
    ret
DSP1_21:  ; Set Attitude Matrix C
    ret
DSP1_0D:  ; Convert from global to object coords Matrix A
    push eax
    mov ax,[DSP1VARS]
    mov [Op0DX],ax
    mov ax,[DSP1VARS+2]
    mov [Op0DY],ax
    mov ax,[DSP1VARS+4]
    mov [Op0DZ],ax
    pushad
    call DSPOp0D
    popad
    mov ax,[Op0DF]
    mov word[DSP1RET],ax
    mov ax,[Op0DL]
    mov word[DSP1RET+2],ax
    mov ax,[Op0DU]
    mov word[DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret
DSP1_1D:  ; Convert from global to object coords Matrix B
    ret
DSP1_2D:  ; Convert from global to object coords Matrix C
    ret
DSP1_03:  ; Convert from object to global coords Matrix A
    push eax
    mov ax,[DSP1VARS]
    mov [Op03F],ax
    mov ax,[DSP1VARS+2]
    mov [Op03L],ax
    mov ax,[DSP1VARS+4]
    mov [Op03U],ax
    pushad
    call DSPOp03
    popad
    mov ax,[Op03X]
    mov word[DSP1RET],ax
    mov ax,[Op03Y]
    mov word[DSP1RET+2],ax
    mov ax,[Op03Z]
    mov word[DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret
DSP1_13:  ; Convert from object to global coords Matrix B
    ret
DSP1_23:  ; Convert from object to global coords Matrix C
    ret
DSP1_0B:  ; Calculation of inner product Matrix A
    ret
DSP1_1B:  ; Calculation of inner product Matrix B
    ret
DSP1_2B:  ; Calculation of inner product Matrix C
    ret
DSP1_14:  ; 3D angle rotation
    push eax
    mov ax,[DSP1VARS]
    mov [Op14Zr],ax
    mov ax,[DSP1VARS+2]
    mov [Op14Xr],ax
    mov ax,[DSP1VARS+4]
    mov [Op14Yr],ax
    mov ax,[DSP1VARS+6]
    mov [Op14U],ax
    mov ax,[DSP1VARS+8]
    mov [Op14F],ax
    mov ax,[DSP1VARS+10]
    mov [Op14L],ax
    pushad
    call DSPOp14
    popad
    mov ax,[Op14Zrr]
    mov word[DSP1RET],ax
    mov ax,[Op14Xrr]
    mov word[DSP1RET+2],ax
    mov ax,[Op14Yrr]
    mov word[DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret

NEWSYM Dsp1ProcAsmEnd
