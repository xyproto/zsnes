;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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

EXTSYM romdata,sramb4save,pressed,vidbuffer,oamram
EXTSYM C4TransfWireFrame2
EXTSYM C4WFXVal,C4WFYVal,C4WFX2Val,C4WFY2Val,C4CalcWireFrame
EXTSYM C4WFDist,C4WFScale,C4TransfWireFrame,C4WFZVal
EXTSYM C41FXVal,C41FYVal,C41FAngleRes,C41FDist,C4Op1F,C4Op15
EXTSYM C41FDistVal,C4Op0D,C4Op22,SinTable,CosTable
EXTSYM SFXEnable,regptra,sfxramdata,snesmmap,wramdataa,C4Ram,C4Enable
EXTSYM C4RamR,C4RamW,snesmap2,SPC7110Enable
EXTSYM DSP1Read16b
EXTSYM DSP1Write8b,regptwa,writeon
EXTSYM Bank0datr8,Bank0datw8,Bank0datr16,Bank0datw16,xd,SA1xd
EXTSYM DSP1Read8b,DSP1Type,SA1Enable
EXTSYM DSP1Write16b
EXTSYM CurDecompPtr,PrevDecompPtr,CurDecompSize
EXTSYM SPCDecmPtr,SPCCompPtr,SPCCompCounter
EXTSYM ramsize,ramsizeand,sram
EXTSYM ram7fa
EXTSYM DosExit,invalid,invopcd,previdmode,printhex8
EXTSYM SA1Status,IRAM,CurBWPtr,SA1RAMArea
EXTSYM SA1Overflow,OBCEnable
EXTSYM Sdd1Mode,Sdd1Bank,Sdd1Addr,Sdd1NewAddr,memtabler8,AddrNoIncr,SDD1BankA

; C4SprScale

;*******************************************************
; Register & Memory Access Banks (0 - 3F) / (80 - BF)
;*******************************************************
; enter : BL = bank number, CX = address location
; leave : AL = value read


; ******************************************************
; C4 Emulation, reverse engineered & written by zsKnight
; ******************************************************

SECTION .text

C4ProcessVectors:
    mov esi,[C4Ram]
    mov edi,esi
    add edi,1F8Ch
    xor edx,edx
    mov dx,[esi+1F8Ah]
    cmp dx,128
    ja .ret
    cmp dx,0
    jne .nozero
.ret
    ret
.nozero
    cmp dx,10h
    jb .less
    mov dx,10h
.less
    mov esi,[C4Ram]
    add esi,800h
.loop
    mov ecx,100h
    xor ebx,ebx
    xor eax,eax
    movsx bx,byte[edi]
.spotloop
    add ah,80h
    mov [esi],ah
    sub ah,80h
    add ax,bx
    inc esi
    dec ecx
    jnz .spotloop
    add edi,3
    dec dx
    jnz .loop
    ret

section .data
C4ColRot db 1

SECTION .bss
C4ObjDisp resd 1
NEWSYM C4ObjSelec, resb 1
NEWSYM C4SObjSelec, resb 1
NEWSYM C4Pause, resb 1
C4DataCopy resb 64
CObjNum resw 1
C4Temp resd 1
section .text

C4Edit:
    ; C4 editing routines
    ; Register keyboard presses
    ;  [ = prev object, ] = next object
    ;  p = pause/unpause
    cmp byte[pressed+1Ah],0
    je .notpressed
    mov byte[pressed+1Ah],0
    inc byte[C4ObjSelec]
    inc byte[C4Temp]
.notpressed
    cmp byte[pressed+1Bh],0
    je .notpressed2
    mov byte[pressed+1Bh],0
    dec byte[C4ObjSelec]
    dec byte[C4Temp]
.notpressed2
    cmp byte[pressed+19h],0
    je .notpressed3
    mov byte[pressed+19h],0
    xor byte[C4Pause],1
.notpressed3

    ; Setup variables
    mov esi,[C4Ram]
    add byte[C4ColRot],16
    mov al,[esi+620h]
    cmp byte[C4ObjSelec],0FFh
    jne .notneg
    dec al
    mov [C4ObjSelec],al
    jmp .notof
.notneg
    cmp byte[C4ObjSelec],al
    jb .notof
    xor al,al
    mov [C4ObjSelec],al
.notof

    ; Draw the dots on-screen
    xor eax,eax
    mov al,[C4ObjSelec]
    shl eax,4
    add eax,[C4Ram]
    add eax,220h
    mov byte[.flipped],0
    test byte[eax+6],40h
    jz .notflip
    mov byte[.flipped],1
.notflip

;              00/01 - x position relative to BG scroll value
;              02/03 - y position relative to BG scroll value
;              04    - palette/priority settings
;              05    - OAM pointer value
;              06    - flip settings : b6 = flipx, b7 = flipy
;              07    - looks like some sprite displacement values
;              08/09 - ???
;              0A-0F - unused
    xor ebx,ebx
    mov bx,[eax+8]
    mov [CObjNum],bx
    cmp bx,4096
    jae near .skipall
    shl ebx,6
    add ebx,[C4Data]

    ; t,f,g,h = move current object
    ; q = copy current object structure, w = paste current object structure
    cmp byte[pressed+14h],0
    je .notmove
    mov byte[pressed+14h],0
    pushad
    mov ecx,15
.next
    add ebx,4
    dec byte[ebx+1]
    dec ecx
    jnz .next
    popad
.notmove
    cmp byte[pressed+21h],0
    je .notmove2
    mov byte[pressed+21h],0
    pushad
    mov ecx,15
.next2
    add ebx,4
    cmp byte[.flipped],0
    je .noflipx
    add byte[ebx],2
.noflipx
    dec byte[ebx]
    dec ecx
    jnz .next2
    popad
.notmove2
    cmp byte[pressed+22h],0
    je .notmove3
    mov byte[pressed+22h],0
    pushad
    mov ecx,15
.next3
    add ebx,4
    inc byte[ebx+1]
    dec ecx
    jnz .next3
    popad
.notmove3
    cmp byte[pressed+23h],0
    je .notmove4
    mov byte[pressed+23h],0
    pushad
    mov ecx,15
.next4
    add ebx,4
    cmp byte[.flipped],0
    je .noflipx2
    sub byte[ebx],2
.noflipx2
    inc byte[ebx]
    dec ecx
    jnz .next4
    popad
.notmove4
    cmp byte[pressed+10h],0
    je .notcopy
    mov byte[pressed+10h],0
    pushad
    mov edx,C4DataCopy
    mov ecx,64
.copylp
    mov al,[ebx]
    mov [edx],al
    inc ebx
    inc edx
    dec ecx
    jnz .copylp
    popad
.notcopy
    cmp byte[pressed+11h],0
    je .notpaste
    mov byte[pressed+11h],0
    pushad
    mov edx,C4DataCopy
    mov ecx,64
.pastelp
    mov al,[edx]
    mov [ebx],al
    inc ebx
    inc edx
    dec ecx
    jnz .pastelp
    popad
.notpaste

    ;  - = remove sub-object, + = add sub-object
    ;  ; = previous sub-object, ' = next sub-object
    cmp byte[pressed+0Ch],0
    je .notpressed4
    mov byte[pressed+0Ch],0
    cmp byte[ebx],0
    je .notpressed4
    dec byte[ebx]
.notpressed4
    cmp byte[pressed+0Dh],0
    je .notpressed5
    mov byte[pressed+0Dh],0
    cmp byte[ebx],15
    je .notpressed5
    inc byte[ebx]
.notpressed5
    cmp byte[pressed+27h],0
    je .notpressed6
    mov byte[pressed+27h],0
    dec byte[C4SObjSelec]
.notpressed6
    cmp byte[pressed+28h],0
    je .notpressed7
    mov byte[pressed+28h],0
    inc byte[C4SObjSelec]
.notpressed7

    ; get current sub-object displacement (0 if no sub-objects)
    xor ecx,ecx
    cmp byte[ebx],0
    je near .nosubobjs

    mov cl,[ebx]
    cmp byte[C4ObjSelec],0FFh
    jne .sobjokay2
    dec cl
    mov [C4SObjSelec],cl
    jmp .sobjokay
.sobjokay2
    cmp byte[C4SObjSelec],cl
    jb .sobjokay
    mov byte[C4SObjSelec],0
.sobjokay

    xor ecx,ecx
    mov cl,[C4SObjSelec]
    shl ecx,2
    add ebx,ecx
    add ebx,4

    ; i,j,k,l = move current sub-object (17,24,25,26)
    ; u = toggle between 8x8 and 16x16 tiles
    ; o = toggle between high/low oam value
    ; . = decrease oam value, / = increase oam value of sub-object
    cmp byte[pressed+17h],0
    je .notpressed8
    mov byte[pressed+17h],0
    dec byte[ebx+1]
.notpressed8
    cmp byte[pressed+24h],0
    je .notpressed9
    mov byte[pressed+24h],0
    dec byte[ebx]
    cmp byte[.flipped],0
    je .notpressed9
    add byte[ebx],2
.notpressed9
    cmp byte[pressed+26h],0
    je .notpressed11
    mov byte[pressed+26h],0
    inc byte[ebx]
    cmp byte[.flipped],0
    je .notpressed11
    sub byte[ebx],2
.notpressed11
    cmp byte[pressed+25h],0
    je .notpressed10
    mov byte[pressed+25h],0
    inc byte[ebx+1]
.notpressed10
    cmp byte[pressed+16h],0
    je .notpressed12
    mov byte[pressed+16h],0
    xor byte[ebx+3],2
.notpressed12
    cmp byte[pressed+18h],0
    je .notpressed13
    mov byte[pressed+18h],0
    xor byte[ebx+3],1
.notpressed13
    cmp byte[pressed+34h],0
    je .notpressed14
    mov byte[pressed+34h],0
    dec byte[ebx+2]
.notpressed14
    cmp byte[pressed+35h],0
    je .notpressed15
    mov byte[pressed+35h],0
    inc byte[ebx+2]
.notpressed15

    mov cl,[ebx]
    mov ch,[ebx+1]
.nosubobjs
    mov edx,ecx
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[eax]
    sub bl,[esi+621h]
    add bl,dl
    mov cl,[eax+2]
    sub cl,[esi+623h]
    add cl,dh
    mov esi,[vidbuffer]
    add esi,16*2+256*2+32*2
    add esi,ebx
    add esi,ebx
    mov ebx,ecx
    shl ebx,9
    shl ecx,6
    add esi,ebx
    add esi,ecx
    mov al,[C4ColRot]
    mov ah,al
    xor ah,0FFh
    mov [esi],ax
    mov [esi+16],ax
    mov [esi+288*8*2],ax
    mov [esi+16+288*8*2],ax
.skipall
    ret
SECTION .bss
.flipped resb 1
SECTION .text

C4AddSprite:
    cmp dword[C4count],0
    je near .nosprite
    mov [edi],ax
    mov [edi+2],bx
    mov ebx,[C4usprptr]
    and [ebx],dl
    mov al,dl
    xor al,0FFh
    and dh,al
    or [ebx],dh
    add edi,4
    rol dl,2
    rol dh,2
    dec dword[C4count]
    cmp dl,0FCh
    jne .nosprite
    inc dword[C4usprptr]
.nosprite
    ret

