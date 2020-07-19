#include "CTRPluginFramework/System/Mutex.hpp"

namespace CTRPluginFramework
{
    Mutex::Mutex(void)
    {
        RecursiveLock_Init(&_lock);
    }

    Mutex::~Mutex(void)
    {
        // I suppose that we can "force" unlock the mutex
        if (_lock.counter > 0)
        {
            _lock.counter = 1;
            RecursiveLock_Unlock(&_lock);
        }
    }

    void    Mutex::Lock(void)
    {
        RecursiveLock_Lock(&_lock);
    }

    bool    Mutex::TryLock(void)
    {
        return RecursiveLock_TryLock(&_lock) != 0;
    }

    void    Mutex::Unlock(void)
    {
        RecursiveLock_Unlock(&_lock);
    }
}
