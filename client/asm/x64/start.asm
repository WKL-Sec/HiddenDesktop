; https://github.com/SolomonSklash/netntlm/blob/master/asm/x64/start.asm
[BITS 64]

GLOBAL  GetIp
GLOBAL  Leave

[SEGMENT .text$F]

GetIp:
    call get_ret_ptr

    get_ret_ptr:
    pop rax
    sub rax, 5
    ret

Leave:
    db 'W', 'K', 'L', 'H', 'V', 'N', 'C'
