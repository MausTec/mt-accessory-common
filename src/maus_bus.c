#include "eom-hal.h"
#include "maus_bus.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "drivers/sc16is740.h"
#include "drivers/generic_tscode.h"

static const uint8_t EEPROM_IDS[] = { 0x50, 0x69 };
static const uint8_t IGNORE_IDS[] = { 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 };

static struct _device_scan_node {
    maus_bus_address_t address;
    maus_bus_device_t device;
    struct _device_scan_node *next;
} *_scan_list = NULL;

static struct _device_driver_node {
    maus_bus_driver_t *driver;
    maus_bus_address_t address;
    maus_bus_device_t device;
    struct _device_driver_node *next;
} *_driver_list = NULL;


static maus_bus_config_t _config = {
    .read = NULL,
    .write = NULL,
};

maus_bus_err_t maus_bus_init(maus_bus_config_t *config) {
    _config = *config;
    return MAUS_BUS_OK;
}

maus_bus_err_t maus_bus_write(uint8_t address, uint8_t subaddress, uint8_t *data, size_t len) {
    if (_config.write == NULL) return MAUS_BUS_FAIL;
    return _config.write(address, subaddress, data, len);
}

maus_bus_err_t maus_bus_write_byte(uint8_t address, uint8_t subaddress, uint8_t data) {
    // printf("write_byte(%02x, %02x, %02x);\n", address, subaddress, data);
    if (_config.write == NULL) return MAUS_BUS_FAIL;
    uint8_t tmp = data;
    return _config.write(address, subaddress, &tmp, 1);
} 

maus_bus_err_t maus_bus_write_str(uint8_t address, uint8_t subaddress, char *str) {
    if (_config.write == NULL) return MAUS_BUS_FAIL;
    return _config.write(address, subaddress, (uint8_t*) str, strlen(str));
}

maus_bus_err_t maus_bus_read(uint8_t address, uint8_t subaddress, uint8_t *data, size_t len) {
    if (_config.read == NULL) return MAUS_BUS_FAIL;
    memset(data, 0, len);
    return _config.read(address, subaddress, data, len);
}

maus_bus_err_t maus_bus_read_byte(uint8_t address, uint8_t subaddress, uint8_t *data) {
    if (_config.read == NULL) return MAUS_BUS_FAIL;
    *data = 0x00;
    return _config.read(address, subaddress, data, 1);
}

// Scan Functions

size_t maus_bus_scan_bus_quick(maus_bus_scan_callback_t cb, void *ptr) {
    size_t count = 0;
    // TODO - scan hubs. This will be a recursive case.

    // Scan EEPROM ID chips:

    for (size_t i = 0; i < sizeof(EEPROM_IDS); i++) {
        uint8_t address = EEPROM_IDS[i];
        uint16_t guard = 0x0000;
        eom_hal_accessory_master_read_register(address, 0x00, (uint8_t*) &guard, 2);

        if (guard == 0xCAFE) {
            struct _device_scan_node *node = malloc(sizeof(struct _device_scan_node));
            if (node == NULL) return count;

            maus_bus_address_t addr_path = (maus_bus_address_t) malloc(1 + 1);

            if (addr_path != NULL) {
                addr_path[0] = address;
                addr_path[1] = 0x00;
            }

            node->address = addr_path;
            node->next = NULL;

            eom_hal_accessory_master_read_register(address, 0x00, (uint8_t*) &node->device, sizeof(maus_bus_device_t));
            if (node->device.__guard != guard) return count;

            // Clean up data:
            node->device.vendor_name[23] = '\0';
            node->device.product_name[23] = '\0';

            if (_scan_list == NULL) {
                _scan_list = node;
            } else {
                struct _device_scan_node *p = _scan_list;
                while (p->next != NULL) p = p->next;
                p->next = node;
            }

            if (cb != NULL) {
                (*cb)(&node->device, node->address, ptr);
            }

            count++;
        }
    }

    return count;
}

int maus_bus_addrcmp(maus_bus_address_t a, maus_bus_address_t b) {
    size_t idx = 0;

    while (a[idx] != 0x00 && b[idx] != 0x00) {
        if (a[idx] != b[idx]) return 0;
        idx++;
    }

    if (a[idx] != b[idx]) return 0;
    return 1;
}

size_t maus_bus_get_address_depth(maus_bus_address_t address) {
    size_t depth = 0;
    while (address[depth++] != 0x00) depth++;
    return depth;
}

uint8_t maus_bus_get_final_address(maus_bus_address_t address) {
    return address[maus_bus_get_address_depth(address)];
}

size_t maus_bus_addr2str(char *str, size_t max_len, maus_bus_address_t address) {
    size_t depth = maus_bus_get_address_depth(address);
    size_t len = ((depth + 1) * 2) + depth;

    if (str != NULL && max_len > 0) {
        for (size_t idx = 0; idx < depth; idx++) {
            char *cursor = str + (idx * 3);
            if (cursor > str + max_len - 3) break;
            snprintf(cursor, max_len - (cursor - str), idx == depth ? "%02X:" : "%02X", address[idx]);
        }
    }

    return len;
}

