#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "args.h"
#include "bmp.h"
#include "commons.h"
#include "dir.h"
#include "poly.h"
#include "prng.h"

static int read_carriers(char** paths, int n, size_t min_pixels, Bmp* out) {
  int read = 0;
  for (; read < n; read++) {
    if (bmp_read(paths[read], &out[read]) < 0) goto fail;
    if (out[read].width != out[0].width ||
        out[read].height != out[0].height) {
      fprintf(stderr,
              "Error: carrier '%s' is %dx%d but '%s' is %dx%d; all carriers "
              "must share the same size.\n",
              paths[read], out[read].width, out[read].height, paths[0],
              out[0].width, out[0].height);
      read++;
      goto fail;
    }
    size_t px = (size_t)out[read].width * (size_t)out[read].height;
    if (px < min_pixels) {
      fprintf(stderr,
              "Error: carrier '%s' has %zu pixels but the scheme needs at "
              "least %zu.\n",
              paths[read], px, min_pixels);
      read++;
      goto fail;
    }
  }
  return 0;

fail:
  for (int i = 0; i < read; i++) bmp_free(&out[i]);
  return -1;
}

static int distribute(const Args* a) {
  Bmp secret = {0};
  Bmp* carriers = NULL;
  char** paths = NULL;
  int count = 0;
  int n = 0;
  int carriers_read = 0;
  uint8_t** shadows = NULL;
  int* shares = NULL;
  int rc = -1;

  if (bmp_read(a->secret_image, &secret) < 0) return -1;

  size_t npix = (size_t)secret.width * (size_t)secret.height;
  size_t sections = (npix + (size_t)a->k - 1) / (size_t)a->k;
  size_t min_pixels = sections * 8;

  uint16_t seed = (uint16_t)(rand() & 0xFFFF);
  prng_permute((int64_t)seed, secret.pixels, npix);

  if (dir_list_bmps(a->dir, &paths, &count) < 0) goto done;
  if (count == 0) {
    fprintf(stderr, "Error: no BMP carriers found in '%s'.\n",
            a->dir ? a->dir : ".");
    goto done;
  }

  n = a->n == -1 ? count : a->n;
  if (n < N_MIN || a->k > n) {
    fprintf(stderr, "Error: need %d <= k(%d) <= n(%d).\n", N_MIN, a->k, n);
    goto done;
  }
  if (count < n) {
    fprintf(stderr, "Error: need %d carriers but only %d found in '%s'.\n", n,
            count, a->dir ? a->dir : ".");
    goto done;
  }

  carriers = calloc((size_t)n, sizeof(Bmp));
  shadows = calloc((size_t)n, sizeof(uint8_t*));
  shares = malloc((size_t)n * sizeof(int));
  if (!carriers || !shadows || !shares) {
    fprintf(stderr, "Error: out of memory.\n");
    goto done;
  }

  if (read_carriers(paths, n, min_pixels, carriers) < 0) goto done;
  carriers_read = n;

  for (int c = 0; c < n; c++) {
    shadows[c] = malloc(sections);
    if (!shadows[c]) {
      fprintf(stderr, "Error: out of memory.\n");
      goto done;
    }
  }

  for (size_t s = 0; s < sections; s++) {
    int coeffs[K_MAX];
    for (int j = 0; j < a->k; j++) {
      size_t idx = s * (size_t)a->k + (size_t)j;
      coeffs[j] = idx < npix ? secret.pixels[idx] : 0;
    }
    poly_share(coeffs, a->k, n, shares);
    for (int c = 0; c < n; c++) shadows[c][s] = (uint8_t)shares[c];
  }

  for (int c = 0; c < n; c++) {
    bmp_lsb_embed(carriers[c].pixels, shadows[c], sections);
    carriers[c].seed = seed;
    carriers[c].shadow_idx = (uint16_t)(c + 1);
    carriers[c].secret_width = (uint32_t)secret.width;
    carriers[c].secret_height = (uint32_t)secret.height;
    if (bmp_write(paths[c], &carriers[c]) < 0) goto done;
  }

  rc = 0;

done:
  if (shadows)
    for (int c = 0; c < n; c++) free(shadows[c]);
  free(shadows);
  free(shares);
  for (int c = 0; c < carriers_read; c++) bmp_free(&carriers[c]);
  free(carriers);
  if (paths) dir_free(paths, count);
  bmp_free(&secret);
  return rc;
}

