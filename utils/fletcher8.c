//
// Created by rleroux on 4/27/24.
//

#include <stdint.h>
#include "fletcher8.h"

uint16_t fletcher8(const uint8_t* buffer, const uint16_t length) {
    uint8_t ck_a = 0, ck_b = 0;

    for (uint16_t i = 0; i < length; i++) {
        ck_a += buffer[i];
        ck_b += ck_a;
    }

    return (uint16_t)ck_b << 8 | ck_a;
}
