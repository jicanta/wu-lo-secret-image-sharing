#include "args.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    Args args;

    if (args_parse(argc, argv, &args) < 0)
        return EXIT_FAILURE;

    /* ---- Dispatch ---- */
    printf("Mode:   %s\n", args.mode == MODE_DISTRIBUTE ? "distribute" : "recover");
    printf("Secret: %s\n", args.secret_image);
    printf("k:      %d\n", args.k);
    if (args.n != -1)
        printf("n:      %d\n", args.n);
    else if (args.mode == MODE_DISTRIBUTE)
        printf("n:      (from directory)\n");
    printf("dir:    %s\n", args.dir ? args.dir : ".");

    return EXIT_SUCCESS;
}
