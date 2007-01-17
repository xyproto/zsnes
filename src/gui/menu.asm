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

EXTSYM FPSOn,MessageOn,cbitmode,copyvid
EXTSYM MsgCount,Msgptr,OutputGraphicString,OutputGraphicString16b,vidbuffer
EXTSYM curblank,drawhline,drawhline16b,drawvline,drawvline16b,frameskip
EXTSYM pressed,dumpsound,Grab_BMP_Data,Grab_BMP_Data_8
EXTSYM spcon,vesa2_bpos,vesa2_clbit,vesa2_gpos,vesa2_rpos,
EXTSYM spritetablea,sprlefttot,newengen,Get_Key,continueprognokeys
EXTSYM ForceNonTransp,GUIOn,Check_Key,JoyRead,GetScreen,SSKeyPressed
EXTSYM SPCKeyPressed,StopSound,StartSound,ExecExitOkay,t1cc,Clear2xSaIBuffer
EXTSYM ScreenShotFormat,spcsaved,savespcdata
EXTSYM exiter,xpb,xpc,snesmmap,memtabler8,snesmap2,regaccessbankr8,dmadata,initaddrl
EXTSYM spcPCRam,xp,curcyc,Curtableaddr,UpdateDPage,splitflags,execsingle,joinflags
EXTSYM pdh,SPCRAM,cvidmode

%ifdef __MSDOS__
EXTSYM GUI16VID
%endif

%ifndef NO_DEBUGGER
EXTSYM numinst,debuggeron
%endif

%ifndef NO_PNG
EXTSYM Grab_PNG_Data
%endif

SECTION .text

GUIBufferData:
    mov ecx,16000
    cmp byte[cbitmode],1
    jne near .16b
    add ecx,16384
.16b
    ; copy to spritetable
    mov esi,[vidbuffer]
    mov edi,[spritetablea]
    add esi,4*384
    add edi,4*384
.loop
    mov eax,[esi]
    mov [edi],eax
    add esi,4
    add edi,4
    dec ecx
    jnz .loop
    mov edi,sprlefttot
    mov ecx,64*5
.a
    mov dword[edi],0
    add edi,4
    dec ecx
    jnz .a
    ret

GUIUnBuffer:
    mov ecx,16000
    cmp byte[cbitmode],1
    jne near .16b
    add ecx,16384
.16b
    ; copy from spritetable
    mov esi,[vidbuffer]
    mov edi,[spritetablea]
    add esi,4*384
    add edi,4*384
.loop
    mov eax,[edi]
    mov [esi],eax
    add esi,4
    add edi,4
    dec ecx
    jnz .loop
    ret

SECTION .bss
NEWSYM nextmenupopup, resb 1
NEWSYM NoInputRead, resb 1
NEWSYM PrevMenuPos, resb 1
NEWSYM MenuDisplace, resd 1
NEWSYM MenuDisplace16, resd 1
NEWSYM MenuNoExit, resb 1
NEWSYM SPCSave, resb 1

SECTION .text

NEWSYM showmenu
    mov byte[ForceNonTransp],1
    cmp byte[cbitmode],1
    je near .nopalread
    mov edi,[vidbuffer]
    add edi,100000
    mov dx,03C7h
    mov al,0
    out dx,al
    mov dx,03C9h
    mov ecx,768
    mov byte[edi],12
    inc edi
.b
    in al,dx
    shl al,2
    mov [edi],al
    inc edi
    dec ecx
    jnz .b
.nopalread

    cmp byte[cbitmode],1
    je near .nopal16b
    ; set palette of colors 128,144, and 160 to white, blue, and red
    mov al,128
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,63
    out dx,al
    out dx,al
    out dx,al
    mov al,144
    mov dx,03C8h
    out dx,al
    inc dx
    xor al,al
    out dx,al
    out dx,al
    mov al,50
    out dx,al
    mov al,160
    mov dx,03C8h
    out dx,al
    inc dx
    mov al,45
    out dx,al
    xor al,al
    out dx,al
    out dx,al
.nopal16b

    mov byte[NoInputRead],0
    cmp byte[newengen],0
    je .nong16b
    cmp byte[cbitmode],0
    je .nong16b
    call GetScreen
.nong16b
    cmp byte[SSKeyPressed],1
    jne .nosskey
    mov byte[SSKeyPressed],0
    call saveimage
    jmp .nopalwrite
