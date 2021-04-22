#ifndef BP_H
#define BP_H

#include <stdbool.h>

void bp_compile(char* src, const char* name, bool parser_flag, bool table_flag, bool jit_flag);
void bp_compile_file(const char* filename, bool parser_flag, bool table_flag, bool jit_flag);

#endif