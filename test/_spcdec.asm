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

EXTERN spcA
EXTERN spcNZ
EXTERN spcP
section .text
%include "_spcdec.inc"
