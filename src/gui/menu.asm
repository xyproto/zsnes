;Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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

EXTSYM cbitmode,vidbuffer,pressed,Grab_BMP_Data,Grab_BMP_Data_8,Get_Key
EXTSYM Check_Key,ScreenShotFormat,exiter,xpb,xpc,snesmmap,memtabler8,snesmap2
EXTSYM regaccessbankr8,dmadata,initaddrl,spcPCRam,xp,curcyc,Curtableaddr
EXTSYM UpdateDPage,splitflags,execsingle,joinflags,pdh,SPCRAM

%ifndef NO_DEBUGGER
EXTSYM numinst,debuggeron
%endif

%ifndef NO_PNG
EXTSYM Grab_PNG_Data
%endif

SECTION .bss
NEWSYM MenuDisplace16, resd 1
NEWSYM SPCSave, resb 1

NEWSYM menucloc, resd 1

NEWSYM menudrawbox16b_allred, resw 1

SECTION .text

NEWSYM menudrawcursor16b
    ; draw a small red box
    mov esi,41*2+34*288*2
    add esi,[menucloc]
    add esi,[menucloc]
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,148
    mov al,9
    mov bx,[menudrawbox16b_allred]
.loop
    mov [esi],bx
    add esi,2
    dec ecx
    jnz .loop
    add esi,288*2-148*2
    dec al
    mov ecx,148
    jnz .loop
    mov al,128
    ret

NEWSYM saveimage
    mov byte[pressed+1],0
    mov byte[pressed+59],0

%ifndef NO_PNG
    cmp byte[ScreenShotFormat],1
    jne .notpng
    ccallv Grab_PNG_Data
    ret
.notpng
%endif

%ifdef __MSDOS__
    cmp byte[cbitmode],1
    je near .save16b
    ccallv Grab_BMP_Data_8
    ret
.save16b
%endif
    ccallv Grab_BMP_Data
    ret

SECTION .data
NEWSYM keyonsn, db 0
NEWSYM prbreak, db 0
SECTION .text

NEWSYM breakatsignb
    mov byte[keyonsn],0
    mov byte[prbreak],0
%ifndef NO_DEBUGGER
    cmp byte[SPCSave],1
    jne .nospcsave
    mov byte[debuggeron],1
.nospcsave
%endif

    mov byte[exiter],01h
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov bl,[xpb]
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    jmp .skiplower
.loweraddr
    cmp ax,4300h
    jb .lower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
.lower
    mov esi,[snesmap2+ebx*4]
    jmp .skiplower
.dma
    mov esi,dmadata-4300h
.skiplower
    mov [initaddrl],esi
    add esi,eax                 ; add program counter to address
    mov ebp,[spcPCRam]
    mov dl,[xp]                 ; set flags
    mov dh,[curcyc]             ; set cycles
    mov edi,[Curtableaddr]
    call UpdateDPage
    ; execute
.loopa
    call splitflags
    call execsingle
    call joinflags
    mov dh,[pdh]
%ifndef NO_DEBUGGER
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    ccall Check_Key
    test al,0FFh
    jz .skipa
    ccall Get_Key
    cmp al,27
    je .skipc
.skipa
%endif
    cmp byte[SPCRAM+6],40h
    je .skipc
    cmp byte[keyonsn],1
    jne .loopa
    jmp .noesc
.skipc
    mov byte[prbreak],1
.noesc
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    mov byte[exiter],0
%ifndef NO_DEBUGGER
    cmp byte[SPCSave],1
    jne .nospcsave2
    mov byte[debuggeron],0
.nospcsave2
%endif
    ret
