#if BUILD_MCU
#include "drive.h"
#include "motor.h"
#include "trace.h"
#else
#include "NsumoController/nsumo/drive.h"
#include "NsumoController/nsumo/drivers/motor.h"
#endif

#define DUTY_CYCLE_STOP (0)
#define DUTY_CYCLE_SLOW (20)
#define DUTY_CYCLE_MEDIUM (40)
#define DUTY_CYCLE_FAST (70)
#define DUTY_CYCLE_FASTEST (100)

#define ARCTURN_DIFF_CONSTANT (0.375f)

#if 0
static const char *drive_str(drive_t drive)
{
    switch (drive)
    {
    case STOP: return "STOP";
    case FORWARD: return "FORWARD";
    case REVERSE: return "REVERSE";
    case ROTATE_LEFT: return "ROTATE_LEFT";
    case ROTATE_RIGHT: return "ROTATE_RIGHT";
    case ARCTURN_LEFT: return "ARCTURN_LEFT";
    case ARCTURN_RIGHT: return "ARCTURN_RIGHT";
    }
    return "";
}
#endif

static uint32_t get_duty_cycle(drive_speed_t drive_speed)
{
    switch (drive_speed) {
    case DRIVE_SPEED_SLOW: return DUTY_CYCLE_SLOW;
    case DRIVE_SPEED_MEDIUM: return DUTY_CYCLE_MEDIUM;
    case DRIVE_SPEED_FAST: return DUTY_CYCLE_FAST;
    case DRIVE_SPEED_FASTEST: return DUTY_CYCLE_FASTEST;
    }
    return DUTY_CYCLE_STOP;
}

void drive_stop()
{
    motor_set_duty_cycle(MOTORS_LEFT, DUTY_CYCLE_STOP);
    motor_set_duty_cycle(MOTORS_RIGHT, DUTY_CYCLE_STOP);
}

void drive_set(drive_t drive, drive_speed_t drive_speed)
{
    const uint32_t duty_cycle = get_duty_cycle(drive_speed);
    switch (drive)
    {
    case DRIVE_FORWARD:
        motor_set_duty_cycle(MOTORS_LEFT, duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, duty_cycle);
        break;
    case DRIVE_REVERSE:
        motor_set_duty_cycle(MOTORS_LEFT, -duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, -duty_cycle);
        break;
    case DRIVE_ROTATE_LEFT:
        motor_set_duty_cycle(MOTORS_LEFT, -duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, duty_cycle);
        break;
    case DRIVE_ROTATE_RIGHT:
        motor_set_duty_cycle(MOTORS_LEFT, duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, -duty_cycle);
        break;
    case DRIVE_ARCTURN_LEFT:
#ifndef BUILD_MCU // TODO: Disabled for MCU atm (float point not allowed)
        motor_set_duty_cycle(MOTORS_LEFT, ARCTURN_DIFF_CONSTANT * duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, duty_cycle);
#endif
        break;
    case DRIVE_ARCTURN_RIGHT:
#ifndef BUILD_MCU // TODO: Disabled for MCU atm (float point not allowed)
        motor_set_duty_cycle(MOTORS_LEFT, duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, ARCTURN_DIFF_CONSTANT * duty_cycle);
#endif
        break;
    }
}

void drive_init()
{
    motor_init();
}
