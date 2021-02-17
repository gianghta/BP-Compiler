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

        "x := getBool();\n"
        "if(x) then\n"
            "tmp := putString(\"T\");\n"
        "else"
            "tmp := putString(\"F\");\n"
        "end if;\n"
        "end program."
    );

    return 0;
}