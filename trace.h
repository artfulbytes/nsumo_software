#ifndef TRACE_H
#define TRACE_H

#include "stdbool.h"
#include <stdarg.h>

/* This should be used for printing, because we really don't want to
 * include stdio.h functions when build for the MCU, adds ~5kB and ROM is
 * only 16KB on MSP430G2553. */

#define TRACE_NOPREFIX(fmt, ...) TRACE(fmt "\n", ##__VA_ARGS__)
#define TRACE_INFO(fmt, ...) TRACE("[I] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define TRACE_WARN(fmt, ...) TRACE("[W] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#ifdef TRACE_DISABLED
#define TRACE(fmt, ...)
#define TRACE_INIT() (true)
#else
#define TRACE(fmt, ...) trace(fmt, ##__VA_ARGS__);
#define TRACE_INIT() trace_init()
#endif

/* NOTE: Don't use this function directly, use TRACE_INIT macro! */
bool trace_init(void);

/* You can read the traces from the MCU by hooking up the USB to the Launchpad
 * (make sure you rotate the RXD/TXD bridges 90 degrees), and if you are on Linux,
 * you can then connect with your favorite terminal emulation program
 * (e.g. picocom -b 9600 /dev/ttyACM1).
 *
 * NOTE: Don't use this function directly, use the macros above!
 */
void trace(const char *format, ...);

#endif /* TRACE_H */
