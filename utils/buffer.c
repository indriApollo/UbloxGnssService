//
// Created by rleroux on 9/2/24.
//

#include "buffer.h"

#include <stdio.h>

uint16_t as_uint16(const uint8_t *buf) {
    return (uint16_t)buf[1] << 8 | buf[0];
}

int32_t as_int32(const uint8_t *buf) {
    return (int32_t)buf[3] << 24 | (int32_t)buf[2] << 16 | (int32_t)buf[1] << 8 | buf[0];
}

uint32_t as_uint32(const uint8_t *buf) {
    return (uint32_t)buf[3] << 24 | (uint32_t)buf[2] << 16 | (uint32_t)buf[1] << 8 | buf[0];
}

void printf_buffer(const uint8_t *buf, const int n) {
    for (int i = 0; i < n; i++) {
        const uint8_t a = buf[i];
        if (a == 10)
            printf("[LF]");
        else if (a == 13)
            printf("[CR]");
        else if (a >= 32 && a <= 126)
            printf("%c", a);
        else
            printf("[%02x]", a);
    }
    printf("\n");
}
