#ifndef CTRPLUGINFRAMEWORKIMPL_ACTIONREPLAY_ARCODEEDITOR
#define CTRPLUGINFRAMEWORKIMPL_ACTIONREPLAY_ARCODEEDITOR

#include "types.h"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFrameworkImpl/Menu/KeyboardImpl.hpp"
#include "ARCode.hpp"

namespace CTRPluginFramework
{
    class ARCodeEditor
    {
        using EventList = std::vector<Event>;
    public:
        struct CodeLine
        {
            enum
            {
                Empty = 1,
                Error = 1 << 2,
                Modified = 1 << 3,
                PatchData = 1 << 4
            };

            explicit CodeLine(ARCode &code);

            ARCode&         base;
            ARCode*         parent;
            u32             flags;
            std::string     display;

            void    Edit(u32 index, u32 value);
            void    Update(void);
        };

        ARCodeEditor(void);
        ~ARCodeEditor(void) {}

        // Return true if the user decided to close it
        bool    operator()(EventList &eventList);

        static void Edit(ARCodeContext &ctx);
    private:

        void    _ProcessEvent(Event &event);
        void    _DrawSubMenu();
        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _Update(void);

        bool                        _exit;
        int                         _line;
        ARCodeContext               *_context;
        KeyboardImpl                _keyboard;
        std::vector<CodeLine>       _codes;
    };
}

#endif
