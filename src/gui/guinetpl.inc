;Copyright (C) 1997-2004 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
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


; GUI NetPlay Routines (Modem,IPX,UDP+TCP/IP)

ProcessModem:
%ifndef __MSDOS__
    cmp byte[ModemProcess],40
    jne near .noprocess40
    cmp byte[maxskip],3
    jae .okaymf
    mov byte[maxskip],3
.okaymf
    mov byte[frameskip],0
    mov byte[Latencytochange],3
    mov byte[Latency],3
;    mov byte[IPXSearchval],0
    mov byte[RemoteDisconnect],0
    mov byte[HoldCommand],1
    mov byte[RemoteCommand],1
    ; Clear Screen Buffer
    mov eax,ViewBuffer
    mov ecx,50*32
.clooptcpip
    mov byte[eax],0
    inc eax
    dec ecx
    jnz .clooptcpip
    mov dword[CCursLoc],0
    mov dword[CViewLoc],0
    mov dword[NViewLoc],0
    mov esi,.tcpipinit
    call .writestr
    mov esi,.tcpipport
    call .writestr
    mov esi,GUINetTextl2
    call .writestr
    mov esi,.tcpipret
    call .writestr
    mov esi,.tcpipportt
    cmp byte[UDPConfig],0
    je .notudp
    mov esi,.tcpipportu
.notudp
    call .writestr
    mov esi,.tcpipret
    call .writestr
    mov esi,.tcpipserv
    call .writestr
    mov byte[ModemProcess],42
    ret
SECTION .data
.tcpipdones db 'DONE',13,10,13,10,'WAITING FOR CLIENT...',0
.tcpipdonec db 'DONE',13,10,13,10,'CONNECTING TO SERVER...',0
.tcpipdones2 db 13,10,13,10,'FOUND CLIENT.',0
.tcpipdonec2 db 13,10,13,10,'CONNECTION SUCCESSFUL.',0
.tcpiperror db 'FAILED',13,10,13,10,'ERROR: ',0
.tcpipip db 'IP : ',0
.tcpipip2 db 'SIP     : ',0
.tcpipport db 'PORT    : ',0
.tcpipportu db 'PROTOCOL: UDP',13,10,0
.tcpipportt db 'PROTOCOL: TCP/IP',13,10,0
.tcpipret db 13,10,0
.errorinit db 'FAILED NET INIT',0
.errorinit2 db 'CANNOT START SERVER',0
.errorinit3 db 'CANNOT CONNECT',0
SECTION .text

.noprocess40
    cmp byte[ModemProcess],41
    jne near .noprocess41
    cmp byte[maxskip],3
    jae .okaymf2
    mov byte[maxskip],3
.okaymf2
    mov byte[frameskip],0
    mov byte[Latencytochange],3
    mov byte[Latency],3
;    mov byte[IPXSearchval],0
    mov byte[RemoteDisconnect],0
    mov byte[HoldCommand],1
    mov byte[RemoteCommand],1
    ; Clear Screen Buffer
    mov eax,ViewBuffer
    mov ecx,50*32
.clooptcpip2
    mov byte[eax],0
    inc eax
    dec ecx
    jnz .clooptcpip2
    mov dword[CCursLoc],0
    mov dword[CViewLoc],0
    mov dword[NViewLoc],0
    mov esi,.tcpipinit
    call .writestr
    mov esi,.tcpipport
    call .writestr
    mov esi,GUINetTextl2
    call .writestr
    mov esi,.tcpipret
    call .writestr
    mov esi,.tcpipportt
    cmp byte[UDPConfig],0
    je .notudp2
    mov esi,.tcpipportu
.notudp2
    call .writestr
    mov esi,.tcpipip2
    call .writestr
    mov esi,TCPIPAddress
    call .writestr
    mov esi,.tcpipret
    call .writestr
    mov esi,.tcpipret
    call .writestr
    mov esi,.tcpipclie
    call .writestr
    mov byte[ModemProcess],43
    ret
.noprocess41
    cmp byte[ModemProcess],42
    jne near .noprocess42
    call TCPIPStartServer
    or eax,eax
    jnz .tcpfailed
    mov esi,.tcpipdones
    call .writestr
    mov byte[ModemProcess],44
    ret
.tcpfailed
    mov esi,.tcpiperror
    call .writestr
    mov esi,.errorinit
    call .writestr
    mov byte[ModemProcess],255
    ret
.noprocess42
    cmp byte[ModemProcess],43
    jne near .noprocess43
    call TCPIPInitConnectToServer
    or eax,eax
    jnz .tcpfailed2
    mov esi,.tcpipdonec
    call .writestr
    mov byte[ModemProcess],45
    ret
.tcpfailed2
    mov esi,.tcpiperror
    call .writestr
    mov esi,.errorinit
    call .writestr
    mov byte[ModemProcess],255
    ret
.noprocess43
    cmp byte[ModemProcess],44
    jne near .noprocess44
    call TCPIPWaitForConnection
    cmp eax,1
    je .successb
    or eax,eax
    jnz .tcpfailed3
    ret
.successb
    mov esi,.tcpipdones2
    call .writestr
    mov byte[ModemProcess],46
    ret
.tcpfailed3
;    add al,'Z'
;    mov byte[.tcpiperror],al
    mov esi,.tcpiperror
    call .writestr
    mov esi,.errorinit2
    call .writestr
    mov byte[ModemProcess],255
    ret
.noprocess44
    cmp byte[ModemProcess],45
    jne near .noprocess45
    call TCPIPConnectToServer
    or eax,eax
    jnz .tcpfailed4
    mov byte[ModemProcess],47
    ret
.tcpfailed4
;    add al,'Z'
;    mov [.errorinit3],al
    mov esi,.tcpiperror
    call .writestr
    mov esi,.errorinit3
    call .writestr
    mov byte[ModemProcess],255
    ret
