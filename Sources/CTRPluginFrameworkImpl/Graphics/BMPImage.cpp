#include "CTRPluginFrameworkImpl/Graphics/BMPImage.hpp"

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/System/Heap.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "ctrulib/allocator/vram.h"

namespace CTRPluginFramework
{
    static void memset32(void *dst, u8 c, u32 size)
    {
        u8 *dst8 = (u8 *)dst;

        // Get an aligned address first
        while ((u32)dst8 & 3 && size)
        {
            *dst8++ = c;
            --size;
        }

        if (!size || !dst8)
            return;

        // Do a 4 bytes memset
        u32     *dst32 = (u32 *)dst8;
        u32     *end32 = dst32 + (size >> 2);
        u32     v32 = c | (c << 8);

        v32 |= v32 << 16;

        while (dst32 != end32)
            *dst32++ = v32;

        if (!size || !dst32)
            return;

        // Do the last bytes
        dst8 = (u8 *)dst32;
        size &= 3;

        while (size--)
            *dst8++ = c;
    }

    void    BMPImage::BitmapFileHeader::Clear(void)
    {
        memset32(this, 0, sizeof(BitmapFileHeader));
    }

    bool    BMPImage::BitmapFileHeader::Read(File &file)
    {
        return file.Read(this, sizeof(BitmapFileHeader)) != File::OPResult::SUCCESS;
    }

    void    BMPImage::BitmapInformationHeader::Clear(void)
    {
        memset32(this, 0, sizeof(BitmapInformationHeader));
    }

    bool    BMPImage::BitmapInformationHeader::Read(File &file)
    {
        return file.Read(this, sizeof(BitmapInformationHeader)) != File::OPResult::SUCCESS;
    }

    const u32     BMPImage::HeaderSize = sizeof(BMPImage::BitmapFileHeader) + sizeof(BMPImage::BitmapInformationHeader);

    // Default CTOR
    BMPImage::BMPImage() = default;

    BMPImage::BMPImage(const std::string &filename) :
        _filename{ filename }
    {
        // Load the file
        _LoadBitmap();
        // Reverse channels
        RGBtoBGR();
    }

    BMPImage::BMPImage(const u32 width, const u32 height, bool canUseVram) :
        _width{ width }, _height{ height }
    {
        _rowIncrement = _width * _bytesPerPixel;
        _dataSize = height * _rowIncrement;

        // Create buffer
        if (canUseVram)
            _data = static_cast<u8 *>(vramAlloc(_dataSize + HeaderSize));

        if (_data == nullptr)
            _data = static_cast<u8 *>(Heap::Alloc(_dataSize + HeaderSize));

        if (_data == nullptr)
            return;

        DataClear();
        _loaded = true;
    }

    BMPImage::BMPImage(const BMPImage &src, const u32 width, const u32 height)
    {
        _CreateBitmap();
        FillWithImg(src);
    }

    BMPImage::~BMPImage(void)
    {
        if (_data != nullptr)
        {
            if (((u32)_data >> 24) < 0x10)
                Heap::Free(_data);
            else
                vramFree(_data);

            _data = nullptr;
        }
    }

    bool    BMPImage::IsLoaded(void) const
    {
        return _loaded;
    }

    u32     BMPImage::Width(void) const
    {
        return _width;
    }

    u32     BMPImage::Height(void) const
    {
        return _height;
    }

    u32     BMPImage::BytesPerPixel(void) const
    {
        return _bytesPerPixel;
    }

    const IntVector & BMPImage::GetDimensions(void) const
    {
        return _dimensions;
    }

    u8  *BMPImage::data(void) const
    {
        return _data + HeaderSize;
    }

    u8  *BMPImage::end(void) const
    {
        return _data + _dataSize + HeaderSize;
    }

    u8  *BMPImage::Row(u32 rowIndex) const
    {
        return &(_data + HeaderSize)[(rowIndex * _rowIncrement)];
    }

    void    BMPImage::DataClear(void)
    {
        memset32(_data + HeaderSize, 0, _dataSize);
    }

    void    BMPImage::SetWidthHeight(const u32 width, const u32 height)
    {
        if (width != _width || _height != height)
        {
            _width = width;
            _height = height;

            _CreateBitmap();
        }
    }

