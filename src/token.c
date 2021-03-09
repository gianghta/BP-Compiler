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
    {"enum", K_ENUM}
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

char* print_token(Token* token)
{
    switch(token->type) {
        case T_UNKNOWN: return "";
        case T_EOF:
            printf("T_END_OF_FILE\n"); return "";
        case T_ASSIGNMENT:
            printf("T_ASSIGNMENT\n"); return "";
        case T_NUMBER_FLOAT:
            printf("T_NUMBER_FLOAT, %f\n", token->value.floatVal); return "FLOAT";
        case T_NUMBER_INT:
            printf("T_NUMBER_INT, %d\n", token->value.intVal); return "INTEGER";
        case T_PLUS:
            printf("T_PLUS\n"); return "+";
        case T_MULTIPLY:
            printf("T_MULTIPLY\n"); return "*";
        case T_DIVIDE:
            printf("T_DIVIDE\n"); return "/";
        case T_MINUS:
            printf("T_MINUS\n"); return "-";
        case T_CHAR:
            printf("T_CHAR, '%c'\n", token->value.charVal); return "CHAR";
        case T_STRING:
            printf("T_STRING, \"%s\"\n", token->value.stringVal); return "STRING";
        case T_ID:
            printf("T_IDENTIFIER, %s\n", token->value.stringVal); return "IDENTIFIER";
        case T_COLON:
            printf("T_COLON\n"); return ":";
        case T_SEMI_COLON:
            printf("T_SEMI_COLON\n"); return ";";
        case T_COMMA:
            printf("T_COMMA\n"); return ",";
        case T_LPAREN:
            printf("T_LPAREN\n"); return "(";
        case T_RPAREN:
            printf("T_RPAREN\n"); return ")";
        case T_LBRACKET:
            printf("T_LBRACKET\n"); return "{";
        case T_RBRACKET:
            printf("T_RBRACKET\n"); return "}";
        case T_EQ:
            printf("T_EQ\n"); return "=";
        case T_NOT_EQ:
            printf("T_NOT_EQ\n"); return "!=";
        case T_LT:
            printf("T_LT\n"); return "<";
        case T_LTEQ:
            printf("T_LTEQ\n"); return "<=";
        case T_GT:
            printf("T_GT\n"); return ">";
        case T_GTEQ:
            printf("T_GTEQ\n"); return ">=";
        case K_PROGRAM:
            printf("K_PROGRAM\n"); return "keyword PROGRAM";
        case K_IS:
            printf("K_IS\n"); return "keyword IS";
        case K_GLOBAL:
            printf("K_GLOBAL\n"); return "keyword GLOBAL";
        case K_INT:
            printf("K_INT\n"); return "keyword INTEGER";
        case K_FLOAT:
            printf("K_FLOAT\n"); return "keyword FLOAT";
        case K_STRING:
            printf("K_STRING\n"); return "keyword STRING";
        case K_BOOL:
            printf("K_BOOL\n"); return "keyword BOOL";
        case K_CHAR:
            printf("K_CHAR\n"); return "keyword CHAR";
        case K_PROCEDURE:
            printf("K_PROCEDURE\n"); return "keyword PROCEDURE";
        case K_RETURN:
            printf("K_RETURN\n"); return "keyword RETURN";
        case K_NOT:
            printf("K_NOT\n"); return "keyword NOT";
        case K_BEGIN:
            printf("K_BEGIN\n"); return "keyword BEGIN";
        case K_END:
            printf("K_END\n"); return "keyword END";
        case K_IF:
            printf("K_IF\n"); return "keyword IF";
        case K_ELSE:
            printf("K_ELSE\n"); return "keyword ELSE";
        case K_THEN:
            printf("K_THEN\n"); return "keyword THEN";
        case K_TRUE:
            printf("K_THEN\n"); return "keyword TRUE";
        case K_FALSE:
            printf("K_FALSE\n"); return "keyword FALSE";
        case K_WHILE:
            printf("K_WHILE\n"); return "keyword WHILE";
        case K_FOR:
            printf("K_FOR\n"); return "keyword FOR";
        case K_ENUM:
            printf("K_ENUM\n"); return "keyword ENUM";
        default:
            return "";
    }
}