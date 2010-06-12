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

NEWSYM decodegg
;Genie Hex:    D  F  4  7  0  9  1  5  6  B  C  8  A  2  3  E
;Normal  Hex:  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
;              4  6  D  E  2  7  8  3  B  5  C  9  A  0  F  1
    ; Convert code
    mov esi,GUICheatTextZ1
    mov eax,[esi+5]
    mov [esi+4],eax
    ; convert code to number format
    mov ecx,8
    xor eax,eax
.loopb3
    mov al,[esi]
    cmp al,'A'
    jb .num3
    sub al,'A'
    add al,10
    jmp .let3
.num3
    sub al,'0'
.let3
    mov al,[.GG2Norm+eax]
    mov [esi],al
    inc esi
    dec ecx
    jnz .loopb3

    mov esi,GUICheatTextZ1
    xor ecx,ecx
    mov cl,[esi+2]
    shl ecx,4
    add cl,[esi+3]
    shl ecx,4
    add cl,[esi+4]
    shl ecx,4
    add cl,[esi+5]
    shl ecx,4
    add cl,[esi+6]
    shl ecx,4
    add cl,[esi+7]
;                        0123456789ABCDEF01234567
; 24bit encoded address: ijklqrstopabcduvwxefghmn
;                        abcdefghijklmnopqrstuvwx
;                        >8  >12 >6<10 >6  <14 <10
    mov ebx,ecx
    and ebx,000000000011110000000000b   ;abcd
    shl ebx,10
    mov eax,ecx
    and eax,000000000000000000111100b   ;efgh
    shl eax,14
    add ebx,eax
    mov eax,ecx
    and eax,111100000000000000000000b   ;ijkl
    shr eax,8
    add ebx,eax
    mov eax,ecx
    and eax,000000000000000000000011b   ;mn
    shl eax,10
    add ebx,eax
    mov eax,ecx
    and eax,000000001100000000000000b   ;op
    shr eax,6
    add ebx,eax
    mov eax,ecx
    and eax,000011110000000000000000b   ;qrst
    shr eax,12
    add ebx,eax
    mov eax,ecx
    and eax,000000000000001111000000b   ;uvwx
    shr eax,6
    add ebx,eax
    xor ecx,ecx
    mov cx,bx
    shr ebx,16
    test bl,40h
    jz .noover
;    xor cx,8000h
.noover
    mov al,[esi]
    shl al,4
    add al,[esi+1]

    ; store into cheatdata
    mov edx,[NumCheats]
    shl edx,5
    sub edx,[NumCheats]
    sub edx,[NumCheats]
    sub edx,[NumCheats]
    sub edx,[NumCheats]
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
    inc byte[NumCheats]
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
SECTION .data
.GG2Norm db 04h,06h,0Dh,0Eh,02h,07h,08h,03h,0Bh,05h,0Ch,09h,0Ah,00h,0Fh,01h
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
