#!/bin/sh
# mkmvs.sh - build the asm oracle for difftest_mvs.c (draw8x816tsms).
set -e
REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- video/mv16tms.asm); do
        git -C .. cat-file -e "${r}:video/mv16tms.asm" 2>/dev/null || continue
        if git -C .. show "${r}:video/mv16tms.asm" | grep -q '^    drawtilegrpfull draw8x816tcms'; then
            REV=$r; break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkmvs.sh: no pre-port revision found" >&2; exit 1; }

git -C .. show "${REV}:video/mv16tms.asm" > _mvs_src.asm
git -C .. show "${REV}:video/vidmacro.mac" > _mvs_src.mac

python3 - _mvs_src.asm _mvs_src.mac > _mvs.inc <<'PYEOF'
import sys
out = []
# The two group macros out of vidmacro.mac.
mac = open(sys.argv[2]).read().split('\n')
on = 0
for l in mac:
    if l.startswith('%macro drawtilegrpfull ') or l.startswith('%macro drawtilegrpfullf '):
        on = 1
    if on:
        out.append(l)
    if on and l.strip() == '%endmacro':
        on = 0
# The writer macro and the routine out of mv16tms.asm.
asm = open(sys.argv[1]).read().split('\n')
on = 0
for l in asm:
    if (l.startswith('%macro draw8x816tcms')
            or l.startswith('%macro draw8x816tcwinonms')
            or l.startswith('%macro draw8x816tcwinonbms')):
        on = 1
    if on == 1:
        out.append(l)
    if on == 1 and l.strip() == '%endmacro':
        on = 0
    if l.startswith('NEWSYM draw8x816tsms'):
        on = 2
        out.append('NEWSYM asm_draw8x816tsms')
        continue
    if l.startswith('NEWSYM draw8x816tswinonms'):
        on = 3
        out.append('NEWSYM asm_draw8x816tswinonms')
        continue
    if on == 2:
        if l.startswith('NEWSYM draw8x816twinonms'):
            on = 0
        else:
            out.append(l)
    elif on == 3:
        if l.startswith(';****'):
            break
        out.append(l)
text = '\n'.join(out)
for want in ('%macro drawtilegrpfull ', '%macro draw8x816tcms',
             '%macro draw8x816tcwinonms', '%macro draw8x816tcwinonbms',
             'NEWSYM asm_draw8x816tsms', 'NEWSYM asm_draw8x816tswinonms'):
    if want not in text:
        sys.exit('mkmvs.sh: %s not found' % want)
print(text)
PYEOF

cat > _mvs.asm <<'EOF'
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
EXTERN tileleft16b
EXTERN drawn
EXTERN temp
EXTERN bshifter
EXTERN curbgpr
EXTERN bgcoloradder
EXTERN tempcach
EXTERN temptile
EXTERN bgofwptr
EXTERN bgsubby
EXTERN yadder
EXTERN yrevadder
EXTERN pal16bxcl
EXTERN coadder16
EXTERN winptrref
EXTERN fulladdtab
EXTERN MVSAX
EXTERN MVSBX
EXTERN MVSCX
EXTERN MVSDX
EXTERN MVSSI
EXTERN MVSDI
EXTERN MVSBP

section .text

%imacro MVS_ENTRY 2
NEWSYM asm_mvs%1
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[MVSAX]
    mov ebx,[MVSBX]
    mov ecx,[MVSCX]
    mov edx,[MVSDX]
    mov esi,[MVSSI]
    mov edi,[MVSDI]
    mov ebp,[MVSBP]
    call asm_%2
    mov [MVSAX],eax
    mov [MVSBX],ebx
    mov [MVSCX],ecx
    mov [MVSDX],edx
    mov [MVSSI],esi
    mov [MVSDI],edi
    mov [MVSBP],ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
%endmacro

MVS_ENTRY 0, draw8x816tsms
MVS_ENTRY 1, draw8x816tswinonms

%include "_mvs.inc"
EOF
nasm -Ox -f elf32 -w-orphan-labels -o _mvs.o _mvs.asm
echo "wrote _mvs.o (oracle from $(git -C .. rev-parse --short $REV))"
