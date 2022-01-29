#ifdef BUILD_MCU
#include "state_machine.h"
#include "drive.h"
#include "enemy_detection.h"
#include "line_detection.h"
#include "trace.h"
#include "ir_remote.h"
#include "timer.h"
#else
#include "NsumoController/nsumo/state_machine.h"
#include "microcontroller_c_bindings.h"
#include "NsumoController/voltage_lines.h"
#include "NsumoController/nsumo/drive.h"
#include "NsumoController/nsumo/line_detection.h"
#include "NsumoController/nsumo/enemy_detection.h"
#include "NsumoController/nsumo/trace.h"
#include "NsumoController/nsumo/timer.h"
#endif

#include <stdbool.h>
#include <stddef.h>

#define SM_ASSERT(exp, msg) do { \
    if (!exp) { \
        TRACE_NOPREFIX("Assert:"#exp" %s", msg); \
        drive_stop(); \
        while(1); \
    } \
} while (0)
#define HISTORY_SIZE (8)
#define SEARCH_STATE_ROTATE_TIMEOUT (1000)
#define SEARCH_STATE_FORWARD_TIMEOUT (3000)
#define ATTACK_STATE_TIMEOUT (4000)

typedef enum state {
    MAIN_STATE_SEARCH,
    MAIN_STATE_ATTACK,
    MAIN_STATE_RETREAT,
#ifdef MCU_TEST_STATE
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
    RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT,
    RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT,
} retreat_state_t;

#ifdef MCU_TEST_STATE
typedef enum {
    TEST_STATE_NONE,
    TEST_STATE_DRIVE_REVERSE,
    TEST_STATE_DRIVE_FORWARD,
    TEST_STATE_DRIVE_ROTATE_LEFT,
    TEST_STATE_DRIVE_ROTATE_RIGHT,
} test_state_t;
#endif

typedef struct
{
    search_state_t current_state;
    bool entered_new_state;
    timer_t timer;
} search_state_data_t;

typedef struct
{
    attack_state_t current_state;
    bool entered_new_state;
    timer_t timer;
} attack_state_data_t;

typedef struct
{
    drive_t drive;
    drive_speed_t speed;
    uint16_t duration;
} move_t;

#define MOVE_MAX_CNT (3)
typedef struct {
    move_t moves[MOVE_MAX_CNT];
    uint8_t cnt;
} retreat_moves_t;

typedef struct
{
    retreat_state_t current_state;
    uint8_t move_idx;
    timer_t timer;
} retreat_state_data_t;

typedef struct
{
    line_detection_t line;
    enemy_detection_t enemy;
} detection_t;

typedef struct
{
    detection_t detections[HISTORY_SIZE];
    uint8_t idx;
} history_t;

typedef struct
{
    history_t history;
    search_state_data_t search_data;
    attack_state_data_t attack_data;
    retreat_state_data_t retreat_data;
} state_machine_data_t;

static bool detection_cmp(const detection_t *a, const detection_t *b)
{
    return a->line == b->line &&
           a->enemy.position == b->enemy.position &&
           a->enemy.range == b->enemy.range;
}

static bool valid_history_entry(const detection_t *detection)
{
    return detection->enemy.position != ENEMY_POS_NONE ||
           detection->line != LINE_DETECTION_NONE;
}

static const enemy_detection_t *last_enemy_seen_left_or_right(const history_t *history)
{
    for (uint16_t i = 0; i < HISTORY_SIZE; i++) {
        const uint16_t hist_idx = (i <= history->idx) ?
                                  (history->idx - i) :
                                  (HISTORY_SIZE + history->idx - i);
        if (valid_history_entry(&history->detections[hist_idx])) {
            if (enemy_is_to_left(&history->detections[hist_idx].enemy) ||
                enemy_is_to_right(&history->detections[hist_idx].enemy)) {
                return &(history->detections[hist_idx].enemy);
            }
        } else {
            break;
        }
    }
    return NULL;
}

/*
static void print_history(history_t *history)
{
    trace("===========\n");
    for (uint16_t i = 0; i < HISTORY_SIZE; i++) {
        const uint16_t hist_idx = (i <= history->idx) ?
                                  (history->idx - i) :
                                  (HISTORY_SIZE + history->idx - i);
        if (valid_history_entry(&history->detections[hist_idx])) {
            trace("%s %s\n", line_detection_str(history->detections[hist_idx].line),
                             enemy_pos_str(history->detections[hist_idx].enemy.position));
        } else {
            break;
        }
    }
}
*/