    void    BMPImage::Unload(void)
    {
        if (_data)
        {
            if (((u32)_data >> 24) < 0x10)
                Heap::Free(_data);
            else
                vramFree(_data);
        }
        _data = nullptr;
        _dataSize = 0;
        _loaded = false;
    }

    void     BMPImage::Draw(int x, int y)
    {
        ScreenImpl *scr = Renderer::GetContext()->screen;

        int posX = scr->IsTopScreen() ? x + (340 - _width) / 2 : x + (280 - _width) / 2;
        int posY = y + (200 - _height) / 2;


        u8      *img = (u8 *)data();
        u8      *left = scr->GetLeftFramebuffer(posX, posY);
        int     bpp = scr->GetBytesPerPixel();
        int     width = _width;
        int     height = _height;
        int     stride = scr->GetStride();
        Color   imgc;

        while (height--)
        {
            u8 *framebuf = left;
            for (x = 0; x < width; x++)
            {
                Pixel *pix = (Pixel *)img;
                imgc.r = pix->r;
                imgc.g = pix->g;
                imgc.b = pix->b;
                PrivColor::ToFramebuffer(framebuf, imgc);
                framebuf += stride;
                img += 3;
            }
            left -= bpp;
        }
    }

    void    BMPImage::Draw(const IntVector &point)
    {
        Draw(point.x, point.y);
    }

    void     BMPImage::Draw(const IntRect &area, float fade)
    {
        ScreenImpl *scr = Renderer::GetContext()->screen;

        int posX = area.leftTop.x;
        int posY = area.leftTop.y;
        int width = area.size.x;
        int height = area.size.y;

        int xOffset = 0;
        int startY = 0;

        if (_width < width)
        {
          posX += (width - _width) / 2;
          width = _width;
        }
        else if (_width > width)
          xOffset = ((_width - area.size.x) / 2) * 3;

        if (_height < area.size.y)
        {
          posY += (height - _height) / 2;
          height = _height;
        }
        else if (_height > height)
          startY = (_height - height) / 2;


        if (xOffset < 0)
          xOffset = 0;

        Color   imgc;
        u32     bpp = scr->GetBytesPerPixel();
        u32     stride = scr->GetStride();

        if (fade == 0.f)
        {
            u8 *left = scr->GetLeftFramebuffer(posX, posY);

            while (height--)
            {
                u8 *img = Row(startY++) + xOffset;
                u8 *framebuf = left;
                for (int x = 0; x < width; x++)
                {
                    Pixel *pix = (Pixel *)img;

                    imgc.r = pix->r;
                    imgc.g = pix->g;
                    imgc.b = pix->b;

                    PrivColor::ToFramebuffer(framebuf, imgc);

                    framebuf += stride;
                    img += 3;
                }
                left -= bpp;
            }
        }
        else
        {
            u8 *left = scr->GetLeftFramebuffer(posX, posY);

            while (height--)
            {
                u8 *framebuf = left;
                u8 *img = Row(startY++) + xOffset;

                for (int x = 0; x < width; x++)
                {
                    Pixel *pix = (Pixel *)img;

                    imgc.r = pix->r;
                    imgc.g = pix->g;
                    imgc.b = pix->b;

                    PrivColor::ToFramebuffer(framebuf, imgc.Fade(fade));
                    framebuf += stride;
                    img += 3;
                }
                left -= bpp;
            }
        }
    }

    extern Handle __fileHandle;
    void    BMPImage::SaveImage(const std::string &fileName) const
    {
        File file(fileName, File::RWC | File::TRUNCATE);

        //u32 priority = 0xFFFFFFF0;

        if (!file.IsOpen())
        {
            OSD::Notify("BMP Error: couldn't open the file");
            return;
        }

        // I suppose the lower the better like threads
        //  file.SetPriority(0);

        BitmapFileHeader *          bfh = (BitmapFileHeader *)_data;
        BitmapInformationHeader *   bih = (BitmapInformationHeader *)(_data + sizeof(BitmapFileHeader));

        bih->width            = _width;
        bih->height           = _height;
        bih->bit_count        = static_cast<unsigned short>(_bytesPerPixel << 3);
        bih->clr_important    = 0;
        bih->clr_used         = 0;
        bih->compression      = 0;
        bih->planes           = 1;
        bih->size             = sizeof(BitmapInformationHeader);
        bih->x_pels_per_meter = 0;
        bih->y_pels_per_meter = 0;
        bih->size_image       = (((bih->width * _bytesPerPixel) + 3) & 0x0000FFFC) * bih->height;

        bfh->type             = 19778;
        bfh->size             = sizeof(BitmapFileHeader) + sizeof(BitmapInformationHeader) + bih->size_image;
        bfh->reserved1        = 0;
        bfh->reserved2        = 0;
        bfh->off_bits         = sizeof(BitmapInformationHeader) + sizeof(BitmapFileHeader);


        // Data have to be on Heap and not in vram
        file.Write(_data, _dataSize + HeaderSize);
    }

