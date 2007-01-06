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


SECTION .text

; GUI Cheat Code Routines

AddCSCheatCode:
    cmp byte[CSInputDisplay],'_'
    je .nodisplay
    jmp .okay
.nodisplay
    ret
.okay
    mov eax,[curentryval]
    mov [curaddrvalcs],eax
    mov eax,[CSCurValue]
    mov [curvaluecs],eax
    xor ecx,ecx
    mov cl,[CheatSrcByteSize]
    inc cl
    cmp byte[CheatUpperByteOnly],0
    je .cspardisploop
    mov cl,1
.cspardispagain
    cmp dword[curvaluecs],0FFh
    jbe .cspardisploop
    shr dword[curvaluecs],8
    inc dword[curaddrvalcs]
    jmp .cspardispagain
.cspardisploop
    push ecx
    mov esi,GUICSrcTextG1
    mov ecx,3
    mov eax,[curaddrvalcs]
    add eax,7E0000h
    mov bl,[curvaluecs]
    ; write bl at address eax
    call AddCheatCode

    shr dword[curvaluecs],8
    mov byte[GUItextcolor],223

    add dword[CheatSearchYPos],10
    inc dword[curaddrvalcs]
    pop ecx
    dec ecx
    jnz near .cspardisploop
    mov byte[CheatWinMode],2
    ret

%macro SearchCMPByte 0
    mov bl,[edi]
    cmp bl,[esi]
%endmacro

%macro SearchCMP2Bytes 0
    mov bx,[edi]
    cmp bx,[esi]
%endmacro

%macro SearchCMP3Bytes 0
    push ecx
    mov ebx,[edi]
    mov ecx,[esi]
    and ebx,0FFFFFFh
    and ecx,0FFFFFFh
    cmp ebx,ecx
    pop ecx
%endmacro

%macro SearchCMP4Bytes 0
    mov ebx,[edi]
    cmp ebx,[esi]
%endmacro

%macro SearchMacro 2
    mov dl,0FEh
%%searchloop
    %2
    %1 %%nofail
    and [eax],dl
%%nofail
    inc edi
    inc esi
    cmp dl,7Fh
    jne %%noinceax
    inc eax
%%noinceax
    rol dl,1
;    loop %%searchloop
    dec ecx
    jnz %%searchloop
%endmacro

%macro SearchMacroB 1
    cmp byte[CheatCompareValue],0
    je near %%greater
    cmp byte[CheatCompareValue],1
    je near %%lessthan
    cmp byte[CheatCompareValue],3
    je near %%notequalthan
    SearchMacro je, %1
    jmp .end
%%greater
    SearchMacro ja, %1
    jmp .end
%%lessthan
    SearchMacro jb, %1
    jmp .end
%%notequalthan
    SearchMacro jne, %1
    jmp .end
%endmacro

CheatCodeSearchProcess:
    cmp byte[CheatSrcSearchType],1
    je near .comparative
    cmp byte[CSInputDisplay],0
    je near .nodisplay
    cmp byte[CSInputDisplay],'_'
    je near .nodisplay
    cmp byte[CSOverValue],1
    je near .nodisplay
    mov byte[CSInputDisplay],'_'
    mov byte[CSInputDisplay+1],0
    ; Process Cheat Search
    mov ecx,65536*2
    xor ebx,ebx
    mov bl,[CheatSrcByteSize]
    sub ecx,ebx
    mov edx,[SrcMask+ebx*4]
    mov edi,[vidbuffer]
    add edi,129600+65536*2
    mov esi,[wramdata]
    mov bl,0FEh
    mov bh,01h
.exactloop
    mov eax,[esi]
    and eax,edx
    cmp eax,[CSCurValue]
    je .found
    inc eax
    and eax,edx
    cmp eax,[CSCurValue]
    je .foundb
.failedfind
    and [edi],bl
    jmp .foundc
.found
    test byte[edi+16384],bh
    jz .failedfind
    jmp .foundc
.foundb
    cmp byte[FirstSearch],1
    je .clear
    test byte[edi+16384],bh
    jnz .failedfind
    jmp .foundc
