#ifndef ENEMY_DETECTION_H
#define ENEMY_DETECTION_H

#include <stdint.h>

/* Enum can be used as bitwise flags */
typedef enum enemy_detection {
    ENEMY_DETECTION_NONE = 0,
    ENEMY_DETECTION_LEFT = 1,
    ENEMY_DETECTION_FRONT_LEFT = 2,
    ENEMY_DETECTION_FRONT = 4,
    ENEMY_DETECTION_FRONT_RIGHT = 8,
    ENEMY_DETECTION_RIGHT = 16
} enemy_detection_t;

/* TODO: Rework: Need to retrieve distances as well...*/
uint8_t enemy_detection_get(void);
void enemy_detection_init(void);

#endif /* ENEMY_DETECTION_H */
