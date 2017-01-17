#include "CTRPluginFramework.hpp"

namepace CTRPluginFramework
{
    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void        Renderer::RenderRGBA8(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 4, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;
        while (--_length >= 0)
        {
            *(pos++) = 0xFF;
            *(pos++) = color.b;
            *(pos++) = color.g;
            *(pos++) = color.r;
            if (_target == TOP && _render3D)
            {
                *(posR++) = 0xFF;
                *(posR++) = color.b;
                *(posR++) = color.g;
                *(posR++) = color.r;
            }         
        }
        _length = 1;
    }

    void        Renderer::RenderBGR8(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 3, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        while (--_length >= 0)
        {
            *(pos++) = color.b;
            *(pos++) = color.g;
            *(pos++) = color.r;  
            if (_target == TOP && _render3D)
            {
                *(posR++) = color.b;
                *(posR++) = color.g;
                *(posR++) = color.r;                
            }          
        }
        _length = 1;
    }

    void        Renderer::RenderRGB565(int posX, int posY, Color color)
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

        half.u  = (color.b & 0xF8) << 8;
        half.u |= (color.g & 0xFC) << 3;
        half.u |= (color.r & 0xF8) >> 3;
        while (--_length >= 0)
        {
            *(pos++) = half.b[0];
            *(pos++) = half.b[1]; 
            if (_target == TOP && _render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1];              
            }              
        }
        _length = 1;
    }

    void        Renderer::RenderRGB5A1(int posX, int posY, Color color)
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

        half.u  = (color.b & 0xF8) << 8;
        half.u |= (color.g & 0xF8) << 3;
        half.u |= (color.r & 0xF8) >> 2;
        half.u |= 1;
        while (--_length >= 0)
        {
            *(pos++) = half.b[0];
            *(pos++) = half.b[1]; 
            if (_target == TOP && _render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1]; 
            }           
        }
        _length = 1;
    }

    void        Renderer::RenderRGBA4(int posX, int posY, Color color)
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

        half.u  = (color.b & 0xF0) << 8;
        half.u |= (color.g & 0xF0) << 4;
        half.u |= (color.r & 0xF0);
        half.u |= 0x0F;
        while (--_length >= 0)
        {
            *(pos++) = half.b[0];
            *(pos++) = half.b[1];   
            if (_target == TOP && _render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1];               
            }        
        }
        _length = 1;
    }

 // ##################################################################################################
     void        Renderer::RenderRGBA8(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 4, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                *(pos++) = 0xFF;
                *(pos++) = *(data++);
                *(pos++) = *(data++);
                *(pos++) = *(data++);
            }
        }
        else
        {
            while (--height >= 0)
            {
                *(pos++) = 0xFF;
                *(posR++) = 0xFF;
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
}