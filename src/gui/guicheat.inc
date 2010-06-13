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


; GUI Cheat Code Routines

SECTION .text

NEWSYM decodepar
    ; convert code to number format
    mov ecx,8
    mov esi,GUICheatTextZ1
.loopb
    mov al,[esi]
    cmp al,'A'
    jb .num
    sub al,'A'
    add al,10
    jmp .let
.num
    sub al,'0'
.let
    mov [esi],al
    inc esi
    dec ecx
    jnz .loopb
    ; get address
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[GUICheatTextZ1]
    shl bl,4
    add bl,[GUICheatTextZ1+1]
    mov cl,[GUICheatTextZ1+2]
    shl cx,4
    add cl,[GUICheatTextZ1+3]
    shl cx,4
    add cl,[GUICheatTextZ1+4]
    shl cx,4
    add cl,[GUICheatTextZ1+5]
    mov al,[GUICheatTextZ1+6]
    shl al,4
    add al,[GUICheatTextZ1+7]

    ; store into cheatdata
    xor edx,edx
    mov dl,[NumCheats]
    push ebx
    mov ebx,edx
    shl edx,5
    sub edx,ebx
    sub edx,ebx
    sub edx,ebx
    sub edx,ebx
    pop ebx
    mov byte[cheatdata+edx],0
    mov [cheatdata+edx+1],al
    mov [cheatdata+edx+2],cx
    mov [cheatdata+edx+4],bl
    push eax
    mov al,[guicheatvalrep]
    or [cheatdata+edx],al
    pop eax
    push ecx
    push eax
    push ebx
    push edx
    call dword near [memtabler8+ebx*4]
    pop edx
    mov [cheatdata+edx+5],al
    pop ebx
    pop eax
    pop ecx

    mov byte[writeon],1
    test byte[cheatdata+edx],80h
    jnz .nowrite
    test byte[cheatdata+edx-28],80h
    jnz .nowrite
    call dword near [memtablew8+ebx*4]
.nowrite
    mov byte[writeon],0
    mov byte[CheatOn],1
    inc dword[NumCheats]
    mov dword[GUICheatTextZ1],0
    mov dword[GUICheatTextZ2],0
    mov byte[GUICheatPosA],0
    mov byte[GUICheatPosB],0
    mov dword[GUIcurrentcheatwin],1
    mov eax,[NumCheats]
    dec eax
    mov [GUIcurrentcheatcursloc],eax
    sub eax,11
    mov [GUIcurrentcheatviewloc],eax
    test dword[GUIcurrentcheatviewloc],80000000h
    jz .noview
    mov dword[GUIcurrentcheatviewloc],0
.noview
    ret

SECTION .bss
NEWSYM guicheatvalrep, resb 1
