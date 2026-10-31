
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "pkg_loader.h"

int pkg_loader_is_pkg(const char *path) {
    if (path == NULL) return 0;

    const char *ext = strrchr(path, '.');
    if (ext == NULL) return 0;

    return strcmp(ext, ".pkg") == 0 ||
           strcmp(ext, ".PKG") == 0;
}

int pkg_loader_get_info(const char *path, PkgFileInfo *info) {
    if (path == NULL || info == NULL ||
        !pkg_loader_is_pkg(path)) {
        return 0;
    }

    struct stat st;

    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return 0;
    }

    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    snprintf(info->name, sizeof(info->name), "%s", name);
    info->size_bytes = (unsigned long long)st.st_size;

    return 1;
}

const char *pkg_loader_status_text(void) {
    return "Arquivo PKG identificado com sucesso.";
}
