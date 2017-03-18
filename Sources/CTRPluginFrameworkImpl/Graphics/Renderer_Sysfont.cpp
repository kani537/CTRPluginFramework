#include "types.h"
#include "ctrulib/services/gspgpu.h"
#include "ctrulib/util/utf.h"

#include "ctrulib/allocator/linear.h"
#include "CTRPluginFramework/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include <algorithm>
#include <cmath>
#include "CTRPluginFrameworkImpl/Graphics/Font.hpp"

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

     u8 *Renderer::DrawTile(u8 *tile, u8 iconsize, u8 tilesize, u16 startX, \
     u16 startY, u16 endX, u16 endY, u8 charWidth, u8 charHeight, Color color)
    {
        u16     alpha;
        u16     py;
        u16     px;
        u32     y;
        u32     x;
        int     size = 2;
        
        if (iconsize == 2 || iconsize == 1)
        {
            if (endY + 1 <= charHeight)
            {           
                alpha = *(u16 *)tile;
                alpha = (alpha & 0x0F0F) + ((alpha >> 4) & 0x0F0F);
                alpha = ((alpha + (alpha >> 8)) >> 2) & 0x0F;
                py = startY + (endY / 2);
                if (alpha && endX + 1 <= charWidth)
                {
                    px = startX + (endX + 1) / 2;

                    if (!(px >= 400 || px < 0 || py >= 240 || py < 0))
                    {
                        //u32 offset = GetFramebufferOffset(px, py, _screens[_target]->GetBytesPerPixel(), _rowSize[_target]);
                        u8 *left = (u8 *)_screen->GetLeftFramebuffer(px, py);
                        //u8 *right = (u8 *)_screens[_target]->GetRightFramebuffer(px, py) + offset;
                        GSPGPU_FramebufferFormats fmt = _format;
                        Color l = PrivColor::FromFramebuffer(left);
                        int aneg = 0x0F - alpha;
                        int apos = alpha + 1;
                        Color res;
                        res.b = (l.b * aneg + color.b * apos) >> 4;  
                        res.g = (l.g * aneg + color.g * apos) >> 4;
                        res.r = (l.r * aneg + color.r * apos) >> 4;
                        PrivColor::ToFramebuffer(left, res);
                        /*if (_useDoubleBuffer)
                        {
                            u8 *right = (u8 *)_screen->GetLeftFramebuffer(true);
                            u32 offset = GetFramebufferOffset(px, py, _screen->GetBytesPerPixel(), _screen->GetRowSize());
                            Color::ToFramebuffer(right + offset, res);
                        }*/
                        //if (!_useRender3D)
                        //res.ToMemory(left, fmt);
                        /*else
                            res.ToMemory(left, fmt, right);*/
                    }
                }
            }
            tile += iconsize;
        }
        else
        {
            for (y = 0; y < iconsize; y += tilesize)
            {
                for (x = 0; x < iconsize; x += tilesize)
                    tile = DrawTile(tile, tilesize, tilesize / 2, startX, startY, endX + x, endY + y, charWidth, charHeight, color);
            }
        }
        return tile;
    }

    uint8_t Renderer::DrawGlyph(fontGlyphPos_s  &glyphPos, charWidthInfo_s *cwi, u16 x, u16 y, Color color, float offset)
    {
        u8          *texSheet;

        u16         glyphXoffs;
        u16         glyphYoffs;        

        u16         tileX;
        u16         tileXend;
        u16         tileY;
        u16         tileYend;

        //float       size = 16;
        //float       padding = 8;
        //float       height = 30;
        texSheet = (u8 *)fontGetGlyphSheetTex(glyphPos.sheetIndex);


        glyphXoffs = (int)ceilf(128.0f * glyphPos.texcoord.left - 1 + (offset * 2));
        tileX = glyphXoffs & 0xFFFFFFF8;  
        tileXend = (int)ceilf((128.0f * glyphPos.texcoord.right)) & 0xFFFFFFF8;

        glyphYoffs = (int)ceilf(128.0f * glyphPos.texcoord.bottom);
        tileY = (int)ceilf(128.0f * glyphPos.texcoord.top) & 0xFFFFFFF8;
        tileYend = glyphYoffs & 0xFFFFFFF8;

        glyphXoffs &= 0x00000007;
        glyphYoffs &= 0x00000007;

        //*********
        int st = (int)(ceilf(glyphPos.xOffset) + 1);
        u16 startX = x + (offset ? 0 : st);
        u16 startY = y;
        //u16 endX = 0;
        u16 endY = 0;
        u8  charWidth = cwi->charWidth - (offset * 2);

        //********
        for (u16 charY = tileY; charY <= tileYend; charY += 8, endY += 8)
        {
            for (u16 charX = tileX; charX <= tileXend; charX += 8)
            {
                u8 *tile = texSheet + (u32)(4 * (charX + charY * 16));
                
                    DrawTile(tile, 8, 8, \
                        startX, startY, charX - tileX - glyphXoffs, endY - glyphYoffs, \
                        charWidth, 30, color);
            }
        }
        int ret = (int)ceilf(glyphPos.xAdvance);
        if (offset)
            ret -= (offset);
        else
            ret += 1;
        return (ret);
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
            units = decode_utf8(&code, c);
            if (units == -1) break;
            c += units;
            if (code > 0)
            {
                glyphIndex = fontGlyphIndexFromCodePoint(code);
                charWidthInfo_s *cwi;
                FontCalcGlyphPos(&data, &cwi, glyphIndex, 0.5f, 0.5f);
                w += data.xAdvance + 1;
            }
        } while (code > 0);
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
            units = decode_utf8(&code, c);
            if (units == -1) break;
            c += units;
            if (code > 0)
            {
                glyphIndex = fontGlyphIndexFromCodePoint(code);
                charWidthInfo_s *cwi;
                FontCalcGlyphPos(&data, &cwi, glyphIndex, 0.5f, 0.5f);
                if (w + data.xAdvance + 1 > maxWidth)
                {
                    lineCount++;
                    w = data.xAdvance + 1;
                }
                else
                    w += data.xAdvance + 1;
            }
        } while (code > 0);

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

    int Renderer::DrawSysString(const char *str, int posX, int &posY, int xLimits, Color color, float offset, const char *end)
    {
        u32             glyphcode;
        int             units;
        int             lineCount;
        size_t          i;
        fontGlyphPos_s glyphPos;
        int             x = posX;
        
        if (!(str && *str))
            return (x);

        lineCount = 1;
        xLimits = std::min(xLimits, (_target == TOP ? 400 : 320));

        // Skip UTF8 sig
        if (str[0] == 0xEF && str[1] == 0xBB && str[2] == 0xBF)
            str += 3;

        do
        {
            if (x >= xLimits || str == end)
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

            units = decode_utf8(&glyphcode, (const u8 *)str);
            if (units == -1)
                break;
            str += units;
            u32 index = fontGlyphIndexFromCodePoint(glyphcode);
            charWidthInfo_s *cwi;
            FontCalcGlyphPos(&glyphPos, &cwi, index, 0.5f, 0.5f);
            if (offset >= glyphPos.xAdvance + 1)
            {
                offset -= glyphPos.xAdvance + 1;
                if (x + glyphPos.xAdvance > xLimits)
                    x+= glyphPos.xAdvance;
                continue;
            }
            if (x + glyphPos.xAdvance + 1 > xLimits)
            {
                x+= glyphPos.xAdvance + 1;
                str -= units;
                continue;
            }      
            x += DrawGlyph(glyphPos, cwi, x, posY, color, offset);
            offset = 0;
        } while (glyphcode > 0 && *str);

        posY += 16 * lineCount;
        return (x);
    }

    int Renderer::DrawSysStringReturn(const char *str, int posX, int &posY, int xLimits, Color color, int maxY)
    {
        u32             glyphcode;
        int             units;
        int             lineCount;
        size_t          i;
        fontGlyphPos_s  glyphPos;
        int             x = posX;
        
        if (!(str && *str))
            return (x);

        lineCount = 1;
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

            units = decode_utf8(&glyphcode, (const u8 *)str);
            if (units == -1)
                break;

            str += units;

            u32 index = fontGlyphIndexFromCodePoint(glyphcode);
            charWidthInfo_s *cwi;
            FontCalcGlyphPos(&glyphPos, &cwi, index, 0.5f, 0.5f);

            if (x + glyphPos.xAdvance + 1 > xLimits)
            {
                x+= glyphPos.xAdvance + 1;
                str -= units;
                continue;
            }      
            x += DrawGlyph(glyphPos, cwi, x, posY, color, 0);

        } while (glyphcode > 0 && *str);

        posY += 16 * lineCount;
        return (x);
    }

    int Renderer::DrawSysString2(const char *stri, int posX, int &posY, int xLimits, Color color, float offset, const char *end)
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
            if (x + glyph->xAdvance > xLimits)
            {
                break;
            }
            int  posxx = x + glyph->xOffset;
            int  y = posY;
            u8   *data = glyph->glyph;

            int i = offset > 0.f ? (13 * offset) : 0; 
            for (; i < 208; i++)
            {
                if (i != 0 && i % 13 == 0)
                {
                    y++;
                    x = posxx;
                }
                color.a = data[i];

                u8 *left = (u8 *)_screen->GetLeftFramebuffer(x, y);
                Color l = PrivColor::FromFramebuffer(left);

                Color c = l.Blend(color, Color::BlendMode::Alpha);

                PrivColor::ToFramebuffer(left, c);
                x++;
            }
            x = posxx + glyph->xAdvance;
        } while (*str);

        posY += 16;
        return (x);
    }
}