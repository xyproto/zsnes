;Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
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


EXTSYM Get_First_Entry,Get_Next_Entry,clearmem2,GetLoadHeader

SECTION .text

; GUI File (Game) Loader

%macro GUIGetEntry 1
    mov edx,%1
    mov cx,20h
    call Get_First_Entry
    jc %%nomore
%%moreentries
    test byte[DTALoc+15h],10h
    jnz %%nofile
    inc dword[GUInumentries]
    mov esi,DTALoc+1Eh
    mov byte[edi],0
    inc edi
    mov ecx,13
%%nextchar
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    dec ecx
    jnz %%nextchar
%%nofile
    call Get_Next_Entry
    jnc %%moreentries
%%nomore
%endmacro

%macro GUIGetEntry2 1
    mov edx,%1
    mov cx,20h
    call Get_First_Entry
    jc %%nomore
%%moreentries
    test byte[DTALoc+15h],10h
    jnz %%nofile
    mov eax,[GUInumentries]
    shl eax,2
    add eax,spcRamcmp
    mov [eax],edi
    inc dword[GUInumentries]
    mov esi,DTALoc+1Eh
    mov byte[edi],0
    inc edi
%%nextchar
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    or al,al
    jnz %%nextchar
%%nofile
    call Get_Next_Entry
    jnc %%moreentries
%%nomore
%endmacro

%macro GUIGetDir2 0
    mov edx,GUIfindDIR
    mov cx,10h
    call Get_First_Entry
    jc .nomore
.moreentries
    test byte[DTALoc+15h],10h
    jz .nodir
    mov eax,[GUInumentries]
    shl eax,2
    add eax,spcRamcmp
    mov [eax],edi
    inc dword[GUInumentries]
    inc dword[GUIdirentries]
    mov esi,DTALoc+1Eh
    mov byte[edi],1
    inc edi
.nextchar
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    or al,al
    jnz .nextchar
.nodir
    call Get_Next_Entry
    jnc .moreentries
.nomore
%endmacro

%macro GUIGetDir 0
    mov edx,GUIfindDIR
    mov cx,10h
    call Get_First_Entry
    jc .nomore
.moreentries
    test byte[DTALoc+15h],10h
    jz .nodir
    inc dword[GUInumentries]
    inc dword[GUIdirentries]
    mov esi,DTALoc+1Eh
    mov byte[edi],1
    inc edi
    mov ecx,13
.nextchar
    mov al,[esi]
    mov [edi],al
    inc esi
    inc edi
    dec ecx
    jnz .nextchar
.nodir
    call Get_Next_Entry
    jnc .moreentries
.nomore
%endmacro

%macro GUIPutDrive 0
    mov byte[edi],2
    inc edi
    mov byte[edi],'['
    mov [edi+1],al
    mov byte[edi+2],':'
    mov byte[edi+3],']'
    mov byte[edi+4],0
    add edi,13
    mov byte[esi],2
    inc esi
    mov byte[esi],'['
    mov [esi+1],al
    mov byte[esi+2],':'
    mov byte[esi+3],']'
    mov byte[esi+4],0
    add esi,31
    inc dword[GUInumentries]
    inc dword[GUIdirentries]
%endmacro

%macro GUIPutDrive2 0
    mov byte[edi],2
    inc edi
    mov byte[edi],'['
    mov [edi+1],al
    mov byte[edi+2],':'
    mov byte[edi+3],']'
    mov byte[edi+4],0
    add edi,5
    inc dword[GUInumentries]
    inc dword[GUIdirentries]
%endmacro

GUIStringGreater:
  ; compares string at esi to edi and returns 1 to al if esi is >, else 0
  push esi
  cmp word[esi],'.'
  je .less
  cmp word[esi+32],'.'
  je .greater
  cmp word[esi],'..'
  je .less
  cmp word[esi+32],'..'
  je .greater
.nextchar
  cmp byte[esi],0
  je .less
  cmp byte[esi+32],0
  je .greater
  mov al,[esi]
  mov cl,[esi+32]
  cmp al,'a'
  jb .noucase1
  cmp al,'z'
  ja .noucase1
  sub al,'z'-'Z'
.noucase1
  cmp cl,'a'
  jb .noucase2
  cmp cl,'z'
  ja .noucase2
  sub cl,'z'-'Z'
.noucase2
  cmp al,cl
  jb .less
  ja .greater
  inc esi
  jmp .nextchar