.noprocess45
    cmp byte[ModemProcess],46
    jne near .noprocess46
    mov byte[pl2neten],2
    mov byte[pl1neten],1
    mov byte[UseRemoteSRAMData],1
    mov byte[ModemProcess],66
    ret
.noprocess46
    cmp byte[ModemProcess],47
    jne near .noprocess47
    call TCPIPConnectToServerW
    or eax,eax
    jnz .connected
    ret
.connected
    mov esi,.tcpipdonec2
    call .writestr
    mov byte[pl2neten],1
    mov byte[pl1neten],2
    mov byte[UseRemoteSRAMData],2
    mov byte[ModemProcess],66
    ret
.noprocess47
%endif
%ifdef __MSDOS__
    cmp byte[ModemProcess],30
    jne near .noprocess30
    mov byte[Latencytochange],2
    mov byte[Latency],2
    mov byte[IPXSearchval],0
    ; Clear Screen Buffer
    mov eax,ViewBuffer
    mov ecx,50*32
.cloopipx
    mov byte[eax],0
    inc eax
    dec ecx
    jnz .cloopipx
    mov dword[CCursLoc],0
    mov dword[CViewLoc],0
    mov dword[NViewLoc],0
    mov esi,.initc
    call .writestr
    mov esi,.localid
    call .writestr
    push es
    mov es,[selc0040]
    mov ax,[es:108]
    mov [IPXInfoStr],ax
    pop es
    call .writehex
    ; ipx
;    cmp byte[OSPort],1
;    je .noinitipx
    call initipx
.noinitipx
    cmp ax,0
    je .success
    mov byte[ModemProcess],255
    mov esi,.noipx
    call .writestr
    ret
.success
    mov byte[ModemProcess],31
    mov dword[ModemPTimer],36
    mov esi,.yesipx
    call .writestr
    ret
.noprocess30
    cmp byte[ModemProcess],31
    jne .noprocess31
    ; Wait for remote
    cmp dword[ModemPTimer],0
    jne .nolookforconnect
    mov dword[ModemPTimer],36
;    cmp byte[OSPort],1
;    je .noipxlook
    call ipxlookforconnect
.noipxlook
    cmp byte[IPXSearchval],0
    je .nolookforconnect
    mov esi,.connectest
    call .writestr
    mov esi,.remoteid
    call .writestr
    mov ax,[IPXInfoStrR]
    call .writehex
    mov byte[ModemProcess],64
    mov byte[Connected],1
    mov byte[WhichRemote],2
.nolookforconnect
    ret
.noprocess31
    cmp byte[ModemProcess],0
    jne near .noprocess0
    mov byte[Latencytochange],2
    mov byte[Latency],2
    cmp byte[ModemInitStat],1
    je .nomodeminit
    call InitModem
    mov byte[ModemInitStat],1
.nomodeminit
    mov byte[Connected],0
    ; Clear Screen Buffer
    mov eax,ViewBuffer
    mov ecx,50*32
.cloop
    mov byte[eax],0
    inc eax
    dec ecx
    jnz .cloop
    mov dword[CCursLoc],0
    mov dword[CViewLoc],0
    mov dword[NViewLoc],0
    mov esi,.init
    call .writestr
    mov esi,.uart0
    cmp byte[UartType],0
    je .nouart0
    mov esi,.uart1
    cmp byte[UartType],1
    je .nouart0
    mov esi,.uart2
.nouart0
    call .writestr
    mov esi,.initb
    call .writestr
    mov byte[ModemProcess],8
    mov dword[ModemPTimer],36
    mov byte[InitStr1Done],0
    mov byte[ModemOKStat],0
    cmp byte[cantinitmodem],0
    je .okayinit
    mov esi,.unableinit
    call .writestr
    mov byte[ModemProcess],255
.okayinit
    ret
.noprocess0
    cmp byte[ModemProcess],8
    jne .noprocess8
    cmp dword[ModemPTimer],0
    jne .skipnext8
    mov dword[ModemPTimer],18
    mov byte[ModemProcess],1
    cmp byte[CNetType],12
    jne .nomanual
    mov byte[ModemProcess],7
    mov dword[ModemPTimer],0
.nomanual
    mov al,13
    call ModemSendChar
    mov al,13
    call ModemSendChar
.skipnext8
    ret
.noprocess8
    cmp byte[ModemProcess],1
    jne .noprocess1
    cmp dword[ModemPTimer],0
    jne .skipnext
    mov byte[ModemProcess],2
    call ModemClearBuffer
.skipnext
    ret
.noprocess1
    cmp byte[ModemProcess],2
    jne .noprocess2
    mov byte[ModemProcess],3
    mov dword[ModemPTimer],36*5
    mov esi,GUIInitSt1
    cmp byte[InitStr1Done],0
    je .notinit2
    mov esi,GUIInitSt2
.notinit2
    call ModemSendStr
    mov al,13
    call ModemSendChar
    ret
.noprocess2
    cmp byte[ModemProcess],3
    jne .noprocess3
    cmp byte[ModemOKStat],0
    je .norecv
    mov byte[ModemProcess],9
.norecv
    cmp dword[ModemPTimer],0
    jne .notimeout
    mov esi,.tout
    call .writestr
    mov byte[ModemProcess],255
.notimeout
    ret
.noprocess3
    cmp byte[ModemProcess],9
    jne .noprocess9
    cmp dword[ModemPTimer],0
    je .timeout
    cmp byte[ModemOKStat],4
    jne .notcomplete
.timeout
    mov byte[ModemProcess],4
.notcomplete
    ret
.noprocess9
    cmp byte[ModemProcess],4
    jne .noprocess4
    inc byte[InitStr1Done]
    cmp byte[InitStr1Done],2
    je .noinit2
    cmp byte[GUIInitSt2],0
    je .noinit2
    mov byte[ModemProcess],1
    mov dword[ModemPTimer],18
    mov byte[ModemOKStat],0
    ret
.noinit2
    mov byte[ModemProcess],5
    mov dword[ModemPTimer],18
    ret
