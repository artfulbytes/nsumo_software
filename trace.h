#ifndef TRACE_H
#define TRACE_H

#include <stdarg.h>
#include "stdbool.h"

#define TRACE_INFO(fmt, ...) trace("[I] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define TRACE_WARN(fmt, ...) trace("[W] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/* This should be used for printing, because we really don't want to
 * include stdio.h functions when build for the MCU, adds ~5kB and ROM is
 * only 16KB on MSP430G2553. */

bool trace_init(void);

/* You can read the traces from the MCU by hooking up the USB to the Launchpad
 * (make sure you rotate the RXD/TXD bridges 90 degrees), and if you are on Linux,
 * you can then connect with your favorite terminal emulation program
 * (e.g. picocom -b 9600 /dev/ttyACM1). */
void trace(const char *format, ...);

#endif /* TRACE_H */
