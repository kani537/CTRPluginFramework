#include "CTRPluginFrameworkImpl/Menu/PluginMenuActionReplay.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/MenuEntryActionReplay.hpp"
#include "CTRPluginFramework/Utils.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCodeEditor.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"

namespace CTRPluginFramework
{
    static PluginMenuActionReplay *__pmARinstance = nullptr;
    PluginMenuActionReplay::PluginMenuActionReplay() :
        _topMenu{ "ActionReplay" },
        _noteBtn(*this, nullptr, IntRect(90, 30, 25, 25), Icon::DrawInfo, false),
        _editorBtn(*this, &PluginMenuActionReplay::_EditorBtn_OnClick, IntRect(130, 30, 25, 25), Icon::DrawEdit, false),
        _newBtn(*this, &PluginMenuActionReplay::_NewBtn_OnClick, IntRect(165, 30, 25, 25), Icon::DrawPlus, false),
        _cutBtn(*this, &PluginMenuActionReplay::_CutBtn_OnClick, IntRect(200, 30, 25, 25), Icon::DrawCut, false),
        _pasteBtn(*this, &PluginMenuActionReplay::_PasteBtn_OnClick, IntRect(200, 30, 25, 25), Icon::DrawClipboard, false),
        _duplicateBtn(*this, &PluginMenuActionReplay::_DuplicateBtn_OnClick, IntRect(235, 30, 25, 25), Icon::DrawDuplicate, false),
        _trashBtn(*this, &PluginMenuActionReplay::_TrashBtn_OnClick, IntRect(50, 30, 25, 25), Icon::DrawTrash, false),
        _clipboard{ nullptr }
    {
        _newBtn.Enable(true);
        __pmARinstance = this;
    }

    PluginMenuActionReplay::~PluginMenuActionReplay()
    {
        if (_clipboard)
            delete _clipboard;
    }

    void    PluginMenuActionReplay::Initialize(void)
    {
        ActionReplay_LoadCodes(_topMenu.GetFolder());
    }

    bool    PluginMenuActionReplay::operator()(EventList &eventList, const Time &delta)
    {
        // Process events
        _ProcessEvent(eventList);

        // Update components
        _Update(delta);

        // Check if note state must be changed
        if (_noteBtn())
        {
            if (!_topMenu.IsNoteOpen())
            {
                if (!_topMenu.ShowNote())
                    _noteBtn.Enable(false);
            }
            else
                _topMenu.CloseNote();
        }

        // Check editor btn
        _editorBtn();
        _newBtn();
        _cutBtn();
        _pasteBtn();
        _duplicateBtn();
        _trashBtn();

        // Draw menu on top screen
        _topMenu.Draw();

        // Draw bottom screen
        _DrawBottom();

        // Return if user want to close the window
        return (Window::BottomWindow.MustClose());
    }

    void    PluginMenuActionReplay::_DrawBottom(void)
    {
        Renderer::SetTarget(BOTTOM);
        Window::BottomWindow.Draw();

        _noteBtn.Draw();
        _editorBtn.Draw();
        _newBtn.Draw();
        _cutBtn.Draw();
        _pasteBtn.Draw();
        _duplicateBtn.Draw();
        _trashBtn.Draw();
    }

    void    PluginMenuActionReplay::_ProcessEvent(EventList &eventList)
    {
        for (Event &event : eventList)
        {
            // Process top menu's event
            MenuItem *item = nullptr;
            int action = _topMenu.ProcessEvent(event, &item);

            // If user want to exit the current menu
            if (action == MenuEvent::MenuClose)
                Window::BottomWindow.Close();
        }
    }

    void    PluginMenuActionReplay::_Update(const Time &delta)
    {
        _topMenu.Update(delta);
        Window::BottomWindow.Update(Touch::IsDown(), IntVector(Touch::GetPosition()));

        bool        touchIsDown = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());
        MenuItem    *item = _topMenu.GetSelectedItem();

        _noteBtn.Enable(item && !item->note.empty());
        _noteBtn.SetState(_topMenu.IsNoteOpen());
        _noteBtn.Update(touchIsDown, touchPos);

        _editorBtn.Enable(item != nullptr);
        _editorBtn.Update(touchIsDown, touchPos);

        _cutBtn.Enable(item != nullptr && _clipboard == nullptr);
        _pasteBtn.Enable(_clipboard != nullptr);
        _duplicateBtn.Enable(item != nullptr && !item->IsFolder());
        _trashBtn.Enable(item != nullptr);

