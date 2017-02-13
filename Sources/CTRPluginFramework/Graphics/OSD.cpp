#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "font6x10Linux.h"

namespace CTRPluginFramework
{

    int     OSD::Notify(std::string str, Color fg, Color bg)
    {
        OSDImpl *inst = OSDImpl::GetInstance();

        if (inst == nullptr || inst->_list.size() >= 50)
            return (-1);

        inst->_list.push_back(OSDImpl::OSDMessage(str, fg, bg));
        return (0);
    }

    // TODO : optimize this code
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
                    PrivColor::ToFramebuffer(framebuf0, bg);
                    PrivColor::ToFramebuffer(framebuf1, bg);                
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
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);
                        PrivColor::ToFramebuffer(framebuf2, bg);
                        PrivColor::ToFramebuffer(framebuf3, bg);                   
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
                                PrivColor::ToFramebuffer(framebuf0, fg);
                                PrivColor::ToFramebuffer(framebuf1, fg);
                                PrivColor::ToFramebuffer(framebuf2, fg);
                                PrivColor::ToFramebuffer(framebuf3, fg);
                                //_DrawPixel(posX + x, posY + yy, fg);
                            }
                            else
                            {
                                PrivColor::ToFramebuffer(framebuf0, bg);
                                PrivColor::ToFramebuffer(framebuf1, bg);
                                PrivColor::ToFramebuffer(framebuf2, bg);
                                PrivColor::ToFramebuffer(framebuf3, bg);
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
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);                
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
        }
    }
}