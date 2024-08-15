//
// Created by rleroux on 7/26/24.
//

#include "ublox.h"

#include <assert.h>

#include "../serial/serial_port.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../utils/Fletcher8.h"

#define WAIT_FOR_FIRST_BYTE 1
#define INTER_BYTE_TIMEOUT  1

enum ubx_class {
    UBX_ACK = 0x05,
    UBX_CFG = 0x06,
    UBX_MON = 0x0a
};

uint8_t buffer[128];

enum read_progress {
    SEARCH_FOR_SYNC,
    HANDLE_HEADER_PAYLOAD,
    VERIFY_CK
};

int buffer_read_offset = 0;
int buffer_requested_read_count = UBX_MIN_LEN;
enum read_progress buffer_read_progress = SEARCH_FOR_SYNC;
uint8_t *ubx_msg = NULL;

int setup_ublox(const char *port_name, const speed_t baud_rate) {
    const int fd = open_serial_port_blocking_io(port_name);
    if (fd < 0) return -1;

    if (set_serial_port_access_exclusive(fd) < 0) return -1;

    if (configure_serial_port(fd, WAIT_FOR_FIRST_BYTE, INTER_BYTE_TIMEOUT, baud_rate) < 0) {
        set_serial_port_access_nonexclusive(fd);
        return -1;
    }

    return fd;
}

void close_ublox(const int fd) {
    set_serial_port_access_nonexclusive(fd);
    close(fd);
}

int handle_ubx_ack(const uint8_t* msg) {
    const uint8_t msg_id = *(msg + UBX_ID_OFFSET);

    return 0;
}

int handle_ublox_msg(const uint8_t* msg) {
    const uint8_t msg_class = *(msg + UBX_CLASS_OFFSET);

    switch (msg_class) {
        case UBX_ACK:
            return handle_ubx_ack(msg);
        default:
            return -1;
    }
}

void reset_read(const int nbytes_to_keep) {
    ubx_msg = NULL;
    buffer_read_offset = nbytes_to_keep;
    buffer_requested_read_count = UBX_MIN_LEN - nbytes_to_keep;
    buffer_read_progress = SEARCH_FOR_SYNC;
}

int parse_ublox_msg(const int fd, uint8_t **msg) {
    assert(buffer_read_offset + buffer_requested_read_count <= sizeof(buffer));
    const ssize_t c = read(fd, buffer + buffer_read_offset, buffer_requested_read_count);

    if (c == -1)
        return -1;

    buffer_read_offset += c;
    buffer_requested_read_count -= c;

    if (buffer_requested_read_count > 0) {
        return PARTIAL_MSG;
    }

    if (buffer_read_progress == SEARCH_FOR_SYNC) {
        assert(buffer_read_offset == UBX_MIN_LEN);
        for (int i = 0; i < UBX_MIN_LEN; i++) {
            if (buffer[i] == UBX_SYNC_CHAR_1) {
                ubx_msg = buffer + i;
                buffer_requested_read_count = i;
                buffer_read_progress = HANDLE_HEADER_PAYLOAD;

                if (buffer_requested_read_count > 0)
                    return PARTIAL_MSG;

                break;
            }
        }
    }

    if (buffer_read_progress == HANDLE_HEADER_PAYLOAD) {
        if (ubx_msg[1] != UBX_SYNC_CHAR_2) {
            // We expected sync char 2 immediately after sync char 1
            // Look for sync 1 again
            const int nbytes_to_keep = UBX_MIN_LEN - 1;
            memmove(buffer, ubx_msg + 1, nbytes_to_keep);

            reset_read(nbytes_to_keep);

            return PARTIAL_MSG;
        }

        const uint16_t payload_len = *(uint16_t*)(ubx_msg + UBX_LEN_OFFSET);

        buffer_read_progress = VERIFY_CK;

        if (payload_len != 0) {
            buffer_requested_read_count = payload_len;

            if (buffer_read_offset + buffer_requested_read_count > sizeof(buffer)) {
                const uint8_t msg_class = *(ubx_msg + UBX_CLASS_OFFSET);
                const uint8_t msg_id = *(ubx_msg + UBX_ID_OFFSET);

                fprintf(stderr, "ubx msg class %d id %d len %d is too big for our %lu buffer", msg_class, msg_id,
                        payload_len, sizeof(buffer));

                reset_read(0);

                return -PAYLOAD_TOO_BIG;
            }

            return PARTIAL_MSG;
        }
    }

    assert(buffer_read_progress == VERIFY_CK);
    const uint16_t payload_len = *(uint16_t *) (ubx_msg + UBX_LEN_OFFSET);
    const uint16_t expected_ck = *(uint16_t *) (ubx_msg + UBX_PAYLOAD_OFFSET + payload_len);

    const uint16_t actual_ck = fletcher8(ubx_msg + UBX_CLASS_OFFSET, UBX_HEADER_LEN + payload_len);
    if (actual_ck != expected_ck) {
        fprintf(stderr, "ck actual %d but expected %d", actual_ck, expected_ck);
        return -CK_FAIL;
    }

    *msg = ubx_msg;
    reset_read(0);

    return 0;
}

int handle_incoming_ublox_msg(const int fd) {
    uint8_t *msg;
    const int r = parse_ublox_msg(fd, &msg);
    if (r != 0)
        return r;

    return handle_ublox_msg(msg);
}