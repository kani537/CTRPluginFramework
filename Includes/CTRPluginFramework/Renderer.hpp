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
        static void     StartRenderer(void);
        static void     EndRenderer(void);
        static void     GetFramebuffersInfos(u32 *infos);
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
        friend void     Initialize(void);
        static void     Initialize(void);
        static void     FlushAndSwap(void);

        static Target       _target;
        static bool         _render3D;
        static bool         _isRendering;
        static Screen       *_screenTarget[2];
        static u8           *_framebuffer[2];
        static u8           *_framebufferR[2];
        static u32          _rowSize[2];
        static u32          _targetWidth[2];
        static u32          _targetHeight[2];
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