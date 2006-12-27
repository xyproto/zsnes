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

EXTSYM memaccessbankr8,memaccessbankr16,memaccessbankw8,memaccessbankw16
EXTSYM regaccessbankr8,regaccessbankr16,regaccessbankw8,regaccessbankw16
EXTSYM sramaccessbankr8b,sramaccessbankr16b,sramaccessbankw8b,sramaccessbankw16b
EXTSYM SPC7110PackPtr,SPC7110IndexPtr,SPC7110IndexSize,SPC7_Data_Load
EXTSYM SPC7110Entries,SPC7110filep,Get_Time,Get_TimeDate,snesmmap,snesmap2
EXTSYM curromsize,regptw,regptr,romdata

%include "cpu/regs.mac"
%include "cpu/regsw.mac"

%ifndef NO_DEBUGGER
EXTSYM debuggeron
%endif

; SPC7110 emulation.  Information fully reverse engineered
;    by Dark Force and John Weidman, ZSNES code by zsKnight

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
NEWSYM SPC7110TempPosition, dd 0
NEWSYM SPC7110TempLength, dd 0
NEWSYM SPCPrevCompPtr, dd 0

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

%ifndef NO_DEBUGGER
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
%endif

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
    add ebx,[SPC7110PackPtr]
    mov al,[ebx]
    pop ebx
;    xor al,al
;    inc dword[SPCCompPtr]

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
    add ebx,[SPC7110PackPtr]
    mov al,[ebx]
    pop ebx

    dec word[SPCCompCounter]
;    inc dword[SPCCompPtr]
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
NEWSYM CurPtrLen, resd 1
NEWSYM CurValUsed, resb 1
NEWSYM PrevDecompPtr, resw 1
NEWSYM CurDecompPtr, resw 1
NEWSYM CurDecompSize, resw 1
NEWSYM DecompArray, resb 65536
NEWSYM DecompAPtr, resd 1
lastentry resd 1

SECTION .text
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
;    cmp dword[SPCCompPtr],0124AD48h
;    jne .nodata
;    mov byte[debstop3],1
;.nodata

    push ebx
    mov ebx,[SPCCompPtr]
    and ebx,0ffffffh
    push ecx
    movzx ecx,byte[SPCCompPtr+3]
    shl ecx,2
    add ebx,ecx
    pop ecx
    add ebx,100000h
    add ebx,[romdata]
    cmp byte[ebx],3
    jne .try2
    shl word[SPCDecmPtr],3
.try2
    cmp byte[ebx],2
    jne .try1
    shl word[SPCDecmPtr],2
.try1
    cmp byte[ebx],1
    jne .skip
    shl word[SPCDecmPtr],1
.skip
    pop ebx


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
    mov ax,[SPCDecmPtr]
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
    mov eax,[SPC7110IndexPtr]
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
    add edx,[SPC7110PackPtr]
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
    mov edx,[SPC7110IndexPtr]
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

    pushad
    call SPC7_Data_Load
    popad
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
    ret
SPC480Aw:
    mov [SPCCompCounter+1],al
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
    mov ebx,[SPCROMPtr]
    add ebx,[romdata]
    add ebx,100000h
    test byte[SPCROMCom],2
    jz .no2
    add ebx,[SPCROMAdj]
    inc word[SPCROMAdj]
    mov al,[ebx]
    pop ebx
    ret
.no2
    mov al,[ebx]
    cmp byte[SPCROMCom+1],0
    jne .noincr1
    mov ebx,[SPCROMtoI]
    inc dword[ebx]
.noincr1
    cmp byte[SPCROMCom+1],1     ; add 4816 after 4810 read
    jne .noincr1b
    mov ebx,[SPCROMtoI]
    push ecx
    mov ecx,[SPCROMInc]
    add dword[ebx],ecx
    pop ecx
.noincr1b
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
    xor ebx,ebx
    mov bx,[SPCROMAdj]
    add ebx,[SPCROMPtr]
    add ebx,[romdata]
    add ebx,100000h
    mov al,[ebx]

    cmp byte[SPCROMCom+1],4     ; 16bit 4814
    jne .notincr
    mov ebx,[SPCROMAdj]
    push ecx
    mov ecx,[SPCROMtoI]
    add [ecx],ebx
    pop ecx
.notincr
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

%ifndef NO_DEBUGGER
    cmp byte[debuggeron],1
    je near .dontupdate
%endif
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

;SPC7110 Sram Map
;$006000 - $007FFF sram 8k (slow rom?)
;$306000 - $307FFF mirrored sram from $006000 - $007FFF (fast rom?)
%macro SRAMAccessSPC7110 1
    test ecx,8000h
    jnz memaccessbank%1
    cmp ecx,6000h
    jb regaccessbank%1
    push ecx
    sub ecx,6000h
    shl ebx,13
    add ecx,ebx
    and ecx,0FFFFh
    call sramaccessbank%1b
    pop ecx
%endmacro

NEWSYM SPC7110ReadSRAM8b
    SRAMAccessSPC7110 r8
    ret

NEWSYM SPC7110ReadSRAM16b
    SRAMAccessSPC7110 r16
    ret

NEWSYM SPC7110WriteSRAM8b
    SRAMAccessSPC7110 w8
    ret

NEWSYM SPC7110WriteSRAM16b
    SRAMAccessSPC7110 w16
    ret

;data decompressed from data rom by spc7110 mapped to $50:0000-$50:FFFF
NEWSYM memaccessspc7110r8
    push ebx
    movzx ebx,word[SPCDecmPtr]
    add ebx,[SPC7110PackPtr]
    mov al,[ebx]
    pop ebx

    dec word[SPCCompCounter]
    inc word[SPCDecmPtr]
    inc word[CurDecompSize]
    ret

NEWSYM memaccessspc7110r16
    mov ebx,[SPC7110PackPtr]
    mov ax,[ebx+ecx]
    cmp cx,[CurDecompPtr]
    jb .noptr
    mov [CurDecompPtr],cx
    mov bx,cx
    sub bx,[PrevDecompPtr]
    add bx,2
    mov [CurDecompSize],bx
.noptr
    xor ebx,ebx
    ret

NEWSYM memaccessspc7110w8
    mov ebx,[SPC7110PackPtr]
    mov [ebx+ecx],al
    xor ebx,ebx
    ret

NEWSYM memaccessspc7110w16
    mov ebx,[SPC7110PackPtr]
    mov [ebx+ecx],ax
    xor ebx,ebx
    ret