static void save_detection_to_history(history_t *history, const detection_t *detection)
{
    if (!detection_cmp(detection, &history->detections[history->idx])) {
        if (detection->enemy.position != ENEMY_POS_NONE ||
            detection->line != LINE_DETECTION_NONE) {
            history->idx = (history->idx + 1) % HISTORY_SIZE;
            history->detections[history->idx] = *detection;
        }
    }
}

static const char *main_state_str(main_state_t main_state)
{
    static const char* main_state_map[] = {
        [MAIN_STATE_SEARCH] = "ST_SEARCH",
        [MAIN_STATE_ATTACK] = "ST_ATTACK",
        [MAIN_STATE_RETREAT] = "ST_RETREAT",
#ifdef MCU_TEST_STATE
        [MAIN_STATE_TEST] = "ST_TEST",
#endif
    };
    return main_state_map[main_state];
}

static const char *retreat_state_str(retreat_state_t retreat_state)
{
    static const char* retreat_state_map[] = {
        [RETREAT_STATE_NONE] = "RET_NONE",
        [RETREAT_STATE_DRIVE_REVERSE] = "RET_REVERSE",
        [RETREAT_STATE_DRIVE_FORWARD] = "RET_FORWARD",
        [RETREAT_STATE_DRIVE_ROTATE_LEFT] = "RETE_ROTATE_LEFT",
        [RETREAT_STATE_DRIVE_ROTATE_RIGHT] = "RET_ROTATE_RIGHT",
        [RETREAT_STATE_DRIVE_ARCTURN_LEFT] = "RET_ARCTURN_LEFT",
        [RETREAT_STATE_DRIVE_ARCTURN_RIGHT] = "RET_ARCTURN_RIGHT",
        [RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT] = "RET_ALIGN_ENEMY_LEFT",
        [RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT] = "RET_ALIGN_ENEMY_RIGHT",
    };
    return retreat_state_map[retreat_state];
}

#ifdef MCU_TEST_STATE
static const char *test_state_str(test_state_t test_state)
{
    static const char* test_state_map[] = {
        [TEST_STATE_NONE] = "TEST_STATE_NONE",
        [TEST_STATE_DRIVE_REVERSE] = "TEST_STATE_DRIVE_REVERSE",
        [TEST_STATE_DRIVE_FORWARD] = "TEST_STATE_DRIVE_FORWARD",
        [TEST_STATE_DRIVE_ROTATE_LEFT] = "TEST_STATE_DRIVE_ROTATE_LEFT",
        [TEST_STATE_DRIVE_ROTATE_RIGHT] = "TEST_STATE_DRIVE_ROTATE_RIGHT",
    };
    return test_state_map[test_state];
}
#endif

static const retreat_moves_t retreat_moves[] =
{
    [RETREAT_STATE_NONE] = { .cnt = 0 },
    [RETREAT_STATE_DRIVE_REVERSE] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_REVERSE, DRIVE_SPEED_FASTEST, 300 } },
    },
    [RETREAT_STATE_DRIVE_FORWARD] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_FORWARD, DRIVE_SPEED_MEDIUM, 300 } },
    },
    [RETREAT_STATE_DRIVE_ROTATE_LEFT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ROTATE_LEFT, DRIVE_SPEED_MEDIUM, 150 } },
    },
    [RETREAT_STATE_DRIVE_ROTATE_RIGHT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ROTATE_RIGHT, DRIVE_SPEED_MEDIUM, 150 } },
    },
    [RETREAT_STATE_DRIVE_ARCTURN_LEFT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ARCTURN_SHARP_LEFT, DRIVE_SPEED_MEDIUM, 150 } },
    },
    [RETREAT_STATE_DRIVE_ARCTURN_RIGHT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ARCTURN_SHARP_RIGHT, DRIVE_SPEED_MEDIUM, 150 } },
    },
    [RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT] =
    {
        .cnt = 3,
        .moves = {
            [0] = { DRIVE_REVERSE, DRIVE_SPEED_MEDIUM, 300 },
            [1] = { DRIVE_ARCTURN_SHARP_RIGHT, DRIVE_SPEED_MEDIUM, 250 },
            [2] = { DRIVE_ARCTURN_MID_LEFT, DRIVE_SPEED_FAST, 300 },
        },
    },
    [RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT] =
    {
        .cnt = 3,
        .moves = {
            [0] = { DRIVE_REVERSE, DRIVE_SPEED_MEDIUM, 300 },
            [1] = { DRIVE_ARCTURN_SHARP_LEFT, DRIVE_SPEED_MEDIUM, 250 },
            [2] = { DRIVE_ARCTURN_MID_RIGHT, DRIVE_SPEED_FAST, 300 },
        },
    },
};
static void start_retreat_move(retreat_state_data_t *data, const move_t *move)
{
    timer_start(&data->timer);
    drive_set(move->drive, false, move->speed);
}

