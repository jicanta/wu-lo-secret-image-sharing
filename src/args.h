#ifndef ARGS_H
#define ARGS_H

#include "commons.h"

typedef enum {
  MODE_DISTRIBUTE,
  MODE_RECOVER,
} Mode;

typedef struct {
  Mode mode;
  char* secret_image;
  int k;
  int n;     /* -1 if not provided */
  char* dir; /* NULL means current directory */
} Args;

int args_parse(int argc, char* argv[], Args* out);

void args_print_usage(const char* progname);

#endif
