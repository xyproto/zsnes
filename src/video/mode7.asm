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

EXTSYM mode7tab,winptrref,nglogicval,winlogicaval
EXTSYM curmosaicsz,curvidoffset,cwinptr,domosaic,mode7A,mode7B
EXTSYM mode7C,mode7D,mode7X0,mode7Y0,mode7set,vram,vrama,winon,xtravbuf
EXTSYM ngwinen, winbg1enval, BuildWindow, ngwintable, ngcwinptr, domosaicng
EXTSYM pesimpng
EXTSYM mode7hr
EXTSYM BGMA, mode7ab, mode7cd, BG1SYl, BG1SXl, mosenng, mosszng

%include "video/mode7.mac"






;*******************************************************
; Processes & Draws Mode 7
;*******************************************************

%macro Mode7Normal 0
    or dl,dl
    jz %%nodrawb
    mov byte[esi],dl
%%nodrawb
    inc esi
%endmacro

%macro Mode7Window 0
    or dl,dl
    jz %%nodrawbw
    test byte[ebp],0FFh
    jnz %%nodrawbw
    mov byte[esi],dl
%%nodrawbw
    inc esi
    inc ebp
%endmacro

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

;mode7tab times 65536 db 0

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
.temp        dd 0       ; for byte move left
.mode7xpos   dd 0       ; x position
.tempa2      dd 0       ; keep this blank!
.mode7xrpos  dd 0       ; x position
.tempa       dd 0       ; keep this blank!
.mode7ypos   dd 0       ; y position
.tempb2      dd 0       ; keep this blank!
.mode7yrpos  dd 0       ; y position
.tempb       dd 0       ; keep this blank!
.mode7xadder dd 0       ; number to add for x
.tempc2      dd 0       ; keep this blank!
.mode7xadd2  dd 0       ; number to add for x
.tempc       dd 0       ; keep this blank!
.mode7yadder dd 0       ; number to add for y
.tempd2      dd 0       ; keep this blank!
.mode7yadd2  dd 0       ; number to add for y
.tempd       dd 0       ; keep this blank!
.mode7ptr    dd 0       ; pointer value
.mode7xinc   dd 0       ; number to add for x
.mode7xincc  dd 0       ; range check for x
.mode7yinc   dd 0       ; number to add for y
.mode7xsloc  dd 0       ; which screen x
.mode7ysloc  dd 0       ; which screen y
.mode7xsrl   dd 0       ; which relative screen x
.mode7ysrl   dd 0       ; which relative screen y
.cxloc       dw 0       ; cx location
.cyloc       dw 0       ; cy location
.m7xaddofa   dd 0
.m7xaddof2a  dd 0
.m7yaddofa   dd 0
.m7yaddof2a  dd 0

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
.temp        dd 0       ; for byte move left
.mode7xpos   dd 0       ; x position
.tempa2      dd 0       ; keep this blank!
.mode7xrpos  dd 0       ; x position
.tempa       dd 0       ; keep this blank!
.mode7ypos   dd 0       ; y position
.tempb2      dd 0       ; keep this blank!
.mode7yrpos  dd 0       ; y position
.tempb       dd 0       ; keep this blank!
.mode7xadder dd 0       ; number to add for x
.tempc2      dd 0       ; keep this blank!
.mode7xadd2  dd 0       ; number to add for x
.tempc       dd 0       ; keep this blank!
.mode7yadder dd 0       ; number to add for y
.tempd2      dd 0       ; keep this blank!
.mode7yadd2  dd 0       ; number to add for y
.tempd       dd 0       ; keep this blank!
.mode7ptr    dd 0       ; pointer value
.mode7xinc   dd 0       ; number to add for x
.mode7xincc  dd 0       ; range check for x
.mode7yinc   dd 0       ; number to add for y
.mode7xsloc  dd 0       ; which screen x
.mode7ysloc  dd 0       ; which screen y
.mode7xsrl   dd 0       ; which relative screen x
.mode7ysrl   dd 0       ; which relative screen y
.cxloc       dw 0       ; cx location
.cyloc       dw 0       ; cy location
.m7xaddofa   dd 0
.m7xaddof2a  dd 0
.m7yaddofa   dd 0
.m7yaddof2a  dd 0

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
.temp        dd 0       ; for byte move left
.mode7xpos   dd 0       ; x position
.tempa2      dd 0       ; keep this blank!
.mode7xrpos  dd 0       ; x position
.tempa       dd 0       ; keep this blank!
.mode7ypos   dd 0       ; y position
.tempb2      dd 0       ; keep this blank!
.mode7yrpos  dd 0       ; y position
.tempb       dd 0       ; keep this blank!
.mode7xadder dd 0       ; number to add for x
.tempc2      dd 0       ; keep this blank!
.mode7xadd2  dd 0       ; number to add for x
.tempc       dd 0       ; keep this blank!
.mode7yadder dd 0       ; number to add for y
.tempd2      dd 0       ; keep this blank!
.mode7yadd2  dd 0       ; number to add for y
.tempd       dd 0       ; keep this blank!
.mode7ptr    dd 0       ; pointer value
.mode7xinc   dd 0       ; number to add for x
.mode7xincc  dd 0       ; range check for x
.mode7yinc   dd 0       ; number to add for y
.mode7xsloc  dd 0       ; which screen x
.mode7ysloc  dd 0       ; which screen y
.mode7xsrl   dd 0       ; which relative screen x
.mode7ysrl   dd 0       ; which relative screen y
.cxloc       dw 0       ; cx location
.cyloc       dw 0       ; cy location
.m7xaddofa   dd 0
.m7xaddof2a  dd 0
.m7yaddofa   dd 0
.m7yaddof2a  dd 0

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
.temp        dd 0       ; for byte move left
.temp2       dd 0       ; for byte move left
.mode7xpos   dd 0       ; x position
.tempa2      dd 0       ; keep this blank!
.mode7xrpos  dd 0       ; x position
.tempa       dd 0       ; keep this blank!
.mode7ypos   dd 0       ; y position
.tempb2      dd 0       ; keep this blank!
.mode7yrpos  dd 0       ; y position
.tempb       dd 0       ; keep this blank!
.mode7xadder dd 0       ; number to add for x
.tempc2      dd 0       ; keep this blank!
.mode7xadd2  dd 0       ; number to add for x
.tempc       dd 0       ; keep this blank!
.mode7yadder dd 0       ; number to add for y
.tempd2      dd 0       ; keep this blank!
.mode7yadd2  dd 0       ; number to add for y
.tempd       dd 0       ; keep this blank!
.mode7ptr    dd 0       ; pointer value
.mode7xinc   dd 0       ; number to add for x
.mode7xincc  dd 0       ; range check for x
.mode7yinc   dd 0       ; number to add for y
.mode7xsloc  dd 0       ; which screen x
.mode7ysloc  dd 0       ; which screen y
.mode7xsrl   dd 0       ; which relative screen x
.mode7ysrl   dd 0       ; which relative screen y
.cxloc       dw 0       ; cx location
.cyloc       dw 0       ; cy location
.m7xaddofa   dd 0
.m7xaddof2a  dd 0
.m7yaddofa   dd 0
.m7yaddof2a  dd 0

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
.temp        dd 0       ; for byte move left
.temp2       dd 0       ; for byte move left
.mode7xpos   dd 0       ; x position
.tempa2      dd 0       ; keep this blank!
.mode7xrpos  dd 0       ; x position
.tempa       dd 0       ; keep this blank!
.mode7ypos   dd 0       ; y position
.tempb2      dd 0       ; keep this blank!
.mode7yrpos  dd 0       ; y position
.tempb       dd 0       ; keep this blank!
.mode7xadder dd 0       ; number to add for x
.tempc2      dd 0       ; keep this blank!
.mode7xadd2  dd 0       ; number to add for x
.tempc       dd 0       ; keep this blank!
.mode7yadder dd 0       ; number to add for y
.tempd2      dd 0       ; keep this blank!
.mode7yadd2  dd 0       ; number to add for y
.tempd       dd 0       ; keep this blank!
.mode7ptr    dd 0       ; pointer value
.mode7xinc   dd 0       ; number to add for x
.mode7xincc  dd 0       ; range check for x
.mode7yinc   dd 0       ; number to add for y
.mode7xsloc  dd 0       ; which screen x
.mode7ysloc  dd 0       ; which screen y
.mode7xsrl   dd 0       ; which relative screen x
.mode7ysrl   dd 0       ; which relative screen y
.cxloc       dw 0       ; cx location
.cyloc       dw 0       ; cy location
.m7xaddofa   dd 0
.m7xaddof2a  dd 0
.m7yaddofa   dd 0
.m7yaddof2a  dd 0

