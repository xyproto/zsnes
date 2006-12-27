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

EXTSYM mode7tab,winptrref,nglogicval,winlogicaval,curmosaicsz,curvidoffset
EXTSYM cwinptr,domosaic,mode7A,mode7B,mode7C,mode7D,mode7X0,mode7Y0,mode7set
EXTSYM vram,vrama,winon,xtravbuf,ngwinen,winbg1enval,BuildWindow,ngwintable
EXTSYM ngcwinptr,domosaicng,pesimpng,mode7hr,BGMA,mode7ab,mode7cd,BG1SYl,BG1SXl

%include "video/mode7.mac"

;*******************************************************
; Processes & Draws Mode 7
;*******************************************************

%macro Mode7Normal 0
    or dl,dl
    jz %%nodrawb
    mov [esi],dl
%%nodrawb
    inc esi
%endmacro

%macro Mode7Window 0
    or dl,dl
    jz %%nodrawbw
    test byte[ebp],0FFh
    jnz %%nodrawbw
    mov [esi],dl
%%nodrawbw
    inc esi
    inc ebp
%endmacro

SECTION .text

NEWSYM Makemode7Table
    xor eax,eax
.nextentry
    mov cl,al
    mov dl,ah
    and cl,07h
    and dl,07h
    shl cl,4
    shl dl,1
    inc dl
    add dl,cl
    mov [mode7tab+eax],dl
    dec ax
    jnz .nextentry
    ret

; backup mode7X0, mode7Y0, Mode7A, and Mode7B
NEWSYM drawmode7
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
    Mode7Process Mode7Normal, domosaic, 1
.nextval3
    Mode7ProcessB Mode7Normal, domosaic, 1

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

    Mode7Process Mode7Window, domosaic, 1
.nextval3w
    Mode7ProcessB Mode7Window, domosaic, 1


NEWSYM drawmode7win
    cmp byte[mode7hr+ebx],1
    je near drawmode7winhr
    ProcessBuildWindow 0
.nohr

    mov esi,[cwinptr]
    mov [winptrref],esi
    Mode7Calculate

    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    mov [pesimpng],esi
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
    Mode7Process Mode7Normal, domosaicng, 1
.nextval3
    Mode7ProcessB Mode7Normal, domosaicng, 1

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
    mov ebx,[.mode7xpos]
    mov [mode7xpos],ebx
    mov ebx,[.mode7ypos]
    mov [mode7ypos],ebx

    mov edi,[vram]
    Mode7Processngw Mode7Normal, domosaicng, 1

NEWSYM drawmode7winB
    cmp byte[mode7hr+ebx],1
    je near drawmode7winBhr
    ProcessBuildWindow 0
.nohr

    mov esi,[cwinptr]
    mov [winptrref],esi
    Mode7CalculateB

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
    Mode7Process Mode7Normal, domosaic, 1
.nextval3
    Mode7ProcessB Mode7Normal, domosaic, 1

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
    mov ebx,[.mode7xpos]
    mov [mode7xpos],ebx
    mov ebx,[.mode7ypos]
    mov [mode7ypos],ebx
    mov ebx,[.mode7xadder]
    mov [mode7xadder],ebx
    mov ebx,[.mode7yadder]
    mov [mode7yadder],ebx

    mov edi,[vram]
    Mode7Processngw Mode7Normal, domosaic, 1

NEWSYM drawmode7winhr
    ProcessBuildWindow 0

    cmp byte[ngwinen],1
    jne .notwinen
    mov byte[mode7hr+ebx],0
    jmp drawmode7win.nohr
.notwinen

    mov esi,[cwinptr]
    mov [winptrref],esi
    Mode7Calculate

    ; esi = pointer to video buffer
    mov esi,[curvidoffset]       ; esi = [vidbuffer] + curypos * 288 + 16
    mov [pesimpng],esi
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

    sar dword[.mode7xadder],1
    sar dword[.mode7yadder],1

    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3
    Mode7Processhr Mode7Normal, domosaicng, 1
.nextval3
    Mode7ProcessBhr Mode7Normal, domosaicng, 1

ALIGN32
SECTION .bss
.temp        resd 1       ; for byte move left
.temp2       resd 1       ; for byte move left
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

NEWSYM drawmode7winBhr
    ProcessBuildWindow 0

    cmp byte[ngwinen],1
    jne .notwinen
    mov byte[mode7hr+ebx],0
    jmp drawmode7winB.nohr
.notwinen

    mov esi,[cwinptr]
    mov [winptrref],esi
    Mode7CalculateB

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

    sar dword[.mode7xadder],1
    sar dword[.mode7yadder],1

    mov edi,[vram]
    cmp dword[.mode7xadder],7F0h
    jg near .nextval3
    cmp dword[.mode7xadder],-7F0h
    jl near .nextval3
    cmp dword[.mode7yadder],7F0h
    jg near .nextval3
    cmp dword[.mode7yadder],-7F0h
    jl near .nextval3
    Mode7Processhr Mode7Normal, domosaic, 1
.nextval3
    Mode7ProcessBhr Mode7Normal, domosaic, 1

ALIGN32
SECTION .bss
.temp        resd 1       ; for byte move left
.temp2       resd 1       ; for byte move left
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

