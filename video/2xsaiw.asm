; Portable scalar 2xSaI line filters.

%include "macros.mac"

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
