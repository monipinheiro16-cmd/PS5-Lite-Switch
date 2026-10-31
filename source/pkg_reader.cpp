#include "pkg_reader.hpp"
#include <cstdio>

PkgInfo inspectPkg(const std::string& path) {
    PkgInfo info;

    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        info.message = "Nao foi possivel abrir o arquivo.";
        return info;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        info.message = "Nao foi possivel medir o arquivo.";
        return info;
    }

    long fileSize = ftell(file);
    if (fileSize < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        info.message = "Nao foi possivel medir o arquivo.";
        return info;
    }

    unsigned char header[16] = {};
    size_t count = fread(header, 1, sizeof(header), file);
    fclose(file);

    info.size = static_cast<uint64_t>(fileSize);

    if (fileSize < 16 || count != sizeof(header)) {
        info.message = "Arquivo incompleto.";
        return info;
    }

    info.magic =
        (static_cast<uint32_t>(header[0]) << 24) |
        (static_cast<uint32_t>(header[1]) << 16) |
        (static_cast<uint32_t>(header[2]) << 8) |
         static_cast<uint32_t>(header[3]);

    info.valid = true;
    info.message = "Cabecalho lido. Compatibilidade com PS5 nao confirmada.";

    return info;
}
