#include <switch.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <dirent.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ps5lite_ai.h"
#include "ps5lite_optimizer.h"
#include "pkg_loader.h"
#include "pkg_reader.hpp"
#include "emulator_core.hpp"

#define ROOT_DIR   "sdmc:/switch/switchps5lite"
#define GAMES_DIR  "sdmc:/switch/switchps5lite/games"
#define IMPORT_DIR "sdmc:/switch/switchps5lite/imports"

#define MAX_FILES 256
#define NAME_LEN  256
#define PATH_LEN  512
#define VISIBLE_ROWS 20

enum GameStatus {
    STATUS_UNKNOWN,
    STATUS_NRO,
    STATUS_PKG,
    STATUS_UNSUPPORTED
};

static char files[MAX_FILES][NAME_LEN];
static int file_count = 0;
static int selected = 0;
static int top_row = 0;
static char message[256] = "A: abrir/verificar | X: importar | Y: executar NRO";
static char current_dir[PATH_LEN] = GAMES_DIR;
static Ps5AiStatus ai_status;
static Ps5OptProfile selected_profile = PS5OPT_BALANCED;
static bool launch_requested = false;

static void set_message(const char *text) {
    snprintf(message, sizeof(message), "%s", text);
}

static bool join_path(char *out, size_t size, const char *dir, const char *name) {
    int n = snprintf(out, size, "%s/%s", dir, name);
    return n >= 0 && (size_t)n < size;
}

static bool ends_with_ci(const char *name, const char *ext) {
    size_t n = strlen(name);
    size_t e = strlen(ext);
    if (n < e) return false;
    return strcasecmp(name + n - e, ext) == 0;
}

static bool is_directory(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static GameStatus check_game(const char *filename) {
    if (ends_with_ci(filename, ".nro"))
        return STATUS_NRO;

    if (ends_with_ci(filename, ".pkg"))
        return STATUS_PKG;

    if (ends_with_ci(filename, ".ps5") ||
        ends_with_ci(filename, ".elf") ||
        ends_with_ci(filename, ".iso"))
        return STATUS_UNSUPPORTED;

    return STATUS_UNKNOWN;
}

static int compare_names(const void *a, const void *b) {
    return strcasecmp((const char *)a, (const char *)b);
}

static void load_files(void) {
    file_count = 0;
    selected = 0;
    top_row = 0;

    DIR *dir = opendir(current_dir);
    if (!dir) {
        set_message("Erro ao abrir a pasta.");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(files[file_count], NAME_LEN, "%s", entry->d_name);
        file_count++;
    }
    closedir(dir);

    qsort(files, (size_t)file_count, NAME_LEN, compare_names);

    snprintf(message, sizeof(message), "Lista atualizada: %d arquivos.", file_count);
}

static void go_up(void) {
    if (strcmp(current_dir, GAMES_DIR) == 0) {
        load_files();
        return;
    }

    char *slash = strrchr(current_dir, '/');
    if (slash && (size_t)(slash - current_dir) >= strlen(GAMES_DIR))
        *slash = '\0';
    else
        snprintf(current_dir, sizeof(current_dir), "%s", GAMES_DIR);

    load_files();
}

static void clamp_scroll(void) {
    if (selected < top_row) top_row = selected;
    if (selected >= top_row + VISIBLE_ROWS) top_row = selected - VISIBLE_ROWS + 1;
    if (top_row < 0) top_row = 0;
}

static void open_or_verify_selected(void) {
    if (file_count == 0) {
        set_message("Nenhum arquivo para verificar.");
        return;
    }

    char path[PATH_LEN];
    if (!join_path(path, sizeof(path), current_dir, files[selected])) {
        set_message("Caminho muito longo.");
        return;
    }

    if (is_directory(path)) {
        snprintf(current_dir, sizeof(current_dir), "%s", path);
        load_files();
        return;
    }

    switch (check_game(files[selected])) {
        case STATUS_NRO:
            set_message("NRO reconhecido; compatibilidade nao garantida.");
            break;

        case STATUS_PKG: {
            PkgFileInfo finfo;
            if (!pkg_loader_get_info(path, &finfo)) {
                set_message("Nao foi possivel ler o PKG.");
                break;
            }

            PkgInfo header = inspectPkg(path);
            if (header.valid) {
                snprintf(message, sizeof(message),
                         "PKG %llu MB | magic 0x%08X | %s",
                         finfo.size_bytes >> 20,
                         (unsigned)header.magic,
                         header.message.c_str());
            } else {
                snprintf(message, sizeof(message), "PKG: %s", header.message.c_str());
            }
            break;
        }

        case STATUS_UNSUPPORTED:
            set_message("Formato nao suportado para execucao nativa.");
            break;

        default:
            set_message("Desconhecido: nao foi possivel confirmar.");
            break;
    }
}

static bool copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return false;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return false;
    }

    static char buffer[16384];
    size_t n;
    bool success = true;

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            success = false;
            break;
        }
    }

    if (ferror(in)) success = false;

    fclose(in);
    if (fclose(out) != 0) success = false;

    if (!success) remove(dst);

    return success;
}

