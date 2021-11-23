#ifdef BUILD_MCU
#include "state_machine.h"
#include "drive.h"
#include "opponent_detection.h"
#include "line_detection.h"
#include "time.h"
#include "sleep.h"
#include "trace.h"
#include "ir_remote.h"
#else
#include "NsumoController/nsumo/state_machine.h"
#include "microcontroller_c_bindings.h"
#include "NsumoController/voltage_lines.h"
#include "NsumoController/nsumo/drive.h"
#include "NsumoController/nsumo/line_detection.h"
#include "NsumoController/nsumo/opponent_detection.h"
#include "NsumoController/nsumo/trace.h"
#endif

#include <stdbool.h>

typedef enum state {
    MAIN_STATE_SEARCH,
    MAIN_STATE_ATTACK,
    MAIN_STATE_RETREAT,
#if BUILD_MCU
    MAIN_STATE_TEST
#endif
} main_state_t;

typedef enum {
    SEARCH_STATE_NONE,
    SEARCH_STATE_ROTATE,
    SEARCH_STATE_FORWARD,
} search_state_t;

typedef enum {
    RETREAT_STATE_NONE,
    RETREAT_STATE_DRIVE_REVERSE,
    RETREAT_STATE_DRIVE_FORWARD,
    RETREAT_STATE_DRIVE_ROTATE_LEFT,
    RETREAT_STATE_DRIVE_ROTATE_RIGHT,
    RETREAT_STATE_DRIVE_ARCTURN_LEFT,
    RETREAT_STATE_DRIVE_ARCTURN_RIGHT,
} retreat_state_t;

typedef enum {
    TEST_STATE_NONE,
    TEST_STATE_DRIVE_REVERSE,
    TEST_STATE_DRIVE_FORWARD,
    TEST_STATE_DRIVE_ROTATE_LEFT,
    TEST_STATE_DRIVE_ROTATE_RIGHT,
} test_state_t;

const char *main_state_str(main_state_t main_state)
{
    switch (main_state)
    {
    case MAIN_STATE_SEARCH: return "MAIN_STATE_SEARCH";
    case MAIN_STATE_ATTACK: return "MAIN_STATE_ATTACK";
    case MAIN_STATE_RETREAT: return "MAIN_STATE_RETREAT";
#if BUILD_MCU
    case MAIN_STATE_TEST: return "MAIN_STATE_TEST";
#endif
    }
    return "";
}

const char *retreat_state_str(retreat_state_t retreat_state)
{
    switch (retreat_state)
    {
    case RETREAT_STATE_NONE: return "RETREAT_STATE_NONE";
    case RETREAT_STATE_DRIVE_REVERSE: return "RETREAT_STATE_DRIVE_REVERSE";
    case RETREAT_STATE_DRIVE_FORWARD: return "RETREAT_STATE_DRIVE_FORWARD";
    case RETREAT_STATE_DRIVE_ROTATE_LEFT: return "RETREAT_STATE_DRIVE_ROTATE_LEFT";
    case RETREAT_STATE_DRIVE_ROTATE_RIGHT: return "RETREAT_STATE_DRIVE_ROTATE_RIGHT";
    case RETREAT_STATE_DRIVE_ARCTURN_LEFT: return "RETREAT_STATE_DRIVE_ARCTURN_LEFT";
    case RETREAT_STATE_DRIVE_ARCTURN_RIGHT: return "RETREAT_STATE_DRIVE_ARCTURN_RIGHT";
    }
    return "";
}

const char *test_state_str(test_state_t test_state)
{
    switch (test_state)
    {
    case TEST_STATE_NONE: return "TEST_STATE_NONE";
    case TEST_STATE_DRIVE_REVERSE: return "TEST_STATE_DRIVE_REVERSE";
    case TEST_STATE_DRIVE_FORWARD: return "TEST_STATE_DRIVE_FORWARD";
    case TEST_STATE_DRIVE_ROTATE_LEFT: return "TEST_STATE_DRIVE_ROTATE_LEFT";
    case TEST_STATE_DRIVE_ROTATE_RIGHT: return "TEST_STATE_DRIVE_ROTATE_RIGHT";
    }
    return "";
}

static uint32_t search_state_start_time = 0;
static void search_timer_start()
{
    search_state_start_time = time_ms();
}

static uint32_t search_timer_passed()
{
    return time_ms() - search_state_start_time;
}

