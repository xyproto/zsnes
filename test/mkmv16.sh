#!/bin/sh
# mkmv16.sh - build the asm oracle for difftest_mv16.c.
#
# Extracts the shared draw*ms prologue as it was before the port (from git).
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(./asmgit.sh log --format=%H -- video/mv16tms.asm); do
        ./asmgit.sh cat-file -e "${r}:video/mv16tms.asm" 2>/dev/null || continue
        if ./asmgit.sh show "${r}:video/mv16tms.asm" | grep -q '^    mov \[bshifter\],ah'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkmv16.sh: no pre-port revision found" >&2; exit 1; }

./asmgit.sh show "${REV}:video/mv16tms.asm" > _mv16_src.asm

python3 - _mv16_src.asm > _mv16.inc <<'PYEOF'
import sys
src = open(sys.argv[1]).read().split('\n')
out, on = [], False
for l in src:
    if l.strip() == 'mov [temp],al':
        on = True
        out.append('NEWSYM asm_draw16tms_setup')
    if on:
        out.append(l)
    if on and l.strip() == 'pop ecx':
        out.append('    ret')
        break
text = '\n'.join(out)
if 'NEWSYM asm_draw16tms_setup' not in text:
    sys.exit('mkmv16.sh: prologue not found')
print(text)
PYEOF

cat > _mv16.asm <<'EOF'
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
EXTERN temp
EXTERN bshifter
EXTERN yadder
EXTERN yrevadder
EXTERN tempcach
EXTERN temptile
EXTERN bgsubby
EXTERN bgofwptr
EXTERN cwinptr
EXTERN winptrref
EXTERN curvidoffset
EXTERN curmosaicsz
EXTERN xtravbuf
EXTERN vcache2b
EXTERN vcache4b
EXTERN vcache8b
EXTERN MVAX
EXTERN MVBX
EXTERN MVCX
EXTERN MVDX
EXTERN MVSI

SECTION .bss
NEWSYM asm_mvdi, resd 1
NEWSYM asm_mvbp, resd 1
SECTION .text

; void asm_mv16(void) - the seam, plus edi/ebp so the test can check the
; prologue leaves them alone.
NEWSYM asm_mv16
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[MVAX]
    mov ebx,[MVBX]
    mov ecx,[MVCX]
    mov edx,[MVDX]
    mov esi,[MVSI]
    mov edi,0D1D1D1D1h
    mov ebp,0B9B9B9B9h
    call asm_draw16tms_setup
    mov [MVAX],eax
    mov [MVBX],ebx
    mov [MVCX],ecx
    mov [MVDX],edx
    mov [MVSI],esi
    mov [asm_mvdi],edi
    mov [asm_mvbp],ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

%include "_mv16.inc"
EOF

nasm -Ox -f elf32 -w-orphan-labels -o _mv16.o _mv16.asm
echo "wrote _mv16.o (oracle from $(./asmgit.sh rev-parse --short $REV))"
