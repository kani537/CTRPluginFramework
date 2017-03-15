#include "CTRPluginFrameworkImpl/Menu/PluginMenu_SearchMenu.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/System/Process.hpp"

#include <cstdlib>
#include <ctime>

namespace CTRPluginFramework
{
    SearchMenu::SearchMenu(SearchBase* &curSearch) : _currentSearch(curSearch)
    {
        _index = 0;
        _selector = 0;
        _submenuSelector = 0;

        _options.push_back("Save");
        _options.push_back("Edit");
        _options.push_back("Jump in editor");
        _options.push_back("Export");
        _options.push_back("Export all");

        _isSubmenuOpen = false;
        _action = false;
        _alreadyExported = false;

        if (File::Open(_export, "ExportedAddresses.txt", File::WRITE | File::APPEND))
        {
            File::Open(_export, "ExportedAddresses.txt", File::WRITE | File::CREATE);
        }
    }

    /*
    ** ProcessEvent
    ****************/
    bool    SearchMenu::ProcessEvent(EventList &eventList, Time &delta)
    {
        static Clock    _fastScroll;
        static Clock    _startFastScroll;

        for (int i = 0; i < eventList.size(); i++)
        {
            Event &event = eventList[i];

            // Pressed
            if (event.type == Event::EventType::KeyPressed)
            {
                if (_currentSearch != nullptr)
                {
                    switch (event.key.code)
                    {
                        case Key::DPadUp:
                        {
                            if (!_isSubmenuOpen)
                            {
                                _selector = std::max((int)(_selector - 1),(int)(0));
                                _startFastScroll.Restart();
                            }
                            else
                            {
                                _submenuSelector = std::max((int)(_submenuSelector - 1), (int)0);
                            }
                            break;
                        }
                        case Key::DPadDown:
                        {
                            if (!_isSubmenuOpen)
                            {
                                _selector = std::min((int)(_selector + 1),(int)(499));
                                if (_index + _selector > _currentSearch->ResultCount)
                                    _selector = _currentSearch->ResultCount % 500; 
                                _startFastScroll.Restart();
                            }     
                            else
                            {
                                _submenuSelector = std::min((int)(_submenuSelector + 1), (int)4);
                            }       
                            break;
                        }
                        case Key::DPadLeft:
                        {
                            if (!_isSubmenuOpen)
                            {
                                _selector = 0;
                                _index = std::max((int)(_index - 500), (int)(0));
                                _startFastScroll.Restart();
                                Update();
                            }
                            break;
                        }
                        case Key::DPadRight:
                        {
                            if (!_submenuSelector)
                            {
                                _selector = 0;
                                _index = std::min((int)(_index + 500),(int)(_currentSearch->ResultCount / 500 * 500));
                                _startFastScroll.Restart();
                                Update();
                            }
                            break;
                        }
                        case Key::X:
                        {
                            _isSubmenuOpen = !_isSubmenuOpen;
                            break;
                        }
                        case Key::A:
                        {
                            if (_isSubmenuOpen)
                            {
                                switch (_submenuSelector)
                                {
                                    case 0: _Save(); break;
                                    case 1: _Edit(); break;
                                    case 2: _JumpInEditor(); break;
                                    case 3: _Export(); break;
                                    case 4: _ExportAll(); break;
                                    default: break;
                                }
                            }
                            break;
                        }
                        case Key::B:
                        {
                            if (_isSubmenuOpen)
                                _isSubmenuOpen = false;
                            break;
                        }
                    } // end switch
                } // end if
            }
            // Hold
            else if (event.type == Event::EventType::KeyDown)
            {
                if (_currentSearch != nullptr && !_isSubmenuOpen && _startFastScroll.HasTimePassed(Seconds(0.5f)) && _fastScroll.HasTimePassed(Seconds(0.1f)))
                {
                    switch (event.key.code)
                    {
                        case Key::DPadUp:
                        {
                            _selector = std::max((int)(_selector - 1),(int)(0));
                            _fastScroll.Restart();
                            break;
                        }
                        case Key::DPadDown:
                        {
                            _selector = std::min((int)(_selector + 1),(int)(499));
                            if (_index + _selector > _currentSearch->ResultCount)
                                _selector = _currentSearch->ResultCount % 500;
                            _fastScroll.Restart();              
                            break;
                        }
                        case Key::DPadLeft:
                        {
                            _selector = 0;
                            _index = std::max((int)(_index - 500),(int)(0));
                            _fastScroll.Restart();
                            Update();
                            break;
                        }
                        case Key::DPadRight:
                        {
                            _selector = 0;
                            _index = std::min((int)(_index + 500),(int)(_currentSearch->ResultCount / 500 * 500));
                            _fastScroll.Restart();
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

        if (_isSubmenuOpen)
        {
            _DrawSubMenu();
            return;
        }
    }

    void    SearchMenu::Update(void)
    {
        _resultsAddress.clear();
        _resultsNewValue.clear();
        _resultsOldValue.clear();
        _isSubmenuOpen = false;

        if (_currentSearch == nullptr)
            return;

        _currentSearch->FetchResults(_resultsAddress, _resultsNewValue, _resultsOldValue, _index, 500);
    }

    void    SearchMenu::_DrawSubMenu(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static Color    darkgrey(169, 169, 169);
        static Color    gainsboro(220, 220, 220);
        static Color    skyblue(0, 191, 255);
        static Color    silver(192, 192, 192);
        static IntRect  background(240, 20, 130, 200);

        // DrawBackground
        Renderer::DrawRect2(background, black, dimGrey);

        int posY = 25;

        // Draw title's menu
        int xx = Renderer::DrawSysString("Options", 245, posY, 340, blank);
        Renderer::DrawLine(245, posY, xx - 225, skyblue);

        posY = 46;

        IntRect selRect = IntRect(241, 45 + _submenuSelector * 12, 110, 12);

        for (int i = 0; i < _options.size(); i++)
        {
            std::string &str = _options[i];

            if (i == _submenuSelector)
            {
                if (_action || !_buttonFade.HasTimePassed(Seconds(0.2f)))
                {
                    if (_action)
                    {
                        _action = false;
                        _buttonFade.Restart();
                    }
                    

                    // Draw action rectangle
                    Renderer::DrawRect(selRect, gainsboro);
                    // Draw selector
                    Renderer::DrawRect(selRect, darkgrey, false);
                    // Draw text
                    Renderer::DrawString((char *)str.c_str(), 245, posY, black);
                }
                else
                {
                    // Draw selector
                    Renderer::DrawRect(selRect, darkgrey, false);

                    // Draw text
                    Renderer::DrawString((char *)str.c_str(), 245, posY, blank);
                }
            }
            else
            {
                Renderer::DrawString((char *)str.c_str(), 245, posY, blank);
            }
            posY += 2;
        }
    }

    void    SearchMenu::_Save(void)
    {
        _action = true;
    }

    void    SearchMenu::_Edit(void)
    {
        _action = true;

        Keyboard keyboard;

        keyboard.DisplayTopScreen = false;

        u32 address = strtoul(_resultsAddress[_selector].c_str(), NULL, 16);

        switch (_currentSearch->Size)
        {
            case SearchSize::Bits8:
            {
                u8 value = *(u8 *)(address);//strtoul(_resultsNewValue[_selector].c_str(), NULL, 16);

                int res = keyboard.Open(value, value);

                if (res != -1)
                {
                    if (Process::CheckAddress(address))
                        *(u8 *)(address) = value;
                }
                break;
            }
            case SearchSize::Bits16:
            {
                u16 value = *(u16 *)(address);//strtoul(_resultsNewValue[_selector].c_str(), NULL, 16);

                int res = keyboard.Open(value, value);

                if (res != -1)
                {
                    if (Process::CheckAddress(address))
                        *(u16 *)(address) = value;
                }
                break;
            }
            case SearchSize::Bits32:
            {
                u32 value = *(u32 *)(address);//strtoul(_resultsNewValue[_selector].c_str(), NULL, 16);

                int res = keyboard.Open(value, value);

                if (res != -1)
                {
                    if (Process::CheckAddress(address))
                        *(u32 *)(address) = value;
                }
                break;
            }
            case SearchSize::Bits64:
            {
                u64 value = *(u64 *)(address);//strtoull(_resultsNewValue[_selector].c_str(), NULL, 16);

                int res = keyboard.Open(value, value);

                if (res != -1)
                {
                    if (Process::CheckAddress(address))
                        *(u64 *)(address) = value;
                }
                break;
            }
            case SearchSize::FloatingPoint:
            {
                float value = *(float *)(address);//strtof(_resultsNewValue[_selector].c_str(), NULL);

                int res = keyboard.Open(value, value);

                if (res != -1)
                {
                    if (Process::CheckAddress(address))
                        *(float *)(address) = value;
                }
                break;
            }
            case SearchSize::Double:
            {
                double value = *(double *)(address);//strtod(_resultsNewValue[_selector].c_str(), NULL);

                int res = keyboard.Open(value, value);

                if (res != -1)
                {
                    if (Process::CheckAddress(address))
                        *(double *)(address) = value;
                }
                break;
            }
        }
    }

    void    SearchMenu::_JumpInEditor(void)
    {
        _action = true;
    }

    void    SearchMenu::_Export(void)
    {
        _action = true;
        if (!_alreadyExported)
        {
            _export.WriteLine("");
            time_t t = time(NULL);
            char *ct = ctime(&t);

            ct[strlen(ct)] = '\0';
            std::string text = ct;

            text += " :\r\n";
            _export.WriteLine(text);
            _alreadyExported = true;
        }
        std::string str = _resultsAddress[_selector] +" : " + _resultsNewValue[_selector];
        _export.WriteLine(str);
    }

    void    SearchMenu::_ExportAll(void)
    {
        _action = true;
        if (!_alreadyExported)
        {
            _export.WriteLine("");
            time_t t = time(NULL);
            char *ct = ctime(&t);

            ct[strlen(ct)] = '\0';
            std::string text = ct;

            text += " :\r\n";
            _export.WriteLine(text);
            _alreadyExported = true;
        }

        for (int i = _selector; i < _selector + 10; i++)
        {
            if (i >= _resultsAddress.size())
                break;
            std::string str = _resultsAddress[i] +" : " + _resultsNewValue[i];
            _export.WriteLine(str);
        }

    }
}