.noprocess4
    cmp byte[ModemProcess],5
    jne .noprocess5
    cmp dword[ModemPTimer],0
    jne .finishtimer

    ; ***** - Comment the following 4 lines
;    mov byte[ModemProcess],64
;    mov byte[Connected],1
;    mov byte[WhichRemote],1
;    ret

    ; dial, wait for ring, etc.
    cmp byte[CNetType],11       ; dial
    jne .nodial
    mov esi,GUIDialSt
    call ModemSendStr
    mov esi,DialNumber
    call ModemSendStr
    mov al,13
    call ModemSendChar
    mov byte[ModemProcess],7
    ret
.nodial
    cmp byte[CNetType],10       ; wait for ring
    jne .noanswer
    mov esi,.rdet
    call .writestr
    mov byte[ModemProcess],6
    ret
.noanswer
    mov byte[ModemProcess],7
    ret
.finishtimer
.noprocess5
    cmp byte[ModemProcess],6
    jne .noprocess6
    call ModemCheckRing
    cmp al,1
    jne .noprocess6
    mov esi,.ata
    call ModemSendStr
    mov byte[ModemProcess],7
.noprocess6
    cmp byte[ModemProcess],7
    jne .noprocess7
    call ModemCheckDCD
    cmp al,1
    jne .nocdc7
    mov esi,.cdet
    call .writestr
    mov byte[ModemProcess],64
    mov byte[WhichRemote],1
.nocdc7
    ret
.noprocess7
%endif

    ; Connect initialization
    cmp byte[ModemProcess],64
    jne .noprocess64
    mov dword[ModemPTimer],72  ;36
    mov byte[ModemProcess],65
    mov byte[Connected],1
    ret
.noprocess64
    cmp byte[ModemProcess],65
    jne .noprocess65
    cmp dword[ModemPTimer],0
    je .okay65
    ret
.okay65
    mov byte[ModemProcess],66
    ret
.noprocess65
    cmp byte[ModemProcess],69
    jne .noprocess69
    cmp dword[ModemPTimer],0
    jne .notimer69
    mov dword[ModemPTimer],90   ; 2.5s
    call PreparePacket
    mov al,228
    call RemoteSendChar
    call SendPacket
.notimer69
    call RemoteGetChar
    cmp dh,0
    je .doneprocess69
    cmp dl,228
    jne .doneprocess69
    mov byte[ModemProcess],66
.doneprocess69
    ret
.noprocess69
    cmp byte[ModemProcess],66
    jne .noprocess66
    ; Send ID and version #
    call PreparePacket
    mov al,'I'
    call RemoteSendChar
    mov al,'D'
    call RemoteSendChar
    mov al,222
    call RemoteSendChar
    mov al,[welcome+7]
    call RemoteSendChar
    mov al,[welcome+9]
    call RemoteSendChar
    mov al,[welcome+10]
    call RemoteSendChar
    mov al,[welcome+11]
    call RemoteSendChar
    mov al,[soundon]
    call RemoteSendChar
    cmp al,byte[CNetType]
    call RemoteSendChar
    call SendPacket
    mov byte[ModemProcess],67
    mov byte[IDCheckPos],0
    ret
    ; Check if versions are the same
.noprocess66
    cmp byte[ModemProcess],67
    jne near .noprocess67
    call RemoteGetChar
    cmp dh,0
    je near .doneprocess
    cmp dl,'I'
    jne .noI
    mov byte[IDCheckPos],1
    ret
.noI
    cmp byte[IDCheckPos],1
    jne .nocheck2
    cmp dl,'D'
    jne .noD
    mov byte[IDCheckPos],2
    ret
.noD
    mov byte[IDCheckPos],0
    ret
.nocheck2
    cmp byte[IDCheckPos],2
    jne .nocheck3
    cmp dl,222
    jne .no222
    mov byte[IDCheckPos],3
    ret
.no222
    mov byte[IDCheckPos],0
    ret
.nocheck3
    cmp byte[IDCheckPos],3
    jne .nocheck4
    cmp dl,[welcome+7]
    jne near .invversion
    mov byte[IDCheckPos],4
    ret
.nocheck4
    cmp byte[IDCheckPos],4
    jne .nocheck5
    cmp dl,[welcome+9]
    jne near .invversion
    mov byte[IDCheckPos],5
    ret
.nocheck5
    cmp byte[IDCheckPos],5
    jne .nocheck6
    cmp dl,[welcome+10]
    jne .invversion
    mov byte[IDCheckPos],6
    ret
.nocheck6
    cmp byte[IDCheckPos],6
    jne .nocheck7
    cmp dl,[welcome+11]
    jne .invversion
    mov byte[IDCheckPos],9
    ret
.nocheck7
    cmp byte[IDCheckPos],9
    jne .nocheck10
    cmp dl,[soundon]
    jne .invsound
    mov byte[IDCheckPos],7
    ret
.nocheck10
    cmp byte[IDCheckPos],7
    jne .nocheck8
    mov [RemCNetType],dl
    mov byte[IDCheckPos],0
    mov byte[ModemProcess],80
    cmp byte[WhichRemote],4
    jne .nocheck8
    mov byte[ModemProcess],81
.nocheck8
    ret
.invversion
    mov esi,.invver
    call .writestr
    mov byte[IDCheckPos],8
    ret
.invsound
    mov esi,.invsnd
    call .writestr
    mov byte[IDCheckPos],8
    ret
.doneprocess
    ret
.noprocess67
    cmp byte[ModemProcess],81
    je near .nonetpl
    cmp byte[ModemProcess],80
    jne near .noprocess80
    mov dword[pl1neten],0
    mov byte[pl5neten],0
    mov byte[UseRemoteSRAMData],0
    cmp byte[CNetType],10
    jne .noans
    mov byte[pl2neten],2
    mov byte[pl1neten],1
.noans
    cmp byte[RemCNetType],10
    jne .noans2
    mov byte[pl2neten],1
    mov byte[pl1neten],2
