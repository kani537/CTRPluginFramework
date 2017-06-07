#include "types.h"
#include "ctrulib/services/gspgpu.h"

#include "CTRPluginFramework/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

namespace CTRPluginFramework
{
    extern "C" unsigned char *About15;
    extern "C" unsigned char *AddFavorite25;
    extern "C" unsigned char *AddFavoriteFilled25;
    extern "C" unsigned char *CheckedCheckbox;
    extern "C" unsigned char *UnCheckedCheckbox;
    extern "C" unsigned char *CapsLockOn15;
    extern "C" unsigned char *CapsLockOnFilled15;
    extern "C" unsigned char *CentreofGravity15;
    extern "C" unsigned char *ClearSymbol15;
    extern "C" unsigned char *ClearSymbolFilled15;
    extern "C" unsigned char *CloseWindow20;
    extern "C" unsigned char *CloseWindowFilled20;
    extern "C" unsigned char *Controller15;
    extern "C" unsigned char *EnterKey15;
    extern "C" unsigned char *EnterKeyFilled15;
    extern "C" unsigned char *FolderFilled;
    extern "C" unsigned char *File15;
    extern "C" unsigned char *GameController15;
    extern "C" unsigned char *Grid15;
    extern "C" unsigned char *Happy15;
    extern "C" unsigned char *HappyFilled15;
    extern "C" unsigned char *Info25;
    extern "C" unsigned char *InfoFilled25;
    extern "C" unsigned char *HandCursor15;
    extern "C" unsigned char *Maintenance15;
    extern "C" unsigned char *Keyboard25;
    extern "C" unsigned char *KeyboardFilled25;
    extern "C" unsigned char *Search15;
    extern "C" unsigned char *Settings15;
    extern "C" unsigned char *Star15;
    extern "C" unsigned char *UserManualFilled15;

    #define RGBA8 GSP_RGBA8_OES

    struct Pixel
    {
        u8 a;
        u8 b;
        u8 g;
        u8 r;
    };

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
                Pixel *pix = (Pixel *)img;
                Color px(pix->r, pix->g, pix->b, pix->a);// = PrivColor::FromMemory(img, RGBA8);
                Color bg = PrivColor::FromFramebuffer(dst);
                Color blended = bg.Blend(px, Color::BlendMode::Alpha);
                dst = PrivColor::ToFramebuffer(dst, blended);
                img += 4;
            }
        }
        return (posX + sizeX);

    }

    /*
    ** About
    ** 15px * 15px
    **************/
    int     Icon::DrawAbout(int posX, int posY)
    {
        return (DrawImg(About15, posX, posY, 15, 15));
    }

    /*
    ** CapsLockOn
    ** 15px * 15px
    ************/
    int Icon::DrawCapsLockOn(int posX, int posY, bool isFilled)
    {
        u8 *img = isFilled ? CapsLockOnFilled15 : CapsLockOn15;

        return (DrawImg(img, posX, posY, 15, 15));
    }

    /*
    ** CentreOfGravity
    ** 15px * 15px
    **************/
    int     Icon::DrawCentreOfGravity(int posX, int posY)
    {
        return (DrawImg(CentreofGravity15, posX, posY, 15, 15));
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
    ** ClearSymbol
    ** 15px * 15px
    **********/
    int     Icon::DrawClearSymbol(int posX, int posY, bool filled)
    {
        u8 *img = filled ? ClearSymbolFilled15 : ClearSymbol15;

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
    ** Enter Key
    ** 15px * 15px
    **************/
    int     Icon::DrawEnterKey(int posX, int posY, bool filled)
    {
        u8 *img = filled ? EnterKeyFilled15 : EnterKey15;

        return (DrawImg(img, posX, posY, 15, 15));
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
    ** File
    ** 15px * 15px
    **************/
    int     Icon::DrawFile(int posX, int posY)
    {
        return (DrawImg(File15, posX, posY, 15, 15));
    }

    /*
    ** Game Controller
    ** 15px * 15px
    **************/
    int     Icon::DrawGameController(int posX, int posY)
    {
        return (DrawImg(GameController15, posX, posY, 15, 15));
    }

    /*
    ** Grid
    ** 15px * 15px
    **************/
    int     Icon::DrawGrid(int posX, int posY)
    {
        return (DrawImg(Grid15, posX, posY, 15, 15));
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
        u8      *framebuf = nullptr;
        u8      *framebuf2 = nullptr;
        int     rowstride;
        int     bpp;

        posY += 15;
        GSPGPU_FramebufferFormats fmt;
        // Get target infos
        switch (Renderer::_target)
        {
            case Target::TOP:
            {
                framebuf = Screen::Top->GetLeftFramebuffer(posX, posY);
                framebuf2 = Screen::Top->GetLeftFramebuffer(posX, posY, true);
                Screen::Top->GetFramebufferInfos(rowstride, bpp, fmt);
                break;
            }
            case Target::BOTTOM:
            {
                framebuf = Screen::Bottom->GetLeftFramebuffer(posX, posY);
                framebuf2 = Screen::Bottom->GetLeftFramebuffer(posX, posY, true);
                Screen::Bottom->GetFramebufferInfos(rowstride, bpp, fmt);
                break;                
            }
            default:
                return (posX);
        }
        if (framebuf == nullptr)
            return (posX);

        u8 *img = HandCursor15;
        // Draw
        for (int x = 0; x < 15; x++)
        {
            u8 *dst = framebuf + rowstride * x;
            u8 *dst2 = framebuf2 + rowstride * x;
            int y = 0;
            while (y++ < 15)
            {
                Pixel *pix = (Pixel *)img;
                Color px(pix->r, pix->g, pix->b, pix->a);//PrivColor::FromMemory(img, RGBA8);
                Color bg = PrivColor::FromFramebuffer(dst);
                Color blended = bg.Blend(px, Color::BlendMode::Alpha);
                dst = PrivColor::ToFramebuffer(dst, blended);
                dst2 = PrivColor::ToFramebuffer(dst2, blended);
                img += 4;
            }
        }
        return (posX + 15);
    }

    /*
    ** Happy face
    ** 15 px * 15 px
    ****************/
    int     Icon::DrawHappyFace(int posX, int posY, bool isFilled)
    {
        u8 *img = isFilled ? HappyFilled15 : Happy15;

        return (DrawImg(img, posX, posY, 15, 15));
    }

    /*
    ** Keyboard
    ** 25px * 25px
    **************/
    int     Icon::DrawKeyboard(int posX, int posY, bool filled)
    {
        u8 *img = filled ? KeyboardFilled25 : Keyboard25;

        return (DrawImg(img, posX, posY, 25, 25));
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
    ** Settings
    ** 15px * 15px
    ***************/

    int     Icon::DrawSettings(int posX, int posY)
    {
        return (DrawImg(Settings15, posX, posY, 15, 15));
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
