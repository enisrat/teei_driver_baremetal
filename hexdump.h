#include <stdio.h>

static void hexdump(const void *memory, int length) {
    unsigned char *bytes = (unsigned char*)memory;
    for (int i = 0; i < length; i++) {
        printf("%02x ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}