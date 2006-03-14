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



%include "macros.mac"

EXTSYM DosExit,curblank,start65816,UpdateDPage,splitflags,joinflags,delay
EXTSYM Open_File,Read_File,Create_File,Write_File,Close_File,Check_Key,Get_Key
EXTSYM LastLog,endprog,printhex,vesa2_rfull,vesa2_rtrcl,vesa2_gfull,vesa2_gtrcl
EXTSYM vesa2_bfull,vesa2_btrcl,BRRBuffer,DSPMem,ResetState,PHdspsave,printnum
EXTSYM PHspcsave,ssdatst,timeron,timincr0,timincr1,timincr2,timinl0,timinl1
EXTSYM timinl2,vram,spcnumread,Curtableaddr,statesaver,memtablew8,printhex8
EXTSYM writeon,curcyc,dmadata,execsingle,initaddrl,memtabler8,pdh,debugloadstate
EXTSYM regaccessbankr8,selcB800,snesmap2,snesmmap,ram7f,StringLength,exiter
EXTSYM CurrentCPU,SA1RegP,curypos,xa,xd,xdb,xe,xp,xpb,xpc,xs,xx,xy,SA1xpb,SA1xpc
EXTSYM SA1xa,SA1xx,SA1xy,SA1xd,SA1xdb,SA1xs,cycpbl,debugbuf,soundon,spcA,spcNZ
EXTSYM spcP,spcPCRam,SPCRAM,spcS,spcX,spcY

; debstop at regsw.asm 2118/2119

SECTION .text

NEWSYM startdebugger
    mov byte[curblank],40h
    mov byte[debuggeron],1
    mov ax,0003h
    int 10h
;    mov edx,.fname3+1
;    call Open_File
;    mov bx,ax
;    mov ecx,480h
;    mov edx,[romdata]
;    add edx,65536*13h
;    call Read_File
;    call Close_File

    mov byte[execut],0
    call startdisplay
    call debugloop
    call cleardisplay
    ; sort SDD1
;    jmp .noSDD1
;    call SDD1Sort
;.noSDD1

    pushad
    call LastLog
;    cmp dword[DecompAPtr],0
;    je near .nofile
    mov edx,.fname2+1
    call Create_File
    xor ecx,ecx
    mov bx,ax
    mov ecx,65536
    mov edx,[vram]
    EXTSYM oamram
    mov edx,oamram
    mov ecx,544

    or ecx,ecx
    jz .nofilecontents
    call Write_File
.nofilecontents
    call Close_File
.nofile
    popad

    cmp byte[execut],1
    je near start65816
;    mov al,[previdmode]
;    mov ah,0
;    int 10h
;    jmp DosExit

    jmp endprog
    jmp DosExit


    mov ax,3
    int 10h

    mov ax,[vesa2_rfull]
    call printhex
    mov ax,[vesa2_rtrcl]
    call printhex
    mov ax,[vesa2_gfull]
    call printhex
    mov ax,[vesa2_gtrcl]
    call printhex
    mov ax,[vesa2_bfull]
    call printhex
    mov ax,[vesa2_btrcl]
    call printhex

    jmp DosExit

SECTION .data
.fname2 db 9,'vram.dat',0
.fname3 db 9,'vram2.dat',0

; global variables
NEWSYM debugh,  dw 0            ; debug head
NEWSYM debugt,  dw 0            ; debug tail
NEWSYM debugv,  dw 0            ; debug view
NEWSYM debugds, db 0            ; debug disable (bit 0 = 65816, bit 1 = SPC)
NEWSYM numinst, dd 0            ; # of instructions
NEWSYM wx,      db 0
NEWSYM wy,      db 0
NEWSYM wx2,     db 0
NEWSYM wy2,     db 0
NEWSYM execut,  db 0
NEWSYM debuggeron, db 1
NEWSYM debstop, db 0
NEWSYM debstop2, db 0
NEWSYM debstop3, db 0
NEWSYM debstop4, db 0
SECTION .text

NEWSYM loadtempstuff
    ; Load stuff
    mov edx,.spcfname
    call Open_File
    mov bx,ax
;    mov ecx,64
;    mov edx,ssdatst
;    call Read_File
    ; Load SPC stuff
    mov ecx,[PHspcsave]
    mov edx,SPCRAM
    call Read_File
    ; Load DSP stuff
    mov ecx,[PHdspsave]
    mov edx,BRRBuffer
    call Read_File
    mov ecx,256
    mov edx,DSPMem
    call Read_File
    call Close_File
    pushad
    call ResetState
    popad
    ret
    mov dword[spcPCRam],0
    xor eax,eax
    mov ax,[ssdatst+37]
    mov [spcPCRam],ax
    call printnum
    mov al,[ssdatst+39]
    mov [spcA],al
    mov al,[ssdatst+40]
    mov [spcX],al
    mov al,[ssdatst+41]
    mov [spcY],al
    mov al,[ssdatst+42]
    mov [spcP],al
    mov al,[ssdatst+43]
    mov [spcS],al
    add dword[spcPCRam],SPCRAM
    ; Assemble N/Z flags into P
    mov byte[spcNZ],0
    test byte[spcP],02h
    jnz .zero
    mov byte[spcNZ],1
.zero
    test byte[spcP],80h
    jz .noneg
    or byte[spcNZ],80h
.noneg
    ; Init separate variables
    xor eax,eax
    mov al,[SPCRAM+0F1h]
    mov [timeron],al
    mov al,[SPCRAM+0FAh]
    mov [timincr0],al
    mov [timinl0],al
    mov al,[SPCRAM+0FBh]
    mov [timincr1],al
    mov [timinl1],al
    mov al,[SPCRAM+0FCh]
    mov [timincr2],al
    mov [timinl2],al
    ret

SECTION .data
.spcfname db 'temp.spc',0
SECTION .text

;*******************************************************
; Debug Loop
;*******************************************************

NEWSYM debugloop
NEWSYM debugloopa
    test byte[debugds],02h
    jnz .no65816
    call nextopcode
.no65816
    test byte[debugds],01h
    jnz .nospc
    call nextspcopcode
.nospc

NEWSYM debugloopb
    call showdd
.loopb
    mov byte[spcnumread],0
    ; wait for key
    mov ah,07h
    int 21h
    ; capitalize
    cmp al,'a'
    jb .nocap
    cmp al,'z'
    ja .nocap
    sub al,'a'-'A'
.nocap
    cmp al,0
    jne .loopc
    mov ah,07h
    int 21h
    cmp al,59
    je near .execute65816
    cmp al,62
    jne .noloadstate
    pushad
    call debugloadstate
    popad
    jmp debugloopa
.noloadstate
    cmp al,60
    je near debugsavestate
    jmp .loopd
.loopc
    cmp al,27
    je .exit
    cmp al,13
    je near .loope
    cmp al,'-'
    je near .skipopcode
    cmp al,'C'
    je near .clear
    cmp al,'M'
    je near modify
    cmp al,'B'
    je near breakpoint
    cmp al,'R'
    je near repeatbreakpoint
    cmp al,'S'
    je near SPCbreakpoint
    cmp al,'A'
    je near SPCmodify
    cmp al,'T'
    je near trace
    cmp al,'D'
    je near debugdump
    cmp al,'W'
    je near breakatsign
    cmp al,'L'
    je near breakatsignlog
    cmp al,'1'
    je .disableSPC
    cmp al,'2'
    je .disable65816
.loopd
    jmp .loopb
.execute65816
   mov byte[execut],1
.exit
    ret

.disableSPC
    xor byte[debugds],01h
    jmp debugloopa

.disable65816
    xor byte[debugds],02h
    jmp debugloopa

.clear
;    call loadtempstuff
    mov dword[numinst],0
;    mov byte[DSPDet],0
;    mov esi,fxtrace
;    mov ecx,16384
;.n
;    mov dword[esi],0
;    add esi,4
;    dec ecx
;    jnz .n
    jmp debugloopa

.skipopcode
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov bl,[xpb]
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddrso
    mov esi,[snesmmap+ebx*4]
    jmp .skiplowerso
.loweraddrso
    cmp ax,4300h
    jb .lowerso
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dmaso
.lowerso
    mov esi,[snesmap2+ebx*4]
    jmp .skiplowerso
.dmaso
    mov esi,dmadata-4300h
.skiplowerso
    mov [initaddrl],esi
    add esi,eax                 ; add program counter to address
    ; 10,30,50,70,80,90,B0,D0,F0
    cmp byte[esi],10h
    je .okay
    cmp byte[esi],30h
    je .okay
    cmp byte[esi],50h
    je .okay
    cmp byte[esi],70h
    je .okay
    cmp byte[esi],80h
    je .okay
    cmp byte[esi],90h
    je .okay
    cmp byte[esi],0B0h
    je .okay
    cmp byte[esi],0D0h
    je .okay
    cmp byte[esi],0F0h
    je .okay
    jmp .notokay
.okay
    mov word[esi],0EAEAh
.notokay
    jmp debugloopa

.loope
    mov byte[skipdebugsa1],0
    call execnextop
    mov byte[skipdebugsa1],1
    cmp byte[soundon],0
    je .nosnd
    test byte[debugds],02h
    jz .nosnd
    cmp dword[cycpbl],55
    jnb .loope
.nosnd
    jmp debugloopa

;*******************************************************
; Debug dump
;*******************************************************

NEWSYM debugdump
    ; Dump SPCRam
    mov edx,.fname
    call Create_File
    mov bx,ax
    mov ecx,65536
    mov edx,SPCRAM
    call Write_File
    call Close_File
    mov edx,.fname2
    call Create_File
    mov bx,ax
    mov ecx,256
    mov edx,DSPMem
    call Write_File
    call Close_File
    jmp debugloopb

SECTION .data
.fname db 'SPCRAM.DMP',0
.fname2 db 'DSP.DMP',0
SECTION .text

;*******************************************************
; Debug save states (debug load state ported to c)
;*******************************************************
NEWSYM debugsavestate
    pushad
    call statesaver
    popad
    jmp debugloopb

;*******************************************************
; DrawWindow          Draws a Window using WX,WY,WX2,WY2
;*******************************************************

NEWSYM drawwindow
    push es
    mov ax,[selcB800]
    mov es,ax
    xor eax,eax
    xor ebx,ebx
    xor edi,edi
    mov al,[wy]
    mov bl,160
    mul bx
    mov edi,eax
    xor eax,eax
    mov al,[wx]
    shl al,1
    add edi,eax
    mov ah,[wx2]
    shl ah,1
    sub ah,al
    mov dl,ah
    mov al,160
    sub al,dl
    mov dh,al
    mov bl,[wy]
    shr dl,1
