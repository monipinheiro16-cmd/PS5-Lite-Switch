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

#define BASE_DIR    "sdmc:/"
#define ROOT_DIR    "sdmc:/switch/switchps5lite"
#define IMPORT_DIR  "sdmc:/switch/switchps5lite/imports"
#define CONFIG_FILE "sdmc:/switch/switchps5lite/pasta.txt"

#define MAX_FILES 512
#define NAME_LEN  256
#define PATH_LEN  512
#define VISIBLE_ROWS 20

enum GameStatus {
    STATUS_UNKNOWN,
    STATUS_NRO,
    STATUS_PKG,
    STATUS_UNSUPPORTED
};

struct Entry {
    char name[NAME_LEN];
    bool dir;
};

static Entry entries[MAX_FILES];
static int file_count = 0;
static int selected = 0;
static int top_row = 0;
static char message[256] = "A: abrir pasta/verificar | ZR: salvar esta pasta";
static char current_dir[PATH_LEN] = BASE_DIR;
static Ps5AiStatus ai_status;
static Ps5OptProfile selected_profile = PS5OPT_BALANCED;
static bool launch_requested = false;

static void set_message(const char *text) {
    snprintf(message, sizeof(message), "%s", text);
}

static bool join_path(char *out, size_t size, const char *dir, const char *name) {
    size_t len = strlen(dir);
    const char *sep = (len > 0 && dir[len - 1] == '/') ? "" : "/";
    int n = snprintf(out, size, "%s%s%s", dir, sep, name);
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

// Pastas primeiro, depois arquivos, ambos em ordem alfabetica.
static int compare_entries(const void *a, const void *b) {
    const Entry *ea = (const Entry *)a;
    const Entry *eb = (const Entry *)b;
    if (ea->dir != eb->dir) return ea->dir ? -1 : 1;
    return strcasecmp(ea->name, eb->name);
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

        Entry *e = &entries[file_count];
        snprintf(e->name, NAME_LEN, "%s", entry->d_name);

        char path[PATH_LEN];
        e->dir = join_path(path, sizeof(path), current_dir, e->name) &&
                 is_directory(path);
        file_count++;
    }
    closedir(dir);

    qsort(entries, (size_t)file_count, sizeof(Entry), compare_entries);

    snprintf(message, sizeof(message), "%d itens nesta pasta.", file_count);
}

static void go_up(void) {
    if (strcmp(current_dir, BASE_DIR) == 0) {
        load_files();
        return;
    }

    char *slash = strrchr(current_dir, '/');
    if (slash) *slash = '\0';

    // "sdmc:" -> volta para a raiz "sdmc:/"
    if (strchr(current_dir, '/') == NULL)
        snprintf(current_dir, sizeof(current_dir), "%s", BASE_DIR);

    load_files();
}

static void save_folder(void) {
    mkdir(ROOT_DIR, 0777);

    FILE *f = fopen(CONFIG_FILE, "wb");
    if (!f) {
        set_message("Nao foi possivel salvar a pasta.");
        return;
    }

    fputs(current_dir, f);
    fclose(f);
    set_message("Pasta salva: abre aqui na proxima vez.");
}

static void load_saved_folder(void) {
    FILE *f = fopen(CONFIG_FILE, "rb");
    if (!f) return;

    char buf[PATH_LEN] = {};
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[n] = '\0';

    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
        buf[--n] = '\0';

    if (n > 0 && strncmp(buf, "sdmc:", 5) == 0 && is_directory(buf))
        snprintf(current_dir, sizeof(current_dir), "%s", buf);
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

    const Entry &sel = entries[selected];

    char path[PATH_LEN];
    if (!join_path(path, sizeof(path), current_dir, sel.name)) {
        set_message("Caminho muito longo.");
        return;
    }

    if (sel.dir) {
        snprintf(current_dir, sizeof(current_dir), "%s", path);
        load_files();
        return;
    }

    switch (check_game(sel.name)) {
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

    const Entry &sel = entries[selected];

    if (sel.dir) {
        set_message("Selecione um arquivo, nao uma pasta.");
        return;
    }

    char src[PATH_LEN];
    char dst[PATH_LEN];

    if (!join_path(src, sizeof(src), current_dir, sel.name) ||
        !join_path(dst, sizeof(dst), IMPORT_DIR, sel.name)) {
        set_message("Caminho muito longo.");
        return;
    }

    if (strcmp(src, dst) == 0) {
        set_message("Arquivo ja esta na pasta de importacao.");
        return;
    }

    mkdir(ROOT_DIR, 0777);
    mkdir(IMPORT_DIR, 0777);

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

    const Entry &sel = entries[selected];

    if (sel.dir) {
        set_message("Selecione um arquivo, nao uma pasta.");
        return;
    }

    char path[PATH_LEN];
    if (!join_path(path, sizeof(path), current_dir, sel.name)) {
        set_message("Caminho muito longo.");
        return;
    }

    if (check_game(sel.name) != STATUS_NRO) {
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

    PS5LiteCore core;
    core.initialize();

    ps5ai_init();
    ai_status = ps5ai_update(16666667ULL);

    load_saved_folder();
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
        if (down & HidNpadButton_B) go_up();
        if (down & HidNpadButton_X) import_selected();
        if (down & HidNpadButton_Y) execute_selected();
        if (down & HidNpadButton_ZR) save_folder();

        if (down & HidNpadButton_Minus) {
            char path[PATH_LEN];
            if (file_count > 0 && !entries[selected].dir &&
                join_path(path, sizeof(path), current_dir, entries[selected].name)) {
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
        printf("%s\n\n", ps5opt_profile_description(selected_profile));

        if (file_count == 0) {
            printf("Pasta vazia.\n");
            printf("B: voltar para a pasta anterior.\n");
        } else {
            int end = top_row + VISIBLE_ROWS;
            if (end > file_count) end = file_count;

            for (int i = top_row; i < end; i++) {
                printf("%s %s%s\n", i == selected ? ">" : " ",
                       entries[i].name, entries[i].dir ? "/" : "");
            }
        }

        printf("\n----------------------------------------\n");
        printf("%s\n\n", message);
        printf("CIMA/BAIXO: selecionar | A: abrir pasta/verificar\n");
        printf("B: voltar | ZR: salvar esta pasta como padrao\n");
        printf("X: importar | Y: executar NRO | L/R: perfil\n");
        printf("-: nucleo experimental | +: sair\n");

        consoleUpdate(nullptr);
    }

    consoleExit(nullptr);
    return 0;
}
