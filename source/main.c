#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ps5lite_ai.h"
#include "ps5lite_optimizer.h"

#define ROOT_DIR   "sdmc:/switch/switchps5lite"
#define GAMES_DIR  "sdmc:/switch/switchps5lite/games"
#define IMPORT_DIR "sdmc:/switch/switchps5lite/imports"

#define MAX_FILES 256
#define NAME_LEN 256
#define PATH_LEN 512

typedef enum {
    STATUS_UNKNOWN,
    STATUS_NRO,
    STATUS_UNSUPPORTED
} GameStatus;

static char files[MAX_FILES][NAME_LEN];
static int file_count = 0;
static int selected = 0;
static char message[256] = "A: verificar | X: importar | Y: executar NRO";
static char current_dir[PATH_LEN] = GAMES_DIR;
static Ps5AiStatus ai_status = {0};
static Ps5OptProfile selected_profile = PS5OPT_BALANCED;

static int ends_with_ci(const char *name, const char *ext) {
    size_t n = strlen(name);
    size_t e = strlen(ext);

    if (n < e) return 0;

    name += n - e;

    for (size_t i = 0; i < e; i++) {
        if (tolower((unsigned char)name[i]) !=
            tolower((unsigned char)ext[i])) {
            return 0;
        }
    }

    return 1;
}

static int is_directory(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static GameStatus check_game(const char *filename) {
    if (ends_with_ci(filename, ".nro"))
        return STATUS_NRO;

    if (ends_with_ci(filename, ".pkg") ||
        ends_with_ci(filename, ".ps5") ||
        ends_with_ci(filename, ".elf") ||
        ends_with_ci(filename, ".iso"))
        return STATUS_UNSUPPORTED;

    return STATUS_UNKNOWN;
}

static void load_files(void) {
    DIR *dir = opendir(current_dir);
    struct dirent *entry;

    file_count = 0;
    selected = 0;

    if (!dir) {
        snprintf(message, sizeof(message),
                 "Erro ao abrir a pasta games.");
        return;
    }

    while ((entry = readdir(dir)) != NULL &&
           file_count < MAX_FILES) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(files[file_count], NAME_LEN, "%s",
                 entry->d_name);
        file_count++;
    }

    closedir(dir);

    snprintf(message, sizeof(message),
             "Lista atualizada: %d arquivos.", file_count);
}

static void verify_selected(void) {
    if (file_count == 0) {
        snprintf(message, sizeof(message),
                 "Nenhum arquivo para verificar.");
        return;
    }

    char path[PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s",
             current_dir, files[selected]);

    if (is_directory(path)) {
        snprintf(message, sizeof(message),
                 "Pasta selecionada, nao um jogo.");
        return;
    }

    GameStatus status = check_game(files[selected]);

    switch (status) {
        case STATUS_NRO:
            snprintf(message, sizeof(message),
                     "NRO reconhecido; compatibilidade nao garantida.");
            break;

        case STATUS_UNSUPPORTED:
            snprintf(message, sizeof(message),
                     "Formato nao suportado para execucao nativa.");
            break;

        default:
            snprintf(message, sizeof(message),
                     "Desconhecido: nao foi possivel confirmar.");
            break;
    }
}

static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return 0;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }

    char buffer[16384];
    size_t n;
    int success = 1;

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            success = 0;
            break;
        }
    }

    if (ferror(in)) success = 0;

    fclose(in);
    fclose(out);

    if (!success) remove(dst);

    return success;
}

static void import_selected(void) {
    if (file_count == 0) {
        snprintf(message, sizeof(message),
                 "Nenhum arquivo selecionado.");
        return;
    }

    char src[PATH_LEN];
    char dst[PATH_LEN];

    snprintf(src, sizeof(src), "%s/%s",
             current_dir, files[selected]);

    if (is_directory(src)) {
        snprintf(message, sizeof(message),
                 "Selecione um arquivo, nao uma pasta.");
        return;
    }

    if (snprintf(dst, sizeof(dst), "%s/%s",
                 IMPORT_DIR, files[selected]) >= (int)sizeof(dst)) {
        snprintf(message, sizeof(message), "Caminho muito longo.");
        return;
    }

    if (strcmp(src, dst) == 0) {
        snprintf(message, sizeof(message),
                 "Arquivo ja esta na pasta de importacao.");
        return;
    }

    if (copy_file(src, dst)) {
        snprintf(message, sizeof(message),
                 "Importacao concluida: arquivo copiado.");
    } else {
        snprintf(message, sizeof(message),
                 "Falha ao copiar. Confira espaco e arquivo.");
    }
}

