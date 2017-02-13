#ifndef CTRPLUGINFRAMEWORK_RENDERER_HPP
#define CTRPLUGINFRAMEWORK_RENDERER_HPP

#include "types.h"

#include "ctrulib/font.h"
#include "ctrulib/gfx.h"

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Vector.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Rect.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Line.hpp"

#include <algorithm>

namespace CTRPluginFramework
{
    
    #define     RANGE(x, y, z) (y >= x && y <= z)

    enum Target
    {
        BOTTOM = 0,
        TOP = 1
    };
    
    using DrawPixelP = void (*)(int, int, Color&);
    using DrawDataP = void (*)(int, int, u8*, int);
    
    class Icon;
    class Screen;
    class TextBox;
    class Renderer
    {
    public:
        
        static void     SetTarget(Target target);
        static void     StartFrame(bool current = false);
        static void     EndFrame(bool copy = false);
        static void     UseDoubleBuffer(bool useIt);
        // Forms
        //#############################################################################################
        static void     DrawLine(int posX, int posY, int length, Color color, int width = 1);
        static void     DrawLine(IntVector &start, IntVector &end, Color color);
        static void     DrawRect(int posX, int posY, int width, int height, Color color, bool fill = true, int thickness = 1);
        static void     DrawRect(const IntRect &rect, Color &color, bool fill = true);
        static void     DrawRect2(const IntRect &rect, Color &color1, Color &color2);
        template <typename T>
        static void     Line(Vector<T> &start, Vector<T> &end, Color color);  
        static void     Arc(int x, int y, int r, Color color);      
        static void     Ellipse(int posX, int posY, long a, long b, Color color);
        static void     EllipseIncomplete(int posX, int posY, float a, float b, int max, int aff, Color color);
        static void     RoundedRectangle(const IntRect &rect, float radius, int max, Color color, bool mustFill = false, Color fillColor = Color());
        static     void  ComputeRoundedRectangle(std::vector<IntLine> &out, const IntRect &rect, float radius, int max);
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
        static int      DrawSysString(const char *str, int posX, int &posY, int max, Color color, float offset = 0, const char *end = nullptr);
        static float    GetTextSize(const char *text);
        static int      GetLineCount(const char *text, float maxWidth);

        static void     DrawSysCheckBox(const char *str, int posX, int &posY, int xLimits, Color color, bool isChecked = false,  float offset = 0);
        static void     DrawSysFolder(const char *str, int posX, int &posY, int xLimits, Color color, float offset = 0);

        // Misc
        //#############################################################################################
        static void     DrawBuffer(u8 *buffer, int posX, int posY, int width, int height);
        static DrawPixelP   _DrawPixel;
    private:

        friend void     Initialize(void);
        friend class    Icon;

        friend class    TextBox;
        friend class    BMPImage;

        // Initalize Renderer
        static void     Initialize(void);
        // Allocate buffer to process files in ram
        static void     InitBuffer(u32 size);
        // Calulate sysfont glyph
        static void     FontCalcGlyphPos(fontGlyphPos_s *out,  charWidthInfo_s **cwout, int glyphIndex, float scaleX, float scaleY);
        // Draw glyph
        static uint8_t  DrawGlyph(fontGlyphPos_s &pos,  charWidthInfo_s *cwi, uint16_t x, uint16_t y, Color color, float offset);
        static u8       *DrawTile(u8 *tile, u8 iconsize, u8 tilesize, u16 startX, u16 startY, u16 endX, u16 endY, u8 charWidth, u8 charHeight, Color color);

        static Target       _target;
        
        static bool         _isRendering;
        static bool         _useDoubleBuffer;
        //static bool         _useSystemFont;

        //static Screen       *_screens[2];
        static Screen       *_screen;

        //static u8           *_framebuffer[4];
        //static u8           *_framebufferR[4];
        static u32          _rowstride;
        static u8           _smallBuffer[1000];
        static GSPGPU_FramebufferFormats _format;
        static u8           *_buffer;
        static u32          _bufferSize;
        static int          _length;

        static void         RenderRGBA8(int posX, int posY, Color &color);
        static void         RenderBGR8(int posX, int posY, Color &color);
        static void         RenderRGB565(int posX, int posY, Color &color);
        static void         RenderRGB5A1(int posX, int posY, Color &color);
        static void         RenderRGBA4(int posX, int posY, Color &color);

        
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