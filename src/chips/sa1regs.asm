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


; SPC7110 emulation.  Information fully reverse engineered
;    by Dark Force and John Weidman, ZSNES code by zsKnight
; SA-1 emulation.  Information provided by Ivar of Snes9x.
;    coded by zsKnight
; SDD-1 emulation.  SDD-1 MMC reverse engineered by zsKnight,
;    SDD-1 decompress Dark Force and John Weidman,


; - Add MMC support
; - Add end of DMA IRQ support
; - Add Char Conversion #1
; - Add Char Conversion #2
; - Add Memory Disables (Guess this isn't needed)
; IRQs - IRQ Clear (also clear 2300), IRQ Disable, and IRQ Enable

; Mario RPG Level-up not working - it was using one of the IRQ functions
;   that no other place in the game is using, which I suppose is the cause
;   of the problem, but it's been so long since I worked on SA-1, that
;   I forgot which part.



%include "macros.mac"

EXTSYM regptr,regptw,romdata,SA1Status,debstop4,SDD1BankA,curromsize,debuggeron
EXTSYM Get_Time,Get_TimeDate,spc7110romptr,SPC7110Entries,SPC7110IndexSize
EXTSYM SPC7110nfname,SPC7110filep,SPC7_Convert_Upper,SPC7_Convert_Lower
EXTSYM Open_File,Close_File,Read_File,File_Seek,irqv2,irqv,nmiv2,nmiv,snesmmap
EXTSYM snesmap2,curypos,CurrentExecSA1,memaccessbankr8sdd1,memtabler8,AddrNoIncr
EXTSYM NumofBanks,BWUsed2

%include "cpu/regs.mac"
%include "cpu/regsw.mac"


SECTION .data

NEWSYM SPCMultA, dd 0
NEWSYM SPCMultB, dd 0
NEWSYM SPCDivEnd, dd 0
NEWSYM SPCMulRes, dd 0
NEWSYM SPCDivRes, dd 0
NEWSYM SPC7110BankA, dd 020100h
NEWSYM SPC7110RTCStat, dd 0
NEWSYM SPC7110RTC, db 00,00,00,00,00,00,01,00,01,00,00,00,00,00,0Fh,00
NEWSYM SPC7110RTCB, db 00,00,00,00,00,00,01,00,01,00,00,00,00,01,0Fh,06
NEWSYM SPCROMPtr, dd 0
NEWSYM SPCROMtoI, dd SPCROMPtr
NEWSYM SPCROMAdj, dd 0
NEWSYM SPCROMInc, dd 0
NEWSYM SPCROMCom, dd 0
NEWSYM SPCCompPtr, dd 0
NEWSYM SPCDecmPtr, dd 0
NEWSYM SPCCompCounter, dd 0
NEWSYM SPCCompCommand, dd 0
NEWSYM SPCCheckFix, dd 0
NEWSYM SPCSignedVal, dd 0
num2writespc7110reg equ $-SPCMultA
NEWSYM PHnum2writespc7110reg, dd num2writespc7110reg

NEWSYM RTCData, db 0Fh,0,0,0,0,0,0,0,0,0,0,0,0,0,0Fh,0
NEWSYM RTCPtr, dd 0
NEWSYM RTCPtr2, dd 0
NEWSYM RTCRest, dd 0
NEWSYM SPC7110TempPosition, dd 0
NEWSYM SPC7110TempLength, dd 0
NEWSYM SPCPrevCompPtr, dd 0

SECTION .text

RTC2800:
    push ebx
    cmp dword[RTCRest],100
;    je .go
;    inc dword[RTCRest]
;    jmp .notfirst
.go
    cmp dword[RTCPtr],0
    jne near .notfirst
    cmp byte[debuggeron],1
    je near .notfirst
    ; fill time/date
    push ebx
    push eax
    call Get_Time
    mov bl,al
    and bl,0Fh
    mov [RTCData+1],bl  ; seconds
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [RTCData+2],bl
;    jmp .done
    shr eax,4
    cmp word[RTCData+1],0
;    jne .notminch
    mov bl,al
    and bl,0Fh
    mov [RTCData+3],bl  ; minutes
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [RTCData+4],bl
.notminch
;    jmp .done
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [RTCData+5],bl  ; hours
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [RTCData+6],bl
    call Get_TimeDate
    mov bl,al
    and bl,0Fh
    mov [RTCData+7],bl  ; day
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov bl,al
    and bl,0Fh
    mov [RTCData+8],bl
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [RTCData+9],bl  ; month
    shr eax,8
    mov bl,al
    and bl,0Fh
    mov [RTCData+10],bl  ; year
    shr eax,4
    mov bl,al
    and bl,01Fh
    xor bh,bh
.notokay
    cmp bl,9
    jbe .okay
    inc bh
    sub bl,10
    jmp .notokay
.okay
    mov [RTCData+11],bl
    add bh,9
    mov [RTCData+12],bh
    shr eax,8
    and al,0Fh
    mov [RTCData+13],al ; day of week
.done
    pop eax
    pop ebx
.notfirst
    mov ebx,[RTCPtr]
    mov al,[RTCData+ebx]
    inc dword[RTCPtr]
    cmp dword[RTCPtr],0Fh
    jne .notclear
    mov dword[RTCPtr],0
.notclear
    pop ebx
    ret

RTC2801w:
;    mov byte[debstop3],1
    mov dword[RTCRest],0
    mov dword[RTCPtr],0
    cmp al,0Eh
    jne .notreset
    mov dword[RTCPtr2],0
    ret
.notreset
    cmp al,0Dh
    jne .notstop
    mov dword[RTCPtr2],0
    ret
.notstop
    push ebx
    mov ebx,[RTCPtr2]
    cmp ebx,0
    je .next
    cmp ebx,13
    ja .nomore
    mov [RTCData+ebx],al
.next
    inc dword[RTCPtr2]
.nomore
    pop ebx
    ret

NEWSYM RTCinit
    mov dword[RTCPtr],0
    ret
NEWSYM RTCReset
    setreg 2800h*4,RTC2800
    ret
NEWSYM RTCReset2
    setregw 2801h*4,RTC2801w
    ret

SPC4850:
    mov al,[SPC7110RTC]
    ret
SPC4851:
    mov al,[SPC7110RTC+01h]
    ret
SPC4852:
    mov al,[SPC7110RTC+02h]
    ret
SPC4853:
    mov al,[SPC7110RTC+03h]
    ret
SPC4854:
    mov al,[SPC7110RTC+04h]
    ret
SPC4855:
    mov al,[SPC7110RTC+05h]
    ret
SPC4856:
    mov al,[SPC7110RTC+06h]
    ret
SPC4857:
    mov al,[SPC7110RTC+07h]
    ret
SPC4858:
    mov al,[SPC7110RTC+08h]
    ret
SPC4859:
    mov al,[SPC7110RTC+09h]
    ret
SPC485A:
    mov al,[SPC7110RTC+0Ah]
    ret
SPC485B:
    mov al,[SPC7110RTC+0Bh]
    ret
SPC485C:
    mov al,[SPC7110RTC+0Ch]
    ret
SPC485D:
    mov al,[SPC7110RTC+0Dh]
    ret
SPC485E:
    mov al,[SPC7110RTC+0Eh]
    ret
SPC485F:
    mov al,[SPC7110RTC+0Fh]
    ret

SECTION .bss
NEWSYM SPCDecompFin, resd 1

SECTION .text

NEWSYM SPC7110init
    mov dword[SPCMultA],0
    mov dword[SPCMultB],0
    mov dword[SPCDivEnd],0
    mov dword[SPCMulRes],0
    mov dword[SPCDivRes],0
    mov dword[SPC7110BankA],020100h
    mov dword[SPC7110RTCStat],0
    mov dword[SPC7110RTCStat],0
    mov dword[SPCROMPtr],0
    mov dword[SPCROMtoI],SPCROMPtr
    mov dword[SPCROMAdj],0
    mov dword[SPCROMInc],0
    mov dword[SPCROMCom],0
    mov dword[SPCDecompFin],0
    mov dword[SPCCompPtr],0
    mov dword[SPCDecmPtr],0
    mov dword[SPCCompCounter],0
    mov dword[SPCCompCommand],0
    mov dword[SPCCheckFix],0
    mov dword[SPCPrevCompPtr],0
    ret

NEWSYM SPC7110Reset
    setregw 4801h*4,SPC4801w
    setregw 4802h*4,SPC4802w
    setregw 4803h*4,SPC4803w
    setregw 4804h*4,SPC4804w
    setregw 4805h*4,SPC4805w
    setregw 4806h*4,SPC4806w
    setregw 4807h*4,SPC4807w
    setregw 4808h*4,SPC4808w
    setregw 4809h*4,SPC4809w
    setregw 480Ah*4,SPC480Aw
    setregw 480Bh*4,SPC480Bw

    setregw 4811h*4,SPC4811w
    setregw 4812h*4,SPC4812w
    setregw 4813h*4,SPC4813w
    setregw 4814h*4,SPC4814w
    setregw 4815h*4,SPC4815w
    setregw 4816h*4,SPC4816w
    setregw 4817h*4,SPC4817w
    setregw 4818h*4,SPC4818w

    setregw 4820h*4,SPC4820w
    setregw 4821h*4,SPC4821w
    setregw 4822h*4,SPC4822w
    setregw 4823h*4,SPC4823w
    setregw 4824h*4,SPC4824w
    setregw 4825h*4,SPC4825w
    setregw 4826h*4,SPC4826w
    setregw 4827h*4,SPC4827w
    setregw 482Eh*4,SPC482Ew

    setregw 4831h*4,SPC4831w
    setregw 4832h*4,SPC4832w
    setregw 4833h*4,SPC4833w

    setregw 4840h*4,SPC4840w
    setregw 4841h*4,SPC4841w
    setregw 4842h*4,SPC4842w
    ret

NEWSYM initSPC7110regs
    setreg 4800h*4,SPC4800
    setreg 4801h*4,SPC4801
    setreg 4802h*4,SPC4802
    setreg 4803h*4,SPC4803
    setreg 4804h*4,SPC4804
    setreg 4805h*4,SPC4805
    setreg 4806h*4,SPC4806
    setreg 4807h*4,SPC4807
    setreg 4808h*4,SPC4808
    setreg 4809h*4,SPC4809
    setreg 480Ah*4,SPC480A
    setreg 480Bh*4,SPC480B
    setreg 480Ch*4,SPC480C

    setreg 4810h*4,SPC4810
    setreg 4811h*4,SPC4811
    setreg 4812h*4,SPC4812
    setreg 4813h*4,SPC4813
    setreg 4814h*4,SPC4814
    setreg 4815h*4,SPC4815
    setreg 4816h*4,SPC4816
    setreg 4817h*4,SPC4817
    setreg 4818h*4,SPC4818
    setreg 481Ah*4,SPC481A

    setreg 4820h*4,SPC4820
    setreg 4821h*4,SPC4821
    setreg 4822h*4,SPC4822
    setreg 4823h*4,SPC4823
    setreg 4824h*4,SPC4824
    setreg 4825h*4,SPC4825
    setreg 4826h*4,SPC4826
    setreg 4827h*4,SPC4827
    setreg 4828h*4,SPC4828
    setreg 4829h*4,SPC4829
    setreg 482Ah*4,SPC482A
    setreg 482Bh*4,SPC482B
    setreg 482Ch*4,SPC482C
    setreg 482Dh*4,SPC482D
    setreg 482Eh*4,SPC482E
    setreg 482Fh*4,SPC482F

    setreg 4831h*4,SPC4831
    setreg 4832h*4,SPC4832
    setreg 4833h*4,SPC4833
    setreg 4834h*4,SPC4834

    setreg 4840h*4,SPC4840
    setreg 4841h*4,SPC4841
    setreg 4842h*4,SPC4842


    setreg 4850h*4,SPC4850
    setreg 4851h*4,SPC4851
    setreg 4852h*4,SPC4852
    setreg 4853h*4,SPC4853
    setreg 4854h*4,SPC4854
    setreg 4855h*4,SPC4855
    setreg 4856h*4,SPC4856
    setreg 4857h*4,SPC4857
    setreg 4858h*4,SPC4858
    setreg 4859h*4,SPC4859
    setreg 485Ah*4,SPC485A
    setreg 485Bh*4,SPC485B
    setreg 485Ch*4,SPC485C
    setreg 485Dh*4,SPC485D
    setreg 485Eh*4,SPC485E
    setreg 485Fh*4,SPC485F
    ret

%macro BankSwitchSPC7110 2
    push ecx
    push edx
    push eax
    mov [SPC7110BankA+%1],al
    inc al
    cmp byte[curromsize],13
    jne .mbit24
.mbit40
    cmp al,5
    jb .okaymbit
    sub al,4
    jmp .mbit40
.mbit24
    cmp al,3
    jb .okaymbit
    sub al,2
    jmp .mbit24
.okaymbit
    and eax,07h
    shl eax,20
    add eax,[romdata]
    mov ecx,10h
    mov ebx,snesmap2+%2*4
    mov edx,snesmmap+%2*4
.mmaploop2
    mov [ebx],eax
    mov [edx],eax
    add eax,10000h
    add ebx,4
    add edx,4
    dec ecx
    jnz .mmaploop2
    pop eax
    pop edx
    pop ecx
    ret
%endmacro

NEWSYM LastLog
    pushad
    cmp byte[CurValUsed],0
    je near .novalue
    xor ebx,ebx
    mov edx,DecompArray
    mov eax,[CurPtrVal]
    cmp dword[DecompAPtr],0
    je .noptr
.loop
    mov ecx,[edx]
    cmp ecx,eax
    je .match
    add edx,8
    inc ebx
    cmp ebx,[DecompAPtr]
    jne .loop
.noptr
    cmp dword[DecompAPtr],8192
    je .novalue
    mov [edx],eax
    xor eax,eax
    mov ax,[CurDecompSize]
    mov [edx+4],ax
    mov ax,[CurPtrLen]
    mov [edx+6],ax
    mov al,[CurPtrLen+2]
    mov [edx+3],al
    inc dword[DecompAPtr]
    jmp .novalue
.match
    add edx,4
    mov bx,[CurDecompSize]
    xor ebx,ebx
    cmp [edx],bx
    jae .novalue
    mov [edx],bx
.novalue
    mov [lastentry],edx
    mov byte[CurValUsed],1
    mov eax,[SPCCompPtr]
    and eax,0FFFFFFh
    mov [CurPtrVal],eax
    popad
    ret


SPC4800:
;    mov byte[debstop3],1
;    cmp word[SPCCompCounter],0FFFFh
;    jne .notzero
;    xor al,al
;    ret
;.notzero
    cmp byte[SPCCompCommand],0
    je .manual
    xor al,al
    dec word[SPCCompCounter]
    push ebx
    xor ebx,ebx
;    mov ebx,[SPCCompPtr]
;    and ebx,0FFFFFFh
;    add ebx,[romdata]
;    add ebx,100000h
    mov bx,[SPCDecmPtr]
    add ebx,[romdata]
    add ebx,510000h
    mov al,[ebx]
    pop ebx
;    xor al,al
    inc dword[SPCCompPtr]

    push eax
    inc word[SPCDecmPtr]
    mov ax,[SPCDecmPtr]
    mov [CurDecompPtr],ax
    sub ax,[PrevDecompPtr]
    mov [CurDecompSize],ax
    pop eax
;    cmp word[SPCCompCounter],0FFFFh
;    jne .exit
;    mov byte[SPCDecompFin],80h
;.exit
    ret
.manual
    xor al,al
    push ebx
    xor ebx,ebx
    mov bx,[SPCDecmPtr]
    add ebx,[romdata]
    add ebx,510000h
    mov al,[ebx]
    pop ebx

    dec word[SPCCompCounter]
    inc dword[SPCCompPtr]
    inc word[SPCDecmPtr]
    inc word[CurDecompSize]
;    cmp word[SPCCompCounter],0FFFFh
;    jne .exit2
;    mov byte[SPCDecompFin],80h
;.exit2
    ret
SPC4801:
    mov al,[SPCCompPtr]
    ret
SPC4802:
    mov al,[SPCCompPtr+1]
    ret
SPC4803:
    mov al,[SPCCompPtr+2]
    ret
SPC4804:
    mov al,[SPCCompPtr+3]
    ret
SPC4805:
    mov al,[SPCDecmPtr]
    ret
SPC4806:
    mov al,[SPCDecmPtr+1]
    ret
SPC4807:
    xor al,al
    ret
SPC4808:
    xor al,al
    ret
SPC4809:
    mov al,[SPCCompCounter]
    ret
SPC480A:
    mov al,[SPCCompCounter+1]
    ret
SPC480B:
    mov al,[SPCCompCommand]
    mov dword[SPCDecmPtr],0
    ret
SPC480C:        ; decompression finished status
    mov al,[SPCDecompFin]
    mov byte[SPCDecompFin],0
    ret

SECTION .bss
NEWSYM CurPtrVal, resd 1
NEWSYM CurCompCounter2, resd 1
NEWSYM CurPtrLen, resd 1
NEWSYM CurValUsed, resb 1
NEWSYM PrevDecompPtr, resw 1
NEWSYM CurDecompPtr, resw 1
NEWSYM CurDecompSize, resw 1
NEWSYM DecompArray, resb 65536
NEWSYM DecompAPtr, resd 1
lastentry resd 1

SECTION .text


NEWSYM UpdateRTC
    test byte[SPC7110RTC+0Dh],02h
    jnz .notimer
.notimer
    ret

SPC4801w:
    mov [SPCCompPtr],al
    ret
SPC4802w:
    mov [SPCCompPtr+1],al
    ret
SPC4803w:
    mov [SPCCompPtr+2],al
    ret
SPC4804w:
    mov [SPCCompPtr+3],al
    ret
SPC4805w:
    mov [SPCDecmPtr],al
    ret
SPC4806w:
    mov [SPCDecmPtr+1],al
    cmp dword[SPCCompPtr],0124AD48h
    jne .nodata
;    mov byte[debstop3],1
.nodata

    pushad
    cmp byte[CurValUsed],0
    je near .novalue
    xor ebx,ebx
    mov edx,DecompArray
    mov eax,[CurPtrVal]
;    and eax,0FFFFFFh

    cmp dword[DecompAPtr],0
    je .noptr
.loop
    mov ecx,[edx]
;    and ecx,0FFFFFFh
    cmp ecx,eax
    je .match
    add edx,8
    inc ebx
    cmp ebx,[DecompAPtr]
    jne .loop
.noptr
    cmp dword[DecompAPtr],8192
    je .novalue
    mov [edx],eax
    xor eax,eax
    mov ax,[CurDecompSize]
    mov [edx+4],ax
    mov ax,[CurPtrLen]
    mov [edx+6],ax
    mov al,[CurPtrLen+2]
    mov [edx+3],al
    inc dword[DecompAPtr]
    jmp .novalue
.match
    add edx,4
    xor ebx,ebx
    mov bx,[CurDecompSize]
    cmp [edx],bx
    jae .novalue
    mov [edx],bx
.novalue
    mov [lastentry],edx
    mov byte[CurValUsed],1
    mov eax,[SPCCompPtr]
    and eax,0FFFFFFh
    mov [CurPtrVal],eax
    popad
    mov word[CurDecompSize],0

    push eax
    mov al,[SPCCompPtr+3]
    mov [CurPtrLen+2],al
    mov ax,[SPCDecmPtr]  ;CurCompCounter2]
    mov [CurPtrLen],ax
    mov eax,[SPCCompPtr]
    mov [CurPtrVal],eax

    mov ax,[SPCDecmPtr]
    mov [PrevDecompPtr],ax
    mov [CurDecompPtr],ax
    mov word[CurDecompSize],0
    pop eax

    mov byte[SPCDecompFin],0h
    ; Start Decompression

    pushad
    mov eax,[SPCCompPtr]
    cmp [SPCPrevCompPtr],eax
    je near .previousequal
    mov [SPCPrevCompPtr],eax

    mov ecx,[SPC7110Entries]
    mov ebx,[SPCCompPtr]
    and ebx,0FFFFFFh
    mov eax,[spc7110romptr]
    or ecx,ecx
    jz .noentries
