#ifndef __drivers__sc16is740c
#define __drivers__sc16is740c

#ifdef __cplusplus
extern "C" {
#endif

#include "maus_bus.h"
#include <stdint.h>
#include <stddef.h>

#define SC16_ADDRESS 0x4D
#define SC16_CRYSTAL_FREQ 3072000UL

#define SC16_REG_RHR        0x00
#define SC16_REG_THR        0x00
#define SC16_REG_IER        0x01
#define SC16_REG_FCR        0x02
#define SC16_REG_IIR        0x02
#define SC16_REG_LCR        0x03
#define SC16_REG_MCR        0x04
#define SC16_REG_LSR        0x05
#define SC16_REG_MSR        0x06
#define SC16_REG_SPR        0x07
#define SC16_REG_TCR        0x06
#define SC16_REG_TLR        0x07
#define SC16_REG_TXLVL      0x08
#define SC16_REG_RXLVL      0x09
#define SC16_REG_EFCR       0x0F
#define SC16_REG_DLL        0x00
#define SC16_REG_DLH        0x01
#define SC16_REG_EFR        0x02
#define SC16_REG_XON1       0x04
#define SC16_REG_XON2       0x05
#define SC16_REG_XOFF1      0x06
#define SC16_REG_XOFF2      0x07

typedef uint32_t sc16_baud_t;

typedef enum {
    SC16_DATA_5 = 0x0,
    SC16_DATA_6 = 0x1,
    SC16_DATA_7 = 0x2,
    SC16_DATA_8 = 0x3,
} sc16_data_bits_t;

typedef enum {
    SC16_PARITY_NONE = 0x0,
    SC16_PARITY_ODD = 0x1,
    SC16_PARITY_EVEN = 0x3,
    SC16_PARITY_FORCE_1 = 0x5,
    SC16_PARITY_FORCE_0 = 0x7,
} sc16_parity_t;

typedef enum {
    SC16_STOP_1 = 0x0,
    SC16_STOP_2 = 0x1,
} sc16_stop_bits_t;

maus_bus_err_t sc16_set_baud_rate(sc16_baud_t baud);
maus_bus_err_t sc16_set_format(sc16_data_bits_t data_bits, sc16_parity_t parity, sc16_stop_bits_t stop);
maus_bus_err_t sc16_enable_fifo(void);
maus_bus_err_t sc16_disable_fifo(void);
maus_bus_err_t sc16_init(sc16_baud_t baud, sc16_data_bits_t data_bits, sc16_parity_t parity, sc16_stop_bits_t stop);
maus_bus_err_t sc16_tx(uint8_t *data, size_t length);
maus_bus_err_t sc16_rx(uint8_t *data, size_t *count, size_t max_length);

#ifdef __cplusplus
}
#endif

#endif
