;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
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

EXTSYM ipx_init               ; To init ipx
EXTSYM ipx_initcode           ; return 0 if everything is ok (int)
EXTSYM ipx_packet             ; 80 bytes buffer to send
EXTSYM ipx_packet_size        ; size to send (max 80 bytes) (dword)
EXTSYM sendpacket             ; to send a packet
EXTSYM checkpacket            ; check if a packet is ready to receive
EXTSYM ipx_packet_ready       ; return 1 if there is a packet ready (byte)
EXTSYM read_packet            ; to read an incoming packet
EXTSYM ipx_read_packet        ; 80 bytes buffer of received packet
EXTSYM ipx_deinit             ; to deinit the ipx
EXTSYM IPXInfoStr,IPXInfoStrR
EXTSYM modembuffer, modemhead, modemtail












NEWSYM ipxinited, db 0

NEWSYM initipx
    mov dword[modemhead],0
    mov dword[modemtail],0
    xor ax,ax
    cmp byte[ipxinited],1
    je .notokay
    call ipx_init
    mov ax,[ipx_initcode]
    cmp ax,0
    jne .notokay
    mov byte[ipxinited],1
.notokay
    ret

NEWSYM deinitipx
    cmp byte[ipxinited],0
    je .notinitialized
    mov byte[ipxinited],0
    call ipx_deinit
.notinitialized
    ret

NEWSYM PacketPointer, dd 0

NEWSYM PreparePacketIPX
    cmp byte[ipxinited],1
    jne .noipx
    call ipxpp
.noipx
    ret

NEWSYM SendPacketIPX
    cmp byte[ipxinited],1
    jne .noipx
    call ipxsp
.noipx
    ret

NEWSYM ipxsendchar    ; prepare packet
    push esi
    mov esi,[PacketPointer]
    mov [esi],al
    inc dword[PacketPointer]
    pop esi
    ret

NEWSYM IPXSearchval, db 0
NEWSYM ipxlookforconnect
    cmp byte[ipxinited],0
    je .initialized
    ret
.initialized
    pushad
    call checkpacket
    cmp byte[ipx_packet_ready],1
    jne near .nopacket
    call read_packet
    cmp dword[ipx_read_packet],'ZZ|Z'
    jne .nopacketf
    cmp byte[ipx_read_packet+6],'L'
    jne .nopacketf
    mov ax,[ipx_read_packet+4]
    cmp ax,[IPXInfoStr]
    je .nopacketf
    mov [IPXInfoStrR],ax
    mov eax,ipx_packet
    mov dword[eax],'ZY|Z'
    mov bx,[IPXInfoStr]
    mov [eax+4],bx
    mov bx,[IPXInfoStrR]
    mov [eax+6],bx
    mov dword[ipx_packet_size],8
    call sendpacket
    mov byte[IPXSearchval],1
    jmp .skipall
.nopacketf
    cmp dword[ipx_read_packet],'ZY|Z'
    jne .nopacket
    mov bx,[IPXInfoStr]
    cmp [eax+6],bx
    jne .nopacket
    mov bx,[eax+4]
    cmp bx,[IPXInfoStr]
    je .nopacket
    mov [IPXInfoStrR],bx
    mov byte[IPXSearchval],1
    jmp .skipall
.nopacket
    mov eax,ipx_packet
    mov dword[eax],'ZZ|Z'
    mov bx,[IPXInfoStr]
    mov [eax+4],bx
    mov byte[eax+6],'L'
    mov dword[ipx_packet_size],7
    call sendpacket
.skipall
    popad
    ret

NEWSYM ipxpp    ; prepare packet
    pushad
    mov eax,ipx_packet
    mov byte[eax],'Z'
    mov byte[eax+1],'|'
    mov byte[eax+2],'S'
    mov bx,[IPXInfoStr]
    mov [eax+3],bx
    add eax,6
    mov [PacketPointer],eax
    popad
    ret

NEWSYM ipxsp    ; send packet
    pushad
    mov eax,[PacketPointer]
    sub eax,ipx_packet
    mov [ipx_packet+5],al
    mov [ipx_packet_size],eax
    call sendpacket
    popad
    ret

NEWSYM ipxgetchar
    pushad
    call checkpacket
    cmp byte[ipx_packet_ready],1
    jne .nopacket
    call read_packet
    cmp byte[ipx_read_packet],'Z'
    jne .nopacket
    cmp byte[ipx_read_packet+1],'|'
    jne .nopacket
    cmp byte[ipx_read_packet+2],'S'
    jne .nopacket
    mov bx,[IPXInfoStrR]
    cmp [ipx_read_packet+3],bx
    jne .nopacket
    mov cl,[ipx_read_packet+5]
    sub cl,6
    mov esi,ipx_read_packet+6
    cmp cl,0
    je .nopacket
.loop
    mov edi,[modemtail]
    mov al,[esi]
    mov [modembuffer+edi],al
    inc dword[modemtail]
    inc esi
    and dword[modemtail],2047
    dec cl
    jnz .loop
.nopacket
    popad
    push eax
    xor dh,dh
    mov eax,[modemhead]
    cmp eax,[modemtail]
    je .nonewchar
    mov dh,1
    mov dl,[modembuffer+eax]
    inc dword[modemhead]
    and dword[modemhead],2047
.nonewchar
    pop eax
    ret

