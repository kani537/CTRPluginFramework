#include "3DS.h"
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Font.hpp"
#include "csvc.h"

extern "C" void     abort(void);
extern "C" void     initSystem();
extern "C" void     initLib();
extern "C" void     resumeHook(void);
extern "C" Result   __sync_init(void);
extern "C" void     __system_initSyscalls(void);
extern "C" void     CResume(void);
extern "C" Thread   g_mainThread;

u32     g_resumeHookAddress = 0; ///< Used in arm11k.s for resume hook
Thread  g_mainThread = nullptr; ///< Used in syscalls.c for __ctru_get_reent

namespace CTRPluginFramework
{
    void    ThreadExit(void) __attribute__((noreturn));
    static void    Resume(void);
}

void abort(void)
{
    CTRPluginFramework::Color c(255, 69, 0); //red(255, 0, 0);
    CTRPluginFramework::ScreenImpl::Top->Flash(c);
    CTRPluginFramework::ScreenImpl::Bottom->Flash(c);

    CTRPluginFramework::ThreadExit();
}

void CResume(void)
{
    CTRPluginFramework::Resume();
}

namespace CTRPluginFramework
{
    // Threads stacks
    static u32  threadStack[0x4000] ALIGN(8);
    static u32  keepThreadStack[0x1000] ALIGN(8);

    // Some globals
    FS_Archive  _sdmcArchive;
    Handle      g_continueGameEvent = 0;
    Handle      g_keepThreadHandle;
    Handle      g_keepEvent = 0;
    Handle      g_resumeEvent = 0;
    bool        g_keepRunning = true;
    
    static u32      g_backup[2] = {0}; ///< For the resume hook
    extern u32      g_linearOp; ///< allocateHeaps.cpp
    extern bool     g_heapError; ///< allocateHeaps.cpp

    void    ThreadInit(void *arg);
    void    InstallOSD(void); ///< OSDImpl
    // From main.cpp
    void    PatchProcess(void);
    int     main(void);

   /* static char      *ToString(char *buffer, u64 tid)
    {
        static const char c[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        for (int i = 56; i >= 0; i -= 8)
        {
            u8 byte = (tid >> i) & 0xFF;
            u8 left = byte >> 4;
            u8 right = byte & 0xF;

            *buffer++ = c[left];
            *buffer++ = c[right];
        }
        *buffer = 0;
        return (buffer);
    }

    static bool     IsFileExists(const char *filename)
    {
        Handle      file = 0;
        bool        res = false;
        char        path[0x100] = "/plugin/";
        // Append tid
        char        *p = ToString(path + 8, Process::GetTitleID());

        *p++ = '/';
        // Append filename
        while (*filename)
            *p++ = *filename++;
        *p = 0;

        FS_Path fspath = fsMakePath(PATH_ASCII, path);
        res = R_SUCCEEDED(FSUSER_OpenFile(&file, _sdmcArchive, fspath, FS_OPEN_READ, 0));
        if (res)
            FSFILE_Close(file);
        return (res);
    }

    static void    Resume(void)
    {
        svcSignalEvent(g_resumeEvent);
        //svcFlushProcessDataCache(0xFFFF8001, (void *)g_resumeHookAddress, 8);
        *(u32 *)g_resumeHookAddress = g_backup[0];
        *(u32 *)(g_resumeHookAddress + 4) = g_backup[1];
        //svcInvalidateProcessDataCache(0xFFFF8001, (void *)g_resumeHookAddress, 8);
        svcWaitSynchronization(g_resumeEvent, U64_MAX);
        svcCloseHandle(g_resumeEvent);
        g_resumeEvent = 0;
    }

    static bool     Blacklist(void)
    {
        u32     lowTid = static_cast<u32>(Process::GetTitleID());

        switch (lowTid)
        {
        case 0x00188100: ///< Alphadia
        case 0x00188600: ///< Chronus Arc EUR
        case 0x00179000: ///< Chronus Arc NA
            return (true);
        default:
            break;
        }
        return (false);
    }

    static u32      InstallResumeHook(void)
    {
        u32 lowtid = (u32)Process::GetTitleID();

        if (lowtid != 0x00164800 && lowtid != 0x00175E00)
            return (0);
        if (Blacklist() || IsFileExists("NoHookButSleep"))
            return (0);

        Hook    hook;
        u32     pattern = 0xE59F0014;
        u32     address = 0;

        for (u32 addr = 0x100000; addr < 0x100050; addr += 4)
        {
            if (*(u32 *)addr == pattern)
            {
                address = addr;
                break;
            }
        }

        if (address == 0)
            return (0);

        address -= 0x14;
        g_resumeHookAddress = address;

        g_backup[0] = *(u32 *)address;
        g_backup[1] = *(u32 *)(address + 4);

        hook.Initialize(address, (u32)resumeHook);
        hook.Enable();
        return (address);
    }*/
    static bool     IsPokemonSunOrMoon(void)
    {
        u32 lowtid = (u32)Process::GetTitleID();

        return (lowtid == 0x00164800 || lowtid == 0x00175E00);
    }
    void    KeepThreadMain(void *arg)
    {
        // Initialize the synchronization subsystem
        __sync_init();

        // Initialize newlib's syscalls
        __system_initSyscalls();
        
        // Initialize services
        srvInit();
        acInit();
        amInit();
        fsInit();
        cfguInit();

        // Init Framework's system constants
        SystemImpl::Initialize();

        // Init Process info
        ProcessImpl::Initialize();

        // Init Screen
        ScreenImpl::Initialize();

        // Init sdmcArchive
        {
            FS_Path sdmcPath = { PATH_EMPTY, 1, (u8*)"" };
            FSUSER_OpenArchive(&_sdmcArchive, ARCHIVE_SDMC, sdmcPath);
        }

        // Protect code
        Process::CheckAddress(0x00100000, 7);
        Process::CheckAddress(0x00102000, 7); ///< NTR seems to protect the first 0x1000 bytes, which split the region in two
        // Protect VRAM
        Process::ProtectRegion(0x1F000000, 3);

        // Patch process before it starts
        PatchProcess();

        // Install resume hook
      /*  svcCreateEvent(&g_resumeEvent, RESET_ONESHOT);
        
        if (InstallResumeHook())
        {
            // Continue game
            svcSignalEvent(g_continueGameEvent);

            // Wait until game signal our thread
            svcWaitSynchronization(g_resumeEvent, U64_MAX);
            svcClearEvent(g_resumeEvent);
        }
        else*/
        {
            // Clean event
           /* svcCloseHandle(g_resumeEvent);
            g_resumeEvent = 0;*/

            // Continue game
            svcSignalEvent(g_continueGameEvent);

            // More time for Pokemon as it'll freeze if the plugin wake up while the video is played
            Time time = IsPokemonSunOrMoon() ? Seconds(10.f) : Seconds(5.f);
            Sleep(time);
        }

        // Init heap and newlib's syscalls
        initLib();

        // If heap error, exit
        if (g_heapError)
            goto exit;

        // Create plugin's main thread
        svcCreateEvent(&g_keepEvent, RESET_ONESHOT);
        g_mainThread = threadCreate(ThreadInit, (void *)threadStack, 0x4000, 0x18, -2, false);

        while (g_keepRunning)
        {
            svcWaitSynchronization(g_keepEvent, U64_MAX); 
            svcClearEvent(g_keepEvent);

            while (ProcessImpl::IsPaused())
            {
                if (ProcessImpl::IsAcquiring())
                    Sleep(Milliseconds(100));
            }
        }

        threadJoin(g_mainThread, U64_MAX);
        
    exit:
        svcCloseHandle(g_keepEvent);
       /* if (g_resumeEvent)
            svcSignalEvent(g_resumeEvent);*/
        exit(1);
    }

