#include "drive.h"
#include "motor.h"

#define DUTY_CYCLE_STOP (0)
#define DUTY_CYCLE_SLOW (20)
#define DUTY_CYCLE_MEDIUM (50)
#define DUTY_CYCLE_FAST (80)
#define DUTY_CYCLE_FASTEST (100)

#define ARCTURN_DIFF_CONSTANT (0.7f)

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

void set_drive(drive_t drive, drive_speed_t drive_speed)
{
    const uint32_t duty_cycle = get_duty_cycle(drive_speed);
    switch (drive)
    {
    case DRIVE_STOP:
        motor_set_duty_cycle(MOTORS_LEFT, DUTY_CYCLE_STOP);
        motor_set_duty_cycle(MOTORS_RIGHT, DUTY_CYCLE_STOP);
        break;
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
        motor_set_duty_cycle(MOTORS_LEFT, ARCTURN_DIFF_CONSTANT * duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, duty_cycle);
        break;
    case DRIVE_ARCTURN_RIGHT:
        motor_set_duty_cycle(MOTORS_LEFT, duty_cycle);
        motor_set_duty_cycle(MOTORS_RIGHT, ARCTURN_DIFF_CONSTANT * duty_cycle);
        break;
    }
}

void drive_init()
{
    motor_init();
}
