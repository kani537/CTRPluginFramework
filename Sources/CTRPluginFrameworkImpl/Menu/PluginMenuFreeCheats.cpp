#include "CTRPluginFrameworkImpl/Menu/PluginMenuFreeCheats.hpp"
#include <string>
#include "CTRPluginFramework/Menu/Keyboard.hpp"

namespace CTRPluginFramework
{
    FreeCheats      *FreeCheats::_instance = nullptr;

    static void   GetName(std::string &name)
    {
        Keyboard    keyboard;

        keyboard.DisplayTopScreen = false;
        keyboard.CanAbort(false);
        keyboard.SetCompareCallback([](const void *in, std::string &error)
        {
            bool isValid = true;
            const std::string *input = reinterpret_cast<const std::string*>(in);

            if (input->empty())
            {
                isValid = false;
                error = "Error, name can't be empty.";
            }

            return (isValid);
        });

        keyboard.Open(name);
    }

    FreeCheats::FreeCheats(void) :
    _menu("Free cheats"),
    _addressTextBox(150, 60, 130, 15),
    _valueTextBox(150, 80, 130, 15),
    _hexBtn("Hex", *this, nullptr, IntRect(110, 80, 38, 15), nullptr),
    _saveBtn("Save", *this, nullptr, IntRect(85, 90, 66, 15)),
    _cancelBtn("Cancel", *this, nullptr, IntRect(214, 90, 66, 15)),
    _openInEditorBtn("Open in HexEditor", *this, nullptr, IntRect(85, 110, 66, 15))
    {
        if (_instance == nullptr)
            _instance = this;
        else
        {
            delete _instance;
            _instance = this;
        }

        _hexBtn.UseSysFont(false);
        _saveBtn.UseSysFont(false);
        _cancelBtn.UseSysFont(false);
        _openInEditorBtn.UseSysFont(false);
    }

    void    FreeCheats::Create(u32 address, u8 value) const
    {
        std::string name;

        GetName(name);
        _menu.Append(new MenuEntryFreeCheat(name, address, value));
    }

    void    FreeCheats::Create(u32 address, u16 value) const
    {
        std::string name;

        GetName(name);
        _menu.Append(new MenuEntryFreeCheat(name, address, value));
    }

    void    FreeCheats::Create(u32 address, u32 value) const
    {
        std::string name;

        GetName(name);
        _menu.Append(new MenuEntryFreeCheat(name, address, value));
    }

    void    FreeCheats::Create(u32 address, u64 value) const
    {
        std::string name;

        GetName(name);
        _menu.Append(new MenuEntryFreeCheat(name, address, value));
    }

    void    FreeCheats::Create(u32 address, float value) const
    {
        std::string name;

        GetName(name);
        _menu.Append(new MenuEntryFreeCheat(name, address, value));
    }

    void    FreeCheats::Create(u32 address, double value) const
    {
        std::string name;

        GetName(name);
        _menu.Append(new MenuEntryFreeCheat(name, address, value));
    }

    void    FreeCheats::Create(SavedCheat &savedCheat) const
    {
        _menu.Append(new MenuEntryFreeCheat(savedCheat));
    }

    bool    FreeCheats::operator()(EventList& eventList)
    {
        _ProcessEvent(eventList);
        _Update();

        // Draw Top
        Renderer::SetTarget(TOP);
        _menu.Draw();

        _DrawBottom();

        return (Window::BottomWindow.MustClose());
    }

    FreeCheats  *FreeCheats::GetInstance(void)
    {
        return (_instance);
    }

    void    FreeCheats::LoadFromFile(Preferences::Header &header, File &file)
    {
        if (_instance == nullptr || header.freeCheatsCount == 0)
            return;

        std::vector<SavedCheat> freecheats;
        u32     count = header.freeCheatsCount;

        file.Seek(header.freeCheatsOffset, File::SET);

        while (count > 0)
        {
            u32 nb = count > 100 ? 100 : count;
            
            freecheats.resize(nb);
            
            if (file.Read(freecheats.data(), nb * sizeof(SavedCheat)) != 0)
                break;

            for (SavedCheat &fc : freecheats)
                _instance->Create(fc);

            count -= nb;
        }
    }

    void    FreeCheats::WriteToFile(Preferences::Header &header, File &file)
    {
        if (_instance == nullptr)
            return;

        if (_instance->_menu.GetFolder()->ItemsCount())
        {
            u64             offset = file.Tell();
            MenuFolderImpl  *folder = _instance->_menu.GetFolder();
            u32             count = folder->ItemsCount();
            u32             index = 0;
            std::vector<SavedCheat> svcheats;

            while (count > 0)
            {
                u32 nb = count > 100 ? 100 : count;

                svcheats.resize(nb);

                for (int i = 0; i < nb && index < folder->ItemsCount(); ++i, ++index)
                {
                    reinterpret_cast<MenuEntryFreeCheat *>(folder->_items[index])->ToSavedSearch(svcheats[i]);
                }

                if (file.Write(svcheats.data(), nb * sizeof(SavedCheat)) != 0)
                    goto error;

                count -= nb;
            }
            header.freeCheatsCount = folder->ItemsCount();
            header.freeCheatsOffset = offset;
            error:
                return;
        }
        
    }

    void    FreeCheats::_DrawBottom(void)
    {
        Renderer::SetTarget(BOTTOM);
        Window::BottomWindow.Draw();

        // Draw TextBox
        _addressTextBox.Draw();
        _valueTextBox.Draw();

        // Draw buttons
        _hexBtn.Draw();
        _saveBtn.Draw();
        _cancelBtn.Draw();
        _openInEditorBtn.Draw();
    }

    void    FreeCheats::_ProcessEvent(EventList& eventList)
    {
        for (Event &event : eventList)
        {
            MenuItem *item = nullptr;
            MenuEvent menuevent = (MenuEvent)_menu.ProcessEvent(event, &item);

            // Process Event
        }
    }

    void    FreeCheats::_Update(void)
    {
        bool        isTouchDown = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        _addressTextBox.Update(isTouchDown, touchPos);
        _valueTextBox.Update(isTouchDown, touchPos);
        _hexBtn.Update(isTouchDown, touchPos);
        _saveBtn.Update(isTouchDown, touchPos);
        _cancelBtn.Update(isTouchDown, touchPos);
        _openInEditorBtn.Update(isTouchDown, touchPos);

        Window::BottomWindow.Update(isTouchDown, touchPos);
    }
}
