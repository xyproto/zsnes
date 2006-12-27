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

EXTSYM regptr,regptw,romdata,SA1Status,SDD1BankA,NumofBanks,BWUsed2
EXTSYM Get_Time,Get_TimeDate,irqv2,irqv,nmiv2,nmiv,snesmmap,snesmap2
EXTSYM curypos,CurrentExecSA1,memaccessbankr8sdd1,memtabler8,AddrNoIncr

%ifndef NO_DEBUGGER
EXTSYM debuggeron,debstop4
%endif

%include "cpu/regs.mac"
%include "cpu/regsw.mac"

SECTION .data
NEWSYM RTCData, db 0Fh,0,0,0,0,0,0,0,0,0,0,0,0,0,0Fh,0
NEWSYM RTCPtr, dd 0
NEWSYM RTCPtr2, dd 0
NEWSYM RTCRest, dd 0

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
%ifndef NO_DEBUGGER
    cmp byte[debuggeron],1
    je near .notfirst
%endif
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
%ifndef NO_DEBUGGER
    mov byte[debstop4],1
%endif
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