.clear
    and byte[edi+16384],bl
.foundc
    cmp bl,7Fh
    jne .noincedi
    inc edi
.noincedi
    rol bl,1
    rol bh,1
    inc esi
    dec ecx
    jnz .exactloop
    mov byte[CopyRamToggle],1
    mov byte[CheatSearchStatus],1
.nodisplay
    ret
.comparative
    mov byte[CSInputDisplay],'_'
    mov byte[CSInputDisplay+1],0
    mov byte[CheatSearchStatus],1
    mov eax,[vidbuffer]
    add eax,129600+65536*2
    mov esi,[vidbuffer]
    add esi,129600
    mov edi,[wramdata]
    mov ecx,65536*2
    cmp byte[CheatSrcByteSize],1
    je near .bytesize2
    cmp byte[CheatSrcByteSize],2
    je near .bytesize3
    cmp byte[CheatSrcByteSize],3
    je near .bytesize4
    SearchMacroB SearchCMPByte
.bytesize2
    dec ecx
    SearchMacroB SearchCMP2Bytes
.bytesize3
    sub ecx,2
    SearchMacroB SearchCMP3Bytes
.bytesize4
    sub ecx,3
    SearchMacroB SearchCMP4Bytes
.end
    mov byte[CopyRamToggle],1
    ret

SECTION .bss
CopyRamToggle resb 1
FirstSearch   resb 1
SECTION .text

CheatCodeSearchInit:
    mov byte[CSInputDisplay],'_'
    mov byte[CSInputDisplay+1],0
    mov byte[CheatWinMode],1
    mov byte[CheatSearchStatus],0
    mov byte[FirstSearch],1
    mov eax,[vidbuffer]
    add eax,129600
    ; copy 128k ram
    mov ebx,[wramdata]
    mov ecx,32768
.loop
    mov edx,[ebx]
    mov [eax],edx
    add ebx,4
    add eax,4
    dec ecx
    jnz .loop
    ; fill searched buffer with 0xFF
    mov eax,[vidbuffer]
    add eax,129600+65536*2
    mov ecx,8192
.loop2
    mov dword[eax],0FFFFFFFFh
    add eax,4
    dec ecx
    jnz .loop2
    cmp byte[CheatSrcSearchType],1
    jne .nottype1
    mov byte[CheatSearchStatus],1
.nottype1
    mov dword[CheatCompareValue],0
    mov eax,[vidbuffer]
    add eax,129600+65536*2
    cmp byte[CheatSrcByteSize],1
    je near .bytesize2
    cmp byte[CheatSrcByteSize],2
    je near .bytesize3
    cmp byte[CheatSrcByteSize],3
    je near .bytesize4
    ret
.bytesize2
    and byte[eax+16383],07Fh
    ret
.bytesize3
    and byte[eax+16383],03Fh
    ret
.bytesize4
    and byte[eax+16383],01Fh
    ret

NEWSYM DisableCheatsOnLoad
     ; Disable all codes
    mov esi,cheatdata
    mov ecx,[NumCheats]
    cmp ecx,0
    je .skip
.loop2
    test byte[esi],4
    jnz .disabled2
    push esi
    push ecx
    call DisableCheatCode
    pop ecx
    pop esi
.disabled2
    add esi,28
    dec ecx
    jnz .loop2
.skip
    ret

NEWSYM EnableCheatsOnLoad
    ; Enable all ON toggled cheat codes
    mov esi,cheatdata
    mov ecx,[NumCheats]
    cmp ecx,0
    je .skip2
.loop
    test byte[esi],4
    jnz .disabled
    push esi
    push ecx
    call EnableCheatCode
    pop ecx
    pop esi
.disabled
    add esi,28
    dec ecx
    jnz .loop
.skip2
    ret

CheatCodeRemove:
    mov byte[GUICBHold],0
    cmp dword[NumCheats],0
    jne .nonone
    ret
