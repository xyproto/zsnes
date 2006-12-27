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

EXTSYM mode7tab,winptrref
EXTSYM curmosaicsz,curvidoffset,cwinptr,domosaic,mode7A,mode7B
EXTSYM mode7C,mode7D,mode7X0,mode7Y0,mode7set,vram,vrama,winon,xtravbuf
EXTSYM ngwleft,ngwleftb,mode7xpos,mode7ypos,mode7xrpos,mode7yrpos,mode7xadder
EXTSYM mode7yadder
EXTSYM nglogicval,winlogicaval,ProcessMode7ngwinD,ProcessMode7ngwinC
EXTSYM ngwinen, winbg1enval, BuildWindow, ngwintable, ngcwinptr
EXTSYM ProcessMode7ngwin,ProcessMode7ngwinB

%include "video/mode7.mac"





;*******************************************************
; Processes & Draws Mode 7
;*******************************************************

SECTION .text

NEWSYM drawmode7extbg
    mov esi,[cwinptr]
    mov [winptrref],esi
    ; mode 7, ax = curyposition, dx = curxposition (left side)
    ; draw center map coordinates at (X0-bg1scrolx,Y0-bg1scroly) on screen
    ; center map coordinates = (X0,Y0)
    ; 1.) cx=X0-bg1scrolx, cy =Y0-ax

    mov bx,[mode7X0]
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonega
    or bx,1110000000000000b
.nonega
    mov [.cxloc],bx
    mov bx,dx
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonegb
    or bx,1110000000000000b
.nonegb
    sub [.cxloc],bx
    mov bx,ax
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonegc
    or bx,1110000000000000b
.nonegc
    mov [.cyloc],bx
    mov bx,[mode7Y0]
    and bx,0001111111111111b    ; 13 -> 16 bit signed value
    test bx,0001000000000000b
    jz .nonegd
    or bx,1110000000000000b
.nonegd
    sub word[.cyloc],bx

    ; 2.) Find position at scaled y, centered x at SCX=X0-(cy*C),SCY=Y0-(cy*D)

    movsx eax,word[mode7B]
    movsx ebx,word[.cyloc]
    imul eax,ebx
    mov [.mode7xpos],eax
    mov bx,[mode7X0]
    add [.mode7xpos+1],bx

    movsx ebx,word[.cyloc]
    movsx eax,word[mode7D]
    imul eax,ebx
    mov [.mode7ypos],eax
    mov bx,[mode7Y0]
    add [.mode7ypos+1],bx

    ; 3.) Find left scaled location : SCX=SCX-(cx*A),SCY=SCY-(cx*B)

    movsx ebx,word[.cxloc]
    movsx eax,word[mode7A]
    mov [.mode7xadder],eax
    imul eax,ebx
    neg eax
    add [.mode7xpos],eax

    movsx eax,word[mode7C]
    movsx ebx,word[.cxloc]
    neg eax
    mov [.mode7yadder],eax
    imul eax,ebx
    add [.mode7ypos],eax

    test byte[mode7set],1
    jz .nohflip
    mov eax,[.mode7xadder]
    shl eax,8
    add [.mode7xpos],eax
    neg dword[.mode7xadder]
    mov eax,[.mode7yadder]
    shl eax,8
    sub [.mode7ypos],eax
    neg dword[.mode7yadder]
.nohflip

    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16

    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
.nomosaic

    ; esi = pointer to video buffer
    ; edi = pointer to vram
    ; [.mode7xadder] = dword value to add to x value (decimal between 7 & 8bit)
    ; [.mode7yadder] = dword value to add to y value (decimal between 7 & 8bit)
    ; [.mode7xpos]   = dword value of x position, decimal between 7 & 8bit
    ; [.mode7xpos+1] = word value of x position
    ; [.mode7ypos]   = dword value of y position, decimal between 7 & 8bit
    ; [.mode7ypos+1] = word value of y position
    mov dword[.temp],256
    xor ebx,ebx
    xor edx,edx
    xor ecx,ecx
    mov dword[.mode7xadd2],800h
    mov byte[.mode7xinc],2
    mov byte[.mode7xincc],0
    test dword[.mode7xadder],80000000h
    jz .noneg
    mov dword[.mode7xadd2],-800h
    mov byte[.mode7xinc],-2
    mov byte[.mode7xincc],0FEh
