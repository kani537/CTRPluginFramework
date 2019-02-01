.arm
.fpu vfp
.align(4)
.syntax unified

.macro FUNCTION name
    .section .text.\name
    .global \name
    .type \name, %function
    .align 2
\name:
.endm

FUNCTION    LoadCROHooked
    @stmfd  sp!, {r0-r12, lr}  @ Overwritten instruction
    str     lr, [sp, #-4]!     @ Save lr
    bl      OnLoadCro          @ Execute codes
    ldr     lr, [sp], #4       @ Restore lr
    ldmfd   sp, {r0-r12}       @ Restore registers, do not update sp
    bx      lr                 @ Return to game
@ End of function LoadCROHooked

FUNCTION    AR__ExecuteRoutine
    stmfd   sp!, {r0-r12, lr}
    vpush   {d0-d15}
    ldr     r2, =stackBak
    str     sp, [r2]

    ldmia   r0, {r4-r10, sp}
    blx     r1

    ldr     r0, =stackBak
    ldr     sp, [r0]
    vpop    {d0-d15}
    ldmfd   sp!, {r0-r12, pc}
@ End of function AR__ExecuteRoutine

stackBak:
    .word 0

FUNCTION	GSPGPU__RegisterInterruptHook
    ldr     r2, [r4, 0xC] @event handle
    svc     0x32
    ands    r1, r0, #0x80000000
    bxmi	lr
	stmfd	sp!, {r0, lr}
    ldr     r0, [r4, #8] @ thread id
	mov		r2, r1
    ldr     r2, [r4, 0x10] @ shared mem handle
    bl      __gsp__Update
	ldmfd	sp!, {r0, pc}


FUNCTION dbgReturnFromExceptionDirectly
	ldr sp, [r0,#0x34] @sp
	ldr r1, [r0, #0x3c] @pc
	str r1, [sp, #-4]!
	ldr r1, [r0, #0x38] @lr
	str r1, [sp, #-4]!
	mov r2, #0x30
_store_reg_loop:
	ldr r1, [r0, r2]
	str r1, [sp, #-4]!
	sub r2, r2, #4
	cmp r2, #0
	bge _store_reg_loop
	ldr r1, [r0, #0x40]
	msr cpsr, r1
	ldmfd sp!, {r0-r12, lr, pc}
