#include "enemy_detection.h"

#ifdef BUILD_MCU
#include "drivers/vl53l0x.h"
#else // Simulator
#include "microcontroller_c_bindings.h"
#include "NsumoController/voltage_lines.h"
#endif

#include <stdbool.h>

#ifndef BUILD_MCU
// Should match max range of the sumobot spec
#define VOLTAGE_TO_RANGE (800)
#endif

#define DETECT_THRESHOLD (600) // mm
#define INVALID_RANGE (8190)
#define RANGE_CLOSE (100) // mm
#define RANGE_MID (200) // mm
#define RANGE_FAR (300) // mm

const char *enemy_pos_str(enemy_pos_t pos)
{
    static const char *enemy_pos_str_map[] = {
        [ENEMY_POS_NONE] = "NONE",
        [ENEMY_POS_FRONT_LEFT] = "FRONT_LEFT",
        [ENEMY_POS_FRONT] = "FRONT",
        [ENEMY_POS_FRONT_RIGHT] = "FRONT_RIGHT",
        [ENEMY_POS_LEFT] = "LEFT",
        [ENEMY_POS_RIGHT] = "RIGHT",
        [ENEMY_POS_FRONT_AND_FRONT_LEFT] = "FRONT_AND_FRONT_LEFT",
        [ENEMY_POS_FRONT_AND_FRONT_RIGHT] = "FRONT_AND_FRONT_RIGHT",
        [ENEMY_POS_FRONT_ALL] = "FRONT_ALL",
        [ENEMY_POS_IMPOSSIBLE] = "IMPOSSIBLE",
    };
    return enemy_pos_str_map[pos];
}

const char *enemy_range_str(enemy_range_t range)
{
    static const char *enemy_range_str_map[] = {
        [ENEMY_RANGE_NONE] = "NONE",
        [ENEMY_RANGE_CLOSE] = "CLOSE",
        [ENEMY_RANGE_MID] = "MID",
        [ENEMY_RANGE_FAR] = "FAR",
    };
    return enemy_range_str_map[range];
}

enemy_detection_t enemy_detection_get()
{
    enemy_detection_t detection = { 0 };
#ifdef BUILD_MCU
    vl53l0x_ranges_t ranges;
    bool fresh_values = false;
    bool success = vl53l0x_read_range_multiple(ranges, &fresh_values);
    if (!success) {
        return detection;
    }

    // const uint16_t range_left = ranges[VL53L0X_IDX_LEFT];
    const uint16_t range_front_left = ranges[VL53L0X_IDX_FRONT_LEFT];
    const uint16_t range_front = ranges[VL53L0X_IDX_FRONT];
    const uint16_t range_front_right = ranges[VL53L0X_IDX_FRONT_RIGHT];
    // const uint16_t range_right = ranges[VL53L0X_IDX_RIGHT];
#else // Simulator
    // const uint16_t range_left = get_voltage(VOLTAGE_LEFT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    const uint16_t range_front_left =
        get_voltage(VOLTAGE_FRONT_LEFT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    const uint16_t range_front = get_voltage(VOLTAGE_FRONT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    const uint16_t range_front_right =
        get_voltage(VOLTAGE_FRONT_RIGHT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    // const uint16_t range_right = get_voltage(VOLTAGE_RIGHT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
#endif
    // const bool left = range_left < DETECT_THRESHOLD;
    const bool front_left = range_front_left < DETECT_THRESHOLD;
    const bool front = range_front < DETECT_THRESHOLD;
    const bool front_right = range_front_right < DETECT_THRESHOLD;
    // const bool right = range_right < DETECT_THRESHOLD;

    uint16_t range = INVALID_RANGE;
    detection.position = ENEMY_POS_IMPOSSIBLE;
    /*
        if (left) {
            if (front_right || right) {
                // Impossible
            } else {
                // TODO: Return something else if also front_left or front
                detection.position = ENEMY_POS_LEFT;
                range = range_left;
            }
        } else if (right) {
            if (front_left || left) {
                // Impossible
            } else {
                // TODO: Return something else if also front_left or front
                detection.position = ENEMY_POS_RIGHT;
                range = range_right;
            }
        } else if (front_left && front && front_right) {
    */
    if (front_left && front && front_right) {
        detection.position = ENEMY_POS_FRONT_ALL;
        // Average
        range = ((((range_front_left + range_front) / 2) + range_front_right) / 2);
    } else if (front_left && front_right) {
        // Impossible
    } else if (front_left) {
        if (front) {
            detection.position = ENEMY_POS_FRONT_AND_FRONT_LEFT;
            // Average
            range = (range_front_left + range_front) / 2;
        } else {
            detection.position = ENEMY_POS_FRONT_LEFT;
            range = range_front_left;
        }
    } else if (front_right) {
        if (front) {
            detection.position = ENEMY_POS_FRONT_AND_FRONT_RIGHT;
            // Average
            range = (range_front_right + range_front) / 2;
        } else {
            detection.position = ENEMY_POS_FRONT_RIGHT;
            range = range_front_right;
        }
    } else if (front) {
        detection.position = ENEMY_POS_FRONT;
        range = range_front;
    } else {
        detection.position = ENEMY_POS_NONE;
    }

    // Convert range value to enum
    if (range != INVALID_RANGE) {
        if (range < RANGE_CLOSE) {
            detection.range = ENEMY_RANGE_CLOSE;
        } else if (range < RANGE_MID) {
            detection.range = ENEMY_RANGE_MID;
        } else {
            detection.range = ENEMY_RANGE_FAR;
        }
    }
    return detection;
}

bool enemy_detected(const enemy_detection_t *enemy)
{
    return enemy->position != ENEMY_POS_NONE && enemy->position != ENEMY_POS_IMPOSSIBLE;
}

bool enemy_at_left(const enemy_detection_t *enemy)
{
    return enemy->position == ENEMY_POS_LEFT || enemy->position == ENEMY_POS_FRONT_LEFT
        || enemy->position == ENEMY_POS_FRONT_AND_FRONT_LEFT;
}

bool enemy_at_right(const enemy_detection_t *enemy)
{
    return enemy->position == ENEMY_POS_RIGHT || enemy->position == ENEMY_POS_FRONT_RIGHT
        || enemy->position == ENEMY_POS_FRONT_AND_FRONT_RIGHT;
}

bool enemy_at_front(const enemy_detection_t *enemy)
{
    return enemy->position == ENEMY_POS_FRONT || enemy->position == ENEMY_POS_FRONT_ALL;
}

void enemy_detection_init()
{
#if BUILD_MCU
    // TODO Return false if fail
    vl53l0x_init();
#endif
}