.noans2
    cmp byte[CNetType],11
    jne .noans3
    mov byte[pl2neten],1
    mov byte[pl1neten],2
.noans3
    cmp byte[RemCNetType],11
    jne .noans4
    mov byte[pl2neten],2
    mov byte[pl1neten],1
.noans4
    cmp byte[CNetType],15
    jne near .nonetpl
    mov bx,[IPXInfoStr]
    mov ax,[IPXInfoStrR]
.nextpldet
    push eax
    push ebx
    xor ax,bx
    test ax,1
    jz .nochipx
    test bx,1
    jz .pl2
    mov byte[pl2neten],2
    mov byte[pl1neten],1
    jmp .skipipxpl
.pl2
    mov byte[pl2neten],1
    mov byte[pl1neten],2
    jmp .skipipxpl
.nochipx
    pop ebx
    pop eax
    shr eax,1
    shr ebx,1
    jmp .nextpldet
.skipipxpl
    pop ebx
    pop eax
    mov byte[UseRemoteSRAMData],0
.nonetpl
    mov eax,ViewBuffer
    mov ecx,50*32
.cloop2
    mov byte[eax],0
    inc eax
    dec ecx
    jnz .cloop2
    mov dword[CCursLoc],0
    mov dword[CViewLoc],0
    mov dword[NViewLoc],0
    mov byte[CNetType],20
    mov dword[ChatString],0
    mov byte[RemoteCommand],1
    mov byte[romloadskip],1
    call LoadDetermine
    call PreparePacket
    mov al,1
    call RemoteSendChar
    call SendPacket
.noprocess80
    ret
.writestr
    mov dh,1
.loop
    mov dl,[esi]
    or dl,dl
    jz .nomore
    call NetAddChar
    inc esi
    jmp .loop
.nomore
    ret
.writehex
    mov dx,ax
    shr dx,12
    and edx,0Fh
    mov dl,[.hexval+edx]
    push eax
    call NetAddChar
    pop eax
    mov dx,ax
    shr dx,8
    and edx,0Fh
    mov dl,[.hexval+edx]
    push eax
    call NetAddChar
    pop eax
    mov dx,ax
    shr dx,4
    and edx,0Fh
    mov dl,[.hexval+edx]
    push eax
    call NetAddChar
    pop eax
    mov dx,ax
    and edx,0Fh
    mov dl,[.hexval+edx]
    call NetAddChar
    ret
SECTION .data
.hexval db '0123456789ABCDEF'
%ifdef __MSDOS__
.unableinit db 'CANNOT INIT DRIVER!',13,10,0
.init db 'ZSNES MODEM TERMINAL',13,10,0
.initb db 13,10,'INITIALIZING MODEM :',13,10,13,10,0
.initc db 'ZSNES IPX TERMINAL',13,10,13,10,0
.uart0 db 'USING 8250 UART',13,10,0
.uart1 db 'USING 16550A UART',13,10,0
.uart2 db 'USING FOSSIL DRIVER',13,10,0
.tout db 'MODEM RESPONSE TIMEOUT',13,10,0
.ata db 'ATA',13,0
.cdet db 'CARRIER DETECTED',13,10,0
.rdet db 'WAITING FOR RING...',13,10,0
.noipx db 13,10,13,10,'UNABLE TO INIT IPX!',13,10,0
.yesipx db 13,10,13,10,'IPX INITIALIZED.',13,10,13,10,'WAITING FOR REMOTE',13,10,0
.localid db 'LOCAL ID  : ',0
.remoteid db 'REMOTE ID : ',0
.connectest db 'CONNECTION ESTABLISHED',13,10,0
%endif
.invver db 13,10,'INCOMPATIBLE VERSION!',13,10,0
.invsnd db 'INVALID SOUND SETTING!',13,10,0
%ifndef __MSDOS__
.tcpipinit db 'ZSNES NET INTERFACE',13,10,13,10,0
.tcpipserv db 'INIT SERVER ... ',0
.tcpipclie db 'INIT CLIENT ... ',0
%endif
SECTION .bss
InitStr1Done resb 1
SECTION .text

NEWSYM PreparePacket
%ifdef __MSDOS__
    call PreparePacketIPX
%endif
    cmp byte[WhichRemote],4
    jne .nottcpip
    call TCPIPPreparePacket
.nottcpip
    ret

NEWSYM SendPacket
%ifdef __MSDOS__
    call SendPacketIPX
%endif
    cmp byte[WhichRemote],4
    jne .nottcpip
    call TCPIPSendPacket
.nottcpip
    ret

NEWSYM SendPacketUDP
%ifdef __MSDOS__
    call SendPacketIPX
%endif
    cmp byte[WhichRemote],4
    jne .nottcpip
    call TCPIPSendPacketUDP
.nottcpip
    ret

NEWSYM WritetochatBuffer
    ; 28 characters
    mov dh,1
.loop
    mov dl,[esi]
    or dl,dl
    jz .nomore
    call NetAddChar
    inc esi
    jmp .loop
.nomore
    ret

SECTION .bss
NEWSYM RemoteCommand, resb 1
HoldCommand resb 1
SECTION .text

RemoteDeInit:
%ifdef __MSDOS__
    cmp byte[ModemInitStat],0
    je .nodeinitmodem
    call DeInitModemC
    mov byte[ModemInitStat],0
.nodeinitmodem
%endif
    mov byte[CNetType],1
    cmp byte[WhichRemote],2
    jne .noipx
    mov byte[CNetType],0
.noipx
    cmp byte[WhichRemote],4
    jne .notcpip
    cmp byte[TCPIPStatus],0
    je .nottcpipinit
    call TCPIPDisconnect
    mov byte[TCPIPStatus],0
.nottcpipinit
    mov byte[CNetType],4
.notcpip
    ret

Checkfileexist:
    mov edx,FileNameMod
    call Open_File
    jc .failed
    mov bx,ax
    mov ecx,1
    mov edx,.temp
    call Read_File
    jc .failed2
    call Close_File
    jc .failed
    xor eax,eax
    ret