.loopa
    mov ah,31
    xor ecx,ecx
    mov cl,dl
    ; check if first, middle or last line
    cmp bl,[wy]
    je .first
    cmp bl,[wy2]
    je .last
    ; middle
    mov al,''
    stosw
    mov al,' '
    sub cx,2
    rep stosw
    mov al,''
    stosw
    jmp .addnext
.first
    ; first
    mov al,218
    stosw
    mov al,196
    sub cx,2
    rep stosw
    mov al,191
    stosw
.addnext
    inc bl
    xor eax,eax
    mov al,dh
    add edi,eax
    jmp .loopa
.last
    ; last
    mov al,192
    stosw
    mov al,196
    sub cx,2
    rep stosw
    mov al,217
    stosw
    pop es
    ret

;*******************************************************
; GetString                                Inputs String
;*******************************************************
NEWSYM getstring
    mov edi,numstr
    mov ecx,9
    mov al,[clearchar]
.nz
    mov [edi],al
    inc edi
    dec ecx
    jnz .nz
    xor ebx,ebx
.tryinputagain
    ; input string
    mov ah,07h
    int 21h
    cmp al,27
    je near .exit
    cmp al,13
    je near .trynextentry
    cmp al,8
    je near .trybackspace
    ; capitalize
    cmp al,'a'
    jb .nocap
    cmp al,'z'
    ja .nocap
    sub al,'a'-'A'
.nocap
    cmp al,'0'
    jb .tryinputagain
    cmp al,'F'
    ja .tryinputagain
    cmp al,'9'
    jbe .okinput
    cmp byte[hexok],0
    je .tryinputagain
    cmp al,'A'
    jae .okinput
    jmp .tryinputagain
.okinput
    cmp bl,0
    jne .skipnextc
    cmp al,'0'
    je .tryinputagain
.skipnextc
    cmp bl,[maxstrchar]
    jae .tryinputagain
    mov [numstr+ebx],al
    mov edi,[charmemloc]
    mov esi,numstr
    xor eax,eax
    mov al,bl
    add esi,eax
    mov ecx,eax
    inc ecx
.okloop
    mov al,[esi]
    dec esi
    mov [es:edi],al
    sub edi,2
    dec ecx
    jnz .okloop
    inc bl
    jmp .tryinputagain
.trybackspace
    cmp bl,0
    je near .tryinputagain
    dec bl
    mov edi,[charmemloc]
    mov esi,numstr
    xor eax,eax
    mov al,bl
    add esi,eax
    mov dl,'0'
    mov [esi],dl
    dec esi
    mov ecx,eax
    cmp bl,0
    je .noloop
.okloop2
    mov al,[esi]
    dec esi
    mov [es:edi],al
    sub edi,2
    dec ecx
    jnz .okloop2
.noloop
    mov dl,[clearchar]
    mov [es:edi],dl
    jmp .tryinputagain
.trynextentry
    mov al,0
.exit
    ret

SECTION .data
NEWSYM numstr,      db '000000000'
NEWSYM maxstrchar,  db 0
NEWSYM charmemloc,  dd 0
NEWSYM clearchar,   db 0
NEWSYM hexok,       db 0
SECTION .text

;*******************************************************
; Modify                   Draws Window and Allows Input
;*******************************************************
NEWSYM modify
    mov byte[clearchar],'0'
    mov byte[hexok],1
    mov byte[wx],32
    mov byte[wx2],48
    mov byte[wy],11
    mov byte[wy2],13
    mov ecx,3
.loopa
    push ecx
    mov cx,4000
    call delay
    call drawwindow
    dec byte[wy]
    inc byte[wy2]
    sub byte[wx],4
    add byte[wx2],4
    pop ecx
    dec ecx
    jnz .loopa
    push es
    mov ax,[selcB800]
    mov es,ax
    mov edi,10*160+29*2
    mov esi,.message1
    mov ecx,22
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
    mov edi,12*160+29*2
    mov esi,.message2
    mov ecx,18
    mov ah,31
.loopc
    lodsb
    stosw
    dec ecx
    jnz .loopc
    mov edi,14*160+29*2
    mov esi,.message3
    mov ecx,18
    mov ah,31
.loopd
    lodsb
    stosw
    dec ecx
    jnz .loopd
    ; set cursor to (10,50)
    mov ah,02h
    mov bl,0
    mov dh,10
    mov dl,50
    int 10h
    mov byte[maxstrchar],6
    mov dword[charmemloc],10*160+50*2
    call getstring
    cmp al,27
    je near .exit
    ; convert to number
    mov cl,bl
    mov ch,bl
    xor edx,edx
    xor eax,eax
    xor ebx,ebx
    cmp cl,4
    jna .nextnum2
    mov cl,4
.nextnum2
    sub ch,cl
    mov bl,ch
    cmp cl,0
    je .skipconv2
.nextnum
    shl dx,4
    mov al,[numstr+ebx]
    inc ebx
    call converthex2num
    add dx,ax
    dec cl
    jnz .nextnum
.skipconv2
    ; if ch = 1, then [numstr] is the value
    ; if ch = 2, then [numstr] SHL 4+[numstr+1] is the value
    xor al,al
    cmp ch,1
    jne .check2
    mov al,[numstr]
    call converthex2num
    jmp .endcheck
.check2
    cmp ch,2
    jne .endcheck
    mov al,[numstr]
    call converthex2num
    shl al,4
    mov ah,al
    mov al,[numstr+1]
    call converthex2num
    add al,ah
.endcheck
    xor ecx,ecx
    xor ebx,ebx
    mov bl,al
    mov cx,dx
    mov [.value],cx
    mov [.bank],bl
    call dword near [memtabler8+ebx*4]
    ; set cursor to (12,45)
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,45
    int 10h
    call printhex8
    mov ah,02h
    mov bl,0
    mov dh,14
    mov dl,46
    int 10h
    mov byte[maxstrchar],2
    mov dword[charmemloc],14*160+46*2
    call getstring
    cmp al,27
    je near .exit
    xor al,al
    cmp bl,1
    jne .checktwo
    mov al,[numstr]
    call converthex2num
    jmp .endconv
.checktwo
    cmp bl,2
    jne .endconv
    mov al,[numstr]
    call converthex2num
    shl al,4
    mov ah,al
    mov al,[numstr+1]
    call converthex2num
    add al,ah
.endconv
    xor ebx,ebx
    xor ecx,ecx
    mov cx,[.value]
    mov bl,[.bank]
    mov byte[writeon],1
    call dword near [memtablew8+ebx*4]
    mov byte[writeon],0
.exit
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
;    cmp byte[sfxdebugon],1
;    je .sfx
    jmp debugloopb
.sfx
;    jmp dosfxdebug.keyboardloop

SECTION .data
.message1 db 'Enter Address : 000000'
.message2 db 'Previous Value: 00'
.message3 db 'Enter Value   : 00'
.value dw 0
.bank  db 0
SECTION .text

;*******************************************************
; SPCModify                Draws Window and Allows Input
;*******************************************************

NEWSYM SPCmodify
    mov byte[clearchar],'0'
    mov byte[hexok],1
    mov byte[wx],32
    mov byte[wx2],48
    mov byte[wy],11
    mov byte[wy2],13
    mov ecx,3
.loopa
    push ecx
    mov cx,4000
    call delay
    call drawwindow
    dec byte[wy]
    inc byte[wy2]
    sub byte[wx],4
    add byte[wx2],4
    pop ecx
    dec ecx
    jnz .loopa
    push es
    mov ax,[selcB800]
    mov es,ax
    mov edi,10*160+30*2
    mov esi,.message1
    mov ecx,20
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
    mov edi,12*160+30*2
    mov esi,.message2
    mov ecx,18
    mov ah,31
.loopc
    lodsb
    stosw
    dec ecx
    jnz .loopc
    mov edi,14*160+30*2
    mov esi,.message3
    mov ecx,18
    mov ah,31
.loopd
    lodsb
    stosw
    dec ecx
    jnz .loopd
    ; set cursor to (10,50)
    mov ah,02h
    mov bl,0
    mov dh,10
    mov dl,49
    int 10h
    mov byte[maxstrchar],4
    mov dword[charmemloc],10*160+49*2
    call getstring
    cmp al,27
    je near .exit
    ; convert to number
    mov cl,bl
    mov ch,bl
    xor edx,edx
    xor eax,eax
    xor ebx,ebx
    cmp cl,4
    jna .nextnum2
    mov cl,4
.nextnum2
    sub ch,cl
    mov bl,ch
    cmp cl,0
    je .skipconv2
.nextnum
    shl dx,4
    mov al,[numstr+ebx]
    inc ebx
    call converthex2num
    add dx,ax
    dec cl
    jnz .nextnum
.skipconv2
    ; if ch = 1, then [numstr] is the value
    ; if ch = 2, then [numstr] SHL 4+[numstr+1] is the value
    xor al,al
    cmp ch,1
    jne .check2
    mov al,[numstr]
    call converthex2num
    jmp .endcheck
.check2
    cmp ch,2
    jne .endcheck
    mov al,[numstr]
    call converthex2num
    shl al,4
    mov ah,al
    mov al,[numstr+1]
    call converthex2num
    add al,ah
.endcheck
    xor ecx,ecx
    xor ebx,ebx
    mov cx,dx
    mov bx,dx
    add ebx,SPCRAM
    mov [.value],cx
    mov al,[ebx]
    ; set cursor to (12,46)
    xor bh,bh
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,46
    int 10h
    call printhex8
    mov ah,02h
    mov bl,0
    mov dh,14
    mov dl,47
    int 10h
    mov byte[maxstrchar],2
    mov dword[charmemloc],14*160+47*2
    call getstring
    cmp al,27
    je near .exit
    xor al,al
    cmp bl,1
    jne .checktwo
    mov al,[numstr]
    call converthex2num
    jmp .endconv
.checktwo
    cmp bl,2
    jne .endconv
    mov al,[numstr]
    call converthex2num
    shl al,4
    mov ah,al
    mov al,[numstr+1]
    call converthex2num
    add al,ah
.endconv
    xor ebx,ebx
    xor ecx,ecx
    mov bx,[.value]
    add ebx,SPCRAM
    mov [ebx],al
.exit
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    jmp debugloopb

SECTION .data
.message1 db 'Enter Address : 0000'
.message2 db 'Previous Value: 00'
.message3 db 'Enter Value   : 00'
.value dw 0
.bank  db 0
SECTION .text

;*******************************************************
; SPCBreakPoint        Draws Window and Input Breakpoint
;*******************************************************

