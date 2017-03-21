#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearch.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    PluginMenuSearch::PluginMenuSearch() :
    _hexEditor(0),
    _searchMenu(_currentSearch, _hexEditor, _inEditor),
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
    _memoryRegions(150, 40, 130, 15),
    _startRangeTextBox(85, 60, 66, 15),
    _endRangeTextBox(214, 60, 66, 15),
    _searchSize(150, 80, 130, 15),
    _searchType(150, 100, 130, 15),
    _compareType(150, 120, 130, 15),
    _alignmentTextBox(150, 140, 130, 15),
    _valueTextBox(150, 160, 130, 15),
    _searchBtn("Search", *this, &PluginMenuSearch::_searchBtn_OnClick, IntRect(35, 195, 80, 15)),
    _undoBtn("Undo", *this, &PluginMenuSearch::_undoBtn_OnClick, IntRect(120, 195, 80, 15)),
    _cancelBtn("Cancel", *this, &PluginMenuSearch::_cancelBtn_OnClick, IntRect(120, 195, 80, 15)),
    _resetBtn("Reset", *this, &PluginMenuSearch::_resetBtn_OnClick, IntRect(205, 195, 80, 15)),
    _inSearch(false),
    _firstRegionInit(false),
    _step(0),
    _waitForUser(false),
    _buildResult(false),
    _inEditor(false)
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

        _compareType.Add("Equal To");
        _compareType.Add("Not Equal To");
        _compareType.Add("Bigger Than");
        _compareType.Add("Bigger Or Equal");
        _compareType.Add("Smaller Than");
        _compareType.Add("Smaller Or Equal");
        _compareType.Add("Different By");
        _compareType.Add("Different By Less");
        _compareType.Add("Different By More");

        // Init buttons
        _searchBtn.UseSysFont(false);
        _undoBtn.UseSysFont(false);
        _undoBtn.IsEnabled = false;
        _resetBtn.UseSysFont(false);
        _resetBtn.IsEnabled = false;
        _cancelBtn.UseSysFont(false);
        _cancelBtn.IsEnabled = false;

        // Set 4 Bytes as default
        u32   al = 4;
        _alignmentTextBox.SetValue(al);
        _searchSize.SelectedItem = 2;

        // Set all memory search by default
        _startRangeTextBox.SetValue((u32)(0x0));
        _endRangeTextBox.SetValue((u32)(0xFFFFFFF0));
        _startRangeTextBox.IsEnabled = false;
        _endRangeTextBox.IsEnabled = false;
    }

    bool    PluginMenuSearch::operator()(EventList &eventList, Time &delta)
    {
        static bool trigger = false;

        if (_inEditor)
        {
            if (_hexEditor(eventList))
                _inEditor = false;
            return (false);
        }

        if (trigger)
        {
            trigger = false;
            _searchMenu.Update();
        }

        // If we're on first search
        if (!_firstRegionInit)
        {
            _ListRegion();
            _firstRegionInit = true;
        }


        // Process Event
        _searchMenu.ProcessEvent(eventList, delta);
        for (int i = 0; i < eventList.size(); i++)
        {
            _ProcessEvent(eventList[i]);
        }

        // Update
        _Update(delta);

        // Render Top
        _RenderTop();

        // Render Bottom
        _RenderBottom();

        // Do search
        if (_inSearch)
        {
            if (_cancelBtn())
                return (false);

            bool finish = _currentSearch->DoSearch();
            _inSearch = !finish;
            if (finish)
            {
                _waitForUser = true;
                _compareType.IsEnabled = true;
            }
            return (false);
        }

        if (_buildResult)
        {
            _buildResult = false;
            trigger = true;   
            return (false);  
        }

        if (_waitForUser)
            return (false);

        // Check buttons

        if (_closeBtn())
            return (true);

        _searchBtn();
        _undoBtn();
        _resetBtn();


        // Check ComboBox

        // Changed memory region
        if (_memoryRegions())
        {
            u32     startRange = _startRangeTextBox.Bits32;
            u32     endRange = _endRangeTextBox.Bits32;
            bool    isEnabled = false;
            Region  reg;
            Region  &region = (_memoryRegions.SelectedItem <= 1) ? reg : _regionsList[_memoryRegions.SelectedItem - 2];

            switch (_memoryRegions.SelectedItem)
            {
                case 0: startRange = 0; endRange = 0xFFFFFFF0; break;
                case 1: isEnabled = true; break;
                default: startRange = region.startAddress; endRange = region.endAddress; break;
            }

            _startRangeTextBox.SetValue(startRange);
            _endRangeTextBox.SetValue(endRange);
            _startRangeTextBox.IsEnabled = isEnabled;
            _endRangeTextBox.IsEnabled = isEnabled;
        }

        // Value type changed
        if (_searchSize())
        {
            switch (_searchSize.SelectedItem)
            {
                case 0: _alignmentTextBox.SetValue((u32)(1)); _valueTextBox.ValueType = NumericTextBox::Type::Bits8; break;
                case 1: _alignmentTextBox.SetValue((u32)(2)); _valueTextBox.ValueType = NumericTextBox::Type::Bits16; break;
                case 2: _alignmentTextBox.SetValue((u32)(4)); _valueTextBox.ValueType = NumericTextBox::Type::Bits32; break;
                case 3: _alignmentTextBox.SetValue((u32)(8)); _valueTextBox.ValueType = NumericTextBox::Type::Bits64; break;
                case 4: _alignmentTextBox.SetValue((u32)(4)); _valueTextBox.ValueType = NumericTextBox::Type::Float; break;
                case 5: _alignmentTextBox.SetValue((u32)(8)); _valueTextBox.ValueType = NumericTextBox::Type::Double; break;
            }
            _valueTextBox.Clear();
        }

        // Search type changed
        if (_searchType())
        {
            // Specified value
            if (_searchType.SelectedItem == 0)
            {
                _valueTextBox.IsEnabled = true;
                _compareType.IsEnabled = true;
            }
            // Unknown search
            else
            {
                // First search
                if (_currentSearch == nullptr)
                {
                    _compareType.IsEnabled = false;
                    _valueTextBox.IsEnabled = false;
                }
                else
                {
                    _compareType.IsEnabled = true;
                    if (_compareType.SelectedItem <= 5 && _searchType.SelectedItem == 1)
                        _valueTextBox.IsEnabled = false;
                    else
                        _valueTextBox.IsEnabled = true;
                }
            }
        }

        // Compare type changed
        if (_compareType())
        {
            if (_compareType.SelectedItem <= 5 && _searchType.SelectedItem == 1)
                _valueTextBox.IsEnabled = false;
            else
                _valueTextBox.IsEnabled = true;
        }

        // Check NumericTextBoxes
        _alignmentTextBox();
        _valueTextBox();
        _startRangeTextBox();
        _endRangeTextBox();

        return (false);
    }

    /*
    ** Process Event
    *****************/
    void    PluginMenuSearch::_ProcessEvent(Event &event)
    {
        // Close result window
        if (_waitForUser && event.type == Event::KeyDown && event.key.code == Key::A)
        {
            _waitForUser = false;
            _buildResult = true;
            _cancelBtn.IsEnabled = false;
            _resetBtn.IsEnabled = true;
            if (_step > 1)
                _undoBtn.IsEnabled = true;
        }
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

        if (_inSearch || _waitForUser)
        {
            _ShowProgressWindow();
            return;
        }

        if (_buildResult)
        {
            _ShowBuildResultWindow();
            return;
        }

        // Draw background
        if (Preferences::topBackgroundImage->IsLoaded())
            Preferences::topBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(32, 22, 336, 196, blank, false);            
        }

        _searchMenu.Draw();
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

        int posY = 42;
        int textPosX = 30;
        int choicePosX = 150;

        // MemRegion
        Renderer::DrawString((char *)"MemRegion:", textPosX, posY, blank);
        posY = 62;

        // Start Range
        Renderer::DrawString((char *)"Start:", textPosX, posY, blank);
        posY = 62;

        // End Range
        Renderer::DrawString((char *)"Stop:", 170, posY, blank);
        posY = 82;

        // Value Type
        Renderer::DrawString((char *)"Value Type:", textPosX, posY, blank);
        posY = 102;

        // Search Type
        Renderer::DrawString((char *)"Search Type:", textPosX, posY, blank);
        posY = 122;

        // Scan Type
        Renderer::DrawString((char *)"Scan Type:", textPosX, posY, blank);
        posY = 142;

        // Alignment
        Renderer::DrawString((char *)"Alignment:", textPosX, posY, blank);
        posY = 162;

        // Value
        Renderer::DrawString((char *)"Value:", textPosX, posY, blank);
        posY = 182;


        // Draw ComboBoxes
        _memoryRegions.Draw();
        _searchSize.Draw();
        _searchType.Draw();
        _compareType.Draw();

        // Draw NumericTextBoxes
        _alignmentTextBox.Draw();
        _valueTextBox.Draw();
        _startRangeTextBox.Draw();
        _endRangeTextBox.Draw();


        // Draw buttons
        _closeBtn.Draw();
        _searchBtn.Draw();
        _cancelBtn.Draw();
        _undoBtn.Draw();
        _resetBtn.Draw();
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

        // Update NumericTextBoxes
        _alignmentTextBox.Update(isTouched, touchPos);
        _valueTextBox.Update(isTouched, touchPos);
        _startRangeTextBox.Update(isTouched, touchPos);
        _endRangeTextBox.Update(isTouched, touchPos);

        // Update buttons
        _closeBtn.Update(isTouched, touchPos);
        _searchBtn.Update(isTouched, touchPos);
        _cancelBtn.Update(isTouched, touchPos);
        _undoBtn.Update(isTouched, touchPos);
        _resetBtn.Update(isTouched, touchPos);
    }

    /*
    ** Search button On_Click
    ************/
    void    PluginMenuSearch::_searchBtn_OnClick(void)
    {
        // If it's not the first research, add it to the history
        if (_currentSearch != nullptr)
            _searchHistory.push_back(_currentSearch);

        if (_memoryRegions.SelectedItem == -1 || _memoryRegions.SelectedItem > _regionsList.size())
            return;

        u32     startRange = _startRangeTextBox.Bits32;
        u32     endRange = _endRangeTextBox.Bits32;

        /*if (_memoryRegions.SelectedItem == 0)
        {
            startRange = 0;
            endRange = 0;
        }
        else
        {
            startRange = _regionsList[_memoryRegions.SelectedItem - 1].startAddress;
            endRange = _regionsList[_memoryRegions.SelectedItem - 1].endAddress;  
        }*/

        u32     step = _searchHistory.size();

        //Flush memory
        svcFlushProcessDataCache(Process::GetHandle(), (void *)startRange, endRange - startRange);

        // Create search object and set SearchSize
        if (_memoryRegions.SelectedItem == 0)
        {
            switch (_searchSize.SelectedItem)
            {
                case 0: _currentSearch = new Search<u8>(_valueTextBox.Bits8, _regionsList, _alignmentTextBox.Bits32, _currentSearch);  _currentSearch->Size = SearchSize::Bits8; break;
                case 1: _currentSearch = new Search<u16>(_valueTextBox.Bits16, _regionsList, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits16; break;
                case 2: _currentSearch = new Search<u32>(_valueTextBox.Bits32, _regionsList, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits32; break;
                case 3: _currentSearch = new Search<u64>(_valueTextBox.Bits64, _regionsList, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits64; break;
                case 4: _currentSearch = new Search<float>(_valueTextBox.Single, _regionsList, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::FloatingPoint; break;
                case 5: _currentSearch = new Search<double>(_valueTextBox.Double, _regionsList, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Double; break;
            }

        }
        else
        {
            switch (_searchSize.SelectedItem)
            {
                case 0: _currentSearch = new Search<u8>(_valueTextBox.Bits8, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch);  _currentSearch->Size = SearchSize::Bits8; break;
                case 1: _currentSearch = new Search<u16>(_valueTextBox.Bits16, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits16; break;
                case 2: _currentSearch = new Search<u32>(_valueTextBox.Bits32, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits32; break;
                case 3: _currentSearch = new Search<u64>(_valueTextBox.Bits64, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits64; break;
                case 4: _currentSearch = new Search<float>(_valueTextBox.Single, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::FloatingPoint; break;
                case 5: _currentSearch = new Search<double>(_valueTextBox.Double, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Double; break;
            }
        }


        // Set Search Type
        _currentSearch->Type = static_cast<SearchType>(_searchType.SelectedItem);

        // Set CompareType
        _currentSearch->Compare = static_cast<CompareType>(_compareType.SelectedItem);

        // Lock memory region
        _memoryRegions.IsEnabled = false;
        _startRangeTextBox.IsEnabled = false;
        _endRangeTextBox.IsEnabled = false;
        // Lock search size
        _searchSize.IsEnabled = false;
        // Lock alignment
        _alignmentTextBox.IsEnabled = false;

        // Enable Cancel button
        _cancelBtn.IsEnabled = true;

        _inSearch = true;

        _step++;
    }

    void    PluginMenuSearch::_cancelBtn_OnClick(void)
    {
            _currentSearch->Cancel();
            _inSearch = false;
            _waitForUser = true;
            _compareType.IsEnabled = true;
            _cancelBtn.IsEnabled = false;
    }

    void    PluginMenuSearch::_resetBtn_OnClick(void)
    {
        // Clear history
        if (_searchHistory.size() > 0)
        {
            for (auto it = _searchHistory.begin(); it != _searchHistory.end(); it++)
                delete *it;
            _searchHistory.clear();         
        }

        if (_currentSearch != nullptr)
            delete _currentSearch;

        // Reset _currentSearch
        _currentSearch = nullptr;

        // Update memory regions
        _ListRegion();

        // Unlock memory rgions
        _memoryRegions.IsEnabled = true;
        //_startRangeTextBox.IsEnabled = true;
        //_endRangeTextBox.IsEnabled = true;

        // Unlock search size
        _searchSize.IsEnabled = true;

        // Unlock alignment
        _alignmentTextBox.IsEnabled = true;

        // Reset step
        _step = 0;

        _resetBtn.IsEnabled = false;
        _undoBtn.IsEnabled = false;
        _searchMenu.Update();
    }

    void    PluginMenuSearch::_undoBtn_OnClick(void)
    {
        // Reset _currentSearch
        if (_currentSearch != nullptr)
            delete _currentSearch;

        // Get last search data
        if (_searchHistory.size())
        {
            _currentSearch = (_searchHistory.back());
            _searchHistory.pop_back();
        }
        else
            _currentSearch = nullptr;

        // Reset step
        _step--;

        if (_step <= 1)
            _undoBtn.IsEnabled = false;
        _searchMenu.Update();
    }

    void    PluginMenuSearch::_ShowProgressWindow(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    gainsboro(225, 225, 225);
        static Color    dimGrey(15, 15, 15);
        static Color    silver(160, 160, 160);
        static IntRect  background(125, 80, 150, 70);
        static IntRect  background2(125, 80, 150, 85);
        static Color    skyblue(0, 191, 255);
        static Color    limegreen(50, 205, 50);
        static Clock    timer;
        static int      phase = 0;

        std::string     waitLogo[] = 
        {
            "\uE020", "\uE021", "\uE022", "\uE023", "\uE024", "\uE025", "\uE026", "\uE027"
        };

        Renderer::SetTarget(TOP);

        // Draw "window" background
        Renderer::DrawRect2(_inSearch ? background : background2, black, dimGrey);

        int posY = 90;

        // Draw logobackground
       // Renderer::DrawSysString("\uE021", 192, posY, 300, silver); //Top
       // posY = 85;
       // Renderer::DrawSysString("\uE025", 192, posY, 300, silver); //Bottom
       // posY = 85;

        // Draw logo phase
        if (_inSearch)
            Renderer::DrawSysString(waitLogo[phase].c_str(), 192, posY, 300, skyblue);
        else
            Renderer::DrawSysString("Done", 173, posY, 300, skyblue);

        if (timer.HasTimePassed(Seconds(0.125f)))
        {
            phase++;
            timer.Restart();
        }
        
        if (phase > 7) phase = 0;

        posY += 10;

        // Progressbar
        // Draw border
        IntRect progBarBorder = IntRect(130, posY, 140, 15);
        Renderer::DrawRect(progBarBorder, gainsboro, false);

        float percent = 138.f / 100.f;
        float prog = _inSearch ? _currentSearch->Progress * percent : 138.f;      

        // Draw progress fill
        IntRect progBarFill = IntRect(131, posY + 1, (u32)prog, 13);
        Renderer::DrawRect(progBarFill, limegreen);
        posY += 20;

        if (_inSearch)
        {
            // Draw Result count
            std::string res = "Hit(s): " + std::to_string(_currentSearch->ResultCount);            
            Renderer::DrawString((char *)res.c_str(), 131, posY, blank);
        }
        if (!_inSearch)
        {
            std::string res = std::to_string(_currentSearch->ResultCount) + " hit(s) found";
            std::string res2 =  "in " + std::to_string(_currentSearch->SearchTime.AsSeconds()) + "s";            
            Renderer::DrawString((char *)res.c_str(), 131, posY, blank);
            Renderer::DrawString((char *)res2.c_str(), 131, posY, blank);
            posY -= 10;
            Renderer::DrawSysString("\uE000", 255, posY, 300, skyblue);
        }
    }


    void    PluginMenuSearch::_ShowBuildResultWindow(void)
    {
        static Color    black = Color();
        static Color    dimGrey(15, 15, 15);
        static IntRect  background(125, 80, 150, 70);
        static Color    skyblue(0, 191, 255);

        Renderer::SetTarget(TOP);

        // Draw "window" background
        Renderer::DrawRect2(background, black, dimGrey);

        int posY = 107;

        float   length = Renderer::GetTextSize("Build hit(s) list...");
        Renderer::DrawSysString("Build hit(s) list...", 125 + ((150 - length) /2), posY, 300, skyblue);
    }
    extern "C" u32 __ctru_linear_heap;
    extern "C" u32 __ctru_linear_heap_size;
    void    PluginMenuSearch::_ListRegion(void)
    {

        Handle      target = Process::GetHandle();
        PageInfo    page_info;
        MemInfo     meminfo;
        u32         save_addr;
        int         i;
        int         index = _memoryRegions.SelectedItem;
        Result      ret;

        _regionsList.clear();
        _memoryRegions.Clear();

        {
            //Region reg = (Region){0x100000, 0x50000000};
            //_regionsList.push_back(reg);
            _memoryRegions.Add("All memory");
            _memoryRegions.Add("Custom range");
            if (index == 1)
            {
                _memoryRegions.SelectedItem = 1;
                _startRangeTextBox.IsEnabled = true;
                _endRangeTextBox.IsEnabled = true;
            }
        }

        svcQueryProcessMemory(&meminfo, &page_info, target, 0x00100000);
        {
            Region reg = (Region){meminfo.base_addr, meminfo.base_addr + meminfo.size};
            _regionsList.push_back(reg);
            char    buffer[0x100] = {0};

            sprintf(buffer, "%08X-%08X", reg.startAddress, reg.endAddress);
            _memoryRegions.Add(buffer);
        }
        save_addr = meminfo.base_addr + meminfo.size + 1;
        i = 1;
        while (save_addr < 0x50000000)
        {
            if (i >= 99)
                break;
            ret = svcQueryProcessMemory(&meminfo, &page_info, target, save_addr);
            if (R_FAILED(ret))
            {
                if (meminfo.base_addr >= 0x50000000)
                    break;
                if (save_addr >= meminfo.base_addr + meminfo.size + 1)
                    save_addr += 0x1000;
                else
                    save_addr = meminfo.base_addr + meminfo.size + 1;
                continue;
            }

            save_addr = meminfo.base_addr + meminfo.size + 1;
            if (meminfo.base_addr == 0x06000000 || meminfo.base_addr == 0x07000000 || meminfo.base_addr == 0x07500000 || meminfo.base_addr == __ctru_linear_heap)
                continue;

            if (meminfo.state != 0x0 && meminfo.state != 0x2 && meminfo.state != 0x3 && meminfo.state != 0x6)
            {
                if (meminfo.perm & MEMPERM_READ)
                {
                    if (meminfo.base_addr < __ctru_linear_heap && meminfo.base_addr + meminfo.size > __ctru_linear_heap)
                    {
                        meminfo.size = __ctru_linear_heap - meminfo.base_addr;
                    }

                    Region reg = (Region){meminfo.base_addr, meminfo.base_addr + meminfo.size};
                    _regionsList.push_back(reg);
                    char    buffer[0x100] = {0};

                    sprintf(buffer, "%08X-%08X", reg.startAddress, reg.endAddress);
                    _memoryRegions.Add(buffer);
                    i++;
                }
            }
            
            if (meminfo.base_addr >= 0x50000000)
                break;
        }
        //if (meminfo[i].state == 0x0)
        //  i--;
        i++;
    }
}