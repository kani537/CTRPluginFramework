#ifndef CTRPLUGINFRAMEWORK_RENDERER_HPP
#define CTRPLUGINFRAMEWORK_RENDERER_HPP

#include "types.h"
#include "Color.hpp"
#include "Screen.hpp"
#include "3DS.h"
#include <cstdio>

namespace CTRPluginFramework
{
    enum Target
    {
        BOTTOM = 0,
        TOP = 1
    };
    class Renderer;
    typedef void (*DrawPixelP)(int, int, Color);
    typedef void (*DrawDataP)(int, int, u8*, int);
    class Renderer
    {
    public:
        


        static void     SetTarget(Target target);
        static void     UpdateTarget(void);
        static void     StartRenderer(bool current = false);
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
        static void     DrawFile(std::FILE *file, int posX, int posY, int width, int height);
        static void     DrawBuffer(u8 *buffer, int posX, int posY, int width, int height);
        //
        static int      DrawString(char *str, int posX, int &posY, Color fg);
        static int      DrawString(char *str, int posX, int &posY, Color fg, Color bg);
        static int      DrawString(char *str, int offset, int posX, int &posY, Color fg);
        static void      DrawCheckBoxString(char *str, int posX, int &posY, bool isChecked, Color fg, Color checked);

        static void            DrawSysString(const char *str, int posX, int &posY, int max, Color color, float offset = 0, bool autoReturn = false);
        static float GetTextSize(const char *text);
    private:
        friend void     Initialize(void);
        // Initalize Renderer
        static void     Initialize(void);
        // Allocate buffer to process files in ram
        static void     InitBuffer(u32 size);
        // Calulate sysfont glyph
        static void     FontCalcGlyphPos(fontGlyphPos_s* out, int glyphIndex, float scaleX, float scaleY);
        // Draw glyph
        static uint8_t  DrawGlyph(uint16_t x, uint16_t y, u32 glyphCode, Color color, float offset);
        static u8       *DrawTile(u8 *tile, u8 iconsize, u8 tilesize, u16 startX, \
 u16 startY, u16 endX, u16 endY, u8 charWidth, u8 charHeight, Color color);


        static Target       _target;
        static bool         _render3D;
        static bool         _isRendering;
        static bool         _doubleBuffer;

        static Screen       *_screenTarget[2];

        static u8           *_framebuffer[4];
        static u8           *_framebufferR[4];
        static u32          _rowSize[2];
        static u32          _targetWidth[2];
        static u32          _targetHeight[2];
        static u8           _smallBuffer[1000];
        static u8           *_buffer;
        static u32          _bufferSize;
        static int          _length;

        static void         RenderRGBA8(int posX, int posY, Color color);
        static void         RenderBGR8(int posX, int posY, Color color);
        static void         RenderRGB565(int posX, int posY, Color color);
        static void         RenderRGB5A1(int posX, int posY, Color color);
        static void         RenderRGBA4(int posX, int posY, Color color);

        static void         RenderRGBA8(int posX, int posY, u8 *data, int height);
        static void         RenderBGR8(int posX, int posY, u8 *data, int height);
        static void         RenderRGB565(int posX, int posY, u8 *data, int height);
        static void         RenderRGB5A1(int posX, int posY, u8 *data, int height);
        static void         RenderRGBA4(int posX, int posY, u8 *data, int height);

        static DrawPixelP   _DrawPixel;
        static DrawDataP    _DrawData;

    };
}

#endif 