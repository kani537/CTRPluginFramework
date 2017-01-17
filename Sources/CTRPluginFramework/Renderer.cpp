#include "CTRPluginFramework/Color.hpp"
#include "CTRPluginFramework/Renderer.hpp"
#include "3DS.h"
#include "ctrulib/services/gspgpu.h"
#include <cstdio>
#include <cmath>


namespace CTRPluginFramework
{
    #define     RANGE(x, y, z) (y >= x && y <= z)

    Target      Renderer::_target = BOTTOM;
    bool        Renderer::_render3D = false;
    bool        Renderer::_isRendering = false;
    Screen      *Renderer::_screenTarget[2] = {Screen::Bottom, Screen::Top};
    u8          *Renderer::_framebuffer[4] = {nullptr};
    u8          *Renderer::_framebufferR[4] = {nullptr};
    u32         Renderer::_rowSize[2] = {0};
    u32         Renderer::_targetWidth[2] = {0};
    u32         Renderer::_targetHeight[2] = {0};
    DrawPixelP  Renderer::_DrawPixel = nullptr;
    DrawDataP   Renderer::_DrawData = nullptr;
    int         Renderer::_length = 1;
    u8          Renderer::_smallBuffer[1000] = {0};
    u8          *Renderer::_buffer = nullptr;
    u32         Renderer::_bufferSize = 1000;

    static FileCommand      _fileCmd;


    void    Renderer::InitBuffer(u32 size)
    {
        _buffer = new u8(size);
        if (_buffer == nullptr)
        {
            if (size > 0x1000)
                InitBuffer(size - 0x1000);
            else
            {
                _buffer = _smallBuffer;
                _bufferSize = 1000;
            }
        }
        else
            _bufferSize = size;
    }

    void        Renderer::Initialize(void)
    {
        _screenTarget[BOTTOM] = Screen::Bottom;
        _screenTarget[TOP] = Screen::Top;
        InitBuffer(0x50000);
    }

    void        Renderer::SetTarget(Target target)
    {
        _target = target;

        switch (_screenTarget[_target]->GetFormat())
        {
            case GSP_RGBA8_OES:
                _DrawPixel = RenderRGBA8;
                _DrawData = RenderRGBA8;
                break;
            case GSP_BGR8_OES:
                _DrawPixel = RenderBGR8;
                _DrawData = RenderBGR8;
                break;
            case GSP_RGB565_OES:
                _DrawPixel = RenderRGB565;
                _DrawData = RenderRGB565;
                break;
            case GSP_RGB5_A1_OES:
                _DrawPixel = RenderRGB5A1;
                _DrawData = RenderRGB5A1;
                break;
            case GSP_RGBA4_OES:
                _DrawPixel = RenderRGBA4;
                _DrawData = RenderRGBA4;
                break;        
        }
    }

    void        Renderer::StartRenderer(bool current)
    {
        _isRendering = true;
        _screenTarget[BOTTOM]->Update();
        _framebuffer[BOTTOM] = _screenTarget[BOTTOM]->GetLeftFramebuffer(current);
        _framebufferR[BOTTOM] = 0;        
        _rowSize[BOTTOM] = _screenTarget[BOTTOM]->GetRowSize();
        _targetWidth[BOTTOM] = _screenTarget[BOTTOM]->GetWidth();
        _targetHeight[BOTTOM] = _screenTarget[BOTTOM]->GetHeight();

        // Screen TOP
        _screenTarget[TOP]->Update();
        _framebuffer[TOP] = _screenTarget[TOP]->GetLeftFramebuffer(current);
        _framebufferR[TOP] = _screenTarget[TOP]->GetRightFramebuffer(current);
        _render3D = _screenTarget[TOP]->Is3DEnabled();
        _rowSize[TOP] = _screenTarget[TOP]->GetRowSize();
        _targetWidth[TOP] = _screenTarget[TOP]->GetWidth();
        _targetHeight[TOP] = _screenTarget[TOP]->GetHeight();

        // Copy current framebuffer into the second to avoid frame glitch
        if (!current)
        {
            u8  *current = _screenTarget[BOTTOM]->GetLeftFramebuffer(true);
            int size = _screenTarget[BOTTOM]->GetFramebufferSize();
            memcpy(_framebuffer[BOTTOM], current, size);

            current = _screenTarget[TOP]->GetLeftFramebuffer(true);
            size = _screenTarget[TOP]->GetFramebufferSize();
            memcpy(_framebuffer[TOP], current, size);

            current = _screenTarget[TOP]->GetRightFramebuffer(true);
            if (current)
                memcpy(_framebufferR[TOP], current, size);
        }

    }

