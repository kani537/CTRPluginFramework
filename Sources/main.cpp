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
#include "OSDManager.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include <list>

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

    // This function is called on the plugin starts, before main
    void    PatchProcess(FwkSettings &settings)
    {
        ToggleTouchscreenForceOn();
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

    // Warning: This class do not own any data, be sure that the source stays available
    class String
    {
    public:
        String(void);
        String(const char *src);
        explicit String(const std::string &src);

        // Implicit cast to const char *
        operator const char *() const;

        class const_iterator : std::iterator<
                        std::random_access_iterator_tag,   // iterator_category
                        char,                      // value_type
                        u32,                       // difference_type
                        const char *,              // pointer
                        const char                 // reference
                                      >
        {
        public:
            // Ctor
            explicit const_iterator(const char *pos);

            // Increment
            const_iterator& operator++(void);
            const_iterator  operator++(int);
            const_iterator& operator+=(const u32 offset);
            const_iterator  operator+ (const u32 offset) const;

            // Decrement
            const_iterator& operator--(void);
            const_iterator  operator--(int);
            const_iterator& operator-=(const u32 offset);
            const_iterator  operator- (const u32 offset) const;
            u32             operator- (const const_iterator &right) const;

            // Tests
            bool            operator==(const const_iterator &right) const;
            bool            operator!=(const const_iterator &right) const;
            bool            operator< (const const_iterator &right) const;
            bool            operator<=(const const_iterator &right) const;
            bool            operator> (const const_iterator &right) const;
            bool            operator>=(const const_iterator &right) const;

            // Accessor
            char            operator*(void) const;
            char            operator[](const u32 offset) const;

        private:
            const char *_val;
        };

        const_iterator  begin(void) const;
        const_iterator  end(void) const;
        u32             size(void) const;

        // Accessor
        char            operator[](const u32 offset) const;
        const char *    data(void) const;

        // Assignment
        String &        operator=(const char * const right);
        String &        operator=(const std::string &right);

    private:

        const char *_src;
        u32         _size;
    };

    String::String(void) : _src{ nullptr }, _size{ 0 }
    {
    }

    String::String(const char *src) : _src{ src }, _size{ strlen(src) }
    {
    }

    String::String(const std::string &src) : _src{ src.c_str() }, _size{ src.size() }
    {
    }

    String::operator const char*() const
    {
        return _src;
    }

    String::const_iterator::const_iterator(const char *pos) :
        _val{ pos }
    {
    }

    String::const_iterator & String::const_iterator::operator++(void)
    {
        ++_val;
        return *this;
    }

    String::const_iterator String::const_iterator::operator++(int)
    {
        const_iterator old(_val);

        ++_val;
        return old;
    }

    String::const_iterator & String::const_iterator::operator+=(const u32 offset)
    {
        _val += offset;
        return *this;
    }

    String::const_iterator String::const_iterator::operator+(const u32 offset) const
    {
        return const_iterator(_val + offset);
    }

    String::const_iterator & String::const_iterator::operator--(void)
    {
        --_val;
        return *this;
    }

    String::const_iterator String::const_iterator::operator--(int)
    {
        const_iterator old(_val);

        --_val;
        return old;
    }

    String::const_iterator & String::const_iterator::operator-=(const u32 offset)
    {
        _val -= offset;
        return *this;
    }

    String::const_iterator String::const_iterator::operator-(const u32 offset) const
    {
        return const_iterator(_val - offset);
    }

    u32 String::const_iterator::operator-(const const_iterator &right) const
    {
        return _val - right._val;
    }

    bool String::const_iterator::operator==(const const_iterator &right) const
    {
        return _val == right._val;
    }

    bool String::const_iterator::operator!=(const const_iterator &right) const
    {
        return _val != right._val;
    }

    bool String::const_iterator::operator<(const const_iterator &right) const
    {
        return _val < right._val;
    }

    bool String::const_iterator::operator<=(const const_iterator &right) const
    {
        return _val <= right._val;
    }

    bool String::const_iterator::operator>(const const_iterator &right) const
    {
        return _val > right._val;
    }

    bool String::const_iterator::operator>=(const const_iterator &right) const
    {
        return _val >= right._val;
    }

    char String::const_iterator::operator*() const
    {
        // TODO: Some security check ?
        return *_val;
    }

    char String::const_iterator::operator[](const u32 offset) const
    {
        // TODO: Some security check
        return _val[offset];
    }

    String::const_iterator String::begin(void) const
    {
        return const_iterator(_src);
    }

    String::const_iterator String::end(void) const
    {
        return const_iterator(_src + _size);
    }

    u32 String::size(void) const
    {
        return _size;
    }

    char String::operator[](const u32 offset) const
    {
        // TODO: Some security check ?
        return _src[offset >= _size ? _size - 1 : offset];
    }

    const char * String::data() const
    {
        return _src;
    }

    String & String::operator=(const char * const right)
    {
        _src = right;
        _size = strlen(right);
        return *this;
    }

    String & String::operator=(const std::string &right)
    {
        _src = right.c_str();
        _size = right.size();
        return *this;
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

    struct ITrace
    {
        u32     address{0};
        u32     value{0};
        ITrace *offset{nullptr};
        ~ITrace(void)
        {
            OSDManager.Remove(address);
            if (offset)
                delete offset;
        }
    };

    std::vector<ITrace> _itraces;

    int     main(void)
    {
        PluginMenu  *m = new PluginMenu("Action Replay", 0, 5, 1);
        PluginMenu  &menu = *m;

        menu.SynchronizeWithFrame(true);
        /*menu.SynchronizeWithFrame(false);
        menu += [](void)
        {
            Sleep(Milliseconds(1));
        };*/

        //menu += new MenuEntry("Check screen fmt", nullptr, CheckScreenFormat);
        // Launch menu and mainloop
        int ret = menu.Run();

        delete m;
        // Exit plugin
        return (0);
    }
}
