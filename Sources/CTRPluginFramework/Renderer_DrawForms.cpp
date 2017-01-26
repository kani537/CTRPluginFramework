#include "CTRPluginFramework.hpp"
#include <algorithm>
#include <queue>

namespace CTRPluginFramework
{
    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
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
        _length = 1;
    }

    void        Renderer::DrawLine(IntVector start, IntVector end, Color color)
    {  
        int posX = start.x;
        int posY = start.y;
        int width = end.x - posX;
        int height = 1 + end.y - posY;

        // Correct posY
        posY += (_rowSize[_target] - 240);
        for (int x = 0; x < width; x++)
        {
            _length = height;
            _DrawPixel(posX + x, posY, color);
        }
        _length = 1;
    }



    void        Renderer::Arc(int posX, int posY, int r, Color color)
    {
        int d;
        int y;
        int x = 0;
        y = r;
        d = 1 - r;
        _DrawPixel(x,y, color);
        while ( y > x ) 
        {
            if ( d < 0 )
                d += 2 * x + 3 ;
            else 
            {
                d += 2 * (x - y) + 5 ;
                y--; 
            }
            x++ ;
            _DrawPixel(x + posX, posY + (r - y), color) ; 
        }
    }

    void Renderer::Ellipse(int posX, int posY, long a, long b, Color color) 
    {
        int x;
        int y;
        double d1;
        double d2;
        x = 0;
        y = b;
        d1 = b * b - a * a * b + a * a / 4;
        _DrawPixel(posX, posY, color);
        while ( a * a * (y - .5) > b * b * (x + 1)) 
        {
            if ( d1 < 0 ) 
            {
                d1 += b * b * (2 * x + 3);
                x++; 
            }
            else 
            {
                d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
                x++;
                y--; 
            }
            _DrawPixel(posX + x, posY + (b - y), color); 
        }
        d2 = b * b * (x + .5) * (x + .5) + a * a * (y - 1) * (y - 1) - a * a * b * b;
        while ( y > 0 ) 
        {
            if ( d2 < 0 ) 
            {
                d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
                y-- ;
                x++ ; 
            }
            else 
            {
                d2 += a * a * (-2 * y + 3);
                y--; 
            }
            _DrawPixel(posX + x, posY + (b - y), color);
        }
    }

    void Renderer::EllipseIncomplete(int posX, int posY, float a, float b, int max, int aff, Color color) 
    {
        int     x;
        int     y;
        float   d1;
        int     d2;

        x = 0;
        y = b;
        d1 = b * b - a * a * b + a * a / 4;
        _DrawPixel(posX + x, posY, color);
        _DrawPixel(posX + x, posY + (b - y), color);
        _DrawPixel(posX + x, posY - (b - y), color);
        _DrawPixel(posX - x, posY - (b - y), color);
        _DrawPixel(posX - x, posY + (b - y), color);
        int cpt = 0;

        while ((a * a * (y - .5) > b * b * (x + 1)) && (cpt < max)) 
        {
            cpt++;
            if (d1 < 0) 
            {
                d1 += b * b * (2 * x + 3);
                x++;
            }
            else 
            {
                d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
                x++;
                y--; 
            }
            _DrawPixel(posX + x, posY + (b - y), color);
            _DrawPixel(posX + x, posY - (b - y), color);
            _DrawPixel(posX - x, posY - (b - y), color);
            _DrawPixel(posX - x, posY + (b - y), color);
        }
        d2 =(float)(b * b * (x + .5) * (x + .5) + a * a * (y - 1) * (y - 1) - a * a * b * b);
        while ((y > 0 ) && (cpt < max )) 
        {
            cpt++;
            if (d2 < 0) 
            {
                d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
                y--;
                x++; 
            }
            else 
            {
                d2 += a * a * (-2 * y + 3);
                y--; 
            }
            _DrawPixel(posX + x, posY + (b - y), color);
            _DrawPixel(posX + x, posY - (b - y), color);
            _DrawPixel(posX - x, posY - (b - y), color);
            _DrawPixel(posX - x, posY + (b - y), color);        }
    }

    void Renderer::RoundedRectangle(const IntRect &rect, float radius, int max, Color color, bool mustFill, Color fillColor) 
    {
        int     x;
        int     y;
        float   d1;
        int     d2;

        
        

        int     width = rect._size.x;
        int     height = rect._size.y;

        int     posX = rect._leftTopCorner.x;
        int     posY = rect._leftTopCorner.y;

        int     posYBak;
        

        if (width < 0)
        {
            width = -width;
            posX -= width;
        }
        if (height < 0)
        {
            height = -height;
            posY -= height;
        }
        posYBak = posY;
        // Correct posY
        posY += (_rowSize[_target] - 240);
        int cpt = 0;

        x = 0;
        y = radius;
        d1 = radius * radius - radius * radius * radius + radius * radius / 4;

        float d1Bak = d1;

        while ((radius * radius * (radius - .5) > radius * radius * (x + 1)) && (cpt < max)) 
        {
            cpt++;
            if (d1 < 0) 
            {
                d1 += radius * radius * (2 * x + 3);
                x++;
            }
            else 
            {
                d1 += radius * radius * (2 * x + 3) + radius * radius * (-2 * y + 2);
                x++;
                y--; 
            }
        }
        while ((y > 0 ) && (cpt < max )) 
        {
            cpt++;
            if (d2 < 0) 
            {
                d2 += radius * radius * (2 * x + 2) + radius * radius * (-2 * y + 3);
                y--;
                x++; 
            }
            else 
            {
                d2 += radius * radius * (-2 * y + 3);
                y--; 
            }
        }
        d1 = d1Bak;
        int rHeight = radius - y;
        int rWidth = x;

        y = radius;
        x = 0; 

        width -= rWidth;
        // Right Top Corner
        _DrawPixel(posX + x + width, posY + (radius - y), color);
        // Right Bottom Corner
        _DrawPixel(posX + x + width , posY + height - (radius - y), color);
        // Left Bottom Corner
        _DrawPixel(posX - x + rWidth, posY - (radius - y) + height, color);
        // Left Top Corner
        _DrawPixel(posX - x + rWidth, posY + (radius - y), color);
        cpt = 0;

        while ((radius * radius * (radius - .5) > radius * radius * (x + 1)) && (cpt < max)) 
        {
            cpt++;
            if (d1 < 0) 
            {
                d1 += radius * radius * (2 * x + 3);
                x++;
            }
            else 
            {
                d1 += radius * radius * (2 * x + 3) + radius * radius * (-2 * y + 2);
                x++;
                y--; 
            }
            // Right Top Corner
            _DrawPixel(posX + x + width, posY + (radius - y), color);
            // Right Bottom Corner
            _DrawPixel(posX + x + width , posY + height - (radius - y), color);
            // Left Bottom Corner
            _DrawPixel(posX - x + rWidth, posY - (radius - y) + height, color);
            // Left Top Corner
            _DrawPixel(posX - x + rWidth, posY + (radius - y), color);
        }
        d2 =(float)(radius * radius * (x + .5) * (x + .5) + radius * radius * (y - 1) * (y - 1) - radius * radius * radius * radius);
        while ((y > 0 ) && (cpt < max )) 
        {
            cpt++;
            if (d2 < 0) 
            {
                d2 += radius * radius * (2 * x + 2) + radius * radius * (-2 * y + 3);
                y--;
                x++; 
            }
            else 
            {
                d2 += radius * radius * (-2 * y + 3);
                y--; 
            }
            // Right Top Corner
            _DrawPixel(posX + x + width, posY + (radius - y), color);
            // Right Bottom Corner
            _DrawPixel(posX + x + width , posY + height - (radius - y), color);
            // Left Bottom Corner
            _DrawPixel(posX - x + rWidth, posY - (radius - y) + height, color);
            // Left Top Corner
            _DrawPixel(posX - x + rWidth, posY + (radius - y), color);
        }

        // Top line
        DrawLine(posX + rWidth, posYBak - 1, width - rWidth, color);
        // Bottom line
        DrawLine(posX + rWidth, posYBak + height - 1, width - rWidth, color);
        // Left line
        DrawLine(posX - x + rWidth, posYBak + rHeight - 1, 1, color, height - (rHeight* 2));
        // Right line
        DrawLine(posX + x + width, posYBak + rHeight, 1, color, height - (rHeight * 2));

        if (mustFill)
        {
            IntVector start = rect._leftTopCorner;
            IntRect area = rect;
            area._leftTopCorner.x -= 1;
            area._size.x += 2;

            start.x += rWidth + 1;
            start.y++;

            FormFiller(start, area, true, fillColor, color);           
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
            DrawLine(posX, posY + height - thickness, width, color, thickness);
            // Left line
            DrawLine(posX, posY, thickness, color, height);
            // Right line
            DrawLine(posX + width - thickness, posY, thickness, color, height);
        }
    }

    void    Renderer::FormFiller(const IntVector &start, const IntRect &area, bool singlePoint, Color &fill, Color &limit) 
    {
        std::queue<IntVector> fpQueue;

        const int minX = area._leftTopCorner.x;
        const int minY = area._leftTopCorner.y;        
        const int maxX = minX + area._size.x;
        const int maxY = minY + area._size.y;

        Screen *scr = _screens[_target];
        GSPGPU_FramebufferFormats fmt = scr->GetFormat();

        int y = start.y;

        do
        {
            int x = start.x;
            // Get line frontiers points
            do
            {
                Color bg = Color::FromMemory(scr->GetLeftFramebuffer(x, y), fmt);
                if (bg != limit && x < maxX)
                {
                    do 
                    {
                        x++;
                        bg = Color::FromMemory(scr->GetLeftFramebuffer(x, y), fmt);
                    } while ((bg != limit) && x < maxX);     
                    // Store frontier point
                    if (bg == limit)
                        fpQueue.push(IntVector(x, y));    
                }            
                x++;
            } while (x < maxX && !singlePoint);
            // While queue isn't empty
            while (!fpQueue.empty())
            {
                IntVector rPoint = fpQueue.front();
                fpQueue.pop();

                int left = rPoint.x;
                Color bg;
                do
                {
                    left--;
                    bg = Color::FromMemory(scr->GetLeftFramebuffer(left, y), fmt);
                } while (bg != limit && left > minX);

                // If width is at least 1, draw line
                if (rPoint.x - left > 1)
                {
                    IntVector lPoint = IntVector(left + 1, y);
                    DrawLine(lPoint, rPoint, fill);
                }
            }
            y++;
        } while (y <= maxY);
    }
}
