
#ifdef BUILD_MCU
#include "state_machine.h"
#include "drive.h"
#include <opponent_detection.h>
#include <line_detection.h>
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
    MAIN_STATE_SEARCH_1,
    MAIN_STATE_SEARCH_2,
    MAIN_STATE_ATTACK,
    MAIN_STATE_RETREAT,
    MAIN_STATE_TEST
} main_state_t;

typedef enum {
    RETREAT_STATE_NONE,
    RETREAT_STATE_DRIVE_REVERSE,
    RETREAT_STATE_DRIVE_FORWARD,
    RETREAT_STATE_DRIVE_ROTATE_LEFT,
    RETREAT_STATE_DRIVE_ROTATE_RIGHT,
    RETREAT_STATE_DRIVE_ARCTURN_LEFT,
    RETREAT_STATE_DRIVE_ARCTURN_RIGHT,
} retreat_state_t;

const char *main_state_str(main_state_t main_state)
{
    switch (main_state)
    {
    case MAIN_STATE_SEARCH_1: return "MAIN_STATE_SEARCH_1";
    case MAIN_STATE_SEARCH_2: return "MAIN_STATE_SEARCH_2";
    case MAIN_STATE_ATTACK: return "MAIN_STATE_ATTACK";
    case MAIN_STATE_RETREAT: return "MAIN_STATE_RETREAT";
    case MAIN_STATE_TEST: return "MAIN_STATE_TEST";
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

static uint32_t retreat_state_start_time = 0;
static retreat_state_t current_retreat_state = RETREAT_STATE_NONE;

static void set_retreat_drive(retreat_state_t retreat_state)
{
    switch (retreat_state) {
    case RETREAT_STATE_NONE:
        drive_stop();
        break;
    case RETREAT_STATE_DRIVE_REVERSE:
        drive_set(DRIVE_REVERSE, DRIVE_SPEED_SLOW);
        break;
    case RETREAT_STATE_DRIVE_FORWARD:
        drive_set(DRIVE_FORWARD, DRIVE_SPEED_SLOW);
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

static void reset_retreat_state_start_time()
{
    retreat_state_start_time = time_ms();
}

static bool is_retreat_state_done()
{
    return (time_ms() - retreat_state_start_time) >= retreat_state_timeouts[current_retreat_state];
}

/* TODO: Comment... Goal of the retreat machine is to ... */
/* TODO: Comment each state too */
/* TODO: Handle the case when we detect line front right/left<->back right/left in
 * quick succession, so we don't get stuck! */
static main_state_t state_machine_retreat_run()
{
    retreat_state_t next_retreat_state = current_retreat_state;
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

    current_retreat_state = next_retreat_state;

    /* Keep resetting the time until we no longer detect the line */
    if (line_detection != LINE_DETECTION_NONE) {
        reset_retreat_state_start_time();
    }

    if (is_retreat_state_done()) {
        current_retreat_state = RETREAT_STATE_NONE;
        /* TODO: Make it possible to do two retreat states after each other, get_next_retreat_state...
         * and keep int of how many retreat states we been in, we are only done when this is 2 (or 1 for forward and back...)
         * reset this count as soon as we detect a line */
    }

    set_retreat_drive(current_retreat_state);
    if (current_retreat_state != RETREAT_STATE_NONE) {
        return MAIN_STATE_RETREAT;
    }

    retreat_state_start_time = 0;
    /* TODO: What if we detect enemy? */
    return MAIN_STATE_SEARCH_1;
}

typedef enum {
    TEST_STATE_NONE,
    TEST_STATE_DRIVE_REVERSE,
    TEST_STATE_DRIVE_FORWARD,
    TEST_STATE_DRIVE_ROTATE_LEFT,
    TEST_STATE_DRIVE_ROTATE_RIGHT,
} test_state_t;


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


#ifdef BUILD_MCU
/* TODO: Move to new file? */
static test_state_t current_test_state = TEST_STATE_NONE;

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
#endif

#ifdef BUILD_MCU
static main_state_t state_machine_test_run(ir_remote_command_t remote_command)
{
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
        return MAIN_STATE_SEARCH_1;
    }


    return MAIN_STATE_TEST;
}
#endif

static void init()
{
    /* TODO: Init time here? */

    // TODO: Uncomment when enemy detection works properly
    //enemy_detection_init();
    line_detection_init();
#ifdef BUILD_MCU
    ir_remote_init();
#endif
    drive_init();
}

bool is_enemy_out(line_detection_t line_detection, enemy_detection_t enemy_detection)
{
    return (enemy_detection & (ENEMY_DETECTION_FRONT | ENEMY_DETECTION_FRONT_LEFT | ENEMY_DETECTION_FRONT_RIGHT)) &&
           (line_detection == LINE_DETECTION_FRONT);
}

/* TODO: Create functions to print the enum values */
/* "Block" sleep here to make sure we get away from the line */
/* TODO: We shouldn't block sleep here */
#define MAIN_STATE_SEARCH_1_TIME_MS (1500)
#define MAIN_STATE_SEARCH_2_TIME_MS (5000)

void state_machine_run()
{
    init();
    main_state_t current_state = MAIN_STATE_SEARCH_1;
    main_state_t next_state = current_state;
    uint32_t time_at_state_change = time_ms();

    while(true) {
        /* To be able to gather the variables up here only requires us to not allow for
         * any longer sleep inside the state machine (TODO: make sure there is none)) */
        /* Say also that this is a bonus with havning no sleep and continuosly going around, we
         * don't need to gather variables over and over */
        /* TODO: Define all sleep constants as defines! */
        line_detection_t line_detection = line_detection_get();
        // TODO: Retrieve enemy detection here once it works
        enemy_detection_t enemy_detection = enemy_detection_get();
#ifdef BUILD_MCU
        ir_remote_command_t remote_command = ir_remote_get_command();
        if (remote_command != COMMAND_NONE) {
            drive_stop();
            current_state = next_state = MAIN_STATE_TEST;
        }
#endif
        switch (current_state)
        {
        case MAIN_STATE_SEARCH_1:
            /* TODO: Handle line detection here, we might be pushed by the enemy */
            if (enemy_detection & ENEMY_DETECTION_FRONT) {
                next_state = MAIN_STATE_ATTACK;
                drive_stop();
                break;
            }
            if (line_detection != LINE_DETECTION_NONE || (time_ms() - time_at_state_change > MAIN_STATE_SEARCH_1_TIME_MS)) {
                next_state = MAIN_STATE_SEARCH_2;
                break;
            }
            drive_set(DRIVE_ROTATE_RIGHT, DRIVE_SPEED_SLOW);

            break;
        case MAIN_STATE_SEARCH_2: /* Drive around to find the enemy */
            if (enemy_detection & ENEMY_DETECTION_FRONT) {
                next_state = MAIN_STATE_ATTACK;
                drive_stop();
                break;
            }
            if (line_detection == LINE_DETECTION_NONE) {
                drive_set(DRIVE_FORWARD, DRIVE_SPEED_FASTEST);
            } else {
                drive_stop();
                next_state = MAIN_STATE_RETREAT;
                break;
            }
            uint32_t time = time_ms();
            if (time - time_at_state_change > MAIN_STATE_SEARCH_2_TIME_MS) {
                next_state = MAIN_STATE_SEARCH_1;
                break;
            }

            break;
        case MAIN_STATE_ATTACK:
            if (line_detection != LINE_DETECTION_NONE) {
                next_state = MAIN_STATE_RETREAT;
                drive_stop();
                break;
            }
            if (enemy_detection & ENEMY_DETECTION_FRONT) {
                drive_set(DRIVE_FORWARD, DRIVE_SPEED_FASTEST);
            } else {
                drive_stop();
                next_state = MAIN_STATE_SEARCH_1;
            }
            /* TODO: Timeout..., we may be stuck in a head on battle... */
            break;
        case MAIN_STATE_RETREAT:
            next_state = state_machine_retreat_run();
            /* TODO: This is a bit dangerous, we may be going between retreat manuevers and never
             * getting to RETREAT_NONE, e.g. if we drive back and forth detecting line before
             * retreat maneuver "timeouts", perhaps detect such behaviour as line to left/right, and
             * do arcturn manuever. Basically, if retreat manuever is 200ms drive back, then we should
             * see any back<->forth detection that happens under 200ms as line being to left/right...*/
            /* TODO:Should handle having an enemy and a line detected... */
            break;
        case MAIN_STATE_TEST:
#ifdef BUILD_MCU
            next_state = state_machine_test_run(remote_command);
#endif
            break;
        }

        if (is_enemy_out(line_detection, enemy_detection)) {
            trace("Enemy out!\n");
        }

        if (next_state != current_state) {
            time_at_state_change = time_ms();
            current_state = next_state;
            trace("New state: %s\n", main_state_str(current_state));
        }

        /* Sleep a bit to offload the host CPU :) */
#ifndef BUILD_MCU
        sleep_ms(1);
#endif
    }
}
