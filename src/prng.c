#include "prng.h"

static uint64_t seed;

void prng_set_seed(int64_t s) {
  seed = ((uint64_t)s ^ 0x5DEECE66DULL) & ((1ULL << 48) - 1);
}

uint8_t prng_next_char(void) {
  seed = (seed * 0x5DEECE66DULL + 0xBULL) & ((1ULL << 48) - 1);
  return (uint8_t)(seed >> 40);
}

void prng_permute(int64_t s, uint8_t* data, size_t len) {
  prng_set_seed(s);
  for (size_t i = 0; i < len; i++) data[i] ^= prng_next_char();
}
