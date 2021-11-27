
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
#define MAX_VOLTAGE_RANGE_SENSOR (1.0f)
#endif

#define DETECT_THRESHOLD (400)

uint8_t enemy_detection_get()
{
#ifdef BUILD_MCU
    vl53l0x_ranges_t ranges;
    bool success = vl53l0x_read_range_multiple(ranges);
    if (!success) {
        return ENEMY_DETECTION_NONE;
    }

    const bool left = ranges[VL53L0X_IDX_LEFT] <= DETECT_THRESHOLD;
    const bool front_left = ranges[VL53L0X_IDX_FRONT_LEFT] <= DETECT_THRESHOLD;
    const bool front = ranges[VL53L0X_IDX_FRONT] <= DETECT_THRESHOLD;
    const bool front_right = ranges[VL53L0X_IDX_FRONT_RIGHT] <= DETECT_THRESHOLD;
    const bool right = ranges[VL53L0X_IDX_RIGHT] <= DETECT_THRESHOLD;
#else
    const bool left = get_voltage(VOLTAGE_LEFT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool front_left = get_voltage(VOLTAGE_FRONT_LEFT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool front = get_voltage(VOLTAGE_FRONT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool front_right = get_voltage(VOLTAGE_FRONT_RIGHT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool right = get_voltage(VOLTAGE_RIGHT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
#endif

    uint8_t enemy_detected = ENEMY_DETECTION_NONE;
    if (left) {
        enemy_detected |= ENEMY_DETECTION_LEFT;
    }
    if (front_left) {
        enemy_detected |= ENEMY_DETECTION_FRONT_LEFT;
    }
    if (front) {
        enemy_detected |= ENEMY_DETECTION_FRONT;
    }
    if (front_right) {
        enemy_detected |= ENEMY_DETECTION_FRONT_RIGHT;
    }
    if (right) {
        enemy_detected |= ENEMY_DETECTION_RIGHT;
    }
    return enemy_detected;
}

void enemy_detection_init()
{
#if BUILD_MCU
    // TODO Return false if fail
    vl53l0x_init();
#endif
}