    void    BMPImage::BGRtoRGB(void)
    {
        if ((BGR_Mode == _channelMode) && (3 == _bytesPerPixel))
        {
            ReverseChannels();
            _channelMode = RGB_Mode;
        }
    }

    void    BMPImage::RGBtoBGR(void)
    {
        if ((RGB_Mode == _channelMode) && (3 == _bytesPerPixel))
        {
            ReverseChannels();
            _channelMode = BGR_Mode;
        }
    }

    void    BMPImage::ReverseChannels(void)
    {
        if (3 != _bytesPerPixel)
            return;

        for (unsigned char *itr = data(); itr < end(); itr += _bytesPerPixel)
        {
            std::swap(*(itr + 0), *(itr + 2));
        }
    }

    void    BMPImage::FillWithImg(const BMPImage &src)
    {
        if (!src._loaded)
          return;

        _loaded = true;

        DataClear();

        int   startY = 0;
        int   offsetX = 0;

        int   srcWidth = src._width;
        int   srcHeight = src._height;

        if (srcWidth < _width)
          offsetX = (_width - srcWidth) / 2 * 3;
        if (src._height < _height)
          startY = (_height - srcHeight) / 2;

        for (int y = 0; y < srcHeight; y++)
        {
            unsigned char  *imgSrc = src.Row(y);
            unsigned char  *imgDst = Row(y + startY) + offsetX;
            for (int x = 0; x < srcWidth; x++)
            {
               *imgDst++ = *imgSrc++;
               *imgDst++ = *imgSrc++;
               *imgDst++ = *imgSrc++;
            }
        }
    }

    bool    BMPImage::Resample(BMPImage &dest, int newWidth, int newHeight)
    {
        //if(_data == NULL) return false;
        //
        // Get a new buuffer to interpolate into
        if (!_loaded)
            return (false);

        dest.SetWidthHeight(newWidth, newHeight);

        if (!dest._loaded)
            return (false);

        unsigned char *newData = dest.data(); //new unsigned char [newWidth * newHeight * 3];

        double scaleWidth = (double)newWidth / (double)_width;
        double scaleHeight = (double)newHeight / (double)_height;

        for (int cy = 0; cy < newHeight; cy++)
        {
            for (int cx = 0; cx < newWidth; cx++)
            {
                int pixel = (cy * (newWidth * 3)) + (cx * 3);
                int nearestMatch = (((int)(cy / scaleHeight) * (_width * 3)) + ((int)(cx / scaleWidth) * 3));

                newData[pixel] = _data[nearestMatch];
                newData[pixel + 1] = _data[nearestMatch + 1];
                newData[pixel + 2] = _data[nearestMatch + 2];
            }
        }

        return true;
    }

