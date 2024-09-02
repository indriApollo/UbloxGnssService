//
// Created by rleroux on 9/2/24.
//

#include "buffer.h"

#include <stdio.h>

uint16_t as_uint16(const uint8_t *buf) {
    return (uint16_t)buf[1] << 8 | buf[0];
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