.less
  mov al,0
  jmp .skip
.greater
  mov al,1
.skip
  pop esi
  ret

GUIStringGreater2:
  ; compares string at ebx to edx and returns 1 to al if esi is >, else 0
  push edx
  push ebx
  cmp word[ebx],'.'
  je .less
  cmp word[edx],'.'
  je .greater
  cmp word[ebx],'..'
  je .less
  cmp word[ebx],'..'
  je .greater
.nextchar
  cmp byte[ebx],0
  je .less
  cmp byte[edx],0
  je .greater
  mov al,[ebx]
  mov cl,[edx]
  cmp al,'a'
  jb .noucase1
  cmp al,'z'
  ja .noucase1
  sub al,'z'-'Z'
.noucase1
  cmp cl,'a'
  jb .noucase2
  cmp cl,'z'
  ja .noucase2
  sub cl,'z'-'Z'
.noucase2
  cmp al,cl
  jb .less
  ja .greater
  inc ebx
  inc edx
  jmp .nextchar
.less
  mov al,0
  jmp .skip
.greater
  mov al,1
.skip
  pop ebx
  pop edx
  ret

NEWSYM GUIloadfilename
    mov [gui_rom],eax
    pushad
    call init_rom_path_gui
    popad

    ; get LoadDrive/LoadDir & SRAMDrive/SRAMDir
    mov ebx,LoadDir
    mov edx,LoadDrive
    call Get_Dir

    ; clear memory
    mov edi,[sram]
    mov eax,0FFFFFFFFh
    mov ecx,8192
.n
    mov [edi],eax
    add edi,4
    dec ecx
    jnz .n
    pushad
    call clearmem2
    popad
    mov dword[nmiprevaddrl],0
    mov dword[nmiprevaddrh],0
    mov dword[nmirept],0
    mov dword[nmiprevline],224
    mov dword[nmistatus],0
    mov byte[spcnumread],0
    mov byte[NextLineCache],0
    mov byte[curexecstate],1

    mov byte[romloadskip],1
    ; load file
    call loadfileGUI
    cmp byte[GUIloadfailed],1
    je near .noloader

    mov byte[CheatOn],0
    mov byte[NumCheats],0
;    mov byte[CheatSrcByteSize],0
;    mov byte[CheatSrcByteBase],0
;    mov byte[CheatSrcSearchType],0
    mov byte[CheatWinMode],0
    mov byte[CheatSearchStatus],0

    pushad
    call SetupROM
    cmp byte[DisplayInfo],0
    je .noshowinfo
    call showinfogui
.noshowinfo
    call initsnes
    popad
    mov byte[sramsavedis],0
    mov eax,regsbackup
    mov ebx,sndrot
    mov ecx,3019
.rbackupl
    mov dl,[eax]
    mov [ebx],dl
    inc ebx
    inc eax
    dec ecx
    jnz .rbackupl
    cmp byte[yesoutofmemory],1
    jne .noout
    call outofmemfix
.noout
    call GUIDoReset
    call GUIQuickLoadUpdate
    cmp byte[AutoLoadCht],0
    je .nocheatload
    ; change dir to LoadDrive/LoadDir
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir
    pushad
    call CheatCodeLoad
    popad
.nocheatload
    cmp byte[AutoState],0
    je .noloader
    call LoadSecondState
.noloader
    cmp byte[GUIwinptr],1
    jne .nowinopen
    mov al,[GUIpmenupos]
    mov [GUIcmenupos],al
.nowinopen
    ; change dir to LoadDrive/LoadDir
    mov dl,[LoadDrive]
    mov ebx,LoadDir
    call Change_Dir
    ret

GUILoadData:
    mov byte[GUICBHold],0
    cmp byte[GUIcurrentfilewin],0
    je near .loadfiles
    ; Change directory
    cmp byte[GUIloadfntype],0
    jne .nottype0c
    mov eax,[GUIcurrentdircursloc]
    add eax,[GUIfileentries]
    mov eax,[spcRamcmp+eax*4]
    jmp .donetype0
.nottype0c
    mov eax,[GUIcurrentdircursloc]
    mov ebx,eax
    lea eax,[eax*8]
    add ebx,ebx
    add eax,ebx
    add ebx,ebx
    add eax,ebx
    add eax,[GUIdirStartLoc]
