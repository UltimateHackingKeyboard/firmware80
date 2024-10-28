#ifndef __DEVICE_H__
#define __DEVICE_H__


// Includes:

    #ifdef __ZEPHYR__
        #include "autoconf.h"
    #endif

// Typedefs:

    #define DEVICE_ID_UHK60V1 1
    #define DEVICE_ID_UHK60V2 2
    #define DEVICE_ID_UHK60V1_RIGHT 1
    #define DEVICE_ID_UHK60V2_RIGHT 2
    #define DEVICE_ID_UHK80_LEFT 3
    #define DEVICE_ID_UHK80_RIGHT 4
    #define DEVICE_ID_UHK_DONGLE 5
    #define DEVICE_COUNT 5

    #ifdef __ZEPHYR__
        #ifndef CONFIG_DEVICE_ID
            #error "CONFIG_DEVICE_ID is undefined!"
        #else
            #define DEVICE_ID CONFIG_DEVICE_ID
        #endif
    #else
        #ifndef DEVICE_ID
            #error "DEVICE_ID is undefined!"
        #else
            #define CONFIG_DEVICE_ID DEVICE_ID
        #endif
    #endif

    #define DEVICE_IS_UHK60V1_RIGHT (CONFIG_DEVICE_ID == DEVICE_ID_UHK60V1_RIGHT)
    #define DEVICE_IS_UHK60V2_RIGHT (CONFIG_DEVICE_ID == DEVICE_ID_UHK60V2_RIGHT)
    #define DEVICE_IS_UHK80_LEFT (CONFIG_DEVICE_ID == DEVICE_ID_UHK80_LEFT)
    #define DEVICE_IS_UHK80_RIGHT (CONFIG_DEVICE_ID == DEVICE_ID_UHK80_RIGHT)
    #define DEVICE_IS_UHK_DONGLE (CONFIG_DEVICE_ID == DEVICE_ID_UHK_DONGLE)

    #define DEVICE_IS_UHK80 (DEVICE_IS_UHK80_LEFT || DEVICE_IS_UHK80_RIGHT)
    #define DEVICE_IS_UHK60 (DEVICE_IS_UHK60V1_RIGHT || DEVICE_IS_UHK60V2_RIGHT)
    #define DEVICE_IS_MASTER (DEVICE_IS_UHK80_RIGHT || DEVICE_IS_UHK60V1_RIGHT || DEVICE_IS_UHK60V2_RIGHT)

    #define DEVICE_NAME CONFIG_BT_DIS_MODEL

    #if (DEVICE_ID == DEVICE_ID_UHK60V1_RIGHT || DEVICE_ID == DEVICE_ID_UHK60V2_RIGHT)
        #define KEY_MATRIX_ROWS 5
        #define KEY_MATRIX_COLS 7
        #define DEVICE_HAS_MERGE_SENSOR 1
        #define DEVICE_HAS_SEGMENT_DISPLAY 1
        #define DEVICE_HAS_OLED 0
        #define DEVICE_IS_KEYBOARD 1
        #define DEVICE_HAS_NRF 0
        #define DEVICE_HAS_BATTERY 0
    #elif (DEVICE_ID == DEVICE_ID_UHK80_LEFT)
        #define KEY_MATRIX_ROWS 6
        #define KEY_MATRIX_COLS 7
        #define DEVICE_HAS_MERGE_SENSOR 1
        #define DEVICE_HAS_SEGMENT_DISPLAY 0
        #define DEVICE_HAS_OLED 0
        #define DEVICE_IS_KEYBOARD 1
        #define DEVICE_HAS_NRF 1
        #define DEVICE_HAS_BATTERY 1

        // module extras
        #define MODULE_ID ModuleId_LeftKeyboardHalf
        #define MODULE_KEY_COUNT 41
        #define MODULE_POINTER_COUNT 0
    #elif (DEVICE_ID == DEVICE_ID_UHK80_RIGHT)
        #define KEY_MATRIX_ROWS 6
        #define KEY_MATRIX_COLS 10
        #define DEVICE_HAS_MERGE_SENSOR 0
        #define DEVICE_HAS_SEGMENT_DISPLAY 0
        #define DEVICE_HAS_OLED 1
        #define DEVICE_IS_KEYBOARD 1
        #define DEVICE_HAS_NRF 1
        #define DEVICE_HAS_BATTERY 1
    #elif (DEVICE_ID == DEVICE_ID_UHK_DONGLE)
        #define KEY_MATRIX_ROWS 6
        #define KEY_MATRIX_COLS 10
        #define DEVICE_HAS_MERGE_SENSOR 0
        #define DEVICE_HAS_SEGMENT_DISPLAY 0
        #define DEVICE_HAS_OLED 0
        #define DEVICE_IS_KEYBOARD 0
        #define DEVICE_HAS_NRF 1
        #define DEVICE_HAS_BATTERY 0
    #elif (defined(__ZEPHYR__))
        #define DEVICE_HAS_MERGE_SENSOR 0
        #define DEVICE_HAS_SEGMENT_DISPLAY 0
        #define DEVICE_HAS_OLED 0
        #define DEVICE_IS_KEYBOARD 0
        #define DEVICE_HAS_NRF 1
        #define DEVICE_HAS_BATTERY 1
    #else
        #error "unknown DEVICE_ID!"
    #endif

    typedef enum {
        DeviceId_Uhk60v1_Right = DEVICE_ID_UHK60V1_RIGHT,
        DeviceId_Uhk60v2_Right = DEVICE_ID_UHK60V2_RIGHT,
        DeviceId_Uhk80_Right = DEVICE_ID_UHK80_RIGHT,
        DeviceId_Uhk80_Left = DEVICE_ID_UHK80_LEFT,
        DeviceId_Uhk_Dongle = DEVICE_ID_UHK_DONGLE,
        DeviceId_Uhk80_Dongle = DeviceId_Uhk_Dongle,
        DeviceId_Count = DEVICE_ID_UHK_DONGLE + 1,
    } device_id_t;

#endif
