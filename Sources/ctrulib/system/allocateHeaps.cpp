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
    void __attribute__((weak))  __system_allocateHeaps(void)
    {
        Color red(255, 0, 0);

        u32     NTRSize = 0;
        bool    isLoaderNTR = Process::CheckRegion(0x06000000, NTRSize, 7);

        if (isLoaderNTR)
        {
            // Heap params
            // Allocate the main heap
            __ctru_heap = 0x07500000;
            __ctru_heap_size = 0x100000;

            /*
            __ctru_heap = 0x07500000; //__ctru_heap = 0x06000100; ///< Leave enough space to relocate (nop ?) NTR's shared functions
            __ctru_heap_size = 0x100000; // O3DS: reduce heap size for Mode 3 ?

            //u32 temp = __ctru_heap;//u32 temp = 0x06000000 + NTRSize;
           // u32 *heap = (u32 *)0x07500000;
           */
            // TODO: check for Mode3 or O3DS special games to avoid allocating too much memory
        again:
            if (R_SUCCEEDED(arm11kSvcControlMemory(&__ctru_heap, __ctru_heap, __ctru_heap_size, 0x203, 3)))
            {
                // Fix heap perms
                Process::CheckRegion(__ctru_heap, __ctru_heap_size, 7);

                // Clear memory
               // for (u32 i = 0; i < __ctru_heap_size / 4; i++)
               //     *heap++ = 0;
            }
            else
                goto fatal;
        }
        else
        {
            // Heap params
            __ctru_heap = 0x06000000;
            __ctru_heap_size = 0x200000; // O3DS: reduce heap size for Mode 3 ?

            while (R_FAILED(arm11kSvcControlMemory(&__ctru_heap, __ctru_heap, __ctru_heap_size, 0x203u, MEMPERM_READ | MEMPERM_WRITE)))
            {
                __ctru_heap_size -= 0x1000;
            }

            // Fix heap perms
            if (!Process::ProtectRegion(__ctru_heap, 7))
                goto fatal;
        }

        // Set up newlib heap
        fake_heap_start = reinterpret_cast<char*>(__ctru_heap);
        fake_heap_end = fake_heap_start + __ctru_heap_size;
        return;
    fatal:
        ScreenImpl::Top->Flash(red);
        g_heapError = true;

        // Allocate the linear heap
        /*__ctru_linear_heap_size = 0x100000;
        if (R_FAILED(arm11kSvcControlMemory(&__ctru_linear_heap, 0, __ctru_linear_heap_size, g_linearOp, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
        {
            // Try again with a different mode
            if (R_FAILED(arm11kSvcControlMemory(&__ctru_linear_heap, 0, __ctru_linear_heap_size, 0x10203u, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
            {
                // Try again in EcoMode
                Preferences::EcoMemoryMode = true;

                __ctru_linear_heap_size = 0x50000;

                if (R_FAILED(arm11kSvcControlMemory(&__ctru_linear_heap, 0, __ctru_linear_heap_size, g_linearOp, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
                {
                    if (R_FAILED(arm11kSvcControlMemory(&__ctru_linear_heap, 0, __ctru_linear_heap_size, 0x10203u, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
                    {
                        ScreenImpl::Bottom->Flash(red);
                        g_heapError = true;
                    }
                }
            }
        }*/
    }
}
