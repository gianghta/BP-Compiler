#include "include/parser.h"

LLVMBuilderRef llvm_builder;
LLVMModuleRef llvm_module;
LLVMExecutionEngineRef llvm_engine;
LLVMValueRef main_func;

/*
 * Parser constructor
 */
parser_T* init_parser(lexer_T* lexer, Semantic* sem)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->sem = sem;
    parser->current_token = (void*) 0;
    parser->look_ahead = lexer_get_next_token(lexer);

    // Create LLVM module with program identifier
    llvm_module = LLVMModuleCreateWithName("test program");
    llvm_builder = LLVMCreateBuilder();
    return parser;
}

/*
 * Eat/consume a token and look ahead the next one
 */
bool parser_eat(parser_T* parser, TokenType type)
{
    Token* tmp = init_token(type);
    printf("Matching token. Expected type: ");
    printf("%s", print_token(tmp));

    if (!is_token_type(parser, type))
    {
        printf("Token doesn't match. Current look ahead is: %s", print_token(parser->look_ahead));
        return false;
    }
    else
    {
        printf("Token matched. Current look ahead is: %s", print_token(parser->look_ahead));
        parser->current_token = parser->look_ahead;
        parser->look_ahead = lexer_get_next_token(parser->lexer);
        printf("Current token: %sLook ahead is: %s\n", print_token(parser->current_token), print_token(parser->look_ahead));
        return true;
    }

    free(tmp);
}

/*
 * Helper function checking for correct token type
 */
bool is_token_type(parser_T* parser, TokenType type)
{
    if (parser->look_ahead->type != type)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool outputAssembly(parser_T* parser)
{
    printf("\nStart parsing....\n");
    bool status = parse(parser);

    printf("Parse result is: %s\n", status ? "true" : "false");
    printf("\nPrinting global symbol table:\n");
    print_scope(parser->sem, true);

    char *error = NULL;
    LLVMVerifyModule(llvm_module, LLVMAbortProcessAction, &error);
    if (error) {
        if (*error)
        {
            printf("Module verification failed:\n%s", error);
        }
        LLVMDisposeMessage(error);
        return false;
    }

    // Initialize target registery
    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmParser();
    LLVMInitializeNativeAsmPrinter();

    if (LLVMCreateExecutionEngineForModule(&llvm_engine, llvm_module, &error) != 0) {
        fprintf(stderr, "failed to create execution engine\n");
        abort();
    }

    LLVMDumpModule(llvm_module);
    // Write out bitcode to file
    if (LLVMWriteBitcodeToFile(llvm_module, "bin/a.bc") != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
        return false;
    }

    LLVMRemoveModule(llvm_engine, llvm_module, &llvm_module, &error);
    LLVMDisposeBuilder(llvm_builder);
    LLVMDisposeExecutionEngine(llvm_engine);
    return true;
}

// Holy entry point
bool parse(parser_T* parser)
{
    return program(parser);
}

/*
 * <program> ::= <program_header> <program_body> .
 */
bool program(parser_T* parser)
{
    if (!program_header(parser))
    {
        return false;
    }

    // Main entry code block
    LLVMTypeRef main_func_type = LLVMFunctionType(LLVMVoidType(), NULL, 0, false);
    main_func = LLVMAddFunction(llvm_module, "main", main_func_type);
    LLVMBasicBlockRef main_entry = LLVMAppendBasicBlock(main_func, "main_entry");
    LLVMPositionBuilderAtEnd(llvm_builder, main_entry);

    Symbol s = *init_symbol_with_id_symbol_type("main", T_ID, ST_PROCEDURE, TC_INT);
    s.llvm_function = main_func;
    set_current_procedure(parser->sem, s);

    if (!program_body(parser))
    {
        return false;
    }
    
    if (parser->look_ahead->type == T_EOF) parser_eat(parser, T_EOF);

    // End main function, return 0
    LLVMBuildRetVoid(llvm_builder);

    exit_current_scope(parser->sem);

    return true;
}

/*
 * <program_header> ::= program <identifier> is 
 */
bool program_header(parser_T* parser)
{
    parser_eat(parser, K_PROGRAM);

    Symbol *id = init_symbol();
    id->stype = ST_VARIABLE;

    if (!identifier(parser, id))
    {
        return false;
    }

    // After module created, add runtime functions
    insert_runtime_functions(parser->sem);

    if (has_current_global_symbol(parser->sem, id->id, true))
    {
        printf("Identifier %s is already used.\n", id->id);
        return false;
    }
    else
    {
        set_symbol_semantic(parser->sem, id->id, *id, true);
    }
    
    parser_eat(parser, K_IS);

    free(id);

    return true;
}

/* <program_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end program
 */
bool program_body(parser_T* parser)
{
    if (!declaration_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_BEGIN))
    {
        return false;
    }

    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;

    // Set main entrypoint
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
    LLVMInsertExistingBasicBlockAfterInsertBlock(llvm_builder, entry);

    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_PROGRAM))
    {
        return false;
    }

    return true;
}

/*
 * <declaration> ::=
 *      [ global ] <procedure_declaration>
 *    | [ global ] <variable_declaration>
 */
