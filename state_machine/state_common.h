#ifndef STATE_COMMON_H
#define STATE_COMMON_H

#include "enemy_detection.h"
#include "line_detection.h"
#include "drive.h"
#include <stdint.h>

#define SM_ASSERT(exp, msg)                                                                        \
    do {                                                                                           \
        if (!exp) {                                                                                \
            TRACE_NOPREFIX("Assert:" #exp " %s", msg);                                             \
            drive_stop();                                                                          \
            while (1)                                                                              \
                ;                                                                                  \
        }                                                                                          \
    } while (0)

typedef enum state
{
    MAIN_STATE_SEARCH,
    MAIN_STATE_ATTACK,
    MAIN_STATE_RETREAT,
    MAIN_STATE_TEST
} main_state_t;

typedef struct
{
    line_detection_t line;
    enemy_detection_t enemy;
} detection_t;

typedef struct
{
    drive_t drive;
    drive_speed_t speed;
    uint16_t duration;
} move_t;

#endif // STATE_COMMON_H
