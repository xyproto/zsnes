;Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;zsknight@zsnes.com
;_demo_@zsnes.com
;pagefault@zsnes.com
;n-a-c-h@users.sf.net
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
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

EXTSYM ComNum, ComIRQ, BaudRate
EXTSYM FossilUse
EXTSYM GUIinit18_2hz,GUIinit36_4hz
EXTSYM GUIMenuItem
EXTSYM delay

%ifdef __MSDOS__
EXTSYM dssel
%endif

SECTION .bss
NEWSYM UartType, resb 1

SECTION .data
NEWSYM cantinitmodem, db 1

SECTION .bss
NEWSYM ModemInited, resb 1

SECTION .data
ComPort dw 2E8h  ; 1=3F8,2=2F8,3=3E8,4=2E8
PortData dw 0,3F8h,2F8h,3E8h,2E8h

SECTION .bss
ComInt  resb 1

SECTION .data
BRateSel dw 000Ch,0008h,0006h,0004h,0003h,0002h,0001h

SECTION .bss
oldhandmodems resw 1
oldhandmodemo resd 1

SECTION .data
PICMaskPm   db 21h

SECTION .bss
PortNum       resw 1
CharStore     resb 1
SECTION .text

NEWSYM ModemGetChar
   cmp byte[UartType],2
   je near FossilGetChar
   xor dh,dh
   mov eax,[modemhead]
   cmp eax,[modemtail]
   je .nonewchar
   mov dh,1
   mov dl,[modembuffer+eax]
   inc dword[modemhead]
   and dword[modemhead],2047
.nonewchar
   ret

FossilGetChar:
   pushad
   mov ah,0Ch
   mov dx,[PortNum]
   int 14h
   cmp ax,0FFFFh
   je .nochar
   mov ah,02h
   mov dx,[PortNum]
   int 14h
   mov [CharStore],al
   popad
   mov dh,1
   mov dl,[CharStore]
   ret
.nochar
   popad
   xor dh,dh
   ret

NEWSYM ModemCheckRing
   cmp byte[UartType],2
   je near .fossil
   mov dx,[ComPort]
   add dx,6
   in al,dx
   shr al,6
   and al,01h
   ret
.fossil
   pushad
   mov ah,03h
   mov dx,[PortNum]
   int 14h
   test al,40h
   jnz .ring
   popad
   xor al,al
   ret
.ring
   popad
   mov al,1
   ret

NEWSYM ModemCheckDCD
   cmp byte[UartType],2
   je near .fossil
   mov dx,[ComPort]
   add dx,6
   in al,dx
   shr al,7
   and al,01h
   ret
.fossil
   pushad
   mov ah,03h
   mov dx,[PortNum]
   int 14h
   test al,80h
   jnz .dcd
   popad
   xor al,al
   ret
.dcd
   popad
   mov al,1
   ret

NEWSYM ModemSendChar
   cmp byte[UartType],2
   je near FossilSendChar
   push ecx
   push edx
   push ebx
   mov ecx,1000000
   mov bl,al
.loop
   mov dx,[ComPort]
   add dx,5
   in al,dx
   test al,00100000b
   jnz .transokay
   dec ecx
   jnz .loop
   xor al,al
   pop ebx
   pop edx
   pop ecx
   ret
.transokay
   mov al,bl
   mov dx,[ComPort]             ; Send the char through the modem
   out dx,al
   pop ebx
   pop edx
   pop ecx
   ret

FossilSendChar:
   pushad
   mov ah,01h
   mov dx,[PortNum]
   int 14h
   popad
   ret

NEWSYM InitModem
   mov byte[ModemInited],1
   cmp byte[FossilUse],0
   jne near InitFossil
   mov byte[cantinitmodem],0
   cli
   ; Get Port value
   xor eax,eax
   mov al,[ComNum]
   mov ax,[PortData+eax*2]
   mov [ComPort],ax

   mov dx,[ComPort]
   add dx,2
   xor al,al
   out dx,al

   ; Set IRQ PIC Mask Port
   mov byte[PICMaskPm],21h
   mov bl,[ComIRQ]
   cmp bl,7
   jbe .noupper
   add bl,60h
   add byte[PICMaskPm],80h
