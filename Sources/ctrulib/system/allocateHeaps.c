#include <3DS.h>
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
extern char* fake_heap_start;
extern char* fake_heap_end;

u32 __tmp;
u32 __ctru_heap;
u32 __ctru_heap_size;
u32 __ctru_linear_heap;
u32 __ctru_linear_heap_size;
u32	__linearOp = 0x10003u;
//static char heap[0x10000];

void __attribute__((weak)) __system_allocateHeaps(void) 
{

	__ctru_heap_size = 0x100000;

	__ctru_linear_heap_size = 0x250000;
		//__ctru_linear_heap_size = size;

	// Allocate the application heap
	__ctru_heap = 0x07500000;
	arm11kSvcControlMemory(&__ctru_heap, __ctru_heap,  __ctru_heap_size, 0x203u, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE);

	arm11kSvcControlMemory(&__ctru_linear_heap, 0x0,  __ctru_linear_heap_size, __linearOp, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE);
	/*if (R_FAILED(arm11kSvcControlMemory(__ctru_heap, __ctru_heap_size, 0x203u, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE)))
	{
		__ctru_heap = 0x07500000;
		__ctru_heap_size = 0x10000;
		//svcControlMemory(&__tmp, __ctru_heap, 0x0, __ctru_heap_size, 0x203u, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE);

	}*/

	// Allocate the linear heap
	//svcControlMemory(&__ctru_linear_heap, 0x0, 0x0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE);

	// Set up newlib heap
	fake_heap_start = (char*)__ctru_heap;
	fake_heap_end = fake_heap_start + __ctru_heap_size;
}
