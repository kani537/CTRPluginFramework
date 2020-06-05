#if __INTELLISENSE__
typedef unsigned int __SIZE_TYPE__;
typedef unsigned long __PTRDIFF_TYPE__;
//#undef __cplusplus
//#define __cplusplus 201103L
#undef __cpp_exceptions
#define __cpp_exceptions 0
#define __attribute__(q)
#define __extension__
#define __asm__(expr)
#define __builtin_labs(a) 0
#define __builtin_llabs(a) 0
#define __builtin_fabs(a) 0
#define __builtin_fabsf(a) 0
#define __builtin_fabsl(a) 0
#define __builtin_strcmp(a,b) 0
#define __builtin_strlen(a) 0
#define __builtin_memcpy(a,b) 0
#define __builtin_va_list void*
#define __builtin_va_start(a,b)
#endif

#include "3DS.h"
#include "CTRPluginFramework.hpp"
#include <string>
#include <iterator>
#include "csvc.h"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "CTRPluginFrameworkImpl/Graphics/BMPImage.hpp"
//#include "OSDManager.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include <list>
#include "CTRPluginFrameworkImpl/Graphics/TextBox.hpp"
#include "plgldr.h"

namespace CTRPluginFramework
{
    volatile u32 g_mainThreadTls;

    static void    ToggleTouchscreenForceOn(void)
    {
        static u32 original = 0;
        static u32 *patchAddress = nullptr;

        if (patchAddress && original)
        {
            *patchAddress = original;
            return;
        }

        static const std::vector<u32> pattern =
        {
            0xE59F10C0, 0xE5840004, 0xE5841000, 0xE5DD0000,
            0xE5C40008, 0xE28DD03C, 0xE8BD80F0, 0xE5D51001,
            0xE1D400D4, 0xE3510003, 0x159F0034, 0x1A000003
        };

        Result  res;
        Handle  processHandle;
        s64     textTotalRoundedSize = 0;
        s64     startAddress = 0;
        u32 *   found;

        if (R_FAILED(svcOpenProcess(&processHandle, 16)))
            return;

        svcGetProcessInfo(&textTotalRoundedSize, processHandle, 0x10002);
        svcGetProcessInfo(&startAddress, processHandle, 0x10005);
        if(R_FAILED(svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, processHandle, (u32)startAddress, textTotalRoundedSize)))
            goto exit;

        found = (u32 *)Utils::Search<u32>(0x14000000, (u32)textTotalRoundedSize, pattern);

        if (found != nullptr)
        {
            original = found[13];
            patchAddress = (u32 *)PA_FROM_VA((found + 13));
            found[13] = 0xE1A00000;
        }

        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, textTotalRoundedSize);
