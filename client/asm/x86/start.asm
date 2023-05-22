; https://github.com/SolomonSklash/netntlm/blob/master/asm/x86/start.asm
[BITS 32]

GLOBAL	_GetIp
GLOBAL	_Leave

[SEGMENT .text$F]

_GetIp:
	call	_get_ret_ptr

	_get_ret_ptr:
	pop	eax
	sub	eax, 5
	ret

_Leave:
	db 'W', 'K', 'L', 'H', 'V', 'N', 'C'