.noneg
    mov dword[.mode7yadd2],800h
    mov byte[.mode7yinc],1
    test dword[.mode7yadder],80000000h
    jz .noneg2
    mov dword[.mode7yadd2],-800h
    mov byte[.mode7yinc],-1
.noneg2

    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near .drawmode7win
.domosaic
    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3

    test byte[mode7set],80h
    jnz near .norep2

    mov eax,[.mode7xpos]
    and eax,7FFh
    mov [.mode7xrpos],eax
    mov eax,[.mode7ypos]
    and eax,7FFh
    mov [.mode7yrpos],eax

    ; get tile data offset into edi
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    shl ebx,5
    shr eax,3
    and ebx,07FF8h
    shl al,1
    mov bl,al
    mov edi,[vram]
    xor ch,ch
    mov [.mode7ptr],ebx
    mov cl,[edi+ebx]
    shl ecx,7
    add edi,ecx

    mov eax,[.mode7xadder]
    mov [.m7xrpa-4],eax
    mov eax,[.mode7yadder]
    mov [.m7yrpa-4],eax
    mov al,[.mode7xinc]
    mov [.m7xin-1],al
    mov eax,[.mode7xadd2]
    mov [.m7xad2-4],eax
    mov al,[.mode7yinc]
    mov [.m7yin-1],al
    mov eax,[.mode7yadd2]
    mov [.m7yad2-4],eax
    mov eax,[.mode7xrpos]
    mov ebx,[.mode7ptr]
    jmp .nextval
    ALIGN16
.nextval
    test ah,08h
    jnz .rposoffx
.nextposx
    test byte[.mode7yrpos+1],08h
    jnz .rposoffy
.nextposy
    mov ch,ah
    mov cl,[.mode7yrpos+1]
    add eax,0
.m7xrpa
    mov dl,[mode7tab+ecx]
    sub dword[.mode7yrpos],0
.m7yrpa
    mov cl,[edi+edx]
    mov [esi+288],cl
    test cl,80h
    jnz .nodrawb
    or cl,cl
    jz .nodrawb
    mov [esi],cl
.nodrawb
    inc esi
    dec dword[.temp]
    jnz .nextval
    jmp .finishmode7

.rposoffx
    add bl,0
.m7xin
    xor ecx,ecx
    mov cl,[vrama+ebx]
    shl ecx,7
    sub eax,0
.m7xad2
    lea edi,[ecx+vrama]
    jmp .nextposx

.rposoffy
    sub bh,0
.m7yin
    and ebx,07FFFh
    xor ecx,ecx
    mov cl,[vrama+ebx]
    shl ecx,7
    add dword[.mode7yrpos],0
.m7yad2
    lea edi,[ecx+vrama]
    jmp .nextposy

.finishmode7
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret

;**********************************************************
; Mode 7, no repetition mode
;**********************************************************

.norep2
    test byte[mode7set],40h
    jnz .tilerep2
    jmp .nextvalb2
    ALIGN16
.nextvalb2
    cmp byte[.mode7ypos+2],3
    ja .offscr2
    cmp byte[.mode7xpos+2],3
    jbe near .offscr3
.offscr2
    mov eax,[.mode7xadder]
    mov ebx,[.mode7yadder]
    add [.mode7xpos],eax
    sub [.mode7ypos],ebx
    inc esi
    dec dword[.temp]
    jnz near .nextvalb2
    jmp .finishmode7
