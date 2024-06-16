#include "shell.h"
#include "bt_conn.h"
#include "device.h"
#include "keyboard/charger.h"
#include "keyboard/leds.h"
#include "keyboard/oled/oled.h"
#include "usb/usb.h"
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>
#include "bt_conn.h"
#include "keyboard/charger.h"

shell_t Shell = {
    .keyLog = 0,
    .statLog = 0,
    .ledsAlwaysOn = 0,
    .oledEn = 1,
    .sdbState = 1,
    .chargerState = 1,
};


static int cmd_uhk_keylog(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(shell, SHELL_NORMAL, "%i\n", Shell.keyLog);
    } else {
        Shell.keyLog = argv[1][0] == '1';
    }
    return 0;
}
#if !DEVICE_IS_UHK_DONGLE
static int cmd_uhk_statlog(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(shell, SHELL_NORMAL, "%i\n", Shell.statLog);
    } else {
        Shell.statLog = argv[1][0] == '1';
    }
    return 0;
}

static int cmd_uhk_leds(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(shell, SHELL_NORMAL, "%i\n", Shell.ledsAlwaysOn);
    } else {
        Shell.ledsAlwaysOn = argv[1][0] == '1';
    }
    return 0;
}

static int cmd_uhk_sdb(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(shell, SHELL_NORMAL, "%i\n", Shell.sdbState ? 1 : 0);
    } else {
        Shell.sdbState = argv[1][0] == '1';
        gpio_pin_set_dt(&ledsSdbDt, Shell.sdbState);
    }
    return 0;
}

// Charger behavior:
// - If CHARGER_EN=0 or USB is disconnected, then TS reads 6552[0-9] raw value and sometimes drops to 0.
// - If CHARGER_EN=1, USB is connected, and the battery is disconnected, then STAT alternates between 0 and 1 per second

static int cmd_uhk_charger(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(shell, SHELL_NORMAL, "CHARGER_EN: %i | ", Shell.chargerState ? 1 : 0);
        shell_fprintf(shell, SHELL_NORMAL, "STAT: %i ", gpio_pin_get_dt(&chargerStatDt) ? 1 : 0);

        Charger_PrintState();
    } else {
        Shell.chargerState = argv[1][0] == '1';
        gpio_pin_set_dt(&chargerEnDt, Shell.chargerState);
    }
    return 0;
}
#endif // !DEVICE_IS_UHK_DONGLE

#ifdef DEVICE_HAS_OLED
static int cmd_uhk_oled(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(shell, SHELL_NORMAL, "%i\n", Shell.oledEn ? 1 : 0);
    } else {
        Shell.oledEn = argv[1][0] == '1';
        gpio_pin_set_dt(&oledEn, Shell.oledEn);
    }
    return 0;
}
#endif

#ifdef DEVICE_HAS_MERGE_SENSE
static const struct gpio_dt_spec mergeSenseDt = GPIO_DT_SPEC_GET(DT_ALIAS(merge_sense), gpios);

static int cmd_uhk_merge(const struct shell *shell, size_t argc, char *argv[])
{
    shell_fprintf(shell, SHELL_NORMAL, "%i\n", gpio_pin_get_dt(&mergeSenseDt) ? 1 : 0);
    return 0;
}
#endif

static int cmd_uhk_rollover(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(
            shell, SHELL_NORMAL, "%c\n", (HID_GetKeyboardRollover() == Rollover_NKey) ? 'n' : '6');
    } else {
        HID_SetKeyboardRollover((argv[1][0] == '6') ? Rollover_6Key : Rollover_NKey);
    }
    return 0;
}

static int cmd_uhk_gamepad(const struct shell *shell, size_t argc, char *argv[])
{
    if (argc == 1) {
        shell_fprintf(shell, SHELL_NORMAL, "%c\n", HID_GetGamepadActive() ? 'y' : 'n');
    } else {
        HID_SetGamepadActive(argv[1][0] != '0');
    }
    return 0;
}

static int cmd_uhk_btacc(const struct shell *shell, size_t argc, char *argv[])
{
    num_comp_reply(argv[1][0]);
    return 0;
}

static int cmd_uhk_btunpair(const struct shell *shell, size_t argc, char *argv[])
{
    int err = bt_unpair(BT_ID_DEFAULT, NULL);
    printk("unpair operation result: %d\n", err);
    return 0;
}

void InitShell(void)
{
    SHELL_STATIC_SUBCMD_SET_CREATE(uhk_cmds,
        SHELL_CMD_ARG(keylog, NULL, "get/set key logging", cmd_uhk_keylog, 1, 1),
#if !DEVICE_IS_UHK_DONGLE
        SHELL_CMD_ARG(statlog, NULL, "get/set stat logging", cmd_uhk_statlog, 1, 1),
        SHELL_CMD_ARG(leds, NULL, "get/set LEDs always on state", cmd_uhk_leds, 1, 1),
        SHELL_CMD_ARG(sdb, NULL, "get/set LED driver SDB pin", cmd_uhk_sdb, 1, 1),
        SHELL_CMD_ARG(charger, NULL, "get/set CHARGER_EN pin", cmd_uhk_charger, 1, 1),
#endif // !DEVICE_IS_UHK_DONGLE
#ifdef DEVICE_HAS_OLED
        SHELL_CMD_ARG(oled, NULL, "get/set OLED_EN pin", cmd_uhk_oled, 1, 1),
#endif
#ifdef DEVICE_HAS_MERGE_SENSE
        SHELL_CMD_ARG(merge, NULL, "get the merged state of UHK halves", cmd_uhk_merge, 1, 0),
#endif
        SHELL_CMD_ARG(
            rollover, NULL, "get/set keyboard rollover mode (n/6)", cmd_uhk_rollover, 1, 1),
        SHELL_CMD_ARG(gamepad, NULL, "switch gamepad on/off", cmd_uhk_gamepad, 1, 1),
        SHELL_CMD_ARG(btacc, NULL, "accept bluetooth pairing", cmd_uhk_btacc, 1, 1),
        SHELL_CMD_ARG(btunpair, NULL, "unpair bluetooth devices", cmd_uhk_btunpair, 1, 1),
        SHELL_SUBCMD_SET_END);

    SHELL_CMD_REGISTER(uhk, &uhk_cmds, "UHK commands", NULL);
}
