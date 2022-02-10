#ifndef IR_REMOTE_H
#define IR_REMOTE_H

/* Corresponds to the buttons on the remote */
typedef enum
{
    IR_KEY_0,
    IR_KEY_1,
    IR_KEY_2,
    IR_KEY_3,
    IR_KEY_4,
    IR_KEY_5,
    IR_KEY_6,
    IR_KEY_7,
    IR_KEY_8,
    IR_KEY_9,
    IR_KEY_STAR,
    IR_KEY_HASH,
    IR_KEY_UP,
    IR_KEY_DOWN,
    IR_KEY_LEFT,
    IR_KEY_RIGHT,
    IR_KEY_OK,
    IR_KEY_NONE,
} ir_key_t;

void ir_remote_wait_for_start_signal(void);
void ir_remote_init(void);
ir_key_t ir_remote_get_key(void);

#endif /* IR_REMOTE_H */
