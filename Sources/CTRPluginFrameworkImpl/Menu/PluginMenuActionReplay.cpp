#include "CTRPluginFrameworkImpl/Menu/PluginMenuActionReplay.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/MenuEntryActionReplay.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"

namespace CTRPluginFramework
{
    PluginMenuActionReplay::PluginMenuActionReplay() :
        _topMenu{ "ActionReplay" }
    {
    }

    PluginMenuActionReplay::~PluginMenuActionReplay()
    {
    }

    void    PluginMenuActionReplay::Initialize(void)
    {
        ActionReplay_LoadCodes(_topMenu.GetFolder());
    }

    bool    PluginMenuActionReplay::operator()(EventList &eventList)
    {
        // Process events
        _ProcessEvent(eventList);

        // Update components
        _Update();

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

        MenuItem *item = _topMenu.GetSelectedItem();

        if (!item || item->IsFolder())
            return;

        MenuEntryActionReplay *ar = reinterpret_cast<MenuEntryActionReplay *>(item);

        int posY = 30;

        if (!ar)
        {
            Renderer::DrawString("nullptr", 30, posY, Color::Blank);
            return;
        }

        if (!ar->note.empty())
            Renderer::DrawSysStringReturn((u8 *)ar->note.c_str(), 30, posY, 230, Color::Blank);

        if (ar->context.hasError)
        {
            Renderer::DrawString(Utils::Format("error, %d", ar->context.codes.size()).c_str(), 30, posY, Color::Blank);
            Renderer::DrawSysStringReturn((u8 *)ar->context.data.c_str(), 30, posY, 0, Color::Blank);
        }

        if (ar->context.codes.empty())
            Renderer::DrawString("empty", 30, posY, Color::Blank);

        ARCodeVector &codes = ar->context.codes;
        int max = std::min(15, (int)codes.size());
        for (int i = 0; i < max; ++i)
            Renderer::DrawString(codes[i].ToString().c_str(), 30, posY, Color::Blank);
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

    void    PluginMenuActionReplay::_Update(void)
    {
        Window::BottomWindow.Update(Touch::IsDown(), IntVector(Touch::GetPosition()));
    }
}
