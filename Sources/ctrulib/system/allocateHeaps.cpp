#include <3DS.h>
#include "CTRPluginFramework/System/FwkSettings.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/System/MMU.hpp"

namespace CTRPluginFramework
{
    extern "C" char *fake_heap_start;
    extern "C" char *fake_heap_end;

    extern "C" u32 __ctru_heap;
    extern "C" u32 __ctru_heap_size;

    #define MEMPERM_RW ((MemPerm)(MEMPERM_READ | MEMPERM_WRITE))

    static u32 Fail(u32 res)
    {
        return *(u32 *)0xDEADC0DE = res;
    }

    extern "C" void   __system_allocateHeaps(void);
    void __system_allocateHeaps(void)
    {
        // Heap params
        __ctru_heap = FwkSettings::Header->heapVA;
        __ctru_heap_size = FwkSettings::Header->heapSize - 0x2000;

        // Map Hook memory + shared page
        Result res = svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x1E80000, CUR_PROCESS_HANDLE, __ctru_heap + __ctru_heap_size, 0x2000);

        if (R_FAILED(res))
            Fail(res);

        // Set up newlib heap
        fake_heap_start = reinterpret_cast<char*>(__ctru_heap);
        fake_heap_end = fake_heap_start + __ctru_heap_size;
    }
}