.tilerep2
.nextvalb3
    cmp byte[.mode7ypos+2],3
    ja .offscr2b
    cmp byte[.mode7xpos+2],3
    jbe .offscr3
.offscr2b
    mov ch,[.mode7xpos+1]
    mov eax,[.mode7xadder]
    mov cl,[.mode7ypos+1]
    mov ebx,[.mode7yadder]
    mov dl,[mode7tab+ecx]
    add [.mode7xpos],eax
    mov cl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2trb2
    or cl,cl
    jz .nodraw2trb2
    mov [esi],cl
.nodraw2trb2
    inc esi
    dec dword[.temp]
    jnz .nextvalb3
    jmp .finishmode7
.offscr3
    mov eax,[.mode7xpos]
    and eax,7FFh
    mov [.mode7xrpos],eax
    mov eax,[.mode7ypos]
    and eax,7FFh
    mov [.mode7yrpos],eax

    ; get tile data offset into edi
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    shl ebx,5
    shr eax,3
    and ebx,07FF8h
    shl al,1
    mov bl,al
    mov edi,[vram]
    xor ch,ch
    mov [.mode7ptr],ebx
    mov cl,[edi+ebx]
    shl ecx,7
    add edi,ecx

    mov eax,[.mode7xadder]
    mov [.m7xrpa2-4],eax
    mov eax,[.mode7yadder]
    mov [.m7yrpa2-4],eax

    jmp .nextvalr
    ALIGN16
.nodr2
    inc esi
    dec dword[.temp]
    jz .fin2
.nextvalr
    test byte[.mode7xrpos+1],08h
    jnz .rposoffxr
.nextposxr
    test byte[.mode7yrpos+1],08h
    jnz near .rposoffyr
.nextposyr
    mov cl,[.mode7yrpos+1]
    mov ch,[.mode7xrpos+1]
    add dword[.mode7xrpos],0
.m7xrpa2
    mov dl,[mode7tab+ecx]
    sub dword[.mode7yrpos],0
.m7yrpa2
    mov cl,[edi+edx]
    mov [esi+288],cl
    test cl,80h
    jnz .nodr2
    or cl,cl
    jz .nodr2
    mov [esi],cl
.nodrawbr
    inc esi
    dec dword[.temp]
    jnz .nextvalr
.fin2
    jmp .finishmode7
.rposoffxr
    mov al,[.mode7xinc]
    mov edi,[vram]
    add [.mode7ptr],al
    mov cl,[.mode7xincc]
    cmp byte[.mode7ptr],cl
    je .roff
.roffxretb
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7xadd2]
    shl ecx,7
    sub [.mode7xrpos],eax
    add edi,ecx
    jmp .nextposxr
.rposoffyr
    mov al,[.mode7yinc]
    mov edi,[vram]
    sub [.mode7ptr+1],al
    js .roff
.roffyretb
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7yadd2]
    shl ecx,7
    add [.mode7yrpos],eax
    add edi,ecx
    jmp .nextposyr
.roff
    test byte[mode7set],40h
    jnz .tilerep3
    jmp .finishmode7
