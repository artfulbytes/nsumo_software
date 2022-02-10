#include "state_attack.h"
#include "trace.h"
#include "timer.h"

#define ATTACK_STATE_TIMEOUT (4000)

static attack_state_t enemy_detect_to_attack_state(const enemy_detection_t *enemy)
{
    if (enemy_at_front(enemy)) {
        return ATTACK_STATE_FORWARD;
    } else if (enemy_at_left(enemy)) {
        return ATTACK_STATE_LEFT;
    } else if (enemy_at_right(enemy)) {
        return ATTACK_STATE_RIGHT;
    } else {
        return ATTACK_STATE_NONE;
    }
}

main_state_t main_state_attack(attack_state_data_t *attack_data, bool entered,
                               const detection_t *detection)
{
    if (detection->line != LINE_DETECTION_NONE) {
        return MAIN_STATE_RETREAT;
    }
    const attack_state_t next_attack_state = enemy_detect_to_attack_state(&detection->enemy);
    if (next_attack_state == ATTACK_STATE_NONE) {
        return MAIN_STATE_SEARCH;
    }

    if (entered || attack_data->current_state != next_attack_state) {
        attack_data->current_state = next_attack_state;
        timer_start(&attack_data->timer);
    } else if (timer_ms_elapsed(&attack_data->timer) > ATTACK_STATE_TIMEOUT) {
        // Stuck in same attack state for long. We might be stuck in a head-to-head
        // battle.
        //     Best way to break out of it? Increase power for a while...
        //     Timeout again? Sharp arc turn reverse
        // TODO: Try to break out of it:
        //    (New attack state ATTACK_STATE_BREAKOUT_FAST_FORWARD) // Could get us to drive out on
        //    our own if unlucky... (New attack state ATTACK_STATE_BREAKOUT_SHARP_LEFT_BACK) //
        //    Could get them to drive out on their own actually...
        //
        //
        SM_ASSERT(false, "Attack timeout");
    } else {
        return MAIN_STATE_ATTACK;
    }

    switch (attack_data->current_state) {
    case ATTACK_STATE_FORWARD:
        drive_set(DRIVE_FORWARD, false, DRIVE_SPEED_FASTEST);
        break;
    case ATTACK_STATE_RIGHT:
        // TODO: Use range to decide drive speed
        drive_set(DRIVE_ARCTURN_WIDE_RIGHT, false, DRIVE_SPEED_FASTEST);
        break;
    case ATTACK_STATE_LEFT:
        // TODO: Use range to decide drive speed
        drive_set(DRIVE_ARCTURN_WIDE_LEFT, false, DRIVE_SPEED_FASTEST);
        break;
    case ATTACK_STATE_NONE:
        SM_ASSERT(false, "Unexpected state");
        return MAIN_STATE_SEARCH;
    }

    return MAIN_STATE_ATTACK;
}
