//
// Created by rleroux on 4/21/24.
//

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H
#include <termios.h>

int open_serial_port_blocking_io(const char *port_name);

int set_serial_port_access_exclusive(int fd);

int set_serial_port_access_nonexclusive(int fd);

int configure_serial_port(int fd, cc_t vtime, cc_t vmin, speed_t speed);

#endif //SERIAL_PORT_H
