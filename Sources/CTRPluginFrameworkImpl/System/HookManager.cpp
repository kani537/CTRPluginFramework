#include "CTRPluginFrameworkImpl/System/HookManager.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "ctrulib/result.h"
#include "ctrulib/svc.h"
#include "csvc.h"

using namespace CTRPluginFramework;

HookManager  *HookManager::instance = nullptr;

bool     HookManager::Init(void)
{
    if (instance != nullptr)
        return true;

    // Check if region exists
    if (CTRPluginFramework::Process::CheckAddress(0x1E80000))
    {
        // Clear the memory
        u32     *mem = reinterpret_cast<u32 *>(0x1E80000);
        for (u32 i = 0; i < 1024; ++i)
            mem[i] = 0;

        // Create the manager
        instance = new HookManager;

        // Initialize all wrappers (ctor take care of other variables initialization)
        HookWrapper *wrapper = reinterpret_cast<HookWrapper *>(0x1E80000);

        for (HookWrapperStatus &wps : instance->hws)
            wps.wrapper = wrapper++;

        return true;
    }

    // Allocate the region
    u32     dest = 0x1E80000;
    if (R_FAILED(svcControlMemoryEx(&dest, dest, dest, 0x1000, (MemOp)0x203u, (MemPerm)(MEMPERM_READ | MEMPERM_WRITE), true)))
        return false;

    // Fix perms
    CTRPluginFramework::Process::CheckRegion(dest, dest, 7);

    // Call this function once again
    return Init();
}

Mutex&  HookManager::Lock(void)
{
    Init();

    return instance->lock;
}

int     HookManager::AllocNewHook(u32 address)
{
    if (!Init())
        return -1;

    int index = -1;
    for (HookWrapperStatus &hws : instance->hws)
    {
        ++index;

        // Wait 5 seconds after a hook is disabled before reusing its wrapper
        // to be sure that all threads have exited the hook
        if (hws.isEnabled || !hws.disabledSince.HasTimePassed(CTRPluginFramework::Seconds(5.f)))
            continue;

        // If the target is already hooked
        if (address != 0 && hws.target == address)
            return -2;

        // We found a free entry in the list
        return index;
    }

    return -3;
}

void    HookManager::FreeHook(u32 &index)
{
    if (!Init() || index >= MAX_HOOK_WRAPPERS)
        return;

    HookWrapperStatus &hws = instance->hws[index];

    // Disable the wrapper
    hws.isEnabled = false;
    hws.disabledSince.Restart();
    hws.target = 0;

    // Update Hook::index to be invalid
    index = -1;
}