NEWSYM SPCbreakpoint
    mov byte[clearchar],'0'
    mov byte[hexok],1
    mov byte[wx],24
    mov byte[wx2],56
    mov byte[wy],11
    mov byte[wy2],13
    mov cx,4000
    call delay
    call drawwindow
    push es
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+30*2
    mov esi,.message1
    mov ecx,20
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
    ; set cursor to (10,50)
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,49
    int 10h
    mov byte[maxstrchar],4
    mov dword[charmemloc],12*160+49*2
    xor ebx,ebx
    call getstring
    cmp al,27
    je near .exit
    ; convert to number
    mov cl,bl
    mov ch,bl
    xor edx,edx
    xor eax,eax
    xor ebx,ebx
    cmp cl,4
    jna .nextnum2
    mov cl,4
.nextnum2
    sub ch,cl
    mov bl,ch
    cmp cl,0
    je .skipconv2
.nextnum
    shl dx,4
    mov al,[numstr+ebx]
    inc ebx
    call converthex2num
    add dx,ax
    dec cl
    jnz .nextnum
.skipconv2
    ; if ch = 1, then [numstr] is the value
    ; if ch = 2, then [numstr] SHL 4+[numstr+1] is the value
    xor al,al
    cmp ch,1
    jne .check2
    mov al,[numstr]
    call converthex2num
    jmp .endcheck
.check2
    cmp ch,2
    jne .endcheck
    mov al,[numstr]
    call converthex2num
    shl al,4
    mov ah,al
    mov al,[numstr+1]
    call converthex2num
    add al,ah
.endcheck
    xor ecx,ecx
    xor ebx,ebx
    mov cx,dx
    call SPCbreakops
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    jmp debugloopa
.exit
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    jmp debugloopb

SECTION .data
.message1 db 'Enter Address : 0000'
PrevBreakPt dd 0
SECTION .text

NEWSYM repeatbreakpoint
    push es
    mov ax,[selcB800]
    mov es,ax
    xor ecx,ecx
    xor ebx,ebx
    mov cx,[PrevBreakPt]
    mov bl,[PrevBreakPt+2]
    call breakops
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    pop es
    jmp debugloopa

;*******************************************************
; BreakPoint           Draws Window and Input Breakpoint
;*******************************************************

NEWSYM breakpoint
    mov byte[clearchar],'0'
    mov byte[hexok],1
    mov byte[wx],24
    mov byte[wx2],56
    mov byte[wy],11
    mov byte[wy2],13
    mov cx,4000
    call delay
    call drawwindow
    push es
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+29*2
    mov esi,.message1
    mov ecx,22
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
    ; set cursor to (10,50)
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,50
    int 10h
    mov byte[maxstrchar],6
    mov dword[charmemloc],12*160+50*2
    call getstring
    cmp al,27
    je near .exit
    ; convert to number
    mov cl,bl
    mov ch,bl
    xor edx,edx
    xor eax,eax
    xor ebx,ebx
    cmp cl,4
    jna .nextnum2
    mov cl,4
.nextnum2
    sub ch,cl
    mov bl,ch
    cmp cl,0
    je .skipconv2
.nextnum
    shl dx,4
    mov al,[numstr+ebx]
    inc ebx
    call converthex2num
    add dx,ax
    dec cl
    jnz .nextnum
.skipconv2
    ; if ch = 1, then [numstr] is the value
    ; if ch = 2, then [numstr] SHL 4+[numstr+1] is the value
    xor al,al
    cmp ch,1
    jne .check2
    mov al,[numstr]
    call converthex2num
    jmp .endcheck
.check2
    cmp ch,2
    jne .endcheck
    mov al,[numstr]
    call converthex2num
    shl al,4
    mov ah,al
    mov al,[numstr+1]
    call converthex2num
    add al,ah
.endcheck
    xor ecx,ecx
    xor ebx,ebx
    mov bl,al
    mov cx,dx
    call breakops
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    jmp debugloopa
.exit
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    jmp debugloopb

SECTION .data
.message1 db 'Enter Address : 000000'
SECTION .text

;*******************************************************
; Trace        Draws Window and Inputs # of instructions
;*******************************************************

NEWSYM trace
    mov byte[clearchar],32
    mov byte[hexok],0
    mov byte[wx],14
    mov byte[wx2],65
    mov byte[wy],11
    mov byte[wy2],13
    call drawwindow
    push es
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+17*2
    mov esi,.message1
    mov ecx,34
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
    mov edi,12*160+52*2
    mov al,32
    mov ah,79
    mov ecx,9
    rep stosw
    ; set cursor to (12,60)
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,60
    int 10h
    mov byte[maxstrchar],9
    mov dword[charmemloc],12*160+60*2
    call getstring
    cmp al,27
    je .notrace
    cmp bl,0
    je .notrace
    ; convert to string
    mov esi,numstr
    xor eax,eax
    mov cl,bl
    mov ebx,10
.loopc
    mul ebx
    xor edx,edx
    mov dl,[esi]
    sub dl,30h
    inc esi
    add eax,edx
    dec cl
    jnz .loopc
    mov [num2trace], eax
    ; start tracing
    call traceops
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    jmp debugloopa
.notrace
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    jmp debugloopb

SECTION .data
.message1  db 'Enter # of Instructions to Trace :'
NEWSYM num2trace, dd 0
SECTION .text
;*******************************************************
; Convert Hex 2 Num      Converts Hex ASCII to Hex in AL
;*******************************************************

NEWSYM converthex2num
    sub al,'0'
    cmp al,9
    jna .skipconv
    sub al,'A'-'9'-1
.skipconv
    ret

;*******************************************************
; TraceOps               Traces [num2trace] # of opcodes
;*******************************************************

NEWSYM traceops
    mov byte[wx],14
    mov byte[wx2],65
    mov byte[wy],11
    mov byte[wy2],13
    call drawwindow
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+20*2
    mov esi,.message1
    mov ecx,28
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
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
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    mov ah,0bh
    int 21h
    test al,0FFh
    jz .skipa
    mov ah,07h
    int 21h
    cmp al,27
    je .skipc
.skipa
    dec dword[num2trace]
    jnz .loopa
.skipc
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov bl,[xpb]
    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    ret

SECTION .data
.message1  db 'Tracing.  Press ESC to stop.'
SECTION .text

;*******************************************************
; SPCBreakOps                 Breaks at Breakpoint @ SPC
;*******************************************************

NEWSYM SPCbreakops
    ; set cursor to (12,60)
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,60
    int 10h
    xor eax,eax
    mov ax,cx
    add eax,SPCRAM
    mov [breakarea],eax

    mov byte[wx],14
    mov byte[wx2],65
    mov byte[wy],11
    mov byte[wy2],13
    call drawwindow
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+18*2
    mov esi,.message1
    mov ecx,42
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
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
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    mov ah,0bh
    int 21h
    test al,0FFh
    jz .skipa
    mov ah,07h
    int 21h
    cmp al,27
    je .skipc
.skipa
    mov eax,[breakarea]
    cmp ebp,eax
    jne .loopa
.skipc
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    ret

SECTION .data
.message1  db 'Locating Breakpoint ... Press ESC to stop.'
SECTION .text

;*******************************************************
; BreakatSign                Breaks whenever debstop = 1
;*******************************************************

NEWSYM breakatsign
    push es
    mov byte[debstop3],0
    ; set cursor to (12,60)
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,60
    int 10h
    xor eax,eax
    mov ax,cx
    add eax,SPCRAM
    mov [breakarea],eax

    mov byte[wx],14
    mov byte[wx2],65
    mov byte[wy],11
    mov byte[wy2],13
    call drawwindow
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+18*2
    mov esi,.message1
    mov ecx,42
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
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
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    mov ah,0bh
    int 21h
    test al,0FFh
    jz .skipa
    mov ah,07h
    int 21h
    cmp al,27
    je .skipc
.skipa
    cmp byte[debstop3],1
    jne near .loopa
    mov byte[debstop3],0
.skipc
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    pop es
    jmp debugloopa

SECTION .data
.message1  db 'Waiting for Signal .... Press ESC to stop.'
SECTION .text

;*******************************************************
; BreakatSign&Log            Breaks whenever debstop = 1
;*******************************************************

NEWSYM breakatsignlog
    push es
    mov byte[debstop3],0
    ; set cursor to (12,60)
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,60
    int 10h
    xor eax,eax
    mov ax,cx
    add eax,SPCRAM
    mov [breakarea],eax

    mov byte[wx],14
    mov byte[wx2],65
    mov byte[wy],11
    mov byte[wy2],13
    call drawwindow
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+18*2
    mov esi,.message1
    mov ecx,42
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb

    pushad
    mov ax,ds
    mov es,ax
    mov edx,.logfname
    call Create_File
    mov [.handle],ax
    popad

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

    pushad
    mov ax,ds
    mov es,ax
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh
    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    mov edi,.loggeddata
    call nextopcode.outputbuffer
    mov byte[edi],13
    mov byte[edi+1],10
    mov byte[edi+2],0
    mov eax,.loggeddata
    call StringLength
    mov edx,.loggeddata
    mov bx,[.handle]
    call Write_File
    popad

    call splitflags
    call execsingle
    call joinflags
    mov dh,[pdh]
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    mov ah,0bh
    int 21h
    test al,0FFh
    jz .skipa
    mov ah,07h
    int 21h
    cmp al,27
    je .skipc
.skipa
;    cmp byte[SPCRAM+6],40h
;    je .skipc
    mov eax,[ram7f]
;    jmp .loopa
    cmp byte[debstop3],1
    jne near .loopa
.skipc
    mov byte[debstop3],0
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si

    mov bx,[.handle]
    call Close_File

    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    pop es
    jmp debugloopa


;    mov edx,.logfname
;    call Create_File
;    mov [.handle],ax


    jmp .blah
    pushad
    mov ax,ds
    mov es,ax
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh
    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    mov edi,.loggeddata
    call nextopcode.outputbuffer
    mov byte[edi],13
    mov byte[edi+1],10
    mov byte[edi+2],0
    mov eax,.loggeddata
    call StringLength
    mov edx,.loggeddata
    mov bx,[.handle]
    call Write_File
    popad
.blah

    mov bx,[.handle]
    call Close_File

SECTION .data
.loggeddata times 128 db 0
.message1  db 'Waiting for Signal .... Press ESC to stop.',0
.handle dw 0
.logfname db 'debug.log',0

;*******************************************************
; BreakatSignB               Breaks whenever keyonsn = 1
;*******************************************************
NEWSYM keyonsn, db 0
NEWSYM prbreak, db 0
SECTION .text

EXTSYM SPCSave

NEWSYM breakatsignb
    mov byte[keyonsn],0
    mov byte[prbreak],0
    cmp byte[SPCSave],1
    jne .nospcsave
    mov byte[debuggeron],1
