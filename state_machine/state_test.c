#include "state_test.h"
#include "trace.h"

typedef enum
{
    TEST_STATE_NONE,
    TEST_STATE_DRIVE_REVERSE,
    TEST_STATE_DRIVE_FORWARD,
    TEST_STATE_DRIVE_ROTATE_LEFT,
    TEST_STATE_DRIVE_ROTATE_RIGHT,
} test_state_t;

static const char *test_state_str(test_state_t test_state)
{
    static const char *test_state_map[] = {
        [TEST_STATE_NONE] = "TEST_STATE_NONE",
        [TEST_STATE_DRIVE_REVERSE] = "TEST_STATE_DRIVE_REVERSE",
        [TEST_STATE_DRIVE_FORWARD] = "TEST_STATE_DRIVE_FORWARD",
        [TEST_STATE_DRIVE_ROTATE_LEFT] = "TEST_STATE_DRIVE_ROTATE_LEFT",
        [TEST_STATE_DRIVE_ROTATE_RIGHT] = "TEST_STATE_DRIVE_ROTATE_RIGHT",
    };
    return test_state_map[test_state];
}

static void handle_test_state(test_state_t test_state)
{
    switch (test_state) {
    case TEST_STATE_NONE:
        TRACE_WARN("Test state is none");
        break;
    case TEST_STATE_DRIVE_REVERSE:
        drive_set(DRIVE_REVERSE, false, DRIVE_SPEED_FASTEST);
        break;
    case TEST_STATE_DRIVE_FORWARD:
        drive_set(DRIVE_FORWARD, false, DRIVE_SPEED_FASTEST);
        break;
    case TEST_STATE_DRIVE_ROTATE_LEFT:
        drive_set(DRIVE_ROTATE_LEFT, false, DRIVE_SPEED_FASTEST);
        break;
    case TEST_STATE_DRIVE_ROTATE_RIGHT:
        drive_set(DRIVE_ROTATE_RIGHT, false, DRIVE_SPEED_FASTEST);
        break;
    }
}

main_state_t main_state_test(ir_key_t remote_command)
{
    static test_state_t current_test_state = TEST_STATE_NONE;
    test_state_t next_test_state = current_test_state;

    switch (remote_command) {
    case IR_KEY_HASH:
        next_test_state = TEST_STATE_NONE;
        break;
    case IR_KEY_UP:
        next_test_state = TEST_STATE_DRIVE_FORWARD;
        break;
    case IR_KEY_DOWN:
        next_test_state = TEST_STATE_DRIVE_REVERSE;
        break;
    case IR_KEY_LEFT:
        next_test_state = TEST_STATE_DRIVE_ROTATE_LEFT;
        break;
    case IR_KEY_RIGHT:
        next_test_state = TEST_STATE_DRIVE_ROTATE_RIGHT;
        break;
    case IR_KEY_0:
    case IR_KEY_1:
    case IR_KEY_2:
    case IR_KEY_3:
    case IR_KEY_4:
    case IR_KEY_5:
    case IR_KEY_6:
    case IR_KEY_7:
    case IR_KEY_8:
    case IR_KEY_9:
    case IR_KEY_STAR:
    case IR_KEY_OK:
        TRACE_WARN("Command not implemented");
        break;
    case IR_KEY_NONE:
        break;
    }

    if (next_test_state != current_test_state) {
        drive_stop();
        current_test_state = next_test_state;
        TRACE_INFO("New test state: %s", test_state_str(current_test_state));
    }

    if (current_test_state != TEST_STATE_NONE) {
        handle_test_state(current_test_state);
    } else {
        /* Leave the test state machine */
        drive_stop();
        return MAIN_STATE_SEARCH;
    }

    return MAIN_STATE_TEST;
}
