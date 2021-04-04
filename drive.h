#ifndef DRIVE_H
#define DRIVE_H

typedef enum {
    DRIVE_SPEED_SLOW,
    DRIVE_SPEED_MEDIUM,
    DRIVE_SPEED_FAST,
    DRIVE_SPEED_FASTEST
} drive_speed_t;

typedef enum {
    DRIVE_FORWARD,
    DRIVE_REVERSE,
    DRIVE_ROTATE_LEFT,
    DRIVE_ROTATE_RIGHT,
    DRIVE_ARCTURN_LEFT,
    DRIVE_ARCTURN_RIGHT
} drive_t;

void drive_init(void);
void drive_stop();
void drive_set(drive_t drive, drive_speed_t drive_speed);

#endif /* DRIVE_H */
