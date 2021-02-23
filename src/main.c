#include "include/bp.h"
#include <stdio.h>


int main(int argc, char *argv[])
{
    // Usage check for correct filename
    // if (argc < 2)
    // {
    //     printf("usage: ./bp.out <file name>\n");
    //     return 1;
    // }

    bp_compile(
        "begin\n"
        "c := 16.50\n"
        "y := 2520;\n"
        "x := false;\n"
        "if(x) then\n"
            "tmp := \"Tasdczczxc\";\n"
        "else\n"
            "tmp := \"Fasdasdzxc\";\n"
        "end if\n;"
        "end program."
    );

    return 0;
}