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
        if (!text) return (0.0f);

        float   w = 0.f;
        u8      *c = (u8 *)text;        

        while (*c == '\n') c++;

        do
        {
            if (!*c) break;

            if (*c == '\r' || *c == '\n')
            {
                c++;
                continue;
            }

            if (*c == 0x18)
            {
                c++;
                continue;
            }

            if (*c == 0x1B)
            {
                c += 4;
                continue;
            }

            Glyph *glyph = Font::GetGlyph(c);
            if (glyph == nullptr) break;

            w += glyph->Width();
        } while (true);
        return (w);
    }


    int Renderer::GetLineCount(const char *text, float maxWidth)
    {
        if (!text) return (0.0f);

        int     lineCount = 1;
        float   w = 0.0f;
        u8      *c = (u8 *)text;

        while (*c)
        {
            if (*c == '\r')
            {
                c++;
                continue;
            }

            if (*c == '\n')
            {
                lineCount++;
                w = 0.f;
                c++;
                continue;
            }

            if (*c == 0x18)
            {
                c++;
                continue;
            }

            if (*c == 0x1B)
            {
                c += 4;
                continue;
            }

            Glyph *glyph = Font::GetGlyph(c);

            if (glyph == nullptr)
                break;

            float gSize = glyph->Width();

            if (w + gSize > maxWidth)
            {
                lineCount++;
                w = gSize;
            }
            else
                w += gSize;            
        }

        return (lineCount);
    }

    void Renderer::GetTextInfos(const char *text, int &lineCount, float &lineMaxWidth, float maxWidth)
    {
        lineCount = 0;
        lineMaxWidth = 0.f;

        if (!text || !*text) return;

        lineCount = 1;
        float   w = 0.0f;
        u8      *c = (u8 *)text;

        while (*c)
        {
            if (*c == '\n')
            {
                lineCount++;
                if (w > lineMaxWidth)
                    lineMaxWidth = w;
                w = 0.f;
                c++;
                continue;
            }

            if (*c == 0x18)
            {
                c++;
                continue;
            }

            if (*c == 0x1B)
            {
                c += 4;
                continue;
            }

            Glyph *glyph = Font::GetGlyph(c);

            if (glyph == nullptr)
                break;

            float gSize = glyph->Width();

            if (w + gSize > maxWidth)
            {
                lineCount++;
                if (w > lineMaxWidth)
                    lineMaxWidth = w;
                w = gSize;
            }
            else
                w += gSize;
        }
        if (w > lineMaxWidth)
            lineMaxWidth = w;
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

    int Renderer::DrawGlyph(Glyph *glyph, int posX, int posY, Color &color)
    {
        posX += glyph->xOffset;
        int  x = posX;
        int  y = posY;
        u8   *data = glyph->glyph;

        for (int i = 0; i < 208; i++)
        {
            if (i != 0 && i % 13 == 0)
            {
                y++;
                x = posX;
            }
            color.a = data[i];

            u8 *left = static_cast<u8 *>(_screen->GetLeftFramebuffer(x, y));
            Color l = PrivColor::FromFramebuffer(left);
            Color c = l.Blend(color, Color::BlendMode::Alpha);

            PrivColor::ToFramebuffer(left, c);
            x++;
        }

        return (posX + glyph->xAdvance);
    }

    int Renderer::DrawGlyph(Glyph *glyph, int posX, int posY, float &offset, Color &color)
    {
        posX += glyph->xOffset;

        int  x = posX;
        int  y = posY;
        u8   *data = glyph->glyph;

        for (int i = static_cast<int>(offset); i < 208; i++)
        {
            if (i != 0 && i % 13 == 0)
            {
                y++;
                x = posX;
                if (offset)
                    i += offset;
            }
            color.a = data[i];

            u8 *left = static_cast<u8 *>(_screen->GetLeftFramebuffer(x, y));
            Color l = PrivColor::FromFramebuffer(left);
            Color c = l.Blend(color, Color::BlendMode::Alpha);

            PrivColor::ToFramebuffer(left, c);
            x++;
        }
        if (offset > 0.f)
        {
            posX -= offset;
            offset = 0;
        }

        return (posX + glyph->xAdvance);
    }

    int Renderer::DrawSysStringReturn(const unsigned char *stri, int posX, int& posY, int xLimits, Color color, int maxY)
    {
        // Check for a valid pointer
        if (!(stri && *stri))
            return (posX);
        Color           bakColor(color);
        int             lineCount = 1;
        u8              *str = const_cast<u8 *>(stri);
        int             x = posX;

        xLimits = std::min(xLimits, (_target == TOP ? 400 : 320));

        // Skip UTF8 sig
        if (str[0] == 0xEF && str[1] == 0xBB && str[2] == 0xBF)
            str += 3;

        do
        {
            u8 c = *str;

            if (c == '\r')
                str++;
            if (c == '\n')
            {
                x = posX;
                lineCount++;    
                posY += 16;
                str++;
                continue;
            }

            if (c == 0x18)
            {
                color = bakColor;
                str++;
                continue;
            }

            if (c == 0x1B)
            {
                str++;
                color.r = *str++;
                color.g = *str++;
                color.b = *str++;
                continue;
            }

            if (posY >= maxY)
                break;

            Glyph *glyph = Font::GetGlyph(str);

            if (glyph == nullptr)
                break;

            if (x + glyph->Width() >= xLimits)
            {
                x = posX;
                lineCount++;
                posY += 16;
            }

            x = DrawGlyph(glyph, x, posY, color);

        } while (*str);

        posY += 16;
        return (x);
    }

    int Renderer::DrawSysString(const char *stri, int posX, int &posY, int xLimits, Color color, float offset, const char *end)
    {
        Color   bakColor(color);
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
            u8 c = *str;

            if (x >= xLimits || str == (u8 *)end)
            {
                break;
            }

            if (c == '\n' || c == '\r')
            {
                str++;
                continue;
            }

            if (c == 0x18)
            {
                color = bakColor;
                str++;
                continue;
            }

            if (c == 0x1B)
            {
                str++;
                color.r = *str++;
                color.g = *str++;
                color.b = *str++;
                continue;
            }

            if (posY >= 240)
                break;

            glyph = Font::GetGlyph(str);

            if (glyph == nullptr)
                break;

            float gWidth = glyph->Width();

            if (x + gWidth > xLimits)
            {
                break;
            }

            if (offset >= gWidth)
            {
                offset -= (gWidth);
                continue;
            }

            x = DrawGlyph(glyph, x, posY, offset, color);
        } while (*str);

        posY += 16;
        return (x);
    }
}
