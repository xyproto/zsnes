#!/bin/sh
# mkm7proc.sh - build the asm oracle for difftest_m7proc.c.
#
# Extracts the Mode7Process macro (video/mode716.mac) and the pixel writers it
# inlines (video/mode716.asm) as they were before the port, then instantiates
# one entry point per writer.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(./asmgit.sh log --format=%H -- video/mode716.mac); do
        ./asmgit.sh cat-file -e "${r}:video/mode716.mac" 2>/dev/null || continue
        # The macro *names* survive the port as thunks, so key off a line only
        # the original bodies have - otherwise the oracle becomes the port.
        if ./asmgit.sh show "${r}:video/mode716.mac" | grep -q '^    mov dword\[mtemp\],256'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkm7proc.sh: no pre-port revision found" >&2; exit 1; }

./asmgit.sh show "${REV}:video/mode716.mac" > _m7proc_src.mac
./asmgit.sh show "${REV}:video/mode716.asm" > _m7proc_src.asm

python3 - _m7proc_src.mac _m7proc_src.asm > _m7proc.inc <<'PYEOF'
import sys
out = []

src = open(sys.argv[1]).read().split('\n')
on = False
for l in src:
    if (l.startswith('%macro Mode7Process 3')
            or l.startswith('%macro Mode7ProcessB 3')
            or l.startswith('%macro Mode7Processngw16b 3')
            or l.startswith('%macro Mode7Processngw216b 4')):
        on = True
    if on:
        out.append(l)
    if on and l.strip() == '%endmacro':
        on = False
for want in ('%macro Mode7Process 3', '%macro Mode7ProcessB 3',
             '%macro Mode7Processngw16b 3', '%macro Mode7Processngw216b 4'):
    if want not in '\n'.join(out):
        sys.exit('mkm7proc.sh: %s not found' % want)

# The writers live in the .asm; take every Mode7Normal*/Mode7ExtBG* macro.
asm = open(sys.argv[2]).read().split('\n')
on, n = False, 0
for l in asm:
    if l.startswith('%macro Mode7Normal') or l.startswith('%macro Mode7ExtBG'):
        on = True
        n += 1
    if on:
        out.append(l)
    if on and l.strip() == '%endmacro':
        on = False
if n < 14:
    sys.exit('mkm7proc.sh: expected 14 writers, found %d' % n)
print('\n'.join(out))
PYEOF

cat > _m7proc.asm <<'EOF'
bits 32
section .note.GNU-stack noalloc noexec nowrite progbits
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
%macro ALIGN16 0
  times ($$-$) & 1Fh nop
%endmacro
EXTERN mtemp
EXTERN mmode7xpos
EXTERN mmode7ypos
EXTERN mmode7xrpos
EXTERN mmode7yrpos
EXTERN mmode7xadder
EXTERN mmode7yadder
EXTERN mmode7xadd2
EXTERN mmode7yadd2
EXTERN mmode7ptr
EXTERN mmode7xinc
EXTERN mmode7xincc
EXTERN mmode7yinc
EXTERN mode7set
EXTERN mode7tab
EXTERN vram
EXTERN vrama
EXTERN curmosaicsz
EXTERN UnusedBit
EXTERN UnusedBitXor
EXTERN M7PAX
EXTERN M7PBX
EXTERN M7PCX
EXTERN M7PDX
EXTERN M7PSI
EXTERN M7PDI
EXTERN M7PBP
EXTERN ngcwinptr
EXTERN mm7xaddof
EXTERN mm7xaddof2
EXTERN mm7yaddof
EXTERN mm7yaddof2
EXTERN m7xaddof
EXTERN m7xaddof2
EXTERN m7yaddof
EXTERN m7yaddof2
EXTERN mode7xpos
EXTERN mode7ypos
EXTERN mode7xrpos
EXTERN mode7yrpos
EXTERN mode7xadder
EXTERN mode7yadder
EXTERN ngwleft
EXTERN ngwleftb
EXTERN switchtorep3
EXTERN c_ProcessMode7ngwin16b
EXTERN c_ProcessMode7ngwinB16b
EXTERN c_ProcessMode7ngwinC16b
EXTERN c_ProcessMode7ngwinD16b
EXTERN c_ProcessMode7ngwinE16b

