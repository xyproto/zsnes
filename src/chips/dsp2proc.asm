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

; Apr 29, 2004
;   DSP2 support code written by kentaro-k.21 <danmasu-lj@infoseek.jp>
;   coding hints are based on DSP2 function overview at http://users.tpg.com.au/trauma/dsp/dsp2.html maintained by Overload.
;   i have stolen the useful technical info there to implement DSP2 codes without any permission from its author.
;   i do NOT know and peep the s9x source codes for either DSP2 or any purpose.
; May 02, 2004
;   bug fix for Dungeon Master.
;   Command 05h fixed.
;   when you got the magic missile damage such as fireball, DSP2 support in prior version entered halt state.

%include "macros.mac"

DSP2F_HALT              equ 1
DSP2F_AUTO_BUFFER_SHIFT equ 2
DSP2F_NO_ADDR_CHK       equ 4

;*******************************************************
; .bss section
;*******************************************************
SECTION .bss
NEWSYM dsp2buffer, resb 256
NEWSYM dsp2enforcerQueue, resb 8*512
NEWSYM dsp2enforcer, resb 8

;*******************************************************
; .data section
;*******************************************************
SECTION .data
NEWSYM dsp2f03KeyLo, db 0   ; Current transparent-color in lower-byte
NEWSYM dsp2f03KeyHi, db 0   ; Current transparent-color in higher-byte

NEWSYM dsp2enforcerReaderCursor, dd 0   ; T/O
NEWSYM dsp2enforcerWriterCursor, dd 1   ; T/O
NEWSYM dsp2state, dd 1                  ; Flags: 1=HALT, 2=AUTO_BUFFER_SHIFT, 4=NO_ADDR_CHK
NEWSYM dsp2input, dd 0                  ; Saving input 8-bit data
NEWSYM dsp2inputTemp, dd 0              ; Temporary variable
NEWSYM dsp2f0dSizeOrg, dd 0             ; Command 0D, Original BMP width
NEWSYM dsp2f0dSizeNew, dd 0             ; Command 0D, New BMP width

NEWSYM dsp2f01TblByte       ; Conversion table for Command 01
db  0, 1,16,17, 0, 1,16,17
db  0, 1,16,17, 0, 1,16,17
db  0, 1,16,17, 0, 1,16,17
db  0, 1,16,17, 0, 1,16,17
db  2, 3,18,19, 2, 3,18,19
db  2, 3,18,19, 2, 3,18,19
db  2, 3,18,19, 2, 3,18,19
db  2, 3,18,19, 2, 3,18,19
db  4, 5,20,21, 4, 5,20,21
db  4, 5,20,21, 4, 5,20,21
db  4, 5,20,21, 4, 5,20,21
db  4, 5,20,21, 4, 5,20,21
db  6, 7,22,23, 6, 7,22,23
db  6, 7,22,23, 6, 7,22,23
db  6, 7,22,23, 6, 7,22,23
db  6, 7,22,23, 6, 7,22,23
db  8, 9,24,25, 8, 9,24,25
db  8, 9,24,25, 8, 9,24,25
db  8, 9,24,25, 8, 9,24,25
db  8, 9,24,25, 8, 9,24,25
db 10,11,26,27,10,11,26,27
db 10,11,26,27,10,11,26,27
db 10,11,26,27,10,11,26,27
db 10,11,26,27,10,11,26,27
db 12,13,28,29,12,13,28,29
db 12,13,28,29,12,13,28,29
db 12,13,28,29,12,13,28,29
db 12,13,28,29,12,13,28,29
db 14,15,30,31,14,15,30,31
db 14,15,30,31,14,15,30,31
db 14,15,30,31,14,15,30,31
db 14,15,30,31,14,15,30,31

NEWSYM dsp2f01TblBitMask    ; Conversion table for Command 01
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2
db  64, 64, 64, 64,128,128,128,128
db  16, 16, 16, 16, 32, 32, 32, 32
db   4,  4,  4,  4,  8,  8,  8,  8
db   1,  1,  1,  1,  2,  2,  2,  2

;*******************************************************
; .text section
;*******************************************************
SECTION .text