.donetype0
    inc eax
    cmp byte[eax+2],':'
    je .drive
    mov edx,eax
    call Change_Single_Dir
    jc .error
    call GetLoadData.a
.error
    ret
.drive
    ; Change Drive
    mov dl,[eax+1]
    sub dl,65
    call Change_Drive
    call GetLoadData.a
    ret

.loadfiles
    mov byte[LoadDuplicFound],0
    cmp byte[prevlfreeze],0
    jne near .frozen
    ; check if a duplicate exists
    mov esi,prevloadnames
.checkanother
    push esi
    mov edx,[GUIcurrentcursloc]
    shl edx,5
    add edx,[spcBuffera]
    cmp byte[GUIloadfntype],0
    jne .nottype0
    mov edx,[GUIcurrentcursloc]
    mov edx,[spcRamcmp+edx*4]
.nottype0
    inc edx
    mov cl,16
.dloop
    mov al,[edx]
    cmp [esi],al
    jne .nomatch
    inc edx
    inc esi
    dec cl
    jz .matchfound
    cmp al,0
    jne .dloop
.matchfound
    pop esi
    mov byte[LoadDuplicFound],1
    jmp .frozen
.nomatch
    pop esi
    add esi,16
    cmp esi,prevloadnames+10*16
    jne near .checkanother

    ; shift names down
    mov esi,prevloadnames+16*9-1
    mov ecx,16*9
.loop
    mov al,[esi]
    mov [esi+16],al
    dec esi
    dec edi
    dec ecx
    jnz .loop
    mov esi,prevloadfnamel+512*9-1
    mov edi,prevloaddnamel+512*9-1
    mov ecx,512*9
.loop2
    mov al,[esi]
    mov [esi+512],al
    mov al,[edi]
    mov [edi+512],al
    dec esi
    dec edi
    dec ecx
    jnz .loop2
.skip
    ; copy game name
    mov edx,[GUIcurrentcursloc]
    shl edx,5
    add edx,[spcBuffera]
    cmp byte[GUIloadfntype],0
    jne .nottype0b
    mov edx,[GUIcurrentcursloc]
    mov edx,[spcRamcmp+edx*4]
.nottype0b
    inc edx
    mov ebx,prevloadnames
    mov ecx,16
.loopfnc
    mov al,[edx]
    mov [ebx],al
    inc edx
    inc ebx
    dec ecx
    jnz .loopfnc
    ; get Drive/Dir
    mov ebx,prevloaddnamel+1
    mov edx,prevloaddnamel
    call Get_Dir

.frozen
    cmp byte[GUIloadfntype],0
    jne .nottype0b2
    mov eax,[GUIcurrentcursloc]
    mov eax,[spcRamcmp+eax*4]
    jmp .donetype0b
.nottype0b2
    mov eax,[GUIcurrentcursloc]
    mov ebx,eax
    lea eax,[eax*8]
    add ebx,ebx
    add eax,ebx
    add ebx,ebx
    add eax,ebx
    add eax,spcRamcmp
.donetype0b
    inc eax
    cmp byte[prevlfreeze],0
    jne .foundend
    cmp byte[LoadDuplicFound],0
    jne .foundend
    mov esi,eax
    mov edi,prevloadfnamel
    ; copy filename
.findnext
    mov ch,[esi]
    mov [edi],ch
    cmp ch,0
    je .foundend
    inc esi
    inc edi
    jmp .findnext
.foundend
    call GUIloadfilename
    movzx eax,byte[GUIwinptr]
    dec eax
    mov byte[GUIwinactiv+1],0
    mov byte[GUIwinorder+eax],0
    dec byte[GUIwinptr]
    ret

GetNormalEntries2:
    cmp byte[showallext],1
    je near .guishowall
    GUIGetEntry2 GUIsmcfind
    GUIGetEntry2 GUIsfcfind
    GUIGetEntry2 GUIswcfind
    GUIGetEntry2 GUIfigfind
    GUIGetEntry2 GUImgdfind
    GUIGetEntry2 GUIufofind
%ifndef __MSDOS__
    GUIGetEntry2 GUIfindGZIP
%endif
    GUIGetEntry2 GUIfind058
    GUIGetEntry2 GUIfind078
    GUIGetEntry2 GUIfindUSA
    GUIGetEntry2 GUIfindEUR
    GUIGetEntry2 GUIfindJAP
    GUIGetEntry2 GUIfindBIN
    GUIGetEntry2 GUIfindZIP
