#include "CTRPluginFrameworkImpl/Menu/PluginMenu_SearchMenu.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

namespace CTRPluginFramework
{
    SearchMenu::SearchMenu(SearchBase* &curSearch) : _currentSearch(curSearch)
    {
        _index = 0;
        _selector = 0;
    }

    /*
    ** ProcessEvent
    ****************/
    bool    SearchMenu::ProcessEvent(EventList &eventList, Time &delta)
    {
        for (int i = 0; i < eventList.size(); i++)
        {
            Event &event = eventList[i];

            if (event.type == Event::EventType::KeyPressed)
            {
                if (_currentSearch != nullptr)
                {
                    switch (event.key.code)
                    {
                        case Key::DPadUp:
                        {
                            _selector = std::max((u32)(_selector - 1),(u32)(0));
                            break;
                        }
                        case Key::DPadDown:
                        {
                            _selector = std::min((u32)(_selector + 1),(u32)(499));                            
                            break;
                        }
                        case Key::DPadLeft:
                        {
                            _selector = 0;
                            _index = std::max((u32)(_index - 500),(u32)(0));
                            Update();
                            break;
                        }
                        case Key::DPadRight:
                        {
                            _selector = 0;
                            _index = std::min((u32)(_index + 500),(u32)(_currentSearch->ResultCount / 500 * 500));
                            Update();
                            break;
                        }
                    } // end switch
                } // end if
            }
        }
    }

    /*
    ** Draw
    ********/
    void    SearchMenu::Draw(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static Color    darkgrey(169, 169, 169);
        static Color    gainsboro(220, 220, 220);
        static Color    skyblue(0, 191, 255);
        static Color    silver(192, 192, 192);
        //static IntRect  background(30, 20, 340, 200);

        /*330
        ADDRESS (8 * 6) = 48 + 10   = 58 + 20
        OLD (16 * 6 ) = 96 + 10     = 106 + 20
        NEW (16 * 6) = 96 + 10      = 106 + 20
                                    = 270 = 330*/

        int posY = 25;
        int xx = Renderer::DrawSysString("Search", 35, posY, 330, blank);
        Renderer::DrawLine(35, posY, xx, skyblue);

        posY += 10;

        if (_currentSearch != nullptr)
        {
            std::string str = "Step: " + std::to_string(_currentSearch->Step);
            Renderer::DrawString((char *)str.c_str(), 35, posY, blank);
            str = "Hit(s): " + std::to_string(_currentSearch->ResultCount);
            Renderer::DrawString((char *)str.c_str(), 35, posY, blank);
        }

        posY = 80;
        /*************     Columns headers    ********************************/
        /**/    // Name
        /**/    Renderer::DrawRect(35, 75, 78, 20, darkgrey);
        /**/    Renderer::DrawString((char *)"Address", 53, posY, black);
        /**/    posY = 80;
        /**/
        /**/    // New value
        /**/    Renderer::DrawRect(113, 75, 126, 20, darkgrey);
        /**/    Renderer::DrawString((char *)"New Value", 149, posY, black);
        /**/    posY = 80;
        /**/
        /**/    // OldValue
        /**/    Renderer::DrawRect(239, 75, 126, 20, darkgrey);
        /**/    Renderer::DrawString((char *)"Old Value", 275, posY, black);
        /**/
        /**********************************************************************/

        posY = 95;
        /*************************     Grid    ********************************/
        /**/    for (int i = 0; i < 10; i++)
        /**/    {
        /**/        Color &c = i % 2 ? gainsboro : blank;
        /**/        Renderer::DrawRect(35, posY, 330, 10, c);
        /**/        posY += 10;
        /**/    }
        /**/
        /**********************************************************************/

        if (_currentSearch == nullptr || _resultsAddress.size() == 0 || _resultsNewValue.size() == 0)
            return;

        posY = 95;
        int posX1 = 47;
        int posX2 = 113;
        int posX3 = 239;

        int start = std::max((int)0, (int)_selector - 5);

        int end = std::min((int)_resultsAddress.size(), (int)(start + 10));

        for (int i = start; i < end; i++)
        {
            if (i >= _resultsAddress.size())
                return;

            // Selector
            if (i == _selector)
                Renderer::DrawRect(35, 95 + (i - start) * 10, 330, 10, silver);

            int pos = posX1;
            int posy = posY;

            // Address
            Renderer::DrawString((char *)_resultsAddress[i].c_str(), pos, posy, black);

            // newval
            posy = posY;
            std::string &nval = _resultsNewValue[i];
            pos = posX2 + (126 - (nval.size() * 6)) / 2;
            Renderer::DrawString((char *)nval.c_str(), pos, posy, black);

            if (i >= _resultsOldValue.size())
            {
                posY += 10;
                continue;
            }

            // oldval
            std::string &oval = _resultsOldValue[i];
            pos = posX3 + (126 - (oval.size() * 6)) / 2;
            Renderer::DrawString((char *)oval.c_str(), pos, posY, black);
        }

        start += _index;
        std::string str = std::to_string(start) + "-" + std::to_string(std::min((u32)(start + 10), (u32)_currentSearch->ResultCount))
                    + " / " + std::to_string(_currentSearch->ResultCount);
        posY = 196;
        Renderer::DrawString((char *)str.c_str(), 38, posY, blank);

        posY = 205;
        Renderer::DrawString((char *)"Options:", 260, posY, blank);
        posY -= 14;
        Renderer::DrawSysString((char *)"\uE002", 314, posY, 330, blank);
    }

    void    SearchMenu::Update(void)
    {
        _resultsAddress.clear();
        _resultsNewValue.clear();
        _resultsOldValue.clear();

        if (_currentSearch == nullptr)
            return;

        _currentSearch->FetchResults(_resultsAddress, _resultsNewValue, _resultsOldValue, _index, 500);
    }
}