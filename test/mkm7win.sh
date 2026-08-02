#!/bin/sh
# mkm7win.sh - build the asm oracle for difftest_m7win.c.
#
# Extracts the five ProcessMode7ngwin*16b routines as they were before the port
# (from git) into _m7win.o, each renamed asm_*.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- video/mode716.mac); do
        git -C .. cat-file -e "${r}:video/mode716.mac" 2>/dev/null || continue
        if git -C .. show "${r}:video/mode716.mac" | grep -q '^\.rposoffxr'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkm7win.sh: no pre-port revision found" >&2; exit 1; }

git -C .. show "${REV}:video/mode716.mac" > _m7win_src.mac

python3 - _m7win_src.mac > _m7win.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
# One contiguous run: the first NEWSYM of the cluster to the line before the
# ProcessBuildWindow macro that follows it.
out, on = [], False
for l in src:
    if l.startswith('NEWSYM ProcessMode7ngwin16b'):
        on = True
    if on and l.startswith('%macro ProcessBuildWindow'):
        break
    if on:
        out.append(re.sub(r'\bProcessMode7ngwin(\w*)16b\b',
                          r'asm_ProcessMode7ngwin\g<1>16b', l))
text = '\n'.join(out)
for want in ('', 'B', 'C', 'D', 'E'):
    if 'NEWSYM asm_ProcessMode7ngwin%s16b' % want not in text:
        sys.exit('mkm7win.sh: ngwin%s16b not found' % want)
print(text)
PYEOF

cat > _m7win.asm <<'EOF'
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
EXTERN ngcwinptr
EXTERN ngwleft
EXTERN ngwleftb
EXTERN pixelsleft
EXTERN mode7xpos
EXTERN mode7ypos
EXTERN mode7xrpos
EXTERN mode7yrpos
EXTERN mode7xadder
EXTERN mode7yadder
EXTERN m7xaddof
EXTERN m7xaddof2
EXTERN m7yaddof
EXTERN m7yaddof2
EXTERN mmode7ptr
EXTERN mmode7xadd2
EXTERN mmode7yadd2
EXTERN mmode7xinc
EXTERN mmode7xincc
EXTERN mmode7yinc
EXTERN switchtorep3
EXTERN mode7set
EXTERN vram
EXTERN vrama
EXTERN M7WinAX
EXTERN M7WinBX
EXTERN M7WinCX
EXTERN M7WinSI
EXTERN M7WinDI

SECTION .bss
NEWSYM asm_winedx, resd 1

section .text

%include "_m7win.inc"

; void asm_m7win<n>(void) - load the seam into the register ABI, run the
; original, write every register back so the test can compare them.
%imacro WIN_ENTRY 2
NEWSYM asm_m7win%1
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[M7WinAX]
    mov ebx,[M7WinBX]
    mov ecx,[M7WinCX]
    mov esi,[M7WinSI]
    mov edi,[M7WinDI]
    mov edx,0D1D10000h
    call asm_ProcessMode7ngwin%{2}16b
    mov [M7WinAX],eax
    mov [M7WinBX],ebx
    mov [M7WinCX],ecx
    mov [M7WinSI],esi
    mov [M7WinDI],edi
    mov [asm_winedx],edx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
%endmacro

WIN_ENTRY a,
WIN_ENTRY b,B
WIN_ENTRY c,C
WIN_ENTRY d,D
WIN_ENTRY e,E
EOF

# -O0 keeps every jump long: with the cluster lifted out of its original
# context NASM cannot converge on the short forms (label-redef-late).
nasm -Ox -f elf32 -w-orphan-labels -w-pp-macro-params-legacy -o _m7win.o _m7win.asm
echo "wrote _m7win.o (oracle from $(git -C .. rev-parse --short $REV))"
