#include "CTRPluginFramework/System/Hook.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFrameworkImpl/System/HookManager.hpp"
#include "CTRPluginFramework/System/Lock.hpp"


using namespace CTRPluginFramework;

Hook::Hook(void)
{
    flags.isEnabled = false;
    flags.useLinkRegisterToReturn = true;
    flags.ExecuteOverwrittenInstructionBeforeCallback = true;
    flags.ExecuteOverwrittenInstructionAfterCallback = false;
    targetAddress = 0;
    returnAddress = 0;
    callbackAddress = 0;
    overwrittenInstr = 0;
    index = -1;
}

void    Hook::Initialize(u32 targetAddr, u32 callbackAddr, u32 returnAddr)
{
    targetAddress = targetAddr;
    callbackAddress = callbackAddr;
    if (returnAddr == 0)
        returnAddr = targetAddr + 4;
    returnAddress = returnAddr;
}

static bool  IsTargetAlreadyHooked(u32 target, u32 instruction)
{
    if ((instruction >> 24) != 0xEA)
        return false;

    u32     minOffset = (0x1E80000 - (target + 8)) >> 2;
    u32     maxOffset = (0x1E81000 - (target + 8)) >> 2;
    u32     off = instruction & 0xFFFFFF;

    return off >= minOffset && off < maxOffset;
}

// Ensure that the instruction isn't a memory access depending on PC
static bool  IsInstructionPCDependant(u32 instruction)
{
    static const u32  forbiddenInstructions[] =
    {
        0xE59F, ///< ldr x, [pc, x]
        0xE58F, ///< str x, [pc, x]
        0xED9F, ///< vldr x, [pc, x] even register (s0, s2 etc)
        0xEDDF, ///< vldr x, [pc, x] odd register (s1, s3 etc)
        0xED8F, ///< vstr x, [pc, x] even register (s0, s2 etc)
        0xEDCF, ///< vstr x, [pc, x] odd register (s1, s3 etc)
    };

    instruction >>= 16;

    for (u32 forbiddenInstruction : forbiddenInstructions)
        if (instruction == forbiddenInstruction)
            return true;

    return false;
}

HookResult    Hook::Enable(void)
{
    if (flags.isEnabled)
        return HookResult::Success;

    // Only 1 thread at a time please
    Lock    lock(HookManager::Lock());

    // Check hook parameters
    if (flags.ExecuteOverwrittenInstructionBeforeCallback && flags.ExecuteOverwrittenInstructionAfterCallback)
        return HookResult::HookParamsError;

    // Check that the target is writable
    if (!CTRPluginFramework::Process::CheckAddress(targetAddress, 7))
        return HookResult::InvalidAddress;

    // Get the current instruction
    overwrittenInstr = *reinterpret_cast<u32 *>(targetAddress);

    // Check if the target is already hooked
    if (IsTargetAlreadyHooked(targetAddress, overwrittenInstr))
        return HookResult::AddressAlreadyHooked;

    // Try to get a free slot in the HookManager
    int i = HookManager::AllocNewHook();

    if (i >= 0)
        index = i;
    else if (i == -1)
        return HookResult::HookParamsError; ///< Technically the plugin will crash since OSD's hook will fail
    else if (i == -2)
        return HookResult::AddressAlreadyHooked;
    else if (i == -3)
        return HookResult::TooManyHooks;

    // Check if the instruction is PC dependant
    if (flags.ExecuteOverwrittenInstructionBeforeCallback || flags.ExecuteOverwrittenInstructionAfterCallback)
        if (IsInstructionPCDependant(overwrittenInstr))
            return HookResult::TargetInstructionCannotBeHandledAutomatically;

    // Time to configure the wrapper
    u32         nop = 0xE320F000;
    u32         jmpAddr = 0xE51FF004;
    HookWrapperStatus &hws = HookManager::instance->hws[index];
    HookWrapper *wrapper = hws.wrapper;

    // Use BX LR option
    if (flags.useLinkRegisterToReturn)
    {
        /*  Backup LR
        str     lr, [pc, #32]
        */
        wrapper->backupAndUpdateLR = 0xE58FE020;

        /* Set LR
        mov     lr, pc
        add     lr, lr, #8
        */
        wrapper->setLR[0] = 0xE1A0E00F;
        wrapper->setLR[1] = 0xE28EE008;

        /* Restore LR
        ldr     lr, [pc, #8]
        */
        wrapper->restoreLR = 0xE59FE008;
    }
    else
    {
        wrapper->backupAndUpdateLR = nop;
        wrapper->setLR[0] = nop;
        wrapper->setLR[1] = nop;
        wrapper->restoreLR = nop;
    }

    // Execute overwritten instruction before callback
    if (flags.ExecuteOverwrittenInstructionBeforeCallback)
    {
        wrapper->overwrittenInstr = overwrittenInstr;
    }
    else
        wrapper->overwrittenInstr = nop;

    // Jump code to callback
    wrapper->jumpToCallback = jmpAddr;
    wrapper->callbackAddress = callbackAddress;

    // Execute overwritten instruction after callback
    if (flags.ExecuteOverwrittenInstructionAfterCallback)
    {
        wrapper->overwrittenInstr2 = overwrittenInstr;
    }
    else
        wrapper->overwrittenInstr2 = nop;

    // Return to game
    wrapper->jumpBackToGame = jmpAddr;
    wrapper->returnAddress = returnAddress;

    // Set hook status
    hws.isEnabled = true;
    hws.target = targetAddress;

    // Now set the branch
    u32 off = reinterpret_cast<u32>(wrapper) - (targetAddress + 8);
    *reinterpret_cast<u32 *>(targetAddress) = 0xEA000000 | ((off >> 2) & 0xFFFFFF);

    // We're done
    flags.isEnabled = true;
    return HookResult::Success;
}

void    Hook::Disable(void)
{
    if (!flags.isEnabled)
        return;

    // Only 1 thread at a time please
    Lock    lock(HookManager::Lock());

    if (CTRPluginFramework::Process::Write32(targetAddress, overwrittenInstr))
    {
        flags.isEnabled = false;
        HookManager::FreeHook(index);
    }
}
