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

EXTSYM csounddisable,SPCRAM
EXTSYM dssel
EXTSYM SB_quality_limiter
;EXTSYM DSPMem,DSPBuffer,BufferSizeB,BufferSizeW
;EXTSYM ProcessSoundBuffer

section .data

;SoundBlaster DSP Ports
NEWSYM SBPort, dw 220
NEWSYM SBIrq,  db 5
NEWSYM SBDMA,  db 1
NEWSYM SBDMAPage, db 83
NEWSYM SBHDMA, db 0
NEWSYM SBHDMAPage, db 0
NEWSYM vibracard, db 0


;****************************************************
; Sound Blaster Interrupt Stuff
;****************************************************

section .bss

NEWSYM oldhandSBs, resw 1
NEWSYM oldhandSBo, resd 1
NEWSYM SBswitch,   resb 1         ; which block to process next

section .text

NEWSYM SBHandler
    cli
    push ds
    push eax


NEWSYM handlersbseg
    mov ax,[cs:dssel]
    mov ds,ax

    cmp byte[SBHDMA],0
    jne near SBHandler16

    ; code added by peter santing
    cmp byte[vibracard], 1
    je  near SBHandler16

    push ebx
    push ecx
    push edx
    push edi
    push esi
    push es

    call GetCDMAPos

    cmp byte[csounddisable],1
    je near stopsbsound
    ;test byte[DSPMem+6Ch],11000000b
    jnz near stopsbsound

    ; Process the sound :I

    mov es,[sbselec]
    cmp byte[SBswitch],0
    jne .2ndblock
    mov edi,[sbpmofs]
    jmp .startblockcopy
.2ndblock
    ; copy to 2nd block
    ; clear memory
    mov edi,[sbpmofs]
    ;add edi,[BufferSizeB]
.startblockcopy

    ;mov esi,DSPBuffer
    ;mov ecx,[BufferSizeB]
.loopb
    mov eax,[esi]
    cmp eax,-32768
    jge .noneg3
    mov eax,-32768
.noneg3
    cmp eax,32767
    jle .noneg4
    mov eax,32767
.noneg4
    xor ah,80h
    mov [es:edi],ah
    add esi,4
    inc edi
    dec ecx
    jnz .loopb
    jmp .sbend
.sbend
    xor byte[SBswitch],1

   ; move the good data at SPCRAM+0f3h
      xor eax,eax
      mov al,[SPCRAM+0F2h]
      ;mov bl,[DSPMem+eax]
      mov [SPCRAM+0F3h],bl
    ; acknowledge SB for IRQing
    mov dx,[SBPort]
    add dl,0Eh
    in al,dx
    mov al,20h
    out 20h,al
    cmp byte[SBIrq],7
    jbe .nohighirq
    mov al,20h
    out 0A0h,al
.nohighirq
    sti
    jmp Startprocsbdata

NEWSYM stopsbsound
;    mov byte[Voice0Status],0
;    mov byte[Voice1Status],0
;    mov byte[Voice2Status],0
;    mov byte[Voice3Status],0
;    mov byte[Voice4Status],0
;    mov byte[Voice5Status],0
;    mov byte[Voice6Status],0
;    mov byte[Voice7Status],0

    mov ax,ds
    mov es,ax
    ;mov edi,DSPBuffer
    ;mov ecx,[BufferSizeB]
    xor eax,eax
    rep stosd

    cmp byte[SBswitch],0
    jne near .2ndblock

    ; clear block
    mov es,[sbselec]
    mov edi,[sbpmofs]
    ;mov ecx,[BufferSizeB]
    shr ecx,2
.loopa
    mov dword[es:edi],80808080h
    add edi,4
    dec ecx
    jnz .loopa
    jmp .sbend
.2ndblock
    ; copy to 2nd block
    ; clear memory
    mov es,[sbselec]
    mov edi,[sbpmofs]
    ;add edi,[BufferSizeB]
    ;mov ecx,[BufferSizeB]
    shr ecx,2
.loopb
    mov dword[es:edi],80808080h
    add edi,4
    dec ecx
    jnz .loopb
.sbend
    xor byte[SBswitch],1

    ; acknowledge SB for IRQing
    mov dx,[SBPort]
    add dl,0Eh
    in al,dx
    mov al,20h
    out 20h,al
    cmp byte[SBIrq],7
    jbe .nohighirq
    mov al,20h
    out 0A0h,al
.nohighirq

    pop es
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ds
    sti
    iretd

section .bss
NEWSYM sbhandexec, resd 1
section .text

; Process 20 blocks * 8 voices (no pitch yet)
NEWSYM SBHandler16
    push ebx
    push ecx
    push edx
    push edi
    push esi
    push es
    inc dword[sbhandexec]

    cmp byte[vibracard], 1
    je .donotcallcmdapos
    call GetCDMAPos
.donotcallcmdapos

    cmp byte[csounddisable],1
    je near stopsbsound16
    ;test byte[DSPMem+6Ch],11000000b
    jnz near stopsbsound16

    mov es,[sbselec]

    cmp byte[SBswitch],0
    jne near .2ndblock
    mov edi,[sbpmofs]
    jmp .doneblock
