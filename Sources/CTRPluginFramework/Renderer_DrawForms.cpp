#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{
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
            DrawLine(posX, posY + height - (thickness - 1), width, color, thickness);
            // Left line
            DrawLine(posX, posY, thickness, color, height);
            // Right line
            DrawLine(posX + width - (thickness - 1), posY, thickness, color, height);
        }
    }
}