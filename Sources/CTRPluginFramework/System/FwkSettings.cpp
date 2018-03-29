#include "CTRPluginFramework/System/FwkSettings.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    FwkSettings & FwkSettings::Get(void)
    {
        return Preferences::Settings;
    }

    void    FwkSettings::SetThemeDefault(void)
    {
        FwkSettings &settings = Get();

        // UI Colors
        settings.MenuSelectedItemColor = settings.BackgroundBorderColor = settings.WindowTitleColor = settings.MainTextColor = Color::Blank;
        settings.MenuUnselectedItemColor = Color::Silver;
        settings.BackgroundMainColor = Color::Black;
        settings.BackgroundSecondaryColor = Color::BlackGrey;
        settings.CursorFadeValue = 0.2f;

        // Keyboard colors
        {
            auto &kb = settings.Keyboard;

            kb.KeyBackground = kb.Background = Color::Black;
            kb.KeyBackgroundPressed = Color::Silver;
            kb.Input = kb.Cursor = kb.KeyTextPressed = kb.KeyText = Color::Blank;
        }

        // Custom keyboard colors
        {
            auto &kb = settings.CustomKeyboard;

            kb.BackgroundMain = Color::Black;
            kb.BackgroundSecondary = Color::BlackGrey;
            kb.BackgroundBorder = Color::Blank;
            kb.KeyBackground = Color(51, 51, 51);
            kb.KeyBackgroundPressed = Color::Gainsboro;
            kb.KeyText = Color::Blank;
            kb.KeyTextPressed = Color::Black;
            kb.ScrollBarBackground = Color::Silver;
            kb.ScrollBarThumb = Color::DimGrey;
        }
    }
}
