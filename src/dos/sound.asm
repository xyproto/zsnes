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

EXTSYM ProcessSoundBuffer,DosExit,getenv,PrintStr,printhex,WaitForKey
EXTSYM SBHDMA,soundon,csounddisable,DisplayS,SPCRAM,DSPMem
EXTSYM StereoSound,SoundQuality,SoundSpeeds,SBToSPCSpeeds2
EXTSYM SoundSpeedt,DSPBuffer,BufferSize,BufferSizes,BufferSizeB
EXTSYM BufferSizeW,dssel,PrintChar

SECTION .text

printnum:
    ; process through each digit
    push edx
    push eax
    push ebx
    push cx
    xor edx,edx           ; clear high byte
    xor cx,cx             ; clear counter variable
    mov ebx,10
.loopa
    div ebx              ; get quotent and remainder
    push edx              ; store number to stack
    inc cl
    xor edx,edx
    test eax,0FFFFFFFFh
    jnz .loopa
.loopb
    pop edx              ; get number back from stack
    add dl,30h          ; adjust to ASCII value
    call PrintChar
    dec cl
    jnz .loopb
    pop cx
    pop ebx
    pop eax
    pop edx
    ret

NEWSYM SB_alloc_dma

    mov ax,0100h                ; Allocate DOS memory
    mov bx,16384/16             ; Allocate 16384 bytes
    int 31h
    ; To delocate this, use ax=0101h, dx=selector of block/int 31h
    jc near .error
    ; Check which 8192 byte boundary doesn't cross a page
    mov word[memoryloc+2],0
    mov dword[memoryloc],0
    mov [memoryloc],ax
    mov [sbselec],dx
    shl dword[memoryloc],4
    mov edx,[memoryloc]
    shr edx,16
    mov al,dl
    mov edx,[memoryloc]
    add edx,8192
    shr edx,16
    mov dword[sbpmofs],0
    cmp al,dl
    je .nonextarea
    mov dword[sbpmofs],8192
    add dword[memoryloc],8192
.nonextarea
    mov edi,[sbpmofs]
    mov [SBBufferLoc],edi
    mov dword[SBBufferMov],1
    mov dword[SBBufferInc],4
    ; clear dos memory
    push es
    mov es,[sbselec]
    mov edi,[sbpmofs]
    mov ecx,2048
    mov eax,0
    rep stosd
    pop es
    ret

.error
    mov edx,.nohand         ;use extended
    mov ah,9                ;DOS- API
    int 21h                 ;to print a string
    call DosExit

SECTION .data
.nohand db 'Unable to allocate conventional memory!',13,10,'$'
SECTION .text


NEWSYM DeInitSPC
    cmp byte[SBDeinitType],0
    je .nodoublereset
    call SB_dsp_reset
    call SB_dsp_reset
.nodoublereset
    ; Turn off speakers
    mov al,0d3h
    call SB_dsp_write

;      k) Perform Halt DMA Operation, 8-bit command (0D0h - for virtual speaker)
    mov al,0d0h
    call SB_dsp_write
;      l) Perform Exit Auto-Initialize DMA Operation, 8-bit command (0DAh)
    cmp byte[SBHDMA],0
    je .8b
    mov al,0d9h
    call SB_dsp_write
    jmp .16b
.8b
    mov al,0dAh
    call SB_dsp_write
.16b
;      m) Perform Halt DMA Operation, 8-bit command (0D0h - for virtual speaker)
    mov al,0d0h
    call SB_dsp_write
    ; Disable DMA
    mov al,4
    add al,[SBDMA]
    mov dx,0ah
    out dx,al
    ret

section .data

;SoundBlaster DSP Ports
NEWSYM SBPort, dw 220
NEWSYM SBInt,  db 5+8
NEWSYM SBIrq,  db 5
NEWSYM SBDMA,  db 1
NEWSYM SBDMAPage, db 83
;NEWSYM SBHDMA, db 0
NEWSYM SBHDMAPage, db 0
NEWSYM vibracard, db 0

