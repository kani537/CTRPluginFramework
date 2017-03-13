#include "CTRPluginFrameworkImpl/Menu/PluginMenu_SearchMenu.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

namespace CTRPluginFramework
{
    SearchMenu::SearchMenu(SearchBase* &curSearch) : _currentSearch(curSearch)
    {

    }

    /*
    ** ProcessEvent
    ****************/
    bool    SearchMenu::ProcessEvent(EventList &eventList, Time &delta)
    {

    }

    /*
    ** Draw
    ********/
    void    SearchMenu::Draw(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static Color    silver(160, 160, 160);
        static Color    gainsboro(225, 225, 225);
        //static IntRect  background(30, 20, 340, 200);

        //Draw sheet

        IntRect     rect = IntRect(35, 25, 330, 190);

        // Big rect
        Renderer::DrawRect(rect, gainsboro, false);

        // Column name underlight
        Renderer::DrawLine(35, 45, 330, gainsboro);

        // Column separator

        /*330
        ADDRESS (8 * 6) = 48 + 10   = 58 + 20
        OLD (16 * 6 ) = 96 + 10     = 106 + 20
        NEW (16 * 6) = 96 + 10      = 106 + 20
                                    = 270 = 330*/

        // Name | New  35 + 58 = 93 + 20 = 113
        Renderer::DrawLine(113, 25, 1, gainsboro, 190);

        // New | Old  113 + 106 = 219 + 20 = 239
        Renderer::DrawLine(239, 25, 1, gainsboro, 190);

        int posY = 30;
        // Column names
        Renderer::DrawString((char *)"Address", 53, posY, blank);

        posY = 30;
        Renderer::DrawString((char *)"New Value", 149, posY, blank);

        posY = 30;
        Renderer::DrawString((char *)"Old Value", 275, posY, blank);

        // Draw values

        posY = 50;
        int posX1 = 47;
        int posX2 = 118;
        int posX3 = 244;

        for (int i = 0; i < 15; i++)
        {
            if (i >= _resultsAddress.size())
                return;

            int pos = posX1;
            int posy = posY;

            // Address
            Renderer::DrawString((char *)_resultsAddress[i].c_str(), pos, posy, blank);

            // newval
            posy = posY;
            std::string &nval = _resultsNewValue[i];
            pos = posX2 + (126 - (nval.size() * 6)) / 2;
            Renderer::DrawString((char *)nval.c_str(), pos, posy, blank);

            if (i >= _resultsOldValue.size())
            {
                posY += 10;
                continue;
            }

            // oldval
            std::string &oval = _resultsOldValue[i];
            pos = posX3 + (126 - (oval.size() * 6)) / 2;
            Renderer::DrawString((char *)oval.c_str(), pos, posY, blank);
        }
    }

    void    SearchMenu::Update(void)
    {
        _resultsAddress.clear();
        _resultsNewValue.clear();
        _resultsOldValue.clear();

        if (_currentSearch == nullptr)
            return;

        _currentSearch->FetchResults(_resultsAddress, _resultsNewValue, _resultsOldValue);
    }
}