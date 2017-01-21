#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{
    #define ABS(x) (x >= 0 ? x : -x)

    EventManager::EventManager(void)
    {

    }

    bool    EventManager::PollEvent(Event &event)
    {
        if (PopEvent(event, false))
            return (true);
        return (false);
    }

    bool    EventManager::WaitEvent(Event &event)
    {
        if (PopEvent(event, true))
            return (true);
        return (false);
    }

    bool    EventManager::PopEvent(Event &event, bool isBlocking)
    {
        if (_eventsQueue.empty())
        {
            ProcessEvents();
            if (isBlocking)
            {
                while (_eventsQueue.empty())
                {
                    Sleep(Milliseconds(10));
                    ProcessEvents();
                }
            }
        }

        if (!_eventsQueue.empty())
        {
            event = _eventsQueue.front();
            _eventsQueue.pop();
            return (true);
        }
        return (false);
    }

    void    EventManager::PushEvent(const Event &event)
    {
        _eventsQueue.push(event);
    }

    void    EventManager::ProcessEvents(void)
    {
        Event   event;
        int     i;

        
        Controller::Update();

        // Key Event
        for (int i = 0; i < 32; i++)
        {
            Key code = static_cast<Key>(1 << i);
            if (Controller::IsKeyPressed(code))
            {
                event.type = Event::KeyPressed;
                event.key.code = code;
                PushEvent(event);
            }
            if (Controller::IsKeyReleased(code))
            {
                event.type = Event::KeyReleased;
                event.key.code = code;
            }
        }

        // Touch Event
        static bool  isTouching = false;        
        static int horizontalOffset = 0;
        static int verticalOffset = 0;

        touchPosition touchPos;
        hidTouchRead(&touchPos);

        if (Controller::IsKeyDown(Key::Touchpad))
        {
            if (touchPos.px != _lastTouch.px 
            || touchPos.py != _lastTouch.py 
            || !isTouching)
            {
                _lastTouch = touchPos;
                if (isTouching)
                {
                    event.type = Event::TouchMoved;
                    if (touchPos.px > _lastTouch.px)
                        horizontalOffset += (touchPos.px - _lastTouch.px);
                    else
                        horizontalOffset -= (_lastTouch.px - touchPos.px);
                    if (touchPos.py > _lastTouch.py)
                        verticalOffset += (touchPos.py - _lastTouch.py);
                    else
                        verticalOffset -= (_lastTouch.py - touchPos.py);
                }
                else
                {
                    event.type = Event::TouchBegan;
                    horizontalOffset = 0;
                    verticalOffset = 0;
                }
                isTouching = true;
                event.touch.x = touchPos.px;
                event.touch.y = touchPos.py;
                PushEvent(event);
                _lastTouch = touchPos;
            }
        }
        else if (isTouching)
        {
            isTouching = false;
            event.type = Event::TouchEnded;
            event.touch.x = _lastTouch.px;
            event.touch.y = _lastTouch.py;
            PushEvent(event);

            Event::SwipDirection swip = Event::None;
            if (ABS(horizontalOffset) > 10 || ABS(verticalOffset) > 10)
            {
                if (horizontalOffset > 0 && verticalOffset == 0)
                    swip = Event::Right;
                else if (horizontalOffset < 0 && verticalOffset == 0)
                    swip = Event::Left;
                else if (verticalOffset > 0 && horizontalOffset == 0)
                    swip = Event::Up;
                else if (verticalOffset < 0 && horizontalOffset == 0)
                    swip = Event::Down;
                else if (horizontalOffset < 0 && verticalOffset > 0)
                    swip = Event::LeftUp;
                else if (horizontalOffset > 0 && verticalOffset > 0)
                    swip = Event::RightUp;
                else if (horizontalOffset < 0 && verticalOffset < 0)
                    swip = Event::LeftDown;
                else if (horizontalOffset > 0 && verticalOffset < 0)
                    swip = Event::RightDown;
            }
            if (swip != Event::None)
            {
                event.type = Event::TouchSwipped;
                event.swip.direction = swip;
                PushEvent(event);
            }
        }
    }

}