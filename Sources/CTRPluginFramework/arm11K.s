.arm
.align(4)
.section .text
.global  dispatchArm11KernelCmd
.type    dispatchArm11KernelCmd, %function

dispatchArm11KernelCmd:  
        STMFD       SP!, {R4,LR} @ Store Block to Memory
        LDR         R4, =g_kernelParams @ Load from Memory
        LDR         R3, [R4] @ Load from Memory
        CMP         R3, #1  @ Set cond. codes on Op1 - Op2
        BNE         loc_108230 @ Branch

arm11kMemcpy:
        LDR         R2, [R4,#0xC]
        LDR         R1, [R4,#4] @ g_KObject = dst
        LDR         R0, [R4,#8] @ g_kernelParam = src
        MOV         R3, #0  @ Rd = Op2

_memcpy:                 @ CODE XREF: dispatchArm11KernelCmd+34j
        CMP         R3, R2  @ Set cond. codes on Op1 - Op2
        LDRCC       R12, [R3,R0] @ Load from Memory
        STRCC       R12, [R3,R1] @ Store to Memory
        ADDCC       R3, R3, #4 @ Rd = Op1 + Op2
        BCC         _memcpy @ Branch
        LDMFD       SP!, {R4,PC} @ Load Block from Memory
@ ---------------------------------------------------------------------------

loc_108230:
        CMP         R3, #2  @ Set cond. codes on Op1 - Op2
        BNE         loc_10825C @ Branch

GetKprocessFromHandle:
        LDR         R3, =g_KProcessHandleDataOffset
        LDR         R1, [R4,#4] @ processHandle
        LDR         R0, [R3] @ Load from Memory
        MOV         R3, #0xFFFF9FFF
        LDR         R3, [R3,#-0xFFB] @ R3 = (0xFFFF9004 == currentKProcess)
        ADD         R0, R0, R3 @ KProcessHandleTable
        BL          getKernelObjectPtr @ Branch with Link
        STR         R0, [R4,#8] @ Store to Memory
        LDMFD       SP!, {R4,PC} @ Load Block from Memory
@ ---------------------------------------------------------------------------

loc_10825C:
        CMP         R3, #3  @ Set cond. codes on Op1 - Op2

GetCurrentKprocess:
        MOVEQ       R3, #0xFFFF9FFF
        LDREQ       R3, [R3,#-0xFFB] @ R3 = (0xFFFF9004 == currentKProcess)
        STREQ       R3, [R4,#4] @ g_kernelParams[1] = currentKProcess
        LDMEQFD     SP!, {R4,PC} @ Load Block from Memory

test4:
        CMP         R3, #4

SetCurrentKprocess:
        MOVEQ       R3, #0xFFFF9FFF
        LDREQ       R1, [R4,#8]
        LDREQ       R2, [R3,#-0xFFB]
        STREQ       R2, [R2]
        LDREQ       R2, [R4,#4] @ R2 = kprocess
        STREQ       R2, [R3,#-0xFFB] @ (0xFFFF9004 == currentKProcess) = g_KObject
        LDMEQFD     SP!, {R4,PC} @ Load Block from Memory

        CMP         R3, #5  @ Set cond. codes on Op1 - Op2
        BNE         loc_1082AC @ Branch

SetKProcessID:
        LDR         R3, =g_KProcessPIDOffset
        LDR         R1, [R4,#4] @ R1 = g_kernelParams[2]
        LDR         R2, [R3] @ R2 = *(g_KProcessPIDOffset)
        LDR         R3, [R4] @ R3 = g_kernelParams[1]
        LDR         R0, [R2,R3] @ r0 = *(kprocess + g_KProcessPIDOffset)
        STR         R0, [R4,#4] @ g_kernelParams[1] = *(kprocess + g_KProcessPIDOffset)
        STR         R1, [R2,R3] @ Store to Memory
        LDMFD       SP!, {R4,PC} @ Load Block from Memory
@ ---------------------------------------------------------------------------

loc_1082AC:
        LDMFD       SP!, {R4,PC};
@        CMP         R3, #6  @ Set cond. codes on Op1 - Op2
@        LDMNEFD     SP!, {R4,PC} @ Load Block from Memory
@        LDMFD       SP!, {R4,LR} @ Load Block from Me @mory
@       B           installSVCHook @ Branch

@ End of function dispatchArm11KernelCmd

.global  executeKernelCmd
.type    executeKernelCmd, %function
executeKernelCmd:
        CPSID       IF  @ Disable Interrupts
        STMFD       SP!, {R3-R11,LR} @ Store Block to Memory
        BL          dispatchArm11KernelCmd @ Branch with Link
        LDMFD       SP!, {R3-R11,PC} @ Load Block from Memory
@ End of function executeKernelCmd