#include "error.h"

void throw_error(char* msg)
{
    error_flag = true;
    printf("ERROR: %s", msg);
}