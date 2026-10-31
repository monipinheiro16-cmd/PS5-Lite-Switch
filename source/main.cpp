
#include <switch.h>
#include <cstdio>
#include "pkg_reader.hpp"
#include "emulator_core.hpp"

int main() {
    consoleInit(nullptr);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    PS5LiteCore emulator;
    emulator.initialize();

    int selected = 0;
    const char* menu[] = {
        "Inspecionar arquivo PKG",
        "Iniciar nucleo experimental",
        "Informacoes do projeto",
        "Sair"
    };

    bool running = true;

    while (appletMainLoop() && running) {
        padUpdate(&pad);
        u64 keys = padGetButtonsDown(&pad);

        if (keys & HidNpadButton_Down)
            selected = (selected + 1) % 4;

        if (keys & HidNpadButton_Up)
            selected = (selected + 3) % 4;

        if (keys & HidNpadButton_A) {
            switch (selected) {
                case 0:
                    break;

                case 1:
                    emulator.run();
                    break;

                case 2:
                    break;

                case 3:
                    running = false;
                    break;
            }
        }

        consoleClear();

        printf("================================\n");
        printf("       PS5-LITE-SWITCH\n");
        printf("    NINTENDO ULTIMATE ENGINE\n");
        printf("================================\n\n");

        for (int i = 0; i < 4; i++) {
            printf("%s %s\n",
                selected == i ? ">" : " ",
                menu[i]);
        }

        printf("\nEstado: EXPERIMENTAL\n");
        printf("Nucleo: %s\n", emulator.status().c_str());

        printf("\nEste projeto ainda nao executa jogos PS5.\n");
        printf("Use + para sair.\n");

        consoleUpdate(nullptr);
    }

    consoleExit(nullptr);
    return 0;
}
