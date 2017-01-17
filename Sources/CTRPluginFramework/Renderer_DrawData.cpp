#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{
    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void        Renderer::RenderRGBA8(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 4, _rowSize[_target]);

        bool    is3D = _target == TOP && _render3D;
        bool    isLeftDouble = _doubleBuffer;
        bool    isRightDouble = _doubleBuffer;        

        u8  *screen = _screens[_target]->GetLeftFramebuffer() + offset;

again:
        int     length = height;
        u8      *d = data;
        while (--length >= 0)
        {
            *(screen++) = 0xFF;
            *(screen++) = *d++;
            *(screen++) = *d++;
            *(screen++) = *d++;
        }

        if (isLeftDouble)
        {
            screen = _screens[_target]->GetLeftFramebuffer(true) + offset;
            isLeftDouble = false;
            goto again;
        }

        if (is3D)
        {
            screen = _screens[_target]->GetRightFramebuffer() + offset;
            is3D = false;
            goto again;
        }

        if (isRightDouble)
        {
            screen = _screens[_target]->GetRightFramebuffer(true) + offset;
            isRightDouble = false;
            goto again;
        }
    }

    void        Renderer::RenderBGR8(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 3, _rowSize[_target]);
        bool    is3D = _target == TOP && _render3D;
        bool    isLeftDouble = _doubleBuffer;
        bool    isRightDouble = _doubleBuffer;        

        u8  *screen = _screens[_target]->GetLeftFramebuffer() + offset;

again:
        int     length = height;
        u8      *d = data;
        while (--length >= 0)
        {
            *(screen++) = *d++;
            *(screen++) = *d++;
            *(screen++) = *d++;
        }

        if (isLeftDouble)
        {
            screen = _screens[_target]->GetLeftFramebuffer(true) + offset;
            isLeftDouble = false;
            goto again;
        }

        if (is3D)
        {
            screen = _screens[_target]->GetRightFramebuffer() + offset;
            is3D = false;
            goto again;
        }

        if (isRightDouble)
        {
            screen = _screens[_target]->GetRightFramebuffer(true) + offset;
            isRightDouble = false;
            goto again;
        }
        _length = 1;
    }

    void        Renderer::RenderRGB565(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        bool    is3D = _target == TOP && _render3D;
        bool    isLeftDouble = _doubleBuffer;
        bool    isRightDouble = _doubleBuffer;        

        u8  *screen = _screens[_target]->GetLeftFramebuffer() + offset;

again:
        int     length = height;
        u8      *d = data;
        while (--length >= 0)
        {
            union
            {
                u16     u;
                u8      b[2];
            }     half;

            half.u = (*(d++) & 0xF8) >> 3;
            half.u |= (*(d++) & 0xFC) << 3;
            half.u |= (*(d++) & 0xF8) << 8;                
                
            *(screen++) = half.b[0];
            *(screen++) = half.b[1];
        }

        if (isLeftDouble)
        {
            screen = _screens[_target]->GetLeftFramebuffer(true) + offset;
            isLeftDouble = false;
            goto again;
        }

        if (is3D)
        {
            screen = _screens[_target]->GetRightFramebuffer() + offset;
            is3D = false;
            goto again;
        }

        if (isRightDouble)
        {
            screen = _screens[_target]->GetRightFramebuffer(true) + offset;
            isRightDouble = false;
            goto again;
        }
    }

    void        Renderer::RenderRGB5A1(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        bool    is3D = _target == TOP && _render3D;
        bool    isLeftDouble = _doubleBuffer;
        bool    isRightDouble = _doubleBuffer;        

        u8  *screen = _screens[_target]->GetLeftFramebuffer() + offset;

again:
        int     length = height;
        u8      *d = data;
        while (--length >= 0)
        {
            union
            {
                u16     u;
                u8      b[2];
            }     half;

            half.u = 1;
            half.u |= (*(d++) & 0xF8) >> 2;
            half.u |= (*(d++) & 0xF8) << 3;
            half.u |= (*(d++) & 0xF8) << 8;                
                
            *(screen++) = half.b[0];
            *(screen++) = half.b[1];
        }

        if (isLeftDouble)
        {
            screen = _screens[_target]->GetLeftFramebuffer(true) + offset;
            isLeftDouble = false;
            goto again;
        }

        if (is3D)
        {
            screen = _screens[_target]->GetRightFramebuffer() + offset;
            is3D = false;
            goto again;
        }

        if (isRightDouble)
        {
            screen = _screens[_target]->GetRightFramebuffer(true) + offset;
            isRightDouble = false;
            goto again;
        }
    }

    void        Renderer::RenderRGBA4(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset =  + GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        bool    is3D = _target == TOP && _render3D;
        bool    isLeftDouble = _doubleBuffer;
        bool    isRightDouble = _doubleBuffer;        

        u8  *screen = _screens[_target]->GetLeftFramebuffer() + offset;

again:
        int     length = height;
        u8      *d = data;
        while (--length >= 0)
        {
            union
            {
                u16     u;
                u8      b[2];
            }     half;

            half.u = (*(d++) & 0xF0);
            half.u |= (*(d++) & 0xF0) << 4;
            half.u |= (*(d++) & 0xF0) << 8;
            half.u |= 0x0F;               
                
            *(screen++) = half.b[0];
            *(screen++) = half.b[1];
        }

        if (isLeftDouble)
        {
            screen = _screens[_target]->GetLeftFramebuffer(true) + offset;
            isLeftDouble = false;
            goto again;
        }

        if (is3D)
        {
            screen = _screens[_target]->GetRightFramebuffer() + offset;
            is3D = false;
            goto again;
        }

        if (isRightDouble)
        {
            screen = _screens[_target]->GetRightFramebuffer(true) + offset;
            isRightDouble = false;
            goto again;
        }
    }   
}