.tilerep3
    and byte[.mode7yrpos+1],07h
    and byte[.mode7xrpos+1],07h
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov cl,[vrama+edx]
    sub [.mode7yrpos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodrawbr2
    or cl,cl
    jz .nodrawbr2
    mov [esi],cl
.nodrawbr2
    inc esi
    dec dword[.temp]
    jnz .tilerep3
    jmp .finishmode7

;**********************************************************
; Mode 7, old routines
;**********************************************************

.nextval3
    test byte[mode7set],80h
    jnz near .norep
    jmp .nextval2
    ALIGN16
.nextval2
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    mov cl,bl
    mov ch,al
    shl ebx,5
    shr eax,3
    mov dl,[mode7tab+ecx]
    and ebx,07FF8h
    shl al,1
    mov bl,al
    xor ch,ch
    mov cl,[edi+ebx]
    mov eax,[.mode7xadder]
    shl ecx,7
    add [.mode7xpos],eax
    add ecx,edx
    mov eax,[.mode7yadder]
    mov cl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw
    or cl,cl
    jz .nodraw
    mov [esi],cl
.nodraw
    inc esi
    dec dword[.temp]
    jnz .nextval2
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret
    ; Color repetition
.norep
    test byte[mode7set],40h
    jnz near .tilerep
    jmp .nextvalb
    ALIGN16
.nextvalb
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscr
    cmp byte[.mode7xpos+2],3
    ja near .offscr
.offscrc
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    mov cl,bl
    mov ch,al
    shl ebx,5
    shr eax,3
    mov dl,[mode7tab+ecx]
    and ebx,07FF8h
    shl al,1
    mov bl,al
    xor ch,ch
    mov cl,[edi+ebx]
    mov eax,[.mode7xadder]
    shl ecx,7
    add [.mode7xpos],eax
    add ecx,edx
    mov eax,[.mode7yadder]
    mov cl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2
    or cl,cl
    jz .nodraw2
    mov [esi],cl
.nodraw2
    inc esi
    dec dword[.temp]
    jnz near .nextvalb
    jmp .goon
.offscrb
    cmp byte[.mode7ypos+2],3
    ja .offscr
    cmp byte[.mode7xpos+2],3
    jbe near .offscrc
.offscr
    mov eax,[.mode7xadder]
    mov ebx,[.mode7yadder]
    add [.mode7xpos],eax
    sub [.mode7ypos],ebx
    inc esi
    dec dword[.temp]
    jnz .offscrb
.goon
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret

.tilerep
    jmp .nextvalbtr
    ALIGN16
.nextvalbtr
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscrtr
    cmp byte[.mode7xpos+2],3
    ja near .offscrtr
.notoffscrtr
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    mov cl,bl
    mov ch,al
    shl ebx,5
    shr eax,3
    mov dl,[mode7tab+ecx]
    and ebx,07FF8h
    shl al,1
    mov bl,al
    xor ch,ch
    mov cl,[edi+ebx]
    mov eax,[.mode7xadder]
    shl ecx,7
    add [.mode7xpos],eax
    add ecx,edx
    mov eax,[.mode7yadder]
    mov cl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2tr
    or cl,cl
    jz .nodraw2tr
    mov [esi],cl
.nodraw2tr
    inc esi
    dec dword[.temp]
    jnz near .nextvalbtr
    jmp .goon
.offscrtrb
    cmp byte[.mode7ypos+2],3
    ja .offscrtr
    cmp byte[.mode7xpos+2],3
    jbe .notoffscrtr
.offscrtr
    mov ch,[.mode7xpos+1]
    mov eax,[.mode7xadder]
    mov cl,[.mode7ypos+1]
    mov ebx,[.mode7yadder]
    mov dl,[mode7tab+ecx]
    add [.mode7xpos],eax
    mov cl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2trb
    or cl,cl
    jz .nodraw2trb
    mov [esi],cl
.nodraw2trb
    inc esi
    dec dword[.temp]
    jnz near .offscrtrb
    jmp .goon

ALIGN32
SECTION .bss
.temp        resd 1       ; for byte move left
.mode7xpos   resd 1       ; x position
.tempa2      resd 1       ; keep this blank!
.mode7xrpos  resd 1       ; x position
.tempa       resd 1       ; keep this blank!
.mode7ypos   resd 1       ; y position
.tempb2      resd 1       ; keep this blank!
.mode7yrpos  resd 1       ; y position
.tempb       resd 1       ; keep this blank!
.mode7xadder resd 1       ; number to add for x
.tempc2      resd 1       ; keep this blank!
.mode7xadd2  resd 1       ; number to add for x
.tempc       resd 1       ; keep this blank!
.mode7yadder resd 1       ; number to add for y
.tempd2      resd 1       ; keep this blank!
.mode7yadd2  resd 1       ; number to add for y
.tempd       resd 1       ; keep this blank!
.mode7ptr    resd 1       ; pointer value
.mode7xinc   resd 1       ; number to add for x
.mode7xincc  resd 1       ; range check for x
.mode7yinc   resd 1       ; number to add for y
.mode7xsloc  resd 1       ; which screen x
.mode7ysloc  resd 1       ; which screen y
.mode7xsrl   resd 1       ; which relative screen x
.mode7ysrl   resd 1       ; which relative screen y
.cxloc       resw 1       ; cx location
.cyloc       resw 1       ; cy location
SECTION .text

.drawmode7win
.domosaicw

    mov ebp,[cwinptr]
    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3w
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3w
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3w
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3w

    test byte[mode7set],80h
    jnz near .norep2w

    mov eax,[.mode7xpos]
    and eax,7FFh
    mov [.mode7xrpos],eax
    mov eax,[.mode7ypos]
    and eax,7FFh
    mov [.mode7yrpos],eax

    ; get tile data offset into edi
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    shl ebx,5
    shr eax,3
    and ebx,07FF8h
    shl al,1
    mov bl,al
    mov edi,[vram]
    xor ch,ch
    mov [.mode7ptr],ebx
    mov cl,[edi+ebx]
    shl ecx,7
    add edi,ecx
    jmp .nextvalw
    ALIGN16
.nextvalw
    test byte[.mode7xrpos+1],08h
    jnz .rposoffxw
.nextposxw
    test byte[.mode7yrpos+1],08h
    jnz near .rposoffyw
.nextposyw
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov cl,[edi+edx]
    sub [.mode7yrpos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodrawbw
    or cl,cl
    jz .nodrawbw
    test byte[ebp],0FFh
    jnz .nodrawbw
    mov [esi],cl
.nodrawbw
    inc esi
    inc ebp
    dec byte[.temp]
    jnz .nextvalw
    jmp .finishmode7w
.rposoffxw
    mov al,[.mode7xinc]
    mov edi,[vram]
    add [.mode7ptr],al
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7xadd2]
    shl ecx,7
    sub [.mode7xrpos],eax
    add edi,ecx
    jmp .nextposxw
.rposoffyw
    mov al,[.mode7yinc]
    mov edi,[vram]
    sub [.mode7ptr+1],al
    and byte[.mode7ptr+1],7Fh
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7yadd2]
    shl ecx,7
    add [.mode7yrpos],eax
    add edi,ecx
    jmp .nextposyw