bool declaration(parser_T* parser)
{   
    Symbol *decl = init_symbol();

    bool state;
    if (is_token_type(parser, K_GLOBAL))
    {
        decl->is_global = parser_eat(parser, K_GLOBAL);
    }

    switch (parser->look_ahead->type)
    {
        case K_PROCEDURE:
            state = procedure_declaration(parser, decl);
            break;
        case K_VARIABLE:
            state = variable_declaration(parser, decl);
            break;
        default:
            state = false;
    }

    free(decl);

    return state;
}

/*
 * <procedure_declaration> ::= <procedure_header> <procedure_body>
 */
bool procedure_declaration(parser_T* parser, Symbol* decl)
{
    if (!procedure_header(parser, decl))
    {
        return false;
    }

    decl->stype = ST_PROCEDURE;

    // Check for duplicate identifier in current scope and global
    if (has_current_global_symbol(parser->sem, decl->id, decl->is_global))
    {
        printf("Procedure name %s is already used in current scope.\n", decl->id);
        return false;
    }

    // Function codegen
    int param_cnt = params_size(decl);
    int counter = 0;
    LLVMTypeRef param_types[param_cnt];
    
    SymbolNode *tmp;
    tmp = decl->params;
    while (tmp != NULL && counter < param_cnt)
    {
        param_types[counter] = create_llvm_type(tmp->symbol.type);
        
        if (tmp->symbol.is_arr)
        {
            param_types[counter] = LLVMArrayType(create_llvm_type(tmp->symbol.type), params_size(&tmp->symbol));
        }
        tmp = tmp->next_symbol;
        counter += 1;
    }

    LLVMTypeRef ft = LLVMFunctionType(create_llvm_type(decl->type), param_types, param_cnt, false);
    LLVMValueRef func = LLVMAddFunction(llvm_module, decl->id, ft);

    // Set param names
    tmp = decl->params;
    LLVMValueRef current_param = LLVMGetFirstParam(func);
    while (tmp != NULL)
    {
        Symbol current_entry = tmp->symbol;
        LLVMSetValueName2(current_param, current_entry.id, strlen(current_entry.id));
        current_param = LLVMGetNextParam(current_param);
        tmp = tmp->next_symbol;
    }

    decl->llvm_function = func;

    // Set symbol in current scope
    set_symbol_semantic(parser->sem, decl->id, *decl, decl->is_global);

    // Set to procedure scope for type checking return type
    set_current_procedure(parser->sem, *decl);

    if (!procedure_body(parser))
    {
        return false;
    }

    exit_current_scope(parser->sem);

    // If global, already added to global table
    if (!decl->is_global)
    {
        // Error for duplicate name in local scope outside the function
        if (has_current_global_symbol(parser->sem, decl->id, decl->is_global))
        {
            printf("Procedure name \'%s\' is already used in this scope.\n", decl->id);
            return false;
        }

        // Set in local scope outside the function
        set_symbol_semantic(parser->sem, decl->id, *decl, decl->is_global);
    }

    return true;
}

/*
 * <procedure_header> ::=
 *      procedure <identifier> : <type_mark> ( [ <parameter_list> ] )
 */
bool procedure_header(parser_T* parser, Symbol* decl)
{
    if (!parser_eat(parser, K_PROCEDURE))
    {
        return false;
    }

    create_new_scope(parser->sem);

    if (!identifier(parser, decl))
    {
        return false;
    }

    if (!parser_eat(parser, T_COLON))
    {
        return false;
    }

    if (!type_mark(parser, decl))
    {
        return false;
    }

    if (!parser_eat(parser, T_LPAREN))
    {
        return false;
    }

    // Optional parameter list
    parameter_list(parser, decl);

    if (!parser_eat(parser, T_RPAREN))
    {
        return false;
    }
    return true;
}

/*
 * <parameter_list> ::=
 *      <parameter>, <parameter_list>
 *    | <parameter>
 */
bool parameter_list(parser_T* parser, Symbol* decl)
{   
    Symbol param = *init_symbol();
    if (!parameter(parser, &param))
    {
        return false;
    }

    if (decl->params->symbol.id == NULL)
    {
        decl->params->symbol = param;
        decl->params->next_symbol = NULL;
    }
    else
    {
        SymbolNode *ptr, *new_node;
        new_node = calloc(1, sizeof(SymbolNode));

        new_node->symbol = param;
        new_node->next_symbol = NULL;

        ptr = decl->params;

        while (ptr != NULL && ptr->next_symbol != NULL)
        {
            ptr = ptr->next_symbol;
        }

        ptr->next_symbol = new_node;
    }

    // Optional parameters
    while (is_token_type(parser, T_COMMA))
    {
        parser_eat(parser, T_COMMA);
        param = *init_symbol();
        if (!parameter(parser, &param))
        {
            return false;
        }

        if (decl->params->symbol.id == NULL)
        {
            decl->params->symbol = param;
            decl->params->next_symbol = NULL;
        }
        else
        {
            SymbolNode *ptr, *new_node;
            new_node = calloc(1, sizeof(SymbolNode));

            new_node->symbol = param;
            new_node->next_symbol = NULL;

            ptr = decl->params;

            while (ptr != NULL && ptr->next_symbol != NULL)
            {
                ptr = ptr->next_symbol;
            }

            ptr->next_symbol = new_node;
        }
    }
    return true;
}

