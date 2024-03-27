#ifndef __mt_accessory_common__maus_bus_err_h
#define __mt_accessory_common__maus_bus_err_h

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic error enumeration for Maus-Bus system functions.
 */
typedef enum {
    MAUS_BUS_OK,
    MAUS_BUS_FAIL,
    MAUS_BUS_TIMEOUT,
    MAUS_BUS_NOT_SUPPORTED,
    MAUS_BUS_NO_MEMORY,
} maus_bus_err_t;

#ifdef __cplusplus
}
#endif

#endif
