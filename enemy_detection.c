
#ifdef BUILD_MCU
#include "enemy_detection.h"
#include "drivers/vl53l0x.h"
#else /* Simulator */
#include "NsumoController/nsumo/enemy_detection.h"
#include "microcontroller_c_bindings.h"
#include "NsumoController/voltage_lines.h"
#endif

#include <stdbool.h>

#ifdef BUILD_MCU
#else
//#define MAX_VOLTAGE_RANGE_SENSOR (1.0f)
// Should match max range of the sumobot spec
#define VOLTAGE_TO_RANGE (800)
#endif

// TODO If deffa for simulator
#define DETECT_THRESHOLD (400) // mm
#define INVALID_RANGE (8190)

#define RANGE_CLOSE (100) // mm
#define RANGE_MID (200) // mm
#define RANGE_FAR (300) // mm

const char *enemy_pos_str(enemy_pos_t pos)
{
    switch (pos)
    {
    case ENEMY_POS_NONE:        return "ENEMY_POS_NONE       ";
    case ENEMY_POS_FRONT_LEFT:  return "ENEMY_POS_FRONT_LEFT ";
    case ENEMY_POS_FRONT:       return "ENEMY_POS_FRONT      ";
    case ENEMY_POS_FRONT_RIGHT: return "ENEMY_POS_FRONT_RIGHT";
    case ENEMY_POS_LEFT:        return "ENEMY_POS_LEFT       ";
    case ENEMY_POS_RIGHT:       return "ENEMY_POS_RIGHT      ";
    case ENEMY_POS_IMPOSSIBLE:  return "ENEMY_POS_IMPOSSIBLE ";
    }
    return "";
}

const char *enemy_range_str(enemy_range_t range)
{
    switch (range)
    {
    case ENEMY_RANGE_NONE: return  "ENEMY_RANGE_NONE ";
    case ENEMY_RANGE_CLOSE: return "ENEMY_RANGE_CLOSE";
    case ENEMY_RANGE_MID: return   "ENEMY_RANGE_MID  ";
    case ENEMY_RANGE_FAR: return   "ENEMY_RANGE_FAR  ";
    }
    return "";
}

enemy_detection_t enemy_detection_get()
{
    enemy_detection_t detection = { 0 };
#ifdef BUILD_MCU
    vl53l0x_ranges_t ranges;
    bool success = vl53l0x_read_range_multiple(ranges);
    if (!success) {
        return detection;
    }

    const uint16_t range_left = ranges[VL53L0X_IDX_LEFT];
    const uint16_t range_front_left = ranges[VL53L0X_IDX_FRONT_LEFT];
    const uint16_t range_front = ranges[VL53L0X_IDX_FRONT];
    const uint16_t range_front_right = ranges[VL53L0X_IDX_FRONT_RIGHT];
    const uint16_t range_right = ranges[VL53L0X_IDX_RIGHT];
#else
    const uint16_t range_left = get_voltage(VOLTAGE_LEFT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    const uint16_t range_front_left = get_voltage(VOLTAGE_FRONT_LEFT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    const uint16_t range_front = get_voltage(VOLTAGE_FRONT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    const uint16_t range_front_right = get_voltage(VOLTAGE_FRONT_RIGHT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
    const uint16_t range_right = get_voltage(VOLTAGE_RIGHT_RANGE_SENSOR) * VOLTAGE_TO_RANGE;
#endif
    const bool left = range_left < DETECT_THRESHOLD;
    const bool front_left = range_front_left < DETECT_THRESHOLD;
    const bool front = range_front < DETECT_THRESHOLD;
    const bool front_right = range_front_right < DETECT_THRESHOLD;
    const bool right = range_right < DETECT_THRESHOLD;

    uint16_t range = INVALID_RANGE;
    if (front_left && front && front_right) {
        detection.position = ENEMY_POS_FRONT;
        // Average
        range = ((((range_left + range_front) >> 2) + range_right) >> 2);
    } else if (front_left && front_right) {
        detection.position = ENEMY_POS_IMPOSSIBLE;
    } else if (front_left) {
        // TODO Impossible left right
        // TODO Should I account for front range here?
        detection.position = ENEMY_POS_FRONT_LEFT;
        range = range_front_left;
    } else if (front_right) {
        // TODO Impossible left right
        // TODO Should I account for front range here?
        detection.position = ENEMY_POS_FRONT_RIGHT;
        range = range_front_right;
    } else if (front) {
        if (left || right) {
            detection.position = ENEMY_POS_IMPOSSIBLE;
        } else {
            detection.position = ENEMY_POS_FRONT;
            range = range_front;
        }
    } else if (left && right) {
        detection.position = ENEMY_POS_IMPOSSIBLE;
    } else if (left) {
        detection.position = ENEMY_POS_LEFT;
        range = range_left;
    } else if (right) {
        detection.position = ENEMY_POS_RIGHT;
        range = range_right;
    } else {
        detection.position = ENEMY_POS_NONE;
    }

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

void enemy_detection_init()
{
#if BUILD_MCU
    // TODO Return false if fail
    vl53l0x_init();
#endif
}