NEWSYM SBBufferLoc,    dd 0
NEWSYM SBBufferMov,    dd 0
NEWSYM SBBufferInc,    dd 0
NEWSYM SoundInterrupt, dd 0


; ViBRA16X fixes!
EXTSYM MsgCount         ; points to counter
EXTSYM MessageOn        ; points to "message" delay counter
EXTSYM Msgptr           ; points to the message to be displayed
NEWSYM vibmsg, db 'VIBRA16X MODE ENABLED', 0

section .text

NEWSYM SB_dsp_reset
    mov dx,[SBPort]
    add dl,06h
    mov al,01h
    out dx,al
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    mov al,00h
    out dx,al

    mov si,200
    mov dx,[SBPort]
    add dl,0Eh
.readloop

    ; wait until port[SBDSPRdStat] AND 80h = 80h
    mov cx,20000
.tryagain
    in al,dx
    dec cx
    jz .cardfailed
    or al,al
    jns .tryagain
    sub dx,4
    in al,dx
    cmp al,0AAh
    jne .tryagain2
    ret
.tryagain2
    add dx,4
    dec si
    jnz .readloop
.cardfailed
    mov ax,0003h
    int 10h
    mov edx,initfailed      ;use extended
    mov ah,9                ;DOS- API
    int 21h                 ;to print a string
    jmp DosExit

section .data
NEWSYM initfailed, db 'Sound card failed to initialize!',13,10,'$'
section .text

; Write AL into DSP port
NEWSYM SB_dsp_write
    mov dx,[SBPort]
    add dl,0Ch
    mov bl,al
.tryagain
    in al,dx
    test al,80h
    jnz .tryagain
    mov al,bl
    out dx,al
    ret

; Read DSP port into AL
NEWSYM SB_dsp_read
    mov dx,[SBPort]
    add dl,0Eh
    mov bl,al
.tryagain
    in al,dx
    test al,80h
    jz .tryagain
    mov dx,[SBPort]
    add dl,0Ah
    mov al,bl
    in al,dx
    ret

;****************************************************
; Sound Blaster Interrupt Stuff
;****************************************************

NEWSYM Interror
    sti
    mov edx,.nohand         ;use extended
    mov ah,9                ;DOS- API
    int 21h                 ;to print a string
    call DosExit

section .data
.nohand db 'Cannot process interrupt handler!',13,10,'$'

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
    test byte[DSPMem+6Ch],11000000b
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
    add edi,[BufferSizeB]
.startblockcopy

    mov esi,DSPBuffer
    mov ecx,[BufferSizeB]
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
      mov bl,[DSPMem+eax]
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
    mov edi,DSPBuffer
    mov ecx,[BufferSizeB]
    xor eax,eax
    rep stosd

    cmp byte[SBswitch],0
    jne near .2ndblock

    ; clear block
    mov es,[sbselec]
    mov edi,[sbpmofs]
    mov ecx,[BufferSizeB]
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
    add edi,[BufferSizeB]
    mov ecx,[BufferSizeB]
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
    test byte[DSPMem+6Ch],11000000b
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
    add edi,[BufferSizeW]
.doneblock
    mov esi,DSPBuffer
    mov ecx,[BufferSizeB]
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
    call ProcessSoundBuffer
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
    mov edi,DSPBuffer
    mov ecx,[BufferSizeB]
    xor eax,eax
    rep stosd

    cmp byte[SBswitch],0
    jne near .2ndblock

    ; clear block
    mov es,[sbselec]
    mov edi,[sbpmofs]
    mov ecx,[BufferSizeB]
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
    add edi,[BufferSizeW]
    mov ecx,[BufferSizeB]
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

;****************************************************
; Sound Blaster Initialization Stuff
;****************************************************

section .bss
NEWSYM memoryloc, resd 1        ; Memory offset in conventional memory
NEWSYM memoryloc2, resd 1       ; Memory offset in conventional memory
NEWSYM sbselec,   resw 1        ; Selector of Memory location
NEWSYM sbpmofs,   resd 1        ; offset of Memory location
SBDeinitType resb 1
section .text

