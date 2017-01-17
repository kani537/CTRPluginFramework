#include "CTRPluginFramework/Color.hpp"
#include "CTRPluginFramework/Renderer.hpp"
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
        int y = posY + (_rowSize[_target] - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, y, fg);
            posX += 6;
        }
            posY += 10;
        return (posY);
    }

    int    Renderer::DrawString(char *str, int posX, int &posY, Color fg, Color bg)
    {
        // Correct posY
        int y = posY + (_rowSize[_target] - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, y, fg, bg);
            posX += 6;
        }
        posY += 10;
        return (posY);
    }

    int    Renderer::DrawString(char *str, int offset, int posX, int &posY, Color fg)
    {
        // Correct posY
        int y = posY + (_rowSize[_target] - 240);
        str += (offset / 6);
        offset %= 6;
        while (*str)
        {
            DrawCharacter(*str++, offset, posX, y, fg);
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
}
