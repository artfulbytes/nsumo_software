#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdbool.h>
#include <stdint.h>

#define VL53L0X_OUT_OF_RANGE (8190)

/* Comment these out if not connected */
#define VL53L0X_FRONT
#define VL53L0X_LEFT
#define VL53L0X_RIGHT
#define VL53L0X_FRONT_LEFT
#define VL53L0X_FRONT_RIGHT

typedef enum
{
#ifdef VL53L0X_FRONT
    VL53L0X_IDX_FRONT,
#endif
#ifdef VL53L0X_LEFT
    VL53L0X_IDX_LEFT,
#endif
#ifdef VL53L0X_RIGHT
    VL53L0X_IDX_RIGHT,
#endif
#ifdef VL53L0X_FRONT_LEFT
    VL53L0X_IDX_FRONT_LEFT,
#endif
#ifdef VL53L0X_FRONT_RIGHT
    VL53L0X_IDX_FRONT_RIGHT,
#endif
} vl53l0x_idx_t;

/**
 * Initializes the sensors in the vl53l0x_idx_t enum.
 * @note Each sensor must have its XSHUT pin connected.
 */
bool vl53l0x_init(void);

/**
 * Does a single range measurement
 * @param idx selects specific sensor
 * @param range contains the measured range or VL53L0X_OUT_OF_RANGE
 *        if out of range.
 * @return True if success, False if error
 * @note   Polling-based
 */
bool vl53l0x_read_range_single(vl53l0x_idx_t idx, uint16_t *range);

#endif /* VL53L0X_H */
