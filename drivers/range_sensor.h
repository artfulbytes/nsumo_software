#ifndef __range_sensor
#define __range_sensor

#include "stdint.h"

#define OUT_OF_RANGE (0)

typedef struct
{
    uint16_t left;
    uint16_t front_left;
    uint16_t front;
    uint16_t front_right;
    uint16_t right;
} range_sensor_distances_t;

void range_sensor_init(void);
void range_sensor_get_distances(range_sensor_distances_t* distances);

#endif //__range_sensor
