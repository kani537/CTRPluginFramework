#ifndef CTRPLUGINFRAMEWORKIMPL_OSDIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_OSDIMPL_HPP

#include "types.h"

#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFramework/System/Hook.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFrameworkImpl/Graphics/FloatingButton.hpp"
#include "../../../Sources/ctrulib/internal.h"

#include <string>
#include <list>
#include <queue>


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

            OSDMessage(const std::string &str, const Color &fg, const Color &bg);
        };

        using OSDIter = std::list<OSDMessage>::iterator;

    public:

        static void    Update(void);
        static bool    Draw(void);

        static void    Lock(void);
        static bool    TryLock(void);
        static void    Unlock(void);

        static bool             DrawSaveIcon;
        static bool             MessColors;
        static u32              FramesToPlay;
        static OSDReturn        HookReturn;

        static Hook             OSDHook;
        static RecursiveLock    RecLock;
        static FloatingButton   FloatingBtn;
        static Screen           TopScreen;
        static Screen           BottomScreen;
        static std::list<OSDMessage*>      Notifications;
        static std::vector<OSDCallback>    Callbacks;

        static bool             IsFramePaused;
        static LightEvent       OnNewFrameEvent;
        static LightEvent       OnFramePaused;
        static LightEvent       OnFrameResume;

        static  int     MainCallback(u32 isBottom, int arg2, void *addr, void *addrB, int stride, int format, int arg7);
        static  int     MainCallback2(u32 r0, u32 *params, u32 isBottom, u32 arg);
        static  void    CallbackGlobal(u32 isBottom, void *addr, void *addrB, int stride, int format);
        static  void    UpdateScreens(void);

        static void     WaitFramePaused(void);
        static void     ResumeFrame(const u32 nbFrames = 0);

    private:
        friend class PluginMenu;
        friend class OSD;
        friend void  Initialize(void);

        static  void    _Initialize(void);
    };
}

#endif
