#include "hw.h"
#include "state_machine.h"
#include "trace.h"
#include "test.h"

int main(void)
{
    hw_init();
    //TRACE_INFO("Booted");

    test_vl53l0x();
    //state_machine_run();

    // TODO: Disable everything and endless loop in case of failure
}
