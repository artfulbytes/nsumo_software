#ifndef TIME_H
#define TIME_H

#include <stdint.h>

typedef uint32_t timer_t;
void timer_start(timer_t *timer);
uint32_t timer_ms_elapsed(const timer_t *timer);

#endif /* TIMER_H */
