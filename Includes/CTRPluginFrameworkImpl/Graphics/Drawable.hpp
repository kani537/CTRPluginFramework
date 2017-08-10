#ifndef CTRPLUGINFRAMEWORKIMPL_DRAWABLE_HPP
#define CTRPLUGINFRAMEWORKIMPL_DRAWABLE_HPP
#include "CTRPluginFramework/System/Vector.hpp"

namespace CTRPluginFramework
{
    class Drawable
    {
    public:
        Drawable() {}
        virtual ~Drawable(){}

        virtual void Draw(void) = 0;
        virtual void Update(bool isTouchDown, IntVector touchPos) = 0;
    };
}

#endif