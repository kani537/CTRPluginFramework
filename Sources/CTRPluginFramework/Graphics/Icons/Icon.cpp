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
    extern "C" unsigned char *Controller15;
    extern "C" unsigned char *FolderFilled;
    extern "C" unsigned char *Info25;
    extern "C" unsigned char *InfoFilled25;
    extern "C" unsigned char *HandCursor15;
    extern "C" unsigned char *Maintenance15;
    extern "C" unsigned char *Search15;
    extern "C" unsigned char *Star15;
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
                Color bg = Color::FromFramebuffer(dst);
                Color blended = bg.Blend(px, BlendMode::Alpha);
                dst = Color::ToFramebuffer(dst, blended);
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
    ** Controller
    ** 15px * 15px
    ***************/

    int     Icon::DrawController(int posX, int posY)
    {
        return (DrawImg(Controller15, posX, posY, 15, 15));
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

    /*
    ** Favorite
    ** 25px * 25px
    ***************/

    int     Icon::DrawAddFavorite(IntVector &pos, bool filled)
    {
        u8 *img = filled ? AddFavoriteFilled25 : AddFavorite25;
        return (DrawImg(img, pos.x, pos.y, 25, 25));
    }

    int     Icon::DrawAddFavorite(int posX, int posY, bool filled)
    {
        u8 *img = filled ? AddFavoriteFilled25 : AddFavorite25;
        return (DrawImg(img, posX, posY, 25, 25));
    } 

    int     Icon::DrawFavorite(IntVector &pos)
    {
        return (DrawImg(Star15, pos.x, pos.y, 15, 15));
    }

    int     Icon::DrawFavorite(int posX, int posY)
    {
        return (DrawImg(Star15, posX, posY, 15, 15));
    } 

    /*
    ** Info
    ** 25px * 25px
    ***************/

    int     Icon::DrawInfo(IntVector &pos, bool filled)
    {
        u8 *img = filled ? InfoFilled25 : Info25;
        return (DrawImg(img, pos.x, pos.y, 25, 25));
    }

    int     Icon::DrawInfo(int posX, int posY, bool filled)
    {
        u8 *img = filled ? InfoFilled25 : Info25;
        return (DrawImg(img, posX, posY, 25, 25));
    } 

    /*
    ** Guide
    ** 15px * 15px
    ***************/

    int     Icon::DrawGuide(IntVector &pos)
    {
        return (DrawImg(UserManualFilled15, pos.x, pos.y, 15, 15));
    }

    int     Icon::DrawGuide(int posX, int posY)
    {
        return (DrawImg(UserManualFilled15, posX, posY, 15, 15));
    }

    /*
    ** Hand Cursor
    ** 15px * 15px
    ***************/
    int     Icon::DrawHandCursor(int posX, int posY)
    {
        return (DrawImg(HandCursor15, posX, posY, 15, 15));
    }

    /*
    ** Search
    ** 15px * 15px
    ***************/

    int     Icon::DrawSearch(int posX, int posY)
    {
        return (DrawImg(Search15, posX, posY, 15, 15));
    }

    /*
    ** Tools
    ** 15px * 15px
    ***************/

    int     Icon::DrawTools(int posX, int posY)
    {
        return (DrawImg(Maintenance15, posX, posY, 15, 15));
    }
}
