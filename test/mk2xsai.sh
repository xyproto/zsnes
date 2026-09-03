#!/bin/sh
# mk2xsai.sh - build the Kreed 2xSaI oracle for difftest_2xsai.c.
#
# The MMX filters were replaced by a scale2x stub in 5ff6d63d; this assembles
# the last revision that still had them and renames the three entry points and
# the mask globals so they can link beside the C port.
set -e

# Runs from test/ whichever directory make was started in: asmgit.sh and
# every intermediate below are named relative to this one.
cd "$(dirname "$0")"

REV=0aa85977391391c0b470b8529aa98fcec39097fb
MACREV=93aca790ff0948a87316b043fde560eeb9830aae

./asmgit.sh show "$REV:video/2xsaiw.asm" > _2xsai.asm
./asmgit.sh show "$MACREV^:macros.mac" > macros.mac

nasm -f elf32 -DELF -I. _2xsai.asm -o _2xsai_raw.o
objcopy \
    --redefine-sym _2xSaILine=asm_2xSaILine \
    --redefine-sym _2xSaISuper2xSaILine=asm_2xSaISuper2xSaILine \
    --redefine-sym _2xSaISuperEagleLine=asm_2xSaISuperEagleLine \
    --redefine-sym colorMask=asm_colorMask \
    --redefine-sym lowPixelMask=asm_lowPixelMask \
    --redefine-sym qcolorMask=asm_qcolorMask \
    --redefine-sym qlowpixelMask=asm_qlowpixelMask \
    _2xsai_raw.o _2xsai.o
rm -f macros.mac _2xsai_raw.o
echo "wrote _2xsai.o (oracle from $(./asmgit.sh rev-parse --short $REV))"
