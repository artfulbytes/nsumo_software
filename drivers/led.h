#ifndef LED_H
#define LED_H

#include <stdbool.h>

typedef enum
{
    LED_TEST,
} led_t;

void led_init(void);
void led_set_enable(led_t led, bool enable);

#endif /* LED_H */
