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

        // New | Old  93 + 106 = 199 + 20 = 219
        Renderer::DrawLine(219, 25, 1, gainsboro, 190);

        int posY = 40;
        // Column names
        Renderer::DrawString("Address", 53, posY, blank);

        posY = 40;
        Renderer::DrawString("New Value", 149, posY, blank);

        posY = 40;
        Renderer::DrawString("Old Value", 255, posY, blank);
    }

    void    SearchMenu::Update(void)
    {

    }
}