C4ConvOAM:
    inc byte[C4Timer]
    and byte[C4Timer],15
    inc byte[C4Timer2]
    and byte[C4Timer2],7
    mov esi,[C4Ram]
    xor ecx,ecx
    mov edi,esi
    mov cl,[esi+620h]
    mov bx,[esi+621h]
    mov [.addx],bx
    mov bx,[esi+623h]
    mov [.addy],bx
    mov [C4usprptr],esi
    add dword[C4usprptr],200h
    mov eax,[C4ObjDisp]
    add edi,eax
    shr eax,4
    add dword[C4usprptr],eax
    add esi,220h
    ; Convert from esi to edi
    mov dl,0FCh
    push ecx
    mov cl,[C4sprites]
    and cl,3
    add cl,cl
    rol dl,cl
    pop ecx
    cmp cl,0
    je near .none
    mov dword[C4count],128
    mov eax,[C4sprites]
    sub dword[C4count],eax

.loop
    push ecx
    push esi
;              00/01 - x position relative to BG scroll value
;              02/03 - y position relative to BG scroll value
;              04    - palette/priority settings
;              05    - OAM pointer value
;              06    - flip settings : b6 = flipx, b7 = flipy
;              07    - ???
;              08/09 - Pointer to Sprite Structure
;              0A-0F - unused
;bit 1-3 = palette number bit 4,5 = playfield priority
;bit 6   = horizontal flip bit 7   = horizonal flip
    mov ax,[esi]
    sub ax,[.addx]
    mov [C4SprX],ax
    mov ax,[esi+2]
    sub ax,[.addy]
    mov [C4SprY],ax
    mov al,[esi+5]
    mov [C4SprOAM],al
    mov al,[esi+4]
    mov ah,al
    and ah,0Eh
    cmp ah,0
    jmp .notstage2
    jne .notstage1
    cmp byte[C4Timer],0
    je .flash
    jmp .noflash
.notstage1
    jmp .notstage2
    cmp ah,4
    jne .notstage2
    cmp byte[C4Timer2],0
    je .flash
.noflash
    and al,0F1h
    or al,2
    jmp .notstage2
.flash
    and al,0F1h
.notstage2
    mov [C4SprAttr],al
    mov al,[esi+6]
    or [C4SprAttr],al
;    mov [C4SprFlip],al

    xor ecx,ecx
    mov cl,[esi+9]
    shl ecx,16
    mov cx,[esi+7]
    add cx,cx
    shr ecx,1
    add ecx,[romdata]

    mov al,[ecx]
    or al,al
    jz near .singlespr
    mov [C4SprCnt],al
    inc ecx
.nextspr
    xor ebx,ebx
    movsx bx,byte[ecx+1]
    test byte[C4SprAttr],40h
    jz .notflipx
    neg bx
    sub bx,8
.notflipx
    add bx,[C4SprX]
    xor dh,dh
    test byte[ecx],20h
    jz .no16x16
    or dh,10101010b
    test byte[C4SprAttr],40h
    jz .no16x16
    sub bx,8
.no16x16
    cmp bx,-16
    jl near .nosprite
    cmp bx,272
    jg near .nosprite
    mov al,bl
    test bx,100h
    jz .not512b
    or dh,01010101b
.not512b
    xor ebx,ebx
    movsx bx,byte[ecx+2]
    test byte[C4SprAttr],80h
    jz .notflipy
    neg bx
    sub bx,8
.notflipy
    add bx,[C4SprY]
    test byte[ecx],20h
    jz .no16x16b
    test byte[C4SprAttr],80h
    jz .no16x16b
    sub bx,8
.no16x16b
    cmp bx,-16
    jl near .nosprite
    cmp bx,224
    jg near .nosprite
    mov ah,bl
    mov bh,[C4SprAttr]
    mov bl,[ecx]
    and bl,0C0h
    xor bh,bl
    mov bl,[C4SprOAM]
    add bl,[ecx+3]
    call C4AddSprite
.nosprite
    add ecx,4
    dec byte[C4SprCnt]
    jnz near .nextspr
    jmp .donemultispr
.singlespr
    mov dh,10101010b
    test byte[C4SprX+1],1
    jz .not512
    or dh,01010101b
.not512
    mov al,[C4SprX]
    mov ah,[C4SprY]
    mov bl,[C4SprOAM]
    mov bh,[C4SprAttr]
    call C4AddSprite
.donemultispr
    pop esi
    pop ecx
    add esi,16
    dec cl
    jnz near .loop
.none
    mov esi,oamram
    mov edi,[C4Ram]
    mov ecx,544
.next
    mov al,[edi]
    mov [esi],al
    inc edi
    inc esi
    dec ecx
    jnz .next
    ret

section .bss
.addx resw 1
.addy resw 1

C4count   resd 1
C4usprptr resd 1
C4SprX    resw 1
C4SprY    resw 1
C4SprCnt  resb 1
C4SprAttr resb 1
C4SprOAM  resb 1
C4SprFlip resb 1
C4Timer   resb 1
C4Timer2  resb 1

section .text

NEWSYM C4VBlank
    ret
NEWSYM C4ProcessSprites
    push ecx
    push esi
    push edi
    push ebx
    push edx
;    call C4ProcessVectors

;    call C4Edit

    mov esi,[C4Ram]
    mov dword[C4count],8
    mov cl,[esi+626h]
    mov [C4sprites],cl
    mov ecx,[C4sprites]
    shl ecx,2
    mov [C4ObjDisp],ecx
    mov ecx,128
;    cmp byte[esi+65],50h
;    jne .noincdisp
    mov dword[C4count],32
    sub ecx,[C4sprites]
.noincdisp
    add esi,[C4ObjDisp]
    ; Clear OAM to-be ram
.next
    mov byte[esi+1],0E0h
    add esi,4
    dec ecx
    jnz .next

    call C4ConvOAM

    pop edx
    pop ebx
    pop edi
    pop esi
    pop ecx
    ret

section .bss
NEWSYM SprValAdd, resb 1
C4Data resd 1
C4sprites resd 1
OBClog resd 1
NumSprites resb 1
OBCOldRegArray resb 1

section .text

NEWSYM InitOBC
    pushad
    mov esi,[romdata]
    add esi,4096*1024
    mov [C4RamR],esi
    mov [C4RamW],esi
    mov [C4Ram],esi
    add dword[C4RamW],8192*4
    add dword[C4Ram],8192*8
    mov ecx,8192
.c4loop
    mov dword[esi],OBCReadReg
    mov dword[esi+8192*4],OBCWriteReg
    mov dword[esi+8192*8],0
    add esi,4
    dec ecx
    jnz .c4loop
    mov esi,[romdata]
    add esi,4096*1024
    mov dword[esi+3A1Eh*4],OBCClear
    mov dword[esi+3FF0h*4],OBCRegs
    mov dword[esi+3FF1h*4],OBCRegs
    mov dword[esi+3FF2h*4],OBCRegs
    mov dword[esi+3FF3h*4],OBCRegs
    mov dword[esi+3FF4h*4],OBCRegs
    mov dword[esi+3FF5h*4],OBCRegs
    mov dword[esi+3FF6h*4],OBCRegs
    mov dword[esi+3FF7h*4],OBCRegs
    popad
    ret

OBCSprites:
    pushad
    mov byte[NumSprites],0
    mov esi,[C4Ram]
    mov edi,esi
    add edi,1800h
    add byte[OBCRegArray],2
    and byte[OBCRegArray],0FEh
    cmp byte[OBCRegArray],0FEh
    je .ohno
    cmp byte[OBCRegArray],0
    je .ohno
    jmp .okay
.ohno
    mov al,[OBCOldRegArray]
    mov [OBCRegArray],al
    jmp .loop
.okay
    mov al,[OBCRegArray]
    mov [OBCOldRegArray],al
.loop
    cmp byte[OBCRegArray],0
    je .nomore
    sub byte[OBCRegArray],2
    xor ebx,ebx
    mov bl,[esi+6]
    shl ebx,2

    ; Get X,Y,OAM, and Attr
    mov al,[esi+3]         ;0,3
    mov [edi+ebx],al
    mov al,[esi+9]
    mov [edi+ebx+1],al
    mov al,[esi+10]       ;2,10
    mov [edi+ebx+2],al
    mov al,[esi+0Bh]
    mov [edi+ebx+3],al

    xor ebx,ebx
    mov bl,[esi+6]
    shr ebx,2
    add ebx,512
    mov cl,[esi+6]
    and cl,03h
    add cl,cl

    xor al,al
    mov ah,0FCh
    mov al,[esi+4]   ;1,4
    and al,03h
    shl al,cl
    rol ah,cl
    and byte[edi+ebx],ah
    or byte[edi+ebx],al

    inc byte[NumSprites]
    add esi,16
    jmp .loop
.nomore

    mov esi,[C4Ram]
    mov edi,esi
    add edi,1800h
;    mov dword[edi+200h],0AAAAAAAAh
    popad
    ret

OBCClear:
    call OBCSprites
    mov byte[clearmem],1
    mov dword[OBClog],0
    ret

; Affected values: 0,1,2,3,4,6,7,9,A,B
; 0,1 - Another X value (unused?)
; 2   - OAM value
; 3/4 - X value (bits 0-8)
; 5   - N/A (not written to)
; 6   - OAM #
; 7   - Always 0?
; 9   - Y value (bits 0-7)
; A   - OAM value
; B   - OAM Status
; X,Y,OAM,Attr / xhighbit / OAM highbit / Sprite size
;bit 0 = OAM b8, bit 1-3 = palette number bit 4,5 = playfield priority
;bit 6   = horizontal flip bit 7   = horizonal flip
; Extra: bit 0 = X bit 8, bit 1 = Larger sprite size

SECTION .bss
OBCRegArray resb 8
clearmem resb 1
SECTION .data
OBCIncArray db 2,1,1,1,2,2,2,2
SECTION .text

OBCRegs:
    pushad
    sub ecx,1FF0h

    cmp byte[clearmem],0
    je near .noclearmem
    cmp ecx,6
    je .okay
    popad
    ret
.okay
    mov dword[OBCRegArray],0
    mov dword[OBCRegArray+4],0
    mov byte[OBCRegArray],0FEh
    mov byte[clearmem],0
.noclearmem

    mov ebx,[C4Ram]
    add ebx,1000h
    add ebx,[OBClog]
    inc dword[OBClog]
    mov [ebx],cl
    cmp cl,6
    jne .notsix
    add byte[OBCRegArray],2
    mov bl,[OBCRegArray]
    mov bh,bl
    mov [OBCRegArray+1],bl
    mov [OBCRegArray+2],bx
    mov [OBCRegArray+4],bx
    mov [OBCRegArray+6],bx
.notsix

    xor ebx,ebx
    mov bl,[OBCRegArray+ecx]
    cmp byte[OBCIncArray+ecx],1
    jne .noinc
    or byte[OBCRegArray+ecx],1
.noinc
    shl ebx,3
    add ecx,ebx
    add ecx,[C4Ram]
    mov [ecx],al
;    cmp dl,1
;    jne .second
    mov byte[ecx+8],0FFh
;    jmp .first
;.second
;    mov byte[ecx+16],0FFh
;.first
    popad
    ret

OBCReadReg:
    add ecx,[C4Ram]
    mov al,[ecx]
    sub ecx,[C4Ram]
    ret

OBCWriteReg
    add ecx,[C4Ram]
    mov [ecx],al
    sub ecx,[C4Ram]
    ret

NEWSYM InitC4
    pushad
    mov esi,[romdata]
    add esi,4096*1024
    mov [C4Data],esi
    add dword[C4Data],128*1024
    mov [C4RamR],esi
    mov [C4RamW],esi
    mov [C4Ram],esi
    add dword[C4RamW],8192*4
    add dword[C4Ram],8192*8
    mov ecx,8192