.finishmode7w
    xor eax,eax
    ret

;**********************************************************
; Mode 7, no repetition mode
;**********************************************************

.norep2w
    test byte[mode7set],40h
    jnz .tilerep2w
.nextvalb2w
    cmp byte[.mode7ypos+2],3
    ja .offscr2w
    cmp byte[.mode7xpos+2],3
    jbe near .offscr3w
.offscr2w
    mov eax,[.mode7xadder]
    mov ebx,[.mode7yadder]
    add [.mode7xpos],eax
    sub [.mode7ypos],ebx
    inc esi
    inc ebp
    dec byte[.temp]
    jnz near .nextvalb2w
    jmp .finishmode7w
.tilerep2w
.nextvalb3w
    cmp byte[.mode7ypos+2],3
    ja .offscr2bw
    cmp byte[.mode7xpos+2],3
    jbe .offscr3w
.offscr2bw
    mov ch,[.mode7xpos+1]
    mov eax,[.mode7xadder]
    mov cl,[.mode7ypos+1]
    mov ebx,[.mode7yadder]
    mov dl,[mode7tab+ecx]
    add [.mode7xpos],eax
    mov cl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2trb2w
    or cl,cl
    jz .nodraw2trb2w
    test byte[ebp],0FFh
    jnz .nodraw2trb2w
    mov [esi],cl
.nodraw2trb2w
    inc esi
    inc ebp
    dec byte[.temp]
    jnz .nextvalb3w
    jmp .finishmode7w
