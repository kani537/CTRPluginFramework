#ifndef CTRPLUGINFRAMEWORK_BMPIMAGE_HPP
#define CTRPLUGINFRAMEWORK_BMPIMAGE_HPP

#include "types.h"

#include "CTRPluginFramework/System/File.hpp"
#include "CTRPluginFramework/System/Vector.hpp"
#include "CTRPluginFramework/System/Rect.hpp"

#include <cstring>

namespace CTRPluginFramework
{
    class BMPImage
    {
    public:

        enum ChannelMode
        {
            RGB_Mode = 0,
            BGR_Mode = 1
        };

        enum ColorPlane
        {
            BluePlane  = 0,
            GreenPlane = 1,
            RedPlane   = 2
        };

        ~BMPImage();

        BMPImage(void) :
        _data(nullptr),
        _dataSize(0),
        _fileName(""),
        _width(0),
        _height(0),
        _rowIncrement(0),
        _bytesPerPixel(3),
        _channelMode(BGR_Mode),
        _loaded(false)
        {

        }

        BMPImage(const std::string &filename) :
        _data(nullptr),
        _dataSize(0),
        _fileName(filename),
        _width(0),
        _height(0),
        _rowIncrement(0),
        _bytesPerPixel(0),
        _channelMode(BGR_Mode),
        _loaded(false)
        {
            LoadBitmap();
            RGBtoBGR();
        }

        BMPImage(const u32 width, const u32 height) :
        _data(nullptr),
        _dataSize(0),
        _fileName(""),
        _width(width),
        _height(height),
        _rowIncrement(0),
        _bytesPerPixel(3),
        _channelMode(BGR_Mode),
        _loaded(false)
        {
            CreateBitmap();
        }

        BMPImage(const BMPImage &src, const unsigned int width, const unsigned int height) :
        _data(nullptr),
        _dataSize(0),
        _fileName(src._fileName),
        _width(width),
        _height(height),
        _rowIncrement(0),
        _bytesPerPixel(3),
        _channelMode(BGR_Mode),
        _loaded(false)
        {
            CreateBitmap();
            FillWithImg(src);
        }

        bool IsLoaded(void)
        {
            return (_loaded);
        }

        void Unload(void);

        const IntVector &GetDimensions(void)
        {
            return (_dimensions);
        }

       inline void Clear(const unsigned char v = 0x00)
       {
          DataClear();
       }

       void     Draw(IntVector point)
       {
            Draw(point.x, point.y);
       }

       struct Pixel
       {
          u8 b;
          u8 g;
          u8 r;
       };

       void     Draw(int x, int y);
       void     Draw(const IntRect &area, float fade = 0.f);

       inline unsigned char *Row(unsigned int rowIndex) const
       {
          return const_cast<unsigned char*>(&_data[(rowIndex * _rowIncrement)]);
       }

       inline void GetPixel(const unsigned int x, const unsigned int y,
                             unsigned char& red,
                             unsigned char& green,
                             unsigned char& blue)
       {
          const unsigned int y_offset = y * _rowIncrement;
          const unsigned int x_offset = x * _bytesPerPixel;

          blue  = _data[y_offset + x_offset + 0];
          green = _data[y_offset + x_offset + 1];
          red   = _data[y_offset + x_offset + 2];
       }

       template <typename RGB>
       inline void GetPixel(const unsigned int x, const unsigned int y,
                             RGB& colour)
       {
          GetPixel(x, y, colour.red, colour.green, colour.blue);
       }

       inline void SetPixel(const unsigned int x, const unsigned int y,
                             const unsigned char red,
                             const unsigned char green,
                             const unsigned char blue)
       {
          const unsigned int y_offset = y * _rowIncrement;
          const unsigned int x_offset = x * _bytesPerPixel;

          _data[y_offset + x_offset + 0] = blue;
          _data[y_offset + x_offset + 1] = green;
          _data[y_offset + x_offset + 2] = red;
       }

