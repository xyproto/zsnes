#!/bin/sh
# mkregs.sh - build the asm oracle for difftest_regs.c.
#
# Extracts the register handlers as they were before the port (from git) into
# _regs.o, renaming each to asm_*, together with the checkmultchange macro.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- cpu/regs.inc); do
        git -C .. cat-file -e "$r:cpu/regs.inc" 2>/dev/null || continue
        if git -C .. show "$r:cpu/regs.inc" | grep -q '^NEWSYM reg2134r' \
       && git -C .. show "$r:cpu/regsw.inc" | grep -q '^NEWSYM reg2126w'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkregs.sh: no pre-port revision of cpu/regs.inc found" >&2; exit 1; }

git -C .. show "$REV:cpu/regs.inc" > _regs_src.inc
git -C .. show "$REV:cpu/regsw.inc" >> _regs_src.inc

python3 - _regs_src.inc ../test/regs.list > _regs.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
names = open(sys.argv[2]).read().split()
# A leading '=' marks a bare-label helper that the handlers jump to rather than
# a handler itself: it is carried across under its own name, not renamed, so
# the `je near DetermineHIRQExec` inside $4207/$4208 still resolves.
helpers = {n[1:] for n in names if n.startswith('=')}
want = {n for n in names if not n.startswith('=')}
# Two shapes to get right. Several handlers are stacked aliases sharing one
# body (reg21C2r/reg21C3r, reg420Ar..reg420Fr), and several have more than one
# `ret`. So: a NEWSYM seen before any instruction has been emitted for the
# current handler is an alias - keep going; a NEWSYM seen after one ends the
# handler. Stopping at the first `ret` instead would silently truncate a body
# and let the label fall into the next handler.
out, cur, emitted = [], None, False
for l in src:
    m = re.match(r'NEWSYM (\w+)', l.strip())
    if m:
        if cur and not emitted:
            out.append('NEWSYM asm_' + m.group(1))
            continue
        cur = m.group(1) if m.group(1) in want else None
        emitted = False
        if cur:
            out.append('NEWSYM asm_' + m.group(1))
        continue
    # Not every handler boundary is a NEWSYM. DetermineHIRQExec is a bare
    # unindented label, and swallowing it into the handler above drags in a
    # whole extra body plus its externs. Instructions are always indented in
    # these files, so a *global* label at column 0 ends the handler - but not a
    # local one, which is how the multi-exit bodies mark their branch targets.
    lm = re.match(r'([A-Za-z_]\w*)\s*$', l)
    if lm and not re.match(r'(section |%)', l, re.I):
        if lm.group(1) in helpers:
            out.append(l)
            cur, emitted = lm.group(1), True
        else:
            cur = None
        continue
    if re.match(r'[A-Za-z_]', l) and not re.match(r'(section |%)', l, re.I):
        cur = None
        continue
    # The section directives are written in both cases in the .inc files; a
    # stray `section .data` inside a body silently assembles the rest of the
    # oracle into .data, where it links but faults when called.
    # Keep %if/%else/%endif: dropping the guard but not its body leaves
    # reg2119's debugger-only debstop write unconditional.
    body_drop = (re.match(r'section ', l.strip(), re.I)
                 or (l.strip().startswith('%')
                     and not re.match(r'%(if|else|elif|endif)', l.strip(), re.I)))
    if cur and not body_drop:
        out.append(l)
        if l.strip():
            emitted = True
missing = want - {re.match(r'NEWSYM asm_(\w+)', l.strip()).group(1)
                  for l in out if l.strip().startswith('NEWSYM asm_')}
if missing:
    sys.exit('mkregs.sh: not found: ' + ' '.join(sorted(missing)))
print('\n'.join(out))
PYEOF

# UpdateScrollRegX/Y, Mode7Regs and friends are defined inside the .inc files
# themselves, not the .mac ones, and the handler extractor drops %-lines - so
# lift the macro definitions across as well.
python3 - _regs_src.inc > _regs_inline.mac <<'PYEOF2'
import re, sys
src = open(sys.argv[1]).read().split('\n')
out, on = [], False
for l in src:
    if re.match(r'%macro ', l):
        on = True
    if on:
        out.append(l)
    if on and l.strip() == '%endmacro':
        on = False
print('\n'.join(out))
PYEOF2

git -C .. show "$REV:cpu/regs.mac" > _regs.mac
git -C .. show "$REV:cpu/regsw.mac" >> _regs.mac

