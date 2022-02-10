#ifndef STATE_RETREAT_H
#define STATE_RETREAT_H

#include "state_common.h"

typedef uint32_t timer_t;

typedef enum
{
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

typedef struct
{
    retreat_state_t current_state;
    uint8_t move_idx;
    timer_t timer;
} retreat_state_data_t;

main_state_t main_state_retreat(retreat_state_data_t *retreat_data, bool entered,
                                const detection_t *detection);

#endif // STATE_RETREAT_H
