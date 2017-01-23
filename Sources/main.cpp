#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{
    #define READU32(x) *(u32 *)(x)
    #define READU8(x) *(u8 *)(x)
    #define WRITEU8(a, v) *(u8 *)(a) = (u8)v
    #define WRITEU16(a, v) *(u16 *)(a) = (u16)v
    #define WRITEU32(a, v) *(u32 *)(a) = (u32)v 

    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {
        u64 tid = Process::GetTitleID();

        // Pokemon Moon / Sun
        // Patch game to prevent deconnection from Stream / debugger
        if (tid == 0x0004000000175E00 
            || tid == 0x0004000000164800)
        {
            u32     patch  = 0xE3A01000;
            Process::Patch(0x003DFFD0, (u8 *)&patch, 4);
        }
    }

    static u32  *basePointer = (u32 *)0x005A2E3C;

    void    MoonJump(MenuEntry *entry)
    {
        u32 pointer = *basePointer;

        if (!pointer)
            return;

        pointer += 0x77; 
        if (Controller::IsKeyDown(Key::A))
            WRITEU16(pointer, 0xCB40);
        return;
    }

    void    MoveFast(MenuEntry *entry)
    {
        u32 pointer = *basePointer;
        u32 jumpPointer;

        static u32 jump = 0;
        
        if (!pointer)
            return;

        jumpPointer = pointer + 0x77;
        pointer += 0x222C;
        if (Controller::IsKeyDown(Key::CPad) && Controller::IsKeyDown(Key::ZL))
        {
            if (jump < 3)
            {
                WRITEU16(jumpPointer, 0xCB40);
                WRITEU16(jumpPointer, 0xCB40);
                jump++;
            }
            else
                WRITEU32(pointer, 0x41A00000);
        }
        else
            jump = 0;
        return;
    }

    void    RefillHeart(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::Touchpad))
            WRITEU16(0x058799C, 0x140);
    }

    void    RefillMagic(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::Touchpad))
            if (READU8(0x05879A8) == 1)
                WRITEU8(0x058799F, 0x60);
            else
                WRITEU8(0x058799F, 0x30);
    }

    int    main(void)
    {   

        Menu    menu("Zelda Ocarina Of Time 3D");

        MenuFolder *folder = new MenuFolder("Movement codes");

        folder->Append(new MenuEntry("MoonJump (\uE000)", MoonJump));
        folder->Append(new MenuEntry("Fast Move (\uE077 + \uE054)", MoveFast));
        menu.Append(folder);

        folder = new MenuFolder("Battle codes");

        folder->Append(new MenuEntry("Refill Heart (\uE058)", RefillHeart));
        folder->Append(new MenuEntry("Refill Magic (\uE058)", RefillMagic));
        menu.Append(folder);
        std::string str = "\uE000\uE001\uE002\uE003\uE004\uE005\uE006\uE007\uE008\uE009\uE00A\uE00B\uE00C\uE00D\uE00E\uE00F";
        str += "\uE010\uE011\uE012\uE013\uE014\uE015\uE016\uE017\uE018\uE019\uE01A\uE01B\uE01C\uE01D\uE01E\uE01F";
        str += "\uE030\uE031\uE032\uE033\uE034\uE035\uE036\uE037\uE038\uE039\uE03A\uE03B\uE03C\uE03D\uE03E\uE03F";
        str += "\uE040\uE041\uE042\uE043\uE044\uE045\uE046\uE047\uE048\uE049\uE04A\uE04B\uE04C\uE04D\uE04E\uE04F";
        str += "\uE050\uE051\uE052\uE053\uE054\uE055\uE056\uE057\uE058\uE059\uE05A\uE05B\uE05C\uE05D\uE05E\uE05F";
        str += "\uE060\uE061\uE062\uE063\uE064\uE065\uE066\uE067\uE068\uE069\uE06A\uE06B\uE06C\uE06D\uE06E\uE06F";
        str += "\uE070\uE071\uE072\uE073\uE074\uE075\uE076\uE077\uE078\uE079\uE07A\uE07B\uE07C\uE07D\uE07E\uE07F";
        menu.Append(new MenuEntry(str));

        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

