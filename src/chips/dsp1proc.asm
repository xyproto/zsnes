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

EXTSYM regaccessbankr8,regaccessbankr16,regaccessbankw8,regaccessbankw16
EXTSYM DSPOp0A,Op0AA,Op0AB,Op0AC,Op0AD,Op0AVS,DSPOp10,DSPOp00,Op00Multiplicand
EXTSYM Op00Multiplier,Op00Result,DSPOp0F,Op0FPass
EXTSYM DSPOp04,Op04Angle,Op04Cos,Op04Radius,Op04Sin
EXTSYM DSPOp28,Op28R,Op28X,Op28Y,Op28Z
EXTSYM DSPOp0C,Op0CA,Op0CX1,Op0CX2,Op0CY1,Op0CY2
EXTSYM DSPOp02,Op02AAS,Op02AZS,Op02CX,Op02CY,Op02FX,Op02FY
EXTSYM Op02FZ,Op02LES,Op02LFE,Op02VOF,Op02VVA
EXTSYM DSPOp06,Op06X,Op06Y,Op06Z,Op06H,Op06V,Op06M
EXTSYM DSPOp0E,Op0EH,Op0EV,Op0EX,Op0EY
EXTSYM Op01m,Op01Zr,Op01Xr,Op01Yr,DSPOp01
EXTSYM Op11m,Op11Zr,Op11Xr,Op11Yr,DSPOp11
EXTSYM Op21m,Op21Zr,Op21Xr,Op21Yr,DSPOp21
EXTSYM Op0DX,Op0DY,Op0DZ,Op0DF,Op0DL,Op0DU,DSPOp0D
EXTSYM Op1DX,Op1DY,Op1DZ,Op1DF,Op1DL,Op1DU,DSPOp1D
EXTSYM Op2DX,Op2DY,Op2DZ,Op2DF,Op2DL,Op2DU,DSPOp2D
EXTSYM Op03X,Op03Y,Op03Z,Op03F,Op03L,Op03U,DSPOp03
EXTSYM Op13X,Op13Y,Op13Z,Op13F,Op13L,Op13U,DSPOp13
EXTSYM Op23X,Op23Y,Op23Z,Op23F,Op23L,Op23U,DSPOp23
EXTSYM Op14Zr,Op14Xr,Op14Yr,Op14U,Op14F,Op14L
EXTSYM Op14Zrr,Op14Xrr,Op14Yrr,DSPOp14
EXTSYM Op0BX,Op0BY,Op0BZ,Op0BS,DSPOp0B
EXTSYM Op1BX,Op1BY,Op1BZ,Op1BS,DSPOp1B
EXTSYM Op2BX,Op2BY,Op2BZ,Op2BS,DSPOp2B
EXTSYM Op08X,Op08Y,Op08Z,Op08Ll,Op08Lh,DSPOp08
EXTSYM Op18X,Op18Y,Op18Z,Op18R,Op18D,DSPOp18
EXTSYM Op1CX,Op1CY,Op1CZ,Op1CXBR,Op1CYBR,Op1CZBR,Op1CXAR,Op1CYAR,Op1CZAR,DSPOp1C
EXTSYM Op10Exponent,Op10ExponentR,Op10Coefficient,Op10CoefficientR

SECTION .bss
NEWSYM dsp1ptr, resd 1
NEWSYM dsp1array, resb 4096

SECTION .text

;*******************************************************
; DSP1 Read Functions
;*******************************************************

NEWSYM DSP1Read8b3F
    test ecx,8000h
    jnz .dsp1area
    cmp bl,0E0h
    je .dsp1area
    jmp regaccessbankr8
.dsp1area
    mov al,80h
    ret

NEWSYM DSP1Read16b3F
    test ecx,8000h
    jnz .dsp1area
    cmp bl,0E0h
    je .dsp1area
    jmp regaccessbankr16
.dsp1area
    or ecx,08000h
    cmp ecx,0C000h
    jae .doC000
    cmp byte[DSP1RLeft],0
    jne .movestuff
    xor ax,ax
    ret
.doC000
    mov ax,08000h
    cmp byte[DSP1WLeft],0
    je .notwleft
    mov ax,0C000h
.notwleft
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
    xor ax,ax
    ret
.do7000
    mov ax,8000h
    cmp byte[DSP1WLeft],0
    je .notwleft
    mov ax,0C000h
.notwleft
;    test ecx,01h
;    jz .norev
;    mov ax,0080h
;.norev
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
    cmp bl,0E0h
    je .dsp1area
    jmp regaccessbankw8
.dsp1area
    call DSP1Write8b
    ret

