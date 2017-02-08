#ifndef CTRPLUGINFRAMEWORK_BMPIMAHE_HPP
#define CTRPLUGINFRAMEWORK_BMPIMAHE_HPP

#include "types.h"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFramework/File.hpp"

#include <string>
#include <vector>
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

        BMPImage(void) : 
        _fileName(""),
        _width(0),
        _height(0),
        _rowIncrement(0),
        _bytesPerPixel(3),
        _channelMode(BGR_Mode)
        {

        }

        BMPImage(const std::string &filename) : 
        _fileName(filename),
        _width(0),
        _height(0),
        _rowIncrement(0),
        _bytesPerPixel(0),
        _channelMode(BGR_Mode)
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
        _channelMode(BGR_Mode)
        {
            CreateBitmap();
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
    /*
       inline unsigned int width() const
       {
          return width_;
       }

       inline unsigned int height() const
       {
          return height_;
       }

       inline unsigned int bytes_per_pixel() const
       {
          return bytes_per_pixel_;
       }

       inline unsigned int pixel_count() const
       {
          return width_ *  height_;
       }

       inline void setwidth_height(const unsigned int width,
                                   const unsigned int height,
                                   const bool clear = false)
       {
          data_.clear();
          width_  = width;
          height_ = height;

          create_bitmap();

          if (clear)
          {
             std::fill(data_.begin(),data_.end(),0x00);
          }
       }*/

        void    SaveImage(const std::string &fileName) const
        {
            File file;
            Color Red = Color(255, 0, 0);
            Color Black = Color();
            char  buffer[200] = {0};
            int res = File::Open(file, fileName, 7);

            if (res != 0)
            {
                sprintf(buffer, "BMP Error: couldn't open: %d, ec: %X", fileName, res);
                OSD::Notify(buffer, Red, Black);
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

        inline void     HorizontalFlip(void)
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
        }

    /*
       inline void subsample(bitmap_image& dest)
       {
          /*
             Half sub-sample of original image.
          */
     /*     unsigned int w = 0;
          unsigned int h = 0;

          bool odd_width = false;
          bool odd_height = false;

          if (0 == (width_ % 2))
             w = width_ / 2;
          else
          {
             w = 1 + (width_ / 2);
             odd_width = true;
          }

          if (0 == (height_ % 2))
             h = height_ / 2;
          else
          {
             h = 1 + (height_ / 2);
             odd_height = true;
          }

          unsigned int horizontal_upper = (odd_width)  ? (w - 1) : w;
          unsigned int vertical_upper   = (odd_height) ? (h - 1) : h;

          dest.setwidth_height(w,h);
          dest.clear();

                unsigned char* s_itr[3];
          const unsigned char*  itr1[3];
          const unsigned char*  itr2[3];

          s_itr[0] = dest.data() + 0;
          s_itr[1] = dest.data() + 1;
          s_itr[2] = dest.data() + 2;

          itr1[0] = data() + 0;
          itr1[1] = data() + 1;
          itr1[2] = data() + 2;

          itr2[0] = data() + row_increment_ + 0;
          itr2[1] = data() + row_increment_ + 1;
          itr2[2] = data() + row_increment_ + 2;

          unsigned int total = 0;

          for (unsigned int j = 0; j < vertical_upper; ++j)
          {
             for (unsigned int i = 0; i < horizontal_upper; ++i)
             {
                for (unsigned int k = 0; k < bytes_per_pixel_; s_itr[k] += bytes_per_pixel_, ++k)
                {
                   total = 0;
                   total += *(itr1[k]);
                   total += *(itr1[k]);
                   total += *(itr2[k]);
                   total += *(itr2[k]);

                   itr1[k] += bytes_per_pixel_;
                   itr1[k] += bytes_per_pixel_;
                   itr2[k] += bytes_per_pixel_;
                   itr2[k] += bytes_per_pixel_;

                   *(s_itr[k]) = static_cast<unsigned char>(total >> 2);
                }
             }

             if (odd_width)
             {
                for (unsigned int k = 0; k < bytes_per_pixel_; s_itr[k] += bytes_per_pixel_, ++k)
                {
                   total = 0;
                   total += *(itr1[k]);
                   total += *(itr2[k]);

                   itr1[k] += bytes_per_pixel_;
                   itr2[k] += bytes_per_pixel_;

                   *(s_itr[k]) = static_cast<unsigned char>(total >> 1);
                }
             }

             for (unsigned int k = 0; k < bytes_per_pixel_; ++k)
             {
                itr1[k] += row_increment_;
             }

             if (j != (vertical_upper - 1))
             {
                for (unsigned int k = 0; k < bytes_per_pixel_; ++k)
                {
                   itr2[k] += row_increment_;
                }
             }
          }

          if (odd_height)
          {
             for (unsigned int i = 0; i < horizontal_upper; ++i)
             {
                for (unsigned int k = 0; k < bytes_per_pixel_; s_itr[k] += bytes_per_pixel_, ++k)
                {
                   total = 0;
                   total += *(itr1[k]);
                   total += *(itr2[k]);

                   itr1[k] += bytes_per_pixel_;
                   itr2[k] += bytes_per_pixel_;

                   *(s_itr[k]) = static_cast<unsigned char>(total >> 1);
                }
             }

             if (odd_width)
             {
                for (unsigned int k = 0; k < bytes_per_pixel_; ++k)
                {
                   (*(s_itr[k])) = *(itr1[k]);
                }
             }
          }
       }
    /*
       inline void upsample(bitmap_image& dest)
       {
          /*
             2x up-sample of original image.
          */
    /*
          dest.setwidth_height(2 * width_ ,2 * height_);
          dest.clear();

          const unsigned char* s_itr[3];
                unsigned char*  itr1[3];
                unsigned char*  itr2[3];

          s_itr[0] = data() + 0;
          s_itr[1] = data() + 1;
          s_itr[2] = data() + 2;

          itr1[0] = dest.data() + 0;
          itr1[1] = dest.data() + 1;
          itr1[2] = dest.data() + 2;

          itr2[0] = dest.data() + dest.row_increment_ + 0;
          itr2[1] = dest.data() + dest.row_increment_ + 1;
          itr2[2] = dest.data() + dest.row_increment_ + 2;

          for (unsigned int j = 0; j < height_; ++j)
          {
             for (unsigned int i = 0; i < width_; ++i)
             {
                for (unsigned int k = 0; k < bytes_per_pixel_; s_itr[k] += bytes_per_pixel_, ++k)
                {
                   *(itr1[k]) = *(s_itr[k]); itr1[k] += bytes_per_pixel_;
                   *(itr1[k]) = *(s_itr[k]); itr1[k] += bytes_per_pixel_;

                   *(itr2[k]) = *(s_itr[k]); itr2[k] += bytes_per_pixel_;
                   *(itr2[k]) = *(s_itr[k]); itr2[k] += bytes_per_pixel_;
                }
             }

             for (unsigned int k = 0; k < bytes_per_pixel_; ++k)
             {
                itr1[k] += dest.row_increment_;
                itr2[k] += dest.row_increment_;
             }
          }
       }*/

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
        inline void ReadFromFile(File &file, T &t)
        {
            file.Read(reinterpret_cast<char *>(&t), sizeof(T));
        }

        template <typename T>
        inline void WriteToFile(File &file, const T &t) const
        {
            file.Write(reinterpret_cast<const char *>(&t), sizeof(T));
        }

        inline void     ReadBFH(File &file, BitmapFileHeader &bfh)
        {
            ReadFromFile(file, bfh.type);
            ReadFromFile(file, bfh.size);
            ReadFromFile(file, bfh.reserved1);
            ReadFromFile(file, bfh.reserved2);
            ReadFromFile(file, bfh.off_bits);
        }

        inline void     WriteBFH(File &file, const BitmapFileHeader &bfh) const
        {
            WriteToFile(file, bfh.type);
            WriteToFile(file, bfh.size);
            WriteToFile(file, bfh.reserved1);
            WriteToFile(file, bfh.reserved2);
            WriteToFile(file, bfh.off_bits);
        }

        inline void     ReadBIH(File &file, BitmapInformationHeader &bih)
        {
            ReadFromFile(file, bih.size            );
            ReadFromFile(file, bih.width           );
            ReadFromFile(file, bih.height          );
            ReadFromFile(file, bih.planes          );
            ReadFromFile(file, bih.bit_count       );
            ReadFromFile(file, bih.compression     );
            ReadFromFile(file, bih.size_image      );
            ReadFromFile(file, bih.x_pels_per_meter);
            ReadFromFile(file, bih.y_pels_per_meter);
            ReadFromFile(file, bih.clr_used        );
            ReadFromFile(file, bih.clr_important   );
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

       void     CreateBitmap(void)
       {
          _rowIncrement = _width * _bytesPerPixel;
          _data.resize(_height * _rowIncrement);
       }

       void     LoadBitmap(void)
       {
            File  file;
            char  buffer[200] = {0};
            Color red = Color(255, 0, 0);
            Color black = Color();

            if (!File::Exists(_fileName))
            {
                sprintf(buffer, "%s not found !", _fileName.c_str());
                OSD::Notify(buffer, red, black);
                return;
            }

            u32 res = File::Open(file, _fileName);
            if (res != 0)
            {
                sprintf(buffer, "Error opening: %s, code: %08X", _fileName.c_str(), res);
                OSD::Notify(buffer, red, black);
                return;        
            }

            _width  = 0;
            _height = 0;

            BitmapFileHeader        bfh;
            BitmapInformationHeader bih;

            bfh.Clear();
            bih.Clear();

            ReadBFH(file, bfh);
            ReadBIH(file, bih);

            if (bfh.type != 19778)
            {
                bfh.Clear();
                bih.Clear();

                file.Close();

                sprintf(buffer, "BMP Error: Invalid type value: %d", bfh.type);
                OSD::Notify(buffer, red, black);
                return;
            }

            if (bih.bit_count != 24)
            {
                bfh.Clear();
                bih.Clear();

                file.Close();

                sprintf(buffer, "BMP Error: Invalid bit depth: %d", bih.bit_count);
                OSD::Notify(buffer, red, black);

                return;
            }

            if (bih.size != bih.StructSize())
            {
                bfh.Clear();
                bih.Clear();

                file.Close();

                sprintf(buffer, "BMP Error: Invalid BIH size: %X, expected; %X", bih.size, bih.StructSize());
                OSD::Notify(buffer, red, black);
                return;
            }

            _width  = bih.width;
            _height = bih.height;

            _bytesPerPixel = bih.bit_count >> 3;

            unsigned int padding = (4 - ((3 * _width) % 4)) % 4;
            char paddingData[4] = {0,0,0,0};

            std::size_t bitmapFileSize = file.GetSize();

            std::size_t bitmapLogicalSize = (_height * _width * _bytesPerPixel) + (_height * padding) + bih.StructSize() + bfh.StructSize();

            if (bitmapFileSize != bitmapLogicalSize)
            {
                bfh.Clear();
                bih.Clear();

                file.Close();

                OSD::Notify("BMP Error: Mismatch between logical and physical sizes of BMP.", red, black);

                return;
            }

            CreateBitmap();

            for (unsigned int i = 0; i < _height; ++i)
            {
                unsigned char *dataPtr = Row(_height - i - 1); // read in inverted row order

                file.Read(dataPtr, sizeof(char) * _bytesPerPixel * _width);
                file.Read(paddingData, padding);
            }
            file.Close();
       }

       template <typename T>
       inline T clamp(const T& v, const T& lower_range, const T& upper_range)
       {
          if (v < lower_range)
             return lower_range;
          else if (v >  upper_range)
             return upper_range;
          else
             return v;
       }

       std::string      _fileName;
       unsigned int     _width;
       unsigned int     _height;
       unsigned int     _rowIncrement;
       unsigned int     _bytesPerPixel;
       ChannelMode      _channelMode;
       std::vector<unsigned char>   _data;
    };
}

#endif