       template <typename RGB>
       inline void SetPixel(const unsigned int x, const unsigned int y, const RGB& colour)
       {
          SetPixel(x, y, colour.red, colour.green, colour.blue);
       }

       inline unsigned int Width(void) const
       {
          return _width;
       }

       inline unsigned int Height(void) const
       {
          return _height;
       }

       inline unsigned int BytesPerPixel(void) const
       {
          return _bytesPerPixel;
       }

       inline unsigned int PixelCount(void) const
       {
          return _width *  _height;
       }

       inline void SetWidthHeight(const unsigned int width,
                                   const unsigned int height,
                                   const bool clear = false)
       {
          DataClear();

           if (width != _width || _height != height)
           {
               _width = width;
               _height = height;

               CreateBitmap();
           }

          if (clear)
          {
             //std::fill(_data.begin(), _data.end(), 0x00);
            std::memset(_data, 0, _dataSize);
          }
       }

       inline void DataClear(void)
       {
        std::memset(_data, 0, _dataSize);
       }

        void    SaveImage(const std::string &fileName) const;

        void FillWithImg(const BMPImage &src);

        inline const unsigned char   *data() const
        {
            return _data;
        }

        inline unsigned char     *data()
        {
            return const_cast<unsigned char*>(_data);
        }

        inline void BGRtoRGB(void)
        {
            if ((BGR_Mode == _channelMode) && (3 == _bytesPerPixel))
            {
                ReverseChannels();
                _channelMode = RGB_Mode;
            }
        }

        inline void RGBtoBGR(void)
        {
            if ((RGB_Mode == _channelMode) && (3 == _bytesPerPixel))
            {
                ReverseChannels();
                _channelMode = BGR_Mode;
            }
        }

        // Perform a basic 'pixel' enlarging resample.
        inline bool Resample(BMPImage &dest, int newWidth, int newHeight)
        {
            //if(_data == NULL) return false;
            //
            // Get a new buuffer to interpolate into
          if (!_loaded)
                return (false);

          dest.SetWidthHeight(newWidth, newHeight);
          if (!dest._loaded)
              return (false);
          dest.Clear();

            unsigned char *newData = dest.data();//new unsigned char [newWidth * newHeight * 3];

            double scaleWidth =  (double)newWidth / (double)_width;
            double scaleHeight = (double)newHeight / (double)_height;

            for(int cy = 0; cy < newHeight; cy++)
            {
                for(int cx = 0; cx < newWidth; cx++)
                {
                    int pixel = (cy * (newWidth *3)) + (cx*3);
                    int nearestMatch =  (((int)(cy / scaleHeight) * (_width *3)) + ((int)(cx / scaleWidth) *3) );

                    newData[pixel    ] =  _data[nearestMatch    ];
                    newData[pixel + 1] =  _data[nearestMatch + 1];
                    newData[pixel + 2] =  _data[nearestMatch + 2];
                }
            }

            //
            //delete[] _data;
            //_data = newData;
            //_width = newWidth;
            //_height = newHeight;

            return true;
        }


       inline void SubSample(BMPImage& dest)
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

          unsigned int horizontal_upper = (odd_width)  ? (w - 1) : w;
          unsigned int vertical_upper   = (odd_height) ? (h - 1) : h;

          dest.SetWidthHeight(w,h);
          dest.Clear();

                unsigned char* s_itr[3];
          const unsigned char*  itr1[3];
          const unsigned char*  itr2[3];

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

       inline void UpSample(BMPImage &dest)
       {
          /*
             2x up-sample of original image.
          */

          dest.SetWidthHeight(2 * _width , 2 * _height);
          dest.Clear();

          const unsigned char* s_itr[3];
                unsigned char*  itr1[3];
                unsigned char*  itr2[3];

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
                   *(itr1[k]) = *(s_itr[k]); itr1[k] += _bytesPerPixel;
                   *(itr1[k]) = *(s_itr[k]); itr1[k] += _bytesPerPixel;

                   *(itr2[k]) = *(s_itr[k]); itr2[k] += _bytesPerPixel;
                   *(itr2[k]) = *(s_itr[k]); itr2[k] += _bytesPerPixel;
                }
             }

