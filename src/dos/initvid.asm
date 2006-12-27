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

EXTSYM InitVesa2,cbitmode,cvidmode,dosmakepal,scanlines,selcA000,vesa2_bits
EXTSYM vesa2_x,vesa2_y,vesa2selec,InitVesa12,videotroub,cscopymodeq,cscopymodex
EXTSYM res640,res480


SECTION .text
NEWSYM initvideo2
    cmp byte[cvidmode],2
    jne .nomodeq
    jmp dosinitvideo.initmodeq256
.nomodeq
    cmp byte[cvidmode],5
    jne .nomodex
    jmp dosinitvideo.initmodex256
.nomodex

;*******************************************************
; InitVideo
;*******************************************************
NEWSYM dosinitvideo
    mov byte[cbitmode],0
    mov byte[res640],0
    mov byte[res480],0
    cmp byte[cvidmode],0
    je near .initmodeq224
    cmp byte[cvidmode],1
    je near .initmodeq240
    cmp byte[cvidmode],2
    je near .initmodeq256
    cmp byte[cvidmode],3
    je near .initmodex224
    cmp byte[cvidmode],4
    je near .initmodex240
    cmp byte[cvidmode],5
    je near .initmodex256
    cmp byte[cvidmode],6
    je near .initvesa12640x480x16
    cmp byte[cvidmode],7
    je near .initvesa2320x240x8
    cmp byte[cvidmode],8
    je near .initvesa2320x240x16
    cmp byte[cvidmode],9
    je near .initvesa2320x480x8
    cmp byte[cvidmode],10
    je near .initvesa2320x480x16
    cmp byte[cvidmode],11
    je near .initvesa2512x384x8
    cmp byte[cvidmode],12
    je near .initvesa2512x384x16
    cmp byte[cvidmode],13
    je near .initvesa2640x400x8
    cmp byte[cvidmode],14
    je near .initvesa2640x400x16
    cmp byte[cvidmode],15
    je near .initvesa2640x480x8
    cmp byte[cvidmode],16
    je near .initvesa2640x480x16
    cmp byte[cvidmode],17
    je near .initvesa2800x600x8
    cmp byte[cvidmode],18
    je near .initvesa2800x600x16
    ret

%include "dos/vga.inc"


;*******************************************************
; InitModeQ 224             Sets up 256x224 chained mode
;*******************************************************

.initmodeq224
    SetVGAMode .Mode256x224c
    call cscopymodeq
    call dosmakepal
    ret

;*******************************************************
; InitModeQ 240             Sets up 256x240 chained mode
;*******************************************************

.initmodeq240
    SetVGAMode .Mode256x240c
    call cscopymodeq
    call dosmakepal
    ret

;*******************************************************
; InitModeQ 256             Sets up 256x256 chained mode
;*******************************************************

.initmodeq256
    cmp byte[scanlines],1
    je near .scanlines
    SetVGAMode .Mode256x256c
    jmp .done
.scanlines
    SetVGAMode .Mode256x256cs
    jmp .done
.done
    call cscopymodeq
    call dosmakepal
    ret


;*******************************************************
; InitModeX 224           Sets up 320x224 unchained mode
;*******************************************************

.initmodex224
    SetVGAMode .Mode320x224
    call cscopymodex
    call dosmakepal
    ret

;*******************************************************
; InitModeX 240           Sets up 320x240 unchained mode
;*******************************************************

.initmodex240
    SetVGAMode .Mode320x240
    call cscopymodex
    call dosmakepal
    ret

;*******************************************************
; InitModeX 256           Sets up 320x256 unchained mode
;*******************************************************

.initmodex256
    cmp byte[scanlines],1
    je near .scanlines2
    SetVGAMode .Mode320x256
    jmp .done2
.scanlines2
    SetVGAMode .Mode320x256s
    jmp .done2
.done2
    call cscopymodex
    call dosmakepal
    ret


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
    call dosmakepal
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
    call dosmakepal
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
; InitVESA2 800x600x8           Set up Linear 800x600x8b
;*******************************************************

.initvesa2800x600x8
    mov byte[res640],1
    mov byte[res480],1
    mov word[vesa2_x],800
    mov word[vesa2_y],600
    mov byte[vesa2_bits],8
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble11
    ret
.notrouble11
    call dosmakepal
    ; clear screen (800*600 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,800*600
.looph2
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .looph2
    pop es
    ret

;*******************************************************
; InitVESA2 800x600x16         Set up Linear 800x600x16b
;*******************************************************

.initvesa2800x600x16
    mov byte[res640],1
    mov byte[res480],1
    mov byte[cbitmode],1
    mov word[vesa2_x],800
    mov word[vesa2_y],600
    mov byte[vesa2_bits],16
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble12
    ret
.notrouble12
    ; clear screen (800*600*2 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,800*600*2
.looph3
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .looph3
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
    call dosmakepal
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
    mov byte[cbitmode],0
    mov word[vesa2_x],512
    mov word[vesa2_y],384
    mov byte[vesa2_bits],8
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble7
    ret
.notrouble7
    call dosmakepal
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
; InitVESA2 640x400x8           Set up Linear 640x400x8b
;*******************************************************

.initvesa2640x400x8
    mov byte[res640],2
    mov word[vesa2_x],640
    mov word[vesa2_y],400
    mov byte[vesa2_bits],8
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble9
    ret
.notrouble9
    call dosmakepal
    ; clear screen (640*400 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,640*400
.loopg
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopg
    pop es
    ret

;*******************************************************
; InitVESA2 640x400x16         Set up Linear 640x400x16b
;*******************************************************

.initvesa2640x400x16
    mov byte[res640],2
    mov byte[cbitmode],1
    mov word[vesa2_x],640
    mov word[vesa2_y],400
    mov byte[vesa2_bits],16
    call InitVesa2
    cmp byte[videotroub],1
    jne .notrouble10
    ret
.notrouble10
    ; clear screen (640*400*2 bytes)
    push es
    mov ax,[vesa2selec]
    mov es,ax
    mov edi,0
    mov ecx,640*400*2
.looph
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .looph
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
.loopi
    mov byte[es:edi],0
    inc edi
    dec ecx
    jnz .loopi
    dec ebx
    jnz .loopbanks

    mov ecx,6144
    pop es
    ret
