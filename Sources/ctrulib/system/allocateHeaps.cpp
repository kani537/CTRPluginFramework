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
    u32	    g_linearOp = 0x10003u;

    extern "C" void             __system_allocateHeaps(void);
    void __attribute__((weak))  __system_allocateHeaps(void)
    {
        Color red(255, 0, 0);

        // Allocate the main heap
        __ctru_heap = 0x07500000;
        __ctru_heap_size = 0xB0000;

        if (R_FAILED(arm11kSvcControlMemory(&__ctru_heap, __ctru_heap, __ctru_heap_size, 0x203u, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
        {
            Screen::Top->Flash(red);
            g_heapError = true;
            return;
        }

        // Allocate the linear heap
        __ctru_linear_heap_size = 0xD0000;
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
                        Screen::Bottom->Flash(red);
                        g_heapError = true;
                    }
                }
            }           
        }

        // Set up newlib heap
        fake_heap_start = reinterpret_cast<char*>(__ctru_heap);
        fake_heap_end = fake_heap_start + __ctru_heap_size;
    }
}
