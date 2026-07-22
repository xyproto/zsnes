%macro CalculatePMod 1
    movzx eax,byte[PModBuffer+esi]
    mov ebx,[Voice0Freq+%1*4]
    add al,80h
    mul ebx
    shr eax,7
    shl edx,25
    or eax,edx
    mov ebx,eax
%endmacro

%macro ProcessPMod 1
    push ecx
    push edx
    mov cl,[Voice0EnvInc+%1*4+2]
    mov ax,[edi+edx*2]
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    mov [PModBuffer+esi],ah
    pop edx
    pop ecx
%endmacro
NEWSYM asm_NonEchoMonoPM
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax
    add esi,2
    CalculatePMod ebp
    add [BRRPlace0+ebp*8],ebx
    ret

ALIGN16
NEWSYM asm_NonEchoStereoPM
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    add esi,2
    mov cx,[VolumeConvTable+eax*2]
    pop eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4-4],eax
    CalculatePMod ebp
    add [BRRPlace0+ebp*8],ebx
    ret

ALIGN16
NEWSYM asm_EchoMonoPM
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax
    movzx eax,byte[Voice0Volumee+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]
    pop eax

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*2],eax

    add esi,2
    CalculatePMod ebp
    add [BRRPlace0+ebp*8],ebx
    ret

ALIGN16
NEWSYM asm_EchoStereoPM
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    mov ebx,eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
    movzx eax,byte[Voice0VolumeRe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*4],eax
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]

    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4+4],eax
    movzx eax,byte[Voice0VolumeLe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    mov ebx,[Voice0Freq+ebp*4]
    add [EchoBuffer+esi*4+4],eax

    add esi,2
    CalculatePMod ebp
    add [BRRPlace0+ebp*8],ebx
    ret
NEWSYM asm_NonEchoMono
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax
    add esi,2
    add [BRRPlace0+ebp*8],ebx
    ret

ALIGN16
NEWSYM asm_NonEchoStereo
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    add esi,2
    mov cx,[VolumeConvTable+eax*2]
    pop eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4-4],eax
    add [BRRPlace0+ebp*8],ebx
    ret

ALIGN16
NEWSYM asm_EchoMono
    movzx eax,byte[Voice0Volume+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1
.PMod
    ProcessPMod ebp
.NotNoise1
    mov ax,[edi+edx*2]
.AfterNoise1
    movsx eax,ax
    push eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [DSPBuffer+esi*2],eax

    movzx eax,byte[Voice0Volumee+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]
    pop eax

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*2],eax

    add esi,2
    add [BRRPlace0+ebp*8],ebx
    ret

ALIGN16
NEWSYM asm_EchoStereo
    movzx eax,byte[Voice0VolumeR+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov edx,[BRRPlace0+ebp*8+3]
    mov cx,[VolumeConvTable+eax*2]
    cmp byte[UniqueSoundv],0
    je .NotNoise1b
    mov al,[powhack]
    test byte[DSPMem+3Dh],al
    jz .PMod
    mov eax, [NoiseInc]
    add dword[NoisePointer],eax
    mov eax,[NoisePointer]
    shr eax,18 ; maybe will need a change
    mov ax,[NoiseData+eax*2]
    jmp .AfterNoise1b
.PMod
    ProcessPMod ebp
.NotNoise1b
    mov ax,[edi+edx*2]
.AfterNoise1b
    movsx eax,ax
    mov ebx,eax
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4],eax
    movzx eax,byte[Voice0VolumeRe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add [EchoBuffer+esi*4],eax

    movzx eax,byte[Voice0VolumeL+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]

    mov eax,ebx
    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    add dword[DSPBuffer+esi*4+4],eax
    movzx eax,byte[Voice0VolumeLe+ebp]
    mov ah,[Voice0EnvInc+ebp*4+2]
    mov cx,[VolumeConvTable+eax*2]
    mov eax,ebx

    imul cx
    shr ax,7
    add dl,dl
    or ah,dl
    movsx eax,ax
    mov ebx,[Voice0Freq+ebp*4]
    add [EchoBuffer+esi*4+4],eax

    add esi,2
    add [BRRPlace0+ebp*8],ebx
    ret
