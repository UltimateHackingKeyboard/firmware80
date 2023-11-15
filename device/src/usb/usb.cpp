extern "C"
{
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <soc.h>
#include <assert.h>
#include <zephyr/spinlock.h>

#include <zephyr/settings/settings.h>

#include "bluetooth.h"
#include "key_scanner.h"
}
#include "device.h"
#include "usb.hpp"
#include "keyboard_app.hpp"
#include "mouse_app.hpp"
#include "command_app.hpp"
#include "controls_app.hpp"
#include "gamepad_app.hpp"
#include "usb/df/device.hpp"
#include "usb/df/port/zephyr/udc_mac.hpp"
#include "usb/df/class/hid.hpp"
#include "usb/df/vendor/microsoft_os_extension.hpp"
#include "usb/df/vendor/microsoft_xinput.hpp"

// Thread definitions

#define THREAD_STACK_SIZE 1000
#define THREAD_PRIORITY 5

static K_THREAD_STACK_DEFINE(stack_area, THREAD_STACK_SIZE);
static struct k_thread thread_data;

constexpr usb::product_info prinfo {
    0x1D50, "Ultimage Gadget Laboratories",
    USB_DEVICE_PRODUCT_ID, DEVICE_NAME, usb::version("1.0")
};

scancode_buffer prevKeys, keys;
mouse_buffer prevMouseState, mouseState;
controls_buffer prevControls, controls;
gamepad_buffer prevGamepad, gamepad;

void sendUsbReports(void*, void*, void*) {
    while (true) {
        keys.set_code(scancode::A, KeyStates[0][0]);
        if (keys != prevKeys) {
            auto result = keyboard_app::handle().send(keys);
            if (result == hid::result::OK) {
                // buffer accepted for transmit
                prevKeys = keys;
            }
        }

        mouseState.set_button(mouse_button::RIGHT, KeyStates[0][1]);
        mouseState.x = -50;
        // mouseState.y = -50;
        // mouseState.wheel_y = -50;
        // mouseState.wheel_x = -50;
        if (mouseState != prevMouseState) {
            auto result = mouse_app::handle().send(mouseState);
            if (result == hid::result::OK) {
                // buffer accepted for transmit
                prevMouseState = mouseState;
            }
        }

        controls.set_code(consumer_code::VOLUME_INCREMENT, KeyStates[0][2]);
        if (controls != prevControls) {
            auto result = controls_app::handle().send(controls);
            if (result == hid::result::OK) {
                // buffer accepted for transmit
                prevControls = controls;
            }
        }

        gamepad.set_button(gamepad_button::X, KeyStates[0][3]);
        // gamepad.left.X = 50;
        // gamepad.right.Y = 50;
        // gamepad.right_trigger = 50;
        if (gamepad != prevGamepad) {
            auto result = gamepad_app::handle().send(gamepad);
            if (result == hid::result::OK) {
                // buffer accepted for transmit
                prevGamepad = gamepad;
            }
        }

        k_msleep(1);
    }
}

extern "C" {

void usb_init(bool gamepad_enable) {
    static constexpr auto speed = usb::speed::FULL;
    static usb::df::zephyr::udc_mac mac {DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0))};

    static usb::df::hid::function usb_kb { keyboard_app::handle(), usb::hid::boot_protocol_mode::KEYBOARD };
    static usb::df::hid::function usb_mouse { mouse_app::handle() };
    static usb::df::hid::function usb_command { command_app::handle() };
    static usb::df::hid::function usb_controls { controls_app::handle() };
    static usb::df::hid::function usb_gamepad { gamepad_app::handle() };
    static usb::df::microsoft::xfunction usb_xpad { gamepad_app::handle() };

    constexpr auto config_header = usb::df::config::header(usb::df::config::power::bus(500, true));
    const auto shared_config_elems = usb::df::config::join_elements(
            usb::df::hid::config(usb_kb, speed, usb::endpoint::address(0x81), 1),
            usb::df::hid::config(usb_mouse, speed, usb::endpoint::address(0x82), 1),
            usb::df::hid::config(usb_command, speed, usb::endpoint::address(0x83), 20,
                    usb::endpoint::address(0x03), 20),
            usb::df::hid::config(usb_controls, speed, usb::endpoint::address(0x84), 1)
    );

    static const auto base_config = usb::df::config::make_config(config_header, shared_config_elems);

    static const auto gamepad_config = usb::df::config::make_config(config_header, shared_config_elems,
            usb::df::hid::config(usb_gamepad, speed, usb::endpoint::address(0x85), 1)
    );

    static const auto xpad_config = usb::df::config::make_config(config_header, shared_config_elems,
            usb::df::microsoft::xconfig(usb_xpad, usb::endpoint::address(0x85), 1,
                    usb::endpoint::address(0x05), 255)
    );

    static usb::df::microsoft::alternate_enumeration<usb::speeds(usb::speed::FULL)> ms_enum {};
    static usb::df::device_instance<usb::speeds(usb::speed::FULL)> device {mac, prinfo, ms_enum};

    if (device.is_open()) {
        device.close();
        // TODO: add enough sleep to be effective
        k_sleep(K_MSEC(100));
    }
    if (gamepad_enable) {
        ms_enum.set_config(xpad_config);
        device.set_config(gamepad_config);
    } else {
        ms_enum.set_config({});
        device.set_config(base_config);
    }
    device.open();

    k_thread_create(
        &thread_data, stack_area,
        K_THREAD_STACK_SIZEOF(stack_area),
        sendUsbReports,
        NULL, NULL, NULL,
        THREAD_PRIORITY, 0, K_NO_WAIT
    );
}

uint8_t USB_GetKeyboardRollover(void)
{
    return (uint8_t)keyboard_app::handle().get_rollover();
}

void USB_SetKeyboardRollover(uint8_t mode)
{
    keyboard_app::handle().set_rollover((keyboard_app::rollover)mode);
}

} // extern "C"