%ifndef NO_JMA
    GUIGetEntry2 GUIfindJMA
%endif
    GUIGetEntry2 GUIfind1
    jmp .guiNext
.guishowall
    GUIGetEntry2 GUIfindall
.guiNext
    ret

GetNormalEntries:
    cmp byte[showallext],1
    je near .guishowall
    GUIGetEntry GUIsmcfind
    GUIGetEntry GUIsfcfind
    GUIGetEntry GUIswcfind
    GUIGetEntry GUIfigfind
    GUIGetEntry GUImgdfind
    GUIGetEntry GUIufofind
    GUIGetEntry GUIgd3find
%ifndef __MSDOS__
    GUIGetEntry GUIfindGZIP
%endif
    GUIGetEntry GUIfind058
    GUIGetEntry GUIfind078
    GUIGetEntry GUIfindUSA
    GUIGetEntry GUIfindEUR
    GUIGetEntry GUIfindJAP
    GUIGetEntry GUIfindBIN
    GUIGetEntry GUIfindZIP
    GUIGetEntry GUIfind1
    jmp .guiNext
.guishowall
    GUIGetEntry GUIfindall
.guiNext
    ret

GUIGetEntryLFNfunc:
    push es
    push edi
    mov eax,714Eh  ; Findfirst
    mov cx,ds
    mov es,cx
    mov ch,0
    mov cl,0
    mov edi,LFNData
    mov si,0
    int 21h
    mov [LFNHandle],ax
    pop edi
    pop es
    jc near .done
.next
    mov esi,LFNData+44
    mov ecx,23
.loop
    mov al,[esi]
    cmp al,'a'
    jb .nocap
    cmp al,'z'
    ja .nocap
    sub al,'a'-'A'
.nocap
    cmp al,'%'
    jne .noperc
    cmp byte[esi+1],'0'
    jb .noperc
    cmp byte[esi+1],'F'
    ja .noperc
    cmp byte[esi+2],'0'
    jb .noperc
    cmp byte[esi+2],'F'
    ja .noperc
    mov al,[esi+2]
    sub al,'0'
    cmp al,9
    jbe .nodec
    sub al,'A'-'0'-10
.nodec
    mov ah,[esi+1]
    sub ah,'0'
    cmp ah,9
    jbe .nodecb
    sub ah,'A'-'0'-10
.nodecb
    shl ah,4
    or al,ah
    add esi,2
.noperc
    mov [edi],al
    inc edi
    inc esi
    dec ecx
    jnz near .loop
    mov byte[edi],0
    add edi,9
    push es
    push edi
    mov bx,[LFNHandle]
    mov eax, 714Fh  ; Findnext
    mov dx,ds
    mov es,dx
    mov edi,LFNData
    mov si,0
    int 21h
    pop edi
    pop es
    jnc near .next
.done
    mov eax,71A1h ;Filefind Close
    mov bx,[LFNHandle]
    int 21h
    ret

GUIGetEntryLFNfuncdir:
    mov edx,GUIfindall
    push es
    push edi
    mov eax,714Eh  ; Findfirst
    mov cx,ds
    mov es,cx
    mov ch,10h
    mov cl,10h
    mov edi,LFNData
    mov si,0
    int 21h
    mov [LFNHandle],ax
    pop edi
    pop es
    jc near .done
.next
    mov esi,LFNData+44
    mov ecx,11
.loop
    mov al,[esi]
    cmp al,'a'
    jb .nocap
    cmp al,'z'
    ja .nocap
    sub al,'a'-'A'
.nocap
    mov [edi],al
    inc edi
    inc esi
    dec ecx
    jnz .loop
    mov byte[edi],0
    add edi,21
    push es
    push edi
    mov bx,[LFNHandle]
    mov eax, 714Fh  ; Findnext
    mov dx,ds
    mov es,dx
    mov edi,LFNData
    mov si,0
    int 21h
    pop edi
    pop es
    jnc near .next
.done
    mov eax,71A1h ;Filefind Close
    mov bx,[LFNHandle]
    int 21h
    ret

%macro GUIGetEntryLFN 1
    mov edx,%1
    call GUIGetEntryLFNfunc
%endmacro

