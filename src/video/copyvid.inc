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


%macro MMXStuff 0
%%1
    movq mm0,[esi]
    movq [es:edi],mm0
    movq mm1,[esi+8]
    movq [es:edi+8],mm1
    add esi,16
    add edi,16
    dec ecx
    jnz %%1
%endmacro

%macro FilterTest 1
    cmp byte[GUIOn],1
    jne %%nogui
    cmp byte[FilteredGUI],1
    jne %%nofilter
%%nogui
    cmp byte[antienab],1
    je near %1
%%nofilter
%endmacro


; Blends two 16-bit pixels at 50%.

; ax = first pixel, bx = second pixel
; ax (after) = resulting pixel

%macro SSHalfBlend 0
    shr eax,byte 1
    shr ebx,byte 1
    and eax,7befh
    and ebx,7befh
    add eax,ebx
%endmacro


; Horizontal recursive anti-aliasing,
; blurs edges without increasing size.

; ax = current pixel, bx = working pixel
; ecx = number of pixels (passed)

%macro SSInterpLineH 0
    mov ax,[esi]
    mov [es:edi],ax
    add esi,byte 2
    add edi,byte 2
    mov ecx,254
%%loop
    mov ax,[esi]
    mov bx,[esi-2]
    cmp ax,bx
    jne %%loop2
    mov [es:edi],ax
    mov [es:edi-2],bx
    jmp %%loop3
%%loop2
    SSHalfBlend
    mov [es:edi],ax
    mov [es:edi-2],ax
%%loop3
    add esi,byte 2
    add edi,byte 2
    dec ecx
    jnz %%loop
    mov ax,[esi]
    mov [es:edi],ax
    add esi,byte 2
    add edi,byte 2
%endmacro


; Used for 640x400 16bit...

; Kills the vertical stretch effect.

; ax = current pixel, bx = working pixel
; ecx = number of pixels (passed)

%macro SSInterpLineV 0
%%loop
    mov ax,[esi]
    mov bx,[esi-(288*2)]
    cmp ax,bx
    je %%loop2
    SSHalfBlend
%%loop2
    mov [es:edi],ax
    mov [es:edi+2],ax
    add esi,byte 2
    add edi,byte 4
    dec ecx
    jnz %%loop
%endmacro


; True high-resolution interpolation.

; Don't forget to skip every other line
; (on the screen, not in vidbuffer).

; ax = current pixel, bx = working pixel
; ecx = number of pixels (passed)

%macro SSInterpFull 1
%%loop
    mov ax,[esi]
    mov [es:edi],ax
    mov bx,[esi+2]
    SSHalfBlend
    mov [es:edi+2],ax
    mov ax,[esi]
    mov bx,[esi+(288*2)]
    SSHalfBlend
    mov [es:edi+(%1*2)],ax
    mov ax,[esi]
    mov bx,[esi+(288*2)+2]
    SSHalfBlend
    mov [es:edi+(%1*2)+2],ax
    add esi,byte 2
    add edi,byte 4
    dec ecx
    jnz %%loop
%endmacro


%macro FlipCheck 0
   cmp byte[FlipWait],0
   je %%noflip
   mov edx,3DAh            ;VGA status port
   in al,dx
   test al,8
   jz %%noflip
   mov eax,4F07h
   mov bh,00h
   mov bl,00h
   xor ecx,ecx
   mov dx,[NextLineStart]
   mov [LastLineStart],dx
   int 10h
   mov byte[FlipWait],0
%%noflip
%endmacro


SECTION .text

NEWSYM ResetTripleBuf
   mov byte[FlipWait],0
   mov dword[VidStartDraw],0
   mov byte[CVidStartAd],0
   ret

%ifdef __MSDOS__
GUITripleBuffer:
   cmp byte[TriplebufTech],0
   je near .tech2
   cmp byte[ApplyStart],0
   je .notstartedb
   mov byte[ApplyStart],0
   cmp word[LastLineStart],0
   je .notstartedb
   mov eax,4F07h
   mov bh,00h
   mov bl,00h
   xor ecx,ecx
   xor edx,edx
   int 10h
.notstartedb
   mov byte[FlipWait],0
   mov dword[VidStartDraw],0
   mov byte[CVidStartAd],0
   mov dword[LastLineStart],0
   ret

.tech2
   xor ecx,ecx
   mov cl,[cvidmode]
   cmp byte[VidModeComp+ecx],0
   je .notbuf
   cmp byte[Triplebufen],0
   je .notbuf
   jmp .yestbuf
.notbuf
   ret
.yestbuf
   cmp byte[ApplyStart],0
   je .notstarted
   mov eax,4F07h
   mov bh,00h
   mov bl,02h
   xor ecx,ecx
   xor edx,edx
   int 10h
   cmp byte[ApplyStart],4
   jne .nocheck
   cmp al,4Fh
   jne .failed
   cmp ah,0
   ja .failed
.nocheck
   mov dword[VidStartDraw],0
   mov byte[CVidStartAd],0
   mov byte[ApplyStart],0
.notstarted
   ret
.failed
   mov byte[TriplebufTech],1
   ret

PostTripleBuffer:
   xor ecx,ecx
   mov cl,[cvidmode]
   cmp byte[VidModeComp+ecx],0
   je .notbuf
   cmp byte[Triplebufen],0
   je .notbuf
   jmp .yestbuf
.notbuf
   ret
.yestbuf
   xor ecx,ecx
   cmp byte[CVidStartAd],2
   je .nooffset0
   mov cl,[cvidmode]
   mov ecx,[VidModeSize+ecx*4]
   cmp byte[CVidStartAd],0
   je .nooffset0
   add ecx,ecx
.nooffset0
   mov [VidStartDraw],ecx
   inc byte[CVidStartAd]
   cmp byte[CVidStartAd],3
   jne .notof
   mov byte[CVidStartAd],0
.notof
   ret

PreTripleBuffer2:
   cmp byte[TriplebufTech],0
   je near PreTripleBuffer
   xor ecx,ecx
   mov cl,[cvidmode]
   cmp byte[VidModeComp+ecx],0
   je .notbuf
   cmp byte[Triplebufen],0
   jne .yestbuf
.notbuf
   ret
.yestbuf
   cmp byte[FlipWait],0
   je .noflip
   mov edx,3DAh            ;VGA status port
.loop
   in al,dx
   test al,8
   jz .loop
   mov eax,4F07h
   mov bh,00h
   mov bl,00h
   xor ecx,ecx
   mov dx,[NextLineStart]
   mov [LastLineStart],dx
   int 10h
   mov byte[FlipWait],0
.noflip
   xor ecx,ecx
   cmp byte[CVidStartAd],2
   je .nooffset0
   mov cl,[cvidmode]
   mov ecx,[VidModeLine+ecx*4]
   cmp byte[CVidStartAd],0
   je .nooffset0
   add ecx,ecx
.nooffset0
   mov [NextLineStart],ecx
   mov byte[ApplyStart],1
   mov byte[FlipWait],1
   ret

PreTripleBuffer:
   xor ecx,ecx
   mov cl,[cvidmode]
   cmp byte[VidModeComp+ecx],0
   je .notbuf
   cmp byte[Triplebufen],0
   jne .yestbuf
.notbuf
   ret
.yestbuf
   cmp byte[ApplyStart],2
   jne .noflip
.notflipped
; *** I have no idea why this code doesn't work (freezes on NVidia cards)
;   mov eax,4F07h
;   mov bx,04h
;   int 10h
;   or ah,ah
;   jnz .noflip
;   or cx,cx
;   jz .notflipped
.noflip
   mov eax,4F07h
   mov bh,00h
   mov bl,02h
   xor ecx,ecx
   cmp byte[CVidStartAd],0
   je .nooffset0
   mov cl,[cvidmode]
   mov ecx,[VidModeSize+ecx*4]
   cmp byte[CVidStartAd],1
   je .nooffset0
   add ecx,ecx
.nooffset0
   xor edx,edx
   int 10h
   cmp byte[ApplyStart],4
   jne .nocheck
   cmp al,4Fh
   jne .failed
   cmp ah,0
   ja .failed
   mov byte[ApplyStart],0
.nocheck
   cmp byte[ApplyStart],2
   je .skipcheckb
   inc byte[ApplyStart]
.skipcheckb
   ret
.failed
   mov byte[Triplebufen],0
   ret
%endif

SECTION .data
; Please don't break this again. :)
VidModeSize dd 0,0,0,0,0,0,0,320*240,320*240*2,320*480,320*480*2,512*384
            dd 512*384*2,640*400,640*400*2,640*480,640*480*2,800*600,800*600*2
VidModeLine dd 0,0,0,0,0,0,0,240,240,480,480,384,384,400,400,480,480,600,600
NEWSYM VidStartDraw, dd 0
VidModeComp db 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1
CVidStartAd db 0
ApplyStart db 4

SECTION .bss
NEWSYM NextLineStart, resd 1
NEWSYM LastLineStart, resd 1
NEWSYM FlipWait, resb 1
NEWSYM TriplebufTech, resb 1
SECTION .text

%ifdef __MSDOS__
NEWSYM DosDrawScreen
   cmp byte[curblank],40h
   je .nocopy
   call PreTripleBuffer2
   call PostTripleBuffer
.nocopy
   call ScreenShow
   FlipCheck
   ret

NEWSYM DosDrawScreenB
   cmp byte[curblank],40h
   je .nocopy
   call GUITripleBuffer
.nocopy
   call ScreenShow
   ret

ScreenShow:
    cmp byte[debugdisble],0
    je .debug
    cmp byte[cvidmode],2
    je near copymodeq256
.debug
    cmp byte[cvidmode],0
    je near copymodeq224
    cmp byte[cvidmode],1
    je near copymodeq240
    cmp byte[cvidmode],3
    je near copymodex224
    cmp byte[cvidmode],4
    je near copymodex240
    cmp byte[cvidmode],5
    je near copymodex256
    cmp byte[cvidmode],6
    je near copyvesa12640x480x16b
    cmp byte[cvidmode],7
    je near copyvesa2320x240x8b
    cmp byte[cvidmode],8
    je near copyvesa2320x240x16b
    cmp byte[cvidmode],9
    je near copyvesa2320x480x8b
    cmp byte[cvidmode],10
    je near copyvesa2320x480x16b
    cmp byte[cvidmode],11
    je near copyvesa2512x384x8b
    cmp byte[cvidmode],12
    je near copyvesa2512x384x16b
    cmp byte[cvidmode],13
    je near copyvesa2640x400x8b
    cmp byte[cvidmode],14
    je near copyvesa2640x400x16b
    cmp byte[cvidmode],15
    je near copyvesa2640x480x8b
    cmp byte[cvidmode],16
    je near copyvesa2640x480x16b
    cmp byte[cvidmode],17
    je near copyvesa2800x600x8b
    cmp byte[cvidmode],18
    je near copyvesa2800x600x16b
    cmp byte[curblank],40h
    jne .startcopy
    inc byte[curfps2]
;    call sounddisplay
    call hextestoutput
.startcopy
    jmp copymodeq256

;NEWSYM dosvidpastecopyscr
;   cmp byte[curblank],40h
;   je .nocopy
;   call GUITripleBuffer
;.nocopy
;   call ScreenShowGUI
;   ret

;ScreenShowGUI:
;    cmp byte[cvidmode],0
;    je near copymodeq224
;    cmp byte[cvidmode],1
;    je near copymodeq240
;    cmp byte[cvidmode],2
;    je near copymodeq256
;    cmp byte[cvidmode],3
;    je near copymodex224
;    cmp byte[cvidmode],4
;    je near copymodex240
;    cmp byte[cvidmode],5
;    je near copymodex256
;    cmp byte[cvidmode],6
;    je near copyvesa12640x480x16bgui
;    cmp byte[cvidmode],7
;    je near copyvesa2320x240x8b
;    cmp byte[cvidmode],8
;    je near copyvesa2320x240x16bgui
;    cmp byte[cvidmode],9
;    je near copyvesa2320x480x8bgui
;    cmp byte[cvidmode],10
;    je near copyvesa2320x480x16bgui
;    cmp byte[cvidmode],11
;    je near copyvesa2512x384x8b
;    cmp byte[cvidmode],12
;    je near copyvesa2512x384x16bgui
;    cmp byte[cvidmode],13
;    je near copyvesa2640x400x8b
;    cmp byte[cvidmode],14
;    je near copyvesa2640x400x16b
;    cmp byte[cvidmode],15
;    je near copyvesa2640x480x8bgui
;    cmp byte[cvidmode],16
;    je near copyvesa2640x480x16bgui
;    cmp byte[cvidmode],17
;    je near copyvesa2800x600x8b
;    cmp byte[cvidmode],18
;    je near copyvesa2800x600x16b
;    jmp copymodeq256
%endif


%ifdef __MSDOS__
;*******************************************************
; CopyModeX 224     Copies buffer into unchained 320x224
;*******************************************************

NEWSYM copymodex224
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy

    ; video memory selector
    push es
    mov ax,[selcA000]
    mov es,ax

    mov esi,[vidbuffer]

    ; center on output screen
    mov edi,(320-256)/2/4

    ; address of first source line to copy
    add esi,(16+256+16)+16

    ; 2nd page address
    mov eax,(320*225)/4

    mov bl,224-2
    jmp copymodexloop

;*******************************************************
; CopyModeX 240     Copies buffer into unchained 320x240
;*******************************************************

NEWSYM copymodex240
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy

    ; video memory selector
    push es
    mov ax,[selcA000]
    mov es,ax

    mov esi,[vidbuffer]

    ; center on output screen
    mov edi,(320-256)/2/4

    cmp word[resolutn],224
    jne .res239
    mov edi,(8*320+32)/4
.res239

    ; address of first source line to copy
    add esi,(16+256+16)+16

    ; 2nd page address
    mov eax,(320*240)/4

    mov bl,[resolutn]
    sub bl,2
    jmp copymodexloop

;*******************************************************
; CopyModeX 256     Copies buffer into unchained 320x256
;*******************************************************

NEWSYM copymodex256
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy

    ; video memory selector
    push es
    mov ax,[selcA000]
    mov es,ax

    mov esi,[vidbuffer]

    ; center on output screen
    mov edi,(8*320+(320-256)/2)/4

    cmp word[resolutn],224
    jne .res239
    mov edi,(16*320+(320-256)/2)/4
.res239

    ; address of first source line to copy
    add esi,(16+256+16)+16

    ; 2nd page address
    mov eax,(320*256)/4

    mov bl,[resolutn]
    sub bl,2

;eax = VGA address of 2nd page
;edi = offset in current page of first line
;esi = address of first line to be copied
;bl = number of lines to copy
copymodexloop:
    ; select output video page
    mov bh,[whichpage]
    test bh,bh
    mov bh,1
    jz .pagea
    xor eax,eax
    mov bh,0
.pagea

    mov [whichpage],bh
    add edi,eax
    mov [.pageaddress],eax

; register allocation
; bl = line counter (0-total lines)
; bh = other line counter (descriptive, eh?) (0-8)
; ebp = plane counter
; ch = plane enable bit
; cl = 4-pixel copy counter
; edx = pixel processing & I/O address
; eax = pixel processing & I/O data

    mov edx,03C4h

.loopa
    mov ebp,4
    mov ch,1
    cmp bl,8
    mov bh,bl
    jb .loopb
    mov bh,8

.loopb
    ; set write plane
    mov ah,ch
    add ch,ch
    mov al,02h
    out dx,ax

    push ebx
    push edi
    push esi

.loopc
    ; loop count
    mov cl,16
.loopd
    mov al,[esi+8]
    mov ah,[esi+12]
    shl eax,16
    mov al,[esi+0]
    mov ah,[esi+4]
    add esi,byte 16
    mov [es:edi],eax
    add edi,byte 4
    dec cl
    jnz .loopd

    add esi,byte 16+16
    add edi,byte (320-256)/4
    dec bh
    jnz .loopc

    pop esi
    pop edi
    pop ebx

    inc esi
    dec ebp
    jnz .loopb

    ; next line
    add esi,(16+256+16)*8-4
    add edi,320*8/4
    sub bl,bh
    jnz .loopa

    pop es

    ; flip pages by setting new offset
    mov edx,03D4h
    mov al,0Ch
    mov ah,[.pageaddress+1]
    out dx,ax
    mov al,0Dh
    mov ah,[.pageaddress]
    out dx,ax

    ret

