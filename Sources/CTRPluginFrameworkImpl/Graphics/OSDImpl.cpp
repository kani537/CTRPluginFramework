#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "font6x10Linux.h"

#include <vector>

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
    }

    OSDImpl     *OSDImpl::GetInstance(void)
    {
        return (_single);
    }

	void	OSDImpl::Start(void)
	{
        Screen::Top->Acquire(true);
        Screen::Top->Flush();
			
        Screen::Bottom->Acquire(true);
        Screen::Bottom->Flush();
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

    void    OSDImpl::operator()(void)
    {
        if (_list.empty())
            return;        

        int posX;
        int posY = std::min((u32)15, (u32)_list.size());
        posY = 230 - (15 * posY);

        OSDIter iter = _list.begin();
        OSDIter iterEnd = _list.end();

		// Restart timer for every notif not displayed yet
		for (; iter != iterEnd; ++iter)
		{
			
			if (!iter->drawn)
			    iter->time.Restart();
		}

		iter = _list.begin();

        std::vector<OSDIter> remove;

        for (; iter != iterEnd; ++iter)
        {
            posX = XEND - iter->width;
            _DrawMessage(iter, posX, posY);

            if (!iter->drawn)
                iter->drawn = true;

            if (iter->time.HasTimePassed(Seconds(5)))
            {
                remove.push_back(iter);
            }
        }

        if (!remove.empty())
            for (int i = remove.size() - 1; i >= 0; i--)
            {
                OSDIter rem = remove[i];
                _list.erase(rem);
            }
    }

	void    OSDImpl::_DrawMessage(OSDIter &iter, int posX, int &posY)
    {
        _DrawTop(iter->text, posX, posY, 10, iter->foreground, iter->background);
    }

    void    OSDImpl::_DrawTop(std::string &text, int posX, int &posY, int offset, Color &fg, Color &bg)
    {
        const char  *str = text.c_str();
        int         stride = Screen::Top->GetStride();
        int         off3D = offset < 0 ? offset - 1 : offset + 1;

        _topModified = true;

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
    }
}
