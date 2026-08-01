#!/bin/sh
# mkm7start.sh - build the asm oracle for difftest_m7start.c.
#
# Extracts Mode7Startup16b and the CLIP / Convert13Bit / Mode7Calculate16b
# macros as they were before the port (from git) into _m7start.o, renamed
# asm_Mode7Startup16b.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- video/mode716.mac); do
        git -C .. cat-file -e "${r}:video/mode716.mac" 2>/dev/null || continue
        if git -C .. show "${r}:video/mode716.mac" | grep -q '^%macro Mode7Calculate16b'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkm7start.sh: no pre-port revision found" >&2; exit 1; }

git -C .. show "${REV}:video/mode716.mac" > _m7start_src.mac

python3 - _m7start_src.mac > _m7start.inc <<'PYEOF'
import sys
src = open(sys.argv[1]).read().split('\n')
# One contiguous run: the CLIP macro through the ret that ends Mode7Startup16b.
out, on = [], False
for l in src:
    if l.startswith('%macro CLIP'):
        on = True
    if on:
        out.append(l.replace('Mode7Startup16b:', 'NEWSYM asm_Mode7Startup16b'))
    if on and out[-1].strip() == 'ret':
        break
text = '\n'.join(out)
if 'NEWSYM asm_Mode7Startup16b' not in text:
    sys.exit('mkm7start.sh: Mode7Startup16b not found')
# A stray `SECTION .text` inside the run would be harmless here, but the .bss
# one that precedes m7starty is not - drop any section switch.
text = '\n'.join(l for l in text.split('\n')
                if not l.strip().lower().startswith('section '))
print(text)
PYEOF

cat > _m7start.asm <<'EOF'
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
EXTERN M7HROn
EXTERN mcxloc
EXTERN mcyloc
EXTERN mmode7xpos
EXTERN mmode7ypos
EXTERN mmode7xadder
EXTERN mmode7yadder
EXTERN mmode7xadd2
EXTERN mmode7yadd2
EXTERN mmode7xinc
EXTERN mmode7xincc
EXTERN mmode7yinc
EXTERN m7starty
EXTERN mode7A
EXTERN mode7B
EXTERN mode7C
EXTERN mode7D
EXTERN mode7X0
EXTERN mode7Y0
EXTERN mode7set
EXTERN curmosaicsz
EXTERN pesimpng
EXTERN xtravbuf
EXTERN M7StartAX
EXTERN M7StartDX
EXTERN M7StartSI
EXTERN M7StartDI

section .text

; void asm_m7start(void) - load the seam into the register ABI, run the
; original, write every register back so the test can compare them.
NEWSYM asm_m7start
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[M7StartAX]
    mov edx,[M7StartDX]
    mov esi,[M7StartSI]
    mov edi,[M7StartDI]
    call asm_Mode7Startup16b
    mov [M7StartAX],eax
    mov [M7StartSI],esi
    mov [M7StartDI],edi
    mov [asm_bx],ebx
    mov [asm_cx],ecx
    mov [asm_dx],edx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

SECTION .bss
NEWSYM asm_bx, resd 1
NEWSYM asm_cx, resd 1
NEWSYM asm_dx, resd 1
SECTION .text

%include "_m7start.inc"
EOF

nasm -O1 -f elf32 -w-orphan-labels -o _m7start.o _m7start.asm
echo "wrote _m7start.o (oracle from $(git -C .. rev-parse --short $REV))"
