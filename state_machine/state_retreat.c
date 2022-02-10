#include "state_retreat.h"
#include "trace.h"
#include "timer.h"

#define MOVE_MAX_CNT (3)
typedef struct
{
    move_t moves[MOVE_MAX_CNT];
    uint8_t cnt;
} retreat_moves_t;

static const char *retreat_state_str(retreat_state_t retreat_state)
{
    static const char *retreat_state_map[] = {
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
        .moves = { [0] = { DRIVE_FORWARD, DRIVE_SPEED_FAST, 300 } },
    },
    [RETREAT_STATE_DRIVE_ROTATE_LEFT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ROTATE_LEFT, DRIVE_SPEED_FAST, 150 } },
    },
    [RETREAT_STATE_DRIVE_ROTATE_RIGHT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ROTATE_RIGHT, DRIVE_SPEED_FAST, 150 } },
    },
    [RETREAT_STATE_DRIVE_ARCTURN_LEFT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ARCTURN_SHARP_LEFT, DRIVE_SPEED_FASTEST, 150 } },
    },
    [RETREAT_STATE_DRIVE_ARCTURN_RIGHT] =
    {
        .cnt = 1,
        .moves = { [0] = { DRIVE_ARCTURN_SHARP_RIGHT, DRIVE_SPEED_FASTEST, 150 } },
    },
    [RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT] =
    {
        .cnt = 3,
        .moves = {
            [0] = { DRIVE_REVERSE, DRIVE_SPEED_FASTEST, 300 },
            [1] = { DRIVE_ARCTURN_SHARP_RIGHT, DRIVE_SPEED_FASTEST, 250 },
            [2] = { DRIVE_ARCTURN_MID_LEFT, DRIVE_SPEED_FASTEST, 300 },
        },
    },
    [RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT] =
    {
        .cnt = 3,
        .moves = {
            [0] = { DRIVE_REVERSE, DRIVE_SPEED_FASTEST, 300 },
            [1] = { DRIVE_ARCTURN_SHARP_LEFT, DRIVE_SPEED_FASTEST, 250 },
            [2] = { DRIVE_ARCTURN_MID_RIGHT, DRIVE_SPEED_FASTEST, 300 },
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

main_state_t main_state_retreat(retreat_state_data_t *retreat_data, bool entered,
                                const detection_t *detection)
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
        if (enemy_at_right(&detection->enemy) || enemy_at_front(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT;
        } else if (enemy_at_left(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT;
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_REVERSE;
        }
        break;
    case LINE_DETECTION_FRONT_RIGHT:
        if (enemy_at_left(&detection->enemy) || enemy_at_front(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT;
        } else if (enemy_at_right(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_RIGHT;
        } else {
            next_retreat_state = RETREAT_STATE_DRIVE_REVERSE;
        }
        break;
    case LINE_DETECTION_FRONT:
        if (enemy_at_left(&detection->enemy)) {
            next_retreat_state = RETREAT_STATE_DRIVE_ALIGN_ENEMY_LEFT;
        } else if (enemy_at_right(&detection->enemy)) {
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
        || detection->line
            != LINE_DETECTION_NONE /* Keep resetting the time until we no longer detect line */
    ) {
        if (retreat_data->current_state != next_retreat_state) {
            TRACE_NOPREFIX("New retreat state %s %s", retreat_state_str(next_retreat_state),
                           line_detection_str(detection->line));
        }
        retreat_data->current_state = next_retreat_state;
        retreat_data->move_idx = 0;
        start_retreat_move(
            retreat_data,
            &retreat_moves[retreat_data->current_state].moves[retreat_data->move_idx]);
    } else {
        if (is_retreat_move_done(retreat_data)) {
            retreat_data->move_idx++;
            if (retreat_data->move_idx < retreat_moves[retreat_data->current_state].cnt) {
                start_retreat_move(
                    retreat_data,
                    &retreat_moves[retreat_data->current_state].moves[retreat_data->move_idx]);
            } else {
                return MAIN_STATE_SEARCH;
            }
        }
    }

    return MAIN_STATE_RETREAT;
}