.nospcsave

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
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    call Check_Key
    test al,0FFh
    jz .skipa
    call Get_Key
    cmp al,27
    je .skipc
.skipa
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
    cmp byte[SPCSave],1
    jne .nospcsave2
    mov byte[debuggeron],0
.nospcsave2

    ret

;*******************************************************
; BreakatSignC               Breaks whenever sndwrit = 1
;*******************************************************
SECTION .data
NEWSYM sndwrit, db 0
SECTION .text

NEWSYM breakatsignc
    mov byte[prbreak],0
    mov byte[sndwrit],0
;    mov byte[debuggeron],1

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
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    mov ah,0bh
    int 21h
    test al,0FFh
    jz .skipa
    mov ah,07h
    int 21h
    cmp al,27
    je .skipc
.skipa
    cmp byte[SPCRAM+6],40h
    je .skipc
    cmp byte[sndwrit],1
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
;    mov byte[debuggeron],0
    ret

;*******************************************************
; BreakOps                          Breaks at Breakpoint
;*******************************************************

NEWSYM breakops
    ; set cursor to (12,60)
    mov [PrevBreakPt],cx
    mov [PrevBreakPt+2],bl
    push ebx
    mov ah,02h
    mov bl,0
    mov dh,12
    mov dl,60
    int 10h
    pop ebx
    test cx,8000h
    jz .loweraddr2
    mov esi,[snesmmap+ebx*4]
    jmp .skiplower2
.loweraddr2
    mov esi,[snesmap2+ebx*4]
.skiplower2
    add esi,ecx                 ; add program counter to address
    mov [breakarea],esi

    mov byte[wx],14
    mov byte[wx2],65
    mov byte[wy],11
    mov byte[wy2],13
    call drawwindow
    mov ax,[selcB800]
    mov es,ax
    mov edi,12*160+18*2
    mov esi,.message1
    mov ecx,42
    mov ah,31
.loopb
    lodsb
    stosw
    dec ecx
    jnz .loopb
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
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    mov ah,0bh
    int 21h
    test al,0FFh
    jz .skipa
    mov ah,07h
    int 21h
    cmp al,27
    je .skipc
.skipa
    cmp esi,[breakarea]
    jne .loopa
.skipc
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    ret

SECTION .data
.message1  db 'Locating Breakpoint ... Press ESC to stop.'
NEWSYM breakarea, dd 0
SECTION .text

;*******************************************************
; Execute Next Opcode
;*******************************************************

NEWSYM execnextop
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
    call splitflags
    call execsingle
    call joinflags
    call UpdateDPage
    ; execute
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov dh,[pdh]
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    inc dword[numinst]
    ret

;*******************************************************
; Start Display
;*******************************************************
; use [debugbuf], 1000 pages of 77

NEWSYM startdisplay
    push es
    mov ax,[selcB800]
    mov es,ax
    ; clear the screen
    mov edi,0
    mov ecx,1000
    mov eax,0F000F00h
    rep stosd
    ; draw to screen
    mov edi,0
    mov ax,0F00h
    stosw
    mov edi,160
    mov al,213
    mov al,[CurrentCPU]
    add al,48
    mov ah,31
    stosw
    mov al,205
    mov cx,15
    rep stosw
    mov al,32
    stosw
    mov al,'C'
    stosw
    stosw
    mov al,':'
    stosw
    mov al,' '
    mov cl,4
    rep stosw
    mov al,'Y'
    stosw
    mov al,':'
    stosw
    mov al,' '
    mov cl,4
    rep stosw
    mov al,205
    mov cx,19
    rep stosw
    mov al,32
    stosw
    mov al,'-'
    mov cx,11
    rep stosw
    mov al,32
    stosw
    mov al,205
    mov cx,16
    rep stosw
    mov al,184
    stosw
    mov edi,320
    mov bl,20
.loopa
    mov al,179
    stosw
    mov al,32
    mov cx,77
    rep stosw
    mov al,179
    stosw
    mov al,177
    mov ah,7
    stosw
    mov ah,31
    dec bl
    jnz .loopa
    mov al,192
    stosw
    mov al,196
    mov cx,77
    rep stosw
    mov al,217
    stosw
    mov ah,7
    mov al,177
    stosw
    add edi,2
    mov cx,79
    rep stosw
    ; print debugger information
    mov edi,4
    mov esi,.debuginfo
    mov ah,15
    call .printinfo
    mov edi,160+8          ; (X:4,Y:1)
    mov esi,.D65816
    mov ah,31
    call .printinfo
    mov edi,160*24         ; (X:0,Y:24)
    mov esi,.mbar
    mov ah,15
    call .printinfo
    pop es
    ret

.printinfo
.loopprint
    lodsb
    cmp al,'$'
    je .doneprint
    cmp al,'@'
    je .changecol
    stosw
    jmp .loopprint
.changecol
    lodsb
    mov ah,al
    sub ah,40
    jmp .loopprint
.doneprint
    ret

SECTION .data
.debuginfo db '- @5Z@4S@3N@2E@6S@7 debugger -$'
.D65816    db ' 65816 $'
.mbar      db '@4(@6T@4)@7race for  @4(@6B@4)@7reakpoint  '
           db '@4(@6Enter@4)@7 Next  '
           db '@4(@6M@4)@7odify  @4(@6F9@4)@7 Signal  @4(@6F1@4)@7 Run$'

;*******************************************************
; Next Opcode              Writes the next opcode & regs
;*******************************************************
; 008000 STZ $123456,x A:0000 X:0000 Y:0000 S:01FF DB:00 D:0000 P:33 E+
NEWSYM debugsa1, db 0
NEWSYM skipdebugsa1, db 1
SECTION .text

NEWSYM nextopcode
    push es
    mov es,[selcB800]
    mov edi,160+100
    mov eax,[numinst]
    mov byte[.addernum],11
    mov byte[.charprin],'-'
    call .printnum
    xor eax,eax
    mov al,[curcyc]
    mov byte[.addernum],3
    mov edi,160+40
    mov byte[.charprin],' '
    call .printnum
    mov edi,160+52
    mov ax,[curypos]
    call .printnum

    mov ax,ds
    mov es,ax

    ; set output pointer
    mov edi,[debugbuf]          ; set write pointer
    xor eax,eax
    mov ax,[debugt]
    push bx
    mov bx,77
    mul bx
    pop bx
    add edi,eax

    cmp byte[debugsa1],1
    je .sa1
    call .outputbuffer
    jmp .nosa1
.sa1
    mov byte[debugsa1],0
    call .outputbuffersa1
.nosa1

    ; increment tail/head
    call .addtail
    cmp word[debugh],0
    jne .changeview
    cmp word[debugt],21
    jb .nochangeview
.changeview
    mov ax,[debugt]
    sub ax,20
    jns .notneg
    add ax,100
.notneg
    mov [debugv],ax
    jmp .finchangeview
.nochangeview
    mov word[debugv],0
.finchangeview
    ; set cursor to (1,1)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    pop es
    ret

.outputbuffer
    ; output pb/pc
    mov al,[xpb]
    call .printhex8
    mov ax,[xpc]
    call .printhex16
    mov al,32
    stosb

    ; output instruction
    xor ebx,ebx
    mov bl,[xpb]
    xor eax,eax
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddr
    mov ebx,[snesmmap+ebx*4]
    jmp .skiplower
.loweraddr
    cmp ax,4300h
    jb .lower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
.lower
    mov ebx,[snesmap2+ebx*4]
    jmp .skiplower
.dma
    mov ebx,dmadata-4300h
.skiplower
    add ebx,eax                 ; add program counter to address
    xor ah,ah
    mov al,[ebx]                ; get instruction number
    mov esi,ocname
    shl eax,2
    add esi,eax
    mov ecx,4
    rep movsb
    call .outaddrmode

    ; output registers
    mov al,'A'
    stosb
    mov al,':'
    stosb
    mov ax,[xa]
    call .printhex16
    mov al,32
    stosb
    mov al,'X'
    stosb
    mov al,':'
    stosb
    mov ax,[xx]
    call .printhex16
    mov al,32
    stosb
    mov al,'Y'
    stosb
    mov al,':'
    stosb
    mov ax,[xy]
    call .printhex16
    mov al,32
    stosb
    mov al,'S'
    stosb
    mov al,':'
    stosb
    mov ax,[xs]
    call .printhex16
    mov al,32
    stosb
    mov al,'D'
    stosb
    mov al,'B'
    stosb
    mov al,':'
    stosb
    mov al,[xdb]
    call .printhex8
    mov al,32
    stosb
    mov al,'D'
    stosb
    mov al,':'
    stosb
    mov ax,[xd]
    call .printhex16
    mov al,32
    stosb
    mov al,'P'
    stosb
    mov al,':'
    stosb
    mov al,[xp]
    call .printhex8
    mov al,32
    stosb
;    mov al,'e'
;    stosb
    mov al,'e'
    cmp byte[xe],1
    jne .nopos
    mov al,'E'
.nopos
    stosb
    ret

.outputbuffersa1
    mov al,'-'
    stosb

    ; output pb/pc
    mov al,[SA1xpb]
    call .printhex8
    mov ax,[xpc]
    call .printhex16
    mov al,32
    stosb

    ; output instruction
    xor ebx,ebx
    mov bl,[SA1xpb]
    xor eax,eax
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddrsa1
    mov ebx,[snesmmap+ebx*4]
    jmp .skiplowersa1
.loweraddrsa1
    cmp ax,4300h
    jb .lowersa1
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dmasa1
.lowersa1
    mov ebx,[snesmap2+ebx*4]
    jmp .skiplowersa1
.dmasa1
    mov ebx,dmadata-4300h
.skiplowersa1
    add ebx,eax                 ; add program counter to address
    xor ah,ah
    mov al,[ebx]                ; get instruction number
    mov esi,ocname
    shl eax,2
    add esi,eax
    mov ecx,4
    rep movsb
    call .outaddrmode

    ; output registers
    mov al,'A'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xa]
    call .printhex16
    mov al,32
    stosb
    mov al,'X'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xx]
    call .printhex16
    mov al,32
    stosb
    mov al,'Y'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xy]
    call .printhex16
    mov al,32
    stosb
    mov al,'S'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xs]
    call .printhex16
    mov al,32
    stosb
    mov al,'D'
    stosb
    mov al,'B'
    stosb
    mov al,':'
    stosb
    mov al,[SA1xdb]
    call .printhex8
    mov al,32
    stosb
    mov al,'D'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xd]
    call .printhex16
    mov al,32
    stosb
    mov al,'P'
    stosb
    mov al,':'
    stosb
    mov al,[SA1RegP]
    call .printhex8
    ret

.addtail
    inc word[debugt]
    cmp word[debugt],100
    jne .nowrap
    mov word[debugt],0
