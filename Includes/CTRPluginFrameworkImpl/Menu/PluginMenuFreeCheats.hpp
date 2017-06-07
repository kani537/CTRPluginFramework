#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUFREECHEATS_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUFREECHEATS_HPP

#include "types.h"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryFreeCheat.hpp"

namespace CTRPluginFramework
{
    class FreeCheats
    {
        using EventList = std::vector<Event>;
        using SavedCheat = Preferences::SavedCheats;
    public:
        FreeCheats(void);
        ~FreeCheats(void) { _instance = nullptr; }

        void    Create(u32 address, u8 value) const;
        void    Create(u32 address, u16 value) const;
        void    Create(u32 address, u32 value) const;
        void    Create(u32 address, u64 value) const;
        void    Create(u32 address, float value) const;
        void    Create(u32 address, double value) const;
        void    Create(SavedCheat &savedCheat) const;

        // Display menu
        // Return true if menu must close
        bool operator()(EventList &eventList);

        static FreeCheats   *GetInstance(void);
        
        static void     LoadFromFile(Preferences::Header &header, File &file);
        static void     WriteToFile(Preferences::Header& header, File &file);
    private:
        Menu                                _menu;
        NumericTextBox                      _addressTextBox;
        NumericTextBox                      _valueTextBox;
        CheckedButton<FreeCheats, void>     _hexBtn;
        Button<FreeCheats, void>            _saveBtn;
        Button<FreeCheats, void>            _cancelBtn;
        Button<FreeCheats, void>            _openInEditorBtn;
        
        MenuEntryFreeCheat                  *_selectedFC;

        static FreeCheats                   *_instance;

        void    _DrawBottom(void);
        void    _ProcessEvent(EventList &eventList);
        void    _Update(void);
    };
}

#endif