NEWSYM InitSB
    mov eax,[SoundQuality]
    cmp byte[StereoSound],1
    jne .nostereobuf
    mov ax,[BufferSizes+eax*2]
    jmp .skipstereobuf
.nostereobuf
    mov ax,[BufferSize+eax*2]
.skipstereobuf

    mov [BufferSizeB],ax
    add ax,ax
    mov [BufferSizeW],ax

    mov byte[SBswitch],0
    ; Allocate pointer
    ; Set up SB
    call SB_dsp_reset

    ; code added by peter santing
    cmp byte[vibracard], 1
    je  near .vibrafix2

    cmp byte[SBHDMA],0
    je .no16bit
    cmp byte[SBHDMA],4
    jb near .init16bitlowhdma
    jmp .init16bit
.no16bit

    ; Determine Version #
    mov al,0E1h
    call SB_dsp_write
    call SB_dsp_read
    mov [.Versionnum],al
    call SB_dsp_read
    mov [.Versionnum+1],al

    ; Turn on speakers
    mov al,0D1h
    call SB_dsp_write

    ; Set Time-Constant Data ( = 256 - (1000000/sampling rate) )
    ; 8000=131, 22050=210, 44100=233, 11025=165
    mov al,40h
    call SB_dsp_write

    cmp byte[StereoSound],1
    jne .nostereo8b
.surround8b
    mov eax,[SoundQuality]
    cmp eax,4
    je .okay
    cmp eax,2
    jbe .okay
    mov eax,2
.okay
    mov al,[SoundSpeedt+eax]
    call SB_dsp_write
    ; Set Stereo
    mov dx, [SBPort]
    add dx, 04h
    mov al,0Eh
    out dx,al
    inc dx
    in  al,dx
    or  al,022h
    out dx,al
    jmp .donestereo
.nostereo8b
    mov eax,[SoundQuality]
    mov al,[SoundSpeeds+eax]
    call SB_dsp_write
.donestereo

    cmp byte[StereoSound],1
    je .highmode
    mov eax,[SoundQuality]
    cmp byte[SoundSpeeds+eax],211
    ja .highmode
    mov byte[.Versionnum],1
.highmode
    ; Setup DMA
    ; Select DMA channel
    mov al,[SBDMA]
    add al,4
    mov dx,000Ah
    out dx,al
    ; Clear DMA
    mov al,00h
    mov dx,000Ch
    out dx,al
    ; Set autoinit/write (set as DAC)
    mov al,58h
    add al,[SBDMA]
    mov dx,000Bh
    out dx,al
    ; Send Offset Address
    mov al,[memoryloc]
    mov dl,[SBDMA]
    shl dl,1
    out dx,al
    mov al,[memoryloc+1]
    out dx,al
    ; Send length of entire block
    mov ax,[BufferSizeW]
    dec ax
    inc dx
    out dx,al
    mov al,ah
    out dx,al
    ; Send page # (address/65536)
    mov al,[memoryloc+2]
    mov dl,[SBDMAPage]
    out dx,al
    ; turn on DMA
    mov al,[SBDMA]
    mov dx,000Ah
    out dx,al

    ; Prepare SB for the first block
    ; 8-bit auto-init, mono, unsigned
    mov al,048h   ; Sb 2.0 version...
    call SB_dsp_write

    ; Send Length-1 to DSP port
    mov ax,[BufferSizeB]
    dec ax
    call SB_dsp_write
    mov al,ah
    call SB_dsp_write
    mov byte[SBDeinitType],1
    mov al,090h   ; Sb 2.0 version...
    cmp byte[.Versionnum],2
    jne .noversion2
    cmp byte[.Versionnum+1],0
    je .slowspeed
.noversion2
    cmp byte[.Versionnum],1
    ja .notversion1
.slowspeed
    mov byte[SBDeinitType],0
    mov al,1Ch
