#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFrameworkImpl/Graphics/MessageBoxImpl.hpp"

namespace CTRPluginFramework
{
    MessageBox::MessageBox(std::string message, DialogType dialogType) :
    _messageBox(new MessageBoxImpl(message, dialogType))
    {
        
    }

    MessageBox::~MessageBox(void)
    {
        
    }

    bool MessageBox::operator()(void)
    {
        return ((*_messageBox)());
    }



}