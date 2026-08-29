#!/bin/sh
# mkfxops.sh - build the asm oracle for difftest_fxops.c.
#
# Extracts all 92 SuperFX opcode handlers as they were before the port (from
# git) into _fxops.o, renaming each FxOpXX to asm_FxOpXX, and adds the helpers
# the difftest needs to talk to the core's register ABI:
#
#   asm_fxcall(fn)  load the ABI registers from the seam block, call fn, write
#                   them back  (drives the *asm* handler)
#   fxstub/b/c      stand in for the next opcode: record what they were
#                   dispatched with, then return. One per table, so the test can
#                   tell which table a handler chained through. SfxB is captured
#                   too: WITH sets it purely so the *nested* opcode sees it.
#                   These read the register ABI, so the difftest keeps a C
#                   twin of each for the ported side.
set -e

# Default to the newest revision whose fxemu2b.asm predates the port, i.e. the
# last one where no handler body is an `fxcop` thunk yet. The files are deleted
# in current revisions, so skip any where they are absent - a failed lookup
# is empty, which would otherwise look like a clean pre-port revision.
REV=$1
if [ -z "$REV" ]; then
    for r in $(./asmgit.sh log --format=%H -- chips/fxemu2b.asm); do
        ./asmgit.sh cat-file -e "$r:chips/fxemu2b.asm" 2>/dev/null || continue
        ./asmgit.sh cat-file -e "$r:chips/fxemu2.asm" 2>/dev/null || continue
        if ! ./asmgit.sh show "$r:chips/fxemu2b.asm" | grep -q fxcop \
            && ! ./asmgit.sh show "$r:chips/fxemu2.asm" | grep -q fxcop; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkfxops.sh: no pre-port revision of chips/fxemu2b.asm found" >&2; exit 1; }

SRC=_fxops_src.asm
./asmgit.sh show "$REV:chips/fxemu2b.asm" > "$SRC"

# The handlers start at the first opcode entry point and run to end of file.
FIRST=$(grep -n '^NEWSYM FxOp' "$SRC" | head -1 | cut -d: -f1)
sed -n "${FIRST},\$p" "$SRC" > _fxops.inc

# chips/fxemu2.asm holds the base-table handlers, mixed in with data and with
# routines the difftest has no business linking. Pull out only the opcode
# bodies named on stdin (one per line), in file order.
./asmgit.sh show "$REV:chips/fxemu2.asm" > _fxops_base.asm
python3 - _fxops_base.asm ../test/fxops_base.list >> _fxops.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
want = set(open(sys.argv[2]).read().split())
out = []
cur = None
for i, l in enumerate(src):
    m = re.match(r'NEWSYM (\w+)', l.strip())
    if m:
        cur = m.group(1) if m.group(1) in want else None
        if cur:
            out.append(l)
        continue
    if cur and not re.match(r'(SECTION |%)', l.strip()):
        out.append(l)
    elif cur:
        cur = None
missing = want - {re.match(r'NEWSYM (\w+)', l.strip()).group(1)
                  for l in out if l.strip().startswith('NEWSYM ')}
if missing:
    sys.exit('mkfxops.sh: not found in fxemu2.asm: ' + ' '.join(sorted(missing)))
print('\n'.join(out))
PYEOF

# The d table (chips/fxemu2c.asm) threads rather than calls: each handler
# tail-jumps to the next through FXReturn. Extract those handlers too, and emit
# a matching thunk per handler so the C side runs the real seam and tail-chain
# rather than the bare C body.
./asmgit.sh show "$REV:chips/fxemu2c.asm" > _fxops_d.asm
python3 - _fxops_d.asm >> _fxops.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
out = []
cur = None
for l in src:
    m = re.match(r'NEWSYM (FxOpd[A-Za-z0-9]+)', l.strip())
    if m:
        cur = m.group(1)
        out.append(l)
        continue
    if cur and not re.match(r'(SECTION |%)', l.strip()):
        out.append(l)
    elif cur:
        cur = None
print('\n'.join(out))
PYEOF

# MainLoop and its epilogue, for the loop test. Renamed so the d handlers'
# FXReturn keeps reaching the difftest's own FXEndLoop stub.
python3 - _fxops_base.asm >> _fxops.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
out = []
cur = False
for l in src:
    m = re.match(r'NEWSYM (\w+)', l.strip())
    if m:
        cur = m.group(1) in ('MainLoop', 'FXEndLoop')
        if cur:
            out.append(l)
        continue
    if cur and not re.match(r'(SECTION |%)', l.strip()):
        out.append(l)
    elif cur:
        cur = False
