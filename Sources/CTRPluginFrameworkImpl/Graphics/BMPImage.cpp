#include "CTRPluginFrameworkImpl/Graphics/BMPImage.hpp"

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"

#include "ctrulib/allocator/linear.h"
#include "3DS.h"
#include "CTRPluginFramework/Utils/Utils.hpp"

namespace CTRPluginFramework
{
    BMPImage::~BMPImage()
    {
        if (_data != nullptr)
        {
            //linearFree(_data);
            delete[] _data;
            _data = nullptr;
        }
    }

    void    BMPImage::SaveImage(const std::string &fileName) const
    {
        File file;
        const Color &Red = Color::Red;
        const Color &Black = Color::Black;
        int res = File::Open(file, fileName, 7);

        if (res != 0)
        {
            OSD::Notify(Utils::Format("BMP Error: couldn't open: %s, ec: %X", fileName.c_str(), res), Red, Black);
            return;
        }

        BitmapInformationHeader bih;

        bih.width            = _width;
        bih.height           = _height;
        bih.bit_count        = static_cast<unsigned short>(_bytesPerPixel << 3);
        bih.clr_important    = 0;
        bih.clr_used         = 0;
        bih.compression      = 0;
        bih.planes           = 1;
        bih.size             = bih.StructSize();
        bih.x_pels_per_meter = 0;
        bih.y_pels_per_meter = 0;
        bih.size_image       = (((bih.width * _bytesPerPixel) + 3) & 0x0000FFFC) * bih.height;

        BitmapFileHeader bfh;

        bfh.type             = 19778;
        bfh.size             = bfh.StructSize() + bih.StructSize() + bih.size_image;
        bfh.reserved1        = 0;
        bfh.reserved2        = 0;
        bfh.off_bits         = bih.StructSize() + bfh.StructSize();

        WriteBFH(file, bfh);
        WriteBIH(file, bih);

        unsigned int padding = (4 - ((3 * _width) % 4)) % 4;
        char paddingData[4] = { 0x00, 0x00, 0x00, 0x00 };

        for (unsigned int i = 0; i < _height; ++i)
        {
            const unsigned char* dataPtr = &_data[(_rowIncrement * (_height - i - 1))];

            file.Write(reinterpret_cast<const char*>(dataPtr),sizeof(unsigned char) * _bytesPerPixel * _width);
            file.Write(paddingData, padding);
        }

        file.Close();
    }

    void BMPImage::FillWithImg(const BMPImage &src)
    {
        if (!src._loaded)
          return;
        _loaded = true;

        std::memset(_data, 0, _dataSize);
        
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

    void     BMPImage::Draw(int x, int y)
    {
        ScreenImpl *scr = Renderer::_screen;

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

    void     BMPImage::Draw(const IntRect &area, float fade)
    {    
        ScreenImpl *scr = Renderer::_screen;

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

    int     BMPImage::CreateBitmap(void)
    {
      _rowIncrement = _width * _bytesPerPixel;
      if (_data != nullptr)
        delete[] _data;

      _dataSize = _height * _rowIncrement;
      _data = new u8[_dataSize];

      if (_data == nullptr)
      {
        _dataSize = 0;
        return (-1);
      }
        
        return (0);
    }

    void     BMPImage::LoadBitmap(void)
    {
        File  file;
        const Color &red = Color::Red;
        const Color &black = Color::Black;

        if (!File::Exists(_fileName))
            return;

        u32 res = File::Open(file, _fileName, File::READ);
        if (res != 0)
        {
            OSD::Notify(Utils::Format("Error opening: %s, code: %08X", _fileName.c_str(), res), red, black);
            return;        
        }

        _width  = 0;
        _height = 0;

        BitmapFileHeader        bfh;
        BitmapInformationHeader bih;

        bfh.Clear();
        bih.Clear();

        if (ReadBFH(file, bfh))
        {
            file.Close();

            OSD::Notify("BMP Error: Error while reading BFH", red, black);
            return;                
        }
        if (ReadBIH(file, bih))
        {
            file.Close();

            OSD::Notify("BMP Error: Error while reading BIH", red, black);
            return;                    
        }

        if (bfh.type != 19778)
        {
            bfh.Clear();
            bih.Clear();

            file.Close();

            OSD::Notify(Utils::Format("BMP Error: Invalid type value: %d", bfh.type), red, black);
            return;
        }

        if (bih.bit_count != 24)
        {
            bfh.Clear();
            bih.Clear();

            file.Close();

            OSD::Notify(Utils::Format("BMP Error: Invalid bit depth: %d", bih.bit_count), red, black);

            return;
        }

        /*if (bih.size != bih.StructSize())
        {
            bfh.Clear();
            bih.Clear();

            file.Close();

            sprintf(buffer, "BMP Error: Invalid BIH size: %X, expected; %X", bih.size, bih.StructSize());
            OSD::Notify(buffer, red, black);
            return;
        }*/

        _width  = bih.width;
        _height = bih.height;

       /* if (_width > 340 || _height > 200)
        {
            bfh.Clear();
            bih.Clear();

            file.Close();   

            OSD::Notify("BMP Error: file should not be higher than 340px * 200px", red, black);
            return;             
        }*/
        _dimensions = IntVector(_width, _height);
        _bytesPerPixel = bih.bit_count >> 3;
        file.Seek(bfh.off_bits, File::SET);

        unsigned int padding = (4 - ((3 * _width) % 4)) % 4;
        char paddingData[4] = {0,0,0,0};

        std::size_t bitmapFileSize = file.GetSize();

        if (bitmapFileSize >= 0x50000)
        {
            bfh.Clear();
            bih.Clear();

            file.Close();

            OSD::Notify("BMP Error: The file is too big.", red, black);

            return;                
        }

       /* std::size_t bitmapLogicalSize = (_height * _width * _bytesPerPixel) + (_height * padding) + bih.StructSize() + bfh.StructSize();

        if (bitmapFileSize != bitmapLogicalSize)
        {
            bfh.Clear();
            bih.Clear();

            file.Close();

            OSD::Notify("BMP Error: Mismatch between logical and physical sizes of BMP.", red, black);

            return;
        }*/

        if (CreateBitmap())
        {
            bfh.Clear();
            bih.Clear();

            file.Close();

            OSD::Notify("BMP Error: Error while allocating requiered space.", red, black);
            //_data.resize(0);
            return;                
        }

        int   rows = _height / 5;
        
        int   rowWidth = _rowIncrement + padding;
        int   totalwidth = rows * rowWidth;
        int   oddRows = rows * 5;

        unsigned char *buf = new u8[totalwidth];
        if (buf == nullptr)
            return;
        int i = 0;
        for (i = 0; i < oddRows; )
        {
            file.Read(buf, totalwidth);

            for (int j = 0; j < rows && i < oddRows; j++)
            {
                unsigned char *dataPtr = Row(_height - i++ - 1); // read in inverted row order
                unsigned char *bufPtr = buf + j * rowWidth;

                std::memcpy(dataPtr, bufPtr, _rowIncrement);
            }
        }
        int rest = _height % 5;
        file.Read(buf, totalwidth);

        for (int k = 0; k < rest; k++)
        {
            unsigned char *dataPtr = Row(_height - i++ - 1);
            unsigned char *bufPtr = buf + k * rowWidth;

            std::memcpy(dataPtr, bufPtr, _rowIncrement);
        }

        delete[] buf;
        file.Close();
        _loaded = true;
    }
}
