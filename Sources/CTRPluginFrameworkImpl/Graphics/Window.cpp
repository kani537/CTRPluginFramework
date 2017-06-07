#include "CTRPluginFrameworkImpl/Graphics/Window.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"

namespace CTRPluginFramework
{
    Window  Window::BottomWindow = Window(20, 20, 280, 200, true, nullptr);
    Window  Window::TopWindow = Window(30, 20, 340, 200, false, nullptr);

    Window::Window(u32 posX, u32 posY, u32 width, u32 height, bool closeBtn, BMPImage *image) :
    _rect(posX, posY, width, height),
    _border(posX + 2, posY + 2, width - 4, height - 4),
    _image(image)
    {
        if (closeBtn)
            _closeBtn = new IconButton<Window, void>(*this, nullptr, IntRect(posX + width - 25, posY + 4, 20, 20), Icon::DrawClose);
        else
            _closeBtn = nullptr;
    }

    Window::~Window(void)
    {
        delete _closeBtn;
        _closeBtn = nullptr;
    }

    void    Window::Draw(void) const
    {
        // Background
        if (_image->IsLoaded())
            _image->Draw(_rect.leftTop);
        else
        {
            Renderer::DrawRect2(_rect, Color::Black, Color::BlackGrey);
            Renderer::DrawRect(_border, Color::Blank, false);
        }

        // Close button
        if (_closeBtn != nullptr)
            _closeBtn->Draw();
    }

    void    Window::Draw(const std::string& title) const
    {
        // Background
        if (_image->IsLoaded())
            _image->Draw(_rect.leftTop);
        else
        {
            Renderer::DrawRect2(_rect, Color::Black, Color::BlackGrey);
            Renderer::DrawRect(_border, Color::Blank, false);
        }

        // Title
        int posY = _rect.leftTop.y + 5;
        int xx = Renderer::DrawSysString(title.c_str(), _rect.leftTop.x + 10, posY, 330, Color::Blank);
        Renderer::DrawLine(40, posY, xx, Color::Blank);

        // Close button
        if (_closeBtn != nullptr)
            _closeBtn->Draw();
    }

    void    Window::Update(bool isTouched, IntVector touchPos) const
    {
        if (_closeBtn != nullptr)
            _closeBtn->Update(isTouched, touchPos);
    }

    bool    Window::MustClose(void) const
    {
        if (_closeBtn == nullptr)
            return (false);

        return ((*_closeBtn)());
    }

    void    Window::Initialize(void)
    {
        BottomWindow._image = Preferences::bottomBackgroundImage;
        TopWindow._image = Preferences::topBackgroundImage;
    }
}
