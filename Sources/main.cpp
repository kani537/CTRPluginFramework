#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include "NTR.h"

extern "C" int main(void);
extern "C" void abort(void);

u32     fsUserHandle;
u32     sdmcArchive;

extern "C" void    initSystem();

void abort(void)
{
  for (;;) { }
}

static  u32 threadStack[0x4000];
Handle hThread;

using namespace CTRPluginFramework;

void    MainThread(u32 arg)
{   
    // Wait for the game to be launched
    svcSleepThread(0x100000000);
    srvInit();    
    initSystem();   
    CTRPluginFramework::System::Initialize();
    CTRPluginFramework::Screen::Initialize();
    svcSleepThread(0x50000000);

    int     i =0;

    Color y = Color(255, 255, 0);
    Color z = Color(0, 255, 255);

    while (i++ < 20)
    {
        Screen::Top->Flash(i % 2 ? y : z);
        Screen::Bottom->Flash(i % 2 ? z : y);

        svcSleepThread(0x50000000);
    }

    srvExit();
    svcExitThread();
}

int   main(void)
{
    rtReleaseLock(&((NS_CONFIG*)(NS_CONFIGURE_ADDR))->debugBufferLock);
     
    //thread = threadCreate(MainThread, 0, 0x1000, 0x3F, -2, false);
    svcCreateThread(&hThread, MainThread, 0, &threadStack[0x4000], 0x3F, -2);
    return (0);
}
