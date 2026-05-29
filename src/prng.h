#ifndef PRNG_H
#define PRNG_H

#include <stddef.h>
#include <stdint.h>

void prng_set_seed(int64_t s);

uint8_t prng_next_char(void);

void prng_permute(int64_t s, uint8_t* data, size_t len);

#endif
