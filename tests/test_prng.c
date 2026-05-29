#include <stdio.h>

#include "prng.h"

static int failures = 0;

#define CHECK(cond)                                          \
  do {                                                       \
    if (!(cond)) {                                           \
      printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                            \
    }                                                        \
  } while (0)

/* Golden sequence for setSeed(10), matching java.util.Random and the C
   reference in the course annex (section 4.2.4). */
static const uint8_t GOLDEN[16] = {186, 114, 65,  105, 15,  172, 62,  94,
                                   209, 91,  94,  167, 219, 228, 183, 105};

static void test_golden_seed_10(void) {
  prng_set_seed(10);
  for (int i = 0; i < 16; i++) CHECK(prng_next_char() == GOLDEN[i]);
}

static void test_permute_is_involutive(void) {
  uint8_t data[8] = {0, 17, 42, 99, 128, 200, 255, 7};
  uint8_t original[8];
  for (int i = 0; i < 8; i++) original[i] = data[i];

  prng_permute(1234, data, 8);
  prng_permute(1234, data, 8); /* same seed restores the input */
  for (int i = 0; i < 8; i++) CHECK(data[i] == original[i]);
}

int main(void) {
  test_golden_seed_10();
  test_permute_is_involutive();

  if (failures == 0)
    printf("test_prng: all tests passed\n");
  else
    printf("test_prng: %d check(s) failed\n", failures);
  return failures == 0 ? 0 : 1;
}