    void        Renderer::GetFramebuffersInfos(u32 *infos)
    {
        /*
        ** Bottom size
        ** Bottom FB
        ** Top size
        ** IS3D
        ** Top LFB
        ** Top RFB
        **/
        infos[0] = _screenTarget[BOTTOM]->GetFramebufferSize();
        infos[1] = (u32)_framebuffer[BOTTOM];

        infos[2] = _screenTarget[TOP]->GetFramebufferSize();
        infos[3] = _render3D ? 1 : 0;
        infos[4] = (u32)_framebuffer[TOP];
        infos[5] = (u32)_framebufferR[TOP];
    }

    void        Renderer::EndRenderer(void)
    {  
        u32 infos[6];
        //GetFramebuffersInfos(infos);
        //Renderer::GetFramebuffersInfos(infos);
        //GSPGPU_FlushDataCache((void *)infos[4], infos[2]);
        // if (infos[3])
          //  GSPGPU_FlushDataCache((void *)infos[5], infos[2]);
        //GSPGPU_FlushDataCache((void *)infos[1], infos[0]);
        Screen::Top->SwapBuffer();
        Screen::Bottom->SwapBuffer();
        gspWaitForVBlank();
        gspWaitForVBlank1();
        _isRendering = false;
    }

    void        Renderer::DrawLine(int posX, int posY, int width, Color color, int height)
    {  
        // Correct posY
        posY += (_rowSize[_target] - 240);
        for (int x = 0; x < width; x++)
        {
            _length = height;
            _DrawPixel(posX + x, posY + height, color);
        }
    }

    void        Renderer::DrawRect(int posX, int posY, int width, int height, Color color, bool fill, int thickness)
    {
        if (fill)
        {
            DrawLine(posX, posY, width, color, height);
        }
        else
        {
            // Top line
            DrawLine(posX, posY, width, color, thickness);
            // Bottom line
            DrawLine(posX, posY + height - (thickness - 1), width, color, thickness);
            // Left line
            DrawLine(posX, posY, thickness, color, height);
            // Right line
            DrawLine(posX + width - (thickness - 1), posY, thickness, color, height);
        }
    }

    void    Renderer::DrawFile(std::FILE *file, int posX, int posY, int width, int height)
    {
        
            return;

/*
        // Init buffer
        if (_buffer == nullptr)
            return;

        int     rowsize = height * 3;
        int     fileSize = width * height * 3;
        
        // reset pos in file
        std::fseek(file, static_cast<std::size_t>(0), SEEK_SET);

        _fileCmd.file = file;        
        _fileCmd.size = fileSize;
        _fileCmd.dst = _buffer;

        _fileCmd.read = 0;
        ThreadCommands::SetArgs((int)&_fileCmd);
        ThreadCommands::Execute(Commands::FS_READFILE);
        if (_fileCmd.read == _fileCmd.size)
        {
            DrawBuffer(_buffer, posX, posY, width, height);
            return;
        }
        // Correct posY
        posY = _rowSize[_target] - posY;


        int     rowPerRead = _bufferSize / rowsize;
        int     readSize = rowPerRead * rowsize;

        int     totalRead = width / rowPerRead;
        int     leftOver = width % rowPerRead;
        totalRead += leftOver > 0 ? 1 : 0;

        _fileCmd.file = file;        
        _fileCmd.size = readSize;
        _fileCmd.dst = _buffer;

        // reset pos in file
        std::fseek(file, static_cast<std::size_t>(0), SEEK_SET);
        
        while (--totalRead >= 0)
        {
            if (totalRead > 0)
            {
                _fileCmd.read = 0;
                ThreadCommands::SetArgs((int)&_fileCmd);
                ThreadCommands::Execute(Commands::FS_READFILE);
                if (_fileCmd.read != _fileCmd.size)
                {
                    ThreadCommands::SetArgs((int)&_fileCmd);
                    ThreadCommands::Execute(Commands::FS_READFILE);
                    if (_fileCmd.read != _fileCmd.size)
                        return;
                }
                for (int i = 0; i < rowPerRead; i++)
                {
                    _DrawData(posX, posY, _buffer + (rowsize * i), height);
                    posX++;
                }
            }
            else
            {
                _fileCmd.size = leftOver * rowsize;
                _fileCmd.read = 0;
                ThreadCommands::SetArgs((int)&_fileCmd);
                ThreadCommands::Execute(Commands::FS_READFILE);
                if (_fileCmd.read != _fileCmd.size)
                {
                    ThreadCommands::SetArgs((int)&_fileCmd);
                    ThreadCommands::Execute(Commands::FS_READFILE);
                    if (_fileCmd.read != _fileCmd.size)
                        return;
                }
                for (int i = 0; i > leftOver; i++)
                {
                    _DrawData(posX, posY, _buffer + (rowsize * i), height);
                    posX++;
                }
            }            
        }*/
    }

    void    Renderer::DrawBuffer(u8 *buffer, int posX, int posY, int width, int height)
    {
        const int padding = height * 3;
        // Correct posY
        //posY += (_rowSize[_target] - 240);

        posY = _rowSize[_target] - posY;
        int i = 0;
        while (--width >= 0)
        {
            _DrawData(posX, posY, buffer + i, height);
            posX++;
            i += padding;
        }

    }

