#ifndef BMP_H
#define BMP_H

#include <stdint.h>

typedef struct {
  int width;
  int height;
  uint8_t* pixels; /* bottom-up row order (native BMP) */
  uint16_t seed;   /* PRNG seed — stored in BMP reserved bytes 6-7 */
  uint16_t
      shadow_idx; /* shadow number 1..n — stored in BMP reserved bytes 8-9 */
} Bmp;

int bmp_read(const char* path, Bmp* out);
int bmp_write(const char* path, const Bmp* bmp);
void bmp_free(Bmp* bmp);

#endif
