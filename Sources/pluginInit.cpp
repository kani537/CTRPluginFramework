#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include "NTR.h"
#include <stdlib.h>

extern "C" void abort(void);


u32     fsUserHandle;
u32     sdmcArchive;

extern "C" void    initSystem();

int    main(void);

void abort(void)
{
  for (;;) { }
}

static  u32 threadStack[0x4000];
Handle threadHandle;

using namespace CTRPluginFramework;

void  ThreadInit(u32 arg)
{
    // Wait for the game to be launched
    svcSleepThread(5000000000);
    // Init heap and services
    initSystem();

    System::Initialize();
    Screen::Initialize();

    PageInfo pinfo;
    MemInfo minfo;

    svcQueryProcessMemory(&minfo, &pinfo, Process::GetHandle(), 0x1F000000);
    svcControlProcessMemory(Process::GetHandle(), minfo.base_addr, minfo.base_addr, minfo.size, 6, 7);

    int ret = main();

    // Free heap and services and exit thread
    exit(ret);
}
extern "C" int   LaunchMainThread(int arg);

int   LaunchMainThread(int arg)
{
    rtReleaseLock(&((NS_CONFIG*)(NS_CONFIGURE_ADDR))->debugBufferLock);
     
    svcCreateThread(&threadHandle, ThreadInit, 0, &threadStack[0x4000], 0x3F, -2);
    return (0);
}
