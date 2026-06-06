#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t read_u16(const uint8_t* b) {
  return (uint16_t)(b[0] | (b[1] << 8));
}
static uint32_t read_u32(const uint8_t* b) {
  return (uint32_t)(b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
}
static void write_u16(uint8_t* b, uint16_t v) {
  b[0] = v & 0xFF;
  b[1] = v >> 8;
}
static void write_u32(uint8_t* b, uint32_t v) {
  b[0] = v & 0xFF;
  b[1] = (v >> 8) & 0xFF;
  b[2] = (v >> 16) & 0xFF;
  b[3] = (v >> 24) & 0xFF;
}

int bmp_read(const char* path, Bmp* out) {
  FILE* f = NULL;
  uint8_t* pixels = NULL;
  uint8_t* row_buf = NULL;

  f = fopen(path, "rb");
  if (!f) {
    fprintf(stderr, "Error: cannot open '%s'.\n", path);
    goto fail;
  }

  uint8_t hdr[54];
  if (fread(hdr, 1, 54, f) != 54) {
    fprintf(stderr, "Error: '%s' is too small to be a BMP.\n", path);
    goto fail;
  }

  if (hdr[0] != 'B' || hdr[1] != 'M') {
    fprintf(stderr, "Error: '%s' is not a BMP file.\n", path);
    goto fail;
  }

  uint16_t seed = read_u16(hdr + 6);
  uint16_t shadow_idx = read_u16(hdr + 8);
  uint32_t pixel_offset = read_u32(hdr + 10);
  int32_t width = (int32_t)read_u32(hdr + 18);
  int32_t height = (int32_t)read_u32(hdr + 22);
  uint16_t bpp = read_u16(hdr + 28);
  uint32_t compression = read_u32(hdr + 30);
  uint16_t secret_width = read_u16(hdr + 50);
  uint16_t secret_height = read_u16(hdr + 52);

  if (bpp != 8) {
    fprintf(stderr,
            "Error: '%s' is %d bpp; only 8 bpp grayscale is supported.\n", path,
            bpp);
    goto fail;
  }
  if (compression != 0) {
    fprintf(
        stderr,
        "Error: '%s' uses compression; only uncompressed BMP is supported.\n",
        path);
    goto fail;
  }
  if (width <= 0 || height <= 0) {
    fprintf(stderr, "Error: '%s' has invalid dimensions (%dx%d).\n", path,
            width, height);
    goto fail;
  }

  if (fseek(f, (long)pixel_offset, SEEK_SET) != 0) {
    fprintf(stderr, "Error: cannot seek to pixel data in '%s'.\n", path);
    goto fail;
  }

  int row_stride = (width + 3) & ~3;

  pixels = malloc((size_t)width * (size_t)height);
  if (!pixels) {
    fprintf(stderr, "Error: out of memory.\n");
    goto fail;
  }

  row_buf = malloc((size_t)row_stride);
  if (!row_buf) {
    fprintf(stderr, "Error: out of memory.\n");
    goto fail;
  }

  for (int y = 0; y < height; y++) {
    if (fread(row_buf, 1, (size_t)row_stride, f) != (size_t)row_stride) {
      fprintf(stderr, "Error: unexpected end of pixel data in '%s'.\n", path);
      goto fail;
    }
    memcpy(pixels + (size_t)y * (size_t)width, row_buf, (size_t)width);
  }

  free(row_buf);
  fclose(f);
  out->width = (int)width;
  out->height = (int)height;
  out->pixels = pixels;
  out->seed = seed;
  out->shadow_idx = shadow_idx;
  out->secret_width = secret_width;
  out->secret_height = secret_height;
  return 0;

fail:
  free(row_buf);
  free(pixels);
  if (f) fclose(f);
  return -1;
}

int bmp_write(const char* path, const Bmp* bmp) {
  FILE* f = fopen(path, "wb");
  if (!f) {
    fprintf(stderr, "Error: cannot create '%s'.\n", path);
    return -1;
  }

  int width = bmp->width;
  int height = bmp->height;
  int row_stride = (width + 3) & ~3;
  int pad = row_stride - width;
  uint32_t pixel_offset =
      54 + 1024; /* 54-byte header + 256-entry color table */
  uint32_t pixel_data_size = (uint32_t)(row_stride * height);
  uint32_t file_size = pixel_offset + pixel_data_size;

  uint8_t hdr[54];
  memset(hdr, 0, sizeof(hdr));
  hdr[0] = 'B';
  hdr[1] = 'M';
  write_u32(hdr + 2, file_size);
  write_u16(hdr + 6, bmp->seed);
  write_u16(hdr + 8, bmp->shadow_idx);
  write_u32(hdr + 10, pixel_offset);
  write_u32(hdr + 14, 40);
  write_u32(hdr + 18, (uint32_t)width);
  write_u32(hdr + 22, (uint32_t)height);
  write_u16(hdr + 26, 1);
  write_u16(hdr + 28, 8);
  write_u32(hdr + 34, pixel_data_size);
  write_u32(hdr + 38, 2835); /* ~72 DPI */
  write_u32(hdr + 42, 2835);
  write_u32(hdr + 46, 256);
  write_u16(hdr + 50, bmp->secret_width);
  write_u16(hdr + 52, bmp->secret_height);
  fwrite(hdr, 1, 54, f);

  /* BMP color table is BGRA, not RGBA */
  uint8_t color_table[1024];
  for (int i = 0; i < 256; i++) {
    color_table[i * 4 + 0] = (uint8_t)i;
    color_table[i * 4 + 1] = (uint8_t)i;
    color_table[i * 4 + 2] = (uint8_t)i;
    color_table[i * 4 + 3] = 0;
  }
  fwrite(color_table, 1, 1024, f);

  static const uint8_t zero[3] = {0, 0, 0};
  for (int y = 0; y < height; y++) {
    fwrite(bmp->pixels + (size_t)y * (size_t)width, 1, (size_t)width, f);
    if (pad > 0) fwrite(zero, 1, (size_t)pad, f);
  }

  fclose(f);
  return 0;
}

void bmp_free(Bmp* bmp) {
  free(bmp->pixels);
  bmp->pixels = NULL;
  bmp->width = 0;
  bmp->height = 0;
  bmp->seed = 0;
  bmp->shadow_idx = 0;
}

void bmp_lsb_embed(uint8_t* pixels, const uint8_t* data, size_t nbytes) {
  size_t p = 0;
  for (size_t i = 0; i < nbytes; i++)
    for (int bit = 7; bit >= 0; bit--) {
      pixels[p] = (uint8_t)((pixels[p] & 0xFE) | ((data[i] >> bit) & 1));
      p++;
    }
}

void bmp_lsb_extract(const uint8_t* pixels, uint8_t* data, size_t nbytes) {
  size_t p = 0;
  for (size_t i = 0; i < nbytes; i++) {
    uint8_t v = 0;
    for (int bit = 7; bit >= 0; bit--) v = (uint8_t)((v << 1) | (pixels[p++] & 1));
    data[i] = v;
  }
}
