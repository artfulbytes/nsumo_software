#ifndef ENEMY_DETECTION_H
#define ENEMY_DETECTION_H

#include <stdint.h>

typedef enum {
    ENEMY_POSITION_UNKNOWN,
    ENEMY_POSITION_LEFT,
    ENEMY_POSITION_FRONT_LEFT,
    ENEMY_POSITION_FRONT,
    ENEMY_POSITION_FRONT_RIGHT,
    ENEMY_POSITION_RIGHT
} enemy_position_t;

typedef struct enemy_detection {
    enemy_position_t position;
    uint16_t distance;
} enemy_detection_t;

void enemy_detection_init(void);
enemy_detection_t enemy_detection_get(void);

#endif /* ENEMY_DETECTION_H */
