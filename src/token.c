#include "include/token.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/*
 * Table for mapping reserved word
 * into Token Type
 */
struct {
    char keyword[MAX_STRING_LENGTH];
    TokenType type;
} reserved_words[MAX_KEYWORD_LENGTH] = {
    {"program", K_PROGRAM},
    {"is", K_IS},
    {"global", K_GLOBAL},
    {"variable", K_VARIABLE},
    {"integer", K_INT},
    {"float", K_FLOAT},
    {"string", K_STRING},
    {"bool", K_BOOL},
    {"char", K_CHAR},
    {"procedure", K_PROCEDURE},
    {"return", K_RETURN},
    {"not", K_NOT},
    {"begin", K_BEGIN},
    {"end", K_END},
    {"if", K_IF},
    {"else", K_ELSE},
    {"then", K_THEN},
    {"true", K_TRUE},
    {"false", K_FALSE},
    {"while", K_WHILE},
    {"for", K_FOR},
};

/*
 * Token constructor
 */
Token* init_token(TokenType type)
{
    Token* token = calloc(1, sizeof(Token));
    token->type = type;

    return token;
}

/*
 * Convert all input into lowercase
 */
void to_lower_case_str(char *p) {
    for ( ; *p; ++p) *p = tolower(*p);
}

/*
 * Check for reserved word from table
 */
TokenType check_for_reserved_word(char* str)
{
    int len = sizeof(reserved_words)/sizeof(reserved_words[0]);

    for (int i = 0; i < len; i++)
    {
        if (strcmp(reserved_words[i].keyword, str) == 0)
        {
            return reserved_words[i].type;
        }
    }
    return T_ID;
}

/*
 * String representation of token
 */
char* print_token(Token* token)
{
    switch(token->type) {
        case T_UNKNOWN: return "";
        case T_EOF:
            return concatf("T_END_OF_FILE\n");
        case T_ASSIGNMENT:
            return concatf("T_ASSIGNMENT\n");
        case T_NUMBER_FLOAT:
            return concatf("T_NUMBER_FLOAT: %f\n", token->value.floatVal);
        case T_NUMBER_INT:
            return concatf("T_NUMBER_INT: %d\n", token->value.intVal);
        case T_PLUS:
            return concatf("T_PLUS: %s\n", "+");
        case T_MULTIPLY:
            return concatf("T_MULTIPLY: %s\n", "*");
        case T_DIVIDE:
            return concatf("T_DIVIDE: %s\n", "/");
        case T_MINUS:
            return concatf("T_MINUS: %s\n", "-");
        case T_CHAR:
            return concatf("T_CHAR: '%c'\n", token->value.charVal);
        case T_STRING:
            return concatf("T_STRING: \"%s\"\n", token->value.stringVal);
        case T_ID:
            return concatf("T_IDENTIFIER: %s\n", token->value.stringVal);
        case T_COLON:
            return concatf("T_COLON: %s\n", ":");
        case T_SEMI_COLON:
            return concatf("T_SEMI_COLON: %s\n", ";");
        case T_COMMA:
            return concatf("T_COMMA: %s\n", ",");
        case T_LPAREN:
            return concatf("T_LPAREN: %s\n", "(");
        case T_RPAREN:
            return concatf("T_RPAREN: %s\n", ")");
        case T_LBRACKET:
            return concatf("T_LBRACKET: %s\n", "[");
        case T_RBRACKET:
            return concatf("T_RBRACKET: %s\n", "]");
        case T_EQ:
            return concatf("T_EQ: %s\n", "=");
        case T_NOT_EQ:
            return concatf("T_NOT_EQ: %s\n", "!=");
        case T_LT:
            return concatf("T_LT: %s\n", "<");
        case T_LTEQ:
            return concatf("T_LTEQ: %s\n", "<=");
        case T_GT:
            return concatf("T_GT: %s\n", ">");
        case T_GTEQ:
            return concatf("T_GTEQ: %s\n", ">=");
        case K_PROGRAM:
            return concatf("K_PROGRAM: %s\n", "program");
        case K_IS:
            return concatf("K_IS: %s\n", "is");
        case K_GLOBAL:
            return concatf("K_GLOBAL: %s\n", "global");
        case K_INT:
            return concatf("K_INT: %s\n", "integer");
        case K_FLOAT:
            return concatf("K_FLOAT: %s\n", "float");
        case K_STRING:
            return concatf("K_STRING: %s\n", "string");
        case K_BOOL:
            return concatf("K_BOOL: %s\n", "bool");
        case K_CHAR:
            return concatf("K_CHAR: %s\n", "char");
        case K_PROCEDURE:
            return concatf("K_PROCEDURE: %s\n", "procedure");
        case K_RETURN:
            return concatf("K_RETURN: %s\n", "return");
        case K_NOT:
            return concatf("K_NOT: %s\n", "not");
        case K_BEGIN:
            return concatf("K_BEGIN: %s\n", "begin");
        case K_END:
            return concatf("K_END: %s\n", "end");
        case K_IF:
            return concatf("K_IF: %s\n", "if");
        case K_ELSE:
            return concatf("K_ELSE: %s\n", "else");
        case K_THEN:
            return concatf("K_THEN: %s\n", "then");
        case K_TRUE:
            return concatf("K_TRUE: %s\n", "true");
        case K_FALSE:
            return concatf("K_FALSE: %s\n", "false");
        case K_WHILE:
            return concatf("K_WHILE: %s\n", "while");
        case K_FOR:
            return concatf("K_FOR: %s\n", "for");
        case K_VARIABLE:
            return concatf("K_VARIABLE: %s\n", "variable");
        default:
            return "";
    }
}