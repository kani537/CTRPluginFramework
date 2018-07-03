#include <3DS.h>
#include "CTRPluginFramework/System/FwkSettings.hpp"

namespace CTRPluginFramework
{
    extern "C" char *fake_heap_start;
    extern "C" char *fake_heap_end;

    extern "C" u32 __ctru_heap;
    extern "C" u32 __ctru_heap_size;

    #define MEMPERM_RW ((MemPerm)(MEMPERM_READ | MEMPERM_WRITE))

    extern "C" void   __system_allocateHeaps(void);
    void __system_allocateHeaps(void)
    {
        // Heap params
        __ctru_heap = FwkSettings::Header->heapVA;
        __ctru_heap_size = FwkSettings::Header->heapSize - 0x1000;

        // Map Hook memory
        Handle handle;
        svcCreateMemoryBlock(&handle, __ctru_heap + __ctru_heap_size - 0x1000, 0x1000, MEMPERM_RW, MEMPERM_RW);
        svcControlProcess(CUR_PROCESS_HANDLE, PROCESSOP_MAP_MEMBLOCK, 0x01E80000, handle);
        svcCloseHandle(handle);

        svcControlProcess(CUR_PROCESS_HANDLE, PROCESSOP_SET_MMU_TO_RWX, 0, 0);

        // Set up newlib heap
        fake_heap_start = reinterpret_cast<char*>(__ctru_heap);
        fake_heap_end = fake_heap_start + __ctru_heap_size;
    }
}
