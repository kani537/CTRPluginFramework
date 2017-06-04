#ifndef CTRPLUGINFRAMEWORKIMPL_WINDOW_HPP
#define CTRPLUGINFRAMEWORKIMPL_WINDOW_HPP

#include "CTRPluginFramework/System/Vector.hpp"
#include "CTRPluginFramework/System/Rect.hpp"
#include "IconButton.hpp"

namespace CTRPluginFramework
{
    class BMPImage;
    class Window
    {
    public:

        Window(u32 posX, u32 posY, u32 width, u32 height, bool closeBtn, BMPImage *image);
        ~Window(void);

        void    Draw(void) const;
        void    Update(bool isTouched, IntVector touchPos) const;
        bool    MustClose(void) const;

        static Window   TopWindow;
        static Window   BottomWindow;

    private:

        IntRect     _rect;
        IntRect     _border;
        IconButton<Window, void>    *_closeBtn;
        BMPImage    *_image;

        friend void     ThreadInit(void *arg);
        static void     Initialize(void);
    };
}

#endif