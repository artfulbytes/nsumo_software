#include "detection_history.h"
#include "enemy_detection.h"

#include <stddef.h>
#include <stdint.h>

static bool detection_cmp(const detection_t *a, const detection_t *b)
{
    return a->line == b->line && a->enemy.position == b->enemy.position
        && a->enemy.range == b->enemy.range;
}

static bool valid_history_entry(const detection_t *detection)
{
    return detection->enemy.position != ENEMY_POS_NONE || detection->line != LINE_DETECTION_NONE;
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

void detection_history_save(detection_history_t *history, const detection_t *detection)
{
    if (!detection_cmp(detection, &history->detections[history->idx])) {
        if (detection->enemy.position != ENEMY_POS_NONE || detection->line != LINE_DETECTION_NONE) {
            history->idx = (history->idx + 1) % HISTORY_SIZE;
            history->detections[history->idx] = *detection;
        }
    }
}

const enemy_detection_t *detection_history_last_enemy_direction(const detection_history_t *history)
{
    for (uint16_t i = 0; i < HISTORY_SIZE; i++) {
        const uint16_t hist_idx =
            (i <= history->idx) ? (history->idx - i) : (HISTORY_SIZE + history->idx - i);
        if (valid_history_entry(&history->detections[hist_idx])) {
            if (enemy_at_left(&history->detections[hist_idx].enemy)
                || enemy_at_right(&history->detections[hist_idx].enemy)) {
                return &(history->detections[hist_idx].enemy);
            }
        } else {
            break;
        }
    }
    return NULL;
}
