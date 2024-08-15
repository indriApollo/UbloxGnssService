//
// Created by rleroux on 8/14/24.
//

#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "../ublox/ublox.h"
#include "../utils/Fletcher8.h"

#define PIPE_READ 0
#define PIPE_WRITE 1

void test_no_payload() {
    int pipe_fd[2];
    pipe(pipe_fd);

    uint8_t ubx_msg[] = {
        UBX_SYNC_CHAR_1, UBX_SYNC_CHAR_2,
        0x01,       // class
        0X02,       // id
        0x00, 0x00, // len
        0x12, 0x34  // ck
    };

    const uint16_t ck = fletcher8(ubx_msg + UBX_CLASS_OFFSET, sizeof(ubx_msg) - UBX_SYNC_LEN - UBX_CK_LEN);
    *(uint16_t*)(ubx_msg + UBX_PAYLOAD_OFFSET) = ck;

    write(pipe_fd[PIPE_WRITE], ubx_msg, sizeof(ubx_msg));

    uint8_t *msg;
    const int retval = parse_ublox_msg(pipe_fd[PIPE_READ], &msg);

    close(pipe_fd[PIPE_WRITE]);
    close(pipe_fd[PIPE_READ]);

    assert(retval == 0);
    assert(msg != NULL);
}

void test_partial_message() {
    int pipe_fd[2];
    pipe(pipe_fd);

    uint8_t ubx_msg[] = {
        UBX_SYNC_CHAR_1, UBX_SYNC_CHAR_2,
        0x01,       // class
        0X02,       // id
        0x00, 0x00, // len
        0x12, 0x34  // ck
    };

    const uint16_t ck = fletcher8(ubx_msg + UBX_CLASS_OFFSET, sizeof(ubx_msg) - UBX_SYNC_LEN - UBX_CK_LEN);
    *(uint16_t*)(ubx_msg + UBX_PAYLOAD_OFFSET) = ck;

    write(pipe_fd[PIPE_WRITE], ubx_msg, sizeof(ubx_msg) / 2);

    uint8_t *msg;
    int retval = parse_ublox_msg(pipe_fd[PIPE_READ], &msg);
    if (retval < 0) goto finally;

    write(pipe_fd[PIPE_WRITE], ubx_msg + sizeof(ubx_msg)/ 2, sizeof(ubx_msg)/ 2);

    retval = parse_ublox_msg(pipe_fd[PIPE_READ], &msg);

    finally:
    close(pipe_fd[PIPE_WRITE]);
    close(pipe_fd[PIPE_READ]);

    assert(retval == 0);
    assert(msg != NULL);
}

void test_false_start() {
    int pipe_fd[2];
    pipe(pipe_fd);

    uint8_t ubx_msg[] = {
        UBX_SYNC_CHAR_1, // false start
        UBX_SYNC_CHAR_1, UBX_SYNC_CHAR_2,
        0x01,       // class
        0X02,       // id
        0x00, 0x00, // len
        0x12, 0x34  // ck
    };

    const uint16_t ck = fletcher8(ubx_msg + 1 + UBX_CLASS_OFFSET, sizeof(ubx_msg) - 1 - UBX_SYNC_LEN - UBX_CK_LEN);
    *(uint16_t*)(ubx_msg + 1 + UBX_PAYLOAD_OFFSET) = ck;

    write(pipe_fd[PIPE_WRITE], ubx_msg, sizeof(ubx_msg));

    int retval;
    uint8_t *msg;
    do {
        retval = parse_ublox_msg(pipe_fd[PIPE_READ], &msg);
    } while (retval > 0);

    close(pipe_fd[PIPE_WRITE]);
    close(pipe_fd[PIPE_READ]);

    assert(retval == 0);
    assert(msg != NULL);
}

void test_offset_start() {
    int pipe_fd[2];
    pipe(pipe_fd);

    uint8_t ubx_msg[] = {
        0x99, 0x98, 0x97, // offset start
        UBX_SYNC_CHAR_1, UBX_SYNC_CHAR_2,
        0x01,       // class
        0X02,       // id
        0x00, 0x00, // len
        0x12, 0x34  // ck
    };

    const int start_offset = 3;

    const uint16_t ck = fletcher8(ubx_msg + start_offset + UBX_CLASS_OFFSET, sizeof(ubx_msg) - start_offset - UBX_SYNC_LEN - UBX_CK_LEN);
    *(uint16_t*)(ubx_msg + start_offset + UBX_PAYLOAD_OFFSET) = ck;

    write(pipe_fd[PIPE_WRITE], ubx_msg, sizeof(ubx_msg));

    int retval;
    uint8_t *msg;
    do {
        retval = parse_ublox_msg(pipe_fd[PIPE_READ], &msg);
    } while (retval > 0);

    close(pipe_fd[PIPE_WRITE]);
    close(pipe_fd[PIPE_READ]);

    assert(retval == 0);
    assert(msg != NULL);
}

void test_payload() {
    int pipe_fd[2];
    pipe(pipe_fd);

    uint8_t ubx_msg[] = {
        UBX_SYNC_CHAR_1, UBX_SYNC_CHAR_2,
        0x01,             // class
        0X02,             // id
        0x03, 0x00,       // len
        0x11, 0x12, 0x13, // payload
        0x12, 0x34        // ck
    };

    const int payload_len = 3;

    const uint16_t ck = fletcher8(ubx_msg + UBX_CLASS_OFFSET, sizeof(ubx_msg) - UBX_SYNC_LEN - UBX_CK_LEN);
    *(uint16_t*)(ubx_msg + UBX_PAYLOAD_OFFSET + payload_len) = ck;

    write(pipe_fd[PIPE_WRITE], ubx_msg, sizeof(ubx_msg));

    int retval;
    uint8_t *msg;
    do {
        retval = parse_ublox_msg(pipe_fd[PIPE_READ], &msg);
    } while (retval > 0);

    close(pipe_fd[PIPE_WRITE]);
    close(pipe_fd[PIPE_READ]);

    assert(retval == 0);
    assert(msg != NULL);
}

void test_parse_ublox_msg() {
    test_no_payload();
    test_partial_message();
    test_false_start();
    test_offset_start();
    test_payload();
}