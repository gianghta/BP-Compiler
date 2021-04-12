#include "include/lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


lexer_T* init_lexer(char* source, Semantic* sem)
{
    lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
    lexer->source = source;
    lexer->sem = sem;
    lexer->start = 0;
    lexer->current_char = source[lexer->start];

    return lexer;
}

// Lexer main function to detect each type of tokens
Token* lexer_get_next_token(lexer_T* lexer)
{
    Token* token;

    lexer_skip_whitespace(lexer);
    
    switch (lexer->current_char)
    {
        case '/':
            lexer_advance(lexer);

            switch(lexer->current_char) {
                case '/':
                    lexer_advance(lexer);
                    lexer_skip_line_comment(lexer);
                    return lexer_get_next_token(lexer);
                case '*':
                    lexer_advance(lexer);
                    lexer_skip_block_comment(lexer);
                    return  lexer_get_next_token(lexer);
                default:
                    return init_token(T_DIVIDE);
            }
        
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
        case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
        case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z':
            return lexer_collect_id(lexer);
        
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '9':
            return lexer_collect_integer(lexer);
        case '"':
            return lexer_collect_string(lexer);
        case ':':
            lexer_advance(lexer);
            if (lexer->current_char != EOF && lexer->current_char == '=')
            {
                token = init_token(T_ASSIGNMENT);
                lexer_advance(lexer);
            }
            else
                token = init_token(T_COLON);
            return token;
        case '+':
            token = init_token(T_PLUS);
            lexer_advance(lexer);
            return token;
        case '-':
            token = init_token(T_MINUS);
            lexer_advance(lexer);
            return token;
        case '*':
            token = init_token(T_MULTIPLY);
            lexer_advance(lexer);
            return token;
        case '=':
            lexer_advance(lexer);
            if (lexer->current_char != EOF && lexer->current_char == '=')
            {
                token = init_token(T_EQ);
                lexer_advance(lexer);
                return token;
            }
            else
            {
                token = init_token(T_UNKNOWN);
                printf("Unknown token!\n");
                return token;
            }
        case '!':
            lexer_advance(lexer);
            if (lexer->current_char != EOF && lexer->current_char == '=')
            {
                token = init_token(T_NOT_EQ);
                lexer_advance(lexer);
                return token;
            }
            else
            {
                token = init_token(T_UNKNOWN);
                return token;
            }
        case '<':
            lexer_advance(lexer);
            if (lexer->current_char != EOF && lexer->current_char == '=')
            {
                token = init_token(T_LTEQ);
                lexer_advance(lexer);
            }
            else
                token = init_token(T_LT);
            return token;
        case '>':
            lexer_advance(lexer);
            if (lexer->current_char != EOF && lexer->current_char == '=')
            {
                token = init_token(T_GTEQ);
                lexer_advance(lexer);
            }
            else
                token = init_token(T_GT);
            return token;
        case ';':
            token = init_token(T_SEMI_COLON);
            lexer_advance(lexer);
            return token;
        case '(':
            token = init_token(T_LPAREN);
            lexer_advance(lexer);
            return token;
        case ')':
            token = init_token(T_RPAREN);
            lexer_advance(lexer);
            return token;
        case '[':
            token = init_token(T_LBRACKET);
            lexer_advance(lexer);
            return token;
        case ']':
            token = init_token(T_RBRACKET);
            lexer_advance(lexer);
            return token;
        case ',':
            token = init_token(T_COMMA);
            lexer_advance(lexer);
            return token;
        case EOF:
        case '.':
            return init_token(T_EOF);
        default:
            token = init_token(T_UNKNOWN);
            printf("Invalid input!\n");
            lexer_advance(lexer);
            return token;
    }
}

