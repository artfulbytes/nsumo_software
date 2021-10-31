
#ifdef BUILD_MCU
#include "opponent_detection.h"
#include "drivers/vl53l0x.h"
#else /* Simulator */
#include "NsumoController/nsumo/opponent_detection.h"
#include "microcontroller_c_bindings.h"
#include "NsumoController/voltage_lines.h"
#endif

#include <stdbool.h>

#ifdef BUILD_MCU
#else
#define MAX_VOLTAGE_RANGE_SENSOR (1.0f)
#endif

#ifdef BUILD_MCU
typedef struct ranges
{
     uint16_t left;
     uint16_t front_left;
     uint16_t front;
     uint16_t front_right;
     uint16_t right;
} ranges_t;
#endif

uint8_t enemy_detection_get()
{
#ifdef BUILD_MCU
    ranges_t ranges;
    bool success = false;
    success = vl53l0x_read_range_single(VL53L0X_IDX_FRONT, &ranges.front);
    success &= vl53l0x_read_range_single(VL53L0X_IDX_LEFT, &ranges.left);
    success &= vl53l0x_read_range_single(VL53L0X_IDX_RIGHT, &ranges.right);
    success &= vl53l0x_read_range_single(VL53L0X_IDX_FRONT_LEFT, &ranges.front_left);
    success &= vl53l0x_read_range_single(VL53L0X_IDX_FRONT_RIGHT, &ranges.front_right);
    if (!success) {
        return ENEMY_DETECTION_NONE;
    }

    const bool left = ranges.left <= 100;//VL53L0X_OUT_OF_RANGE;
    const bool front_left = ranges.front_left != VL53L0X_OUT_OF_RANGE;
    const bool front = ranges.front <= 300; //!= VL53L0X_OUT_OF_RANGE;
    const bool front_right = ranges.front_right != VL53L0X_OUT_OF_RANGE;
    const bool right = ranges.right <= 100; //!= VL53L0X_OUT_OF_RANGE;
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
