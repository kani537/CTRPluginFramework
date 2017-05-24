#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEMIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEMIMPL_HPP

namespace CTRPluginFramework
{    
    class SystemImpl
    {
    public:
        static void    Initialize(void);

        static u32     GetIOBaseLCD(void);
        static u32     GetIOBasePAD(void);
        static u32     GetIOBasePDC(void);

    private:
        friend class System;
        static bool     _isInit;
        static bool     _isNew3DS;
        static u32      _IOBaseLCD;
        static u32      _IOBasePAD;
        static u32      _IOBasePDC;
        static u8       _language;
    };
}

#endif