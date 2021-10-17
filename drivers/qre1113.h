#ifndef QRE1113_H
#define QRE1113_H

#include <stdint.h>

typedef struct
{
    uint16_t front_left;
    uint16_t front_right;
    uint16_t back_left;
    uint16_t back_right;
} qre1113_voltages_t;

void qre1113_init(void);
void qre1113_get_voltages(qre1113_voltages_t *voltages);

#endif /* QRE1113_H */
