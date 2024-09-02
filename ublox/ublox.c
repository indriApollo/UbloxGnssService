//
// Created by rleroux on 7/26/24.
//

#include "ublox.h"

#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../serial/serial_port.h"
#include "../utils/fletcher8.h"
#include "../utils/buffer.h"

#define WAIT_FOR_FIRST_BYTE 1
#define INTER_BYTE_TIMEOUT  1

#define CFG_KEY_SIZE sizeof(uint32_t)
#define CFG_VERSION  0
#define CFG_LAYER_RAM      1

#define SW_VERSION_STR_LEN  30
#define HW_VERSION_STR_LEN  10
#define EXT_VERSION_OFFSET (SW_VERSION_STR_LEN + HW_VERSION_STR_LEN)
#define EXT_VERSION_STR_LEN 30

uint8_t input_buffer[256];

enum read_progress {
    SEARCH_FOR_SYNC,
    HANDLE_HEADER_PAYLOAD,
    VERIFY_CK
};

struct {
    int offset;
    int requested_count;
    enum read_progress progress;
    uint8_t *ubx_msg;
} buffer_read = { 0, UBX_MIN_LEN, SEARCH_FOR_SYNC, NULL };

void reset_read(const int nbytes_to_keep) {
    buffer_read.ubx_msg = NULL;
    buffer_read.offset = nbytes_to_keep;
    buffer_read.requested_count = UBX_MIN_LEN - nbytes_to_keep;
    buffer_read.progress = SEARCH_FOR_SYNC;
}

int setup_ublox_port(const char *port_name, const speed_t baud_rate) {
    const int fd = open_serial_port_blocking_io(port_name);
    if (fd < 0) return -1;

    if (set_serial_port_access_exclusive(fd) < 0) return -1;

    if (configure_serial_port(fd, WAIT_FOR_FIRST_BYTE, INTER_BYTE_TIMEOUT, baud_rate) < 0) {
        set_serial_port_access_nonexclusive(fd);
        return -1;
    }

    return fd;
}

void close_ublox_port(const int fd) {
    set_serial_port_access_nonexclusive(fd);
    close(fd);
}

int disable_i2c(uint8_t *buf) {
    const uint32_t cfg_key = CFG_I2C_ENABLED;
    memcpy(buf, &cfg_key, CFG_KEY_SIZE);
    buf[CFG_KEY_SIZE] = false;

    return CFG_KEY_SIZE + 1;
}

int send_cmd(const int fd, const uint8_t *cmd, const int n) {
    const ssize_t c = write(fd, cmd, n);
    if (c == -1) {
        perror("send_cmd write");
        return -1;
    }

    if (c != n) {
        fprintf(stderr, "incomplete send_cmd : actual %zd expected %d\n", c, n);
        return -1;
    }

    printf_buffer(cmd, c);

    return 0;
}

int configure_ublox(const int fd) {
    int i = 0;
    uint8_t ubx_msg[128];

    // Set header
    ubx_msg[i++] = UBX_SYNC_CHAR_1;
    ubx_msg[i++] = UBX_SYNC_CHAR_2;
    ubx_msg[i++] = UBX_CFG;
    ubx_msg[i] = UBX_CFG_VALSET;

    // Set payload
    i = UBX_PAYLOAD_OFFSET;
    ubx_msg[i++] = CFG_VERSION;
    ubx_msg[i++] = CFG_LAYER_RAM;
    ubx_msg[i++] = 0; // reserved
    ubx_msg[i++] = 0; // reserved
    i += disable_i2c(ubx_msg + i);

    // set len
    const uint16_t payload_len = i - UBX_PAYLOAD_OFFSET;
    memcpy(ubx_msg + UBX_LEN_OFFSET, &payload_len, sizeof(uint16_t));

    const uint16_t ck = fletcher8(ubx_msg + UBX_CLASS_OFFSET, UBX_HEADER_LEN + payload_len);
    memcpy(ubx_msg + i, &ck, sizeof(uint16_t));

    i += UBX_CK_LEN;

    assert(i < sizeof(ubx_msg));
    return send_cmd(fd, ubx_msg, i);
}

int request_ublox_version(const int fd) {
    const uint8_t ubx_msg[] = {
        UBX_SYNC_CHAR_1, UBX_SYNC_CHAR_2, UBX_MON, UBX_MON_VER, 0, 0, 0x0e, 0x34
    };

    return send_cmd(fd, ubx_msg, sizeof(ubx_msg));
}

