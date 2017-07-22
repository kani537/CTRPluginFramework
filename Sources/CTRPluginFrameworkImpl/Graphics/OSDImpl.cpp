#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "font6x10Linux.h"

#include <vector>
#include "NTR.hpp"
#include "NTRImpl.hpp"
#include "CTRPluginFramework/System/Sleep.hpp"

namespace CTRPluginFramework
{
    OSDImpl     *OSDImpl::_single = nullptr;

	OSDImpl::OSDImpl(void) :
	_topModified(false),
	_bottomModified(false)
    {        
    }

    void    OSDImpl::_Initialize(void)
    {
        if (_single != nullptr)
            return;
        _single = new OSDImpl();
        RecursiveLock_Init(&_single->_lock);
    }

    OSDImpl     *OSDImpl::GetInstance(void)
    {
        return (_single);
    }

	void	OSDImpl::Start(void)
	{
       // Screen::Top->Acquire(true);
       // Screen::Top->Flush();
			
       // Screen::Bottom->Acquire(true);
      //  Screen::Bottom->Flush();
	}

	void	OSDImpl::Finalize(void)
	{
		if (_topModified)
		{
			Screen::Top->Invalidate();
			_topModified = false;
		}

		if (_bottomModified)
		{
			Screen::Bottom->Invalidate();
			_bottomModified = false;
		}
	}


#define XEND    390

    bool    OSDImpl::operator()(bool drawOnly)
    {
        Lock();

        if (_messages.empty())
        {
            Unlock();
            return (false);
        }

        if (!NTRImpl::IsOSDAvailable)
            Screen::Top->Acquire(true);
        else
        {
            PrivColor::SetFormat((GSPGPU_FramebufferFormats)NTRImpl::OSDParameters.format);
        }

        int posX;
        int posY = std::min((u32)15, (u32)_messages.size());
        posY = 230 - (15 * posY);

		// Restart timer for every notif not displayed yet
      /*  for (int i = 0; i < _messages.size(); i++)
        {
            OSDMessage *message = _messages[i];
            if (!message->drawn)
                message->time.Restart();
        } 

        for (OSDMessage *message : _messages)
        {
            posX = XEND - message.width;
            _DrawMessage(*message, posX, posY);

            message.drawn = true;
        } */

        if (drawOnly)
        {
            Unlock();
            return (true);
        }

        if (!NTR::IsOSDAvailable())
            Screen::Top->Invalidate();
        

        RecursiveLock_Unlock(&_lock);

        return (true);
    }

    void    OSDImpl::Update(void)
    {
        if (TryLock())
            return;

        while (_messages.size() && _messages.front()->drawn)
        {
            OSDMessage *message = _messages.front();

            if (message->time.HasTimePassed(Seconds(5.f)))
            {
                delete message;
                _messages.pop_front();
            }
            else
                break;
        }

        Unlock();
    }

    bool    OSDImpl::Draw(void)
    {
        Lock();

        if (_messages.empty())
        {
            Unlock();
            return (false);
        }

        if (!NTRImpl::IsOSDAvailable)
            Screen::Top->Acquire(true);
        else
        {
            PrivColor::SetFormat((GSPGPU_FramebufferFormats)NTRImpl::OSDParameters.format);
        }

        int posX;
        int posY = std::min((u32)15, (u32)_messages.size());
        posY = 230 - (15 * posY);
        int count = 0;

        for (OSDMessage *message : _messages)
        {
            posX = XEND - message->width;
            _DrawMessage(*message, posX, posY);

            if (!message->drawn)
                message->time.Restart();

            message->drawn = true;

            count++;
            if (count >= 15)
                break;
        }

        if (!NTR::IsOSDAvailable())
            Screen::Top->Invalidate();

        Unlock();
        return (true);
    }

    void    OSDImpl::Lock(void)
    {
        RecursiveLock_Lock(&_lock);
    }

    bool OSDImpl::TryLock(void)
    {
        return (RecursiveLock_TryLock(&_lock));
    }

    void    OSDImpl::Unlock(void)
    {
        RecursiveLock_Unlock(&_lock);
    }

    void    OSDImpl::_DrawMessage(OSDMessage &message, int posX, int &posY)
    {
        _DrawTop(message.text, posX, posY, 10, message.foreground, message.background);
    }