GetLoadLfn:
    call GetNormalEntries
    push edi
    mov edi,[spcBuffera]
    inc edi
    cmp byte[showallext],1
    je near .guishowall
    GUIGetEntryLFN GUIsmcfind
    GUIGetEntryLFN GUIsfcfind
    GUIGetEntryLFN GUIswcfind
    GUIGetEntryLFN GUIfigfind
    GUIGetEntryLFN GUImgdfind
    GUIGetEntryLFN GUIufofind
%ifndef __MSDOS__
    GUIGetEntryLFN GUIfindGZIP
%endif
    GUIGetEntryLFN GUIfind058
    GUIGetEntryLFN GUIfind078
    GUIGetEntryLFN GUIfindUSA
    GUIGetEntryLFN GUIfindEUR
    GUIGetEntryLFN GUIfindJAP
    GUIGetEntryLFN GUIfindBIN
    GUIGetEntryLFN GUIfindZIP
    GUIGetEntryLFN GUIfind1
    jmp .guiNext
.guishowall
    GUIGetEntryLFN GUIfindall
.guiNext
    pop edi
    ret

SECTION .bss
LFNData resb 400
LFNHandle resw 1
SECTION .text

GetLoadDirNormal:
    GUIGetDir
    push edi
    ; Copy every 14 bytes of spcRamcmp into every 32 bytes of spcBuffer
    mov ecx,[GUIdirentries]
    mov esi,[GUIdirStartLoc]
    mov edi,[GUIfileentries]
    shl edi,5
    add edi,[spcBuffera]
    cmp ecx,0
    je .none
.loop
    mov eax,[esi]
    mov [edi],eax
    mov eax,[esi+4]
    mov [edi+4],eax
    mov eax,[esi+8]
    mov [edi+8],eax
    xor ax,ax
    mov [edi+12],ax
    add esi,14
    add edi,32
    dec ecx
    jnz .loop
.none
    pop edi
    ret

GetLoadDirNormallfn:
    GUIGetDir
    push edi
    ; Copy every 14 bytes of spcRamcmp into every 32 bytes of spcBuffer
    mov edi,[GUIfileentries]
    shl edi,5
    add edi,[spcBuffera]
    inc edi
    call GUIGetEntryLFNfuncdir
    pop edi
    ret

NEWSYM GetCurDir
    ; get Drive/Dir
%ifndef __UNIXSDL__
    mov ebx,GUIcurrentdir+3
%else
    mov ebx,GUIcurrentdir
%endif
    mov edx,GUIcurrentdir
    call Get_Dir

%ifndef __UNIXSDL__
    add byte[GUIcurrentdir],65
    mov byte[GUIcurrentdir+1],':'
    mov byte[GUIcurrentdir+2],'\'
%endif

    cmp byte[GUIcurrentdir+35],0
    je .nodotend
    mov byte[GUIcurrentdir+35],'.'
    mov byte[GUIcurrentdir+36],'.'
    mov byte[GUIcurrentdir+37],'.'
    mov byte[GUIcurrentdir+38],0
.nodotend
    ; Set DTA address
    call Set_DTA_Address
    ret

GetLoadData:
    mov dword[GUIcurrentfilewin],0
.a
    call GetCurDir
    mov dword[GUInumentries],0
    mov dword[GUIcurrentviewloc],0
    mov dword[GUIcurrentcursloc],0
    mov dword[GUIcurrentdirviewloc],0
    mov dword[GUIcurrentdircursloc],0
    mov edi,spcRamcmp
%ifdef __MSDOS__
    cmp byte[GUIloadfntype],2
    jne .nowin95lfn
    clc
    mov ax,7100h
    int 21h
    jc .nowin95lfn
%endif
    mov byte[GUIloadfntype],0
.nowin95lfn
    cmp byte[GUIloadfntype],2
    je .win95lfn
    cmp byte[GUIloadfntype],1
    je .snesheader
    mov edi,[spcBuffera]
    call GetNormalEntries2
    jmp .doneheader
.snesheader
    mov byte[showallext],0
    call GetNormalEntries
    pushad
    call GetLoadHeader
    popad
    jmp .doneheader
.win95lfn
    call GetLoadLfn
.doneheader
    mov eax,[GUInumentries]
    mov [GUIfileentries],eax
    cmp dword[GUIfileentries],0
    jne .nonoentry
    mov dword[GUIcurrentfilewin],1
.nonoentry

    ; Sort List
    push edi
    ; compares string at esi to edi and returns 1 to al if esi is >, else 0
