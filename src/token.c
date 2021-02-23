#include "include/token.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

struct {
    char keyword[MAX_STRING_LENGTH];
    token_type type;
} reserved_words[MAX_KEYWORD_LENGTH] = {
    {"program", K_PROGRAM},
    {"is", K_IS},
    {"global", K_GLOBAL},
    {"integer", K_INT},
    {"float", K_FLOAT},
    {"string", K_STRING},
    {"bool", K_BOOL},
    {"char", K_CHAR},
    {"procedure", K_PROCEDURE},
    {"in", K_IN},
    {"out", K_OUT},
    {"inout", K_INOUT},
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
    {"for", K_FOR}
};

Token* init_token(token_type type)
{
    Token* token = calloc(1, sizeof(Token));
    token->type = type;

    return token;
}

void to_lower_case_str(char *p) {
    for ( ; *p; ++p) *p = tolower(*p);
}

token_type check_for_reserved_word(char* str)
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

void print_token(Token* token)
{
    switch(token->type) {
        case T_EOF:
            printf("T_END_OF_FILE\n"); break;
        case T_ASSIGNMENT:
            printf("T_ASSIGNMENT\n"); break;
        case T_NUMBER_FLOAT:
            printf("T_NUMBER_FLOAT, %f\n", token->value.floatVal); break;
        case T_NUMBER_INT:
            printf("T_NUMBER_INT, %d\n", token->value.intVal); break;
        case T_PLUS:
            printf("T_PLUS\n"); break;
        case T_MULTIPLY:
            printf("T_MULTIPLY\n"); break;
        case T_DIVIDE:
            printf("T_DIVIDE\n"); break;
        case T_MINUS:
            printf("T_MINUS\n"); break;
        case T_CHAR:
            printf("T_CHAR, '%c'\n", token->value.charVal); break;
        case T_STRING:
            printf("T_STRING, \"%s\"\n", token->value.stringVal); break;
        case T_ID:
            printf("T_IDENTIFIER, %s\n", token->value.stringVal); break;
        case T_COLON:
            printf("T_COLON\n"); break;
        case T_SEMI_COLON:
            printf("T_SEMI_COLON\n"); break;
        case T_COMMA:
            printf("T_COMMA\n"); break;
        case T_LPAREN:
            printf("T_LPAREN\n"); break;
        case T_RPAREN:
            printf("T_RPAREN\n"); break;
        case T_LBRACKET:
            printf("T_LBRACKET\n"); break;
        case T_RBRACKET:
            printf("T_RBRACKET\n"); break;
        case T_EQ:
            printf("T_EQ\n"); break;
        case T_NOT_EQ:
            printf("T_NOT_EQ\n"); break;
        case T_LT:
            printf("T_LT\n"); break;
        case T_LTEQ:
            printf("T_LTEQ\n"); break;
        case T_GT:
            printf("T_GT\n"); break;
        case T_GTEQ:
            printf("T_GTEQ\n"); break;
        case K_PROGRAM:
            printf("K_PROGRAM\n"); break;
        case K_IS:
            printf("K_IS\n"); break;
        case K_GLOBAL:
            printf("K_GLOBAL\n"); break;
        case K_INT:
            printf("K_INT\n"); break;
        case K_FLOAT:
            printf("K_FLOAT\n"); break;
        case K_STRING:
            printf("K_STRING\n"); break;
        case K_BOOL:
            printf("K_BOOL\n"); break;
        case K_CHAR:
            printf("K_CHAR\n"); break;
        case K_PROCEDURE:
            printf("K_PROCEDURE\n"); break;
        case K_IN:
            printf("K_IN\n"); break;
        case K_OUT:
            printf("K_OUT\n"); break;
        case K_INOUT:
            printf("K_INOUT\n"); break;
        case K_RETURN:
            printf("K_RETURN\n"); break;
        case K_NOT:
            printf("K_NOT\n"); break;
        case K_BEGIN:
            printf("K_BEGIN\n"); break;
        case K_END:
            printf("K_END\n"); break;
        case K_IF:
            printf("K_IF\n"); break;
        case K_ELSE:
            printf("K_ELSE\n"); break;
        case K_THEN:
            printf("K_THEN\n"); break;
        case K_TRUE:
            printf("K_THEN\n"); break;
        case K_FALSE:
            printf("K_FALSE\n"); break;
        case K_WHILE:
            printf("K_WHILE\n"); break;
        case K_FOR:
            printf("K_FOR\n"); break;
        default:
            printf("T_UNKNOWN\n"); break;
    }
}