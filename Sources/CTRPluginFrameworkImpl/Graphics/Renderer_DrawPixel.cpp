#include "types.h"

#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"

namespace CTRPluginFramework
{
    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void        Renderer::RenderRGBA8(int posX, int posY, Color &color)
    {
       /* if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;*/

       // u32     offset = GetFramebufferOffset(posX, posY, 4, _rowSize[_target]);

      /*  bool    is3D = _target == TOP && _useRender3D;
        bool    isLeftDouble = _useDoubleBuffer;
        bool    isRightDouble = _useDoubleBuffer; */       

        u8  *screen = _screen->GetLeftFramebuffer(posX, posY);// + offset;

again:
        int     length = _length;
        while (--length >= 0)
        {
            *(screen++) = 0xFF;
            *(screen++) = color.b;
            *(screen++) = color.g;
            *(screen++) = color.r;
        }
        /*
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
        }*/
    }

    void        Renderer::RenderBGR8(int posX, int posY, Color &color)
    {
       /* if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;*/
        /*u32     offset = GetFramebufferOffset(posX, posY, 3, _rowSize[_target]);

        bool    is3D = _target == TOP && _useRender3D;
        bool    isLeftDouble = _useDoubleBuffer;
        bool    isRightDouble = _useDoubleBuffer;  */      

        u8  *screen = _screen->GetLeftFramebuffer(posX, posY);// + offset;

again:
        int     length = _length;
        while (--length >= 0)
        {
            *(screen++) = color.b;
            *(screen++) = color.g;
            *(screen++) = color.r;
        }

        /*if (isLeftDouble)
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
        }*/
    }

    void        Renderer::RenderRGB565(int posX, int posY, Color &color)
    {
       /* if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;*/

        /*u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);

        bool    is3D = _target == TOP && _useRender3D;
        bool    isLeftDouble = _useDoubleBuffer;
        bool    isRightDouble = _useDoubleBuffer;     */   

        u8  *screen = _screen->GetLeftFramebuffer(posX, posY);// + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        half.u  = (color.b & 0xF8) << 8;
        half.u |= (color.g & 0xFC) << 3;
        half.u |= (color.r & 0xF8) >> 3;

again:
        int     length = _length;
        while (--length >= 0)
        {
            *(screen++) = half.b[0];
            *(screen++) = half.b[1];
        }

       /* if (isLeftDouble)
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
        }*/
    }

    void        Renderer::RenderRGB5A1(int posX, int posY, Color &color)
    {
      /*  if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;*/

        /*u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);

        bool    is3D = _target == TOP && _useRender3D;
        bool    isLeftDouble = _useDoubleBuffer;
        bool    isRightDouble = _useDoubleBuffer; */       

        u8  *screen = _screen->GetLeftFramebuffer(posX, posY);// + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        half.u  = (color.b & 0xF8) << 8;
        half.u |= (color.g & 0xF8) << 3;
        half.u |= (color.r & 0xF8) >> 2;
        half.u |= 1;

again:
        int     length = _length;
        while (--length >= 0)
        {
            *(screen++) = half.b[0];
            *(screen++) = half.b[1];
        }

        /*if (isLeftDouble)
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
        }*/
    }

    void        Renderer::RenderRGBA4(int posX, int posY, Color &color)
    {
     /*   if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;


        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);

        bool    is3D = _target == TOP && _useRender3D;
        bool    isLeftDouble = _useDoubleBuffer;
        bool    isRightDouble = _useDoubleBuffer;        
*/
        u8  *screen = _screen->GetLeftFramebuffer(posX, posY);// + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        half.u  = (color.b & 0xF0) << 8;
        half.u |= (color.g & 0xF0) << 4;
        half.u |= (color.r & 0xF0);
        half.u |= 0x0F;

again:
        int     length = _length;
        while (--length >= 0)
        {
            *(screen++) = half.b[0];
            *(screen++) = half.b[1];
        }

        /*if (isLeftDouble)
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
        }*/
    }
}
