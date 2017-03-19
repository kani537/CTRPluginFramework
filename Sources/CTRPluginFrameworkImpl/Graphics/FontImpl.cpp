#include "CTRPluginFrameworkImpl/Graphics/Font.hpp"
#include "ctrulib/allocator/linear.h"
#include "ctrulib/font.h"
#include "ctrulib/util/utf.h"
#include <cstring>
#include <cmath>
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
namespace CTRPluginFramework
{
    extern "C" CFNT_s* g_sharedFont;
    extern "C" int charPerSheet;

    u32            *defaultSysFont = nullptr;

    void    Font::Initialize(void)
    {
        // Sysfont has 7505 glyph
        if (defaultSysFont != nullptr)
            linearFree(defaultSysFont);

        defaultSysFont = (u32 *)linearAlloc(sizeof(u32) * 7505);
        std::memset(defaultSysFont, 0, sizeof(u32) * 7505);
    }

    Glyph   *Font::GetGlyph(u8* &c)
    {
        u32     code;
        u32     glyphIndex;
        ssize_t units;

        units = decode_utf8(&code, c);
        if (units == -1) 
            return (nullptr);

        c += units;
        if (code > 0)
        {
            glyphIndex = fontGlyphIndexFromCodePoint(code);
            if (defaultSysFont[glyphIndex] != 0)
                return ((Glyph *)defaultSysFont[glyphIndex]);
            return (CacheGlyph(glyphIndex));
        }
        return (nullptr);
    }

