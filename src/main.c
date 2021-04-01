#include "include/bp.h"
#include <stdio.h>


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: ./bp.out <file name>\n");
        return 1;
    }

    bp_compile_file(argv[1]);

    return 0;
}