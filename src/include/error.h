#ifndef ERROR_H
#define ERROR_H

#include "token.h"

#define MAX_ERRORS 20

typedef enum {
    E_END_OF_COMMENT,
    E_IDENT_TOO_LONG,
    E_STRING_TOO_LONG,
    E_INVALID_STRING,
    E_INVALID_SYMBOL,
    E_INVALID_IDENT,
} error_code;

#define E_MES_ENDOFCOMMENT "Missing end of comment annotation"
#define E_MES_STRINGTOOLONG "String is too long"
#define E_MES_INVALID_STRING "Invalid string"

void throw_error(error_code code, int line_number, int column_number);
void missing_token(TokenType type, int line_number, int column_number);
void assert_parser(char* msg);

#endif