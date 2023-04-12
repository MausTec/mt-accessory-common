#ifndef __drivers__pca_h
#define __drivers__pca_h

#ifdef __cplusplus
extern "C" {
#endif

#include "maus_bus.h"
#include <stdint.h>

#define PCA9554_ADDRESS      0x20
#define PCA9554A_ADDRESS     0x28

#define PCA9554_REG_INPUT    0x00
#define PCA9554_REG_OUTPUT   0x01
#define PCA9554_REG_POLARITY 0x02
#define PCA9554_REG_CONFIG   0x03

typedef enum {
    PCA9554_LOW,
    PCA9554_HIGH,
} pca9554_gpio_level_t;

typedef enum {
    PCA9554_OUTPUT,
    PCA9554_INPUT,
} pca9554_gpio_mode_t;

maus_bus_err_t pca9554_set_gpio_level(uint8_t address, uint8_t gpio_num, pca9554_gpio_level_t level);
maus_bus_err_t pca9554_set_all_gpio_levels(uint8_t address, uint8_t gpio_levels);
maus_bus_err_t pca9554_set_gpio_mode(uint8_t address, uint8_t gpio_num, pca9554_gpio_mode_t mode);
maus_bus_err_t pca9554_set_all_gpio_modes(uint8_t address, uint8_t gpio_modes);
maus_bus_err_t pca9554_get_gpio_level(uint8_t address, uint8_t gpio_num, pca9554_gpio_level_t *level);
maus_bus_err_t pca9554_get_all_gpio_levels(uint8_t address, uint8_t *levels);

// Todo: Add support for PCIs and inversion.

#ifdef __cplusplus
}
#endif

#endif
