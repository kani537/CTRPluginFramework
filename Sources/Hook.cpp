#include "Hook.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include <cstring>
#include "ctrulib/result.h"
#include "CTRPluginFrameworkImpl/arm11kCommands.h"

#define MAX_HOOK_WRAPPERS 93

struct HookWrapper
{
    u32     backupAndUpdateLR;  ///< Default: nop
    u32     overwrittenInstr;   ///< Default: nop
    u32     setLR[2];           ///< Default: nop
    u32     jumpToCallback;     ///< ldr pc, [pc, #-4]
    u32     callbackAddress;    ///< Address of the code to jump to
    u32     restoreLR;          ///< Default: nop
    u32     overwrittenInstr2;  ///< Default: nop
    u32     jumpBackToGame;     ///< ldr pc, [pc, #-4]
    u32     returnAddress;
    u32     lrBackup;
};

struct HookManager  ///< 0x1E8 0000
{
    HookWrapper     wrappers[MAX_HOOK_WRAPPERS];

    // Return index free or -1 if error
    static int      AllocNewHook(void);
    // Free a new index
    static void     FreeHook(u32 &index);
};

static bool     HookManagerInit(void);

static HookManager  *g_manager = nullptr;

int     HookManager::AllocNewHook(void)
{
    if (!HookManagerInit())
        return -1;

    for (int i = 0; i < MAX_HOOK_WRAPPERS; ++i)
    {
        HookWrapper *wrapper = &g_manager->wrappers[i];

        if (wrapper->callbackAddress == 0)
            return i;
    }

    return -1;
}

void    HookManager::FreeHook(u32 &index)
{
    if (!HookManagerInit() || index >= MAX_HOOK_WRAPPERS)
        return;

    memset(&g_manager->wrappers[index], 0, sizeof(HookWrapper));

    index = -1;
}

static bool     HookManagerInit(void)
{
    if (g_manager != nullptr)
        return true;

    if (CTRPluginFramework::Process::CheckAddress(0x1E80000))
    {
        g_manager = reinterpret_cast<HookManager *>(0x1E80000);

        // Clear the memory
        u32     *mem = reinterpret_cast<u32 *>(0x1E80000);
        for (u32 i = 0; i < 1024; ++i)
            mem[i] = 0;

        return true;
    }

    // Allocate the region
    u32     dest = 0x1E80000;
    if (R_FAILED(arm11kSvcControlMemory(&dest, dest, 0x1000, 0x203u, MEMPERM_READ | MEMPERM_WRITE)))
        return false;

    // Fix perms
    CTRPluginFramework::Process::CheckRegion(dest, dest, 7);

    return HookManagerInit();
}

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
    index = HookManager::AllocNewHook();

    if (index >= MAX_HOOK_WRAPPERS)
        return HookResult::TooManyHooks;

    // Check if the instruction is PC dependant
    if (flags.ExecuteOverwrittenInstructionBeforeCallback || flags.ExecuteOverwrittenInstructionAfterCallback)
        if (IsInstructionPCDependant(overwrittenInstr))
            return HookResult::TargetInstructionCannotBeHandledAutomatically;

    // Time to configure the wrapper
    u32         nop = 0xE320F000;
    u32         jmpAddr = 0xE51FF004;
    HookWrapper *wrapper = &g_manager->wrappers[index];

    // Use BX LR option
    if (flags.useLinkRegisterToReturn)
    {
        /*  Backup LR
        str     lr, [pc, #36]
        */
        wrapper->backupAndUpdateLR = 0xE58FE024;

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

    if (CTRPluginFramework::Process::Write32(targetAddress, overwrittenInstr))
    {
        flags.isEnabled = false;
        HookManager::FreeHook(index);
    }
}
