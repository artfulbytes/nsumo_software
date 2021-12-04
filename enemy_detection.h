#ifndef ENEMY_DETECTION_H
#define ENEMY_DETECTION_H

#include <stdint.h>

typedef enum enemy_pos {
    ENEMY_POS_NONE,
    ENEMY_POS_FRONT_LEFT,
    ENEMY_POS_FRONT,
    ENEMY_POS_FRONT_RIGHT,
    ENEMY_POS_LEFT,
    ENEMY_POS_RIGHT,
    ENEMY_POS_IMPOSSIBLE // Keep this for debugging
} enemy_pos_t;

typedef enum enemy_range {
    ENEMY_RANGE_NONE,
    ENEMY_RANGE_CLOSE,
    ENEMY_RANGE_MID,
    ENEMY_RANGE_FAR,
} enemy_range_t;

typedef struct enemy_detection
{
    enemy_pos_t position;
    enemy_range_t range;
} enemy_detection_t;

enemy_detection_t enemy_detection_get(void);
void enemy_detection_init(void);
const char *enemy_pos_str(enemy_pos_t pos);
const char *enemy_range_str(enemy_range_t range);

#endif /* ENEMY_DETECTION_H */
