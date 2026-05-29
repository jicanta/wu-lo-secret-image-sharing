#include <stdio.h>

#include "poly.h"

static int failures = 0;

#define CHECK(cond)                                              \
  do {                                                           \
    if (!(cond)) {                                               \
      printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);     \
      failures++;                                                \
    }                                                            \
  } while (0)

static void test_eval(void) {
  /* f(x) = 3 + 2x + x^2 over GF(257) */
  int coeffs[3] = {3, 2, 1};
  CHECK(poly_eval(coeffs, 3, 0) == 3);
  CHECK(poly_eval(coeffs, 3, 1) == 6);
  CHECK(poly_eval(coeffs, 3, 2) == 11);
  CHECK(poly_eval(coeffs, 3, 5) == 38);
  /* wraps mod 257: f(20) = 3 + 40 + 400 = 443 = 443 - 257 = 186 */
  CHECK(poly_eval(coeffs, 3, 20) == 186);
}

static void test_lagrange_roundtrip(void) {
  int coeffs[8] = {10, 20, 30, 40, 50, 60, 70, 255};
  int xs[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  int ys[8];
  for (int i = 0; i < 8; i++) ys[i] = poly_eval(coeffs, 8, xs[i]);

  int out[8];
  lagrange_recover(xs, ys, 8, out);
  for (int i = 0; i < 8; i++) CHECK(out[i] == coeffs[i]);
}

static void test_lagrange_nonconsecutive_x(void) {
  int coeffs[3] = {123, 45, 200};
  int xs[3] = {2, 5, 9};
  int ys[3];
  for (int i = 0; i < 3; i++) ys[i] = poly_eval(coeffs, 3, xs[i]);

  int out[3];
  lagrange_recover(xs, ys, 3, out);
  for (int i = 0; i < 3; i++) CHECK(out[i] == coeffs[i]);
}

static void test_share_handles_256(void) {
  /* f(x) = 255 + x hits 256 at x=1; poly_share keeps decrementing the first
     non-zero coeff and retrying until no evaluation equals 256. */
  int coeffs[2] = {255, 1};
  int out[3];
  poly_share(coeffs, 2, 3, out);

  CHECK(coeffs[0] < 255);  /* first non-zero coeff was decremented */
  CHECK(coeffs[1] == 1);   /* later coeffs untouched */
  for (int x = 1; x <= 3; x++) {
    CHECK(out[x - 1] != 256);
    CHECK(out[x - 1] == poly_eval(coeffs, 2, x)); /* shares match final poly */
  }
}

int main(void) {
  poly_init();
  test_eval();
  test_lagrange_roundtrip();
  test_lagrange_nonconsecutive_x();
  test_share_handles_256();

  if (failures == 0)
    printf("test_poly: all tests passed\n");
  else
    printf("test_poly: %d check(s) failed\n", failures);
  return failures == 0 ? 0 : 1;
}
