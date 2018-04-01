#include <3DS.h>
#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    extern "C" char *fake_heap_start;
    extern "C" char *fake_heap_end;
    extern "C" u32 __tmp;
    extern "C" u32 __ctru_heap;
    extern "C" u32 __ctru_heap_size;
    extern "C" u32 __ctru_linear_heap;
    extern "C" u32 __ctru_linear_heap_size;

    bool    g_heapError = false;
    //u32	    g_linearOp = 0x10003u;

    extern "C" void             __system_allocateHeaps(void);
    void __system_allocateHeaps(void)
    {
        Color red(255, 0, 0);

        u32     NTRSize = 0;
        bool    isLoaderNTR = Process::CheckRegion(0x06000000, NTRSize, 7);

        if (isLoaderNTR)
        {
            // Heap params
            // Allocate the main heap
            __ctru_heap = 0x07500000;

            // TODO: check for Mode3 or O3DS special games to avoid allocating too much memory
            if (R_SUCCEEDED(arm11kSvcControlMemory(&__ctru_heap, __ctru_heap, __ctru_heap_size, 0x203, 3)))
            {
                // Fix heap perms
                Process::CheckRegion(__ctru_heap, __ctru_heap_size, 7);
            }
            else
                goto fatal;
        }
        else
        {
            // Heap params
            __ctru_heap = 0x06000000;

            while (R_FAILED(arm11kSvcControlMemory(&__ctru_heap, __ctru_heap, __ctru_heap_size, 0x203u, MEMPERM_READ | MEMPERM_WRITE)))
            {
                __ctru_heap_size -= 0x1000;
            }

            // Fix heap perms
            Process::CheckRegion(__ctru_heap, __ctru_heap_size, 7);
        }

        // Set up newlib heap
        fake_heap_start = reinterpret_cast<char*>(__ctru_heap);
        fake_heap_end = fake_heap_start + __ctru_heap_size;
        return;

    fatal:
        ScreenImpl::Top->Flash(red);
        g_heapError = true;
    }
}