/*
 * <parameter> ::= <variable_declaration>
 */
bool parameter(parser_T* parser, Symbol* param)
{
    return variable_declaration(parser, param);
}

/*
 * <procedure_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end procedure
 */
bool procedure_body(parser_T* parser)
{
    if (!declaration_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_BEGIN))
    {
        return false;
    }

    Symbol current_proc = get_current_procedure(parser->sem);
    LLVMValueRef func = current_proc.llvm_function;
    // Set entrypoint
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
    LLVMPositionBuilderAtEnd(llvm_builder, entry);
    
    printf("Current symbol table of procedure %s scope is: \n", current_proc.id);
    print_symbol_table(parser->sem->current_local);

    // Allocate address for parameters and variable in current symbol table
    SymbolTable* current_symbol;

    unsigned int num_symbols = symbol_table_size(parser->sem->current_local->table);
    int counter = 0;

    for (current_symbol = parser->sem->current_local->table; current_symbol != NULL; current_symbol = current_symbol->hh.next)
    {
        if (counter == num_symbols)
        {
            break;
        }

        Symbol current_entry = current_symbol->entry;
        printf("Current entry is: %s\n", current_entry.id);
        if (current_entry.stype != ST_VARIABLE)
        {
            continue;
        }

        LLVMTypeRef ty = create_llvm_type(current_entry.type);
        if (current_entry.is_arr)
        {
            ty = LLVMArrayType(create_llvm_type(current_entry.type), params_size(&current_entry));
        }

        current_entry.llvm_address = LLVMBuildAlloca(llvm_builder, ty, current_entry.id);
        update_symbol_semantic_global(parser->sem, current_entry, current_entry.is_global);
        counter += 1;
    }

    // Store argument values in allocated addresses
    SymbolNode* tmp = current_proc.params;
    LLVMValueRef current_param = LLVMGetFirstParam(func);
    while (tmp != NULL)
    {
        // Get current symbol from local table since
        // parameter linked list is not up to date with symbol table
        Symbol param = get_current_symbol(parser->sem, tmp->symbol.id);

        // Store parameter value in address
        if (param.is_arr)
        {
            // Create a dummy symbol to pass
            // Type must be same as param,
            // otherwise it would've failed in type_checking
            Symbol tmp_param_val = *init_symbol_with_id_symbol_type("", T_ID, ST_VARIABLE, param.type);
            tmp_param_val.llvm_address = current_param;
            // current_param is an llvm_address to the array;

            // Loop through each index and copy values from
            // argument to parameter (local) array
            array_assignment_codegen(parser, &param, &tmp_param_val);
        }
        else
        {
            // current_param is a normal llvm_value
            LLVMBuildStore(llvm_builder, current_param, param.llvm_address);

            // Update symbol
            param.llvm_value = current_param;
            update_symbol_semantic_global(parser->sem, param, param.is_global);
        }
        current_param = LLVMGetNextParam(current_param);
        tmp = tmp->next_symbol;
    }


    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_PROCEDURE))
    {
        return false;
    }

    LLVMBuildRet(llvm_builder, func);
    LLVMPositionBuilderAtEnd(llvm_builder, LLVMGetLastBasicBlock(main_func));

    // Verify that function has a return value
    // bool invalid = LLVMVerifyFunction(func, LLVMReturnStatusAction);
    // if (invalid)
    // {
    //     printf("Function does not have a return value.\n");
    //     return false;
    // }
    return true;
}

/*
 * <variable_declaration> ::=
 *      variable <identifier> : <type_mark> [ [ <bound ] ]
 */
bool variable_declaration(parser_T* parser, Symbol* decl)
{
    if (!parser_eat(parser, K_VARIABLE))
    {
        return false;
    }

    decl->stype = ST_VARIABLE;

    if (!identifier(parser, decl))
    {
        return false;
    }

    // Check for duplicate identifier name in current scope
    if (has_current_global_symbol(parser->sem, decl->id, decl->is_global))
    {
        printf("Variable name \'%s\' is already in used in current scope.\n", decl->id);
        return false;
    }

    if (!parser_eat(parser, T_COLON))
    {
        return false;
    }

    if (!type_mark(parser, decl))
    {
        return false;
    }

    // Optional array type
    if (is_token_type(parser, T_LBRACKET))
    {
        printf("Parsing optional variable array.\n");
        parser_eat(parser, T_LBRACKET);
        if (!bound(parser, decl))
        {
            return false;
        }

        decl->is_arr = true;

        parser_eat(parser, T_RBRACKET);
    }

    // Global variable allocation
    if (decl->is_global)
    {
        LLVMTypeRef ty = create_llvm_type(decl->type);
        if (decl->is_arr)
        {
            ty = LLVMArrayType(ty, decl->arr_size);
        }

        LLVMValueRef address = LLVMAddGlobal(llvm_module, ty, decl->id);
        decl->llvm_address = address;
    }

    // Set symbol to current scope
    set_symbol_semantic(parser->sem, decl->id, *decl, decl->is_global);

    return true;
}

/*
 * <type_mark> ::=
 *      integer | float | string | bool
 */
