#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h> 

#include "scope.h"

typedef struct Semantic {
    Scope* global;
    Scope* current_local;
} Semantic;

Semantic* init_semantic_analyzer();
void create_new_scope(Semantic* sem);
void exit_current_scope(Semantic* sem);
void set_symbol(Semantic* sem, char* s, Symbol sym, bool is_global);
Symbol get_current_symbol(Semantic* sem, char* s);
Symbol get_current_global_symbol(Semantic* sem, char* s, bool is_global);
void has_current_symbol(Semantic* sem, char* s);
void has_current_global_symbol(Semantic* sem, char* s, bool is_global);

void set_current_procedure(Semantic* sem, Symbol proc);
Symbol get_current_procedure(Semantic* sem);

#endif