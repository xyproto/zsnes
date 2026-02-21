; hq3x16 asm entry kept as scalar-only nearest-neighbor path.

%include "macros.mac"

EXTSYM vidbuffer,curblank,GUIOn2,vidbufferofsb,FilteredGUI
EXTSYM resolutn,AddEndBytes,NumBytesPerLine,WinVidMemStart

SECTION .text

NEWSYM hq3x_16b
    cmp byte[curblank],40h
    jne .startcopy
    ret
.startcopy
    pushad
    mov ax,ds
    mov es,ax
    mov esi,[vidbuffer]
    mov edi,[WinVidMemStart]
    add esi,16*2+256*2+32*2
    cmp byte[FilteredGUI],0
    jne .filtergui
    cmp byte[GUIOn2],1
    je  .nointerp
.filtergui
.nointerp
    mov dl,[resolutn]
    mov ebx,[NumBytesPerLine]
.loopy
    mov ecx,256
.loopx
    mov ax,[esi]
    shl eax,16
    mov ax,[esi]
    mov [edi],eax
    mov [edi+ebx],eax
    mov [edi+ebx*2],eax
    mov [edi+4],ax
    mov [edi+ebx+4],ax
    mov [edi+ebx*2+4],ax
    add esi,2
    add edi,6
    dec ecx
    jnz .loopx
    add edi,[AddEndBytes]
    add edi,ebx
    add edi,ebx
    add esi,64
    dec dl
    jnz .loopy
    popad
    ret
