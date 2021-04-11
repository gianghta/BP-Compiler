#include "include/semantic.h"


Semantic* init_semantic_analyzer()
{
    Semantic* sem = calloc(1, sizeof(struct Semantic));
    sem->global = init_scope();
    sem->current_local = sem->global;
}