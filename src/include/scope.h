#ifndef SCOPE_H
#define SCOPE_H

#define MAX_KEYWORD_LENGTH 50

#include "symbol.h"
#include "uthash.h"


typedef struct SymbolTable {
    char id[MAX_KEYWORD_LENGTH];
    Symbol entry;
    UT_hash_handle hh;
} SymbolTable;

typedef struct Scope {
    SymbolTable* table;
    struct Scope* prev_scope;
} Scope;

Scope* init_scope();
void free_scope(Scope* scope);
void set_symbol(Scope* scope, char* s, Symbol sym);
Symbol get_symbol(Scope* scope, char* id);
bool has_symbol(Scope* scope, char* id);
void print_symbol_table(Scope* scope);
void free_symbol_table(Scope* scope);

#endif