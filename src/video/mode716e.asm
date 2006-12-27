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

EXTSYM curmosaicsz,curvidoffset,domosaic16b,winptrref,mode7A,mode7B,mode7C
EXTSYM mode7D,mode7X0,mode7Y0,mode7set,pal16b,vram,vrama,winon,mode7tab,xtravbuf
EXTSYM cwinptr

;*******************************************************
; Processes & Draws Mode 7
;*******************************************************

SECTION .text

ALIGN16

NEWSYM drawmode716extbg
;    test byte[scaddset],1
;    jnz near drawmode7dcolor
    mov esi,[cwinptr]
    mov [winptrref],esi
    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near drawmode716bwinonextbg
.domosaic
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
    mov edi,[vram]

    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+32
    mov ecx,128
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+32
.nomosaic

    ; esi = pointer to video buffer
    ; edi = pointer to vram
    ; [.mode7xadder] = dword value to add to x value (decimal between 7 & 8bit)
    ; [.mode7yadder] = dword value to add to y value (decimal between 7 & 8bit)
    ; [.mode7xpos]   = dword value of x position, decimal between 7 & 8bit
    ; [.mode7xpos+1] = word value of x position
    ; [.mode7ypos]   = dword value of y position, decimal between 7 & 8bit
    ; [.mode7ypos+1] = word value of y position
    mov byte[.temp],0
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

.nextval
    test byte[.mode7xrpos+1],08h
    jnz .rposoffx
.nextposx
    test byte[.mode7yrpos+1],08h
    jnz near .rposoffy
