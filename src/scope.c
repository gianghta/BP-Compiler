#include "include/scope.h"


Scope* init_scope()
{
    Scope* new_scope = calloc(1, sizeof(struct Scope));
    new_scope->table = NULL;
    new_scope->prev_scope = NULL;
    return new_scope;
}

void free_scope(Scope* scope)
{
    if (scope != NULL)
    {
        free(scope);
        scope = NULL;
    }
}

void set_symbol(Scope* scope, char* s, Symbol sym)
{
    SymbolTable* new_symbol;

    new_symbol = calloc(1, sizeof(SymbolTable));
    new_symbol->id = s;
    new_symbol->entry = sym;
    HASH_ADD_STR(scope->table, id, new_symbol);
}

Symbol get_symbol(Scope* scope, char* id)
{
    if (has_symbol(scope, id))
    {
        SymbolTable* scope_table;
        HASH_FIND_STR(scope->table, id, scope_table);
        return scope_table->entry;
    }
    else
    {
        Symbol* ptr = init_symbol();
        return *ptr;
    }
}

bool has_symbol(Scope* scope, char* id)
{
    SymbolTable* scope_table;
    HASH_FIND_STR(scope->table, id, scope_table);


    Symbol sym = scope_table->entry;
    if (sym.is_not_empty != false)
    {
        return true;
    }
    return false;
}

void print_symbol_table(Scope* scope)
{

}

void free_symbol_table(Scope* scope)
{
    HASH_CLEAR(hh, scope->table);
}