.nosskey
    cmp byte[SPCKeyPressed],1
    je near .savespckey
    test byte[pressed+14],1
    jz .nof12
    call saveimage
    jmp .nopalwrite
.nof12
    mov dword[menucloc],0
    cmp byte[nextmenupopup],0
    je .nomenuinc2
    mov byte[pressed+1Ch],0
    mov dword[menucloc],40*288
    cmp byte[PrevMenuPos],1
    jne .nomenuinc
    mov dword[menucloc],50*288
.nomenuinc
    cmp byte[PrevMenuPos],2
    jne .nomenuinc2
    mov dword[menucloc],60*288
.nomenuinc2
    cmp byte[PrevMenuPos],3
    jne .nomenuinc3
    mov dword[menucloc],70*288
.nomenuinc3

    mov dword[menudrawbox8b.stringi+13],' BMP'
%ifndef NO_PNG
    cmp byte[ScreenShotFormat],0
    je .normalscrn
%ifdef __MSDOS__
    movzx eax,byte[cvidmode]
    cmp byte[GUI16VID+eax],1
    je .pngok
    mov byte[ScreenShotFormat],0
    jmp .normalscrn
%endif
.pngok
    mov dword[menudrawbox8b.stringi+13],' PNG'
%endif
.normalscrn
    mov byte[nextmenupopup],0
    mov byte[menu16btrans],0
    mov byte[pressed+1],0
    mov byte[pressed+59],0
    mov byte[curblank],00h
    call GUIBufferData
    ; Draw box
    call menudrawbox8b
    call menudrawbox8b
    cmp byte[newengen],0
    je .notng
    mov byte[GUIOn],1
.notng
    pushad
    call copyvid
    popad
    call StopSound
.nextkey
    ;call GUIUnBuffer
    call menudrawbox8b
    push eax
    call copyvid
    pop eax

    call JoyRead
    call Check_Key
    or al,al
    jz .nextkey
    call Get_Key
    cmp al,0
    jne near .processextend

    call Get_Key
    cmp al,72
    jne .noup
    cmp dword[menucloc],0
    jne .nogoup
    add dword[menucloc],80*288
.nogoup
    sub dword[menucloc],10*288
    call menudrawbox8b
;    mov al,[newengen]                  ; WTF?
;    mov byte[newengen],0

;    mov [newengen],al
    jmp .nextkey
.noup
    cmp al,80
    jne .nodown
    cmp dword[menucloc],70*288
    jne .nogodown
    sub dword[menucloc],80*288
.nogodown
    add dword[menucloc],10*288
    call menudrawbox8b
;    mov al,[newengen]
;    mov byte[newengen],0
;    push eax
    call copyvid
;    pop eax
;    mov [newengen],al
    jmp .nextkey
.nodown
    jmp .nextkey
.processextend
    cmp al,27
    je near .exitloop
    cmp al,13
    je .done
    jmp .nextkey
.done
    call GUIUnBuffer
;    mov al,[newengen]
;    mov byte[newengen],0
;    push eax
    call copyvid
;    pop eax
;    mov [newengen],al
    cmp dword[menucloc],0
    jne .nosaveimg
    call saveimage
.nosaveimg
    cmp dword[menucloc],40*288
    jne .nosaveimg2
    call saveimage
    mov byte[ExecExitOkay],0
    mov byte[nextmenupopup],3
    mov byte[NoInputRead],1
    mov byte[t1cc],0
    mov byte[PrevMenuPos],0
.nosaveimg2
    cmp dword[menucloc],50*288
    jne .noskipframe
    mov byte[ExecExitOkay],0
    mov byte[nextmenupopup],3
    mov byte[NoInputRead],1
    mov byte[t1cc],0
    mov byte[PrevMenuPos],1
.noskipframe
    cmp dword[menucloc],70*288
    jne .noimagechange
    cmp byte[cbitmode],0
    je .noimagechange
    xor byte[ScreenShotFormat],1
    mov byte[MenuNoExit],1
    mov byte[ExecExitOkay],0
    mov byte[nextmenupopup],1
    mov byte[NoInputRead],1
    mov byte[t1cc],0
    mov byte[PrevMenuPos],3
