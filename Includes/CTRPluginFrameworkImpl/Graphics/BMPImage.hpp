#ifndef CTRPLUGINFRAMEWORK_BMPIMAHE_HPP
#define CTRPLUGINFRAMEWORK_BMPIMAHE_HPP

#include "types.h"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFramework/System/File.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Vector.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Rect.hpp"

#include <string>
#include <vector>
#include <cstring>
#include <cstdio>

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

        ~BMPImage(){}

        BMPImage(void) : 
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

        BMPImage(const unsigned int width, const unsigned int height) : 
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

        IntVector &GetDimensions(void)
        {
            return (_dimensions);
        }

        /*bitmap_image(const & image) : 
        _fileName_(image.fileName),
         width_(image.width_),
         height_(image.height_),
         row_increment_(0),
         bytes_per_pixel_(3),
         channel_mode_(bgr_mode)
        {
          create_bitmap();
          data_ = image.data_;
        }

       bitmap_image& operator=(const bitmap_image& image)
       {
          if (this != &image)
          {
             file_name_       = image.file_name_;
             bytes_per_pixel_ = image.bytes_per_pixel_;
             width_           = image.width_;
             height_          = image.height_;
             row_increment_   = 0;
             channel_mode_    = image.channel_mode_;
             create_bitmap();
             data_ = image.data_;
          }

          return *this;
       }*/

        /*
       inline bool operator!()
       {
          return (data_.size()   == 0) ||
                 (width_         == 0) ||
                 (height_        == 0) ||
                 (row_increment_ == 0);
       }*/

       inline void Clear(const unsigned char v = 0x00)
       {
          std::fill(_data.begin(), _data.end(), v);
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
       void     Draw(IntRect &area, float fade = 0.f);
/*
       inline unsigned char RedChannel(const unsigned int x, const unsigned int y) const
       {
          return _data[(y * _rowIncrement) + (x * _bytesPerPixel + 2)];
       }

       inline unsigned char GreenChannel(const unsigned int x, const unsigned int y) const
       {
          return _data[(y * _rowIncrement) + (x * _bytesPerPixel + 1)];
       }

       inline unsigned char BlueChannel(const unsigned int x, const unsigned int y) const
       {
          return _data[(y * _rowIncrement) + (x * _bytesPerPixel + 0)];
       }

       inline void RedChannel(const unsigned int x, const unsigned int y, const unsigned char value)
       {
            _data[(y * _rowIncrement) + (x * _bytesPerPixel + 2)] = value;
       }

       inline void GreenChannel(const unsigned int x, const unsigned int y, const unsigned char value)
       {
          _data[(y * _rowIncrement) + (x * _bytesPerPixel + 1)] = value;
       }

       inline void BlueChannel(const unsigned int x, const unsigned int y, const unsigned char value)
       {
          _data[(y * _rowIncrement) + (x * _bytesPerPixel + 0)] = value;
       }
*/
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

       /*inline bool copy_from(const bitmap_image& image)
       {
          if (
               (image.height_ != height_) ||
               (image.width_  != width_ )
             )
          {
             return false;
          }

          data_ = image.data_;

          return true;
       }*/

      /* inline bool copy_from(const bitmap_image& source_image,
                             const unsigned int& x_offset,
                             const unsigned int& y_offset)
       {
          if ((x_offset + source_image.width_ ) > width_ ) { return false; }
          if ((y_offset + source_image.height_) > height_) { return false; }

          for (unsigned int y = 0; y < source_image.height_; ++y)
          {
             unsigned char* itr1           = row(y + y_offset) + x_offset * bytes_per_pixel_;
             const unsigned char* itr2     = source_image.row(y);
             const unsigned char* itr2_end = itr2 + source_image.width_ * bytes_per_pixel_;

             std::copy(itr2,itr2_end,itr1);
          }

          return true;
       }*/
    /*
       void reflective_image(bitmap_image& image, const bool include_diagnols = false)
       {
          image.setwidth_height(3 * width_, 3 * height_,true);

          image.copy_from(*this, width_, height_);

          vertical_flip();

          image.copy_from(*this, width_, 0);
          image.copy_from(*this, width_, 2 * height_);

          vertical_flip();
          horizontal_flip();

          image.copy_from(*this, 0, height_);
          image.copy_from(*this, 2 * width_, height_);

          horizontal_flip();

          if (include_diagnols)
          {
             bitmap_image tile = *this;

             tile.vertical_flip();
             tile.horizontal_flip();

             image.copy_from(tile, 0 , 0);
             image.copy_from(tile, 2 * width_, 0);
             image.copy_from(tile, 2 * width_, 2 * height_);
             image.copy_from(tile, 0, 2 * height_);
          }
       }*/
    
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
          _data.clear();
          _width  = width;
          _height = height;

          CreateBitmap();

          if (clear)
          {
             std::fill(_data.begin(), _data.end(), 0x00);
          }
       }

        void    SaveImage(const std::string &fileName) const;

        void FillWithImg(const BMPImage &src);

     /*  
       inline void convert_to_grayscale()
       {
          double r_scaler = 0.299;
          double g_scaler = 0.587;
          double b_scaler = 0.114;

          if (rgb_mode == channel_mode_)
          {
             std::swap(r_scaler,b_scaler);
          }

          for (unsigned char* itr = data(); itr < end(); )
          {
             unsigned char gray_value = static_cast<unsigned char>(
                                                                    (r_scaler * (*(itr + 2))) +
                                                                    (g_scaler * (*(itr + 1))) +
                                                                    (b_scaler * (*(itr + 0)))
                                                                  );
             *(itr++) = gray_value;
             *(itr++) = gray_value;
             *(itr++) = gray_value;
          }
       }*/

        inline const unsigned char   *data() const
        {
            return _data.data();
        }

        inline unsigned char     *data()
        {
            return const_cast<unsigned char*>(_data.data());
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

    /*   inline void reverse()
       {
          unsigned char* itr1 = data();
          unsigned char* itr2 = end() - bytes_per_pixel_;

          while (itr1 < itr2)
          {
             for (std::size_t i = 0; i < bytes_per_pixel_; ++i)
             {
                unsigned char* citr1 = itr1 + i;
                unsigned char* citr2 = itr2 + i;

                std::swap(*citr1,*citr2);
             }

             itr1 += bytes_per_pixel_;
             itr2 -= bytes_per_pixel_;
          }
       }*/

       /* inline void     HorizontalFlip(void)
        {
            for (unsigned int y = 0; y < _height; ++y)
            {
                unsigned char* itr1 = Row(y);
                unsigned char* itr2 = itr1 + _rowIncrement - _bytesPerPixel;

                while (itr1 < itr2)
                {
                    for (unsigned int i = 0; i < _bytesPerPixel; ++i)
                    {
                        unsigned char* p1 = (itr1 + i);
                        unsigned char* p2 = (itr2 + i);

                        std::swap(*p1, *p2);
                    }

                    itr1 += _bytesPerPixel;
                    itr2 -= _bytesPerPixel;
                }
            }
        }

        inline void     VerticalFlip(void)
        {
            for (unsigned int y = 0; y < (_height / 2); ++y)
            {
                unsigned char* itr1 = Row(y);
                unsigned char* itr2 = Row(_height - y - 1);

                for (std::size_t x = 0; x < _rowIncrement; ++x)
                {
                    std::swap(*(itr1 + x),*(itr2 + x));
                }
            }
        }*/
        // Perform a basic 'pixel' enlarging resample.
        inline bool Resample(BMPImage &dest, int newWidth, int newHeight)
        {
            //if(_data == NULL) return false;
            //
            // Get a new buuffer to interpolate into
          if (!_loaded)
                return (false);
            dest._loaded = true;

          dest.SetWidthHeight(newWidth, newHeight);
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
          return _data.data() + _data.size();
       }

       inline unsigned char* end()
       {
          return const_cast<unsigned char*>(data() + _data.size());
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
       std::vector<unsigned char>   _data;
       IntVector        _dimensions;
       bool             _loaded;
    };
}

#endif