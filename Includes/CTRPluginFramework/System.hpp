#ifndef CTRPLUGINFRAMEWORK_SYSTEM_HPP
#define CTRPLUGINFRAMEWORK_SYSTEM_HPP

namespace CTRPluginFramework
{    
    class System
    {
    public:
        static void    Initialize(void);

        static u32     GetIOBaseLCD(void);
        static u32     GetIOBasePAD(void);
        static u32     GetIOBasePDC(void);
        static bool    IsNew3DS(void);

    private:
        System(){}
        static bool     _isInit;
        static bool     _isNew3DS;
        static u32      _IOBaseLCD;
        static u32      _IOBasePAD;
        static u32      _IOBasePDC;
    };
}

#endif