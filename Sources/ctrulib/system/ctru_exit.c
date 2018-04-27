#include <3DS.h>
#include "CTRPluginFrameworkImpl/arm11kCommands.h"

extern u32 __ctru_heap;
extern u32 __ctru_heap_size;
extern u32 __ctru_linear_heap;
extern u32 __ctru_linear_heap_size;

void __appExit(); ///< Close services

void __libc_fini_array(void);

Result __sync_fini(void) __attribute__((weak));

void    __attribute__((noreturn)) __libctru_exit(int rc)
{
	u32 tmp=0;

	// Unmap the application heap
	arm11kSvcControlMemory(&tmp, __ctru_heap, __ctru_heap_size, MEMOP_FREE, 0x0);

	if (__sync_fini)
		__sync_fini();

	// End this thread
	svcExitThread();
}
