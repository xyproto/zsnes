%macro SPCSetFlagnzc 0
  js .setsignflag
  jz .setzeroflag
  mov byte[spcNZ],1
  jc .setcarryflag
  and byte[spcP],0FEh
  ret
.setsignflag
  mov byte[spcNZ],80h
  jc .setcarryflag
  and byte[spcP],0FEh
  ret
.setzeroflag
  mov byte[spcNZ],0
  jc .setcarryflag
  and byte[spcP],0FEh
  ret
.setcarryflag
  or byte[spcP],1
  ret
%endmacro
NEWSYM asm_OpBE     ; DAS A     decimal adjust for sub  N......ZC
    ; copy al flags into AH
    xor ah,ah
    test byte[spcNZ],80h
    jz .noneg
    or ah,10000000b
.noneg
    test byte[spcP],01h
    jz .nocarry
    or ah,00000001b
.nocarry
    test byte[spcNZ],0FFh
    jnz .nozero
    or ah,01000000b
.nozero
    test byte[spcP],08h
    jz .nohcarry
    or ah,00010000b
.nohcarry
    mov al,[spcA]
    sahf
    das
    mov [spcA],al
    SPCSetFlagnzc

NEWSYM asm_OpDF     ; DAA A      decimal adjust for add  N......ZC
    ; copy al flags into AH
    xor ah,ah
    test byte[spcNZ],80h
    jz .noneg
    or ah,10000000b
.noneg
    test byte[spcP],01h
    jz .nocarry
    or ah,00000001b
.nocarry
    test byte[spcNZ],0FFh
    jnz .nozero
    or ah,01000000b
.nozero
    test byte[spcP],08h
    jz .nohcarry
    or ah,00010000b
.nohcarry
    mov al,[spcA]
    sahf
    daa
    mov [spcA],al
    SPCSetFlagnzc
