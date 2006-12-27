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

EXTSYM vesa2_usbit,vesa2_clbit,vesa2_clbitng,vesa2_clbitng2,vesa2_clbitng3
EXTSYM vesa2_x,vesa2_y,vesa2_bits,vesa2_rpos,vesa2_gpos,vesa2_bpos,vesa2_rposng
EXTSYM vesa2_gposng,vesa2_bposng,vesa2_rtrcl,vesa2_rtrcla,vesa2_rfull
EXTSYM vesa2_gtrcl,vesa2_gtrcla,vesa2_gfull,vesa2_btrcl,vesa2_btrcla,vesa2_bfull
EXTSYM vesa2red10,videotroub,genfulladdtab,DosExit,noblocks
EXTSYM bytesperscanline,vesamode,VESAmodelist

; add 0214h video mode

SECTION .bss
NEWSYM granularity, resw 1
NEWSYM granadd, resd 1
SECTION .text

NEWSYM VESA12EXITTODOS
    mov ax,0003h
    int 10h
    push edx
    mov edx,.exitfromvesa12
    mov ah,9
    int 21h
    pop edx
    mov ah,9
    int 21h
    mov edx,.return
    mov ah,9
    int 21h

    mov byte[videotroub],1
    jmp DosExit


SECTION .data
.exitfromvesa12 db 'Unable to Initialize VESA1.2 $'
.return db 10,13,'$'
SECTION .text

;*******************************************************
;   Set up Vesa 2
;*******************************************************

NEWSYM InitVesa12
;-------------------------------------------------;
; First - allocate some bytes in DOS memory for ;
; communication with VBE                        ;
;-------------------------------------------------;

    mov eax,0100h
    mov ebx,512/16 ; 512 bytes
    int 31h        ; Function 31h,100h - Allocate DOS memory (512 bytes)
    jnc .gotmem
    mov edx,.nomemmessage
    jmp VESA12EXITTODOS
.gotmem
    mov fs,dx          ; FS now points to the DOS buffer


    ;--------------------------------------------------;
     ; Now, get information about the video card into ;
     ; a data structure                               ;
    ;--------------------------------------------------;

    mov edi,RMREGS
    mov dword[fs:0],'VBE1'      ; Request VBE 2.0 info
    mov dword[RMREGS.eax],4f00h
    mov [RMREGS.es],ax      ; Real mode segment of DOS buffer
    mov dword[RMREGS.edi],0

    push es
    push ds
    pop es
    mov eax,300h
    mov ebx,10h
    xor ecx,ecx
    int 31h                 ; Simulate real mode interrupt
    pop es

    jnc .int1ok
    mov edx,.noint1message
    jmp VESA12EXITTODOS

.int1ok             ; Real mode int successful!!!
    mov eax,[RMREGS.eax]
    cmp al,4fh      ; Check vbe interrupt went OK
    jz .vbedetected
    mov edx,.novbemessage
    jmp VESA12EXITTODOS

.vbedetected
    cmp dword[fs:0000],'VESA'
    jz .vesadetected         ; Check for presence of vesa
    mov edx,.novesamessage
    jmp VESA12EXITTODOS


.vesadetected
    cmp word[fs:0004],102h
    jae .vesa12detected      ; Check we've got VESA 1.2 or greater
    mov edx,.novesa2message
    jmp VESA12EXITTODOS


    ;-----------------------------------------------------;
     ; OK - vesa 2.0 or greater has been detected. Copy  ;
     ; mode information into VESAmodelist                ;
    ;-----------------------------------------------------;

.vesa12detected
    mov ax,[fs:12h]             ; Get no. of 64k blocks
    mov [noblocks],ax
    mov ax, 2
    mov bx,[fs:10h]
    int 31h

    jnc .wegottheselector
    mov edx, .oopsnoselector
    jmp VESA12EXITTODOS

.wegottheselector

    mov gs,ax
    xor eax,eax
    mov ebp,VESAmodelist
    mov ecx,512
    mov ax,[fs:0eh]

.loopcopymodes
    mov bx,[gs:eax]
    mov [ebp],bx
    cmp bx,0ffffh
    jz .copiedmodes
    add ebp,2
    add eax,2
    dec ecx
    jz .outofmodelistspace
    jmp .loopcopymodes

.outofmodelistspace
    mov edx,.outofmodelistspacemessage
    jmp VESA12EXITTODOS

    ;----------------------------------------------;
     ; OK - Scan the mode list to find a matching ;
     ; mode for vesa2_x, vesa2_y and vesa2_depth  ;
    ;----------------------------------------------;

.copiedmodes

    mov ebp,VESAmodelist
    xor ecx,ecx

.loopcheckmodes
    mov cx, [ebp]
    cmp cx, 0ffffh
    jnz .notendoflist

    mov edx,.endoflist
    jmp VESA12EXITTODOS