.failed2
    call Close_File
.failed
    mov eax,1
    ret
SECTION .bss
.temp resb 1
SECTION .text

ProcessRemoteCommand:
%ifdef __MSDOS__
    cmp byte[WhichRemote],1
    jne .notmodem
    push edx
    call ModemCheckDCD
    pop edx
    cmp al,1
    jne near .nomulti
.notmodem
%endif
    cmp byte[RemoteDisconnect],1
    je near .nomulti
    call ProcRewind
    ; 1 = Idle, 2 = Chat String, 3..7 = Player Select
    ; 10 = Load Game, 11 = Proceed Load, 12 = Load failed, 13 = File not Found
    ; 20 = Send Controls
    cmp dl,1
    je near .finish
    cmp dl,2
    je near .chatstr
    cmp dl,10
    je near .loadstr
    cmp dl,11
    je near .startloading
    cmp dl,12
    je near .loadfail  ; ***************************
    cmp dl,13
    je near .loadfnf
    cmp dl,14
    je near .loadstate
    cmp dl,15
    je near .skipstate
    cmp dl,20
    je near .latencychange
    cmp dl,30
    je near .quitgui
    cmp dl,40
    je near .guireset
    cmp dl,8
    ja near .nomulti
    cmp [HoldCommand],dl
    je near .forgetcommand
    cmp dl,3
    jne .nopl1
    cmp byte[pl1neten],1
    je .nopl1
    xor byte[pl1neten],2
.nopl1
    cmp dl,4
    jne .nopl2
    cmp byte[pl2neten],1
    je .nopl2
    xor byte[pl2neten],2
.nopl2
    cmp dl,5
    jne .nopl3
    cmp byte[pl3neten],1
    je .nopl3
    xor byte[pl3neten],2
.nopl3
    cmp dl,6
    jne .nopl4
    cmp byte[pl4neten],1
    je .nopl4
    xor byte[pl4neten],2
.nopl4
    cmp dl,7
    jne .nopl5
    cmp byte[pl5neten],1
    je .nopl5
    xor byte[pl5neten],2
.nopl5
    cmp dl,8
    jne .nobackstate
    xor byte[BackState],1
    cmp byte[BackState],0
    jne .notbstate0
    mov byte[Latency],10
    mov byte[Latencytochange],10
.notbstate0
    cmp byte[BackState],1
    jne .notbstate1
    mov byte[Latency],3
    mov byte[Latencytochange],3
.notbstate1
.nobackstate
    jmp .finish
.forgetcommand
    mov byte[HoldCommand],1
    jmp .finish
.nomulti
    cmp dl,50
    jne .nosramnone
    cmp byte[HoldCommand],41
    je .forgetcommand
    cmp byte[HoldCommand],42
    je .forgetcommand
    mov byte[UseRemoteSRAMData],0
    jmp .finish
.nosramnone
    cmp dl,41
    jne .nosramlocal
    cmp byte[HoldCommand],40
    je .forgetcommand
    cmp byte[HoldCommand],41
    je .forgetcommand
    mov byte[UseRemoteSRAMData],2
    jmp .finish
.nosramlocal
    cmp dl,42
    jne .nosramremote
    cmp byte[HoldCommand],40
    je .forgetcommand
    cmp byte[HoldCommand],42
    je .forgetcommand
    mov byte[UseRemoteSRAMData],1
    jmp .finish
.nosramremote
    cmp byte[NetQuitAfter],0
    je .noguiquit
    mov byte[GUIQuit],1
.noguiquit
    ; error
    call RemoteDeInit
    ret
.loadfail
    cmp dword[TCPIPAddress],'127.'
    jne .nonum
    cmp dword[TCPIPAddress+4],'0.0.'
    jne .nonum
    cmp byte[TCPIPAddress+8],'1'
    je near .startloadingb
.nonum
    mov esi,.filenotu
    call ProcessModem.writestr
    jmp .finish
.startloadingb
    mov byte[UseRemoteSRAMData],0
    jmp .startloading
.loadfnf
    mov esi,.filenotfr
    call ProcessModem.writestr
    jmp .finish
.loadstate
    mov byte[CNetType],22
    call loadstaterecvinit
.skipstate
    jmp .finish
.latencychange
    cmp [HoldCommand],dl
    je near .invlatencych
.keepgoinglc
    call RemoteGetChar
    cmp dh,0
    je .keepgoinglc
    mov [Latency],dl
    mov [Latencytochange],dl
    jmp .finish
.quitgui
    mov byte[GUIQuit],2
    jmp .finsend
.guireset
    call GUIDoReset
    jmp .finsend
.invlatencych
    mov al,[Latency]
    mov [Latencytochange],al
    jmp .finsend
.loadstr
    push edx
    mov esi,.loading
    call ProcessModem.writestr
    xor ebx,ebx
.keepgoing2
    push ebx
    call RemoteGetChar
    pop ebx
    cmp dh,0
    je .keepgoing2
    mov [FileNameMod+ebx],dl
    inc ebx
    or dl,dl
    je .doneload
    jmp .keepgoing2
.doneload
    mov esi,FileNameMod
    call ProcessModem.writestr
    mov dl,13
    call NetAddChar
    mov dl,10
    call NetAddChar
    pop edx
    cmp byte[HoldCommand],11
    je .loadfailed
    cmp [HoldCommand],dl
    jne .okayload
    ; ***** - Add comment
;    jmp .okayload               ; ************
.loadfailed
    ; load failed
    mov esi,.filenotu
    call ProcessModem.writestr
    mov byte[RemoteCommand],12
    jmp .finish
.loadfailednf
    ; File Not Found
    mov esi,.filenotf
    call ProcessModem.writestr
    mov byte[RemoteCommand],13
    jmp .finish
.okayload
    call Checkfileexist
    cmp eax,0
    jne .loadfailednf
    mov byte[RemoteCommand],11
