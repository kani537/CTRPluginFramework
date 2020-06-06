#include <sys/iosupport.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <string.h>
#include <3ds.h>
#include "ctrulibExtension.h"

void __ctru_exit(int rc);
int __libctru_gtod(struct _reent *ptr, struct timeval *tp, struct timezone *tz);

extern const u8 __tdata_lma[];
extern const u8 __tdata_lma_end[];
extern u8 __tls_start[];

static ThreadVars * g_mainThreadVars;

ThreadVars* __ctrpf_getThreadVars(ThreadVars * mainThreadVars);

static struct _reent* __ctru_get_reent()
{
	ThreadVars* tv = getThreadVars();

	if (tv->magic != THREADVARS_MAGIC)
	{
        // We're probably hooked from game so get main thread's reent
        return (__ctrpf_getThreadVars(g_mainThreadVars)->reent);
	}
	return tv->reent;
}

void*     __getThreadLocalStorage(void)
{
	ThreadVars* tv = getThreadVars();

	if (tv->magic != THREADVARS_MAGIC)
	{
        // We're probably hooked from game so get main thread's tls
        return __ctrpf_getThreadVars(g_mainThreadVars)->tls_tp;
	}

	return tv->tls_tp;
}

void __system_initSyscalls(void)
{
	// Register newlib syscalls
	__syscalls.exit     = __ctru_exit;
	__syscalls.gettod_r = __libctru_gtod;
	__syscalls.getreent = __ctru_get_reent;

	// Register locking syscalls
	__syscalls.lock_init                  = LightLock_Init;
	__syscalls.lock_acquire               = LightLock_Lock;
	__syscalls.lock_try_acquire           = LightLock_TryLock;
	__syscalls.lock_release               = LightLock_Unlock;
	__syscalls.lock_init_recursive        = RecursiveLock_Init;
	__syscalls.lock_acquire_recursive     = RecursiveLock_Lock;
	__syscalls.lock_try_acquire_recursive = RecursiveLock_TryLock;
	__syscalls.lock_release_recursive     = RecursiveLock_Unlock;

	// Initialize thread vars for the main thread
	ThreadVars* tv = getThreadVars();
	tv->magic = THREADVARS_MAGIC;
	tv->reent = _impure_ptr;
	tv->thread_ptr = NULL;
	tv->tls_tp = __tls_start-8; // ARM ELF TLS ABI mandates an 8-byte header

    g_mainThreadVars = tv;

	u32 tls_size = __tdata_lma_end - __tdata_lma;
	if (tls_size)
		memcpy(__tls_start, __tdata_lma, tls_size);
}