.c4loop
    mov dword[esi],C4ReadReg
    mov dword[esi+8192*4],C4WriteReg
    mov dword[esi+8192*8],0
    add esi,4
    dec ecx
    jnz .c4loop
    mov esi,[C4RamW]
    mov dword[esi+1F4Fh*4],C4RegFunction
    mov esi,[C4Data]
    mov ecx,16*4096
.c4loopb
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .c4loopb
    popad
    ret

C4ClearSpr:
    mov esi,ebx
    mov edi,eax
;    xor ecx,ecx
;    mov cx,[eax+1F44h]
;    sub cx,6000h
;    add eax,ecx
    shl ch,3
.scloop2
    mov cl,[C4SprPos]
    shl cl,2
.scloop
    mov byte[edi],0
    mov byte[edi+2000h],0
    inc edi
    dec cl
    jnz .scloop
    dec ch
    jnz .scloop2
    ret

C4SprBitPlane:
    mov edi,eax
    shl ebx,2
.scloop3
    mov ch,[C4SprPos]
    push esi
.scloop4
    push esi
    mov cl,8
.loop
    mov dh,8
    mov dl,80h
    mov eax,[esi]
.nextd
    test al,1
    jz .not0
    or byte[edi],dl
.not0
    test al,2
    jz .not1
    or byte[edi+1],dl
.not1
    test al,4
    jz .not2
    or byte[edi+16],dl
.not2
    test al,8
    jz .not3
    or byte[edi+17],dl
.not3
    shr eax,4
    shr dl,1
    dec dh
    jnz .nextd
    add esi,ebx
    add edi,2
    dec cl
    jnz .loop
    add edi,16
    pop esi
    add esi,4
    dec ch
    jnz .scloop4
    pop esi
    shl ebx,3
    add esi,ebx
    add edi,dword[C4SprPtrInc]
    shr ebx,3
    dec byte[C4SprPos+1]
    jnz .scloop3
.end
    ret

SECTION .bss
C4XXScale resw 1
C4XYScale resw 1
C4YXScale resw 1
C4YYScale resw 1
C4CXPos resw 1
C4CYPos resw 1
C4CXMPos resd 1
C4CYMPos resd 1
C4PCXMPos resd 1
C4PCYMPos resd 1
SECTION .text

DoScaleRotate:
    pushad
    mov esi,eax
    ; Calculate X scaler
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[CosTable+eax*2]
    mov bx,[esi+1F8Fh]
    test bx,8000h
    jz .notover
    mov bx,7FFFh
.notover
    imul bx
    add ax,ax
    adc dx,dx
    mov [C4XXScale],dx
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[SinTable+eax*2]
    imul bx
    add ax,ax
    adc dx,dx
    mov [C4XYScale],dx
    ; Calculate Y scaler
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[CosTable+eax*2]
    mov bx,[esi+1F92h]
    test bx,8000h
    jz .notoverb
    mov bx,7FFFh
.notoverb
    imul bx
    add ax,ax
    add dx,dx
    mov [C4YYScale],dx
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[SinTable+eax*2]
    imul bx
    add ax,ax
    adc dx,dx
    neg dx
    mov [C4YXScale],dx
    cmp word[esi+1F80h],0
    jne .effect
    cmp word[esi+1F92h],1000h
    jne .effect
    mov word[C4YYScale],1000h
    mov word[C4YXScale],0
.effect
    ; Calculate Pixel Resolution
    mov cl,[C4SprPos]
    shl cl,3
    mov [C4SprPos+2],cl
    mov cl,[C4SprPos+1]
    shl cl,3
    mov [C4SprPos+3],cl
    ; Calculate Positions
    ; (1-scale)*(pos/2)
    xor eax,eax
    mov al,[C4SprPos+2]
    shl eax,11
    mov [C4PCXMPos],eax
    xor eax,eax
    mov al,[C4SprPos+3]
    shl eax,11
    mov [C4PCYMPos],eax

    mov bx,[C4XXScale]
    xor eax,eax
    mov al,[C4SprPos+2]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCXMPos],edx
    mov bx,[C4YXScale]
    xor eax,eax
    mov al,[C4SprPos+3]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCXMPos],edx

    mov bx,[C4XYScale]
    xor eax,eax
    mov al,[C4SprPos+2]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCYMPos],edx
    mov bx,[C4YYScale]
    xor eax,eax
    mov al,[C4SprPos+3]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCYMPos],edx

    ; Start loop
    mov word[C4CYPos],0
    xor edi,edi
.loop
    mov ecx,[C4PCXMPos]
    mov [C4CXMPos],ecx
    mov ecx,[C4PCYMPos]
    mov [C4CYMPos],ecx
    mov al,[C4SprPos+2]
    mov [C4CXPos],al
.loop2
    xor eax,eax
    mov al,[C4SprPos+2]
    mov ebx,[C4CXMPos]
    sar ebx,12
    cmp ebx,eax
    jae near .blank
    xor eax,eax
    mov al,[C4SprPos+3]
    mov ebx,[C4CYMPos]
    sar ebx,12
    cmp ebx,eax
    jae near .blank
    ; Get pixel value
    mov ebx,[C4CYMPos]
    xor eax,eax
    shr ebx,12
    mov al,[C4SprPos+2]
    mul ebx
    mov ebx,[C4CXMPos]
    shr ebx,12
    add eax,ebx
    mov ebx,[C4SprPtr]
    test al,1
    jnz .upperb
    shr eax,1
    add ebx,eax
    mov al,[ebx]
    jmp .lowerb
.upperb
    shr eax,1
    add ebx,eax
    mov al,[ebx]
    shr al,4
.lowerb
    mov ebx,edi
    shr ebx,1
    add ebx,esi
    test edi,1
    jnz .upperb2
    and al,0Fh
    and byte[ebx+2000h],0F0h
    or byte[ebx+2000h],al
    jmp .done
.upperb2
    shl al,4
    and byte[ebx+2000h],0Fh
    or byte[ebx+2000h],al
    jmp .done
.blank
    mov eax,edi
    shr eax,1
    add eax,esi
    test edi,1
    jnz .upper
    and byte[eax+2000h],0F0h
    jmp .done
.upper
    and byte[eax+2000h],0Fh
.done
    movsx eax,word[C4XXScale]
    add [C4CXMPos],eax
    movsx eax,word[C4XYScale]
    add [C4CYMPos],eax
    inc edi
    dec byte[C4CXPos]
    jne near .loop2
    movsx eax,word[C4YXScale]
    add [C4PCXMPos],eax
    movsx eax,word[C4YYScale]
    add [C4PCYMPos],eax
    inc word[C4CYPos]
    mov al,[C4SprPos+3]
    cmp byte[C4CYPos],al
    jne near .loop
.noimage
    popad
    ret

DoScaleRotate2:
    pushad
    xor ebx,ebx
    mov bx,[eax+1F8Fh]
    cmp bx,1000h
    ja .scaled
    mov bx,1000h
.scaled
    mov [C4SprScale],ebx
    xor ebx,ebx
    mov bx,[eax+1F92h]
    cmp bx,1000h
    ja .scaledb
    mov bx,1000h
.scaledb
    mov [C4SprScaleY],ebx
    mov cl,[C4SprPos]
    shl cl,3
    mov ch,cl
    xor ebx,ebx
.leftovercheck
    dec ch
    add ebx,[C4SprScale]
.leftovercheckb
    cmp ebx,1000h
    jb .leftovercheck
    sub ebx,1000h
    dec cl
    jz .donecheck
    jmp .leftovercheckb
.donecheck
    shr ch,1
    mov cl,ch
    and ecx,0FFh
    mov esi,ecx

    mov cl,[C4SprPos+1]
    shl cl,3
    mov ch,cl
    xor ebx,ebx
.leftovercheckc
    dec ch
    add ebx,[C4SprScaleY]
.leftovercheckd
    cmp ebx,1000h
    jb .leftovercheckc
    sub ebx,1000h
    dec cl
    jz .donecheckc
    jmp .leftovercheckd
.donecheckc
    shr ch,1
    mov cl,ch
    and ecx,0FFh
    push eax
    xor eax,eax
    mov al,[C4SprPos]
    shl al,3
    mul ecx
    add esi,eax
    pop eax

    mov dword[C4SprScalerY],0
    xor edi,edi
    mov cl,[C4SprPos+1]
    shl cl,3
    mov [C4SprPos+3],cl
.next
    push esi
    push edi
    xor ecx,ecx
    mov cl,[C4SprPos]
    shl cl,3
    mov ch,cl
    mov dword[C4SprScaler],0
    xor edx,edx
.loop
    mov edx,edi
    shr edx,1
    add edx,[C4SprPtr]
    mov bl,[edx]
    test esi,1
    jz .notupper
    shr bl,4
.notupper
    and bl,0Fh
    mov edx,esi
    shr edx,1
    test esi,1
    jz .notupperb
    shl bl,4
    or byte[eax+edx+2000h],bl
    jmp .notlowerb
.notupperb
    or byte[eax+edx+2000h],bl
.notlowerb
    inc esi
    mov ebx,[C4SprScale]
    add dword[C4SprScaler],ebx
    dec ch
.nextcheck
    cmp dword[C4SprScaler],1000h
    jb near .loop
    sub dword[C4SprScaler],1000h
    inc edi
    dec cl
    jz .done
    jmp .nextcheck
.done
    pop edi
    pop esi
    xor edx,edx
    mov dl,[C4SprPos]
    shl dl,3
    add esi,edx

    mov ebx,[C4SprScaleY]
    add dword[C4SprScalerY],ebx
.nextcheckb
    cmp dword[C4SprScalerY],1000h
    jb near .next
    sub dword[C4SprScalerY],1000h
    add edi,edx
    dec byte[C4SprPos+3]
    jz .doneb
    jmp .nextcheckb
.doneb

    popad
    ret

C4SprScaleR:
    push ecx
    push ebx
    push edx
    push esi
    push edi
    mov dword[C4SprPtrInc],0
    xor ebx,ebx
    mov bl,[eax+1F42h]
    shl ebx,16
    mov bx,[eax+1F40h]
    add bx,bx
    shr ebx,1
    add ebx,[romdata]
    mov ch,[eax+1F8Ch]
    shr ch,3
    mov cl,[eax+1F89h]
    shr cl,3
    mov [C4SprPos],cx
    mov [C4SprPtr],ebx

    call C4ClearSpr

    call DoScaleRotate

    mov esi,eax
    add esi,2000h
    xor ebx,ebx
    mov bl,[C4SprPos]
    call C4SprBitPlane
    pop edi
    pop esi
    pop edx
    pop ebx
    pop ecx
    ret

C4SprRotateR:
    push ecx
    push ebx
    push edx
    push esi
    push edi
    xor ebx,ebx
    mov ebx,600h
    add ebx,[C4Ram]
    mov [C4SprPtr],esi
    mov ch,[eax+1F8Ch]
    shr ch,3
    mov cl,[eax+1F89h]
    shr cl,3
    add ch,2
    mov [C4SprPos],cx
    mov dword[C4SprPtrInc],64
    mov [C4SprPtr],ebx
    sub byte[C4SprPos+1],2
    call C4ClearSpr

    call DoScaleRotate
    mov esi,eax
    add esi,2000h
    xor ebx,ebx
    mov bl,[C4SprPos]
    add byte[C4SprPos+1],2
    call C4SprBitPlane
    pop edi
    pop esi
    pop edx
    pop ebx
    pop ecx
    ret

