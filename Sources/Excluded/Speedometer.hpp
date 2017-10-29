#pragma once
#include "CTRPluginFramework.hpp"
#include <array>

namespace CTRPluginFramework
{
    #define BUFFER_WIDTH 10
    #define BUFFER_HEIGHT 10
    #define BUFFER_SIZE (BUFFER_WIDTH * BUFFER_HEIGHT)

    struct Pixel
    {
        u16     raw;

        void    Set(const u8 r, const u8 g, const u8 b);
        void    Set(const u32 color); // 0xRRGGBB00
        void    Set(const Color &color);
        Color   ToColor(void) const;
        u8      Red(void) const;
        void    Red(const u8 value);
        u8      Green(void) const;
        void    Green(const u8 value);
        u8      Blue(void) const;
        void    Blue(const u8 value);
    }PACKED;

    struct Image
    {
        Image(const u32 *src, const u32 width, const u32 height);

        void    Draw(std::array<Pixel, BUFFER_SIZE> &dst);
        void    Rotate(float angle, std::array<Pixel, BUFFER_SIZE> &dst) const;

        const u32   width;
        const u32   height;
        const u32   *source;
    };

    class Speedometer
    {
    public:
        ~Speedometer(void);
        static bool     Draw(const Screen &screen);
        static void     Worker(void);
        
        static const Speedometer &GetInstance(void);
        
        void    Update(void) const;
        void    Enable(void) const;
        void    Disable(void) const;

    private:
        Speedometer(void);

        static  Speedometer     _instance;

        float                   _speed;
        float                   _angle;
        int                     _renderedId;
        std::array<Pixel, BUFFER_SIZE>  _rendered[2];
        static const u8         *_compteur;
        static const u8         *_needle;
    };
}