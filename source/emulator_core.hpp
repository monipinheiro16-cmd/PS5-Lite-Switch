#pragma once
#include <cstdint>
#include <string>

class PS5LiteCore {
public:
    bool initialize() {
        initialized = true;
        lastMessage = "Nucleo inicializado (sem carregador PS5).";
        return true;
    }

    bool loadPackage(const std::string& path) {
        (void)path;
        if (!initialized) return false;

        // Ainda nao existe carregador de executaveis PS5.
        loaded = false;
        lastMessage = "Carregador PS5 ainda nao implementado.";
        return false;
    }

    void run() {
        if (!initialized) {
            lastMessage = "Nucleo nao inicializado.";
            return;
        }
        // Futuro: CPU, memoria, GPU e APIs do sistema.
        lastMessage = "Nucleo de emulacao ainda em desenvolvimento.";
    }

    bool isLoaded() const { return loaded; }

    std::string status() const {
        return lastMessage;
    }

private:
    bool initialized = false;
    bool loaded = false;
    std::string lastMessage = "Nucleo nao inicializado.";
};
