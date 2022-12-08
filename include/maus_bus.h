#ifndef __maus_bus_h
#define __maus_bus_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/**
 * @brief Feature flags can be used as an alternative to VID/PID matching for generic plug-and-play driver support.
 * 
 */
typedef struct {
    uint32_t tscode : 1;                // Device supports TS-Code control signaling in I2C mode. If any Serial
                                        // feature is available TS-Code signaling will use UART/Serial stream.

    uint32_t serial : 1;                // Device supports serial (UART) data streams using the ??? UART chip.

} maus_bus_device_features_t;


typedef struct {
    uint8_t input_type;
    char display_name[16];
    uint8_t i2c_address;
    uint8_t register_address;
} maus_bus_autoconfig_feature_t;


/**
 * @brief Device ID struct read from EEPROM of all Maus-Bus devices.
 * 
 * EEPROM Header must contain at least 512 bits (64 bytes) of data, as follows. Additional extended features and
 * autoconfig values can be read at offset 0x40. User data begins in the specified address.
 */
typedef struct __attribute__ ((packed)) {
    // 0x00 - 0x01
    uint16_t __guard;                       // Always set to 0xCAFE to indicate initialization.

    // 0x02 - 0x03
    uint16_t vendor_id;                     // Vendor ID assigned by Maus-Tec. Check vendors.csv for the list.
    // 0x04 - 0x05
    uint16_t product_id;                    // Vendor-assigned Product ID. This, and vendor_id, can be used when
                                            // writing custom control drivers.

    // 0x06 - 0x07
    uint16_t serial;                        // Vendor-assigned serial number. Up to you, buddy.
    // 0x08 - 0x09
    uint16_t product_type;                  // This identifies the type of product from the list of generic product
                                            // types. Further types and classifications can be added. It's a sort-of
                                            // bitwise field.

    // 0x0a
    uint8_t feature_config_count;           // Number of feature config items to read at offset 0x40
    // 0x0b
    uint8_t user_data_address;              // Starting offset of any user data

    // 0x0c - 0x0f
    union {
        uint32_t feature_flags;                 // See maus_bus_eeprom_features_t definition above.
        maus_bus_device_features_t features;
    };

    // 0x10 - 0x27
    char vendor_name[24];                   // User-defined Vendor Name for the product. Null-terminated.
    // 0x28 - 0x3f
    char product_name[24];                  // User-defined display name for the product. Null-terminated.
} maus_bus_device_t;

#ifdef __cplusplus
}
#endif

#endif