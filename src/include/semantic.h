#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "scope.h"

typedef struct SemanticAnalyzer {
    Scope* global;
    Scope* current_local;
} SemanticAnalyzer;


#endif SEMANTIC_H