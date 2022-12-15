#include "drivers/sc16is740.h"

maus_bus_err_t sc16_init(sc16_baud_t baud, sc16_data_bits_t data_bits, sc16_parity_t parity, sc16_stop_bits_t stop) {
    maus_bus_err_t err = MAUS_BUS_OK;

    err = err || sc16_set_baud_rate(baud);
    err = err || sc16_set_format(data_bits, parity, stop);
    err = err || sc16_enable_fifo();

    return err;
}

maus_bus_err_t sc16_set_baud_rate(sc16_baud_t baud) {
    maus_bus_err_t err = MAUS_BUS_OK;

    uint8_t tmp_lcr = 0x00;
    uint8_t tmp_mcr = 0x00;

    uint16_t baud_divisor = 0xFF;
    uint8_t prescaler = 1;

    err = err || maus_bus_read_byte(SC16_ADDRESS, SC16_REG_LCR << 3, &tmp_mcr);
    if (err != MAUS_BUS_OK) return err;

    if (tmp_mcr & 0x80) {
        prescaler = 4;
    }

    baud_divisor = (SC16_CRYSTAL_FREQ / prescaler) / (baud * 16);

    // printf("Divisor: %d (%04x), prescaler: %d, baud: %d\n", baud_divisor, baud_divisor, prescaler, baud);

    err = err || maus_bus_read_byte(SC16_ADDRESS, SC16_REG_LCR << 3, &tmp_lcr);
    err = err || maus_bus_write_byte(SC16_ADDRESS, SC16_REG_LCR << 3, tmp_lcr | 0x80);
    err = err || maus_bus_write_byte(SC16_ADDRESS, SC16_REG_DLL << 3, (uint8_t) baud_divisor);
    err = err || maus_bus_write_byte(SC16_ADDRESS, SC16_REG_DLH << 3, (uint8_t) (baud_divisor >> 8));
    err = err || maus_bus_write_byte(SC16_ADDRESS, SC16_REG_LCR << 3, tmp_lcr);

    return err;
}

maus_bus_err_t sc16_set_format(sc16_data_bits_t data_bits, sc16_parity_t parity, sc16_stop_bits_t stop) {
    maus_bus_err_t err = MAUS_BUS_OK;

    uint8_t tmp_lcr = 0x00;

    err = err || maus_bus_read_byte(SC16_ADDRESS, SC16_REG_LCR << 3, &tmp_lcr);
    if (err != MAUS_BUS_OK) return err;

    tmp_lcr =
        (tmp_lcr & 0b11000000) | 
        ((parity & 0b111) << 3) |
        ((data_bits & 0b1) << 2) | 
        (data_bits & 0b11);

    err = err || maus_bus_write_byte(SC16_ADDRESS, SC16_REG_LCR << 3, tmp_lcr);

    return err;
}

maus_bus_err_t sc16_enable_fifo(void) {
    maus_bus_err_t err = MAUS_BUS_OK;

    uint8_t tmp_fcr = 0b00000001;

    err = err || maus_bus_write_byte(SC16_ADDRESS, SC16_REG_FCR << 3, tmp_fcr);

    return err;
}

maus_bus_err_t sc16_disable_fifo(void) {
    maus_bus_err_t err = MAUS_BUS_OK;

    err = err || maus_bus_write_byte(SC16_ADDRESS, SC16_REG_FCR << 3, 0x00);

    return err;
}

maus_bus_err_t sc16_tx(uint8_t *data, size_t length) {
    maus_bus_err_t err = MAUS_BUS_OK;

    // todo - the thr/rhr are 64 byte fifos, so we should loop and poll LSR until
    //        lsr[6] or lsr[5] is set.
    err = err || maus_bus_write(SC16_ADDRESS, SC16_REG_THR << 3, data, length);

    return err;
}

maus_bus_err_t sc16_rx(uint8_t *data, size_t *count, size_t max_length) {
    maus_bus_err_t err = MAUS_BUS_OK;

    // todo - we should read bytes from the fifo until lsr[0] is cleared or max-length reached
    err = err || maus_bus_read(SC16_ADDRESS, SC16_REG_RHR << 3, data, max_length);

    return err;
}