.trynext
    cmp byte[GUIwinptr],0
    je near .noclose
    xor eax,eax
    mov al,[GUIwinptr]
    dec eax
    xor ebx,ebx
    mov bl,[GUIwinorder+eax]
    ; close window
    mov byte[GUIwinorder+eax],0
    mov byte[GUIwinactiv+ebx],0
    dec byte[GUIwinptr]
    cmp byte[GUIwinptr],0
    jne .noclose
    mov al,[GUIpmenupos]
    mov [GUIcmenupos],al
    jmp .trynext
.noclose
    jmp .finish
.chatstr
    ; Keep receiving and printing characters until 0 is found
;    mov dl,'R'
;    call NetAddChar
;    mov dl,'>'
;    call NetAddChar
.keepgoing
    call RemoteGetChar
    cmp dh,0
    je .keepgoing
    or dl,dl
    je .finishst
    call NetAddChar
    jmp .keepgoing
.finishst
    mov dl,13
    call NetAddChar
    mov dl,10
    call NetAddChar
.finish
    mov al,[HoldCommand]
    cmp al,2
    jne .noprintst
    mov dl,'L'
;    call NetAddChar
    mov dl,'>'
;    call NetAddChar
    xor ebx,ebx
.next
    mov dl,[ChatString+ebx]
    cmp dl,0
    je .donenac
    push ebx
;    call NetAddChar
    pop ebx
    inc ebx
    jmp .next
.donenac
    mov dl,13
;    call NetAddChar
    mov dl,10
;    call NetAddChar
    mov dword[ChatString],0
    jmp .skipcommand
.noprintst
    cmp al,20
    jne .nolatencyc
    mov al,[Latencytochange]
    mov [Latency],al
    jmp .skipcommand
.nolatencyc
    cmp al,3
    jne .nopl1b
    cmp byte[pl1neten],2
    je .nopl1b
    xor byte[pl1neten],1
.nopl1b
    cmp al,4
    jne .nopl2b
    cmp byte[pl2neten],2
    je .nopl2b
    xor byte[pl2neten],1
.nopl2b
    cmp al,5
    jne .nopl3b
    cmp byte[pl3neten],2
    je .nopl3b
    xor byte[pl3neten],1
.nopl3b
    cmp al,6
    jne .nopl4b
    cmp byte[pl4neten],2
    je .nopl4b
    xor byte[pl4neten],1
.nopl4b
    cmp al,7
    jne .nopl5b
    cmp byte[pl5neten],2
    je .nopl5b
    xor byte[pl5neten],1
.nopl5b
    cmp al,8
    jne .nobackstateb
    xor byte[BackState],1
    cmp byte[BackState],0
    jne .notbstate0b
    mov byte[Latency],10
    mov byte[Latencytochange],10
.notbstate0b
    cmp byte[BackState],1
    jne .notbstate1b
    mov byte[Latency],3
    mov byte[Latencytochange],3
.notbstate1b
.nobackstateb
    cmp al,50
    jne .nosramnoneb
    mov byte[UseRemoteSRAMData],0
.nosramnoneb
    cmp al,41
    jne .nosramlocalb
    mov byte[UseRemoteSRAMData],1
.nosramlocalb
    cmp al,42
    jne .nosramremoteb
    mov byte[UseRemoteSRAMData],2
.nosramremoteb
    cmp al,11
    jne .noload
.startloading
    mov byte[HoldCommand],1
    mov byte[RemoteCommand],1
    mov eax,FileNameMod
    call GUIloadfilename.nocnettype
    mov byte[sramsavedis],1
    call transfersram
    ret
.noload
.skipcommand
    mov al,[RemoteCommand]
    mov [HoldCommand],al
    cmp al,2
    jne near .nosendstcheck
    cmp byte[ChatString],0
    jne .nosendstcheck
    mov byte[RemoteCommand],1
    mov byte[HoldCommand],1
    mov al,1
.nosendstcheck
    cmp al,1
    je near .finsend
    call PreparePacket
    call RemoteSendChar
.nosend
    mov al,[RemoteCommand]
    cmp al,2
    jne near .nosendst

    push eax
    ; Capitalize ChatString
    mov eax,ChatString
.nextletter
    cmp byte[eax],0
    je .doneletter
    cmp byte[eax],'a'
    jb .noletter
    cmp byte[eax],'z'
    jb .noletter
    sub byte[eax],'a'-'A'
.noletter
    inc eax
    jmp .nextletter
.doneletter
    cmp dword[ChatString],'/ME '
    je .action
    cmp dword[ChatString],'/NIC'
    jne .notaction
    cmp word[ChatString+4],'K '
    je near .nickchange
.notaction
    mov eax,.chatinitial
    mov ebx,ChatString
    call ConcatString
    mov eax,ChatNick
    mov ebx,ChatString
    call ConcatString
    jmp .noaction
.action
    mov eax,ChatString
    mov ebx,4
    call ClipString
    mov eax,.actinitial2
    mov ebx,ChatString
    call ConcatString
    mov eax,ChatNick
    mov ebx,ChatString
    call ConcatString
    mov eax,.actinitial
    mov ebx,ChatString
    call ConcatString
    jmp .noaction
.nickchange
    mov eax,ChatString
    mov ebx,6
    call ClipString
    ; Valid String?
    mov eax,ChatString
    xor ebx,ebx
.loopnickcheck
    cmp byte[eax],0
    je .donestcheck
    cmp byte[eax],'-'
    je .okaystring
    cmp byte[eax],'_'
    je .okaystring
    cmp byte[eax],'^'
    je .okaystring
    cmp byte[eax],'='
    je .okaystring
    cmp byte[eax],'+'
    je .okaystring
    cmp byte[eax],'['
    je .okaystring
    cmp byte[eax],']'
    je .okaystring
    cmp byte[eax],'0'
    jb .notokaystring
    cmp byte[eax],'9'
    jbe .okaystring
    cmp byte[eax],'A'
    jb .notokaystring
    cmp byte[eax],'Z'
    jbe .okaystring
    jmp .notokaystring
