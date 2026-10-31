#include <switch.h>
#include <stdio.h>
#include <string.h>

// Estado da Inteligência Artificial de Otimização
bool aiOptimizerActive = false;
char statusText[128] = "EQUILIBRADO (Perfil Informativo)";

int main(int argc, char **argv)
{
    // Inicializa o modo de consola de texto padrão no ecrã principal
    consoleInit(NULL);

    // Configura o manuseamento de comandos (Joy-Con / Pro Controller)
    padConfigureInput(1, HID_NINTENDO_PRO_CON_POWERED);
    PadState pad;
    padInitializeDefault(&pad);

    printf("\x1b[1;1H====================================");
    printf("\x1b[2;1H       PS5 Lite AI - Switch         ");
    printf("\x1b[3;1H====================================");
    printf("\x1b[5;1H Pressione [Y] para Alternar Otimizacao IA");
    printf("\x1b[6;1H Pressione [+ ] para Sair");
    printf("\x1b[7;1H------------------------------------");

    // Loop principal da aplicação
    while (appletMainLoop())
    {
        // Atualiza a leitura dos botões do comando
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        // Se pressionar o botão (+), encerra a aplicação
        if (kDown & HidNpadButton_Plus) {
            break;
        }

        // Se pressionar o botão (Y), alterna o estado da IA de Otimização
        if (kDown & HidNpadButton_Y) {
            aiOptimizerActive = !aiOptimizerActive;

            if (aiOptimizerActive) {
                snprintf(statusText, sizeof(statusText), "IA OTIMIZADORA ATIVA (Modo Dinamico)");
            } else {
                snprintf(statusText, sizeof(statusText), "EQUILIBRADO (Perfil Informativo)");
            }
        }

        // Atualiza as informações no ecrã
        printf("\x1b[10;1H Status Atual: %s                ", statusText);
        if (aiOptimizerActive) {
            printf("\x1b[12;1H [!] Perfil de desempenho dinâmico em execucao.");
        } else {
            printf("\x1b[12;1H [i] Monitoramento padrao ativo.                ");
        }

        // Renderiza o frame na consola do Switch
        consoleUpdate(NULL);
    }

    // Finaliza os serviços da consola antes de fechar
    consoleExit(NULL);
    return 0;
}
