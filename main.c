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
    TRACE_INFO("Booted");

    //test_state_machine_ir();
    //test_vl53l0x();
    //test_enemy_detection();
    //test_line_detection();
    //test_vl53l0x();
    //test_vl53l0x_multiple();
    test_qre1113();
    //state_machine_run();
    //test_state_machine_ir();
    // TODO: Disable everything and endless loop in case of failure
    while (1);
}
