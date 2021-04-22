#ifndef ERROR_FILE_H
#define ERROR_FILE_H

#include "token.h"

#define MAX_ERRORS 20

extern bool error_flag;

// void throw_error(error_code code, int line_number, int column_number);
// void missing_token(TokenType type, int line_number, int column_number);
// void assert_parser(char* msg);

void set_file_name(const char* name);
void throw_error(char* msg, Token* tok);
void debug_parser_statement(char* msg, bool flag);

#endif