;*******************************************************
;
;*******************************************************
%macro CommandJmp 2
    cmp al,%1
    je near %2
%endmacro

%macro DevWriteX 1
  %ifdef _USE_DEV
    pushad
    mov dword[_DSP2Dev_arg],%1
    call _DevWriteX
    popad
  %endif
%endmacro

%macro EnterInsideCommand 1
    DevWriteX %1
%endmacro

%macro QueueInsideCommand 1
    DevWriteX %1+0ffff0000h
%endmacro

%macro LeaveInsideCommand 0
  %ifdef _USE_DEV
    pushad
    call _Leave
    popad
  %endif
%endmacro

;*******************************************************
;
;*******************************************************

NEWSYM InitDSP2
.enter
    mov dword[dsp2state],0
    mov dword[dsp2enforcerQueue+8*0+0],0
    mov dword[dsp2enforcerQueue+8*0+4],8000h
    mov dword[dsp2enforcerReaderCursor],0
    mov dword[dsp2enforcerWriterCursor],1
.leave
    ret

;*******************************************************
;
;*******************************************************

NEWSYM DSP2Read8b
.enter
    test byte[dsp2state],DSP2F_HALT
    jnz .halt

    test cx,8000h
    jz .undef
    test cx,7000h
    jnz .undef

    and ecx,255
    mov al,[dsp2buffer+ecx]
    xor ecx,ecx

    test byte[dsp2state],DSP2F_AUTO_BUFFER_SHIFT
    jnz .shiftbuffer
    jmp .leave

.shiftbuffer
    sar dword[dsp2buffer],8
    jmp .leave

.halt
.undef
    xor eax,eax
.leave
    ret

NEWSYM DSP2Read16b
.enter
    xor eax,eax
.leave
    ret

;*******************************************************
;
;*******************************************************

NEWSYM _DSP2Add2Queue
.enter
    push eax
    push ebx
    ; *** Locates the write cursor
    mov eax,[dsp2enforcerWriterCursor]
    lea ebx,[dsp2enforcerQueue+eax*8]
    inc eax
    and eax,511
    mov [dsp2enforcerWriterCursor],eax
    ; *** Copies the local inside command into his queue
    mov eax,[dsp2enforcer]
    mov [ebx],eax
    mov eax,[dsp2enforcer+4]
    mov [ebx+4],eax
    pop ebx
    pop eax
.leave
    ret

NEWSYM DSP2Write8b
.enter
    ; Tests halt flag
    test byte[dsp2state],DSP2F_HALT
    jnz near .halt

    ; *** Locates current predicator store
    mov [dsp2input],al
    mov eax,[dsp2enforcerReaderCursor]
    lea ebx,[dsp2enforcerQueue+8*eax]
    ; *** Copies inside command box into box in order to spare indirection cost
    mov eax,[ebx]
    mov [dsp2enforcer],eax
    mov eax,[ebx+4]
    mov [dsp2enforcer+4],eax
    xor ebx,ebx
    ; *** About some commands need to be relaxed the write address check
    test byte[dsp2state],DSP2F_NO_ADDR_CHK
    jnz .noaddrchk
    ; *** Tests whether cx points expected address
    cmp [dsp2enforcer+4],cx
    jne near .gohalt
.noaddrchk
    ; *** Reads next inside command should be proceeded
    mov al,[dsp2enforcer]
    ; *** Branches to inside commands respectively
    CommandJmp 00h,.w00
    CommandJmp 01h,.w01
    CommandJmp 02h,.w02
    CommandJmp 03h,.w03
    CommandJmp 04h,.w04
    CommandJmp 05h,.w05
    CommandJmp 06h,.w06
    CommandJmp 07h,.w07
    CommandJmp 08h,.w08
    CommandJmp 09h,.w09
    CommandJmp 0Ah,.w0A
    CommandJmp 0Bh,.w0B
    jmp .gohalt

.w0B ; ---
    EnterInsideCommand 0Bh

    mov al,[dsp2input]
    xor ecx,ecx
    mov cl,[dsp2enforcer+1]
    mov [dsp2buffer+ecx],al

    jmp .done

