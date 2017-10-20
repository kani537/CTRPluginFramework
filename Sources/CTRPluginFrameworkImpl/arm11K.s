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

.section .text
.global  dispatchArm11KernelCmd
.type    dispatchArm11KernelCmd, %function

dispatchArm11KernelCmd:  
        STMFD       SP!, {R4,LR}
        LDR         R4, =g_kernelParams
        LDR         R3, [R4]
        CMP         R3, #1
        BNE         loc_108230

arm11kMemcpy:
        LDR         R2, [R4,#0xC]
        LDR         R1, [R4,#4] @ g_KObject = dst
        LDR         R0, [R4,#8] @ g_kernelParam = src
        MOV         R3, #0  @ Rd = Op2

_memcpy:
        CMP         R3, R2
        LDRCC       R12, [R3,R0]
        STRCC       R12, [R3,R1]
        ADDCC       R3, R3, #4
        BCC         _memcpy
        LDMFD       SP!, {R4,PC}
@ ---------------------------------------------------------------------------

loc_108230:
        CMP         R3, #2 
        BNE         loc_10825C

GetKprocessFromHandle:
        LDR         R3, =g_KProcessHandleDataOffset
        LDR         R1, [R4,#4] @ processHandle
        LDR         R0, [R3]
        MOV         R3, #0xFFFF9FFF
        LDR         R3, [R3,#-0xFFB] @ R3 = (0xFFFF9004 == currentKProcess)
        ADD         R0, R0, R3 @ KProcessHandleTable
        BL          getKernelObjectPtr
        STR         R0, [R4,#8]
        LDMFD       SP!, {R4,PC}
@ ---------------------------------------------------------------------------

loc_10825C:
        CMP         R3, #3

GetCurrentKprocess:
        MOVEQ       R3, #0xFFFF9FFF
        LDREQ       R3, [R3,#-0xFFB] @ R3 = (0xFFFF9004 == currentKProcess)
        STREQ       R3, [R4,#4] @ g_kernelParams[1] = currentKProcess
        LDMEQFD     SP!, {R4,PC}

test4:
        CMP         R3, #4

SetCurrentKprocess:
        MOVEQ       R3, #0xFFFF9FFF
        LDREQ       R1, [R4,#8]
        LDREQ       R2, [R3,#-0xFFB]
        STREQ       R2, [R2]
        LDREQ       R2, [R4,#4] @ R2 = kprocess
        STREQ       R2, [R3,#-0xFFB] @ (0xFFFF9004 == currentKProcess) = g_KObject
        LDMEQFD     SP!, {R4,PC}

        CMP         R3, #5
        BNE         loc_1082AC

SetKProcessID:
        LDR         R3, =g_KProcessPIDOffset
        LDR         R1, [R4,#4] @ R1 = g_kernelParams[1]
        LDR         R2, [R3] @ R2 = *(g_KProcessPIDOffset)
        LDR         R3, [R4, #8] @ R3 = g_kernelParams[2]
        LDR         R0, [R1,R2] @ r0 = *(kprocess + g_KProcessPIDOffset)
        STR         R0, [R4,#4] @ g_kernelParams[1] = *(kprocess + g_KProcessPIDOffset)
        LDR	    R0, [R4, #0xC] @ R0 = g_kernelParams[3]
	CMP         R0, #0
	STRNE       R3, [R1,R2]
        LDMFD       SP!, {R4,PC}
@ ---------------------------------------------------------------------------

loc_1082AC:
        CMP         R3, #6
        BNE         ReadCTXID

GetKProcessState:
        LDR         R1, [R4, #4]
        LDR         R2, [R1]
        STR         R2, [R4, #8]

ReadCTXID:
	CMP     R3, #7
	BNE	    exit

	MRC         p15, 0, R1,c13,c0, 1
	STR	    R1, [R4]

exit:
        LDMFD       SP!, {R4,PC};

@ End of function dispatchArm11KernelCmd

.global  executeKernelCmd
.type    executeKernelCmd, %function
executeKernelCmd:
        CPSID       IF  @ Disable Interrupts
        STMFD       SP!, {R3-R11,LR}
        BL          dispatchArm11KernelCmd
        LDMFD       SP!, {R3-R11,PC}
@ End of function executeKernelCmd

FUNCTION    resumeHook
    bl  CResume
    ldr lr, =g_resumeHookAddress
    ldr lr, [lr]
    mov pc, lr
