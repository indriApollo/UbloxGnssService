//
// Created by rleroux on 9/2/24.
//

#ifndef BUFFER_H
#define BUFFER_H
#include <stdint.h>

uint16_t as_uint16(const uint8_t *buf);

int32_t as_int32(const uint8_t *buf);

uint32_t as_uint32(const uint8_t *buf);

void printf_buffer(const uint8_t *buf, int n);

#endif //BUFFER_H