        _newBtn.Update(touchIsDown, touchPos);
        _cutBtn.Update(touchIsDown, touchPos);
        _pasteBtn.Update(touchIsDown, touchPos);
        _duplicateBtn.Update(touchIsDown, touchPos);
        _trashBtn.Update(touchIsDown, touchPos);
    }

    static bool ActionReplay_GetInput(std::string &ret)
    {
        Keyboard    keyboard;

        keyboard.SetCompareCallback([](const void *in, std::string &error)
        {
            std::string &input = *(std::string *)(in);
            if (input.empty())
                return false;
            ActionReplay_ProcessString(input, false);
            return true;
        });

        return keyboard.Open(ret, ret) != -1;
    }

    void    PluginMenuActionReplay::_EditorBtn_OnClick(void)
    {
        MenuItem    *item = _topMenu.GetSelectedItem();
        Keyboard   optKbd;
        std::vector<std::string>    options = { "Name", "Note" };

        if (!item)
            return;
        if (!item->IsFolder())
            options.push_back("Code");

        optKbd.Populate(options);
        int choice = optKbd.Open();
        if (choice >= 0)
        {
            // Name edition
            if (choice == 0)
            {
                ActionReplay_GetInput(item->name);
            }
            // Note edition
            else if (choice == 1)
            {
                ActionReplay_GetInput(item->note);
                ActionReplay_ProcessString(item->note);
            }
            // Code edition
            else if (choice == 2)
            {
                MenuEntryActionReplay *e = reinterpret_cast<MenuEntryActionReplay *>(item);

                e->Disable();
                // Edit code
                ARCodeEditor::Edit(e->context);
                e->context.Update();
            }
        }
    }

    void    PluginMenuActionReplay::_NewBtn_OnClick(void)
    {
        Keyboard    kbd("", {"Code", "Folder"});

        int choice = kbd.Open();

        if (choice == -1)
            return;

        std::string name;

        if (!ActionReplay_GetInput(name))
            return;

        // Create a new code
        if (choice == 0)
        {
            MenuEntryActionReplay *entry = new MenuEntryActionReplay(name);
            _topMenu.Insert(entry);
        }

        // Create a new folder
        if (choice == 1)
        {
            MenuFolderImpl *folder = new MenuFolderImpl(name);
            _topMenu.Insert(folder);
        }
    }

    void    PluginMenuActionReplay::_CutBtn_OnClick(void)
    {
        // If clipboard already exists, abort
        if (_clipboard)
            return;
        _clipboard = _topMenu.Pop();
    }

    void    PluginMenuActionReplay::_PasteBtn_OnClick(void)
    {
        if (!_clipboard)
            return;
        _topMenu.Insert(_clipboard);
        _clipboard = nullptr;
    }

    void    PluginMenuActionReplay::_DuplicateBtn_OnClick(void)
    {
        MenuItem *current = _topMenu.GetSelectedItem();

        if (!current || current->IsFolder())
            return;

        MenuEntryActionReplay *old = reinterpret_cast<MenuEntryActionReplay *>(current);
        MenuEntryActionReplay *dup = new MenuEntryActionReplay(current->name, current->note);

        dup->context = old->context;
        _topMenu.Insert(dup);
    }

    void    PluginMenuActionReplay::_TrashBtn_OnClick(void)
    {
        MenuItem *item = _topMenu.GetSelectedItem();

        if (item == nullptr)
            return;

        if (!(MessageBox(Color::Orange << "Warning", "Do you really want to delete: " << item->name, DialogType::DialogYesNo )()))
            return;

        item = _topMenu.Pop();

        delete item;
    }

    void    PluginMenuActionReplay::SaveCodes(void)
    {
        if (!__pmARinstance)
            return;

        File        file;
        LineWriter  writer(file);

        ActionReplay_OpenCheatsFile(file, true);

        if (!file.IsOpen())
            return;

        MenuFolderImpl *folder = __pmARinstance->_topMenu.GetRootFolder();

        if (!folder) return;

        MenuFolderImpl &f = *folder;
        for (u32 i = 0; i < f.ItemsCount(); i++)
            ActionReplay_WriteToFile(writer, f[i]);

        writer.Close();
    }

    void    PluginMenuActionReplay::NewARCode(u8 type, u32 address, u32 value)
    {
        if (!__pmARinstance)
            return;

        std::string name;

        if (!ActionReplay_GetInput(name))
            return;

        MenuEntryActionReplay *ar = new MenuEntryActionReplay(name);

        u32 offset = address & 0xFF000000;
        address &= 0xFFFFFF;
        ar->context.codes.push_back(ARCode(0xD3, 0, offset));
        ar->context.codes.push_back(ARCode(type, address, value));
        ar->context.codes.push_back(ARCode(0xD2, 0, 0));

        ar->context.Update();
        MenuFolderImpl *f = __pmARinstance->_topMenu.GetRootFolder();

        if (f)
            f->Append(ar);
        ARCodeEditor::Edit(ar->context);
        ar->context.Update();
    }
}
