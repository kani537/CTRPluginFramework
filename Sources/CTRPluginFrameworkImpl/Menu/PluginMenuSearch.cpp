#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearch.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    PluginMenuSearch::PluginMenuSearch() :
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
    _memoryRegions(150, 50, 130, 15),
    _searchSize(150, 70, 130, 15),
    _searchType(150, 90, 130, 15),
    _compareType(150, 110, 130, 15),
    _alignmentTextBox(150, 130, 130, 15),
    _valueTextBox(150, 150, 130, 15),
    _searchBtn("Search", *this, &PluginMenuSearch::_searchBtn_OnClick, IntRect(35, 195, 80, 15)),
    _undoBtn("Undo", *this, nullptr, IntRect(120, 195, 80, 15)),
    _resetBtn("Reset", *this, nullptr, IntRect(205, 195, 80, 15)),
    _inSearch(false),
    _firstRegionInit(false)
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

        // Init buttons
        _searchBtn.UseSysFont(false);
        _undoBtn.UseSysFont(false);
        _resetBtn.UseSysFont(false);

        // Set 4 Bytes as default
        u32   al = 4;
        _alignmentTextBox.SetValue(al);
        _searchSize.SelectedItem = 2;
    }

    bool    PluginMenuSearch::operator()(EventList &eventList, Time &delta)
    {
        // If we're on first search
        if (!_firstRegionInit)
        {
            _ListRegion();
            _firstRegionInit = true;
        }

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

        _searchBtn();
        _undoBtn();
        _resetBtn();

        // Check ComboBox

        _memoryRegions();

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

        if (_inSearch)
        {
            _ShowProgressWindow();
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

        if (_inSearch)
            return;

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
        Renderer::DrawString((char *)"MemRegion:", textPosX, posY, blank);
        posY = 72;

        // Value Type
        Renderer::DrawString((char *)"Value Type:", textPosX, posY, blank);
        posY = 92;

        // Search Type
        Renderer::DrawString((char *)"Search Type:", textPosX, posY, blank);
        posY = 112;

        // Scan Type
        Renderer::DrawString((char *)"Scan Type:", textPosX, posY, blank);
        posY = 132;

        // Alignment
        Renderer::DrawString((char *)"Alignment:", textPosX, posY, blank);
        posY = 152;

        // Value
        Renderer::DrawString((char *)"Value:", textPosX, posY, blank);
        posY = 172;


        // Draw ComboBoxes
        _memoryRegions.Draw();
        _searchSize.Draw();
        _searchType.Draw();
        _compareType.Draw();

        // Draw NumericTextBoxes
        _alignmentTextBox.Draw();
        _valueTextBox.Draw();

        // Draw buttons
        _closeBtn.Draw();
        _searchBtn.Draw();
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

        // Update buttons
        _closeBtn.Update(isTouched, touchPos);
        _searchBtn.Update(isTouched, touchPos);
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

        if (_memoryRegions.SelectedItem == -1 || _memoryRegions.SelectedItem >= _regionsList.size())
            return;

        u32     startRange = _regionsList[_memoryRegions.SelectedItem].startAddress;
        u32     endRange = _regionsList[_memoryRegions.SelectedItem].endAddress;
        u32     step = _searchHistory.size();



        // Create search object and set SearchSize
        switch (_searchSize.SelectedItem)
        {
            case 0: _currentSearch = new Search<u8>(_valueTextBox.Bits8, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch);  _currentSearch->Size = SearchSize::Bits8; break;
            case 1: _currentSearch = new Search<u16>(_valueTextBox.Bits16, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits16; break;
            case 2: _currentSearch = new Search<u32>(_valueTextBox.Bits32, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits32; break;
            case 3: _currentSearch = new Search<u64>(_valueTextBox.Bits64, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Bits64; break;
            case 4: _currentSearch = new Search<float>(_valueTextBox.Single, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::FloatingPoint; break;
            case 5: _currentSearch = new Search<double>(_valueTextBox.Double, startRange, endRange, _alignmentTextBox.Bits32, _currentSearch); _currentSearch->Size = SearchSize::Double; break;
        }

        // Set Search Type
        _currentSearch->Type = static_cast<SearchType>(_searchType.SelectedItem);

        // Set CompareType
        _currentSearch->Compare = static_cast<CompareType>(_compareType.SelectedItem);

        // Lock memoy region
        _memoryRegions.IsEnabled = false;

        _inSearch = true;
    }

    void    PluginMenuSearch::_ShowProgressWindow(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    gainsboro(225, 225, 225);
        static Color    dimGrey(15, 15, 15);
        static Color    silver(160, 160, 160);
        static IntRect  background(125, 80, 150, 70);
        static Color    skyblue(0, 191, 255);
        static Color    limegreen(50, 205, 50);
        //static Clock    timer;
        static int      phase = 0;

        std::string     waitLogo[] = 
        {
            "\uE020", "\uE021", "\uE022", "\uE023", "\uE024", "\uE025", "\uE026", "\uE027"
        };

        // Draw "window" background
        Renderer::DrawRect2(background, black, dimGrey);

        int posY = 90;

        // Draw logobackground
       // Renderer::DrawSysString("\uE021", 192, posY, 300, silver); //Top
       // posY = 85;
       // Renderer::DrawSysString("\uE025", 192, posY, 300, silver); //Bottom
       // posY = 85;

        // Draw logo phase
        Renderer::DrawSysString(waitLogo[phase].c_str(), 192, posY, 300, skyblue);

        phase++;
        if (phase > 7) phase = 0;

        posY += 10;

        // Progressbar
        // Draw border
        IntRect progBarBorder = IntRect(130, posY, 140, 15);
        Renderer::DrawRect(progBarBorder, gainsboro, false);

        float percent = 138.f / 100.f;
        float prog = _currentSearch->Progress * percent;      

        // Draw progress fill
        IntRect progBarFill = IntRect(131, posY + 1, (u32)prog, 13);
        Renderer::DrawRect(progBarFill, limegreen);
        // Draw Result count
        std::string res = "Result(s): " + std::to_string(_currentSearch->ResultCount);
        posY += 20;
        Renderer::DrawString((char *)res.c_str(), 131, posY, blank);
    }

    void    PluginMenuSearch::_ListRegion(void)
    {

        Handle      target = Process::GetHandle();
        PageInfo    page_info;
        MemInfo     meminfo;
        u32         save_addr;
        int         i;
        Result      ret;

        _regionsList.clear();
        _memoryRegions.Clear();

        svcQueryProcessMemory(&meminfo, &page_info, target, 0x00100000);
        save_addr = meminfo.base_addr + meminfo.size + 1;
        i = 1;
        while (save_addr < 0x50000000)
        {
            if (i >= 99)
                break;
            ret = svcQueryProcessMemory(&meminfo, &page_info, target, save_addr);//, "svc_queryProcessMemory");
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
            if (meminfo.state != 0x0 && meminfo.state != 0x2 && meminfo.state != 0x3 && meminfo.state != 0x6)
            {
                if (meminfo.perm & MEMPERM_READ)
                {
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