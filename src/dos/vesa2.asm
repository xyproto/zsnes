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

EXTSYM DosExit,PrintStr,HalfTransB,HalfTransC,Init_2xSaIMMX,ZSNESBase
EXTSYM UnusedBit,HalfTrans,UnusedBitXor,ngrposng,nggposng,ngbposng
EXTSYM videotroub,ExitFromGUI,ErrorPointer,vesa2_x,vesa2_y,vesa2_bits,TripBufAvail,vesa2red10
EXTSYM vesa2_rposng,vesa2_gposng,vesa2_bposng,vesa2_clbitng,vesa2_clbitng2,vesa2_clbitng3
EXTSYM vesa2_clbit,vesa2_usbit,vesa2_rpos,vesa2_rfull,vesa2_rtrcl,vesa2_rtrcla,genfulladdtab
EXTSYM vesa2_gpos,vesa2_gfull,vesa2_gtrcl,vesa2_gtrcla,vesa2_bpos,vesa2_bfull,vesa2_btrcl,vesa2_btrcla

SECTION .data
; add 0214h video mode
anticrash times 10 db 0

ALIGN32
NEWSYM vesa2selec,      dd 0            ; VESA2 Selector Location
NEWSYM vesa3en,         dd 0
NEWSYM VESAAddr,        dd 0

SECTION .bss

SECTION .text

NEWSYM VESA2EXITTODOS
    mov byte[videotroub],1
    cmp byte[ExitFromGUI],0
    je .nogui
    mov [ErrorPointer],edx
    ret
.nogui
    mov ax,0003h
    int 10h
    push edx
    mov edx,.exitfromvesa2
    call PrintStr
    pop edx
    call PrintStr
    mov edx,.return
    call PrintStr

    jmp DosExit

SECTION .data
.exitfromvesa2 db 'Unable to Initialize VESA2 : ',0
.return db 10,13,0
SECTION .text

;*******************************************************
;   Set up Vesa 2
;*******************************************************

NEWSYM InitVesa2
    ;-------------------------------------------------;
     ; First - allocate some bytes in DOS memory for ;
     ; communication with VBE                        ;
    ;-------------------------------------------------;

    mov eax,0100h
    mov ebx,512/16  ; 512 bytes
    int 31h         ; Function 31h,100h - Allocate
                    ; DOS memory (512 bytes)
    jnc .gotmem
    mov edx,.nomemmessage
    jmp VESA2EXITTODOS

.gotmem
    mov fs,dx   ; FS now points to the DOS buffer

    ;--------------------------------------------------;
     ; Now, get information about the video card into ;
     ; a data structure                               ;
    ;--------------------------------------------------;

    mov edi,RMREGS
    mov dword[fs:0],'VBE2'   ; Request VBE 2.0 info
    mov dword[RMREGS.eax],4f00h
    mov [RMREGS.es],ax       ; Real mode segment of DOS buffer
    mov dword[RMREGS.edi],0

    push es
    push ds
    pop es
    mov eax,300h
    mov ebx,10h
    xor ecx,ecx
    int 31h                  ; Simulate real mode interrupt
    pop es

    jnc .int1ok
    mov edx,.noint1message
    jmp VESA2EXITTODOS


.int1ok              ; Real mode int successful!!!
    mov eax,[RMREGS.eax]
    cmp al,4fh       ; Check vbe interrupt went OK
    jz .vbedetected
    mov edx,.novbemessage
    jmp VESA2EXITTODOS

.vbedetected
    cmp dword[fs:0000],'VESA'
    jz .vesadetected  ; Check for presence of vesa
    mov edx,.novesamessage
    jmp VESA2EXITTODOS


.vesadetected
    cmp word[fs:0004],200h
    jae .vesa2detected ; Check we've got VESA 2.0 or greater
    mov edx,.novesa2message
    jmp VESA2EXITTODOS


    ;-----------------------------------------------------;
     ; OK - vesa 2.0 or greater has been detected. Copy  ;
     ; mode information into VESAmodelist                ;
    ;-----------------------------------------------------;

