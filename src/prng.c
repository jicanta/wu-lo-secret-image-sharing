#include "prng.h"

static int64_t seed;

void prng_set_seed(int64_t s) {
  seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint8_t prng_next_char(void) {
  seed = (seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
  return (uint8_t)(seed >> 40);
}
