#include "line_detection.h"
#if BUILD_MCU
#include "drivers/qre1113.h"
#define LINE_SENSOR_VOLTAGE_THRESHOLD (700)
#else // Simulator
#include "NsumoController/voltage_lines.h"
#include "microcontroller_c_bindings.h"
#define LINE_SENSOR_VOLTAGE_THRESHOLD (0.0f)
#endif

#include <stdbool.h>

const char *line_detection_str(line_detection_t line_detection)
{
    static const char *line_detection_str_map[] = { [LINE_DETECTION_NONE] = "NONE",
                                                    [LINE_DETECTION_FRONT] = "FRONT",
                                                    [LINE_DETECTION_BACK] = "BACK",
                                                    [LINE_DETECTION_FRONT_LEFT] = "FRONT_LEFT",
                                                    [LINE_DETECTION_BACK_LEFT] = "BACK_LEFT",
                                                    [LINE_DETECTION_FRONT_RIGHT] = "FRONT_RIGHT",
                                                    [LINE_DETECTION_BACK_RIGHT] = "BACK_RIGHT",
                                                    [LINE_DETECTION_LEFT] = "LEFT",
                                                    [LINE_DETECTION_RIGHT] = "RIGHT",
                                                    [LINE_DETECTION_DIAGONAL_LEFT] = "DIAGONALLEFT",
                                                    [LINE_DETECTION_DIAGONAL_RIGHT] =
                                                        "DIAGONAL_RIGHT" };
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
#else // Simulator
    const bool frontLeft =
        get_voltage(VOLTAGE_FRONT_LEFT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool frontRight =
        get_voltage(VOLTAGE_FRONT_RIGHT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool backLeft =
        get_voltage(VOLTAGE_BACK_LEFT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
    const bool backRight =
        get_voltage(VOLTAGE_BACK_RIGHT_LINE_DETECTOR) > LINE_SENSOR_VOLTAGE_THRESHOLD;
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
