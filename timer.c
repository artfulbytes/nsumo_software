#include "timer.h"
#ifdef BUILD_MCU
#include "drivers/millis.h"
#else
#include "microcontroller_c_bindings.h"
#endif

void timer_start(timer_t *timer) { *timer = millis(); }

uint32_t timer_ms_elapsed(const timer_t *timer)
{
    if (*timer) {
        return millis() - *timer;
    } else {
        return 0;
    }
}
