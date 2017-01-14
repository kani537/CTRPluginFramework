#ifndef CTRPLUGINFRAMEWORK_RENDERER_HPP
#define CTRPLUGINFRAMEWORK_RENDERER_HPP

#include "types.h"
#include "Color.hpp"
#include "Screen.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    enum Target
    {
        BOTTOM = 0,
        TOP = 1
    };
    class Renderer;
    typedef void (*DrawPixelP)(int, int, Color);
    class Renderer
    {
    public:
        


        static void     SetTarget(Target target);
        static void     UpdateTarget(void);
        static void     StartRendering(void);
        static void     EndRendering(void);
        static void     DrawLine(int posX, int posY, int length, Color color, int width = 1);
        static void     DrawRect(int posX, int posY, int width, int height, Color color, bool fill = true, int thickness = 1);

        // Draw Character without background
        static void     DrawCharacter(int c, int posX, int posY, Color fg);
        // Draw Character with background
        static void     DrawCharacter(int c, int posX, int posY, Color fg, Color bg);
        // Draw Character with offset
        static void     DrawCharacter(int c, int offset, int posX, int posY, Color fg);

        //
        static void     DrawString(char *str, int posX, int posY, Color fg);
        static void     DrawString(char *str, int posX, int posY, Color fg, Color bg);
        static void     DrawString(char *str, int offset, int posX, int posY, Color fg);
    private:

        static Target       _target;
        static bool         _render3D;
        static Screen       *_screenTarget;
        static u8           *_framebuffer;
        static u8           *_framebufferR;
        static u8           *_framebufferP;
        static u8           *_framebufferRP;
        static u32          _rowSize;
        static u32          _targetWidth;
        static u32          _targetHeight;
        static int          _length;

        static void         RenderRGBA8(int posX, int posY, Color color);
        static void         RenderBGR8(int posX, int posY, Color color);
        static void         RenderRGB565(int posX, int posY, Color color);
        static void         RenderRGB5A1(int posX, int posY, Color color);
        static void         RenderRGBA4(int posX, int posY, Color color);

        static DrawPixelP   _DrawPixel;

    };
}

#endif 