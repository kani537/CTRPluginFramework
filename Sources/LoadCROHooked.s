.arm
.fpu vfp
.align(4)

.macro FUNCTION name
    .section .text.\name
    .global \name
    .type \name, %function
    .align 2
\name:
.endm

FUNCTION	LoadCROHooked
	@stmfd	sp!, {r0-r12, lr}  @ Overwritten instruction
	str		lr, [sp, #-4]!     @ Save lr
	bl		OnLoadCro          @ Execute codes
	ldr		lr, [sp], #4       @ Restore lr
	ldmfd	sp, {r0-r12}       @ Restore registers, do not update sp
	bx      lr                 @ Return to game
@ End of function LoadCROHooked
