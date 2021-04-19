#include "include/semantic.h"


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

void update_symbol_semantic_global(Semantic* sem, Symbol sym, bool is_global)
{
    if (sem != NULL)
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
        return get_symbol(sem->current_local, s);
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

// void create_runtime_functions(Semantic* sem, char* name)
// {
//     Symbol s = get_symbol(sem->global, name);
//     LLVMTypeRef proc_type;
//     LLVMValueRef proc;
//     LLVMTypeRef params[1];

//     // Condition for get functions
//     if (name[0] == 'g')
//     {
//         if (s.type != TC_STRING) // string case is handled separately
//         {
//             params[0] = LLVMPointerType(create_type(s.type), 0);
//         }
//     }

//     proc_type = LLVMFunctionType(LLVMVoidType(), params, 1, false);
//     proc = LLVMAddFunction(llvm_module, s.id, proc_type);
//     LLVMBasicBlockRef proc_entry = LLVMAppendBasicBlock(proc, s.id);
//     LLVMPositionBuilderAtEnd(llvm_builder, proc_entry);

//     LLVMValueRef value = LLVMGetParam(proc, 0);
//     LLVMSetValueName(value, "value");
//     s.params->symbol.llvm_value = value;
//     s.params->next_symbol = NULL;

//     const char* format_str = "";

//     if (strcmp(name, "putbool") == 0 || strcmp(name, "putinteger") == 0 ||  \
// 		strcmp(name, "getbool") == 0 || strcmp(name, "getinteger") == 0)
//     {
// 	    format_str = "%d";
//     } else if (strcmp(name, "putfloat") == 0 || strcmp(name, "getfloat") == 0) {
//         format_str = "%f";
//     } else if (strcmp(name, "putstring") == 0 || strcmp(name, "getstring") == 0) {
//         format_str = "%s";
//     } else if (strcmp(name, "putchar") == 0 || strcmp(name, "getchar") == 0) {
//         format_str = "%c";
//     }

//     LLVMValueRef format = LLVMBuildGlobalStringPtr(llvm_builder, format_str, "format_str");

//     if (name[0] == 'p') { // put* functions
//         LLVMValueRef args[] = { format, s.params->symbol.llvm_value };
//         LLVMBuildCall(llvm_builder, llvm_printf, args, 2, name);
//     } else if (name[0] == 'g') { // get* functions
//         LLVMValueRef args[] = { format, s.params->symbol.llvm_value };
//         LLVMBuildCall(llvm_builder, llvm_scanf, args, 2, name);
//     } else {
//         LLVMValueRef args[] = { s.params->symbol.llvm_value };
//         LLVMBuildCall(llvm_builder, llvm_sqrtf, args, 2, name);
//     }

//     s.llvm_function = proc;
//     set_symbol(sem->global, name, s);

//     LLVMBuildRetVoid(llvm_builder);
//     LLVMPositionBuilderAtEnd(llvm_builder, LLVMGetLastBasicBlock(main_func));
// }

LLVMTypeRef create_llvm_type(TypeClass entry_type) {
	LLVMTypeRef type;
	switch (entry_type) {
		case TC_INT:
            type = LLVMInt32Type(); break;
		case TC_FLOAT:
			type = LLVMFloatType(); break;
		case TC_STRING:
			type = LLVMPointerType(LLVMInt8Type(), 0); break;
		case TC_BOOL:
			type = LLVMInt32Type(); break;
		default:
			type = LLVMVoidType(); break;
	}
	return type;
}

void insert_runtime_functions(Semantic* sem)
{
    Symbol s;
    char* str;
    LLVMTypeRef ft;
    LLVMValueRef func;
    LLVMTypeRef params[1];

    str = "getbool";
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMInt1Type(), NULL, 0, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "getinteger";
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMInt32Type(), NULL, 0, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "getfloat";
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMFloatType(), NULL, 0, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "getstring";
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMPointerType(LLVMInt8Type(), 0), NULL, 0, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putbool";
    params[0] = LLVMInt1Type();
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMInt1Type(), params, 1, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putinteger";
    params[0] = LLVMInt32Type();
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMInt32Type(), params, 1, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putfloat";
    params[0] = LLVMFloatType();
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMFloatType(), params, 1, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "putstring";
    params[0] = LLVMPointerType(LLVMInt8Type(), 0);
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMPointerType(LLVMInt8Type(), 0), params, 1, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);

    str = "sqrt";
    params[0] = LLVMInt32Type();
    s = get_symbol(sem->global, str);
    ft = LLVMFunctionType(LLVMInt32Type(), params, 1, false);
    func = LLVMAddFunction(llvm_module, str, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    s.llvm_function = func;
    update_symbol(sem->global, str, s);
}