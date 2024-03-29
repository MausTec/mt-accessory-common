#ifndef __drivers__generic_tscode_h
#define __drivers__generic_tscode_h

#ifdef __cplusplus
extern "C" {
#endif

#include "maus_bus.h"

maus_bus_err_t generic_tscode_tx(uint8_t *data, size_t length);
maus_bus_err_t generic_tscode_rx(uint8_t *data, size_t *count, size_t max_length);

#ifdef __cplusplus
}
#endif

#endif