.notversion1
    call SB_dsp_write
    jmp .fixsurround

SECTION .bss
.Versionnum resw 1
SECTION .text

; *****************************************
; **** alternate ViBRA16X SB init code **** by Peter Santing
; ***************************************** copied portions of original code
; and modified it.

.vibrafix2
    ; notify user that we're in ViBRA16x mode..
    push eax
    mov  dword[Msgptr], vibmsg
    mov  eax, [MsgCount]
    mov  [MessageOn], eax
    pop  eax

    ; Set Time-Constant Data ( = 256 - (1000000/sampling rate) )
    ; 8000=131, 22050=210, 44100=233, 11025=165

    ; Setup DMA
    ; Select DMA channel
    mov al,[SBDMA]
    add al,4
    mov dx,000Ah
    out dx,al
    ; Clear DMA
    mov al,00h
    mov dx,000Ch
    out dx,al
    ; Set autoinit/write (set as DAC)
    mov al,58h
    add al,[SBDMA]
    mov dx,000Bh
    out dx,al
    ; Send Offset Address
    mov al,[memoryloc]
    mov dl,[SBDMA]
    shl dl,1
    out dx,al
    mov al,[memoryloc+1]
    out dx,al
    ; Send length of entire block
    mov ax,[BufferSizeW]
    shl ax, 1
    dec ax
    inc dx
    out dx,al
    mov al,ah
    out dx,al
    ; Send page # (address/65536)
    mov al,[memoryloc+2]
    mov dh, 0
    mov dl,[SBDMAPage]
    out dx,al
    ; turn on DMA
    mov al,[SBDMA]
    mov dx,000Ah
    out dx,al

    mov al,41h
    call SB_dsp_write
    push ecx
    mov ecx,[SoundQuality]
    mov al,[SBToSPCSpeeds2+ecx*4+1]
    pop ecx
    call SB_dsp_write
    push ecx
    mov ecx,[SoundQuality]
    mov al,[SBToSPCSpeeds2+ecx*4]
    pop ecx
    call SB_dsp_write

    ; Prepare SB for the first block
    ; 16-bit auto-init, mono, unsigned
    mov al,0B6h   ; Sb 16 version (DSP 4)
    call SB_dsp_write
    cmp byte[StereoSound],1
    jne ._Mono
._surround
    mov al,30h    ; stereo/signed
    call SB_dsp_write
    jmp ._AfterStereo
._Mono
    mov al,10h    ; mono/signed
    call SB_dsp_write
._AfterStereo

    ; Send Length-1 to DSP port
    mov ax,[BufferSizeB]
    dec ax
    call SB_dsp_write
    mov al,ah
    call SB_dsp_write

    ; Turn on speakers
    mov al,0D1h
    call SB_dsp_write

    jmp .fixsurround

; ******* end of alternate SB init code for ViBRA ********

.init16bitlowhdma
    ; Set Time-Constant Data ( = 256 - (1000000/sampling rate) )
    ; 8000=131, 22050=210, 44100=233, 11025=165
    mov al,40h
    call SB_dsp_write

    push ecx
    mov ecx,[SoundQuality]
    mov al,[SoundSpeeds+ecx]
    pop ecx
    call SB_dsp_write

    mov edx,[memoryloc]
    shr edx,1
    mov [memoryloc2],edx

    ; Setup DMA

    ; turn off DMA
;    mov al,[SBHDMA]
;    and al,03h
;    or al,04h
;    mov dx,00D4h
;    out dx,al

    ; Setup DMA
    ; Select DMA channel
    mov al,[SBHDMA]
    and al,03h
    or al,04h
    mov dx,000Ah
    out dx,al

    ; clear flip-flop
    mov dx,00D8h
    xor al,al
    out dx,al

    ; Set autoinit/write (set as DAC)
    mov al,[SBHDMA]
    and al,3
    add al,58h
    mov dx,00D6h
    out dx,al

    ; Send Offset Address