bool type_mark(parser_T* parser, Symbol* id)
{   
    switch (parser->look_ahead->type)
    {
        case K_INT:
            parser_eat(parser, K_INT);
            id->type = TC_INT;
            break;
        case K_FLOAT:
            parser_eat(parser, K_FLOAT);
            id->type = TC_FLOAT;
            break;
        case K_STRING:
            parser_eat(parser, K_STRING);
            id->type = TC_STRING;
            break;
        case K_BOOL:
            parser_eat(parser, K_BOOL);
            id->type = TC_BOOL;
            break;
        default:
            return false;
    }
    return true;
}

/*
 */
bool bound(parser_T* parser, Symbol* id)
{
    Symbol* num = init_symbol();
    int tmp = parser->look_ahead->value.intVal;

    if (number(parser, num) && num->type == TC_INT)
    {
        id->arr_size = tmp;
        return true;
    }
    else
    {
        return false;
    }
}

/*
 * <statement> ::=
 *      <assigment_statement>
 *    | <if_statement>
 *    | <loop_statement>
 *    | <return_statement>
 */
bool statement(parser_T* parser)
{
    // bool state;
    switch (parser->look_ahead->type)
    {
        case K_IF:
           if_statement(parser);
           return true;
        case K_FOR:
            loop_statement(parser);
            return true;
        case K_RETURN:
            return_statement(parser);
            return true;
        default:
            return assignment_statement(parser);
    }
    return false;
}

/*
 * <assignment_statement> ::= <destination> := <expression>
 */
bool assignment_statement(parser_T* parser)
{
    Symbol dest = *init_symbol();
    Symbol exp = *init_symbol();

    if (!destination(parser, &dest))
    {
        return false;
    }

    if (!parser_eat(parser, T_ASSIGNMENT))
    {
        return false;
    }

    if (!expression(parser, &exp))
    {
        return false;
    }

    // Type checking
    if (!type_checking(&dest, &exp))
    {
        return false;
    }

    return true;
}

/*
 * <destination> ::= <identifier> [ [ <expression> ] ]
 */
bool destination(parser_T* parser, Symbol* id)
{
    if (!identifier(parser, id))
    {
        return false;
    }

    // Check if identifier is in local or global scope
    if (!has_current_symbol(parser->sem, id->id))
    {
        printf("\'%s\' is not declared in scope.\n", id->id);
        return false;
    }

    // Get id from local or global scope
    *id = get_current_symbol(parser->sem, id->id);

    if (!array_index(parser, id))
    {
        return false;
    }
    return true;
}

/*
 * <if_statement> ::=
 *      if ( <expression> ) then ( <statement> ; )*
 *      [ else ( <statement> ; )* ]
 *      end if
 */
bool if_statement(parser_T* parser)
{
    if (!parser_eat(parser, K_IF))
    {
        return false;
    }

    if (!parser_eat(parser, T_LPAREN))
    {
        return false;
    }

    Symbol exp = *init_symbol();

    if (!expression(parser, &exp))
    {
        return false;
    }

    // Type checking for boolean value
    if (exp.type == TC_INT)
    {
        exp.type = TC_BOOL;
    }
    else if (exp.type != TC_BOOL)
    {
        printf("If statement expressions must evaluate to boolean value.\n");
    }

    if (!parser_eat(parser, T_RPAREN))
    {
        return false;
    }

    if (!parser_eat(parser, K_THEN))
    {
        return false;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    // Optional else statement
    if (is_token_type(parser, K_ELSE))
    {
        parser_eat(parser, K_ELSE);
        if (!statement_list(parser))
        {
            return false;
        }
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_IF))
    {
        return false;
    }
    return true;
}

/*
 * <loop_statement> ::=
 *      for ( <assignment_statement> ; <expression> )
 *          ( <statement> ; )*
 *      end for
 */
bool loop_statement(parser_T* parser)
{
    if (!parser_eat(parser, K_FOR))
    {
        return false;
    }

    if (!parser_eat(parser, T_LPAREN))
    {
        return false;
    }

    if (!assignment_statement(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_SEMI_COLON))
    {
        return false;
    }

    Symbol exp = *init_symbol();

    if (!expression(parser, &exp))
    {
        return false;
    }

    // Type checking for boolean value
    if (exp.type == TC_INT)
    {
        exp.type = TC_BOOL;
    }
    else if (exp.type != TC_BOOL)
    {
        printf("Loop statement expressions must evalutate to boolean value.\n");
    }

    if (!parser_eat(parser, T_RPAREN))
    {
        return false;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_FOR))
    {
        return false;
    }

    return true;
}

/*
 * <return_statement> ::= return <expression>
 */
bool return_statement(parser_T* parser)
{
    if (!parser_eat(parser, K_RETURN))
    {
        return false;
    }

    Symbol exp = *init_symbol();

    if (!expression(parser, &exp))
    {
        return false;
    }

    // Type checking to match procedure return type
    Symbol proc = get_current_procedure(parser->sem);
    if (proc.type == TC_UNKNOWN)
    {
        printf("Return statements must be within a procedure.\n");
        return false;
    }
    else if (!type_checking(&proc, &exp))
    {
        return false;
    }

    return true;
}

/*
 * <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
 */ 
