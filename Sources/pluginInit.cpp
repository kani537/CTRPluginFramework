#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include <stdlib.h>
#include <cstdio>
#include "CTRPluginFramework/Directory.hpp"
#include "CTRPluginFramework/arm11kCommands.h"
#include "CTRPluginFramework/Graphics/OSD.hpp"

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
    Handle      _keepEvent = 0;
    bool        keepRunning = true;
    Thread      mainThread;

    // From main.cpp
    void    PatchProcess(void);
    int     main(void);

    void    ThreadInit(void *arg);

    extern "C" u32 __ctru_heap;
    extern "C" u32 __ctru_heap_size;

    void    KeepThreadMain(void *arg)
    {
        // Init Framework's system constants
        System::Initialize();

        // Init Process info
        Process::Initialize();

        // Correction for some games like Kirby
        u64 tid = Process::GetTitleID();
        if (tid == 0x0004000000183600)
            Sleep(Seconds(5));

        // Init heap and newlib's syscalls
        initSystem();

        // Create plugin's main thread
        mainThread = threadCreate(ThreadInit, (void *)threadStack, 0x4000, 0x3F, -2, 0);

        svcCreateEvent(&_keepEvent, RESET_ONESHOT);

        while (keepRunning)
        {
            svcWaitSynchronization(_keepEvent, U64_MAX); //Stopped working, need to debug
            svcClearEvent(_keepEvent); //Stopped working, need to debug for proper sleep
            if (Process::IsPaused())
            {
                while (Process::IsPaused())
                {
                    if (Process::IsAcquiring())
                        Sleep(Milliseconds(100));
                }               
            }
            else
            {
              //  Sleep(Milliseconds(500)); // temporary fix
            }
        }

        threadJoin(mainThread, U64_MAX);
        exit(1);
    }

    extern "C" void __appInit(void);

    void    Initialize(void)
    {        
        // Init Services
        __appInit();
        
        // Init Screen
        Screen::Initialize();
        Renderer::Initialize();
        // could probably get swapped for lighter implement of gspevent init
        gfxInit(Screen::Top->GetFormat(), Screen::Bottom->GetFormat(), false);

        // Init Process info
        //Process::Initialize(keepEvent);
        Process::UpdateThreadHandle();

        // Patch process before it starts
        PatchProcess();

        //Init OSD
        OSD::_Initialize();  
    }

    extern "C" vu32* hidSharedMem;
    FS_Archive  _sdmcArchive;

    void  ThreadInit(void *arg)
    {
        CTRPluginFramework::Initialize();

        // Init sdmcArchive
        FS_Path sdmcPath = { PATH_EMPTY, 1, (u8*)"" };
        FSUSER_OpenArchive(&_sdmcArchive, ARCHIVE_SDMC, sdmcPath);

        // Reduce Priority
        Process::Play(true);

        // Set current working directory
        u64     tid = Process::GetTitleID();
        char    path[256] = {0};

        sprintf(path, "/plugin/%016llX/", tid);
        Directory::ChangeWorkingDirectory(path);

        // Protect VRAM
        Process::ProtectRegion(0x1F000000);

        // Protect HID Shared Memory in case we want to push inputs
        Process::ProtectMemory((u32)hidSharedMem, 0x1000);

        if (tid != 0x0004000000183600)
            Sleep(Seconds(5));

        // Start plugin
        int ret = main();

        // Release process in case it was forgotten
        Process::Play(true);

        gfxExit();

        // Exit loop in keep thread
        keepRunning = false;
        svcSignalEvent(_keepEvent);

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
