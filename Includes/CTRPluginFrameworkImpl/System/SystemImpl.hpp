#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEMIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEMIMPL_HPP

namespace CTRPluginFramework
{
    class SystemImpl
    {
    public:
        static void     Initialize(void);

        static bool     IsNew3DS;
        static bool     IsLoaderNTR;
        static u32      IoBaseLCD;
        static u32      IoBasePAD;
        static u32      IoBasePDC;
        static u32      CFWVersion;
        static u32      RosalinaHotkey;
        static u8       Language;
    };
}

#endif