    void    OSDImpl::_DrawTop(std::string &text, int posX, int &posY, int offset, Color &fg, Color &bg)
    {
       // Screen::Top->Acquire(true);

        const char  *str = text.c_str();
        int         stride = NTRImpl::IsOSDAvailable ? NTRImpl::OSDParameters.stride : Screen::Top->GetStride();
        int         off3D = offset < 0 ? offset - 1 : offset + 1;

       // _topModified = true;

        if (NTRImpl::IsOSDAvailable)
        {
            if (NTRImpl::OSDParameters.is3DEnabled)
            {
                {
                    for (int x = -2; x < 0; x++)
                    {
                        for (int i = 0; i < 10; i++)
                        {
                            u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX + x, posY + i);
                            u8  *framebuf1 = (u8 *)NTR::GetRightFramebuffer(posX + x - offset, posY + i);
                            PrivColor::ToFramebuffer(framebuf0, bg);
                            PrivColor::ToFramebuffer(framebuf1, bg);
                        }
                    }

                }
                while (*str)
                {
                    char c = *str;
                    int index = c * 10;

                    for (int yy = 0; yy < 10; yy++)
                    {
                        u8 charPos = font[index + yy];

                        int x = 0;
                        u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);
                        u8  *framebuf1 = (u8 *)NTR::GetRightFramebuffer(posX - offset, posY + yy);
                        for (int xx = 6; xx >= 0; xx--, x++)
                        {
                            if ((charPos >> xx) & 1)
                            {
                                PrivColor::ToFramebuffer(framebuf0, fg);
                                PrivColor::ToFramebuffer(framebuf1, fg);
                            }
                            else
                            {
                                PrivColor::ToFramebuffer(framebuf0, bg);
                                PrivColor::ToFramebuffer(framebuf1, bg);
                            }
                            framebuf0 += stride;
                            framebuf1 += stride;
                        }
                    }
                    str++;
                    posX += 6;
                }            
            }
            // No 3D
            else
            {
                {
                    for (int x = -2; x < 0; x++)
                    {
                        for (int i = 0; i < 10; i++)
                        {
                            u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX + x, posY + i);
                            PrivColor::ToFramebuffer(framebuf0, bg);
                        }
                    }

                }
                while (*str)
                {
                    char c = *str;
                    int index = c * 10;

                    for (int yy = 0; yy < 10; yy++)
                    {
                        u8 charPos = font[index + yy];

                        int x = 0;
                        u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);
                        for (int xx = 6; xx >= 0; xx--, x++)
                        {
                            if ((charPos >> xx) & 1)
                            {
                                PrivColor::ToFramebuffer(framebuf0, fg);
                            }
                            else
                            {
                                PrivColor::ToFramebuffer(framebuf0, bg);
                            }
                            framebuf0 += stride;
                        }
                    }
                    str++;
                    posX += 6;
                }
            }
            
            posY += 15;
            return;
        }

        // If 3D
        if (*(float *)(0x1FF81080) > 0.f)
        {
            {
                for (int x = -2; x < 0; x++)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i);
                        u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i, true);
                        u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - off3D, posY + i);
                        u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - off3D, posY + i, true);
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);
                        PrivColor::ToFramebuffer(framebuf2, bg);
                        PrivColor::ToFramebuffer(framebuf3, bg);
                    }
                }
            }
            while (*str)
            {
                char c = *str;

                for (int yy = 0; yy < 10; yy++)
                {
                    u8 charPos = font[c * 10 + yy];
                    u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX, posY + yy);
                    u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX, posY + yy, true);
                    u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - offset, posY + yy);
                    u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - offset, posY + yy, true);

                    int x = 0;
                    for (int xx = 6; xx >= 0; xx--, x++)
                    {
                        if ((charPos >> xx) & 1)
                        {
                            PrivColor::ToFramebuffer(framebuf0, fg);
                            PrivColor::ToFramebuffer(framebuf1, fg);
                            PrivColor::ToFramebuffer(framebuf2, fg);
                            PrivColor::ToFramebuffer(framebuf3, fg);
                        }
                        else
                        {
                            PrivColor::ToFramebuffer(framebuf0, bg);
                            PrivColor::ToFramebuffer(framebuf1, bg);
                            PrivColor::ToFramebuffer(framebuf2, bg);
                            PrivColor::ToFramebuffer(framebuf3, bg);
                        }
                        framebuf0 += stride;
                        framebuf1 += stride;
                        framebuf2 += stride;
                        framebuf3 += stride;
                    }
                }
                str++;
                posX += 6;
            }
        }
        else
        {
            {
                for (int x = -2; x < 0; x++)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i);
                        u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i, true);
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);
                    }
                }

            }
            while (*str)
            {
                char c = *str;
                int index = c * 10;

                for (int yy = 0; yy < 10; yy++)
                {
                    u8 charPos = font[index + yy];

                    int x = 0;
                    u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX, posY + yy);
                    u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX, posY + yy, true);
                    for (int xx = 6; xx >= 0; xx--, x++)
                    {
                        if ((charPos >> xx) & 1)
                        {
                            PrivColor::ToFramebuffer(framebuf0, fg);
                            PrivColor::ToFramebuffer(framebuf1, fg);
                        }
                        else
                        {
                            PrivColor::ToFramebuffer(framebuf0, bg);
                            PrivColor::ToFramebuffer(framebuf1, bg);
                        }
                        framebuf0 += stride;
                        framebuf1 += stride;
                    }
                }
                str++;
                posX += 6;
            }
        }
        posY += 15;
    }

    void    OSDImpl::_DrawBottom(std::string &text, int posX, int &posY, Color &fg, Color &bg)
    {
        Screen::Bottom->Acquire(true);

        const char *str = text.c_str();
        int     stride = Screen::Bottom->GetStride();

        _bottomModified = true;

        {
            for (int x = -2; x < 0; x++)
            {
                for (int i = 0; i < 10; i++)
                {
                    u8  *framebuf0 = Screen::Bottom->GetLeftFramebuffer(posX + x, posY + i);
                    u8  *framebuf1 = Screen::Bottom->GetLeftFramebuffer(posX + x, posY + i, true);
                    PrivColor::ToFramebuffer(framebuf0, bg);
                    PrivColor::ToFramebuffer(framebuf1, bg);
                }
            }

        }
        while (*str)
        {
            char c = *str;
            int index = c * 10;

            for (int yy = 0; yy < 10; yy++)
            {
                u8 charPos = font[index + yy];

                int x = 0;
                u8  *framebuf0 = Screen::Bottom->GetLeftFramebuffer(posX, posY + yy);
                u8  *framebuf1 = Screen::Bottom->GetLeftFramebuffer(posX, posY + yy, true);
                for (int xx = 6; xx >= 0; xx--, x++)
                {
                    if ((charPos >> xx) & 1)
                    {
                        PrivColor::ToFramebuffer(framebuf0, fg);
                        PrivColor::ToFramebuffer(framebuf1, fg);
                    }
                    else
                    {
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);
                    }
                    framebuf0 += stride;
                    framebuf1 += stride;
                }
            }
            str++;
            posX += 6;
        }
        posY += 15;

        Screen::Bottom->Invalidate();
    }
}
