#include "3DS.h"
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Font.hpp"

extern "C" void     abort(void);
extern "C" void     initSystem();
extern "C" void     initLib();
extern "C" void     resumeHook(void);
extern "C" Result   __sync_init(void);
extern "C" void     __system_initSyscalls(void);
extern "C" Thread   g_mainThread;
extern "C" void     loadCROHooked(void);
extern "C" u32      croReturn;
u32 croReturn = 0;

u32     g_resumeHookAddress = 0; ///< Used in arm11k.s for resume hook
Thread  g_mainThread = nullptr; ///< Used in syscalls.c for __ctru_get_reent

namespace CTRPluginFramework
{
    void    ThreadExit(void);
}

void abort(void)
{
    if (CTRPluginFramework::System::OnAbort)
        CTRPluginFramework::System::OnAbort();

    CTRPluginFramework::Color c(255, 69, 0); //red(255, 0, 0);
    CTRPluginFramework::ScreenImpl::Top->Flash(c);
    CTRPluginFramework::ScreenImpl::Bottom->Flash(c);

    CTRPluginFramework::ThreadExit();
    while (true);
}

static Hook    g_loadCroHook;

void    ExecuteLoopOnEvent(void)
{
    using CTRPluginFramework::PluginMenuExecuteLoop;

    PluginMenuExecuteLoop::LockAR();
    PluginMenuExecuteLoop::ExecuteAR();
    PluginMenuExecuteLoop::UnlockAR();

    PluginMenuExecuteLoop::Lock();
    PluginMenuExecuteLoop::ExecuteBuiltin();
    PluginMenuExecuteLoop::Unlock();
}

using LoadCROReturn = u32 (*)(u32, u32, u32, u32, u32, u32, u32);
static LightLock onLoadCroLock;

