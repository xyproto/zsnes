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

EXTERN Voice0Volume
EXTERN Voice0VolumeR
EXTERN Voice0VolumeL
EXTERN Voice0Volumee
EXTERN Voice0VolumeRe
EXTERN Voice0VolumeLe
EXTERN Voice0EnvInc
EXTERN Voice0Freq
EXTERN BRRPlace0
EXTERN VolumeConvTable
EXTERN UniqueSoundv
EXTERN powhack
EXTERN DSPMem
EXTERN NoiseInc
EXTERN NoisePointer
EXTERN NoiseData
EXTERN PModBuffer
EXTERN DSPBuffer
EXTERN EchoBuffer
section .text
%include "_mix.inc"
