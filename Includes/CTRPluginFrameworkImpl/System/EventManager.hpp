#ifndef CTRPLUGINFRAMEWORK_EVENTMANAGER_HPP
#define CTRPLUGINFRAMEWORK_EVENTMANAGER_HPP

#include "ctrulib/services/hid.h"
#include <queue>

namespace CTRPluginFramework
{
    class Event;
    class EventManager
    {
    public:
        EventManager(void);

        bool PollEvent(Event &event);
        bool WaitEvent(Event &event);

    private:

        bool PopEvent(Event &event, bool isBlocking);
        void PushEvent(const Event &event);
        void ProcessEvents(void);

        std::queue<Event>   _eventsQueue;
        touchPosition       _lastTouch;
        float               _slider3D;
        u32                 _keysHeld;

    };
}

#endif