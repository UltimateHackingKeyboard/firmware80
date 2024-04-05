#ifndef __LINK_PROTOCOL_H__
#define __LINK_PROTOCOL_H__

// Includes:

#include <stdbool.h>
#include <stdint.h>

// Typedefs:

typedef struct {
    syncable_property_id_t id;
    bool dirty;
} syncable_property_t;

typedef enum {
    SyncablePropertyId_UserConfiguration,
    SyncablePropertyId_CurrentKeymapId,
    SyncablePropertyId_CurrentLayerId,
    SyncablePropertyId_KeyboardReport,
    SyncablePropertyId_MouseReport,
    SyncablePropertyId_GamepadReport,
    SyncablePropertyId_LeftHalfKeyStates,
    SyncablePropertyId_LeftModuleKeyStates,
    SyncablePropertyId_LeftBatteryPercentage,
    SyncablePropertyId_LeftBatteryChargingState,
} syncable_property_id_t;

#endif // __LINK_PROTOCOL_H__