C4SprDisintegrate:
    pushad
    mov dword[C4SprPtrInc],0
    xor ebx,ebx
    mov bl,[eax+1F42h]
    shl ebx,16
    mov bx,[eax+1F40h]
    add bx,bx
    shr ebx,1
    add ebx,[romdata]
    mov ch,[eax+1F8Ch]
    shr ch,3
    mov cl,[eax+1F89h]
    shr cl,3
    mov [C4SprPos],cx
    mov [C4SprPtr],ebx

    call C4ClearSpr

    mov esi,[C4Ram]
    xor ebx,ebx
    mov bx,[esi+1F86h]
    xor eax,eax
    mov al,[esi+1F89h]
    shr al,1
    mul ebx
    neg eax
    xor ebx,ebx
    mov bl,[esi+1F89h]
    shr bl,1
    shl ebx,8
    add eax,ebx
    push eax
    xor ebx,ebx
    mov bx,[esi+1F8Fh]
    xor eax,eax
    mov al,[esi+1F8Ch]
    shr al,1
    mul ebx
    neg eax
    xor ebx,ebx
    mov bl,[esi+1F8Ch]
    shr bl,1
    shl ebx,8
    add ebx,eax
    mov edx,ebx
    pop ebx
    mov esi,[C4Ram]
    mov cl,[esi+1F89h]
    mov ch,[esi+1F8Ch]
    mov [C4SprPos+2],cx
    movsx eax,word[esi+1F86h]
    mov [.scalex],eax
    movsx eax,word[esi+1F8Fh]
    mov [.scaley],eax
    mov esi,[C4SprPtr]
    mov edi,[C4Ram]
    add edi,2000h

    ; convert to 8-bit bitmap
    mov cx,[C4SprPos+2]
    shr cl,1
.loop2
    mov al,[esi]
    mov [edi],al
    mov al,[esi]
    shr al,4
    mov [edi+1],al
    inc esi
    add edi,2
    dec cl
    jnz .loop2
    dec ch
    jnz .loop2

    mov edi,[C4Ram]
    add edi,4000h
    mov ecx,2000h
.lp
    mov byte[edi],0
    inc edi
    dec ecx
    jnz .lp

    mov esi,[C4Ram]
    add esi,2000h
    mov edi,[C4Ram]
    add edi,4000h

    mov cx,[C4SprPos+2]
.next2
    push ebx
.next
    xor eax,eax
    mov ah,[C4SprPos+2]
    cmp ebx,eax
    jae .fail
    xor eax,eax
    mov ah,[C4SprPos+3]
    cmp edx,eax
    jae .fail
    push ecx
    push edx
    xor eax,eax
    mov al,[C4SprPos+2]
    xor ecx,ecx
    mov cl,dh
    mul ecx
    mov ecx,ebx
    shr ecx,8
    add eax,ecx
    mov dl,[esi]
    cmp eax,2000h
    jae .skipdisi
    mov [edi+eax],dl
.skipdisi
    pop edx
    pop ecx
.fail
    inc esi
    add ebx,[.scalex]
    dec cl
    jnz near .next
    pop ebx
    add edx,[.scaley]
    mov cl,[C4SprPos+2]
    dec ch
    jnz near .next2

.skipall
    ; convert to 4-bit bitmap
    mov esi,[C4Ram]
    add esi,4000h
    mov edi,[C4Ram]
    add edi,6000h
    mov cx,[C4SprPos+2]
    shr cl,1
.loop
    mov al,[esi]
    mov [edi],al
    mov al,[esi+1]
    shl al,4
    or [edi],al
    inc edi
    add esi,2
    dec cl
    jnz .loop
    dec ch
    jnz .loop

    mov esi,[C4Ram]
    add esi,6000h
;    mov esi,[C4SprPtr]
    mov eax,[C4Ram]
    xor ebx,ebx
    mov bl,[C4SprPos]
    call C4SprBitPlane

    popad
    ret
SECTION .bss
.scalex resd 1
.scaley resd 1
SECTION .text

C4BitPlaneWave:
    pushad
    mov esi,[C4Ram]
    mov dword[.temp],10h
    xor eax,eax
    mov al,[esi+1F83h]
    mov [.waveptr],eax
    mov word[.temp+4],0C0C0h
    mov word[.temp+6],03F3Fh
.bmloopb
    mov edi,[C4Ram]
    add edi,[.waveptr]
    xor eax,eax
    movsx ax,byte[edi+$0B00]
    neg ax
    sub ax,16
    mov edi,[C4Ram]
    add edi,0A00h
    xor ecx,ecx
.bmloopa
    mov ebx,[.bmptr+ecx*4]
    mov dx,[.temp+6]
    and [esi+ebx],dx
    xor dx,dx
    cmp ax,0
    jl .less
    mov dx,0FF00h
    cmp ax,8
    jae .less
    mov dx,[edi+eax*2]
.less
    and dx,[.temp+4]
    or [esi+ebx],dx
    inc ax
    inc ecx
    cmp ecx,28h
    jne .bmloopa
    add dword[.waveptr],1
    and dword[.waveptr],07Fh
    ror word[.temp+4],2
    ror word[.temp+6],2
    cmp word[.temp+4],0C0C0h
    jne near .bmloopb
    add esi,16
.bmloopa2b
    mov edi,[C4Ram]
    add edi,[.waveptr]
    xor eax,eax
    movsx ax,byte[edi+$0B00]
    neg ax
    sub ax,16
    mov edi,[C4Ram]
    add edi,0A00h
    xor ecx,ecx
.bmloopa2
    mov ebx,[.bmptr+ecx*4]
    mov dx,[.temp+6]
    and [esi+ebx],dx
    xor dx,dx
    cmp ax,0
    jl .less2
    mov dx,0FF00h
    cmp ax,8
    jae .less2
    mov dx,[edi+eax*2+16]
.less2
    and dx,[.temp+4]
    or [esi+ebx],dx
    inc ax
    inc ecx
    cmp ecx,28h
    jne .bmloopa2
    add dword[.waveptr],1
    and dword[.waveptr],07Fh
    ror word[.temp+4],2
    ror word[.temp+6],2
    cmp word[.temp+4],0C0C0h
    jne near .bmloopa2b
    add esi,16
    dec dword[.temp]
    jnz near .bmloopb
    mov esi,[C4Ram]
;    mov cx,[esi+1F80h]
;    mov [C4values],cx
;    mov cx,[esi+1F83h]
;    mov [C4values+2],cx
    popad
    ret
SECTION .data
.bmptr dd 0000h,0002h,0004h,0006h,0008h,000Ah,000Ch,000Eh
       dd 0200h,0202h,0204h,0206h,0208h,020Ah,020Ch,020Eh
       dd 0400h,0402h,0404h,0406h,0408h,040Ah,040Ch,040Eh
       dd 0600h,0602h,0604h,0606h,0608h,060Ah,060Ch,060Eh
       dd 0800h,0802h,0804h,0806h,0808h,080Ah,080Ch,080Eh
SECTION .bss
.temp resd 2
.waveptr resd 1

SECTION .text

C4DrawLine:
    pushad

    ; transform both coordinates
    push esi
    mov ax,[C4X1]
    mov [C4WFXVal],ax
    mov ax,[C4Y1]
    mov [C4WFYVal],ax
    mov ax,[C4Z1]
    mov [C4WFZVal],ax
    mov al,[esi+1F90h]
    mov [C4WFScale],al
    mov al,[esi+1F86h]
    mov [C4WFX2Val],al
    mov al,[esi+1F87h]
    mov [C4WFY2Val],al
    mov al,[esi+1F88h]
    mov [C4WFDist],al
    call C4TransfWireFrame2
    mov ax,[C4WFXVal]
    mov [C4X1],ax
    mov ax,[C4WFYVal]
    mov [C4Y1],ax

    mov ax,[C4X2]
    mov [C4WFXVal],ax
    mov ax,[C4Y2]
    mov [C4WFYVal],ax
    mov ax,[C4Z2]
    mov [C4WFZVal],ax
    call C4TransfWireFrame2
    mov ax,[C4WFXVal]
    mov [C4X2],ax
    mov ax,[C4WFYVal]
    mov [C4Y2],ax

    add word[C4X1],48
    add word[C4Y1],48
    add word[C4X2],48
    add word[C4Y2],48
    shl dword[C4X1],8
    shl dword[C4X2],8
    shl dword[C4Y1],8
    shl dword[C4Y2],8
    ; get line info
    mov ax,[C4X1+1]
    mov [C4WFXVal],ax
    mov ax,[C4Y1+1]
    mov [C4WFYVal],ax
    mov ax,[C4X2+1]
    mov [C4WFX2Val],ax
    mov ax,[C4Y2+1]
    mov [C4WFY2Val],ax
    call C4CalcWireFrame
    xor ecx,ecx
    mov cx,[C4WFDist]
    or ecx,ecx
    jnz .not0
    mov ecx,1
.not0
    movsx eax,word[C4WFXVal]
    mov [C4X2],eax
    movsx eax,word[C4WFYVal]
    mov [C4Y2],eax
    pop esi
    ; render line
.loop
    ; plot pixel
    cmp word[C4X1+1],0
    jl near .noplot
    cmp word[C4Y1+1],0
    jl near .noplot
    cmp word[C4X1+1],95
    jg near .noplot
    cmp word[C4Y1+1],95
    jg near .noplot
    xor eax,eax
    mov dx,[C4Y1+1]
    shr dx,3
    mov ax,dx
    shl ax,6
    shl dx,8
    sub dx,ax
    mov ax,[C4X1+1]
    shr ax,3
    shl ax,4
    add ax,dx
    mov dx,[C4Y1+1]
    and dx,07h
    add dx,dx
    add ax,dx
    mov dl,07Fh
    push ecx
    mov cl,[C4X1+1]
    and cl,07h
    ror dl,cl
    pop ecx
    and byte[esi+eax+300h],dl
    and byte[esi+eax+301h],dl
    xor dl,0FFh
    test byte[C4Col],1
    jz .nocolor0
    or byte[esi+eax+300h],dl
.nocolor0
    test byte[C4Col],2
    jz .nocolor1
    or byte[esi+eax+301h],dl
.nocolor1
.noplot
    mov eax,[C4X2]
    add [C4X1],eax
    mov eax,[C4Y2]
    add [C4Y1],eax
    dec ecx
    jnz near .loop
    popad
    ret

DrawWireFrame:
    mov esi,[C4Ram]
    mov edi,esi
    xor ebx,ebx
    mov bl,[esi+1F82h]
    shl ebx,16
    mov bx,[esi+1F80h]
    add bx,bx
    shr ebx,1
    add ebx,[romdata]
    mov edi,ebx
    xor ecx,ecx
    mov cl,[esi+295h]
.loop
    xor eax,eax
    mov al,[esi+1F82h]
    shl eax,16
    mov al,[edi+1]
    mov ah,[edi+0]
    mov edx,edi
.nextprev
    cmp ax,0FFFFh
    jne .notprev
    sub edx,5
    mov al,[edx+3]
    mov ah,[edx+2]
    jmp .nextprev
.notprev
    add ax,ax
    shr eax,1
    add eax,[romdata]
    xor edx,edx
    mov dl,[esi+1F82h]
    shl edx,16
    mov dl,[edi+3]
    mov dh,[edi+2]
