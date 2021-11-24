#ifndef TIME_H
#define TIME_H

#include <stdint.h>

/* TODO: In case I want to use timer module instead of delay_cycles... */

uint32_t millis(void);
void time_init(void);

#endif /* TIME_H */