EXTERN M7WinAX
EXTERN M7WinBX
EXTERN M7WinCX
EXTERN M7WinSI
EXTERN M7WinDI

SECTION .bss
NEWSYM asm_mosaic, resd 1
SECTION .text

; The ngwin cluster is already C and verified by difftest_m7win; both sides
; reach it through the same thunks the real build uses.
NEWSYM ProcessMode7ngwin16b
    push edx
    mov [M7WinAX], eax
    mov [M7WinBX], ebx
    mov [M7WinCX], ecx
    mov [M7WinSI], esi
    mov [M7WinDI], edi
    call c_ProcessMode7ngwin16b
    mov eax, [M7WinAX]
    mov ebx, [M7WinBX]
    mov ecx, [M7WinCX]
    mov esi, [M7WinSI]
    mov edi, [M7WinDI]
    pop edx
    ret

NEWSYM ProcessMode7ngwinB16b
    push edx
    mov [M7WinAX], eax
    mov [M7WinBX], ebx
    mov [M7WinCX], ecx
    mov [M7WinSI], esi
    mov [M7WinDI], edi
    call c_ProcessMode7ngwinB16b
    mov eax, [M7WinAX]
    mov ebx, [M7WinBX]
    mov ecx, [M7WinCX]
    mov esi, [M7WinSI]
    mov edi, [M7WinDI]
    pop edx
    ret

NEWSYM ProcessMode7ngwinC16b
    push edx
    mov [M7WinAX], eax
    mov [M7WinBX], ebx
    mov [M7WinCX], ecx
    mov [M7WinSI], esi
    mov [M7WinDI], edi
    call c_ProcessMode7ngwinC16b
    mov eax, [M7WinAX]
    mov ebx, [M7WinBX]
    mov ecx, [M7WinCX]
    mov esi, [M7WinSI]
    mov edi, [M7WinDI]
    pop edx
    ret

NEWSYM ProcessMode7ngwinD16b
    push edx
    mov [M7WinAX], eax
    mov [M7WinBX], ebx
    mov [M7WinCX], ecx
    mov [M7WinSI], esi
    mov [M7WinDI], edi
    call c_ProcessMode7ngwinD16b
    mov eax, [M7WinAX]
    mov ebx, [M7WinBX]
    mov ecx, [M7WinCX]
    mov esi, [M7WinSI]
    mov edi, [M7WinDI]
    pop edx
    ret

NEWSYM ProcessMode7ngwinE16b
    push edx
    mov [M7WinAX], eax
    mov [M7WinBX], ebx
    mov [M7WinCX], ecx
    mov [M7WinSI], esi
    mov [M7WinDI], edi
    call c_ProcessMode7ngwinE16b
    mov eax, [M7WinAX]
    mov ebx, [M7WinBX]
    mov ecx, [M7WinCX]
    mov esi, [M7WinSI]
    mov edi, [M7WinDI]
    pop edx
    ret



%include "_m7proc.inc"

; One entry per writer. Mode7Process either rets or tail-jumps to its second
; argument, so the body is reached by call and both exits land back here.
%imacro PROC_ENTRY 3
NEWSYM asm_m7proc%1
    push ebx
    push esi
    push edi
    push ebp
    mov dword[asm_mosaic],0
    mov eax,[M7PAX]
    mov ebx,[M7PBX]
    mov ecx,[M7PCX]
    mov edx,[M7PDX]
    mov esi,[M7PSI]
    mov edi,[M7PDI]
    mov ebp,[M7PBP]
    call %%body
    jmp %%save
%%body
    %3 %2, %%mosaic, 2
%%mosaic
    mov dword[asm_mosaic],1
    ret
%%save
    mov [M7PAX],eax
    mov [M7PBX],ebx
    mov [M7PCX],ecx
    mov [M7PDX],edx
    mov [M7PSI],esi
    mov [M7PDI],edi
    mov [M7PBP],ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
%endmacro

