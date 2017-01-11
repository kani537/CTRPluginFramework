#include <3DS.h>

extern char* fake_heap_start;
extern char* fake_heap_end;

u32 __ctru_heap;
u32 __ctru_heap_size;
u32 __ctru_linear_heap;
u32 __ctru_linear_heap_size;
//static char heap[0x10000];

void __attribute__((weak)) __system_allocateHeaps(void) {
	u32 tmp=0;

	
	__ctru_heap_size = 0x10000;
		//__ctru_linear_heap_size = size;

	// Allocate the application heap
	__ctru_heap = 0x07000000;
	svcControlMemory(&tmp, __ctru_heap, 0x0, __ctru_heap_size, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE | MEMPERM_EXECUTE);

	// Allocate the linear heap
	//svcControlMemory(&__ctru_linear_heap, 0x0, 0x0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE);

	// Set up newlib heap
	fake_heap_start = (char*)__ctru_heap;
	fake_heap_end = fake_heap_start + __ctru_heap_size;
}