.loopc
    mov edx,[eax]
    cmp dl,[SPCCompPtr+3]
    jne .notfound
    shr edx,8
    cmp ebx,edx
    je .found
.notfound
    add eax,12
    dec ecx
    jnz .loopc
    jmp .noentries
.found
    xor word[CurPtrLen],0FFFFh
    mov ecx,[eax+8]
    mov ebx,[eax+4]
    xor edx,edx
    mov dx,[SPCDecmPtr]
    add edx,[romdata]
    add edx,510000h
    push eax
.loopb
    mov al,[ebx]
    mov [edx],al
    inc ebx
    inc edx
    dec ecx
    jnz .loopb
    pop eax
    mov ebx,[eax+4]
    mov edx,[lastentry]
;    mov [edx+4],ebx
    mov ebx,[eax]
;    mov [edx],ebx
    jmp .foundentry
.noentries

    mov ecx,[SPC7110IndexSize]
    ; Address/index, pointer, length, SPC7110nfname
    mov edx,[romdata]
    add edx,580000h
.sploop
    mov eax,[SPCCompPtr]
    shl eax,8
    mov al,[SPCCompPtr+3]
    cmp [edx],eax
    je .foundsp
    add edx,12
    sub ecx,12
    jc .overflow
    jnz .sploop
