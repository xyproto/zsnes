#!/bin/sh
# mkm7ext2.sh - build the asm oracle for difftest_m7ext2.c.
#
# Extracts drawmode7ngextbg216b and the ExtBG2/ExtBGNormal* macros as they were
# before the port (from git) into _m7ext2.o, renamed asm_drawmode7ngextbg216b.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(./asmgit.sh log --format=%H -- video/mode716.asm); do
        ./asmgit.sh cat-file -e "${r}:video/mode716.asm" 2>/dev/null || continue
        if ./asmgit.sh show "${r}:video/mode716.asm" | grep -q '^%macro ExtBG2'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkm7ext2.sh: no pre-port revision found" >&2; exit 1; }

./asmgit.sh show "${REV}:video/mode716.asm" > _m7ext2_src.asm
./asmgit.sh show "${REV}:video/mode716.mac" > _m7ext2_src.mac

python3 - _m7ext2_src.asm > _m7ext2.inc <<'PYEOF'
import sys
src = open(sys.argv[1]).read().split('\n')
# The routine and its macros are one contiguous run: from the ExtBG2 macro to
# the ALIGN32 that closes the file's last renderer.
out, on = [], False
for l in src:
    if l.startswith('%macro ExtBG2'):
        on = True
    if on and l.strip() == 'ALIGN32':
        break
    if on:
        out.append(l.replace('NEWSYM drawmode7ngextbg216b',
                             'NEWSYM asm_drawmode7ngextbg216b'))
text = '\n'.join(out)
if 'NEWSYM asm_drawmode7ngextbg216b' not in text:
    sys.exit('mkm7ext2.sh: drawmode7ngextbg216b not found')
print(text)
PYEOF

# Only CheckTransparency is needed out of mode716.mac; pulling the whole file in
# would drag the renderer macros and their externs along with it.
python3 - _m7ext2_src.mac > _m7ext2.mac <<'PYEOF2'
import sys
src = open(sys.argv[1]).read().split('\n')
out, on = [], False
for l in src:
    if l.startswith('%macro CheckTransparency'):
        on = True
    if on:
        out.append(l)
    if on and l.strip() == '%endmacro':
        break
if not out:
    sys.exit('mkm7ext2.sh: CheckTransparency not found')
print('\n'.join(out))
PYEOF2

cat > _m7ext2.asm <<'EOF'
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
EXTERN scrndis
EXTERN mode7hr
EXTERN BGMS1
EXTERN FillSubScr
EXTERN scadtng
EXTERN curvidoffset
EXTERN UnusedBitXor
EXTERN M7SeamA
EXTERN M7SeamB
EXTERN M7SeamC
EXTERN M7SeamD
EXTERN M7SeamSI
EXTERN M7SeamBP

section .text

%include "_m7ext2.mac"

; void asm_m7ext2(void) - load the seam into the register ABI, run the
; original, write the registers back so the test can compare them.
NEWSYM asm_m7ext2
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[M7SeamA]
    mov ebx,[M7SeamB]
    mov ecx,[M7SeamC]
    mov edx,[M7SeamD]
    mov esi,[M7SeamSI]
    mov ebp,[M7SeamBP]
    call asm_drawmode7ngextbg216b
    mov [M7SeamA],eax
    mov [M7SeamB],ebx
    mov [M7SeamC],ecx
    mov [M7SeamD],edx
    mov [M7SeamSI],esi
    mov [M7SeamBP],ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

%include "_m7ext2.inc"
EOF

nasm -O1 -f elf32 -w-orphan-labels -o _m7ext2.o _m7ext2.asm
echo "wrote _m7ext2.o (oracle from $(./asmgit.sh rev-parse --short $REV))"
