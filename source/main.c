#include <switch.h>
#include <stdio.h>
#include "ps5lite_ai.h"

int main(int argc, char* argv[]) {
    consoleInit(NULL);

    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    ps5ai_init();

    uint64_t previous_tick = armGetSystemTick();

    while (appletMainLoop()) {
        padUpdate(&pad);

        u64 down = padGetButtonsDown(&pad);

        if (down & HidNpadButton_Plus)
            break;

        if (down & HidNpadButton_A)
            svcSleepThread(50000000LL);

        if (down & HidNpadButton_X) {
            ps5ai_init();
            previous_tick = armGetSystemTick();
        }

        uint64_t now = armGetSystemTick();
        uint64_t elapsed_ns = armTicksToNs(now - previous_tick);
        previous_tick = now;

        Ps5AiStatus s = ps5ai_update(elapsed_ns);

        consoleClear();

        printf("========================================\n");
        printf("          PS5 LITE AI - SWITCH\n");
        printf("========================================\n\n");

        printf("Estado: %s\n", ps5ai_mode_name(s.mode));
        printf("Tempo do ciclo: %.2f ms\n", s.frame_ms);
        printf("FPS equivalente: %.1f\n", s.fps);
        printf("Media suavizada: %.2f ms\n", s.average_frame_ms);
        printf("Amostras: %u\n", s.samples);
        printf("Ciclos lentos: %u\n\n", s.slow_frames);

        printf("Diagnostico: %s\n\n", s.recommendation);

        printf("A: simular pausa de 50 ms\n");
        printf("X: limpar estatisticas\n");
        printf("+: sair\n\n");

        printf("PS5 Lite AI - Diagnostico local\n");

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
