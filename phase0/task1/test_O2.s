	.file	"test_0.cpp"
	.text
	.p2align 4
	.globl	_Z5fun_0v
	.type	_Z5fun_0v, @function
_Z5fun_0v:
.LFB0:
	.cfi_startproc
	endbr64
	ret
	.cfi_endproc
.LFE0:
	.size	_Z5fun_0v, .-_Z5fun_0v
	.p2align 4
	.globl	_Z4testv
	.type	_Z4testv, @function
_Z4testv:
.LFB5:
	.cfi_startproc
	endbr64
	ret
	.cfi_endproc
.LFE5:
	.size	_Z4testv, .-_Z4testv
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB3:
	.cfi_startproc
	endbr64
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE3:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04.2) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
