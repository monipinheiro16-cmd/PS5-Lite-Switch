#include "resolution_manager.h"

static const ResolutionProfile profiles[] = {
    { RES_LOW,    "BAIXO", 1280,  720, 0.75f },
    { RES_MEDIUM, "MEDIO", 1600,  900, 0.90f },
    { RES_HIGH,   "ALTO",  1920, 1080, 1.00f },
    { RES_MEGA,   "MEGA",  2560, 1440, 1.25f }
};

const ResolutionProfile *resolution_get_profile(ResolutionPreset preset) {
    if ((int)preset < 0 || (int)preset > 3)
        preset = RES_LOW;
    return &profiles[preset];
}

ResolutionPreset resolution_auto_detect(uint32_t game_w, uint32_t game_h) {
    uint64_t pixels = (uint64_t)game_w * (uint64_t)game_h;

    if (pixels >= 8000000ULL) return RES_LOW;
    if (pixels >= 5000000ULL) return RES_MEDIUM;
    if (pixels >= 3000000ULL) return RES_HIGH;

    return RES_MEGA;
}

ResolutionPreset resolution_next_preset(ResolutionPreset preset) {
    return (ResolutionPreset)(((int)preset + 1) % 4);
}
