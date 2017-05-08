#include <3DS.h>
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
//extern char* fake_heap_start;
//extern char* fake_heap_end;


/*
u32 __tmp;
u32 __ctru_heap;
u32 __ctru_heap_size;
u32 __ctru_linear_heap;
u32 __ctru_linear_heap_size;
u32	__linearOp = 0x10003u;*/
//static char heap[0x10000];


using namespace CTRPluginFramework;

extern "C" char *fake_heap_start;
extern "C" char *fake_heap_end;
extern "C" u32 __tmp;
extern "C" u32 __ctru_heap;
extern "C" u32 __ctru_heap_size;
extern "C" u32 __ctru_linear_heap;
extern "C" u32 __ctru_linear_heap_size;
extern "C" u32	__linearOp;

extern "C" void __system_allocateHeaps(void);
void __attribute__((weak)) __system_allocateHeaps(void) 
{

	__ctru_heap_size = 0x100000;    

	__ctru_linear_heap_size = 0x200000;
		//__ctru_linear_heap_size = size;

	// Allocate the application heap
	__ctru_heap = 0x07500000;

    Color red(255, 0, 0);

    if (R_FAILED(arm11kSvcControlMemory(&__ctru_heap, __ctru_heap, __ctru_heap_size, 0x203u, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
        Screen::Top->Flash(red);

    if (R_FAILED(arm11kSvcControlMemory(&__ctru_linear_heap, 0, __ctru_linear_heap_size, __linearOp, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
        Screen::Bottom->Flash(red);

	// Set up newlib heap
	fake_heap_start = (char*)__ctru_heap;
	fake_heap_end = fake_heap_start + __ctru_heap_size;
}
