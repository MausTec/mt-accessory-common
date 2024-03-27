#ifndef __maus_bus_h
#define __maus_bus_h

#ifdef __cplusplus
extern "C" {
#endif

#include "drivers/pca9554.h"
#include <stddef.h>
#include <stdint.h>

#define MAUS_BUS_VENDOR_MAX_LENGTH 23
#define MAUS_BUS_PRODUCT_MAX_LENGTH 23

/**
 * @brief Feature flags can be used as an alternative to VID/PID matching for generic plug-and-play
 * driver support.
 */
typedef struct {
    uint32_t tscode : 1; // Device supports TS-Code control signaling in I2C mode. If any Serial
                         // feature is available TS-Code signaling will use UART/Serial stream.

    uint32_t serial : 1; // Device supports serial (UART) data streams using the ??? UART chip.

    uint32_t gpio : 1; // Device supports GPIO commands.

} maus_bus_device_features_t;

/**
 * @brief Feature config flags. This can be used to define directly editing registers.
 */
typedef struct {
    uint8_t input_type;
    char display_name[16];
    uint8_t i2c_address;
    uint8_t register_address;
} maus_bus_autoconfig_feature_t;

/**
 * @brief Device ID struct read from EEPROM of all Maus-Bus devices.
 *
 * EEPROM Header must contain at least 512 bits (64 bytes) of data, as follows. Additional extended
 * features and autoconfig values can be read at offset 0x40. User data begins in the specified
 * address.
 */
typedef struct __attribute__((packed)) {
    // 0x00 - 0x01
    uint16_t __guard; // Always set to 0xCAFE to indicate initialization.

    // 0x02 - 0x03
    uint16_t vendor_id; // Vendor ID assigned by Maus-Tec. Check vendors.csv for the list.
    // 0x04 - 0x05
    uint16_t product_id; // Vendor-assigned Product ID. This, and vendor_id, can be used when
                         // writing custom control drivers.

    // 0x06 - 0x07
    uint16_t serial; // Vendor-assigned serial number. Up to you, buddy.
    // 0x08 - 0x09
    uint16_t product_type; // This identifies the type of product from the list of generic product
                           // types. Further types and classifications can be added. It's a sort-of
                           // bitwise field.

    // 0x0a
    uint8_t feature_config_count; // Number of feature config items to read at offset 0x40
    // 0x0b
    uint8_t user_data_address; // Starting offset of any user data

    // 0x0c - 0x0f
    union {
        uint32_t feature_flags; // See maus_bus_eeprom_features_t definition above.
        maus_bus_device_features_t features;
    };

    // 0x10 - 0x27
    char vendor_name[MAUS_BUS_VENDOR_MAX_LENGTH + 1]; // User-defined Vendor Name for the product.
                                                      // Null-terminated.
    // 0x28 - 0x3f
    char product_name[MAUS_BUS_PRODUCT_MAX_LENGTH + 1]; // User-defined display name for the
                                                        // product. Null-terminated.
} maus_bus_device_t;

/**
 * @brief Write callback to interface with your specific hardware setup.
 */
typedef maus_bus_err_t (*maus_bus_master_write_fn
)(uint8_t address, uint8_t subaddress, uint8_t* data, size_t len);

/**
 * @brief Read callback to interface with your specific hardware setup.
 */
typedef maus_bus_err_t (*maus_bus_master_read_fn
)(uint8_t address, uint8_t subaddress, uint8_t* data, size_t len);

/**
 * @brief Probe callback to detect if a device exists.
 */
typedef maus_bus_err_t (*maus_bus_master_probe_fn)(uint8_t address);

/**
 * @brief Configuration struct for integrating Maus-Bus driver into your hardware.
 */
typedef struct {
    maus_bus_master_read_fn read;
    maus_bus_master_write_fn write;
    maus_bus_master_probe_fn probe;
} maus_bus_config_t;

/**
 * @brief Addresses are a null-terminated array of bytes.
 * Each step in the address is one hop through a multiplexer.
 */
typedef uint8_t* maus_bus_address_t;

typedef enum {
    MAUS_BUS_STATUS_CONNECTED,
    MAUS_BUS_STATUS_PROBED,
    MAUS_BUS_STATUS_TIMEOUT,
    MAUS_BUS_STATUS_DISCONNECTED,
} maus_bus_status_t;

typedef struct maus_bus_uart_driver {
    maus_bus_err_t (*transmit)(uint8_t* data, size_t length);
    maus_bus_err_t (*receive)(uint8_t* data, size_t* count, size_t max_length);
    maus_bus_err_t (*set_mode)(uint32_t baud, uint8_t data, uint8_t parity, uint8_t stop);
} maus_bus_uart_driver_t;

typedef struct maus_bus_gpio_driver {
    maus_bus_err_t (*mode)(uint8_t address, uint8_t gpio_num, pca9554_gpio_mode_t mode);
    maus_bus_err_t (*set)(uint8_t address, uint8_t gpio_num, pca9554_gpio_level_t level);
    maus_bus_err_t (*get)(uint8_t address, uint8_t gpio_num, pca9554_gpio_level_t* level);
} maus_bus_gpio_driver_t;

typedef struct maus_bus_driver {
    maus_bus_uart_driver_t* uart;
    maus_bus_gpio_driver_t* gpio;
} maus_bus_driver_t;

typedef struct maus_bus_device_link {
    maus_bus_address_t address;
    maus_bus_device_t* device;
    maus_bus_status_t status;
    maus_bus_driver_t* driver;
} maus_bus_device_link_t;

/**
 * @brief Callback for devices found on the bus.
 *
 * The address of the device is included here, as well as any optional pointer you may have passed
 * in from the original call to the scan routine.
 *
 * @todo These addresses will eventually be a pointer to a null-terminated array.
 */
typedef void (*maus_bus_scan_callback_t
)(maus_bus_device_t* device, maus_bus_address_t address, void* ptr);

maus_bus_err_t maus_bus_init(maus_bus_config_t* config);
maus_bus_err_t maus_bus_write(uint8_t address, uint8_t subaddress, uint8_t* data, size_t len);
maus_bus_err_t maus_bus_write_byte(uint8_t address, uint8_t subaddress, uint8_t data);
maus_bus_err_t maus_bus_write_str(uint8_t address, uint8_t subaddress, char* str);
maus_bus_err_t maus_bus_read(uint8_t address, uint8_t subaddress, uint8_t* data, size_t len);
maus_bus_err_t maus_bus_read_byte(uint8_t address, uint8_t subaddress, uint8_t* data);

/**
 * @brief Scans the accessory bus only walking hubs and ID chips.
 *
 * This will report any accessory identified by an ID chip, but not unknown devices. This can be
 * called when PIDET is toggled, which comes from HAL. For a manual scan to catch all devices,
 * please use maus_bus_scan_bus_full.
 *
 * @param cb
 * @return size_t Count of devices found.
 */
size_t maus_bus_scan_bus_quick(maus_bus_scan_callback_t cb, void* ptr);

/**
 * @brief Returns a full list of devices found on the bus.
 *
 * This will report any unknown device as well as what the quick scan reports. It only ignores the
 * one internal chip that shares the bus on first-gen EOM boards.
 *
 * @param cb
 * @return size_t Count of devices found.
 */
size_t maus_bus_scan_bus_full(maus_bus_scan_callback_t cb, void* ptr);

/**
 * @brief Frees the internal device list generated from the last scan.
 *
 * Be sure to call this when you're done with the scan results! All device pointers will be freed,
 * so be sure to copy those to your own structure.
 */
void maus_bus_free_device_scan(void);

/**
 * @brief returns false if addresses are not the same
 *
 * @param a
 * @param b
 * @return int
 */
int maus_bus_addrcmp(maus_bus_address_t a, maus_bus_address_t b);

size_t maus_bus_addr2str(char* str, size_t max_len, maus_bus_address_t address);

// Scan Functions

maus_bus_device_t* maus_bus_get_scan_item_by_address(maus_bus_address_t address);
void maus_bus_free_driver(maus_bus_driver_t* driver);

/**
 * @brief Registers a device and allocates a driver.
 *
 * This MUST be called while scan results are still available.
 *
 * @param address
 * @return maus_bus_err_t
 */
maus_bus_err_t maus_bus_register_device(maus_bus_address_t address);

typedef void (*maus_bus_enumeration_callback_t
)(maus_bus_driver_t* driver, maus_bus_device_t* device, maus_bus_address_t address, void* ptr);

/**
 * @brief Iterates over known devices, executing a callback for each.
 *
 * @param cb
 * @param ptr
 * @return maus_bus_err_t
 */
maus_bus_err_t maus_bus_enumerate_devices(maus_bus_enumeration_callback_t cb, void* ptr);

#ifdef __cplusplus
}
#endif

#endif