;    mov [C4values+6],dx
    add dx,dx
    shr edx,1
    add edx,[romdata]
    xor ebx,ebx
    mov bh,[eax]
    mov bl,[eax+1]
    mov [C4X1],ebx
    mov bh,[eax+2]
    mov bl,[eax+3]
    mov [C4Y1],ebx
    mov bh,[eax+4]
    mov bl,[eax+5]
    mov [C4Z1],ebx
    mov bh,[edx]
    mov bl,[edx+1]
    mov [C4X2],ebx
    mov bh,[edx+2]
    mov bl,[edx+3]
    mov [C4Y2],ebx
    mov bh,[edx+4]
    mov bl,[edx+5]
    mov [C4Z2],ebx
    mov al,[edi+4]
    mov [C4Col],al
    add edi,5
    call C4DrawLine
    dec ecx
    jnz near .loop
    ret

SECTION .bss
C4X1 resd 1
C4Y1 resd 1
C4Z1 resd 1
C4X2 resd 1
C4Y2 resd 1
C4Z2 resd 1
C4Col resd 1
SECTION .text

WireFrameB:
    pushad
    ; 28EECA
    ; 7F80 (3bytes) = pointer to data
    ; 7F86-7F88 = rotation, 7F90 = scale (/7A?)
    ; 6295 = # of lines, 7FA5 = ???
    mov esi,[C4Ram]
    add esi,300h
    mov ecx,16*12*3
.loop
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .loop
    call DrawWireFrame

    mov esi,[C4Ram]
    mov cx,[esi+1FA5h]
;    mov [C4values],cx
;    mov cx,[esi+1F86h]
;    mov [C4values],cx
;    mov cx,[esi+1F88h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F90h]
;    mov [C4values+4],cx
    popad
    ret

WireFrameB2:
    pushad
    call DrawWireFrame
    popad
    ret

C4WireFrame:
    pushad
    mov esi,[C4Ram]
    mov ax,[esi+1F83h]
    and ax,0FFh
    mov [C4WFX2Val],ax
;    mov [C4values],ax
    mov ax,[esi+1F86h]
    and ax,0FFh
    mov [C4WFY2Val],ax
;    mov [C4values+2],ax
    mov ax,[esi+1F89h]
    and ax,0FFh
    mov [C4WFDist],ax
;    mov [C4values+4],ax
    mov ax,[esi+1F8Ch]
    and ax,0FFh
    mov [C4WFScale],ax
;    mov [C4values+6],ax

    ; transform vertices (MMX2 - 36 vertices, 54 lines)
    xor ecx,ecx
    mov cx,[esi+1F80h]
    xor al,al
.loop
    mov ax,[esi+1]
    mov [C4WFXVal],ax
    mov ax,[esi+5]
    mov [C4WFYVal],ax
    mov ax,[esi+9]
    mov [C4WFZVal],ax
    push esi
    push ecx
    call C4TransfWireFrame
    pop ecx
    pop esi
    ; Displace
    mov ax,[C4WFXVal]
    add ax,80h
    mov [esi+1],ax
    mov ax,[C4WFYVal]
    add ax,50h
    mov [esi+5],ax
    add esi,10h
    dec ecx
    jnz .loop
    ; Uses 6001,6005,6600,6602,6605

    mov esi,[C4Ram]
    mov word[esi+$600],23
    mov word[esi+$602],60h
    mov word[esi+$605],40h
    mov word[esi+$600+8],23
    mov word[esi+$602+8],60h
    mov word[esi+$605+8],40h

    xor ecx,ecx
    mov cx,[esi+0B00h]
    mov edi,esi
    add edi,0B02h
.lineloop
    xor eax,eax
    mov al,[edi]
    shl eax,4
    add eax,[C4Ram]
    mov bx,[eax+1]
    mov [C4WFXVal],bx
    mov bx,[eax+5]
    mov [C4WFYVal],bx
    xor eax,eax
    mov al,[edi+1]
    shl eax,4
    add eax,[C4Ram]
    mov bx,[eax+1]
    mov [C4WFX2Val],bx
    mov bx,[eax+5]
    mov [C4WFY2Val],bx
    push esi
    push edi
    push ecx
    call C4CalcWireFrame
    pop ecx
    pop edi
    pop esi
    mov ax,[C4WFDist]
    or ax,ax
    jnz .yeswire
    mov ax,1
.yeswire
    mov [esi+$600],ax
    mov ax,[C4WFXVal]
    mov [esi+$602],ax
    mov ax,[C4WFYVal]
    mov [esi+$605],ax
    add edi,2
    add esi,8
    dec ecx
    jnz near .lineloop
.done
    popad
    ret

C4Transform:
    ; 7F81,4,7,9,A,B,0,1,D
    pushad
    mov esi,[C4Ram]
    mov ax,[esi+1F81h]
    mov [C4WFXVal],ax
    mov ax,[esi+1F84h]
    mov [C4WFYVal],ax
    mov ax,[esi+1F87h]
    mov [C4WFZVal],ax
    mov al,[esi+1F90h]
    mov [C4WFScale],al
    mov al,[esi+1F89h]
    mov [C4WFX2Val],al
    mov al,[esi+1F8Ah]
    mov [C4WFY2Val],al
    mov al,[esi+1F8Bh]
    mov [C4WFDist],al
    call C4TransfWireFrame2
    mov ax,[C4WFXVal]
    mov [esi+1F80h],ax
    mov ax,[C4WFYVal]
    mov [esi+1F83h],ax
    popad
    ret

SECTION .bss
C4SprPos resd 1
C4SprScale resd 1
C4SprScaleY resd 1
C4SprScaler resd 1
C4SprScalerY resd 1
C4SprPtr resd 1
C4SprPtrInc resd 1
NEWSYM C4values, resd 3

section .text

C4activate:
    cmp ecx,1F4Fh
    jne .noc4test
    push esi
    mov esi,[C4Ram]
    cmp byte[esi+1F4Dh],0Eh
    jne .befnoc4test
    test al,0C3h
    jnz .befnoc4test
    shr al,2
    mov [esi+1F80h],al
    pop esi
    ret
.befnoc4test
    pop esi
.noc4test
    cmp al,00h
    je near .dosprites
    cmp al,01h
    je near .dowireframe
    cmp al,05h          ; ?
    je near .propulsion
    cmp al,0Dh          ; ?
    je near .equatevelocity
    cmp al,10h          ; supply angle+distance, return x/y displacement
    je near .direction
    cmp al,13h          ; Convert polar coordinates to rectangular 2 (similar to 10)
    je near .polarcord2
    cmp al,15h          ; ?
    je near .calcdistance
    cmp al,1Fh          ; supply x/y displacement, return angle (+distance?)
    je near .calcangle
    cmp al,22h          ; supply x/y displacement, return angle (+distance?)
    je near .linearray
    cmp al,25h
    je near .multiply
    cmp al,2Dh          ; ???
    je near .transform
    cmp al,40h
    je near .sum
    cmp al,54h
    je near .square
    cmp al,5Ch
    je near .immediatereg
    cmp al,89h
    je near .immediaterom
    ret
.dowireframe
    call WireFrameB
    ret

.linearray
    pushad
    call C4Op22
    popad
    ret

.propulsion
    pushad
    ; 81 = 5B, 83 = 0x300
    ; 0x300 = /1, 0x280 = /4
    mov esi,[C4Ram]

    mov cx,[esi+1F83h]
    mov [C4values+2],cx
    mov cx,[esi+1F81h]
    mov [C4values],cx
    xor bx,bx

;    mov ax,256*256
    xor ax,ax
    mov dx,1
    mov bx,[esi+1F83h]
    or dx,dx
    jz .done
    idiv bx
    mov [C4values+6],ax
    mov bx,[esi+1F81h]
    imul bx
    shl edx,16
    mov dx,ax
    sar edx,8
.done
    mov [esi+1F80h],dx
    mov [C4values+4],dx

;    and eax,1FFh
;    mov bx,[SinTable+eax*2]
;    mov ax,[esi+1F81h]          ; distance?
;    imul bx
;    mov ax,dx
;    shl ax,1
;    shl dx,3
;    add dx,ax

    popad
    ret
.polarcord2
    pushad
    mov esi,[C4Ram]
    xor ecx,ecx
    mov cx,[esi+1F80h]
    and ecx,1FFh
    movsx eax,word[esi+1F83h]
    add eax,eax
    movsx ebx,word[CosTable+ecx*2]
    imul ebx,eax
    sar ebx,8
    adc ebx,0
    mov [esi+1F86h],ebx
    movsx ebx,word[SinTable+ecx*2]
    imul ebx,eax
    sar ebx,8
    adc ebx,0
    mov [esi+1F89h],bx
    sar ebx,16
    mov [esi+1F8Bh],bl
    popad
    ret
.dosprites
    push eax
    mov eax,[C4Ram]
    cmp byte[eax+1F4Dh],0
    je near .sprites
    cmp byte[eax+1F4Dh],3
    je near .scaler
    cmp byte[eax+1F4Dh],5
    je near .lines
    cmp byte[eax+1F4Dh],7
    je near .rotater
    cmp byte[eax+1F4Dh],8
    je near .wireframeb
    cmp byte[eax+1F4Dh],0Bh
    je near .disintegrate
    cmp byte[eax+1F4Dh],0Ch
    je near .bitmap
    pop eax
    ret
.wireframeb
    pop eax
    call WireFrameB2
    ret
.sprites
    pop eax
    call C4ProcessSprites
    ret
.disintegrate
    call C4SprDisintegrate
    pop eax
    ret
.dolines
    ret
.bitmap
    call C4BitPlaneWave
    pop eax
    ret
.calcdistance
    pushad
    mov esi,[C4Ram]
    mov bx,[esi+1F80h]
    mov [C41FXVal],bx
    mov bx,[esi+1F83h]
    mov [C41FYVal],bx
;    mov eax,[C4Ram]
;    mov cx,[eax+1F80h]
;    mov [C4values+0],cx
;    mov cx,[eax+1F83h]
;    mov [C4values+2],cx
    call C4Op15
    mov eax,[C4Ram]
    mov bx,[C41FDist]
    mov [eax+1F80h],bx
;    mov word[eax+1F80h],50
;    mov cx,[eax+1F80h]
;    mov [C4values+4],cx
    popad
    ret
.calcangle
    pushad
    mov esi,[C4Ram]
    mov bx,[esi+1F80h]
    mov [C41FXVal],bx
    mov bx,[esi+1F83h]
    mov [C41FYVal],bx
    call C4Op1F
    mov eax,[C4Ram]
    mov bx,[C41FAngleRes]
    mov [eax+1F86h],bx
;    mov esi,[C4Ram]
;    mov cx,[esi+1F86h]
;    mov [C4values],cx
;    mov cx,[esi+1F80h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F83h]
;    mov [C4values+4],cx
    popad
    ret
.transform
    ; 7F81,4,7,9,A,B,0,1,D
    pushad
;    mov eax,[C4Ram]
    call C4Transform
;    mov word[eax+1F80h],0
;    mov word[eax+1F83h],0
    popad
    ret
.multiply
    pushad
    mov esi,[C4Ram]
    mov eax,[esi+1F80h]
    and eax,0FFFFFFh
    mov ebx,[esi+1F83h]
    and ebx,0FFFFFFh
    imul eax,ebx
    mov [esi+1F80h],eax
    popad
    ret
.sum
    pushad
    xor eax,eax
    xor ebx,ebx
    mov esi,[C4Ram]
    mov ecx,800h
.sumloop
    mov bl,[esi]
    inc esi
    add ax,bx
    dec ecx
    jnz .sumloop
    mov [esi+1F80h-0800h],ax
    popad
    ret
