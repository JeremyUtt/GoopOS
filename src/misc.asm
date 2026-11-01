; Miscellaneous functions that need to be written in asm

SECTION .text

[bits 32]
global loadIdt
loadIdt:
    mov edx, [esp + 4]
    lidt [edx]
    sti
    ret

; Assembly code (example)
[GLOBAL divide_error_handler]
divide_error_handler:
    ; Save registers (example)
    push eax
    push ebx
    ; ...

    ; Get the faulting instruction address (example)
    mov eax, [esp + 4] ; Assuming esp points to the return address
    ; ...
    extern handle_divide_error_c
    ; Call a C function to handle the error (example)
    call handle_divide_error_c

    ; Restore registers
    pop ebx
    pop eax
    ; ...

    ; Return from interrupt
    iret