static const move_t *get_current_retreat_move(retreat_state_data_t *data)
{
    return &retreat_moves[data->current_state].moves[data->move_idx];
}

static bool is_retreat_move_done(retreat_state_data_t *data)
{
    return timer_ms_elapsed(&data->timer) >= get_current_retreat_move(data)->duration;
}

static main_state_t main_state_search(search_state_data_t *search_data, bool entered,
                                      const detection_t *detection, const history_t *hist)
{
    search_state_t next_search_state = search_data->current_state;
    if (entered) {
        timer_start(&search_data->timer);
        search_data->current_state = SEARCH_STATE_ROTATE;
        next_search_state = search_data->current_state;
        search_data->entered_new_state = true;
    }
    if (detection->line != LINE_DETECTION_NONE) {
        return MAIN_STATE_RETREAT;
    }
    if (enemy_detected(&detection->enemy)) {
        return MAIN_STATE_ATTACK;
    }

    switch (search_data->current_state) {
    case SEARCH_STATE_ROTATE:
        if (timer_ms_elapsed(&search_data->timer) < SEARCH_STATE_ROTATE_TIMEOUT) {
            if (search_data->entered_new_state) {
                const enemy_detection_t *last_enemy = last_enemy_seen_left_or_right(hist);
                // TODO: Account for pure left and right too?
                if (last_enemy && enemy_is_to_right(last_enemy)) {
                    drive_set(DRIVE_ROTATE_RIGHT, false, DRIVE_SPEED_FAST);
                } else {
                    drive_set(DRIVE_ROTATE_LEFT, false, DRIVE_SPEED_FAST);
                }
            }
        } else {
            next_search_state = SEARCH_STATE_FORWARD;
        }
        // TODO: Arc turn if detected on left or right (let distance determine sharpness)
        break;
    case SEARCH_STATE_FORWARD:
        if (timer_ms_elapsed(&search_data->timer) < SEARCH_STATE_FORWARD_TIMEOUT) {
            if (search_data->entered_new_state) {
                drive_set(DRIVE_FORWARD, false, DRIVE_SPEED_FASTEST);
            }
        } else {
            next_search_state = SEARCH_STATE_ROTATE;
        }
        break;
    }

    search_data->entered_new_state = (next_search_state != search_data->current_state);
    if (search_data->entered_new_state) {
        timer_start(&search_data->timer);
        search_data->current_state = next_search_state;
    }
    return MAIN_STATE_SEARCH;
}

