#include "Icon.hpp"
#include "Color.hpp"
#include "Screen.hpp"
#include "Renderer.hpp"

namespace CTRPluginFramework
{
    extern "C" unsigned char *AddFavorite25;
    extern "C" unsigned char *AddFavoriteFilled25;
    extern "C" unsigned char *CloseWindow20;
    extern "C" unsigned char *CloseWindowFilled20;
    extern "C" unsigned char *Info25;
    extern "C" unsigned char *InfoFilled25;
    extern "C" unsigned char *UserManualFilled15;

    #define RGBA8 GSP_RGBA8_OES
    /*
    ** CheckBox
    ************/

    int     Icon::DrawCheckBox(IntVector &pos, bool isChecked)
    {

    }

    int     Icon::DrawCheckBox(int posX, int posY, bool isChecked)
    {

    }

    /*
    ** Close
    ** 20px * 20px
    **********/

    int     Icon::DrawClose(IntVector &pos, bool filled)
    {
        u8      *framebuf = nullptr;
        int     rowsize;
        int     bpp;

        GSPGPU_FramebufferFormats fmt;

        // Get target infos
        switch (Renderer::_target)
        {
            case Target::TOP:
            {
                framebuf = Screen::Top->GetLeftFramebuffer(pos.x, pos.y);
                fmt = Screen::Top->GetFormat();
                rowsize = Screen::Top->GetRowSize();
                bpp = Screen::Top->GetBytesPerPixel();
                break;
            }
            case Target::BOTTOM:
            {
                framebuf = Screen::Bottom->GetLeftFramebuffer(pos.x, pos.y);
                fmt = Screen::Bottom->GetFormat();
                rowsize = Screen::Bottom->GetRowSize();
                bpp = Screen::Bottom->GetBytesPerPixel();
                break;                
            }
            default:
                return (pos.x);
        }
        if (framebuf == nullptr)
            return (pos.x);

        // Define which version to draw
        u8 *img = filled ? CloseWindowFilled20 : CloseWindow20;

        // Draw
        for (int x = 0; x < 20; x++)
        {
            u8 *dst = framebuf + rowsize * x;
            int y = 0;
            while (y++ < 20)
            {
                Color px = Color::FromMemory(img, RGBA8);
                px.ToMemoryBlend(dst, fmt, BlendMode::Alpha);
                dst += bpp;
                img += 4;
            }
        }

        pos.x += 20;
        return (pos.x);
    }
}