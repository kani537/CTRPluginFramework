#include "types.h"
#include "ctrulib/services/hid.h"

#include "CTRPluginFramework/Controller.hpp"

namespace CTRPluginFramework
{
    u32     Controller::_keysDown = 0;
    u32     Controller::_keysHeld = 0;
    u32     Controller::_keysReleased = 0;

    extern "C" vu32* hidSharedMem;

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
        return ((_keysHeld & keys) == keys);
    }

    bool    Controller::IsKeysPressed(u32 keys)
    {
        if ((_keysDown & keys) == keys)
            return (true);
        return (false);
    }

    bool    Controller::IsKeysReleased(u32 keys)
    {
        if ((_keysReleased & keys) == keys)
            return (true);
        return (false);
    }
    extern "C" u32 hidCheckSectionUpdateTime(vu32 *sharedmem_section, u32 id);

    void    Controller::InjectTouch(u16 posX, u16 posY)
    {
        //u32 id = hidSharedMem[42 + 4];
        touchPosition tpos = {.px = posX, .py = posY};

       // if (id > 7) id = 7;
        //if (hidCheckSectionUpdateTime(&hidSharedMem[42], id)==0)
        for (int i = 0; i < 8; i++)
        {
            int j = 42 + 8 + i *2;
            *(touchPosition *)(&hidSharedMem[j]) = tpos;
            hidSharedMem[j + 1] = 1;           
        }
    }

    void    Controller::InjectKey(u32 key)
    {
        for (int i = 0; i < 8; i++)
        {
            int j = 10 + i * 4;
            *(u32 *)(&hidSharedMem[j]) |= key;        
        }       
    }

    extern "C"     void    CtruInjectTouch(vu32 *hid);
    bool inject = false;
    void    CtruInjectTouch(vu32 *hid)
    {
            touchPosition tpos = {0};
            tpos.px = 10;
            tpos.py = 10; 
            *(touchPosition *)hid = tpos;
            *(hid + 1) = 1;      
    }

    void    Controller::Update(void)
    {
        hidScanInput();
        _keysDown = hidKeysDown();
        _keysHeld = hidKeysHeld();
        _keysReleased = hidKeysUp();
    }
}
