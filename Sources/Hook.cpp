#include "Hook.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include <cstring>

static void  generate_jump_code(u32 jump_addr, u32 *jump_code)
{
    jump_code[0] = 0xE51FF004; // LDR   PC, [PC, #-4]
    jump_code[1] = jump_addr;
}

static void  generate_jump_code_with_pc(u32 jump_addr, u32 *jump_code)
{
    jump_code[0] = 0xE52DF004; // PUSH  {PC}
    jump_code[1] = 0xE51FF004; // LDR   PC, [PC, #-4]
    jump_code[2] = jump_addr;
}

Hook::Hook(void)
{
    flags.isInitialized = false;
    flags.isEnabled = false;
    flags.isPcSavingHook = false;
    targetAddress = 0;
    afterHookAddress = 0;
    memset(targetCode, 0, 12);
    memset(jumpCode, 0, 12);
    memset(returnCode, 0, 20);
}

void    Hook::Initialize(u32 addr, u32 callbackAddr, bool savePc)
{
    if (flags.isInitialized)
        return;

    flags.isEnabled = false;
    flags.isPcSavingHook = savePc;
    targetAddress = addr;
    afterHookAddress = addr + (savePc ? 12 : 8);

    // Backup original code
    if (!CTRPluginFramework::Process::CopyMemory(targetCode, (void *)addr, 12))
        goto error;

    // Generate jump instruction
    if (savePc)
        generate_jump_code_with_pc(callbackAddr, jumpCode);
    else
        generate_jump_code(callbackAddr, jumpCode);

    // Create return code, no need to adapt to PC mode as it shouldn't be used with this kind of hook
    if (!CTRPluginFramework::Process::CopyMemory(returnCode, (void *)addr, 8))
        goto error;

    generate_jump_code(targetAddress + 8, returnCode + 2);

    flags.isInitialized = true;
    return;

error:
    flags.isInitialized = false;
}

void    Hook::DeInitialize(void)
{
    if (!flags.isInitialized)
        return;

    if (flags.isEnabled)
        Disable();

    flags.isInitialized = false;
}

void    Hook::Enable(void)
{
    if (flags.isEnabled || !flags.isInitialized)
        return;

    if (CTRPluginFramework::Process::CheckAddress(targetAddress, 7))
        if (CTRPluginFramework::Process::CopyMemory((void *)targetAddress, jumpCode, flags.isPcSavingHook ? 12 : 8))
            flags.isEnabled = true;
}

void    Hook::Disable(void)
{
    if (!flags.isEnabled || !flags.isInitialized)
        return;

    if (CTRPluginFramework::Process::CheckAddress(targetAddress, 7))
        if (CTRPluginFramework::Process::CopyMemory((void *)targetAddress, targetCode, flags.isPcSavingHook ? 12 : 8))
            flags.isEnabled = false;
}
