; hq4x32 asm entry kept as scalar-only nearest-neighbor path.

%include "macros.mac"

EXTSYM vidbuffer,curblank,GUIOn2,vidbufferofsb,FilteredGUI
EXTSYM resolutn,lineleft,AddEndBytes,NumBytesPerLine,WinVidMemStart,BitConv32Ptr

SECTION .text

NEWSYM hq4x_32b
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
    mov [lineleft],dl
    mov ebx,[NumBytesPerLine]
.loopy
    mov ecx,256
.loopx
    movzx eax,word[esi]
    mov edx,[BitConv32Ptr]
    mov eax,[edx+eax*4]
    mov [edi],eax
    mov [edi+4],eax
    mov [edi+8],eax
    mov [edi+12],eax
    mov [edi+ebx],eax
    mov [edi+ebx+4],eax
    mov [edi+ebx+8],eax
    mov [edi+ebx+12],eax
    mov edx,edi
    add edx,ebx
    add edx,ebx
    mov [edx],eax
    mov [edx+4],eax
    mov [edx+8],eax
    mov [edx+12],eax
    mov [edx+ebx],eax
    mov [edx+ebx+4],eax
    mov [edx+ebx+8],eax
    mov [edx+ebx+12],eax
    add esi,2
    add edi,16
    dec ecx
    jnz .loopx
    add edi,[AddEndBytes]
    add edi,ebx
    add edi,ebx
    add edi,ebx
    add esi,64
    dec byte[lineleft]
    jnz .loopy
    popad
    ret
