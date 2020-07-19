#include "CTRPluginFramework/System/Lock.hpp"
#include "CTRPluginFramework/System/Mutex.hpp"
#include "CTRPluginFrameworkImpl/System/Kernel.hpp"

namespace CTRPluginFramework
{
#define LIGHTLOCK   1
#define RECLOCK     2
#define MUTEX       3

    Lock::Lock(LightLock &llock) :
        _type{LIGHTLOCK}, _llock{&llock}
    {
        LightLock_Lock(_llock);
    }

    Lock::Lock(RecursiveLock &rlock) :
        _type{RECLOCK}, _rlock{&rlock}
    {
        RecursiveLock_Lock(_rlock);
    }

    Lock::Lock(Mutex &mutex) :
        _type{MUTEX}, _mutex{&mutex}
    {
        mutex.Lock();
    }

    Lock::~Lock(void)
    {
        if (_type == LIGHTLOCK)
            LightLock_Unlock(_llock);
        else if (_type == RECLOCK)
            RecursiveLock_Unlock(_rlock);
        else if (_type == MUTEX)
            _mutex->Unlock();
    }
}
