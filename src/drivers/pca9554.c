#include "drivers/pca9554.h"

maus_bus_err_t pca9554_set_gpio_level(uint8_t address, uint8_t gpio_num, pca9554_gpio_level_t level) {
    return MAUS_BUS_OK;
}

maus_bus_err_t pca9554_set_all_gpio_levels(uint8_t address, uint8_t gpio_levels) {
    return MAUS_BUS_OK;
}

maus_bus_err_t pca9554_set_gpio_mode(uint8_t address, uint8_t gpio_num, pca9554_gpio_mode_t mode) {
    return MAUS_BUS_OK;
}

maus_bus_err_t pca9554_set_all_gpio_modes(uint8_t address, uint8_t gpio_modes) {
    return MAUS_BUS_OK;
}

maus_bus_err_t pca9554_get_gpio_level(uint8_t address, uint8_t gpio_num, pca9554_gpio_level_t *level) {
    return MAUS_BUS_OK;
}

maus_bus_err_t pca9554_get_all_gpio_levels(uint8_t address, uint8_t *levels) {
    return MAUS_BUS_OK;
}