SECTION .bss
.startesi resd 1
.startedi resd 1
.pageaddress resd 1
.linecount resd 1

NEWSYM whichpage, resb 1          ; active page and visual page locations
SECTION .text

;*******************************************************
; CopyModeQ 224       Copies buffer into chained 256x224
;*******************************************************

NEWSYM copymodeq224
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy

    ; video memory selector
    push es
    mov ax,[selcA000]
    mov es,ax

    mov esi,[vidbuffer]

    ; center on output screen
    mov edi,256

    ; address of first source line to copy
    add esi,(16+256+16)+16

    mov bl,224-2
    jmp copymodeqloop


;*******************************************************
; CopyModeQ 240       Copies buffer into chained 256x240
;*******************************************************

NEWSYM copymodeq240
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy

    ; video memory selector
    push es
    mov ax,[selcA000]
    mov es,ax

    mov esi,[vidbuffer]

    ; center on output screen
    mov edi,256

    cmp word[resolutn],224
    jne .res239
    mov edi,8*256+256
.res239

    ; address of first source line to copy
    add esi,(16+256+16)+16

    mov bl,[resolutn]
    sub bl,2
    jmp copymodeqloop


;********************************************************
; CopyModeQ 256       Copies buffer into chained 256x256
;********************************************************

NEWSYM copymodeq256
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy

    ; video memory selector
    push es
    mov ax,[selcA000]
    mov es,ax

    mov esi,[vidbuffer]

    ; center on output screen
    mov edi,8*256+256

    cmp word[resolutn],224
    jne .res239
    mov edi,16*256+256
.res239

    ; address of first source line to copy
    add esi,(16+256+16)+16

    mov bl,[resolutn]
    sub bl,2

;edi = offset in output of first line
;esi = address of first line to be copied
;bl = number of lines to copy*dl
copymodeqloop:
    cmp byte[MMXSupport],1
    je near .loopb
.loopa
    mov ecx,256/4
    rep movsd
    add esi,16+16
    dec bl
    jnz .loopa
    jmp .done
.loopb
    mov ecx,256/16
    MMXStuff
    add esi,16+16
    dec bl
    jnz .loopb
    emms

.done
    pop es
    ret


;*******************************************************
; Copy VESA2 320x240x8b  Copies buffer to 320x240x8bVBE2
;*******************************************************
;     Input:    AX   = 4F07h   VBE Set/Get Display Start Control
;               BH   = 00h          Reserved and must be 00h
;               BL   = 00h          Set Display Start
;                    = 01h          Get Display Start
;                    = 80h          Set Display Start during Vertical
;     Retrace
;               CX   =         First Displayed Pixel In Scan Line
;                              (Set Display Start only)
;               DX   =         First Displayed Scan Line (Set Display Start
;     only)

NEWSYM copyvesa2320x240x8b
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,320+32
    cmp word[resolutn],224
    jne .res239
    add edi,8*320
.res239
    add edi,[VidStartDraw]
    add esi,16+256+16+16
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[ScreenScale],1
    je .fullscreen

    cmp byte[MMXSupport],1
    je near .loopb
.loopa
    mov ecx,256/4
    rep movsd
    add esi,16+16
    add edi,32+32
    dec dl
    jnz .loopa
    jmp .done
.loopb
    mov ecx,256/16
    MMXStuff
    add esi,16+16
    add edi,32+32
    dec dl
    jnz .loopb
    emms
    jmp .done

.fullscreen
    sub edi,32
.fsloopa
    mov ecx,256/4
    jmp .fsloopb
.fsloopb
    mov eax,[esi]
    mov [es:edi],al
    mov [es:edi+1],eax
    add esi,byte 4
    add edi,byte 5
    dec ecx
    jnz .fsloopb
    add esi,16+16
    dec dl
    jnz .fsloopa

.done
    pop es
    ret


;*******************************************************
; Copy VESA2 320x480x8b  Copies buffer to 320x480x8bVBE2
;*******************************************************

;NEWSYM copyvesa2320x480x8bgui
;    jmp copyvesa2320x480x8b
;    mov byte[CurrentGUIOn],1
;    jmp copyvesa2320x480x8b.nogui

NEWSYM copyvesa2320x480x8b
;    mov byte[CurrentGUIOn],0
; .nogui
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,320*2+32
    cmp word[resolutn],224
    jne .res239
    add edi,8*320
.res239
    add edi,[VidStartDraw]
    add esi,16+256+16+16
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[ScreenScale],1
    je near .fullscreen
    cmp byte[scanlines],1
    je near .scanlines

;    cmp byte[GUIOn],1
;    je .loopa
;    cmp byte[f3menuen],1
;    je .loopa
;    cmp byte[ForceNewGfxOff],1
;    je .loopa
;    cmp byte[newengen],1
;    je near copyvesa2320x480x8ng

    cmp byte[MMXSupport],1
    je .loopb
.loopa
    mov ecx,256/4
    rep movsd
    sub esi,256
    add edi,32+32
    mov ecx,256/4
    rep movsd
    add esi,16+16
    add edi,32+32
    dec dl
    jnz .loopa
    jmp .done
.loopb
    mov ecx,256/16
    MMXStuff
    sub esi,256
    add edi,32+32
    mov ecx,256/16
    MMXStuff
    add esi,16+16
    add edi,32+32
    dec dl
    jnz .loopb
    emms
    jmp .done

.scanlines
    cmp byte[MMXSupport],1
    je .sloopb
.sloopa
    mov ecx,256/4
    rep movsd
    add esi,16+16
    add edi,32+320+32
    dec dl
    jnz .sloopa
    jmp .done
.sloopb
    mov ecx,256/16
    MMXStuff
    add esi,16+16
    add edi,32+320+32
    dec dl
    jnz .sloopb
    emms
    jmp .done

.fullscreen
    sub edi,32
    cmp byte[scanlines],1
    je .fsloopb
.fsloopa
    mov ecx,256/4
    call .fsloopc
    sub esi,256
    mov ecx,256/4
    call .fsloopc
    add esi,16+16
    dec dl
    jnz .fsloopa
    jmp .done
.fsloopb
    mov ecx,256/4
    call .fsloopc
    add esi,16+16
    add edi,320
    dec dl
    jnz .fsloopb
    jmp .done
.fsloopc
    mov eax,[esi]
    mov [es:edi],al
    mov [es:edi+1],eax
    add esi,byte 4
    add edi,byte 5
    dec ecx
    jnz .fsloopc
    ret

.done
    pop es
    ret


;copyvesa2320x480x8ng:
;    jmp .mode7hires
;    mov ebx,1
;.loopa
;    cmp dword[ignor512],0
;    je .ignore
;    test byte[intrlng+ebx],01h
;    jnz near .interlaced
;.ignore
;    cmp byte[Mode7HiRes],0
;    je .nomode7hires
;    test byte[mosenng+ebx],1
;    jz .yesmode7hires
;    cmp byte[mosszng+ebx],0
;    jne .nomode7hires
;.yesmode7hires
;    test byte[intrlng+ebx],40h
;    jnz .nomode7hires
;    cmp byte[BGMA+ebx],7
;    jne .nomode7hires
;    cmp byte[BGMA+ebx+1],7
;    je near .mode7hires
;.nomode7hires
;    mov ecx,64
;.a
;    mov eax,[esi]
;    mov [es:edi],eax
;    add esi,4
;    add edi,4
;    dec ecx
;    jnz .a
;    mov ecx,64
;    add edi,64
;    sub esi,256
;.a2r
;    mov eax,[esi]
;    mov [es:edi],eax
;    add esi,4
;    add edi,4
;    dec ecx
;    jnz .a2r
;.returnloop
;    add esi,32
;    add edi,64
;    inc ebx
;    dec dl
;    jnz near .loopa
;    pop es
;    ret
;
;.mode7hires
;    mov ecx,64
;.a7
;    mov eax,[esi]
;    mov [es:edi],eax
;    add esi,4
;    add edi,4
;    dec ecx
;    jnz .a7
;    mov ecx,64
;    add edi,64
;    add esi,75036-256
;.a2r7
;    mov eax,[esi]
;    mov [es:edi],eax
;    add esi,4
;    add edi,4
;    dec ecx
;    jnz .a2r7
;    sub esi,75036
;    jmp .returnloop
;
;.interlaced
;    test byte[cfield],1
;    jz .b
;    add edi,320
;.b
;    mov ecx,64
;.ai
;    mov eax,[esi]
;    mov [es:edi],eax
;    add esi,4
;    add edi,4
;    dec ecx
;    jnz .ai
;    test byte[cfield],1
;    jnz .bi
;    add edi,320
;.bi
;    jmp .returnloop


;*******************************************************
; Copy VESA2 800x600x8b  Copies buffer to 800x600x8bVBE2
;*******************************************************

NEWSYM copyvesa2800x600x8b
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,60*800+144
    cmp word[resolutn],224
    jne .res239
    add edi,8*800
.res239
    add edi,[VidStartDraw]
    add esi,16+256+16+16
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[smallscreenon],1
    je near .smallscreen

    cmp byte[scanlines],1
    je .loopa2
.loopa
    mov ecx,256/2
    call .loopa3
    sub esi,256
    add edi,144+144
    mov ecx,256/2
    call .loopa3
    add esi,16+16
    add edi,144+144
    dec dl
    jnz .loopa
    jmp .done
.loopa2
    mov ecx,256/2
    call .loopa3
    add esi,16+16
    add edi,144+800+144
    dec dl
    jnz .loopa2
    jmp .done
.loopa3
    mov al,[esi]
    mov bl,[esi+1]
    mov ah,al
    mov bh,bl
    mov [es:edi],ax
    mov [es:edi+2],bx
    add esi,byte 2
    add edi,byte 4
    dec ecx
    jnz .loopa3
    ret

.smallscreen
    add edi,120*800+128
    cmp byte[MMXSupport],1
    je .ssloopb
.ssloopa
    mov ecx,256/4
    rep movsd
    add esi,16+16
    add edi,272+272
    dec dl
    jnz .ssloopa
    jmp .done
.ssloopb
    mov ecx,256/16
    MMXStuff
    add esi,16+16
    add edi,272+272
    dec dl
    jnz .ssloopb
    jmp .done

.done
    pop es
    ret


;*********************************************************
; Copy VESA2 800x600x16b  Copies buffer to 800x600x16bVBE2
;*********************************************************

NEWSYM copyvesa2800x600x16b
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,60*800*2+144*2
    cmp word[resolutn],224
    jne .res239
    add edi,8*800*2
.res239
    add edi,[VidStartDraw]
    add esi,16*2+256*2+16*2+16*2
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[smallscreenon],1
    je near .smallscreen

    FilterTest .interpolate

    cmp byte[scanlines],1
    je .loopa2
.loopa
    mov ecx,256/2
    call .loopa3
    sub esi,256*2
    add edi,144*2+144*2
    mov ecx,256/2
    call .loopa3
    add esi,16*2+16*2
    add edi,144*2+144*2
    dec dl
    jnz .loopa
    jmp .done
.loopa2
    mov ecx,256/2
    call .loopa3
    add esi,16*2+16*2
    add edi,144*2+800*2+144*2
    dec dl
    jnz .loopa2
    jmp .done
.loopa3
    mov ax,[esi]
    mov bx,[esi+2]
    mov [es:edi],ax
    mov [es:edi+2],ax
    mov [es:edi+4],bx
    mov [es:edi+6],bx
    add esi,byte 4
    add edi,byte 8
    dec ecx
    jnz .loopa3
    ret

.smallscreen
    add edi,120*800*2+128*2
    cmp byte[MMXSupport],1
    je .ssloopb
.ssloopa
    mov ecx,256/4*2
    rep movsd
    add esi,16*2+16*2
    add edi,272*2+272*2
    dec dl
    jnz .ssloopa
    jmp .done
.ssloopb
    mov ecx,256/16*2
    MMXStuff
    add esi,16*2+16*2
    add edi,272*2+272*2
    dec dl
    jnz .ssloopb
    jmp .done

.interpolate
    mov ecx,256
    SSInterpFull 800
    add esi,16*2+16*2
    add edi,144*2+800*2+144*2
    dec dl
    jnz near .interpolate

.done
    pop es
    ret


;*******************************************************
; Copy VESA2 640x400x8b  Copies buffer to 640x400x8bVBE2
;*******************************************************

NEWSYM copyvesa2640x400x8b
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,640+20*640+64
    cmp word[resolutn],224
    jne .res239
    add edi,12*640
.res239
    add edi,[VidStartDraw]
    add esi,16+256+16+16
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[ScreenScale],1
    je near .fullscreen
    cmp byte[smallscreenon],1
    je near .smallscreen

.loopa
    mov ecx,256
    call .loopa2
    sub esi,256
    add edi,64+64
    mov ecx,256
    call .loopa2
    add esi,16+16
    add edi,64+64
    dec dl
    jz near .done
    mov ecx,256
    call .loopa2
    add esi,16+16
    add edi,64+64
    dec dl
    jnz .loopa
    jmp .done
.loopa2
    mov al,[esi+1]
    xor ebx,ebx
    mov ah,al
    mov bl,[esi]
    shl eax,16
    mov bh,bl
    add esi,byte 2
    add eax,ebx
    mov [es:edi],eax
    sub ecx,byte 2
    lea edi,[edi+4]
    jnz .loopa2
    ret

.smallscreen
    add edi,60*640+128
    cmp byte[MMXSupport],1
    je .ssloopb
.ssloopa
    mov ecx,256/4
    rep movsd
    add esi,16+16
    add edi,192+192
    dec dl
    jnz .ssloopa
    jmp .done
.ssloopb
    mov ecx,256/16
    MMXStuff
    add esi,16+16
    add edi,192+192
    dec dl
    jnz .ssloopb
    emms
    jmp .done

.fullscreen
    sub edi,64
.fsloopa
    mov ecx,256/4
    call .fsloopb
    add esi,16+16
    mov ecx,256/4
    call .fsloopb
    sub esi,256
    mov ecx,256/4
    dec dl
    jz near .done
    call .fsloopb
    add esi,16+16
    dec dl
    jnz .fsloopa
    jmp .done
.fsloopb
    mov ebx,1
    call .fsloopc
    sub esi,byte 1
    mov ebx,4
    call .fsloopc
    dec ecx
    jnz .fsloopb
    ret
.fsloopc
    mov al,[esi]
    mov [es:edi],al
    inc esi
    mov [es:edi+1],al
    add edi,byte 2
    dec ebx
    jnz .fsloopc
    ret

.done
    pop es
    ret


NEWSYM copyvesa2640x400x16b
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,640*2+20*640*2+64*2
    cmp word[resolutn],224
    jne .res239
    add edi,12*640*2
.res239
    add edi,[VidStartDraw]
    add esi,16*2+256*2+16*2+16*2
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[ScreenScale],1
    je near .fullscreen
    cmp byte[smallscreenon],1
    je near .smallscreen

.loopa
    mov ecx,256
    call .loopa2
    add esi,16*2+16*2
    add edi,64*2+64*2
    mov ecx,256
    SSInterpLineV
    sub esi,256*2
    add edi,64*2+64*2
    dec dl
    jz near .done
    mov ecx,256
    call .loopa2
    add esi,16*2+16*2
    add edi,64*2+64*2
    dec dl
    jnz near .loopa
    jmp .done
.loopa2
    mov ax,[esi]
    mov [es:edi],ax
    mov [es:edi+2],ax
    add esi,byte 2
    add edi,byte 4
    dec ecx
    jnz .loopa2
    ret

.smallscreen
    add edi,60*640*2+128*2
    cmp byte[MMXSupport],1
    je .ssloopb
.ssloopa
    mov ecx,256/4*2
    rep movsd
    add esi,16*2+16*2
    add edi,192*2+192*2
    dec dl
    jnz .ssloopa
    jmp .done
.ssloopb
    mov ecx,256/16*2
    MMXStuff
    add esi,16*2+16*2
    add edi,192*2+192*2
    dec dl
    jnz .ssloopb
    emms
    jmp .done