.offscr3w
    mov eax,[.mode7xpos]
    and eax,7FFh
    mov [.mode7xrpos],eax
    mov eax,[.mode7ypos]
    and eax,7FFh
    mov [.mode7yrpos],eax

    ; get tile data offset into edi
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    shl ebx,5
    shr eax,3
    and ebx,07FF8h
    shl al,1
    mov bl,al
    mov edi,[vram]
    xor ch,ch
    mov [.mode7ptr],ebx
    mov cl,[edi+ebx]
    shl ecx,7
    add edi,ecx

.nextvalrw
    test byte[.mode7xrpos+1],08h
    jnz .rposoffxrw
.nextposxrw
    test byte[.mode7yrpos+1],08h
    jnz near .rposoffyrw
.nextposyrw
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov cl,[edi+edx]
    sub [.mode7yrpos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodrawbrw
    or cl,cl
    jz .nodrawbrw
    test byte[ebp],0FFh
    jnz .nodrawbrw
    mov [esi],cl
.nodrawbrw
    inc esi
    inc ebp
    dec byte[.temp]
    jnz .nextvalrw
    jmp .finishmode7w
.rposoffxrw
    mov al,[.mode7xinc]
    mov edi,[vram]
    add [.mode7ptr],al
    mov cl,[.mode7xincc]
    cmp byte[.mode7ptr],cl
    je .roffw
.roffxretbw
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7xadd2]
    shl ecx,7
    sub [.mode7xrpos],eax
    add edi,ecx
    jmp .nextposxrw
.rposoffyrw
    mov al,[.mode7yinc]
    mov edi,[vram]
    sub [.mode7ptr+1],al
    js .roffw
.roffyretbw
    mov ebx,[.mode7ptr]
    xor ecx,ecx
    mov cl,[edi+ebx]
    mov eax,[.mode7yadd2]
    shl ecx,7
    add [.mode7yrpos],eax
    add edi,ecx
    jmp .nextposyrw
.roffw
    test byte[mode7set],40h
    jnz .tilerep3w
    jmp .finishmode7w
