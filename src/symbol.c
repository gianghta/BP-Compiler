#include "include/symbol.h"


Symbol* init_symbol()
{
    Symbol* sym = calloc(1, sizeof(struct Symbol));
    sym->id = "";
    sym->ttype = T_UNKNOWN;
    sym->stype = ST_UNKOWN;
    sym->type = TC_UNKNOWN;
    sym->is_global = false;
    sym->is_arr = false;
    sym->arr_size = 0;
    sym->is_indexed = false;
    sym->is_not_empty = true;
    sym->params = calloc(1, sizeof(struct SymbolNode));
    return sym;
}

Symbol* init_symbol_with_id(char* id_name, TokenType token_type)
{
    Symbol* sym = calloc(1, sizeof(struct Symbol));
    sym->id = id_name;
    sym->ttype = token_type;
    sym->stype = ST_UNKOWN;
    sym->type = TC_UNKNOWN;
    sym->is_global = false;
    sym->is_arr = false;
    sym->arr_size = 0;
    sym->is_indexed = false;
    sym->is_not_empty = true;
    sym->params = calloc(1, sizeof(struct SymbolNode));
    return sym;
}
Symbol* init_symbol_with_id_symbol_type(char* id_name, TokenType token_type, SymbolType sym_type, TypeClass type_c)
{
    Symbol* sym = calloc(1, sizeof(struct Symbol));
    sym->id = id_name;
    sym->ttype = token_type;
    sym->stype = sym_type;
    sym->type = type_c;
    sym->is_global = false;
    sym->is_arr = false;
    sym->arr_size = 0;
    sym->is_indexed = false;
    sym->is_not_empty = true;
    sym->params = calloc(1, sizeof(struct SymbolNode));
    return sym;
}

void free_symbol(Symbol* sym)
{
    if (sym != NULL)
    {
        if (sym->params != NULL)
        {
            free_params_list(sym->params);
        }
        free(sym);
        sym = NULL;
    }
}

void free_params_list(SymbolNode* head)
{
    SymbolNode* tmp = NULL;
    
    while (head != NULL)
    {
        tmp = head;
        head = head->next_symbol;
        free(tmp);
    }
}

char* print_symbol_type(SymbolType type)
{
    switch(type)
    {
        case ST_KEYWORD:
            return concatf("ST_KEYWORD");
        case ST_VARIABLE:
            return concatf("ST_VARIABLE");
        case ST_PROCEDURE:
            return concatf("ST_PROCEDURE");
        default:
            return concatf("ST_UNKNOWN");
    }
}