.nonone
    mov esi,[GUIcurrentcheatcursloc]
    shl esi,5
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    add esi,cheatdata
    push esi
    call DisableCheatCode
    pop esi
    mov eax,255
    sub eax,[GUIcurrentcheatcursloc]
    mov ebx,eax
    shl eax,4
    add eax,ebx
    add eax,ebx
.loop
    mov bl,[esi+28]
    mov [esi],bl
    inc esi
    dec eax
    jnz .loop
    dec dword[NumCheats]
    mov eax,[GUIcurrentcheatcursloc]
    cmp dword[NumCheats],0
    je .okay
    cmp eax,[NumCheats]
    jne .okay
    dec eax
    mov [GUIcurrentcheatcursloc],eax
    sub eax,11
    mov [GUIcurrentcheatviewloc],eax
    test dword[GUIcurrentcheatviewloc],80000000h
    jz .noview
    mov dword[GUIcurrentcheatviewloc],0
.noview
.okay
    cmp byte[NumCheats],0
    jne .cheatexists
    mov byte[CheatOn],0
.cheatexists
    ret

CheatCodeFix:
    mov byte[GUICBHold],0
    cmp dword[NumCheats],0
    jne .nonone
    ret
.nonone
    mov esi,[GUIcurrentcheatcursloc]
    shl esi,5
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    add esi,cheatdata
    push esi
    call DisableCheatCode
    pop esi
    xor byte[esi+3],80h
    call EnableCheatCodeNoPrevMod
    ret

CheatCodeToggle:
    mov byte[GUICBHold],0
    cmp dword[NumCheats],0
    jne .nonone
    ret
.nonone
    mov esi,[GUIcurrentcheatcursloc]
    shl esi,5
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    sub esi,[GUIcurrentcheatcursloc]
    add esi,cheatdata
    test byte[esi],4
    jz DisableCheatCode
    jmp EnableCheatCodeNoPrevMod

DisableCheatCode:
    ; code is at esi
    xor ecx,ecx
    xor ebx,ebx
    or byte[esi],4
    test byte[esi],1
    jnz .gfrom
    mov al,[esi+5]
    mov cx,[esi+2]
    mov bl,[esi+4]
    mov byte[writeon],1
    test byte[esi],80h
    jnz .nowrite
    test byte[esi-28],80h
    jnz .nowrite
    call dword near [memtablew8+ebx*4]
.nowrite
    mov byte[writeon],0
    ret
.gfrom
    mov al,[esi+5]
    mov ecx,[esi+2]
    and ecx,0FFFFFFh
    mov esi,[romdata]
    mov [esi+ecx],al
    ret
.gfram
    mov al,[esi+5]
    mov ecx,[esi+2]
    and ecx,0FFFFFFh
    mov esi,[sram]
    mov [esi+ecx],al
    ret

EnableCheatCode:
    ; code is at esi
    xor ecx,ecx
    xor ebx,ebx
    and byte[esi],0FBh
    test byte[esi],1
    jnz .gfrom
    mov al,[esi+1]
    mov cx,[esi+2]
    mov bl,[esi+4]
    push ecx
    push eax
    push ebx
    push edx
    call dword near [memtabler8+ebx*4]
    pop edx
    mov [esi+5],al
    pop ebx
    pop eax
    pop ecx
    mov byte[writeon],1
    test byte[esi],80h
    jnz .nowrite
    test byte[esi-28],80h
    jnz .nowrite
    call dword near [memtablew8+ebx*4]
.nowrite
    mov byte[writeon],0
    ret
.gfrom
    mov al,[esi+1]
    mov ecx,[esi+2]
    and ecx,0FFFFFFh
    mov esi,[romdata]
    mov bl,[esi+ecx]
    mov [esi+ecx],al
    mov [esi+5],bl
    ret
.gfram
    mov al,[esi+1]
    mov ecx,[esi+2]
    and ecx,0FFFFFFh
    mov esi,[sram]
    mov bl,[esi+ecx]
    mov [esi+ecx],al
    mov [esi+5],bl
    ret

