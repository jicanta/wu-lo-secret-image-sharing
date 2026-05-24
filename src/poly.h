#ifndef POLY_H
#define POLY_H

#define MOD 257

int poly_eval(const int* coeffs, int r, int x);

void lagrange_recover(const int* xs, const int* ys, int r, int* coeffs_out);

#endif
