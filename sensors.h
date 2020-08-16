#include "stdint.h"

#define OUT_OF_RANGE 0;

typedef struct
{
    uint16_t left;
    uint16_t front_left;
    uint16_t front;
    uint16_t front_right;
    uint16_t right;
} sensor_distances_t;

void sensors_init(void);
void sensors_get_distances(sensor_distances_t* distances);
