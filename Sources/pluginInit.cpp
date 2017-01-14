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
    static  u32 threadStack[0x4000];
    Handle  threadHandle;

    extern "C" int      LaunchMainThread(int arg);

    void    PatchProcess(void);
    int     main(void);

    void    Initialize(void)
    {
        // Init heap and services   
        initSystem();
        // Init Framework's system constants
        System::Initialize();
        // Init Screen
        Screen::Initialize();
        gfxInit(Screen::Top->GetFormat(), Screen::Bottom->GetFormat(), false);
        // Init Process info
        Process::Initialize(threadHandle, System::IsNew3DS());    
        // Launch commands thread
        ThreadCommands::Initialize();
        // Patch process before it starts
        PatchProcess();        
    }

    // Declared in ctrulib/hid
    extern "C" vu32* hidSharedMem;

    void  ThreadInit(u32 arg)
    {

        CTRPluginFramework::Initialize();

        PageInfo pinfo;
        MemInfo minfo;

        // Reduce Priority
        Process::Play();
        // Wait for the game to be launched
        Sleep(Seconds(5));

        // Protect VRAM
        Process::ProtectRegion(0x1F000000);
        // Protect HID Shared Memory in case we want to push / redirects inputs
        Process::ProtectMemory((u32)hidSharedMem, 0x1000);

        // Start plugin
        int ret = main();

        // Release process in case it was forgotten
        Process::Play();

        // Exit commands thread
        ThreadCommands::Exit();

        // Free heap and deinit services and exit thread
        exit(ret);
    }


    int   LaunchMainThread(int arg)
    {
        svcCreateThread(&threadHandle, ThreadInit, 0, &threadStack[0x4000], 0x19, -2);
        return (0);
    }

}
