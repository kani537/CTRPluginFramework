#ifndef HOOK_H
#define HOOK_H

#include "types.h"

union   HookStatus
{
    struct
    {
        bool    isEnabled : 1;  ///< Hold the hook's status
        bool    useLinkRegisterToReturn : 1;    ///< Enable the use of bx lr to return from callback. LR will be properly restored after the callback is executed | Default: true
        bool    ExecuteOverwrittenInstructionBeforeCallback : 1;    ///< If the instruction overwriten by the hook (target) must be executed before the callback | Default: true
        bool    ExecuteOverwrittenInstructionAfterCallback : 1;     ///< If the instruction overwritten by the hook (target) must be executed after the callback | Default: false
    };
    u32     raw;
};

struct  Hook
{
    HookStatus  flags{};  ///< See HookStatus
    u32         targetAddress;  ///< The address to hook from
    u32         returnAddress;  ///< The address to return to after callback | Default: targetAddress + 4
    u32         callbackAddress;   ///< The address of the callback
    u32         overwritterInstr;
    u32         index;

    Hook();

    /**
     * \brief Initialize hook target and callback
     * \param targetAddr The address to hook from
     * \param callbackAddr The callback to be called by the hook
     * \param returnAddr Optional return address. If not passed (0) then the return address is targetAddr + 4
     */
    void        Initialize(u32 targetAddr, u32 callbackAddr, u32 returnAddr = 0);

    /**
     * \brief Apply the hook
     * \return true if the operation was a success
     */
    bool        Enable(void);

    /**
     * \brief Disable the hook
     */
    void        Disable(void);
};

#endif
