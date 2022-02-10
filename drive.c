#include "drive.h"
#include "motor.h"

#define DUTY_CYCLE_STOP (0)

typedef struct
{
    int8_t left;
    int8_t right;
} duty_cycles_t;

/* Each drive is a pair (e.g. (FORWARD,REVERSE), (ROTATE_LEFT, ROTATE_RIGHT)). To save
 * space, only store one drive in each pair, and use is_opposite to map to the opposing
 * drive. */
static const duty_cycles_t drive_to_duty_cycle[][4] =
{
#if BUILD_MCU
    [DRIVE_FORWARD] =
        {
            [DRIVE_SPEED_SLOW] = {25, 25},
            [DRIVE_SPEED_MEDIUM] = {45, 45},
            [DRIVE_SPEED_FAST] = {55, 55},
            [DRIVE_SPEED_FASTEST] = {100, 100},
        },
    [DRIVE_ROTATE_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {-25, 25},
            [DRIVE_SPEED_MEDIUM] = {-50, 50},
            [DRIVE_SPEED_FAST] = {-60, 60},
            [DRIVE_SPEED_FASTEST] = {-100, 100},
        },
    [DRIVE_ARCTURN_SHARP_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {-10, 25},
            [DRIVE_SPEED_MEDIUM] = {-10, 50},
            [DRIVE_SPEED_FAST] = {-25, 75},
            [DRIVE_SPEED_FASTEST] = {-20, 100},
        },
    [DRIVE_ARCTURN_MID_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {15, 30},
            [DRIVE_SPEED_MEDIUM] = {25, 50},
            [DRIVE_SPEED_FAST] = {35, 70},
            [DRIVE_SPEED_FASTEST] = {50, 100},
        },
    [DRIVE_ARCTURN_WIDE_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {20, 25},
            [DRIVE_SPEED_MEDIUM] = {40, 50},
            [DRIVE_SPEED_FAST] = {60, 70},
            [DRIVE_SPEED_FASTEST] = {85, 100},
        },
#else // Simulator (TODO: Tune simulator so the above values match instead)
    [DRIVE_FORWARD] =
        {
            [DRIVE_SPEED_SLOW] = {25, 25},
            [DRIVE_SPEED_MEDIUM] = {60, 60},
            [DRIVE_SPEED_FAST] = {80, 80},
            [DRIVE_SPEED_FASTEST] = {100, 100},
        },
    [DRIVE_ROTATE_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {-25, 25},
            [DRIVE_SPEED_MEDIUM] = {-50, 50},
            [DRIVE_SPEED_FAST] = {-60, 60},
            [DRIVE_SPEED_FASTEST] = {-100, 100},
        },
    [DRIVE_ARCTURN_SHARP_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {-10, 25},
            [DRIVE_SPEED_MEDIUM] = {-10, 50},
            [DRIVE_SPEED_FAST] = {-15, 60},
            [DRIVE_SPEED_FASTEST] = {-20, 100},
        },
    [DRIVE_ARCTURN_MID_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {15, 25},
            [DRIVE_SPEED_MEDIUM] = {30, 50},
            [DRIVE_SPEED_FAST] = {55, 80},
            [DRIVE_SPEED_FASTEST] = {50, 100},
        },
    [DRIVE_ARCTURN_WIDE_LEFT] =
        {
            [DRIVE_SPEED_SLOW] = {20, 25},
            [DRIVE_SPEED_MEDIUM] = {39, 50},
            [DRIVE_SPEED_FAST] = {67, 80},
            [DRIVE_SPEED_FASTEST] = {78, 100},
        },
#endif
};

static void negate(drive_t drive, bool reverse, int8_t *left, int8_t *right)
{
    switch (drive) {
    case DRIVE_REVERSE:
    case DRIVE_ROTATE_RIGHT:
        *left = -*left;
        *right = -*right;
        break;
    case DRIVE_ARCTURN_SHARP_RIGHT:
    case DRIVE_ARCTURN_MID_RIGHT:
    case DRIVE_ARCTURN_WIDE_RIGHT:
    {
        const int8_t left_tmp = *left;
        *left = *right;
        *right = left_tmp;
        break;
    }
    default:
        break;
    }
    if (reverse) {
        *left = -*left;
        *right = -*right;
    }
}

void drive_stop()
{
    motor_set_duty_cycle(MOTORS_LEFT, DUTY_CYCLE_STOP);
    motor_set_duty_cycle(MOTORS_RIGHT, DUTY_CYCLE_STOP);
}

void drive_set(drive_t drive, bool reverse, drive_speed_t drive_speed)
{
    int8_t left = drive_to_duty_cycle[drive - (drive % 2)][drive_speed].left;
    int8_t right = drive_to_duty_cycle[drive - (drive % 2)][drive_speed].right;
    negate(drive, reverse, &left, &right);
    motor_set_duty_cycle(MOTORS_LEFT, left);
    motor_set_duty_cycle(MOTORS_RIGHT, right);
}

void drive_init() { motor_init(); }
