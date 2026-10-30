#include <switch.h>
#include <stdio.h>
#include "ps5lite_ai.h"

int main(int argc, char* argv[]) {
    consoleInit(NULL);
    ps5ai_init();
    uint64_t previous_tick = armGetSystemTick();

    while (appletMainLoop()) {
        hidScanInput();
        const u64 down = hidKeysDown(CONTROLLER_P1_AUTO);
        if (down & KEY_PLUS) break;
        if (down & KEY_A) svcSleepThread(50000000LL); /* pausa de teste de 50 ms */
        if (down & KEY_X) { ps5ai_init(); previous_tick = armGetSystemTick(); }

        const uint64_t now = armGetSystemTick();
        const uint64_t elapsed_ns = armTicksToNs(now - previous_tick);
        previous_tick = now;
        const Ps5AiStatus s = ps5ai_update(elapsed_ns);

        consoleClear();
        printf("========================================\n");
        printf("          PS5 LITE AI - SWITCH\n");
        printf("========================================\n\n");
        printf("Estado: %s\n", ps5ai_mode_name(s.mode));
        printf("Tempo do ciclo: %.2f ms\n", s.frame_ms);
        printf("FPS equivalente do ciclo: %.1f\n", s.fps);
        printf("Media suavizada: %.2f ms\n", s.average_frame_ms);
        printf("Amostras: %u\n", s.samples);
        printf("Ciclos lentos seguidos: %u\n\n", s.slow_frames);
        printf("Diagnostico: %s\n\n", s.recommendation);
        printf("A: simular pausa de 50 ms\nX: limpar estatisticas\n+: sair\n\n");
        printf("Aviso: nao executa jogos de PS5 nem altera jogos externos.\n");
        consoleUpdate(NULL);
    }
    consoleExit(NULL);
    return 0;
}
