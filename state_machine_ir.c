#include "ir_remote.h"
#include "motor.h"

typedef enum
{
    ROBOT_STOP,
    ROBOT_FORWARD,
    ROBOT_REVERSE,
    ROBOT_LEFT,
    ROBOT_RIGHT,
    ROBOT_SET_SPEED_10,
    ROBOT_SET_SPEED_20,
    ROBOT_SET_SPEED_30,
    ROBOT_SET_SPEED_40,
    ROBOT_SET_SPEED_50,
    ROBOT_SET_SPEED_60,
    ROBOT_SET_SPEED_70,
    ROBOT_SET_SPEED_80,
    ROBOT_SET_SPEED_90,
    ROBOT_SET_SPEED_100
} robot_command_t;

typedef enum
{
    STATE_STOP,
    STATE_FORWARD,
    STATE_REVERSE,
    STATE_LEFT,
    STATE_RIGHT
} robot_state_t;

static const robot_command_t ir_key_to_robot_command[] = {
    [IR_KEY_1] = ROBOT_SET_SPEED_10, [IR_KEY_2] = ROBOT_SET_SPEED_20,
    [IR_KEY_3] = ROBOT_SET_SPEED_30, [IR_KEY_4] = ROBOT_SET_SPEED_40,
    [IR_KEY_5] = ROBOT_SET_SPEED_50, [IR_KEY_6] = ROBOT_SET_SPEED_60,
    [IR_KEY_7] = ROBOT_SET_SPEED_70, [IR_KEY_8] = ROBOT_SET_SPEED_80,
    [IR_KEY_9] = ROBOT_SET_SPEED_90, [IR_KEY_0] = ROBOT_SET_SPEED_100,
    [IR_KEY_STAR] = ROBOT_STOP,      [IR_KEY_HASH] = ROBOT_STOP,
    [IR_KEY_UP] = ROBOT_FORWARD,     [IR_KEY_DOWN] = ROBOT_REVERSE,
    [IR_KEY_LEFT] = ROBOT_LEFT,      [IR_KEY_RIGHT] = ROBOT_RIGHT,
    [IR_KEY_OK] = ROBOT_STOP,        [IR_KEY_NONE] = ROBOT_STOP
};

// TODO: struct for holding speed and state?
static robot_state_t current_state = STATE_STOP;
static uint16_t current_speed = 0;

static uint16_t state_machine_command_to_speed(robot_command_t command)
{
    switch (command) {
    case ROBOT_SET_SPEED_10:
        return 10;
    case ROBOT_SET_SPEED_20:
        return 20;
    case ROBOT_SET_SPEED_30:
        return 30;
    case ROBOT_SET_SPEED_40:
        return 40;
    case ROBOT_SET_SPEED_50:
        return 50;
    case ROBOT_SET_SPEED_60:
        return 60;
    case ROBOT_SET_SPEED_70:
        return 70;
    case ROBOT_SET_SPEED_80:
        return 80;
    case ROBOT_SET_SPEED_90:
        return 80;
    case ROBOT_SET_SPEED_100:
        return 100;
    case ROBOT_FORWARD:
        return 0;
    case ROBOT_LEFT:
        return 0;
    case ROBOT_REVERSE:
        return 0;
    case ROBOT_RIGHT:
        return 0;
    case ROBOT_STOP:
        return 0;
    }
    return 0;
}

static robot_state_t handle_stop_command()
{
    if (current_state == STATE_STOP) {
        return STATE_STOP;
    }
    motor_set_duty_cycle(MOTORS_LEFT, 0);
    motor_set_duty_cycle(MOTORS_RIGHT, 0);
    return STATE_STOP;
}

static robot_state_t handle_forward_command()
{
    if (current_state == STATE_FORWARD) {
        return STATE_FORWARD;
    }
    if (current_state != STATE_STOP) {
        motor_stop_safely();
    }
    motor_set_duty_cycle(MOTORS_LEFT, current_speed);
    motor_set_duty_cycle(MOTORS_RIGHT, current_speed);
    return STATE_FORWARD;
}

static robot_state_t handle_reverse_command()
{
    if (current_state == STATE_REVERSE) {
        return STATE_REVERSE;
    }
    if (current_state != STATE_STOP) {
        motor_stop_safely();
    }
    motor_set_duty_cycle(MOTORS_LEFT, -current_speed);
    motor_set_duty_cycle(MOTORS_RIGHT, -current_speed);
    return STATE_REVERSE;
}

static robot_state_t handle_left_command()
{
    if (current_state == STATE_LEFT) {
        return STATE_LEFT;
    }
    if (current_state != STATE_STOP) {
        motor_stop_safely();
    }
    motor_set_duty_cycle(MOTORS_LEFT, -current_speed);
    motor_set_duty_cycle(MOTORS_RIGHT, current_speed);
    return STATE_LEFT;
}

static robot_state_t handle_right_command()
{
    if (current_state == STATE_RIGHT) {
        return STATE_RIGHT;
    }
    if (current_state != STATE_STOP) {
        motor_stop_safely();
    }
    motor_set_duty_cycle(MOTORS_LEFT, current_speed);
    motor_set_duty_cycle(MOTORS_RIGHT, -current_speed);
    return STATE_RIGHT;
}

// TODO: Add new layer/function to reduce code duplication for motor control
static void handle_speed_command(robot_command_t speed_command)
{
    uint16_t new_speed = state_machine_command_to_speed(speed_command);

    switch (current_state) {
    case STATE_STOP:
        break;
    case STATE_FORWARD:
        motor_set_duty_cycle(MOTORS_LEFT, new_speed);
        motor_set_duty_cycle(MOTORS_RIGHT, new_speed);
        break;
    case STATE_REVERSE:
        motor_set_duty_cycle(MOTORS_LEFT, -new_speed);
        motor_set_duty_cycle(MOTORS_RIGHT, -new_speed);
        break;
    case STATE_LEFT:
        motor_set_duty_cycle(MOTORS_LEFT, -new_speed);
        motor_set_duty_cycle(MOTORS_RIGHT, new_speed);
        break;
    case STATE_RIGHT:
        motor_set_duty_cycle(MOTORS_LEFT, new_speed);
        motor_set_duty_cycle(MOTORS_RIGHT, -new_speed);
        break;
    }
    current_speed = new_speed;
}

void state_machine_ir_handle_key(ir_key_t key)
{
    if (key == IR_KEY_NONE) {
        return;
    }

    robot_command_t robot_command = ir_key_to_robot_command[key];
    robot_state_t new_state = current_state;
    switch (robot_command) {
    case ROBOT_STOP:
        new_state = handle_stop_command();
        break;
    case ROBOT_FORWARD:
        new_state = handle_forward_command();
        break;
    case ROBOT_REVERSE:
        new_state = handle_reverse_command();
        break;
    case ROBOT_LEFT:
        new_state = handle_left_command();
        break;
    case ROBOT_RIGHT:
        new_state = handle_right_command();
        break;
    case ROBOT_SET_SPEED_10:
    case ROBOT_SET_SPEED_20:
    case ROBOT_SET_SPEED_30:
    case ROBOT_SET_SPEED_40:
    case ROBOT_SET_SPEED_50:
    case ROBOT_SET_SPEED_60:
    case ROBOT_SET_SPEED_70:
    case ROBOT_SET_SPEED_80:
    case ROBOT_SET_SPEED_90:
    case ROBOT_SET_SPEED_100:
        handle_speed_command(robot_command);
        break;
    }
    current_state = new_state;
}

void state_machine_ir_init() { motor_init(); }
