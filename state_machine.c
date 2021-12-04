#ifdef BUILD_MCU
#include "state_machine.h"
#include "drive.h"
#include "enemy_detection.h"
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
#include "NsumoController/nsumo/enemy_detection.h"
#include "NsumoController/nsumo/trace.h"
#endif

#include <stdbool.h>

#define SM_ASSERT(exp, msg) do { \
    if (!exp) { \
        TRACE_WARN("Assert:"#exp" %s", msg); \
        drive_stop(); \
        while(1); \
    } \
} while (0)

typedef enum state {
    MAIN_STATE_SEARCH,
    MAIN_STATE_ATTACK,
    MAIN_STATE_RETREAT,
#if BUILD_MCU
    MAIN_STATE_TEST
#endif
} main_state_t;

typedef enum {
    SEARCH_STATE_ROTATE,
    SEARCH_STATE_FORWARD,
} search_state_t;

typedef enum {
    ATTACK_STATE_FORWARD,
    ATTACK_STATE_LEFT,
    ATTACK_STATE_RIGHT,
} attack_state_t;

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

static const char *main_state_str(main_state_t main_state)
{
    switch (main_state)
    {
    case MAIN_STATE_SEARCH: return "MAIN_STATE_SEARCH";
    case MAIN_STATE_ATTACK: return "MAIN_STATE_ATTACK";
    case MAIN_STATE_RETREAT: return "MAIN_STATE_RETREAT";
#ifdef MCU_TEST
    case MAIN_STATE_TEST: return "MAIN_STATE_TEST";
#endif
    }
    return "";
}

static const char *retreat_state_str(retreat_state_t retreat_state)
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

#ifdef MCU_TEST
static const char *test_state_str(test_state_t test_state)
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
#endif

static uint32_t search_state_start_time = 0;
static void search_timer_start()
{
    search_state_start_time = millis();
}

static uint32_t search_timer_elapsed()
{
    return millis() - search_state_start_time;
}

// TODO: Timeout on head-to-head for too long
//     Best way to break out of it? Increase power for a while...
//     Timeout again? Sharp arc turn reverse

//TODO: Get global enemy detect instead...
static bool search_go_to_attack(enemy_pos_t pos)
{
    return pos == ENEMY_POS_FRONT ||
           pos == ENEMY_POS_FRONT_LEFT ||
           pos == ENEMY_POS_FRONT_RIGHT;
}

#define SEARCH_STATE_ROTATE_TIMEOUT (1000)
#define SEARCH_STATE_FORWARD_TIMEOUT (3000)
static main_state_t main_state_search(bool entered)
{
    // TODO: Use history to make better guesses of where to search
    //     Was the enemy on front right last? turn left
    //     Was the enemy on front left last? turn right
    static search_state_t current_search_state = SEARCH_STATE_ROTATE;
    static bool entered_new_search_state = true;
    search_state_t next_search_state = current_search_state;
    if (entered) {
        search_timer_start();
        current_search_state = SEARCH_STATE_ROTATE;
        next_search_state = current_search_state;
        entered_new_search_state = true;
    }
    const volatile line_detection_t line_detection = line_detection_get();
    if (line_detection != LINE_DETECTION_NONE) {
        return MAIN_STATE_RETREAT;
    }

    const enemy_detection_t enemy = enemy_detection_get();
    if (search_go_to_attack(enemy.position)) {
        return MAIN_STATE_ATTACK;
    }

    switch (current_search_state) {
    case SEARCH_STATE_ROTATE:
        if (search_timer_elapsed() < SEARCH_STATE_ROTATE_TIMEOUT) {
            if (entered_new_search_state) {
                drive_set(DRIVE_ROTATE_LEFT, DRIVE_SPEED_MEDIUM);
            }
        } else {
            next_search_state = SEARCH_STATE_FORWARD;
        }
        break;
    case SEARCH_STATE_FORWARD:
        if (search_timer_elapsed() < SEARCH_STATE_FORWARD_TIMEOUT) {
            if (entered_new_search_state) {
                drive_set(DRIVE_FORWARD, DRIVE_SPEED_FAST);
            }
        } else {
            next_search_state = SEARCH_STATE_ROTATE;
        }
        break;
    }

    entered_new_search_state = (next_search_state != current_search_state);
    if (entered_new_search_state) {
        search_timer_start();
        current_search_state = next_search_state;
    }
    return MAIN_STATE_SEARCH;
}

