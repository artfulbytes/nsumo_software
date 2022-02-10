#ifndef DETECTION_HISTORY_H
#define DETECTION_HISTORY_H

// TODO: Shouldn't be needed...
#include "state_common.h"

#define HISTORY_SIZE (8)
typedef struct
{
    detection_t detections[HISTORY_SIZE];
    uint8_t idx;
} detection_history_t;

// TODO: Comment these...
void detection_history_save(detection_history_t *history, const detection_t *detection);
const enemy_detection_t *detection_history_last_enemy_direction(const detection_history_t *history);

#endif // DETECTION_HISTORY_H
