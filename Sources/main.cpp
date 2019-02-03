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

   u32 mainTls;
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

    extern "C"
    {
        void dbgReturnFromExceptionDirectly(CpuRegisters *regs);
    }

    // Uncomment to stall process at the beginning until Y is pressed
    //void    DebugFromStart(void){}

    //extern "C" const u8 BottomBackground_bmp[];

    MenuEntry *g_entry = new MenuEntry("Please use Find Base PTR first");

    #define REG32(x) *(vu32 *)((x) | (1u << 31))
    u32     cfgProt = REG32(0x10140140);

    // Uncomment to stall process at the beginning until Y is pressed
    //void    DebugFromStart(void){}

    void __ExceptionHandler(ERRF_ExceptionInfo* excep, CpuRegisters* regs)
    {
        std::string str = Color::Red << "Exception: ";

        if (excep->type == ERRF_EXCEPTION_DATA_ABORT)
            str += Color::Orange << "Data Abort" << Color::White;

        str += Utils::Format(", pc: %08X", regs->pc);
        OSD::Notify(str);
        regs->pc += 4;
        OSD::Notify("Return from exception: " << Color::LimeGreen << "SUCCESS");
        dbgReturnFromExceptionDirectly(regs);
    }

    Result  MyHookedCode(u32 a1, u32 a2)
    {
        // Get the current hook context without the need for a global
        HookContext& ctx = HookContext::GetCurrent();

        Result res = ctx.OriginalFunction<Result>(a1, a2);

        return res;
    }

    int     main(void)
    {
        PluginMenu  *m = new PluginMenu("Action Replay", 0, 5, 2);
        PluginMenu  &menu = *m;

        menu.SynchronizeWithFrame(true);

        menu += new MenuEntry("View TLS", nullptr, [](MenuEntry *entry)
        {
            Utils::OpenInHexEditor(mainTls, HexEditorView::Integer);
        });

        int ret = menu.Run();


        delete m;
        // Exit plugin
        return (0);
    }
}
