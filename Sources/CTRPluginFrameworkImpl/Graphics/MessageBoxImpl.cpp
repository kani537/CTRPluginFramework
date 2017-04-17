#include "CTRPluginFrameworkImpl/Graphics/MessageBoxImpl.hpp"

namespace CTRPluginFramework
{
    MessageBoxImpl::MessageBoxImpl(std::string message, DialogType dType) :
        _dialogType(dType), _message(message)
    {
    }

    bool MessageBoxImpl::operator()(void)
    {
            
        return (true);
    }
}
