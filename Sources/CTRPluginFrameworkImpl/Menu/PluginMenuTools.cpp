#include "CTRPluginFrameworkImpl/Menu/PluginMenuTools.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    PluginMenuTools::PluginMenuTools() :
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose)
    {
        Renderer::GetAlpha('A');
        Renderer::ScaleFont();
    }

    bool    PluginMenuTools::operator()(EventList &eventList, Time &delta)
    {
        // Process Event
        for (int i = 0; i < eventList.size(); i++)
            _ProcessEvent(eventList[i]); 

        // Update
        _Update(delta);

        // Render Top
        _RenderTop();

        // Render Bottom
        _RenderBottom();

        // Check buttons

        if (_closeBtn())
            return (true);

        return (false);
    }

    /*
    ** Process Event
    *****************/
    void    PluginMenuTools::_ProcessEvent(Event &event)
    {

    }

    /*
    ** Render Top
    **************/

    void    PluginMenuTools::_RenderTop(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static Color    silver(160, 160, 160);
        static IntRect  background(30, 20, 340, 200);

        int   posY = 25;
        int   posX = 40;

        // Enable renderer
        Renderer::SetTarget(TOP);

        // Draw background
        if (Preferences::topBackgroundImage->IsLoaded())
            Preferences::topBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(32, 22, 336, 196, blank, false);            
        }
    }

    /*
    ** Render Bottom
    *****************/
    void    PluginMenuTools::_RenderBottom(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static IntRect  background(20, 20, 280, 200);

        // Enable renderer
        Renderer::SetTarget(BOTTOM);

        // Background
        if (Preferences::bottomBackgroundImage->IsLoaded())
            Preferences::bottomBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(22, 22, 276, 196, blank, false);            
        }

        static bool type = false;

        if (Controller::IsKeyPressed(Key::X))
            type = !type;

        if (!type)
            Renderer::DrawChar('A', 150, 100);
        else
        {
            Renderer::DrawArray();
            Renderer::DrawArray2();
        }

        int posY = 205;
        Renderer::DrawString((char *)"CTRPluginFramework Alpha V.0.0.4", 40, posY, blank);

        // Draw buttons
        _closeBtn.Draw();
    }

    /*
    ** Update
    ************/
    void    PluginMenuTools::_Update(Time delta)
    {
        /*
        ** Buttons
        *************/
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        _closeBtn.Update(isTouched, touchPos);
    }
}