.okaystring
    inc eax
    inc ebx
    jmp .loopnickcheck
.donestcheck
    cmp ebx,9
    ja .notokaystringb
    mov eax,ChatString
    mov ebx,.tempnick
    call CopyString
    mov eax,.nickchangeb
    mov ebx,ChatString
    call ConcatString
    mov eax,ChatNick
    mov ebx,ChatString
    call ConcatString
    mov eax,.tempnick
    mov ebx,ChatNick
    call CopyString
    jmp .noaction
.notokaystring
    mov esi,.strinvalid
    call WritetochatBuffer
    pop eax
    jmp .finsend
.notokaystringb
    mov esi,.strtoolong
    call WritetochatBuffer
    pop eax
    jmp .finsend
.noaction
    pop eax

    xor ebx,ebx
.next2
    mov al,[ChatString+ebx]
    push ebx
    push eax
    call RemoteSendChar
    pop eax
    pop ebx
    inc ebx
    cmp al,0
    jne .next2
    mov esi,ChatString
    call WritetochatBuffer
    mov dl,13
    call NetAddChar
    mov dl,10
    call NetAddChar
    jmp .finsend2
.nosendst
    cmp al,20
    jne .nolatencych
    mov al,[Latencytochange]
    call RemoteSendChar
    jmp .finsend2
.nolatencych
; FileNameMod times 29 db 0
    cmp al,10
    jne .nosendfn
    xor ebx,ebx
.next3
    mov al,[FileNameMod+ebx]
    push ebx
    push eax
    call RemoteSendChar
    pop eax
    pop ebx
    inc ebx
    cmp al,0
    jne .next3
    jmp .finsend2
.nosendfn
.finsend2
    call SendPacket
.finsend
    mov byte[RemoteCommand],1
    ret

SECTION .data
.chatinitial db '>',0
.actinitial db '*',0
.actinitial2 db ' ',0
.loading db 'LOADING ',0
.filenotf db 'FILE NOT FOUND!',13,10,0
.filenotfr db 'FILE NOT FOUND AT REMOTE!',13,10,0
.filenotu db 'UNABLE TO LOAD!',13,10,0
.tempnick times 16 db 0
.strinvalid db '** INVALID NICK **',13,10,0
.strtoolong db '** NICK IS TOO LONG **',13,10,0
.nickchangeb db ' IS NOW KNOWN AS ',0
SECTION .text

transfersram:
    ;Do players want SRAM sent?
    cmp byte[UseRemoteSRAMData],0
    je .nosram
    
    ;It has a battery, send it.
    cmp byte[CHIPBATT],1
    je .yessram
    
    ;If last if failed, no battery, so no SRAM sending for SA1 or SFX
    cmp byte[SFXEnable],1
    je .nosram
    cmp byte[SA1Enable],1
    je .nosram
    cmp dword[ramsize],0
    je .nosram
.yessram
    cmp byte[UseRemoteSRAMData],1
    je .uselocal
    cmp byte[UseRemoteSRAMData],2
    je near .useremote
.nosram
    call clearsram
    ret

    
.uselocal
    call PreparePacket
    mov al,233
    call RemoteSendChar
    call SendPacket
.notimer
    pushad
    call JoyRead
    popad
    call RemoteGetChar
    cmp dh,0
    je .notimer
    cmp dl,234
    jne .notimer

    ; send sram
    mov ecx,[ramsize]
    mov ebx,[sram]
    xor esi,esi
.loop
    push ebx
    push ecx
    mov ebx,esi
    and ebx,0FFh
    cmp ebx,0
    jne .notzero
    call PreparePacket
.notzero
    pop ecx
    pop ebx
    mov al,[ebx]
    push ebx
    push ecx
    call RemoteSendChar
    pop ecx
    mov ebx,esi
    and ebx,0FFh
    cmp ebx,0FFh
    jne .notsend
    call SendPacket
.notsend
    pop ebx
    inc esi
    inc ebx
    dec ecx
    jnz .loop
    mov byte[sramsavedis],0
    ret

.useremote
    call PreparePacket
    mov al,234
    call RemoteSendChar
    call SendPacket
.notimerb
    pushad
    call JoyRead
    popad
    call RemoteGetChar
    cmp dh,0
    je .notimerb
    cmp dl,233
    jne .notimerb

    ; receive sram
    mov ecx,[ramsize]
    mov ebx,[sram]
.loopb
.getnext
    pushad
    call JoyRead
    popad
    call RemoteGetChar
    cmp dh,0
    je .getnext
    mov [ebx],dl
    inc ebx
    dec ecx
    jnz .loopb
    ret

NEWSYM RemoteSendEAX
;    call RemoteSendChar
    shr eax,8
    call RemoteSendChar
    shr eax,8
    call RemoteSendChar
    shr eax,8
    call RemoteSendChar
    ret

NEWSYM RemoteGetEAX
;.getnext
;    call RemoteGetChar
;    cmp dh,0
;    je .getnext
;    mov bl,dl
    xor ebx,ebx
;    shl ebx,8
.getnext2
    call RemoteGetChar
    cmp dh,0
    je .getnext2
    mov bl,dl
    shl ebx,8
.getnext3
    call RemoteGetChar
    cmp dh,0
    je .getnext3
    mov bl,dl
    shl ebx,8
.getnext4
    call RemoteGetChar
    cmp dh,0
    je .getnext4
    mov bl,dl
    bswap ebx
    mov eax,ebx
    ret


NEWSYM RemoteSendChar
%ifdef __MSDOS__
    cmp byte[WhichRemote],1
    jne .nomodem
    call ModemSendChar
.nomodem
    cmp byte[WhichRemote],2
    jne .noipx
    call ipxsendchar
.noipx
%endif
    cmp byte[WhichRemote],4
    jne .notcpip
    call TCPIPStoreByte
.notcpip
    ret

NEWSYM RemoteGetChar
%ifdef __MSDOS
    cmp byte[WhichRemote],1
    jne .nomodem
    call ModemGetChar
