NEWSYM asm_SPCRegF0
    mov [SPCRAM+0F0h],al
    ret
NEWSYM asm_SPCRegF1
    cmp byte[disablespcclr],1
    je .No23Clear
    test al,10h
    jz .No01Clear
    mov byte[SPCRAM+0F4h],0
    mov byte[SPCRAM+0F5h],0
.No01Clear
    test al,20h
    jz .No23Clear
    mov byte[SPCRAM+0F6h],0
    mov byte[SPCRAM+0F7h],0
.No23Clear
    cmp byte[SPCSkipXtraROM],1
    je near .AfterNoROM
    test al,80h
    jz .NoROM
    push eax
    push ebx
    xor eax,eax
.loopa
    mov bl,[SPCROM+eax]
    mov [SPCRAM+0FFC0h+eax],bl
    inc eax
    cmp eax,040h
    jne .loopa
    pop ebx
    pop eax
    jmp .AfterNoROM
.NoROM
    push eax
    push ebx
    xor eax,eax
.loopb
    mov bl,[spcextraram+eax]
    mov [SPCRAM+0FFC0h+eax],bl
    inc eax
    cmp eax,040h
    jne .loopb
    pop ebx
    pop eax
.AfterNoROM
    mov [SPCRAM+0F1h],al
    and al,0Fh
    mov [timeron],al
    ret
NEWSYM asm_SPCRegF2
    mov [SPCRAM+0F2h],al
    push eax
    push ebx
    xor eax,eax
    mov al,[SPCRAM+0F2h]
    mov bl,[DSPMem+eax]
    mov [SPCRAM+0F3h],bl
    pop ebx
    pop eax
    ret
NEWSYM asm_SPCRegF3
    push ebx
    xor ebx,ebx
    mov bl,[SPCRAM+0F2h]
    and bl,07fh
    call dword near [dspWptr+ebx*4]
    pop ebx
    mov [SPCRAM+ebx],al
    ret
NEWSYM asm_SPCRegF4
    mov [reg1read],al
    inc dword[spc700read]
    ret
NEWSYM asm_SPCRegF5
    mov [reg2read],al
    inc dword[spc700read]
    ret
NEWSYM asm_SPCRegF6
    mov [reg3read],al
    inc dword[spc700read]
    ret
NEWSYM asm_SPCRegF7
    mov [reg4read],al
    inc dword[spc700read]
    ret
NEWSYM asm_SPCRegF8
    mov [SPCRAM+ebx],al
    ret
NEWSYM asm_SPCRegF9
    mov [SPCRAM+ebx],al
    ret
NEWSYM asm_SPCRegFA
    mov [timincr0],al
    test byte[timinl0],0FFh
    jne .nowrite
    mov [timinl0],al
.nowrite
    mov [SPCRAM+ebx],al
    ret
NEWSYM asm_SPCRegFB
    mov [timincr1],al
    test byte[timinl1],0FFh
    jne .nowrite
    mov [timinl1],al
.nowrite
    mov [SPCRAM+ebx],al
    ret
NEWSYM asm_SPCRegFC
    mov [timincr2],al
    test byte[timinl2],0FFh
    jne .nowrite
    mov [timinl2],al
.nowrite
    mov [SPCRAM+ebx],al
    ret
NEWSYM asm_SPCRegFD
    ret
NEWSYM asm_SPCRegFE
    ret
NEWSYM asm_SPCRegFF
    ret

; SPC Read Registers
; DO NOT MODIFY ANY REG!
; return data true al
NEWSYM asm_RSPCRegF0
    mov al,[SPCRAM+0f0h]
    ret
NEWSYM asm_RSPCRegF1
    mov al,[SPCRAM+0f1h]
    ret
NEWSYM asm_RSPCRegF2
    mov al,[SPCRAM+0f2h]
    ret
NEWSYM asm_RSPCRegF3
    mov al,[SPCRAM+0f3h]
    ret
NEWSYM asm_RSPCRegF4
    mov al,[SPCRAM+0f4h]
    ret
NEWSYM asm_RSPCRegF5
    mov al,[SPCRAM+0f5h]
    ret
NEWSYM asm_RSPCRegF6
    mov al,[SPCRAM+0f6h]
    ret
NEWSYM asm_RSPCRegF7
    mov al,[SPCRAM+0f7h]
    ret
NEWSYM asm_RSPCRegF8
    mov al,0 ;[SPCRAM+0f8h]
    ret
NEWSYM asm_RSPCRegF9
    mov al,0 ;[SPCRAM+0f9h]
    ret
NEWSYM asm_RSPCRegFA
    mov al,[SPCRAM+0fah]
    ret
NEWSYM asm_RSPCRegFB
    mov al,[SPCRAM+0fbh]
    ret
NEWSYM asm_RSPCRegFC
    mov al,[SPCRAM+0fch]
    ret

NEWSYM asm_RSPCRegFD
    mov al,[SPCRAM+0fdh]
    and al,0Fh
    mov byte[SPCRAM+0fdh],0
    mov byte[spcnumread],0
    ret

NEWSYM asm_RSPCRegFE
    mov al,[SPCRAM+0feh]
    and al,0Fh
    mov byte[SPCRAM+0feh],0
    mov byte[spcnumread],0
    ret

NEWSYM asm_RSPCRegFF
    mov al,[SPCRAM+0ffh]
    and al,0Fh
    mov byte[SPCRAM+0ffh],0
    mov byte[spcnumread],0
    ret
