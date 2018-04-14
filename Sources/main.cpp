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

struct Course
{
    const char  *name;
    const u8    id;
};

static const Course g_courses[] =
{
    { "Mario Circuit", 0x00 },
    { "Alphine Pass", 0x01 },
    { "Cheep Cheep Lagoon", 0x02 },
    { "Daisy Hills", 0x03 },
    { "Toad Circuit", 0x04 },
    { "Shyguy Bazaar", 0x05 },
    { "Koopa City", 0x06 },
    { "DK Jungle", 0x07 },
    { "Wuhu Island Loop", 0x08 },
    { "Wuhu Mountain Loop", 0x09 },
    { "Rosalina Ice World", 0x0A },
    { "Bowser Castle", 0x0B },
    { "Piranha Plant Slide", 0x0C },
    { "Rainbow Road", 0x0D },
    { "Wario Shipwreck", 0x0E },
    { "Melody Motorway", 0x0F },
    { "Wii Coconut Mall", 0x10 },
    { "WIi Koopa Cape", 0x11 },
    { "Wii Maple Treeway", 0x12 },
    { "Wii Mushroom Gorge", 0x13 },
    { "DS Luigi Mansion", 0x14 },
    { "DS Airship Fortress", 0x15},
    { "DS DK Pass", 0x16 },
    { "DS Waluigi Pinball", 0x17 },
    { "GCN Dino Dino Jungle", 0x18 },
    { "GCN Daisy Cruiser", 0x19 },
    { "N64 Luigi Raceway", 0x1A },
    { "N64 Kalamari Desert", 0x1B },
    { "N64 Koopa Troopa Beach", 0x1C },
    { "GBA Bowser Castle 1", 0x1D },
    { "SNES Mario Circuit 2", 0x1E },
    { "SNES Rainbow Road", 0x1F },
    { "Wuhu Town", 0x20 },
    { "Honey Hive", 0x21 },
    { "Ice Rink", 0x22 },
    { "Palm Shore", 0x23 },
    { "Big Donut", 0x24 },
    { "Battle Course 1", 0x25 },
    { "Winning Run", 0x26 },
    { "Test", 0x27 },
    { "150cc", 0x28 },
    { "100cc", 0x29 },
    { "50cc", 0x2A }
};

    int     main(void)
    {
        for (const Course &course : g_courses)
            ;

        PluginMenu  *m = new PluginMenu("Action Replay", 1, 0, 5);
        PluginMenu  &menu = *m;

        menu.SynchronizeWithFrame(true);

        menu += new MenuEntry("SD Browser", nullptr, [](MenuEntry *entry)
        {
            //MessageBox(Utils::Format("%08X", getMemFree()))();
            std::string out;
            Utils::SDExplorer(out);
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
