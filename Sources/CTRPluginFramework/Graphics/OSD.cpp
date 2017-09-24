#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "font6x10Linux.h"
#include "CTRPluginFramework/System/Sleep.hpp"
#include <algorithm>

namespace CTRPluginFramework
{
    u8      *Screen::GetFramebuffer(u32 posX, u32 posY, bool useRightFb) const
    {
        if (useRightFb && (!IsTop || !Is3DEnabled))
            return (nullptr);

        u8  *fb = useRightFb ? (u8 *)RightFramebuffer : (u8 *)LeftFramebuffer;
        u32 rowsize = Stride / BytesPerPixel;

        posX = std::min(posX, (IsTop ? (u32)400 : (u32)320));
        posY = std::min(posY, (u32)240);

        // Correct posY
        posY += rowsize - 240;
        u32 offset = (rowsize - 1 - posY + posX * rowsize) * BytesPerPixel;

        return (fb + offset);
    }

    u32     Screen::Draw(const std::string &str, u32 posX, u32 posY, const Color& foreground, const Color& background) const
    {
        Renderer::DrawString(str.c_str(), posX, (int &)posY, foreground, background);
        return (posY);
    }

    void    Screen::DrawRect(u32 posX, u32 posY, u32 width, u32 height, const Color& color, bool filled) const
    {
        Renderer::DrawRect(posX, posY, width, height, color, filled);
    }

    void    Screen::DrawPixel(u32 posX, u32 posY, const Color& color) const
    {
        Renderer::DrawPixel(posX, posY, color);
    }

    void    Screen::ReadPixel(u32 posX, u32 posY, Color& pixel, bool fromRightFb) const
    {
        u8 *fb = GetFramebuffer(posX, posY, fromRightFb);

        pixel = PrivColor::FromFramebuffer(fb);
    }

    int     OSD::Notify(std::string str, Color fg, Color bg)
    {
        OSDImpl::Lock();

        if (OSDImpl::Notifications.size() >= 50)
        {
            OSDImpl::Unlock();
            return (-1);
        }           

        OSDImpl::Notifications.push_back(new OSDImpl::OSDMessage(str, fg, bg));
        OSDImpl::Unlock();

        return (0);
    }

    void    OSD::Run(OSDCallback cb)
    {
        OSDImpl::Lock();
        OSDImpl::Callbacks.push_back(cb);
        OSDImpl::Unlock();
    }

    void    OSD::Stop(OSDCallback cb)
    {
        OSDImpl::Lock();
        OSDImpl::Callbacks.erase(std::remove(OSDImpl::Callbacks.begin(), OSDImpl::Callbacks.end(), cb), OSDImpl::Callbacks.end());
        OSDImpl::Unlock();
    }

    void    OSD::Lock()
    {
        OSDImpl::Lock();
    }

    bool    OSD::TryLock()
    {
        return (OSDImpl::TryLock());    
    }

    void OSD::Unlock()
    {
        OSDImpl::Unlock();
    }
}
