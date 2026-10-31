#include "ps5lite_ai.h"

#define DEFAULT_FRAME_MS   16.666667
#define EMA_ALPHA          0.15
#define SLOW_FRAME_MS      25.0
#define STALL_FRAME_MS     100.0
#define SLOW_TO_LIGHT      10u
#define FAST_TO_NORMAL     120u

static double avg_ms = 0.0;
static unsigned int slow_run = 0;
static unsigned int fast_run = 0;
static unsigned int total_samples = 0;
static Ps5AiMode current_mode = PS5AI_MODE_NORMAL;

void ps5ai_init(void) {
    avg_ms = 0.0;
    slow_run = 0;
    fast_run = 0;
    total_samples = 0;
    current_mode = PS5AI_MODE_NORMAL;
}

Ps5AiStatus ps5ai_update(uint64_t elapsed_ns) {
    Ps5AiStatus st;
    double frame_ms = (double)elapsed_ns / 1000000.0;

    if (frame_ms <= 0.0)
        frame_ms = DEFAULT_FRAME_MS;

    if (avg_ms <= 0.0)
        avg_ms = frame_ms;
    else
        avg_ms = avg_ms * (1.0 - EMA_ALPHA) + frame_ms * EMA_ALPHA;

    if (frame_ms > SLOW_FRAME_MS) {
        slow_run++;
        fast_run = 0;
    } else {
        fast_run++;
        slow_run = 0;
    }

    if (current_mode == PS5AI_MODE_NORMAL && slow_run >= SLOW_TO_LIGHT)
        current_mode = PS5AI_MODE_LIGHT;
    else if (current_mode == PS5AI_MODE_LIGHT && fast_run >= FAST_TO_NORMAL)
        current_mode = PS5AI_MODE_NORMAL;

    if (total_samples < 0xFFFFFFFFu)
        total_samples++;

    st.frame_ms = frame_ms;
    st.fps = 1000.0 / frame_ms;
    st.average_frame_ms = avg_ms;
    st.slow_frames = slow_run;
    st.samples = total_samples;
    st.mode = current_mode;
    st.stall_detected = frame_ms > STALL_FRAME_MS;

    if (st.stall_detected)
        st.recommendation = "Travamento detectado neste ciclo.";
    else if (current_mode == PS5AI_MODE_LIGHT)
        st.recommendation = "Ciclos lentos seguidos: modo leve ativo.";
    else
        st.recommendation = "Ciclo estavel.";

    return st;
}

const char *ps5ai_mode_name(Ps5AiMode mode) {
    switch (mode) {
        case PS5AI_MODE_NORMAL: return "Normal";
        case PS5AI_MODE_LIGHT:  return "Leve";
        default:                return "Desconhecido";
    }
}
