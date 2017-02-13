#include <3DS.h>
#include "CTRPluginFrameworkImpl/arm11kCommands.h"

extern u32 __ctru_heap;
extern u32 __ctru_heap_size;
extern u32 __ctru_linear_heap;
extern u32 __ctru_linear_heap_size;

extern void (*__system_retAddr)(void);

void envDestroyHandles(void);

void __appExit();

void __libc_fini_array(void);

Result __sync_fini(void) __attribute__((weak));

void __attribute__((weak)) __attribute__((noreturn)) __libctru_exit(int rc)
{
	u32 tmp=0;

	// Unmap the linear heap
	//svcControlMemory(&tmp, __ctru_linear_heap, 0x0, __ctru_linear_heap_size, MEMOP_FREE, 0x0);

	// Unmap the application heap

	arm11kSvcControlMemory(__ctru_heap, __ctru_heap_size, 0x201, 0x0);
	arm11kSvcControlMemory(__ctru_heap, __ctru_heap_size, 0x001, 0x0);
	svcControlMemory(&tmp, __ctru_heap, 0x0, __ctru_heap_size, MEMOP_FREE, 0x0);
	svcControlMemory(&tmp, __ctru_heap, 0x0, __ctru_heap_size, 0x200 | MEMOP_FREE, 0x0);
//	arm11kSvcControlMemory(&tmp, __ctru_heap, 0x0, __ctru_heap_size, MEMOP_FREE, 0x0);

	// Close some handles
	//envDestroyHandles();

	if (__sync_fini)
		__sync_fini();

	// Jump to the loader if it provided a callback
	//if (__system_retAddr)
		//__system_retAddr();

	// Since above did not jump, end this process
	svcExitThread();
}
