#ifndef CTRPLUGINFRAMEWORKIMPL_COMBOBOX_HPP
#define CTRPLUGINFRAMEWORKIMPL_COMBOBOX_HPP

#include "CTRPluginFramework/System/Vector.hpp"
#include "CTRPluginFramework/System/Rect.hpp"
#include <string>
#include <vector>

namespace CTRPluginFramework
{
    class ComboBox
    {
    public:
        ComboBox(int posX, int posY, int width, int height);

        void    Add(std::string item);
        void    Clear(void);
        void    Draw(void);
        void    Update(bool isTouchDown, IntVector touchPos);
        bool    operator()(void);

        bool    IsEnabled;
        bool    IsVisible;
        int     SelectedItem;


    private:
        bool                        _execute;
        bool                        _isTouched;
        IntRect                     _rectPos;
        std::vector<std::string>    _items;

    };
}

#endif