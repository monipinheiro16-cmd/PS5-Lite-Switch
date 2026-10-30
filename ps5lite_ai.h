#ifndef PS5LITE_AI_H
#define PS5LITE_AI_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PS5AI_MODE_NORMAL = 0,
    PS5AI_MODE_LIGHT = 1
} Ps5AiMode;

typedef struct {
    double frame_ms;
    double fps;
    double average_frame_ms;
    unsigned int slow_frames;
    unsigned int samples;
    Ps5AiMode mode;
    bool stall_detected;
    const char *recommendation;
} Ps5AiStatus;

void ps5ai_init(void);
Ps5AiStatus ps5ai_update(uint64_t elapsed_ns);
const char *ps5ai_mode_name(Ps5AiMode mode);

#endif
