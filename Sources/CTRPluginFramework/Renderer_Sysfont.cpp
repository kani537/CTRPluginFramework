#include "CTRPluginFramework.hpp"
#include <cmath>
#include "ctrulib/services/gspgpu.h"
#include <algorithm>

namespace CTRPluginFramework
{
    extern "C" CFNT_s* g_sharedFont;
    extern "C" int charPerSheet;

    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void Renderer::FontCalcGlyphPos(fontGlyphPos_s* out, int glyphIndex, float scaleX, float scaleY)
    {
        FINF_s* finf = &g_sharedFont->finf;
        TGLP_s* tglp = finf->tglp;
        charWidthInfo_s* cwi = fontGetCharWidthInfo(glyphIndex);

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
        uint16_t alpha;
        uint16_t py;
        uint16_t px;
        size_t y;
        size_t x;
        int     size = 2;
        
        if (iconsize == 2 || iconsize == 1)
        {
            if ((uint16_t)(endY + 1) <= charHeight)
            {           
                alpha = *(uint16_t*)tile;
                alpha = (alpha & 0x0F0F) + ((alpha >> 4) & 0x0F0F);
                alpha = ((alpha + (alpha >> 8)) >> 2) & 0x0F;
                py = startY + (endY / 2);
                if ((uint16_t)(endX + 1) <= charWidth && alpha)
                {
                    px = startX + (endX + 1) / 2;

                    if (!(px >= 400 || px < 0 || py >= 240 || py < 0))
                    {
                        u32 offset = GetFramebufferOffset(px, py, _screens[_target]->GetBytesPerPixel(), _rowSize[_target]);
                        u8 *left = (u8 *)_screens[_target]->GetLeftFramebuffer() + offset;

                        GSPGPU_FramebufferFormats fmt = _screens[_target]->GetFormat();
                        Color l = Color::FromMemory(left, fmt);

                        Color res  = Color();
                        res.b = (l.b * (0x0F - alpha) + color.b * (alpha + 1)) >> 4;  
                        res.g = (l.g * (0x0F - alpha) + color.g * (alpha + 1)) >> 4;
                        res.r = (l.r * (0x0F - alpha) + color.r * (alpha + 1)) >> 4;

                        res.ToMemory(left, fmt);

                        if (_useRender3D)
                        {
                            u8 *right = (u8 *)_screens[_target]->GetRightFramebuffer() + offset;
                            res.ToMemory(right, fmt);
                        }
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

    uint8_t Renderer::DrawGlyph(uint16_t x, uint16_t y, u32 glyphCode, Color color, float offset)
    {
        fontGlyphPos_s      glyphPos;
        charWidthInfo_s     *cwi;
        u8                  *texSheet;

        u16         glyphXoffs;
        u16         glyphYoffs;        

        u16         tileX;
        u16         tileXend;
        u16         tileY;
        u16         tileYend;

        //float       size = 16;
        //float       padding = 8;
        //float       height = 30;
        
        FontCalcGlyphPos(&glyphPos, glyphCode, 0.5f, 0.5f);
        texSheet = (u8 *)fontGetGlyphSheetTex(glyphPos.sheetIndex);

        cwi = fontGetCharWidthInfo(glyphCode);


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
        u16 endX = 0;
        u16 endY = 0;
        u8  charWidth = cwi->charWidth - (offset * 2);


        //********
        for (u16 charY = tileY; charY <= tileYend; charY += 8, endY += 8)
        {

            for (u16 charX = tileX; charX <= tileXend; charX += 8, endX += 8)
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
                FontCalcGlyphPos(&data, glyphIndex, 0.5f, 0.5f);
                w += data.xAdvance + 1;
            }
        } while (code > 0);
        return (w);
    }
    extern "C" unsigned char *CheckedCheckbox;
    extern "C" unsigned char *UnCheckedCheckbox;

    void Renderer::DrawSysCheckBox(const char *str, int posX, int &posY, int xLimits, Color color, bool isChecked,  float offset)
    {
     /*   u16 outterBorder[] =
        {
            0x3FF8, 0x600C, 0xCFE6, 0x9012,
            0xA00A, 0xA00A, 0xA00A, 0xA00A,
            0xA00A, 0xA00A, 0xA00A, 0x9012,
            0xCFE6, 0x600C, 0x3FF8
        };

        u16 innerBorder[] =
        {
            0x0, 0x1FF0, 0x3018, 0x600C,
            0x4004, 0x4004, 0x4004, 0x4004,
            0x4004, 0x4004, 0x4004, 0x600C,
            0x3018, 0x1FF0, 0x0
        };

        u16 borderChecked[] =
        {
            0x0, 0xF, 0x9, 0x11,
            0x22, 0x1044, 0x888, 0x510,
            0x220, 0x1040, 0x880, 0x500,
            0x0, 0x0, 0x0
        };

        u16 Checked[] =
        {
            0x0, 0x0, 0x6, 0xE,
            0x1C, 0x38, 0x1070, 0x18E0,
            0x1DC0, 0xF80, 0x700, 0x200,
            0x0, 0x0, 0x0
        };*/

        // Correct posY
        int y = posY + (_rowSize[_target] - 240);

        // temp color
        Color grey = Color(105, 105, 105);
        Color blank = Color(192, 192, 192);
        Color green = Color(0, 255, 0);

       /* for (int yy = 0; yy < 15; yy++)
        {
            u32 out = outterBorder[yy];
            u32 in = innerBorder[yy];
            u32 chb = borderChecked[yy];
            u32 ch = Checked[yy];

            int x = 0;
            for (int xx = 16; xx >= 0; xx--, x++)
            {
                if ((out >> xx) & 1)
                {
                    _DrawPixel(posX + x, y + yy, blank);
                }
                if ((in >> xx) & 1)
                {
                    _DrawPixel(posX + x, y + yy, grey);
                }
                if (isChecked && (chb >> xx) & 1)
                {
                    _DrawPixel(posX + x, y + yy, blank);
                }
                if (isChecked && (ch >> xx) & 1)
                {
                    _DrawPixel(posX + x, y + yy, green);
                }
            }
        }*/
        u8 *ucb = isChecked ? CheckedCheckbox :  UnCheckedCheckbox;
        u8 *left = _screens[_target]->GetLeftFramebuffer();
        u8 *right = _screens[_target]->GetRightFramebuffer();
        GSPGPU_FramebufferFormats fmt = _screens[_target]->GetFormat();
        int rowsize = _rowSize[_target];
        int bpp = _screens[_target]->GetBytesPerPixel();

        for (int yy = 0; yy < 15; yy++)
        {
            for (int xx = 0; xx < 15; xx++)
            {
                u32 offset = GetFramebufferOffset(posX + xx, y + yy, bpp, rowsize);
                Color c = Color::FromMemory(ucb, GSP_RGBA8_OES);
                ucb += 4;
                c.ToMemory(left + offset, fmt);
                if (_useRender3D)
                    c.ToMemory(right + offset, fmt);
            }
        }
        posX += 20;
        // posY += ;
        DrawSysString(str, posX, posY, xLimits, color, offset);
        posY += 1;

    }
    extern "C" unsigned char *FolderFilled;
    void Renderer::DrawSysFolder(const char *str, int posX, int &posY, int xLimits, Color color, float offset)
    {
        // Correct posY
        int y = posY + (_rowSize[_target] - 240);

        u8 *folder = FolderFilled;
        u8 *left = _screens[_target]->GetLeftFramebuffer();
        u8 *right = _screens[_target]->GetRightFramebuffer();
        GSPGPU_FramebufferFormats fmt = _screens[_target]->GetFormat();
        int rowsize = _rowSize[_target];
        int bpp = _screens[_target]->GetBytesPerPixel();

        for (int yy = 0; yy < 15; yy++)
        {
            for (int xx = 0; xx < 15; xx++)
            {
                u32 offset = GetFramebufferOffset(posX + xx, y + yy, bpp, rowsize);
                Color c = Color::FromMemory(folder, GSP_RGBA8_OES);
                folder += 4;
                c.ToMemory(left + offset, fmt);
                if (_useRender3D)
                    c.ToMemory(right + offset, fmt);
            }
        }
        posX += 20;
        // posY += ;
        DrawSysString(str, posX, posY, xLimits, color, offset);
        posY += 1;

    }

    int Renderer::DrawSysString(const char *str, int posX, int &posY, int xLimits, Color color, float offset, bool autoReturn)
    {
        u32             glyphcode;
        int             units;
        int             lineCount;
        size_t          i;
        fontGlyphPos_s glyphPos;
        int             x = posX;
        
        if (!(str && *str))
            return (x);

        // Correct posY
        int y = posY + (_rowSize[_target] - 240);
        lineCount = 1;
        xLimits = std::min(xLimits, (_target == TOP ? 400 : 320));

        do
        {
            if (x >= xLimits)
            {
                if (autoReturn)
                {
                    x = posX;
                    lineCount++;
                    y += 16;
                }
                else break;
            }
            if (*str == '\n')
            {
                x = posX;
                lineCount++;    
                y += 16;
                str++;
            }           
            if (y >= 240) break;
            glyphcode = 0;
            if (!*str)
                break;
            units = decode_utf8(&glyphcode, (const u8 *)str);
            if (units == -1)
                break;
            str += units;
            u32 index = fontGlyphIndexFromCodePoint(glyphcode);
            FontCalcGlyphPos(&glyphPos, index, 0.5f, 0.5f);
            //float width = (glyphPos.xAdvance / 2.0f) + ((int)glyphPos.xOffset % 2) + ((int)glyphPos.xAdvance % 2);
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
            x += DrawGlyph(x, y, index, color, offset);
            offset = 0;
        } while (glyphcode > 0);

        posY += 16 * lineCount;
        return (x);
    }
}