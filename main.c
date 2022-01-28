#include "hw.h"
#include "state_machine.h"
#include "trace.h"
#include "test.h"

int main(void)
{
    hw_init();
    if (!TRACE_INIT()) {
        while (1);
    }
    TRACE_NOPREFIX("Booted");

    state_machine_run();

    // TODO: Disable everything and endless loop in case of failure
    while (1);
}