int _device_idx_by_address(maus_bus_address_t address) {
    struct _device_scan_node *p = _scan_list;
    int i = 0;

    while (p != NULL) {
        if (maus_bus_addrcmp(address, p->address)) {
            return i;
        }
        p = p->next;
        i++;
    }

    return -1;
}

maus_bus_device_t *maus_bus_get_scan_item_by_address(maus_bus_address_t address) {
    struct _device_scan_node *p = _scan_list;

    while (p != NULL) {
        if (maus_bus_addrcmp(p->address, address)) {
            return &p->device;
        }

        p = p->next;
    }

    return NULL;
}

int _address_is_ignored(uint8_t address) {
    for (size_t ign = 0; ign < sizeof(IGNORE_IDS); ign++) {
        if (IGNORE_IDS[ign] == address) return 1;
    }
    return 0;
}

size_t maus_bus_scan_bus_full(maus_bus_scan_callback_t cb, void *ptr) {
    size_t count = maus_bus_scan_bus_quick(cb, ptr);
    size_t addr_depth = 0;

    for (size_t i = 0; i < 127; i++) {
        uint8_t address = i;
        uint8_t addr_tmp[] = { address, 0x00 };
        
        if (_device_idx_by_address(addr_tmp) != -1) continue;
        if (_address_is_ignored(address)) continue;

        eom_hal_err_t err = eom_hal_accessory_master_probe(address);

        if (err != EOM_HAL_OK) {
            continue;
        }

        char addr_str[20] = "";
        maus_bus_addr2str(addr_str, 20, addr_tmp);

        // Device can be added as an unknown:
        struct _device_scan_node *node = malloc(sizeof(struct _device_scan_node));
        if (node == NULL) return count;

        maus_bus_address_t addr_path = (maus_bus_address_t) malloc(addr_depth + 2);

        if (addr_path != NULL) {
            memcpy(addr_path, addr_tmp, addr_depth + 2);
        }

        node->address = addr_path;
        node->next = NULL;

        memset(&node->device, 0x00, sizeof(node->device));
        snprintf(node->device.vendor_name, MAUS_BUS_VENDOR_MAX_LENGTH, "<Unknown - %d>", node->device.vendor_id);
        snprintf(node->device.product_name, MAUS_BUS_PRODUCT_MAX_LENGTH, "<Unknown 0x%02X - %d>", address, node->device.product_id);

        // Devices at this address SPECIFICALLY are TS-code listeners.
        if (address == 0x69) {
            node->device.features.tscode = 1;
        }

        if (_scan_list == NULL) {
            _scan_list = node;
        } else {
            struct _device_scan_node *p = _scan_list;
            while (p->next != NULL) p = p->next;
            p->next = node;
        }

        if (cb != NULL) {
            (*cb)(&node->device, node->address, ptr);
        }

        count++;
    }

    return count;
}

void maus_bus_free_device_scan(void) {
    struct _device_scan_node *p = _scan_list;

    while (p != NULL) {
        _scan_list = p->next;
        free(p);
        p = _scan_list;
    }
}

maus_bus_driver_t *maus_bus_discover_driver(maus_bus_device_t *device) {
    maus_bus_driver_t *driver = (maus_bus_driver_t*) malloc(sizeof(maus_bus_driver_t));
    if (driver == NULL) return NULL;

    memset(driver, 0x00, sizeof(maus_bus_driver_t));

    // Enumerate Features

    if (device->features.serial) {
        sc16_init(9600, SC16_DATA_8, SC16_PARITY_NONE, SC16_STOP_1);

        driver->transmit = &sc16_tx;
        driver->receive = &sc16_rx;
    } else if (device->features.tscode) {
        driver->transmit = &generic_tscode_tx;
    }

    return driver;
}

maus_bus_err_t maus_bus_register_device(maus_bus_address_t address) {
    maus_bus_device_t *scan_item = maus_bus_get_scan_item_by_address(address);
    if (scan_item == NULL) return MAUS_BUS_FAIL;

    maus_bus_driver_t *driver = maus_bus_discover_driver(scan_item);
    if (driver == NULL) return MAUS_BUS_NOT_SUPPORTED;

    struct _device_driver_node *node = (struct _device_driver_node*) malloc(sizeof(struct _device_driver_node));
    if (node == NULL) goto error;

    // This is duplicating a shallow copy of the device. It doesn't have pointers, does it?
    memcpy(&node->device, scan_item, sizeof(maus_bus_device_t));

    node->address = malloc(maus_bus_get_address_depth(address));
    if (node->address == NULL) goto error;

    memcpy(node->address, address, maus_bus_get_address_depth(address));

    node->driver = driver;
    node->next = NULL;

    struct _device_driver_node *ptr = _driver_list;

    if (ptr == NULL) {
        _driver_list = node;
        return MAUS_BUS_OK;
    }

    while (ptr->next != NULL) ptr = ptr->next;
    ptr->next = node;

    return MAUS_BUS_OK;

error:
    free(node->address);
    free(node);
    return MAUS_BUS_NO_MEMORY;
}

maus_bus_err_t maus_bus_enumerate_devices(maus_bus_enumeration_callback_t cb, void *ptr) {
    struct _device_driver_node *node = _driver_list;

    while (node != NULL) {
        cb(node->driver, &node->device, node->address, ptr);
        node = node->next;
    }

    return MAUS_BUS_OK;
}