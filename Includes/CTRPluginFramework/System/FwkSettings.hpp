#ifndef CTRPLUGINFRAMEWORK_SYSTEM_FWKSETTINGS_HPP
#define CTRPLUGINFRAMEWORK_SYSTEM_FWKSETTINGS_HPP

#include "types.h"
#include "CTRPluginFramework/System/Time.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"

namespace CTRPluginFramework
{
    struct FwkSettings
    {
        // Plugin init options
        u32     HeapSize;       ///< Size to be allocated for the heap (can be reduced on failure) | Default: 0x100000
        bool    EcoMemoryMode;  ///< Enable EncoMemoryMode: Heap will be reduced, backgrounds won't be loaded
        bool    StartARHandler; ///< Enable the Action Replay's handler, if this is set to off, even if the menu allows to create codes, they won't be executed
        bool    AllowSearchEngine; ///< If false then the search engine won't be available | Default: true
        Time    WaitTimeToBoot; ///< Time to wait for plugin to really starts (from when the game will starts) | Default: 5 seconds

        // UI customizations
        Color   MainTextColor;  ///< The color of all texts within the plugin | Default: Blank
        Color   WindowTitleColor; ///< The color of all window's titles | Default: Blank
        Color   MenuSelectedItemColor; ///< The color of the text for the selected item | Default: Blank
        Color   MenuUnselectedItemColor; ///< The color of the text for the items not selected | Default: Silver
        Color   BackgroundMainColor; ///< The color of the background | Default: Black
        Color   BackgroundSecondaryColor; ///< The color of the background 2 | Default: black/grey (RGB: 15,  15, 15)
        Color   BackgroundBorderColor; ///< The color of the border around the window | Default: Blank
        float   CursorFadeValue; ///< The value to be used to draw the cursor (Shade: [-1.0f - 0f], Tint: [0.f - 1.f]) | Default: 0.2f
    };
}

#endif
