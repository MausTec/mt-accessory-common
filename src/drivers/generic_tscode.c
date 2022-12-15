#include "drivers/generic_tscode.h"
#include <stdio.h>

maus_bus_err_t generic_tscode_tx(uint8_t *data, size_t length) {
    if (length > 1) {
        maus_bus_write(0x69, data[0], data + 1, length - 1);
    } else {
        maus_bus_write(0x69, data[0], NULL, 0);
    }

    return MAUS_BUS_OK;
}

maus_bus_err_t generic_tscode_rx(uint8_t *data, size_t *count, size_t max_length) {
    return MAUS_BUS_OK;
}