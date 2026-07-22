#!/bin/sh
# mkdifftest.sh - extract one routine from a ZSNES .asm file into a standalone
# 32-bit object for differential testing (see difftest.h).
#
# Usage:
#   ./mkdifftest.sh OUT ASM ENTRY "EXTERN1 EXTERN2 ..." RANGE [RANGE ...]
#
#   OUT      output basename (writes OUT.o and OUT.inc in the current dir)
#   ASM      path to the source .asm (e.g. ../cpu/dspproc.asm)
#   ENTRY    the NEWSYM name(s) to expose (space-separated); each `foo` is
#            renamed to `asm_foo`
#   EXTERNs  space-separated globals the routine references (defined by your
#            harness .c). Bare labels used as addresses (e.g. `mov edi,foo`)
#            count too.
#   RANGEs   one or more  START-END  line ranges (inclusive), concatenated in
#            order. Put any macro definitions the routine needs FIRST, then the
#            routine's own range LAST.
#
# Example (NonEchoMono needs the ProcessPMod macro at 825-837):
#   ./mkdifftest.sh _nem ../cpu/dspproc.asm NonEchoMono \
#       "Voice0Volume Voice0EnvInc BRRPlace0 VolumeConvTable UniqueSoundv \
#        powhack DSPMem NoiseInc NoisePointer NoiseData PModBuffer DSPBuffer" \
#       825-837 1248-1277
set -e

OUT=$1; ASM=$2; ENTRY=$3; EXTERNS=$4; shift 4

INC="$OUT.inc"
: > "$INC"
for r in "$@"; do
    s=${r%-*}; e=${r#*-}
    sed -n "${s},${e}p" "$ASM" >> "$INC"
done

# Rename the entry point(s) (and any internal references to their local labels)
# so the standalone object doesn't clash with the real symbols.
for e in $ENTRY; do
    sed -i -e "s/NEWSYM ${e}\b/NEWSYM asm_${e}/" \
           -e "s/\b${e}\.\([A-Za-z0-9_]*\)/asm_${e}.\1/g" "$INC"
done

# Build the EXTERN directives.
EXT=""
for g in $EXTERNS; do
    EXT="$EXT
EXTERN $g"
done

cat > "$OUT.asm" <<EOF
bits 32
%define ALIGN16 align 16
%define ALIGN32 align 32
section .note.GNU-stack noalloc noexec nowrite progbits
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
$EXT
section .text
%include "$INC"
EOF

nasm -f elf32 -w-orphan-labels -o "$OUT.o" "$OUT.asm"
echo "wrote $OUT.o (entries: $ENTRY)"
