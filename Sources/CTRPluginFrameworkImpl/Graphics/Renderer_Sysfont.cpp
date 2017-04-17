#include "types.h"

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Font.hpp"

#include <algorithm>

namespace CTRPluginFramework
{
    extern "C" CFNT_s* g_sharedFont;
    extern "C" int charPerSheet;

    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void Renderer::FontCalcGlyphPos(fontGlyphPos_s *out,  charWidthInfo_s **cwout, int glyphIndex, float scaleX, float scaleY)
    {
        FINF_s* finf = &g_sharedFont->finf;
        TGLP_s* tglp = finf->tglp;
        charWidthInfo_s *cwi = fontGetCharWidthInfo(glyphIndex);
        *cwout = cwi;

        int sheetId = glyphIndex / charPerSheet;
        int glInSheet = glyphIndex % charPerSheet;
        out->sheetIndex = sheetId;
        out->xOffset = scaleX * cwi->left;
        out->xAdvance = scaleX * cwi->charWidth;
        out->width = scaleX * cwi->glyphWidth;

        int lineId = glInSheet / tglp->nRows;
        int rowId = glInSheet % tglp->nRows;

        float tp = (float)(rowId*(tglp->cellWidth+1)+1);
        float tx = (float)(tp / tglp->sheetWidth)   ;
        float ty = 1.0f - (float)((lineId+1)*(tglp->cellHeight+1)+1) / tglp->sheetHeight;
        float tw = (float)cwi->glyphWidth / tglp->sheetWidth;
        float th = (float)tglp->cellHeight / tglp->sheetHeight;
        out->texcoord.left = tx;
        out->texcoord.top = ty;
        out->texcoord.right = tx+tw;
        out->texcoord.bottom = ty + th;
    }

    float Renderer::GetTextSize(const char *text)
    {
        float   w;
        u8      *c;
        u32     code;
        ssize_t units;
        int     glyphIndex;
        fontGlyphPos_s  data;

        w = 0.0f;
        c = (u8 *)text;
        if (!text) return (0.0f);
        while (*c == '\n') c++;
        do
        {
            if (!*c) break;
            Glyph *glyph = Font::GetGlyph(c);
            if (glyph == nullptr) break;
            w += (glyph->xOffset + glyph->xAdvance);
        } while (1);
        return (w);
    }


    int Renderer::GetLineCount(const char *text, float maxWidth)
    {
        float   w;
        u8      *c;
        u32     code;
        ssize_t units;
        int     glyphIndex;
        fontGlyphPos_s  data;
        int     lineCount = 1;

        w = 0.0f;
        c = (u8 *)text;
        if (!text) return (0.0f);
        while (*c == '\n')
        {
            c++;
            lineCount++;
        }
        do
        {
            if (!*c) 
                break;
            if (*c == '\n')
            {
                lineCount++;
                w = 0;
            }
            Glyph *glyph = Font::GetGlyph(c);
            if (glyph == nullptr) break;
            float gSize = glyph->xOffset + glyph->xAdvance;
            if (w + gSize > maxWidth)
            {
                lineCount++;
                w = gSize;
            }
            else
                w += gSize;
            
        } while (1);

        return (lineCount);
    }

    extern "C" unsigned char *CheckedCheckbox;
    extern "C" unsigned char *UnCheckedCheckbox;

    void Renderer::DrawSysCheckBox(const char *str, int posX, int &posY, int xLimits, Color color, bool isChecked,  float offset)
    {
        Icon::DrawCheckBox(posX, posY, isChecked);
        posX += 20;
        DrawSysString(str, posX, posY, xLimits, color, offset);
        posY += 1;

    }

    extern "C" unsigned char *FolderFilled;
    void Renderer::DrawSysFolder(const char *str, int posX, int &posY, int xLimits, Color color, float offset)
    {
        Icon::DrawFolder(posX, posY);
        posX += 20;
        DrawSysString(str, posX, posY, xLimits, color, offset);
        posY += 1;
    }

