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

EXTSYM AddSub256,InitVesa2,cbitmode,cvidmode,makepal
EXTSYM scanlines,selcA000,vesa2_bits,vesa2_bpos,vesa2_clbit
EXTSYM vesa2_gpos,vesa2_rpos,vesa2_usbit,vesa2_x,vesa2_y
EXTSYM vesa2selec,InitVesa12,videotroub

NEWSYM InitVidAsmStart




NEWSYM clearfilter, dw 0
NEWSYM res640,      db 0
NEWSYM res480,      db 0

NEWSYM dosinitvideo2
       jmp dosinitvideo.noaddsub

;*******************************************************
; InitVideo
;*******************************************************
NEWSYM dosinitvideo
    mov byte[cbitmode],0
    mov byte[res640],0
    mov byte[res480],0
    cmp byte[cvidmode],0
    je near .initmodeq
    cmp byte[cvidmode],1
    je near .initmodex
    cmp byte[cvidmode],2
    je near .initvesa12640x480x16
    cmp byte[cvidmode],3
    je near .initvesa2320x240x8
    cmp byte[cvidmode],4
    je near .initvesa2320x240x16
    cmp byte[cvidmode],5
    je near .initvesa2320x480x8
    cmp byte[cvidmode],6
    je near .initvesa2320x480x16
    cmp byte[cvidmode],7
    je near .initvesa2512x384x8
    cmp byte[cvidmode],8
    je near .initvesa2512x384x16
    cmp byte[cvidmode],9
    je near .initvesa2640x480x8
    cmp byte[cvidmode],10
    je near .initvesa2640x480x16
    ret

;*******************************************************
; InitModeX               Sets up 320x240 unchained mode
;*******************************************************

.initmodex
    mov byte[cbitmode],0
    mov ax,0013h
    int 10h

    mov dx,03C4h
    mov ax,0604h
    out dx,ax
    mov dx,03D4h
    mov ax,0E317h 
    out dx,ax
    mov ax,0014h 
    out dx,ax
    mov dx,03C4h
    mov ax,0F02h
    out dx,ax

    mov dx,03C2h
    mov al,0E3h
    out dx,al
    mov dx,03D4h
    mov ax,2C11h
    out dx,ax
    mov ax,0D06h
    out dx,ax
    mov ax,3E07h
    out dx,ax
    mov ax,0EA10h
    out dx,ax
    mov ax,0AC11h
    out dx,ax
    mov ax,0DF12h
    out dx,ax
    mov ax,0E715h
    out dx,ax
    mov ax,0616h
    out dx,ax

    mov dx,03C6h
    mov al,0FFh
    out dx,al
    mov dx,03C4h
    ; select all 4 planes and clear
    mov ax,0F02h
    out dx,ax
    push es
    mov ax,[selcA000]
    mov es,ax
    xor edi,edi
    mov ecx,65536/4
    xor eax,eax
    rep stosd
    pop es
    call makepal
    ret

;*******************************************************
; InitModeQ                 Sets up 256x256 chained mode
;*******************************************************

.initmodeq
    mov byte[cbitmode],0
    cmp byte[AddSub256],1
    jne .noaddsub
    mov byte[cbitmode],1
.noaddsub
    mov byte[vesa2_rpos],11      ; Red bit position 128,256,512,1024,2048
    mov byte[vesa2_gpos],6       ; Green bit position 2^6 = 64
    mov byte[vesa2_bpos],0       ; Blue bit position
    mov word[vesa2_usbit],0020h  ; Unused bit in proper bit location
    mov word[vesa2_clbit],0F7DFh ; clear all bit 0's if AND is used
    mov word[clearfilter],0F7DFh ; Filter out unnecessary bits

