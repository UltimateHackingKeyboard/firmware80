#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include "charger.h"
#include "shell.h"

const struct gpio_dt_spec chargerEnDt = GPIO_DT_SPEC_GET(DT_ALIAS(charger_en), gpios);
const struct gpio_dt_spec chargerStatDt = GPIO_DT_SPEC_GET(DT_ALIAS(charger_stat), gpios);

struct gpio_callback callbackStruct;

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),
const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};

void chargerStatCallback(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
    if (Shell.statLog) {
        printk("STAT changed to %i\n", gpio_pin_get_dt(&chargerStatDt) ? 1 : 0);
    }
}

void InitCharger(void) {
    gpio_pin_configure_dt(&chargerEnDt, GPIO_OUTPUT);
    gpio_pin_set_dt(&chargerEnDt, true);

    gpio_pin_configure_dt(&chargerStatDt, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&chargerStatDt, GPIO_INT_EDGE_BOTH);
    gpio_init_callback(&callbackStruct, chargerStatCallback, BIT(chargerStatDt.pin));
    gpio_add_callback(chargerStatDt.port, &callbackStruct);

    // Init ADC channels
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
        if (!device_is_ready(adc_channels[i].dev)) {
            printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
            return;
        }
        int err;
        err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0) {
            printk("Could not setup channel #%d (%d)\n", i, err);
            return;
        }
    }

    // TODO: Update battery level. See bas_notify()
}
