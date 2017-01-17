#include "CTRPluginFramework.hpp"
#include <cmath>

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
        out->texcoord.top = ty+th;
        out->texcoord.right = tx+tw;
        out->texcoord.bottom = ty;
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
                py = 240 - startY - (endY + 1) / 2;
                if ((uint16_t)(endX + 1) <= charWidth && alpha)
                {
                    px = startX + (endX + 1) / 2;
                    //Pixel   (*pScreen)[240] = (Pixel(*)[240])_framebuffer[_target];


                    if (!(px >= 400 || px < 0 || py >= 240 || py < 0))
                    {
                        u32 offset = GetFramebufferOffset(px, py, 3, _rowSize[_target]);
                        u8 *left = (u8 *)_framebuffer + offset;
                        u8 *right = (u8 *)_framebufferR + offset;

                        Color l = Color(*(left + 2), *(left + 1), *(left));

                        *left++ = (l.b * (0x0F - alpha) + color.b * (alpha + 1)) >> 4;
                        *left++ = (l.g * (0x0F - alpha) + color.g * (alpha + 1)) >> 4;
                        *left++ = (l.r * (0x0F - alpha) + color.r * (alpha + 1)) >> 4;
                        if (_render3D)
                        {
                            Color r = Color(*(right + 2), *(right + 1), *(right));

                            *right++ = (l.b * (0x0F - alpha) + color.b * (alpha + 1)) >> 4;
                            *right++ = (l.g * (0x0F - alpha) + color.g * (alpha + 1)) >> 4;
                            *right++ = (l.r * (0x0F - alpha) + color.r * (alpha + 1)) >> 4;
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
        charWidthInfo_s*    cwi;

        uint16_t    charx;
        uint16_t    chary;
        uint16_t    glyphXoffs;
        uint16_t    glyphYoffs;
        uint8_t     *sheetsrc;
        uint8_t     *sheetsrc3;
        uint16_t    tileX;
        uint16_t    tileXend;
        uint16_t    tileY;
        uint16_t    tileYend;
        //t_tile        tile;
        float       size;
        float       padding;
        float       height;
        u8          *tile;
        
        //size = 128 / 8;
        size = 16; //<-- 128 / 8
        padding = 8; //<-- 
        height = 30;
        //fontCalcGlyphPos(&glyphPos, glyphCode, GLYPH_POS_CALC_VTXCOORD, 1.0f, 1.0f);
        FontCalcGlyphPos(&glyphPos, glyphCode, 0.5f, 0.5f);
        sheetsrc = (u8 *)fontGetGlyphSheetTex(glyphPos.sheetIndex);

        cwi = fontGetCharWidthInfo(glyphCode);

        glyphYoffs = (int)ceilf(128.0f * glyphPos.texcoord.bottom);//glyph.code % (tglp->nRows * tglp->nLines) / tglp->nRows * (tglp->cellHeight + 1) + 1;
        glyphXoffs = (int)ceilf(128.0f * glyphPos.texcoord.left - 1 + offset);

        tileX = (int)ceilf((128.0f * (glyphPos.texcoord.left) - 1) + offset) & 0xFFFFFFF8;  
        tileXend = (int)ceilf((128.0f * glyphPos.texcoord.right)) & 0xFFFFFFF8;

        tileY = glyphYoffs & 0xFFFFFFF8;
        tileYend = (glyphYoffs + (u32)height) & 0xFFFFFFF8;

        glyphXoffs &= 0x00000007;
        glyphYoffs &= 0x00000007;

        //*********
        int st = (int)(ceilf(glyphPos.xOffset) + 1);
        u16 startX = x + (offset ? 0 : st);
        u16 startY = y;
        u16 endX = 0;
        u16 endY = 0;
        u8  charWidth = cwi->charWidth; // !!!! - offset
        u8  charHeight = height;


        //********
        for (chary = tileY; chary <= tileYend; chary += padding, endY += padding)
        {

            for (charx = tileX; charx <= tileXend; charx += padding, endX += padding)
            {
                sheetsrc3 = sheetsrc + (u32)(4 * (charx + chary * size));

                
                    DrawTile(sheetsrc3, padding, padding, \
                        startX, startY, charx - tileX - glyphXoffs, endY - glyphYoffs, \
                        charWidth, height, color);
                    /*
                    DrawTile(pScreen, sheetsrc3, padding, padding, \
                        startX, startY, charx - tilex - glyphXoffs, chary - tiley - glyphYoffs, \
                        glyphPos.width - offset, height, color);*/
            }
        }
        return ((int)ceilf(glyphPos.xAdvance) + 1 - offset);//((glyphPos.xAdvance + 1 - offset) / 2);
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

    void Renderer::DrawSysString(const char *str, int posX, int &posY, int max, Color color, float offset, bool autoReturn)
    {
        size_t          len;
        u32             glyphcode;
        int             units;
        u32             xLimits;
        int             lineCount;
        size_t          i;
        fontGlyphPos_s glyphPos;
        int             x = posX;
        int             y = posY;
        
        if (!(str && *str))
            return;
        len = strlen(str);
        if (!max || max > len)
            max = len;  
        i = max;
        lineCount = 1;
        xLimits = (_target == TOP) ? 400 : 320;
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
            FontCalcGlyphPos(&glyphPos, index, 1.0f, 1.0f);
            float width = (glyphPos.xAdvance / 2.0f) + ((int)glyphPos.xOffset % 2) + ((int)glyphPos.xAdvance % 2);
            if (offset > width)
            {
                offset -= width;
                continue;
            }
            i--;
            x += DrawGlyph(x, y, index, color, offset);
            offset = 0;
        } while (glyphcode > 0);

        posY += 16 * lineCount;
    }
}