#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#define MAX_STRING_LENGTH 50

typedef enum
{
    T_UNKNOWN,
    T_EOF,
    T_ERROR,
    T_ID,
    T_EQ,
    T_NOT_EQ,
    T_LTEQ,
    T_LT,
    T_GTEQ,
    T_GT,
    T_STRING,
    T_CHAR,
    T_NUMBER_INT,
    T_NUMBER_FLOAT,
    T_SEMI_COLON,
    T_LPAREN,
    T_RPAREN,
    T_LBRACKET,
    T_RBRACKET,
    T_COMMA,
    T_ASSIGNMENT,
    T_COLON,
    T_PLUS,
    T_MINUS,
    T_MULTIPLY,
    T_DIVIDE
} token_type;

typedef struct
{
    token_type type;
    union
    {
        char stringVal[MAX_STRING_LENGTH];
        int intVal;
        float floatVal;
        bool boolVal;
        char charVal;
    } value;
} Token;

Token* init_token(token_type type);
#endif