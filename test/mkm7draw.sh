#!/bin/sh
# mkm7draw.sh - build the asm oracle for difftest_m7draw.c.
#
# Takes the pre-port drawmode7win16b / drawmode7ngextbg16b and everything they
# expand (the four Mode7*Sub wrappers, the four Mode7Process* macros and the
# pixel writers) straight out of git, renamed asm_*.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(./asmgit.sh log --format=%H -- video/mode716.asm); do
        ./asmgit.sh cat-file -e "${r}:video/mode716.asm" 2>/dev/null || continue
        if ./asmgit.sh show "${r}:video/mode716.asm" | grep -q '^    Mode7NonMainSub Mode7Normal$'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkm7draw.sh: no pre-port revision found" >&2; exit 1; }

./asmgit.sh show "${REV}:video/mode716.asm" > _m7draw_src.asm
./asmgit.sh show "${REV}:video/mode716.mac" > _m7draw_src.mac

python3 - _m7draw_src.asm _m7draw_src.mac > _m7draw.inc <<'PYEOF'
import re, sys

# The .mac verbatim, minus its EXTSYM lines (the wrapper declares what it needs)
# and minus the Mode7Startup16b thunk, which is already C and is linked in.
mac = open(sys.argv[2]).read().split('\n')
out = []
for l in mac:
    if l.startswith('EXTSYM ') or l.startswith('%include'):
        continue
    out.append(l)

# From the .asm: the pixel writers through the last Mode7MainSube. The thunks
# after that call C the test does not exercise.
asm = open(sys.argv[1]).read().split('\n')
on = False
for l in asm:
    if l.startswith('%macro Mode7Normal') or l.startswith('%macro Mode7ExtBG'):
        on = True
    if on:
        out.append(l)
    if l.startswith('    Mode7MainSube Mode7ExtBGmsnt,Mode7ExtBGnt'):
        break
text = '\n'.join(out)
for want in ('NEWSYM drawmode7win16b', 'NEWSYM drawmode7ngextbg16b',
             '%macro Mode7NonMainSub 1', '%macro Mode7Process 3'):
    if want not in text:
        sys.exit('mkm7draw.sh: %s not found' % want)
text = text.replace('NEWSYM drawmode7win16b', 'NEWSYM asm_drawmode7win16b')
text = text.replace('NEWSYM drawmode7ngextbg16b', 'NEWSYM asm_drawmode7ngextbg16b')
print(text)
PYEOF

cat > _m7draw.asm <<'EOF'
bits 32
section .note.GNU-stack noalloc noexec nowrite progbits
%define ELF 1
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
; Defined near the top of mode716.asm, above the part the extractor takes.
%macro ALIGN32 0
  times ($$-$) & 1Fh nop
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
EXTERN mode7set
EXTERN mode7tab
EXTERN mode7hr
EXTERN vram
EXTERN vrama
EXTERN curmosaicsz
EXTERN curvidoffset
EXTERN cwinptr
EXTERN winptrref
EXTERN UnusedBit
EXTERN UnusedBitXor
EXTERN scrndis
EXTERN BGMS1
EXTERN FillSubScr
EXTERN scadtng
EXTERN CMainWinScr
EXTERN CSubWinScr
EXTERN winbg1enval
EXTERN winlogicaval
EXTERN nglogicval
EXTERN ngwinen
EXTERN ngwintable
EXTERN ngcwinptr
EXTERN ngwleft
EXTERN ngwleftb
EXTERN pixelsleft
EXTERN switchtorep3
EXTERN BuildWindow
EXTERN M7StartAX
EXTERN M7StartDX
EXTERN M7StartSI
EXTERN M7StartDI
EXTERN c_Mode7Startup16b

SECTION .bss
NEWSYM asm_mosaic, resd 1
NEWSYM asm_regs, resd 7
SECTION .text

; The mosaic tail is a tail-jump, so stub it as a recorder that returns to
; whoever called the draw routine.
NEWSYM domosaicng16b
    mov dword[asm_mosaic],1
    ret

%include "_m7draw.inc"

; void asm_m7draw<n>(void) - 0 = drawmode7win16b, 1 = drawmode7ngextbg16b.
%imacro DRAW_ENTRY 2
NEWSYM asm_m7draw%1
    push ebx
    push esi
    push edi
    push ebp
    mov dword[asm_mosaic],0
    mov eax,[asm_regs]
    mov ebx,[asm_regs+4]
    mov ecx,[asm_regs+8]
    mov edx,[asm_regs+12]
    mov esi,[asm_regs+16]
    mov edi,[asm_regs+20]
    mov ebp,[asm_regs+24]
    call asm_%2
    mov [asm_regs],eax
    mov [asm_regs+4],ebx
    mov [asm_regs+8],ecx
    mov [asm_regs+12],edx
    mov [asm_regs+16],esi
    mov [asm_regs+20],edi
    mov [asm_regs+24],ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
%endmacro

DRAW_ENTRY 0, drawmode7win16b
DRAW_ENTRY 1, drawmode7ngextbg16b
EOF

nasm -Ox -f elf32 -w-orphan-labels -o _m7draw.o _m7draw.asm
echo "wrote _m7draw.o (oracle from $(./asmgit.sh rev-parse --short $REV))"
