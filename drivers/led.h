#ifndef LED_H
#define LED_H

typedef enum
{
    LED_FIRST,
    LED_SECOND
} led_t;

typedef enum
{
    LED_MODE_ON,
    LED_MODE_OFF
} led_mode_t;

/* Should only be used for testing on the Launchpad! The pins are normally
 * used for other functions. */
void led_init(led_t led);
void led_set_mode(led_t led, led_mode_t mode);

#endif /* LED_H */
