#ifndef CTRPLUGINFRAMEWORKIMPL_DRAWABLE_HPP
#define CTRPLUGINFRAMEWORKIMPL_DRAWABLE_HPP
#include "CTRPluginFramework/System/Vector.hpp"

namespace CTRPluginFramework
{
    class Drawable
    {
    public:
        virtual ~Drawable(){}

        virtual void Draw(void) = 0;
        virtual bool operator()(void){}
        virtual void Update(bool isTouchDown, IntVector touchPos) = 0;
    };
}

#endif