.nowrap
    mov ax,[debugt]
    cmp ax,[debugh]
    jne .nohead
    inc word[debugh]
.nohead
    cmp word[debugh],100
    jne .nowrap2
    mov word[debugh],0
.nowrap2
    ret

SECTION .data
.addernum db 0
.charprin db 0
SECTION .text

.printnum
    ; process through each digit
    push edx
    push eax
    push ebx
    push cx
    xor ebx,ebx           ; quotent variable
    xor edx,edx           ; clear high byte
    xor cx,cx             ; clear counter variable
    mov ebx,10
.loopa2
    div ebx              ; get quotent and remainder
    push dx              ; store number to stack
    inc cl
    xor edx,edx
    test eax,0FFFFFFFFh
    jnz .loopa2
    xor eax,eax
    mov al,[.addernum]
    xor ah,ah
    sub ax,cx
    cmp ax,0
    je .loopb4
.loopb3
    push ax
    mov al,[.charprin]
    mov ah,31
    stosw
    pop ax
    dec ax
    jnz .loopb3
.loopb4
.loopb2
    pop ax              ; get number back from stack
    add al,30h          ; adjust to ASCII value
    mov ah,31
    stosw
    dec cl
    jnz .loopb2
    pop cx
    pop ebx
    pop eax
    pop edx
    ret

; Outputs the rest of the instruction depending on the address mode

.outaddrmode
    mov esi,ebx
    xor ebx,ebx
    mov bl,[esi]
    mov ah,[addrmode+ebx]
    cmp ah,0
    jne .check1
    jmp .out0
.check1
    cmp ah,1
    jne .check2
    jmp .out1
.check2
    cmp ah,2
    jne .check3
    jmp .out2
.check3
    cmp ah,3
    jne .check4
    jmp .out3
.check4
    cmp ah,4
    jne .check5
    jmp .out4
.check5
    cmp ah,5
    jne .check6
    jmp .out5
.check6
    cmp ah,6
    jne .check7
    jmp .out6
.check7
    cmp ah,7
    jne .check8
    jmp .out7
.check8
    cmp ah,8
    jne .check9
    jmp .out8
.check9
    cmp ah,9
    jne .check10
    jmp .out9
.check10
    cmp ah,10
    jne .check11
    jmp .out10
.check11
    cmp ah,11
    jne .check12
    jmp .out11
.check12
    cmp ah,12
    jne .check13
    jmp .out12
.check13
    cmp ah,13
    jne .check14
    jmp .out13
.check14
    cmp ah,14
    jne .check15
    jmp .out14
.check15
    cmp ah,15
    jne .check16
    jmp .out15
.check16
    cmp ah,16
    jne .check17
    jmp .out16
.check17
    cmp ah,17
    jne .check18
    jmp .out17
.check18
    cmp ah,18
    jne .check19
    jmp .out18
.check19
    cmp ah,19
    jne .check20
    jmp .out19
.check20
    cmp ah,20
    jne .check21
    jmp .out20
.check21
    cmp ah,21
    jne .check22
    jmp .out21
.check22
    cmp ah,22
    jne .check23
    jmp .out22
.check23
    cmp ah,23
    jne .check24
    jmp .out23
.check24
    cmp ah,24
    jne .check25
    jmp .out24
.check25
    cmp ah,25
    jne .check26
    jmp .out25
.check26
    cmp ah,26
    jne .check27
    jmp .out26
.check27
    jmp .out27
    ret

; each mode must output 10 characters

%macro getxb 1
 push eax
 xor eax,eax
 mov al,[esi]
 shl eax,2
 mov al,[ocname+eax]
 cmp al,'J'
 jz %%usepbr
 pop eax
 mov %1,[xdb]
 jmp %%usedbr
%%usepbr
 pop eax
 mov %1,[xpb]
%%usedbr
%endmacro

.out0           ;
    mov al,' '
    mov ecx,19
    rep stosb
    ret

.out1           ; #$12,#$1234 (M-flag)
    mov al,'#'
    stosb
    mov al,'$'
    stosb
    test byte[xp],20h
    jz .out116b
    mov al,[esi+1]
    call .printhex8
    mov al,' '
    mov ecx,15
    rep stosb
    ret
.out116b
    mov ax,[esi+1]
    call .printhex16
    mov al,' '
    mov ecx,13
    rep stosb
    ret

.out2           ; $1234 : db+$1234
    mov al,'$'
    stosb
    mov ax,[esi+1]
    call .printhex16
    mov al,' '
    mov ecx,5
    rep stosb
    mov al,'['
    stosb
    getxb(al)
    call .printhex8
    mov ax,[esi+1]
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out3           ; $123456
    mov al,'$'
    stosb
    mov al,[esi+3]
    call .printhex8
    mov ax,[esi+1]
    call .printhex16
    mov al,' '
    mov ecx,12
    rep stosb
    ret

.out4           ; $12 : $12+d
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,' '
    mov ecx,7
    rep stosb
    mov al,'['
    stosb
    xor ax,ax
    call .printhex8
    mov al,[esi+1]
    add ax,[xd]
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out5           ; A
    mov al,'A'
    stosb
    mov al,' '
    mov ecx,18
    rep stosb
    ret

.out6           ; i
    mov al,' '
    mov ecx,19
    rep stosb
    ret

.out7           ; ($12),y : ($12+$13+d)+y
    mov al,'('
    stosb
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,')'
    stosb
    mov al,','
    stosb
    mov al,'Y'
    stosb
    mov al,' '
    stosb
    stosb
    stosb
    mov al,'['
    stosb
    xor eax,eax
    call .printhex8
    xor eax,eax
    xor ecx,ecx
    mov cl,[esi+1]
    add cx,[xd]
    call dword near [memtabler8]
    mov dl,al
    xor eax,eax
    inc cx
    call dword near [memtabler8]
    mov dh,al
    mov ax,dx
    test byte[xp],10h
    jz .out7w
    add al,[xy]
    jmp .out7n
.out7w
    add ax,[xy]
.out7n
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out8           ; [$12],y : [$12+$13+$14+d]+y
    mov al,'['
    stosb
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,']'
    stosb
    mov al,','
    stosb
    mov al,'Y'
    stosb
    mov al,' '
    stosb
    stosb
    stosb
    mov al,'['
    stosb
    xor eax,eax
    xor ecx,ecx
    xor edx,edx
    mov cl,[esi+1]
    add cx,[xd]
    call dword near [memtabler8]
    mov dl,al
    xor eax,eax
    inc cl
    call dword near [memtabler8]
    mov dh,al
    inc cl
    call dword near [memtabler8]
    shl eax,16
    or edx,eax
    test byte[xp],10h
    jz .out8w
    add dl,[xy]
    jmp .out8n
.out8w
    add dx,[xy]
.out8n
    mov eax,edx
    shr eax,16
    call .printhex8
    mov eax,edx
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out9           ; ($12,x) : ($12+$13+d+x)
    mov al,'('
    stosb
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,','
    stosb
    mov al,'X'
    stosb
    mov al,')'
    stosb
    mov al,' '
    stosb
    stosb
    stosb
    mov al,'['
    stosb
    xor eax,eax
    call .printhex8
    xor ecx,ecx
    mov cl,[esi+1]
    add cx,[xd]
    test byte[xp],10h
    jz .out9w
    add cl,[xx]
    jmp .out9n
.out9w
    add cx,[xx]
.out9n
    call dword near [memtabler8]
    mov dl,al
    xor eax,eax
    inc cl
    call dword near [memtabler8]
    mov dh,al
    mov ax,dx
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out10          ; $12,x : $12+d+x
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,','
    stosb
    mov al,'X'
    stosb
    mov al,' '
    mov ecx,5
    rep stosb
    mov al,'['
    stosb
    xor eax,eax
    call .printhex8
    mov al,[esi+1]
    add ax,[xd]
    test byte[xp],10h
    jz .out10w
    add al,[xx]
    jmp .out10n
.out10w
    add ax,[xx]
.out10n
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out11          ; $12,y : $12+d+y
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,','
    stosb
    mov al,'Y'
    stosb
    mov al,' '
    mov ecx,5
    rep stosb
    mov al,'['
    stosb
    xor eax,eax
    call .printhex8
    mov al,[esi+1]
    add ax,[xd]
    test byte[xp],10h
    jz .out11w
    add al,[xy]
    jmp .out11n
.out11w
    add ax,[xy]
.out11n
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out12          ; $1234,x : dbr+$1234+x
    mov al,'$'
    stosb
    mov ax,[esi+1]
    call .printhex16
    mov al,','
    stosb
    mov al,'X'
    stosb
    mov al,' '
    stosb
    stosb
    stosb
    mov al,'['
    stosb
    mov al,[xdb]
    call .printhex8
    xor eax,eax
    xor ecx,ecx
    mov ax,[esi+1]
    test byte[xp],10h
    jz .out12w
    add al,[xx]
    jmp .out12n
.out12w
    add ax,[xx]
.out12n
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out13          ; $1234,y : dbr+$1234+y
    mov al,'$'
    stosb
    mov ax,[esi+1]
    call .printhex16
    mov al,','
    stosb
    mov al,'Y'
    stosb
    mov al,' '
    stosb
    stosb
    stosb
    mov al,'['
    stosb
    mov al,[xdb]
    call .printhex8
    xor eax,eax
    xor ecx,ecx
    mov ax,[esi+1]
    test byte[xp],10h
    jz .out13w
    add al,[xy]
    jmp .out13n
.out13w
    add ax,[xy]
.out13n
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out14          ; $123456,x : $123456+x
    mov al,'$'
    stosb
    mov al,[esi+3]
    call .printhex8
    mov ax,[esi+1]
    call .printhex16
    mov al,','
    stosb
    mov al,'X'
    stosb
    mov al,' '
    stosb
    mov al,'['
    stosb
    xor eax,eax
    mov al,[esi+1]
    mov ecx,eax
    mov al,[esi+2]
    shl eax,8
    or ecx,eax
    xor ax,ax
    mov al,[esi+3]
    shl eax,16
    or ecx,eax
    test byte[xp],10h
    jz .out14w
    add cl,[xx]
    jmp .out14n
.out14w
    add cx,[xx]
