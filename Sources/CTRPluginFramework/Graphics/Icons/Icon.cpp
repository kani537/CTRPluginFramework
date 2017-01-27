#include "types.h"
#include "ctrulib/services/gspgpu.h"

#include "CTRPluginFramework/Vector.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/Graphics/Icon.hpp"
#include "CTRPluginFramework/Screen.hpp"
#include "CTRPluginFramework/Graphics/Renderer.hpp"



namespace CTRPluginFramework
{
    extern "C" unsigned char *AddFavorite25;
    extern "C" unsigned char *AddFavoriteFilled25;
    extern "C" unsigned char *CheckedCheckbox;
    extern "C" unsigned char *UnCheckedCheckbox;
    extern "C" unsigned char *CloseWindow20;
    extern "C" unsigned char *CloseWindowFilled20;
    extern "C" unsigned char *FolderFilled;
    extern "C" unsigned char *Info25;
    extern "C" unsigned char *InfoFilled25;
    extern "C" unsigned char *UserManualFilled15;

    #define RGBA8 GSP_RGBA8_OES

    inline int Icon::DrawImg(u8 *img, int posX, int posY, int sizeX, int sizeY)
    {
        u8      *framebuf = nullptr;
        int     rowstride;
        int     bpp;

        posY += sizeY;
        GSPGPU_FramebufferFormats fmt;
        // Get target infos
        switch (Renderer::_target)
        {
            case Target::TOP:
            {
                framebuf = Screen::Top->GetLeftFramebuffer(posX, posY);
                Screen::Top->GetFramebufferInfos(rowstride, bpp, fmt);
                break;
            }
            case Target::BOTTOM:
            {
                framebuf = Screen::Bottom->GetLeftFramebuffer(posX, posY);
                Screen::Bottom->GetFramebufferInfos(rowstride, bpp, fmt);
                break;                
            }
            default:
                return (posX);
        }
        if (framebuf == nullptr)
            return (posX);

        // Draw
        for (int x = 0; x < sizeX; x++)
        {
            u8 *dst = framebuf + rowstride * x;
            int y = 0;
            while (y++ < sizeY)
            {
                Color px = Color::FromMemory(img, RGBA8);
                px.ToMemoryBlend(dst, fmt, BlendMode::Alpha);
                dst += bpp;
                img += 4;
            }
        }
        return (posX + sizeX);

    }

    /*
    ** CheckBox
    ** 15px * 15px
    ************/

    int     Icon::DrawCheckBox(IntVector &pos, bool isChecked)
    {
        // Define which version to draw
        u8 *img = isChecked ? CheckedCheckbox : UnCheckedCheckbox;

        return (DrawImg(img, pos.y, pos.y, 15, 15));
    }

    int     Icon::DrawCheckBox(int posX, int posY, bool isChecked)
    {
        // Define which version to draw
        u8 *img = isChecked ? CheckedCheckbox : UnCheckedCheckbox;

        return (DrawImg(img, posX, posY, 15, 15));
    }

    /*
    ** Close
    ** 20px * 20px
    **********/

    int     Icon::DrawClose(IntVector &pos, bool filled)
    {
        // Define which version to draw
        u8 *img = filled ? CloseWindowFilled20 : CloseWindow20;

        return (DrawImg(img, pos.x, pos.y, 20, 20));
    }

    int     Icon::DrawClose(int posX, int posY, bool filled)
    {
        // Define which version to draw
        u8 *img = filled ? CloseWindowFilled20 : CloseWindow20;

        return (DrawImg(img, posX, posY, 20, 20));
    }

    /*
    ** Folder
    ** 15px * 15px
    ***************/

    int     Icon::DrawFolder(IntVector &pos)
    {
        return (DrawImg(FolderFilled, pos.x, pos.y, 15, 15));
    }

    int     Icon::DrawFolder(int posX, int posY)
    {
        return (DrawImg(FolderFilled, posX, posY, 15, 15));
    }
}