    void    BMPImage::SubSample(BMPImage &dest)
    {
        /*
           Half sub-sample of original image.
        */
        if (!_loaded)
            return;

        dest._loaded = true;
        unsigned int w = 0;
        unsigned int h = 0;

        bool odd_width = false;
        bool odd_height = false;

        if (0 == (_width % 2))
            w = _width / 2;
        else
        {
            w = 1 + (_width / 2);
            odd_width = true;
        }

        if (0 == (_height % 2))
            h = _height / 2;
        else
        {
            h = 1 + (_height / 2);
            odd_height = true;
        }

        unsigned int horizontal_upper = (odd_width) ? (w - 1) : w;
        unsigned int vertical_upper = (odd_height) ? (h - 1) : h;

        dest.SetWidthHeight(w, h);

        unsigned char *s_itr[3]{};
        const unsigned char *itr1[3]{};
        const unsigned char *itr2[3]{};

        s_itr[0] = dest.data() + 0;
        s_itr[1] = dest.data() + 1;
        s_itr[2] = dest.data() + 2;

        itr1[0] = data() + 0;
        itr1[1] = data() + 1;
        itr1[2] = data() + 2;

        itr2[0] = data() + _rowIncrement + 0;
        itr2[1] = data() + _rowIncrement + 1;
        itr2[2] = data() + _rowIncrement + 2;

        unsigned int total = 0;

        for (unsigned int j = 0; j < vertical_upper; ++j)
        {
            for (unsigned int i = 0; i < horizontal_upper; ++i)
            {
                for (unsigned int k = 0; k < _bytesPerPixel; s_itr[k] += _bytesPerPixel, ++k)
                {
                    total = 0;
                    total += *(itr1[k]);
                    total += *(itr1[k]);
                    total += *(itr2[k]);
                    total += *(itr2[k]);

                    itr1[k] += _bytesPerPixel;
                    itr1[k] += _bytesPerPixel;
                    itr2[k] += _bytesPerPixel;
                    itr2[k] += _bytesPerPixel;

                    *(s_itr[k]) = static_cast<unsigned char>(total >> 2);
                }
            }

            if (odd_width)
            {
                for (unsigned int k = 0; k < _bytesPerPixel; s_itr[k] += _bytesPerPixel, ++k)
                {
                    total = 0;
                    total += *(itr1[k]);
                    total += *(itr2[k]);

                    itr1[k] += _bytesPerPixel;
                    itr2[k] += _bytesPerPixel;

                    *(s_itr[k]) = static_cast<unsigned char>(total >> 1);
                }
            }

            for (unsigned int k = 0; k < _bytesPerPixel; ++k)
            {
                itr1[k] += _rowIncrement;
            }

            if (j != (vertical_upper - 1))
            {
                for (unsigned int k = 0; k < _bytesPerPixel; ++k)
                {
                    itr2[k] += _rowIncrement;
                }
            }
        }

        if (odd_height)
        {
            for (unsigned int i = 0; i < horizontal_upper; ++i)
            {
                for (unsigned int k = 0; k < _bytesPerPixel; s_itr[k] += _bytesPerPixel, ++k)
                {
                    total = 0;
                    total += *(itr1[k]);
                    total += *(itr2[k]);

                    itr1[k] += _bytesPerPixel;
                    itr2[k] += _bytesPerPixel;

                    *(s_itr[k]) = static_cast<unsigned char>(total >> 1);
                }
            }

            if (odd_width)
            {
                for (unsigned int k = 0; k < _bytesPerPixel; ++k)
                {
                    (*(s_itr[k])) = *(itr1[k]);
                }
            }
        }
    }

    void    BMPImage::UpSample(BMPImage &dest)
    {
        /*
           2x up-sample of original image.
        */

        dest.SetWidthHeight(2 * _width, 2 * _height);

        const unsigned char *s_itr[3];
        unsigned char *itr1[3];
        unsigned char *itr2[3];

        s_itr[0] = data() + 0;
        s_itr[1] = data() + 1;
        s_itr[2] = data() + 2;

        itr1[0] = dest.data() + 0;
        itr1[1] = dest.data() + 1;
        itr1[2] = dest.data() + 2;

        itr2[0] = dest.data() + dest._rowIncrement + 0;
        itr2[1] = dest.data() + dest._rowIncrement + 1;
        itr2[2] = dest.data() + dest._rowIncrement + 2;

        for (unsigned int j = 0; j < _height; ++j)
        {
            for (unsigned int i = 0; i < _width; ++i)
            {
                for (unsigned int k = 0; k < _bytesPerPixel; s_itr[k] += _bytesPerPixel, ++k)
                {
                    *(itr1[k]) = *(s_itr[k]);
                    itr1[k] += _bytesPerPixel;
                    *(itr1[k]) = *(s_itr[k]);
                    itr1[k] += _bytesPerPixel;

                    *(itr2[k]) = *(s_itr[k]);
                    itr2[k] += _bytesPerPixel;
                    *(itr2[k]) = *(s_itr[k]);
                    itr2[k] += _bytesPerPixel;
                }
            }

            for (unsigned int k = 0; k < _bytesPerPixel; ++k)
            {
                itr1[k] += dest._rowIncrement;
                itr2[k] += dest._rowIncrement;
            }
        }
    }

