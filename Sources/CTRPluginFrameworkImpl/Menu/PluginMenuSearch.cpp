#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearch.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "3DS.h"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFramework/System/Directory.hpp"
#include <iterator>
#include "CTRPluginFramework/Utils/Utils.hpp"

namespace CTRPluginFramework
{
    PluginMenuSearch::PluginMenuSearch(HexEditor &hexEditor, FreeCheats &freeCheats) :
    _hexEditor(hexEditor),
    _searchMenu(_currentSearch, hexEditor, _inEditor, _hexInput, freeCheats),
   // _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
    _memoryRegions(150, 45, 130, 15),
    _startRangeTextBox(85, 65, 66, 15),
    _endRangeTextBox(214, 65, 66, 15),
    _searchSize(150, 85, 130, 15),
    _searchType(150, 105, 130, 15),
    _compareType(150, 125, 130, 15),
    _valueTextBox(150, 145, 130, 15),
    _searchBtn("Search", *this, &PluginMenuSearch::_searchBtn_OnClick, IntRect(35, 195, 80, 15)),
    _undoBtn("Undo", *this, &PluginMenuSearch::_undoBtn_OnClick, IntRect(120, 195, 80, 15)),
    _cancelBtn("Cancel", *this, &PluginMenuSearch::_cancelBtn_OnClick, IntRect(120, 195, 80, 15)),
    _resetBtn("Reset", *this, &PluginMenuSearch::_resetBtn_OnClick, IntRect(205, 195, 80, 15)),
    _inSearch(false),
    _firstRegionInit(false),
    _step(0),
    _waitForUser(false),
	_hexInput(false),
    _inEditor(false),
    _hexBtn("Hex", *this, nullptr, IntRect(110, 145, 38, 15), nullptr)
    {
        _currentSearch = nullptr;

        // Init ComboBoxes
        _searchSize.Add("1 Byte");
        _searchSize.Add("2 Bytes");
        _searchSize.Add("4 Bytes");
       // _searchSize.Add("8 Bytes");
        _searchSize.Add("Float");
       // _searchSize.Add("Double");

        _searchType.Add("Specified value");
        _searchType.Add("Unknown search");

        _PopulateSearchType(true);

        // Init buttons
        _searchBtn.UseSysFont(false);
        _undoBtn.UseSysFont(false);
        _undoBtn.IsEnabled = false;
        _resetBtn.UseSysFont(false);
        _resetBtn.IsEnabled = false;
        _cancelBtn.UseSysFont(false);
        _cancelBtn.IsEnabled = false;
        _hexBtn.UseSysFont(false);

        _searchSize.SelectedItem = 2;

        // Set all memory search by default
        _startRangeTextBox.SetValue((u32)(0x0));
        _endRangeTextBox.SetValue((u32)(0xFFFFFFF0));
        _startRangeTextBox.IsEnabled = false;
        _endRangeTextBox.IsEnabled = false;
	    _valueTextBox.UseHexadecimal(false);
        _valueTextBox.SetValue((u32)0);

        // Allocate search's pool
        AllocatePool();

        _uiContainer += &_memoryRegions;
        _uiContainer += &_searchSize;
        _uiContainer += &_searchType;
        _uiContainer += &_compareType;
        _uiContainer += &_startRangeTextBox;
        _uiContainer += &_endRangeTextBox;
        _uiContainer += &_valueTextBox;
        _uiContainer += &_searchBtn;
        _uiContainer += &_cancelBtn;
        _uiContainer += &_undoBtn;
        _uiContainer += &_resetBtn;
        _uiContainer += &_hexBtn;
    }