.fullscreen
    sub edi,64*2
.fsloopa
    mov ecx,256/4
    call .fsloopb
    sub esi,256*2
    mov ecx,256/4
    call .fsloopb
    add esi,16*2+16*2
    dec dl
    jz near .done
    mov ecx,256/4
    call .fsloopb
    add esi,16*2+16*2
    dec dl
    jnz .fsloopa
    jmp .done
.fsloopb
    mov ebx,1
    call .fsloopc
    sub esi,byte 2
    mov ebx,4
    call .fsloopc
    dec ecx
    jnz .fsloopb
    ret
.fsloopc
    mov ax,[esi]
    mov [es:edi],ax
    mov [es:edi+2],ax
    add esi,byte 2
    add edi,byte 4
    dec ebx
    jnz .fsloopc
    ret

.done
    pop es
    ret

;*******************************************************
; Copy VESA2 640x480x8b  Copies buffer to 640x480x8bVBE2
;*******************************************************
%endif

SECTION .bss
NEWSYM res640, resb 1
NEWSYM res480, resb 1

SECTION .data

NEWSYM EagleHold, dd 0
NEWSYM CurrentGUIOn, dd 0
vesavaland dd 0,0
mmxvalanda dd 11111111110000001111111111000000b,11111111110000001111111111000000b
mmxvalandb dd 00000000000111110000000000011111b,00000000000111110000000000011111b
SECTION .text

%ifdef __MSDOS__
NEWSYM copyvesa2640x480x8bgui
    mov byte[CurrentGUIOn],1
    cmp byte[smallscreenon],1
    je near smallscreen640x480x8b
    cmp byte[ScreenScale],1
    je near smallscreen640x480x8b.fullscreen
    cmp byte[antienab],1
    je near proceagle
    cmp byte[scanlines],1
    je near copyvesa2640x480x8bs
    mov byte[res640],1
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    jmp copyvesa2640x480x8b.startcopy2

NEWSYM copyvesa2640x480x8b
    mov byte[CurrentGUIOn],0
    cmp byte[smallscreenon],1
    je near smallscreen640x480x8b
    cmp byte[ScreenScale],1
    je near smallscreen640x480x8b.fullscreen

    FilterTest proceagle

    cmp byte[scanlines],1
    je near copyvesa2640x480x8bs
    mov byte[res640],1
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    cmp byte[f3menuen],1
    je .startcopy2
    cmp byte[ForceNewGfxOff],0
    jne .startcopy2
    cmp byte[newengen],0
    jne near copyvesa2640x480x8ng
.startcopy2
    mov dword[ignor512],0
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,32*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*640+32*2
.res239
    add edi,[VidStartDraw]
    add esi,16+256+32
    xor eax,eax
    mov ebx,hirestiledat+1
    mov dl,[resolutn]
    dec dl
    dec dl
.loopa
    cmp byte[Triplebufen],1
    je .ignorehr
    cmp byte[ebx],1
    je near .yeshires
.ignorehr
    mov ecx,128
.a
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
    mov ecx,128
    add edi,64*2
    sub esi,256
.a2r
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2r
.returnloop
    add esi,32
    add edi,64*2
    inc ebx
    dec dl
    jnz .loopa
    pop es
    cmp byte[Triplebufen],1
    je .ignorehr2
    xor byte[res512switch],1
.ignorehr2
    ret
.yeshires
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightside
    mov ecx,256
.b
    mov al,[esi]
    mov [es:edi],al
    mov [es:edi+640],al
    inc esi
    add edi,byte 2
    dec ecx
    jnz .b
    add edi,640
    jmp .returnloop
.rightside
    mov ecx,256
.b2
    mov al,[esi]
    mov [es:edi+1],al
    mov [es:edi+641],al
    inc esi
    add edi,byte 2
    dec ecx
    jnz .b2
    add edi,640
    jmp .returnloop

NEWSYM copyvesa2640x480x8ng
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,32*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*640+32*2
.res239
    add edi,[VidStartDraw]
    add esi,16+256+32
    xor eax,eax
    mov ebx,1
    mov dl,[resolutn]
    dec dl
    dec dl
.loopa
    mov ecx,256
    cmp dword[ignor512],0
    je .ignore
    test byte[intrlng+ebx],01h
    jnz near .interlaced
    cmp byte[BGMA+ebx],5
    je near .hires
    cmp byte[BGMA+ebx],6
    je near .hires
    cmp byte[Mode7HiRes],0
    je .nomode7hires
    test byte[mosenng+ebx],1
    jz .yesmode7hires
    cmp byte[mosszng+ebx],0
    jne .nomode7hires
.yesmode7hires
    test byte[intrlng+ebx],40h
    jnz .nomode7hires
    cmp byte[BGMA+ebx],7
    jne .nomode7hires
    cmp byte[BGMA+ebx+1],7
    je near .mode7hires
.nomode7hires
.ignore
    mov ecx,128
.a
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
    mov ecx,128
    add edi,64*2
    sub esi,256
.a2r
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2r
.returnloop
    add esi,32
    add edi,64*2 ;+640
    inc ebx
    dec dl
    jnz near .loopa
    mov dword[ignor512],0
    pop es
    ret
.hires
    mov ecx,128
.a3
    mov al,[esi+1]
    mov ah,[esi+75037]
    shl eax,16
    mov al,[esi]
    mov ah,[esi+75036]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a3
    add edi,64*2
    sub esi,256
    mov ecx,128
.a6
    mov al,[esi+1]
    mov ah,[esi+75037]
    shl eax,16
    mov al,[esi]
    mov ah,[esi+75036]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a6
    jmp .returnloop

.interlaced
    mov ecx,128
    cmp byte[BGMA+ebx],5
    je .hiresi
    cmp byte[BGMA+ebx],6
    je .hiresi
    test byte[cfield],1
    jz .b
    add edi,640
.b
.a2
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2
    test byte[cfield],1
    jnz .bi
    add edi,640
.bi
    jmp .returnloop

.hiresi
    test byte[cfield],1
    jz .b2
    add edi,640
.b2
.a4
    mov al,[esi+1]
    mov ah,[esi+75037]
    shl eax,16
    mov al,[esi]
    mov ah,[esi+75036]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a4
    test byte[cfield],1
    jnz .bi2
    add edi,640
.bi2
    jmp .returnloop

.mode7hires
    cmp byte[mode7hr+ebx],1
    je near .mode7hiresb
    mov ecx,128
.a7
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a7
    mov ecx,128
    add edi,64*2
    add esi,75036-256
.a7r
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a7r
    sub esi,75036
    jmp .returnloop

.mode7hiresb
    mov ecx,64
.a7hr
    mov eax,[esi]
    mov [es:edi],eax
    add esi,4
    add edi,4
    dec ecx
    jnz .a7hr
    mov ecx,64
    add esi,75036*2-256
.a7hrb
    mov eax,[esi]
    mov [es:edi],eax
    add esi,4
    add edi,4
    dec ecx
    jnz .a7hrb
    sub esi,75036*2
    add edi,64*2
    add esi,75036-256
    mov ecx,64
.a7hrr
    mov eax,[esi]
    mov [es:edi],eax
    add esi,4
    add edi,4
    dec ecx
    jnz .a7hrr
    mov ecx,64
    add esi,75036*2-256
.a7hrbr
    mov eax,[esi]
    mov [es:edi],eax
    add esi,4
    add edi,4
    dec ecx
    jnz .a7hrbr
    sub esi,75036*3
    jmp .returnloop

NEWSYM smallscreen640x480x8b
    mov byte[res640],0
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,32*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*640+32*2
.res239
    add edi,[VidStartDraw]
    add edi,128+120*640
    add esi,16+256+32
    xor eax,eax
    mov ebx,hirestiledat+1
    mov dl,[resolutn]
    dec dl
    dec dl
.loopa
    mov ecx,64
    rep movsd
    add esi,32
    add edi,640-256
    inc ebx
    dec dl
    jnz .loopa
    pop es
    ret

.fullscreen
    cmp byte[curblank],40h
    jne .startcopy2
    ret
.startcopy2
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    xor edi,edi
    cmp word[resolutn],224
    jne .res239b
    mov edi,8*640
.res239b
    add edi,[VidStartDraw]
    add esi,16+256+32
    xor eax,eax
    mov ebx,hirestiledat+1
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[scanlines],1
    je near .scanlines
.loopa3
    mov ecx,128
.loopa2
    mov al,[esi]
    mov [es:edi],al
    mov [es:edi+1],al
    mov [es:edi+640],al
    mov [es:edi+641],al
    mov al,[esi+1]
    mov [es:edi+2],al
    mov [es:edi+3],al
    mov [es:edi+4],al
    mov [es:edi+642],al
    mov [es:edi+643],al
    mov [es:edi+644],al
    add esi,byte 2
    add edi,5
    dec ecx
    jnz .loopa2
    add esi,32
    add edi,640
    inc ebx
    dec dl
    jnz .loopa3
    pop es
    ret

.scanlines
.loopa5
    mov ecx,128
.loopa4
    mov al,[esi]
    mov [es:edi],al
    mov [es:edi+1],al
    mov al,[esi+1]
    mov [es:edi+2],al
    mov [es:edi+3],al
    mov [es:edi+4],al
    add esi,byte 2
    add edi,5
    dec ecx
    jnz .loopa4
    add esi,32
    add edi,640
    inc ebx
    dec dl
    jnz .loopa5
    pop es
    ret

NEWSYM copyvesa2640x480x8bs
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    mov byte[res640],2
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,32*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*640+32*2
.res239
    add edi,[VidStartDraw]
    add esi,16+256+32
    xor eax,eax
    mov ebx,hirestiledat+1
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[CurrentGUIOn],1
    je .loopa
    cmp byte[ForceNewGfxOff],0
    jne .loopa
    cmp byte[newengen],0
    jne near copyvesa2640x480x8bsng
.loopa
    cmp byte[Triplebufen],1
    je .ignorehr
    cmp byte[ebx],1
    je .yeshires
.ignorehr
    mov ecx,256
.a
    mov al,[esi]
    mov [es:edi],al
    inc esi
    mov [es:edi+1],al
    add edi,byte 2
    dec ecx
    jnz .a
.returnloop
    add esi,32
    add edi,64*2+640
    inc ebx
    dec dl
    jnz .loopa
    pop es
    cmp byte[Triplebufen],1
    je .ignorehr2
    xor byte[res512switch],1
.ignorehr2
    ret
.yeshires
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightside
    mov ecx,256
.b
    mov al,[esi]
    inc esi
    mov [es:edi],al
    add edi,byte 2
    dec ecx
    jnz .b
    jmp .returnloop
.rightside
    mov ecx,256
.b2
    mov al,[esi]
    inc esi
    mov [es:edi+1],al
    add edi,byte 2
    dec ecx
    jnz .b2
    jmp .returnloop

copyvesa2640x480x8bsng:
    xor ebx,ebx
.loopa
    cmp byte[BGMA+ebx],5
    je near .hires
    cmp byte[BGMA+ebx],6
    je near .hires
    mov ecx,128
.a
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
.returnloop
    add esi,32
    add edi,64*2+640
    inc ebx
    dec dl
    jnz .loopa
    pop es
    xor byte[res512switch],1
    ret
.hires
    mov ecx,128
.a2
    mov al,[esi+1]
    mov ah,[esi+75037]
    shl eax,16
    mov al,[esi]
    mov ah,[esi+75036]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2
    jmp .returnloop

NEWSYM proceagle
    mov byte[res640],0
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    add esi,16+256+32
    mov dl,237
    mov edi,32*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*640+32*2
    add edi,[VidStartDraw]
    mov dl,222
.res239
    call draweagle
    pop es
    ret

NEWSYM draweagle
    ; copies a buffer from esi to es:edi with dl # of lines
    ; This only works under vesa 2 640x480x8b mode
    mov [lineleft],dl

    ; copy the first line directly
    mov ecx,128
.drawnext
    mov al,[esi]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .drawnext

    dec byte[lineleft]
    add edi,128         ; 512 + 128 = 640
    add esi,32          ; There are 32 extra pixels in the buffer used
                        ; for clipping

    xor eax,eax
    xor ebx,ebx
    mov edx,[spritetablea]
.drawloop
    ; process EAGLE on the bottom line
    ; process the first pixel
    ; copy to the left pixel
    mov al,[esi]
    mov [edx],al
    ; draw the right pixel depending on the pixels right & below
    mov al,[esi+1]
    mov ah,al
    mov bx,[esi+288]
    cmp ebx,eax
    je .matchf
    mov al,[esi]
.matchf
    mov [edx+1],al
    inc esi
    add edx,2

    ; Start drawing the in-between pixels
    mov ecx,256-2
    mov bx,[esi+287]
.lineloopd
    ; draw the left pixel depending on the pixels left & below
    mov al,[esi-1]
    mov ah,al
    cmp ebx,eax
    je .matchlp
    mov al,[esi]
.matchlp
    mov [edx],al

    ; draw the right pixel depending on the pixels right & below
    mov al,[esi+1]
    mov ah,al
    mov bx,[esi+288]
    cmp ebx,eax
    je .matchrp
    mov al,[esi]
.matchrp
    mov [edx+1],al
    ;increment the addresses
    add edx,2
    inc esi
    dec ecx
    jnz .lineloopd

    ; process the last pixel
    ; draw the left pixel depending on the pixels left & below
    mov al,[esi-1]
    mov ah,al
    mov bx,[esi+287]
    cmp ebx,eax
    je .matchl
    mov al,[esi]
.matchl
    mov [edx],al
    ; copy to the right pixel
    mov al,[esi]
    mov [edx+1],al
    inc esi

    sub edx,510
    mov ecx,128
.copyloop
    mov eax,[edx]
    mov [es:edi],eax
    add edx,4
    add edi,4
    dec ecx
    jnz .copyloop
    xor eax,eax

    ; process EAGLE on the upper line
    add edi,128
    add esi,32

    mov edx,[spritetablea]
    ; process the first pixel
    ; copy to the left pixel
    mov al,[esi]
    mov [edx],al
    ; draw the right pixel depending on the pixels right & above
    mov al,[esi+1]
    mov ah,al
    mov bx,[esi-288]
    cmp ebx,eax
    je .matchf2
    mov al,[esi]
.matchf2
    mov [edx+1],al
    inc esi
    add edx,2

    ; Start drawing the in-between pixels
    mov ecx,256-2
    mov bx,[esi-289]

.lineloopd2
    ; draw the left pixel depending on the pixels left & above
    mov al,[esi-1]
    mov ah,al
    cmp ebx,eax
    je .matchlp2
    mov al,[esi]
.matchlp2
    mov [edx],al
    ; draw the right pixel depending on the pixels right & below
    mov al,[esi+1]
    mov ah,al
    mov bx,[esi-288]
    cmp ebx,eax
    je .matchrp2
    mov al,[esi]
.matchrp2
    mov [edx+1],al
    ;increment the addresses
    add edx,2
    inc esi
    dec ecx
    jnz .lineloopd2

    ; process the last pixel
    ; draw the left pixel depending on the pixels left & above
    mov al,[esi-1]
    mov ah,al
    mov bx,[esi-289]
    cmp ebx,eax
    je .matchl2
    mov al,[esi]
.matchl2
    mov [edx],al
    ; copy to the right pixel
    mov al,[esi]
    mov [edx+1],al
    inc esi

    sub edx,510
    mov ecx,128
.copyloop2
    mov eax,[edx]
    mov [es:edi],eax
    add edx,4
    add edi,4
    dec ecx
    jnz .copyloop2
    xor eax,eax

    sub esi,256         ; move esi back to left side of the line
    add edi,128
    dec byte[lineleft]
    jnz near .drawloop

    ; copy the last line directly
    mov ecx,128
.drawlast
    mov al,[esi]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .drawlast
    ret

;*******************************************************
; Copy VESA2 512x384x8b  Copies buffer to 512x384x8bVBE2
;*******************************************************

NEWSYM copyvesa2512x384x8b
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    cmp byte[smallscreenon],1
    je near .smallscreen
    cmp byte[ForceNewGfxOff],0
    jne .nong16b
    cmp byte[newengen],0
    jne near copyvesa2512x384x8ng