.overflow
    jmp .notfoundentry
.foundsp
    mov eax,[edx+4]
    mov [SPC7110TempPosition],eax
    mov eax,[edx+8]
    mov [SPC7110TempLength],eax

    mov edx,[SPC7110filep]
    mov eax,[SPCCompPtr]
    and eax,0FFFFFFh
    mov ecx,6
.sploop2
    mov ebx,eax
    shr ebx,20
    and ebx,0Fh
    cmp bl,9
    jbe .below9
    add bl,55-48
.below9
    add bl,48
    mov [edx],bl
    inc edx
    shl eax,4
    dec ecx
    jnz .sploop2

    mov edx,SPC7110nfname
    call Open_File
    jnc .nocaseerror
    pushad
    call SPC7_Convert_Upper
    popad
    call Open_File
    jnc .nocaseerror
    pushad
    call SPC7_Convert_Lower
    popad
    call Open_File
    jc .error
.nocaseerror
    mov bx,ax
    mov dx,[SPC7110TempPosition]
    mov cx,[SPC7110TempPosition+2]
    call File_Seek
    xor edx,edx
    mov dx,[SPCDecmPtr]
    add edx,[romdata]
    add edx,510000h
    mov ecx,[SPC7110TempLength]
    call Read_File
    call Close_File
    jmp .foundentry
.error
;    mov dword[Msgptr],SPC7110nfname
;    mov dword[MessageOn],60*6
.notfoundentry
.foundentry
.previousequal
    popad
.fin
.blah
    ; Finished
;    mov word[SPCCompCounter],0FFFFh
    mov byte[SPCDecompFin],80h
    ret
SPC4807w:
    ret
SPC4808w:
    ret
SPC4809w:
    mov [SPCCompCounter],al
    mov [CurCompCounter2],al
    ret
SPC480Aw:
    mov [SPCCompCounter+1],al
    mov [CurCompCounter2+1],al
    ret
SPC480Bw:
    mov [SPCCompCommand],al
    ret

; 01,
;$4810   DATA ROM CONTINUOUS READ PORT: returns a byte from data rom at data
;            rom pointer location, defval:00
;$4811   DATA ROM POINTER: ($0000FF) r/w low offset, defval:00
;$4812   DATA ROM POINTER: ($00FF00) r/w high offset, defval:00
;$4813   DATA ROM POINTER: ($FF0000) r/w bank offset, defval:00
;            bank offset is zero based from start of data rom: banks $00-$3f
;            data rom -> $10-$4f full rom
;$4814   DATA ROM POINTER ADJUST: ($00FF) low byte, defval:00
;$4815   DATA ROM POINTER ADJUST: ($FF00) high byte, defval:00
;$4816   DATA ROM POINTER INCREMENT: ($00FF) low byte, defval:00
;$4817   DATA ROM POINTER INCREMENT: ($FF00) high byte, defval:00
;$4818   DATA ROM COMMAND MODE: bit field control of data rom pointer (see
;            data rom command mode byte), defval:00
;            write: set command mode,
;            read: performs action instead of returning value, unknown purpose,
;            command mode is loaded to $4818 but only set after writing to both
;            $4814 and $4815 in any order
;$481A   DATA ROM READ AFTER ADJUST PORT: returns a byte from data rom at
;            data rom pointer location + adjust value ($4814/5), defval:00


SPC4810:
    cmp dword[SPCCheckFix],0
    jne .okay
    xor al,al
    ret
.okay
    push ebx
    push ecx
    mov ebx,[SPCROMPtr]
    add ebx,[romdata]
    add ebx,100000h
    mov al,[ebx]
    cmp byte[SPCROMCom+1],0
    jne .noincr1
    mov ebx,[SPCROMtoI]
    inc dword[ebx]
.noincr1
    cmp byte[SPCROMCom+1],1     ; add 4816 after 4810 read
    jne .noincr1b
    mov ebx,[SPCROMtoI]
    mov ecx,[SPCROMInc]
    add dword[ebx],ecx
.noincr1b
    pop ecx
    pop ebx
    ret
SPC4811:
    mov al,[SPCROMPtr]
    ret
SPC4812:
    mov al,[SPCROMPtr+1]
    ret
SPC4813:
    mov al,[SPCROMPtr+2]
    ret
SPC4814:
    mov al,[SPCROMAdj]
    ret
SPC4815:
    mov al,[SPCROMAdj+1]
    ret
SPC4816:
    mov al,[SPCROMInc]
    ret
SPC4817:
    mov al,[SPCROMInc+1]
    ret
SPC4818:
    mov al,[SPCROMCom]
    ret
SPC481A:
    cmp dword[SPCCheckFix],0
    jne .okay
    xor al,al
    ret
.okay
    push ebx
    push ecx
    xor ebx,ebx
    xor ecx,ecx
    mov bx,[SPCROMAdj]
    add ebx,[SPCROMPtr]
    add ebx,[romdata]
    add ebx,100000h
    mov al,[ebx]

    cmp byte[SPCROMCom+1],4     ; 16bit 4814
    jne .notincr
    mov ecx,[SPCROMtoI]
    mov ebx,[SPCROMAdj]
    add [ecx],ebx
.notincr
    pop ecx
    pop ebx
    ret

SPC4811w:
    mov [SPCROMPtr],al
    mov byte[SPCCheckFix],1
    ret
SPC4812w:
    mov [SPCROMPtr+1],al
    ret
SPC4813w:
    mov [SPCROMPtr+2],al
    ret
SPC4814w:
    mov [SPCROMAdj],al
    cmp byte[SPCROMCom+1],2     ; 8 bit 4814
    jne .notincr
    mov ebx,[SPCROMtoI]
    xor ecx,ecx
    mov cl,[SPCROMAdj]
    test byte[SPCROMCom],08h
    jz .noneg
    movsx ecx,byte[SPCROMAdj]
.noneg
    add dword[ebx],ecx
.notincr
    ret
SPC4815w:
    mov [SPCROMAdj+1],al
    mov word[SPCROMAdj+2],0
    test byte[SPCROMCom],08h
    jz .noneg
    test byte[SPCROMAdj+1],80h
    jz .noneg
    mov word[SPCROMAdj+2],0FFFFh
.noneg
    cmp byte[SPCROMCom+1],3     ; 16bit 4814
    jne .notincr
    push ebx
    push ecx
    mov ecx,[SPCROMtoI]
    mov ebx,[SPCROMAdj]
    add [ecx],ebx
    pop ecx
    pop ebx
.notincr
    ret
SPC4816w:
    mov [SPCROMInc],al
    ret
SPC4817w:
    mov [SPCROMInc+1],al
    mov word[SPCROMInc+2],0
    test byte[SPCROMCom],04h
    jz .noneg
    test byte[SPCROMInc+1],40h
    jz .noneg
    mov word[SPCROMInc+2],0FFFFh
.noneg
    ret