.notendoflist

    mov edi, RMREGS
    mov dword[RMREGS.eax],4f01h
    mov dword[RMREGS.ebx],0
    mov [RMREGS.ecx],ecx
    mov dword[RMREGS.edi],0

    push es
    push ds
    pop es
    mov eax,300h
    mov ebx,10h
    xor ecx,ecx
    int 31h             ; Simulate real mode interrupt
    pop es
    jnc .modecheckok
    mov edx,.modecheckfail
    jmp VESA12EXITTODOS

.modecheckok
    add ebp,2

    test word[fs:0000h],1b
    jz near .loopcheckmodes     ; If mode is not available

;
;        xor eax,eax
;        mov ax,[fs:12h]
;        call printnum
;        mov ah,02h
;        mov dl,'x'
;        int 21h
;        mov ax,[fs:14h]
;        call printnum
;        mov ah,02h
;        mov dl,'x'
;        int 21h
;        xor ah,ah
;        mov al,[fs:19h]
;        call printnum
;        mov ah,02h
;        mov dl,13
;        int 21h
;        mov dl,10
;        int 21h

    mov eax,[vesa2_x]
    cmp [fs:12h],ax             ; Check that the height matches
    jnz near .loopcheckmodes
    mov eax,[vesa2_y]
    cmp [fs:14h],ax             ; Check that the width matches
    jnz near .loopcheckmodes
    mov al,[vesa2_bits]
    cmp [fs:19h],al             ; Check bits/pixel for match
    jnz near .loopcheckmodes

;        mov ah,07h
;        int 21h

;        D0 = Window supported
;                0 = Window is not supported
;                1 = Window is supported
;        D1 = Window readable
;                0 = Window is not readable
;                1 = Window is readable
;        D2 = Window writeable
;                0 = Window is not writeable
;                1 = Window is writeable
;        D3-D7 = Reserved

    mov byte[.whichwin],0
    mov al,[fs:2]   ; Get window A attributes
    and al,0100b
    cmp al,0100b
    je .foundwin    ; Mode supported
    mov al,[fs:3]   ; Get window B attributes
    and al,0100b
    cmp al,0100b
    jne .foundwin   ; Mode not supported
    mov byte[.whichwin],1
.foundwin

    ; Success - a match has been found!!

    sub ebp,2
    mov ax,[ebp]
    mov [vesamode],ax               ; Store vesa 1.2 mode number
;        and eax,0FFFFh
;        push eax
;        mov ax,0003h
;        int 10h
;        pop eax
;        call printnum
;        jmp DosExit

    mov ax,[fs:10h]
    mov byte[vesa2red10],0
    mov byte[vesa2_rposng],11
    mov byte[vesa2_gposng],6
    mov byte[vesa2_bposng],0
    mov dword[vesa2_clbitng],1111011111011110b
    mov dword[vesa2_clbitng2],11110111110111101111011111011110b
    mov dword[vesa2_clbitng2+4],11110111110111101111011111011110b
    mov dword[vesa2_clbitng3],0111101111101111b
    mov [bytesperscanline],ax   ; Store bytes per scan line
    cmp byte[fs:20h],10
    jne .nored10
    mov byte[fs:20h],11
    mov byte[vesa2red10],1
    mov byte[vesa2_rposng],10
    mov byte[vesa2_gposng],5
    mov dword[vesa2_clbitng],0111101111011110b
    mov dword[vesa2_clbitng2],01111011110111100111101111011110b
    mov dword[vesa2_clbitng2+4],01111011110111100111101111011110b
    mov dword[vesa2_clbitng3],0011110111101111b
.nored10
    ; fix up bit lengths
    mov al,16
    sub al,[fs:20h]
    mov ah,[fs:22h]
    sub ah,[fs:20h]
    mov bl,[fs:24h]
    sub bl,[fs:20h]
    mov bh,al
    cmp bh,ah
    jb .scheck1
    mov bh,ah
.scheck1
    cmp bh,bl
    jb .scheck2
    mov bh,bl
.scheck2
    mov byte[fs:19h],5

    mov al,16
    sub al,[fs:22h]
    mov ah,[fs:20h]
    sub ah,[fs:22h]
    mov bl,[fs:24h]
    sub bl,[fs:22h]
    mov bh,al
    cmp bh,ah
    jb .scheck1b
    mov bh,ah
.scheck1b
    cmp bh,bl
    jb .scheck2b
    mov bh,bl
.scheck2b
    mov [fs:21h],bh

    mov al,16
    sub al,[fs:24h]
    mov ah,[fs:20h]
    sub ah,[fs:24h]
    mov bl,[fs:22h]
    sub bl,[fs:24h]
    mov bh,al
    cmp bh,ah
    jb .scheck1c
    mov bh,ah
.scheck1c
    cmp bh,bl
    jb .scheck2c
    mov bh,bl
