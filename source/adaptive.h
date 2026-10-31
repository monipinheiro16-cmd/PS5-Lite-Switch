#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PROFILE_HIGH = 0,
    PROFILE_BALANCED = 1,
    PROFILE_AUTO = 2
} GraphicsProfile;

typedef struct {
    float fps;
    float average_fps;
    unsigned int low_fps_count;
    GraphicsProfile profile;
    int reduced_mode;
} AdaptiveStatus;

void adaptive_init(void);
AdaptiveStatus adaptive_update(float fps);
GraphicsProfile adaptive_next_profile(GraphicsProfile profile);
const char *adaptive_profile_name(GraphicsProfile profile);

#ifdef __cplusplus
}
#endif

#endif
