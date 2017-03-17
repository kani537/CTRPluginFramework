#include "types.h"
#include "ctrulib/services/gspgpu.h"
#include "ctrulib/util/utf.h"

#include "ctrulib/allocator/linear.h"
#include "CTRPluginFramework/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include <algorithm>
#include <cmath>

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

    void    Renderer::DrawChar(char c, int posX, int posY)
    {
        TGLP_s  *tglp = fontGetGlyphInfo();
        u32     code;
        decode_utf8(&code, (u8*)&c);
        u32     glyphIndex = fontGlyphIndexFromCodePoint(code);
        u8      *data = (u8 *)fontGetGlyphSheetTex(glyphIndex / charPerSheet);

        //int offset = (int)((glyphIndex % charPerSheet) * 24);

        charWidthInfo_s *cwi;
        fontGlyphPos_s  glyphPos;
        FontCalcGlyphPos(&glyphPos, &cwi, glyphIndex, 1.f, 1.f);

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


        // Sheet is composed of 8x8 pixel tiles
        for (int tileY = 0; tileY < tileHeight; tileY++)
        {
            int start = index * (tileWidth / 5);
            int end = start + (tileWidth / 5);
            for (int tileX = start; tileX < end; tileX++)
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

                                        int pixelX = (xxx + (xx * 2) + (x * 4) + (tileX * 8));
                                        int pixelY = (yyy + (yy * 2) + (y * 4) + (tileY * 8));

                                        int dataX = (xxx + (xx * 4) + (x * 16) + (tileX * 64));
                                        int dataY = ((yyy * 2) + (yy * 8) + (y * 32) + (tileY * width * 8));

                                        int dataPos = dataX + dataY;
                                        // int bmpPos = pixelX + (pixelY * width);

                                        u8 byte = data[dataPos / 2];
                                        int shift = (dataPos & 1) * 4;
                                        float alpha = ((byte >> shift) & 0x0F) * 0x11;
                                        Color blank(255, 255, 255, alpha);

                                        u8 *left = (u8 *)_screen->GetLeftFramebuffer(posX + pixelX, posY + pixelY);
                                        Color l = PrivColor::FromFramebuffer(left);

                                        Color c = l.Blend(blank, Color::BlendMode::Alpha);

                                        PrivColor::ToFramebuffer(left, c);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    u8      *alphaArray = nullptr;
    u32     alphaSize = 0;

    void    Renderer::GetAlpha(char c)
    {
        TGLP_s  *tglp = fontGetGlyphInfo();
        u32     code;
        decode_utf8(&code, (u8*)&c);
        u32     glyphIndex = fontGlyphIndexFromCodePoint(code);
        u8      *data = (u8 *)fontGetGlyphSheetTex(glyphIndex / charPerSheet);

        //int offset = (int)((glyphIndex % charPerSheet) * 24);

        charWidthInfo_s *cwi;
        fontGlyphPos_s  glyphPos;
        FontCalcGlyphPos(&glyphPos, &cwi, glyphIndex, 1.f, 1.f);

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

        if (alphaArray == nullptr)
        {
            alphaSize = 800;
            alphaArray = (u8 *)linearAlloc(alphaSize);
            memset(alphaArray, 0, 800);
        }
        //else
         //   memset(alphaArray, 0, tWidth * tileHeight);

        // Sheet is composed of 8x8 pixel tiles
        for (int tileY = 0; tileY < tileHeight; tileY++)
        {
            int start = index * (tileWidth / 5);
            int end = start + (tileWidth / 5);
            for (int tileX = start; tileX < end; tileX++)
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

                                        int pixelX = (xxx + (xx * 2) + (x * 4) + ((tileX - start) * 8));
                                        int pixelY = (yyy + (yy * 2) + (y * 4) + (tileY * 8));

                                        int dataX = (xxx + (xx * 4) + (x * 16) + (tileX * 64));
                                        int dataY = ((yyy * 2) + (yy * 8) + (y * 32) + (tileY * width * 8));

                                        int dataPos = dataX + dataY;
                                        int bmpPos = pixelX + (pixelY * 25);

                                        u8 byte = data[dataPos / 2];
                                        int shift = (dataPos & 1) * 4;
                                       // float alpha = ((byte >> shift) & 0x0F) * 0x11;

                                        alphaArray[bmpPos] = ((byte >> shift) & 0x0F) * 0x11;
                                        //Color blank(255, 255, 255, alpha);

                                        /*u8 *left = (u8 *)_screen->GetLeftFramebuffer(posX + pixelX, posY + pixelY);
                                        Color l = PrivColor::FromFramebuffer(left);

                                        Color c = l.Blend(blank, Color::BlendMode::Alpha);

                                        PrivColor::ToFramebuffer(left, c);*/
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void    Renderer::DrawArray(void)
    {
        if (alphaArray == nullptr)
            return;

        // 25px * 32px

        IntRect rect(100, 100, 25, 32);
        Color blank(255, 255, 255);

        DrawRect(rect, blank, false);

        int posX =100;
        int posY = 100;
        int i = 0;
        while (i < 800)
        {
            if (i != 0 && i % 25 == 0)
            {
                posX = 100;
                posY++;
            }

            blank.a = alphaArray[i];

            u8 *left = (u8 *)_screen->GetLeftFramebuffer(posX, posY);
            Color l = PrivColor::FromFramebuffer(left);

            Color c = l.Blend(blank, Color::BlendMode::Alpha);

            PrivColor::ToFramebuffer(left, c);
            posX++;
            i++;
        }
    }

void shrink(unsigned char *dest, int dwidth, int dheight, unsigned char *src, int swidth, int sheight)
{
  int x, y;
  int i, ii;
  float alpha;
  float xfrag, yfrag, xfrag2, yfrag2;
  float xt, yt, dx, dy;
  int xi, yi;


  dx = ((float)swidth)/dwidth;
  dy = ((float)sheight)/dheight;

  for(yt= 0, y=0;y<dheight;y++, yt += dy)
    {
      yfrag = ceil(yt) - yt;
      if(yfrag == 0)
    yfrag = 1;
      yfrag2 = yt+dy - (float) floor(yt + dy);
      if(yfrag2 == 0 && dy != 1.0f)
    yfrag2 = 1;
    
      for(xt = 0, x=0;x<dwidth;x++, xt+= dx)
    {
      xi = (int) xt;
      yi = (int) yt;
      xfrag = (float) ceil(xt) - xt;
      if(xfrag == 0)
        xfrag = 1;
      xfrag2 = xt+dx - (float) floor(xt+dx);
      if(xfrag2 == 0 && dx != 1.0f)
        xfrag2 = 1;
      alpha =  xfrag * yfrag * src[(yi*swidth+xi)];
      
      for(i=0; xi + i + 1 < xt+dx-1; i++)
        {
          alpha += yfrag * src[(yi*swidth+xi+i+1)];
        } 
      
      alpha += xfrag2 * yfrag * src[(yi*swidth+xi+i+1)];
        
      
      for(i=0; yi+i+1 < yt +dy-1 && yi + i+1 < sheight;i++)
        {
          alpha += xfrag * src[((yi+i+1)*swidth+xi)];
        
          for (ii = 0; xi + ii + 1 < xt + dx - 1 && xi + ii + 1 < swidth; ii++)
        {
          alpha += src[((yi+i+1)*swidth+xi+ii+1)];
        }
          
          if (yi + i + 1 < sheight && xi + ii + 1 < swidth)
        {
          alpha += xfrag2 * src[((yi+i+1)*swidth+xi+ii+1)];
        }
        }
    
      if (yi + i + 1 < sheight)
        {
          alpha += xfrag * yfrag2 * src[((yi + i + 1)*swidth + xi)];

          for (ii = 0; xi + ii + 1 < xt + dx - 1 && xi + ii + 1 < swidth; ii++)
        {
          alpha += yfrag2 * src[((yi + i + 1)*swidth + xi + ii + 1)];
        }
        }
      
      if (yi + i + 1 < sheight && xi + ii + 1 < swidth)
        {
          alpha += xfrag2 * yfrag2 * src[((yi + i + 1)*swidth + xi + ii + 1)];
        }
       
        
      alpha /= dx * dy;

      alpha = alpha <= 0.f ? 0.f : (alpha >= 255.f ? 255.f : alpha);
   
      dest[(y*dwidth+x)] = (unsigned char) alpha;
    }
    }
}

    u8  *alphaArray2 = nullptr;
    u8  *alphaArray3 = nullptr;
    u8  *alphaArray4 = nullptr;
    u8  *alphaArray5 = nullptr;
    void    Renderer::ScaleFont(void)
    {
        if (alphaArray2 == nullptr)
        {
            alphaArray2 = (u8 *)linearAlloc(800);
            memset(alphaArray2, 0, 800);

            alphaArray3 = (u8 *)linearAlloc(800);
            memset(alphaArray3, 0, 800);

            alphaArray4 = (u8 *)linearAlloc(800);
            memset(alphaArray4, 0, 800);

            alphaArray5 = (u8 *)linearAlloc(800);
            memset(alphaArray5, 0, 800);
        }

        shrink(alphaArray2, 20, 26, alphaArray, 25, 32);
        shrink(alphaArray3, 11, 20, alphaArray, 25, 32); //16, 20
        shrink(alphaArray4, 16, 20, alphaArray, 25, 32); //13, 16
        shrink(alphaArray5, 8, 10, alphaArray, 25, 32);
        /*for (int y = 0; y < 32; y+= 2)
        {
            for (int x = 0; x < 25; x+= 2)
            {
                u8  ax = alphaArray[x + (y * 32)];
                u8  ax1 = alphaArray[x + 1 + (y * 32)];
                u8  ay = alphaArray[x + ((y + 1) * 32)];
                u8  ay1 = alphaArray[x + 1 + ((y + 1) * 32)];

                u8 alpha = ((ax + ax1 + ay + ay1) / 4);

                alphaArray2[x + (y * 10)] = alpha;
            }
        }*/
    }


    void    Renderer::DrawArr(u8 *src, int posXX, int height, int width)
    {
        Color blank(255, 255, 255);

        int posY = 100;
        int i = 0;

        int posX = posXX;
        int max = height * width;
        while (i < max)
        {
            if (i != 0 && i % width == 0)
            {
                posX = posXX;
                posY++;
            }

            blank.a = src[i];

            u8 *left = (u8 *)_screen->GetLeftFramebuffer(posX, posY);
            Color l = PrivColor::FromFramebuffer(left);

            Color c = l.Blend(blank, Color::BlendMode::Alpha);

            PrivColor::ToFramebuffer(left, c);
            posX++;
            i++;
        }  
    }

    void    Renderer::DrawArray2(void)
    {
        if (alphaArray == nullptr)
            return;

        // 25px * 32px

        IntRect rect(150, 100, 13, 16);
        Color blank(255, 255, 255);

        //DrawRect(rect, blank, false);

        int posX = 125;
        DrawArr(alphaArray2, 125, 26, 20);
        DrawArr(alphaArray3, 151, 20, 16);
        DrawArr(alphaArray4, 167, 20, 11);
        DrawArr(alphaArray5, 180, 10, 8);
    }

}