.nextposy
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov dl,[edi+edx]
    sub [.mode7yrpos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodrawb
    test dl,0FFh
    jz .nodrawb
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodrawb
    add esi,2
    dec byte[.temp]
    jnz .nextval
    jmp .finishmode7
.rposoffx
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
    jmp .nextposx
.rposoffy
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
    jmp .nextposy
.finishmode7
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

;**********************************************************
; Mode 7, no repetition mode
;**********************************************************

.norep2
    test byte[mode7set],40h
    jnz .tilerep2
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
    add esi,2
    dec byte[.temp]
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
    mov dl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2trb2
    test dl,0FFh
    jz .nodraw2trb2
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2trb2
    add esi,2
    dec byte[.temp]
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

.nextvalr
    test byte[.mode7xrpos+1],08h
    jnz .rposoffxr
.nextposxr
    test byte[.mode7yrpos+1],08h
    jnz near .rposoffyr
.nextposyr
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov dl,[edi+edx]
    sub [.mode7yrpos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodrawbr
    test dl,0FFh
    jz .nodrawbr
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodrawbr
    add esi,2
    dec byte[.temp]
    jnz .nextvalr
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
    mov dl,[vrama+edx]
    sub [.mode7yrpos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodrawbr2
    test dl,0FFh
    jz .nodrawbr2
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodrawbr2
    add esi,2
    dec byte[.temp]
    jnz .tilerep3
    jmp .finishmode7

;**********************************************************
; Mode 7, old routines
;**********************************************************

.nextval3
    test byte[mode7set],80h
    jnz near .norep
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
    mov dl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw
    test dl,0FFh
    jz .nodraw
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw
    add esi,2
    dec byte[.temp]
    jnz .nextval2
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret
    ; Color repetition
.norep
    test byte[mode7set],40h
    jnz near .tilerep
.nextvalb
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscr
    cmp byte[.mode7xpos+2],3
    ja near .offscr
.offscrb
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
    mov dl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2
    test dl,0FFh
    jz .nodraw2
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2
    add esi,2
    dec byte[.temp]
    jnz near .nextvalb
    jmp .goon
.offscrc
    cmp byte[.mode7ypos+2],3
    ja .offscr
    cmp byte[.mode7xpos+2],3
    jbe near .offscrb
.offscr
    mov eax,[.mode7xadder]
    mov ebx,[.mode7yadder]
    add [.mode7xpos],eax
    sub [.mode7ypos],ebx
    add esi,2
    dec byte[.temp]
    jnz .offscrc
.goon
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret

.tilerep
.nextvalbtr
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscrtr
    cmp byte[.mode7xpos+2],3
    ja near .offscrtr
.offscrtrb
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
    mov dl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2tr
    test dl,0FFh
    jz .nodraw2tr
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2tr
    add esi,2
    dec byte[.temp]
    jnz near .nextvalbtr
    jmp .goon
.offscrtrc
    cmp byte[.mode7ypos+2],3
    ja .offscrtr
    cmp byte[.mode7xpos+2],3
    jbe near .offscrtrb
.offscrtr
    mov ch,[.mode7xpos+1]
    mov eax,[.mode7xadder]
    mov cl,[.mode7ypos+1]
    mov ebx,[.mode7yadder]
    mov dl,[mode7tab+ecx]
    add [.mode7xpos],eax
    mov dl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2trb
    test dl,0FFh
    jz .nodraw2trb
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2trb
    add esi,2
    dec byte[.temp]
    jnz near .offscrtrc
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
.cxloc       resd 1       ; cx location
.cyloc       resd 1       ; cy location
SECTION .text

NEWSYM drawmode716bwinonextbg

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

    movsx ebx,word[.cyloc]
    movsx eax,word[mode7C]
    imul eax,ebx
    neg eax
    mov [.mode7xpos],eax
    mov bx,[mode7X0]
    add [.mode7xpos+1],bx

    movsx ebx,word[.cyloc]
    movsx eax,word[mode7D]
    imul eax,ebx
;    neg ax
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

    movsx ebx,word[.cxloc]
    movsx eax,word[mode7B]
    mov [.mode7yadder],eax
    imul eax,ebx
    add [.mode7ypos],eax

    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    mov edi,[vram]

    ; esi = pointer to video buffer
    ; edi = pointer to vram
    ; [.mode7xadder] = dword value to add to x value (decimal between 7 & 8bit)
    ; [.mode7yadder] = dword value to add to y value (decimal between 7 & 8bit)
    ; [.mode7xpos]   = dword value of x position, decimal between 7 & 8bit
    ; [.mode7xpos+1] = word value of x position
    ; [.mode7ypos]   = dword value of y position, decimal between 7 & 8bit
    ; [.mode7ypos+1] = word value of y position
    mov byte[.temp],0
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

    mov edi,[vram]
    mov ebp,[cwinptr]

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

.nextval
    test byte[.mode7xrpos+1],08h
    jnz .rposoffx
.nextposx
    test byte[.mode7yrpos+1],08h
    jnz near .rposoffy
.nextposy
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov dl,[edi+edx]
    sub [.mode7yrpos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodrawb
    test dl,0FFh
    jz .nodrawb
    test byte[ebp],0FFh
    jnz .nodrawb
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodrawb
    add esi,2
    inc ebp
    dec byte[.temp]
    jnz .nextval
    jmp .finishmode7
.rposoffx
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
    jmp .nextposx
.rposoffy
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
    jmp .nextposy
.finishmode7
    xor eax,eax
    ret

;**********************************************************
; Mode 7, no repetition mode
;**********************************************************

.norep2
    test byte[mode7set],40h
    jnz .tilerep2
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
    add esi,2
    dec byte[.temp]
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
    mov dl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2trb2
    test dl,0FFh
    jz .nodraw2trb2
    test byte[ebp],0FFh
    jnz .nodraw2trb2
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2trb2
    add esi,2
    inc ebp
    dec byte[.temp]
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

.nextvalr
    test byte[.mode7xrpos+1],08h
    jnz .rposoffxr
.nextposxr
    test byte[.mode7yrpos+1],08h
    jnz near .rposoffyr
.nextposyr
    mov cl,[.mode7yrpos+1]
    mov eax,[.mode7xadder]
    mov ch,[.mode7xrpos+1]
    add [.mode7xrpos],eax
    mov dl,[mode7tab+ecx]
    mov eax,[.mode7yadder]
    mov dl,[edi+edx]
    sub [.mode7yrpos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodrawbr
    test dl,0FFh
    jz .nodrawbr
    test byte[ebp],0FFh
    jnz .nodrawbr
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodrawbr
    add esi,2
    inc ebp
    dec byte[.temp]
    jnz .nextvalr
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
    mov dl,[vrama+edx]
    sub [.mode7yrpos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodrawbr2
    test dl,0FFh
    jz .nodrawbr2
    test byte[ebp],0FFh
    jnz .nodrawbr2
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodrawbr2
    add esi,2
    inc ebp
    dec byte[.temp]
    jnz .tilerep3
    jmp .finishmode7

;**********************************************************
; Mode 7, old routines
;**********************************************************

.nextval3
    test byte[mode7set],80h
    jnz near .norep
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
    mov dl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw
    test dl,0FFh
    jz .nodraw
    test byte[ebp],0FFh
    jnz .nodraw
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw
    add esi,2
    inc ebp
    dec byte[.temp]
    jnz .nextval2
    xor eax,eax
    ret
    ; Color repetition
.norep
    test byte[mode7set],40h
    jnz near .tilerep
.nextvalb
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscr
    cmp byte[.mode7xpos+2],3
    ja near .offscr
.offscrb
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
    mov dl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2
    test dl,0FFh
    jz .nodraw2
    test byte[ebp],0FFh
    jnz .nodraw2
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2
    add esi,2
    inc ebp
    dec byte[.temp]
    jnz near .nextvalb
    jmp .goon
.offscrc
    cmp byte[.mode7ypos+2],3
    ja .offscr
    cmp byte[.mode7xpos+2],3
    jbe near .offscrb
.offscr
    mov eax,[.mode7xadder]
    mov ebx,[.mode7yadder]
    add [.mode7xpos],eax
    sub [.mode7ypos],ebx
    add esi,2
    dec byte[.temp]
    jnz .offscrc
.goon
    xor eax,eax
    ret

.tilerep
.nextvalbtr
    ; get tile # @ ([.mode7xpos],[.mode7ypos])
    ; get tile location in vram (tileloc=x*2+y*256)
    cmp byte[.mode7ypos+2],3
    ja near .offscrtr
    cmp byte[.mode7xpos+2],3
    ja near .offscrtr
.offscrtrb
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
    mov dl,[edi+ecx]
    sub [.mode7ypos],eax
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2tr
    test dl,0FFh
    jz .nodraw2tr
    test byte[ebp],0FFh
    jnz .nodraw2tr
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2tr
    add esi,2
    inc ebp
    dec byte[.temp]
    jnz near .nextvalbtr
    jmp .goon
.offscrtrc
    cmp byte[.mode7ypos+2],3
    ja near .offscrtr
    cmp byte[.mode7xpos+2],3
    jbe near .offscrtrb
.offscrtr
    mov ch,[.mode7xpos+1]
    mov eax,[.mode7xadder]
    mov cl,[.mode7ypos+1]
    mov ebx,[.mode7yadder]
    mov dl,[mode7tab+ecx]
    add [.mode7xpos],eax
    mov dl,[vrama+edx]
    sub [.mode7ypos],ebx
    mov [esi+288*2],dl
    test dl,80h
    jnz .nodraw2trb
    test dl,0FFh
    jz .nodraw2trb
    test byte[ebp],0FFh
    jnz .nodraw2trb
    mov ecx,[pal16b+edx*4]
    mov [esi],cx
.nodraw2trb
    add esi,2
    inc ebp
    dec byte[.temp]
    jnz near .offscrtrc
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
.cxloc       resd 1       ; cx location
.cyloc       resd 1       ; cy location
SECTION .text

NEWSYM drawmode716extbg2
    mov esi,[cwinptr]
    mov [winptrref],esi

    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    mov edi,[vram]
    cmp byte[curmosaicsz],1
    je .nomosaic
    mov esi,xtravbuf+32
    mov ecx,128
.clearnext
    mov dword[esi],0
    add esi,4
    dec ecx
    jnz .clearnext
    mov esi,xtravbuf+32
.nomosaic

    cmp byte[curmosaicsz],1
    jne .domosaic
    cmp byte[winon],0
    jne near .drawwin
.domosaic
    mov ecx,256
    xor eax,eax
.loop
    mov al,[esi+288*2]
    test al,80h
    jz .nopr2
    and al,7Fh
    mov ebx,[pal16b+eax*4]
    mov [esi],bx
.nopr2
    add esi,2
    dec ecx
    jnz .loop
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret
.drawwin
    mov ebp,[cwinptr]
    mov [esi],cl
.nodrawbw
    mov ecx,256
    xor eax,eax
.loop2
    mov al,[esi+288*2]
    test byte[ebp],0FFh
    jnz .nopr2b
    test al,80h
    jz .nopr2b
    and al,7Fh
    mov ebx,[pal16b+eax*4]
    mov [esi],bx
.nopr2b
    add esi,2
    inc ebp
    dec ecx
    jnz .loop2
    xor eax,eax
    mov dh,[curmosaicsz]
    cmp dh,1
    jne near domosaic16b
    ret