cat > _regs.asm <<'EOF'
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
EXTERN vidbright
EXTERN forceblnk
EXTERN multchange
EXTERN compmult
EXTERN mode7A
EXTERN mode7B
EXTERN regptra
EXTERN rtoflags
EXTERN romispal
EXTERN ppustatus
EXTERN cfield
EXTERN extlatch
EXTERN ppu2_mdr
EXTERN latchxr
EXTERN latchyr
EXTERN NMIEnab
EXTERN cpu_mdr
EXTERN curnmi
EXTERN irqon
EXTERN wramrwadr
EXTERN wramdata
EXTERN ioportval
EXTERN divres
EXTERN multres
EXTERN JoyARead
EXTERN JoyBRead
EXTERN JoyCRead2
EXTERN JoyDRead
EXTERN oamram
EXTERN cgram
EXTERN oamaddr
EXTERN cgaddr
EXTERN latchx
EXTERN latchy
EXTERN winl1
EXTERN winr1
EXTERN winl2
EXTERN winr2
EXTERN winlogica
EXTERN winlogicb
EXTERN scrnon
EXTERN winenabm
EXTERN winenabs
EXTERN scaddset
EXTERN scaddtype
EXTERN INTEnab
EXTERN multa
EXTERN diva
EXTERN regptwa
EXTERN bgscrolPrev
EXTERN vramread
EXTERN bg1scrolx
EXTERN bg2scrolx
EXTERN bg3scrolx
EXTERN bg4scrolx
EXTERN bg1scroly
EXTERN bg2scroly
EXTERN bg3scroly
EXTERN bg4scroly
EXTERN bg1scrolx_m7
EXTERN bg1scroly_m7
EXTERN mode7C
EXTERN mode7D
EXTERN mode7X0
EXTERN mode7Y0
EXTERN dmadata
EXTERN hdmarestart
EXTERN nohdmaframe
EXTERN hdmadelay
EXTERN SPC7110Enable
EXTERN resolutn
EXTERN curypos
EXTERN vrama
EXTERN vidmemch2
EXTERN vidmemch4
EXTERN vidmemch8
EXTERN vramincby8left
EXTERN vramincby8totl
EXTERN vramincby8var
EXTERN vramincby8ptri
EXTERN addrincr
EXTERN vramaddr
EXTERN vramread2
EXTERN mode7set
EXTERN vram
EXTERN HIRQLoc
EXTERN VIRQLoc
EXTERN HIRQCycNext
EXTERN HIRQNextExe
EXTERN totlines
EXTERN iohvlatch
EXTERN MultiTapStat
EXTERN cycpl
EXTERN cycphb
EXTERN xirqb
EXTERN cycpblt
EXTERN opexec268
EXTERN opexec268cph
EXTERN opexec358
EXTERN opexec358cph
EXTERN cycpb268
EXTERN cycpb358
EXTERN cgmod
EXTERN winbg1en
EXTERN winbg2en
EXTERN winbg3en
EXTERN winbg4en
EXTERN winobjen
EXTERN wincolen
EXTERN coladdr
EXTERN coladdg
EXTERN coladdb
EXTERN interlval
EXTERN NextLineCache
EXTERN prevoamptr
EXTERN oamlow
EXTERN nexthprior
EXTERN nosprincr
EXTERN objhipr
EXTERN objptr
EXTERN objptrn
EXTERN objsize1
EXTERN objsize2
EXTERN objmovs1
EXTERN objmovs2
EXTERN objadds1
EXTERN objadds2
EXTERN reg2101w_objsize1
EXTERN reg2101w_objsize2
EXTERN reg2101w_objmovs1
EXTERN reg2101w_objmovs2
EXTERN reg2101w_objadds1
EXTERN reg2101w_objadds2
EXTERN oamaddrs
EXTERN poamaddrs
EXTERN bgmode
EXTERN bg3highst
EXTERN bgtilesz
EXTERN mosaicon
EXTERN mosaicsz
EXTERN BG116x16t
EXTERN BG216x16t
EXTERN BG316x16t
EXTERN BG416x16t
EXTERN bg1ptr
EXTERN bg1ptrb
EXTERN bg1ptrc
EXTERN bg1ptrd
EXTERN bg1ptrx
EXTERN bg1ptry
EXTERN bg1scsize
EXTERN bg1objptr
EXTERN bg2ptr
EXTERN bg2ptrb
EXTERN bg2ptrc
EXTERN bg2ptrd
EXTERN bg2ptrx
EXTERN bg2ptry
EXTERN bg2scsize
EXTERN bg2objptr
EXTERN bg3ptr
EXTERN bg3ptrb
EXTERN bg3ptrc
EXTERN bg3ptrd
EXTERN bg3ptrx
EXTERN bg3ptry
EXTERN bg3scsize
EXTERN bg3objptr
EXTERN bg4ptr
EXTERN bg4ptrb
EXTERN bg4ptrc
EXTERN bg4ptrd
EXTERN bg4ptrx
EXTERN bg4ptry
EXTERN bg4scsize
EXTERN bg4objptr

%include "_regs.mac"
%include "_regs_inline.mac"

section .text
%include "_regs.inc"
EOF

# Match the build: reg2119's debugger branch references debstop otherwise.
nasm -O1 -f elf32 -DNO_DEBUGGER -w-orphan-labels -o _regs.o _regs.asm
echo "wrote _regs.o (oracle from $(git -C .. rev-parse --short $REV), $(grep -c '^NEWSYM asm_' _regs.inc) handlers)"
