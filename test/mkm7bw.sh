#!/bin/sh
# mkm7bw.sh - build the asm oracle for difftest_m7bw.c.
#
# Extracts the ProcessBuildWindow macro as it was before the port (from git),
# instantiates it once with 0 and wraps it as asm_m7bw.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(./asmgit.sh log --format=%H -- video/mode716.mac); do
        ./asmgit.sh cat-file -e "${r}:video/mode716.mac" 2>/dev/null || continue
        if ./asmgit.sh show "${r}:video/mode716.mac" | grep -q '^    call BuildWindow'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkm7bw.sh: no pre-port revision found" >&2; exit 1; }

./asmgit.sh show "${REV}:video/mode716.mac" > _m7bw_src.mac

python3 - _m7bw_src.mac > _m7bw.inc <<'PYEOF'
import sys
src = open(sys.argv[1]).read().split('\n')
out, on = [], False
for l in src:
    if l.startswith('%macro ProcessBuildWindow'):
        on = True
    if on:
        out.append(l)
    if on and l.strip() == '%endmacro':
        break
text = '\n'.join(out)
if '%macro ProcessBuildWindow' not in text:
    sys.exit('mkm7bw.sh: ProcessBuildWindow not found')
print(text)
PYEOF

cat > _m7bw.asm <<'EOF'
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
EXTERN ngwinen
EXTERN nglogicval
EXTERN ngwintable
EXTERN ngcwinptr
EXTERN winlogicaval
EXTERN BuildWindow
EXTERN M7BWBX

SECTION .bss
NEWSYM M7BWAX, resd 1
NEWSYM M7BWSI, resd 1
SECTION .text

%include "_m7bw.inc"

; void asm_m7bw(void) - eax is the register the broken call site leaks into
; BuildWindow's second argument, so the test gets to choose it.
NEWSYM asm_m7bw
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[M7BWAX]
    mov ebx,[M7BWBX]
    mov esi,[M7BWSI]
    ProcessBuildWindow 0
    mov [M7BWBX],ebx
    mov [M7BWSI],esi
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
EOF

nasm -Ox -f elf32 -w-orphan-labels -o _m7bw.o _m7bw.asm
echo "wrote _m7bw.o (oracle from $(./asmgit.sh rev-parse --short $REV))"
