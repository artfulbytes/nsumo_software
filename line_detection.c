#if BUILD_MCU
#include "line_detection.h"
#else
#include "NsumoController/nsumo/line_detection.h"
#include "NsumoController/voltage_lines.h"
#include "microcontroller_c_bindings.h"
#endif

#include <stdbool.h>

#define LINE_SENSOR_VOLTAGE_THRESHOLD (0.0f)

line_detection_t line_detection_get()
{
#if BUILD_MCU
    const bool frontLeft = false;
    const bool frontRight = false;
    const bool backLeft = false;
    const bool backRight = false;
#else
    const bool frontLeft = get_voltage(VOLTAGE_FRONT_LEFT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool frontRight = get_voltage(VOLTAGE_FRONT_RIGHT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool backLeft = get_voltage(VOLTAGE_BACK_LEFT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool backRight = get_voltage(VOLTAGE_BACK_RIGHT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
#endif

    if (frontLeft) {
        if (frontRight) {
            return LINE_DETECTION_FRONT;
        } else if (backLeft) {
            return LINE_DETECTION_LEFT;
        } else {
            return LINE_DETECTION_FRONT_LEFT;
        }
    } else if (frontRight) {
        if (backRight) {
            return LINE_DETECTION_RIGHT;
        } else {
            return LINE_DETECTION_FRONT_RIGHT;
        }
    } else if (backLeft) {
        if (backRight) {
            return LINE_DETECTION_BACK;
        } else {
            return LINE_DETECTION_BACK_LEFT;
        }
    } else if (backRight) {
        return LINE_DETECTION_BACK_RIGHT;
    }
    return LINE_DETECTION_NONE;
}

void line_detection_init()
{
    /* TODO: Init line sensors */
}
