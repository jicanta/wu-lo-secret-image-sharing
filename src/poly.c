#include "poly.h"

#include <string.h>

#define MAXK 10

/* Map any integer to [0, MOD-1] */
static int mod(int a) {
    return ((a % MOD) + MOD) % MOD;
}

/* Modular inverse via extended Euclidean algorithm. MOD must be prime. */
static int modinv(int a) {
    int t = 0, newt = 1;
    int r = MOD, newr = mod(a);
    while (newr != 0) {
        int q = r / newr;
        int tmp;
        tmp = t - q * newt; t = newt; newt = tmp;
        tmp = r - q * newr; r = newr; newr = tmp;
    }
    return t < 0 ? t + MOD : t;
}

/* Evaluate the Lagrange interpolating polynomial at x=0 given r (xs, ys) pairs. */
static int lagrange_eval_at_zero(const int *xs, const int *ys, int r) {
    int result = 0;
    for (int i = 0; i < r; i++) {
        int num = 1, den = 1;
        for (int j = 0; j < r; j++) {
            if (j == i) continue;
            num = (int)((long)num * mod(-xs[j])       % MOD);
            den = (int)((long)den * mod(xs[i] - xs[j]) % MOD);
        }
        int term = (int)((long)mod(ys[i]) * num % MOD * modinv(den) % MOD);
        result = (result + term) % MOD;
    }
    return result;
}

int poly_eval(const int *coeffs, int r, int x) {
    long result = 0;
    for (int i = r - 1; i >= 0; i--)
        result = (coeffs[i] + (long)x * result) % MOD;
    return (int)result;
}

/*
 * Recover coefficients using Lagrange interpolation + polynomial deflation.
 *
 * At each step k:
 *   1. Evaluate the current polynomial at x=0 via Lagrange → a_k.
 *   2. Deflate: new_vals[i] = (vals[i] - a_k) / xs[i]   (mod MOD)
 *      This strips the constant term and divides out x, giving the next
 *      lower-degree polynomial evaluated at the same xs.
 */
void lagrange_recover(const int *xs, const int *ys, int r, int *coeffs_out) {
    int vals[MAXK];
    memcpy(vals, ys, r * sizeof(int));

    for (int k = 0; k < r; k++) {
        coeffs_out[k] = lagrange_eval_at_zero(xs, vals, r);
        for (int i = 0; i < r; i++)
            vals[i] = (int)((long)mod(vals[i] - coeffs_out[k]) * modinv(xs[i]) % MOD);
    }
}
