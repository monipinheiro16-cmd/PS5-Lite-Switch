#ifndef PS5LITE_OPTIMIZER_H
#define PS5LITE_OPTIMIZER_H

typedef enum {
    PS5OPT_BALANCED = 0,
    PS5OPT_PERFORMANCE = 1,
    PS5OPT_COOL = 2
} Ps5OptProfile;

const char *ps5opt_profile_name(Ps5OptProfile profile);
const char *ps5opt_profile_description(Ps5OptProfile profile);
Ps5OptProfile ps5opt_next_profile(Ps5OptProfile profile);

#endif
