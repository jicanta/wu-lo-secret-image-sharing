#ifndef PRNG_H
#define PRNG_H

#include <stdint.h>

void prng_set_seed(int64_t s);

uint8_t prng_next_char(void);

#endif
