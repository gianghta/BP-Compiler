#include "include/semantic.h"


extern LLVMContextRef llvm_context;
extern LLVMBuilderRef llvm_builder;
extern LLVMModuleRef llvm_module;
extern LLVMValueRef main_func;

Semantic* init_semantic_analyzer()
{
    Semantic* sem = calloc(1, sizeof(struct Semantic));
    sem->global = init_scope();
    sem->current_local = sem->global;

    Symbol tmp;

    // Initialize reserved word
    set_symbol(sem->global, "program", *init_symbol_with_id_symbol_type("program", K_PROGRAM, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "is", *init_symbol_with_id_symbol_type("is", K_IS, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "begin", *init_symbol_with_id_symbol_type("begin", K_BEGIN, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "end", *init_symbol_with_id_symbol_type("end", K_END, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "global", *init_symbol_with_id_symbol_type("global", K_GLOBAL, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "variable", *init_symbol_with_id_symbol_type("variable", K_VARIABLE, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "procedure", *init_symbol_with_id_symbol_type("procedure", K_PROCEDURE, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "integer", *init_symbol_with_id_symbol_type("integer", K_INT, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "float", *init_symbol_with_id_symbol_type("float", K_FLOAT, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "bool", *init_symbol_with_id_symbol_type("bool", K_BOOL, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "string", *init_symbol_with_id_symbol_type("string", K_STRING, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "if", *init_symbol_with_id_symbol_type("if", K_IF, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "then", *init_symbol_with_id_symbol_type("then", K_THEN, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "else", *init_symbol_with_id_symbol_type("else", K_ELSE, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "for", *init_symbol_with_id_symbol_type("for", K_FOR, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "return", *init_symbol_with_id_symbol_type("return", K_RETURN, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "not", *init_symbol_with_id_symbol_type("not", K_NOT, ST_KEYWORD, TC_UNKNOWN));
    set_symbol(sem->global, "true", *init_symbol_with_id_symbol_type("true", K_TRUE, ST_KEYWORD, TC_BOOL));
    set_symbol(sem->global, "false", *init_symbol_with_id_symbol_type("false", K_FALSE, ST_KEYWORD, TC_BOOL));

    // Built-in functions
    set_symbol(sem->global, "getbool", *init_symbol_with_id_symbol_type("getbool", T_ID, ST_PROCEDURE, TC_BOOL));
    set_symbol(sem->global, "getinteger", *init_symbol_with_id_symbol_type("getinteger", T_ID, ST_PROCEDURE, TC_INT));
    set_symbol(sem->global, "getfloat", *init_symbol_with_id_symbol_type("getfloat", T_ID, ST_PROCEDURE, TC_FLOAT));
    set_symbol(sem->global, "getstring", *init_symbol_with_id_symbol_type("getstring", T_ID, ST_PROCEDURE, TC_STRING));
    set_symbol(sem->global, "_outOfBoundsError", *init_symbol_with_id_symbol_type("_outOfBoundsError", T_ID, ST_PROCEDURE, TC_UNKNOWN));

    tmp = *init_symbol_with_id_symbol_type("putbool", T_ID, ST_PROCEDURE, TC_BOOL);
    tmp.params->symbol = *init_symbol_with_id_symbol_type("value", T_ID, ST_VARIABLE, TC_BOOL);
    tmp.params->next_symbol = NULL;
    set_symbol(sem->global, "putbool", tmp);

    tmp = *init_symbol_with_id_symbol_type("putinteger", T_ID, ST_PROCEDURE, TC_BOOL);
    tmp.params->symbol = *init_symbol_with_id_symbol_type("value", T_ID, ST_VARIABLE, TC_INT);
    tmp.params->next_symbol = NULL;
    set_symbol(sem->global, "putinteger", tmp);

    tmp = *init_symbol_with_id_symbol_type("putfloat", T_ID, ST_PROCEDURE, TC_BOOL);
    tmp.params->symbol = *init_symbol_with_id_symbol_type("value", T_ID, ST_VARIABLE, TC_FLOAT);
    tmp.params->next_symbol = NULL;
    set_symbol(sem->global, "putfloat", tmp);

    tmp = *init_symbol_with_id_symbol_type("putstring", T_ID, ST_PROCEDURE, TC_BOOL);
    tmp.params->symbol = *init_symbol_with_id_symbol_type("value", T_ID, ST_VARIABLE, TC_STRING);
    tmp.params->next_symbol = NULL;
    set_symbol(sem->global, "putstring", tmp);

    tmp = *init_symbol_with_id_symbol_type("sqrt", T_ID, ST_PROCEDURE, TC_BOOL);
    tmp.params->symbol = *init_symbol_with_id_symbol_type("value", T_ID, ST_VARIABLE, TC_INT);
    tmp.params->next_symbol = NULL;
    set_symbol(sem->global, "sqrt", tmp);

    return sem;
}

void free_semantic_analyzer(Semantic* sem)
{
    if (sem != NULL)
    {
        if (sem->global != sem->current_local)
        {
            free_scope(sem->global);
            sem->global = NULL;
        }
        free_scope(sem->current_local);
        sem->current_local = NULL;

        free(sem);
        sem = NULL;
    }
}

/*
 * Enter and create a new local scope
 */
void create_new_scope(Semantic* sem)
{
    Scope* new_scope = init_scope();
    new_scope->prev_scope = sem->current_local;
    sem->current_local = new_scope;
}

/*
 * Exit current local scope
 * Do nothing to outer/global scope
 */
void exit_current_scope(Semantic* sem)
{
    if (sem->current_local != NULL && sem->current_local != sem->global)
    {
        Scope* tmp = sem->current_local;
        sem->current_local = sem->current_local->prev_scope;
        free_scope(tmp);
    }
}

void set_symbol_semantic(Semantic* sem, char* s, Symbol sym, bool is_global)
{
    if (is_global)
    {
        set_symbol(sem->global, s, sym);
    }
    else
    {
        set_symbol(sem->current_local, s, sym);
    }
}

// Look for exisiting symbol first in local and global scope
// If doesn't exist, set it
// If does, update the current one from its scope
void update_symbol_semantic_global(Semantic* sem, Symbol sym, bool is_global)
{
    if (sem != NULL)
    {
        if (!has_current_global_symbol(sem, sym.id, sym.is_global))
        {
            set_symbol_semantic(sem, sym.id, sym, sym.is_global);
        }
        else
        {
            if (is_global)
            {
                update_symbol(sem->global, sym.id, sym);
            }
            else
            {
                update_symbol(sem->current_local, sym.id, sym);
            }
        }
    }
}

/*
 * Travel upward and get symbol from either local scope or global
 */
Symbol get_current_symbol(Semantic* sem, char* s)
{
    if (has_symbol(sem->current_local, s))
    {
        return get_symbol(sem->current_local, s);
    }
    else
    {
        return get_symbol(sem->global, s);
    }
}

/*
 * Find symbol or return an unknown type symbol
 */
Symbol get_current_global_symbol(Semantic* sem, char* s, bool is_global)
{
    if (is_global)
    {
        return get_symbol(sem->global, s);
    }
    else
    {
        return get_current_symbol(sem, s);
    }
}

/*
 * Check local and global for symbol
 */
bool has_current_symbol(Semantic* sem, char* s)
{
    return has_symbol(sem->current_local, s) || has_symbol(sem->global, s);
}

/*
 * Check global first for symbol
 */
bool has_current_global_symbol(Semantic* sem, char* s, bool is_global)
{
    if (is_global)
    {
        return has_symbol(sem->global, s);
    }
    else
    {
        return has_symbol(sem->current_local, s);
    }
}

void set_current_procedure(Semantic* sem, Symbol proc)
{
    if (sem != NULL)
    {
        if (has_current_symbol(sem, _CUR_PROC))
        {
            update_symbol(sem->current_local, _CUR_PROC, proc);
        }
        else
        {
            set_symbol(sem->current_local, _CUR_PROC, proc);
        }
    }
}

Symbol get_current_procedure(Semantic* sem)
{
    return get_symbol(sem->current_local, _CUR_PROC);
}

void print_scope(Semantic* sem, bool is_global)
{
    if (is_global)
    {
        print_symbol_table(sem->global);
    }
    else
    {
        print_symbol_table(sem->current_local);
    }
}

bool is_current_scope_global(Semantic* sem)
{
    return sem->global == sem->current_local;
}

TokenType check_for_reserved_word(Semantic* sem, char* str)
{
    if (has_current_global_symbol(sem, str, true))
    {
        Symbol tmp = get_current_global_symbol(sem, str, true);
        return tmp.ttype;
    }
    return T_ID;
}

void insert_runtime_functions(Semantic* sem)
{
    Symbol s;
    char* str;
    LLVMValueRef func;

    str = "getbool";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "getbool");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "getinteger";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "getinteger");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "getfloat";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "getfloat");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);
    
    str = "getstring";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "getstring");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putbool";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "putbool");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putinteger";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "putinteger");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putfloat";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "putfloat");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putstring";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "putstring");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "sqrt";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "_sqrt");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "_outOfBoundsError";
    s = get_symbol(sem->global, str);
    func = LLVMGetNamedFunction(llvm_module, "outOfBoundsError");
    s.llvm_function = func;
    update_symbol(sem->global, str, s);
}