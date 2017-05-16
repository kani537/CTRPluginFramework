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

namespace CTRPluginFramework
{
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

            OSDMessage(std::string str, Color fg, Color bg)
            {
                text = str;
                width = text.size() * 6;
				drawn = false;
                foreground = fg;
                background = bg;
                time = Clock();
            }
        };

		struct	WriteLineInstruction
		{
			int				screen;
			std::string		text;			
			u16				posX;
			u16				posY;
			u16				offset;
			Color			foreground;
			Color			background;

			WriteLineInstruction(int scr, std::string &str, u16 x, u16 y, u16 offs, Color &fg, Color &bg) :
			screen(scr), text(str), posX(x), posY(y), offset(offs), foreground(fg), background(bg)
			{				
			}
		};

        using OSDIter = std::list<OSDMessage>::iterator;

    public:
        static OSDImpl      *GetInstance(void);

		void	Start(void);
		void	Finalize(void);
        void    operator()(void);		

    private:
        friend class PluginMenu;
        friend class OSD;
        friend void  Initialize(void);

        static  void    _Initialize(void);
        static  OSDImpl     *_single;

        OSDImpl(void);
        void            _DrawMessage(OSDIter &iter, int posX, int &posY);
        void            _DrawTop(std::string &text, int posX, int& posY, int offset, Color& fg, Color& bg);
        void            _DrawBottom(std::string &text, int posX, int& posY, Color& fg, Color& bg);
        
        std::list<OSDMessage>   _list;
		bool					_topModified;
		bool					_bottomModified;

    };
}

#endif