.w0A ; ---
    EnterInsideCommand 0Ah

    mov al,[dsp2input]
    sar al,1
    mov [dsp2f0dSizeNew],al

    test al,al
    jz near .gohalt

    xor ecx,ecx
    xor eax,eax
    mov bl,[dsp2f0dSizeNew]
    mov bh,[dsp2f0dSizeOrg]
.w0Aploop
    mov al,cl
    mul bl
    div bh

    mov dword[dsp2enforcer+0],0Bh
    mov dword[dsp2enforcer+4],8000h
    mov [dsp2enforcer+1],al
    mov [dsp2enforcer+4],cl
    call _DSP2Add2Queue

    inc cl
    cmp cl,[dsp2f0dSizeOrg]
    jne .w0Aploop

    xor ebx,ebx
    jmp .queueincoming

.w09 ; ---
    EnterInsideCommand 9

    mov al,[dsp2input]
    sar al,1
    mov [dsp2f0dSizeOrg],al

    test al,al
    jz near .gohalt

    mov dword[dsp2enforcer+0],0Ah
    mov dword[dsp2enforcer+4],8000h
    call _DSP2Add2Queue
    jmp .done

.w08 ; ---
    EnterInsideCommand 8

    xor eax,eax
    mov al,[dsp2enforcer+1]
    mov cl,[dsp2input]
    mov [dsp2buffer+eax],cl

    cmp al,3
    jne .w08done

    mov al,[dsp2buffer]
    mul byte[dsp2buffer+2]

    mov [dsp2buffer],eax
    or byte[dsp2state],DSP2F_AUTO_BUFFER_SHIFT

.w08done
    jmp .done

.w07 ; ---
    EnterInsideCommand 7

    mov cl,[dsp2input]
    rol cl,4
    xor eax,eax
    mov al,[dsp2enforcer+1]
    mov [dsp2buffer+eax],cl

    jmp .done

.w06 ; ---
    EnterInsideCommand 6

    cmp byte[dsp2input],0
    je near .gohalt

    xor eax,eax
    xor ecx,ecx
    mov cl,[dsp2input]
.w06ploop
    dec cl
    mov dword[dsp2enforcer+0],7
    mov dword[dsp2enforcer+4],8000h
    mov [dsp2enforcer+1],cl
    mov [dsp2enforcer+4],al
    call _DSP2Add2Queue

    inc al
    test cl,cl
    jnz .w06ploop

    jmp .queueincoming

.w05 ; ---
    EnterInsideCommand 5

    xor eax,eax
    mov al,[dsp2enforcer+4]
    mov cl,[dsp2buffer+eax]

    mov ch,[dsp2input]
    and ch,0f0h
    cmp ch,[dsp2f03KeyHi]
    je .w05pnohi
    and cl,0fh
    or cl,ch
.w05pnohi

    mov ch,[dsp2input]
    and ch,0fh
    cmp ch,[dsp2f03KeyLo]
    je .w05pnolo
    and cl,0f0h
    or cl,ch
.w05pnolo

    mov [dsp2buffer+eax],cl

    jmp .done

.w04 ; ---
    EnterInsideCommand 4

    xor eax,eax
    mov al,[dsp2enforcer+4]
    mov cl,[dsp2input]
    mov [dsp2buffer+eax],cl

    xor ecx,ecx
    jmp .done

.w03 ; ---
    EnterInsideCommand 3

    cmp byte[dsp2input],0
    je near .gohalt

    xor eax,eax
.w03aloop
    mov dword[dsp2enforcer+0],4
    mov dword[dsp2enforcer+4],8000h
    mov [dsp2enforcer+4],al
    call _DSP2Add2Queue

    inc al
    cmp al,[dsp2input]
    jne .w03aloop

    xor eax,eax
.w03bloop
    mov dword[dsp2enforcer+0],5
    mov dword[dsp2enforcer+4],8000h
    mov [dsp2enforcer+4],al
    call _DSP2Add2Queue

    inc al
    cmp al,[dsp2input]
    jne .w03bloop

    xor ecx,ecx
    jmp .queueincoming

