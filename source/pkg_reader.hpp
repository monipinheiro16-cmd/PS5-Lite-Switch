#pragma once
#include <cstdint>
#include <string>

struct PkgInfo {
    bool valid = false;   // true = cabecalho de 16 bytes lido (nao significa PKG valido)
    uint64_t size = 0;
    uint32_t magic = 0;
    std::string message;
};

PkgInfo inspectPkg(const std::string& path);
