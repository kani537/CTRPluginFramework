#include "CTRPluginFramework.hpp"
#include "ctrulib/services/hid.h"

namespace CTRPluginFramework
{
    u32     Controller::_keysDown = 0;
    u32     Controller::_keysHeld = 0;
    u32     Controller::_keysReleased = 0;

    bool    Controller::IsKeyDown(Key key)
    {
        return (_keysHeld & (u32)key);
    }

    bool    Controller::IsKeyPressed(Key key)
    {
        return (_keysDown & (u32)key);
    }

    bool    Controller::IsKeyReleased(Key key)
    {
        return (_keysReleased & (u32)key);
    }

    bool    Controller::IsKeysDown(u32 keys)
    {
        return (((_keysDown | _keysHeld) & keys) == keys);
    }

    void    Controller::Update(void)
    {
        hidScanInput();
        _keysDown = hidKeysDown();
        _keysHeld = hidKeysHeld();
        _keysReleased = hidKeysUp();
    }
}