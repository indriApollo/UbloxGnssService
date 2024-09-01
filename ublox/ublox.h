//
// Created by rleroux on 7/26/24.
//

#ifndef UBLOX_H
#define UBLOX_H

#include <stdint.h>
#include <termios.h>

#define UBX_SYNC_CHAR_1 0xb5
#define UBX_SYNC_CHAR_2 0x62

#define UBX_SYNC_LEN   2
#define UBX_CLASS_LEN  1
#define UBX_ID_LEN     1
#define UBX_LEN_LEN    2
#define UBX_HEADER_LEN (UBX_CLASS_LEN + UBX_ID_LEN + UBX_LEN_LEN)
#define UBX_CK_LEN     2
#define UBX_MIN_LEN    (UBX_SYNC_LEN + UBX_HEADER_LEN + UBX_CK_LEN)

#define UBX_CLASS_OFFSET   UBX_SYNC_LEN
#define UBX_ID_OFFSET      (UBX_CLASS_OFFSET + UBX_CLASS_LEN)
#define UBX_LEN_OFFSET     (UBX_ID_OFFSET + UBX_ID_LEN)
#define UBX_PAYLOAD_OFFSET (UBX_LEN_OFFSET + UBX_LEN_LEN)

#define PARTIAL_MSG     1
#define PAYLOAD_TOO_BIG 2
#define CK_FAIL         3
#define UNKNOWN_DATA    4

enum ubx_class {
    UBX_ACK = 0x05,
    UBX_CFG = 0x06,
    UBX_MON = 0x0a
};

enum ubx_id {
    UBX_CFG_VALSET = 0x8a
};

enum ubx_cfg_key_id {
    CFG_I2C_ENABLED = 0x10510003
};

int setup_ublox_port(const char *port_name, speed_t baud_rate);

void close_ublox_port(int fd);

int parse_ublox_msg(int fd, uint8_t **msg);

int handle_incoming_ublox_msg(int fd);

int configure_ublox(int fd);

#endif //UBLOX_H
