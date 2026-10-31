#ifndef RESOLUTION_MANAGER_H
#define RESOLUTION_MANAGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RES_LOW = 0,
    RES_MEDIUM = 1,
    RES_HIGH = 2,
    RES_MEGA = 3
} ResolutionPreset;

typedef struct {
    ResolutionPreset preset;
    const char *label;
    uint32_t width;
    uint32_t height;
    float scale;
} ResolutionProfile;

const ResolutionProfile *resolution_get_profile(ResolutionPreset preset);
ResolutionPreset resolution_auto_detect(uint32_t game_w, uint32_t game_h);
ResolutionPreset resolution_next_preset(ResolutionPreset preset);

#ifdef __cplusplus
}
#endif

#endif
