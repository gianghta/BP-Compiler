#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include <assert.h>
#include <llvm-c/Core.h>

#include "token.h"

struct SymbolNode;

/*
 * Variable type or return type of procedure
 */
typedef enum TypeClass {
    TC_INT,
    TC_FLOAT,
    TC_STRING,
    TC_BOOL,
    TC_VOID,
    TC_UNKNOWN
} TypeClass;

/*
 * Symbols are either reserved words or identifiers.
 * Identifiers are either variables or procedures
 */
typedef enum SymbolType {
    ST_KEYWORD,
    ST_VARIABLE,
    ST_PROCEDURE,
    ST_UNKOWN
} SymbolType;


/*
 * A symbol in the symbol table
 * It can be either identifier or reserverd word
 */
typedef struct Symbol {
    char* id;
    TokenType ttype;
    SymbolType stype;
    TypeClass type;
    bool is_global;
    bool is_arr;
    int arr_size;
    bool is_indexed;
    bool is_not_empty;
    struct SymbolNode* params;

    // LLVM Values
    LLVMValueRef llvm_value;
    LLVMValueRef llvm_function;
    LLVMValueRef llvm_address;

} Symbol;

/*
 * Linked list implementation for symbols
 */
typedef struct SymbolNode {
    Symbol symbol;
    struct SymbolNode* next_symbol;
} SymbolNode;

Symbol* init_symbol();
Symbol* init_symbol_with_id(char* id_name, TokenType token_type);
Symbol* init_symbol_with_id_symbol_type(char* id_name, TokenType token_type, SymbolType sym_type, TypeClass type_c);
Symbol get_nth_param(Symbol* sym, int idx);
int params_size(Symbol* sym);
void free_symbol(Symbol* sym);
void free_params_list(SymbolNode* head);
char* print_symbol_type(SymbolType type);
char* print_type_class(TypeClass type);

LLVMTypeRef create_llvm_type(TypeClass entry_type);

#endif