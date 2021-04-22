#include "include/bp.h"
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: ./bp.out [OPTIONS] <file name>\n");
        printf("%s",
            "Options:\n"
            "  -dp          Debug statements from parser.\n"
            "  -dt          Debug output from symbol table.\n"
            "  -dm          Debug output from LLVM JIT compiler.\n"
        );
        return 1;
    }

    bool parser_flag = false, table_flag = false, jit_flag = false;
    int counter = 1;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'd')
            {
                if (argv[i][2] == 'p')
                {
                    parser_flag = true;
                    counter++;
                }
                else if (argv[i][2] == 't')
                {
                    table_flag = true;
                    counter++;
                }
                else if (argv[i][2] == 'm')
                {
                    jit_flag = true;
                    counter++;
                }
            }
        }
    }

    bp_compile_file(argv[counter], parser_flag, table_flag, jit_flag);
    return 0;
}