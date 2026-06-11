; Portable scalar 2xSaI line filters.
%ifdef __AMD64__
bits 64
%else
bits 32
%endif

section .text

%ifdef MACHO
section .text align=16
section .data align=4
section .bss  align=4
%endif

%ifdef ELF
section .note.GNU-stack noalloc noexec nowrite progbits
%endif

%ifdef ELF
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%else
%imacro newsym 1
  GLOBAL _%1
  _%1:
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL _%1
  _%1:
  %1: %2
%endmacro
%endif
SECTION .text

%macro SCALE2X_LINE 0
    push ebp
    mov ebp,esp
    push esi
    push edi
    push ebx

    mov esi,[ebp+8]      ; src (u2*)
    mov ecx,[ebp+20]     ; width
    mov edi,[ebp+24]     ; dst (u1*)
    mov edx,[ebp+28]     ; dstPitch (bytes)
    lea ebx,[edi+edx]    ; dst + dstPitch

.loop
    mov ax,[esi]
    mov [edi],ax
    mov [edi+2],ax
    mov [ebx],ax
    mov [ebx+2],ax
    add esi,2
    add edi,4
    add ebx,4
    dec ecx
    jnz .loop

    pop ebx
    pop edi
    pop esi
    pop ebp
    ret
%endmacro

NEWSYM _2xSaISuper2xSaILine
    SCALE2X_LINE

NEWSYM _2xSaISuperEagleLine
    SCALE2X_LINE

NEWSYM _2xSaILine
    SCALE2X_LINE

SECTION .data
NEWSYM colorMask,     dd 0xF7DEF7DE, 0xF7DEF7DE
NEWSYM lowPixelMask,  dd 0x08210821, 0x08210821
NEWSYM qcolorMask,    dd 0xE79CE79C, 0xE79CE79C
NEWSYM qlowpixelMask, dd 0x18631863, 0x18631863