    bool    BMPImage::Region(const u32 &x, const u32 &y, const u32 &width, const u32 &height, BMPImage &destImage)
    {
        if ((x + width) > _width)
            return (false);
        if ((y + height) > _height)
            return (false);

        if ((destImage._width < _width) || (destImage._height < _height))
            destImage.SetWidthHeight(width, height);

        for (u32 r = 0; r < height; ++r)
        {
            u8 *itr1 = Row(r + y) + x * _bytesPerPixel;
            u8 *itr1_end = itr1 + (width * _bytesPerPixel);
            u8 *itr2 = destImage.Row(r);

            std::copy(itr1, itr1_end, itr2);
        }
        destImage._loaded = true;
        return (true);
    }

    bool    BMPImage::RoiFromCenter(const u32 cx, const u32 cy, const u32 &width, const u32 &height, BMPImage &destImage)
    {
        return (Region(cx - (width / 2), cy - (height / 2), width, height, destImage));
    }

    int     BMPImage::_CreateBitmap(void)
    {
        // Free current buffer if necessary
        Unload();

        _rowIncrement = _width * _bytesPerPixel;
        _dataSize = _height * _rowIncrement;

        // Try to alloc in vram first
        if ((_data = static_cast<u8 *>(vramAlloc(_dataSize + HeaderSize))) == nullptr)
            _data = static_cast<u8 *>(Heap::Alloc(_dataSize + HeaderSize));

        // If allocation failed
        if (_data == nullptr)
        {
            _dataSize = 0;
            _loaded = false;
            return (-1);
        }

        // Fill with black
        DataClear();
        _loaded = true;
        return (0);
    }

    void     BMPImage::_LoadBitmap(void)
    {
        Unload();
        _width  = 0;
        _height = 0;

        File                    file(_filename, File::READ);
        BitmapFileHeader        bfh = {};
        BitmapInformationHeader bih = {};

        if (!file.IsOpen())
            return;

        if (file.GetSize() >= 0x50000)
        {
            OSD::Notify("BMP Error: The file is too big.", Color::Red);
            return;
        }

        if (bfh.Read(file))
        {
            OSD::Notify("BMP Error: Error while reading BFH", Color::Red);
            return;
        }

        if (bih.Read(file))
        {
            OSD::Notify("BMP Error: Error while reading BIH", Color::Red);
            return;
        }

        if (bfh.type != 19778)
        {
            OSD::Notify(Utils::Format("BMP Error: Unexpected type value: %d", bfh.type), Color::Red);
            return;
        }

        if (bih.bit_count != 24)
        {
            OSD::Notify(Utils::Format("BMP Error: Unexpected bit depth: %d", bih.bit_count), Color::Red);
            return;
        }

        _width  = bih.width;
        _height = bih.height;

        // Temporary limit ?
        if (_width > 340 || _height > 200)
        {
            OSD::Notify("BMP Error: file should not be higher than 340px * 200px", Color::Red);
            return;
        }

        _dimensions = IntVector(_width, _height);
        _bytesPerPixel = bih.bit_count >> 3;

        // Go to the data position in file
        file.Seek(bfh.off_bits, File::SET);

        // Try to alloc the buffer
        if (_CreateBitmap())
        {
            OSD::Notify("BMP Error: Error while allocating required space.", Color::Red);
            return;
        }

        u32   padding = (4 - ((3 * _width) % 4)) % 4;
        int   rows = _height / 5;
        int   rowWidth = _rowIncrement + padding;
        int   totalwidth = rows * rowWidth;
        int   oddRows = rows * 5;

        // Temporary buffer
        u8    *buf = (u8 *)Heap::Alloc(totalwidth);

        if (buf == nullptr)
        {
            OSD::Notify("BMP Error: temp buffer allocation failed");
            return;
        }

        int i;
        for (i = 0; i < oddRows; )
        {
            file.Read(buf, totalwidth);

            for (int j = 0; j < rows && i < oddRows; ++j)
            {
                u8  *dataPtr = Row(_height - i++ - 1); // read in inverted row order
                u8  *bufPtr = buf + j * rowWidth;

                std::memcpy(dataPtr, bufPtr, _rowIncrement);
            }
        }

        file.Read(buf, totalwidth);

        int rest = _height % 5;
        for (int k = 0; k < rest; ++k)
        {
            unsigned char *dataPtr = Row(_height - i++ - 1);
            unsigned char *bufPtr = buf + k * rowWidth;

            std::memcpy(dataPtr, bufPtr, _rowIncrement);
        }

        // Release our temporary buffer
        Heap::Free(buf);
        _loaded = true;
    }
}
