#ifndef SEMANTIC_H
#define SEMANTIC_H
#define _CUR_PROC "_CUR_PROC"

#include <stdbool.h> 

#include "scope.h"

typedef struct Semantic {
    Scope* global;
    Scope* current_local;
} Semantic;

Semantic* init_semantic_analyzer();
void free_semantic_analyzer(Semantic* sem);

void create_new_scope(Semantic* sem);
void exit_current_scope(Semantic* sem);
void set_symbol_semantic(Semantic* sem, char* s, Symbol sym, bool is_global);
Symbol get_current_symbol(Semantic* sem, char* s);
Symbol get_current_global_symbol(Semantic* sem, char* s, bool is_global);
bool has_current_symbol(Semantic* sem, char* s);
bool has_current_global_symbol(Semantic* sem, char* s, bool is_global);

void set_current_procedure(Semantic* sem, Symbol proc);
void update_symbol_semantic_global(Semantic* sem, Symbol sym, bool is_global);
Symbol get_current_procedure(Semantic* sem);

bool is_current_scope_global(Semantic* sem);
TokenType check_for_reserved_word(Semantic* sem, char* str);

void print_scope(Semantic* sem, bool is_global);

void insert_runtime_functions(Semantic* sem);

#endif