.tilerep3w
    and byte[.mode7yrpos+1],07h
    and byte[.mode7xrpos+1],07h
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov cl,[vrama+edx]
    sub [.mode7yrpos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodrawbr2w
    or cl,cl
    jz .nodrawbr2w
    test byte[ebp],0FFh
    jnz .nodrawbr2w
    mov [esi],cl
.nodrawbr2w
    inc esi
    inc ebp
    dec byte[.temp]
    jnz .tilerep3w
    jmp .finishmode7w

;**********************************************************
; Mode 7, old routines
;**********************************************************

.nextval3w
    test byte[mode7set],80h
    jnz near .norepw
.nextval2w
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    mov cl,bl
    mov ch,al
    shl ebx,5
    shr eax,3
    mov dl,[mode7tab+ecx]
    and ebx,07FF8h
    shl al,1
    mov bl,al
    xor ch,ch
    mov cl,[edi+ebx]
    mov eax,[.mode7xadder]
    shl ecx,7
    add [.mode7xpos],eax
    add ecx,edx
    mov eax,[.mode7yadder]
    mov cl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodraww
    or cl,cl
    jz .nodraww
    test byte[ebp],0FFh
    jnz .nodraww
    mov [esi],cl
.nodraww
    inc esi
    inc ebp
    dec byte[.temp]
    jnz .nextval2w
    xor eax,eax
    ret
    ; Color repetition
.norepw
    test byte[mode7set],40h
    jnz near .tilerepw
.nextvalbw
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscrw
    cmp byte[.mode7xpos+2],3
    ja near .offscrw
.offscrwb
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    mov cl,bl
    mov ch,al
    shl ebx,5
    shr eax,3
    mov dl,[mode7tab+ecx]
    and ebx,07FF8h
    shl al,1
    mov bl,al
    xor ch,ch
    mov cl,[edi+ebx]
    mov eax,[.mode7xadder]
    shl ecx,7
    add [.mode7xpos],eax
    add ecx,edx
    mov eax,[.mode7yadder]
    mov cl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2w
    or cl,cl
    jz .nodraw2w
    test byte[ebp],0FFh
    jnz .nodraw2w
    mov [esi],cl
.nodraw2w
    inc esi
    inc ebp
    dec byte[.temp]
    jnz near .nextvalbw
    jmp .goonw
.offscrwc
    cmp byte[.mode7ypos+2],3
    ja .offscrw
    cmp byte[.mode7xpos+2],3
    jbe near .offscrwb
.offscrw
    mov eax,[.mode7xadder]
    mov ebx,[.mode7yadder]
    add [.mode7xpos],eax
    sub [.mode7ypos],ebx
    inc esi
    inc ebp
    dec byte[.temp]
    jnz .offscrwc
.goonw
    xor eax,eax
    ret

.tilerepw
.nextvalbtrw
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscrtrw
    cmp byte[.mode7xpos+2],3
    ja near .offscrtrw
.notoffscrtrw
    mov ebx,[.mode7ypos+1]
    mov eax,[.mode7xpos+1]
    mov cl,bl
    mov ch,al
    shl ebx,5
    shr eax,3
    mov dl,[mode7tab+ecx]
    and ebx,07FF8h
    shl al,1
    mov bl,al
    xor ch,ch
    mov cl,[edi+ebx]
    mov eax,[.mode7xadder]
    shl ecx,7
    add [.mode7xpos],eax
    add ecx,edx
    mov eax,[.mode7yadder]
    mov cl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2trw
    or cl,cl
    jz .nodraw2trw
    test byte[ebp],0FFh
    jnz .nodraw2trw
    mov [esi],cl
.nodraw2trw
    inc esi
    inc ebp
    dec byte[.temp]
    jnz near .nextvalbtrw
    jmp .goonw
.offscrtrwb
    cmp byte[.mode7ypos+2],3
    ja .offscrtrw
    cmp byte[.mode7xpos+2],3
    jbe near .notoffscrtrw
.offscrtrw
    mov ch,[.mode7xpos+1]
    mov eax,[.mode7xadder]
    mov cl,[.mode7ypos+1]
    mov ebx,[.mode7yadder]
    mov dl,[mode7tab+ecx]
    add [.mode7xpos],eax
    mov cl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288],cl
    test cl,80h
    jnz .nodraw2trbw
    or cl,cl
    jz .nodraw2trbw
    test byte[ebp],0FFh
    jnz .nodraw2trbw
    mov [esi],cl
.nodraw2trbw
    inc esi
    inc ebp
    dec byte[.temp]
    jnz near .offscrtrwb
    jmp .goonw

NEWSYM drawmode7extbg2
    mov esi,[cwinptr]
    mov [winptrref],esi
    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
.nomosaic
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near .drawwin
.domosaic
    mov ecx,256
.loop
    mov al,[esi+288]
    test al,80h
    jz .nopr2
    and al,7Fh
    mov [esi],al
.nopr2
    inc esi
    dec ecx
    jnz .loop
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret
.drawwin
    mov ebp,[cwinptr]
    mov [esi],cl
.nodrawbw
    mov ecx,256
.loop2
    mov al,[esi+288]
    test byte[ebp],0FFh
    jnz .nopr2b
    test al,80h
    jz .nopr2b
    and al,7Fh
    mov [esi],al
.nopr2b
    inc esi
    inc ebp
    dec ecx
    jnz .loop2
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret

%macro Mode7Normalng 0
    mov [esi+75036],dl
    test dl,80h
    jnz %%nodrawb
    or dl,dl
    jz %%nodrawb
    mov [esi],dl
%%nodrawb
    inc esi
%endmacro

