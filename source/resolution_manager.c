#include <switch.h>
#include <stdio.h>
#include <stdint.h>

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

static const ResolutionProfile profiles[] = {
    { RES_LOW,    "BAIXO", 1280, 720,  0.75f },
    { RES_MEDIUM, "MEDIO", 1600, 900,  0.90f },
    { RES_HIGH,   "ALTO",  1920, 1080, 1.00f },
    { RES_MEGA,   "MEGA",  2560, 1440, 1.25f }
};

static uint32_t game_width = 3840;
static uint32_t game_height = 2160;
static ResolutionPreset current = RES_MEGA;

static ResolutionPreset auto_detect_resolution(uint32_t w, uint32_t h) {
    uint32_t pixels = w * h;

    if (pixels >= 8000000) return RES_LOW;
    if (pixels >= 5000000) return RES_MEDIUM;
    if (pixels >= 3000000) return RES_HIGH;

    return RES_MEGA;
}

static const ResolutionProfile *get_profile(ResolutionPreset preset) {
    return &profiles[preset];
}

static void apply_resolution(ResolutionPreset preset) {
    const ResolutionProfile *profile = get_profile(preset);

    current = preset;

    printf("Perfil ativo: %s\n", profile->label);
    printf("Resolucao: %ux%u | escala: %.2f\n",
           profile->width,
           profile->height,
           profile->scale);

    // Aqui voce conecta com seu engine/renderer:
    // renderer->set_resolution(profile->width, profile->height);
    // renderer->set_scale(profile->scale);
}

static ResolutionPreset next_preset(ResolutionPreset preset) {
    return (ResolutionPreset)((preset + 1) % 4);
}

int main(int argc, char **argv) {
    consoleInit(NULL);

    printf("Sistema de resolucao automatica\n");
    printf("Pressione L/R para alternar ou A para auto-ajuste\n");

    while (appletMainLoop()) {
        hidScanInput();

        u64 keys = hidKeysDown(CONTROLLER_P1_AUTO);

        if (keys & KEY_L) {
            current = next_preset(current);
            apply_resolution(current);
        }

        if (keys & KEY_R) {
            current = auto_detect_resolution(game_width, game_height);
            apply_resolution(current);
        }

        if (keys & KEY_A) {
            ResolutionPreset autoPreset = auto_detect_resolution(game_width, game_height);
            apply_resolution(autoPreset);
        }

        if (keys & KEY_PLUS) {
            break;
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
