#ifndef CTRPLUGINFRAMEWORKIMPL_OSDIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_OSDIMPL_HPP

#include "types.h"

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "CTRPluginFramework/System/Clock.hpp"

#include <iterator>
#include <string>
#include <list>
#include <queue>
#include "../../../Sources/ctrulib/internal.h"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "Hook.hpp"

namespace CTRPluginFramework
{
    using OSDReturn = int(*)(u32, int, void *, void *, int, int, int);

    class OSDImpl
    {
        struct  OSDMessage
        {
            std::string     text;
            int             width;
			bool			drawn;
            Color           foreground;
            Color           background;
            Clock           time;

            OSDMessage(const std::string &str, const Color &fg, const Color &bg)
            {
                text = str;
                width = text.size() * 6;
				drawn = false;
                foreground = fg;
                background = bg;
                time = Clock();
            }
        };

        using OSDIter = std::list<OSDMessage>::iterator;

    public:

        static void    Update(void);
        static bool    Draw(void);
        
        static void    Lock(void);
        static bool    TryLock(void);
        static void    Unlock(void);
        
        static  bool            DrawSaveIcon;
        static  Hook            OSDHook;
        static  RecursiveLock   RecLock;
        static  std::list<OSDMessage*>      Notifications;
        static  std::vector<OSDCallback>    Callbacks;

        static  int MainCallback(u32 isBottom, int arg2, void *addr, void *addrB, int stride, int format, int arg7);

    private:
        friend class PluginMenu;
        friend class OSD;
        friend void  Initialize(void);

        static  void    _Initialize(void);
    };
}

#endif
