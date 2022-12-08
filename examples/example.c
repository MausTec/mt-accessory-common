#include <stdio.h>
#include <stddef.h>
#include "include/maus_bus.h"

static const size_t EEPROM_SIZE = 2048;

#define DEBUG_MEMBER(member) printf("  %-20s   0x%02lx - 0x%02lx     0x%02lx     0x%08x (%d)\n", #member, offsetof(maus_bus_device_t, member), offsetof(maus_bus_device_t, member) + sizeof(data.member) - 1, sizeof(data.member), (int)data.member, (int)data.member);
#define DEBUG_MEMBER_STR(member) printf("  %-20s   0x%02lx - 0x%02lx     0x%02lx     %s\n", #member, offsetof(maus_bus_device_t, member), offsetof(maus_bus_device_t, member) + sizeof(data.member) - 1, sizeof(data.member), data.member);

void hexdump(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

int main(void) {
    maus_bus_device_t data = {
        .__guard = 0xCAFE,
        .vendor_id = 0x0001,
        .product_id = 0x0001,
        .serial = 0x0000,
        .product_type = (0x01 << 8) | 0x01,
        .feature_config_count = 4,
        .user_data_address = 0xA0,
        .features = {
            .serial = 1,
            .tscode = 0,
        },
        .vendor_name = "Maus-Tec Electronics",
        .product_name = "MB-232T"
    };

    size_t len = sizeof(data);

    printf("Size of maus_bus_device_t struct: 0x%02lx (%ld) bytes.\n", len, len);
    printf("Standard-defined EEPROM size: %ld bytes (%ld bits).\n", EEPROM_SIZE / 8, EEPROM_SIZE);
    printf("Free space: %ld bytes.\n", (EEPROM_SIZE/8) - len);

    printf("\n");
    printf("Member:                  Offset          Size     Value\n");
    printf("-------------------------------------------------------------------------\n");
    DEBUG_MEMBER(__guard);
    DEBUG_MEMBER(vendor_id);
    DEBUG_MEMBER(product_id);
    DEBUG_MEMBER(serial);
    DEBUG_MEMBER(product_type);
    DEBUG_MEMBER(feature_config_count);
    DEBUG_MEMBER(user_data_address);
    DEBUG_MEMBER(feature_flags);
    DEBUG_MEMBER_STR(vendor_name);
    DEBUG_MEMBER_STR(product_name);

    printf("\n");

    hexdump(&data, len);

    return 0;
}