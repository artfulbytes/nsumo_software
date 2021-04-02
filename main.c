#include "hw.h"
#include "state_machine.h"

void main(void)
{
    hw_init();
    state_machine_run();
}
