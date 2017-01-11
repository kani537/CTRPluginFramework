#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include "NTR.h"
#include <stdlib.h>

extern "C" void abort(void);


u32     fsUserHandle;
u32     sdmcArchive;

extern "C" void    initSystem();

int    main(int arg);

void abort(void)
{
  for (;;) { }
}

static  u32 threadStack[0x4000];
Handle threadHandle;

void  ThreadInit(u32 arg)
{
    svcSleepThread(100000000);
    initSystem();
    int ret = main(0);
    exit(ret);
}
extern "C" int   LaunchMainThread(int arg);

int   LaunchMainThread(int arg)
{
    rtReleaseLock(&((NS_CONFIG*)(NS_CONFIGURE_ADDR))->debugBufferLock);
     
    svcCreateThread(&threadHandle, ThreadInit, 0, &threadStack[0x4000], 0x3F, -2);
    return (0);
}