NEWSYM DSP1Write16b3F
    test ecx,8000h
    jnz .dsp1area
    cmp bl,0E0h
    je .dsp1area
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
    mov [DSP1COp],al
    mov byte[DSP1CPtrW],0
    DSP1WriteInit 00h, 2  ; 16-bit multiply
    DSP1WriteInit 10h, 2  ; Inverse
    DSP1WriteInit 04h, 2  ; Trigonometric
    DSP1WriteInit 08h, 3  ; Vector Size
    DSP1WriteInit 18h, 4  ; Vector Size Comparison
    DSP1WriteInit 28h, 3  ; Vector Absolute Value
    DSP1WriteInit 0Ch, 3  ; Coordinate Rotation
    DSP1WriteInit 1Ch, 6  ; 3D Coordinate Rotation
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
    DSP1WriteInit 0Fh, 1  ; DSP RAM Check
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
    DSP1WriteProc 0Fh, DSP1_0F  ; DSP RAM Check
    ret

SECTION .bss
NEWSYM DSP1COp,   resb 1
NEWSYM DSP1RLeft, resb 1
NEWSYM DSP1WLeft, resb 1
NEWSYM DSP1CPtrW, resb 1
NEWSYM DSP1CPtrR, resb 1
NEWSYM DSP1VARS,  resw 16
NEWSYM DSP1RET,   resw 16
NEWSYM DSPDet,    resb 1

NEWSYM DSPFuncUsed, resb 256

SECTION .text

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
    push eax
    mov ax,[DSP1VARS]
    mov [Op08X],ax
    mov ax,[DSP1VARS+2]
    mov [Op08Y],ax
    mov ax,[DSP1VARS+4]
    mov [Op08Z],ax
    pushad
    call DSPOp08
    popad
    mov ax,[Op08Ll]
    mov [DSP1RET],ax
    mov ax,[Op08Lh]
    mov [DSP1RET+2],ax
    mov byte[DSP1RLeft],2
    pop eax
    ret

DSP1_18:  ; Vector Size Comparison
    push eax
    mov ax,[DSP1VARS]
    mov [Op18X],ax
    mov ax,[DSP1VARS+2]
    mov [Op18Y],ax
    mov ax,[DSP1VARS+4]
    mov [Op18Z],ax
    mov ax,[DSP1VARS+6]
    mov [Op18R],ax
    pushad
    call DSPOp18
    popad
    mov ax,[Op18D]
    mov [DSP1RET],ax
    mov byte[DSP1RLeft],1
    pop eax
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
    push eax
    mov ax,[DSP1VARS]
    mov [Op1CZ],ax
    mov ax,[DSP1VARS+2]
    mov [Op1CY],ax
    mov ax,[DSP1VARS+4]
    mov [Op1CX],ax
    mov ax,[DSP1VARS+6]
    mov [Op1CXBR],ax
    mov ax,[DSP1VARS+8]
    mov [Op1CYBR],ax
    mov ax,[DSP1VARS+10]
    mov [Op1CZBR],ax
    pushad
    call DSPOp1C
    popad
    mov ax,[Op1CXAR]
    mov [DSP1RET],ax
    mov ax,[Op1CYAR]
    mov [DSP1RET+2],ax
    mov ax,[Op1CZAR]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret

DSP1_02:  ; Vector Size
    or byte[DSPDet],10h
    push eax
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
    mov [DSP1RET],ax
    mov ax,[Op06V]
    mov [DSP1RET+2],ax
    mov ax,[Op06M]
    mov [DSP1RET+4],ax
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
    mov bx,[Op06M]
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
    mov [DSP1RET],ax
    mov ax,[Op0EY]
    mov [DSP1RET+2],ax
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
    mov [Op01Yr],ax
    mov ax,[DSP1VARS+6]
    mov [Op01Xr],ax
    pushad
    call DSPOp01
    popad
    pop eax
    ret

DSP1_11:  ; Set Attitude Matrix B
    push eax
    mov ax,[DSP1VARS]
    mov [Op11m],ax
    mov ax,[DSP1VARS+2]
    mov [Op11Zr],ax
    mov ax,[DSP1VARS+4]
    mov [Op11Yr],ax
    mov ax,[DSP1VARS+6]
    mov [Op11Xr],ax
    pushad
    call DSPOp11
    popad
    pop eax
    ret

DSP1_21:  ; Set Attitude Matrix C
    push eax
    mov ax,[DSP1VARS]
    mov [Op21m],ax
    mov ax,[DSP1VARS+2]
    mov [Op21Zr],ax
    mov ax,[DSP1VARS+4]
    mov [Op21Yr],ax
    mov ax,[DSP1VARS+6]
    mov [Op21Xr],ax
    pushad
    call DSPOp21
    popad
    pop eax
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
    mov [DSP1RET],ax
    mov ax,[Op0DL]
    mov [DSP1RET+2],ax
    mov ax,[Op0DU]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret

