#include "hw.h"
#include "state_machine.h"
#include "trace.h"
#include "test.h"

int main(void)
{
    hw_init();
    if (!trace_init()) {
        while (1);
    }
    trace("Booted\n");

    state_machine_run();
    //test_vl53l0x_multiple();
    //test_state_machine_ir();
    //test_drives_remote();
    // TODO: Disable everything and endless loop in case of failure
    while (1);
}
