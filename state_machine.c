#include "state_machine.h"
#include "drive.h"
#include "enemy_detection.h"
#include "line_detection.h"
#include "time.h"
#include "stdbool.h"

typedef enum
{
    MAIN_STATE_SEARCH,
    MAIN_STATE_ATTACK,
    MAIN_STATE_RETREAT,
    MAIN_STATE_TEST,
} main_state_t;

typedef enum {
    RETREAT_STATE_NONE,
    RETREAT_STATE_DRIVE_BACK,
    RETREAT_STATE_DRIVE_FORWARD,
    RETREAT_STATE_DRIVE_ROTATE_LEFT,
    RETREAT_STATE_DRIVE_ROTATE_RIGHT,
    RETREAT_STATE_DRIVE_ARCTURN_LEFT,
    RETREAT_STATE_DRIVE_ARCTURN_RIGHT,
} retreat_state_t;

static const bool retreat_state_timeouts[] =
{
    [RETREAT_STATE_NONE] = 0,
    [RETREAT_STATE_DRIVE_BACK] = 300,
    [RETREAT_STATE_DRIVE_FORWARD] = 300,
    [RETREAT_STATE_DRIVE_ROTATE_LEFT] = 150,
    [RETREAT_STATE_DRIVE_ROTATE_RIGHT] = 150,
    [RETREAT_STATE_DRIVE_ARCTURN_LEFT] = 250,
    [RETREAT_STATE_DRIVE_ARCTURN_RIGHT] = 250,
};

static uint32_t retreat_state_start_time = 0;
static retreat_state_t current_retreat_state = RETREAT_STATE_NONE;

static void set_retreat_drive(retreat_state_t retreat_state)
{
    switch (RETREAT_STATE_NONE) {
    case RETREAT_STATE_NONE:
        break;
    case RETREAT_STATE_DRIVE_BACK:
        break;
    case RETREAT_STATE_DRIVE_FORWARD:
        break;
    case RETREAT_STATE_DRIVE_ROTATE_LEFT:
        break;
    case RETREAT_STATE_DRIVE_ROTATE_RIGHT:
        break;
    case RETREAT_STATE_DRIVE_ARCTURN_LEFT:
        break;
    case RETREAT_STATE_DRIVE_ARCTURN_RIGHT:
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
        next_retreat_state = RETREAT_STATE_DRIVE_BACK;
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
        reset_retreat_state_start_time();
    }

    if (is_retreat_state_done()) {
        current_retreat_state = RETREAT_STATE_NONE;
        /* TODO: Make it possible to do two retreat states after each other, get_next_retreat_state...
         * and keep int of how many retreat states we been in, we are only done when this is 2 (or 1 for forward and back...)
         * reset this count as soon as we detect a line */
    }

    if (current_retreat_state != RETREAT_STATE_NONE) {
        return MAIN_STATE_RETREAT;
    } else {
        retreat_state_start_time = 0;
        /* TODO: What if we detect enemy? */
        return MAIN_STATE_SEARCH;
    }
}

static void init()
{
    /* TODO: Init time here? */
    enemy_detection_init();
    line_detection_init();
}

void state_machine_run()
{
    init();

    main_state_t current_state = MAIN_STATE_SEARCH;
    main_state_t next_state = current_state;
    while (1) {
        switch (current_state) {
        case MAIN_STATE_SEARCH:
            break;
        case MAIN_STATE_ATTACK:
            break;
        case MAIN_STATE_RETREAT:
            next_state = state_machine_retreat_run();
            break;
        case MAIN_STATE_TEST:
            break;
        }
        current_state = next_state;
    }
}
