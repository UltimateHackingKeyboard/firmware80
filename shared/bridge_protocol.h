#ifndef __BRIDGE_PROTOCOL__
#define __BRIDGE_PROTOCOL__

// Macros:

    typedef enum {
        BridgeCommand_GetKeyStates,
        BridgeCommand_SetTestLed,
        BridgeCommand_SetLedPwmBrightness,
        BridgeCommand_SetDisableKeyMatrixScanState,
    } bridge_command_t;

#endif
