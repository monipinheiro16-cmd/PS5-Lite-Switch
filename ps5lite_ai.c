#include "ps5lite_ai.h"

#define PS5AI_SLOW_FRAME_NS 33333333ULL
#define PS5AI_TRIGGER_COUNT 3
#define PS5AI_RECOVERY_COUNT 120

static Ps5AiMode g_mode;
static unsigned int g_slow_frames;
static unsigned int g_recovery_frames;
static unsigned int g_samples;
static double g_avg_ms;

void ps5ai_init(void) {
    g_mode = PS5AI_MODE_NORMAL;
    g_slow_frames = 0;
    g_recovery_frames = 0;
    g_samples = 0;
    g_avg_ms = 0.0;
}

const char *ps5ai_mode_name(Ps5AiMode mode) {
    return mode == PS5AI_MODE_LIGHT ? "LEVE" : "NORMAL";
}

/* Mede apenas o ciclo deste aplicativo; não otimiza outros jogos. */
Ps5AiStatus ps5ai_update(uint64_t elapsed_ns) {
    Ps5AiStatus s = {0};
    const double ms = (double)elapsed_ns / 1000000.0;

    s.frame_ms = ms;
    s.fps = ms > 0.0 ? 1000.0 / ms : 0.0;

    if (g_samples == 0) g_avg_ms = ms;
    else g_avg_ms = (g_avg_ms * 0.9) + (ms * 0.1);
    g_samples++;

    if (elapsed_ns > PS5AI_SLOW_FRAME_NS) {
        g_slow_frames++;
        g_recovery_frames = 0;
    } else {
        g_slow_frames = 0;
        if (g_mode != PS5AI_MODE_NORMAL && ++g_recovery_frames >= PS5AI_RECOVERY_COUNT) {
            g_mode = PS5AI_MODE_NORMAL;
            g_recovery_frames = 0;
        }
    }

    if (g_slow_frames >= PS5AI_TRIGGER_COUNT) {
        g_mode = PS5AI_MODE_LIGHT;
        g_recovery_frames = 0;
    }

    s.average_frame_ms = g_avg_ms;
    s.slow_frames = g_slow_frames;
    s.samples = g_samples;
    s.mode = g_mode;
    s.stall_detected = g_slow_frames >= PS5AI_TRIGGER_COUNT;
    if (s.stall_detected) s.recommendation = "Ciclos lentos detectados neste aplicativo.";
    else if (g_mode == PS5AI_MODE_LIGHT) s.recommendation = "Modo leve ativo; monitorando recuperacao.";
    else s.recommendation = "Desempenho dentro do limite monitorado.";
    return s;
}