;ALIGN32
NEWSYM ngwleft,       resd 1       ; for byte move left
NEWSYM ngwleftb,      resd 1       ; for byte move left
NEWSYM mode7xpos,   resd 2         ; x position
NEWSYM mode7ypos,   resd 2         ; x position
NEWSYM mode7xrpos,  resd 2         ; x position, relative
NEWSYM mode7yrpos,  resd 2         ; y position, relative
NEWSYM mode7xadder, resd 2         ; number to add for x
NEWSYM mode7yadder, resd 2         ; number to add for y
SECTION .text

NEWSYM ProcessMode7ngwin
    mov ecx,[ngcwinptr]
    mov ecx,[ecx]
    or ecx,ecx
    jz near .winb
    cmp ecx,[ngwleft]
    jae .alldisplay
    sub [ngwleft],ecx
    mov [ngwleftb],ecx
    xor ecx,ecx
    mov eax,[mode7xrpos]
    ret
.alldisplay
    mov ecx,[ngwleft]
    mov [ngwleftb],ecx
    mov dword[ngwleft],0
    xor ecx,ecx
    mov eax,[mode7xrpos]
    ret
.winb
NEWSYM ProcessMode7ngwinB
    add dword[ngcwinptr],4
    mov ecx,[ngcwinptr]
    mov ecx,[ecx]
    cmp ecx,[ngwleft]
    jae near .finishmode7
    sub [ngwleft],ecx
    or ecx,ecx
    jz .noclip
.nextvalngw
    mov eax,[mode7xadder]
    add [mode7xrpos],eax
    mov eax,[mode7yadder]
    sub [mode7yrpos],eax
    inc esi
    dec ecx
    jnz near .nextvalngw
.noclip
    add dword[ngcwinptr],4
    jmp ProcessMode7ngwin
.finishmode7
    mov dword[ngwleft],0
    mov dword[ngwleftb],0
    ret

NEWSYM ProcessMode7ngwinC
    mov ecx,[ngcwinptr]
    mov ecx,[ecx]
    or ecx,ecx
    jz near .winb
    cmp ecx,[ngwleft]
    jae .alldisplay
    sub [ngwleft],ecx
    mov [ngwleftb],ecx
    xor ecx,ecx
    mov eax,[mode7xpos]
    ret
.alldisplay
    mov ecx,[ngwleft]
    mov [ngwleftb],ecx
    mov dword[ngwleft],0
    xor ecx,ecx
    mov eax,[mode7xpos]
    ret
.winb
NEWSYM ProcessMode7ngwinD
    add dword[ngcwinptr],4
    mov ecx,[ngcwinptr]
    mov ecx,[ecx]
    cmp ecx,[ngwleft]
    jae near .finishmode7
    sub [ngwleft],ecx
    or ecx,ecx
    jz .noclip
.nextvalngw
    mov eax,[mode7xadder]
    add [mode7xpos],eax
    mov eax,[mode7yadder]
    sub [mode7ypos],eax
    inc esi
    dec ecx
    jnz near .nextvalngw
.noclip
    add dword[ngcwinptr],4
    jmp ProcessMode7ngwin
.finishmode7
    mov dword[ngwleft],0
    mov dword[ngwleftb],0
    ret

%macro newvaluepred 2
    mov dx,[%1+ebx*4+8]
    cmp dx,word[%1+ebx*4]
    je %%nodivide
    cmp byte[BGMA+ebx+2],7
    je %%mode7scaleb
%%nodivide
    movsx edx,word[%1+ebx*4+4]
    movsx ecx,word[%1+ebx*4]
    add ecx,edx
    sar ecx,1
    mov [%2],cx
    jmp %%mode7scalend
%%mode7scaleb
    mov esi,ebx
    movsx ebx,word[%1+esi*4+8]
    movsx edx,word[%1+esi*4]
    sub ebx,edx
    movsx ecx,word[%1+esi*4+4]
    sub ecx,edx
    mov eax,ecx
    imul ecx
    idiv ebx
    add ax,word[%1+esi*4]
    mov ebx,esi
    mov [%2],ax
%%mode7scalend
%endmacro

NEWSYM processmode7hires
    cmp byte[BGMA+ebx+1],7
    jne near .nogo

    push esi
    push ebx
    ; predict new values
    push eax
    push edx
    push ebx
    push esi
    newvaluepred mode7ab,mode7A
    newvaluepred mode7ab+2,mode7B
    newvaluepred mode7cd,mode7C
    newvaluepred mode7cd+2,mode7D
    pop esi
    pop ebx
    pop edx
    pop eax

    mov ecx,edx
    xor edx,edx
    mov dx,[BG1SXl+ebx*2+2]
    add edx,ecx
    shr edx,1

    mov ecx,eax
    mov eax,ebx
    inc eax
    test byte[mode7set],02h
    jz .noflip
    mov eax,255
    sub eax,ebx
.noflip
    mov [m7starty],ax
    mov ax,[BG1SYl+ebx*2+2]
    add eax,ecx

    add esi,75036
    mov [curvidoffset],esi
    call drawmode7winB
    pop ebx
    pop esi
.nogo
    ret