DSP1_0F:  ; DSP RAM Test
    push eax
    mov ax,[DSP1VARS]
    pushad
    call DSPOp0F
    popad
    mov ax,[Op0FPass]
    mov [DSP1RET],ax
    mov byte[DSP1RLeft],1
    pop eax
    ret

DSP1_1D:  ; Convert from global to object coords Matrix B
    push eax
    mov ax,[DSP1VARS]
    mov [Op1DX],ax
    mov ax,[DSP1VARS+2]
    mov [Op1DY],ax
    mov ax,[DSP1VARS+4]
    mov [Op1DZ],ax
    pushad
    call DSPOp1D
    popad
    mov ax,[Op1DF]
    mov [DSP1RET],ax
    mov ax,[Op1DL]
    mov [DSP1RET+2],ax
    mov ax,[Op1DU]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret

DSP1_2D:  ; Convert from global to object coords Matrix C
    push eax
    mov ax,[DSP1VARS]
    mov [Op2DX],ax
    mov ax,[DSP1VARS+2]
    mov [Op2DY],ax
    mov ax,[DSP1VARS+4]
    mov [Op2DZ],ax
    pushad
    call DSPOp2D
    popad
    mov ax,[Op2DF]
    mov [DSP1RET],ax
    mov ax,[Op2DL]
    mov [DSP1RET+2],ax
    mov ax,[Op2DU]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
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
    mov [DSP1RET],ax
    mov ax,[Op03Y]
    mov [DSP1RET+2],ax
    mov ax,[Op03Z]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret

DSP1_13:  ; Convert from object to global coords Matrix B
    push eax
    mov ax,[DSP1VARS]
    mov [Op13F],ax
    mov ax,[DSP1VARS+2]
    mov [Op13L],ax
    mov ax,[DSP1VARS+4]
    mov [Op13U],ax
    pushad
    call DSPOp13
    popad
    mov ax,[Op13X]
    mov [DSP1RET],ax
    mov ax,[Op13Y]
    mov [DSP1RET+2],ax
    mov ax,[Op13Z]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret

DSP1_23:  ; Convert from object to global coords Matrix C
    push eax
    mov ax,[DSP1VARS]
    mov [Op23F],ax
    mov ax,[DSP1VARS+2]
    mov [Op23L],ax
    mov ax,[DSP1VARS+4]
    mov [Op23U],ax
    pushad
    call DSPOp23
    popad
    mov ax,[Op23X]
    mov [DSP1RET],ax
    mov ax,[Op23Y]
    mov [DSP1RET+2],ax
    mov ax,[Op23Z]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret

DSP1_0B:  ; Calculation of inner product Matrix A
    push eax
    mov ax,[DSP1VARS]
    mov [Op0BX],ax
    mov ax,[DSP1VARS+2]
    mov [Op0BY],ax
    mov ax,[DSP1VARS+4]
    mov [Op0BZ],ax
    pushad
    call DSPOp0B
    popad
    mov ax,[Op0BS]
    mov [DSP1RET],ax
    mov byte[DSP1RLeft],1
    pop eax
    ret

DSP1_1B:  ; Calculation of inner product Matrix B
    push eax
    mov ax,[DSP1VARS]
    mov [Op1BX],ax
    mov ax,[DSP1VARS+2]
    mov [Op1BY],ax
    mov ax,[DSP1VARS+4]
    mov [Op1BZ],ax
    pushad
    call DSPOp1B
    popad
    mov ax,[Op1BS]
    mov [DSP1RET],ax
    mov byte[DSP1RLeft],1
    pop eax
    ret

DSP1_2B:  ; Calculation of inner product Matrix C
    push eax
    mov ax,[DSP1VARS]
    mov [Op2BX],ax
    mov ax,[DSP1VARS+2]
    mov [Op2BY],ax
    mov ax,[DSP1VARS+4]
    mov [Op2BZ],ax
    pushad
    call DSPOp2B
    popad
    mov ax,[Op2BS]
    mov [DSP1RET],ax
    mov byte[DSP1RLeft],1
    pop eax
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
    mov [DSP1RET],ax
    mov ax,[Op14Xrr]
    mov [DSP1RET+2],ax
    mov ax,[Op14Yrr]
    mov [DSP1RET+4],ax
    mov byte[DSP1RLeft],3
    pop eax
    ret