#define SEARCH_STATE_ROTATE_TIMEOUT (1000)
#define SEARCH_STATE_FORWARD_TIMEOUT (3000)
static main_state_t state_machine_search_run(bool first_entry)
{
    static search_state_t current_search_state = SEARCH_STATE_NONE;
    static bool search_state_first_entry = true;
    search_state_t next_search_state = current_search_state;
    if (first_entry) {
        current_search_state = SEARCH_STATE_NONE;
    }
    const volatile line_detection_t line_detection = line_detection_get();
    return MAIN_STATE_SEARCH;
    if (line_detection != LINE_DETECTION_NONE) {
        return MAIN_STATE_RETREAT;
    }
    const enemy_detection_t enemy_detection = ENEMY_DETECTION_NONE;//enemy_detection_get();
    if (enemy_detection & ENEMY_DETECTION_FRONT) {
        return MAIN_STATE_ATTACK;
    }

    switch (current_search_state) {
    case SEARCH_STATE_NONE:
        next_search_state = SEARCH_STATE_ROTATE;
        break;
    case SEARCH_STATE_ROTATE:
        if (search_timer_passed() < SEARCH_STATE_ROTATE_TIMEOUT) {
            if (search_state_first_entry) {
                drive_set(DRIVE_ROTATE_LEFT, DRIVE_SPEED_SLOW);
            }
        } else {
            next_search_state = SEARCH_STATE_FORWARD;
        }
        break;
    case SEARCH_STATE_FORWARD:
        if (search_timer_passed() < SEARCH_STATE_FORWARD_TIMEOUT) {
            if (search_state_first_entry) {
                drive_set(DRIVE_FORWARD, DRIVE_SPEED_FAST);
            }
        } else {
            next_search_state = SEARCH_STATE_ROTATE;
        }
        break;
    }

    if (next_search_state != current_search_state) {
        search_timer_start();
        current_search_state = next_search_state;
        search_state_first_entry = true;
    } else {
        search_state_first_entry = false;
    }
    return MAIN_STATE_SEARCH;
}

static const uint16_t retreat_state_timeouts[] =
{
    [RETREAT_STATE_NONE] = 0,
    [RETREAT_STATE_DRIVE_REVERSE] = 300,
    [RETREAT_STATE_DRIVE_FORWARD] = 300,
    [RETREAT_STATE_DRIVE_ROTATE_LEFT] = 150,
    [RETREAT_STATE_DRIVE_ROTATE_RIGHT] = 150,
    [RETREAT_STATE_DRIVE_ARCTURN_LEFT] = 300,
    [RETREAT_STATE_DRIVE_ARCTURN_RIGHT] = 300,
};

static void set_retreat_drive(retreat_state_t retreat_state)
{
    switch (retreat_state) {
    case RETREAT_STATE_NONE:
        drive_stop();
        break;
    case RETREAT_STATE_DRIVE_REVERSE:
        drive_set(DRIVE_REVERSE, DRIVE_SPEED_MEDIUM);
        break;
    case RETREAT_STATE_DRIVE_FORWARD:
        drive_set(DRIVE_FORWARD, DRIVE_SPEED_MEDIUM);
        break;
    case RETREAT_STATE_DRIVE_ROTATE_LEFT:
        drive_set(DRIVE_ROTATE_LEFT, DRIVE_SPEED_SLOW);
        break;
    case RETREAT_STATE_DRIVE_ROTATE_RIGHT:
        drive_set(DRIVE_ROTATE_RIGHT, DRIVE_SPEED_SLOW);
        break;
    case RETREAT_STATE_DRIVE_ARCTURN_LEFT:
        drive_set(DRIVE_ARCTURN_LEFT, DRIVE_SPEED_FAST);
        break;
    case RETREAT_STATE_DRIVE_ARCTURN_RIGHT:
        drive_set(DRIVE_ARCTURN_RIGHT, DRIVE_SPEED_FAST);
        break;
    }
}

static uint32_t retreat_state_start_time = 0;
static void retreat_timer_start()
{
    retreat_state_start_time = time_ms();
}

static bool is_retreat_state_done(retreat_state_t retreat_state)
{
    return (time_ms() - retreat_state_start_time) >= retreat_state_timeouts[retreat_state];
}

static main_state_t state_machine_retreat_run(bool new_entry)
{
    static retreat_state_t current_retreat_state = RETREAT_STATE_NONE;
    retreat_state_t next_retreat_state = current_retreat_state;
    if (new_entry) {
        current_retreat_state = RETREAT_STATE_NONE;
        next_retreat_state = RETREAT_STATE_NONE;
    }

    const line_detection_t line_detection = line_detection_get();
    switch (line_detection) {
    case LINE_DETECTION_NONE:
        /* Do nothing, instead check time passed below */
        break;
    case LINE_DETECTION_FRONT:
    case LINE_DETECTION_FRONT_LEFT:
    case LINE_DETECTION_FRONT_RIGHT:
        next_retreat_state = RETREAT_STATE_DRIVE_REVERSE;
        break;
    case LINE_DETECTION_BACK:
    case LINE_DETECTION_BACK_LEFT:
    case LINE_DETECTION_BACK_RIGHT:
        next_retreat_state = RETREAT_STATE_DRIVE_FORWARD;
        break;
    case LINE_DETECTION_LEFT:
        next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_RIGHT;
        break;
    case LINE_DETECTION_RIGHT:
        next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_LEFT;
        break;
    }
    /* Keep resetting the time until we no longer detect the line */
    if (line_detection != LINE_DETECTION_NONE) {
        retreat_timer_start();
    }

    if (current_retreat_state != next_retreat_state) {
        current_retreat_state = next_retreat_state;
        set_retreat_drive(current_retreat_state);
    }

    if (is_retreat_state_done(current_retreat_state)) {
        return MAIN_STATE_SEARCH;
    }

    return MAIN_STATE_RETREAT;
}