SPC4818w:
    mov [SPCROMCom],al
    mov word[SPCROMAdj+2],0
    test byte[SPCROMCom],08h
    jz .noneg
    test byte[SPCROMAdj+1],80h
    jz .noneg
    mov word[SPCROMAdj+2],0FFFFh
.noneg
    mov word[SPCROMInc+2],0
    test byte[SPCROMCom],04h
    jz .noneg2
    test byte[SPCROMInc+1],40h
    jz .noneg2
    mov word[SPCROMInc+2],0FFFFh
.noneg2
    mov dword[SPCROMtoI],SPCROMPtr
    test byte[SPCROMCom],10h
    jz .nouseadjust
    mov dword[SPCROMtoI],SPCROMAdj
.nouseadjust
    test al,02h
    jz .no4814
    test al,40h
    jz .no16b
    test al,20h
    jz .not481A
    mov byte[SPCROMCom+1],4     ; 16bit 4814 after 481A
    jmp .fin
.not481A
    mov byte[SPCROMCom+1],3     ; 16bit 4814
    jmp .fin
.no16b
    test al,20h
    jz .nooffsetadd
    mov byte[SPCROMCom+1],2     ; 8 bit 4814
    jmp .fin
.nooffsetadd
    mov byte[SPCROMCom+1],0FFh
    jmp .fin
.no4814
    test al,01h
    jz .incrval0
    mov byte[SPCROMCom+1],1     ; add 4816 after 4810 read
    jmp .fin
.incrval0
    mov byte[SPCROMCom+1],0     ; add 1 after 4810 read
.fin
    ret

;Data Rom Command Mode Byte:
;X6543210
;||||||||
;|||||| \__ : 00 - use 1 as the offset increment value, add immediately after reading $4810
;||||||     : 01 - use $4816 as offset increment, add immediately after reading $4810
;||||||     : 10 - use $4814 as offset increment, see below for when to add
;||||||     : 11 - unused
;||||||____ : 0 - unsigned calculation for $4816
;|||||        1 - signed calculation for $4816
;|||||_____ : 0 - unsigned calculation for $4814
;||||         1 - signed calculation for $4814
;||||______ : 0 - offset increment gets added to $4811/2/3
;|||          1 - offset increment gets added to $4814/5
;| \_______ : 00 - disable offset addition
;|          : 01 - 8 bit offset addition using $4814, immediately after writing to $4814/5
;|          : 10 - 16 bit offset addition using $4814/5, immediately after writing to $4814/5
;|          : 11 - 16 bit offset addition using $4814/5, only after reading $481A
;|_________ : unused

SPC4820:
    mov al,[SPCMultA]
;    mov byte[debstop3],1
    ret
SPC4821:
    mov al,[SPCMultA+1]
    ret
SPC4822:
    mov al,[SPCMultA+2]
    ret
SPC4823:
    mov al,[SPCMultA+3]
    ret
SPC4824:
    mov al,[SPCMultB]
    ret
SPC4825:
    mov al,[SPCMultB+1]
    ret
SPC4826:
    mov al,[SPCDivEnd]
    ret
SPC4827:
    mov al,[SPCDivEnd+1]
    ret

SPC4820w:
    mov [SPCMultA],al
    ret
SPC4821w:
    mov [SPCMultA+1],al
    ret
SPC4822w:
    mov [SPCMultA+2],al
    ret
SPC4823w:
    mov [SPCMultA+3],al
    ret
SPC4824w:
    mov [SPCMultB],al
    ret
SPC4825w:
    mov [SPCMultB+1],al
    ; Calculate SPCMultA*SPCMultB -> SPCMulRes
    test byte[SPCSignedVal],1
    jnz .signed
    push edx
    push eax
    push ebx
    xor eax,eax
    xor ebx,ebx
    mov ax,[SPCMultA]
    mov bx,[SPCMultB]
    mul ebx
    mov [SPCMulRes],eax
    pop ebx
    pop eax
    pop edx
    ret
.signed
    push edx
    push eax
    push ebx
    movsx eax,word[SPCMultA]
    movsx ebx,word[SPCMultB]
    imul ebx
    mov [SPCMulRes],eax
    pop ebx
    pop eax
    pop edx
    ret
SPC4826w:
    mov [SPCDivEnd],al
    ret
SPC4827w:
    mov [SPCDivEnd+1],al
    ; Calculte SPCMultA/SPCDivEnd -> SPCMulRes, rem SPCDivRes
    cmp word[SPCDivEnd],0
    je near .nodivide
    test byte[SPCSignedVal],1
    jnz .signed
    push edx
    push eax
    push ebx
    xor edx,edx
    xor ebx,ebx
    mov eax,[SPCMultA]
    mov bx,[SPCDivEnd]
    div ebx
    mov [SPCMulRes],eax
    mov [SPCDivRes],dx
    pop ebx
    pop eax
    pop edx
    ret
.signed
    push edx
    push eax
    push ebx
    xor edx,edx
    mov eax,[SPCMultA]
    test eax,80000000h
    jz .nd
    mov edx,0FFFFFFFFh
.nd
    movsx ebx,word[SPCDivEnd]
    idiv ebx
    mov [SPCMulRes],eax
    mov [SPCDivRes],dx
    pop ebx
    pop eax
    pop edx
    ret
.nodivide
    mov dword[SPCMulRes],0FFFFFFFFh
    mov dword[SPCDivRes],0FFFFh
    ret
SPC4828:
    mov al,[SPCMulRes]
    ret
SPC4829:
    mov al,[SPCMulRes+1]
    ret
SPC482A:
    mov al,[SPCMulRes+2]
    ret
SPC482B:
    mov al,[SPCMulRes+3]
    ret
SPC482C:
    mov al,[SPCDivRes]
    ret
SPC482D:
    mov al,[SPCDivRes+1]
    ret
SPC482E:
    xor al,al
    ret
SPC482Ew:
    mov [SPCSignedVal],al
    mov dword[SPCMultA],0
    mov dword[SPCMultB],0
    mov dword[SPCDivEnd],0
    mov dword[SPCMulRes],0
    mov dword[SPCDivRes],0
    ret
SPC482F:
    xor al,al
    ret

SPC4831w:
    BankSwitchSPC7110 0,0D0h
    ret
SPC4832w:
    BankSwitchSPC7110 1,0E0h
    ret
SPC4833w:
;    mov byte[debstop3],1
    BankSwitchSPC7110 2,0F0h
    ret
SPC4831:
    mov al,[SPC7110BankA]
    ret
SPC4832:
    mov al,[SPC7110BankA+1]
    ret
SPC4833:
    mov al,[SPC7110BankA+2]
    ret

SPC4834:
    xor al,al
    ret

;$4840   RTC CHIP ENABLE/DISABLE: bit 0: on = enable, off = disable, defval:00
;$4841   RTC INDEX/DATA PORT:
;            first write after rtc enable: rtc command mode byte (see rtc command modes)
;            subsequent writes: index of rtc register to read/write (00-0f)
;            read: returns value of indexed rtc register
;            auto-increment of register index occurs after each subsequent read/write
;$4842   RTC READY STATUS: bit 7: on = ready, off = still processing, tested before reading rtc data
;            high bit cleared after successful read

SPC4840w:
    test al,1
    jz .notreset
    mov [SPC7110RTCStat],al
    mov byte[SPC7110RTCStat+1],0FEh
.notreset
    ret
SPC4841w:
    cmp byte[SPC7110RTCStat+1],0FEh
    je .commandbyte
    cmp byte[SPC7110RTCStat+1],0FFh
    je .commandindex
    push ebx
    xor ebx,ebx
    mov bl,[SPC7110RTCStat+1]
    mov [SPC7110RTC+ebx],al
    cmp ebx,0Fh
    jne .notlast
    test al,01h
    jz .notlast
    mov dword[SPC7110RTC],0
    mov dword[SPC7110RTC+4],010000h
    mov dword[SPC7110RTC+8],01h
    mov byte[SPC7110RTC+12],0
.notlast
    pop ebx
    inc byte[SPC7110RTCStat+1]
    and byte[SPC7110RTCStat+1],0Fh
    ret
.commandbyte
    inc byte[SPC7110RTCStat+1]
    mov [SPC7110RTCStat+2],al
    ret
.commandindex
    push eax
    and al,0Fh
    mov [SPC7110RTCStat+1],al
    pop eax
    ret
SPC4842w:
    ret
SPC4840:
    mov al,[SPC7110RTCStat]
    ret
SPC4841:
    cmp byte[SPC7110RTCStat+1],0FEh
    je near .commandbyte
    cmp byte[SPC7110RTCStat+1],0FFh
    je near .commandbyte
    push ebx
    xor ebx,ebx
    mov bl,[SPC7110RTCStat+1]
    or ebx,ebx
    jnz near .dontupdate
    test byte[SPC7110RTC+0Fh],03h
    jnz near .dontupdate
    test byte[SPC7110RTC+0Dh],01h
    jnz near .dontupdate
;00 - seconds 1's digit                 00
;01 - seconds 10's digit                00
;02 - minutes 1's digit                 00
;03 - minutes 10's digit                00
;04 - hours 1's digit                   00
;05 - hours 10's digit                  00
;06 - day of month 1's digit            01
;07 - day of month 10's digit           00
;08 - month 1's digit                   01
;09 - month 10's digit                  00
;0a - year 1's digit                    00
;0b - year 10's digit                   00
;0c - day of week                       00

    cmp byte[debuggeron],1
    je near .dontupdate
    ; fill time/date
    push ebx
    push eax
    call Get_Time
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC],bl  ; seconds
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+1],bl
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+2],bl  ; minutes
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+3],bl
    test byte[SPC7110RTC+0Fh],4