EnableCheatCodeNoPrevMod:
    ; code is at esi
    xor ecx,ecx
    xor ebx,ebx
    and byte[esi],0FBh
    test byte[esi],1
    jnz .gfrom
    mov al,[esi+1]
    mov cx,[esi+2]
    mov bl,[esi+4]
    mov byte[writeon],1
    test byte[esi],80h
    jnz .nowrite
    test byte[esi-28],80h
    jnz .nowrite
    call dword near [memtablew8+ebx*4]
.nowrite
    mov byte[writeon],0
    ret
.gfrom
    mov al,[esi+1]
    mov ecx,[esi+2]
    and ecx,0FFFFFFh
    mov esi,[romdata]
    mov bl,[esi+ecx]
    mov [esi+ecx],al
    mov [esi+5],bl
    ret
.gfram
    mov al,[esi+1]
    mov ecx,[esi+2]
    and ecx,0FFFFFFh
    mov esi,[sram]
    mov bl,[esi+ecx]
    mov [esi+ecx],al
    mov [esi+5],bl
    ret

AddCheatCode:
    mov byte[GUICBHold],0
    cmp dword[NumCheats],255
    jne .okay
    ret
.okay
    push eax
    push ebx
    ; transfer description
    mov eax,[NumCheats]
    shl eax,5
    sub eax,[NumCheats]
    sub eax,[NumCheats]
    sub eax,[NumCheats]
    sub eax,[NumCheats]
    add eax,cheatdata
    mov edx,eax
    push edx
    add eax,6
    mov ebx,CSDescDisplay
    mov ecx,20
.dloop
    mov dl,[ebx]
    mov [eax+2],dl
    mov [eax+18+2],dl
    mov [eax+18*2+2],dl
    inc ebx
    inc eax
    dec ecx
    jnz .dloop
    pop edx
    pop ebx
    pop eax
    ; toggle, value, address, pvalue, name(12)
    mov byte[edx],0
    mov [edx+1],bl
    push eax
    sub eax,7E0000h
    add eax,[wramdata]
    mov bh,[eax]
    mov [eax],bl
    pop eax
    mov [edx+2],eax
    mov [edx+5],bh
    inc dword[NumCheats]
    mov edx,7
    mov al,[GUIpmenupos]
    push eax
    call CheckMenuItemHelp
    pop eax
    mov [GUIpmenupos],al
    mov byte[CheatOn],1
    ret

ProcessCheatCode:
    mov byte[GUICBHold],0
    cmp dword[NumCheats],255
    je .fail
    cmp byte[GUICheatPosA],0
    je .fail
    jmp .okay
.fail
    ret
.okay
    ; make sure flashing cursor doesn't exist
    xor ebx,ebx
    mov bl,[GUICheatPosB]
    mov byte[GUICheatTextZ2+ebx],0
    ; transfer description
    mov eax,[NumCheats]
    shl eax,5
    sub eax,[NumCheats]
    sub eax,[NumCheats]
    sub eax,[NumCheats]
    sub eax,[NumCheats]
    add eax,cheatdata
    add eax,6
    mov ebx,GUICheatTextZ2
    mov ecx,20
.dloop
    mov dl,[ebx]
    mov [eax+2],dl
    mov [eax+18+2],dl
    mov [eax+18*2+2],dl
    inc ebx
    inc eax
    dec ecx
    jnz .dloop
    ; determine whether it is gamegenie, par, or GF
    ; uppercase all codes if necessary
    mov eax,GUICheatTextZ1
    mov ecx,14
.loop
    mov bl,[eax]
    cmp bl,'a'
    jb .nolower
    cmp bl,'z'
    ja .nolower
    sub bl,'a'-'A'
    mov [eax],bl
.nolower
    inc eax
    dec ecx
    jnz .loop
    mov byte[guicheatvalrep],0
    xor eax,eax
    mov al,[GUICheatPosA]
    dec al
    cmp byte[GUICheatTextZ1+eax],'R'
    jne .notrep
    mov byte[guicheatvalrep],80h
    dec byte[GUICheatPosA]
.notrep
    cmp byte[GUICheatPosA],8
    je near .par
    cmp byte[GUICheatPosA],9
    je near .gg
    cmp byte[GUICheatPosA],14
    je near .gf
