#ifndef ir_remote__
#define ir_remote__

// The buttons on the remote control
typedef enum {
    COMMAND_0,
    COMMAND_1,
    COMMAND_2,
    COMMAND_3,
    COMMAND_4,
    COMMAND_5,
    COMMAND_6,
    COMMAND_7,
    COMMAND_8,
    COMMAND_9,
    COMMAND_STAR,
    COMMAND_HASH,
    COMMAND_UP,
    COMMAND_DOWN,
    COMMAND_LEFT,
    COMMAND_RIGHT,
    COMMAND_OK,
    COMMAND_NONE,
} ir_remote_command_t;

void ir_remote_init();
ir_remote_command_t ir_remote_get_command();

#endif //ir_remote__