.nomodem
    cmp byte[WhichRemote],2
    jne .noipx
    call ipxgetchar
.noipx
%endif
    cmp byte[WhichRemote],4
    jne .notcpip
    call TCPIPGetByte
.notcpip
    ret

%ifdef __MSDOS__
ModemSendStr:
.loop
    mov al,[esi]
    or al,al
    jz .nomore
    call ModemSendChar
    inc esi
    jmp .loop
.nomore
    ret
%endif

SECTION .bss
NetLoadHandle resw 1
NetStateSize resd 1
NetStateTotal resd 1
NetStateQuit resb 1
NetStateBuffer resb 2048
SECTION .data
Netfname db 'nettemp.zst',0
SECTION .text

NetLoadStuff:
    call loadnetopen
    mov byte[NetStateQuit],0
    mov edx,fnamest+1
    call Open_File
    jc .nostate
    mov bx,ax
    mov cx,0
    mov dx,0
    call File_Seek_End
    shl edx,16
    mov dx,ax
    mov [NetStateSize],edx
    mov [NetStateTotal],edx
    call Close_File
    mov byte[CNetType],21
    call PreparePacket
    mov al,14
    call RemoteSendChar
    mov al,[NetStateSize]
    call RemoteSendChar
    mov al,[NetStateSize+1]
    call RemoteSendChar
    mov al,[NetStateSize+2]
    call RemoteSendChar
    call SendPacket
    call ChangetoSRAMdir
    mov edx,fnamest+1
    call Open_File
    mov [NetLoadHandle],ax
    call ChangetoLOADdir
.nostate
    ret

loadstaterecvinit:
    mov byte[NetStateQuit],0
    mov dword[NetStateSize],0
.noreceive
    pushad
    call JoyRead
    popad
    call RemoteGetChar
    cmp dh,0
    je .noreceive
    mov [NetStateSize],dl
.noreceive1
    call RemoteGetChar
    cmp dh,0
    je .noreceive1
    mov [NetStateSize+1],dl
.noreceive2
    call RemoteGetChar
    cmp dh,0
    je .noreceive2
    mov [NetStateSize+2],dl
    call ChangetoSRAMdir
    mov edx,Netfname
    call Create_File
    mov edx,[NetStateSize]
    mov [NetStateTotal],edx
    mov [NetLoadHandle],ax
    call ChangetoLOADdir
    ret

loadstatesend:
    call RemoteGetChar
    cmp dh,0
    je .noreceived
    cmp dl,15
    je .finished
    call ProcessRemoteCommand
.noreceived
    cmp byte[NetStateQuit],0
    je .noquit
.finished
    call PreparePacket
    mov al,15
    call RemoteSendChar
    call SendPacket
    mov byte[CNetType],20
    mov esi,StateLoadedFail
    call ProcessModem.writestr
    mov bx,[NetLoadHandle]
    call Close_File
    cmp byte[NetLoadState],0
    je .norun
    mov byte[GUIQuit],2
.norun
    ret
.noquit
    call PreparePacket
    mov al,1
    call RemoteSendChar
    mov ecx,[NetStateSize]
    cmp ecx,2047
    jbe .notsmaller
    mov ecx,2047
.notsmaller
    sub [NetStateSize],ecx
    mov bx,[NetLoadHandle]
    mov edx,NetStateBuffer
    push ecx
    call Read_File
    pop ecx
    mov edx,NetStateBuffer
.loop
    mov al,[edx]
    push edx
    push ecx
    call RemoteSendChar
    pop ecx
    pop edx
    inc edx
    dec ecx
    jnz .loop
    call SendPacket
    cmp dword[NetStateSize],0
    jne .notzero
    pushad
    call loadstate2
    mov byte[RestoreValues],0
    mov bx,[NetLoadHandle]
    call Close_File
    mov byte[CNetType],20
    mov esi,StateLoadedOk
    call ProcessModem.writestr
    cmp byte[NetLoadState],0
    je .norun2
    mov byte[GUIQuit],2
.norun2
    popad
.notzero
    ret

loadstaterecv:
    cmp byte[NetStateQuit],0
    je .noquit2
    mov byte[NetStateQuit],0
    call PreparePacket
    mov al,15
    call RemoteSendChar
    call SendPacket
.noquit2
    call RemoteGetChar
    cmp dh,0
    jne .received
    ret
.received
    cmp dl,1
    je .noquit
    mov byte[CNetType],20
    mov esi,StateLoadedFail
    call ProcessModem.writestr
    cmp byte[NetLoadState],0
    je .norun
    mov byte[GUIQuit],2
.norun
    mov bx,[NetLoadHandle]
    call Close_File
    ret
.noquit
    mov ecx,[NetStateSize]
    cmp ecx,2047
    jbe .notsmaller
    mov ecx,2047
.notsmaller
    sub [NetStateSize],ecx
    mov edx,NetStateBuffer
    push ecx
.loop
    push edx
    push ecx
.notrecv
    call RemoteGetChar
    cmp dh,0
    je .notrecv
    mov al,dl
    pop ecx
    pop edx
    mov [edx],al
    inc edx
    dec ecx
    jnz .loop
    pop ecx
    mov bx,[NetLoadHandle]
    mov edx,NetStateBuffer
    call Write_File
    cmp dword[NetStateSize],0
    jne .notzero
    pushad
    mov bx,[NetLoadHandle]
    call Close_File
    call ChangetoSRAMdir
    mov edx,Netfname
    mov byte[RestoreValues],0
    call loadstate3
    call ChangetoLOADdir
    cmp byte[NetLoadState],0
    je .norun2
    mov byte[GUIQuit],2
.norun2
    mov byte[CNetType],20
    mov esi,StateLoadedOk
    call ProcessModem.writestr
    popad
.notzero
    ret

SECTION .data
StateLoadedOk db 10,13,'STATE LOADED.',10,13,10,13,0
StateLoadedFail db 10,13,'LOAD FAILED.',10,13,10,13,0
SECTION .text
