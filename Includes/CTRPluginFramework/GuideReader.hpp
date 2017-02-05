#ifndef CTRPLUGINFRAMEWORK_GUIDEREADER_HPP
#define CTRPLUGINFRAMEWORK_GUIDEREADER_HPP

namespace CTRPluginFramework
{
    class GuideReader
    {
    public:
        GuideReader(void);
        ~GuideReader(void);
        void    Draw(void);
        int     ProcessEvent(Event &event);

    private:
        bool            _isOpen;
        Menu            _menu;
        TextBox         _guideTB;
        std::string     _text;
    };
}

#endif