#ifndef CTRPLUGINFRAMEWORK_OSD_HPP
#define CTRPLUGINFRAMEWORK_OSD_HPP

#include "types.h"

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "CTRPluginFramework/Clock.hpp"

#include <iterator>
#include <string>
#include <list>

namespace CTRPluginFramework
{
    class OSDImpl
    {
        struct  OSDMessage
        {
            std::string     text;
            int             width;
            Color           foreground;
            Color           background;
            Clock           time;

            OSDMessage(std::string str, Color fg, Color bg)
            {
                text = str;
                width = text.size() * 6;
                foreground = fg;
                background = bg;
                time = Clock();
            }
        };

        using OSDIter = std::list<OSDMessage>::iterator;

    public:
        static OSDImpl      *GetInstance(void);

        void    operator()(void);

    private:
        friend class PluginMenu;
        friend void  Initialize(void);

        static  void    _Initialize(void);
        static  OSDImpl     *_single;

        OSDImpl(void);
        void    _DrawMessage(OSDIter &iter, int posX, int &posY);
        std::list<OSDMessage>   _list;

    };
}

#endif