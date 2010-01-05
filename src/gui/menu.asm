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

EXTSYM FPSOn,cbitmode,copyvid,OutputGraphicString16b,vidbuffer,drawhline16b
EXTSYM drawvline16b,pressed,Grab_BMP_Data,Grab_BMP_Data_8,vesa2_bpos,vesa2_clbit
EXTSYM vesa2_gpos,vesa2_rpos,Get_Key,Check_Key,ScreenShotFormat,exiter,xpb,xpc
EXTSYM snesmmap,memtabler8,snesmap2,regaccessbankr8,dmadata,initaddrl,spcPCRam
EXTSYM xp,curcyc,Curtableaddr,UpdateDPage,splitflags,execsingle,joinflags,pdh
EXTSYM SPCRAM

%ifndef NO_DEBUGGER
EXTSYM numinst,debuggeron
%endif

%ifndef NO_PNG
EXTSYM Grab_PNG_Data
%endif

SECTION .bss
NEWSYM MenuDisplace16, resd 1
NEWSYM SPCSave, resb 1

SECTION .data
NEWSYM menudrawbox_string,  db 'MISC OPTIONS',0
NEWSYM menudrawbox_stringa, db 'SAVE SNAPSHOT',0
NEWSYM menudrawbox_stringb, db 'SHOW FPS',0
NEWSYM menudrawbox_stringc, db 'HIDE FPS',0
NEWSYM menudrawbox_stringd, db 'SAVE SPC DATA',0
NEWSYM menudrawbox_stringe, db 'SOUND BUFFER DUMP',0
NEWSYM menudrawbox_stringf, db 'SNAPSHOT/INCR FRM',0
NEWSYM menudrawbox_stringg, db 'INCR FRAME ONLY',0
NEWSYM menudrawbox_stringh, db 'MOVE THIS WINDOW',0
NEWSYM menudrawbox_stringi, db 'IMAGE FORMAT: ---',0

SECTION .bss
NEWSYM menucloc, resd 1
SECTION .text

NEWSYM menudrawbox16b
    ; draw shadow behind box
    cmp byte[menu16btrans],0
    jne .noshadow
    mov byte[menu16btrans],1
    mov esi,50*2+30*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,150
    mov al,95
    mov ah,5
.loop16b2
    mov dx,[esi]
    and dx,[vesa2_clbit]
    shr dx,1
    mov [esi],dx
    add esi,2
    dec ecx
    jnz .loop16b2
    add esi,288*2-150*2
    dec al
    mov ecx,150
    jnz .loop16b2
.noshadow

    mov ax,01Fh
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov [.allred],ax
    mov ax,012h
    mov cl,[vesa2_bpos]
    shl ax,cl
    mov dx,ax
    mov ax,01h
    mov cl,[vesa2_gpos]
    shl ax,cl
    mov bx,ax
    mov ax,01h
    mov cl,[vesa2_rpos]
    shl ax,cl
    or bx,ax

    ; draw a small blue box with a white border
    mov esi,40*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,150
    mov al,95
    mov ah,5
.loop16b
    mov [esi],dx
    add esi,2
    dec ecx
    jnz .loop16b
    add esi,288*2-150*2
    dec ah
    jnz .nocolinc16b
    add dx,bx
    mov ah,5
.nocolinc16b
    dec al
    mov ecx,150
    jnz .loop16b

    ; Draw lines
    mov ax,0FFFFh
    mov esi,40*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv drawhline16b, esi, 150, eax
    mov esi,40*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv drawvline16b, esi, 95, eax
    mov esi,40*2+114*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv drawhline16b, esi, 150, eax
    mov esi,40*2+32*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv drawhline16b, esi, 150, eax
    mov esi,189*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv drawvline16b, esi, 95, eax
    call menudrawcursor16b

    mov esi,45*2+23*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_string
    mov esi,45*2+35*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_stringa
    mov esi,45*2+45*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox_stringb
    test byte[FPSOn],1
    jz .nofps
    mov edi,menudrawbox_stringc
.nofps
    ccallv OutputGraphicString16b, esi, edi
    mov esi,45*2+55*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_stringd
    mov esi,45*2+65*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_stringe
    mov esi,45*2+75*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_stringf
    mov esi,45*2+85*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_stringg
    mov esi,45*2+95*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_stringh
    mov esi,45*2+105*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    ccallv OutputGraphicString16b, esi, menudrawbox_stringi
;    mov al,[newengen]
;    mov byte[newengen],0
    ccallv copyvid
;    mov [newengen],al
    ret

SECTION .bss
.allred resw 1
.blue   resw 1
.stepb  resw 1

NEWSYM menu16btrans, resb 1

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
    mov bx,[menudrawbox16b.allred]
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