static void execute_selected(void) {
    if (file_count == 0) {
        snprintf(message, sizeof(message),
                 "Nenhum arquivo selecionado.");
        return;
    }

    char path[PATH_LEN];

    if (snprintf(path, sizeof(path), "%s/%s",
                 current_dir, files[selected]) >= (int)sizeof(path)) {
        snprintf(message, sizeof(message), "Caminho muito longo.");
        return;
    }

    if (is_directory(path)) {
        snprintf(message, sizeof(message),
                 "Selecione um arquivo, nao uma pasta.");
        return;
    }

    GameStatus status = check_game(files[selected]);

    if (status != STATUS_NRO) {
        snprintf(message, sizeof(message),
                 "Nao executado: formato nao reconhecido como NRO.");
        return;
    }

    envSetNextLoad(path, NULL);
    snprintf(message, sizeof(message), "Abrindo homebrew NRO...");
    consoleUpdate(NULL);
    consoleExit(NULL);
    exit(0);
}

int main(int argc, char *argv[]) {
    consoleInit(NULL);

    mkdir(ROOT_DIR, 0777);
    mkdir(GAMES_DIR, 0777);
    mkdir(IMPORT_DIR, 0777);

    ps5ai_init();
    selected_profile = PS5OPT_BALANCED;
    ai_status = ps5ai_update(16666667ULL);
    load_files();

    while (appletMainLoop()) {
        hidScanInput();

        u64 down = hidKeysDown(CONTROLLER_P1_AUTO);

        if (down & KEY_PLUS)
            break;

        if (file_count > 0 && (down & KEY_DOWN))
            selected = (selected + 1) % file_count;

        if (file_count > 0 && (down & KEY_UP))
            selected = (selected - 1 + file_count) % file_count;

        if (down & (KEY_L | KEY_R)) {
            selected_profile = ps5opt_next_profile(selected_profile);
            snprintf(message, sizeof(message),
                     "Perfil %s ativado.", ps5opt_profile_name(selected_profile));
        }

        if (down & KEY_A)
            verify_selected();

        if (down & KEY_X)
            import_selected();

        if (down & KEY_Y)
            execute_selected();

        if (down & KEY_B)
            load_files();

        ai_status = ps5ai_update(16666667ULL);

        consoleClear();

        printf("========================================\n");
        printf("          SWITCHPS5 LITE\n");
        printf("========================================\n\n");

        printf("Pasta: %s\n\n", current_dir);
        printf("AI: %s | Perfil: %s\n",
               ps5ai_mode_name(ai_status.mode),
               ps5opt_profile_name(selected_profile));
        printf("Otimizacao: %s\n\n",
               ps5opt_profile_description(selected_profile));

        if (file_count == 0) {
            printf("Nenhum arquivo encontrado.\n");
            printf("Coloque arquivos em games/.\n");
        } else {
            for (int i = 0; i < file_count; i++) {
                char path[PATH_LEN];
                snprintf(path, sizeof(path), "%s/%s",
                         current_dir, files[i]);

                printf("%s %s%s\n",
                       i == selected ? ">" : " ",
                       files[i],
                       is_directory(path) ? "/" : "");
            }
        }

        printf("\n----------------------------------------\n");
        printf("%s\n\n", message);
        printf("CIMA/BAIXO: selecionar\n");
        printf("A: verificar compatibilidade\n");
        printf("X: importar arquivo\n");
        printf("Y: executar NRO reconhecido\n");
        printf("L/R: trocar perfil | B: atualizar lista | +: sair\n");

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