.invalidpar
    jmp guicheaterror
.par
    ; check if code is valid
    mov eax,GUICheatTextZ1
    mov ecx,8
.looppar
    mov bl,[eax]
    cmp bl,'0'
    jb .invalidpar
    cmp bl,'9'
    jbe .okaypar
    cmp bl,'A'
    jb .invalidpar
    cmp bl,'F'
    ja .invalidpar
.okaypar
    inc eax
    dec ecx
    jnz .looppar
    jmp decodepar
.invalidgg
    jmp guicheaterror
.gg
    ; check if code is valid
    mov eax,GUICheatTextZ1
    mov ecx,9
.loopgg
    cmp ecx,5
    jne .nomid
    cmp byte[eax],'-'
    jne .invalidgg
    inc eax
    dec ecx
.nomid
    mov bl,[eax]
    cmp bl,'0'
    jb .invalidgg
    cmp bl,'9'
    jbe .okaygg
    cmp bl,'A'
    jb .invalidgg
    cmp bl,'F'
    ja .invalidgg
.okaygg
    inc eax
    dec ecx
    jnz .loopgg
    jmp decodegg
    ret
.invalidgf
    jmp guicheaterror
.gf
    ; check if code is valid
    mov eax,GUICheatTextZ1
    mov ecx,5
.loopgf
    mov bl,[eax]
    cmp bl,'0'
    jb .invalidgf
    cmp bl,'9'
    jbe .okaygf
    cmp bl,'A'
    jb .invalidgf
    cmp bl,'F'
    ja .invalidgf
.okaygf
    inc eax
    dec ecx
    jnz .loopgf
    mov eax,GUICheatTextZ1+5
    mov ecx,6
.loopgf2
    mov bl,[eax]
    cmp bl,'X'
    je .okaygf2
    cmp bl,'0'
    jb .invalidgf
    cmp bl,'9'
    jbe .okaygf2
    cmp bl,'A'
    jb near .invalidgf
    cmp bl,'F'
    ja near .invalidgf
.okaygf2
    inc eax
    dec ecx
    jnz .loopgf2
    mov bl,[GUICheatTextZ1+13]
    cmp bl,'0'
    je .okaygf3
    cmp bl,'1'
    jne near .invalidgf
.okaygf3
    jmp decodegf

decodegf:
    ; convert code to number format
    mov ecx,14
    mov esi,GUICheatTextZ1
.loopb2
    mov al,[esi]
    cmp al,'X'
    je .let2
    cmp al,'A'
    jb .num2
    sub al,'A'
    add al,10
    jmp .let2
.num2
    sub al,'0'
.let2
    mov [esi],al
    inc esi
    dec ecx
    jnz .loopb2

    ; get address
    xor ebx,ebx
    xor ecx,ecx
    mov cl,[GUICheatTextZ1]
    shl ecx,4
    add cl,[GUICheatTextZ1+1]
    shl ecx,4
    add cl,[GUICheatTextZ1+2]
    shl ecx,4
    add cl,[GUICheatTextZ1+3]
    shl ecx,4
    add cl,[GUICheatTextZ1+4]

    mov edx,[NumCheats]
    shl edx,5
    sub edx,[NumCheats]
    sub edx,[NumCheats]
    sub edx,[NumCheats]
    sub edx,[NumCheats]

    ; Write data to memory
    cmp byte[GUICheatTextZ1+13],1
    je near .modifysram

    ; get data
    cmp byte[GUICheatTextZ1+5],'X'
    je near .nocode
    cmp byte[GUICheatTextZ1+6],'X'
    je near .nocode
    mov bl,[GUICheatTextZ1+5]
    shl bl,4
    add bl,[GUICheatTextZ1+6]
    mov esi,[romdata]
    mov al,[esi+ecx]
    mov [esi+ecx],bl
    mov byte[cheatdata+edx],1
    mov [cheatdata+edx+2],ecx
    mov [cheatdata+edx+1],bl
    mov [cheatdata+edx+5],al
    push eax
    mov al,[guicheatvalrep]
    or [cheatdata+edx],al
    pop eax
    inc byte[NumCheats]
    add edx,28