;    mov al,[memoryloc2]
;    mov dl,[SBHDMA]
;    and dl,3
;    shl dl,2
;    add dl,0C0h
;    out dx,al
;    mov al,[memoryloc2+1]
;    out dx,al

    ; Send Offset Address
    mov al,[memoryloc]
    mov dl,[SBDMA]
    shl dl,1
    out dx,al
    mov al,[memoryloc+1]
    out dx,al

    ; Send length of entire block
    mov ax,[BufferSizeW]
    dec ax
    add dx,2
    out dx,al
    mov al,ah
    out dx,al

    ; Send page # (address/65536)
    mov al,[memoryloc+2]
    mov dl,[SBHDMAPage]
    out dx,al

    ; Prepare SB for the first block
    ; 16-bit auto-init, mono, unsigned
    mov al,0B6h   ; Sb 16 version (DSP 4)
    call SB_dsp_write
    cmp byte[StereoSound],1
    jne .Monol
.surroundl
    mov al,30h    ; stereo/signed
    call SB_dsp_write
    jmp .AfterStereol
.Monol
    mov al,10h    ; mono/signed
    call SB_dsp_write
.AfterStereol

    ; Send Length-1 to DSP port
    mov ax,[BufferSizeB]
    dec ax
    call SB_dsp_write
    mov al,ah
    call SB_dsp_write

    ; turn on DMA
;    mov al,[SBHDMA]
;    and al,03h
;    mov dx,00D4h
;    out dx,al

    ; Setup DMA
    ; Select DMA channel
    mov al,[SBHDMA]
    and al,03h
    mov dx,000Ah
    out dx,al

    ; Turn on speakers
    mov al,0D1h
    call SB_dsp_write
    jmp .fixsurround

.init16bit
    ; Set Time-Constant Data ( = 256 - (1000000/sampling rate) )
    ; 8000=131, 22050=210, 44100=233, 11025=165
    mov al,41h
    call SB_dsp_write
    push ecx
    mov ecx,[SoundQuality]
    mov al,[SBToSPCSpeeds2+ecx*4+1]
    pop ecx
    call SB_dsp_write
    push ecx
    mov ecx,[SoundQuality]
    mov al,[SBToSPCSpeeds2+ecx*4]
    pop ecx
    call SB_dsp_write

    mov edx,[memoryloc]
    shr edx,1
    mov [memoryloc2],edx

    ; Setup DMA

    ; turn off DMA
    mov al,[SBHDMA]
    and al,03h
    or al,04h
    mov dx,00D4h
    out dx,al

    ; clear flip-flop
    mov dx,00D8h
    xor al,al
    out dx,al

    ; Set autoinit/write (set as DAC)
    mov al,[SBHDMA]
    and al,3
    add al,58h
    mov dx,00D6h
    out dx,al

    ; Send Offset Address
    mov al,[memoryloc2]
    mov dl,[SBHDMA]
    and dl,3
    shl dl,2
    add dl,0C0h
    out dx,al
    mov al,[memoryloc2+1]
    out dx,al

    ; Send length of entire block
    mov ax,[BufferSizeW]
    dec ax
    add dx,2
    out dx,al
    mov al,ah
    out dx,al

    ; Send page # (address/65536)
    mov al,[memoryloc+2]
    mov dl,[SBHDMAPage]
    and al,0FEh
    out dx,al

    ; Prepare SB for the first block
    ; 16-bit auto-init, mono, unsigned
    mov al,0B6h   ; Sb 16 version (DSP 4)
    call SB_dsp_write
    cmp byte[StereoSound],1
    jne .Mono
.surround
    mov al,30h    ; stereo/signed
    call SB_dsp_write
    jmp .AfterStereo
.Mono
    mov al,10h    ; mono/signed
    call SB_dsp_write
.AfterStereo

    ; Send Length-1 to DSP port
    mov ax,[BufferSizeB]
    dec ax
    call SB_dsp_write
    mov al,ah
    call SB_dsp_write

    ; Turn on speakers
    mov al,0D1h
    call SB_dsp_write

    ; turn on DMA
    mov al,[SBHDMA]
    and al,03h
    mov dx,00D4h
    out dx,al

