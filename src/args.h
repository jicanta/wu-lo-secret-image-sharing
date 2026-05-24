#ifndef ARGS_H
#define ARGS_H

#define K_MIN  2
#define K_MAX  10
#define N_MIN  2

typedef enum {
    MODE_DISTRIBUTE,
    MODE_RECOVER,
} Mode;

typedef struct {
    Mode  mode;
    char *secret_image; /* value of -secret */
    int   k;            /* value of -k */
    int   n;            /* value of -n; -1 if not provided */
    char *dir;          /* value of -dir; NULL means current directory */
} Args;

/*
 * Parses argv according to the required argument order:
 *   -d|-r  -secret <img>  -k <num>  [-n <num>]  [-dir <dir>]
 *
 * Returns 0 on success, -1 on error (error message already printed).
 * On success, *out is fully populated; caller does not own the strings
 * (they point into argv).
 */
int  args_parse(int argc, char *argv[], Args *out);

/* Prints usage to stderr. */
void args_print_usage(const char *progname);

#endif /* ARGS_H */
