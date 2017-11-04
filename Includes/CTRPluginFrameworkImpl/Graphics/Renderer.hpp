#ifndef CTRPLUGINFRAMEWORK_RENDERER_HPP
#define CTRPLUGINFRAMEWORK_RENDERER_HPP

#include "types.h"

#include "ctrulib/font.h"
#include "ctrulib/gfx.h"

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Vector.hpp"
#include "CTRPluginFramework/System/Rect.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Line.hpp"

#include <vector>
#include "Font.hpp"

#define GEOMETRY 0

namespace CTRPluginFramework
{

#define     RANGE(x, y, z) (y >= x && y <= z)

    enum Target
    {
        BOTTOM = 0,
        TOP = 1
    };

    using DrawPixelP = void(*)(int, int, Color&);
    using DrawDataP = void(*)(int, int, u8*, int);

    class Icon;
    class ScreenImpl;
    class TextBox;
    class Renderer
    {
    public:

        static void     SetTarget(Target target);
        static void     EndFrame(bool copy = false);

        // Forms
        //#############################################################################################
        static void     DrawPixel(int posX, int posY, const Color &color);
        static void     DrawLine(int posX, int posY, int length, const Color &color, int width = 1);
        static void     DrawLine(IntVector &start, IntVector &end, const Color &color);
        static void     DrawStippledLine(int posX, int posY, int width, const Color &color, int height);
        static void     DrawRect(int posX, int posY, int width, int height, const Color &color, bool fill = true, int thickness = 1);
        static void     DrawRect(const IntRect& rect, const Color& color, bool fill = true);
        static void     DrawRect2(const IntRect &rect, const Color &color1, const Color &color2);

#if GEOMETRY
        template <typename T>
        static void     Line(Vector<T> &start, Vector<T>    &end, Color color);
        static void     Arc(int x, int y, int r, Color color);
        static void     Ellipse(int posX, int posY, long a, long b, Color color);
        static void     EllipseIncomplete(int posX, int posY, float a, float b, int max, int aff, Color color);
        static void     RoundedRectangle(const IntRect &rect, float radius, int max, Color color, bool mustFill = false, Color fillColor = Color());
#endif

        static    void  ComputeRoundedRectangle(std::vector<IntLine> &out, const IntRect &rect, float radius, int max);
        // Menu
        //#############################################################################################
        static    void        MenuSelector(int posX, int posY, int width, int height);
        
        // Linux Font
        //#############################################################################################
        // Draw Character without background
        static void     DrawCharacter(int c, int posX, int posY, const Color &fg);
        // Draw Character with background
        static void     DrawCharacter(int c, int posX, int posY, const Color &fg, const Color &bg);

        static int      DrawString(const char *str, int posX, int &posY, Color fg);
        static int      DrawString(const char *str, int posX, int &posY, Color fg, Color bg);

        // System Font
        //#############################################################################################
        static int      DrawSysString(const char *str, int posX, int &posY, int max, Color color, float offset = 0.f, const char *end = nullptr);
        static int      DrawSysStringReturn(const unsigned char* stri, int posX, int& posY, int xLimits, Color color, int maxY = 200);
        static float    GetTextSize(const char *text);
        static int      GetLineCount(const char *text, float maxWidth);
        static void     GetTextInfos(const char* text, int& lineCount, float& lineMaxWidth, float maxWidth);
        static  u32     LinuxFontSize(const char *str);

        static void     DrawSysCheckBox(const char *str, int posX, int &posY, int xLimits, Color color, bool isChecked = false, float offset = 0);
        static void     DrawSysFolder(const char *str, int posX, int &posY, int xLimits, Color color, float offset = 0);
        static int      DrawGlyph(Glyph* glyph, int posX, int posY, Color color);
        static int      DrawGlyph(Glyph* glyph, int posX, int posY, float& offset, Color color);
        // Misc
        //#############################################################################################
    private:

        friend void     Initialize(void);
        friend class    TouchKey;
        friend class    Icon;
        friend class    PrivColor;
        friend class    Font;

        friend class    TextBox;
        friend class    BMPImage;

        // Calulate sysfont glyph
        static void     FontCalcGlyphPos(fontGlyphPos_s *out, charWidthInfo_s **cwout, int glyphIndex, float scaleX, float scaleY);

        static Target           _target;
        static ScreenImpl       *_screen;

       /* static void         RenderRGBA8(int posX, int posY, Color &color);
        static void         RenderBGR8(int posX, int posY, Color &color);
        static void         RenderRGB565(int posX, int posY, Color &color);
        static void         RenderRGB5A1(int posX, int posY, Color &color);
        static void         RenderRGBA4(int posX, int posY, Color &color);*/

    };

#if GEOMETRY
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
        xinc = (dx > 0) ? 1 : -1;
        yinc = (dy > 0) ? 1 : -1;
        dx = abs(dx);
        dy = abs(dy);

        _DrawPixel(x, y, color);

        if (dx > dy)
        {
            cumul = dx / 2;
            for (i = 1; i <= dx; i++)
            {
                x += xinc;
                cumul += dy;
                if (cumul >= dx)
                {
                    cumul -= dx;
                    y += yinc;
                }
                _DrawPixel(x, y, color);
            }
        }
        else
        {
            cumul = dy / 2;
            for (i = 1; i <= dy; i++)
            {
                y += yinc;
                cumul += dx;
                if (cumul >= dy)
                {
                    cumul -= dy;
                    x += xinc;
                }
                _DrawPixel(x, y, color);
            }
        }
    }
#endif

}

#endif 