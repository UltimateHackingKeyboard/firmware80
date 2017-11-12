#ifndef __PIT_H__
#define __PIT_H__

// Macros:

    #define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

    #define PIT_I2C_WATCHDOG_HANDLER PIT0_IRQHandler
    #define PIT_I2C_WATCHDOG_IRQ_ID  PIT0_IRQn
    #define PIT_I2C_WATCHDOG_CHANNEL kPIT_Chnl_0

    #define PIT_KEY_SCANNER_HANDLER  PIT1_IRQHandler
    #define PIT_KEY_SCANNER_IRQ_ID   PIT1_IRQn
    #define PIT_KEY_SCANNER_CHANNEL  kPIT_Chnl_1

#endif
