#include "args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_positive_int(const char* flag, const char* str, int* out) {
  char* end;
  long val = strtol(str, &end, 10);

  if (*end != '\0' || val <= 0) {
    fprintf(stderr, "Error: '%s' expects a positive integer, got '%s'.\n", flag,
            str);
    return -1;
  }

  *out = (int)val;
  return 0;
}

static int require_next(const char* flag, int i, int argc) {
  if (i >= argc) {
    fprintf(stderr, "Error: '%s' requires an argument.\n", flag);
    return -1;
  }
  return 0;
}

void args_print_usage(const char* progname) {
  fprintf(stderr,
          "Usage:\n"
          "  Distribute: %s -d -secret <image.bmp> -k <num> [-n <num>] [-dir "
          "<dir>]\n"
          "  Recover:    %s -r -secret <image.bmp> -k <num> [-n <num>] [-dir "
          "<dir>]\n"
          "\n"
          "Mandatory parameters (must appear in this order):\n"
          "  -d | -r          Distribute or recover the secret image.\n"
          "  -secret <image>  BMP file to hide (-d) or to create (-r).\n"
          "  -k <num>         Minimum shares required (%d <= k <= %d).\n"
          "\n"
          "Optional parameters:\n"
          "  -n <num>         Total shares in the scheme (n >= %d, k <= n).\n"
          "  -dir <dir>       Directory containing carrier images (default: "
          "current dir).\n"
          "\n"
          "Examples:\n"
          "  %s -d -secret clave.bmp -k 2 -n 4 -dir varias\n"
          "  %s -d -secret clave.bmp -k 3\n"
          "  %s -r -secret secreta.bmp -k 2 -n 4 -dir varias\n"
          "  %s -r -secret secreta.bmp -k 3\n",
          progname, progname, K_MIN, K_MAX, N_MIN, progname, progname, progname,
          progname);
}

int args_parse(int argc, char* argv[], Args* out) {
  const char* progname = argc > 0 ? argv[0] : "visualSSS";

  out->mode = MODE_DISTRIBUTE;
  out->secret_image = NULL;
  out->k = 0;
  out->n = -1;
  out->dir = NULL;

  int i = 1;

  if (i >= argc) {
    fprintf(stderr, "Error: missing mode flag (-d or -r).\n");
    args_print_usage(progname);
    return -1;
  }

  if (strcmp(argv[i], "-d") == 0) {
    out->mode = MODE_DISTRIBUTE;
  } else if (strcmp(argv[i], "-r") == 0) {
    out->mode = MODE_RECOVER;
  } else {
    fprintf(stderr, "Error: first argument must be '-d' or '-r', got '%s'.\n",
            argv[i]);
    args_print_usage(progname);
    return -1;
  }
  i++;

  if (i >= argc || strcmp(argv[i], "-secret") != 0) {
    fprintf(stderr, "Error: expected '-secret <image>' after mode flag.\n");
    args_print_usage(progname);
    return -1;
  }
  i++;
  if (require_next("-secret", i, argc) < 0) {
    args_print_usage(progname);
    return -1;
  }
  out->secret_image = argv[i];
  i++;

  if (i >= argc || strcmp(argv[i], "-k") != 0) {
    fprintf(stderr, "Error: expected '-k <num>' after '-secret'.\n");
    args_print_usage(progname);
    return -1;
  }
  i++;
  if (require_next("-k", i, argc) < 0) {
    args_print_usage(progname);
    return -1;
  }
  if (parse_positive_int("-k", argv[i], &out->k) < 0) {
    args_print_usage(progname);
    return -1;
  }
  i++;

  while (i < argc) {
    if (strcmp(argv[i], "-n") == 0) {
      i++;
      if (require_next("-n", i, argc) < 0) {
        args_print_usage(progname);
        return -1;
      }
      if (parse_positive_int("-n", argv[i], &out->n) < 0) {
        args_print_usage(progname);
        return -1;
      }
      i++;
    } else if (strcmp(argv[i], "-dir") == 0) {
      i++;
      if (require_next("-dir", i, argc) < 0) {
        args_print_usage(progname);
        return -1;
      }
      out->dir = argv[i];
      i++;
    } else {
      fprintf(stderr, "Error: unexpected argument '%s'.\n", argv[i]);
      args_print_usage(progname);
      return -1;
    }
  }

  if (out->k < K_MIN || out->k > K_MAX) {
    fprintf(stderr, "Error: k must be between %d and %d, got %d.\n", K_MIN,
            K_MAX, out->k);
    args_print_usage(progname);
    return -1;
  }

  if (out->n != -1) {
    if (out->n < N_MIN) {
      fprintf(stderr, "Error: n must be at least %d, got %d.\n", N_MIN, out->n);
      args_print_usage(progname);
      return -1;
    }
    if (out->k > out->n) {
      fprintf(stderr, "Error: k (%d) must be <= n (%d).\n", out->k, out->n);
      args_print_usage(progname);
      return -1;
    }
  }

  return 0;
}
