#ifndef POLY_H
#define POLY_H

#include "commons.h"

void poly_init(void);

int poly_eval(const int* coeffs, int r, int x);

void lagrange_recover(const int* xs, const int* ys, int r, int* coeffs_out);

#endif
