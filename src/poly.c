#include "poly.h"

#include <string.h>

static inline int mod(int a) { return a < 0 ? a + MOD : a; }

static int inv_table[MOD];

void poly_init(void) {
  inv_table[1] = 1;
  for (int i = 2; i < MOD; i++)
    inv_table[i] = MOD - (int)((long)(MOD / i) * inv_table[MOD % i] % MOD);
}

static int lagrange_eval_at_zero(const int* xs, const int* ys, int r) {
  int result = 0;
  for (int i = 0; i < r; i++) {
    int num = 1, den = 1;
    for (int j = 0; j < r; j++) {
      if (j == i) continue;
      num = (int)((long)num * mod(-xs[j]) % MOD);
      den = (int)((long)den * mod(xs[i] - xs[j]) % MOD);
    }
    int term = (int)((long)mod(ys[i]) * num % MOD * inv_table[den] % MOD);
    result = (result + term) % MOD;
  }
  return result;
}

int poly_eval(const int* coeffs, int r, int x) {
  long result = 0;
  for (int i = r - 1; i >= 0; i--)
    result = (coeffs[i] + (long)x * result) % MOD;
  return (int)result;
}

void lagrange_recover(const int* xs, const int* ys, int r, int* coeffs_out) {
  int vals[K_MAX];
  memcpy(vals, ys, r * sizeof(int));

  for (int k = 0; k < r; k++) {
    coeffs_out[k] = lagrange_eval_at_zero(xs, vals, r);
    for (int i = 0; i < r; i++)
      vals[i] = (int)((long)mod(vals[i] - coeffs_out[k]) * inv_table[xs[i]] % MOD);
  }
}
