#include <3DS.h>
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFrameworkImpl/System/SystemImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "csvc.h"

namespace CTRPluginFramework
{
    extern "C" char *fake_heap_start;
    extern "C" char *fake_heap_end;

    extern "C" u32 __ctru_heap;
    extern "C" u32 __ctru_heap_size;

    bool    g_heapError = false;

    extern "C" void   __system_allocateHeaps(void);
    void __system_allocateHeaps(void)
    {
        Color red(255, 0, 0);

        u32     NTRSize = 0;
        bool    isLoaderNTR = SystemImpl::IsLoaderNTR;

        if (isLoaderNTR)
        {
            // Heap params
            // Allocate the main heap
            __ctru_heap = 0x07500000;

            // TODO: check for Mode3 or O3DS special games to avoid allocating too much memory
            if (R_SUCCEEDED(svcControlMemoryEx(&__ctru_heap, __ctru_heap, __ctru_heap, __ctru_heap_size, (MemOp)0x203, (MemPerm)3, true)))
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

            while (R_FAILED(svcControlMemoryEx(&__ctru_heap, __ctru_heap, __ctru_heap, __ctru_heap_size, (MemOp)0x203u, (MemPerm)(MEMPERM_READ | MEMPERM_WRITE), true)))
            {
                if (__ctru_heap_size <= 0x100000)
                    goto fatal;

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
