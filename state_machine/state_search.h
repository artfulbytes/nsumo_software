#ifndef STATE_SEARCH_H
#define STATE_SEARCH_H

#include "state_common.h"
#include "detection_history.h"

typedef uint32_t timer_t;

typedef enum
{
    SEARCH_STATE_ROTATE,
    SEARCH_STATE_FORWARD,
} search_state_t;

typedef struct
{
    search_state_t current_state;
    // TODO: Rework like attack state so we can remove this:
    bool entered_new_state;
    timer_t timer;
} search_state_data_t;

main_state_t main_state_search(search_state_data_t *search_data, bool entered,
                               const detection_t *detection, const detection_history_t *hist);

#endif // STATE_SEARCH_H