out = [re.sub(r'\b(MainLoop|FXEndLoop)\b', r'asm_\1', l) for l in out]
print('\n'.join(out))
PYEOF

# Rename every entry point so the oracle does not clash with the real symbols,
# including handler-to-handler calls and jumps (LJMP calls the CACHE opcode,
# PLOT jumps to its 4bpp variant) so the
# oracle keeps calling its own pre-port copy rather than the ported one.
sed -i -E 's/^NEWSYM (FxOp[A-Za-z0-9]+)/NEWSYM asm_\1/' _fxops.inc
sed -i -E 's/\b(call|jmp) (FxOp[A-Za-z0-9]+)/\1 asm_\2/' _fxops.inc

# The TO/FROM macro bodies live in a .mac the port deleted; take it from git
# too, along with fxemu2.mac for FETCHPIPE / UpdateR14 / CLRFLAGS.

./asmgit.sh show "$REV:chips/fxemu2.mac" > _fxops_m1.mac
./asmgit.sh show "$REV:chips/fxemu2b.mac" > _fxops_m2.mac
./asmgit.sh show "$REV:chips/fxemu2c.mac" > _fxops_m3.mac
sed -i -E 's/\b(call|jmp) (FxOp[A-Za-z0-9]+)/\1 asm_\2/' _fxops_m1.mac _fxops_m2.mac _fxops_m3.mac

cat > _fxops.asm <<'EOF'
bits 32
%define ALIGN32 align 32
%define ALIGN16 align 16
%macro ccall 1-*
	push ecx
	push edx
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%if %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro
section .note.GNU-stack noalloc noexec nowrite progbits
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
EXTERN FxTable
EXTERN FxTableb
EXTERN FxTablec
EXTERN SfxB
EXTERN SfxCPB
EXTERN SfxCROM
EXTERN SfxCarry
EXTERN SfxOverflow
EXTERN SfxR0
EXTERN SfxR11
EXTERN SfxR13
EXTERN SfxR12
EXTERN SfxR6
EXTERN SfxR4
EXTERN SfxR14
EXTERN SfxR15
EXTERN SfxRomBuffer
EXTERN SfxRAMMem
EXTERN SfxR2
EXTERN SfxR1
EXTERN flagnz
EXTERN fxxand
EXTERN SCBRrel
EXTERN SfxSCBR
EXTERN SFXProc
EXTERN ChangeOps
EXTERN NumberOfOpcodes
EXTERN SfxPIPE
EXTERN SfxCFGR
EXTERN SfxSFR
EXTERN SfxR8
EXTERN SfxR7
EXTERN FxTabled
EXTERN PLOTJmpb
EXTERN PLOTJmpa
EXTERN sfxobjlineloc
EXTERN sfx192lineloc
EXTERN sfx160lineloc
EXTERN sfx128lineloc
EXTERN sfxclineloc
EXTERN fxbit67pcal
EXTERN fxbit45pcal
EXTERN fxbit23pcal
EXTERN fxbit01pcal
EXTERN fxbit67
EXTERN fxbit45
EXTERN fxbit23
EXTERN fxbit01
EXTERN SfxSCMR
EXTERN SfxPOR
EXTERN SfxCOLR
EXTERN sfxramdata
EXTERN SfxnRamBanks
EXTERN SfxROMBR
EXTERN SfxRAMBR
EXTERN SfxLastRamAdr
EXTERN SfxCBR
EXTERN SfxPBR
EXTERN SfxCacheActive
EXTERN SfxMemTable
EXTERN FlushCache
EXTERN SfxSignZero
EXTERN withr15sk
EXTERN FxSeamPC
EXTERN FxSeamSrc
EXTERN FxSeamDst
EXTERN FxSeamCX
EXTERN StubPC
EXTERN StubCX
EXTERN StubSrc
EXTERN StubDst
EXTERN StubHits
EXTERN StubTable
EXTERN StubB
EXTERN StubR15sk
EXTERN StubEndLoop
EXTERN StubPlotIdx
EXTERN StubPlotHits
EXTERN FxTabled
EXTERN StubR15
EXTERN StubWrR15sk
EXTERN SfxSREG
EXTERN SfxDREG
EXTERN StubSetCh
EXTERN StubSetSrc
EXTERN StubSetDst
EXTERN StubIdx
EXTERN fxstub_c
EXTERN fxstubb_c
EXTERN fxstubc_c
EXTERN fxstubd_c

