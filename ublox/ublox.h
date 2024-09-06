//
// Created by rleroux on 7/26/24.
//

#ifndef UBLOX_H
#define UBLOX_H

#include <stdint.h>
#include <termios.h>
#include "../utils/coord.h"

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
#define UNHANDLED_MSG   5

enum ubx_class {
    UBX_NAV = 0x01,
    UBX_ACK = 0x05,
    UBX_CFG = 0x06,
    UBX_MON = 0x0a
};

enum ubx_id {
    UBX_NAV_POSLLH = 0x02,
    UBX_NAV_STATUS = 0x03,
    UBX_MON_VER = 0x04,
    UBX_CFG_VALSET = 0x8a
};

enum ubx_cfg_key_id {
    CFG_USBOUTPROT_UBX = 0x10780001,
    CFG_USBOUTPROT_NMEA = 0x10780002,
    CFG_NAVSPG_FIXMODE = 0x20110011,
    CFG_NAVSPG_DYNMODEL = 0x20110021,
    CFG_MSGOUT_UBX_NAV_POSLLH_USB = 0x2091002c,
    CFG_MSGOUT_UBX_NAV_STATUS_USB = 0x2091001d,
    CFG_RATE_MEAS = 0x30210001,
    CFG_RATE_NAV = 0x30210002
};

int setup_ublox_port(const char *port_name, speed_t baud_rate);

void close_ublox_port(int fd);

int parse_ublox_msg(int fd, uint8_t **msg);

int handle_incoming_ublox_msg(int fd);

int configure_ublox(int fd);

int request_ublox_version(int fd);

void set_ublox_position_callback(void (*callback)(coord pos), uint32_t max_acc);

#endif //UBLOX_H
