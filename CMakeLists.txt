cmake_minimum_required(VERSION 3.28)
project(UbloxGnssService C)

set(CMAKE_C_STANDARD 17)

add_executable(UbloxGnssService main.c
        utils/Fletcher8.c
        utils/Fletcher8.h
        serial/serial_port.c
        serial/serial_port.h
        ublox/ublox.c
        ublox/ublox.h
        ublox/ublox.c
        tests/ublox_tests.c
        tests/ublox_tests.h
        tests/tests.c
        tests/tests.h)
