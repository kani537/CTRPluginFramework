#ifndef CTRPLUGINFRAMEWORK_CONTROLLER_HPP
#define CTRPLUGINFRAMEWORK_CONTROLLER_HPP



namespace CTRPluginFramework
{
    enum Key 
    {
        A           = 1,       
        B           = 1 << 1, 
        Select      = 1 << 2, 
        Start       = 1 << 3, 
        DPadRight   = 1 << 4,
        DPadLeft    = 1 << 5,
        DPadUp      = 1 << 6,
        DPadDown    = 1 << 7,
        R           = 1 << 8, 
        L           = 1 << 9,  
        X           = 1 << 10,
        Y           = 1 << 11,
        ZL          = 1 << 14,               ///< The ZL button (New 3DS only)
        ZR          = 1 << 15,               ///< The ZR button (New 3DS only)
        Touchpad    = 1 << 20,
        CStickRight = 1 << 24,
        CStickLeft  = 1 << 25,
        CStickUp    = 1 << 26,
        CStickDown  = 1 << 27,
        CPadRight   = 1 << 28,
        CPadLeft    = 1 << 29,
        CPadUp      = 1 << 30,
        CPadDown    = 1 << 31,
        Up          = DPadUp    | CPadUp,
        Down        = DPadDown  | CPadDown,
        Left        = DPadLeft  | CPadLeft,
        Right       = DPadRight | CPadRight
    };
    class Controller
    {
    public:


            // Return if the key is still being pressed
            static bool    IsKeyDown(Key key);
            // Return if the key just got pressed
            static bool    IsKeyPressed(Key key);
            // Return if the key was released
            static bool    IsKeyReleased(Key key);
            // Update Controller status
            static void    Update(void);
    private:
            static u32      _keysDown;
            static u32      _keysHeld;
            static u32      _keysReleased;  
    };
}

#endif