/* TODO: Create more general timer module */
static uint32_t attack_state_start_time = 0;
static void attack_timer_start()
{
    attack_state_start_time = millis();
}
static uint32_t attack_timer_elapsed()
{
    return millis() - attack_state_start_time;
}

static main_state_t check_valid_attack_enemy_pos(enemy_pos_t pos)
{
    if (pos == ENEMY_POS_IMPOSSIBLE) {
        SM_ASSERT(false, "");
    } else if (pos == ENEMY_POS_NONE ||
               pos == ENEMY_POS_LEFT ||
               pos == ENEMY_POS_RIGHT) {
        return MAIN_STATE_SEARCH;
    }
    return MAIN_STATE_ATTACK;
}

#define ATTACK_STATE_TIMEOUT (5000)
static main_state_t main_state_attack(bool entered)
{
    static attack_state_t current_attack_state = ATTACK_STATE_FORWARD;
    attack_state_t next_attack_state = current_attack_state;
    static bool entered_new_attack_state = true;

    // TODO: Volatile necessary?
    const volatile line_detection_t line_detection = line_detection_get();
    if (line_detection != LINE_DETECTION_NONE) {
        return MAIN_STATE_RETREAT;
    }
    const enemy_detection_t enemy = enemy_detection_get();

    // Sanity check....
    const main_state_t new_main_state = check_valid_attack_enemy_pos(enemy.position);
    if (MAIN_STATE_ATTACK != new_main_state) {
        return new_main_state;
    }

    if (entered) {
        if (enemy.position == ENEMY_POS_FRONT) {
            current_attack_state = ATTACK_STATE_FORWARD;
        } else if (enemy.position == ENEMY_POS_FRONT_LEFT) {
            current_attack_state = ATTACK_STATE_LEFT;
        } else if (enemy.position == ENEMY_POS_FRONT_RIGHT) {
            current_attack_state = ATTACK_STATE_RIGHT;
        } else {
            SM_ASSERT(false, "");
        }
        next_attack_state = current_attack_state;
        entered_new_attack_state = true;
        attack_timer_start();
    }

    switch (current_attack_state) {
    case ATTACK_STATE_FORWARD:
        if (enemy.position != ENEMY_POS_FRONT) {
            if (enemy.position == ENEMY_POS_FRONT_LEFT) {
                next_attack_state = ATTACK_STATE_LEFT;
            } else if (enemy.position == ENEMY_POS_FRONT_RIGHT) {
                next_attack_state = ATTACK_STATE_RIGHT;
            } else {
                SM_ASSERT(false, "");
            }
            break;
        }
        if (entered_new_attack_state) {
            drive_set(DRIVE_FORWARD, DRIVE_SPEED_FAST);
        }
        break;
    case ATTACK_STATE_LEFT:
        if (enemy.position != ENEMY_POS_FRONT_LEFT) {
            if (enemy.position == ENEMY_POS_FRONT) {
                next_attack_state = ATTACK_STATE_FORWARD;
            } else if (enemy.position == ENEMY_POS_FRONT_RIGHT) {
                // Unlikely but possible
                next_attack_state = ATTACK_STATE_RIGHT;
            } else {
                SM_ASSERT(false, "");
            }
            break;
        }
        if (entered_new_attack_state) {
            // TODO: Dist -> Drive speed
            drive_set(DRIVE_ARCTURN_LEFT, DRIVE_SPEED_FAST);
        }
        break;
    case ATTACK_STATE_RIGHT:
        if (enemy.position != ENEMY_POS_FRONT_RIGHT) {
            if (enemy.position == ENEMY_POS_FRONT) {
                next_attack_state = ATTACK_STATE_FORWARD;
            } else if (enemy.position == ENEMY_POS_FRONT_LEFT) {
                // Unlikely but possible
                next_attack_state = ATTACK_STATE_LEFT;
            } else {
                SM_ASSERT(false, enemy_pos_str(enemy.position));
            }
            break;
        }
        if (entered_new_attack_state) {
            // TODO: Dist -> Drive speed
            drive_set(DRIVE_ARCTURN_RIGHT, DRIVE_SPEED_FAST);
        }
        break;
    }

    // Stuck in same attack state for long. We might be stuck in a head-to-head
    // battle.
    // TODO: Try to break out of it:
    //    (New attack state ATTACK_STATE_BREAKOUT_FAST_FORWARD) // Could get us to drive out on our own if unlucky...
    //    (New attack state ATTACK_STATE_BREAKOUT_SHARP_LEFT_BACK) // Could get them to drive out on their own actually...
    //
    //
    if (attack_timer_elapsed() > ATTACK_STATE_TIMEOUT) {
        SM_ASSERT(false, "Attack timeout");
    }

    entered_new_attack_state = (next_attack_state != current_attack_state);
    if (entered_new_attack_state) {
        attack_timer_start();
        current_attack_state = next_attack_state;
    }
    return MAIN_STATE_ATTACK;
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
    retreat_state_start_time = millis();
}

