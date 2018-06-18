#include "types.h"
#include "ctrulib/services/hid.h"
#include "ctrulib/svc.h"
#include "CTRPluginFramework/System/Controller.hpp"

namespace CTRPluginFramework
{
    u32     Controller::_keysDown = 0;
    u32     Controller::_keysHeld = 0;
    u32     Controller::_keysReleased = 0;

    extern "C" vu32* hidSharedMem;

    u32     Controller::GetKeysDown(void)
    {
        return (_keysDown | _keysHeld);
    }

    u32     Controller::GetKeysReleased(void)
    {
        return _keysReleased;
    }

    bool    Controller::IsKeyDown(Key key)
    {
        return (_keysHeld & (u32)key) != 0;
    }

    bool    Controller::IsKeyPressed(Key key)
    {
        return (_keysDown & (u32)key) != 0;
    }

    bool    Controller::IsKeyReleased(Key key)
    {
        return (_keysReleased & (u32)key) != 0;
    }

    bool    Controller::IsKeysDown(u32 keys)
    {
        return ((_keysHeld & keys) == keys);
    }

    bool    Controller::IsKeysPressed(u32 keys)
    {
        if ((_keysDown & keys) && ((_keysHeld | _keysDown) & keys) == keys)
            return (true);
        return (false);
    }

    bool    Controller::IsKeysReleased(u32 keys)
    {
        if ((_keysReleased & keys) == keys)
            return (true);
        return (false);
    }

    // TODO: a better injection, touch input injection being not good enough
    void    Controller::InjectTouch(u16 posX, u16 posY)
    {
        touchPosition tpos = {.px = posX, .py = posY};

        for (int i = 0; i < 8; i++)
        {
            int j = 42 + 8 + i * 2;
            *(touchPosition *)(&hidSharedMem[j]) = tpos;
            hidSharedMem[j + 1] = 1;
        }

        u64 tick = svcGetSystemTick();
        hidSharedMem[42] = 0;
        *((u64*)&hidSharedMem[42]) = tick;
    }

    void    Controller::InjectKey(u32 key)
    {
        for (int i = 0; i < 8; i++)
        {
            int j = 10 + i * 4;
            *(u32 *)(&hidSharedMem[j]) |= key;
        }
    }

    void    Controller::Update(void)
    {
        hidScanInput();
        _keysDown = hidKeysDown();
        _keysHeld = hidKeysHeld();
        _keysReleased = hidKeysUp();
    }
}