             for (unsigned int k = 0; k < _bytesPerPixel; ++k)
             {
                itr1[k] += dest._rowIncrement;
                itr2[k] += dest._rowIncrement;
             }
          }
       }

       /*inline unsigned int offset(const color_plane color)
       {
          switch (channel_mode_)
          {
             case rgb_mode : {
                                switch (color)
                                {
                                   case red_plane   : return 0;
                                   case green_plane : return 1;
                                   case blue_plane  : return 2;
                                   default          : return std::numeric_limits<unsigned int>::max();
                                }
                             }

             case bgr_mode : {
                                switch (color)
                                {
                                   case red_plane   : return 2;
                                   case green_plane : return 1;
                                   case blue_plane  : return 0;
                                   default          : return std::numeric_limits<unsigned int>::max();
                                }
                             }

             default       : return std::numeric_limits<unsigned int>::max();
          }
       }*/

       inline void ReverseChannels(void)
       {
          if (3 != _bytesPerPixel)
             return;

          for (unsigned char* itr = data(); itr < end(); itr += _bytesPerPixel)
          {
             std::swap(*(itr + 0),*(itr + 2));
          }
       }

        inline bool     Region(const u32 &x, const u32 &y,
                               const u32 &width, const u32 &height,
                               BMPImage &destImage)
        {
            if ((x + width) > _width)
                return (false);
            if ((y + height) > _height)
                return (false);

            if ((destImage._width< _width) || (destImage._height < _height))
                destImage.SetWidthHeight(width, height);

            for (u32 r = 0; r < height; ++r)
            {
                u8 *itr1     = Row(r + y) + x * _bytesPerPixel;
                u8 *itr1_end = itr1 + (width * _bytesPerPixel);
                u8 *itr2  = destImage.Row(r);

                std::copy(itr1, itr1_end, itr2);
            }
            destImage._loaded = true;
            return (true);
        }

        inline bool RoiFromCenter(const u32 cx, const u32 cy,
                                  const u32 &width, const u32 &height,
                                  BMPImage &destImage)
        {
            return (Region(cx - (width / 2), cy - (height / 2), width, height, destImage));
        }

    private:

       inline const unsigned char* end() const
       {
          return _data + _dataSize;
       }

       inline unsigned char* end()
       {
          return const_cast<unsigned char*>(_data + _dataSize);
       }

        struct BitmapFileHeader
        {
            unsigned short type;
            unsigned int   size;
            unsigned short reserved1;
            unsigned short reserved2;
            unsigned int   off_bits;

            unsigned int StructSize(void) const
            {
                return sizeof(type     ) +
                sizeof(size     ) +
                sizeof(reserved1) +
                sizeof(reserved2) +
                sizeof(off_bits ) ;
            }

            void Clear(void)
            {
                std::memset(this, 0x00, sizeof(BitmapFileHeader));
            }
        };

        struct BitmapInformationHeader
        {
            unsigned int   size;
            unsigned int   width;
            unsigned int   height;
            unsigned short planes;
            unsigned short bit_count;
            unsigned int   compression;
            unsigned int   size_image;
            unsigned int   x_pels_per_meter;
            unsigned int   y_pels_per_meter;
            unsigned int   clr_used;
            unsigned int   clr_important;

            unsigned int StructSize(void) const
            {
                return (sizeof(size     ) +
                sizeof(width           ) +
                sizeof(height          ) +
                sizeof(planes          ) +
                sizeof(bit_count       ) +
                sizeof(compression     ) +
                sizeof(size_image      ) +
                sizeof(x_pels_per_meter) +
                sizeof(y_pels_per_meter) +
                sizeof(clr_used        ) +
                sizeof(clr_important   )) ;
            }

            void Clear(void)
            {
                std::memset(this, 0x00, sizeof(BitmapInformationHeader));
            }
        };
        /*
       inline bool big_endian() const
       {
          unsigned int v = 0x01;

          return (1 != reinterpret_cast<char*>(&v)[0]);
       }

       inline unsigned short flip(const unsigned short& v) const
       {
          return ((v >> 8) | (v << 8));
       }

       inline unsigned int flip(const unsigned int& v) const
       {
          return (
                   ((v & 0xFF000000) >> 0x18) |
                   ((v & 0x000000FF) << 0x18) |
                   ((v & 0x00FF0000) >> 0x08) |
                   ((v & 0x0000FF00) << 0x08)
                 );
       }*/

        template <typename T>
        inline int ReadFromFile(File &file, T &t)
        {
            return (file.Read(reinterpret_cast<char *>(&t), sizeof(T)));
        }

        template <typename T>
        inline int WriteToFile(File &file, const T &t) const
        {
            return (file.Write(reinterpret_cast<const char *>(&t), sizeof(T)));
        }

        inline int     ReadBFH(File &file, BitmapFileHeader &bfh)
        {
            if (ReadFromFile(file, bfh.type)) goto error;
            if (ReadFromFile(file, bfh.size)) goto error;
            if (ReadFromFile(file, bfh.reserved1)) goto error;
            if (ReadFromFile(file, bfh.reserved2)) goto error;
            if (ReadFromFile(file, bfh.off_bits)) goto error;

            return (0);
        error:
            return (-1);
        }

        inline void     WriteBFH(File &file, const BitmapFileHeader &bfh) const
        {
            WriteToFile(file, bfh.type);
            WriteToFile(file, bfh.size);
            WriteToFile(file, bfh.reserved1);
            WriteToFile(file, bfh.reserved2);
            WriteToFile(file, bfh.off_bits);
        }

        inline int     ReadBIH(File &file, BitmapInformationHeader &bih)
        {
            if (ReadFromFile(file, bih.size            )) goto error;
            if (ReadFromFile(file, bih.width           )) goto error;
            if (ReadFromFile(file, bih.height          )) goto error;
            if (ReadFromFile(file, bih.planes          )) goto error;
            if (ReadFromFile(file, bih.bit_count       )) goto error;
            if (ReadFromFile(file, bih.compression     )) goto error;
            if (ReadFromFile(file, bih.size_image      )) goto error;
            if (ReadFromFile(file, bih.x_pels_per_meter)) goto error;
            if (ReadFromFile(file, bih.y_pels_per_meter)) goto error;
            if (ReadFromFile(file, bih.clr_used        )) goto error;
            if (ReadFromFile(file, bih.clr_important   )) goto error;

            return (0);
        error:
            return (-1);
        }

        inline void     WriteBIH(File &file, const BitmapInformationHeader &bih) const
        {
            WriteToFile(file, bih.size            );
            WriteToFile(file, bih.width           );
            WriteToFile(file, bih.height          );
            WriteToFile(file, bih.planes          );
            WriteToFile(file, bih.bit_count       );
            WriteToFile(file, bih.compression     );
            WriteToFile(file, bih.size_image      );
            WriteToFile(file, bih.x_pels_per_meter);
            WriteToFile(file, bih.y_pels_per_meter);
            WriteToFile(file, bih.clr_used        );
            WriteToFile(file, bih.clr_important   );
        }

       int      CreateBitmap(void);
       void     LoadBitmap(void);


       std::string      _fileName;
       unsigned int     _width;
       unsigned int     _height;
       unsigned int     _rowIncrement;
       unsigned int     _bytesPerPixel;
       ChannelMode      _channelMode;
       //std::vector<unsigned char>   _data;
       unsigned char    *_data;
       unsigned int     _dataSize;
       IntVector        _dimensions;
       bool             _loaded;
    };
}

#endif