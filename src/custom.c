#include "include/custom.h"

/*
 * Helper function that returns a formatted string
 */
char* concatf(const char* fmt, ...)
{
    va_list args;
    char* buf = NULL;
    va_start(args, fmt);
    int n = vasprintf(&buf, fmt, args);
    va_end(args);
    if (n < 0)
    {
        free(buf);
        buf = NULL;
    }
    return buf;
}