.sortagain
    mov byte[BubbleSorted],0
    mov esi,[spcBuffera]
    inc esi
    mov edi,spcRamcmp+1
    cmp dword[GUInumentries],1
    jbe near .nosort
    cmp byte[GUIloadfntype],0
    je near .sort2
    mov ecx,[GUInumentries]
    dec ecx
.nextentry
    push ecx
    ; check if esi > esi+32
    call GUIStringGreater
    ; if it is, swap!
    cmp al,0
    je near .noswap
    mov ecx,31
.nextswap
    mov al,[esi]
    mov bl,[esi+32]
    mov [esi],bl
    mov [esi+32],al
    inc esi
    dec ecx
    jnz .nextswap
    mov ecx,13
.nextswap2
    mov al,[edi]
    mov bl,[edi+14]
    mov [edi],bl
    mov [edi+14],al
    inc edi
    dec ecx
    jnz .nextswap2
    mov byte[BubbleSorted],1
    sub esi,31
    sub edi,13
.noswap
    add esi,32
    add edi,14
    pop ecx
    dec ecx
    jnz near .nextentry
.nosort
    cmp byte[BubbleSorted],1
    je near .sortagain
    jmp .donesort

.sort2
    mov esi,spcRamcmp
    mov ecx,[GUInumentries]
    dec ecx
.nextentryb
    push ecx
    mov ebx,[esi]
    mov edx,[esi+4]
    inc ebx
    inc edx
    ; check if ebx > edx
    call GUIStringGreater2
    ; if it is, swap!
    cmp al,0
    je near .noswapb
    mov eax,[esi]
    mov ebx,[esi+4]
    mov [esi],ebx
    mov [esi+4],eax
    mov byte[BubbleSorted],1
.noswapb
    add esi,4
    pop ecx
    dec ecx
    jnz near .nextentryb
.nosortb
    cmp byte[BubbleSorted],1
    je near .sortagain
.donesort
    pop edi

part2:
    mov dword[GUIdirentries],0
    mov [GUIdirStartLoc],edi
    cmp byte[GUIloadfntype],0
    jne near .loadtype0
    mov eax,[GUInumentries]
    mov [GUIdirStartLoc],eax
.loadtype0

    cmp byte[GUIloadfntype],2
    je .win95lfn
    cmp byte[GUIloadfntype],0
    je .normaldir
    call GetLoadDirNormal
    jmp .nowin95
.normaldir
    GUIGetDir2
    jmp .nowin95
.win95lfn
    call GetLoadDirNormallfn
.nowin95
    cmp dword[GUInumentries],0
    je near .nodirs
    cmp dword[GUIdirentries],0
    je near .nodirs

    ; Sort List
    ; copy from GUIdirStartLoc to spcBuffer+GUIdirentries*32
    ; compares string at esi to edi and returns 1 to al if esi is >, else 0

    push edi
    ; compares string at esi to edi and returns 1 to al if esi is >, else 0
.sortagain
    mov byte[BubbleSorted],0
    mov esi,[GUIfileentries]
    shl esi,5
    add esi,[spcBuffera]
    mov edi,[GUIdirStartLoc]
    inc esi
    inc edi

    cmp dword[GUIdirentries],1
    jbe near .nosort
    mov ecx,[GUIdirentries]
    dec ecx
    cmp byte[GUIloadfntype],0
    je near .sort2
.nextentry
    push ecx
    ; check if esi > esi+32
    call GUIStringGreater
    ; if it is, swap!
    cmp al,0
    je near .noswap
    mov ecx,31
.nextswap
    mov al,[esi]
    mov bl,[esi+32]
    mov [esi],bl
    mov [esi+32],al
    inc esi
    dec ecx
    jnz .nextswap
    mov ecx,13
.nextswap2
    mov al,[edi]
    mov bl,[edi+14]
    mov [edi],bl
    mov [edi+14],al
    inc edi
    dec ecx
    jnz .nextswap2
    mov byte[BubbleSorted],1
    sub esi,31
    sub edi,13
.noswap
    add esi,32
    add edi,14
    pop ecx
    dec ecx
    jnz near .nextentry
.nosort
    cmp byte[BubbleSorted],1
    je near .sortagain
    pop edi
    jmp .nodirs
.sort2
    mov esi,[GUIfileentries]
    shl esi,2
    add esi,spcRamcmp