int handle_ubx_mon(const uint8_t *msg) {
    const uint8_t msg_id = msg[UBX_ID_OFFSET];

    if (msg_id != UBX_MON_VER) return UNHANDLED_MSG;

    char *sw_version = (char*)msg + UBX_PAYLOAD_OFFSET;
    char *hw_version = (char*)msg + UBX_PAYLOAD_OFFSET + SW_VERSION_STR_LEN;

    printf("versions:\n  sw: %s\n  hw: %s\n  xt: ", sw_version, hw_version);

    const int ext_len = as_uint16(msg + UBX_LEN_OFFSET) - SW_VERSION_STR_LEN - HW_VERSION_STR_LEN;
    for (int i = 0; i < ext_len / EXT_VERSION_STR_LEN; i++) {
        const int offset = UBX_PAYLOAD_OFFSET + EXT_VERSION_OFFSET + (EXT_VERSION_STR_LEN * i);
        printf("%s,", (char*)msg + offset);
    }
    printf("\n");

    return 0;
}

int handle_ubx_ack(const uint8_t* msg) {
    const bool is_ack = msg[UBX_ID_OFFSET] == 1;

    const uint8_t acked_msg_class = msg[UBX_PAYLOAD_OFFSET];
    const uint8_t acked_msg_id = msg[UBX_PAYLOAD_OFFSET + 1];

    if (!is_ack) {
        printf("UBX-ACK-NAK class %d id %d\n", acked_msg_class, acked_msg_id);
        return -1;
    }

    printf("UBX-ACK-ACK class %d id %d\n", acked_msg_class, acked_msg_id);
    return 0;
}

int handle_ublox_msg(const uint8_t* msg) {
    const uint8_t msg_class = msg[UBX_CLASS_OFFSET];

    switch (msg_class) {
        case UBX_MON:
            return handle_ubx_mon(msg);
        case UBX_ACK:
            return handle_ubx_ack(msg);
        default:
            return UNHANDLED_MSG;
    }
}

int parse_ublox_msg(const int fd, uint8_t **msg) {
    assert(buffer_read.requested_count > 0);
    assert(buffer_read.offset + buffer_read.requested_count <= sizeof(input_buffer));
    const ssize_t c = read(fd, input_buffer + buffer_read.offset, buffer_read.requested_count);

    if (c == -1)
        return -1;

    //printf("read %zd from ublox port\n", c);

    //printf_buffer(input_buffer + buffer_read.offset, c);

    buffer_read.offset += c;
    buffer_read.requested_count -= c;

    if (buffer_read.requested_count > 0) {
        return PARTIAL_MSG;
    }

    if (buffer_read.progress == SEARCH_FOR_SYNC) {
        assert(buffer_read.offset == UBX_MIN_LEN);
        for (int i = 0; i < UBX_MIN_LEN; i++) {
            if (input_buffer[i] == UBX_SYNC_CHAR_1) {
                buffer_read.ubx_msg = input_buffer + i;
                buffer_read.requested_count = i;
                buffer_read.progress = HANDLE_HEADER_PAYLOAD;

                if (buffer_read.requested_count > 0)
                    return PARTIAL_MSG;

                break;
            }
        }

        // We didn't find sync char 1 in the available data
        if (buffer_read.progress == SEARCH_FOR_SYNC) {
            reset_read(0);
            return UNKNOWN_DATA;
        }
    }

    if (buffer_read.progress == HANDLE_HEADER_PAYLOAD) {
        if (buffer_read.ubx_msg[1] != UBX_SYNC_CHAR_2) {
            // We expected sync char 2 immediately after sync char 1
            // Look for sync 1 again
            const int nbytes_to_keep = UBX_MIN_LEN - 1;
            memmove(input_buffer, buffer_read.ubx_msg + 1, nbytes_to_keep);

            reset_read(nbytes_to_keep);

            return UNKNOWN_DATA;
        }

        const uint16_t payload_len = as_uint16(buffer_read.ubx_msg + UBX_LEN_OFFSET);

        buffer_read.progress = VERIFY_CK;

        if (payload_len != 0) {
            buffer_read.requested_count = payload_len;

            if (buffer_read.offset + buffer_read.requested_count > sizeof(input_buffer)) {
                const uint8_t msg_class = buffer_read.ubx_msg[UBX_CLASS_OFFSET];
                const uint8_t msg_id = buffer_read.ubx_msg[UBX_ID_OFFSET];

                fprintf(stderr, "ubx msg class %d id %d len %d is too big for our %lu buffer\n", msg_class, msg_id,
                        payload_len, sizeof(input_buffer));

                reset_read(0);

                return -PAYLOAD_TOO_BIG;
            }

            return PARTIAL_MSG;
        }
    }

    assert(buffer_read.progress == VERIFY_CK);
    const uint16_t payload_len = as_uint16(buffer_read.ubx_msg + UBX_LEN_OFFSET);
    const uint16_t expected_ck = as_uint16(buffer_read.ubx_msg + UBX_PAYLOAD_OFFSET + payload_len);

    const uint16_t actual_ck = fletcher8(buffer_read.ubx_msg + UBX_CLASS_OFFSET, UBX_HEADER_LEN + payload_len);
    if (actual_ck != expected_ck) {
        fprintf(stderr, "ck actual %04x but expected %04x\n", actual_ck, expected_ck);
        return -CK_FAIL;
    }

    *msg = buffer_read.ubx_msg;
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
