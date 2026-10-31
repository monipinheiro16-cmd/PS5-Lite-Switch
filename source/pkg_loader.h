#ifndef PKG_LOADER_H
#define PKG_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#define PKG_NAME_MAX 256

typedef struct {
    char name[PKG_NAME_MAX];
    unsigned long long size_bytes;
} PkgFileInfo;

int pkg_loader_is_pkg(const char *path);
int pkg_loader_get_info(const char *path, PkgFileInfo *info);
const char *pkg_loader_status_text(void);

#ifdef __cplusplus
}
#endif

#endif
