cmake_minimum_required(VERSION 3.28)
project(UbloxGnssService C)

set(CMAKE_C_STANDARD 17)

add_executable(UbloxGnssService main.c
        utils/fletcher8.c
        utils/fletcher8.h
        serial/serial_port.c
        serial/serial_port.h
        ublox/ublox.c
        ublox/ublox.h
        ublox/ublox.c
        tests/ublox_tests.c
        tests/ublox_tests.h
        tests/tests.h
        utils/intersect.c
        utils/intersect.h
        utils/coord.h
        utils/buffer.c
        utils/buffer.h
        chrono/chrono.c
        chrono/chrono.h)
