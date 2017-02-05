#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFramework/Screen.hpp"
#include "font6x10Linux.h"

#include <vector>

namespace CTRPluginFramework
{
    OSD     *OSD::_single = nullptr;

    OSD::OSD(void)
    {        
    }

    void    OSD::_Initialize(void)
    {
        if (_single != nullptr)
            return;
        _single = new OSD();
    }

    OSD     *OSD::GetInstance(void)
    {
        return (_single);
    }

    int     OSD::Notify(std::string str, Color &fg, Color &bg)
    {
        if (_single == nullptr)
            return (-1);

        _single->_list.push_back(OSDMessage(str, fg, bg));
        return (0);
    }

    #define XEND    390

    void    OSD::operator()(void)
    {
        if (_list.empty())
            return;

        Screen::Top->Acquire(true);

        int posX;
        int posY = std::min((u32)15, (u32)_list.size());
        posY = 230 - (15 * posY);

        OSDIter iter = _list.begin();
        OSDIter iterEnd = _list.end();

        if (_list.size() > 15)
        {
            iterEnd = iter;
            std::advance(iterEnd, 15);
            OSDIter iterTemp = iterEnd;
            for (; iterTemp != _list.end(); iterTemp++)
                iterTemp->time.Restart();
        }
        else
            iterEnd = _list.end();

        std::vector<OSDIter> remove;

        for (; iter != iterEnd; iter++)
        {
            posX = XEND - iter->width;
            _DrawMessage(iter, posX, posY);

            if (iter->time.HasTimePassed(Seconds(5)))
            {
                remove.push_back(iter);
            }
        }

        Screen::Top->Flush();
        if (!remove.empty())
            for (int i = remove.size() - 1; i >= 0; i--)
            {
                OSDIter rem = remove[i];
                _list.erase(rem);
            }

    }

