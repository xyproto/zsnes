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

EXTERN SPCRAM
EXTERN SPCROM
EXTERN spcextraram
EXTERN DSPMem
EXTERN dspWptr
EXTERN disablespcclr
EXTERN SPCSkipXtraROM
EXTERN reg1read
EXTERN reg2read
EXTERN reg3read
EXTERN reg4read
EXTERN spc700read
EXTERN timeron
EXTERN timincr0
EXTERN timincr1
EXTERN timincr2
EXTERN timinl0
EXTERN timinl1
EXTERN timinl2
EXTERN spcnumread
section .text
%include "_spcio.inc"
