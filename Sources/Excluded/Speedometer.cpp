#include "Speedometer.hpp"
#include <algorithm>
#include <cmath>

namespace CTRPluginFramework
{
    void    Pixel::Set(const u8 r, const u8 g, const u8 b)
    {
        raw = (r & 0xF8) << 8;
        raw |= (g & 0xFC) << 3;
        raw |= (b & 0xF8) >> 3;
    }

    void    Pixel::Set(const u32 color)
    {
        raw = (color & 0xF8) << 8;
        raw |= ((color >> 8) & 0xFC) << 3;
        raw |= ((color >> 24) & 0xF8) >> 3;
    }

    void    Pixel::Set(const Color &color)
    {
        raw = (color.r & 0xF8) << 8;
        raw |= (color.g & 0xFC) << 3;
        raw |= (color.b & 0xF8) >> 3;
    }

    Color   Pixel::ToColor(void) const
    {
        Color       color;

        color.a = 255;
        color.r = (raw >> 8) & 0xF8;
        color.g = (raw >> 3) & 0xFC;
        color.b = (raw << 3) & 0xF8;
        return (color);
    }

    u8      Pixel::Red(void) const
    {
        return ((raw >> 8) & 0xF8);
    }

    void    Pixel::Red(const u8 value)
    {
        raw &= ~0xF800;
        raw |= (value & 0xF8) << 8;
    }

    u8      Pixel::Green(void) const
    {
        return ((raw >> 3) & 0xFC);
    }

    void    Pixel::Green(const u8 value)
    {
        raw &= ~0x7E0;
        raw |= (value & 0xFC) << 3;
    }

    u8      Pixel::Blue(void) const
    {
        return ((raw << 3) & 0xF8);
    }

    void    Pixel::Blue(const u8 value)
    {
        raw &= ~0x1F;
        raw |= (value & 0xF8) >> 3;
    }

    Image::Image(const u32 *src, const u32 width, const u32 height) :
        width(width), height(height), source(src)
    {
    }

    void    Image::Draw(std::array<Pixel, BUFFER_SIZE> &dst)
    {
        const u32 *src = source;

        for (Pixel &pix : dst)
        {
            pix.Set(*src++);
        }
    }

    static inline u32 Clamp(u32 val, u32 low, u32 high)
    {
        return (val = std::min(std::max(val, low), high));
    }

    void    Image::Rotate(const float angle, std::array<Pixel, BUFFER_SIZE> &dst) const
    {
        // Affine transformation matrix 
        // H = [a, b, c]
        //     [d, e, f]

        float matrix[6] =
        {
            cos(-angle), -sin(-angle), BUFFER_WIDTH / 2 - BUFFER_WIDTH * cos(-angle) / 2,
            sin(-angle),  cos(-angle), BUFFER_HEIGHT / 2 - BUFFER_HEIGHT * cos(-angle) / 2
        };

        for (u32 row = 0; row < BUFFER_HEIGHT; ++row)
        {
            for (u32 col = 0; col < BUFFER_WIDTH; ++col)
            {
                int src_col = round(matrix[0] * col + matrix[1] * row + matrix[2]);

                src_col = Clamp(src_col, 0, BUFFER_WIDTH - 1);

                int src_row = round(matrix[3] * col + matrix[4] * row + matrix[5]);

                src_row = Clamp(src_row, 0, BUFFER_HEIGHT - 1);

                dst[row * BUFFER_WIDTH + col].Set(source[src_row * width + src_col]);
            }
        }
    }

    Speedometer::~Speedometer()
    {
    }

    Speedometer::Speedometer()
    {
    }
}