bool identifier(parser_T* parser, Symbol* id)
{
    if (is_token_type(parser, T_ID))
    {
        id->id = parser->look_ahead->value.stringVal;
        id->ttype = parser->look_ahead->type;
    }
    return parser_eat(parser, T_ID);
}

/*
 * <expression> ::= [ not ] <arith_op> <expression_prime>
 */
bool expression(parser_T* parser, Symbol* exp)
{   
    // Optional NOT
    bool not_flag = false;
    if (is_token_type(parser, K_NOT))
    {
        not_flag = true;
        parser_eat(parser, K_NOT);
    }

    if (!arith_op(parser, exp))
    {
        return false;
    }

    // Type checking for not operator
    // Valid:
    // bool
    // int

    if (not_flag)
    {
        if (exp->type != TC_BOOL && exp->type != TC_INT)
        {
            printf("!= operator is defined for bool and int only.\n");
        }
    }

    if (!expression_prime(parser, exp))
    {
        return false;
    }

    return true;
}

/*
 * <expression_prime> ::=
 *      & <arith_op> <expression_prime>
 *    | | <arith_op> <expression_prime>
 *    | null
 */
bool expression_prime(parser_T* parser, Symbol* exp)
{
    switch (parser->look_ahead->type)
    {
        case T_AND:
            parser_eat(parser, T_AND);
            break;
        case T_OR:
            parser_eat(parser, T_OR);
            break;
        // null case
        default:
            return true;
    }

    Symbol rhs = *init_symbol();

    if (!arith_op(parser, &rhs))
    {
        return false;
    }

    // Type checking and convert type for 'and', 'or' operators
    expression_type_checking(exp, &rhs);

    if (!expression_prime(parser, exp))
    {
        return false;
    }

    return true;
}

/*
 * <arith_op> ::= <relation> <arith_op_prime>
 */
bool arith_op(parser_T* parser, Symbol* op)
{
    if (!relation(parser, op))
    {
        return false;
    }

    if (!arith_op_prime(parser, op))
    {
        return false;
    }
    return true;
}

/*
 * <arith_op_prime> ::=
 *      + <relation> <arith_op_prime>
 *    | - <relation> <arith_op_prime>
 *    | null
 */
bool arith_op_prime(parser_T* parser, Symbol* op)
{
    switch (parser->look_ahead->type)
    {
        case T_PLUS:
            parser_eat(parser, T_PLUS);
            break;
        case T_MINUS:
            parser_eat(parser, T_MINUS);
            break;
        default:
            return true;
    }

    Symbol rhs = *init_symbol();

    if (!relation(parser, &rhs))
    {
        return false;
    }

    // Type checking to convert type for + -
    if (!arithmetic_type_checking(op, &rhs))
    {
        return false;
    }

    if (!arith_op_prime(parser, op))
    {
        return false;
    }

    return true;
}

/*
 * <relation> ::= <term> <relation_prime>
 */
bool relation(parser_T* parser, Symbol* rel)
{
    if (!term(parser, rel))
    {
        return false;
    }

    if (!relation_prime(parser, rel))
    {
        return false;
    }

    return true;
}

/*
 * <relation_prime> ::=
 *          <   <term> <relation_prime>
 *        | >=  <term> <relation_prime>
 *        | >   <term> <relation_prime>
 *        | <=  <term> <relation_prime>
 *        | ==  <term> <relation_prime>
 *        | !=  <term> <relation_prime>
 *        | null
 */
bool relation_prime(parser_T* parser, Symbol* rel)
{
    Token op = *parser->look_ahead;
    
    switch (parser->look_ahead->type)
    {
        case T_LT:
            parser_eat(parser, T_LT);
            break;
        case T_LTEQ:
            parser_eat(parser, T_LTEQ);
            break;
        case T_GT:
            parser_eat(parser, T_GT);
            break;
        case T_GTEQ:
            parser_eat(parser, T_GTEQ);
            break;
        case T_EQ:
            parser_eat(parser, T_EQ);
            break;
        case T_NOT_EQ:
            parser_eat(parser, T_NOT_EQ);
            break;
        default:
            return true;
    }

    Symbol rhs = *init_symbol();


    if (!term(parser, &rhs))
    {
        return false;
    }

    // Type checking to convert type for relational operators
    if (!relation_type_checking(rel, &rhs, &op))
    {
        return false;
    }

    // Relation successfully evaluates to boolean
    rel->type = TC_BOOL;

    if (!relation_prime(parser, rel))
    {
        return false;
    }

    return true;
}

/*
 * <term> ::= <factor> <term_prime>
 */
bool term(parser_T* parser, Symbol* tm)
{
    if (!factor(parser, tm))
    {
        return false;
    }

    if (!term_prime(parser, tm))
    {
        return false;
    }

    return true;
}

/*
 * <term_prime> ::=
 *       *  <factor> <term_prime>
 *     | /  <factor> <term_prime>
 *     | null
 */
bool term_prime(parser_T* parser, Symbol* tm)
{
    switch (parser->look_ahead->type)
    {
        case T_MULTIPLY:
            parser_eat(parser, T_MULTIPLY);
            break;
        case T_DIVIDE:
            parser_eat(parser, T_DIVIDE);
            break;
        default:
            return true;
    }

    Symbol rhs = *init_symbol();

    if (!factor(parser, &rhs))
    {
        return false;
    }

    // Type checking to convert type for * / operation
    if (!arithmetic_type_checking(tm, &rhs))
    {
        return false;
    }

    if (!term_prime(parser, tm))
    {
        return false;
    }

    return true;
}