    void    Renderer::DrawCheckBoxString(char *str, int posX, int &posY, bool isChecked, Color fg, Color color)
    {
        // Correct posY
        int y = posY + (_rowSize[_target] - 240);

        u8 unchecked[]=
        {
            0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF
        };

        u8 checked[] =
        {
            0x0, 0x0, 0x2, 0x46, 0x6C, 0x38, 0x10, 0x0
        };

        for (int yy = 0; yy < 8; yy++)
        {
            u8 u = unchecked[yy];
            u8 c = checked[yy];
            int x = 0;
            for (int xx = 7; xx >= 0; xx--, x++)
            {
                if ((u >> xx) & 1)
                {
                    _DrawPixel(posX + x, y + yy, fg);
                }
                if (isChecked && (c >> xx) & 1)
                {
                    _DrawPixel(posX + x, y + yy, color);
                }
            }
        }
        DrawString(str, posX + 10, posY, fg);
    }


//############################################################################################


    void        Renderer::RenderBGR8(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 3, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        if (!_render3D ||_target == BOTTOM)
        {
            while (--height >= 0)
            {
                *(pos++) = *(data++);
                *(pos++) = *(data++);
                *(pos++) = *(data++);
            }
        }
        else
        {
            while (--height >= 0)
            {
                *(pos++) = *(data);
                *(posR++) = *(data++);
                *(pos++) = *(data);
                *(posR++) = *(data++); 
                *(pos++) = *(data);
                *(posR++) = *(data++); 
            }
        }
        _length = 1;
    }

    void        Renderer::RenderRGB565(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 3;
                half.u |= (*(data++) & 0xFC) << 3;
                half.u |= (*(data++) & 0xF8) << 8;
                
                
                *(pos++) = half.b[0];
                *(pos++) = half.b[1];                
            }
        }
        else
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 3;
                half.u |= (*(data++) & 0xFC) << 3;
                half.u |= (*(data++) & 0xF8) << 8;;
                *(pos++) = half.b[0];                
                *(posR++) = half.b[0];
                *(pos++) = half.b[1]; 
                *(posR++) = half.b[1];                
            }
        }
        _length = 1;
    }

    void        Renderer::RenderRGB5A1(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 2;
                half.u |= (*(data++) & 0xF8) << 3;
                half.u |= (*(data++) & 0xF8) << 8;
                half.u |= 1;
                *(pos++) = half.b[0];
                *(pos++) = half.b[1];                
            }
        }
        else
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 2;
                half.u |= (*(data++) & 0xF8) << 3;
                half.u |= (*(data++) & 0xF8) << 8;
                half.u |= 1;
                *(pos++) = half.b[0];                
                *(posR++) = half.b[0];
                *(pos++) = half.b[1]; 
                *(posR++) = half.b[1];                
            }
        }
        _length = 1;
    }

    void        Renderer::RenderRGBA4(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset =  + GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF0);
                half.u |= (*(data++) & 0xF0) << 4;
                half.u |= (*(data++) & 0xF0) << 8;
                half.u |= 0x0F;
                *(pos++) = half.b[0];
                *(pos++) = half.b[1];                
            }
        }
        else
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF0);
                half.u |= (*(data++) & 0xF0) << 4;
                half.u |= (*(data++) & 0xF0) << 8;
                half.u |= 0x0F;
                *(pos++) = half.b[0];                
                *(posR++) = half.b[0];
                *(pos++) = half.b[1]; 
                *(posR++) = half.b[1];                
            }
        }
        _length = 1;
    }   

//###################################################


typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} __attribute__((packed)) Pixel;

extern "C" CFNT_s* g_sharedFont;
extern "C" int charPerSheet;
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
                Pixel   (*pScreen)[240] = (Pixel(*)[240])_framebuffer[_target];


                if (!(px >= 400 || px < 0 || py >= 240 || py < 0))
                {
                    u32 offset = GetFramebufferOffset(px, py, 3, _rowSize[_target]);
                    *(u8 *)left = _framebuffer + offset;
                    *(u8 *)right = _framebufferR + offset;

                    Color l = (*(left + 2), *(left + 1), *(left));

                    *left++ = (l.b * (0x0F - alpha) + color.b * (alpha + 1)) >> 4;
                    *left++ = (l.g * (0x0F - alpha) + color.g * (alpha + 1)) >> 4;
                    *left++ = (l.r * (0x0F - alpha) + color.r * (alpha + 1)) >> 4;
                    if (_render3D)
                    {
                        Color r = (*(right + 2), *(right + 1), *(right));

                        *right++ = (left.b * (0x0F - alpha) + color.b * (alpha + 1)) >> 4;
                        *right++ = (left.g * (0x0F - alpha) + color.g * (alpha + 1)) >> 4;
                        *right++ = (left.r * (0x0F - alpha) + color.r * (alpha + 1)) >> 4;
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