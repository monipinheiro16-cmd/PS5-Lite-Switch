
#include "adaptive.h"

static float average_fps = 0.0f;
static unsigned int low_fps_count = 0;
static GraphicsProfile current_profile = PROFILE_AUTO;
static int reduced_mode = 0;

void adaptive_init(void) {
    average_fps = 0.0f;
    low_fps_count = 0;
    current_profile = PROFILE_AUTO;
    reduced_mode = 0;
}

AdaptiveStatus adaptive_update(float fps) {
    AdaptiveStatus status = {0};

    if (fps < 1.0f || fps > 240.0f) {
        fps = 60.0f;
    }

    if (average_fps == 0.0f) {
        average_fps = fps;
    } else {
        average_fps = average_fps * 0.85f + fps * 0.15f;
    }

    if (current_profile == PROFILE_AUTO) {
        if (average_fps < 25.0f) {
            low_fps_count++;
        } else if (average_fps > 35.0f) {
            low_fps_count = 0;
        }

        if (low_fps_count >= 3) {
            reduced_mode = 1;
        }

        if (average_fps > 45.0f) {
            reduced_mode = 0;
            low_fps_count = 0;
        }
    } else {
        reduced_mode = 0;
        low_fps_count = 0;
    }

    status.fps = fps;
    status.average_fps = average_fps;
    status.low_fps_count = low_fps_count;
    status.profile = current_profile;
    status.reduced_mode = reduced_mode;

    return status;
}

GraphicsProfile adaptive_next_profile(GraphicsProfile profile) {
    return (GraphicsProfile)(((int)profile + 1) % 3);
}

const char *adaptive_profile_name(GraphicsProfile profile) {
    switch (profile) {
        case PROFILE_HIGH:
            return "Alto";

        case PROFILE_BALANCED:
            return "Equilibrado";

        case PROFILE_AUTO:
            return "Automatico";

        default:
            return "Desconhecido";
    }
}
