#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <time.h>

// 00000001-xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx 7d3a2b4e
// <zap. št>, UUID, kjer je M verzija N pa varjanta, <crc32 checksum>
// implementiral sem UUID verzijo 7 na podlagi
// https://www.rfc-editor.org/rfc/rfc9562.html#name-uuid-version-7
// 00000001-xxxxxxxx-xxxx-7xxx-Nxxx-xxxxxxxxxxxx 000000000

/*
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           unix_ts_ms                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          unix_ts_ms           |  ver  |       rand_a          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|var|                        rand_b                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            rand_b                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

static uint64_t get_epoch_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void generate_uuid_v7(uint8_t uuid[16]) {
    uint64_t ms = get_epoch_ms();

    // 0-47 bit - iz milisekund
    for (int i = 0; i < 6; i++)
        uuid[i] = (ms >> (40 - i * 8)) & 0xFF;

    // 48-128 - naključni biti z verzijo in varianto
    getrandom(uuid + 6, 10, 0);

    uuid[6] = (uuid[6] & 0x0F) | 0x70; // verzija 7 (4 biti)
    uuid[8] = (uuid[8] & 0x3F) | 0x80; // varianta 0b10 (2 bita)
}

static void print_uuid(const uint8_t uuid[16]) {
    for (int i = 0; i < 16; i++) {
        printf("%02x", uuid[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) {
            printf("-");
        }
    }
    printf(" ");
}

// CRC32 checksum function
// https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#CRC-32_example

static uint32_t crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc = crc ^ data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

int main(int argc, char *argv[]) {
    int n = 100;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    for (int i = 1; i <= n; i++) {
        uint8_t uuid[16];
        generate_uuid_v7(uuid);
        printf("%08d-", i);
        print_uuid(uuid);
        printf("%08x ", crc32(uuid, 16));
        printf("\n");
    }

    return 0;
}