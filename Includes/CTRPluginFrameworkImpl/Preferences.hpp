#ifndef CTRPPLUGINFRAMEWORK_PREFERENCES_HPP
#define CTRPPLUGINFRAMEWORK_PREFERENCES_HPP

#include "types.h"
#include "ctrulib/os.h"
#include "CTRPluginFrameworkImpl/Graphics/BMPImage.hpp"
#include <vector>

namespace CTRPluginFramework
{
    class Preferences
    {
        #define SETTINGS_VERSION1 SYSTEM_VERSION(1, 0, 0)
        #define SETTINGS_VERSION11 SYSTEM_VERSION(1, 1, 0)

        #define SETTINGS_VERSION SETTINGS_VERSION11
    public:
        enum class SettingsFlags
        {
            AutoSaveCheats = 1,
            AutoSaveFavorites = 1 << 1,
            AutoLoadCheats = 1 << 2,
            AutoLoadFavorites = 1 << 3
        };

        struct HeaderV1
        {
            u32     version;
            u64     flags;
            u32     hotkeys;
            u32     freeCheatsCount;
            u64     freeCheatsOffset;
            u32     enabledCheatsCount;
            u64     enabledCheatsOffset;
            u32     favoritesCount;
            u64     favoritesOffset;
        } PACKED;

        struct HeaderV11
        {
            u32     version;
            u64     flags;
            u32     hotkeys;
            u32     freeCheatsCount;
            u64     freeCheatsOffset;
            u32     enabledCheatsCount;
            u64     enabledCheatsOffset;
            u32     favoritesCount;
            u64     favoritesOffset;
            u32     hotkeysCount;
            u64     hotkeysOffset;
        } PACKED;

        using Header = HeaderV11;

        struct SavedCheats
        {
            u16     flags;
            u32     address;
            u64     value;
            char    name[50];
        } PACKED;

        struct EnabledCheats
        {
            u32     uid;
        };

        struct Favorites
        {
            u32     uid;
        };

        struct HotkeysInfos
        {
            u32     uid; ///< owner of the hotkeys
            u32     count;
            std::vector<u32>   hotkeys;
        };

        static BMPImage     *topBackgroundImage;
        static BMPImage     *bottomBackgroundImage;

        static u32          MenuHotkeys;
        static bool         InjectBOnMenuClose;
        static bool         DrawTouchCursor;
        static bool         EcoMemoryMode;
        static bool         DisplayFilesLoading;
        static bool         SavingInProgress;

        static bool         AutoSaveCheats;
        static bool         AutoSaveFavorites;
        static bool         AutoLoadCheats;
        static bool         AutoLoadFavorites;

        static int          OpenConfigFile(File &file, Header &header);
        static void         LoadSettings(void);
        static void         LoadFreeCheats(void);
        static void         LoadSavedEnabledCheats(void);
        static void         LoadSavedFavorites(void);
        static void         LoadHotkeysFromFile(void);
        static void         WriteSettings(void);

    private:
        static bool         _cheatsAlreadyLoaded;
        static bool         _favoritesAlreadyLoaded;

        friend void     ThreadInit(void *arg);
        
        static void     Initialize(void);
    };
}

#endif