.nong16b
    mov dword[ignor512],0
    push es
    mov byte[.lastrep],0
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov byte[.scratio],61       ; 60.6695
    cmp word[resolutn],224
    jne .res239
    mov byte[.scratio],72       ; 72.4286
.res239
    mov edi,[VidStartDraw]
    add esi,16+256+32
    xor eax,eax
    mov ebx,hirestiledat+1
    mov dl,[resolutn]
    dec dl
    dec dl
    xor dh,dh
.loopa
    mov al,[ebx]
    mov [.p512],al
    cmp byte[Triplebufen],1
    je .ignorehr
    cmp al,1
    je near .yeshires
.ignorehr
    mov ecx,128
.a
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
.returnloop
    cmp byte[.lastrep],1
    je .no2
    sub dh,[.scratio]
    jnc .no2
    add dh,100
    sub esi,256
    mov al,[.p512]
    mov [ebx],al
    inc dl
    dec ebx
    mov byte[.lastrep],1
    jmp .yes2
.no2
    mov byte[.lastrep],0
    add esi,32
.yes2
    inc ebx
    dec dl
    jnz .loopa
    pop es
    cmp byte[Triplebufen],1
    je .ignorehr2
    xor byte[res512switch],1
.ignorehr2
    ret
.yeshires
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightside
    mov ecx,256
.b
    mov al,[esi]
    inc esi
    mov [es:edi],al
    add edi,byte 2
    dec ecx
    jnz .b
    jmp .returnloop
.rightside
    mov ecx,256
.b2
    mov al,[esi]
    inc esi
    mov [es:edi+1],al
    add edi,byte 2
    dec ecx
    jnz .b2
    jmp .returnloop

.smallscreen
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,[VidStartDraw]
    add esi,16+256+32
    add edi,72*512+128
    cmp byte[resolutn],224
    jne .ssres239
    add edi,8*512
.ssres239
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[MMXSupport],1
    je .ssloopb
.ssloopa
    mov ecx,64
    rep movsd
    add esi,32
    add edi,128*2
    dec dl
    jnz .ssloopa
    jmp .done
.ssloopb
    mov ecx,16
    MMXStuff
    add esi,32
    add edi,128*2
    dec dl
    jnz .ssloopb
    emms
.done
    pop es
    ret

SECTION .bss
.scratio resb 1
.lastrep resb 1
.p512    resb 1
SECTION .text

NEWSYM copyvesa2512x384x8ng
    push es
    mov byte[.lastrep],0
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov byte[.scratio],61       ; 60.6695
    cmp word[resolutn],224
    jne .res239
    mov byte[.scratio],72       ; 72.4286
.res239
    mov edi,[VidStartDraw]
    add esi,16+256+32
    xor eax,eax
    mov ebx,1
    mov dl,[resolutn]
    dec dl
    dec dl
    xor dh,dh
.loopa
    cmp dword[ignor512],0
    je .a2
    cmp byte[BGMA+ebx],5
    je near .hires
    cmp byte[BGMA+ebx],6
    je near .hires
.a2
    mov ecx,128
.a
    mov al,[esi+1]
    mov ah,al
    shl eax,16
    mov al,[esi]
    mov ah,al
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
.returnloop
    cmp byte[.lastrep],1
    je .no2
    sub dh,[.scratio]
    jnc .no2
    add dh,100
    sub esi,256
    mov al,[.p512]
    mov [ebx],al
    inc dl
    dec ebx
    mov byte[.lastrep],1
    jmp .yes2
.no2
    mov byte[.lastrep],0
    add esi,32
.yes2
    inc ebx
    dec dl
    jnz near .loopa
    pop es
    mov dword[ignor512],0
    ret
.hires
    mov ecx,256
.b
    mov al,[esi]
    mov ah,[esi+75036]
    inc esi
    mov [es:edi],ax
    add edi,byte 2
    dec ecx
    jnz .b
    jmp .returnloop

SECTION .bss
.scratio resb 1
.lastrep resb 1
.p512    resb 1
SECTION .text

;*******************************************************
; Copy VESA2 320x240x16b Copies buffer to 320x240x16bVB2
;*******************************************************


;copyvesa2320x240x16bgui:
;    cmp byte[curblank],40h
;    jne .startcopy
;    ret
;.startcopy
;    cmp byte[ScreenScale],1
;    je near .scalescreen
;    push es
;    mov ax,[vesa2selec]
;    mov es,ax
;    mov esi,[vidbuffer]
;    mov edi,32*2           ; Draw @ Y from 9 to 247
;    cmp word[resolutn],224
;    jne .res239
;    mov edi,8*320*2+32*2
;.res239
;    add edi,[VidStartDraw]
;    add esi,16+256+32
;    xor eax,eax
;    mov dl,[resolutn]
;    sub dl,2
;.loopa
;    mov ecx,256
;    xor eax,eax
;.loopa2
;    mov al,[esi]
;    inc esi
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    add edi,byte 2
;    dec ecx
;    jnz .loopa2
;    add esi,32
;    add edi,128
;    dec dl
;    jnz .loopa
;    pop es
;    ret
;.scalescreen
;    push es
;    mov ax,[vesa2selec]
;    mov es,ax
;    mov esi,[vidbuffer]
;    xor edi,edi
;    cmp word[resolutn],224
;    jne .res239b
;    mov edi,8*320*2
;.res239b
;    add edi,[VidStartDraw]
;    add esi,16+256+32
;    xor eax,eax
;    mov dl,[resolutn]
;    sub dl,2
;.loopab
;    mov ecx,64
;    xor eax,eax
;.loopbb
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    mov al,[esi+1]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi+4],bx
;    mov al,[esi+2]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi+6],bx
;    mov al,[esi+3]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi+8],bx
;    add esi,4
;    add edi,10
;    dec ecx
;    jnz .loopbb
;    add esi,32
;    dec dl
;    jnz .loopab
;    pop es
;    ret

NEWSYM copyvesa2320x240x16b
;    cmp byte[vesa2red10],1
;    jne .notbr
;    call ConvertToAFormat
;.notbr
;    je near copyvesa2320x240x16br
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,320*2+32*2
    cmp word[resolutn],224
    jne .res239
    add edi,8*320*2
.res239
    add edi,[VidStartDraw]
    add esi,16*2+256*2+16*2+16*2
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[ScreenScale],1
    je near .fullscreen

    FilterTest .interpolate

    cmp byte[MMXSupport],1
    je near .loopb
.loopa
    mov ecx,256/4*2
    rep movsd
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz .loopa
    jmp .done
.loopb
    mov ecx,256/16*2
    MMXStuff
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz .loopb
    emms
    jmp .done

.fullscreen
    sub edi,32*2
.fsloopa
    mov ecx,256/4
.fsloopb
    mov eax,[esi]
    mov [es:edi],ax
    mov [es:edi+2],eax
    add esi,byte 4
    add edi,byte 6
    movsd
    dec ecx
    jnz .fsloopb
    add esi,16*2+16*2
    dec dl
    jnz .fsloopa
    jmp .done

.interpolate
    SSInterpLineH
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz .interpolate

.done
    pop es
    ret

%endif

SECTION .bss
NEWSYM rescompareng, resd 1
NEWSYM nextdrawallng, resd 1
NEWSYM prevcol0ng, resd 1
NEWSYM numbytelng, resd 1
NEWSYM lineleft, resd 1
NEWSYM lineleft2, resd 1
bankpos resd 1
SECTION .text

%ifdef __MSDOS__
;*******************************************************
; Copy VESA2 320x480x16b Copies buffer to 320x480x16bVB2
;*******************************************************

;NEWSYM copyvesa2320x480x16bgui
;    cmp byte[curblank],40h
;    jne .startcopy
;    ret
;.startcopy
;    push es
;    mov ax,[vesa2selec]
;    mov es,ax
;    mov esi,[vidbuffer]
;    mov edi,32*2           ; Draw @ Y from 9 to 247
;    cmp word[resolutn],224
;    jne .res239
;    mov edi,8*320*2+32*2
;.res239
;    add edi,[VidStartDraw]
;    add esi,16+256+32
;    xor eax,eax
;    mov dl,[resolutn]
;    sub dl,2
;    cmp byte[scanlines],3
;    je near .halfscanlines
;    cmp byte[scanlines],2
;    je near .quarterscanlines
;    cmp byte[scanlines],1
;    je near .scanlines
;.loopa
;    mov ecx,256
;    xor eax,eax
;.a
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    inc esi
;    add edi,byte 2
;    dec ecx
;    jnz .a
;    sub esi,256
;    add edi,64*2
;    mov ecx,256
;    xor eax,eax
;.a2
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    inc esi
;    add edi,byte 2
;    dec ecx
;    jnz .a2
;    add esi,32
;    add edi,64*2
;    dec dl
;    jnz .loopa
;    pop es
;    ret
;
;.scanlines
;.loopab
;    mov ecx,256
;    xor eax,eax
;.ab
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    inc esi
;    add edi,byte 2
;    dec ecx
;    jnz .ab
;.returnb
;    add esi,32
;    add edi,64*2+320*2
;    dec dl
;    jnz .loopab
;    pop es
;    ret
;
;.halfscanlines
;.loopabh
;    mov ecx,256
;    xor eax,eax
;.abh
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    inc esi
;    add edi,byte 2
;    dec ecx
;    jnz .abh
;    add edi,64*2
;    mov ecx,128
;    sub esi,256
;.abh2
;    mov al,[esi+1]
;    mov bx,[GUICPC+eax*2]
;    mov al,[esi]
;    shl ebx,16
;    mov bx,[GUICPC+eax*2]
;    and ebx,[vesa2_clbitng2]
;    shr ebx,1
;    mov [es:edi],ebx
;    add esi,byte 2
;    add edi,4
;    dec ecx
;    jnz .abh2
;    add esi,32
;    add edi,64*2
;    dec dl
;    jnz .loopabh
;    pop es
;    ret
;
;.quarterscanlines
;.loopabhq
;    mov ecx,256
;    xor eax,eax
;.abhq
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    inc esi
;    add edi,byte 2
;    dec ecx
;    jnz .abhq
;    add edi,64*2
;    mov ecx,128
;    sub esi,256
;.abh2q
;    mov al,[esi+1]
;    mov bx,[GUICPC+eax*2]
;    mov al,[esi]
;    shl ebx,16
;    mov bx,[GUICPC+eax*2]
;    and ebx,[vesa2_clbitng2]
;    shr ebx,1
;    mov eax,ebx
;    and eax,[vesa2_clbitng2]
;    shr eax,1
;    add ebx,eax
;    mov [es:edi],ebx
;    xor eax,eax
;    add esi,byte 2
;    add edi,4
;    dec ecx
;    jnz .abh2q
;    add esi,32
;    add edi,64*2
;    dec dl
;    jnz .loopabhq
;    pop es
;    ret
%endif

NEWSYM ConvertToAFormat
    cmp byte[GUIOn],1
    je .nonewgfx
    cmp byte[newengen],0
    je .nonewgfx
    ret
.nonewgfx
    mov eax,[vesa2_clbitng2]
    mov ebx,eax
    and eax,11111111110000001111111111000000b
    and ebx,00000000000111110000000000011111b
    or eax,ebx
    mov [vesavaland],eax
    mov [vesavaland+4],eax
    mov esi,[vidbuffer]
    add esi,16*2+256*2+32*2
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[MMXSupport],1
    je near .mmxconv
.crgbloop
    mov ecx,128
.crgbloop2
    mov eax,[esi]
    mov ebx,eax
    and eax,11111111110000001111111111000000b
    and ebx,00000000000111110000000000011111b
    shr eax,1
    or ebx,eax
    mov [esi],ebx
    add esi,4
    dec ecx
    jnz .crgbloop2
    add esi,64
    dec dl
    jnz .crgbloop
    jmp .nocopyvesa2r
.mmxconv
.crgbloopm
    mov ecx,64
    movq mm6,[mmxvalanda]
    movq mm7,[mmxvalandb]
.crgbloop2m
    movq mm0,[esi]
    movq mm1,mm0
    pand mm0,mm6
    pand mm1,mm7
    psrlw mm0,1
    por mm0,mm1
    movq [esi],mm0
    add esi,8
    dec ecx
    jnz .crgbloop2m
    add esi,64
    dec dl
    jnz .crgbloopm
    emms
.nocopyvesa2r
    ret

NEWSYM UnConvertToAFormat
    mov esi,[vidbuffer]
    add esi,16*2+256*2+32*2
    mov dl,[resolutn]
    dec dl
    dec dl
.crgbloop
    mov ecx,128
.crgbloop2
    mov eax,[esi]
    mov ebx,eax
    and eax,01111111111000000111111111100000b
    and ebx,00000000000111110000000000011111b
    shl eax,1
    or ebx,eax
    mov [esi],ebx
    add esi,4
    dec ecx
    jnz .crgbloop2
    add esi,64
    dec dl
    jnz .crgbloop
    ret

%ifdef __MSDOS__
NEWSYM copyvesa2320x480x16b
    cmp byte[curblank],40h
    jne .startcopy
    ret

.startcopy
;    mov eax,[vesa2_clbitng2]
;    mov [vesavaland],eax
;    mov [vesavaland+4],eax
;    cmp byte[vesa2red10],1
;    jne .nocopyvesa2r
;    call ConvertToAFormat
;.nocopyvesa2r
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,320*2*2+32*2
    cmp word[resolutn],224
    jne .res239
    add edi,8*320*2
.res239
    add edi,[VidStartDraw]
    add esi,16*2+256*2+16*2+16*2
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[ScreenScale],1
    je near .fullscreen

    FilterTest .interpolate

    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],3
    je near .halfscanlines
    cmp byte[scanlines],2
    je near .quartscanlines

    cmp byte[MMXSupport],1
    je .loopb
.loopa
    mov ecx,256/4*2
    rep movsd
    sub esi,256*2
    add edi,32*2+32*2
    mov ecx,256/4*2
    rep movsd
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz .loopa
    jmp .done
.loopb
    mov ecx,256/16*2
    MMXStuff
    sub esi,256*2
    add edi,32*2+32*2
    mov ecx,256/16*2
    MMXStuff
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz .loopb
    emms
    jmp .done

.scanlines
    cmp byte[MMXSupport],1
    je .sloopb
.sloopa
    mov ecx,256/4*2
    rep movsd
    add esi,16*2+16*2
    add edi,32*2+320*2+32*2
    dec dl
    jnz .sloopa
    jmp .done
.sloopb
    mov ecx,256/16*2
    MMXStuff
    add esi,16*2+16*2
    add edi,32*2+320*2+32*2
    dec dl
    jnz .sloopb
    emms
    jmp .done

.halfscanlines
    ;cmp byte[MMXSupport]
    ;je near .hsloopb
.hsloopa
    mov ecx,256/4*2
    rep movsd
    sub esi,256*2
    add edi,32*2+32*2
    mov ecx,256/2
.hsloopa2
    mov ax,[esi]
    mov bx,[esi+2]
    shr ax,byte 1
    shr bx,byte 1
    and ax,7befh
    and bx,7befh
    mov [es:edi],ax
    mov [es:edi+2],bx
    add esi,byte 4
    add edi,byte 4
    dec ecx
    jnz .hsloopa2
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz .hsloopa
    jmp .done

.quartscanlines
    ;cmp byte[MMXSupport],1
    ;je .qsloopb
.qsloopa
    mov ecx,256/4*2
    rep movsd
    sub esi,256*2
    add edi,32*2+32*2
    mov ecx,256/2
.qsloopa2
    push ecx
    push edx
    mov ax,[esi]
    mov bx,[esi+2]
    mov cx,ax
    mov dx,bx
    shr cx,byte 2
    shr dx,byte 2
    and cx,39e7h
    and dx,39e7h
    sub ax,cx
    sub bx,dx
    mov [es:edi],ax
    mov [es:edi+2],bx
    pop edx
    pop ecx
    add esi,byte 4
    add edi,byte 4
    dec ecx
    jnz .qsloopa2
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz .qsloopa
    jmp .done