.square
    pushad
    xor edx,edx
    mov esi,[C4Ram]
    mov eax,[esi+1F80h]
    shl eax,8
    sar eax,8
    imul eax
    mov [esi+1F83h],eax
    mov [esi+1F87h],dx
    popad
    ret
.equatevelocity
    pushad
    mov esi,[C4Ram]
    mov bx,[esi+1F80h]
    mov [C41FXVal],bx
    mov bx,[esi+1F83h]
    mov [C41FYVal],bx
    mov bx,[esi+1F86h]
    mov [C41FDistVal],bx
    call C4Op0D
    mov bx,[C41FXVal]
    mov [esi+1F89h],bx
    mov bx,[C41FYVal]
    mov [esi+1F8Ch],bx
    popad
    ret


    pushad
    mov esi,[C4Ram]
    mov cx,[esi+$1F86]
    cmp cx,40h
    jb .nomult
    shr cx,7
.nomult
    mov ax,[esi+$1F80]
;    imul cx
    shl ax,4
    mov [esi+$1F89],ax
    mov ax,[esi+$1F83]
;    imul cx
    shl ax,4
    mov [esi+$1F8C],ax
;    mov cx,[esi+$1F80]
;    mov [C4values],cx
;    mov cx,[esi+$1F83]
;    mov [C4values+2],cx
;    mov cx,[esi+$1F86]
;    mov [C4values+4],cx
    popad
    ret
.lines
    call C4WireFrame
    pop eax
    ret
.scaler
    push esi
    push ecx
    mov esi,[C4Ram]
;    mov cx,[esi+1F8Fh]
;    mov [C4values],cx
;    mov cx,[esi+1F92h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F80h]
;    mov [C4values+4],cx
    pop ecx
    pop esi
    call C4SprScaleR
    pop eax
    ret
.rotater
    push esi
    push ecx
    mov esi,[C4Ram]
;    mov cx,[esi+1F8Fh]
;    mov [C4values],cx
;    mov cx,[esi+1F92h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F80h]
;    mov [C4values+4],cx
    pop ecx
    pop esi
    call C4SprRotateR
    pop eax
    ret
.direction
    push eax
    push ebx
    push esi
    push edx
    push ecx
    mov esi,[C4Ram]
    xor ecx,ecx
    mov ax,[esi+1F80h]
    and eax,1FFh
    mov bx,[CosTable+eax*2]
    mov ax,[esi+1F83h]
    imul bx
    add ax,ax
    adc dx,dx
    mov ax,dx
    movsx edx,dx
    mov [esi+1F86h],edx
    mov ax,[esi+1F80h]
    and eax,1FFh
    mov bx,[SinTable+eax*2]
    mov ax,[esi+1F83h]
    imul bx
    add ax,ax
    adc dx,dx
    mov ax,dx
    movsx edx,dx
    mov eax,edx
    sar eax,6
    sub edx,eax
    mov al,[esi+198Ch]
    mov [esi+1F89h],edx
    mov [esi+198Ch],al
;    mov cx,[esi+1F80h]
;    mov [C4values],cx
;    mov cx,[esi+1F83h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F86h]
;    mov [C4values+4],cx
    pop ecx
    pop edx
    pop esi
    pop ebx
    pop eax
    ret
.immediaterom
    push eax
    mov eax,[C4Ram]
    mov byte[eax+1F80h],36h
    mov byte[eax+1F81h],43h
    mov byte[eax+1F82h],05h
    pop eax
    ret
.immediatereg
    push eax
    mov eax,[C4Ram]
    mov dword[eax+0*4],0FF000000h
    mov dword[eax+1*4],0FF00FFFFh
    mov dword[eax+2*4],0FF000000h
    mov dword[eax+3*4],00000FFFFh
    mov dword[eax+4*4],00000FFFFh
    mov dword[eax+5*4],07FFFFF80h
    mov dword[eax+6*4],0FF008000h
    mov dword[eax+7*4],07FFF007Fh
    mov dword[eax+8*4],0FFFF7FFFh
    mov dword[eax+9*4],0FF010000h
    mov dword[eax+10*4],00100FEFFh
    mov dword[eax+11*4],000FEFF00h
    pop eax
    ret

C4RegFunction:
    add ecx,[C4Ram]
    mov [ecx],al
    sub ecx,[C4Ram]
    cmp ecx,1F4Fh
    je near C4activate
    ret

 ;well, when 7f47 is written, copy the number of bytes specified in
 ;$7f43-4 from the address at $7f40-2 to the address at $7f45-6
 ;(which is presumably in the $6000-$7fff range)

NEWSYM C4ReadReg
    add ecx,[C4Ram]
    mov al,[ecx]
    sub ecx,[C4Ram]
    ret

NEWSYM C4WriteReg
    add ecx,[C4Ram]
    mov [ecx],al
    sub ecx,[C4Ram]
    cmp ecx,1F47h
    je .C4Memcpy
    ret
.C4Memcpy
    pushad
    mov esi,[C4Ram]
    movzx ecx,word[esi+1F43h] ;Num of bytes to copy
    movzx eax,byte[esi+1F42h] ;Source bank
    mov eax,[snesmmap+eax*4]
    movzx edx,word[esi+1F40h]
    add eax,edx
    movzx edx,word[esi+1F45h] ;Destination
    mov ebx,[C4Ram]
    and edx,01FFFh
    add ebx,edx
.c4movloop
    mov dl,[eax]
    mov [ebx],dl
    inc eax
    inc ebx
    dec ecx
    jnz .c4movloop
    popad
    ret


NEWSYM regaccessbankr8
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    mov al,[wramdataa+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .hiromsram
    mov al,ch
    ret
.hiromsram
    cmp byte[SPC7110Enable],1
    je near .spc7110ram
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[C4Enable],1
    je near .c4ram
    cmp byte[OBCEnable],1
    je near .c4ram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor al,al
    xor ebx,ebx
    ret
.dsp1
    xor al,al
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read8b
.nodsp1
    xor ebx,ebx
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankr8b
    pop ecx
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov al,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret
.c4ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[C4RamR]
    call dword near [ebx+ecx*4]
    xor ebx,ebx
    pop ecx
    ret
.spc7110ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankr8b
    pop ecx
    ret

NEWSYM regaccessbankr16
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,1FFFh
    jae .regs
    mov ax,[wramdataa+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
;    jmp regexiter
    cmp ecx,6000h
    jae .hiromsram
    mov al,ch
    mov ah,ch
    ret
.hiromsram
    cmp byte[SPC7110Enable],1
    je near .spc7110ram
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[C4Enable],1
    je near .c4ram
    cmp byte[OBCEnable],1
    je near .c4ram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor ax,ax
    xor ebx,ebx
    ret
.dsp1
    xor ax,ax
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read16b
.nodsp1
    xor ebx,ebx
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankr16b
    pop ecx
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret
.c4ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[C4RamR]
    inc ecx
    call dword near [ebx+ecx*4]
    dec ecx
    mov ah,al
    call dword near [ebx+ecx*4]
    xor ebx,ebx
    pop ecx
    ret
.spc7110ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankr16b
    pop ecx
    ret

NEWSYM regaccessbankw8
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    mov [wramdataa+ecx],al
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
;    jmp regexiter
    cmp ecx,6000h
    jae .hiromsram
    ret
.hiromsram
    cmp byte[SPC7110Enable],1
    je near .spc7110ram
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[C4Enable],1
    je near .c4ram
    cmp byte[OBCEnable],1
    je near .c4ram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor ebx,ebx
    ret
.dsp1
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write8b
.nodsp1
    xor ebx,ebx
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankw8b
    pop ecx
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],al
    xor ebx,ebx
    pop ecx
    ret
.c4ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[C4RamW]
    call dword near [ebx+ecx*4]
    xor ebx,ebx
    pop ecx
    ret
.spc7110ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankw8b
    pop ecx
    ret

NEWSYM regaccessbankw16
    test ecx,8000h
    jnz .romacc
    cmp ecx,1FFFh
    jae .regs
    mov [wramdataa+ecx],ax
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
.regs
    cmp cx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
;    jmp regexiter
    cmp ecx,6000h
    jae .hiromsram
    ret
.hiromsram
    cmp byte[SPC7110Enable],1
    je near .spc7110ram
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[C4Enable],1
    je near .c4ram
    cmp byte[OBCEnable],1
    je near .c4ram
    and ebx,7Fh
    cmp bl,10h
    jb .dsp1
    cmp bl,30h
    jae .hiromsramok
    xor al,al
    xor ebx,ebx
    ret
.dsp1
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write16b
.nodsp1
    ret
.hiromsramok
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    sub bl,30h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankw16b
    pop ecx
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],ax
    xor ebx,ebx
    pop ecx
    ret
.c4ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
;    mov ebx,[C4Ram]
;    mov [ebx+ecx],ax
    mov ebx,[C4RamW]
    push eax
    call dword near [ebx+ecx*4]
    inc ecx
    mov al,ah
    call dword near [ebx+ecx*4]
    pop eax
    dec ecx
    xor ebx,ebx
    pop ecx
    ret
.spc7110ram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbankw16b
    pop ecx
    ret

NEWSYM regaccessbankr8mp
    ret

;*******************************************************
; Register & Memory Bank (Bank 0)
;*******************************************************
; enter : BL = bank number, CX = address location
; leave : AL = value read

EXTSYM BWShift,SA1BWPtr

%macro BWCheck 0
    cmp byte[BWShift],0
    jne near .shift
.nosa1
%endmacro

section .bss
NEWSYM BWUsed2, resb 1
NEWSYM BWUsed, resb 1
section .text

%macro BWCheck2r8 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    ; value of 8Fh
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    ret
.2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    ret
%endmacro

%macro BWCheck2r16 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    ; value of 8Fh
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    push ecx
    push ebx
    sub ecx,6000h
    inc ecx
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov ah,0Fh
    shl ah,cl
    add ebx,[SA1BWPtr]
    and ah,[ebx]
    shr ah,cl
    pop ebx
    pop ecx
    ret
.2bit
    push ecx
    push ebx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and al,[ebx]
    shr al,cl
    pop ebx
    pop ecx
    push ecx
    push ebx
    inc ecx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov ah,03h
    shl ah,cl
    add ebx,[SA1BWPtr]
    and ah,[ebx]
    shr ah,cl
    pop ebx
    pop ecx
    ret
%endmacro

%macro BWCheck2w8 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,0Fh
    shl dh,cl
    xor dh,0FFh
    and al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    ret
.2bit
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,03h
    shl dh,cl
    xor dh,0FFh
    and al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    ret
%endmacro

%macro BWCheck2w16 0
.shift
    cmp byte[SA1Status],0
    je .nosa1
    test byte[SA1Overflow+1],80h
    jnz .2bit
    push eax
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,0Fh
    shl dh,cl
    xor dh,0FFh
    and al,0Fh
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    pop eax
    push ecx
    push ebx
    push edx
    inc ecx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,01h
    shl eax,2
    shr ecx,1
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,0Fh
    shl dh,cl
    xor dh,0FFh
    and ah,0Fh
    shl ah,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],ah
    pop edx
    pop ebx
    pop ecx
    ret
.2bit
    push eax
    push ecx
    push ebx
    push edx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,03h
    shl dh,cl
    xor dh,0FFh
    and al,03h
    shl al,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],al
    pop edx
    pop ebx
    pop ecx
    pop eax
    push ecx
    push ebx
    push edx
    inc ecx
    sub ecx,6000h
    push eax
    mov eax,ecx
    and eax,03h
    shl eax,1
    shr ecx,2
    mov ebx,ecx
    mov cl,al
    pop eax
    mov dh,03h
    shl dh,cl
    xor dh,0FFh
    and ah,03h
    shl ah,cl
    add ebx,[SA1BWPtr]
    and byte[ebx],dh
    or byte[ebx],ah
    pop edx
    pop ebx
    pop ecx
    ret
