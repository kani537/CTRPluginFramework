#include "types.h"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "font6x10Linux.h"

namespace CTRPluginFramework
{
    // Draw Character without background
    void     Renderer::DrawCharacter(int c, int posX, int posY, Color fg)
    {
        for (int yy = 0; yy < 10; yy++)
        {
            u8 charPos = font[c * 10 + yy];
            int x = 0;
            for (int xx = 6; xx >= 0; xx--, x++)
            {
                if ((charPos >> xx) & 1)
                {
                    _DrawPixel(posX + x, posY + yy, fg);
                }
            }
        }        
    }
    // Draw Character with background
    void     Renderer::DrawCharacter(int c, int posX, int posY, Color fg, Color bg)
    {
        for (int yy = 0; yy < 10; yy++)
        {
            u8 charPos = font[c * 10 + yy];
            int x = 0;
            for (int xx = 6; xx >= 0; xx--, x++)
            {
                if ((charPos >> xx) & 1)
                {
                    _DrawPixel(posX + x, posY + yy, fg);
                }
                else
                {
                    _DrawPixel(posX + x, posY + yy, bg);
                }
            }
        } 
    }
    // Draw Character with offset
    void     Renderer::DrawCharacter(int c, int offset, int posX, int posY, Color fg)
    {      
        for (int yy = 0; yy < 10; yy++)
        {
            u8 charPos = font[c * 10 + yy];
            int x = 0;
            for (int xx = 6 - offset; xx >= 0; xx--, x++)
            {
                if ((charPos >> xx) & 1)
                {
                    _DrawPixel(posX + x, posY + yy, fg);
                }
            }
        }  
    }

    int    Renderer::DrawString(char *str, int posX, int &posY, Color fg)
    {
        // Correct posY
       // int y = posY + (_rowSize[_target] - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, posY, fg);
            posX += 6;
        }
            posY += 10;
        return (posY);
    }

    int    Renderer::DrawString(char *str, int posX, int &posY, Color fg, Color bg)
    {
        // Correct posY
        //int y = posY + (_rowSize[_target] - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, posY, fg, bg);
            posX += 6;
        }
        posY += 10;
        return (posY);
    }

    int    Renderer::DrawString(char *str, int offset, int posX, int &posY, Color fg)
    {
        // Correct posY
        //int y = posY + (_rowSize[_target] - 240);
        str += (offset / 6);
        offset %= 6;
        while (*str)
        {
            DrawCharacter(*str++, offset, posX, posY, fg);
            if (offset)
            {                          
                posX -= offset;      
                offset = 0;
            }
            posX += 6;
        }
        posY += 10;
        return (posY);
    }

    void    Renderer::DrawCheckBoxString(char *str, int posX, int &posY, bool isChecked, Color fg, Color color)
    {
        // Correct posY
        //int y = posY + (_rowSize[_target] - 240);

        u8 unchecked[] =
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
                    _DrawPixel(posX + x, posY + yy, fg);
                }
                if (isChecked && (c >> xx) & 1)
                {
                    _DrawPixel(posX + x, posY + yy, color);
                }
            }
        }
        DrawString(str, posX + 10, posY, fg);
    }
}
