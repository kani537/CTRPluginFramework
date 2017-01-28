#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include <stdlib.h>

extern "C" void     abort(void);
extern "C" void     initSystem();

void abort(void)
{
    CTRPluginFramework::Color red(255, 0, 0);
    CTRPluginFramework::Screen::Top->Flash(red);
    CTRPluginFramework::Screen::Bottom->Flash(red);
    exit(-1);  
}

namespace CTRPluginFramework
{
    static u32 threadStack[0x4000] ALIGN(8);
    extern "C" u32 keepThreadStack[0x1000];
    u32 keepThreadStack[0x1000] ALIGN(8);

    

    

    Handle      keepThreadHandle;
    Handle      keepEvent;
    bool        keepRunning = true;
    Thread      mainThread;

    // From main.cpp
    void    PatchProcess(void);
    int     main(void);

    void    ThreadInit(void *arg);

    void    KeepThreadMain(void *arg)
    {
        // Wait for the game to be launched
        //Sleep(Seconds(5));

        // Init heap and services   
        initSystem();
        Sleep(Seconds(5));
        mainThread = threadCreate(ThreadInit, (void *)threadStack, 0x4000, 0x3F, -2, 0);
        svcCreateEvent(&keepEvent, RESET_ONESHOT);
        while (keepRunning)
        {
            svcWaitSynchronization(keepEvent, U64_MAX);
            svcClearEvent(keepEvent);
            while (Process::IsPaused())
                if (Process::IsAcquiring())
                    Sleep(Milliseconds(10));
        }
        threadJoin(mainThread, U64_MAX);
        exit(1);
    }

    extern "C" void __appInit(void);

    void    Initialize(void)
    {        
        // Init Services
        __appInit();

        // Init Framework's system constants
        System::Initialize();

        // Init Screen
        Screen::Initialize();
        Renderer::Initialize();
        gfxInit(Screen::Top->GetFormat(), Screen::Bottom->GetFormat(), false);

        // Init Process info
        Process::Initialize(keepEvent);
        
        // Patch process before it starts
        PatchProcess();        
    }

    // Declared in ctrulib/hid
    extern "C" vu32* hidSharedMem;

    void  ThreadInit(void *arg)
    {
        CTRPluginFramework::Initialize();

        // Reduce Priority
        Process::Play(true);

        // Protect VRAM
        Process::ProtectRegion(0x1F000000);

        // Protect HID Shared Memory in case we want to push / redirects inputs
        Process::ProtectMemory((u32)hidSharedMem, 0x1000);

        // Start plugin
        int ret = main();

        // Release process in case it was forgotten
        Process::Play(true);

        gfxExit();

        // Exit loop in keep thread
        keepRunning = false;
        svcSignalEvent(keepEvent);

        threadExit(1);        
    }

    // For the linker to find the function
    extern "C" int LaunchMainThread(int arg);

    int   LaunchMainThread(int arg)
    {
        svcCreateThread(&keepThreadHandle, KeepThreadMain, 0, &keepThreadStack[0x1000], 0x20, -2);
        return (0);
    }

}