;    jz .not24hrs
;    jmp .not24hrs
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+4],bl  ; hours
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+5],bl
    jmp .24hrs
.not24hrs
    shr eax,4
    xor ebx,ebx
    mov bl,al
    mov al,[SPCTimerVal+ebx]
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+4],bl  ; hours
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+5],bl
.24hrs
    call Get_TimeDate
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+6],bl  ; day
    shr eax,4
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+7],bl
    shr eax,4
    mov bl,al
    and bl,0Fh
    xor bh,bh
    cmp bl,9
    jbe .less
    sub bl,10
    mov bh,1
.less
    mov [SPC7110RTC+8],bl  ; month
    mov [SPC7110RTC+9],bh  ; month
    shr eax,8
    mov bl,al
    and bl,0Fh
    mov [SPC7110RTC+10],bl  ; year
    shr eax,4
    mov bl,al
    and bl,01Fh
    xor bh,bh
.notokay
    cmp bl,9
    jbe .okay
    inc bh
    sub bl,10
    jmp .notokay
.okay
    mov [SPC7110RTC+11],bl
    shr eax,8
    and al,0Fh
    mov [SPC7110RTC+12],al ; day of week
.done
    pop eax
    pop ebx

.dontupdate
;    test byte[SPC7110RTC+0Fh],1
;    jz .realtime
;    cmp ebx,0Dh
;    jae .realtime
;    mov al,[SPC7110RTCB+ebx]
;    jmp .next
;.realtime
    mov al,[SPC7110RTC+ebx]
;.next
    pop ebx
    inc byte[SPC7110RTCStat+1]
    and byte[SPC7110RTCStat+1],0Fh
    ret
.commandbyte
    inc byte[SPC7110RTCStat+1]
    mov al,[SPC7110RTCStat+2]
    ret

SECTION .data
SPCTimerVal:
 db 12h,01h,02h,03h,04h,05h,06h,07h,08h,09h,0,0,0,0,0,0
 db 10h,11h,32h,21h,22h,23h,24h,25h,26h,27h,0,0,0,0,0,0
 db 28h,29h

SECTION .text

SPC4842:
    mov al,80h
    ret


;$4820   16 BIT MULTIPLICAND: ($00FF) low byte, defval:00
;        32 BIT DIVI: ($000000FF) low byte of low word, defval:00
;$4821   16 BIT MULTIPLICAND: ($FF00) high byte, defval:00
;        32 BIT DIVI: ($0000FF00) high byte of low word, defval:00
;$4822   32 BIT DIVI: ($00FF0000) low byte of high word, defval:00
;$4823   32 BIT DIVI: ($FF000000) high byte of high word, defval:00
;$4824   16 BIT MULTIPLIER: ($00FF) low byte, defval:00
;$4825   16 BIT MULTIPLIER: ($FF00) high byte, defval:00
;$4826   16 BIT DIVISOR: ($00FF), defval:00
;$4827   16 BIT DIVISOR: ($FF00), defval:00
;$4828   32 BIT PRODUCT: ($000000FF) low byte of low word, defval:00
;        32 BIT QUOTIENT:($000000FF) low byte of low word, defval:00
;$4829   32 BIT PRODUCT: ($0000FF00) high byte of low word, defval:00
;        32 BIT QUOTIENT:($0000FF00) high byte of low word, defval:00
;$482A   32 BIT PRODUCT: ($00FF0000) low byte of high word, defval:00
;        32 BIT QUOTIENT:($00FF0000) low byte of high word, defval:00
;$482B   32 BIT PRODUCT: ($FF000000) high byte of high word, defval:00
;        32 BIT QUOTIENT:($FF000000) high byte of high word, defval:00
;$482C   16 BIT REMAINDER: ($00FF) low byte, defval:00
;$482D   16 BIT REMAINDER: ($FF00) high byte, defval:00
;$482E   MUL/DIV RESET, write = reset $4820 to $482D, defval:00
;$482F   MUL/DIV FINISHED STATUS: bit 7: on = processing, off = finished,
;            high bit is set after a write to multiplier or divisor regs $4825/$4827, defval:00


; SA-1 Start
; ----------

SECTION .data

; IRQ Stuff
NEWSYM SA1Mode, dd 0     ; 0 = SNES CPU, 1 = SA1 CPU
NEWSYM SA1Control, dd 0         ; don't execute if b5 or 6 are set
NEWSYM SA1BankPtr, dd 0
NEWSYM SA1ResetV, dd 0
NEWSYM SA1NMIV, dd 0
NEWSYM SA1IRQV, dd 0
NEWSYM SA1RV, dd 0
NEWSYM CurBWPtr, dd 0
NEWSYM SA1TempVar, dd 0
NEWSYM SA1IRQEn, dd 0
NEWSYM SA1Message, dd 0
NEWSYM SA1IRQExec, dd 0
NEWSYM SA1IRQEnable, dd 0
NEWSYM SA1DoIRQ, dd 0
; Arithmetic Stuff
NEWSYM SA1ARC, dd 0
NEWSYM SA1AR1, dd 0
NEWSYM SA1AR2, dd 0
NEWSYM SA1ARR1, dd 0
NEWSYM SA1ARR2,dd 0
NEWSYM SA1Stat, dd 0

NEWSYM SNSNMIV, dd 0
NEWSYM SNSIRQV, dd 0
NEWSYM SA1DMACount, dd 0
NEWSYM SA1DMAInfo, dd 0
NEWSYM SA1DMAChar, dd 0
NEWSYM SA1DMASource, dd 0
NEWSYM SA1DMADest, dd 0
NEWSYM SA1IRQTemp, dd 0

NEWSYM SA1BankSw, dd 1
NEWSYM SA1BankVal, db 0,1,2,3

NEWSYM BWShift, dd 0
NEWSYM BWAndAddr, dd 0
NEWSYM BWAnd, dd 0
NEWSYM BWRAnd, dd 0

SA1Reserved times 456 db 0


; SA1 Swap Stuff
NEWSYM SA1xa, dd 0
NEWSYM SA1xx, dd 0
NEWSYM SA1xy, dd 0
NEWSYM SA1xd, dd 0
NEWSYM SA1xdb, dd 0
NEWSYM SA1xpb, dd 0
NEWSYM SA1xs, dd 0
NEWSYM SA1RegP, dd 0
NEWSYM SA1RegE, dd 0
NEWSYM SA1RegPCS,dd 0
NEWSYM SA1BWPtr,dd 0
NEWSYM SA1Ptr, dd 0     ; Current PC, SA-1

NEWSYM SA1Overflow, dd 0
NEWSYM VarLenAddr, dd 0
NEWSYM VarLenAddrB, dd 0
NEWSYM VarLenBarrel, dd 0
NEWSYM SA1TimerVal, dd 0
NEWSYM SA1TimerSet, dd 0
NEWSYM SA1TimerCount, dd 0
NEWSYM SA1IRQData, dd 0

; SNES Swap Stuff
NEWSYM SNSRegP, dd 0
NEWSYM SNSRegE, dd 0
NEWSYM SNSRegPCS,dd 0
NEWSYM SNSBWPtr,dd 0
NEWSYM SNSPtr, dd 0     ; Current PC, SNES

NEWSYM IRAM, times 2049 db 0    ;2 kbytes of iram
num2writesa1reg equ $-SA1Mode
NEWSYM PHnum2writesa1reg, dd num2writesa1reg

NEWSYM SA1RAMArea, dd 0
NEWSYM SA1Temp, dd 0
NEWSYM Sdd1Mode, dd 0
NEWSYM Sdd1Bank, dd 0
NEWSYM Sdd1Addr, dd 0
NEWSYM Sdd1NewAddr, dd 0

SECTION .text

%macro SA1QuickF 2
NEWSYM %1
    mov [%2],al
    ret
%endmacro

NEWSYM SA1Reset
    mov byte[SA1IRQData+1],0
    mov byte[SA1Mode],0
    mov byte[SA1Status],0
    mov byte[SA1Control],20h
    mov dword[SA1DoIRQ],0
    mov ax,[irqv2]
    mov [irqv],ax
    mov ax,[nmiv2]
    mov [nmiv],ax
    mov eax,[romdata]
    sub eax,8000h
    mov [SA1RegPCS],eax
    mov eax,[romdata]
    add eax,4096*1024
    mov [SA1RAMArea],eax
    mov eax,[romdata]
    add eax,4096*1024-6000h
    mov [CurBWPtr],eax
    mov [SA1BWPtr],eax
    mov [SNSBWPtr],eax
    mov dword[SA1xa],0
    mov dword[SA1xx],0
    mov dword[SA1xy],0
    mov dword[SA1xd],0
    mov dword[SA1xdb],0
    mov dword[SA1xpb],0
    mov dword[SA1xs],1FFh
    mov dword[SA1RegP],0
    mov dword[SA1RegE],0
    mov dword[SA1IRQExec],0
    mov dword[SA1IRQEnable],0
    mov dword[SA1Message],0
    mov word[SA1Overflow],0
    ret

%macro BankSwitch 4
    push ecx
    push edx
    push eax
    mov [SA1BankVal+%1],al
    mov ebx,snesmmap+%2*4
    test al,80h
    jz .noupper
    cmp byte[NumofBanks],64
    jne .BSBigBank
    and eax,1