ALIGN32
NEWSYM ngwleft,       dd 0       ; for byte move left
NEWSYM ngwleftb,      dd 0       ; for byte move left
NEWSYM mode7xpos,   dd 0,0       ; x position
NEWSYM mode7ypos,   dd 0,0       ; x position
NEWSYM mode7xrpos,  dd 0,0       ; x position, relative
NEWSYM mode7yrpos,  dd 0,0       ; y position, relative
NEWSYM mode7xadder, dd 0,0       ; number to add for x
NEWSYM mode7yadder, dd 0,0       ; number to add for y

NEWSYM ProcessMode7ngwin
    mov ecx,[ngcwinptr]
    mov ecx,[ecx]
    or ecx,ecx
    jz near .winb
    cmp ecx,[ngwleft]
    jae .alldisplay
    sub [ngwleft],ecx
    mov dword[ngwleftb],ecx
    xor ecx,ecx
    mov eax,[mode7xrpos]
    ret
.alldisplay
    mov ecx,[ngwleft]
    mov dword[ngwleftb],ecx
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
    mov dword[ngwleftb],ecx
    xor ecx,ecx
    mov eax,[mode7xpos]
    ret
.alldisplay
    mov ecx,[ngwleft]
    mov dword[ngwleftb],ecx
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
    mov dx,word[%1+ebx*4+8]
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
    mov eax,261
    sub eax,ebx
.noflip
    add ax,[BG1SYl+ebx*2+2]
    add eax,ecx

    add esi,75036
    mov [curvidoffset],esi
    call drawmode7winB
    pop ebx
    pop esi
.nogo
    ret