.scheck2c
    mov [fs:23h],bh

    mov word[vesa2_clbit],0

    cmp byte[fs:20h],10
    jne .nottopbit
    mov word[vesa2_usbit],8000h
.nottopbit

    ; Process Red Stuff
    mov al,[fs:20h]         ; bit sizes = [fs:19h,21h,23h]
    mov cl,al
    mov bx,1
    shl bx,cl
    cmp byte[fs:19h],6
    jne .no6bit
    mov [vesa2_usbit],bx
    inc al
.no6bit
    or [vesa2_clbit],bx
    mov [vesa2_rpos],al
    dec al
    mov cl,al
    mov bx,001Fh
    cmp cl,0FFh
    je .shrr
    shl bx,cl
    jmp .shlr
.shrr
    shr bx,1
.shlr
    mov [vesa2_rfull],bx
    add al,5
    mov bx,1
    mov cl,al
    shl bx,cl
    mov [vesa2_rtrcl],bx
    xor bx,0FFFFh
    mov [vesa2_rtrcla],bx

    ; Process Green Stuff
    mov al,[fs:22h]
    mov cl,al
    mov bx,1
    shl bx,cl
    cmp byte[fs:21h],6
    jne .no6bitb
    mov [vesa2_usbit],bx
    inc al
.no6bitb
    or [vesa2_clbit],bx
    mov [vesa2_gpos],al
    dec al
    mov cl,al
    mov bx,001Fh
    cmp cl,0FFh
    je .shrg
    shl bx,cl
    jmp .shlg
.shrg
    shr bx,1
.shlg
    mov [vesa2_gfull],bx
    add al,5
    mov bx,1
    mov cl,al
    shl bx,cl
    mov [vesa2_gtrcl],bx
    xor bx,0FFFFh
    mov [vesa2_gtrcla],bx

    ; Process Blue Stuff
    mov al,[fs:24h]
    mov cl,al
    mov bx,1
    shl bx,cl
    cmp byte[fs:23h],6
    jne .no6bitc
    mov [vesa2_usbit],bx
    inc al
.no6bitc
    or [vesa2_clbit],bx
    mov [vesa2_bpos],al
    dec al
    mov cl,al
    mov bx,001Fh
    cmp cl,0FFh
    je .shrb
    shl bx,cl
    jmp .shlb
.shrb
    shr bx,1
.shlb
    mov [vesa2_bfull],bx
    add al,5
    mov bx,1
    mov cl,al
    shl bx,cl
    mov [vesa2_btrcl],bx
    xor bx,0FFFFh
    mov [vesa2_btrcla],bx

    xor word[vesa2_clbit],0FFFFh

    mov ax,[fs:4]
    mov [granularity],ax
    xor edx,edx
    mov ax,64
    mov bx,[granularity]
    div bx
    mov [granadd],ax

    call genfulladdtab

    xor ecx,ecx
    xor edx,edx
    mov eax,4f02h
    movzx ebx,word[vesamode]
    int 10h              ; Set the vesa mode
    cmp ax,004fh
    jz .modesetok
    mov edx,.unableset
    jmp VESA12EXITTODOS  ; Failure!!!

.modesetok
;******************************* EXTRA BIT ****************************

    ret

    ; Check logical scanline length
    mov eax,4f06h
    mov ebx,1
    int 10h
    cmp cx,[vesa2_x]
    je .correctwidth

    mov eax,4f06h           ; VBE Set/Get logical scan line length
    mov ebx,0               ; Set scan line length in pixels
    mov ecx, [vesa2_x]      ; Desired screen width
    int 10h
    cmp ax,04fh
    jz .correctwidth
    mov edx, .unablescan
    jmp VESA12EXITTODOS     ; Failure!!!

.correctwidth

    ret

SECTION .data
.nomemmessage db ': Unable to locate DOS memory.$'
.noint1message db ': Simulated real mode interrupt failed.$'
.oopsnoselector db ': Failed to allocate vesa display selector!$'
.novesa2message db ': VESA 1.2 or greater required!$'
.novbemessage db ': VBE not detected!!$'
.novesamessage db ': VESA not detected!$'
.outofmodelistspacemessage db ': Out of VESA mode list space!$'
.endoflist db ': VESA 1.2 mode does not work on your video card/driver.$'
.whichwin db 0
.modecheckfail db ': Real mode interrupt failure while checking vesa mode$'
.unableset db 'Unable to initialize video mode.$'
.unablescan db 'Unable to set scan line length.$'
SECTION .bss

RMREGS
.edi resd 1
.esi resd 1
.ebp resd 1
.esp resd 1
.ebx resd 1
.edx resd 1
.ecx resd 1
.eax resd 1
.flags   resw 1
.es   resw 1
.ds   resw 1
.fs   resw 1
.gs   resw 1
.ip   resw 1
.cs   resw 1
.sp   resw 1
.ss   resw 1
.spare   times 20 resd 1