.noimagechange
    cmp dword[menucloc],60*288
    jne .nomovewin
    mov byte[MenuNoExit],1
    mov byte[ExecExitOkay],0
    mov byte[nextmenupopup],1
    mov byte[NoInputRead],1
    mov byte[t1cc],0
    mov byte[PrevMenuPos],2
    cmp dword[MenuDisplace],0
    je .movewin
    mov dword[MenuDisplace],0
    mov dword[MenuDisplace16],0
    jmp .nomovewin
.movewin
    mov dword[MenuDisplace],90*288
    mov dword[MenuDisplace16],90*288*2
.nomovewin
    cmp dword[menucloc],10*288
    jne .nofps
    cmp byte[frameskip],0
    je .yesfs
    mov dword[Msgptr],.unablefps
    mov eax,[MsgCount]
    mov [MessageOn],eax
    jmp .nofps
.yesfs
    xor byte[FPSOn],1
.nofps
    cmp dword[menucloc],20*288
    jne near .nospcsave
.savespckey
    cmp byte[spcon],0
    je .nospc

    mov dword[Msgptr],.search
    mov eax,[MsgCount]
    mov [MessageOn],eax
    call copyvid
    mov byte[SPCSave],1
    call breakatsignb
    mov byte[SPCSave],0
    pushad
    call savespcdata
    popad

    mov byte[curblank],40h
    mov dword[Msgptr],spcsaved
    mov eax,[MsgCount]
    mov [MessageOn],eax
    jmp .nospcsave
.nospc
    mov dword[Msgptr],.nosound
    mov eax,[MsgCount]
    mov [MessageOn],eax
    jmp .nospcsave
.unablespc
    mov dword[Msgptr],.unable
    mov eax,[MsgCount]
    mov [MessageOn],eax
    jmp .nospcsave
.yesesc
    mov dword[Msgptr],.escpress
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nospcsave
    cmp dword[menucloc],30*288
    jne .nosnddmp
    pushad
    call dumpsound
    popad
    mov dword[Msgptr],.sndbufsav
    mov eax,[MsgCount]
    mov [MessageOn],eax
.nosnddmp
    cmp byte[SPCKeyPressed],1
    jne .exitloop
    mov byte[SPCKeyPressed],0
    jmp .nopalwrite
.exitloop
    call GUIUnBuffer
;    mov al,[newengen]
;    mov byte[newengen],0
;    push eax
    call copyvid
;    pop eax
;    mov [newengen],al
    cmp byte[cbitmode],1
    je near .nopalwrite
    mov edi,[vidbuffer]
    add edi,100000
    mov dx,03C8h
    mov al,0
    out dx,al
    mov dx,03C9h
    mov ecx,768
    inc edi
.c
    mov al,[edi]
    shr al,2
    out dx,al
    inc edi
    dec ecx
    jnz .c
.nopalwrite
    mov eax,pressed
    mov ecx,256
.looppr
    cmp byte[eax],1
    jne .notpr
    mov byte[eax],2
.notpr
    inc eax
    dec ecx
    jnz .looppr
;    mov byte[pressed+1],2
;    cmp byte[pressed+59],1
;    jne .not59
;    mov byte[pressed+59],2
;.not59
;    cmp byte[pressed+28],1
;    jne .not28
;    mov byte[pressed+28],2
;.not28
    call StartSound
    mov byte[ForceNonTransp],0
    mov byte[GUIOn],0
    pushad
    call Clear2xSaIBuffer
    popad
    cmp byte[MenuNoExit],1
    je .noexitmenu
    jmp continueprognokeys
.noexitmenu
    mov byte[MenuNoExit],0
    jmp showmenu

SECTION .data
.unablefps db 'NEED AUTO FRAMERATE ON',0
.sndbufsav db 'BUFFER SAVED AS SOUNDDMP.RAW',0
.search    db 'SEARCHING FOR SONG START.',0
.nosound   db 'SOUND MUST BE ENABLED.',0
.unable    db 'CANNOT USE IN NEW GFX ENGINE.',0
.escpress  db 'ESC TERMINATED SEARCH.',0
SECTION .text

NEWSYM menudrawbox8b
    cmp byte[cbitmode],1
    je near menudrawbox16b
    ; draw a small blue box with a white border
    mov esi,40+20*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov ecx,150
    mov al,95