#ifdef BUILD_MCU
static void handle_test_state(test_state_t test_state)
{
    switch (test_state) {
    case TEST_STATE_NONE:
        TRACE_WARN("Test state is none");
        break;
    case TEST_STATE_DRIVE_REVERSE:
        drive_set(DRIVE_REVERSE, DRIVE_SPEED_FASTEST);
        break;
    case TEST_STATE_DRIVE_FORWARD:
        drive_set(DRIVE_FORWARD, DRIVE_SPEED_FASTEST);
        break;
    case TEST_STATE_DRIVE_ROTATE_LEFT:
        drive_set(DRIVE_ROTATE_LEFT, DRIVE_SPEED_FASTEST);
        break;
    case TEST_STATE_DRIVE_ROTATE_RIGHT:
        drive_set(DRIVE_ROTATE_RIGHT, DRIVE_SPEED_FASTEST);
        break;
    }
}

static main_state_t state_machine_test_run(ir_remote_command_t remote_command)
{
    static test_state_t current_test_state = TEST_STATE_NONE;
    test_state_t next_test_state = current_test_state;

    switch (remote_command) {
    case COMMAND_HASH:
        next_test_state = TEST_STATE_NONE;
        break;
    case COMMAND_UP:
        next_test_state = TEST_STATE_DRIVE_FORWARD;
        break;
    case COMMAND_DOWN:
        next_test_state = TEST_STATE_DRIVE_REVERSE;
        break;
    case COMMAND_LEFT:
        next_test_state = TEST_STATE_DRIVE_ROTATE_LEFT;
        break;
    case COMMAND_RIGHT:
        next_test_state = TEST_STATE_DRIVE_ROTATE_RIGHT;
        break;
    case COMMAND_0:
    case COMMAND_1:
    case COMMAND_2:
    case COMMAND_3:
    case COMMAND_4:
    case COMMAND_5:
    case COMMAND_6:
    case COMMAND_7:
    case COMMAND_8:
    case COMMAND_9:
    case COMMAND_STAR:
    case COMMAND_OK:
        TRACE_WARN("Command not implemented");
        break;
    case COMMAND_NONE:
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
#endif

static void init()
{
    /* TODO: Init time here? */
    enemy_detection_init();
    line_detection_init();
#ifdef BUILD_MCU
    ir_remote_init();
#endif
    drive_init();
}

void state_machine_run()
{
    init();
    main_state_t current_state = MAIN_STATE_SEARCH;
    main_state_t next_state = current_state;
    bool state_new_entry = true;

    while(true) {
#ifdef BUILD_MCU
        ir_remote_command_t remote_command = ir_remote_get_command();
        if (remote_command != COMMAND_NONE) {
            drive_stop();
            current_state = next_state = MAIN_STATE_TEST;
        }
#endif
        switch (current_state)
        {
        case MAIN_STATE_SEARCH: /* Drive around to find the enemy */
            next_state = state_machine_search_run(state_new_entry);
            break;
        case MAIN_STATE_ATTACK: /* Enemy detected so attack it */
        {
            const line_detection_t line_detection = line_detection_get();
            if (line_detection != LINE_DETECTION_NONE) {
                next_state = MAIN_STATE_RETREAT;
                drive_stop();
                break;
            }
            const enemy_detection_t enemy_detection = ENEMY_DETECTION_NONE;//enemy_detection_get();
            if (enemy_detection & ENEMY_DETECTION_FRONT) {
                if (state_new_entry) {
                    drive_set(DRIVE_FORWARD, DRIVE_SPEED_FASTEST);
                }
            } else {
                drive_stop();
                next_state = MAIN_STATE_SEARCH;
            }
            break;
        }
        case MAIN_STATE_RETREAT: /* Line detected so drive away from it */
            next_state = state_machine_retreat_run(state_new_entry);
            break;
#ifdef BUILD_MCU
        case MAIN_STATE_TEST:
            next_state = state_machine_test_run(remote_command);
            break;
#endif
        }

        if (next_state != current_state) {
            current_state = next_state;
            state_new_entry = true;
            trace("New state: %s\n", main_state_str(current_state));
        } else {
            state_new_entry = false;
        }

#ifndef BUILD_MCU
        /* Sleep a bit to offload the host CPU */
        sleep_ms(1);
#endif
    }
}