.fixsurround
    ret


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
    mov cx,[BufferSizeB]
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

NEWSYM SB_quality_limiter
      cmp byte[StereoSound],1
      jne .nostereo8b
      cmp byte[SBHDMA],0
      jne .nostereo8b

      ; *****************************************
      ; *** ViBRA16X support by Peter Santing ***
      ; *****************************************
      ; before REALLY switching back to 8-bit sucky mono mode
      ; check that we're dealing with a ViBRA16X Creative Labs Card
      cmp byte[vibracard], 1
      je .nostereo8b

      cmp dword[SoundQuality],2
      jbe .nostereo8b
      cmp dword[SoundQuality],4
      je .nostereo8b
      mov dword[SoundQuality],2
.nostereo8b
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

;*******************************************************
; Get Blaster            Locates SET BLASTER environment
;*******************************************************
NEWSYM getblaster
    mov edx,.string2s
    push edx
    call getenv
    pop edx
    cmp eax,0
    je near .nfound

    mov esi,eax
    mov byte[.cursetting],0
.a
    mov dl,[esi]
    cmp dl,'a'
    jb .nocap
    cmp dl,'z'
    ja .nocap
    sub dl,'a'-'A'
.nocap
    inc esi
    mov byte[.blfound],1
    cmp dl,'A'
    jne .afound
    mov byte[.cursetting],1
    mov word[SBPort],0
    jmp .src
.afound
    cmp dl,'I'
    jne .ifound
    mov byte[.cursetting],2
    mov byte[SBIrq],0
    jmp .src
.ifound
    cmp dl,'D'
    jne .dfound
    mov byte[.cursetting],3
    mov byte[SBDMA],0
    jmp .src
.dfound
    cmp dl,'H'
    jne .hfound
    mov byte[.cursetting],4
    mov byte[SBHDMA],0
    jmp .src
.hfound
    cmp dl,' '
    je .src2
    cmp dl,0
    je .src2
    jmp .src3
.src2
    mov byte[.cursetting],0
    jmp .src
.src3
    cmp byte[.cursetting],1
    jne .nproca
    shl word[SBPort],4
    sub dl,48
    add byte[SBPort],dl
    add dl,48
.nproca
    cmp byte[.cursetting],2
    jne .nproci
    cmp byte[SBIrq],1
    jne .no1
    mov byte[SBIrq],10
.no1
    sub dl,48
    add [SBIrq],dl
    add dl,48
.nproci
    cmp byte[.cursetting],3
    jne .nprocd
    sub dl,48
    mov [SBDMA],dl
    add dl,48
.nprocd
    cmp byte[.cursetting],4
    jne .nproch
    sub dl,48
    mov [SBHDMA],dl
    add dl,48
.nproch
.src
    cmp dl,0
    jne near .a
    cmp byte[.blfound],0
    je near .nfound
    cmp byte[SBIrq],2
    jne .noirq9
    mov byte[SBIrq],9
.noirq9
    mov al,[SBIrq]
    add al,08h
    cmp byte[SBIrq],7
    jbe .nohighirq
    add al,60h
    add byte[PICRotateP],80h
    add byte[PICMaskP],80h
.nohighirq
    mov [SBInt],al
    cmp byte[SBDMA],0
    jne .dma0
    mov byte[SBDMAPage],87h
.dma0
    cmp byte[SBDMA],1
    jne .dma1
    mov byte[SBDMAPage],83h
.dma1
    cmp byte[SBDMA],2
    jne .dma2
    mov byte[SBDMAPage],81h
.dma2
    cmp byte[SBDMA],3
    jne .dma3
    mov byte[SBDMAPage],82h