PROC_ENTRY 0, Mode7Normal, Mode7Process
PROC_ENTRY 1, Mode7Normalt, Mode7Process
PROC_ENTRY 2, Mode7Normalmsnt, Mode7Process
PROC_ENTRY 3, Mode7Normalmst, Mode7Process
PROC_ENTRY 4, Mode7Normalsnt, Mode7Process
PROC_ENTRY 5, Mode7ExtBG, Mode7Process
PROC_ENTRY 6, Mode7ExtBGt, Mode7Process
PROC_ENTRY 7, Mode7ExtBGmsnt, Mode7Process
PROC_ENTRY 8, Mode7ExtBGmst, Mode7Process
PROC_ENTRY 9, Mode7ExtBGsnt, Mode7Process
PROC_ENTRY 10, Mode7Normal, Mode7ProcessB
PROC_ENTRY 11, Mode7Normalt, Mode7ProcessB
PROC_ENTRY 12, Mode7Normalmsnt, Mode7ProcessB
PROC_ENTRY 13, Mode7Normalmst, Mode7ProcessB
PROC_ENTRY 14, Mode7Normalsnt, Mode7ProcessB
PROC_ENTRY 15, Mode7ExtBG, Mode7ProcessB
PROC_ENTRY 16, Mode7ExtBGt, Mode7ProcessB
PROC_ENTRY 17, Mode7ExtBGmsnt, Mode7ProcessB
PROC_ENTRY 18, Mode7ExtBGmst, Mode7ProcessB
PROC_ENTRY 19, Mode7ExtBGsnt, Mode7ProcessB
PROC_ENTRY 20, Mode7Normal, Mode7Processngw16b
PROC_ENTRY 21, Mode7Normalt, Mode7Processngw16b
PROC_ENTRY 22, Mode7Normalmsnt, Mode7Processngw16b
PROC_ENTRY 23, Mode7Normalmst, Mode7Processngw16b
PROC_ENTRY 24, Mode7Normalsnt, Mode7Processngw16b
PROC_ENTRY 25, Mode7ExtBG, Mode7Processngw16b
PROC_ENTRY 26, Mode7ExtBGt, Mode7Processngw16b
PROC_ENTRY 27, Mode7ExtBGmsnt, Mode7Processngw16b
PROC_ENTRY 28, Mode7ExtBGmst, Mode7Processngw16b
PROC_ENTRY 29, Mode7ExtBGsnt, Mode7Processngw16b

%imacro PROC_ENTRY2 3
NEWSYM asm_m7proc%1
    push ebx
    push esi
    push edi
    push ebp
    mov dword[asm_mosaic],0
    mov eax,[M7PAX]
    mov ebx,[M7PBX]
    mov ecx,[M7PCX]
    mov edx,[M7PDX]
    mov esi,[M7PSI]
    mov edi,[M7PDI]
    mov ebp,[M7PBP]
    call %%body
    jmp %%save
%%body
    Mode7Processngw216b %2, %%mosaic, 2, %3
%%mosaic
    mov dword[asm_mosaic],1
    ret
%%save
    mov [M7PAX],eax
    mov [M7PBX],ebx
    mov [M7PCX],ecx
    mov [M7PDX],edx
    mov [M7PSI],esi
    mov [M7PDI],edi
    mov [M7PBP],ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
%endmacro

PROC_ENTRY2 30, Mode7Normal, Mode7Normalt
PROC_ENTRY2 31, Mode7Normalt, Mode7Normalmsnt
PROC_ENTRY2 32, Mode7Normalmsnt, Mode7Normalmst
PROC_ENTRY2 33, Mode7Normalmst, Mode7Normalsnt
PROC_ENTRY2 34, Mode7Normalsnt, Mode7Normal
PROC_ENTRY2 35, Mode7ExtBG, Mode7ExtBGt
PROC_ENTRY2 36, Mode7ExtBGt, Mode7ExtBGmsnt
PROC_ENTRY2 37, Mode7ExtBGmsnt, Mode7ExtBGmst
PROC_ENTRY2 38, Mode7ExtBGmst, Mode7ExtBGsnt
PROC_ENTRY2 39, Mode7ExtBGsnt, Mode7ExtBG
EOF

nasm -Ox -f elf32 -w-orphan-labels -o _m7proc.o _m7proc.asm
echo "wrote _m7proc.o (oracle from $(./asmgit.sh rev-parse --short $REV))"
