#include "CTRPluginFramework/System/Controller.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include <math.h>
#include "ctrulib/result.h"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuFreeCheats.hpp"


namespace CTRPluginFramework
{
    BMPImage    *Preferences::topBackgroundImage = nullptr;
    BMPImage    *Preferences::bottomBackgroundImage = nullptr;
    u32         Preferences::MenuHotkeys = static_cast<u32>(Key::Select);
    bool        Preferences::InjectBOnMenuClose = false;
    bool        Preferences::DrawTouchCursor = false;
    bool        Preferences::EcoMemoryMode = false;
    bool        Preferences::DisplayFilesLoading = false;
    bool        Preferences::SavingInProgress = false;
    bool        Preferences::AutoSaveCheats = false;
    bool        Preferences::AutoSaveFavorites = false;
    bool        Preferences::AutoLoadCheats = false;
    bool        Preferences::AutoLoadFavorites = false;
    bool        Preferences::_cheatsAlreadyLoaded = false;
    bool        Preferences::_favoritesAlreadyLoaded = false;

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

    int     Preferences::OpenConfigFile(File &settings, Header &header)
    {

        if (File::Open(settings, "CTRPFData.bin") == 0 && settings.GetSize() > 0)
        {
             // Check version
            u32     version = 0;
            int     res = 0;

            if (settings.Read(&version, 4)) return (-2);

            // Rewind file
            settings.Rewind();

            if (version == SETTINGS_VERSION)
                res = settings.Read(&header, sizeof(Header));
            if (version == SETTINGS_VERSION1)
                res = settings.Read(&header, sizeof(HeaderV1));

            return (res);
        }

        return (-1);
    }

    void    Preferences::LoadSettings(void)
    {
        File    settings;
        Header  header = { 0 };

        if (OpenConfigFile(settings, header) == 0)
        {
            MenuHotkeys = header.hotkeys;
            AutoLoadCheats = (header.flags & (u64)SettingsFlags::AutoLoadCheats != 0);
            AutoLoadFavorites = (header.flags & (u64)SettingsFlags::AutoLoadFavorites != 0);
            AutoSaveCheats = (header.flags & (u64)SettingsFlags::AutoSaveCheats != 0);
            AutoSaveFavorites = (header.flags & (u64)SettingsFlags::AutoSaveFavorites != 0);
        }

        // Check that hotkeys aren't 0
        if (MenuHotkeys == 0)
            MenuHotkeys = Key::Select;
    }

    void    Preferences::LoadFreeCheats(void)
    {
        File    settings;
        Header  header = { 0 };

        if (OpenConfigFile(settings, header) == 0)
        {
            if (header.freeCheatsCount)
                FreeCheats::LoadFromFile(header, settings);
        }
    }

    void    Preferences::LoadSavedEnabledCheats(void)
    {
        File    settings;
        Header  header = { 0 };

        if (_cheatsAlreadyLoaded)
        {
            MessageBox("Error\nCheats already loaded")();
            return;
        }

        if (OpenConfigFile(settings, header) == 0)
        {
            if (header.enabledCheatsCount != 0)
                PluginMenuImpl::LoadEnabledCheatsFromFile(header, settings);
            _cheatsAlreadyLoaded = true;
        }
    }

    void    Preferences::LoadSavedFavorites(void)
    {
        File    settings;
        Header  header = { 0 };

        if (_favoritesAlreadyLoaded)
        {
            MessageBox("Error\nFavorites already loaded")();
            return;
        }

        if (OpenConfigFile(settings, header) == 0)
        {
            if (header.favoritesCount != 0)
                PluginMenuImpl::LoadFavoritesFromFile(header, settings);
            _favoritesAlreadyLoaded = true;
        }
    }

    void    Preferences::LoadHotkeysFromFile(void)
    {
        File    settings;
        Header  header = { 0 };

        if (OpenConfigFile(settings, header) == 0)
        {
            if (header.hotkeysCount != 0)
                PluginMenuImpl::LoadHotkeysFromFile(header, settings);
        }
    }

    void    Preferences::WriteSettings(void)
    {
        SavingInProgress = true;

        File    settings;
        int     mode = File::READ | File::WRITE | File::CREATE | File::TRUNCATE | File::SYNC;
        Header  header = { 0 };

        header.version = SETTINGS_VERSION;
        
        if (AutoSaveCheats) header.flags |= (u64)SettingsFlags::AutoSaveCheats;
        if (AutoLoadCheats) header.flags |= (u64)SettingsFlags::AutoLoadCheats;
        if (AutoSaveFavorites) header.flags |= (u64)SettingsFlags::AutoSaveFavorites;
        if (AutoLoadFavorites) header.flags |= (u64)SettingsFlags::AutoLoadFavorites;

        header.hotkeys = MenuHotkeys;

        if (File::Open(settings, "CTRPFData.bin", mode) == 0)
        {
            if (settings.Write(&header, sizeof(Header)) != 0) goto error;

            FreeCheats::WriteToFile(header, settings);
            if (AutoSaveCheats) PluginMenuExecuteLoop::WriteEnabledCheatsToFile(header, settings);///PluginMenuImpl::WriteEnabledCheatsToFile(header, settings);
            if (AutoSaveFavorites) PluginMenuImpl::WriteFavoritesToFile(header, settings);
            PluginMenuImpl::WriteHotkeysToFile(header, settings);

            settings.Rewind();
            settings.Write(&header, sizeof(Header));
        }

    error:
        SavingInProgress = false;
        return;
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