    void    OSD::_DrawMessage(OSDIter &iter, int posX, int &posY)
    {
        const char *str = iter->text.c_str();
        Color fg = iter->foreground;
        Color bg = iter->background;
        int stride = Screen::Top->GetStride();

        if (*(float *)(0x1FF81080) > 0.f)
        {
            {
                for (int i = 0; i < 10; i++)
                {
                    u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i);
                    u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i, true);
                    u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - 11, posY + i);
                    u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - 11, posY + i, true);
                    Color::ToFramebuffer(framebuf0, bg);
                    Color::ToFramebuffer(framebuf1, bg);
                    Color::ToFramebuffer(framebuf2, bg);
                    Color::ToFramebuffer(framebuf3, bg);                   
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
                    u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - 10, posY + yy);
                    u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - 10, posY + yy, true);
                    
                    int x = 0;
                    for (int xx = 6; xx >= 0; xx--, x++)
                    {
                        if ((charPos >> xx) & 1)
                        {
                            Color::ToFramebuffer(framebuf0, fg);
                            Color::ToFramebuffer(framebuf1, fg);
                            Color::ToFramebuffer(framebuf2, fg);
                            Color::ToFramebuffer(framebuf3, fg);
                            //_DrawPixel(posX + x, posY + yy, fg);
                        }
                        else
                        {
                            Color::ToFramebuffer(framebuf0, bg);
                            Color::ToFramebuffer(framebuf1, bg);
                            Color::ToFramebuffer(framebuf2, bg);
                            Color::ToFramebuffer(framebuf3, bg);
                            //_DrawPixel(posX + x, posY + yy, bg);
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
                for (int i = 0; i < 10; i++)
                {
                    u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i);
                    u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i, true);
                    Color::ToFramebuffer(framebuf0, bg);
                    Color::ToFramebuffer(framebuf1, bg);                
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
                    
                    int x = 0;
                    for (int xx = 6; xx >= 0; xx--, x++)
                    {
                        if ((charPos >> xx) & 1)
                        {
                            Color::ToFramebuffer(framebuf0, fg);
                            Color::ToFramebuffer(framebuf1, fg);
                            //_DrawPixel(posX + x, posY + yy, fg);
                        }
                        else
                        {
                            Color::ToFramebuffer(framebuf0, bg);
                            Color::ToFramebuffer(framebuf1, bg);
                            //_DrawPixel(posX + x, posY + yy, bg);
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

    void    OSD::WriteLine(int screen, std::string line, int posX, int posY, Color fg, Color bg)
    {
        if (line.size() == 0 || posX < 0 || posY < 0 || posY > 240)
            return;

        const char *str = line.c_str();

        //Bottom screen
        if (screen == 0)
        {
            if (posX > 320)
                return;
            int stride = Screen::Bottom->GetStride();

            {
                for (int i = 0; i < 10; i++)
                {
                    u8  *framebuf0 = Screen::Bottom->GetLeftFramebuffer(posX - 1, posY + i);
                    u8  *framebuf1 = Screen::Bottom->GetLeftFramebuffer(posX - 1, posY + i, true);
                    Color::ToFramebuffer(framebuf0, bg);
                    Color::ToFramebuffer(framebuf1, bg);                
                }
            }
            while (*str)
            {
                char c = *str;

                for (int yy = 0; yy < 10; yy++)
                {
                    u8 charPos = font[c * 10 + yy];
                    u8  *framebuf0 = Screen::Bottom->GetLeftFramebuffer(posX, posY + yy);
                    u8  *framebuf1 = Screen::Bottom->GetLeftFramebuffer(posX, posY + yy, true);
                    
                    int x = 0;
                    for (int xx = 6; xx >= 0; xx--, x++)
                    {
                        if ((charPos >> xx) & 1)
                        {
                            Color::ToFramebuffer(framebuf0, fg);
                            Color::ToFramebuffer(framebuf1, fg);
                            //_DrawPixel(posX + x, posY + yy, fg);
                        }
                        else
                        {
                            Color::ToFramebuffer(framebuf0, bg);
                            Color::ToFramebuffer(framebuf1, bg);
                            //_DrawPixel(posX + x, posY + yy, bg);
                        }
                        framebuf0 += stride;
                        framebuf1 += stride;
                    }
                }    
                str++;
                posX += 6;       
            }
        }

        // Top Screen
        else
        {
            int stride = Screen::Top->GetStride();
            if (*(float *)(0x1FF81080) > 0)
            {
                {
                    for (int i = 0; i < 10; i++)
                    {
                        u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i);
                        u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i, true);
                        u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - 11, posY + i);
                        u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - 11, posY + i, true);
                        Color::ToFramebuffer(framebuf0, bg);
                        Color::ToFramebuffer(framebuf1, bg);
                        Color::ToFramebuffer(framebuf2, bg);
                        Color::ToFramebuffer(framebuf3, bg);                   
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
                        u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - 10, posY + yy);
                        u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - 10, posY + yy, true);
                        
                        int x = 0;
                        for (int xx = 6; xx >= 0; xx--, x++)
                        {
                            if ((charPos >> xx) & 1)
                            {
                                Color::ToFramebuffer(framebuf0, fg);
                                Color::ToFramebuffer(framebuf1, fg);
                                Color::ToFramebuffer(framebuf2, fg);
                                Color::ToFramebuffer(framebuf3, fg);
                                //_DrawPixel(posX + x, posY + yy, fg);
                            }
                            else
                            {
                                Color::ToFramebuffer(framebuf0, bg);
                                Color::ToFramebuffer(framebuf1, bg);
                                Color::ToFramebuffer(framebuf2, bg);
                                Color::ToFramebuffer(framebuf3, bg);
                                //_DrawPixel(posX + x, posY + yy, bg);
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
                    for (int i = 0; i < 10; i++)
                    {
                        u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i);
                        u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX - 1, posY + i, true);
                        Color::ToFramebuffer(framebuf0, bg);
                        Color::ToFramebuffer(framebuf1, bg);                
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
                        
                        int x = 0;
                        for (int xx = 6; xx >= 0; xx--, x++)
                        {
                            if ((charPos >> xx) & 1)
                            {
                                Color::ToFramebuffer(framebuf0, fg);
                                Color::ToFramebuffer(framebuf1, fg);
                                //_DrawPixel(posX + x, posY + yy, fg);
                            }
                            else
                            {
                                Color::ToFramebuffer(framebuf0, bg);
                                Color::ToFramebuffer(framebuf1, bg);
                                //_DrawPixel(posX + x, posY + yy, bg);
                            }
                            framebuf0 += stride;
                            framebuf1 += stride;
                        }
                    }    
                    str++;
                    posX += 6;       
                } 
            }
        }
    }

}