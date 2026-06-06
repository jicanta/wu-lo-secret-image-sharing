#ifndef BMP_H
#define BMP_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  int width;
  int height;
  uint8_t* pixels; /* bottom-up row order (native BMP) */
  uint16_t seed;   /* PRNG seed — stored in BMP reserved bytes 6-7 */
  uint16_t
      shadow_idx;         /* shadow number 1..n — stored in BMP reserved bytes 8-9 */
  uint16_t secret_width;  /* secret width, k != 8 only — DIB bytes 50-51 */
  uint16_t secret_height; /* secret height, k != 8 only — DIB bytes 52-53 */
} Bmp;

int bmp_read(const char* path, Bmp* out);
int bmp_write(const char* path, const Bmp* bmp);
void bmp_free(Bmp* bmp);

void bmp_lsb_embed(uint8_t* pixels, const uint8_t* data, size_t nbytes);
void bmp_lsb_extract(const uint8_t* pixels, uint8_t* data, size_t nbytes);

#endif