%include "_fxops_m1.mac"
%include "_fxops_m2.mac"
%include "_fxops_m3.mac"

section .text

; void asm_fxcall(void *fn)
NEWSYM asm_fxcall
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[esp+20]
    mov ebp,[FxSeamPC]
    mov esi,[FxSeamSrc]
    mov edi,[FxSeamDst]
    mov ecx,[FxSeamCX]
    call eax
    mov [FxSeamPC],ebp
    mov [FxSeamSrc],esi
    mov [FxSeamDst],edi
    mov [FxSeamCX],ecx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

%macro FXSTUB 2
NEWSYM %1
    mov [StubPC],ebp
    mov [StubCX],ecx
    mov [StubSrc],esi
    mov [StubDst],edi
    mov dword [StubTable],%2
    mov eax,[SfxB]
    mov [StubB],eax
    ; A real nested opcode may set R15 and claim the jump as its own; let the
    ; test drive both, or the guards around R15 are unobservable. Only some
    ; opcodes touch withr15sk, so leaving it alone has to be reachable too.
    mov eax,[StubWrR15sk]
    test eax,eax
    jz %%nor15sk
    mov eax,[StubR15sk]
    mov [withr15sk],eax
%%nor15sk:
    mov eax,[StubR15]
    mov [SfxR15],eax
    inc dword [StubHits]
    ret
%endmacro

FXSTUB fxstub, 1
FXSTUB fxstubb, 2
FXSTUB fxstubc, 3
FXSTUB fxstubd, 4

; Every dispatch-table slot gets its own 16-byte trampoline, so the index a
; handler dispatched with is observable and not just the table it came from.
; The trampolines clobber nothing, so the same shape works for the oracle
; (register ABI) and for the ported side (plain C call).
%macro IDXSTUBS 2
ALIGN 16
NEWSYM %1
%assign idx 0
%rep 1024
    mov dword [StubIdx], idx
    jmp %2
    ALIGN 16
%assign idx idx+1
%endrep
%endmacro

IDXSTUBS idxa_asm, fxstub
IDXSTUBS idxb_asm, fxstubb
IDXSTUBS idxc_asm, fxstubc
IDXSTUBS idxd_asm, fxstubd
IDXSTUBS idxa_c, fxstub_c
IDXSTUBS idxb_c, fxstubb_c
IDXSTUBS idxc_c, fxstubc_c
IDXSTUBS idxd_c, fxstubd_c

%include "_fxops.inc"


; CMODE patches a PLOTJmp entry into FxTabled[$4C] and the d table then
; tail-jumps through it, so those entries have to be real code. 128 stubs, each
; recording its own index, keep the jump safe while still letting the test tell
; a wrong table or a wrong index apart.
ALIGN 32
NEWSYM plotstubs
%assign plotidx 0
%rep 128
    mov dword [StubPlotIdx], plotidx
    inc dword [StubPlotHits]
    ret
    ALIGN 32
%assign plotidx plotidx+1
%endrep

; The real loop epilogue lives in fxemu2.asm; the difftest only needs to know
; it was reached.
NEWSYM FXEndLoop
    inc dword [StubEndLoop]
    ret

; Stand-ins for a d-table handler, used only by the MainLoop test: consume a
; byte, refetch the opcode, then move the things the epilogue has to write back
; (source/destination register and ALT mode) before the FXReturn tail.
%macro LOOPBODY 0
    inc dword [StubHits]
    inc ebp
    mov cl,[ebp]
    mov ch,[StubSetCh]
    mov esi,[StubSetSrc]
    mov edi,[StubSetDst]
%endmacro

NEWSYM loopstub
    LOOPBODY
    dec dword [NumberOfOpcodes]
    js .end
    jmp [FxTabled+ecx*4]
.end
    jmp asm_FXEndLoop

; The STOP case: leaves the loop without spending an opcode.
NEWSYM loopstop
    LOOPBODY
    jmp asm_FXEndLoop
EOF

nasm -f elf32 -w-orphan-labels -o _fxops.o _fxops.asm
echo "wrote _fxops.o (oracle from $(./asmgit.sh rev-parse --short $REV), $(grep -c '^NEWSYM asm_FxOp' _fxops.inc) handlers)"
