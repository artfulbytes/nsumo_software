#include "hw.h"
#include "state_machine.h"
#include "trace.h"
#include "test.h"

int main(void)
{
    hw_init();
    trace("Booted\n");
    state_machine_run();
}
