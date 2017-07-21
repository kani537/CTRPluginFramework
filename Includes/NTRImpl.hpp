#ifndef NTRIMPL_HPP
#define NTRIMPL_HPP

namespace CTRPluginFramework
{
    struct OSDParams;
    class NTRImpl
    {
    public:
        static void     InitNTR(void);
        
        static OSDParams    OSDParameters;
        static bool         IsOSDAvailable;  
    };
}

#endif