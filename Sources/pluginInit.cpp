#include "3DS.h"
#include <stdlib.h>
#include <cstdio>
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Font.hpp"

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
    //extern "C" u32 keepThreadStack[0x1000];
    u32 keepThreadStack[0x1000] ALIGN(8);

    

    Handle      keepThreadHandle;
    Handle      _keepEvent = 0;
    Handle      _continueGameEvent = 0;
    bool        keepRunning = true;
    Thread      mainThread;

    // From main.cpp
    void    PatchProcess(void);
    int     main(void);

    void    ThreadInit(void *arg);

    extern "C" u32 __ctru_heap;
    extern "C" u32 __ctru_heap_size;
    extern "C" u32 __linearOp;

    extern "C" void __appInit(void);

    void    KeepThreadMain(void *arg)
    {
        // Init Framework's system constants
        SystemImpl::Initialize();

        // Init Process info
        ProcessImpl::Initialize();

        // Initialize services
        srvInit();

        // Patch process before it starts
        PatchProcess();

        // Continue game
        svcSignalEvent(_continueGameEvent);

        // Correction for some games like Kirby
        u64 tid = Process::GetTitleID();

        // Pokemon Sun & Moon
        if (tid == 0x0004000000175E00 || tid == 0x0004000000164800
        // ACNL
        ||  tid == 0x0004000000086300 || tid == 0x0004000000086400 || tid == 0x0004000000086200)
            __linearOp = 0x10203u;
        //if (tid == 0x0004000000183600)
            Sleep(Seconds(5));

        // Init heap and newlib's syscalls
        initSystem();

        // Create plugin's main thread
        mainThread = threadCreate(ThreadInit, (void *)threadStack, 0x4000, 0x18, -2, 0);

        svcCreateEvent(&_keepEvent, RESET_ONESHOT);

        while (keepRunning)
        {
            svcWaitSynchronization(_keepEvent, U64_MAX); 
            svcClearEvent(_keepEvent);

            while (ProcessImpl::IsPaused())
            {
                if (ProcessImpl::IsAcquiring())
                    Sleep(Milliseconds(100));
            }               
        }

        threadJoin(mainThread, U64_MAX);
        exit(1);
    }

    extern "C" vu32* hidSharedMem;
    FS_Archive  _sdmcArchive;

    void    InitColors(void);
    void    Initialize(void)
    {
        // Init Services
        __appInit();

        // Init Screen
        Screen::Initialize();
        Renderer::Initialize();
        Font::Initialize();
        // could probably get swapped for lighter implement of gspevent init
        gfxInit(Screen::Top->GetFormat(), Screen::Bottom->GetFormat(), false);

        // Init Process info
        ProcessImpl::UpdateThreadHandle();

        //Init OSD
        OSDImpl::_Initialize();  

        // Init Colors
        InitColors();

        // Init sdmcArchive
        {
            FS_Path sdmcPath = { PATH_EMPTY, 1, (u8*)"" };
            FSUSER_OpenArchive(&_sdmcArchive, ARCHIVE_SDMC, sdmcPath);
        }
        // Set current working directory
        {
            u64     tid = Process::GetTitleID();
            char    path[256] = { 0 };

            sprintf(path, "/plugin/%016llX/", tid);
            Directory::ChangeWorkingDirectory(path);
        }

        // Protect VRAM
        Process::ProtectRegion(0x1F000000, 3);

        // Protect HID Shared Memory in case we want to push inputs
        Process::ProtectMemory((u32)hidSharedMem, 0x1000);


        //if (tid != 0x0004000000183600)
        //Sleep(Seconds(5));

        // Init Font characters
        /*{
        // 0-9

        std::string str = "";
        for (u32 i = '0'; i <= '9'; i++)
        {
        str += i;
        }

        // a-z
        for (u32 i = 'a'; i <= 'z'; i++)
        {
        str += i;
        }

        // A-Z
        for (u32 i = 'A'; i <= 'Z'; i++)
        {
        str += i;
        }
        str += "\uE000\uE001\uE002\uE003\uE004\uE005\uE006\uE020\uE021\uE022\uE023\uE024\uE025\uE026\uE027";

        u8 *s = (u8 *)str.c_str();

        do
        {
        Glyph *g = Font::GetGlyph(s);
        } while (*s);

        }*/
        
    }

    void  ThreadInit(void *arg)
    {
        CTRPluginFramework::Initialize();

        // Reduce Priority
        ProcessImpl::Play(false);

        // Initialize Globals settings
        Preferences::Initialize();        

        // Start plugin
        int ret = main();

        // Release process in case it was forgotten
        ProcessImpl::Play(false);

        gfxExit();

        // Exit loop in keep thread
        keepRunning = false;
        svcSignalEvent(_keepEvent);

        threadExit(1);        
    }

    extern "C" int LaunchMainThread(int arg);
    int   LaunchMainThread(int arg)
    {
        svcCreateEvent(&_continueGameEvent, RESET_ONESHOT);
        svcCreateThread(&keepThreadHandle, KeepThreadMain, 0, &keepThreadStack[0x1000], 0x1A, -2);
        svcWaitSynchronization(_continueGameEvent, U64_MAX);
        return (0);
    }

}
