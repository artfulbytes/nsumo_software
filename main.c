#include "hw.h"
#include "state_machine.h"
#include "trace.h"
#ifdef TEST
#include "test.h"
#endif

int main(void)
{
    hw_init();
    if (!TRACE_INIT()) {
        while (1)
            ;
    }
    TRACE_NOPREFIX("Booted");

#ifdef TEST
    // Select the test to run by defining TEST (e.g. TEST=test_dimming_led)
    TEST();
#else
    state_machine_run();
#endif

    // TODO: Disable everything and endless loop in case of failure
    while (1)
        ;
}