Token* lexer_collect_string(lexer_T* lexer)
{   
    // Eat the double quote character
    lexer_advance(lexer);

    Token* token = init_token(T_STRING);
    int cnt = 0;

    while (lexer->current_char != EOF && (
        isalnum(lexer->current_char) ||
        isspace(lexer->current_char) ||
        lexer->current_char == '_' ||
        lexer->current_char == ';' ||
        lexer->current_char == ':' ||
        lexer->current_char == '.' ||
        lexer->current_char == '\''
    ))
    {
        if (cnt <= MAX_STRING_LENGTH)
            token->value.stringVal[cnt++] = lexer->current_char;
            lexer_advance(lexer);
    }

    if (cnt > MAX_STRING_LENGTH)
    {
        printf("String too long\n");
        return token;
    }

    token->value.stringVal[cnt] = '\0';

    if (lexer->current_char == '\"')
    {
        lexer_advance(lexer);
    }
    else
    {
        token->type = T_UNKNOWN;
    }

    return token;
}

Token* lexer_collect_integer(lexer_T* lexer)
{
    Token* token = init_token(T_NUMBER_INT);
    token->value.intVal = 0;
    token->value.intVal = lexer->current_char - '0';

    lexer_advance(lexer);
    while (isdigit(lexer->current_char))
    {
        token->value.intVal = token->value.intVal * 10 + lexer->current_char - '0';
        lexer_advance(lexer);
    }
    
    if (lexer->current_char == '.')
        return lexer_collect_float(&lexer, &token);
    
    return token;
}

Token* lexer_collect_float(lexer_T** lexer, Token** token)
{
    Token* new_token = *token;
    new_token->type = T_NUMBER_FLOAT;
    
    int exponent = 1.0;
    new_token->value.floatVal = new_token->value.intVal;

    lexer_advance(*lexer);
    while (isdigit((*lexer)->current_char)) {
        exponent = exponent * 10;
        new_token->value.floatVal = new_token->value.floatVal * 10 + (*lexer)->current_char - '0';
        lexer_advance(*lexer);
    }
    new_token->value.floatVal = new_token->value.floatVal / exponent;
    return new_token;
}

Token* lexer_collect_id(lexer_T* lexer)
{   
    int i = 0;
    Token* token = init_token(T_ID);

    for (i = 0; isalnum(lexer->current_char) || lexer->current_char == '_'; i++)
    {
        if (i == MAX_STRING_LENGTH)
        {
            printf("Identifier is too long!\n");
            return token;
        }
        token->value.stringVal[i] = lexer->current_char;
        lexer_advance(lexer);
    }

    to_lower_case_str(token->value.stringVal);
    token->type = check_for_reserved_word(lexer->sem, (char*)token->value.stringVal);
    return token;
}

void lexer_advance(lexer_T* lexer)
{
    if (lexer->current_char != '\0' && lexer->start < strlen(lexer->source))
    {
        lexer->start += 1;
        lexer->current_char = lexer->source[lexer->start];
    }
}

void lexer_skip_whitespace(lexer_T* lexer)
{
    while (isspace(lexer->current_char) && lexer->current_char != -1)
    {
        lexer_advance(lexer);
    }
}

void lexer_skip_line_comment(lexer_T* lexer)
{
    while (lexer->current_char != '\n' && lexer->current_char != EOF)
        lexer_advance(lexer);
}

void lexer_skip_block_comment(lexer_T* lexer)
{
    int cnt = 1;
    while (cnt != 0 && lexer->current_char != EOF)
    {
        switch (lexer->current_char)
        {
            case '*':
                lexer_advance(lexer);
                if (lexer->current_char != EOF && lexer->current_char == '/')
                {
                    cnt--;
                    lexer_advance(lexer);
                }
                break;
            case '/':
                lexer_advance(lexer);
                if (lexer->current_char != EOF && lexer->current_char == '*')
                {
                    cnt++;
                    lexer_advance(lexer);
                }
                break;
            default:
                lexer_advance(lexer);
                break;
        }
    }
    if (cnt > 0)
        printf("Comment error, did not end properly!\n");
}