;    cmp byte[scanlines],1
;    je near .scanlines
    mov ax,0013h
    int 10h
    mov dx,03D4h
    mov al,11h
    out dx,al

    inc dx
    in al,dx
    and al,7Fh
    mov ah,al
    dec dx
    mov al,11h
    out dx,al
    inc dx
    mov al,ah
    out dx,al

    mov dx,03C2h
    mov al,0E3h
    out dx,al
    mov dx,03D4h
    mov ax,5F00h
    out dx,ax
    mov ax,3F01h
    out dx,ax
    mov ax,4002h
    out dx,ax
    mov ax,8203h
    out dx,ax
    mov ax,4A04h
    out dx,ax
    mov ax,9A05h
    out dx,ax
    mov ax,2306h
    out dx,ax
    mov ax,0B207h
    out dx,ax
    mov ax,0008h
    out dx,ax
    mov ax,6109h
    out dx,ax
    mov ax,0A10h
    out dx,ax
    mov ax,0AC11h
    out dx,ax
    mov ax,0FF12h
    out dx,ax
    mov ax,2013h
    out dx,ax
    mov ax,4014h
    out dx,ax
    mov ax,0715h
    out dx,ax
    mov ax,1A16h
    out dx,ax
    mov ax,0A317h
    out dx,ax
    mov dx,03C4h
    mov ax,0101h
    out dx,ax
    mov ax,0E04h
    out dx,ax
    mov dx,03CEh
    mov ax,4005h
    out dx,ax
    mov ax,0506h
    out dx,ax

    mov dx,03DAh
    in al,dx
    mov dx,03C0h
    mov al,30h
    out dx,al
    mov al,41h
    out dx,al

    mov dx,03DAh
    in al,dx
    mov dx,03C0h
    mov al,33h
    out dx,al
    mov al,0h
    out dx,al

    mov dx,03C6h
    mov al,0FFh
    out dx,al
    cmp byte[cbitmode],1
    je .nopal
    call makepal
.nopal
    ; clear screen
    push es
    mov ax,[selcA000]
    mov es,ax
    xor edi,edi
    mov ecx,256*64
    xor eax,eax
    rep stosd
    pop es
    ret

;.scanlines
;    mov ax,0013h
;    int 10h
;    mov dx,03D4h
;    mov al,11h
;    out dx,al
;
;    inc dx
;    in al,dx
;    and al,7Fh
;    mov ah,al
;    dec dx
;    mov al,11h
;    out dx,al
;    inc dx
;    mov al,ah
;    out dx,al
;
;    mov dx,03C2h
;    mov al,0E3h
;    out dx,al
;    mov dx,03D4h
;    mov ax,5F00h
;    out dx,ax
;    mov ax,3F01h
;    out dx,ax
;    mov ax,4002h
;    out dx,ax
;    mov ax,8203h
;    out dx,ax
;    mov ax,4A04h
;    out dx,ax
;    mov ax,9A05h
;    out dx,ax
;    mov ax,2306h
;    out dx,ax
;    mov ax,01D07h
;    out dx,ax
;    mov ax,0008h
;    out dx,ax
;    mov ax,6009h
;    out dx,ax
;    mov ax,0A10h
;    out dx,ax
;    mov ax,0AC11h
;    out dx,ax
;    mov ax,0FF12h
;    out dx,ax
;    mov ax,2013h
;    out dx,ax
;    mov ax,4014h
;    out dx,ax
;    mov ax,0715h
;    out dx,ax
;    mov ax,1A16h
;    out dx,ax
;    mov ax,0A317h
;    out dx,ax
;    mov dx,03C4h
;    mov ax,0101h
;    out dx,ax
;    mov ax,0E04h
;    out dx,ax
;    mov dx,03CEh
;    mov ax,4005h
;    out dx,ax
;    mov ax,0506h
;    out dx,ax
;
;    mov dx,03DAh
;    in al,dx
;    mov dx,03C0h
;    mov al,30h
;    out dx,al
;    mov al,41h
;    out dx,al
;
;    mov dx,03DAh
;    in al,dx
;    mov dx,03C0h
;    mov al,33h
;    out dx,al
;    mov al,0h
;    out dx,al
;
;    mov dx,03C6h
;    mov al,0FFh
;    out dx,al
;    cmp byte[cbitmode],1
;    je .nopalb
;    call makepal
;.nopalb
;    ; clear screen
;    push es
;    mov ax,[selcA000]
;    mov es,ax
;    xor edi,edi
;    mov ecx,256*64
;    xor eax,eax
;    rep stosd
;    pop es
;    ret

;*******************************************************
; InitVESA2 320x240x8           Set up Linear 320x240x8b
;*******************************************************