.BSBigBank
    and eax,07h
    shl eax,20
    push eax
    jmp .yesupper
.noupper
    cmp byte[NumofBanks],64
    jne .BSBigBank2
    and eax,1
.BSBigBank2
    and eax,07h
    shl eax,20
    push eax
    mov eax,%4
.yesupper
    mov ecx,20h
    add eax,[romdata]
    sub eax,8000h
.mmaploop
    mov [ebx],eax
    add eax,8000h
    add ebx,4
    dec ecx
    jnz .mmaploop
    pop eax
    add eax,[romdata]
    mov ecx,10h
    mov ebx,snesmap2+%3*4
    mov edx,snesmmap+%3*4
.mmaploop2
    mov [ebx],eax
    mov [edx],eax
    add eax,10000h
    add ebx,4
    add edx,4
    dec ecx
    jnz .mmaploop2
    pop eax
    pop edx
    pop ecx
    ret
%endmacro

NEWSYM sa12220w
    BankSwitch 0,0,0C0h,0
NEWSYM sa12221w
    BankSwitch 1,20h,0D0h,100000h
NEWSYM sa12222w
    BankSwitch 2,80h,0E0h,200000h
NEWSYM sa12223w
    BankSwitch 3,0A0h,0F0h,300000h

%macro BankSwitchSDD1 2
    push ecx
    push edx
    push eax
    mov [SDD1BankA+%1],al
    and eax,07h
    shl eax,20
    add eax,[romdata]
    mov ecx,10h
    mov ebx,snesmap2+%2*4
    mov edx,snesmmap+%2*4
.mmaploop2
    mov [ebx],eax
    mov [edx],eax
    add eax,10000h
    add ebx,4
    add edx,4
    dec ecx
    jnz .mmaploop2
    pop eax
    pop edx
    pop ecx
    ret
%endmacro

sdd14804:
    mov al,[SDD1BankA]
    ret
sdd14805:
    mov al,[SDD1BankA+1]
    ret
sdd14806:
    mov al,[SDD1BankA+2]
    ret
sdd14807:
    mov al,[SDD1BankA+3]
    ret

NEWSYM sdd14804w
    BankSwitchSDD1 0,0C0h
NEWSYM sdd14805w
    BankSwitchSDD1 1,0D0h
NEWSYM sdd14806w
    BankSwitchSDD1 2,0E0h
NEWSYM sdd14807w
    BankSwitchSDD1 3,0F0h

NEWSYM sa12200w
    mov bl,al
    and bl,0Fh
    mov [SA1Message],bl
    test al,80h
    jz .noirq
    or byte[SA1DoIRQ],1
.noirq
    test al,10h
    jz .nonmi
    or byte[SA1DoIRQ],2
.nonmi
    test byte[SA1Control],20h
    jz .noreset
    test al,20h
    jnz .noreset
    mov [SA1Control],al
    mov ebx,[romdata]
    mov [SA1BankPtr],ebx
    xor ebx,ebx
    mov bx,[SA1ResetV]
    add ebx,[romdata]
    sub ebx,8000h
    mov [SA1Ptr],ebx
    mov byte[SA1xpb],0
    mov word[SA1xs],1FFh
    mov ebx,[romdata]
    sub ebx,8000h
    mov [SA1RegPCS],ebx
    xor ebx,ebx
    ret
.noreset
    mov [SA1Control],al
    ret

NEWSYM sa12201w         ; IRQ Enable
    mov [SA1IRQEnable],al
    ret
NEWSYM sa12202w         ; IRQ Clear
    test al,80h
    jz .noirqclear
    and byte[SA1IRQExec],0FEh
    and byte[SA1DoIRQ],0FBh
.noirqclear
    test al,20h
    jz .nocdmairqclear
    and byte[SA1IRQExec],0FDh
    and byte[SA1DoIRQ],0F7h
.nocdmairqclear
    ret

SA1QuickF sa12203w, SA1ResetV
SA1QuickF sa12204w, SA1ResetV+1
SA1QuickF sa12205w, SA1NMIV
SA1QuickF sa12206w, SA1NMIV+1
SA1QuickF sa12207w, SA1IRQV
SA1QuickF sa12208w, SA1IRQV+1
NEWSYM sa12209w ; IRQ Stuff
    mov [SA1IRQData+1],al
    test al,80h
    jz .noirq
    ; execute IRQ on the SNES
    or byte[SA1DoIRQ],4
.noirq
    mov bl,al
    and bl,0Fh
    mov [SA1Message+1],bl
    mov bx,[irqv2]
    test al,40h
    jz .noirqchange
    mov bx,[SNSIRQV]
.noirqchange
    mov [irqv],bx
    mov bx,[nmiv2]
    test al,10h
    jz .nonmichange
    mov bx,[SNSNMIV]
.nonmichange
    mov [nmiv],bx
    ret

SA1QuickF sa1220Aw, SA1IRQEn

NEWSYM sa1220Bw         ; SA-1 IRQ Clear
    test al,80h
    jz .noirqclear
    mov byte[SA1IRQExec+1],0
    and byte[SA1DoIRQ],0FEh
.noirqclear
    test al,20h
    jz .nocdmairqclear
.nocdmairqclear
    test al,10h
    jz .nonmiclear
    mov byte[SA1IRQExec+2],0
    and byte[SA1DoIRQ],0FDh
.nonmiclear
    ret

SA1QuickF sa1220Cw, SNSNMIV
SA1QuickF sa1220Dw, SNSNMIV+1
SA1QuickF sa1220Ew, SNSIRQV
SA1QuickF sa1220Fw, SNSIRQV+1

NEWSYM sa12224w ; BWRAM
    mov bl,al
    and ebx,1Fh
    shl ebx,13
    add ebx,[romdata]
    add ebx,1024*4096-6000h
    mov [SNSBWPtr],ebx
    cmp byte[SA1Status],0
    jne .nosnes
    mov [CurBWPtr],ebx
.nosnes
    ret
NEWSYM sa12225w ; BWRAM
    mov [BWUsed2],al
    test al,80h
    jnz .upper
    mov bl,al
    and ebx,1Fh
    shl ebx,13
    add ebx,[romdata]
    add ebx,1024*4096-6000h
    mov [SA1BWPtr],ebx
    cmp byte[SA1Status],0
    je .nosa1b
    mov [CurBWPtr],ebx
.nosa1b
    mov byte[BWShift],0
    mov byte[BWAndAddr],0
    mov byte[BWAnd],0FFh
    mov byte[BWRAnd],0h
    ret
.upper
    mov bl,al
    and ebx,7Fh
    test byte[SA1Overflow+1],80h
    jz .16col
    shl ebx,11
    mov byte[BWShift],2
    mov byte[BWAndAddr],03h
    mov byte[BWAnd],03h
    mov byte[BWRAnd],0FCh
    jmp .4col
.16col
    mov byte[BWShift],1
    mov byte[BWAndAddr],01h
    mov byte[BWAnd],0Fh
    mov byte[BWRAnd],0F0h
    and ebx,3Fh
    shl ebx,12
.4col
    add ebx,[romdata]
    add ebx,1024*4096
    mov [SA1BWPtr],ebx
    cmp byte[SA1Status],0
    je .nosa1
    mov [CurBWPtr],ebx
.nosa1
;    mov byte[debstop3],1
    ret
NEWSYM sa12250w
    mov [SA1ARC],al
    mov byte[SA1ARC+1],1
    test al,2
    jz .notcumul
    mov word[SA1ARR1],0
    mov word[SA1ARR1+2],0
    mov word[SA1ARR1+4],0
.notcumul
    ret
NEWSYM sa12251w
    mov [SA1AR1],al
    mov byte[SA1ARC+1],1
    ret
NEWSYM sa12252w
    mov [SA1AR1+1],al
    mov byte[SA1ARC+1],1
    ret
NEWSYM sa12253w
    mov [SA1AR2],al
    mov byte[SA1ARC+1],1
    ret
NEWSYM sa12254w
    mov [SA1AR2+1],al
    mov byte[SA1ARC+1],1
    test byte[SA1ARC],2
    jnz .cumul
    call UpdateArithStuff
    ret
    ; set overflow bit if exceeds 40bits
.cumul
    pushad
    xor edx,edx
    mov ax,[SA1AR1]
    mov bx,[SA1AR2]
    imul bx
    shl edx,16
    mov dx,ax
    mov byte[SA1Overflow],0
    add [SA1ARR1],edx
    adc byte[SA1ARR2],0
    jnc .notoverflow
    mov byte[SA1Overflow],80h
.notoverflow
    popad
    ret

UpdateArithStuff:
    cmp byte[SA1ARC+1],1
    jne .noarith
    pushad
    mov byte[SA1ARC+1],0
    test byte[SA1ARC],3
    jz .multiply
    test byte[SA1ARC],2
    jnz near .cumulativesum
    test byte[SA1ARC],1
    jnz .division
.multiply
    xor edx,edx
    mov ax,[SA1AR1]
    mov bx,[SA1AR2]
    imul bx
    mov [SA1ARR1],ax
    mov [SA1ARR1+2],dx
    popad
.noarith
    ret
.division
    movsx eax,word[SA1AR1]
    xor edx,edx
    test eax,80000000h
    jz .notneg
    mov edx,0FFFFFFFFh
.notneg
    xor ebx,ebx
    mov bx,[SA1AR2]
    or ebx,ebx
    jz .invalid
    idiv ebx
    mov [SA1ARR1],ax
    mov [SA1ARR1+2],dx
;    mov word[SA1AR1],0
;    mov word[SA1AR2],0
    popad
    ret