/*
 * <factor> ::=
 *      ( <expression> )
 *    | <procedure_call>
 *    | [ - ] <name>
 *    | [ - ] <number>
 *    | <string>
 *    | true
 *    | false
 */
bool factor(parser_T* parser, Symbol* fac)
{
    if (parser_eat(parser, T_LPAREN))
    {
        if (!expression(parser, fac))
        {
            return false;
        }

        if (!parser_eat(parser, T_RPAREN))
        {
            return false;
        }
        return true;
    }
    else if (procedure_call_or_name_handler(parser, fac))
    {
        return true;
    }
    else if (parser_eat(parser, T_MINUS))
    {
        if (name(parser, fac))
        {
            return true;
        }
        else if (number(parser, fac))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (number(parser, fac))
    {
        return true;
    }
    else if (string(parser, fac))
    {
        return true;
    }
    else if (parser_eat(parser, K_TRUE))
    {
        fac->ttype = K_TRUE;
        fac->type = TC_BOOL;
        return true;
    }
    else if (parser_eat(parser, K_FALSE))
    {
        fac->ttype = K_FALSE;
        fac->type = TC_BOOL;
        return true;
    }
    else {
        return false;
    }
}

/*
 * Helper for procedure call and name
 * because they both start with an identifier
 * 
 * <procedure_call> ::= <identifier> ( [argument_list ] )
 * 
 * <name> ::= <identifier> [ [ <expression> ] ]
 */
bool procedure_call_or_name_handler(parser_T* parser, Symbol* id)
{
    if (!identifier(parser, id))
    {
        return false;
    }

    // Check if identifier is defined in local or global scope
    if (!has_current_symbol(parser->sem, id->id))
    {
        printf("Identifier \'%s\' is not declared in local or global scope.\n", id->id);
        return false;
    }
    // Get Identifier
    *id = get_current_symbol(parser->sem, id->id);

    switch (parser->look_ahead->type)
    {
        case T_LPAREN:
            parser_eat(parser, T_LPAREN);

            // Optional arguments
            argument_list(parser, id);

            if (!parser_eat(parser, T_RPAREN))
            {
                printf("Missing \')\' in procedure call.\n");
                return false;
            }
        default:
            // Optional array index
            if (!array_index(parser, id))
            {
                return false;
            }
    }

    return true;
}

/*
 * <name> ::= <identifier> [ [ <expression> ] ]
 */
bool name(parser_T* parser, Symbol* id)
{
    if (!identifier(parser, id))
    {
        return false;
    }

    // Check if identifier is in local or global scope
    if (!has_current_symbol(parser->sem, id->id))
    {
        printf("Identifier \'%s\' is not declared in local or global scope.\n", id->id);
        return false;
    }

    // Get Id
    *id = get_current_symbol(parser->sem, id->id);

    if (!array_index(parser, id))
    {
        return false;
    }
    return true;
}

/*
 * Handler for array index [ [ <expression> ] ]
 */
bool array_index(parser_T* parser, Symbol* id)
{
    if (parser_eat(parser, T_LBRACKET))
    {
        Symbol exp = *init_symbol();
        if (!expression(parser, &exp))
        {
            return false;
        }

        // Check valid array access
        if (!id->is_arr)
        {
            printf("Identifier \'%s\' is not an array. Invalid array access.\n", id->id);
            return false;
        }
        else if (exp.type != TC_INT)
        {
            printf("Array index must be integer.\n");
            return false;
        }

        id->is_indexed = true;

        if (!parser_eat(parser, T_RBRACKET))
        {
            printf("Missing \']\' in array index access.\n");
            return false;
        }
    }
    return true;
}

/*
 * <argument_list> ::=
 *      <expression>, <argument_list>
 *    | <expression>
 */
bool argument_list(parser_T* parser, Symbol* id)
{
    Symbol arg = *init_symbol();
    int arg_index = 0;

    if (!expression(parser, &arg))
    {
        if (arg_index != params_size(id))
        {
            printf("Too few arguments provided for \'%s\'.\n", id->id);
        }
        return false;
    }

    // Grab parameter symbol at corresponding index
    Symbol sym = get_nth_param(id, arg_index);

    // Check for too much parameters 
    if (arg_index >= params_size(id))
    {
        printf("Too many arguments provivded to \'%s\'.\n", id->id);
        return false;
    }
    // Type checking match parameter type
    else if (!type_checking(&sym, &arg))
    {
        return false;
    }

    // Increment count
    arg_index++;
    

    // Optional arguments
    while (is_token_type(parser, T_COMMA))
    {
        arg = *init_symbol();
        parser_eat(parser, T_COMMA);
        if (!expression(parser, &arg))
        {
            printf("Invalid argument.\n");
            return false;
        }

        // Grab parameter symbol at corresponding index
        sym = get_nth_param(id, arg_index);

        // Check for too much parameters 
        if (arg_index >= params_size(id))
        {
            printf("Too many arguments provivded to \'%s\'.\n", id->id);
            return false;
        }
        // Type checking match parameter type
        else if (!type_checking(&sym, &arg))
        {
            return false;
        }

        // Increment count
        arg_index++;
    }

    // Check number of params
    if (arg_index != params_size(id)) {
        printf("Too many arguments provivded to \'%s\'.\n", id->id);
        return false;
    }

    return true;
}

/*
 * <number> ::= [0-9][0-9_]*[.[0-9_]*]
 */
bool number(parser_T* parser, Symbol* num)
{
    if (is_token_type(parser, T_NUMBER_INT))
    {
        num->type = TC_INT;
        num->ttype = T_NUMBER_INT;
        return parser_eat(parser, T_NUMBER_INT);
    }
    else if (is_token_type(parser, T_NUMBER_FLOAT))
    {
        num->type = TC_FLOAT;
        num->ttype = T_NUMBER_FLOAT;
        return parser_eat(parser, T_NUMBER_FLOAT);
    }
    else {
        return false;
    }
}

/*
 * <string> :: = "[^"]*"
 */
bool string(parser_T* parser, Symbol* str)
{
    if (is_token_type(parser, T_STRING))
    {
        str->id = parser->look_ahead->value.stringVal;
        str->ttype = parser->look_ahead->type;
        str->type = TC_STRING;
    }
    // Eat token
    return parser_eat(parser, T_STRING);
}

/* 
 * A list of declaration for ( <declaration> ; )*
 */
bool declaration_list(parser_T* parser)
{
    // Zero or more declarations
    while (declaration(parser)) {
        if (!parser_eat(parser, T_SEMI_COLON)) {
            return false;
        }
    }
    return true;
}

/*
 * A list of statements for ( <statement> ; )*
 */
bool statement_list(parser_T* parser)
{
    // Zero or more statements
    while(statement(parser))
    {
        if (!parser_eat(parser, T_SEMI_COLON))
        {
            return false;
        }
    }
    return true;
}

/*
 * Type checking for relational operators < <= > >= == !=
 */
bool relation_type_checking(Symbol* lhs, Symbol* rhs, Token* op)
{
    bool compatible = false;

    // Convert integer to float or bool for comparision

    if (lhs->type == TC_INT)
    {
        if (rhs->type == TC_BOOL)
        {
            compatible = true;
            // Convert to boolean
            lhs->type = TC_BOOL;
        }
        else if (rhs->type == TC_FLOAT)
        {
            compatible = true;
            //Convert to float
            lhs->type = TC_FLOAT;
        }
        else if (rhs->type == TC_INT)
        {
            // Nothing changes, compatible
            compatible = true;
        }
    }
    else if (lhs->type == TC_FLOAT)
    {
        if (rhs->type == TC_FLOAT)
        {
            // Do nothing
            compatible = true;
        }
        else if (rhs->type == TC_INT)
        {
            compatible = true;
            // Convert to float
            rhs->type = TC_FLOAT;
        }
    }
    else if (lhs->type == TC_BOOL)
    {
        if (rhs->type == TC_BOOL)
        {
            // Do nothing
            compatible = true;
        }
        else if (rhs->type == TC_INT)
        {
            compatible = true;
            // Convert to bool
            rhs->type = TC_BOOL;
        }
    }
    else if (lhs->type == TC_STRING)
    {
        // Only == != operations are performed on string type
        if (rhs->type == TC_STRING && (op->type == T_EQ || op->type == T_NOT_EQ))
        {
            compatible = true;
        }
    }

    if (!compatible)
    {
        printf("Types are not compatible for relational operations.\n");
    }
    return compatible;
}

/*
 * Type checkign for arithmetic operators + - * /
 */
bool arithmetic_type_checking(Symbol* lhs, Symbol* rhs)
{
    if ((lhs->type != TC_INT && lhs->type != TC_FLOAT) || (rhs->type != TC_INT && rhs->type != TC_FLOAT))
    {
        printf("Arithmetic operators are only for int and float.\n");
        return false;
    }

    if (lhs->type == TC_INT)
    {
        if (rhs->type == TC_FLOAT)
        {
            // Convert lhs to float
            lhs->type = TC_FLOAT;
        }

        // Don't match otherwise
    }
    else
    {
        // lhs is float
        if (rhs->type == TC_INT)
        {
            // Convert rhs to float
            rhs->type = TC_FLOAT;
        }
        // Don't match otherwise
    }
    return true;
}

/*
 * Type checking for expression operators & |
 */
bool expression_type_checking(Symbol* lhs, Symbol* rhs)
{
    bool compatible = false;

    if (lhs->type == TC_BOOL && rhs->type == TC_BOOL)
    {
        compatible = true;
    }
    else if (lhs->type == TC_INT && rhs->type == TC_INT)
    {
        compatible = true;
    }

    if (!compatible)
    {
        printf("Expression operators are defined for bool and int only.\n");
    }
    return compatible;
}

// Codegen to copy the elements from one array to another
void array_assignment_codegen(parser_T* parser, Symbol* dest, Symbol* exp)
{
    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;

    LLVMBasicBlockRef arr_copy_block = LLVMAppendBasicBlock(func, "arrCopy");
    LLVMBasicBlockRef arr_copy_merge_block = LLVMAppendBasicBlock(func, "arrCopyMerge");

    // Initial index = 0
    LLVMValueRef ind_addr = LLVMBuildAlloca(llvm_builder, create_llvm_type(TC_INT), "arrCopyInd");
    LLVMValueRef zero_val = LLVMConstInt(LLVMInt32Type(), 0, true);
    LLVMValueRef index = zero_val;
    LLVMBuildStore(llvm_builder, index, ind_addr);

    // Max value of index is arr_size - 1
    LLVMValueRef loop_end = LLVMConstInt(LLVMInt32Type(), dest->arr_size, true);
    LLVMBuildBr(llvm_builder, arr_copy_block);
    LLVMInsertExistingBasicBlockAfterInsertBlock(llvm_builder, arr_copy_block);

    index = LLVMBuildLoad2(llvm_builder, create_llvm_type(TC_INT), ind_addr, "");

    // Get pointer to array element, and load the value
    LLVMValueRef exp_elem_addr = LLVMBuildInBoundsGEP(llvm_builder, exp->llvm_address, &zero_val, (int) index, "");
    LLVMValueRef exp_elem_val = LLVMBuildLoad2(llvm_builder, create_llvm_type(dest->type), exp_elem_addr, "");

    // Get pointer to dest array element, and store the value
    LLVMValueRef dest_elem_addr = LLVMBuildInBoundsGEP(llvm_builder, dest->llvm_address, &zero_val, (int) index, "");
    LLVMBuildStore(llvm_builder, exp_elem_val, dest_elem_addr);

    // Increment index
    LLVMValueRef increment = LLVMConstInt(LLVMInt32Type(), 1, true);
    index = LLVMBuildAdd(llvm_builder, index, increment, "");
    LLVMBuildStore(llvm_builder, index, ind_addr);

    // index < arr size
    LLVMValueRef cond = LLVMBuildICmp(llvm_builder, LLVMIntSLT, index, loop_end, "");
    LLVMBuildCondBr(llvm_builder, cond, arr_copy_block, arr_copy_merge_block);

    LLVMInsertExistingBasicBlockAfterInsertBlock(llvm_builder, arr_copy_merge_block);
}

/*
 * Type checking for assignment operator
 * making destination and return type matches.
 * Also, matching parameters to arguments 
 */

bool type_checking(Symbol* dest, Symbol* exp)
{
    bool compatible = false;

    /*
     * Check valid matching of array and array index
     * Valid check:
     * var = var
     * arr = arr
     * arr[i] = arr[i]
     * arr[i] = var
     * var = arr[i]
     */
    if (dest->is_arr || exp->is_arr)
    {
        if (dest->is_arr && exp->is_arr)
        {
            // In case both are array type

            if (dest->is_indexed != exp->is_indexed)
            {
                printf("Incompatible index match of arrays.\n");
                compatible = false;
            }
            else if (!dest->is_indexed)
            {
                // Both are unindexed. Array lengths must match
                if (dest->arr_size != exp->arr_size)
                {
                    printf("Array lengths must match.\n");
                    compatible = false;
                }
            }
        }
        else
        {
            // One side is array
            // Array must be indexed
            if ((dest->is_arr && !dest->is_indexed) || (exp->is_arr && !exp->is_indexed))
            {
                printf("Array is not indexed.\n");
                compatible = false;
            }
        }
    }
    // Both are not arrays


    if (dest->type == exp->type)
    {
        compatible = true;
    }
    else if (dest->type == TC_INT)
    {
        if (exp->type == TC_BOOL)
        {
            compatible = true;

            if (!(exp->is_arr && !exp->is_indexed))
            {
                // Convert exp to int
                exp->type = TC_INT;
                exp->llvm_value = LLVMConstIntCast(exp->llvm_value, LLVMInt32Type(), false);
            }
        }
        else if (exp->type == TC_FLOAT)
        {
            compatible = true;
            if (!(exp->is_arr && !exp->is_indexed))
            {
                // Convert exp to int
                exp->type = TC_INT;
                exp->llvm_value = LLVMConstFPToSI(exp->llvm_value, LLVMInt32Type());
            }
        }
    }
    else if (dest->type == TC_FLOAT)
    {
        if (exp->type == TC_INT)
        {
            compatible = true;

            if (!(exp->is_arr && !exp->is_indexed))
            {
                // Convert exp to float
                exp->type = TC_FLOAT;
                exp->llvm_value = LLVMConstSIToFP(exp->llvm_value, LLVMFloatType());
            }
        }
    }
    else if (dest->type == TC_BOOL)
    {
        if (exp->type == TC_INT)
        {
            compatible = true;

            if (!exp->is_arr && !exp->is_indexed)
            {
                // Convert exp to bool
                exp->type = TC_BOOL;
                exp->llvm_value = LLVMConstICmp(LLVMIntNE , exp->llvm_value, LLVMConstInt(LLVMInt32Type(), 0, true));
            }
        }
    }

    if (!compatible)
    {
        printf(                                        \
            "Incompatible types \'%s\' and \'%s\'.\n", \
            print_type_class(dest->type),              \
            print_type_class(exp->type)                \
        );                                             \
    }

    return compatible;
}