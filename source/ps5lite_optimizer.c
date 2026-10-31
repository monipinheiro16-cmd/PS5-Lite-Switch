#include "ps5lite_optimizer.h"

const char *ps5opt_profile_name(Ps5OptProfile profile) {
    switch (profile) {
        case PS5OPT_BALANCED:    return "Equilibrado";
        case PS5OPT_PERFORMANCE: return "Desempenho";
        case PS5OPT_COOL:        return "Economia";
        default:                 return "Desconhecido";
    }
}

const char *ps5opt_profile_description(Ps5OptProfile profile) {
    switch (profile) {
        case PS5OPT_BALANCED:
            return "Equilibrio entre fluidez e consumo (informativo).";
        case PS5OPT_PERFORMANCE:
            return "Prioriza fluidez (informativo, nao altera clocks).";
        case PS5OPT_COOL:
            return "Prioriza temperatura e bateria (informativo).";
        default:
            return "Perfil desconhecido.";
    }
}

Ps5OptProfile ps5opt_next_profile(Ps5OptProfile profile) {
    return (Ps5OptProfile)(((int)profile + 1) % 3);
}