.vesa2detected
    mov dword[vesa3en],0
    cmp word[fs:004],300h
    jb .notvbe3
    mov dword[vesa3en],1
.notvbe3
    mov ax,[fs:12h]             ; Get no. of 64k blocks
    mov [noblocks],ax
    mov ax, 2
    mov bx,[fs:10h]
    int 31h

    jnc .wegottheselector
    mov edx, .oopsnoselector
    jmp VESA2EXITTODOS

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
    jmp VESA2EXITTODOS

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
    jmp VESA2EXITTODOS

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
    int 31h                 ; Simulate real mode interrupt
    pop es
    jnc .modecheckok
    mov edx,.modecheckfail
    jmp VESA2EXITTODOS

.modecheckok
    add ebp,2

    test word[fs:0000h],1b
    jz near .loopcheckmodes     ; If mode is not available

    mov eax,[vesa2_x]
    cmp [fs:12h],ax             ; Check that the height matches
    jnz near .loopcheckmodes
    mov eax,[vesa2_y]
    cmp [fs:14h],ax             ; Check that the width matches
    jnz near .loopcheckmodes
    mov al,[vesa2_bits]
    cmp [fs:19h],al             ; Check bits/pixel for match
    jnz near .loopcheckmodes

    mov byte[TripBufAvail],1
    test word[fs:0000h],400h
    jz .notbuf
    mov byte[TripBufAvail],1
.notbuf
.notvesa3

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
    mov al,[fs:2]      ; Get window A attributes
    and al,0100b
    cmp al,0100b
    je .foundwin       ; Mode supported
    mov al,[fs:3]      ; Get window B attributes
    and al,0100b
    cmp al,0100b
    jne .foundwin      ; Mode not supported
    mov byte[.whichwin],1
.foundwin

    ; Success - a match has been found!!

    sub ebp,2
    mov ax,[ebp]
    mov [vesamode],ax  ; Store vesa 2 mode number

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
    jne near .nored10
    mov byte[fs:20h],11
    mov byte[vesa2red10],1
    mov byte[vesa2_rposng],10
    mov byte[vesa2_gposng],5
    mov dword[vesa2_clbitng],0111101111011110b
    mov dword[vesa2_clbitng2],01111011110111100111101111011110b
    mov dword[vesa2_clbitng2+4],01111011110111100111101111011110b
    mov dword[vesa2_clbitng3],0011110111101111b
    mov dword[UnusedBit],     10000000000000001000000000000000b
    mov dword[HalfTrans],     01111011110111100111101111011110b
    mov dword[UnusedBitXor],  01111111111111110111111111111111b
    mov dword[UnusedBit+4],   10000000000000001000000000000000b
    mov dword[HalfTrans+4],   01111011110111100111101111011110b
    mov dword[UnusedBitXor+4],01111111111111110111111111111111b
    mov dword[HalfTransB],    00000100001000010000010000100001b
    mov dword[HalfTransB+4],  00000100001000010000010000100001b
    mov dword[HalfTransC],    01111011110111100111101111011110b
    mov dword[HalfTransC+4],  01111011110111100111101111011110b
    mov dword[ngrposng],10
    mov dword[nggposng],5
    mov dword[ngbposng],0

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

;        mov ax,03h
;        int 10h
;        mov ax,[vesa2_rfull]
;        call printhex
;        jmp DosExit

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

    call genfulladdtab

    test word[fs:0h],10000000b  ; Check if linear available
    jnz .linearavailable
    mov edx,.nolframebuffer
    jmp VESA2EXITTODOS          ; None available

    ;---------------------------------------------;
     ; OK - now set the vesa 2 mode based on the ;
     ; information gleaned...                    ;
    ;---------------------------------------------;

    .linearavailable
    or word[vesamode],4000h  ; Convert mode to its LFB equivalent
    mov ebx,[fs:28h]         ; Read in physical base ptr

    mov cx,bx
    shr ebx,16
    mov si,[noblocks]
    xor edi,edi         ; Since noblocks = number of 64k blocks,
                        ; these lines leave si:di holding byte size
    mov eax,800h
    int 31h
    jnc .mappedphysicalarea
    mov edx,.unablemap
    jmp VESA2EXITTODOS  ; Failure!!!