static main_state_t main_state_attack(attack_state_data_t *attack_data, bool entered, const detection_t *detection)
{
    attack_state_t next_attack_state = attack_data->current_state;

    if (detection->line != LINE_DETECTION_NONE) {
        return MAIN_STATE_RETREAT;
    }
    const enemy_detection_t enemy = detection->enemy;
    if (!enemy_detected(&enemy)) {
        return MAIN_STATE_SEARCH;
    }

    // TODO: Not handling enemy pos left and right even though enemy_detected returns true for those
    if (entered) {
        if (enemy.position == ENEMY_POS_FRONT || enemy.position == ENEMY_POS_FRONT_ALL) {
            attack_data->current_state = ATTACK_STATE_FORWARD;
            // TODO is to left?
        } else if (enemy.position == ENEMY_POS_FRONT_LEFT || enemy.position == ENEMY_POS_FRONT_AND_FRONT_LEFT) {
            attack_data->current_state = ATTACK_STATE_LEFT;
            // TODO is to right?
        } else if (enemy.position == ENEMY_POS_FRONT_RIGHT || enemy.position == ENEMY_POS_FRONT_AND_FRONT_RIGHT) {
            attack_data->current_state = ATTACK_STATE_RIGHT;
        } else {
            // TODO: Undo this
            return MAIN_STATE_SEARCH;
            SM_ASSERT(false, "Unexpected enemy");
        }
        next_attack_state = attack_data->current_state;
        attack_data->entered_new_state = true;
        timer_start(&attack_data->timer);
    }

    switch (attack_data->current_state) {
    case ATTACK_STATE_FORWARD:
        if (enemy.position != ENEMY_POS_FRONT && enemy.position != ENEMY_POS_FRONT_ALL) {
            if (enemy.position == ENEMY_POS_FRONT_LEFT || enemy.position == ENEMY_POS_FRONT_AND_FRONT_LEFT) {
                next_attack_state = ATTACK_STATE_LEFT;
            } else if (enemy.position == ENEMY_POS_FRONT_RIGHT || enemy.position == ENEMY_POS_FRONT_AND_FRONT_RIGHT) {
                next_attack_state = ATTACK_STATE_RIGHT;
            } else {
                SM_ASSERT(false, enemy_pos_str(enemy.position));
            }
            break;
        }
        if (attack_data->entered_new_state) {
            drive_set(DRIVE_FORWARD, false, DRIVE_SPEED_FASTEST);
        }
        break;
    case ATTACK_STATE_LEFT:
        if (enemy.position != ENEMY_POS_FRONT_LEFT && enemy.position != ENEMY_POS_FRONT_AND_FRONT_LEFT) {
            if (enemy.position == ENEMY_POS_FRONT || enemy.position == ENEMY_POS_FRONT_ALL) {
                next_attack_state = ATTACK_STATE_FORWARD;
            } else if (enemy.position == ENEMY_POS_FRONT_RIGHT || enemy.position == ENEMY_POS_FRONT_AND_FRONT_RIGHT) {
                // Unlikely but possible
                next_attack_state = ATTACK_STATE_RIGHT;
            } else {
                SM_ASSERT(false, enemy_pos_str(enemy.position));
            }
            break;
        }
        if (attack_data->entered_new_state) {
            // TODO: Use range to decide drive speed
            drive_set(DRIVE_ARCTURN_WIDE_LEFT, false, DRIVE_SPEED_FASTEST);
        }
        break;
    case ATTACK_STATE_RIGHT:
        if (enemy.position != ENEMY_POS_FRONT_RIGHT && enemy.position != ENEMY_POS_FRONT_AND_FRONT_RIGHT) {
            if (enemy.position == ENEMY_POS_FRONT || enemy.position == ENEMY_POS_FRONT_ALL) {
                next_attack_state = ATTACK_STATE_FORWARD;
            } else if (enemy.position == ENEMY_POS_FRONT_LEFT || enemy.position == ENEMY_POS_FRONT_AND_FRONT_LEFT) {
                // Unlikely but possible
                next_attack_state = ATTACK_STATE_LEFT;
            } else {
                SM_ASSERT(false, enemy_pos_str(enemy.position));
            }
            break;
        }
        if (attack_data->entered_new_state) {
            // TODO: Use range to decide drive speed
            drive_set(DRIVE_ARCTURN_WIDE_RIGHT, false, DRIVE_SPEED_FASTEST);
        }
        break;
    }

    // Stuck in same attack state for long. We might be stuck in a head-to-head
    // battle.
    //     Best way to break out of it? Increase power for a while...
    //     Timeout again? Sharp arc turn reverse
    // TODO: Try to break out of it:
    //    (New attack state ATTACK_STATE_BREAKOUT_FAST_FORWARD) // Could get us to drive out on our own if unlucky...
    //    (New attack state ATTACK_STATE_BREAKOUT_SHARP_LEFT_BACK) // Could get them to drive out on their own actually...
    //
    //
    if (timer_ms_elapsed(&attack_data->timer) > ATTACK_STATE_TIMEOUT) {
        SM_ASSERT(false, "Attack timeout");
    }

    attack_data->entered_new_state = (next_attack_state != attack_data->current_state);
    if (attack_data->entered_new_state) {
        timer_start(&attack_data->timer);
        attack_data->current_state = next_attack_state;
    }
    return MAIN_STATE_ATTACK;
}