.fullscreen
    sub edi,32*2
    cmp byte[scanlines],1
    je .fsloopa2
    cmp byte[scanlines],3
    je .fsloopa3
    cmp byte[scanlines],2
    je .fsloopa4
.fsloopa
    mov ecx,256/4
    call .fsloopb
    sub esi,256*2
    mov ecx,256/4
    call .fsloopb
    add esi,16*2+16*2
    dec dl
    jnz .fsloopa
    jmp .done
.fsloopa2
    mov ecx,256/4
    call .fsloopb
    add esi,16*2+16*2
    add edi,320*2
    dec dl
    jnz .fsloopa2
    jmp .done
.fsloopa3
    mov ecx,256/4
    call .fsloopb
    sub esi,256*2
    mov ecx,256/4
    call .fsloopb2
    add esi,16*2+16*2
    dec dl
    jnz .fsloopa3
    jmp .done
.fsloopa4
    mov ecx,256/4
    call .fsloopb
    sub esi,256*2
    mov ecx,256/4
    call .fsloopb3
    add esi,16*2+16*2
    dec dl
    jnz .fsloopa4
    jmp .done
.fsloopb
    mov eax,[esi]
    mov [es:edi],ax
    mov [es:edi+2],eax
    add esi,byte 4
    add edi,byte 6
    movsd
    dec ecx
    jnz .fsloopb
    ret
.fsloopb2
    mov ax,[esi]
    mov bx,[esi+2]
    shr ax,byte 1
    shr bx,byte 1
    and ax,7befh
    and bx,7befh
    mov [es:edi],ax
    mov [es:edi+2],ax
    mov [es:edi+4],bx
    add esi,byte 4
    add edi,byte 6
    mov ax,[esi]
    mov bx,[esi+2]
    shr ax,byte 1
    shr bx,byte 1
    and ax,7befh
    and bx,7befh
    mov [es:edi],ax
    mov [es:edi+2],bx
    add esi,byte 4
    add edi,byte 4
    dec ecx
    jnz .fsloopb2
    ret
.fsloopb3
    push ecx
    push edx
    mov ax,[esi]
    mov bx,[esi+2]
    mov cx,ax
    mov dx,bx
    shr cx,byte 2
    shr dx,byte 2
    and cx,39e7h
    and dx,39e7h
    sub ax,cx
    sub bx,dx
    mov [es:edi],ax
    mov [es:edi+2],ax
    mov [es:edi+4],bx
    add esi,byte 4
    add edi,byte 6
    mov ax,[esi]
    mov bx,[esi+2]
    mov cx,ax
    mov dx,bx
    shr cx,byte 2
    shr dx,byte 2
    and cx,39e7h
    and dx,39e7h
    sub ax,cx
    sub bx,dx
    mov [es:edi],ax
    mov [es:edi+2],bx
    add esi,byte 4
    add edi,byte 4
    pop edx
    pop ecx
    dec ecx
    jnz .fsloopb3
    ret

.interpolate
    cmp byte[scanlines],1
    je near .inloopa2
.inloopa
    SSInterpLineH
    sub esi,256*2
    add edi,32*2+32*2
    SSInterpLineH
    add esi,16*2+16*2
    add edi,32*2+32*2
    dec dl
    jnz near .inloopa
    jmp .done
.inloopa2
    SSInterpLineH
    add esi,16*2+16*2
    add edi,32*2+320*2+32*2
    dec dl
    jnz .inloopa2

.done
    pop es
    ret

;*******************************************************
; Copy VESA2 640x480x16b Copies buffer to 640x480x16bVB2
;*******************************************************

%macro copya640x480 0
    mov [es:edi],dx
    mov [es:edi+2],dx
    mov [es:edi+640*2],dx
    mov [es:edi+640*2+2],dx
%endmacro

%macro copyb640x480 0
    mov [es:edi+4],dx
    mov [es:edi+6],dx
    mov [es:edi+640*2+4],dx
    mov [es:edi+640*2+6],dx
%endmacro

%macro copya640x480sl 0
    mov [es:edi],dx
    mov [es:edi+2],dx
%endmacro

%macro copyb640x480sl 0
    mov [es:edi+4],dx
    mov [es:edi+6],dx
%endmacro

%macro precheckvesa12 1
    cmp edx,%1
    ja %%a
    mov ecx,edx
%%a
%endmacro

%macro postcheckvesa12 4
    cmp edx,%3
    ja %%a
    call VESA12Bankswitch
    mov ecx,%3
    sub ecx,edx
    add edx,%4
    or ecx,ecx
    jz %%a
    jmp %1
%%a
    sub edx,%3
    sub edx,%2
    jg %%nobankswitch
    add edx,%4
    call VESA12Bankswitch
%%nobankswitch
%endmacro

;NEWSYM copyvesa12640x480x16bgui
;    cmp byte[curblank],40h
;    jne .startcopy
;    ret
;.startcopy
;    push es
;    mov word[bankpos],0
;    call VESA12Bankswitch
;    mov ax,[selcA000]
;    mov es,ax
;    mov esi,[vidbuffer]
;    mov edi,32*2*2           ; Draw @ Y from 9 to 247
;    cmp word[resolutn],224
;    jne .res239
;    mov edi,8*320*2*2+32*2*2
;.res239
;    add esi,16+256+32
;    xor eax,eax
;    mov dl,[resolutn]
;    sub dl,2
;    mov [lineleft],dl
;    mov edx,65536
;    sub edx,edi
;    shr edx,2
;    cmp byte[smallscreenon],1
;    je near .smallscreen
;    cmp byte[scanlines],1
;    je near .scanlines
;.loopa
;    mov ecx,256
;    xor eax,eax
;    precheckvesa12 256
;.a
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .a
;    postcheckvesa12 .a,64,256,16384
;    sub esi,256
;    add edi,128*2
;    mov ecx,256
;    xor eax,eax
;    precheckvesa12 256
;.a2
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .a2
;    postcheckvesa12 .a2,64,256,16384
;    add esi,32
;    add edi,128*2
;    dec byte[lineleft]
;    jnz near .loopa
;    pop es
;    ret
;
;.scanlines
;.loopa2
;    mov ecx,256
;    xor eax,eax
;    precheckvesa12 256
;.ab
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .ab
;    postcheckvesa12 .ab,64+320,256,16384
;    add esi,32
;    add edi,128*2+640*2
;    dec byte[lineleft]
;    jnz near .loopa2
;    pop es
;    ret
;
;.smallscreen
;    add edx,edx
;.loopa3
;    mov ecx,256
;    xor eax,eax
;    precheckvesa12 256
;.ac
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    inc esi
;    add edi,byte 2
;    dec ecx
;    jnz .ac
;    postcheckvesa12 .ac,128+256,256,32768
;    add esi,32
;    add edi,128*2+256*2
;    dec byte[lineleft]
;    jnz near .loopa3
;    pop es
;    ret


VESA12Bankswitch:
    pushad
    mov ax,4F05h
    mov bx,0
    mov dx,[bankpos]
    int 10h
    mov ax,[granadd]
    add word[bankpos],ax
    popad
    sub edi,65536
    ret

;NEWSYM copyvesa2640x480x16bgui
;    je near smallscreen640x480x16bgui.fullscreen
;    cmp byte[curblank],40h
;    jne .startcopy
;    ret
;.startcopy
;    push es
;    mov ax,[vesa2selec]
;    mov es,ax
;    mov esi,[vidbuffer]
;    mov edi,32*2*2           ; Draw @ Y from 9 to 247
;    cmp word[resolutn],224
;    jne .res239
;    mov edi,8*320*2*2+32*2*2
;.res239
;    add edi,[VidStartDraw]
;    add esi,16+256+32
;    xor eax,eax
;    mov dl,[resolutn]
;    sub dl,2
;    cmp byte[scanlines],3
;    je near .halfscanlines
;    cmp byte[scanlines],2
;    je near .quarterscanlines
;    cmp byte[scanlines],1
;    je near .scanlines
;.loopa
;    mov ecx,256
;    xor eax,eax
;.a
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .a
;    sub esi,256
;    add edi,128*2
;    mov ecx,256
;    xor eax,eax
;.a2
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .a2
;    add esi,32
;    add edi,128*2
;    dec dl
;    jnz .loopa
;    pop es
;    ret
;
;.scanlines
;.loopab
;    mov ecx,256
;    xor eax,eax
;.ab
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .ab
;.returnb
;    add esi,32
;    add edi,128*2+640*2
;    dec dl
;    jnz .loopab
;    pop es
;    ret
;
;.halfscanlines
;.loopabh
;    mov ecx,256
;    xor eax,eax
;.abh
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .abh
;    add edi,128*2
;    mov ecx,256
;    sub esi,256
;.abh2
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    shl ebx,16
;    mov bx,[GUICPC+eax*2]
;    and ebx,[vesa2_clbitng2]
;    shr ebx,1
;    mov [es:edi],ebx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .abh2
;    add esi,32
;    add edi,128*2
;    dec dl
;    jnz .loopabh
;    pop es
;    ret
;
;.quarterscanlines
;.loopabhq
;    mov ecx,256
;    xor eax,eax
;.abhq
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .abhq
;    add edi,128*2
;    mov ecx,256
;    sub esi,256
;.abh2q
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    shl ebx,16
;    mov bx,[GUICPC+eax*2]
;    and ebx,[vesa2_clbitng2]
;    shr ebx,1
;    mov eax,ebx
;    and eax,[vesa2_clbitng2]
;    shr eax,1
;    add ebx,eax
;    mov [es:edi],ebx
;    xor eax,eax
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .abh2q
;    add esi,32
;    add edi,128*2
;    dec dl
;    jnz .loopabhq
;    pop es
;    ret
;
;NEWSYM smallscreen640x480x16bgui
;    cmp byte[curblank],40h
;    jne .startcopy
;    ret
;.startcopy
;    push es
;    mov ax,[vesa2selec]
;    mov es,ax
;    mov esi,[vidbuffer]
;    mov edi,32*2*2           ; Draw @ Y from 9 to 247
;    cmp word[resolutn],224
;    jne .res239
;    mov edi,8*640*2+32*2*2
;.res239
;    add edi,[VidStartDraw]
;    add edi,128*2+120*640*2
;    add esi,16+256+32
;    xor eax,eax
;    mov ebx,hirestiledat+1
;    mov dl,[resolutn]
;    sub dl,2
;.loopa
;    mov ecx,256
;    xor eax,eax
;.a3
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    inc esi
;    add edi,byte 2
;    dec ecx
;    jnz .a3
;    add esi,32
;    add edi,640*2-256*2
;    inc ebx
;    dec dl
;    jnz .loopa
;    pop es
;    ret
;
;.fullscreen
;    cmp byte[curblank],40h
;    jne .startcopy2
;    ret
;.startcopy2
;    push es
;    mov ax,[vesa2selec]
;    mov es,ax
;    mov esi,[vidbuffer]
;    xor edi,edi
;    cmp word[resolutn],224
;    jne .res239b
;    mov edi,8*320*2*2
;.res239b
;    add edi,[VidStartDraw]
;    add esi,16+256+32
;    xor eax,eax
;    mov dl,[resolutn]
;    sub dl,2
;    cmp byte[scanlines],1
;    je near .scanlines
;.loopa2
;    mov ecx,128
;    xor eax,eax
;.a
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    mov al,[esi+1]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi+4],bx
;    mov [es:edi+6],bx
;    mov [es:edi+8],bx
;    add esi,byte 2
;    add edi,10
;    dec ecx
;    jnz .a
;    sub esi,256
;    mov ecx,128
;.a2
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    mov al,[esi+1]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi+4],bx
;    mov [es:edi+6],bx
;    mov [es:edi+8],bx
;    add esi,byte 2
;    add edi,10
;    dec ecx
;    jnz .a2
;    add esi,32
;    dec dl
;    jnz near .loopa2
;    pop es
;    ret
;
;.scanlines
;.loopab
;    mov ecx,128
;    xor eax,eax
;.ab
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    mov al,[esi+1]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi+4],bx
;    mov [es:edi+6],bx
;    mov [es:edi+8],bx
;    add esi,byte 2
;    add edi,10
;    dec ecx
;    jnz .ab
;    add esi,32
;    add edi,640*2
;    dec dl
;    jnz .loopab
;    pop es
;    ret

NEWSYM copyvesa2640x480x16b
;    cmp byte[vesa2red10],1
;    je near copyvesa2640x480x16br
    cmp byte[vesa2red10],1
    jne .notbr
    call ConvertToAFormat
;    call UnConvertToAFormat
.notbr
    cmp byte[smallscreenon],1
    je near smallscreen640x480x16b
    cmp byte[ScreenScale],1
    je near smallscreen640x480x16b.fullscreen
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,32*2*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*320*2*2+32*2*2
.res239
    add edi,[VidStartDraw]
    add esi,16*2+256*2+32*2
    xor eax,eax
    ; Check if interpolation mode
    cmp byte[FilteredGUI],0
    jne .yi
    cmp byte[GUIOn],1
    je .nointerp
.yi
    cmp byte[MMXSupport],1
    jne .nommx
    cmp byte[newgfx16b],0
    je .nommx
    cmp byte[En2xSaI],0
    jne near Process2xSaI
.nommx
    cmp byte[antienab],1
    je near interpolate640x480x16b
.nointerp
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],3
    je near .halfscanlines
    cmp byte[scanlines],2
    je near .quartscanlines
    mov ebx,hirestiledat+1
    cmp byte[newengen],0
    je .loopa
    mov ebx,SpecialLine+1
.loopa
    mov ecx,256
    cmp byte[Triplebufen],1
    je .ignorehr
    cmp byte[ebx],1
    je near .yeshires
    cmp byte[GUIOn],1
    je .ignorehr
    cmp byte[ebx],1
    ja near .yeshiresng
.ignorehr
    cmp byte[MMXSupport],1
    je near .mmx
.a
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
    sub esi,256*2
    add edi,128*2
    mov ecx,256
.a2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2
.return
    add esi,64
    add edi,128*2
    inc ebx
    dec dl
    jnz near .loopa
    pop es
    cmp byte[Triplebufen],1
    je .ignorehr2
    xor byte[res512switch],1
.ignorehr2
    cmp byte[MMXSupport],1
    je .mmx2
    ret
.mmx2
    emms
    ret
.yeshires
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightside
.b
    mov ax,[esi]
    mov [es:edi],ax
    mov [es:edi+1280],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .b
    add edi,640*2
    jmp .return
.rightside
.c
    mov ax,[esi]
    mov [es:edi+2],ax
    mov [es:edi+1282],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .c
    add edi,640*2
    jmp .return