.noupper
   add bl,8
   mov [ComInt],bl

   ; Get IRQ handler
   mov ax,204h
   mov bl,[ComInt]
   int 31h
   mov [oldhandmodems],cx
   mov [oldhandmodemo],edx
   ; Set IRQ handler
   mov ax,205h
   mov bl,[ComInt]
   mov cx,cs
   mov edx,modemhandler
   int 31h

   mov dx,[ComPort]
   add dx,3
   mov al,00000011b
   out dx,al

   ; Set Normal Modem functioning, User2 bit, and DTR
   mov dx,[ComPort]
   add dx,4
   mov al,00001011b
   out dx,al

   ; Enable IRQ
   xor dh,dh
   mov dl,[PICMaskPm]           ; Output to IRQ PIC Mask Port
   mov cl,[ComIRQ]              ; Get proper bit
   and cl,07h
   mov al,01h
   shl al,cl
   not al                       ; Complement since clear bit = enable
   mov bl,al
   in al,dx                     ; input to preserve other bits
   and al,bl
   xor al,al
   out dx,al

   ; Enable interrupt to execute only on data available
   mov dx,[ComPort]
   inc dx
   mov al,00000001b
   out dx,al
   sti

   ; Write baudrate
   mov dx,[ComPort]
   add dx,3
   in al,dx
   or al,10000000b
   out dx,al
   mov eax,[BaudRate]
   mov ax,[BRateSel+eax*2]
   mov dx,[ComPort]
   inc dx
   push eax
   mov al,ah
   out dx,al
   pop eax
   dec dx
   out dx,al
   mov dx,[ComPort]
   add dx,3
   in al,dx
   and al,01111111b
   out dx,al

   ; Initialize 16550A UART chip
   mov dx,[ComPort]
   add dx,2
   mov al,0C7h
   out dx,al
   nop
   nop
   in al,dx
   mov byte[UartType],1
   test al,40h
   jnz .passed16550a
   xor al,al
   out dx,al
   mov byte[UartType],0
.passed16550a
   ret

InitFossil:
   xor edx,edx
   mov dl,[ComNum]
   dec dl
   mov [PortNum],dx
   mov byte[cantinitmodem],0
   mov byte[UartType],2
   mov ah,04h
   mov dx,[PortNum]
   int 14h
   cmp ax,1954h
   jne .notsuccess
   xor ah,ah
   mov al,00000011b     ; 19200 baud, 81N
   mov dx,[PortNum]
   int 14h
   ret
.notsuccess
   mov byte[cantinitmodem],1
   ret

modemhandler:
   push ds
   push eax
%ifdef __MSDOS__
   mov ax,[cs:dssel]
%endif
   mov ds,ax
   push edx
   mov dx,[ComPort]
   add dx,4
   in al,dx
   and al,11111101b
   out dx,al

.next
   mov dx,[ComPort]
   in al,dx
   mov edx,[modemtail]
   mov [modembuffer+edx],al
   inc dword[modemtail]
   and dword[modemtail],2047
   mov dx,[ComPort]
   add dx,2
   in al,dx
   test al,1
   jz .next

   mov al,20h
   out 20h,al

   cmp byte[ComIRQ],7
   jbe .nohighirq
   mov al,20h
   out 0A0h,al
.nohighirq

   mov dx,[ComPort]
   add dx,4
   in al,dx
   or al,00000010b
   out dx,al

   pop edx
   pop eax
   pop ds
   iretd

NEWSYM ModemClearBuffer
    mov dword[modemhead],0
    mov dword[modemtail],0
    ret

SECTION .bss
NEWSYM modembuffer, resb 2048
NEWSYM modemhead, resd 1
NEWSYM modemtail, resd 1
SECTION .text

NEWSYM DeInitModem
   cmp byte[ModemInited],1
   je .okaydeinit
   ret
.okaydeinit
   mov byte[ModemInited],0
   cmp byte[UartType],2
   je near DeInitFossil
   cli
   mov dx,[ComPort]
   add dx,2
   xor al,al
   out dx,al

   xor dh,dh
   mov dl,[PICMaskPm]
   mov cl,[ComIRQ]
   and cl,07h
   mov al,01h
   shl al,cl
   mov bl,al
   in al,dx
   or al,bl
   xor al,al
   out dx,al
   mov dx,[ComPort]
   inc dx
   mov al,00h
   out dx,al
   mov dx,[ComPort]
   add dx,4
   out dx,al

   mov cx,[oldhandmodems]
   mov edx,[oldhandmodemo]
   mov ax,205h
   mov bl,[ComInt]
   int 31h
   sti
   ret

DeInitFossil:
   mov byte[cantinitmodem],0
   jne .nodeinit
   mov ax,0600h
   mov dx,[PortNum]
   int 14h              ; Lower DTR
   mov ah,05h
   mov dx,[PortNum]
   int 14h
.nodeinit
   ret

NEWSYM DeInitModemC
   cmp byte[ModemInited],1
   je .okaydeinit
   ret
.okaydeinit
   cmp byte[UartType],2
   je near DeInitFossil
   cli
   mov al,00h
   mov dx,[ComPort]
   add dx,4
   out dx,al

   mov al,13
   mov dx,[ComPort]
   out dx,al

   mov ecx,16384
   call delay

   out dx,al

   mov al,00001001b
   mov dx,[ComPort]
   add dx,4
   out dx,al
   sti
   ret

