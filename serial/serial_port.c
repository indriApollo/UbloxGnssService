//
// Created by rleroux on 4/21/24.
//

#include "serial_port.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>

int open_serial_port_blocking_io(const char *port_name) {
    const int fd = open(port_name, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open serial");
        return -1;
    }

    return fd;
}

int set_serial_port_access_exclusive(const int fd) {
    if (ioctl(fd, TIOCEXCL) < 0) {
        perror("ioctl TIOCEXCL");
        return -1;
    }

    return 0;
}

int set_serial_port_access_nonexclusive(const int fd) {
    if (ioctl(fd, TIOCNXCL) < 0) {
        perror("ioctl TIOCNXCL");
        return -1;
    }

    return 0;
}

int configure_serial_port(const int fd, const cc_t vtime, const cc_t vmin, const speed_t speed) {
    struct termios tty;

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    /*
     * Disable any special handling of received bytes
     * termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
     *
     * Prevent special interpretation of output bytes (e.g. newline chars)
     * termios_p->c_oflag &= ~OPOST;
     *
     * Disable echo, use non-canonical mode, disable interpretation of INTR, QUIT and SUSP
     * termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
     *
     * Clear parity bit, disabling parity, 8 bits per byte
     * termios_p->c_cflag &= ~(CSIZE | PARENB);
     * termios_p->c_cflag |= CS8;
     */
    cfmakeraw(&tty);

    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl

    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = vtime;
    tty.c_cc[VMIN] = vmin;

    // Set in/out baud rate
    cfsetspeed(&tty, speed);

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}