exit:
        svcCloseHandle(processHandle);
    }




    // This function is called on the plugin start, before main
    void    PatchProcess(FwkSettings &settings)
    {
        ToggleTouchscreenForceOn();
        settings.WaitTimeToBoot = Seconds(8);
    }

    void    OnExitProcess(void)
    {
        ToggleTouchscreenForceOn();
    }

    u32     strlen(const char *s)
    {
        u32 size = 0;

        while (s && *s++)
            ++size;

        return size;
    }

    using FuncPointer = void(*)(MenuEntry*);

    static MenuEntry *EntryWithHotkey(const std::string &name, const std::string &note, FuncPointer gamefunc, const Hotkey &hotkey)
    {
        MenuEntry   *entry = new MenuEntry(name, gamefunc, note);

        entry->Hotkeys += hotkey;

        return (entry);
    }

    static MenuEntry *EntryWithHotkey(const std::string &name, const std::string &note, FuncPointer gamefunc, FuncPointer menufunc, const Hotkey &hotkey)
    {
        MenuEntry   *entry = new MenuEntry(name, gamefunc, menufunc, note);

        entry->Hotkeys += hotkey;

        return (entry);
    }

    static MenuEntry *EntryWithHotkey(const std::string &name, const std::string &note, FuncPointer gamefunc, const std::vector<Hotkey> &hotkeys)
    {
        MenuEntry   *entry = new MenuEntry(name, gamefunc, note);

        for (const Hotkey &hk : hotkeys)
        {
            entry->Hotkeys += hk;
        }

        return (entry);
    }

    static MenuEntry *EntryWithHotkey(const std::string &name, const std::string &note, FuncPointer gamefunc, FuncPointer menuFunc, const std::vector<Hotkey> &hotkeys)
    {
        MenuEntry   *entry = new MenuEntry(name, gamefunc, menuFunc, note);

        for (const Hotkey &hk : hotkeys)
        {
            entry->Hotkeys += hk;
        }

        return (entry);
    }

    MenuEntry   *EntryWithNotifier(const std::string &name, FuncPointer gameFunc)
    {
        auto lambda = [](MenuEntry *entry)
        {
            if (entry->WasJustActivated())
                OSD::Notify(entry->Name() << ": " << Color::LimeGreen <<"ON");
            else if (!entry->IsActivated())
                OSD::Notify(entry->Name() << ": " << Color::Red <<"OFF");

            void *arg = entry->GetArg();

            if (arg != nullptr)
                reinterpret_cast<FuncPointer>(arg)(entry);
        };

        MenuEntry *entry = new MenuEntry(name, lambda);

        entry->SetArg((void *)gameFunc);
        return entry;
    }

    void    HexMessageBox(u32 address)
    {
        u32     buffer[18] = {0}; // 3 * 6

        Kernel::Memcpy(buffer, (void *)address, sizeof(buffer));

        std::string text;

        for (u32 i = 0; i < (sizeof(buffer) >> 2); )
        {
            text += Utils::ToHex(buffer[i++]) + "  ";
            text += Utils::ToHex(buffer[i++]) + "  ";
            text += Utils::ToHex(buffer[i++]) + "\n";
        }

        MessageBox(Utils::ToHex(address), text)();
    }

    void    ReadKernelValue(MenuEntry *entry)
    {
        static u32  address = 0xFFFF9000;

        Keyboard    kb((std::vector<std::string>){ "Address", "View"});

        int choice;

        do
        {
            choice = kb.Open();
            if (choice == 0) (Keyboard()).Open(address, address);
            if (choice == 1) HexMessageBox(address);
        } while (choice != -1);
    }

    void    CheckScreenFormat(MenuEntry *entry)
    {
        u32 fmt =  (u32)ScreenImpl::Bottom->GetFormat();
        u32 fmt2 =  (u32)ScreenImpl::Top->GetFormat();

        MessageBox("Screen fmt", Utils::Format("Top: %d\nBottom: %d", fmt2, fmt))();
    }

    u32     FindBasePtr(u32 pbw)
    {
        u32 *addr = (u32 *)0x00100000;
        u32 totalSize = Process::GetTextSize() + Process::GetRoDataSize() + Process::GetRwDataSize();

        totalSize /= 4;

        u32 *addrEnd = (u32 *)0x00100000 + totalSize;

        while (addr != addrEnd)
        {
            u32 val = *addr++;

            if (Process::CheckAddress(val + 0xE30))
            {

                if (*(vu32 *)(val + 0xE30) == pbw)
                {
                    return (u32)addr;
                }
            }
        }
        return 0;
    }

    // Uncomment to stall process at the beginning until Y is pressed
    //void    DebugFromStart(void){}

    //extern "C" const u8 BottomBackground_bmp[];

    MenuEntry *g_entry = new MenuEntry("Please use Find Base PTR first");

    extern "C" void OSDNotify(const char *str)
    {
        OSD::Notify(str);
    }

    struct KThread{};
    struct KAutoObject{};
    struct HandleDescriptor
    {
        u32             info;
        KAutoObject     *obj;
    } PACKED;

    struct KObjectMutex
    {
        KThread *owner;
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

    KAutoObject * KProcess__GetObjFromHandle(u32 kprocess, Handle handle)
    {
        KAutoObject *(*K_GetObjFromHandle)(u32, Handle) = [](u32 process, Handle handle)
        {
            KProcessHandleTable *table = (KProcessHandleTable *)(process + (SystemImpl::IsNew3DS ? 0xDC : 0xD4));

            return table->handleTable[handle & 0xFFFF].obj;
        };

        return (KAutoObject *)svcCustomBackdoor((void *)K_GetObjFromHandle, kprocess, handle);
    }

    u32     KProcess__GetCurrent(void)
    {
        u32 (*K_GetCurrent)(void) = [](void) -> u32
        {
            return *(vu32 *)0xFFFF9004;
        };

        return (u32)svcCustomBackdoor((void *)K_GetCurrent);
    }

    u32     KProcess__SetCurrent(u32 process)
    {
        u32 (*K_SetCurrent)(u32) = [](u32 newProc) -> u32
        {
            u32 current = *(vu32 *)0xFFFF9004;
            *(vu32 *)0xFFFF9004 = newProc;
            return current;
        };

        return (u32)svcCustomBackdoor((void *)K_SetCurrent, process);
    }

    //  Force kill ntr server
    void    KillNTRSOCHandle(void)
    {
        Handle  menuHandle;
        Handle  handles[0x100] = {0};
        u32     count;

        // Open menu process
        svcOpenProcess(&menuHandle, 0x14);
        // List all handles used by menu process
        count = svcControlProcess(menuHandle, PROCESSOP_GET_ALL_HANDLES, (u32)handles, 0);

        // Only keep handles referencing an soc:U handle
        for (u32 i = 0; i < count; ++i)
        {
            Handle copy;
            char name[12];
            svcCopyHandle(&copy, Process::GetHandle(), handles[i], menuHandle);
            if (R_FAILED(svcControlService(SERVICEOP_GET_NAME, name, copy)) || strcmp("soc:U", name))
                handles[i] = 0;
            svcCloseHandle(copy);
        }

        // Now close all soc handles from menu
        u32 current = KProcess__SetCurrent((u32)KProcess__GetObjFromHandle(KProcess__GetCurrent(), menuHandle));

        for (u32 i = 0; i < count; ++i)
            if (handles[i]) svcCloseHandle(handles[i]);

        KProcess__SetCurrent(current);

        svcCloseHandle(menuHandle);
    }

    /*
    ** Debug svc stubbing
    */
    static vu32 *g_svcTable;
    static u32   g_backupSvc[3];

    void    StubDebugSvc(void)
    {
        if (!g_svcTable)
        {
            g_svcTable = (vu32 *)PA_FROM_VA(0xFFF00000);
            while (*g_svcTable) ++g_svcTable;
        }

        vu32 *svcTable = g_svcTable;
        u32  *backup = g_backupSvc;

        *backup++ = svcTable[0x60];
        *backup++ = svcTable[0x6A];
        *backup = svcTable[0x6B];

        svcTable[0x60] = svcTable[0x6A] = svcTable[0x6B] = svcTable[0x2E];
    }

    void    UnStubDebugSvc(void)
    {
        if (!g_svcTable)
            return;

        vu32 *svcTable = g_svcTable;
        u32  *backup = g_backupSvc;

        svcTable[0x60] = *backup++;
        svcTable[0x6A] = *backup++;
        svcTable[0x6B] = *backup;
    }

    #define REG32(x) *(vu32 *)((x) | (1u << 31))
    u32     cfgProt = REG32(0x10140140);

    // Uncomment to stall process at the beginning until Y is pressed
    //void    DebugFromStart(void){}

    int     main(void)
    {
        PluginMenu  *m = new PluginMenu("Action Replay", 0, 5, 2);
        PluginMenu  &menu = *m;

        Task task([](void* args)  {
            long int r = 0;
            return r;
        } );

        task.Start();
        menu.SynchronizeWithFrame(true);

        int ret = menu.Run();

        delete m;
        // Exit plugin
        return (0);
    }
}