    void    Initialize(void)
    {
        // Init HID
        hidInit();

        // Init classes
        Font::Initialize();

        // Init event thread
        gspInit();

        //Init OSD
        OSDImpl::_Initialize();

        // Set current working directory
        {
            Directory::ChangeWorkingDirectory(Utils::Format("/plugin/%016llX/", Process::GetTitleID()));
        }

        // Init Process info
        ProcessImpl::UpdateThreadHandle();
    }
    void    InitializeRandomEngine(void);
    // Main thread's start
    void  ThreadInit(void *arg)
    {
        Initialize();

        // Resume game
        /*if (g_resumeEvent)
        {
            svcSignalEvent(g_resumeEvent);
            Sleep(Seconds(5.f));
        }   */

        // Initialize Globals settings
        InitializeRandomEngine();

        // Start plugin
        int ret = main();

        ThreadExit();
    }

    void  ThreadExit(void)
    {
     /*   if (g_resumeEvent)
            svcSignalEvent(g_resumeEvent); */

        // In which thread are we ?
        if (threadGetCurrent() != nullptr)
        {
            // MainThread

            // Release process in case it's currently paused
            ProcessImpl::Play(false);

            // Remove the OSD Hook
            OSDImpl::OSDHook.Disable();

            // Exit services
            gspExit();

            // Exit loop in keep thread
            g_keepRunning = false;
            svcSignalEvent(g_keepEvent);

            threadExit(1);
        }
        else
        {
            // KeepThread
            if (g_mainThread != nullptr)
            {
                ProcessImpl::Play(false);
                PluginMenuImpl::ForceExit();
                threadJoin(g_mainThread, U64_MAX);
            }
            else
                svcSignalEvent(g_continueGameEvent);

            // Exit services
            acExit();
            amExit();
            fsExit();
            cfguExit();            

            exit(-1);
        }
    }

    extern "C" int LaunchMainThread(int arg);
    int   LaunchMainThread(int arg)
    {
        svcCreateEvent(&g_continueGameEvent, RESET_ONESHOT);
        svcCreateThread(&g_keepThreadHandle, KeepThreadMain, 0, &keepThreadStack[0x1000], 0x1A, -2);
        svcWaitSynchronization(g_continueGameEvent, U64_MAX);
        return (0);
    }
}
