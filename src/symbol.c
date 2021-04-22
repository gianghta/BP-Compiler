#include "include/symbol.h"

extern LLVMBuilderRef llvm_builder;
extern LLVMModuleRef llvm_module;
extern LLVMContextRef llvm_context;

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
    sym->llvm_value = NULL;
    sym->llvm_address = NULL;
    sym->llvm_function = NULL;
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
    sym->llvm_value = NULL;
    sym->llvm_address = NULL;
    sym->llvm_function = NULL;
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
    sym->llvm_value = NULL;
    sym->llvm_address = NULL;
    sym->llvm_function = NULL;
    return sym;
}

/*
 * Take head SymbolNode of params
 * and then traverse the params linked list
 * until reach index node
 */
Symbol get_nth_param(Symbol* sym, int idx)
{
    SymbolNode *ptr = sym->params;
    int count = 0;
    while (ptr !=NULL)
    {
        if (count == idx)
        {
            return ptr->symbol;
        }
        count++;
        ptr = ptr->next_symbol;
    }

    // User ask for a non-existent element, fail.
    assert(0);
}

int params_size(Symbol* sym)
{
    int count = 0;
    SymbolNode *ptr = sym->params;

    while (ptr != NULL && ptr->symbol.is_not_empty)
    {
        ptr = ptr->next_symbol;
        count += 1;
    }

    return count;
}

void free_symbol(Symbol* sym)
{
    if (sym != NULL)
    {
        if (sym->params != NULL)
        {
            free_params_list(sym->params);
        }
        free(sym->llvm_address);
        free(sym->llvm_function);
        free(sym->llvm_value);
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

char* print_type_class(TypeClass type)
{
    switch(type)
    {
        case TC_INT:
            return concatf("TC_INT");
        case TC_FLOAT:
            return concatf("TC_FLOAT");
        case TC_STRING:
            return concatf("TC_STRING");
        case TC_BOOL:
            return concatf("TC_BOOL");
        default:
            return concatf("TC_UNKNOWN");
    }
}

LLVMTypeRef create_llvm_type(TypeClass entry_type) {
	LLVMTypeRef type;
	switch (entry_type) {
		case TC_INT:
            type = LLVMInt32TypeInContext(llvm_context); break;
		case TC_FLOAT:
			type = LLVMFloatTypeInContext(llvm_context); break;
		case TC_STRING:
			type = LLVMPointerType(LLVMInt8TypeInContext(llvm_context), 0); break;
		case TC_BOOL:
			type = LLVMInt1TypeInContext(llvm_context); break;
        case TC_VOID:
            type = LLVMVoidTypeInContext(llvm_context); break;
		default:
			printf("Invalid type\n");
            return NULL;
	}
	return type;
}