static void import_selected(void) {
    if (file_count == 0) {
        set_message("Nenhum arquivo selecionado.");
        return;
    }

    char src[PATH_LEN];
    char dst[PATH_LEN];

    if (!join_path(src, sizeof(src), current_dir, files[selected]) ||
        !join_path(dst, sizeof(dst), IMPORT_DIR, files[selected])) {
        set_message("Caminho muito longo.");
        return;
    }

    if (is_directory(src)) {
        set_message("Selecione um arquivo, nao uma pasta.");
        return;
    }

    if (strcmp(src, dst) == 0) {
        set_message("Arquivo ja esta na pasta de importacao.");
        return;
    }

    if (copy_file(src, dst))
        set_message("Importacao concluida: arquivo copiado.");
    else
        set_message("Falha ao copiar. Confira espaco e arquivo.");
}

static void execute_selected(void) {
    if (file_count == 0) {
        set_message("Nenhum arquivo selecionado.");
        return;
    }

    char path[PATH_LEN];
    if (!join_path(path, sizeof(path), current_dir, files[selected])) {
        set_message("Caminho muito longo.");
        return;
    }

    if (is_directory(path)) {
        set_message("Selecione um arquivo, nao uma pasta.");
        return;
    }

    if (check_game(files[selected]) != STATUS_NRO) {
        set_message("Nao executado: formato nao reconhecido como NRO.");
        return;
    }

    char argv_line[PATH_LEN + 4];
    snprintf(argv_line, sizeof(argv_line), "\"%s\"", path);

    Result rc = envSetNextLoad(path, argv_line);
    if (R_FAILED(rc)) {
        set_message("Nao foi possivel abrir (abra pelo hbmenu).");
        return;
    }

    // O loader executa o proximo NRO quando este aplicativo encerrar.
    launch_requested = true;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    consoleInit(nullptr);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    mkdir(ROOT_DIR, 0777);
    mkdir(GAMES_DIR, 0777);
    mkdir(IMPORT_DIR, 0777);

    PS5LiteCore core;
    core.initialize();

    ps5ai_init();
    ai_status = ps5ai_update(16666667ULL);
    load_files();

    u64 last_tick = armGetSystemTick();

    while (appletMainLoop() && !launch_requested) {
        padUpdate(&pad);
        u64 down = padGetButtonsDown(&pad);

        if (down & HidNpadButton_Plus)
            break;

        if (file_count > 0 && (down & HidNpadButton_Down))
            selected = (selected + 1) % file_count;

        if (file_count > 0 && (down & HidNpadButton_Up))
            selected = (selected - 1 + file_count) % file_count;

        if (down & (HidNpadButton_L | HidNpadButton_R)) {
            selected_profile = ps5opt_next_profile(selected_profile);
            snprintf(message, sizeof(message), "Perfil %s ativado.",
                     ps5opt_profile_name(selected_profile));
        }

        if (down & HidNpadButton_A) open_or_verify_selected();
        if (down & HidNpadButton_X) import_selected();
        if (down & HidNpadButton_Y) execute_selected();
        if (down & HidNpadButton_B) go_up();

        if (down & HidNpadButton_Minus) {
            char path[PATH_LEN];
            if (file_count > 0 &&
                join_path(path, sizeof(path), current_dir, files[selected]) &&
                !is_directory(path)) {
                core.loadPackage(path);
            }
            core.run();
            set_message(core.status().c_str());
        }

        clamp_scroll();

        u64 now = armGetSystemTick();
        u64 elapsed_ns = armTicksToNs(now - last_tick);
        last_tick = now;
        ai_status = ps5ai_update(elapsed_ns);

        consoleClear();

        printf("========================================\n");
        printf("            PS5-LITE-SWITCH\n");
        printf("========================================\n\n");

        printf("Pasta: %s\n", current_dir);
        printf("IA: %s | %.1f FPS | Perfil: %s\n",
               ps5ai_mode_name(ai_status.mode),
               ai_status.fps,
               ps5opt_profile_name(selected_profile));
        printf("%s\n", ps5opt_profile_description(selected_profile));
        printf("%s\n\n", ai_status.recommendation);

        if (file_count == 0) {
            printf("Nenhum arquivo encontrado.\n");
            printf("Coloque arquivos em games/.\n");
        } else {
            int end = top_row + VISIBLE_ROWS;
            if (end > file_count) end = file_count;

            for (int i = top_row; i < end; i++) {
                char path[PATH_LEN];
                bool dir = join_path(path, sizeof(path), current_dir, files[i]) &&
                           is_directory(path);

                printf("%s %s%s\n", i == selected ? ">" : " ",
                       files[i], dir ? "/" : "");
            }
        }

        printf("\n----------------------------------------\n");
        printf("%s\n\n", message);
        printf("CIMA/BAIXO: selecionar\n");
        printf("A: abrir pasta / verificar arquivo\n");
        printf("B: voltar pasta (ou atualizar)\n");
        printf("X: importar | Y: executar NRO\n");
        printf("L/R: perfil | -: nucleo experimental | +: sair\n");

        consoleUpdate(nullptr);
    }

    consoleExit(nullptr);
    return 0;
}