.invalid
    mov word[SA1ARR1],0
    mov word[SA1ARR1+2],0
    popad
    ret
.cumulativesum
    popad
    ret

sa12300r:
    mov al,[SA1Message+1]
    test byte[SA1IRQExec],1
    jz .notexecuted
    or al,80h
.notexecuted
    test byte[SA1IRQExec],2
    jz .notexecutedi
    or al,20h
.notexecutedi
    mov bl,[SA1IRQData+1]
    and bl,50h
    or al,bl
    ret
sa12301r:
    mov al,[SA1Message]
    test byte[SA1IRQExec+1],1
    jz .notexecuted
    or al,80h
.notexecuted
    ret
sa12306r:
;    call UpdateArithStuff
    mov al,[SA1ARR1]
    ret
sa12307r:
;    call UpdateArithStuff
    mov al,[SA1ARR1+1]
    ret
sa12308r:
;    call UpdateArithStuff
    mov al,[SA1ARR1+2]
    ret
sa12309r:
;    call UpdateArithStuff
    mov al,[SA1ARR1+3]
    ret
sa1230Ar:
;    call UpdateArithStuff
    mov al,[SA1ARR2]
    ret
sa1230Br:
    mov al,[SA1Overflow]
    ret

NEWSYM IRamRead
    mov al,[IRAM+ecx-3000h]
    ret

NEWSYM IRamWrite
    mov [IRAM+ecx-3000h],al
    ret
NEWSYM IRamWrite2
    mov [IRAM+ecx-3000h],al
    xor dh,dh
    ret

NEWSYM sa1223Fw
    mov [SA1Overflow+1],al
    ret

; Variable Length Data
NEWSYM sa12258w
    mov [VarLenBarrel+2],al
    mov bl,al
    and bl,0Fh
    cmp bl,0
    jne .not0
    mov bl,16
.not0
    mov [VarLenBarrel+3],bl
    test al,80h
    jz .notchange
    mov [VarLenBarrel],bl
    mov [VarLenBarrel+1],bl
.notchange
    ret
NEWSYM sa12259w
    mov [VarLenAddr],al
    mov [VarLenAddrB],al
    mov byte[VarLenBarrel],0
    mov byte[VarLenBarrel+1],0
    ret
NEWSYM sa1225Aw
    mov [VarLenAddr+1],al
    mov [VarLenAddrB+1],al
    mov byte[VarLenBarrel],0
    mov byte[VarLenBarrel+1],0
    ret
NEWSYM sa1225Bw
    mov [VarLenAddr+2],al
    mov [VarLenAddrB+2],al
    mov byte[VarLenBarrel],0
    mov byte[VarLenBarrel+1],0
    ret

; Variable Length Read
NEWSYM sa1230Cr
    push ecx
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[VarLenAddr+2]
    mov cx,[VarLenAddr]
    test ecx,8000h
    jz .loweraddr
    add ecx,[snesmmap+ebx*4]
    jmp .upperaddr
.loweraddr
    add ecx,[snesmap2+ebx*4]
.upperaddr
    mov ebx,[ecx]
    mov cl,[VarLenBarrel+1]
    shr ebx,cl
    mov al,bl
    pop ecx
    test byte[VarLenBarrel+2],80h
    jnz .autoinc
    ret
.autoinc
    mov bl,[VarLenBarrel+3]
    add [VarLenBarrel],bl
    mov bl,[VarLenBarrel]
    cmp bl,16
    jbe .notover
    sub byte[VarLenBarrel],16
    add dword[VarLenAddr],2
    mov byte[VarLenAddr+3],0
.notover
    ret

NEWSYM sa1230Dr
    push ecx
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[VarLenAddrB+2]
    mov cx,[VarLenAddrB]
    test ecx,8000h
    jz .loweraddr
    add ecx,[snesmmap+ebx*4]
    jmp .upperaddr
.loweraddr
    add ecx,[snesmap2+ebx*4]
.upperaddr
    mov ebx,[ecx]
    mov cl,[VarLenBarrel+1]
    shr ebx,cl
    mov al,bh
    pop ecx
    test byte[VarLenBarrel+2],80h
    jnz .autoinc
    ret
.autoinc
    mov bl,[VarLenBarrel+3]
    add [VarLenBarrel+1],bl
    mov bl,[VarLenBarrel+1]
    cmp bl,16
    jbe .notover
    sub byte[VarLenBarrel+1],16
    add dword[VarLenAddrB],2
    mov byte[VarLenAddrB+3],0
.notover
    ret

NEWSYM sa1230Er
    mov al,10h
    ret

; Approximate H loc
NEWSYM sa12302r
    test byte[SA1TimerSet],80h
    jnz .timeron
    mov al,[CurrentExecSA1]
    shl al,2
    add al,dh
    ret
.timeron
    mov al,[SA1TimerCount]
    ret
NEWSYM sa12303r
    test byte[SA1TimerSet],80h
    jnz .timeron
    mov al,[CurrentExecSA1]
    shr al,3
    ret
.timeron
    mov al,[SA1TimerCount+1]
    and al,1
    ret
NEWSYM sa12304r
    test byte[SA1TimerSet],80h
    jnz .timeron
    mov al,[curypos]
    ret
.timeron
    mov bx,[SA1TimerCount+1]
    shr bx,1
    mov al,bl
    ret
NEWSYM sa12305r
    test byte[SA1TimerSet],80h
    jnz .timeron
    mov al,[curypos+1]
    ret
.timeron
    mov bx,[SA1TimerCount+2]
    shr bx,1
    mov al,bl
    and al,1
    ret

NEWSYM sa12210w ; Timer Settings
   mov [SA1TimerSet],al
   ret
NEWSYM sa12211w ; Timer Clear
   mov dword[SA1TimerVal],0
   ret
NEWSYM sa12212w
   mov [SA1TimerCount],al
   ret
NEWSYM sa12213w
   mov [SA1TimerCount+1],al
   ret
NEWSYM sa12214w
   mov [SA1TimerCount+2],al
   ret
NEWSYM sa12215w
   mov [SA1TimerCount+3],al
   ret

NEWSYM sa12230w
    mov [SA1DMAInfo],al
    ret
NEWSYM sa12231w
    mov [SA1DMAChar],al
    ; if b7=1, then end of chdma process
    ret
SA1QuickF sa12232w, SA1DMASource
SA1QuickF sa12233w, SA1DMASource+1
SA1QuickF sa12234w, SA1DMASource+2
SA1QuickF sa12235w, SA1DMADest
NEWSYM sa12236w
    mov [SA1DMADest+1],al
    test byte[SA1DMAInfo],10h
    jnz near sa1chconv
    test byte[SA1DMAInfo],4
    jnz .noiram
    jmp sa1dmairam
.noiram
    ret
NEWSYM sa12237w
    mov [SA1DMADest+2],al
    test byte[SA1DMAInfo],10h
    jnz .nobwram
    test byte[SA1DMAInfo],4
    jz .nobwram
    jmp sa1dmabwram
.nobwram
    ret
NEWSYM sa12238w
    mov [SA1DMACount],al
    ret
NEWSYM sa12239w
    mov [SA1DMACount+1],al
    ret

SECTION .bss
NEWSYM sa1dmaptr, resd 1
NEWSYM sa1dmaptrs, resd 1

SECTION .text

NEWSYM sa1dmairam
    mov ebx,[SA1DMADest]
    and ebx,7FFh
    add ebx,IRAM
    mov [sa1dmaptr],ebx
    jmp executesa1dma
NEWSYM sa1dmabwram
    mov ebx,[SA1DMADest]
    and ebx,3FFFFh
    add ebx,[SA1RAMArea]
    mov [sa1dmaptr],ebx
executesa1dma:
    test byte[SA1DMAInfo],1
    jz .nobwram
    mov ebx,[SA1DMASource]
    and ebx,3FFFFh
    add ebx,[SA1RAMArea]
    mov [sa1dmaptrs],ebx
    jmp .doneram
.nobwram
    test byte[SA1DMAInfo],2
    jz .noiram
    mov ebx,[SA1DMASource]
    and ebx,7FFh
    add ebx,IRAM
    mov [sa1dmaptrs],ebx
    jmp .doneram
.noiram
    xor ebx,ebx
    mov bl,[SA1DMASource+2]
    mov ebx,[snesmmap+ebx*4]
    push ecx
    xor ecx,ecx
    mov cx,[SA1DMASource]
    add ebx,ecx
    mov [sa1dmaptrs],ebx
    pop ecx
.doneram
    push edx
    push eax
    push ecx
    mov ecx,[SA1DMACount]
    or ecx,ecx
    jz .notransfer
    mov ebx,[sa1dmaptrs]
    mov edx,[sa1dmaptr]
.loop
    mov al,[ebx]
    mov [edx],al
    inc ebx
    inc edx
    dec ecx
    jnz .loop
.notransfer
    pop ecx
    pop eax
    pop edx
    ret

SECTION .bss
tempblah resb 1

SECTION .text

%macro setbit2b 2
    test al,%1
    jz %%nosb
    or word[ebx],%2
%%nosb
%endmacro

%macro setbit2b2 2
    test al,%1
    jz %%nosb
    or word[ebx+16],%2
%%nosb
%endmacro

; Character Conversion DMA
sa1chconv:
;    or byte[SA1DoIRQ],4
    or byte[SA1DoIRQ],8
;    mov byte[debstop3],1


    mov ebx,[SA1DMADest]
    mov byte[debstop4],1
