#ifndef LINE_DETECTION_H
#define LINE_DETECTION_H

typedef enum
{
    LINE_DETECTION_NONE,
    LINE_DETECTION_FRONT,
    LINE_DETECTION_BACK,
    LINE_DETECTION_FRONT_LEFT,
    LINE_DETECTION_BACK_LEFT,
    LINE_DETECTION_FRONT_RIGHT,
    LINE_DETECTION_BACK_RIGHT,
    LINE_DETECTION_LEFT,
    LINE_DETECTION_RIGHT,
    LINE_DETECTION_DIAGONAL_LEFT,
    LINE_DETECTION_DIAGONAL_RIGHT
} line_detection_t;

void line_detection_init(void);
line_detection_t line_detection_get(void);
const char *line_detection_str(line_detection_t line_detection);

#endif /* LINE_DETECTION_H */
