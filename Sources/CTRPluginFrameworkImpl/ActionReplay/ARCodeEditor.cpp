#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCodeEditor.hpp"
#include "CTRPluginFramework/Utils.hpp"

#define PATCH_COLOR Color::Grey
#define TYPE_COLOR Color::Orange
#define IMMEDIATE_COLOR Color::Cyan
#define CONTROL_COLOR Color::Red

namespace CTRPluginFramework
{
    static const char       *__emptyCode = "00000000 00000000";
    static ARCodeEditor     *__arCodeEditor = nullptr;

#define IsEmpty (flags & ARCodeEditor::CodeLine::Empty)
#define IsError (flags & ARCodeEditor::CodeLine::Empty)
#define IsModified (flags & ARCodeEditor::CodeLine::Modified)
#define IsData (flags & ARCodeEditor::CodeLine::PatchData)
#define Clear(bit) (flags &= ~bit)
#define Set(bit) (flags |= bit)


   /* static bool     CodeToString(std::string &out, ARCodeEditor::CodeLine &code)
    {
        bool    error = false;

        out.clear();

        // If codes are patchdata, no need to check anything
        if (IsData(left.flags))
        {
            out = PATCH_COLOR << Utils::ToHex(left.raw) << " " << Utils::ToHex(right.raw);
        }
        // Else analyze codes and check for errors
        {

        }

        return error;
    }*/

    ARCodeEditor::CodeLine::CodeLine(ARCode &code) :
        base(code), parent(nullptr), flags(0)
    {
        if (code.Text.empty())
            code.Text = code.ToString();
        Set(Modified);
    }

    void    ARCodeEditor::CodeLine::Edit(u32 index, u32 value)
    {
        base.Text[index] = (value & 0xF) + '0';
        Set(Modified);
    }

    void    ARCodeEditor::CodeLine::Update(void)
    {
        if (IsModified)
        {
            bool error = base.Update(base.Text);

            display = base.Text; ///< TODO : Analyze and color

            Clear(Modified);
        }
    }

    /*
     * Editor
     */
    ARCodeEditor::ARCodeEditor(void)
    {
        _exit = false;
        _line = 0;
        _keyboard.SetLayout(Layout::HEXADECIMAL);
        _keyboard._Hexadecimal();
        __arCodeEditor = this;
    }

    bool    ARCodeEditor::operator()(EventList &eventList)
    {
        // Process event
        for (Event &event : eventList)
            _ProcessEvent(event);

        // Update states
        _Update();

        // Draw top screen
        _RenderTop();

        // Draw bottom screen
        _RenderBottom();

        return _exit;
    }

    void    ARCodeEditor::Edit(ARCodeContext &ctx)
    {
        if (!__arCodeEditor)
            return;

        ARCodeEditor &editor = *__arCodeEditor;

        editor._context = &ctx;
        editor._exit = false;
        editor._line = 0;
        editor._codes.clear();
        for (ARCode &code : ctx.codes)
            editor._codes.push_back(CodeLine(code));

        Event           event;
        EventList       eventList;
        EventManager    manager;
        bool            exit = false;

        do
        {
            // Fetch all events
            while (manager.PollEvent(event))
                eventList.push_back(event);

            // Execute Editor's loop
            exit = editor(eventList);

            // Swap screens
            Renderer::EndFrame();

        } while (!exit);
    }

    void    ARCodeEditor::_ProcessEvent(Event &event)
    {
        if (event.type == Event::KeyPressed && event.key.code == Key::B)
            _exit = true;
    }

    void    ARCodeEditor::_DrawSubMenu(void)
    {
    }

    void    ARCodeEditor::_RenderTop(void)
    {
        Renderer::SetTarget(TOP);
        Window::TopWindow.Draw("Code Editor");

        // Column headers

        // First column: 31px : 3*7 + 10 => Line
        // Second column: 112px : (17* 6) + 10 => Code
        // Last column: 167px : 157 + 10 => Comment
        // Total: 312px :  31 + 1 + 112 + 1 + 167
        // Margin left / right: 14px

        int   posY = 61;
        // Line header
        Renderer::DrawRect(44, posY, 31, 20, Color::SkyBlue);

        // Code header
        Renderer::DrawRect(76, posY, 112, 20, Color::DeepSkyBlue);

        // Comment header
        Renderer::DrawRect(189, posY, 167, 20, Color::DeepSkyBlue);

        posY += 21;

        // Line body
        Renderer::DrawRect(44, posY, 31, 100, Color::DeepSkyBlue);
        // Code body
        Renderer::DrawRect(76, posY, 112, 100, Color::Blank);
        // Comment body
        Renderer::DrawRect(189, posY, 167, 100, Color::SkyBlue);

        // Draw Selector

        // Draw codes
        int posX = 81;
        posY += 2;

        {
            int max = std::min(_line + 10, (int)_codes.size());

            for (int i = _line; i < max; ++i)
                Renderer::DrawString(_codes[i].display.c_str(), posX, posY, Color::Black);
        }

        // Draw comments ?
    }

    void    ARCodeEditor::_RenderBottom(void)
    {
        Renderer::SetTarget(BOTTOM);
        _keyboard._RenderBottom();
    }

    void    ARCodeEditor::_Update(void)
    {
        for (CodeLine &code : _codes)
            code.Update();
    }
}
