#include <switch.h>
#include <stdio.h>
#include "ps5lite_ai.h"
#include "ps5lite_optimizer.h"

int main(int argc, char* argv[]) {
    consoleInit(NULL);

    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    ps5ai_init();

    Ps5OptProfile profile = PS5OPT_BALANCED;
    uint64_t previous_tick = armGetSystemTick();

    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 down = padGetButtonsDown(&pad);

        if (down & HidNpadButton_Plus)
            break;

        if (down & HidNpadButton_A)
            profile = ps5opt_next_profile(profile);

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
        printf("       PS5 LITE AI - SWITCH OLED\n");
        printf("========================================\n\n");

        printf("Perfil: %s\n", ps5opt_profile_name(profile));
        printf("%s\n\n", ps5opt_profile_description(profile));

        printf("Diagnostico do aplicativo:\n");
        printf("Tempo do ciclo: %.2f ms\n", s.frame_ms);
        printf("FPS equivalente: %.1f\n", s.fps);
        printf("Media suavizada: %.2f ms\n", s.average_frame_ms);
        printf("Amostras: %u\n", s.samples);
        printf("Ciclos lentos: %u\n\n", s.slow_frames);
        printf("Diagnostico: %s\n\n", s.recommendation);

        printf("A: trocar perfil\n");
        printf("X: limpar estatisticas\n");
        printf("+: sair\n\n");
        printf("Aviso: perfis informativos; nao aumentam FPS.\n");
        printf("Nao alteram clocks, tensao ou ventoinha.\n");

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
