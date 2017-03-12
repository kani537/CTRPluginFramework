#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearch.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    PluginMenuSearch::PluginMenuSearch() :
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
    _memoryRegions(150, 50, 130, 15),
    _searchSize(150, 70, 130, 15),
    _searchType(150, 90, 130, 15),
    _compareType(150, 110, 130, 15)
    {
        _currentSearch = nullptr;

        // Init ComboBoxes

        _searchSize.Add("1 Byte");
        _searchSize.Add("2 Bytes");
        _searchSize.Add("4 Bytes");
        _searchSize.Add("8 Bytes");
        _searchSize.Add("Float");
        _searchSize.Add("Double");

        _searchType.Add("Specified value");
        _searchType.Add("Unknown search");

        _compareType.Add("Equal");
        _compareType.Add("Not Equal");
        _compareType.Add("Greater Than");
        _compareType.Add("Greater Or Equal");
        _compareType.Add("Lesser Than");
        _compareType.Add("Lesser Or Equal");
        _compareType.Add("Different By");
        _compareType.Add("Different By Less");
        _compareType.Add("Different By More");

    }

    bool    PluginMenuSearch::operator()(EventList &eventList, Time &delta)
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

        // Check ComboBox

        _memoryRegions();
        _searchSize();
        _searchType();
        _compareType();

        return (false);
    }

    /*
    ** Process Event
    *****************/
    void    PluginMenuSearch::_ProcessEvent(Event &event)
    {

    }

    /*
    ** Render Top
    **************/

    void    PluginMenuSearch::_RenderTop(void)
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
    void    PluginMenuSearch::_RenderBottom(void)
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

        int posY = 52;
        int textPosX = 30;
        int choicePosX = 150;

        // MemRegion
        Renderer::DrawString("MemRegion:", textPosX, posY, blank);
        posY = 72;

        // Value Type
        Renderer::DrawString("Value Type:", textPosX, posY, blank);
        posY = 92;

        // Search Type
        Renderer::DrawString("Search Type:", textPosX, posY, blank);
        posY = 112;

        // Scan Type
        Renderer::DrawString("Scan Type:", textPosX, posY, blank);
        posY = 132;


        // Draw ComboBoxes
        _memoryRegions.Draw();
        _searchSize.Draw();
        _searchType.Draw();
        _compareType.Draw();

        // Draw buttons
        _closeBtn.Draw();
    }

    /*
    ** Update
    ************/
    void    PluginMenuSearch::_Update(Time delta)
    {
        /*
        ** Buttons
        *************/
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        // Update ComboBoxes
        _memoryRegions.Update(isTouched, touchPos);
        _searchSize.Update(isTouched, touchPos);
        _searchType.Update(isTouched, touchPos);
        _compareType.Update(isTouched, touchPos);

        // Update buttons
        _closeBtn.Update(isTouched, touchPos);
    }

    /*
    ** DoSearch
    ************/
    void    PluginMenuSearch::_DoSearch(void)
    {
        if (_currentSearch == nullptr) return;

        bool isFinished = _currentSearch->DoSearch();

        // if finished, write results to file
        if (isFinished)
        {
            //_currentSearch->file.Write(_currentSearch->)
        }

    }
}