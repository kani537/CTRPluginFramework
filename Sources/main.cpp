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

namespace CTRPluginFramework
{
    s64 g_free;
    // This function is called on the plugin starts, before main
    void    PatchProcess(FwkSettings &settings)
    {
        g_free = osGetMemRegionFree(MEMREGION_APPLICATION);
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

    int     main(void)
    {
        PluginMenu  *m = new PluginMenu("Action Replay", 1, 2, 0);
        PluginMenu  &menu = *m;

        menu.SynchronizeWithFrame(true);

        menu += new MenuEntry("Test", nullptr, [](MenuEntry *entry)
        {
            s64 free = osGetMemRegionFree(MEMREGION_APPLICATION);
            u32 vramPa = svcConvertVAToPA((void *)0x1F000000, false);

            MessageBox(Utils::Format("Appmem free: %llX\nAppmem free: %llX", g_free, free))();
            MessageBox(Utils::Format("vram pa: %08X", vramPa))();

        });

        // Launch menu and mainloop
        int ret = menu.Run();

        delete m;
        // Exit plugin
        return (0);
    }
}