.out14n
    mov eax,ecx
    shr eax,16
    call .printhex8
    mov eax,ecx
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out15          ; +-$12 / $1234
    mov al,'$'
    stosb
    mov al,[esi+1]
    cbw
    add ax,[xpc]
    add ax,2
    push eax
    call .printhex16
    mov al,' '
    mov ecx,4
    rep stosb
    mov al,' '
    stosb
    mov al,'['
    stosb
    mov al,[xpb]
    call .printhex8
    pop eax
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out16          ; +-$1234 / $1234
    mov al,'$'
    stosb
    mov ax,[esi+1]
    add ax,[xpc]
    add ax,3
    push eax
    call .printhex16
    mov al,' '
    mov ecx,4
    rep stosb
    mov al,' '
    stosb
    mov al,'['
    stosb
    mov al,[xpb]
    call .printhex8
    pop eax
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out17          ; ($1234)
    mov al,'('
    stosb
    mov al,'$'
    stosb
    mov ax,[esi+1]
    call .printhex16
    mov al,')'
    stosb
    mov al,' '
    stosb
    stosb
    stosb
    mov al,'['
    stosb
    getxb(al)
    call .printhex8
    xor ebx,ebx
    xor ecx,ecx
    getxb(bl)
    mov cx,[esi+1]
    call dword near [memtabler8+ebx*4]
    mov bl,al
    inc cx
    xor ebx,ebx
    getxb(bl)
    call dword near [memtabler8+ebx*4]
    mov ah,al
    mov al,bl
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out18          ; ($12)
    mov al,'('
    stosb
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,')'
    stosb
    mov al,' '
    mov ecx,5
    rep stosb
    mov al,'['
    stosb
    mov al,[xdb]
    call .printhex8
    xor ecx,ecx
    mov cl,[esi+1]
    add cx,[xd]
    call dword near [memtabler8]
    mov bl,al
    inc cl
    call dword near [memtabler8]
    mov ah,al
    mov al,bl
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out19          ; [$12]
    mov al,'['
    stosb
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,']'
    stosb
    mov al,' '
    mov ecx,5
    rep stosb
    mov al,'['
    stosb
    xor eax,eax
    xor ecx,ecx
    xor edx,edx
    mov cl,[esi+1]
    call dword near [memtabler8]
    mov dl,al
    xor eax,eax
    inc cl
    call dword near [memtabler8]
    mov dh,al
    inc cl
    call dword near [memtabler8]
    shl eax,16
    or edx,eax
    mov eax,edx
    shr eax,16
    call .printhex8
    mov eax,edx
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out20          ; ($1234,x)
    mov al,'('
    stosb
    mov al,'$'
    stosb
    mov ax,[esi+1]
    call .printhex16
    mov al,','
    stosb
    mov al,'X'
    stosb
    mov al,')'
    stosb
    mov al,' '
    stosb
    mov al,'['
    stosb
    mov al,[xpb]
    call .printhex8
    xor ebx,ebx
    xor ecx,ecx
    mov cx,[esi+1]
    test byte[xp],10h
    jz .out20w
    add cl,[xx]
    jmp .out20n
.out20w
    add cx,[xx]
.out20n
    xor ebx,ebx
    mov bl,[xpb]
    call dword near [memtabler8+ebx*4]
    mov dl,al
    inc cx
    xor ebx,ebx
    mov bl,[xpb]
    call dword near [memtabler8+ebx*4]
    mov ah,al
    mov al,dl
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out21          ; s
    mov al,' '
    mov ecx,19
    rep stosb
    ret

.out22          ; d,s
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,','
    stosb
    mov al,'S'
    stosb
    mov al,' '
    mov ecx,5
    rep stosb
    mov al,'['
    stosb
    xor eax,eax
    call .printhex8
    mov al,[esi+1]
    add ax,[xs]
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out23          ; (d,s),y
    mov al,'('
    stosb
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,','
    stosb
    mov al,'S'
    stosb
    mov al,')'
    stosb
    mov al,','
    stosb
    mov al,'Y'
    stosb
    mov al,' '
    stosb
    mov al,'['
    stosb
    xor eax,eax
    xor ecx,ecx
    call .printhex8
    mov cl,[esi+1]
    add cx,[xs]
    call dword near [memtabler8]
    mov dh,al
    inc cx
    call dword near [memtabler8]
    mov dl,al
    test byte[xp],10h
    jz .out23w
    add dl,[xy]
    jmp .out23n
.out23w
    add dx,[xy]
.out23n
    mov ax,dx
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.out24          ; xyc - $1234
    mov al,'$'
    stosb
    mov ax,[esi+1]
    call .printhex16
    mov al,' '
    mov ecx,14
    rep stosb
    ret

.out25          ; #$12 (Flag Operations)
    mov al,'#'
    stosb
    mov al,'$'
    stosb
    mov al,[esi+1]
    call .printhex8
    mov al,' '
    mov ecx,15
    rep stosb
    ret

.out26          ; #$12,#$1234 (X-flag)
    mov al,'#'
    stosb
    mov al,'$'
    stosb
    test byte[xp],10h
    jz .out2616b
    mov al,[esi+1]
    call .printhex8
    mov al,' '
    mov ecx,15
    rep stosb
    ret

.out2616b
    mov ax,[esi+1]
    call .printhex16
    mov al,' '
    mov ecx,13
    rep stosb
    ret

.out27 ; [$1234]
    mov al,'['
    stosb
    mov ax,[esi+1]
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    mov ecx,4
    rep stosb
    mov al,'['
    stosb
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[xpb]
    mov cx,[esi+1]
    call dword near [memtabler8+ebx*4]
    mov dl,al
    xor ebx,ebx
    mov bl,[xpb]
    inc cx
    call dword near [memtabler8+ebx*4]
    mov dh,al
    xor ebx,ebx
    mov bl,[xpb]
    inc cx
    call dword near [memtabler8+ebx*4]
    call .printhex8
    mov ax,dx
    call .printhex16
    mov al,']'
    stosb
    mov al,' '
    stosb
    ret

.printhex16
    push ecx
    mov ecx,4
.loopa
    xor ebx,ebx
    mov bx,ax
    and bx,0F000h
    shr bx,12
    push ax
    mov al,[.hexdat+ebx]
    stosb
    pop ax
    shl ax,4
    dec ecx
    jnz .loopa
    pop ecx
    ret

.printhex8
    push ecx
    xor ah,ah
    mov ecx,2
.loopb
    xor ebx,ebx
    mov bx,ax
    and bx,0F0h
    shr bx,4
    push ax
    mov al,[.hexdat+ebx]
    stosb
    pop ax
    shl ax,4
    dec ecx
    jnz .loopb
    pop ecx
    ret

SECTION .data
.hexdat db '0123456789ABCDEF'
SECTION .text

NEWSYM nextopcodesa1
    push es
    mov [.blah],dl
    mov es,[selcB800]
    mov edi,160+100
    mov eax,[numinst]
    mov byte[nextopcode.addernum],11
    mov byte[nextopcode.charprin],'-'
    call nextopcode.printnum
    xor eax,eax
    mov al,[curcyc]
    mov byte[nextopcode.addernum],3
    mov edi,160+40
    mov byte[nextopcode.charprin],' '
    call nextopcode.printnum
    mov edi,160+52
    mov ax,[curypos]
    call nextopcode.printnum
    pop es
    ; set output pointer
    mov edi,[debugbuf]          ; set write pointer
    inc edi
    xor eax,eax
    mov ax,[debugt]
    push bx
    mov bx,77
    mul bx
    pop bx
    add edi,eax

    call .outputtobuffer

    mov al,' '
    stosb
    stosb
    stosb

    ; increment tail/head
    call nextopcode.addtail
    cmp word[debugh],0
    jne .changeview
    cmp word[debugt],21
    jb .nochangeview
.changeview
    mov ax,[debugt]
    sub ax,20
    jns .notneg
    add ax,100
.notneg
    mov [debugv],ax
    jmp .finchangeview
.nochangeview
    mov word[debugv],0
.finchangeview
    ; set cursor to (1,1)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    ret

SECTION .data
.blah db 0
SECTION .text

.outputtobuffer
    ; output pb/pc
    mov al,[SA1xpb]
    call nextopcode.printhex8
    mov ax,[SA1xpc]
    call nextopcode.printhex16
    mov al,'-'
    stosb

    ; output instruction
    xor ebx,ebx
    mov bl,[SA1xpb]
    xor eax,eax
    mov ax,[SA1xpc]
    test ax,8000h
    jz .loweraddr
    mov ebx,[snesmmap+ebx*4]
    jmp .skiplower
.loweraddr
    cmp ax,4300h
    jb .lower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
.lower
    mov ebx,[snesmap2+ebx*4]
    jmp .skiplower
.dma
    mov ebx,dmadata-4300h
.skiplower
    add ebx,eax                 ; add program counter to address
    xor ah,ah
    mov al,[ebx]                ; get instruction number
    mov esi,ocname
    shl eax,2
    add esi,eax
    mov ecx,4
    rep movsb
    call nextopcode.outaddrmode

    ; output registers
    mov al,'A'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xa]
    call nextopcode.printhex16
    mov al,32
    stosb
    mov al,'X'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xx]
    call nextopcode.printhex16
    mov al,32
    stosb
    mov al,'Y'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xy]
    call nextopcode.printhex16
    mov al,32
    stosb
    mov al,'S'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xs]
    call nextopcode.printhex16
    mov al,32
    stosb
    mov al,'D'
    stosb
    mov al,'B'
    stosb
    mov al,':'
    stosb
    mov al,[SA1xdb]
    call nextopcode.printhex8
    mov al,32
    stosb
    mov al,'D'
    stosb
    mov al,':'
    stosb
    mov ax,[SA1xd]
    call nextopcode.printhex16
    mov al,32
    stosb
    mov al,'P'
    stosb
    mov al,':'
    stosb
    mov al,[.blah]
    call nextopcode.printhex8
    ret


;*******************************************************
; Show Debug Display
;*******************************************************

NEWSYM showdd
    ; copy debug buffer on to screen
    push es
    mov es,[selcB800]
    mov edi,160
    mov al,[CurrentCPU]
    add al,48
    mov ah,31
    stosw

    mov edi,2*160+2             ; (X:2,Y:2)
    mov esi,[debugbuf]
    mov eax,0
    mov ax,[debugv]
    mov bx,77
    mul bx
    mov dx,[debugv]
    add esi,eax
    mov bl,20
.loopb
    mov ah,31
    mov ecx,77
.loopa
    lodsb
    stosw
    dec ecx
    jnz .loopa
    add edi,6
    ; check if exceeded limit
    inc dx
    cmp dx,100
    jb .skipadd
    xor dx,dx
    sub esi,7700
.skipadd
    dec bl
    jnz .loopb
    pop es
    ret

;*******************************************************
; Clear Display                       Clears the Display
;*******************************************************

NEWSYM cleardisplay
    push es
    mov ax,[selcB800]
    mov es,ax
    ; clear the screen
    mov edi,0
    mov ecx,1000
    mov eax,07000700h
    rep stosd
    pop es
    ; set cursor to (0,0)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    ret

