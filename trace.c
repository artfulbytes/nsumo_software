#include "trace.h"
#ifdef BUILD_MCU
#include "printf_config.h"
#include "printf.h"
#else /* SIMULATOR */
#include "stdio.h"
#endif /* SIMULATOR */

void trace(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

