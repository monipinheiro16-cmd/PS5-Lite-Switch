#include <switch.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    consoleInit(NULL);

    printf("================================\n");
    printf("       PS5 LITE - SWITCH\n");
    printf("================================\n\n");

    printf("Projeto homebrew para Nintendo Switch\n\n");
    printf("Esta e uma demonstracao inicial.\n");
    printf("Nao executa jogos de PS5.\n\n");
    printf("Controles:\n");
    printf("Pressione + para sair.\n\n");

    while (appletMainLoop()) {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            break;
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