;    and ebx,7FFh
    and ebx,3FFFFh
    add ebx,[SA1RAMArea]
    mov [sa1dmaptr],ebx
    mov ebx,[SA1DMASource]
    and ebx,3FFFFh
    add ebx,[SA1RAMArea]
    mov [sa1dmaptrs],ebx
    ; 4 colors = 32 bytes, 16 colors = 64 bytes, 256 colors = 128 bytes
    ; SA1DMAChar,bit 2-4 = # of 8x8 tiles/horizontal row (0=1,1=2,2=3,..,5=32)
    ; SA1DMAChar,bit 0-1 = Color Mode (0=8b,1=4b,2=2b,3=?)
    test byte[SA1DMAChar],1
    jnz .4bit
    test byte[SA1DMAChar],2
    jnz near .2bit
    mov ebx,[sa1dmaptr]
    push ecx
    pop ecx
    ret
.4bit
    pushad
    mov edx,[sa1dmaptrs]
    mov ebx,[romdata]
    add ebx,4096*1024+1024*1024
    mov edi,16
.loop34b
    push ebx
    push edx
    mov ecx,32
.loop4b
    mov esi,8
    push ebx
    push edx
.loop24b
    mov word[ebx],0
    mov al,[edx+3]
    setbit2b 10h,0001h
    setbit2b 20h,0100h
    setbit2b2 40h,0001h
    setbit2b2 80h,0100h
    setbit2b 01h,0002h
    setbit2b 02h,0200h
    setbit2b2 04h,0002h
    setbit2b2 08h,0200h
    mov al,[edx+2]
    setbit2b 10h,0004h
    setbit2b 20h,0400h
    setbit2b2 40h,0004h
    setbit2b2 80h,0400h
    setbit2b 01h,0008h
    setbit2b 02h,0800h
    setbit2b2 04h,0008h
    setbit2b2 08h,0800h
    mov al,[edx+1]
    setbit2b 10h,0010h
    setbit2b 20h,1000h
    setbit2b2 40h,0010h
    setbit2b2 80h,1000h
    setbit2b 01h,0020h
    setbit2b 02h,2000h
    setbit2b2 04h,0020h
    setbit2b2 08h,2000h
    mov al,[edx]
    setbit2b 10h,0040h
    setbit2b 20h,4000h
    setbit2b2 40h,0040h
    setbit2b2 80h,4000h
    setbit2b 01h,0080h
    setbit2b 02h,8000h
    setbit2b2 04h,0080h
    setbit2b2 08h,8000h
    add ebx,2
    add edx,128
    dec esi
    jnz near .loop24b
    pop edx
    pop ebx
    add edx,4
    add ebx,32
    dec ecx
    jnz near .loop4b
    pop edx
    pop ebx
    add edx,128*8
    add ebx,128*8
    dec edi
    jnz near .loop34b

    mov ecx,10*128*8
    mov edx,[sa1dmaptrs]
    mov ebx,[romdata]
    add ebx,4096*1024+1024*1024
.next4b
    mov al,[ebx]
    mov [edx],al
    inc ebx
    inc edx
    dec ecx
    jnz .next4b

    popad
    ret

.2bit
    pushad
    mov edx,[sa1dmaptrs]
    mov ebx,[romdata]
    add ebx,4096*1024+1024*1024
    mov edi,16
.loop3
    push ebx
    push edx
    mov ecx,32
.loop
    mov esi,8
    push ebx
    push edx
.loop2
    mov word[ebx],0
    mov al,[edx+1]
    setbit2b 40h,0001h
    setbit2b 80h,0100h
    setbit2b 10h,0002h
    setbit2b 20h,0200h
    setbit2b 04h,0004h
    setbit2b 08h,0400h
    setbit2b 01h,0008h
    setbit2b 02h,0800h
    mov al,[edx]
    setbit2b 40h,0010h
    setbit2b 80h,1000h
    setbit2b 10h,0020h
    setbit2b 20h,2000h
    setbit2b 04h,0040h
    setbit2b 08h,4000h
    setbit2b 01h,0080h
    setbit2b 02h,8000h
    add ebx,2
    add edx,64
    dec esi
    jnz near .loop2
    pop edx
    pop ebx
    add edx,2
    add ebx,16
    dec ecx
    jnz near .loop
    pop edx
    pop ebx
    add edx,64*8
    add ebx,64*8
    dec edi
    jnz near .loop3

    mov ecx,10*64*8
    mov edx,[sa1dmaptrs]
    mov ebx,[romdata]
    add ebx,4096*1024+1024*1024
.next
    mov al,[ebx]
    mov [edx],al
    inc ebx
    inc edx
    dec ecx
    jnz .next

    popad
    ret
SECTION .bss
.numrows resd 1

SECTION .text

NEWSYM initSA1regs
    setreg 2300h*4,sa12300r
    setreg 2301h*4,sa12301r
    setreg 2302h*4,sa12302r
    setreg 2303h*4,sa12303r
    setreg 2304h*4,sa12304r
    setreg 2305h*4,sa12305r
    setreg 2306h*4,sa12306r
    setreg 2307h*4,sa12307r
    setreg 2308h*4,sa12308r
    setreg 2309h*4,sa12309r
    setreg 230Ah*4,sa1230Ar
    setreg 230Bh*4,sa1230Br
    setreg 230Ch*4,sa1230Cr
    setreg 230Dh*4,sa1230Dr
    setreg 230Eh*4,sa1230Er
    ; Set IRam, memory address 3000-37FF

    mov edi,3000h*4
    add edi,[regptr]
    mov eax,IRamRead
    mov ecx,800h
.loopr
    mov [edi],eax
    add edi,4
    dec ecx
    jnz .loopr
    ret

NEWSYM initSA1regsw
    setregw 2200h*4,sa12200w
    setregw 2201h*4,sa12201w
    setregw 2202h*4,sa12202w
    setregw 2203h*4,sa12203w
    setregw 2204h*4,sa12204w
    setregw 2205h*4,sa12205w
    setregw 2206h*4,sa12206w
    setregw 2207h*4,sa12207w
    setregw 2208h*4,sa12208w
    setregw 2209h*4,sa12209w
    setregw 220Ah*4,sa1220Aw
    setregw 220Bh*4,sa1220Bw
    setregw 220Ch*4,sa1220Cw
    setregw 220Dh*4,sa1220Dw
    setregw 220Eh*4,sa1220Ew
    setregw 220Fh*4,sa1220Fw
    setregw 2210h*4,sa12210w
    setregw 2211h*4,sa12211w
    setregw 2212h*4,sa12212w
    setregw 2213h*4,sa12213w
    setregw 2214h*4,sa12214w
    setregw 2215h*4,sa12215w

    setregw 2220h*4,sa12220w
    setregw 2221h*4,sa12221w
    setregw 2222h*4,sa12222w
    setregw 2223h*4,sa12223w

    setregw 2224h*4,sa12224w
    setregw 2225h*4,sa12225w
    ; Missing 2226-222A
    ; Missing 2240-224F (Bitmap register file)

    setregw 2230h*4,sa12230w
    setregw 2231h*4,sa12231w
    setregw 2232h*4,sa12232w
    setregw 2233h*4,sa12233w
    setregw 2234h*4,sa12234w
    setregw 2235h*4,sa12235w
    setregw 2236h*4,sa12236w
    setregw 2237h*4,sa12237w
    setregw 2238h*4,sa12238w
    setregw 2239h*4,sa12239w
    setregw 223Fh*4,sa1223Fw

    setregw 2250h*4,sa12250w
    setregw 2251h*4,sa12251w
    setregw 2252h*4,sa12252w
    setregw 2253h*4,sa12253w
    setregw 2254h*4,sa12254w

    setregw 2259h*4,sa12259w
    setregw 225Ah*4,sa1225Aw
    setregw 225Bh*4,sa1225Bw

    mov edi,3000h*4
    add edi,[regptw]
    mov eax,IRamWrite
    mov ecx,800h
.loopw
    mov [edi],eax
    add edi,4
    dec ecx
    jnz .loopw
    setregw 3000h*4,IRamWrite2
    ret

NEWSYM SDD1Reset
    setregw 4801h*4,sdd14801w
    setregw 4802h*4,dbstop
    setregw 4803h*4,dbstop
    setregw 4804h*4,sdd14804w
    setregw 4805h*4,sdd14805w
    setregw 4806h*4,sdd14806w
    setregw 4807h*4,sdd14807w
    setregw 4808h*4,dbstop
    setregw 4809h*4,dbstop
    setregw 480Ah*4,dbstop
    setregw 480Bh*4,dbstop
    setregw 480Ch*4,dbstop
    setregw 480Dh*4,dbstop
    setregw 480Eh*4,dbstop
    setregw 480Fh*4,dbstop
    ret

NEWSYM initSDD1regs
    setreg 4804h*4,sdd14804
    setreg 4805h*4,sdd14805
    setreg 4806h*4,sdd14806
    setreg 4807h*4,sdd14807
    ret

dbstop:
;    mov byte[debstop3],1
    ret

NEWSYM sdd14801w
    cmp al,0
    jne .notzero
    ret
.notzero
    mov byte[AddrNoIncr],0
    ; set banks C0h-FFh to decompressing routine
    push eax
    push ebx
    mov eax,memtabler8+0C0h*4
    mov ebx,40h
.loop
    mov dword[eax],memaccessbankr8sdd1
    add eax,4
    dec ebx
    jnz .loop
    mov dword[Sdd1Mode],1
    pop ebx
    pop eax
    ret
