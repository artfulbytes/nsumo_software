#ifdef BUILD_MCU
#include "enemy_detection.h"
#else /* Simulator */
#include "Sumobot4WheelExample/mini-sumobot/enemy_detection.h"
#include "microcontroller_c_bindings.h"
#include "Sumobot4WheelExample/voltage_lines.h"
#endif

#include <stdbool.h>

#define MAX_VOLTAGE_RANGE_SENSOR (1.0f)

uint8_t enemy_detection_get()
{
#ifdef BUILD_MCU
    const bool detectedLeft = false;
    const bool detectedFrontLeft = false;
    const bool detectedFront = false;
    const bool detectedFrontRight = false;
    const bool detectedRight = false;
#else
    const bool detectedLeft = get_voltage(VOLTAGE_LEFT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool detectedFrontLeft = get_voltage(VOLTAGE_FRONT_LEFT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool detectedFront = get_voltage(VOLTAGE_FRONT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool detectedFrontRight = get_voltage(VOLTAGE_FRONT_RIGHT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
    const bool detectedRight = get_voltage(VOLTAGE_RIGHT_RANGE_SENSOR) < MAX_VOLTAGE_RANGE_SENSOR;
#endif

    uint8_t enemyDetected = ENEMY_DETECTION_NONE;
    if (detectedLeft) {
        enemyDetected |= ENEMY_DETECTION_LEFT;
    }
    if (detectedFrontLeft) {
        enemyDetected |= ENEMY_DETECTION_FRONT_LEFT;
    }
    if (detectedFront) {
        enemyDetected |= ENEMY_DETECTION_FRONT;
    }
    if (detectedFrontRight) {
        enemyDetected |= ENEMY_DETECTION_FRONT_RIGHT;
    }
    if (detectedRight) {
        enemyDetected |= ENEMY_DETECTION_RIGHT;
    }
    return enemyDetected;
}

void enemy_detection_init()
{
}