extern "C" void onLoadCro(void);
void onLoadCro(void)
{
    LightLock_Lock(&onLoadCroLock);
    ExecuteLoopOnEvent();
    LightLock_Unlock(&onLoadCroLock);
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

    extern bool     g_heapError; ///< allocateHeaps.cpp

    void    ThreadInit(void *arg);
    void    InstallOSD(void); ///< OSDImpl

    // From main.cpp
    void    PatchProcess(FwkSettings &settings);
    int     main(void);

    static bool     IsPokemonSunOrMoon(void)
    {
        u32 lowtid = (u32)Process::GetTitleID();

        return (lowtid == 0x00164800 || lowtid == 0x00175E00);
    }

    namespace Heap
    {
        extern u8* __ctrpf_heap;
        extern u32 __ctrpf_heap_size;
    }

    void    KeepThreadMain(void *arg)
    {
        FwkSettings settings;

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

        // Init locks
        PluginMenuExecuteLoop::InitLocks();

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

        // Check loader
        SystemImpl::IsLoaderNTR = Process::CheckAddress(0x06000000, 3);

        // Init default settings
        settings.HeapSize = SystemImpl::IsLoaderNTR ? 0x100000 : 0x200000;
        settings.EcoMemoryMode = false;
        settings.StartARHandler = true;
        settings.AllowSearchEngine = true;
        settings.WaitTimeToBoot = Seconds(5.f);

        // Patch process before it starts & let the dev init some settings
        PatchProcess(settings);

        // Continue game
        svcSignalEvent(g_continueGameEvent);

        // Wait for the game to be fully launched
        Sleep(settings.WaitTimeToBoot);

        // Init heap and newlib's syscalls
        initLib();

        // Copy FwkSettings to the globals (solve initialization issues)
        Preferences::Settings = settings;
        // Set default theme
        FwkSettings::SetThemeDefault();

        void *tst;
        u32 size =  System::IsLoaderNTR() || !settings.AllowSearchEngine ? (settings.EcoMemoryMode ? 0x50000 : 0xC0000) : (settings.EcoMemoryMode ? 0xC0000 : 0x180000);

        // If heap error, exit
        if (g_heapError)
            goto exit;

        // Init System::Heap
        Heap::__ctrpf_heap_size = size;
        Heap::__ctrpf_heap = static_cast<u8*>(::operator new(size));
        tst = Heap::Alloc(0x100);
        Heap::Free(tst);

        // Create plugin's main thread
        svcCreateEvent(&g_keepEvent, RESET_ONESHOT);

        g_mainThread = threadCreate(ThreadInit, (void *)threadStack, 0x4000, 0x18, -2, false);

        // Install CRO hook
        {
            const std::vector<u32> LoadCroPattern =
            {
                0xE92D5FFF, 0xE28D4038, 0xE89407E0, 0xE28D4054,
                0xE8944800, 0xEE1D4F70, 0xE59FC058, 0xE3A00000,
                0xE5A4C080, 0xE284C028, 0xE584500C, 0xE584A020
            };

            /*const std::vector<u32> UnloadCroPattern =
            {
                0xE92D4070, 0xEE1D4F70, 0xE59F502C, 0xE3A0C000,
                0xE5A45080, 0xE2846004, 0xE5840014, 0xE59F001C,
                0xE886100E, 0xE5900000, 0xEF000032, 0xE2001102
            };*/

            u32     loadCroAddress = Utils::Search<u32>(0x00100000, Process::GetTextSize(), LoadCroPattern);

            if (loadCroAddress)
            {
                LightLock_Init(&onLoadCroLock);
                g_loadCroHook.Initialize(loadCroAddress, (u32)loadCROHooked);
                croReturn = loadCroAddress + 8;
                g_loadCroHook.Enable();
            }
        }

        // Reduce priority
        while (R_FAILED(svcSetThreadPriority(g_keepThreadHandle, 0x30)));

        svcWaitSynchronization(g_keepEvent, U64_MAX);

        if (settings.StartARHandler)
        {
            while (g_keepRunning)
            {
                // Wait for a new frame
                LightEvent_Wait(&OSDImpl::OnNewFrameEvent);

                // Lock the AR & execute codes before releasing it
                PluginMenuExecuteLoop::LockAR();
                PluginMenuExecuteLoop::ExecuteAR();
                PluginMenuExecuteLoop::UnlockAR();
            }
        }
        else
        {
            while (g_keepRunning)
            {
                svcWaitSynchronization(g_keepEvent, U64_MAX);
                svcClearEvent(g_keepEvent);
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
        if (!System::IsLoaderNTR()) // Luma's loader
        {
            if (*(vu32 *)0x070000FC)
            {
                std::string path = Utils::Format("/luma/plugins/ActionReplay/%016llX", Process::GetTitleID());

                if (!Directory::IsExists(path))
                    Directory::Create(path);

                Directory::ChangeWorkingDirectory(path + "/");
            }
            else
                Directory::ChangeWorkingDirectory(Utils::Format("/luma/plugins/%016llX/", Process::GetTitleID()));
        }
        else
        {
            std::string dirpath = Utils::Format("/plugin/%016llX", Process::GetTitleID());

            // Check if game's folder exists
            if (!Directory::IsExists(dirpath))
            {
                // If doesn't exist, so create a temporary folder
                dirpath = "/plugin/game/ctrpf";
                if (!Directory::IsExists(dirpath))
                    Directory::Create(dirpath);
                dirpath += "/";
                Process::GetName(dirpath);

                if (!Directory::IsExists(dirpath))
                    Directory::Create(dirpath);
            }

            dirpath += "/";
            Directory::ChangeWorkingDirectory(dirpath);
        }

        // Init Process info
        ProcessImpl::UpdateThreadHandle();
    }

    void    InitializeRandomEngine(void);
    // Main thread's start
    void  ThreadInit(void *arg)
    {
        Initialize();

        // Initialize Globals settings
        InitializeRandomEngine();

        // Wake up init thread
        svcSignalEvent(g_keepEvent);

        // Start plugin
        int ret = main();

        ThreadExit();
    }

    u32   __hasAborted = 0;
    void  ThreadExit(void)
    {
     /*   if (g_resumeEvent)
            svcSignalEvent(g_resumeEvent); */
        __hasAborted = 1;
        // In which thread are we ?
        if (threadGetCurrent() != nullptr)
        {
            // MainThread

            // Remove the OSD Hook
            OSDImpl::OSDHook.Disable();
            // Release process in case it's currently paused
            ProcessImpl::Play(false);

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
