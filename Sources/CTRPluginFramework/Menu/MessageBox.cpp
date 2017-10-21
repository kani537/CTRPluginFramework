#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFrameworkImpl/Graphics/MessageBoxImpl.hpp"

namespace CTRPluginFramework
{
    MessageBox::MessageBox(const std::string &title, const std::string &message, DialogType dialogType) :
        _messageBox(new MessageBoxImpl(title, message, dialogType))
    {
    }

    MessageBox::MessageBox(const std::string &message, DialogType dialogType) :
        _messageBox(new MessageBoxImpl(message, dialogType))
    {
        
    }

    MessageBox::~MessageBox(void)
    {
        
    }

    bool MessageBox::operator()(void) const
    {
        return ((*_messageBox)());
    }
}