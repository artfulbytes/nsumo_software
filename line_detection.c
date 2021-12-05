#if BUILD_MCU
#include "line_detection.h"
#include "drivers/qre1113.h"
#else
#include "line_detection.h"
#include "NsumoController/voltage_lines.h"
#include "microcontroller_c_bindings.h"
#endif

#include <stdbool.h>

#if BUILD_MCU
#define LINE_SENSOR_VOLTAGE_THRESHOLD (700)
#else
#define LINE_SENSOR_VOLTAGE_THRESHOLD (0.0f)
#endif

// TODO: Make this a function instead...
static char* line_detection_str_map[] =
{
    [LINE_DETECTION_NONE] = "LINE_DETECTION_NONE",
    [LINE_DETECTION_FRONT] = "LINE_DETECTION_FRONT",
    [LINE_DETECTION_BACK] = "LINE_DETECTION_BACK",
    [LINE_DETECTION_FRONT_LEFT] = "LINE_DETECTION_FRONT_LEFT",
    [LINE_DETECTION_BACK_LEFT] = "LINE_DETECTION_BACK_LEFT",
    [LINE_DETECTION_FRONT_RIGHT] = "LINE_DETECTION_FRONT_RIGHT",
    [LINE_DETECTION_BACK_RIGHT] = "LINE_DETECTION_BACK_RIGHT",
    [LINE_DETECTION_LEFT] = "LINE_DETECTION_LEFT",
    [LINE_DETECTION_RIGHT] = "LINE_DETECTION_RIGHT",
    [LINE_DETECTION_DIAGONAL_LEFT] = "LINE_DETECTION_DIAGONALLEFT",
    [LINE_DETECTION_DIAGONAL_RIGHT] = "LINE_DETECTION_DIAGONAL_RIGHT"
};

char* line_detection_str(line_detection_t line_detection)
{
    return line_detection_str_map[line_detection];
}

line_detection_t line_detection_get()
{
#if BUILD_MCU
    qre1113_voltages_t voltages;
    qre1113_get_voltages(&voltages);
    const bool frontLeft = voltages.front_left < LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool frontRight = voltages.front_right < LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool backLeft = voltages.back_left < LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool backRight = voltages.back_right < LINE_SENSOR_VOLTAGE_THRESHOLD;
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
        } else if (backRight) {
            return LINE_DETECTION_DIAGONAL_LEFT;
        } else {
            return LINE_DETECTION_FRONT_LEFT;
        }
    } else if (frontRight) {
        if (backRight) {
            return LINE_DETECTION_RIGHT;
        } else if (backLeft) {
            return LINE_DETECTION_DIAGONAL_RIGHT;
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
#if BUILD_MCU
    qre1113_init();
#endif
}