static int recover(const Args* a) {
  Bmp* shadows = NULL;
  char** paths = NULL;
  int count = 0;
  int shadows_read = 0;
  uint8_t** ext = NULL;
  uint8_t* q = NULL;
  int* xs = NULL;
  int rc = -1;

  if (dir_list_bmps(a->dir, &paths, &count) < 0) return -1;
  if (count < a->k) {
    fprintf(stderr, "Error: need at least k=%d shadows but found %d in '%s'.\n",
            a->k, count, a->dir ? a->dir : ".");
    goto done;
  }

  shadows = calloc((size_t)a->k, sizeof(Bmp));
  ext = calloc((size_t)a->k, sizeof(uint8_t*));
  xs = malloc((size_t)a->k * sizeof(int));
  if (!shadows || !ext || !xs) {
    fprintf(stderr, "Error: out of memory.\n");
    goto done;
  }

  for (int i = 0; i < a->k; i++) {
    if (bmp_read(paths[i], &shadows[i]) < 0) goto done;
    shadows_read++;
    if (shadows[i].width != shadows[0].width ||
        shadows[i].height != shadows[0].height) {
      fprintf(stderr, "Error: shadow '%s' has different dimensions.\n",
              paths[i]);
      goto done;
    }
    if (shadows[i].secret_width != shadows[0].secret_width ||
        shadows[i].secret_height != shadows[0].secret_height) {
      fprintf(stderr,
              "Error: shadow '%s' was made for a different secret size.\n",
              paths[i]);
      goto done;
    }
    xs[i] = shadows[i].shadow_idx;
    for (int j = 0; j < i; j++)
      if (xs[j] == xs[i]) {
        fprintf(stderr, "Error: shadows share index %d; need k distinct ones.\n",
                xs[i]);
        goto done;
      }
  }

  size_t shadow_pixels = (size_t)shadows[0].width * (size_t)shadows[0].height;
  size_t m = (size_t)shadows[0].secret_width * (size_t)shadows[0].secret_height;
  if (m == 0) {
    fprintf(stderr, "Error: shadows carry no secret-size metadata.\n");
    goto done;
  }
  size_t sections = (m + (size_t)a->k - 1) / (size_t)a->k;
  if (shadow_pixels < sections * 8) {
    fprintf(stderr,
            "Error: shadow has %zu pixels but %zu are needed to recover the "
            "secret.\n",
            shadow_pixels, sections * 8);
    goto done;
  }

  for (int i = 0; i < a->k; i++) {
    ext[i] = malloc(sections);
    if (!ext[i]) {
      fprintf(stderr, "Error: out of memory.\n");
      goto done;
    }
    bmp_lsb_extract(shadows[i].pixels, ext[i], sections);
  }

  q = malloc(m);
  if (!q) {
    fprintf(stderr, "Error: out of memory.\n");
    goto done;
  }

  for (size_t s = 0; s < sections; s++) {
    int ys[K_MAX];
    int coeffs[K_MAX];
    for (int i = 0; i < a->k; i++) ys[i] = ext[i][s];
    lagrange_recover(xs, ys, a->k, coeffs);
    for (int j = 0; j < a->k; j++) {
      size_t idx = s * (size_t)a->k + (size_t)j;
      if (idx < m) q[idx] = (uint8_t)coeffs[j];
    }
  }

  prng_permute((int64_t)shadows[0].seed, q, m);

  Bmp out = {0};
  out.width = (int)shadows[0].secret_width;
  out.height = (int)shadows[0].secret_height;
  out.pixels = q;
  if (bmp_write(a->secret_image, &out) < 0) goto done;

  rc = 0;

done:
  free(q);
  if (ext)
    for (int i = 0; i < a->k; i++) free(ext[i]);
  free(ext);
  free(xs);
  for (int i = 0; i < shadows_read; i++) bmp_free(&shadows[i]);
  free(shadows);
  if (paths) dir_free(paths, count);
  return rc;
}

int main(int argc, char* argv[]) {
  poly_init();
  srand((unsigned)time(NULL));

  Args args;
  if (args_parse(argc, argv, &args) < 0) return EXIT_FAILURE;

  int rc = args.mode == MODE_DISTRIBUTE ? distribute(&args) : recover(&args);
  return rc < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