.nocode
    inc ecx
    cmp byte[GUICheatTextZ1+7],'X'
    je near .nocode1
    cmp byte[GUICheatTextZ1+8],'X'
    je near .nocode1
    cmp byte[NumCheats],255
    je near .nocode1
    mov bl,[GUICheatTextZ1+7]
    shl bl,4
    add bl,[GUICheatTextZ1+8]
    mov esi,[romdata]
    mov al,[esi+ecx]
    mov [esi+ecx],bl
    mov byte[cheatdata+edx],1
    mov [cheatdata+edx+2],ecx
    mov [cheatdata+edx+1],bl
    mov [cheatdata+edx+5],al
    inc byte[NumCheats]
    add edx,28
.nocode1
    inc ecx
    cmp byte[GUICheatTextZ1+9],'X'
    je near .nocode2
    cmp byte[GUICheatTextZ1+10],'X'
    je near .nocode2
    cmp byte[NumCheats],255
    je near .nocode2
    mov bl,[GUICheatTextZ1+9]
    shl bl,4
    add bl,[GUICheatTextZ1+10]
    mov esi,[romdata]
    mov al,[esi+ecx]
    mov [esi+ecx],bl
    mov byte[cheatdata+edx],1
    mov [cheatdata+edx+2],ecx
    mov [cheatdata+edx+1],bl
    mov [cheatdata+edx+5],al
    inc byte[NumCheats]
.nocode2
    jmp .quit

.modifysram
    cmp ecx,65535
    ja near .quit
    mov esi,[sram]
    ; get data
    cmp byte[GUICheatTextZ1+5],'X'
    je .nocodeb
    cmp byte[GUICheatTextZ1+6],'X'
    je .nocodeb
    mov bl,[GUICheatTextZ1+5]
    shl bl,4
    add bl,[GUICheatTextZ1+6]
    mov al,[esi+ecx]
    mov [esi+ecx],bl
    mov byte[cheatdata+edx],2
    mov [cheatdata+edx+2],ecx
    mov [cheatdata+edx+1],bl
    mov [cheatdata+edx+5],al
    inc byte[NumCheats]
    add edx,28
.nocodeb
    inc ecx
    cmp byte[GUICheatTextZ1+7],'X'
    je .nocode1b
    cmp byte[GUICheatTextZ1+8],'X'
    je .nocode1b
    cmp byte[NumCheats],255
    je .nocode1b
    cmp ecx,65535
    ja .nocode1b
    mov bl,[GUICheatTextZ1+7]
    shl bl,4
    add bl,[GUICheatTextZ1+8]
    mov al,[esi+ecx+1]
    mov [esi+ecx+1],bl
    mov byte[cheatdata+edx],2
    mov [cheatdata+edx+2],ecx
    mov [cheatdata+edx+1],bl
    mov [cheatdata+edx+5],al
    inc byte[NumCheats]
    add edx,28
.nocode1b
    inc ecx
    cmp byte[GUICheatTextZ1+9],'X'
    je .nocode2c
    cmp byte[GUICheatTextZ1+10],'X'
    je .nocode2c
    cmp byte[NumCheats],255
    je .nocode2c
    cmp ecx,65535
    ja .nocode2c
    mov bl,[GUICheatTextZ1+9]
    shl bl,4
    add bl,[GUICheatTextZ1+10]
    mov al,[esi+ecx+2]
    mov [esi+ecx+2],bl
    mov byte[cheatdata+edx],2
    mov [cheatdata+edx+2],ecx
    mov [cheatdata+edx+1],bl
    mov [cheatdata+edx+5],al
    inc byte[NumCheats]
.nocode2c

.quit
    cmp byte[NumCheats],0
    je .nocheat
    mov byte[CheatOn],1
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
.nocheat
    mov dword[GUICheatTextZ1],0
    mov dword[GUICheatTextZ2],0
    mov byte[GUICheatPosA],0
    mov byte[GUICheatPosB],0
    ret

decodegg:
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

decodepar:
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
guicheatvalrep resb 1