    u8    *GetOriginalGlyph(u32 glyphIndex)
    {
        TGLP_s  *tglp = fontGetGlyphInfo();
        u8      *data = (u8 *)fontGetGlyphSheetTex(glyphIndex / charPerSheet);
        u8      *glyph = nullptr;
        u8      *tileData = nullptr;

        int     width = tglp->sheetWidth;
        int     height = tglp->sheetHeight;

        int     dataWidth = width;
        int     dataHeight = height;

        int     index = glyphIndex % charPerSheet;

        // Increase the size of the image to a power-of-two boundary, if necessary
        width = 1 << (int)(std::ceil(std::log2(width)));
        height = 1 << (int)(std::ceil(std::log2(height)));

        int tileWidth = width / 8;
        int tileHeight = height / 8;

        // Allocate buffers
        tileData = (u8 *)linearAlloc(4096);
        glyph = (u8 *)linearAlloc(800);
        std::memset(tileData, 0, 4096);
        std::memset(glyph, 0, 800);


        // Sheet is composed of 8x8 pixel tiles
        for (int tileY = 0; tileY < tileHeight; tileY++)
        {
            //int w = std::round(tileWidth / 5);
            //int start = std::round(index * w);
            //int end = start + w;
            for (int tileX = 0; tileX < tileWidth; tileX++)
            {
                // Tile is composed of 2x2 sub-tiles
                for (int y = 0; y < 2; y++)
                {
                    for (int x = 0; x < 2; x++)
                    {
                        // Subtile is composed of 2x2 pixel groups
                        for (int yy = 0; yy < 2; yy++)
                        {
                            for (int xx = 0; xx < 2; xx++)
                            {
                                // Pixel group is composed of 2x2 pixels
                                for (int yyy = 0; yyy < 2; yyy++)
                                {
                                    for (int xxx = 0; xxx < 2; xxx++)
                                    {
                                        // if the final y value is beyond the input data's height then don't read it
                                        if (tileY + y + yy + yyy >= dataHeight)
                                            continue;
                                        // same for the x and the input data width
                                        if (tileX + x + xx + xxx >= dataWidth)
                                            continue;

                                        int pixelX = (xxx + (xx * 2) + (x * 4) + ((tileX) * 8));
                                        int pixelY = (yyy + (yy * 2) + (y * 4) + (tileY * 8));

                                        int dataX = (xxx + (xx * 4) + (x * 16) + (tileX * 64));
                                        int dataY = ((yyy * 2) + (yy * 8) + (y * 32) + (tileY * width * 8));

                                        int dataPos = dataX + dataY;
                                        int bmpPos = pixelX + (pixelY * width);

                                        u8 byte = data[dataPos / 2];
                                        int shift = (dataPos & 1) * 4;

                                        tileData[bmpPos] = ((byte >> shift) & 0x0F) * 0x11;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Get the part we're interested in
        int w = std::round(width / 5);
        int start = std::round(index * w);
        int end = start + w;
        u8  *p = glyph;
        for (int y = 0; y < 32; y++)
        {
            for (int x = start; x < end; x++)
            {
                *p++ = tileData[x + (y * width)];
            }
        }

        linearFree(tileData);
        return (glyph);
    }

    void    ShrinkGlyph(u8 *dest, int dheight, u8 *src)
    {
        int   x, y;
        int   i, ii;
        float alpha;
        float xfrag, yfrag, xfrag2, yfrag2;
        float xt, yt, dx, dy;
        int   xi, yi;
        int sheight = 32;
        int swidth = 25;
        float ratio = ((float)dheight) / 32.f;
        int   dwidth = std::round((float)(swidth) * ratio);

        dx = std::round(((float)swidth)/dwidth);
        dy = std::round(((float)sheight)/dheight);

        for(yt = 0, y = 0; y < dheight; y++, yt += dy)
        {
            yfrag = ceil(yt) - yt;
            if(yfrag == 0)
                yfrag = 1;
            yfrag2 = yt+dy - (float)floor(yt + dy);
            if(yfrag2 == 0 && dy != 1.0f)
                yfrag2 = 1;

            for(xt = 0, x = 0; x < dwidth; x++, xt += dx)
            {
                xi = (int)xt;
                yi = (int)yt;
                xfrag = (float)ceil(xt) - xt;
                if(xfrag == 0)
                    xfrag = 1;
                xfrag2 = xt + dx - (float)floor(xt+dx);
                if(xfrag2 == 0 && dx != 1.0f)
                    xfrag2 = 1;
                alpha =  xfrag * yfrag * src[(yi * swidth + xi)];
          
                for(i=0; xi + i + 1 < xt+dx-1; i++)
                {
                    alpha += yfrag * src[(yi * swidth + xi + i + 1)];
                } 
                
                alpha += xfrag2 * yfrag * src[(yi*swidth+xi+i+1)];
            
                for(i = 0; yi + i + 1 < yt + dy - 1 && yi + i + 1 < sheight; i++)
                {
                    alpha += xfrag * src[((yi + i + 1) * swidth + xi)];
            
                    for (ii = 0; xi + ii + 1 < xt + dx - 1 && xi + ii + 1 < swidth; ii++)
                    {
                        alpha += src[((yi + i + 1) * swidth + xi + ii + 1)];
                    }
              
                    if (yi + i + 1 < sheight && xi + ii + 1 < swidth)
                    {
                        alpha += xfrag2 * src[((yi + i + 1) * swidth + xi + ii + 1)];
                    }
                }

                if (yi + i + 1 < sheight)
                {
                    alpha += xfrag * yfrag2 * src[((yi + i + 1) * swidth + xi)];

                    for (ii = 0; xi + ii + 1 < xt + dx - 1 && xi + ii + 1 < swidth; ii++)
                    {
                        alpha += yfrag2 * src[((yi + i + 1) * swidth + xi + ii + 1)];
                    }
                }
          
                if (yi + i + 1 < sheight && xi + ii + 1 < swidth)
                {
                    alpha += xfrag2 * yfrag2 * src[((yi + i + 1) * swidth + xi + ii + 1)];
                }
            
                alpha /= dx * dy;

                alpha = alpha <= 0.f ? 0.f : (alpha >= 255.f ? 255.f : alpha);

                dest[(y * dwidth + x)] = (unsigned char) alpha;
            }
        }
    }

    Glyph   *Font::CacheGlyph(u32 glyphIndex)
    {
        // if the glyph already exists
        if (defaultSysFont[glyphIndex] != 0)
            return ((Glyph *)defaultSysFont[glyphIndex]);

        u8  *originalGlyph = GetOriginalGlyph(glyphIndex);
        // 16px * 14px = 224
        u8  *newGlyph = (u8 *)linearAlloc(208);
        std::memset(newGlyph, 0, 208);

        // Shrink glyph data to the requiered size
        ShrinkGlyph(newGlyph, 16, originalGlyph);

        // Free original glyph data
        linearFree(originalGlyph);

        // Allocate new Glyph
        Glyph *glyph = (Glyph *)linearAlloc(sizeof(Glyph));
        std::memset(glyph, 0, sizeof(Glyph));

        // Get Glyph data
        charWidthInfo_s     *cwi;
        fontGlyphPos_s      glyphPos;
        Renderer::FontCalcGlyphPos(&glyphPos, &cwi, glyphIndex, 0.5f, 0.5f);

        glyph->xOffset =  (glyphIndex == 0) ? glyphPos.xOffset : std::round(glyphPos.xOffset);
        glyph->xAdvance = (glyphIndex == 0) ? glyphPos.xAdvance : std::round(glyphPos.xAdvance);
        glyph->glyph = newGlyph;

        // Add Glyph to defaultSysFont
        defaultSysFont[glyphIndex] = (u32)glyph;

        return (glyph);
    }
}