static bool is_retreat_state_done(retreat_state_t retreat_state)
{
    return (millis() - retreat_state_start_time) >= retreat_state_timeouts[retreat_state];
}

static main_state_t main_state_retreat(bool entered)
{
    static retreat_state_t current_retreat_state = RETREAT_STATE_NONE;
    retreat_state_t next_retreat_state = current_retreat_state;
    if (entered) {
        current_retreat_state = RETREAT_STATE_NONE;
        next_retreat_state = RETREAT_STATE_NONE;
    }

    // TODO: No enemy detection in this state?
    trace("Retreat state %s\n", retreat_state_str(current_retreat_state));

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
        next_retreat_state = RETREAT_STATE_DRIVE_FORWARD;
        break;
    case LINE_DETECTION_BACK_LEFT:
        if (current_retreat_state == RETREAT_STATE_DRIVE_REVERSE) {
            // 1. Line detected by both sensors on the right before timeout
            //    This means the line is to the right
            next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_LEFT;
        } else if (current_retreat_state == RETREAT_STATE_DRIVE_ARCTURN_LEFT) {
            // 2. We are still detecting the line to the right
            //    Keep driving
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_FORWARD;
        }
        break;
    case LINE_DETECTION_BACK_RIGHT:
        if (current_retreat_state == RETREAT_STATE_DRIVE_REVERSE) {
            // 1. Line detected by both sensors on the left before timeout
            //    This means the line is to the right
            next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_RIGHT;
        } else if (current_retreat_state == RETREAT_STATE_DRIVE_ARCTURN_RIGHT) {
            // 2. We are still detecting the line to the right
            //    Keep driving
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_FORWARD;
        }
        break;
    case LINE_DETECTION_LEFT:
        next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_RIGHT;
        break;
    case LINE_DETECTION_RIGHT:
        next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_LEFT;
        break;
    case LINE_DETECTION_DIAGONAL_LEFT:
    case LINE_DETECTION_DIAGONAL_RIGHT:
        // Weird but possible state
        // TODO: It's ambigiuous if the dohyo is in front or back. Must use
        // detect history for best guess.
        SM_ASSERT(false, "");
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

#ifdef MCU_TEST
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

static main_state_t main_state_test(ir_remote_command_t remote_command)
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
#ifdef MCU_TEST
    ir_remote_init();
#endif
    drive_init();
}

void state_machine_run()
{
    init();
    main_state_t current_state = MAIN_STATE_SEARCH;
    main_state_t next_state = current_state;
    bool entered_new_state = true;

    while(true) {
        // TODO: Comment that no state is allowed to block...
        // TODO: Retrive stuff globally here every round, makes it easier to save history etc...
        // TODO: Save to static variable and have wrapper functions in this file...
#ifdef MCU_TEST
        ir_remote_command_t remote_command = ir_remote_get_command();
        if (remote_command != COMMAND_NONE) {
            drive_stop();
            current_state = next_state = MAIN_STATE_TEST;
        }
#endif
        switch (current_state)
        {
        case MAIN_STATE_SEARCH: /* Drive around to find the enemy */
            next_state = main_state_search(entered_new_state);
            break;
        case MAIN_STATE_ATTACK: /* Enemy detected so attack it */
        {
            next_state = main_state_attack(entered_new_state);
            break;
        }
        case MAIN_STATE_RETREAT: /* Line detected so drive away from it */
            next_state = main_state_retreat(entered_new_state);
            break;
#ifdef MCU_TEST
        case MAIN_STATE_TEST:
            next_state = main_state_test(remote_command);
            break;
#endif
        }

        if (next_state != current_state) {
            current_state = next_state;
            entered_new_state = true;
            trace("New state: %s\n", main_state_str(current_state));
        } else {
            entered_new_state = false;
        }

#ifndef BUILD_MCU
        /* Sleep a bit to offload the host CPU */
        sleep_ms(1);
#endif
    }
}