.mappedphysicalarea
    shl ebx,16
    mov bx,cx
    mov [LFBpointer],ebx
    mov eax,ebx
    sub eax,[ZSNESBase]
    mov [VESAAddr],eax

    xor ecx,ecx
    xor edx,edx
    mov eax,4f02h
    movzx ebx,word[vesamode]
    int 10h             ; Set the vesa mode
    cmp ax,004fh
    jz .modesetok
    mov edx,.unableset
    jmp VESA2EXITTODOS  ; Failure!!!

.modesetok
;******************************* EXTRA BIT ****************************

;        cmp byte[.whichwin],1           ; Check if Write is at Window B
;        jne .nowinB
;
;        mov ax,4F05h
;        mov bx,1
;        mov dx,0
;        int 10h
;
;.nowinB

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
    jmp VESA2EXITTODOS      ; Failure!!!

.correctwidth

;*************************** END OF EXTRA BIT *************************

    xor eax,eax
    mov ecx,1
    int 31h             ; Allocate a descriptor

    mov bx,ax           ; Move our selector into bx

    mov ecx,[LFBpointer]
    mov dx,cx
    shr ecx,16
    mov eax,7
    int 31h             ; Set our selector to LFB
    jnc .selectornowset
    mov edx,.unablelfb
    jmp VESA2EXITTODOS  ; Failure!!!

.selectornowset

    movzx ecx,word[noblocks]
    shl ecx,16          ; Multiply by 65536
    dec ecx             ; Necessary!!!
    mov dx,cx
    shr ecx,16          ; CX:DX size of screen
    mov eax,8
    int 31h             ; Set size of selector
    jnc .ok
    mov edx,.unablesets
    jmp VESA2EXITTODOS  ; Failure!!!

.ok
    lar ecx,ebx
    shr ecx,8
    and cl,60h
    or cl,93h
    and ch,0c0h         ; Keep granularity bit
    mov ax,9
    int 31h             ; Set selector access rights
    jnc .accessrightsset
    mov edx,.unablesetar
    jmp VESA2EXITTODOS

.accessrightsset
    mov [vesa2selec],bx

    cmp byte[vesa2red10],1
    je .red10
    mov eax,565
    jmp .red11
.red10
    mov eax,555
.red11
    push eax
    call Init_2xSaIMMX
    pop eax
    ret


SECTION .data
.nomemmessage db 'Unable to locate DOS memory.',0
.noint1message db 'Simulated real mode interrupt failed.',0
.novbemessage db 'VBE not detected!!',0
.novesamessage db 'VESA not detected!',0
.novesa2message db 'VESA 2.0 or greater required!',0
.oopsnoselector db 'Failed to allocate vesa display selector!',0
.outofmodelistspacemessage db 'Out of VESA2 mode list space!',0
.endoflist db 'This VESA2 mode does not work on your video card / driver.',0
.whichwin db 0
.modecheckfail db 'Real mode interrupt failure while checking vesa mode',0
.nolframebuffer db 'Linear Frame Buffer not Detected.',0
.unablemap db 'Unable to map physical area.',0
.unableset db 'Unable to initialize video mode.',0
.unablescan db 'Unable to set scan line length.',0
.unablelfb db 'Unable to set selector to LFB.',0
.unablesets db 'Unable to set size of selector.',0
.unablesetar db 'Unable to set selector access rights.',0

NEWSYM LFBpointer
    dd 0
NEWSYM noblocks
    dw 0
NEWSYM bytesperscanline
    dw 0
NEWSYM vesamode
    dw 0
;----------------------------------------------------------------------
;NEWSYM VESAmodelist
;   times 512 dw 0
;----------------------------------------------------------------------


SECTION .bss

NEWSYM VESAmodelist, times 512 resw 1
NEWSYM RMREGS
.edi  resd 1
.esi  resd 1
.ebp  resd 1
.esp  resd 1
.ebx  resd 1
.edx  resd 1
.ecx  resd 1
.eax  resd 1

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
;----------------------------------------------------------------------
