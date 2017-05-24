#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include <math.h>

namespace CTRPluginFramework
{
    BMPImage    *Preferences::topBackgroundImage = nullptr;
    BMPImage    *Preferences::bottomBackgroundImage = nullptr;
    bool        Preferences::InjectBOnMenuClose = false;
    bool        Preferences::DrawTouchCursor = false;
    bool        Preferences::EcoMemoryMode = false;

    BMPImage *RegionFromCenter(BMPImage *img, int maxX, int maxY)
    {
        BMPImage *temp = new BMPImage(maxX, maxY);

        u32 cx = img->Width() / 2;
        u32 cy = img->Height() / 2;

        img->RoiFromCenter(cx, cy, maxX, maxY, *temp);

        delete img;
        return (temp);
    }

    BMPImage *UpSampleUntilItsEnough(BMPImage *img, int maxX, int maxY)
    {
        BMPImage *temp = new BMPImage(img->Width() * 2, img->Height() * 2);

        img->UpSample(*temp);
        delete img;

        if (temp->Width() < maxX || temp->Height() < maxY)
            return (UpSampleUntilItsEnough(temp, maxX, maxY));
        return (temp);
    }

    /*BMPImage *UpSampleThenCrop(BMPImage *img, int maxX, int maxY)
    {
        BMPImage *temp = UpSampleUntilItsEnough(img, maxX, maxY);

        BMPImage *res =  new BMPImage(maxX, maxY);

        u32 cx = temp->Width() / 2;
        u32 cy = temp->Height() / 2;

        temp->RoiFromCenter(cx, cy, maxX, maxY, *res);

        delete temp;

        return (res);        
    }*/

    float GetRatio(int width, int height, int maxX, int maxY)
    {
        if (width >= height)
            return ((float)width / maxX);
        return ((float)height / maxY);
    }

    BMPImage *PostProcess(BMPImage *img, int maxX, int maxY)
    {
        int width = img->Width();
        int height = img->Height();

        float ratio = GetRatio(width, height, maxX, maxY);

        int newWidth = (int)(ceilf((float)width / ratio));
        int newHeight = (int)(ceilf((float)height / ratio));

        BMPImage *temp = new BMPImage(newWidth, newHeight);

        img->Resample(*temp, newWidth, newHeight);
        delete img;

        if (newWidth != maxX || newHeight != maxY)
        {
            BMPImage *res = new BMPImage(*temp, maxX, maxY);
            delete temp;
            return res;
        }        

        return (temp);
    }

    void    Preferences::Initialize(void)
    {
        // Framework globals
        InjectBOnMenuClose = false;
        DrawTouchCursor = false;

        // If EcoMemoryMode, don't load the backgrounds
        if (EcoMemoryMode)
        {
            topBackgroundImage = new BMPImage();
            bottomBackgroundImage = new BMPImage();            
        }
        else // Load the backgrounds
        {
            topBackgroundImage = new BMPImage("TopBackground.bmp");
            if (topBackgroundImage->IsLoaded())
                topBackgroundImage = PostProcess(topBackgroundImage, 340, 200);

            bottomBackgroundImage = new BMPImage("BottomBackground.bmp");
            if (bottomBackgroundImage->IsLoaded())
                bottomBackgroundImage = PostProcess(bottomBackgroundImage, 280, 200);
        }
        
    }
}