.dma3
; ******************************************************
; **** this piece of code is added by Peter Santing ****
; **** it will enable ZSNES to use the full STEREO  ****
; **** capability of the ViBRA16X line of creative  ****
; **** instead of playing 8-bit MONOURAL sound      ****
; ******************************************************
;       cmp byte[SBHDMA], 0
;       jne .vibradma0
;       mov byte[SBDMAPage], 87h
;       mov byte[vibracard], 1         ; set ViBRA16X mode
.vibradma0
        cmp byte[SBHDMA], 1
        jne .vibradma1
        mov byte[SBDMAPage], 83h
        mov byte[vibracard], 1         ; set ViBRA16X mode
.vibradma1
        cmp byte[SBHDMA], 2
        jne .vibradma2
        mov byte[SBDMAPage], 81h
        mov byte[vibracard], 1         ; set ViBRA16X mode
.vibradma2
        cmp byte[SBHDMA], 3
        jne .vibradma3
        mov byte[SBDMAPage], 82h
        mov byte[vibracard], 1         ; set ViBRA16X mode
.vibradma3
        cmp byte[vibracard], 1
        jne .vibrafix
        push ax
        mov  al, [SBHDMA]
        mov  [SBDMA], al
        pop  ax
.vibrafix
        cmp byte[SBHDMA],4
        jae .hdma
        ; vibra implementation (make sure that zSNES doesn't go back
        ; to eight-bit-mode mono)
        mov byte[SBHDMA],0
        cmp byte[vibracard], 1
        jne .hdma
        push edx
        mov edx, vibradetect
        call PrintStr
        ;call WaitForKey
        pop  edx

; ********** END OF ViBRA16X implementation code **********
.hdma
    cmp byte[SBHDMA],4
    jne .hdma4
    mov byte[SBHDMAPage],8Fh
.hdma4
    cmp byte[SBHDMA],5
    jne .hdma5
    mov byte[SBHDMAPage],8Bh
.hdma5
    cmp byte[SBHDMA],6
    jne .hdma6
    mov byte[SBHDMAPage],89h
.hdma6
    cmp byte[SBHDMA],7
    jne .hdma7
    mov byte[SBHDMAPage],8Ah
.hdma7
    cmp byte[DisplayS],1
    je .displaysoundstuff
    ret
.nfound
    cmp byte[soundon],0
    je .nosound
    mov byte[soundon],0
    mov edx, .blasterstr
    call PrintStr
    call WaitForKey
.nosound
    ret
.displaysoundstuff
    mov edx,.blasterinfo
    call PrintStr
    xor eax,eax
    mov ax,[SBPort]
    call printhex
    mov edx,.blinfob
    call PrintStr
    xor eax,eax
    mov al,[SBIrq]
    call printnum
    mov edx,.blinfoc
    call PrintStr
    xor eax,eax
    mov al,[SBDMA]
    call printnum
    mov edx,.blinfod
    call PrintStr
    xor eax,eax
    mov al,[SBHDMA]
    call printnum
    mov edx,.blasterstr2b
    call PrintStr
    call WaitForKey
    ret

SECTION .bss
.blfound  resb 1
.cursetting resb 1

SECTION .data
.string2s db 'BLASTER',0
.blasterstr db 'ERROR : SET BLASTER environment NOT found!',10,13
.blasterstr2 db 'Unable to enable sound.'
.blasterstr2b db 10,13,10,13
.blasterstr3 db 'Press any key to continue.',0
.blasterinfo db 'Sound Blaster Detection Values : ',10,13,10,13
.blinfoa db 'PORT  : ',0
.blinfob db 13,10,'IRQ   : ',0
.blinfoc db 13,10,'DMA   : ',0
.blinfod db 13,10,'HDMA  : ',0

NEWSYM PICRotateP, db 20h
NEWSYM PICMaskP,   db 21h

; Line added by Peter Santing
NEWSYM vibradetect
                 db 'Creative ViBRA16X PnP card detected (support coded by Peter Santing)', 13, 10
                 db 'High-DMA is below dma #4', 13, 10
                 db 13,10, 'you have now full 16-bit stereo sound with the surround option!', 13, 10, 0