.nextentryb
    push ecx
    mov ebx,[esi]
    mov edx,[esi+4]
    inc ebx
    inc edx
    ; check if ebx > edx
    call GUIStringGreater2
    ; if it is, swap!
    cmp al,0
    je near .noswapb
    mov eax,[esi]
    mov ebx,[esi+4]
    mov [esi],ebx
    mov [esi+4],eax
    mov byte[BubbleSorted],1
.noswapb
    add esi,4
    pop ecx
    dec ecx
    jnz near .nextentryb
.nosortb
    cmp byte[BubbleSorted],1
    je near .sortagain
    pop edi
.nodirs
%ifndef __UNIXSDL__
    cmp byte[GUIloadfntype],0
    je near .drive2
    mov esi,[GUInumentries]
    shl esi,5
    add esi,[spcBuffera]
    mov al,65
    mov ecx,[numdrives]
.nextdrive
    GUIPutDrive
    inc al
    dec ecx
    jnz .nextdrive
    jmp .nodrive
.drive2
    mov esi,[GUInumentries]
    shl esi,2
    add esi,spcRamcmp
    mov al,65
    mov ecx,[numdrives]
.nextdriveb
    mov [esi],edi
    GUIPutDrive2
    add esi,4
    inc al
    dec ecx
    jnz .nextdriveb
.nodrive
%endif
    ret

SECTION .bss
BubbleSorted      resb 1
GUIcurrentfilewin resd 1
GUIcurrentviewloc resd 1
GUIcurrentcursloc resd 1
GUIcurrentdirviewloc resd 1
GUIcurrentdircursloc resd 1
GUIdirStartLoc    resd 1
NEWSYM GUInumentries, resd 1
GUIdirentries resd 1
GUIfileentries resd 1

SECTION .data
%ifdef __UNIXSDL__
NEWSYM GUIsmcfind,    db '*.[sS][mM][cC]',0
NEWSYM GUIsfcfind,    db '*.[sS][fF][cC]',0
NEWSYM GUIswcfind,    db '*.[sS][wW][cC]',0
NEWSYM GUIfigfind,    db '*.[fF][iI][gG]',0
NEWSYM GUImgdfind,    db '*.[mM][gG][dD]',0
NEWSYM GUIufofind,    db '*.[uU][fF][oO]',0
NEWSYM GUIgd3find,    db '*.[gG][dD]3',0
%else
NEWSYM GUIsmcfind,    db '*.SMC',0
NEWSYM GUIsfcfind,    db '*.SFC',0
NEWSYM GUIswcfind,    db '*.SWC',0
NEWSYM GUIfigfind,    db '*.FIG',0
NEWSYM GUImgdfind,    db '*.MGD',0
NEWSYM GUIufofind,    db '*.UFO',0
NEWSYM GUIgd3find,    db '*.GD3',0
%endif
NEWSYM GUIfind058,    db '*.058',0
NEWSYM GUIfind078,    db '*.078',0
%ifdef __UNIXSDL__
NEWSYM GUIfindUSA,    db '*.[uU][sS][aA]',0
NEWSYM GUIfindEUR,    db '*.[eE][uU][rR]',0
NEWSYM GUIfindJAP,    db '*.[jJ][aA][pP]',0
NEWSYM GUIfindBIN,    db '*.[bB][iI][nN]',0
NEWSYM GUIfindZIP,    db '*.[zZ][iI][pP]',0
NEWSYM GUIfindJMA,    db '*.[jJ][mM][aA]',0
NEWSYM GUIfindGZIP,    db '*.[gG][zZ]',0
%else
NEWSYM GUIfindUSA,    db '*.USA',0
NEWSYM GUIfindEUR,    db '*.EUR',0
NEWSYM GUIfindJAP,    db '*.JAP',0
NEWSYM GUIfindBIN,    db '*.BIN',0
NEWSYM GUIfindZIP,    db '*.ZIP',0
NEWSYM GUIfindJMA,    db '*.JMA',0
NEWSYM GUIfindGZIP,    db '*.GZ',0
%endif
NEWSYM GUIfind1,      db '*.1',0
%ifdef __UNIXSDL__
GUIfindDIR    db '*',0
NEWSYM GUIfindall,    db '*',0
%else
GUIfindDIR    db '*.*',0
NEWSYM GUIfindall,    db '*.*',0
%endif
NEWSYM DTALoc, times 2048+32 db 0
