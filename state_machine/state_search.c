#include "state_search.h"
#include "timer.h"

#define SEARCH_STATE_ROTATE_TIMEOUT (1000)
#define SEARCH_STATE_FORWARD_TIMEOUT (3000)

main_state_t main_state_search(search_state_data_t *search_data, bool entered,
                               const detection_t *detection, const detection_history_t *hist)
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
                const enemy_detection_t *last_enemy = detection_history_last_enemy_direction(hist);
                if (last_enemy && enemy_at_right(last_enemy)) {
                    drive_set(DRIVE_ROTATE_RIGHT, false, DRIVE_SPEED_FAST);
                } else {
                    drive_set(DRIVE_ROTATE_LEFT, false, DRIVE_SPEED_FAST);
                }
            }
        } else {
            next_search_state = SEARCH_STATE_FORWARD;
        }
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