;*******************************************************
; Next SPC Opcode          Writes the next opcode & regs
;*******************************************************
; 008000 STZ $123456,x A:0000 X:0000 Y:0000 S:01FF DB:00 D:0000 P:33 E+
NEWSYM nextspcopcode
    cmp byte[soundon],0
    jne .noret
    ret
.noret
    cmp byte[cycpbl],55
    jb .noretb
    ret
.noretb
    ; set output pointer
    mov edi,[debugbuf]          ; set write pointer
    inc edi
    xor eax,eax
    mov ax,[debugt]
    push bx
    mov bx,77
    mul bx
    pop bx
    add edi,eax

    ; output spc pc & opcode #
    mov eax,[spcPCRam]
    sub eax,SPCRAM
    call .printhex16
    mov al,'/'
    stosb
    mov ebx,[spcPCRam]
    mov al,[ebx]
    call .printhex8
    mov al,32
    stosb

    ; output instruction
    mov ebx,[spcPCRam]
    xor eax,eax
    mov al,[ebx]                ; get instruction number
    mov esi,spcnametab
    shl eax,3
    add esi,eax
    mov ecx,6
    rep movsb
    call .outaddrmode

    mov al,'A'
    stosb
    mov al,':'
    stosb
    mov al,[spcA]
    call .printhex8
    mov al,32
    stosb
    mov al,'X'
    stosb
    mov al,':'
    stosb
    mov al,[spcX]
    call .printhex8
    mov al,32
    stosb
    mov al,'Y'
    stosb
    mov al,':'
    stosb
    mov al,[spcY]
    call .printhex8
    mov al,32
    stosb
    mov al,'S'
    stosb
    mov al,':'
    stosb
    mov al,[spcS]
    call .printhex8
    mov al,32
    stosb
    mov al,'N'
    stosb
    mov al,'+'
    test byte[spcNZ],80h
    jnz .noflaga
    mov al,'-'
.noflaga
    stosb
    mov al,'O'
    stosb
    mov al,'+'
    test byte[spcP],40h
    jnz .noflagb
    mov al,'-'
.noflagb
    stosb
    mov al,'D'
    stosb
    mov al,'+'
    test byte[spcP],20h
    jnz .noflagc
    mov al,'-'
.noflagc
    stosb
    mov al,'?'
    stosb
    mov al,'+'
    test byte[spcP],10h
    jnz .noflagd
    mov al,'-'
.noflagd
    stosb
    mov al,'H'
    stosb
    mov al,'+'
    test byte[spcP],08h
    jnz .noflage
    mov al,'-'
.noflage
    stosb
    mov al,'I'
    stosb
    mov al,'+'
    test byte[spcP],04h
    jnz .noflagf
    mov al,'-'
.noflagf
    stosb
    mov al,'Z'
    stosb
    mov al,'+'
    test byte[spcNZ],0FFh
    jz .noflagg
    mov al,'-'
.noflagg
    stosb
    mov al,'C'
    stosb
    mov al,'+'
    test byte[spcP],01h
    jnz .noflagh
    mov al,'-'
.noflagh
    stosb
    mov al,32
    stosb
    stosb
    stosb
    stosb
    stosb
    ; increment tail/head
    call .addtail
    cmp word[debugh],0
    jne .changeview
    cmp word[debugt],21
    jb .nochangeview
.changeview
    mov ax,[debugt]
    sub ax,20
    jns .notneg
    add ax,100
.notneg
    mov [debugv],ax
    jmp .finchangeview
.nochangeview
    mov word[debugv],0
.finchangeview
    ; set cursor to (1,1)
    mov ah,02h
    mov bl,0
    mov dh,0
    mov dl,0
    int 10h
    ret

.addtail
    inc word[debugt]
    cmp word[debugt],100
    jne .nowrap
    mov word[debugt],0
.nowrap
    mov ax,[debugt]
    cmp ax,[debugh]
    jne .nohead
    inc word[debugh]
.nohead
    cmp word[debugh],100
    jne .nowrap2
    mov word[debugh],0
.nowrap2
    ret

.outaddrmode
    mov ebx,[spcPCRam]
    xor eax,eax
    mov al,[ebx]
    mov al,[ArgumentTable+eax]
    shl eax,4
    mov esi,AddressTable
    add esi,eax
    mov ecx,15
.nextop
    mov al,[esi]
    cmp al,'%'
    je .doperc
    mov [edi],al
    inc esi
    inc edi
.donext
    dec ecx
    jnz .nextop
    ret
.doperc
    dec ecx
    inc esi
    mov al,[esi]
    inc esi
    cmp al,'1'
    je near .firstbyte
    cmp al,'2'
    je near .secondbyte
    cmp al,'3'
    je near .hinib
    cmp al,'4'
    je near .hinib2
    cmp al,'5'
    je near .rela2pc2
    cmp al,'6'
    je near .dp
    cmp al,'8'
    je near .memorybit
    cmp al,'9'
    je near .memorybitlow
    cmp al,'A'
    je near .rela2pc1
    cmp al,'B'
    je near .rela2pc1at2
    jmp .donext
.firstbyte
    mov ebx,[spcPCRam]
    mov al,[ebx+1]
    call .printhex8
    jmp .donext
.secondbyte
    mov ebx,[spcPCRam]
    mov al,[ebx+2]
    call .printhex8
    jmp .donext
.hinib
    mov ebx,[spcPCRam]
    mov al,[ebx]
    shr al,4
    add al,48
    mov [edi],al
    inc edi
    mov byte[edi],32
    inc edi
    jmp .donext
.hinib2
    mov ebx,[spcPCRam]
    mov al,[ebx]
    shr al,5
    and al,07h
    add al,48
    mov [edi],al
    inc edi
    mov byte[edi],32
    inc edi
    jmp .donext
.rela2pc2
    mov ebx,[spcPCRam]
    movsx eax,byte[ebx+1]
    add eax,2
    add eax,[spcPCRam]
    sub eax,SPCRAM
    call .printhex16
    add esi,3
    jmp .donext
.dp
    mov al,'$'
    mov [edi],al
    inc edi
    mov al,'0'
    test byte[spcP],20h
    jz .nodp
    mov al,'1'
.nodp
    mov [edi],al
    inc edi
    jmp .donext
.memorybit
    mov ebx,[spcPCRam]
    mov ax,[ebx+1]
    shr ax,3
    call .printhex16
    add esi,2
    jmp .donext
.memorybitlow
    mov al,','
    mov [edi],al
    inc edi
    mov ebx,[spcPCRam]
    mov al,[ebx+1]
    and al,07h
    add al,48
    mov [edi],al
    inc edi
    jmp .donext
.rela2pc1
    mov ebx,[spcPCRam]
    movsx eax,byte[ebx+1]
    add eax,2
    add eax,[spcPCRam]
    sub eax,SPCRAM
    call .printhex16
    add esi,2
    jmp .donext
.rela2pc1at2
    mov ebx,[spcPCRam]
    movsx eax,byte[ebx+2]
    add eax,2
    add eax,[spcPCRam]
    sub eax,SPCRAM
    call .printhex16
    add esi,2
    jmp .donext

; %1 = Byte, %2 = Second Byte, %3 = high nibble of opcode #,
; %4 = high nibble of opcode # and 07h, %5 = relative to PC+2
; %6 = dp ($0/$1)
; %7 = memory SHR 3 Low, %8 = memory SHR 3 High, %9 = ,memory AND 7h
; %A = relative to PC+1, %B = relative to PC+1 at second byte
;AddressTable

.printhex16
    push ecx
    mov ecx,4
.loopa
    xor ebx,ebx
    mov bx,ax
    and bx,0F000h
    shr bx,12
    push ax
    mov al,[.hexdat+ebx]
    stosb
    pop ax
    shl ax,4
    dec ecx
    jnz .loopa
    pop ecx
    sub ecx,2
    ret

.printhex8
    push ecx
    xor ah,ah
    mov ecx,2
.loopb
    xor ebx,ebx
    mov bx,ax
    and bx,0F0h
    shr bx,4
    push ax
    mov al,[.hexdat+ebx]
    stosb
    pop ax
    shl ax,4
    dec ecx
    jnz .loopb
    pop ecx
    ret

SECTION .data
.hexdat db '0123456789ABCDEF'

;*******************************************************
; Debugger OpCode Information
;*******************************************************

NEWSYM ocname
       db 'BRK ORA COP ORA TSB ORA ASL ORA PHP ORA ASL PHD TSB ORA ASL ORA '
       db 'BPL ORA ORA ORA TRB ORA ASL ORA CLC ORA INC TCS TRB ORA ASL ORA '
       db 'JSR AND JSL AND BIT AND ROL AND PLP AND ROL PLD BIT AND ROL AND '
       db 'BMI AND AND AND BIT AND ROL AND SEC AND DEC TSC BIT AND ROL AND '
       db 'RTI EOR WDM EOR MVP EOR LSR EOR PHA EOR LSR PHK JMP EOR LSR EOR '
       db 'BVC EOR EOR EOR MVN EOR LSR EOR CLI EOR PHY TCD JMP EOR LSR EOR '
       db 'RTS ADC PER ADC STZ ADC ROR ADC PLA ADC ROR RTL JMP ADC ROR ADC '
       db 'BVS ADC ADC ADC STZ ADC ROR ADC SEI ADC PLY TDC JMP ADC ROR ADC '
       db 'BRA STA BRL STA STY STA STX STA DEY BIT TXA PHB STY STA STX STA '
       db 'BCC STA STA STA STY STA STX STA TYA STA TXS TXY STZ STA STZ STA '
       db 'LDY LDA LDX LDA LDY LDA LDX LDA TAY LDA TAX PLB LDY LDA LDX LDA '
       db 'BCS LDA LDA LDA LDY LDA LDX LDA CLV LDA TSX TYX LDY LDA LDX LDA '
       db 'CPY CMP REP CMP CPY CMP DEC CMP INY CMP DEX WAI CPY CMP DEC CMP '
       db 'BNE CMP CMP CMP PEI CMP DEC CMP CLD CMP PHX STP JML CMP DEC CMP '
       db 'CPX SBC SEP SBC CPX SBC INC SBC INX SBC NOP XBA CPX SBC INC SBC '
       db 'BEQ SBC SBC SBC PEA SBC INC SBC SED SBC PLX XCE JSR SBC INC SBC '

; Immediate Addressing Modes :
;   09 - ORA-M, 29 - AND-M, 49 - EOR-M, 69 - ADC-M, 89 - BIT-M,
;   A0 - LDY-X, A2 - LDX-X, A9 - LDA-M, C0 - CPY-X, C2 - REP-B,
;   C9 - CMP-M, E0 - CPX-X, E2 - SEP-B, E9 - SBC-M
;   Extra Addressing Mode Values : B(1-byte only) = 25, X(by X flag) = 26