    bool    PluginMenuSearch::operator()(EventList &eventList, Time &delta)
    {
        if (_inEditor)
        {
            if (_hexEditor(eventList))
                _inEditor = false;
            return (false);
        }

        // If we're on first search
        if (!_firstRegionInit)
        {
            _ListRegion();
            _firstRegionInit = true;
        }


        // Process Event
        if (!_inSearch)
        {
            _searchMenu.ProcessEvent(eventList, delta);
            for (int i = 0; i < eventList.size(); i++)
            {
                _ProcessEvent(eventList[i]);
            }
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

            bool finish = _currentSearch->ExecuteSearch();
            _inSearch = !finish;
            if (finish)
            {
                _waitForUser = true;
                _compareType.IsEnabled = true;
                // If we just finished first search
                if (_step == 1)
                    _PopulateSearchType(false);

                // Update hits list
                _searchMenu.Update();
            }
            return (false);
        }

        if (_waitForUser)
            return (false);

        // Check buttons

        if (Window::BottomWindow.MustClose())
            return (true);

        _searchBtn();
        _undoBtn();
        _resetBtn();
		if (_hexBtn())
		{
			_hexInput = !_hexInput;
			_valueTextBox.UseHexadecimal(_hexInput);
		}			

        // Check ComboBox

        // Changed memory region
        if (_memoryRegions())
        {
            u32     startRange = _startRangeTextBox.Bits32;
            u32     endRange = _endRangeTextBox.Bits32;
            bool    isEnabled = false;
            Region  reg;
            Region  &region = (_memoryRegions.SelectedItem == 0) ? reg : _regionsList[_memoryRegions.SelectedItem - 1];

            switch (_memoryRegions.SelectedItem)
            {
                case 0: startRange = 0; endRange = 0xFFFFFFF0; break;
                default: startRange = region.startAddress; endRange = region.endAddress; isEnabled = true;  break;
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
                case 0: _valueTextBox.ValueType = NumericTextBox::Type::Bits8; break;
                case 1: _valueTextBox.ValueType = NumericTextBox::Type::Bits16; break;
                case 2: _valueTextBox.ValueType = NumericTextBox::Type::Bits32; break;
               // case 3: _valueTextBox.ValueType = NumericTextBox::Type::Bits64; break;
                case 3: _valueTextBox.ValueType = NumericTextBox::Type::Float; break;
              //  case 5: _valueTextBox.ValueType = NumericTextBox::Type::Double; break;
                default: break;
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
        _valueTextBox();

        if (_startRangeTextBox())
        {
            Region  &region = _regionsList[_memoryRegions.SelectedItem - 1];

            if (_startRangeTextBox.Bits32 >= region.endAddress
                || _startRangeTextBox.Bits32 < region.startAddress
                || _startRangeTextBox.Bits32 >= _endRangeTextBox.Bits32)
                _startRangeTextBox.SetValue(region.startAddress);
        }

        if (_endRangeTextBox())
        {
            Region  &region = _regionsList[_memoryRegions.SelectedItem - 1];

            if (_endRangeTextBox.Bits32 > region.endAddress
                || _endRangeTextBox.Bits32 <= region.startAddress
                || _endRangeTextBox.Bits32 <= _startRangeTextBox.Bits32)
                _endRangeTextBox.SetValue(region.endAddress);
        }

        return (false);
    }

    void    PluginMenuSearch::GetRegionsList(std::vector<Region>& list)
    {
        list.clear();

        _ListRegion();

        for (Region &region : _regionsList)
            list.push_back(region);
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
            _cancelBtn.IsEnabled = false;
            _resetBtn.IsEnabled = true;
            if (_searchHistory.size())
                _undoBtn.IsEnabled = true;
        }
    }

    /*
    ** Render Top
    **************/

    void    PluginMenuSearch::_RenderTop(void)
    {
        const Color    &black = Color::Black;
        const Color    &blank = Color::Blank;
        const Color    &dimGrey = Color::BlackGrey;
        //static IntRect  background(30, 20, 340, 200);

        // Enable renderer
        Renderer::SetTarget(TOP);

        if (_inSearch || _waitForUser)
        {
            _ShowProgressWindow();
            return;
        }

        Window::TopWindow.Draw();
        _searchMenu.Draw();
    }

    /*
    ** Render Bottom
    *****************/
    void    PluginMenuSearch::_RenderBottom(void)
    {
        const Color    &black = Color::Black;
        const Color    &blank = Color::Blank;
        const Color    &dimGrey = Color::BlackGrey;
        //static IntRect  background(20, 20, 280, 200);

        // Enable renderer
        Renderer::SetTarget(BOTTOM);

        Window::BottomWindow.Draw();

        int posY = 47;
        int textPosX = 30;

        // MemRegion
        Renderer::DrawString((char *)"MemRegion:", textPosX, posY, blank);
        posY = 67;

        // Start Range
        Renderer::DrawString((char *)"Start:", textPosX, posY, blank);
        posY = 67;

        // End Range
        Renderer::DrawString((char *)"Stop:", 170, posY, blank);
        posY = 87;

        // Value Type
        Renderer::DrawString((char *)"Value Type:", textPosX, posY, blank);
        posY = 107;

        // Search Type
        Renderer::DrawString((char *)"Search Type:", textPosX, posY, blank);
        posY = 127;

        // Scan Type
        Renderer::DrawString((char *)"Scan Type:", textPosX, posY, blank);
        posY = 147;

        // Value
        Renderer::DrawString((char *)"Value:", textPosX, posY, blank);
        posY = 187;

        // Draw UIControls
        _uiContainer.Draw();

        /*
        // Draw ComboBoxes
        _memoryRegions.Draw();
        _searchSize.Draw();
        _searchType.Draw();
        _compareType.Draw();

        // Draw NumericTextBoxes
        _valueTextBox.Draw();
        _startRangeTextBox.Draw();
        _endRangeTextBox.Draw();


        // Draw buttons
        //_closeBtn.Draw();
        _searchBtn.Draw();
        _cancelBtn.Draw();
        _undoBtn.Draw();
        _resetBtn.Draw();
		_hexBtn.Draw();*/
    }

    /*
    ** Update
    ************/
    void    PluginMenuSearch::_Update(Time delta)
    {
        if (_waitForUser)
            return;

        /*
        ** Buttons
        *************/
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        // Update Window
        Window::BottomWindow.Update(isTouched, touchPos);

        // Update UIControls
        _uiContainer.Update(isTouched, touchPos);

        /*
        // Update ComboBoxes
        _memoryRegions.Update(isTouched, touchPos);
        _searchSize.Update(isTouched, touchPos);
        _searchType.Update(isTouched, touchPos);
        _compareType.Update(isTouched, touchPos);

        // Update NumericTextBoxes
        _valueTextBox.Update(isTouched, touchPos);
        _startRangeTextBox.Update(isTouched, touchPos);
        _endRangeTextBox.Update(isTouched, touchPos);

        // Update buttons
        //_closeBtn.Update(isTouched, touchPos);
        _searchBtn.Update(isTouched, touchPos);
        _cancelBtn.Update(isTouched, touchPos);
        _undoBtn.Update(isTouched, touchPos);
        _resetBtn.Update(isTouched, touchPos);
		_hexBtn.Update(isTouched, touchPos);
        */
    }

    /*
    ** Search button On_Click
    ************/
    void    PluginMenuSearch::_searchBtn_OnClick(void)
    {
        // If it's not the first search, add it to the history
        if (_currentSearch != nullptr)
            _searchHistory.push_back(_currentSearch);

        if (_searchHistory.size() > 5)
        {
            auto first = _searchHistory.front();

            _searchHistory.pop_front();
            delete first;

            // TODO : search-> = nullptr;
        }

        if (_memoryRegions.SelectedItem == -1 || _memoryRegions.SelectedItem > _regionsList.size())
            return;

        u32     startRange = _startRangeTextBox.Bits32;
        u32     endRange = _endRangeTextBox.Bits32;
        SearchParameters parameters = { 0 };

        parameters.previous = _currentSearch;

        // If first search
        if (_currentSearch == nullptr)
            parameters.flags = (u32)SearchFlags::First;
        // If second search or compare with first search
        else if (_currentSearch->Step == 0)
        {
            parameters.flags = (u32)SearchFlags::Second;
            parameters.previous = _searchHistory.front();
        }
            

        // Size flags
        switch (_searchSize.SelectedItem)
        {
            case 0: parameters.flags |= (u32)SearchFlags::U8;  break;
            case 1: parameters.flags |= (u32)SearchFlags::U16;  break;
            case 2: parameters.flags |= (u32)SearchFlags::U32;  break;
            //case 3: parameters.flags |= (u32)SearchFlags::U64;  break;
            case 3: parameters.flags |= (u32)SearchFlags::Float;  break;
            //case 5: parameters.flags |= (u32)SearchFlags::Double;  break;
            default: break;
        }

        // Set Search Flags
        parameters.flags |= _searchType.SelectedItem == 0 ? 0 : (u32)SearchFlags::Unknown;

        // Set Compare Flags
        parameters.flags |= (u32)SearchFlags::Equal << _compareType.SelectedItem;

        // Set CheckValue
        parameters.value32.U32 = _valueTextBox.Bits32;
        //parameters.value64.U64 = _valueTextBox.Bits64;

        // Full memory
        if (_memoryRegions.SelectedItem == 0)
        {
            parameters.fullMemory = true;
            parameters.ranges = _regionsList;
        }
        // Range search
        else
        {
            parameters.fullMemory = false;
            Region range;
            range.startAddress = startRange;
            range.endAddress = endRange;
            parameters.ranges.push_back(range);
        }

        // Create Search item
        //if (_searchSize.SelectedItem == 3 || _searchSize.SelectedItem == 5)
        //    _currentSearch = nullptr;
       // else
            _currentSearch = new Search32(parameters);

        // Check for error
        if (_currentSearch->Error.pool)
        {
            delete _currentSearch;
            _currentSearch = nullptr;

            if (_searchHistory.size())
            {
                Search *search = _searchHistory.back();
                _searchHistory.pop_back();
                _currentSearch = search;
            }
            
            MessageBox("Error\n\nAn error occurred: pool alloc.")();
            return;
        }

        // Lock memory region
        _memoryRegions.IsEnabled = false;
        _startRangeTextBox.IsEnabled = false;
        _endRangeTextBox.IsEnabled = false;
        // Lock search size
        _searchSize.IsEnabled = false;

        // Enable Cancel button
        _cancelBtn.IsEnabled = true;
        // Disable Undo button
        _undoBtn.IsEnabled = false;

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

            // If we canceled first search
            if (_step == 1)
                _PopulateSearchType(false);

            // Update hits list
            _searchMenu.Update();
    }

    void    PluginMenuSearch::_resetBtn_OnClick(void)
    {
        // Clear history
        if (_searchHistory.size() > 0)
        {
            for (auto it = _searchHistory.begin(); it != _searchHistory.end(); ++it)
                delete *it;
            _searchHistory.clear();         
        }

        if (_currentSearch != nullptr)
            delete _currentSearch;

        // Reset _currentSearch
        _currentSearch = nullptr;

        // Update memory regions
        _ListRegion();

        // Unlock memory regions
        _memoryRegions.IsEnabled = true;

        // Set all memory search by default
        //_startRangeTextBox.SetValue((u32)(0x0));
        //_endRangeTextBox.SetValue((u32)(0xFFFFFFF0));
        //_startRangeTextBox.IsEnabled = false;
        //_endRangeTextBox.IsEnabled = false;


        // Unlock search size
        _searchSize.IsEnabled = true;

        // Reset step
        _step = 0;

        _resetBtn.IsEnabled = false;
        _undoBtn.IsEnabled = false;
        _searchMenu.Update();

        // Reset search type available
        _PopulateSearchType(true);

        // If Unknown search is selected
        if (_searchType.SelectedItem != 0)
        {
            _compareType.IsEnabled = false;
            _valueTextBox.IsEnabled = false;
        }

        // Delete every file in Search
        // Open current directory
        Directory dir("Search");
        std::vector<std::string> files;

        // List files
        if (dir.ListFiles(files) > 0)
        {
            for (std::string &name : files)
                File::Remove("Search/" + name);
        }
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

        if (!_searchHistory.size())
            _undoBtn.IsEnabled = false;
        _searchMenu.Update();
    }

    void    PluginMenuSearch::_ShowProgressWindow(void) const
    {
        const Color    &black = Color::Black;
        const Color    &blank = Color::Blank;
        const Color    &gainsboro = Color::Gainsboro;
        const Color    &dimGrey = Color::BlackGrey;
        const Color    &skyblue = Color::SkyBlue;
        const Color    &limegreen = Color::LimeGreen;
        static IntRect  background(125, 80, 150, 70);
        static IntRect  background2(125, 80, 150, 85);
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
        IntRect progBarFill = IntRect(131, posY + 1, std::min((u32)prog, (u32)138), 13);
        Renderer::DrawRect(progBarFill, limegreen);
        posY += 20;

        if (_inSearch)
        {
            // Draw Result count
            //std::string res = "Hit(s): " + std::to_string(_currentSearch->ResultsCount);        
            char buf[100] = { 0 };

            sprintf(buf, "Hit(s): %u", _currentSearch->ResultsCount);
            Renderer::DrawString(buf, 131, posY, blank);
        }
        if (!_inSearch)
        {
            std::string res = std::to_string(_currentSearch->ResultsCount) + " hit(s)";
            std::string res2 =  "in " + std::to_string(_currentSearch->SearchTime.AsSeconds()) + "s";            
            Renderer::DrawString((char *)res.c_str(), 131, posY, blank);
            Renderer::DrawString((char *)res2.c_str(), 131, posY, blank);
            posY -= 10;
            Renderer::DrawSysString("\uE000", 255, posY, 300, skyblue);
        }
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
        Region      bakRegion = _regionsList.size() > _memoryRegions.SelectedItem ? _regionsList[_memoryRegions.SelectedItem] : (Region){0};
        Result      ret;

        _regionsList.clear();
        _memoryRegions.Clear();

        _memoryRegions.Add("All memory");

        svcQueryProcessMemory(&meminfo, &page_info, target, 0x00100000);
        {
            Region reg = (Region){meminfo.base_addr, meminfo.base_addr + meminfo.size};
            _regionsList.push_back(reg);

            _memoryRegions.Add(Utils::Format("%08X-%08X", reg.startAddress, reg.endAddress));
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
                    _memoryRegions.Add(Utils::Format("%08X-%08X", reg.startAddress, reg.endAddress));
                    i++;
                }
            }
            
            if (meminfo.base_addr >= 0x50000000)
                break;
        }

        // Restore custom range
        if (bakRegion.startAddress && bakRegion.endAddress)
        {
            int index = 0;
            for (Region  &reg : _regionsList)
            {
                if (reg.startAddress == bakRegion.startAddress)
                {
                    _memoryRegions.SelectedItem = index;
                    _startRangeTextBox.IsEnabled = true;
                    _endRangeTextBox.IsEnabled = true;

                    if (_endRangeTextBox.Bits32 >= reg.endAddress)
                        _endRangeTextBox.SetValue(reg.endAddress);
                }
                index++;
            }                      
        }
    }

    void    PluginMenuSearch::_PopulateSearchType(bool isFirstSearch)
    {
        _compareType.Clear();

        _compareType.Add("Equal To");
        _compareType.Add("Not Equal To");
        _compareType.Add("Bigger Than");
        _compareType.Add("Bigger Or Equal");
        _compareType.Add("Smaller Than");
        _compareType.Add("Smaller Or Equal");

        if (isFirstSearch)
            return;
        _compareType.Add("Different By");
        _compareType.Add("Different By Less");
        _compareType.Add("Different By More");
    }
}
