#ifndef __FLASH_H__
#define __FLASH_H__

// Includes:

#include "zephyr/storage/flash_map.h"
#include "config_parser/config_globals.h"
#include "storage.h"

// Macros:

    #define HARDWARE_CONFIG_SIZE 64
    #define USER_CONFIG_SIZE (32*1024)

// Functions:

uint8_t Flash_LaunchTransfer(storage_operation_t operation, config_buffer_id_t config_buffer_id, void (*successCallback));

// Variables:

    extern const struct flash_area *hardwareConfigArea;
    extern const struct flash_area *userConfigArea;

#endif // __FLASH_H__