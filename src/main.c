#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "args.h"
#include "bmp.h"
#include "commons.h"
#include "dir.h"
#include "poly.h"
#include "prng.h"

#define STEGO_K 8

static int read_carriers(char** paths, int n, int width, int height,
                         Bmp* out) {
  int read = 0;
  for (; read < n; read++) {
    if (bmp_read(paths[read], &out[read]) < 0) goto fail;
    if (out[read].width != width || out[read].height != height) {
      fprintf(stderr,
              "Error: carrier '%s' is %dx%d but the secret is %dx%d.\n",
              paths[read], out[read].width, out[read].height, width, height);
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

  if (a->k != STEGO_K) {
    fprintf(stderr, "Error: only k=%d is implemented yet, got k=%d.\n", STEGO_K,
            a->k);
    return -1;
  }

  if (bmp_read(a->secret_image, &secret) < 0) return -1;

  size_t npix = (size_t)secret.width * (size_t)secret.height;
  if (npix % (size_t)a->k != 0) {
    fprintf(stderr, "Error: secret has %zu pixels, not a multiple of k=%d.\n",
            npix, a->k);
    goto done;
  }

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

  if (read_carriers(paths, n, secret.width, secret.height, carriers) < 0)
    goto done;
  carriers_read = n;

  size_t sections = npix / (size_t)a->k;
  for (int c = 0; c < n; c++) {
    shadows[c] = malloc(sections);
    if (!shadows[c]) {
      fprintf(stderr, "Error: out of memory.\n");
      goto done;
    }
  }

  for (size_t s = 0; s < sections; s++) {
    int coeffs[K_MAX];
    for (int j = 0; j < a->k; j++)
      coeffs[j] = secret.pixels[s * (size_t)a->k + (size_t)j];
    poly_share(coeffs, a->k, n, shares);
    for (int c = 0; c < n; c++) shadows[c][s] = (uint8_t)shares[c];
  }

  for (int c = 0; c < n; c++) {
    bmp_lsb_embed(carriers[c].pixels, shadows[c], sections);
    carriers[c].seed = seed;
    carriers[c].shadow_idx = (uint16_t)(c + 1);
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
  (void)a;
  fprintf(stderr, "Error: recovery is not implemented yet.\n");
  return -1;
}

int main(int argc, char* argv[]) {
  poly_init();
  srand((unsigned)time(NULL));

  Args args;
  if (args_parse(argc, argv, &args) < 0) return EXIT_FAILURE;

  int rc = args.mode == MODE_DISTRIBUTE ? distribute(&args) : recover(&args);
  return rc < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
