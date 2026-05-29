#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

static int failures = 0;

#define CHECK(cond)                                          \
  do {                                                       \
    if (!(cond)) {                                           \
      printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                            \
    }                                                        \
  } while (0)

static void test_lsb_roundtrip(void) {
  uint8_t data[4] = {0x12, 0xFF, 0x00, 0xA5};
  uint8_t pixels[32];
  for (int i = 0; i < 32; i++) pixels[i] = (uint8_t)(0xF0 | (i & 1));

  bmp_lsb_embed(pixels, data, 4);

  /* only the least significant bit of each pixel must change */
  for (int i = 0; i < 32; i++) CHECK((pixels[i] & 0xFE) == 0xF0);

  uint8_t out[4];
  bmp_lsb_extract(pixels, out, 4);
  for (int i = 0; i < 4; i++) CHECK(out[i] == data[i]);
}

static void test_write_read_roundtrip(void) {
  /* width=5 forces row padding (stride 8) to exercise the padding path */
  const int w = 5, h = 3;
  uint8_t px[15];
  for (int i = 0; i < 15; i++) px[i] = (uint8_t)(i * 7);

  Bmp in = {0};
  in.width = w;
  in.height = h;
  in.pixels = px;
  in.seed = 0xABCD;
  in.shadow_idx = 7;

  const char* path = "/tmp/visualsis_test_bmp.bmp";
  CHECK(bmp_write(path, &in) == 0);

  Bmp out = {0};
  CHECK(bmp_read(path, &out) == 0);
  CHECK(out.width == w);
  CHECK(out.height == h);
  CHECK(out.seed == 0xABCD);
  CHECK(out.shadow_idx == 7);
  if (out.pixels)
    for (int i = 0; i < w * h; i++) CHECK(out.pixels[i] == px[i]);

  bmp_free(&out);
  remove(path);
}

int main(void) {
  test_lsb_roundtrip();
  test_write_read_roundtrip();

  if (failures == 0)
    printf("test_bmp: all tests passed\n");
  else
    printf("test_bmp: %d check(s) failed\n", failures);
  return failures == 0 ? 0 : 1;
}
