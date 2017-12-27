#include "CTRPluginFrameworkImpl/Menu/PluginMenuActionReplay.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/MenuEntryActionReplay.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"

namespace CTRPluginFramework
{
    PluginMenuActionReplay::PluginMenuActionReplay() :
        _topMenu{ "ActionReplay" },
        _noteBtn(*this, nullptr, IntRect(90, 30, 25, 25), Icon::DrawInfo, false),
        _editorBtn(*this, &PluginMenuActionReplay::_EditorBtn_OnClick, IntRect(130, 30, 25, 25), Icon::DrawEdit, false)
    {
    }

    PluginMenuActionReplay::~PluginMenuActionReplay()
    {
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
    }

    void    PluginMenuActionReplay::_EditorBtn_OnClick(void)
    {
        MenuItem    *item = _topMenu.GetSelectedItem();
        Keyboard    option{ "", {"name", "note", "code"} };
        Keyboard    strKbd;

        if (!item)
            return;

        int choice = option.Open();
        if (choice >= 0)
        {
            // Name edition
            if (choice == 0)
                strKbd.Open(item->name, item->name);
            // Note edition
            else if (choice == 1)
                strKbd.Open(item->note, item->note);
            // Code edition
            else if (choice == 2)
            {
                // edit code
            }

        }
    }
}