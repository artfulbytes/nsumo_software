#ifndef STATE_ATTACK_H
#define STATE_ATTACK_H

#include "state_common.h"

typedef uint32_t timer_t;

typedef enum
{
    ATTACK_STATE_FORWARD,
    ATTACK_STATE_LEFT,
    ATTACK_STATE_RIGHT,
    ATTACK_STATE_NONE,
} attack_state_t;

typedef struct
{
    attack_state_t current_state;
    timer_t timer;
} attack_state_data_t;

main_state_t main_state_attack(attack_state_data_t *attack_data, bool entered,
                               const detection_t *detection);

#endif // STATE_ATTACK_H