NEWSYM drawmode7ngextbg
    ProcessBuildWindow 0

    mov esi,[cwinptr]
    mov [winptrref],esi
    Mode7Calculate

    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
.nomosaic

    ; esi = pointer to video buffer
    ; edi = pointer to vram
    ; [.mode7xadder] = dword value to add to x value (decimal between 7 & 8bit)
    ; [.mode7yadder] = dword value to add to y value (decimal between 7 & 8bit)
    ; [.mode7xpos]   = dword value of x position, decimal between 7 & 8bit
    ; [.mode7xpos+1] = word value of x position
    ; [.mode7ypos]   = dword value of y position, decimal between 7 & 8bit
    ; [.mode7ypos+1] = word value of y position
    xor ebx,ebx
    xor edx,edx
    xor ecx,ecx
    mov dword[.mode7xadd2],800h
    mov byte[.mode7xinc],2
    mov byte[.mode7xincc],0
    test dword[.mode7xadder],80000000h
    jz .noneg
    mov dword[.mode7xadd2],-800h
    mov byte[.mode7xinc],-2
    mov byte[.mode7xincc],0FEh
.noneg
    mov dword[.mode7yadd2],800h
    mov byte[.mode7yinc],1
    test dword[.mode7yadder],80000000h
    jz .noneg2
    mov dword[.mode7yadd2],-800h
    mov byte[.mode7yinc],-1
.noneg2

    cmp byte[ngwinen],1
    je near .drawmode7win

    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3
    Mode7Process Mode7Normalng, domosaic, 1
.nextval3
    Mode7ProcessB Mode7Normalng, domosaic, 1
    ret

ALIGN32
SECTION .bss
.temp        resd 1       ; for byte move left
.mode7xpos   resd 1       ; x position
.tempa2      resd 1       ; keep this blank!
.mode7xrpos  resd 1       ; x position
.tempa       resd 1       ; keep this blank!
.mode7ypos   resd 1       ; y position
.tempb2      resd 1       ; keep this blank!
.mode7yrpos  resd 1       ; y position
.tempb       resd 1       ; keep this blank!
.mode7xadder resd 1       ; number to add for x
.tempc2      resd 1       ; keep this blank!
.mode7xadd2  resd 1       ; number to add for x
.tempc       resd 1       ; keep this blank!
.mode7yadder resd 1       ; number to add for y
.tempd2      resd 1       ; keep this blank!
.mode7yadd2  resd 1       ; number to add for y
.tempd       resd 1       ; keep this blank!
.mode7ptr    resd 1       ; pointer value
.mode7xinc   resd 1       ; number to add for x
.mode7xincc  resd 1       ; range check for x
.mode7yinc   resd 1       ; number to add for y
.mode7xsloc  resd 1       ; which screen x
.mode7ysloc  resd 1       ; which screen y
.mode7xsrl   resd 1       ; which relative screen x
.mode7ysrl   resd 1       ; which relative screen y
.cxloc       resw 1       ; cx location
.cyloc       resw 1       ; cy location
.m7xaddofa   resd 1
.m7xaddof2a  resd 1
.m7yaddofa   resd 1
.m7yaddof2a  resd 1
SECTION .text


.drawmode7win
.domosaicw
    mov ebx,[.mode7xrpos]
    mov [mode7xrpos],ebx
    mov ebx,[.mode7yrpos]
    mov [mode7yrpos],ebx
    mov ebx,[.mode7xadder]
    mov [mode7xadder],ebx
    mov ebx,[.mode7yadder]
    mov [mode7yadder],ebx

    mov edi,[vram]
    Mode7Processngw Mode7Normalng, domosaic, 1

NEWSYM drawmode7ngextbg2
    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+16
    mov ecx,64
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+16
.nomosaic
    mov ecx,256
.loop
    mov al,[esi+75036]
    test al,80h
    jz .nopr2
    and al,7Fh
    mov [esi],al
.nopr2
    inc esi
    dec ecx
    jnz .loop
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic
    ret


