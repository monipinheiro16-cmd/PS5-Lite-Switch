#include "ps5lite_optimizer.h"

const char *ps5opt_profile_name(Ps5OptProfile profile) {
    switch (profile) {
        case PS5OPT_PERFORMANCE:
            return "DESEMPENHO (informativo)";
        case PS5OPT_COOL:
            return "FOCO EM TEMPERATURA (informativo)";
        case PS5OPT_BALANCED:
        default:
            return "EQUILIBRADO (informativo)";
    }
}

const char *ps5opt_profile_description(Ps5OptProfile profile) {
    switch (profile) {
        case PS5OPT_PERFORMANCE:
            return "Nao altera clocks nem FPS de outros jogos.";
        case PS5OPT_COOL:
            return "Nao controla ventoinha ou temperatura.";
        case PS5OPT_BALANCED:
        default:
            return "Perfil selecionado; sem alteracoes no sistema.";
    }
}

Ps5OptProfile ps5opt_next_profile(Ps5OptProfile profile) {
    if (profile == PS5OPT_BALANCED)
        return PS5OPT_PERFORMANCE;

    if (profile == PS5OPT_PERFORMANCE)
        return PS5OPT_COOL;

    return PS5OPT_BALANCED;
}