%endmacro

%macro writetobank0table 2
    mov ebx,%1
    mov ecx,%2
%%loop
    mov [eax],ebx
    add eax,4
    dec ecx
    jnz %%loop
%endmacro

section .bss
NEWSYM DPageR8, resd 1
NEWSYM DPageR16, resd 1
NEWSYM DPageW8, resd 1
NEWSYM DPageW16, resd 1
NEWSYM SA1DPageR8, resd 1
NEWSYM SA1DPageR16, resd 1
NEWSYM SA1DPageW8, resd 1
NEWSYM SA1DPageW16, resd 1
section .text

NEWSYM UpdateDPage
    push eax
    xor eax,eax
    mov al,[xd+1]
    push ecx
    mov ecx,[Bank0datr8+eax*4]
    mov [DPageR8],ecx
    mov ecx,[Bank0datr16+eax*4]
    mov [DPageR16],ecx
    mov ecx,[Bank0datw8+eax*4]
    mov [DPageW8],ecx
    mov ecx,[Bank0datw16+eax*4]
    mov [DPageW16],ecx
    pop ecx
    pop eax
    ret

NEWSYM SA1UpdateDPage
    push eax
    xor eax,eax
    mov al,[SA1xd+1]
    push ecx
    mov ecx,[Bank0datr8+eax*4]
    mov [SA1DPageR8],ecx
    mov ecx,[Bank0datr16+eax*4]
    mov [SA1DPageR16],ecx
    mov ecx,[Bank0datw8+eax*4]
    mov [SA1DPageW8],ecx
    mov ecx,[Bank0datw16+eax*4]
    mov [SA1DPageW16],ecx
    pop ecx
    pop eax
    ret

; SA1 Stuff
NEWSYM membank0r8ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov al,[wramdataa+ecx+ebx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov al,[IRAM+ecx+ebx]
    ret
.invaccess
    xor al,al
    ret
NEWSYM membank0r16ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov ax,[wramdataa+ecx+ebx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov ax,[IRAM+ecx+ebx]
    ret
.invaccess
    xor ax,ax
    ret
NEWSYM membank0w8ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx+ebx],al
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx+ebx],al
.invaccess
    ret
NEWSYM membank0w16ramSA1             ; 0000-1FFF
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx+ebx],ax
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx+ebx],ax
.invaccess
    ret

; --- 8 BIT READ STUFF ---
NEWSYM membank0r8ram             ; 0000-1FFF
    mov al,[wramdataa+ebx+ecx]
    ret
NEWSYM membank0r8reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
NEWSYM membank0r8inv             ; 4800-5FFF
    add ecx,ebx
    mov al,ch
    ret
NEWSYM membank0r8chip            ; 6000-7FFF
    add ecx,ebx
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    xor al,al
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read8b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov al,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r8

NEWSYM membank0r8rom             ; 8000-FFFF
    add ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
NEWSYM membank0r8romram             ; 0000-1FFF
    add cx,bx
    test cx,8000h
    jnz .rom
    mov al,[wramdataa+ecx]
    ret
.rom
    mov ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

; --- 16 BIT READ STUFF ---
NEWSYM membank0r16ram             ; 0000-1EFF
    mov ax,[wramdataa+ebx+ecx]
    ret
NEWSYM membank0r16ramh            ; 1F00-1FFF
    add ecx,ebx
    cmp ecx,1FFFh
    je .over
    mov ax,[wramdataa+ecx]
    ret
.over
    mov al,[wramdataa+ecx]
    xor ah,ah
    ret
NEWSYM membank0r16reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
NEWSYM membank0r16inv             ; 4800-5FFF
    add ecx,ebx
    mov al,ch
    mov ah,ch
    mov ax,8080h
    ret
NEWSYM membank0r16chip            ; 6000-FFFF
    add ecx,ebx
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    xor ax,ax
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read16b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r16
NEWSYM membank0r16rom             ; 8000-FFFF
    add ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
NEWSYM membank0r16romram             ; 0000-1FFF
    add cx,bx
    test cx,8000h
    jnz .rom
    mov ax,[wramdataa+ecx]
    ret
.rom
    mov ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

; --- 8 BIT WRITE STUFF ---
NEWSYM membank0w8ram             ; 0000-1FFF
    mov [wramdataa+ebx+ecx],al
    ret
NEWSYM membank0w8reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
NEWSYM membank0w8inv             ; 4800-5FFF
    ret
NEWSYM membank0w8chip            ; 6000-FFFF
    add ecx,ebx
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write8b
.nodsp1
    ret
.sfxram
    push ecx
    sub cx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],al
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
    BWCheck2w8
NEWSYM membank0w8rom             ; 8000-FFFF
    ret
NEWSYM membank0w8romram             ; 0000-1FFF
    add cx,bx
    test cx,8000h
    jnz .rom
    mov [wramdataa+ecx],al
    ret
.rom
    ret

; --- 16 BIT WRITE STUFF ---
NEWSYM membank0w16ram             ; 0000-1EFF
    mov [wramdataa+ebx+ecx],ax
    ret
NEWSYM membank0w16ramh            ; 1F00-1FFF
    add ecx,ebx
    cmp ecx,1FFFh
    je .over
    mov [wramdataa+ecx],ax
    ret
.over
    mov [wramdataa+ecx],al
    ret
NEWSYM membank0w16reg             ; 2000-48FF
    add ecx,ebx
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
NEWSYM membank0w16inv             ; 4800-5FFF
    ret
NEWSYM membank0w16chip            ; 6000-FFFF
    add ecx,ebx
NEWSYM membank0w16rom             ; 8000-FFFF
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[SA1Enable],1
    je .sa1ram
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write16b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],ax
    xor ebx,ebx
    pop ecx
    ret
.sa1ram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
    BWCheck2w16
NEWSYM membank0w16romram             ; 0000-1FFF
    add cx,bx
    test cx,8000h
    jnz .rom
    mov [wramdataa+ecx],ax
    ret
.rom
    ret

NEWSYM membank0r8
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0r8SA1
    cmp ecx,2000h
    jae .regs
    mov al,[wramdataa+ecx]
    ret
.regs
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    mov al,ch
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
    xor al,al
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read8b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov al,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret

NEWSYM membank0r16
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0r16SA1
    cmp ecx,2000h
    jae .regs
    mov ax,[wramdataa+ecx]
    ret
.regs
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptr]
;    call dword near [ebx]
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    xor ax,ax
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
    xor ax,ax
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Read16b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    pop ecx
    ret

NEWSYM membank0w8
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0w8SA1
    cmp ecx,2000h
    jae .regs
    mov [wramdataa+ecx],al
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
.regs
    test ecx,8000h
    jnz .romacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write8b
.nodsp1
    ret
.sfxram
    push ecx
    sub cx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],al
    xor ebx,ebx
    pop ecx
    ret
NEWSYM membank0w16
    and ecx,0FFFFh
    cmp byte[SA1Enable],1
    je near membank0w16SA1
    cmp ecx,2000h
    jae .regs
    mov [wramdataa+ecx],ax
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
.regs
    test ecx,8000h
    jnz .romacc
    cmp ecx,48FFh
    ja .invaccess
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
;    mov ebx,ecx
;    shl ebx,2
;    add ebx,[regptw]
;    call dword near [ebx]
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .dsp1
    ret
.dsp1
    cmp byte[SFXEnable],1
    je .sfxram
    cmp byte[DSP1Type],2
    jne .nodsp1
    call DSP1Write16b
.nodsp1
    ret
.sfxram
    push ecx
    sub ecx,6000h
    and ecx,1fffh
    mov ebx,[sfxramdata]
    mov [ebx+ecx],ax
    xor ebx,ebx
    pop ecx
    ret

NEWSYM membank0r8SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov al,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov al,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor al,al
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r8

NEWSYM membank0r16SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov ax,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov ax,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor ax,ax
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r16

NEWSYM membank0w8SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],al
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],al
    ret
.romacc
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
    BWCheck2w8

NEWSYM membank0w16SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],ax
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],ax
    ret
.romacc
    ret
.regs
    cmp cx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
    BWCheck2w16

;*******************************************************
; ROM Only Access Banks (40 - 6F) / (C0 - FF)
;*******************************************************

NEWSYM memaccessspc7110r8


    xor al,al
    push ebx
    xor ebx,ebx
    mov bx,[SPCDecmPtr]
    add ebx,[romdata]
    add ebx,510000h
    mov al,[ebx]
    pop ebx

    dec word[SPCCompCounter]
    inc dword[SPCCompPtr]
    inc word[SPCDecmPtr]
    inc word[CurDecompSize]
    ret

    mov ebx,[romdata]
    add ebx,510000h
    mov al,[ebx+ecx]
    cmp cx,[CurDecompPtr]
    jb .noptr
    mov [CurDecompPtr],cx
    mov bx,cx
    sub bx,[PrevDecompPtr]
    inc bx
    mov [CurDecompSize],bx
.noptr
    xor ebx,ebx
    ret
NEWSYM memaccessspc7110r16
    mov ebx,[romdata]
    add ebx,510000h
    mov ax,[ebx+ecx]
    cmp cx,[CurDecompPtr]
    jb .noptr
    mov [CurDecompPtr],cx
    mov bx,cx
    sub bx,[PrevDecompPtr]
    add bx,2
    mov [CurDecompSize],bx
.noptr
    xor ebx,ebx
    ret
NEWSYM memaccessspc7110w8
    mov ebx,[romdata]
    add ebx,510000h
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
NEWSYM memaccessspc7110w16
    mov ebx,[romdata]
    add ebx,510000h
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret

NEWSYM memaccessbankr8
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM memaccessbankr16
    mov ebx,[snesmmap+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM memaccessbankw8
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret

NEWSYM memaccessbankw16
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret

NEWSYM memaccessbankr848mb
    test ecx,8000h
    jz .map2
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.map2
    mov ebx,[snesmap2+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM memaccessbankr1648mb
    test ecx,8000h
    jz .map2
    mov ebx,[snesmmap+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.map2
    mov ebx,[snesmap2+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

;*******************************************************
; SRAM Access Bank (70h)
;*******************************************************

NEWSYM sramaccessbankr8
    push ecx
    cmp bl,0F0h
    jne .notf0
    sub bl,80h
.notf0
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr8b
    pop ecx
    ret
NEWSYM sramaccessbankr16
    push ecx
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr16b
    pop ecx
    ret
NEWSYM sramaccessbankw8
    push ecx
    sub bl,70h
    cmp bl,0F0h
    jne .notf0
    sub bl,80h
.notf0
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw8b
    pop ecx
    ret
NEWSYM sramaccessbankw16
    push ecx
    cmp bl,0F0h
    jne .notf0
    sub bl,80h
.notf0
    sub bl,70h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw16b
    pop ecx
    ret

NEWSYM sramaccessbankr8s
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr8b
    pop ecx
    ret
NEWSYM sramaccessbankr16s
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankr16b
    pop ecx
    ret
NEWSYM sramaccessbankw8s
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw8b
    pop ecx
    ret
NEWSYM sramaccessbankw16s
    push ecx
    sub bl,78h
    shl ebx,15
    add ecx,ebx
    call sramaccessbankw16b
    pop ecx
    ret

NEWSYM sramaccessbankr8b
    cmp dword[ramsize],0
    je .noaccess
    push ecx
    and ecx,[ramsizeand]
    mov ebx,[sram]
    mov al,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret
.noaccess
    xor al,al
    xor ebx,ebx
    ret

NEWSYM sramaccessbankr16b
    cmp dword[ramsize],0
    je .noaccess
    mov ebx,[sram]
    push ecx
    and ecx,[ramsizeand]
    mov al,[ebx+ecx]
    inc ecx
    and ecx,[ramsizeand]
    mov ah,[ebx+ecx]
    pop ecx
    xor ebx,ebx
    ret
.noaccess
    xor ax,ax
    xor ebx,ebx
    ret

NEWSYM sramaccessbankw8b
    cmp dword[ramsize],0
    je .noaccess
    mov ebx,[sram]
    push ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    pop ecx
    mov dword[sramb4save],5*60
.noaccess
    xor ebx,ebx
    ret

NEWSYM sramaccessbankw16b
    cmp dword[ramsize],0
    je .noaccess
    mov ebx,[sram]
    push ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],al
    inc ecx
    and ecx,[ramsizeand]
    mov [ebx+ecx],ah
    pop ecx
    mov dword[sramb4save],5*60
.noaccess
    xor ebx,ebx
    ret

;*******************************************************
; WorkRAM/ExpandRAM Access Bank (7Eh)
;*******************************************************

NEWSYM wramaccessbankr8
;    mov ebx,[wramdata]
;    mov al,[ebx+ecx]
;    xor ebx,ebx
    mov al,[wramdataa+ecx]
    ret

NEWSYM wramaccessbankr16
;    mov ebx,[wramdata]
;    mov ax,[ebx+ecx]
;    xor ebx,ebx
    mov ax,[wramdataa+ecx]
    ret

NEWSYM wramaccessbankw8
;    mov ebx,[wramdata]
;    mov [ebx+ecx],al
;    xor ebx,ebx
    mov [wramdataa+ecx],al
    ret

NEWSYM wramaccessbankw16
;    mov ebx,[wramdata]
;    mov [ebx+ecx],ax
;    xor ebx,ebx
    mov [wramdataa+ecx],ax
    ret

;*******************************************************
; ExpandRAM Access Bank (7Fh)
;*******************************************************
NEWSYM eramaccessbankr8
;    mov ebx,[ram7f]
;    mov al,[ebx+ecx]
;    xor ebx,ebx
    mov al,[ram7fa+ecx]
    ret

NEWSYM eramaccessbankr16
;    mov ebx,[ram7f]
;    mov ax,[ebx+ecx]
;    xor ebx,ebx
    mov ax,[ram7fa+ecx]
    ret

NEWSYM eramaccessbankw8
;    mov ebx,[ram7f]
;    mov [ebx+ecx],al
;    xor ebx,ebx
    mov [ram7fa+ecx],al
    ret

NEWSYM eramaccessbankw16
;    mov ebx,[ram7f]
;    mov [ebx+ecx],ax
;    xor ebx,ebx
    mov [ram7fa+ecx],ax
    ret

;*******************************************************
; Invalid Access Bank (710000h-7DFFFFh)
;*******************************************************
NEWSYM invaccessbank
    xor eax,eax
    mov byte[invalid],1
    mov [invopcd],bl
    mov al,[previdmode]
    mov ah,0
    int 10h
    mov ah,9
    mov edx,.invalidbank
    int 21h
    xor eax,eax
    mov al,[invopcd]
    call printhex8
    mov ah,2
    mov dl,13
    int 21h
    mov ah,2
    mov dl,10
    int 21h
    jmp DosExit

SECTION .data
.invalidbank db 'Invalid Bank Access : $'
SECTION .text
    ret


;*******************************************************
; SA-1 Bank Accesses
;*******************************************************

NEWSYM regaccessbankr8SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov al,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov al,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor al,al
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r8

NEWSYM regaccessbankr16SA1
    test ecx,8000h
    jz .regacc
    mov ebx,[snesmmap+ebx*4]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
.regacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov ax,[wramdataa+ecx]
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov ax,[IRAM+ecx]
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptra+ecx*4-8000h]
    inc ecx
    mov ah,al
    call dword near [regptra+ecx*4-8000h]
    mov bl,al
    dec ecx
    mov al,ah
    mov ah,bl
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    xor ax,ax
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret
    BWCheck2r16

NEWSYM regaccessbankw8SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],al
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],al
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
.regs
    cmp ecx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret
    BWCheck2w8

NEWSYM regaccessbankw16SA1
    test ecx,8000h
    jnz .romacc
    cmp ecx,2000h
    jae .regs
    cmp byte[SA1Status],0
    jne .nowram
    mov [wramdataa+ecx],ax
    ret
.nowram
    cmp ecx,800h
    jae .invaccess
    mov [IRAM+ecx],ax
    ret
.romacc
    cmp byte[writeon],0
    jne .modrom
    ret
.modrom
    mov ebx,[snesmmap+ebx*4]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
.regs
    cmp cx,48FFh
    ja .invaccess
    call dword near [regptwa+ecx*4-8000h]
    inc ecx
    mov al,ah
    call dword near [regptwa+ecx*4-8000h]
    dec ecx
    xor ebx,ebx
    ret
.invaccess
    cmp ecx,6000h
    jae .bwram
    ret
.bwram
    BWCheck
    mov ebx,[CurBWPtr]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
    BWCheck2w16

NEWSYM SA1RAMaccessbankr8
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankr16
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov ax,[ebx+ecx]
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw8
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw16
    and ebx,03h
    shl ebx,16
    add ebx,[SA1RAMArea]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret


NEWSYM SA1RAMaccessbankr8b
    test byte[SA1Overflow+1],80h
    jnz .2bit
    and ebx,07h
    shl ebx,15
    test ecx,1
    jnz .4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    xor ebx,ebx
    and al,0Fh
    ret
.4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    xor ebx,ebx
    shr al,4
    ret
.2bit
    and ebx,0Fh
    shl ebx,14
    add ebx,[SA1RAMArea]
    test ecx,2
    jnz .bit1
    test ecx,1
    jnz .bit0
    shr ecx,2
    mov al,[ebx+ecx]
    and al,3
    xor ebx,ebx
    ret
.bit0
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,2
    and al,3
    xor ebx,ebx
    ret
.bit1
    test ecx,1
    jnz .bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,4
    and al,3
    xor ebx,ebx
    ret
.bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,6
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankr16b
    test byte[SA1Overflow+1],80h
    jnz .2bit
    and ebx,07h
    shl ebx,15
    test ecx,1
    jnz .4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov al,[ebx+ecx]
    and al,0Fh
    mov ah,[ebx+ecx]
    shr ah,4
    xor ebx,ebx
    ret
.4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    mov ah,[ebx+ecx+1]
    and ah,0Fh
    mov al,[ebx+ecx]
    shr al,4
    xor ebx,ebx
    ret
.2bit
    and ebx,0Fh
    shl ebx,14
    add ebx,[SA1RAMArea]
    test ecx,2
    jnz .bit1
    test ecx,1
    jnz .bit0
    shr ecx,2
    mov al,[ebx+ecx]
    and al,3
    mov ah,[ebx+ecx]
    shr ah,2
    and ah,3
    xor ebx,ebx
    ret
.bit0
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,2
    and al,2
    mov ah,[ebx+ecx]
    shr ah,4
    and ah,3
    xor ebx,ebx
    ret
.bit1
    test ecx,1
    jnz .bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,4
    and al,3
    mov ah,[ebx+ecx]
    shr ah,6
    xor ebx,ebx
    ret
.bit0b
    shr ecx,2
    mov al,[ebx+ecx]
    shr al,6
    mov ah,[ebx+ecx+1]
    and ah,3
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw8b
    test byte[SA1Overflow+1],80h
    jnz .2bit
    and ebx,07h
    shl ebx,15
    test ecx,1
    jnz .4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    and al,0Fh
    and byte[ebx+ecx],0F0h
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.4bitb
    shr ecx,1
    add ebx,[SA1RAMArea]
    and al,0Fh
    shl al,4
    and byte[ebx+ecx],0Fh
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.2bit
    and ebx,0Fh
    shl ebx,14
    add ebx,[SA1RAMArea]
    test ecx,2
    jnz .bit1
    test ecx,1
    jnz .bit0
    shr ecx,2
    and byte[ebx+ecx],0FCh
    and al,3
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.bit0
    shr ecx,2
    and byte[ebx+ecx],0F3h
    and al,3
    shl al,2
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.bit1
    test ecx,1
    jnz .bit0b
    shr ecx,2
    and byte[ebx+ecx],0CFh
    and al,3
    shl al,4
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret
.bit0b
    shr ecx,2
    and byte[ebx+ecx],03Fh
    and al,3
    shl al,6
    or byte[ebx+ecx],al
    xor ebx,ebx
    ret

NEWSYM SA1RAMaccessbankw16b
    push ecx
    push ebx
    call SA1RAMaccessbankw8b
    pop ebx
    pop ecx
    inc ecx
    mov al,ah
    call SA1RAMaccessbankw8b
    ret

SECTION .text

%macro GetBankLog 1
    cmp bl,0C0h
    jb %%illegal
    cmp bl,0D0h
    jb %%firstbank
    cmp bl,0E0h
    jb %%secondbank
    cmp bl,0F0h
    jb %%thirdbank
    mov %1,[SDD1BankA+3]
    jmp %%done
%%firstbank
    mov %1,[SDD1BankA]
    jmp %%done
%%secondbank
    mov %1,[SDD1BankA+1]
    jmp %%done
%%thirdbank
    mov %1,[SDD1BankA+2]
    jmp %%done
%%illegal
    mov %1,0Fh
%%done
%endmacro

SECTION .data
NEWSYM LatestBank, dd 0FFFFh
SECTION .text

EXTSYM SDD1_init
EXTSYM SDD1_get_byte

; Software decompression version
NEWSYM memaccessbankr8sdd1
    cmp byte[AddrNoIncr],0
    je near .failed

    cmp dword[Sdd1Mode],2
    je near .decompress

    mov [Sdd1Bank],ebx
    mov [Sdd1Addr],ecx
    mov [Sdd1NewAddr],ecx

    mov dword[Sdd1Mode],2
    push edx
    push eax
    push ecx

    and ecx,0FFFFh
    xor eax,eax
    GetBankLog al
    shl eax, 20
    mov edx, [Sdd1Bank]
    and edx, 0Fh
    shl edx, 16
    add eax, edx
    add eax, [romdata]
    add eax, ecx

    pushad
    push eax
    call SDD1_init
    pop eax
    popad

    pop ecx
    pop eax
    pop edx

.decompress
    cmp [Sdd1Bank],ebx
    jne .nomoredec
    cmp [Sdd1Addr],ecx
    je .yesdec
.nomoredec
    mov ebx,[snesmmap+ebx*4]
    mov al,[ebx+ecx]
    push eax
    mov eax,memtabler8+0C0h*4
    mov ebx,40h
.loopb
    mov dword[eax],memaccessbankr8
    add eax,4
    dec ebx
    jnz .loopb
    pop eax
    xor ebx,ebx
    ret
.yesdec
    pushad
    call SDD1_get_byte
    mov [.tmpbyte], al
    popad
    mov al, [.tmpbyte]
    ret

.failed
    push ebx
    call .nomoredec
    pop ebx
    jmp memaccessbankr8
SECTION .bss
.tmpbyte resb 1