.2ndblock
    ; copy to 2nd block
    ; clear memory
    mov edi,[sbpmofs]
    ;add edi,[BufferSizeW]
.doneblock
    ;mov esi,DSPBuffer
    ;mov ecx,[BufferSizeB]
.loopb
    mov eax,[esi]
    cmp eax,-32768
    jge .noneg5
    mov eax,-32768
.noneg5
    cmp eax,32767
    jle .noneg6
    mov eax,32767
.noneg6
    mov [es:edi],ax
    add esi,4
    add edi,2
    dec ecx
    jnz .loopb
    jmp .sbend
.sbend
    xor byte[SBswitch],1

    ; acknowledge SB for IRQing
    mov dx,[SBPort]
    add dl,0Fh
    in al,dx
    mov al,20h
    out 20h,al
    cmp byte[SBIrq],7
    jbe .nohighirq
    mov al,20h
    out 0A0h,al
.nohighirq
    sti


Startprocsbdata:
    push ebp
    ;call ProcessSoundBuffer
    pop ebp
    pop es
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ds
    iretd

NEWSYM stopsbsound16
;    mov byte[Voice0Status],0
;    mov byte[Voice1Status],0
;    mov byte[Voice2Status],0
;    mov byte[Voice3Status],0
;    mov byte[Voice4Status],0
;    mov byte[Voice5Status],0
;    mov byte[Voice6Status],0
;    mov byte[Voice7Status],0

    mov ax,ds
    mov es,ax
    ;mov edi,DSPBuffer
    ;mov ecx,[BufferSizeB]
    xor eax,eax
    rep stosd

    cmp byte[SBswitch],0
    jne near .2ndblock

    ; clear block
    mov es,[sbselec]
    mov edi,[sbpmofs]
    ;mov ecx,[BufferSizeB]
    shr ecx,1
.loopa
    mov dword[es:edi],00000000h
    add edi,4
    dec ecx
    jnz .loopa
    jmp .sbend
.2ndblock
    ; copy to 2nd block
    ; clear memory
    mov es,[sbselec]
    mov edi,[sbpmofs]
    ;add edi,[BufferSizeW]
    ;mov ecx,[BufferSizeB]
    shr ecx,1
.loopb
    mov dword[es:edi],00000000h
    add edi,4
    dec ecx
    jnz .loopb
.sbend
    xor byte[SBswitch],1

    ; acknowledge SB for IRQing
    mov dx,[SBPort]
    add dl,0Fh
    in al,dx
    mov al,20h
    out 20h,al
    cmp byte[SBIrq],7
    jbe .nohighirq
    mov al,20h
    out 0A0h,al
.nohighirq

    pop es
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ds
    sti
    iretd

section .bss
NEWSYM sbselec,   resw 1        ; Selector of Memory location
NEWSYM sbpmofs,   resd 1        ; offset of Memory location
section .text

GetCDMAPos:
    ; clear flipflop
    xor ebx,ebx
    mov bl,[SBDMA]
    cmp byte[SBHDMA],4
    jb .nohdma
    mov bl,[SBHDMA]
    mov dx,0Ch
.nohdma
    mov dx,0D8h
    xor al,al
    out dx,al
    nop
    nop
    nop
    nop
    mov dx,[.wordcountport+ebx*2]

    in al,dx
    nop
    nop
    mov bl,al
    in al,dx
    nop
    nop
    nop
    nop
    mov bh,al
    cmp byte[SBHDMA],4
    jb .ldma2
    add bx,bx
.ldma2
    ; value returned = bx, # of bytes left for transfer
    ;mov cx,[BufferSizeB]
    mov dx,cx
    add cx,cx
    cmp byte[SBHDMA],4
    jb .ldmab
    add cx,cx
    add dx,dx
.ldmab
    sub cx,bx
    mov byte[SBswitch],1
    cmp cx,dx
    jb .parta
    mov byte[SBswitch],0
.parta
    ret
SECTION .data
.wordcountport dw 1,3,5,7,0C2h,0C6h,0CAh,0CEh
SECTION .text

; old routines, doesn't work w/ sb live!
    jmp .fin

.loop
    in al,dx
    nop
    nop
    mov cl,al
    in al,dx
    nop
    nop
    nop
    nop
    mov ch,al
    in al,dx
    nop
    nop
    mov bl,al
    in al,dx
    mov bh,al
    sub cx,bx
    test cx,8000h
    jz .notneg
    neg cx
.notneg
    cmp byte[SBHDMA],4
    jb .ldma
    add cx,cx
    add bx,bx
.ldma
    cmp cx,4
    ja .loop

.fin
		ccallv SB_quality_limiter
		ret

NEWSYM SB_blank
    push es
    mov es,[sbselec]
    mov edi,[sbpmofs]
    mov ecx,320
.loopa
    mov dword[es:edi],0
    add edi,4
    dec ecx
    jnz .loopa
    pop es
    ret

SECTION .data
NEWSYM PICMaskP,   db 21h