.w02 ; ---
    EnterInsideCommand 2

    mov al,[dsp2input]
    and al,0fh
    mov [dsp2f03KeyLo],al
    sal al,4
    mov [dsp2f03KeyHi],al
    jmp .done

.w01 ; ---
    EnterInsideCommand 1

    xor ecx,ecx
    mov cl,[dsp2enforcer+4]
    sal ecx,3
    mov al,[dsp2input]
    mov [dsp2inputTemp],al
    xor ebx,ebx
.w01ploop
    mov bl,[dsp2f01TblByte+ecx]
    mov al,[dsp2f01TblBitMask+ecx]
    test byte[dsp2inputTemp],1h
    jz .w01pclear
    or [dsp2buffer+ebx],al
    jmp .w01pok
.w01pclear
    not al
    and [dsp2buffer+ebx],al
.w01pok
    sar byte[dsp2inputTemp],1
    inc ecx
    test cl,7
    jnz .w01ploop

    xor ebx,ebx
    xor ecx,ecx
    jmp .done

.w00 ; ---
    EnterInsideCommand 0

    and byte[dsp2state],~(DSP2F_AUTO_BUFFER_SHIFT|DSP2F_NO_ADDR_CHK)

    mov al,[dsp2input]
    CommandJmp 01h,.w00p01
    CommandJmp 03h,.w00p03
    CommandJmp 05h,.w00p05
    CommandJmp 06h,.w00p06
    CommandJmp 09h,.w00p09
    CommandJmp 0Dh,.w00p0D
    CommandJmp 0Fh,.w00p0F
    jmp .gohalt

.w00p0D ; ----
    QueueInsideCommand 0Dh
    mov dword[dsp2enforcer+0],9
    mov dword[dsp2enforcer+4],8000h
    call _DSP2Add2Queue
    jmp .done

.w00p09 ; ----
    QueueInsideCommand 09h
    xor eax,eax
    mov al,4
.w00p09loop
    mov dword[dsp2enforcer+0],8
    mov dword[dsp2enforcer+4],8000h
    mov [dsp2enforcer+1],ah
    call _DSP2Add2Queue

    inc ah
    dec al
    jnz .w00p09loop

    jmp .queueincoming

.w00p06 ; ----
    QueueInsideCommand 06h
    mov dword[dsp2enforcer+0],6
    mov dword[dsp2enforcer+4],8000h
    call _DSP2Add2Queue
    jmp .done

.w00p05 ; ----
    or byte[dsp2state],DSP2F_NO_ADDR_CHK

    QueueInsideCommand 05h
    mov dword[dsp2enforcer+0],3
    mov dword[dsp2enforcer+4],8000h
    call _DSP2Add2Queue
    jmp .done

.w00p03 ; ----
    QueueInsideCommand 03h
    mov dword[dsp2enforcer+0],2
    mov dword[dsp2enforcer+4],8000h
    call _DSP2Add2Queue
    jmp .queueincoming

.w00p01 ; ----
    QueueInsideCommand 01h

    xor eax,eax
.w00p01loop
    mov dword[dsp2enforcer+0],1
    mov dword[dsp2enforcer+4],8000h
    mov [dsp2enforcer+4],al
    call _DSP2Add2Queue

    inc al
    cmp al,32
    jne .w00p01loop

    xor ecx,ecx
    jmp .queueincoming

.w00p0F ; ----
    QueueInsideCommand 0Fh

.queueincoming
    mov dword[dsp2enforcer+0],0
    mov dword[dsp2enforcer+4],8000h
    call _DSP2Add2Queue
    jmp .done

.done
    LeaveInsideCommand
    mov eax,[dsp2enforcerReaderCursor]
    inc eax
    and eax,511
    mov [dsp2enforcerReaderCursor],eax

    xor eax,eax
    jmp .leave
.gohalt
    QueueInsideCommand 0ffh
    or byte[dsp2state],DSP2F_HALT
.halt
    xor eax,eax
.leave
    ret

NEWSYM DSP2Write16b
.enter
    xor eax,eax
.leave
    ret