static main_state_t main_state_retreat(retreat_state_data_t *retreat_data, bool entered, const detection_t *detection)
{
    retreat_state_t next_retreat_state = retreat_data->current_state;
    if (entered) {
        retreat_data->current_state = RETREAT_STATE_NONE;
        next_retreat_state = RETREAT_STATE_NONE;
    }

    switch (detection->line) {
    case LINE_DETECTION_NONE:
        /* Do nothing, instead check time passed below */
        break;
    case LINE_DETECTION_FRONT_LEFT:
        if (enemy_is_to_right(&detection->enemy) || enemy_is_to_front(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT;
        } else if (enemy_is_to_left(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT;
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_REVERSE;
        }
        break;
    case LINE_DETECTION_FRONT_RIGHT:
        if (enemy_is_to_left(&detection->enemy) && enemy_is_to_front(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT;
        } else if (enemy_is_to_right(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT;
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_REVERSE;
        }
        break;
    case LINE_DETECTION_FRONT:
        if (enemy_is_to_left(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT;
        } else if (enemy_is_to_right(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT;
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_REVERSE;
        }
        break;
    case LINE_DETECTION_BACK:
        next_retreat_state = RETREAT_STATE_DRIVE_FORWARD;
        break;
    case LINE_DETECTION_BACK_LEFT:
        if (get_current_retreat_move(retreat_data)->drive == DRIVE_REVERSE) {
            // 1. Line detected by both sensors on the right before timeout
            //    This means the line is to the left
            next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_RIGHT;
        } else if (retreat_data->current_state == RETREAT_STATE_DRIVE_ARCTURN_RIGHT) {
            // 2. We are still detecting the line to the left,
            //    keep arc-turning
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_FORWARD;
        }
        break;
    case LINE_DETECTION_BACK_RIGHT:
        if (get_current_retreat_move(retreat_data)->drive == DRIVE_REVERSE) {
            // 1. Line detected by both sensors on the left before timeout
            //    This means the line is to the right
            next_retreat_state = RETREAT_STATE_DRIVE_ARCTURN_LEFT;
        } else if (retreat_data->current_state == RETREAT_STATE_DRIVE_ARCTURN_LEFT) {
            // 2. We are still detecting the line to the right,
            //    keep arc-turning
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
        SM_ASSERT(false, "Diagonal");
        break;
    }


    if (retreat_data->current_state != next_retreat_state
        || detection->line != LINE_DETECTION_NONE /* Keep resetting the time until we no longer detect line */
    ) {
        if (retreat_data->current_state != next_retreat_state) {
            TRACE_NOPREFIX("New retreat state %s %s", retreat_state_str(retreat_data->current_state), line_detection_str(detection->line));
        }
        retreat_data->current_state = next_retreat_state;
        retreat_data->move_idx = 0;
        start_retreat_move(retreat_data, &retreat_moves[retreat_data->current_state].moves[retreat_data->move_idx]);
    } else {
        if (is_retreat_move_done(retreat_data)) {
            retreat_data->move_idx++;
            if (retreat_data->move_idx < retreat_moves[retreat_data->current_state].cnt) {
                start_retreat_move(retreat_data, &retreat_moves[retreat_data->current_state].moves[retreat_data->move_idx]);
            } else {
                return MAIN_STATE_SEARCH;
            }
        }
    }

    return MAIN_STATE_RETREAT;
}

#ifdef MCU_TEST_STATE
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

static main_state_t main_state_test(ir_key_t remote_command)
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
#endif

static void init()
{
    enemy_detection_init();
    line_detection_init();
    drive_init();
}

/**
 * Sets up the state machine data, starts the loop and handles the
 * transitions between the main states.
 *
 * Input data is collected once at the beginning of each loop iteration.
 * Therefore, the state machine and its states must NOT contain any blocking code.
 */
void state_machine_run()
{
    init();

#if BUILD_MCU
    /* This should come after sensor initilization so line measurements are
     * ready when we start */
    ir_remote_wait_for_start_signal();
#endif

    main_state_t current_state = MAIN_STATE_SEARCH;
    main_state_t next_state = current_state;
    bool entered_new_state = true;
    state_machine_data_t sm_data = {0};
    while (1) {
        /* Retrieve the input once every loop iteration */
        const detection_t detection =
        {
            .line = line_detection_get(),
            .enemy = enemy_detection_get()
        };

        save_detection_to_history(&sm_data.history, &detection);

#ifdef MCU_TEST_STATE
        ir_remote_init();
        ir_key_t remote_command = ir_remote_get_key();
        if (remote_command != IR_KEY_NONE) {
            drive_stop();
            current_state = next_state = MAIN_STATE_TEST;
        }
#endif
        switch (current_state)
        {
        case MAIN_STATE_SEARCH: /* Drive around to find the enemy */
            next_state = main_state_search(&sm_data.search_data, entered_new_state, &detection, &sm_data.history);
            break;
        case MAIN_STATE_ATTACK: /* Enemy detected so attack it */
        {
            next_state = main_state_attack(&sm_data.attack_data, entered_new_state, &detection);
            break;
        }
        case MAIN_STATE_RETREAT: /* Line detected so drive away from it */
            next_state = main_state_retreat(&sm_data.retreat_data, entered_new_state, &detection);
            break;
#ifdef MCU_TEST_STATE
        case MAIN_STATE_TEST: /* Control with IR remote */
            next_state = main_state_test(remote_command);
            break;
#endif
        }

        if (next_state != current_state) {
            current_state = next_state;
            entered_new_state = true;
            TRACE_NOPREFIX("New state: %s", main_state_str(current_state));
        } else {
            entered_new_state = false;
        }

#ifndef BUILD_MCU
        /* Sleep a bit to offload the host CPU */
        // TODO: Rename to millis?
        sleep_ms(1);
#endif
    }
}