.loop
    mov byte[esi],144
    inc esi
    dec ecx
    jnz .loop
    add esi,288-150
    dec al
    mov ecx,150
    jnz .loop
    mov al,128
    ; Draw lines
    mov esi,40+20*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov ecx,150
    call drawhline
    mov esi,40+20*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov ecx,95
    call drawvline
    mov esi,40+114*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov ecx,150
    call drawhline
    mov esi,40+32*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov ecx,150
    call drawhline
    mov esi,189+20*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov ecx,95
    call drawvline
    call menudrawcursor8b

    mov esi,45+23*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.string
    call OutputGraphicString
    mov esi,45+35*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringa
    call OutputGraphicString
    mov esi,45+45*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringb
    test byte[FPSOn],1
    jz .nofps
    mov edi,.stringc
.nofps
    call OutputGraphicString
    mov esi,45+55*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringd
    call OutputGraphicString
    mov esi,45+65*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringe
    call OutputGraphicString
    mov esi,45+75*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringf
    call OutputGraphicString
    mov esi,45+85*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringg
    call OutputGraphicString
    mov esi,45+95*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringh
    call OutputGraphicString
    mov esi,45+105*288
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov edi,.stringi
    call OutputGraphicString
;    mov al,[newengen]
;    mov byte[newengen],0
;    push eax
    call copyvid
;    pop eax
;    mov [newengen],al
    ret

SECTION .data
.string db 'MISC OPTIONS',0
.stringa db 'SAVE SNAPSHOT',0
.stringb db 'SHOW FPS',0
.stringc db 'HIDE FPS',0
.stringd db 'SAVE SPC DATA',0
.stringe db 'SOUND BUFFER DUMP',0
.stringf db 'SNAPSHOT/INCR FRM',0
.stringg db 'INCR FRAME ONLY',0
.stringh db 'MOVE THIS WINDOW',0
.stringi db 'IMAGE FORMAT: ---',0
SECTION .text

NEWSYM menudrawcursor8b
    cmp byte[cbitmode],1
    je near menudrawcursor16b
    ; draw a small red box
    mov esi,41+34*288
    add esi,[menucloc]
    add esi,[vidbuffer]
    add esi,[MenuDisplace]
    mov ecx,148
    mov al,9
.loop
    mov byte[esi],160
    inc esi
    dec ecx
    jnz .loop
    add esi,288-148
    dec al
    mov ecx,148
    jnz .loop

    mov al,128
    ret

SECTION .bss
NEWSYM menucloc, resd 1
SECTION .text

NEWSYM menudrawbox16b
    ; draw shadow behind box
    cmp byte[menu16btrans],0
    jne .noshadow
    mov byte[menu16btrans],1
    mov esi,50*2+30*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,150
    mov al,95
    mov ah,5
.loop16b2
    mov dx,[esi]
    and dx,[vesa2_clbit]
    shr dx,1
    mov [esi],dx
    add esi,2
    dec ecx
    jnz .loop16b2
    add esi,288*2-150*2
    dec al
    mov ecx,150
    jnz .loop16b2
.noshadow

    mov ax,01Fh
    mov cl,[vesa2_rpos]
    shl ax,cl
    mov [.allred],ax
    mov ax,012h
    mov cl,[vesa2_bpos]
    shl ax,cl
    mov dx,ax
    mov ax,01h
    mov cl,[vesa2_gpos]
    shl ax,cl
    mov bx,ax
    mov ax,01h
    mov cl,[vesa2_rpos]
    shl ax,cl
    or bx,ax

    ; draw a small blue box with a white border
    mov esi,40*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,150
    mov al,95
    mov ah,5
.loop16b
    mov [esi],dx
    add esi,2
    dec ecx
    jnz .loop16b
    add esi,288*2-150*2
    dec ah
    jnz .nocolinc16b
    add dx,bx
    mov ah,5
.nocolinc16b
    dec al
    mov ecx,150
    jnz .loop16b

    ; Draw lines
    mov ax,0FFFFh
    mov esi,40*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,150
    call drawhline16b
    mov esi,40*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,95
    call drawvline16b
    mov esi,40*2+114*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,150
    call drawhline16b
    mov esi,40*2+32*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,150
    call drawhline16b
    mov esi,189*2+20*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,95
    call drawvline16b
    call menudrawcursor16b

    mov esi,45*2+23*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.string
    call OutputGraphicString16b
    mov esi,45*2+35*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringa
    call OutputGraphicString16b
    mov esi,45*2+45*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringb
    test byte[FPSOn],1
    jz .nofps
    mov edi,menudrawbox8b.stringc
