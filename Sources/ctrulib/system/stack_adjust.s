
	.arm
	.align 2

	.global	initSystem
	.type	initSystem,	%function

initSystem:
	bl	__system_initSyscalls
	bx lr

	.global	initLib
	.type	initLib,	%function
initLib:
	ldr	r2, =saved_stack
	str	sp, [r2]
	str	lr, [r2,#4]
	bl	__system_allocateHeaps
	bl	__libc_init_array
	bl	_init
	ldr	r2, =saved_stack
	ldr	lr, [r2,#4]
 	bx	lr


	.global	__ctru_exit
	.type	__ctru_exit,	%function

__ctru_exit:
	bl	__libc_fini_array
	bl	_fini
	bl	__appExit

	ldr	r2, =saved_stack
	ldr	sp, [r2]
	b	__libctru_exit

	.data
	.align 2
__stacksize__:
	.word	4 * 1024
	.weak	__stacksize__


	.bss
	.align 2
saved_stack:
	.space 8