.mmx
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.mmxr
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    movq [es:edi],mm0
    punpckhwd mm1,mm1
    movq [es:edi+8],mm1
    movq [eax],mm0
    movq [eax+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxr
.nextmmx
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
    add edi,128*2
.mmxr2
    movq mm0,[eax]
    movq [es:edi],mm0
    movq mm1,[eax+8]
    movq [es:edi+8],mm1
    movq mm2,[eax+16]
    movq [es:edi+16],mm2
    movq mm3,[eax+24]
    movq [es:edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2
    jmp .return
.yeshiresng
    call HighResProc
    jmp .return

.scanlines
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopab
    cmp byte[newengen],0
    je .loopab
    mov ebx,SpecialLine+1
.loopab
    mov ecx,256
    cmp byte[Triplebufen],1
    je .ignorehrb
    cmp byte[ebx],1
    je .yeshiresb
    cmp byte[ebx],1
    jbe .ignorehrb
    call HighResProc
    jmp .returnb
.ignorehrb
    cmp byte[MMXSupport],1
    je near .mmxsl
.ab
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .ab
.returnb
    add esi,64
    add edi,128*2+640*2
    inc ebx
    dec dl
    jnz .loopab
    pop es
    cmp byte[Triplebufen],1
    je .ignorehr2b
    xor byte[res512switch],1
.ignorehr2b
    cmp byte[MMXSupport],1
    je near .mmx2
    ret
.yeshiresb
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightsideb
.bb
    mov ax,[esi]
    mov [es:edi],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .bb
    jmp .returnb
.rightsideb
.cb
    mov ax,[esi]
    mov [es:edi+2],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .cb
    jmp .returnb
.mmxsl
    mov ecx,64
.mmxrsl
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    punpckhwd mm1,mm1
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrsl
    jmp .returnb

.halfscanlines
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabh
    cmp byte[newengen],0
    je .loopabh
    mov ebx,SpecialLine+1
.loopabh
    cmp byte[ebx],1
    jbe .ignorehrbh
    call HighResProc
    jmp .returnbh
.ignorehrbh
    cmp byte[MMXSupport],1
    je near .mmxslh
    mov ecx,256
.abh
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .abh
    mov ecx,256
    sub esi,512
    add edi,128*2
.abhs
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    and eax,[vesa2_clbitng2]
    shr eax,1
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .abhs
.returnbh
    add esi,64
    add edi,128*2
    inc ebx
    dec dl
    jnz near .loopabh
    pop es
    cmp byte[MMXSupport],1
    je near .mmx2
    ret
.mmxslh
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.mmxrslh
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    punpckhwd mm1,mm1
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    movq [eax],mm0
    movq [eax+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrslh
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
    add edi,128*2
    movq mm4,[vesa2_clbitng2]
.mmxr2h
    movq mm0,[eax]
    movq mm1,[eax+8]
    movq mm2,[eax+16]
    movq mm3,[eax+24]
    pand mm0,mm4
    pand mm1,mm4
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm0,1
    psrlw mm1,1
    psrlw mm2,1
    psrlw mm3,1
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    movq [es:edi+16],mm2
    movq [es:edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2h
    jmp .returnbh

.quartscanlines
    mov [lineleft],dl
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabh2
    cmp byte[newengen],0
    je .loopabh2
    mov ebx,SpecialLine+1
.loopabh2
    cmp byte[ebx],1
    jbe .ignorehrbh2
    call HighResProc
    jmp .returnbh2
.ignorehrbh2
    cmp byte[MMXSupport],1
    je near .mmxslh2
    mov ecx,256
.abh2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .abh2
    mov ecx,256
    sub esi,512
    add edi,128*2
.abhs2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    and eax,[vesa2_clbitng2]
    shr eax,1
    mov edx,eax
    and edx,[vesa2_clbitng2]
    shr edx,1
    add eax,edx
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .abhs2
.returnbh2
    add esi,64
    add edi,128*2
    inc ebx
    dec byte[lineleft]
    jnz near .loopabh2
    pop es
    cmp byte[MMXSupport],1
    je near .mmx2
    ret
.mmxslh2
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.mmxrslh2
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    punpckhwd mm1,mm1
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    movq [eax],mm0
    movq [eax+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrslh2
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
    add edi,128*2
    movq mm4,[vesa2_clbitng2]
.mmxr2h2
    movq mm0,[eax]
    movq mm1,[eax+8]
    pand mm0,mm4
    pand mm1,mm4
    psrlw mm0,1
    psrlw mm1,1
    movq mm2,mm0
    movq mm3,mm1
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm2,1
    psrlw mm3,1
    paddd mm0,mm2
    paddd mm1,mm3
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    add eax,16
    add edi,16
    dec ecx
    jnz .mmxr2h2
    jmp .returnbh2

HighResProc:
    mov ecx,256
    cmp byte[ebx],3
    je near .hiresmode7
    cmp byte[ebx],7
    je near .hiresmode7
    test byte[ebx],4
    jz .nofield
    cmp byte[scanlines],0
    jne .nofield
    test byte[cfield],1
    jz .nofield
    add edi,640*2
.nofield
    test byte[ebx],3
    jnz near .hires
.a
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
    cmp byte[scanlines],0
    jne .nofield
    test byte[cfield],1
    jnz .nofielde
    add edi,640*2
.nofielde
    ret
.hiresmode7
    cmp byte[MMXSupport],1
    je .yeshiresngmmxmode7
.a2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2
    add edi,128*2
    sub esi,512
    mov ecx,256
    add esi,75036*4
.a2b
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2b
    sub esi,75036*4
    ret
.yeshiresngmmxmode7
    mov ecx,64
.mmxr
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    movq [es:edi],mm0
    punpckhwd mm1,mm1
    movq [es:edi+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxr
    add edi,128*2
    sub esi,512
    add esi,75036*4
    mov ecx,64
.mmxrb
    movq mm0,[esi]
    movq mm1,mm0
    punpcklwd mm0,mm1
    movq [es:edi],mm0
    punpckhwd mm1,mm1
    movq [es:edi+8],mm1
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .mmxrb
    sub esi,75036*4
    ret
.hires
    cmp byte[MMXSupport],1
    je near .yeshiresngmmx
.bng
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .bng
    test byte[ebx],4
    jz .nofieldb
    cmp byte[scanlines],0
    jne .nofieldb
    test byte[cfield],1
    jnz .lowerfield
    add edi,640*2
.lowerfield
    ret
.nofieldb
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],3
    je near .halfscanlines
    cmp byte[scanlines],2
    je near .quartscanlines
    add edi,128*2
    sub esi,256*2
    mov ecx,256
.bngb
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .bngb
    ret
.scanlines
    ret
.yeshiresngmmx
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
.ngal
    movq mm0,[esi]
    movq mm1,[esi+75036*4]
    movq mm2,mm0
    punpcklwd mm0,mm1
    movq [es:edi],mm0
    punpckhwd mm2,mm1
    movq [es:edi+8],mm2
    movq [eax],mm0
    movq [eax+8],mm2
    add esi,8
    add edi,16
    add eax,16
    dec ecx
    jnz .ngal
    test byte[ebx],4
    jz .nofieldc
    cmp byte[scanlines],0
    jne .nofieldc
    test byte[cfield],1
    jnz .lowerfieldb
    add edi,640*2
.lowerfieldb
    ret
.nofieldc
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],3
    je near .halfscanlinesmmx
    cmp byte[scanlines],2
    je near .quartscanlinesmmx
    test byte[ebx+1],3
    jz .noaa
    cmp byte[En2xSaI],0
    jne near .antialias
    cmp byte[antienab],0
    jne near .antialias
.noaa
    add edi,128*2
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
.mmxr2
    movq mm0,[eax]
    movq [es:edi],mm0
    movq mm1,[eax+8]
    movq [es:edi+8],mm1
    movq mm2,[eax+16]
    movq [es:edi+16],mm2
    movq mm3,[eax+24]
    movq [es:edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2
    ret
.antialias
    add edi,128*2
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
    movq mm4,[vesa2_clbitng2]
    sub esi,256*2
.mmxr2aa
    movq mm0,[esi+288*2]
    movq mm1,[esi+288*2+75036*4]
    movq mm2,mm0
    punpcklwd mm0,mm1
    punpckhwd mm2,mm1
    movq mm1,[eax]
    movq mm3,[eax+8]
    pand mm0,mm4
    pand mm1,mm4
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm0,1
    psrlw mm1,1
    psrlw mm2,1
    psrlw mm3,1
    paddd mm0,mm1
    paddd mm2,mm3
    movq [es:edi],mm0
    movq [es:edi+8],mm2
    add eax,16
    add edi,16
    add esi,8
    dec ecx
    jnz .mmxr2aa
    ret
.halfscanlines
    add edi,128*2
    sub esi,256*2
    mov ecx,256
.abhs
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    and eax,[vesa2_clbitng2]
    shr eax,1
    mov edx,eax
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .abhs
    ret
.quartscanlines
    add edi,128*2
    sub esi,256*2
    mov ecx,256
.abhs2
    mov eax,[esi+75036*4-2]
    mov ax,[esi]
    and eax,[vesa2_clbitng2]
    shr eax,1
    mov edx,eax
    and edx,[vesa2_clbitng2]
    shr edx,1
    add eax,edx
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .abhs2
    ret
.halfscanlinesmmx
    mov eax,[spritetablea]
    mov ecx,32
    add eax,512
    add edi,128*2
    movq mm4,[vesa2_clbitng2]
.mmxr2h
    movq mm0,[eax]
    movq mm1,[eax+8]
    movq mm2,[eax+16]
    movq mm3,[eax+24]
    pand mm0,mm4
    pand mm1,mm4
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm0,1
    psrlw mm1,1
    psrlw mm2,1
    psrlw mm3,1
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    movq [es:edi+16],mm2
    movq [es:edi+24],mm3
    add eax,32
    add edi,32
    dec ecx
    jnz .mmxr2h
    ret
.quartscanlinesmmx
    mov eax,[spritetablea]
    mov ecx,64
    add eax,512
    add edi,128*2
    movq mm4,[HalfTransC]
.mmxr2h2
    movq mm0,[eax]
    movq mm1,[eax+8]
    pand mm0,mm4
    pand mm1,mm4
    psrlw mm0,1
    psrlw mm1,1
    movq mm2,mm0
    movq mm3,mm1
    pand mm2,mm4
    pand mm3,mm4
    psrlw mm2,1
    psrlw mm3,1
    paddd mm0,mm2
    paddd mm1,mm3
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    add eax,16
    add edi,16
    dec ecx
    jnz .mmxr2h2
    ret

Process2xSaI:
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabi
    cmp byte[newengen],0
    je .loopabi
    mov ebx,SpecialLine+1
.loopabi
    mov [InterPtr],ebx

;    add edi,[VESAAddr]
    mov dl,[resolutn]
    dec dl
    dec dl    ; Compensate for top/bottom line + 2 lines in 2xSaI
    mov [lineleft],dl
    mov dword[esi+512],0
    mov dword[esi+512+576*2],0
    mov ebx,[vidbufferofsb]
    add ebx,288*2

.next
    mov dword[esi+512+576*3],0

    mov eax,[InterPtr]
    cmp byte[eax],1
    jbe .ignorehr
    push ebx
    mov ebx,[InterPtr]
    call HighResProc
    pop ebx
    push ebx
    mov ecx,144
.nextb
    mov dword[ebx],0FFFFFFFFh
    add ebx,4
    dec ecx
    jnz .nextb
    pop ebx
    jmp .returninterp
.ignorehr

;srcPtr        equ 8
;deltaPtr      equ 12
;srcPitch      equ 16
;width         equ 20
;dstOffset     equ 24
;dstPitch      equ 28
;dstSegment    equ 32

%ifdef asdfasdfasdfsafdasdf
    cmp byte[En2xSaI],3
    je .super2xsai
%endif
    mov eax,1280        ; destination pitch
    push eax
    mov eax,edi         ; destination offset
    push eax
    mov eax,256         ; width
    push eax
    mov eax,576         ; source pitch
    push eax
    push ebx
    mov eax,esi         ; source pointer
    push eax
    cmp byte[En2xSaI],2
    je .supereagle
    call _2xSaILine
    jmp .normal
.supereagle
    call _2xSaISuperEagleLine
    jmp .normal

%ifdef asdgawe65egfdgfdg
.super2xsai
    pushad
    mov eax,1280        ; destination pitch
    push eax
    mov eax,[spritetablea]     ; destination offset
    add eax,512
    push eax
    mov eax,256         ; width
    push eax
    mov eax,576         ; source pitch
    push eax
    push ebx
    mov eax,esi         ; source pointer
    push eax
    call Super2xSaI
    add esp,24
    popad
    ; blit to screen
    mov eax,edi
    mov edx,[spritetablea]
    add edx,512
    mov ecx,128
.s2xsailoop
    movq mm0,[edx]
    movq [es:eax],mm0
    add edx,8
    add eax,8
    dec ecx
    jnz .s2xsailoop
    mov edx,[spritetablea]
    add edx,512+1280
    add eax,128*2
    mov ecx,128
.s2xsailoop2
    movq mm0,[edx]
    movq [es:eax],mm0
    add edx,8
    add eax,8
    dec ecx
    jnz .s2xsailoop2
    jmp .dones2xsai
%endif


.normal
    add esp,24
.dones2xsai
    add esi,576
    inc dword[InterPtr]
    add edi,1280*2
    add ebx,576
    dec dword[lineleft]
    jnz near .next
    mov ecx,256
    sub edi,640*2
.loop
    mov dword[es:edi],0
    add edi,4
    dec ecx
    jnz .loop
    emms
    pop es
    ret
.returninterp
    add esi,64
    inc dword[InterPtr]
    add edi,128*2
    dec byte[lineleft]
    jnz near .next
    emms
    pop es
    ret

NEWSYM smallscreen640x480x16b
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,32*2*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*640*2+32*2*2
.res239
    add edi,[VidStartDraw]
    add edi,128*2+120*640*2
    add esi,16*2+256*2+32*2
    xor eax,eax
    mov ebx,hirestiledat+1
    mov dl,[resolutn]
    dec dl
    dec dl
.loopa
    mov ecx,128
    rep movsd
    add esi,32*2
    add edi,640*2-256*2
    inc ebx
    dec dl
    jnz .loopa
    pop es
    ret

.fullscreen
    cmp byte[curblank],40h
    jne .startcopy2
    ret
.startcopy2
    cmp byte[GUIOn],1
    je .nointerpolat
    cmp byte[antienab],1
    jne .nointerpolat
    cmp byte[MMXSupport],1
    je near MMXInterpolFS
.nointerpolat
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    xor edi,edi
    cmp word[resolutn],224
    jne .res239b
    mov edi,8*320*2*2
.res239b
    add edi,[VidStartDraw]
    add esi,16*2+256*2+32*2
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[scanlines],1
    je near .scanlines
.loopa2
    mov ecx,128
.a
    mov ax,[esi]
    mov [es:edi],ax
    mov [es:edi+2],ax
    mov ax,[esi+2]
    mov [es:edi+4],ax
    mov [es:edi+6],ax
    mov [es:edi+8],ax
    add esi,4
    add edi,10
    dec ecx
    jnz .a
    sub esi,256*2
    mov ecx,128
.a2
    mov ax,[esi]
    mov [es:edi],ax
    mov [es:edi+2],ax
    mov ax,[esi+2]
    mov [es:edi+4],ax
    mov [es:edi+6],ax
    mov [es:edi+8],ax
    add esi,4
    add edi,10
    dec ecx
    jnz .a2
    add esi,64
    inc ebx
    dec dl
    jnz .loopa2
    pop es
    ret

.scanlines
.loopab
    mov ecx,128
.ab
    mov ax,[esi]
    mov [es:edi],ax
    mov [es:edi+2],ax
    mov ax,[esi+2]
    mov [es:edi+4],ax
    mov [es:edi+6],ax
    mov [es:edi+8],ax
    add esi,4
    add edi,10
    dec ecx
    jnz .ab
    add esi,64
    add edi,640*2
    inc ebx
    dec dl
    jnz .loopab
    pop es
    ret

MMXInterpolFS:
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    add esi,16*2+256*2+32*2
    xor edi,edi
    add edi,[VidStartDraw]
    mov dword[lineleft2],0FFFFh
    cmp word[resolutn],224
    jne .res239
    mov dword[lineleft2],14
.res239
    mov dl,[resolutn]
    sub dl,3
    movq mm2,[HalfTrans]
    mov [lineleft],dl
    mov edx,[spritetablea]
    mov ecx,64
    mov eax,[esi+510]
    add edx,512
    mov [esi+512],eax
.a2
    movq mm0,[esi]
    movq mm4,mm0
    movq mm1,[esi+2]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
;    movq [es:edi],mm4
;    movq [es:edi+8],mm5
    movq [edx],mm4
    movq [edx+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz .a2
    add esi,64
    add edi,128*2
.a5
    mov eax,[esi+510]
    mov ecx,32
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512
    ; Process next line
.a3
    ; aaaa/abbb/bbcc/cccd/dddd/
    ; aaaa/bbbA/ccBb/dCcc/Dddd/
    ; a / a >> 48, b << 16 / b >> 48, b >> 32 & 0xFFFF0000, c << 32 /
    ; c >> 32, c >> 16 & 0xFFFF00000000, d << 48 / d >> 16, d &0xFFFF000000000000
    movq mm0,[esi]
    movq mm4,mm0
    movq mm1,[esi+2]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    movq mm5,mm4
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq mm6,[edx]
    movq mm7,[edx+8]
    movq [edx],mm4
    movq [edx+8],mm5
    pand mm6,mm2
    pand mm7,mm2
    pand mm4,mm2
    pand mm5,mm2
    psrlw mm6,1
    psrlw mm7,1
    psrlw mm4,1
    psrlw mm5,1
    paddd mm4,mm6
    paddd mm5,mm7
    movq [es:edi],mm4
    movq [.interpspad],mm4
    movq [.interpspad+8],mm5
    movq mm4,[.interpspad+6]
    movq [es:edi+8],mm4
    movq [.interpspad+6],mm5

    movq mm0,[esi+8]
    movq mm4,mm0
    movq mm1,[esi+10]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    movq mm5,mm4
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq mm6,[edx+16]
    movq mm7,[edx+24]
    movq [edx+16],mm4
    movq [edx+24],mm5
    pand mm6,mm2
    pand mm7,mm2
    pand mm4,mm2
    pand mm5,mm2
    psrlw mm6,1
    psrlw mm7,1
    psrlw mm4,1
    psrlw mm5,1
    paddd mm4,mm6
    paddd mm5,mm7
    movq [.interpspad+16],mm4
    movq mm7,[.interpspad+12]
    movq [es:edi+16],mm7
    movq [.interpspad+14],mm4
    movq [.interpspad+24],mm5
    movq mm7,[.interpspad+18]
    movq [es:edi+24],mm7
    movq [.interpspad+22],mm5
    movq mm7,[.interpspad+24]
    movq [es:edi+32],mm7

    add esi,16
    add edi,40
    add edx,32
    dec ecx
    jnz near .a3
;    add edi,128*2

    mov edx,[spritetablea]
    add edx,512
    mov ecx,32
.a4
    movq mm4,[edx]
    movq mm5,[edx+8]
    movq [es:edi],mm4
    movq [.interpspad],mm4
    movq [.interpspad+8],mm5
    movq mm4,[.interpspad+6]
    movq [es:edi+8],mm4
    movq [.interpspad+6],mm5
    movq mm4,[edx+16]
    movq mm5,[edx+24]
    movq [.interpspad+16],mm4
    movq mm7,[.interpspad+12]
    movq [es:edi+16],mm7
    movq [.interpspad+14],mm4
    movq [.interpspad+24],mm5
    movq mm7,[.interpspad+18]
    movq [es:edi+24],mm7
    movq [.interpspad+22],mm5
    movq mm7,[.interpspad+24]
    movq [es:edi+32],mm7
    add edi,40
    add edx,32
    dec ecx
    jnz near .a4

    add esi,64
;    add edi,128*2

    dec dword[lineleft2]
    cmp dword[lineleft2],0
    jne near .norepeat
    mov dword[lineleft2],14
    mov edx,[spritetablea]
    add edx,512
    mov ecx,32
.a6
    movq mm4,[edx]
    movq mm5,[edx+8]
    movq [es:edi],mm4
    movq [.interpspad],mm4
    movq [.interpspad+8],mm5
    movq mm4,[.interpspad+6]
    movq [es:edi+8],mm4
    movq [.interpspad+6],mm5
    movq mm4,[edx+16]
    movq mm5,[edx+24]
    movq [.interpspad+16],mm4
    movq mm7,[.interpspad+12]
    movq [es:edi+16],mm7
    movq [.interpspad+14],mm4
    movq [.interpspad+24],mm5
    movq mm7,[.interpspad+18]
    movq [es:edi+24],mm7
    movq [.interpspad+22],mm5
    movq mm7,[.interpspad+24]
    movq [es:edi+32],mm7
    add edi,40
    add edx,32
    dec ecx
    jnz near .a6
;    add edi,128*2
.norepeat

    dec byte[lineleft]
    jnz near .a5
    emms
    pop es
    ret

SECTION .bss
.interpspad resd 8
SECTION .text


MMXInterpol:
    mov dl,[resolutn]
    sub dl,3
    movq mm2,[HalfTransC]

    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopab
    cmp byte[newengen],0
    je .loopab
    mov ebx,SpecialLine+1
.loopab
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],2
    je near .scanlinesquart
    cmp byte[scanlines],3
    je near .scanlineshalf
    inc ebx
    mov [lineleft],dl
    ; do scanlines
    mov edx,[spritetablea]
    mov ecx,64
    mov eax,[esi+510]
    add edx,512
    mov [esi+512],eax
.a2
    movq mm0,[esi]
    movq mm3,mm0
    movq mm4,mm0
    movq mm1,[esi+2]
    por mm3,mm1
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    pand mm3,[HalfTransB]
    paddw mm0,mm3
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [es:edi],mm4
    movq [es:edi+8],mm5
    movq [edx],mm4
    movq [edx+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz .a2
    add esi,64
    add edi,128*2
.a5
    cmp byte[ebx],1
    jbe .ignorehr
    call HighResProc
    movq mm2,[HalfTransC]
.nothrcopy
    jmp .returninterp
.ignorehr
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512
    ; Process next line
.a3
    movq mm0,[esi]
    movq mm3,mm0
    movq mm4,mm0
    movq mm1,[esi+2]
    por mm3,mm1
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    pand mm3,[HalfTransB]
    paddw mm0,mm3
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    movq mm6,[edx]
    movq mm7,[edx+8]
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edx],mm4
    movq [edx+8],mm5
    por mm0,mm4
    movq mm0,mm6
    pand mm4,mm2
    pand mm6,mm2
    psrlw mm4,1
    psrlw mm6,1
    pand mm0,[HalfTransB]
    paddd mm4,mm6
    paddw mm4,mm0
    movq mm0,mm5
    por mm0,mm7
    pand mm5,mm2
    pand mm7,mm2
    psrlw mm5,1
    pand mm0,[HalfTransB]
    psrlw mm7,1
    paddd mm5,mm7
    paddw mm5,mm0
    movq [es:edi],mm4
    movq [es:edi+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz near .a3
    add edi,128*2
    mov edx,[spritetablea]
    add edx,512
    mov ecx,64
.a4
    movq mm0,[edx]
    movq mm1,[edx+8]
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    add edi,16
    add edx,16
    dec ecx
    jnz .a4
.returninterp
    add esi,64
    add edi,128*2
    inc ebx
    dec byte[lineleft]
    jnz near .a5
    emms
    pop es
    ret
SECTION .bss
.blank resd 2
SECTION .text

.scanlines
    inc dl
    mov [lineleft],dl
    ; do scanlines
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
.asl
    cmp byte[ebx],1
    jbe .ignorehrs
    call HighResProc
    movq mm2,[HalfTrans]
    jmp .returninterps
.ignorehrs
.a
    movq mm0,[esi]
    movq mm4,mm0
    movq mm1,[esi+2]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [es:edi],mm4
    movq [es:edi+8],mm5
    add esi,8
    add edi,16
    dec ecx
    jnz .a
    mov eax,[esi+510+64]
    mov [esi+512+64],eax
.returninterps
    add esi,64
    add edi,128*2
    add edi,640*2
    inc ebx
    mov ecx,64
    dec byte[lineleft]
    jnz near .asl
    emms
    pop es
    ret

.scanlineshalf
    inc dl
    mov [lineleft],dl
    ; do scanlines
.ahb
    cmp byte[ebx],1
    jbe .ignorehrhs
    call HighResProc
    movq mm2,[HalfTrans]
    jmp .returninterphs
.ignorehrhs
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512
.ah
    movq mm0,[esi]
    movq mm4,mm0
    movq mm1,[esi+2]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edx],mm4
    movq [edx+8],mm5
    movq [es:edi],mm4
    movq [es:edi+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz .ah
    add edi,128*2
    sub edx,16*64
    mov ecx,64
.ahc
    movq mm0,[edx]
    movq mm1,[edx+8]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    add edi,16
    add edx,16
    dec ecx
    jnz .ahc
.returninterphs
    add edi,128*2
    add esi,64
    inc ebx
    dec byte[lineleft]
    jnz near .ahb
    emms
    pop es
    ret

.scanlinesquart
    inc dl
    mov [lineleft],dl
    ; do scanlines
.ahb2
    cmp byte[ebx],1
    jbe .ignorehrqs
    call HighResProc
    movq mm2,[HalfTransC]
    jmp .returninterpqs
.ignorehrqs
    mov eax,[esi+510]
    mov ecx,64
    mov [esi+512],eax
    mov edx,[spritetablea]
    add edx,512
.ah2
    movq mm0,[esi]
    movq mm3,mm0
    movq mm4,mm0
    movq mm1,[esi+2]
    por mm3,mm1
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    paddd mm0,mm1
    pand mm3,[HalfTransB]
    paddw mm0,mm3
    movq mm5,mm4
    ; mm4/mm5 contains original values, mm0 contains mixed values
    punpcklwd mm4,mm0
    punpckhwd mm5,mm0
    movq [edx],mm4
    movq [edx+8],mm5
    movq [es:edi],mm4
    movq [es:edi+8],mm5
    add esi,8
    add edi,16
    add edx,16
    dec ecx
    jnz .ah2
    add edi,128*2
    sub edx,16*64
    mov ecx,64
.ahc2
    movq mm0,[edx]
    movq mm1,[edx+8]
    pand mm0,mm2
    pand mm1,mm2
    psrlw mm0,1
    psrlw mm1,1
    movq mm4,mm0
    movq mm5,mm1
    pand mm4,mm2
    pand mm5,mm2
    psrlw mm4,1
    psrlw mm5,1
    paddd mm0,mm4
    paddd mm1,mm5
    movq [es:edi],mm0
    movq [es:edi+8],mm1
    add edi,16
    add edx,16
    dec ecx
    jnz .ahc2
.returninterpqs
    add esi,64
    add edi,128*2
    inc ebx
    dec byte[lineleft]
    jnz near .ahb2
    emms
    pop es
    ret

SECTION .bss
NEWSYM InterPtr, resd 1
SECTION .text

NEWSYM interpolate640x480x16b
    cmp byte[MMXSupport],1
    je near MMXInterpol
    mov byte[MMXSupport],0

    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabi
    cmp byte[newengen],0
    je .loopabi
    mov ebx,SpecialLine+1
.loopabi
    mov [InterPtr],ebx

    mov dl,[resolutn]
    sub dl,3
    cmp byte[scanlines],1
    je near .scanlines
    cmp byte[scanlines],2
    je near .scanlinesquart
    cmp byte[scanlines],3
    je near .scanlineshalf
    inc dword[InterPtr]
    mov [lineleft],dl
    ; do first line
    mov ecx,255
    mov edx,[spritetablea]
.a
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [es:edi],ebx
    mov [edx],ebx
    add esi,byte 2
    add edi,4
    add edx,4
    dec ecx
    jnz .a
    add esi,66
    add edi,130*2
.loopb
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehr
    call HighResProc
    jmp .returninterp
.ignorehr
    mov ecx,255
    mov edx,[spritetablea]
.c
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov eax,[edx]
    mov bx,[esi]
    and eax,[HalfTrans]
    mov [edx],ebx
    and ebx,[HalfTrans]
    shr eax,1
    shr ebx,1
    add eax,ebx
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    add edx,4
    dec ecx
    jnz .c
    add edi,130*2
    mov edx,[spritetablea]
    mov ecx,255
.d
    mov eax,[edx]
    mov [es:edi],eax
    add edx,4
    add edi,4
    dec ecx
    jnz .d
    inc dword[InterPtr]
    add esi,66
    add edi,130*2
    dec byte[lineleft]
    jnz near .loopb
    pop es
    ret
.returninterp
    inc dword[InterPtr]
    add esi,64
    add edi,128*2
    dec byte[lineleft]
    jnz near .loopb
    pop es
    ret

.scanlines
    xor eax,eax
    mov ebx,hirestiledat+1
    cmp byte[GUIOn],1
    je .loopabis
    cmp byte[newengen],0
    je .loopabis
    mov ebx,SpecialLine+1
.loopabis
.loopab
    mov ecx,255
    cmp byte[Triplebufen],1
    je .ignorehrb
    cmp byte[ebx],1
    jbe .ignorehrs
    call HighResProc
    jmp .returninterps
.ignorehrs
    cmp byte[ebx],1
    je .yeshiresb
.ignorehrb
    push ebx
.ab
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [es:edi],ebx
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .ab
    pop ebx
.returnb
    add esi,66
    add edi,130*2+640*2
    inc ebx
    dec dl
    jnz .loopab
    pop es
    cmp byte[Triplebufen],1
    je .ignorehr2b
    xor byte[res512switch],1
.ignorehr2b
    ret
.yeshiresb
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightsideb
.bb
    mov ax,[esi]
    mov [es:edi],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .bb
    jmp .returnb
.rightsideb
.cb
    mov ax,[esi]
    mov [es:edi+2],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .cb
    jmp .returnb
.returninterps
    add esi,64
    inc dword[InterPtr]
    add edi,128*2+640*2
    inc ebx
    dec byte[lineleft]
    jnz near .loopab
    pop es
    ret

.scanlineshalf
    xor eax,eax
    mov [lineleft],dl
.loopab2
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehrhs
    call HighResProc
    jmp .returninterphs
.ignorehrhs
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512
.ab2
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [edx],ebx
    mov [es:edi],ebx
    add esi,byte 2
    add edi,4
    add edx,4
    dec ecx
    jnz .ab2
    add edi,130*2
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512
.ab2b
    mov eax,[edx]
    and eax,[HalfTrans]
    shr eax,1
    mov [es:edi],eax
    add edi,4
    add edx,4
    dec ecx
    jnz .ab2b
    inc dword[InterPtr]
    add esi,66
    add edi,130*2
    dec byte[lineleft]
    jnz near .loopab2
    pop es
    ret
.returninterphs
    add esi,64
    inc dword[InterPtr]
    add edi,128*2
    dec byte[lineleft]
    jnz near .loopab2
    pop es
    ret

.scanlinesquart
    xor eax,eax
    mov [lineleft],dl
.loopab3
    mov ebx,[InterPtr]
    cmp byte[ebx],1
    jbe .ignorehrqs
    call HighResProc
    jmp .returninterpqs
.ignorehrqs
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512
.ab3
    mov ax,[esi]
    mov bx,[esi+2]
    and ebx,[HalfTrans+6]
    and eax,[HalfTrans+6]
    add ebx,eax
    shl ebx,15
    mov bx,[esi]
    mov [edx],ebx
    mov [es:edi],ebx
    add esi,byte 2
    add edi,4
    add edx,4
    dec ecx
    jnz .ab3
    add edi,130*2
    mov ecx,255
    mov edx,[spritetablea]
    add edx,512
.ab3b
    mov eax,[edx]
    and eax,[HalfTrans]
    shr eax,1
    mov ebx,eax
    and ebx,[HalfTrans]
    shr ebx,1
    add eax,ebx
    mov [es:edi],eax
    add edi,4
    add edx,4
    dec ecx
    jnz .ab3b
    inc dword[InterPtr]
    add esi,66
    add edi,130*2
    dec byte[lineleft]
    jnz near .loopab3
    pop es
    ret
.returninterpqs
    add esi,64
    inc dword[InterPtr]
    add edi,128*2
    dec byte[lineleft]
    jnz near .loopab2
    pop es
    ret

;*******************************************************
; Copy VESA2 640x480x16b, bit setting 1:5:5:5
;*******************************************************

SECTION .bss
.interpspad resd 8

SECTION .text

;*******************************************************
; Copy VESA2 512x384x16b  Copies buffer to 512x384x16bV2
;*******************************************************

;NEWSYM copyvesa2512x384x16bgui
;    cmp byte[curblank],40h
;    jne .startcopy
;    ret
;.startcopy
;    push es
;    mov byte[.lastrep],0
;    mov ax,[vesa2selec]
;    mov es,ax
;    mov esi,[vidbuffer]
;    mov byte[.scratio],61       ; 60.6695
;    cmp word[resolutn],224
;    jne .res239
;    mov byte[.scratio],72       ; 72.4286
;.res239
;    mov edi,[VidStartDraw]
;    add esi,16+256+32
;    xor eax,eax
;    mov dl,[resolutn]
;    sub dl,2
;    xor dh,dh
;.loopa
;    mov ecx,256
;    xor eax,eax
;.a
;    mov al,[esi]
;    mov bx,[GUICPC+eax*2]
;    mov [es:edi],bx
;    mov [es:edi+2],bx
;    inc esi
;    add edi,4
;    dec ecx
;    jnz .a
;    cmp byte[.lastrep],1
;    je .no2
;    sub dh,[.scratio]
;    jnc .no2
;    add dh,100
;    sub esi,256
;    inc dl
;    dec ebx
;    mov byte[.lastrep],1
;    jmp .yes2
;.no2
;    mov byte[.lastrep],0
;    add esi,32
;.yes2
;    dec dl
;    jnz .loopa
;    pop es
;    ret
;

NEWSYM copyvesa2512x384x16b
    cmp byte[vesa2red10],1
    jne .noconvertr
    call ConvertToAFormat
.noconvertr
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    cmp byte[smallscreenon],1
    je near .smallscreen
    push es
    mov byte[.lastrep],0
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov byte[.scratio],61       ; 60.6695
    cmp word[resolutn],224
    jne .res239
    mov byte[.scratio],72       ; 72.4286
.res239
    mov edi,[VidStartDraw]
    add esi,16*2+256*2+32*2
    xor eax,eax
    mov ebx,hirestiledat+1
    mov dl,[resolutn]
    dec dl
    dec dl
    xor dh,dh
.loopa
    mov al,[ebx]
    mov [.p512],al
    cmp byte[Triplebufen],1
    je .ignorehr
    cmp al,1
    je near .yeshires
.ignorehr
    mov ecx,256
.a
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
.returnloop
    cmp byte[.lastrep],1
    je .no2
    sub dh,[.scratio]
    jnc .no2
    add dh,100
    sub esi,512
    inc dl
    mov al,[.p512]
    mov [ebx],al
    dec ebx
    mov byte[.lastrep],1
    jmp .yes2
.no2
    mov byte[.lastrep],0
    add esi,64
.yes2
    inc ebx
    dec dl
    jnz .loopa
    pop es
    cmp byte[Triplebufen],1
    je .ignorehr2
    xor byte[res512switch],1
.ignorehr2
    ret
.yeshires
    mov byte[ebx],0
    test byte[res512switch],1
    jnz .rightside
    mov ecx,256
.b
    mov ax,[esi]
    mov [es:edi],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .b
    jmp .returnloop
.rightside
    mov ecx,256
.b2
    mov ax,[esi]
    mov [es:edi+2],ax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .b2
    jmp .returnloop

.smallscreen
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,[VidStartDraw]
    add esi,16*2+256*2+32*2
    add edi,72*512*2+128*2
    cmp byte[resolutn],224
    jne .ssres239
    add edi,8*512*2
.ssres239
    xor eax,eax
    mov dl,[resolutn]
    dec dl
    dec dl
    cmp byte[MMXSupport],1
    je .ssloopb
.ssloopa
    mov ecx,64*2
    rep movsd
    add esi,32*2
    add edi,128*2*2
    dec dl
    jnz .ssloopa
    jmp .done
.ssloopb
    mov ecx,16*2
    MMXStuff
    add esi,32*2
    add edi,128*2*2
    dec dl
    jnz .ssloopb
    emms
.done
    pop es
    ret

SECTION .bss
.scratio resb 1
.lastrep resb 1
.p512    resb 1
SECTION .text

; Temporary

NEWSYM tempcopy
    cmp byte[pressed+12],1
    jne .nocolch
    mov byte[pressed+12],2
    add byte[.cocol],16
.nocolch
    cmp byte[pressed+13],1
    jne .nocolch2
    mov byte[pressed+13],2
    add dword[.startbuf],512*64
    cmp dword[.startbuf],512*64*4
    jne .nores
    mov dword[.startbuf],0
.nores
.nocolch2
    ; cache all sprites
    call allcache
    pusha
    ; copy [vcache4b]+bg1objptr*2 into
    xor ebx,ebx
    mov bx,[objptr]
    shl ebx,1
    add ebx,[vcache4b]
    add ebx,[.startbuf]
    mov edi,[vidbuffer]
    add edi,16
    mov esi,edi
    mov dh,16
.loopd
    mov dl,32
.loopc
    mov ch,8
.loopb
    mov cl,8
.loopa
    mov al,[ebx]
    add al,[.cocol]
    mov [edi],al
    inc edi
    inc ebx
    dec cl
    jnz .loopa
    add edi,248+32
    dec ch
    jnz .loopb
    add esi,8
    mov edi,esi
    dec dl
    jnz .loopc
    add esi,288*8-32*8
    mov edi,esi
    dec dh
    jnz .loopd
    popa
    ret

SECTION .bss
.cocol resb 1
.startbuf resd 1
SECTION .text

NEWSYM allcache
    pushad
    mov esi,[vram]
    mov edi,[vcache4b]
    mov ecx,2048
.nextcache
    ; convert from [esi] to [edi]
    ; use ah = color 0, bl = color 1, bh = color 2, cl = color 3
    ; ch = color 4, dl = color 5, dh = color 6, .a = color 7
    push edi
    push esi
    push ecx

    mov byte[.rowleft],8
.donext
    xor ah,ah
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov byte[.a],0
    mov al,[esi]                ; bitplane 0
    cmp al,0
    je .skipconva
    test al,01h
    jz .skipa0
    or ah,01h
.skipa0
    test al,02h
    jz .skipa1
    or bl,01h
.skipa1
    test al,04h
    jz .skipa2
    or bh,01h
.skipa2
    test al,08h
    jz .skipa3
    or cl,01h
.skipa3
    test al,10h
    jz .skipa4
    or ch,01h
.skipa4
    test al,20h
    jz .skipa5
    or dl,01h
.skipa5
    test al,40h
    jz .skipa6
    or dh,01h
.skipa6
    test al,80h
    jz .skipa7
    or byte[.a],01h
.skipa7
.skipconva

    mov al,[esi+1]                ; bitplane 1
    cmp al,0
    je .skipconvb
    test al,01h
    jz .skipb0
    or ah,02h
.skipb0
    test al,02h
    jz .skipb1
    or bl,02h
.skipb1
    test al,04h
    jz .skipb2
    or bh,02h
.skipb2
    test al,08h
    jz .skipb3
    or cl,02h
.skipb3
    test al,10h
    jz .skipb4
    or ch,02h
.skipb4
    test al,20h
    jz .skipb5
    or dl,02h
.skipb5
    test al,40h
    jz .skipb6
    or dh,02h
.skipb6
    test al,80h
    jz .skipb7
    or byte[.a],02h
.skipb7
.skipconvb

    mov al,[esi+16]                ; bitplane 2
    cmp al,0
    je .skipconvc
    test al,01h
    jz .skipc0
    or ah,04h
.skipc0
    test al,02h
    jz .skipc1
    or bl,04h
.skipc1
    test al,04h
    jz .skipc2
    or bh,04h
.skipc2
    test al,08h
    jz .skipc3
    or cl,04h
.skipc3
    test al,10h
    jz .skipc4
    or ch,04h
.skipc4
    test al,20h
    jz .skipc5
    or dl,04h
.skipc5
    test al,40h
    jz .skipc6
    or dh,04h
.skipc6
    test al,80h
    jz .skipc7
    or byte[.a],04h
.skipc7
.skipconvc

    mov al,[esi+17]                ; bitplane 3
    cmp al,0
    je .skipconvd
    test al,01h
    jz .skipd0
    or ah,08h
.skipd0
    test al,02h
    jz .skipd1
    or bl,08h
.skipd1
    test al,04h
    jz .skipd2
    or bh,08h
.skipd2
    test al,08h
    jz .skipd3
    or cl,08h
.skipd3
    test al,10h
    jz .skipd4
    or ch,08h
.skipd4
    test al,20h
    jz .skipd5
    or dl,08h
.skipd5
    test al,40h
    jz .skipd6
    or dh,08h
.skipd6
    test al,80h
    jz .skipd7
    or byte[.a],08h
.skipd7
.skipconvd

    ; move all bytes into [edi]
    mov [edi+7],ah
    mov [edi+6],bl
    mov [edi+5],bh
    mov [edi+4],cl
    mov [edi+3],ch
    mov [edi+2],dl
    mov [edi+1],dh
    mov al,[.a]
    mov [edi],al
    add edi,8
    add esi,byte 2
    dec byte[.rowleft]
    jnz near .donext

    pop ecx
    pop esi
    pop edi

    add esi,32
    add edi,64
    dec cx
    jnz near .nextcache
    popad
    ret

SECTION .bss
.nbg resw 1
.a   resb 1
.rowleft resb 1
SECTION .text

;*******************************************************
; Copy VESA1.2 640x480x16b
;*******************************************************

NEWSYM copyvesa12640x480x16b
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    cmp byte[vesa2red10],1
    jne .nocopyvesa2r
    call ConvertToAFormat
.nocopyvesa2r
    push es
    mov word[bankpos],0
    call VESA12Bankswitch
    mov ax,[selcA000]
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,32*2*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*320*2*2+32*2*2
.res239
    add esi,16*2+256*2+32*2
    xor eax,eax
    ; Check if interpolation mode
.nommx
    mov dl,[resolutn]
    dec dl
    dec dl
    mov [lineleft],dl
    mov edx,65536
    sub edx,edi
    shr edx,2
    cmp byte[smallscreenon],1
    je near .smallscreen
    cmp byte[scanlines],1
    je near .scanlines
.loopa
    mov ecx,256
    precheckvesa12 256
.a
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a
    postcheckvesa12 .a,64,256,16384
    sub esi,256*2
    add edi,128*2
    mov ecx,256
    precheckvesa12 256
.a2
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .a2
    postcheckvesa12 .a2,64,256,16384
    add esi,64
    add edi,128*2
    inc ebx
    dec byte[lineleft]
    jnz near .loopa
    pop es
    ret

.scanlines
.loopab
    mov ecx,256
    precheckvesa12 256
.ab
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [es:edi],eax
    add esi,byte 2
    add edi,4
    dec ecx
    jnz .ab
    postcheckvesa12 .ab,64+320,256,16384
    mov ecx,256
    add esi,64
    add edi,128*2+640*2
    inc ebx
    dec byte[lineleft]
    jnz near .loopab
    pop es
    ret

.smallscreen
.loopac
    mov ecx,128
    precheckvesa12 128
.ac
    movsd
    dec ecx
    jnz .ac
    postcheckvesa12 .ac,64+128,128,16384
    mov ecx,128
    add esi,64
    add edi,128*2+256*2
    inc ebx
    dec byte[lineleft]
    jnz near .loopac
    pop es
    ret
%endif
;*******************************************************
; Clear Screen
;*******************************************************
%ifdef __MSDOS__
NEWSYM DOSClearScreen
    cmp byte[cvidmode],0
    je near cscopymodeq
    cmp byte[cvidmode],1
    je near cscopymodeq
    cmp byte[cvidmode],2
    je near cscopymodeq
    cmp byte[cvidmode],3
    je near cscopymodex
    cmp byte[cvidmode],4
    je near cscopymodex
    cmp byte[cvidmode],5
    je near cscopymodex
    cmp byte[cvidmode],6
    je near cscopyvesa12640x480x16b
    cmp byte[cvidmode],7
    je near cscopyvesa2320x240x8b
    cmp byte[cvidmode],8
    je near cscopyvesa2320x240x16b
    cmp byte[cvidmode],9
    je near cscopyvesa2320x480x8b
    cmp byte[cvidmode],10
    je near cscopyvesa2320x480x16b
    cmp byte[cvidmode],11
    je near cscopyvesa2512x384x8b
    cmp byte[cvidmode],12
    je near cscopyvesa2512x384x16b
    cmp byte[cvidmode],13
    je near cscopyvesa2640x400x8b
    cmp byte[cvidmode],14
    je near cscopyvesa2640x400x16b
    cmp byte[cvidmode],15
    je near cscopyvesa2640x480x8b
    cmp byte[cvidmode],16
    je near cscopyvesa2640x480x16b
    cmp byte[cvidmode],17
    je near cscopyvesa2800x600x8b
    cmp byte[cvidmode],18
    je near cscopyvesa2800x600x16b
    ret
%endif

%macro TripleBufferClear 0
   cmp byte[Triplebufen],0
   je %%noclear
   push ebx
   mov ebx,ecx
   add ecx,ebx
   add ecx,ebx
   pop ebx
%%noclear
%endmacro

%ifdef __MSDOS__
NEWSYM cscopymodeq
    push es
    mov ax,[selcA000]
    mov es,ax
    xor eax,eax
    mov ecx,16384
    xor edi,edi
    rep stosd
    pop es
    ret

NEWSYM cscopymodex
    ; select all planes
    mov edx,03C4h
    mov eax,0F02h
    out dx,ax
    push es
    mov ax,[selcA000]
    mov es,ax
    xor edi,edi
    mov ecx,65536/4
    xor eax,eax
    rep stosd
    pop es
    ret

NEWSYM cscopyvesa2320x240x8b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,320*240
    TripleBufferClear
.loop
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loop
    pop es
    ret

NEWSYM cscopyvesa2320x240x16b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,320*240*2
    TripleBufferClear
.loop
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loop
    pop es
    ret

NEWSYM cscopyvesa2640x480x8b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,640*480
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2640x480x16b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,640*480*2
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2800x600x8b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,800*600
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2800x600x16b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,800*600*2
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2640x400x8b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,640*400
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2640x400x16b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,640*400*2
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2320x480x8b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,320*480
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2320x480x16b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,320*480*2
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2512x384x8b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,512*384
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

NEWSYM cscopyvesa2512x384x16b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,[VidStartDraw]
    mov ecx,512*384*2
    TripleBufferClear
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret
%endif

NEWSYM GetScreen
%ifdef __MSDOS__
    cmp byte[cvidmode],9
    je near getcopyvesa2320x240x16b
    cmp byte[cvidmode],14
;    je near getcopyvesa2640x480x16b
    cmp byte[cvidmode],12
;    je near getcopyvesa2512x384x16b
%endif
    ret

%ifdef __MSDOS__
NEWSYM getcopyvesa2320x240x16b
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,32*2           ; Draw @ Y from 9 to 247
    cmp word[resolutn],224
    jne .res239
    mov edi,8*320*2+32*2
.res239
    add edi,[VidStartDraw]
    xor ebx,ebx
    mov bx,[resolutn]
    mov esi,[vidbuffer]
    add esi,32+288*2
    mov ecx,256
.loop
    mov ax,[es:edi]
    mov [esi],ax
    add edi,byte 2
    add esi,byte 2
    dec ecx
    jnz .loop
    add edi,128
    add esi,64
    mov ecx,256
    dec ebx
    jnz .loop
    pop es
    cmp byte[vesa2red10],0
    jne .redvalue
    ret
.redvalue
    call ConvertImageDatared10
    ret

ConvertImageDatared10:
    xor ebx,ebx
    mov bx,[resolutn]
    mov esi,[vidbuffer]
    add esi,32+288*2
    mov ecx,256
.loop
    mov ax,[esi]
    mov dx,ax
    and ax,0000000000011111b
    and dx,0111111111100000b
    shl dx,1
    or ax,dx
    mov [esi],ax
    add esi,byte 2
    dec ecx
    jnz .loop
    add esi,64
    mov ecx,256
    dec ebx
    jnz .loop
    ret

NEWSYM cscopyvesa12640x480x16b
    push es
    mov word[bankpos],0
    call VESA12Bankswitch
    mov ax,[selcA000]
    mov es,ax
    mov edi,32*2*2           ; Draw @ Y from 9 to 247
    xor eax,eax
    mov dl,239
    dec dl
    dec dl
    mov [lineleft],dl
    mov edx,65536
    sub edx,edi
    shr edx,2
.loopa
    mov ecx,256
    precheckvesa12 256
    xor eax,eax
.a
    mov [es:edi],eax
    add edi,4
    dec ecx
    jnz .a
    postcheckvesa12 .a,64,256,16384
    add edi,128*2
    mov ecx,256
    precheckvesa12 256
    xor eax,eax
.a2
    mov [es:edi],eax
    add edi,4
    dec ecx
    jnz .a2
    postcheckvesa12 .a2,64,256,16384
    add edi,128*2
    inc ebx
    dec byte[lineleft]
    jnz near .loopa
    pop es
    ret
%endif


