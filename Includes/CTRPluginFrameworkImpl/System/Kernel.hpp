#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEM_KERNEL_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEM_KERNEL_HPP

#include "types.h"

struct KClassToken
{
    const char  *name;
    u8          flags;
} PACKED;

struct KAutoObject
{
    struct Vtable
    {
        void    *f;
        void    *f1;
        void    *f2;
        void    *f3;
        KAutoObject *(*DecrementRefCount)(KAutoObject *obj);
        void    *f4;
        KClassToken *(*GetClassToken)(KClassToken *out, KAutoObject *obj);
    }*      vtable;
    u32     refcount;

    bool    IsKThread(void);
} PACKED;

struct KCodeSetMemDescriptor
{
    u32     startAddress;
    u32     totalPages;
    u32     kBlockInfoCount;
    u32     *firstKLinkedNode;
    u32     *lastKLinkedNode;
} PACKED;

struct KCodeSet
{
    u32     *vtable;
    u32     refCount;
    KCodeSetMemDescriptor text;
    KCodeSetMemDescriptor rodata;
    KCodeSetMemDescriptor data;
    u32     textPages;
    u32     rodataPages;
    u32     rwPages;
    char    processName[8];
    u32     unknown;
    u64     titleId;
} PACKED;

struct HandleDescriptor
{
    u32             info;
    KAutoObject     *obj;
} PACKED;

struct KObjectMutex
{
    u32     kThreadPointer;
    s16     counter1;
    s16     counter2;
} PACKED;

struct KProcessHandleTable
{
    HandleDescriptor    *handleTable;
    s16                 maxHandle;
    s16                 openedHandleCounter;
    HandleDescriptor    *nextOpenHandleDecriptor;
    s16                 totalHandles;
    s16                 handlesCount;
    KObjectMutex        mutex;
    HandleDescriptor    table[0x28];
} PACKED;

struct KThread
{
    u32     vtable;
    u32     refcount;
    u32     nbKThreadSync;
    u32     firstKThreadSync;
    u32     lastKThreadSync;
    u32     events[8];
    u8      schedulingMask;
    u8      sendSyncRequestWokenUp;
    s8      shallTerminate;
    u8      error;
    u32     kdebug;
    u32     basePriority;
    u32     objWaitingOn;
    s32     status;
    u32     kObjectMutex;
    u32     arbitrationAddress;
    u32     firstWaitingObj;
    u32     lastWaitingObj;
    u32     kMutexLinkedList[2];
    u32     kMutexCount;
    u32     firstKMutexList;
    u32     lastKMutexList;
    s32     dynamicPriority;
    s32     coreId;
    u32     kPreemptionTimer;
    u32     unknown;
    u8      isAlive;
    u8      hasBeenTerminated;
    u8      affinityMask;
    u8      padding;
    u32     ownerKProcess;
    u32     threadId;
    u32     svcRegisterStorage;
    u32     endAddress;
    s32     idealProcessor;
    u32     tls;
    u32     fcramTls;
    u32     padding1;
    u32     scheduledThreads[2];
    u32     ptrToLinkedList;
    s32     priorityForInheritance;

    void    Lock(void);
    void    Unlock(void);
    u32  *  GetTls(void);
    bool    IsPluginThread(void);

} PACKED;

struct KProcess
{
    void    *vtable;
    u32     refcount;
} PACKED;

struct KScheduler
{
    void    *vtable;
    void    *KSyncObj;
    u32     countThreadSwitchAttempt;
    u8      contextSwitchNeeded;
    u8      contextSwitchStartedDuringInterrupt;
    u8      triggerCrossCoreInterrupt;
    u8      postInterruptReschedulingNeeded;
    s16     coreNumber;
    s16     countOfThreads;
    u64     priorityBitfield;
    KThread*schedulerThread;
    u32     threadList[2];
    u32     block[0x80];

    void    AdjustThread(KThread *thread, u32 oldSchedulingMask);
} PACKED;

struct KCoreObjectContext
{
    KThread *       currentThread;
    KThread *       currentProcess;
    KScheduler *    currentScheduler;
    u32             currentKSchedulableInterruptEventLinkedList;
    KThread *       lastExceptionThread;
    u32             padding;
    KThread         schedulerThreadInstance;
    KScheduler      schedulerInstance;
    u32             block[0x344];
} PACKED;

struct KCoreContext
{
    u32     na[0x400];
    u32     l1[0x1000];
    u32     na2[0x400];
    u32     kthreadContext[0x400];
    u32     na3[0x400];
    KCoreObjectContext  objectContext;
} PACKED;

namespace Kernel
{
    void    Memcpy(void *dst, const void *src, const u32 size);
}

#endif