.initvesa2320x240x8
    mov byte[cbitmode],0
    mov word[vesa2_x],320
    mov word[vesa2_y],240
    mov byte[vesa2_bits],8
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble
    ret
.notrouble
    call makepal
    ; clear screen (320*240 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,320*240
.loop
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loop
    pop es
    ret

;*******************************************************
; InitVESA2 320x240x16         Set up Linear 320x240x16b
;*******************************************************

.initvesa2320x240x16
    mov byte[cbitmode],1
    mov word[vesa2_x],320
    mov word[vesa2_y],240
    mov byte[vesa2_bits],16
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble2
    ret
.notrouble2
    ; clear screen (320*240*2 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,320*240*2
.loopb
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopb
    pop es
    ret

;*******************************************************
; InitVESA2 640x480x8           Set up Linear 640x480x8b
;*******************************************************

.initvesa2640x480x8
    mov byte[res640],1
    mov byte[res480],1
    mov word[vesa2_x],640
    mov word[vesa2_y],480
    mov byte[vesa2_bits],8
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble3
    ret
.notrouble3
    call makepal
    ; clear screen (640*480 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,640*480
.loopc3
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopc3
    pop es
    ret

;*******************************************************
; InitVESA2 640x480x16         Set up Linear 640x480x16b
;*******************************************************

.initvesa2640x480x16
    mov byte[res640],1
    mov byte[res480],1
    mov byte[cbitmode],1
    mov word[vesa2_x],640
    mov word[vesa2_y],480
    mov byte[vesa2_bits],16
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble4
    ret
.notrouble4
    ; clear screen (640*480*2 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,640*480*2
.loopd3
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopd3
    pop es
    ret

;*******************************************************
; InitVESA2 320x480x8           Set up Linear 320x480x8b
;*******************************************************

.initvesa2320x480x8
    mov byte[res480],1
    mov word[vesa2_x],320
    mov word[vesa2_y],480
    mov byte[vesa2_bits],8
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble5
    ret
.notrouble5
    call makepal
    ; clear screen (320*480 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,320*480
.loopc
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopc
    pop es
    ret

;*******************************************************
; InitVESA2 320x480x16         Set up Linear 320x480x16b
;*******************************************************

.initvesa2320x480x16
    mov byte[res480],1
    mov byte[cbitmode],1
    mov word[vesa2_x],320
    mov word[vesa2_y],480
    mov byte[vesa2_bits],16
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble6
    ret
.notrouble6
    ; clear screen (320*480*2 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,320*480*2
.loopd
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopd
    pop es
    ret

;*******************************************************
; InitVESA2 512x384x8           Set up Linear 512x384x8b
;*******************************************************

.initvesa2512x384x8
    mov byte[res640],2
    mov word[vesa2_x],512
    mov word[vesa2_y],384
    mov byte[vesa2_bits],8
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble7
    ret
.notrouble7
    call makepal
    ; clear screen (512*384 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,512*384
.loope
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loope
    pop es
    ret

;*******************************************************
; InitVESA2 512x384x16         Set up Linear 512x384x16b
;*******************************************************

.initvesa2512x384x16
    mov byte[res640],2
    mov byte[cbitmode],1
    mov word[vesa2_x],512
    mov word[vesa2_y],384
    mov byte[vesa2_bits],16
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble8
    ret
.notrouble8
    ; clear screen (512*384*2 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,512*384*2
.loopf
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopf
    pop es
    ret


;*******************************************************
; InitVESA1.2 640x480x16              Set up 640x480x16b
;*******************************************************

.initvesa12640x480x16
    mov byte[res640],1
    mov byte[cbitmode],1
    mov word[vesa2_x],640
    mov word[vesa2_y],480
    mov byte[vesa2_bits],16
    call InitVesa12
    ret

    ; clear screen (640*480*2 bytes)
    push es
    mov ax,[selcA000]
    mov es,ax
    mov ebx,9
    xor edx,edx
.loopbanks
    
    xor edi,edi
    mov ecx,16384
.loopg
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopg
    dec ebx
    jnz .loopbanks

    mov ecx,6144
    pop es
    ret

NEWSYM InitVidAsmEnd
