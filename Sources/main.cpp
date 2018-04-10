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

#include "Hook.hpp"
#include "3DS.h"
#include "CTRPluginFramework.hpp"
#include <string>
#include <iterator>

#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(FwkSettings &settings)
    {
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

    static void     ListFolders(MenuFolderImpl &folder, const std::string &filter)
    {
        std::string     &path = folder.note;
        Directory       dir;
        std::vector<std::string>    list;

        int res = Directory::Open(dir, path);

        if (res != 0)
            MessageBox("Error", Utils::Format("%d - %s", res, path.c_str()))();

        if (path.size() > 1 && path[path.size() - 1] != '/')
            path.append("/");
        if (dir.ListDirectories(list) > 0)
        {
            for (std::string &name : list)
            {
                folder.Append(new MenuFolderImpl(name, path + name));
            }
        }

        list.clear();

        if (dir.ListFiles(list, filter) > 0)
        {
            for (std::string &name : list)
            {
                folder.Append(new MenuEntryImpl(name));
            }
        }
    }

    int    SDExplorer(std::string &out, const std::string &filter = "")
    {
        Menu            menu("/", "", Icon::DrawFile);
        MenuFolderImpl  &root = *menu.GetRootFolder();

        // Use note to store current path
        root.note = "/";

        // List root's items
        ListFolders(root, filter);

        // Open menu
        int             menuEvent = Nothing;
        Event           event;
        EventManager    eventManager;
        MenuItem        *item;
        Clock           clock;

        do
        {
            menuEvent = Nothing;
            while (eventManager.PollEvent(event) && menuEvent == Nothing)
                menuEvent = menu.ProcessEvent(event, &item);

            if (menuEvent == FolderChanged)
            {
                MenuFolderImpl *folder = reinterpret_cast<MenuFolderImpl *>(item);

                if (!folder->ItemsCount())
                {
                    ListFolders(*folder, filter);
                }
            }

            menu.Update(clock.Restart());
            menu.Draw();
            Renderer::EndFrame();
        } while (menuEvent != EntrySelected && menuEvent != MenuClose);

        if (menuEvent == MenuClose)
            return -1;

        out = menu.GetFolder()->note;
        out.append("/");
        out.append(menu.GetSelectedItem()->name);
        return 0;
    }

    #define DEBUG 1
#if DEBUG
#   define TRACE Debug_Trace()
#else
#   define TRACE
#endif
    static std::string g_abortDebugStr;

    void    Debug_Trace(void)
    {
        g_abortDebugStr = Utils::Format("%s: %s:%d", __FILE__, __FUNCTION__, __LINE__);
    }

    void    OnAbort(void)
    {
        OSD::Run([](const Screen &screen)
        {
            if (screen.IsTop)
                return false;

            screen.Draw("std::abort!!!", 10, 10, Color::Blank, Color::Red);
            screen.Draw("Last function: " + g_abortDebugStr, 10, 20, Color::Blank, Color::Black);
            return true;
        });

        for (;;); // Don't exit this function or the plugin will be shutdown (threads stopped and OSD unhooked)
    }
    extern "C" void invincibleHooked(void);
    int     main(void)
    {
        PluginMenu  *m = new PluginMenu("Action Replay", 1, 0, 5);
        PluginMenu  &menu = *m;

        menu.SynchronizeWithFrame(true);

        menu += new MenuEntry("SDExplorer test", nullptr,
        [](MenuEntry *entry)
        {
            std::string path;

            if (!SDExplorer(path, ".txt"))
                MessageBox("Info", "You chose: " + path)();
        });

        menu += new MenuEntry("Invincible", [](MenuEntry *entry)
        {
            static Hook hook;

            if (entry->WasJustActivated())
            {
                // No need as I want to replace it
                hook.flags.ExecuteOverwrittenInstructionBeforeCallback = false;
                hook.Initialize(0x00352E24, reinterpret_cast<u32>(invincibleHooked));
                hook.Enable();
            }

            if (!entry->IsActivated())
                hook.Disable();
        });

        menu += new MenuEntry("Notify", [](MenuEntry *entry)
        {
            auto osd = [](const Screen &screen)
            {
                screen.Draw("Hello world !", 10, 10);
                return true;
            };
            if (entry->WasJustActivated())
                OSD::Run(osd);
            if (!entry->IsActivated())
                OSD::Stop(osd);
        });

#if DEBUG
        System::OnAbort = OnAbort;
#endif

        // Launch menu and mainloop
        int ret = menu.Run();

        delete m;
        // Exit plugin
        return (0);
    }
}
