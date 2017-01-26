#ifndef CTRPLUGINFRAMEWORK_RENDERER_HPP
#define CTRPLUGINFRAMEWORK_RENDERER_HPP

#include "types.h"
#include "Color.hpp"
#include "Screen.hpp"
#include "3DS.h"
#include "Vector.h"
#include "Rect.hpp"
#include <cstdio>
#include <algorithm>

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
    #define     RANGE(x, y, z) (y >= x && y <= z)
    class Renderer
    {
    public:
        
        static void     SetTarget(Target target);
        static void     StartFrame(bool current = false);
        static void     EndFrame(void);

        // Forms
        //#############################################################################################
        static void     DrawLine(int posX, int posY, int length, Color color, int width = 1);
        static void     DrawLine(IntVector start, IntVector end, Color color);
        static void     DrawRect(int posX, int posY, int width, int height, Color color, bool fill = true, int thickness = 1);
        template <typename T>
        static void     Line(Vector<T> &start, Vector<T> &end, Color color);  
        static void     Arc(int x, int y, int r, Color color);      
        static void     Ellipse(int posX, int posY, long a, long b, Color color);
        static void     EllipseIncomplete(int posX, int posY, float a, float b, int max, int aff, Color color);
        static void     RoundedRectangle(const IntRect &rect, float radius, int max, Color color, bool mustFill = false, Color fillColor = Color());
        //static void     FormFiller(int posX,int posY, Color &fillColor, Color &limit) ;
        static    void    FormFiller(const IntVector &start, const IntRect &area, bool singlePoint, Color &fill, Color &limit);
        // Menu
        //#############################################################################################
        static    void        MenuSelector(int posX, int posY, int width, int height);
        // Linux Font
        //#############################################################################################
        // Draw Character without background
        static void     DrawCharacter(int c, int posX, int posY, Color fg);
        // Draw Character with background
        static void     DrawCharacter(int c, int posX, int posY, Color fg, Color bg);
        // Draw Character with offset
        static void     DrawCharacter(int c, int offset, int posX, int posY, Color fg);
        //
        static int      DrawString(char *str, int posX, int &posY, Color fg);
        static int      DrawString(char *str, int posX, int &posY, Color fg, Color bg);
        static int      DrawString(char *str, int offset, int posX, int &posY, Color fg);
        static void     DrawCheckBoxString(char *str, int posX, int &posY, bool isChecked, Color fg, Color checked);
        // System Font
        //#############################################################################################
        static int      DrawSysString(const char *str, int posX, int &posY, int max, Color color, float offset = 0, bool autoReturn = false);
        static float    GetTextSize(const char *text);

        static void     DrawSysCheckBox(const char *str, int posX, int &posY, int xLimits, Color color, bool isChecked = false,  float offset = 0);
        static void     DrawSysFolder(const char *str, int posX, int &posY, int xLimits, Color color, float offset = 0);

        // Misc
        //#############################################################################################
        static void     DrawFile(std::FILE *file, int posX, int posY, int width, int height);
        static void     DrawBuffer(u8 *buffer, int posX, int posY, int width, int height);

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
        static u8       *DrawTile(u8 *tile, u8 iconsize, u8 tilesize, u16 startX, u16 startY, u16 endX, u16 endY, u8 charWidth, u8 charHeight, Color color);


        static Target       _target;
        static bool         _useRender3D;
        static bool         _isRendering;
        static bool         _useDoubleBuffer;
        static bool         _useSystemFont;

        static Screen       *_screens[2];

        //static u8           *_framebuffer[4];
        //static u8           *_framebufferR[4];
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

    template <typename T>
    void Renderer::Line(Vector<T> &start, Vector<T> &end, Color color)
    {
        int dx;
        int dy;
        int i;
        int xinc;
        int yinc;
        int cumul;
        int x;
        int y;

        x = static_cast<int>(start.x);
        y = static_cast<int>(start.y);
        // Correct posY
        y += (_rowSize[_target] - 240);
        dx = static_cast<int>(end.x - start.x);
        dy = static_cast<int>(end.y - start.y);
        xinc = ( dx > 0 ) ? 1 : -1;
        yinc = ( dy > 0 ) ? 1 : -1;
        dx = abs(dx) ;
        dy = abs(dy) ;

        _DrawPixel(x,y, color);

        if ( dx > dy )
        {
            cumul = dx / 2 ;
            for ( i = 1 ; i <= dx ; i++ )
            {
                x += xinc ;
                cumul += dy ;
                if ( cumul >= dx ) 
                {
                    cumul -= dx ;
                    y += yinc ; 
                }
                _DrawPixel(x,y, color); 
            } 
        }
        else 
        {
            cumul = dy / 2 ;
            for ( i = 1 ; i <= dy ; i++ ) 
            {
                y += yinc ;
                cumul += dx ;
                if ( cumul >= dy ) 
                {
                    cumul -= dy ;
                    x += xinc ; 
                }
                _DrawPixel(x,y, color); 
            }
        }
    }
}

#endif 