    inline int Renderer::DrawGlyph(Glyph *glyph, int posX, int posY, Color &color)
    {
        int  x = posX + glyph->xOffset;
        int  y = posY;
        u8   *data = glyph->glyph;

        for (int i = 0; i < 208; i++)
        {
            if (i != 0 && i % 13 == 0)
            {
                y++;
                posX = x;
            }
            color.a = data[i];

            u8 *left = static_cast<u8 *>(_screen->GetLeftFramebuffer(posX, y));
            Color l = PrivColor::FromFramebuffer(left);
            Color c = l.Blend(color, Color::BlendMode::Alpha);

            PrivColor::ToFramebuffer(left, c);
            x++;
        }

        return (x + glyph->xAdvance);
    }

    inline int Renderer::DrawGlyph(Glyph *glyph, int posX, int posY, float &offset, Color &color)
    {
        int  x = posX + glyph->xOffset;
        int  y = posY;
        u8   *data = glyph->glyph;

        for (int i = static_cast<int>(offset); i < 208; i++)
        {
            if (i != 0 && i % 13 == 0)
            {
                y++;
                posX = x;
                if (offset)
                    i += offset;
            }
            color.a = data[i];

            u8 *left = static_cast<u8 *>(_screen->GetLeftFramebuffer(posX, y));
            Color l = PrivColor::FromFramebuffer(left);
            Color c = l.Blend(color, Color::BlendMode::Alpha);

            PrivColor::ToFramebuffer(left, c);
            x++;
        }
        if (offset > 0.f)
        {
            x -= offset;
            offset = 0;
        }

        return (x + glyph->xAdvance);
    }

    int Renderer::DrawSysStringReturn(const unsigned char *stri, int posX, int& posY, int xLimits, Color color, int maxY)
    {
        // Check for a valid pointer
        if (!(stri && *stri))
            return (posX);

        u32             glyphcode;
        int             lineCount = 1;
        u8              *str = const_cast<u8 *>(stri);
        fontGlyphPos_s  glyphPos;
        int             x = posX;
        


        xLimits = std::min(xLimits, (_target == TOP ? 400 : 320));

        // Skip UTF8 sig
        if (str[0] == 0xEF && str[1] == 0xBB && str[2] == 0xBF)
            str += 3;

        do
        {
            if (x >= xLimits)
            {
                x = posX;
                lineCount++;
                posY += 16;
            }

            if (*str == '\r')
                str++;
            if (*str == '\n')
            {
                x = posX;
                lineCount++;    
                posY += 16;
                str++;
                continue;
            }

            if (posY >= maxY)
                break;

            Glyph *glyph = Font::GetGlyph(str);

            if (glyph == nullptr)
                break;

            x = DrawGlyph(glyph, x, posY, color);

        } while (glyphcode > 0 && *str);

        posY += 16 * lineCount;
        return (x);
    }

    int Renderer::DrawSysString(const char *stri, int posX, int &posY, int xLimits, Color color, float offset, const char *end)
    {
        Glyph   *glyph;
        int      x = posX;
        //u8      *str = (u8 *)stri.c_str();
        u8 *str = (u8 *)stri;
        
        if (!(str && *str))
            return (x);

        xLimits = std::min(xLimits, (_target == TOP ? 400 : 320));

        // Skip UTF8 sig
        if (str[0] == 0xEF && str[1] == 0xBB && str[2] == 0xBF)
            str += 3;

        do
        {
            if (x >= xLimits || str == (u8 *)end)
            {
                break;
            }
            if (*str == '\n' || *str == '\r')
            {
                str++;
                continue;
            }

            if (posY >= 240)
                break;

            glyph = Font::GetGlyph(str);

            if (glyph == nullptr)
                break;
            if (x + glyph->xAdvance + glyph->xOffset > xLimits)
            {
                break;
            }

            if (offset >= glyph->xAdvance + glyph->xOffset)
            {
                offset -= (glyph->xAdvance + glyph->xOffset);
                continue;
            }

            x = DrawGlyph(glyph, x, posY, offset, color);
        } while (*str);

        posY += 16;
        return (x);
    }
}