.nofps
    call OutputGraphicString16b
    mov esi,45*2+55*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringd
    call OutputGraphicString16b
    mov esi,45*2+65*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringe
    call OutputGraphicString16b
    mov esi,45*2+75*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringf
    call OutputGraphicString16b
    mov esi,45*2+85*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringg
    call OutputGraphicString16b
    mov esi,45*2+95*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringh
    call OutputGraphicString16b
    mov esi,45*2+105*288*2
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov edi,menudrawbox8b.stringi
    call OutputGraphicString16b
;    mov al,[newengen]
;    mov byte[newengen],0
;    push eax
    call copyvid
;    pop eax
;    mov [newengen],al
    ret

SECTION .bss
.allred resw 1
.blue   resw 1
.stepb  resw 1

NEWSYM menu16btrans, resb 1

SECTION .text

NEWSYM menudrawcursor16b
    ; draw a small red box
    mov esi,41*2+34*288*2
    add esi,[menucloc]
    add esi,[menucloc]
    add esi,[vidbuffer]
    add esi,[MenuDisplace16]
    mov ecx,148
    mov al,9
    mov bx,[menudrawbox16b.allred]
.loop
    mov [esi],bx
    add esi,2
    dec ecx
    jnz .loop
    add esi,288*2-148*2
    dec al
    mov ecx,148
    jnz .loop
    mov al,128
    ret

saveimage:
    mov byte[pressed+1],0
    mov byte[pressed+59],0

%ifndef NO_PNG
    cmp byte[ScreenShotFormat],1
    jne .notpng
    pushad
    call Grab_PNG_Data
    popad
    ret
.notpng
%endif

    cmp byte[cbitmode],1
    je near .save16b
    pushad
    call Grab_BMP_Data_8
    popad
    ret

.save16b
    pushad
    call Grab_BMP_Data
    popad
    ret

SECTION .data
NEWSYM keyonsn, db 0
NEWSYM prbreak, db 0
SECTION .text

breakatsignb:
    mov byte[keyonsn],0
    mov byte[prbreak],0
%ifndef NO_DEBUGGER
    cmp byte[SPCSave],1
    jne .nospcsave
    mov byte[debuggeron],1
.nospcsave
%endif

    mov byte[exiter],01h
    xor eax,eax
    xor ebx,ebx
    xor ecx,ecx
    xor edx,edx
    mov bl,[xpb]
    mov ax,[xpc]
    test ax,8000h
    jz .loweraddr
    mov esi,[snesmmap+ebx*4]
    jmp .skiplower
.loweraddr
    cmp ax,4300h
    jb .lower
    cmp dword[memtabler8+ebx*4],regaccessbankr8
    je .dma
.lower
    mov esi,[snesmap2+ebx*4]
    jmp .skiplower
.dma
    mov esi,dmadata-4300h
.skiplower
    mov [initaddrl],esi
    add esi,eax                 ; add program counter to address
    mov ebp,[spcPCRam]
    mov dl,[xp]                 ; set flags
    mov dh,[curcyc]             ; set cycles
    mov edi,[Curtableaddr]
    call UpdateDPage
    ; execute
.loopa
    call splitflags
    call execsingle
    call joinflags
    mov dh,[pdh]
%ifndef NO_DEBUGGER
    inc dword[numinst]
    cmp byte[numinst],0
    jne .skipa
    call Check_Key
    test al,0FFh
    jz .skipa
    call Get_Key
    cmp al,27
    je .skipc
.skipa
%endif
    cmp byte[SPCRAM+6],40h
    je .skipc
    cmp byte[keyonsn],1
    jne .loopa
    jmp .noesc
.skipc
    mov byte[prbreak],1
.noesc
    ; copy back data
    mov [spcPCRam],ebp
    mov [Curtableaddr],edi
    mov [xp],dl
    mov [curcyc],dh

    mov eax,[initaddrl]
    sub esi,eax                 ; subtract program counter by address
    mov [xpc],si
    mov byte[exiter],0
%ifndef NO_DEBUGGER
    cmp byte[SPCSave],1
    jne .nospcsave2
    mov byte[debuggeron],0
.nospcsave2
%endif
    ret