NEWSYM addrmode
         db 25,09,25,22,04,04,04,19,21,01,05,21,02,02,02,03
         db 15,07,18,23,04,10,10,08,06,13,05,06,02,12,12,14
         db 02,09,03,22,04,04,04,19,21,01,05,21,02,02,02,03
         db 15,07,18,23,10,10,10,08,06,13,05,06,12,12,12,14
         db 21,09,00,22,24,04,04,19,21,01,05,21,02,02,02,03
         db 15,07,18,23,24,10,10,08,06,13,21,06,03,12,12,14
         db 21,09,02,22,04,04,04,19,21,01,05,21,17,02,02,03
         db 15,07,18,23,10,10,10,08,06,13,21,06,20,12,12,14
         db 15,09,16,22,04,04,04,19,06,01,06,21,02,02,02,03
         db 15,07,18,23,10,10,11,08,06,13,06,06,02,12,12,14
         db 26,09,26,22,04,04,04,19,06,01,06,21,02,02,02,03
         db 15,07,18,23,10,10,11,08,06,13,06,06,12,12,13,14
         db 26,09,25,22,04,04,04,19,06,01,06,06,02,02,02,03
         db 15,07,18,23,18,10,10,08,06,13,21,06,27,12,12,14
         db 26,09,25,22,04,04,04,19,06,01,06,06,02,02,02,03
         db 15,07,18,23,02,10,10,08,06,13,21,06,20,12,12,14

NEWSYM spcnametab
            db 'NOP     TCALL   SET1    BBS     '
            db 'OR      OR      OR      OR      '
            db 'OR      OR      OR1     ASL     '
            db 'ASL     PUSH    TSET1   BRK     '

            db 'BPL     TCALL   CLR1    BBC     '
            db 'OR      OR      OR      OR      '
            db 'OR      OR      DECW    ASL     '
            db 'ASL     DEC     CMP     JMP     '

            db 'CLRP    TCALL   SET1    BBS     '
            db 'AND     AND     AND     AND     '
            db 'AND     AND     OR1     ROL     '
            db 'ROL     PUSH    CBNE    BRA     '

            db 'BMI     TCALL   CLR1    BBC     '
            db 'AND     AND     AND     AND     '
            db 'AND     AND     INCW    ROL     '
            db 'ROL     INC     CMP     CALL    '

            db 'SETP    TCALL   SET1    BBS     '
            db 'EOR     EOR     EOR     EOR     '
            db 'EOR     EOR     AND1    LSR     '
            db 'LSR     PUSH    TCLR1   PCALL   '

            db 'BVC     TCALL   CLR1    BBC     '
            db 'EOR     EOR     EOR     EOR     '
            db 'EOR     EOR     CMPW    LSR     '
            db 'LSR     MOV     CMP     JMP     '

            db 'CLRC    TCALL   SET1    BBS     '
            db 'CMP     CMP     CMP     CMP     '
            db 'CMP     CMP     AND1    ROR     '
            db 'ROR     PUSH    DBNZ    RET     '

            db 'BVS     TCALL   CLR1    BBC     '
            db 'CMP     CMP     CMP     CMP     '
            db 'CMP     CMP     ADDW    ROR     '
            db 'ROR     MOV     CMP     RET1    '

            db 'SETC    TCALL   SET1    BBS     '
            db 'ADC     ADC     ADC     ADC     '
            db 'ADC     ADC     EOR1    DEC     '
            db 'DEC     MOV     POP     MOV     '

            db 'BCC     TCALL   CLR1    BBC     '
            db 'ADC     ADC     ADC     ADC     '
            db 'ADC     ADC     SUBW    DEC     '
            db 'DEC     MOV     DIV     XCN     '

            db 'EI      TCALL   SET1    BBS     '
            db 'SBC     SBC     SBC     SBC     '
            db 'SBC     SBC     MOV1    INC     '
            db 'INC     CMP     POP     MOV     '

            db 'BCS     TCALL   CLR1    BBC     '
            db 'SBC     SBC     SBC     SBC     '
            db 'SBC     SBC     MOVW    INC     '
            db 'INC     MOV     DAS     MOV     '

            db 'DI      TCALL   SET1    BBS     '
            db 'MOV     MOV     MOV     MOV     '
            db 'CMP     MOV     MOV1    MOV     '
            db 'MOV     MOV     POP     MUL     '

            db 'BNE     TCALL   CLR1    BBC     '
            db 'MOV     MOV     MOV     MOV     '
            db 'MOV     MOV     MOVW    MOV     '
            db 'DEC     MOV     CBNE    DAA     '

            db 'CLRV    TCALL   SET1    BBS     '
            db 'MOV     MOV     MOV     MOV     '
            db 'MOV     MOV     NOT1    MOV     '
            db 'MOV     NOTC    POP     SLEEP   '

            db 'BEQ     TCALL   CLR1    BBC     '
            db 'MOV     MOV     MOV     MOV     '
            db 'MOV     MOV     MOV     MOV     '
            db 'INC     MOV     DBNZ    STOP    '

; %1 = Byte, %2 = Second Byte, %3 = high nibble of opcode #,
; %4 = high nibble of opcode # and 07h, %5 = relative to PC+2
; %6 = dp ($0/$1)
; %7 = memory SHR 3 Low, %8 = memory SHR 3 High, %9 = ,memory AND 7h
; %A = relative to PC+1, %B = relative to PC+1 at second byte
NEWSYM AddressTable
;                   1               1               1
db '                %3              %6%1,%4         B%4 %6%1,$%B+1  '
; 0 : nothing
; 1 : the high nibble
; 2 : the high nibble first 3 bit (and 0111000 then shift)
; 3 : 2 + relative
db 'A,%6%1          A,$%2%1         A,(X)           A,(%6%1+x)      '
; 4 : A,dp
; 5 : A,labs
; 6 : A,(X)
; 7 : A,(dp+X)
db 'A,#$%1          (%6%2),(%6%1)   CF,mbit%8%7%9   %6%1            '
; 8 : A,#inm
; 9 : dp(d),dp(s)   (two dp)
; 10 : Carry flag, memory bit          (can only access from 0 to 1fff)
; 11 : dp
db '$%2%1           PSW             $%A             A,%6%1+X        '
; 12 : labs
; 13 : PSW
; 14 : rel
; 15 : A,dp+X
db 'A,$%2%1+X       A,$%2%1+Y       A,(%6%1)+Y      %6%2,#$%1       '
; 16 : A,labs+X
; 17 : A,labs+Y
; 18 : A,(dp)+Y
; 19 : dp,#inm
db '(X),(Y)         %6%1+X          A               X               '
; 20 : (X),(Y)
; 21 : dp+X
; 22 : A
; 23 : X
db 'X,%2%1          ($%2%1+X)       CF,/(mb%8%7%9)  %6%1            '
; 24 : X,labs
; 25 : (labs+X)
; 26 : C,/mem.bit
; 27 : upage         (same as dp but for a call)
db 'YA,%6%1         X,A             Y,$%2%1         Y               '
; 28 : YA,dp
; 29 : X,A
; 30 : Y,labs
; 31 : Y
db 'Y,%6%1          Y,#$%1          %6%1,$%B        X,%6%1          '
; 32 : Y,dp
; 33 : Y,#inm
; 34 : dp,rel
; 35 : X,dp
db 'A,X             %6%2,#$%1       X,SP            YA,X            '
; 36 : A,X
; 37 : dp,#inm
; 38 : X,SP
; 39 : YA,X
db '(X)+,A          SP,X            A,(X)+          %6%1,A          '
; 40 : (X)+,A
; 41 : SP,X
; 42 : A,(X)+
; 43 : dp,A
db '$%2%1,A         (X),A           %6%1+X,A        X,#$%1          '
; 44 : labs,A
; 45 : (X),A
; 46 : (dp+X),A
; 47 : X,#inm
db '$%2%1,X         mb%8%7%9,CF     %6%1,Y          $%2%1,Y         '
; 48 : labs,X
; 49 : mem.bit,C
; 50 : dp,Y
; 51 : labs,Y
db 'YA              %6%1+X,A        $%2%1+X,A       $%2%1+Y,A       '
; 52 : YA
; 53 : dp+X,A
; 54 : labs+X,A
; 55 : labs+Y,A
db '(%6%1)+Y,A      %6%1,X          %6%1+Y,X        %6%1,YA         '
; 56 : (dp)+Y,A
; 57 : dp,X
; 58 : dp+Y,X
; 59 : dp,YA
db '%6%1+X,Y        A,Y             %6%2+X,$%A      mb%8%7%9,CF     '
; 60 : dp+X,Y
; 61 : A,Y
; 62 : dp+X,rel
; 63 : mem.bit
db 'X,%6%1+Y        Y,%6%1+X        Y,A             Y,$%A           '
; 64 : X,dp+Y
; 65 : Y,dp+X
; 66 : Y,A
; 67 : Y,rel


NEWSYM ArgumentTable
;     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
   db 00,01,02,03,04,05,06,07,08,09,10,11,12,13,12,00
;     10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
   db 14,01,02,03,15,16,17,18,19,20,11,21,22,23,24,25
;     20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F
   db 00,01,02,03,04,05,06,07,08,09,26,11,12,22,34,14
;     30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
   db 14,01,02,03,15,16,17,18,19,20,11,21,22,23,35,12
;     40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F
   db 00,01,02,03,04,05,06,07,08,09,10,11,12,23,12,27
;     50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F
   db 14,01,02,03,15,16,17,18,19,20,28,21,22,29,30,12
;     60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F
   db 00,01,02,03,04,05,06,07,08,09,26,11,12,31,34,00
;     70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F
   db 14,01,02,03,15,16,17,18,19,20,28,21,22,36,32,00
;     80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F
   db 00,01,02,03,04,05,06,07,08,09,10,11,12,33,13,37
;     90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F
   db 14,01,02,03,15,16,17,18,19,20,28,21,22,38,39,22
;     A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF
   db 00,01,02,03,04,05,06,07,08,09,10,11,12,33,22,40
;     B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF
   db 14,01,02,03,15,16,17,18,19,20,28,21,22,41,22,42
;     C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF
   db 00,01,02,03,43,44,45,46,47,48,49,50,51,47,23,52
;     D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF
   db 14,01,02,03,53,54,55,56,57,58,59,60,31,61,62,22
;     E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF
   db 00,01,02,03,04,05,06,07,08,24,63,32,30,00,31,00
;     F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF
   db 14,01,02,03,15,16,17,18,35,64,09,65,31,66,67,00
SECTION .text
