#include "trace.h"
#ifdef BUILD_MCU
#include "printf_config.h"
#include "printf.h"
#include "uart.h"
#else /* SIMULATOR */
#include "stdio.h"
#endif /* SIMULATOR */

static bool initialized = false;

bool trace_init()
{
    if (initialized) {
        return false;
    }
#ifdef BUILD_MCU
    if (!uart_init()) {
        return false;
    }
#endif
    initialized = true;
    return true;
}

void trace(const char *format, ...)
{
    if (!initialized) {
        return;
    }
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
