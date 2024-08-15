//
// Created by rleroux on 4/27/24.
//

#include <stdint.h>
#include "Fletcher8.h"

uint16_t fletcher8(const uint8_t* buffer, const uint16_t length) {
    uint8_t ck_a = 0, ck_b = 0;

    for (uint16_t i = 0; i < length; i++) {
        ck_a += buffer[i];
        ck_b += ck_a;
    }

    return (ck_a << 8) + ck_b;
}
