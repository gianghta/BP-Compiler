#include "include/parser.h"

LLVMBuilderRef llvm_builder;
LLVMModuleRef llvm_module;
LLVMExecutionEngineRef llvm_engine;
LLVMContextRef llvm_context;
LLVMValueRef main_func;

LLVMTypeRef int32_type;
LLVMTypeRef int8_type;
LLVMTypeRef int1_type;
LLVMTypeRef float_type;


bool error_flag;


/*
 * Parser constructor
 */
parser_T* init_parser(lexer_T* lexer, Semantic* sem, bool flag, bool table_flag, bool jit_flag)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->sem = sem;
    parser->current_token = (void*) 0;
    parser->look_ahead = lexer_get_next_token(lexer);

    error_flag = false;
    parser->flag = flag;
    parser->table_flag = table_flag;
    parser->jit_flag = jit_flag;
    return parser;
}

/*
 * Eat/consume a token and look ahead the next one
 */
bool parser_eat(parser_T* parser, TokenType type)
{
    Token* tmp = init_token(type);
    debug_parser_statement("Matching token. Expected type: ", parser->flag);
    debug_parser_statement(concatf("%s", print_token(tmp)), parser->flag);

    if (!is_token_type(parser, type))
    {
        debug_parser_statement(concatf("Token doesn't match. Current look ahead is: %s", print_token(parser->look_ahead)), parser->flag);
        return false;
    }
    else
    {
        debug_parser_statement(concatf("Token matched. Current look ahead is: %s", print_token(parser->look_ahead)), parser->flag);
        parser->current_token = parser->look_ahead;
        parser->look_ahead = lexer_get_next_token(parser->lexer);
        debug_parser_statement(concatf("Current token: %sLook ahead is: %s\n", print_token(parser->current_token), print_token(parser->look_ahead)), parser->flag);
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

bool resync(parser_T* parser, TokenType tokens[], int count)
{
    TokenType always_check_token = T_EOF;

    while (error_flag)
    {
        // Check if the current token is in either array
        for (int i = 0; i < count; i++)
        {
            if (parser->look_ahead->type == tokens[i])
            {
                error_flag = false;
                break;
            }
        }

        if (parser->look_ahead->type == always_check_token)
        {
            return false;
        }

        // Ignore current token and scan the next one
        parser->current_token = parser->look_ahead;
        parser->look_ahead = lexer_get_next_token(parser->lexer);
    }
    return true;
}

bool output_bitcode(parser_T* parser)
{
    // Initialize
    LLVMLinkInMCJIT();
    LLVMLinkInInterpreter();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();

    // Get triple
    char* triple = LLVMGetDefaultTargetTriple();
    LLVMTargetRef target_ref;
    char* err;

    if (LLVMGetTargetFromTriple(triple, &target_ref, &err))
    {
        throw_error(concatf("Error: %s\n", err), parser->look_ahead);
        return 1;
    }

    LLVMTargetMachineRef tm_ref = LLVMCreateTargetMachine(
        target_ref,              // 
        triple,                  // 
        "generic",               // const char* cpu
        "",                      // const char* features
        LLVMCodeGenLevelDefault, // level
        LLVMRelocStatic,         // reloc
        LLVMCodeModelJITDefault  // code model
    );
    LLVMDisposeMessage(triple);

    // Create context
    llvm_context = LLVMContextCreate();

    // Types in context
    int32_type = LLVMInt32TypeInContext(llvm_context);
    int8_type = LLVMInt8TypeInContext(llvm_context);
    int1_type = LLVMInt1TypeInContext(llvm_context);
    float_type = LLVMFloatTypeInContext(llvm_context);

    // Create builder
    // Create and position builder
    llvm_builder = LLVMCreateBuilderInContext(llvm_context);

    debug_parser_statement("\nStart parsing....\n", parser->flag);
    bool status = parse(parser);
    if (!status)
    {
        printf("Failed to parse the program. Exiting...\n");
        // Cleanup
        LLVMDisposeBuilder(llvm_builder);
        LLVMDisposeModule(llvm_module);
        LLVMContextDispose(llvm_context);
        exit(1);
    }

    if (parser->jit_flag)
    {
        printf("Printing out module (before compilation success):\n");
        printf("%s", LLVMPrintModuleToString(llvm_module));
    }

    //--- Analysis and execution

    // Verify the module
    err = NULL;

    LLVMVerifyModule(llvm_module, LLVMAbortProcessAction, &err);
    LLVMDisposeMessage(err);

    // Build executor
    err         = NULL;
    llvm_engine = NULL;

    if (LLVMCreateExecutionEngineForModule(&llvm_engine, llvm_module, &err) != 0)
    {
        fprintf(stderr, "Failed to create execution engine\n");
        abort();
    }

    if (err)
    {
        fprintf(stderr, "Error: %s\n", err);
        LLVMDisposeMessage(err);
        exit(EXIT_FAILURE);
    }

    // Write out bitcode to file
    if (LLVMWriteBitcodeToFile(llvm_module, "dist/result.bc") != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
    }


    if (err)
    {
        fprintf(stderr, "Error: %s\n", err);
        LLVMDisposeMessage(err);
    }

    // Dump module
    // fprintf(stderr, "\n--- Module ---\n");

    // LLVMDumpModule(llvm_module);

    // fprintf(stderr, "--------------\n");

    // Cleanup
    LLVMDisposeBuilder(llvm_builder);
    LLVMDisposeModule(llvm_module);
    LLVMContextDispose(llvm_context);
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
    LLVMTypeRef return_type   = LLVMVoidTypeInContext(llvm_context);
    LLVMTypeRef main_func_type = LLVMFunctionType(return_type, NULL, 0, false);
    main_func = LLVMAddFunction(llvm_module, "main", main_func_type);
    LLVMSetLinkage(main_func, LLVMExternalLinkage);

    Symbol* s = init_symbol_with_id_symbol_type("main", T_ID, ST_PROCEDURE, TC_VOID);
    s->llvm_function = main_func;
    set_current_procedure(parser->sem, *s);

    if (!program_body(parser))
    {
        return false;
    }
    
    // Period denotes end of file, is a must for the program to function
    // in this case.
    if (!parser_eat(parser, T_EOF))
    {
        throw_error("Missing period at the end of program.\n", parser->look_ahead);
        return false;
    }

    if (parser->table_flag)
    {
        printf("Final global symbol table is:\n");
        print_scope(parser->sem, true);
    }

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

    if (!identifier(parser, id))
    {
        return false;
    }

    // Create LLVM module with program identifier
    llvm_module = LLVMModuleCreateWithNameInContext(id->id, llvm_context);
    
    // Create runtime module and link it with main module
    LLVMMemoryBufferRef buffer = NULL;
    char* err = NULL;
    LLVMCreateMemoryBufferWithContentsOfFile("src/runtime.ll", &buffer, &err);
    LLVMParseIRInContext(llvm_context, buffer, &llvm_module, NULL);
    LLVMSetModuleIdentifier(llvm_module, id->id, strlen(id->id));

    // After module created, add runtime functions
    insert_runtime_functions(parser->sem);

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
        throw_error("Missing \'begin\' keyword in the program.\n", parser->look_ahead);
        return false;
    }

    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;

    // Set main entrypoint
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(llvm_context, func, "entry");
    LLVMPositionBuilderAtEnd(llvm_builder, entry);

    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        throw_error("Missing \'end\' keyword in program body.\n", parser->look_ahead);
        return false;
    }

    if (!parser_eat(parser, K_PROGRAM))
    {
        throw_error("Missing \'program\' keyword in program body.\n", parser->look_ahead);
        return false;
    }

    // End main function, return 0
    LLVMBuildRetVoid(llvm_builder);

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

    if (is_token_type(parser, K_GLOBAL))
    {
        decl->is_global = parser_eat(parser, K_GLOBAL);
    }
    else
    {
        decl->is_global = is_current_scope_global(parser->sem);
    }

    bool state = false;
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
        throw_error(concatf("Procedure name %s is already used in current scope.\n", decl->id), parser->look_ahead);
        return false;
    }

    // Function codegen, parsing parameters first
    int param_cnt = params_size(decl);
    int counter = 0;
    LLVMTypeRef* param_types = (LLVMTypeRef *) malloc(sizeof(LLVMTypeRef) * param_cnt);
    
    SymbolNode *tmp;
    LLVMTypeRef ty;
    tmp = decl->params;
    while (tmp != NULL)
    {
        ty = create_llvm_type(tmp->symbol.type);
        if (tmp->symbol.is_arr)
        {
            param_types[counter++] = LLVMArrayType(ty, params_size(&tmp->symbol));
        }
        else
        {
            param_types[counter++] = create_llvm_type(tmp->symbol.type);
        }
        tmp = tmp->next_symbol;
    }


    LLVMTypeRef ft = LLVMFunctionType(create_llvm_type(decl->type), param_types, param_cnt, false);
    LLVMValueRef func = LLVMAddFunction(llvm_module, decl->id, ft);
    LLVMSetLinkage(func, LLVMExternalLinkage);

    // Set parameter names
    tmp = decl->params;
    counter = 0;

    while (tmp != NULL && counter < param_cnt)
    {
        Symbol current_args = tmp->symbol;
        LLVMValueRef param = LLVMGetParam(func, counter);
        LLVMSetValueName2(param, current_args.id, strlen(current_args.id));
        counter++;
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
    
    // Exit scope
    exit_current_scope(parser->sem);

    // If global, already added to global table
    if (!decl->is_global)
    {
        // Error for duplicate name in local scope outside the function
        if (has_current_global_symbol(parser->sem, decl->id, decl->is_global))
        {
            throw_error(concatf("Procedure name \'%s\' is already used in this scope.\n", decl->id), parser->look_ahead);
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
        throw_error(concatf("Invalid identifier \'%s\'\n", decl->id), parser->look_ahead);
        return false;
    }

    if (!parser_eat(parser, T_COLON))
    {
        throw_error(concatf("Missing \':\' in procedure header.\n"), parser->look_ahead);
        return false;
    }

    if (!type_mark(parser, decl))
    {
        throw_error(concatf("Invalid type mark\n"), parser->look_ahead);
        return false;
    }

    if (!parser_eat(parser, T_LPAREN))
    {
        throw_error(concatf("Missing \'(\' in procedure header.\n"), parser->look_ahead);
        return false;
    }

    // Optional parameter list
    parameter_list(parser, decl);

    if (error_flag)
    {
        return false;
    }

    if (!parser_eat(parser, T_RPAREN))
    {
        throw_error("Missing \')\' in procedure header.\n", parser->look_ahead);
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

    if (decl->params->symbol.is_not_empty == false)
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
            throw_error("Invalid parameter.\n", parser->look_ahead);
            return false;
        }

        if (decl->params->symbol.is_not_empty == false)
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

    // Set entrypoint for function
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(llvm_context, func, "entry");
    LLVMPositionBuilderAtEnd(llvm_builder, entry);

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
        if (current_entry.stype != ST_VARIABLE)
        {
            continue;
        }

        LLVMTypeRef ty = NULL;
        LLVMValueRef addr = NULL;
        if (current_entry.is_arr)
        {
            ty = LLVMArrayType(create_llvm_type(current_entry.type), current_entry.arr_size);
            addr = LLVMBuildArrayAlloca(llvm_builder, ty, NULL, current_entry.id);
        }
        else
        {
            ty = create_llvm_type(current_entry.type);
            addr = LLVMBuildAlloca(llvm_builder, ty, current_entry.id);
        }

        current_entry.llvm_address = addr;
        update_symbol_semantic_global(parser->sem, current_entry, current_entry.is_global);
        counter += 1;
    }

    
    // Store argument values in allocated addresses
    SymbolNode *tmp;
    counter = 0;
    int param_cnt = params_size(&current_proc);
    LLVMValueRef current_param = NULL;
    tmp = current_proc.params;

    while (tmp != NULL && counter < param_cnt)
    {
        current_param = LLVMGetParam(func, counter);
        // Get current symbol from local table since
        // parameter linked list is not up to date with symbol table
        Symbol param = get_current_global_symbol(parser->sem, tmp->symbol.id, tmp->symbol.is_global);

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
        tmp = tmp->next_symbol;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        throw_error("Missing \'end\' keyword in procedure body\n", parser->look_ahead);
        return false;
    }

    if (!parser_eat(parser, K_PROCEDURE))
    {
        throw_error("Missing \'procedure\' keyword at the end of procedure.\n", parser->look_ahead);
        return false;
    }

    // Verify that function has a return value
    LLVMBool invalid = LLVMVerifyFunction(func, LLVMReturnStatusAction);
    if (invalid)
    {
        throw_error("Function does not have a return value.\n", parser->look_ahead);
        return false;
    }

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
        throw_error(concatf("Invalid identifier \'%s\'\n", decl->id), parser->look_ahead);
        return false;
    }

    // Check for duplicate identifier name in current scope
    if (has_current_global_symbol(parser->sem, decl->id, decl->is_global))
    {
        throw_error(concatf("Variable name \'%s\' is already in used in current scope.\n", decl->id), parser->look_ahead);
        return false;
    }

    if (!parser_eat(parser, T_COLON))
    {
        throw_error(concatf("Missing \':\' in variable declaration.\n"), parser->look_ahead);
        return false;
    }

    if (!type_mark(parser, decl))
    {
        throw_error("Invalid type mark.\n", parser->look_ahead);
        return false;
    }

    // Optional array type
    if (is_token_type(parser, T_LBRACKET))
    {
        parser_eat(parser, T_LBRACKET);
        if (!bound(parser, decl))
        {
            throw_error("Invalid bound.\n", parser->look_ahead);
            return false;
        }

        decl->is_arr = true;

        if (!parser_eat(parser, T_RBRACKET))
        {
            throw_error("Missing \']\' in variable bound.\n", parser->look_ahead);
        }
    }

    // Global variable allocation
    if (decl->is_global)
    {
        LLVMTypeRef ty = create_llvm_type(decl->type);
        if (decl->is_arr)
        {
            ty = LLVMArrayType(ty, decl->arr_size);
        }

        // Default value
        LLVMValueRef init_val = LLVMConstNull(ty);
        LLVMValueRef address = LLVMAddGlobal(llvm_module, ty, decl->id);
        LLVMSetInitializer(address, init_val);
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

    if (number(parser, num) && num->type == TC_INT && tmp > 0)
    {
        id->arr_size = tmp;
        return true;
    }
    else
    {   
        throw_error("Invalid bound value. Must be a positive integer.\n", parser->look_ahead);
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
    if (assignment_statement(parser))
    {

    }
    else if (if_statement(parser))
    {

    }
    else if (loop_statement(parser))
    {
        
    }
    else if (return_statement(parser))
    {
        
    }
    else
    {
        return false;
    }
    return true;
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
    if (!type_checking(parser, &dest, &exp))
    {
        return false;
    }

    // Assignment codegen
    if (dest.is_arr && !dest.is_indexed)
    {
        // Bot dest and exp are unindexed arrays;
        // Copy element by element
        array_assignment_codegen(parser, &dest, &exp);
    }
    else
    {
        LLVMBuildStore(llvm_builder, exp.llvm_value, dest.llvm_address);
    }

    // Update symbol
    if (!dest.is_arr)
    {
        dest.llvm_value = exp.llvm_value;
        update_symbol_semantic_global(parser->sem, dest, dest.is_global);
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
        throw_error(concatf("\'%s\' is not declared in scope.\n", id->id), parser->look_ahead);
        return false;
    }

    // Get id from local or global scope
    *id = get_current_global_symbol(parser->sem, id->id, id->is_global);

    // Confirm that it's a name
    if (id->stype != ST_VARIABLE)
    {
        throw_error(concatf("%s is not a valid destination\n", id->id), parser->look_ahead);
        return false;
    }

    Symbol ind = *init_symbol();
    if (!array_index(parser, id, &ind))
    {
        return false;
    }


    // Codegen: Get array destination address
    if (id->is_indexed)
    {
        LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);

        // Get pointer to the element of the array
        LLVMValueRef indices[] = { zero_val, ind.llvm_value };
        id->llvm_address = LLVMBuildInBoundsGEP(llvm_builder, id->llvm_address, indices, 2, "");
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
        throw_error("Missing \'(\' in if statement\n", parser->look_ahead);
        return false;
    }

    Symbol exp = *init_symbol();

    if (!expression(parser, &exp))
    {
        return false;
    }

    if (!parser_eat(parser, T_RPAREN))
    {
        throw_error("Missing \')\' in if statement\n", parser->look_ahead);
        return false;
    }

    // Type check/convert to bool
    if (exp.type == TC_INT)
    {
        exp.type = TC_BOOL;

        LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);
        exp.llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntNE, exp.llvm_value, zero_val, "");
    }
    else if (exp.type != TC_BOOL)
    {
        throw_error("If statement expression must evaluate to bool.\n", parser->look_ahead);
        return false;
    }

    // Codegen If statement
    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;
    LLVMValueRef zero_val = LLVMConstInt(int1_type, 0, true);
    LLVMValueRef if_cond = LLVMBuildICmp(llvm_builder, LLVMIntNE, exp.llvm_value, zero_val, "");
    exp.llvm_value = if_cond;

    LLVMBasicBlockRef if_then_block = LLVMAppendBasicBlockInContext(llvm_context, func, "ifThen");
    LLVMBasicBlockRef else_block = LLVMAppendBasicBlockInContext(llvm_context, func, "ifElse");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlockInContext(llvm_context, func, "ifMerge");

    LLVMBuildCondBr(llvm_builder, if_cond, if_then_block, else_block);
    LLVMPositionBuilderAtEnd(llvm_builder, if_then_block);

    if (!parser_eat(parser, K_THEN))
    {
        throw_error("Missing \'then\' in if statement\n", parser->look_ahead);
        return false;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    // Merge if_then_block into merge_block if there wasn't a return
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(llvm_builder)) == NULL)
    {
        LLVMBuildBr(llvm_builder, merge_block);
    }

    LLVMPositionBuilderAtEnd(llvm_builder, else_block);

    // Optional else statement
    if (is_token_type(parser, K_ELSE))
    {
        parser_eat(parser, K_ELSE);
        if (!statement_list(parser))
        {
            return false;
        }
    }

    // Merge else block into merge block if there wasn't a return
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(llvm_builder)) == NULL)
    {
        LLVMBuildBr(llvm_builder, merge_block);
    }

    LLVMPositionBuilderAtEnd(llvm_builder, merge_block);

    if (!parser_eat(parser, K_END))
    {
        throw_error("Missing \'end\' in if statement\n", parser->look_ahead);
        return false;
    }

    if (!parser_eat(parser, K_IF))
    {
        throw_error("Missing closing \'if\'\n", parser->look_ahead);
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
        throw_error("Missing \'(\' in loop\n", parser->look_ahead);
        return false;
    }

    if (!assignment_statement(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_SEMI_COLON))
    {
        throw_error("Missing \':\' in loop\n", parser->look_ahead);
        return false;
    }

    // Codegen: loop
    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;

    LLVMBasicBlockRef loop_header_block = LLVMAppendBasicBlockInContext(llvm_context, func, "loop_head");
    LLVMBasicBlockRef loop_body_block = LLVMAppendBasicBlockInContext(llvm_context, func, "loop_body");
    LLVMBasicBlockRef loop_merge_block = LLVMAppendBasicBlockInContext(llvm_context, func, "loop_merge");

    LLVMBuildBr(llvm_builder, loop_header_block);
    LLVMPositionBuilderAtEnd(llvm_builder, loop_header_block);

    Symbol exp = *init_symbol();

    if (!expression(parser, &exp))
    {
        return false;
    }

    exp = get_current_global_symbol(parser->sem, exp.id, exp.is_global);

    if (!parser_eat(parser, T_RPAREN))
    {
        throw_error("Missing \')\' in loop\n", parser->look_ahead);
        return false;
    }

    // Type checking for boolean value
    if (exp.type == TC_INT)
    {
        exp.type = TC_BOOL;

        LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);
        exp.llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntNE, exp.llvm_value, zero_val, "");
    }
    else if (exp.type != TC_BOOL)
    {
        throw_error("Loop statement expressions must evalutate to boolean value.\n", parser->look_ahead);
        return false;
    }

    // Codegen: Loop condition
    LLVMValueRef zero_val = LLVMConstInt(int1_type, 0, true);
    LLVMValueRef loop_cond = LLVMBuildICmp(llvm_builder, LLVMIntNE, exp.llvm_value, zero_val, "");
    exp.llvm_value = loop_cond;

    LLVMBuildCondBr(llvm_builder, loop_cond, loop_body_block, loop_merge_block);

    // Loop body
    LLVMPositionBuilderAtEnd(llvm_builder, loop_body_block);

    if (!statement_list(parser))
    {
        return false;
    }

    // Go back to the header to check the condition
    // Merge else block into merge if there wasn't a return

    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(llvm_builder)) == NULL)
    {
        LLVMBuildBr(llvm_builder, loop_header_block);
    }

    LLVMPositionBuilderAtEnd(llvm_builder, loop_merge_block);


    if (!parser_eat(parser, K_END))
    {
        throw_error("Missing \'end\' in loop\n", parser->look_ahead);
        return false;
    }

    if (!parser_eat(parser, K_FOR))
    {
        throw_error("Missing closing \'for\' in loop\n", parser->look_ahead);
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
        throw_error("Return statements must be within a procedure.\n", parser->look_ahead);
        return false;
    }
    else if (!type_checking(parser, &proc, &exp))
    {
        return false;
    }

    // Code gen: Return statement
    LLVMBuildRet(llvm_builder, exp.llvm_value);
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
        if (exp->type == TC_BOOL || exp->type == TC_INT)
        {
            exp->llvm_value = LLVMBuildNot(llvm_builder, exp->llvm_value, "");
        }
        else
        {
            throw_error("!= operator is defined for bool and int only.\n", parser->look_ahead);
            return false;
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
        throw_error("Missing operand\n", parser->look_ahead);
        return false;
    }

    // Type checking and convert type for 'and', 'or' operators
    expression_type_checking(parser, exp, &rhs, parser->look_ahead);

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
bool arith_op_prime(parser_T* parser, Symbol* ar_op)
{
    Token op = *parser->look_ahead;
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
        throw_error("Missing operand\n", parser->look_ahead);
        return false;
    }

    // Type checking to convert type for + -
    if (!arithmetic_type_checking(parser, ar_op, &rhs, &op))
    {
        return false;
    }

    if (!arith_op_prime(parser, ar_op))
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
        throw_error("Missing operand.\n", parser->look_ahead);
        return false;
    }

    // Type checking to convert type for relational operators
    if (!relation_type_checking(parser, rel, &rhs, &op))
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
    Token op = *parser->look_ahead;
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
        throw_error("Missing operand\n", parser->look_ahead);
        return false;
    }

    // Type checking to convert type for * / operation
    if (!arithmetic_type_checking(parser, tm, &rhs, &op))
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
    if (is_token_type(parser, T_LPAREN))
    {
        parser_eat(parser, T_LPAREN);
        if (!expression(parser, fac))
        {
            return false;
        }

        if (!parser_eat(parser, T_RPAREN))
        {
            throw_error("Missing \')\' in expression factor\n", parser->look_ahead);
            return false;
        }
        return true;
    }
    else if (procedure_call_or_name_handler(parser, fac))
    {
        // nothing
    }
    else if (is_token_type(parser, T_MINUS))
    {
        parser_eat(parser, T_MINUS);
        if (name(parser, fac) || number(parser, fac))
        {
            // Codegen: negative value
            if (fac->type == TC_INT)
            {
                fac->llvm_value = LLVMBuildNeg(llvm_builder, fac->llvm_value, "");
            }
            else if (fac->type == TC_FLOAT)
            {
                fac->llvm_value = LLVMBuildFNeg(llvm_builder, fac->llvm_value, "");
            }
            else
            {
                throw_error("Minus operator only valid on integers or floats\n", parser->look_ahead);
                return false;
            }
        }
        else
        {
            throw_error("Invalid use of minus operator\n", parser->look_ahead);
            return false;
        }
    }
    else if (number(parser, fac))
    {

    }
    else if (string(parser, fac))
    {

    }
    else if (is_token_type(parser, K_TRUE))
    {
        parser_eat(parser, K_TRUE);
        fac->ttype = K_TRUE;
        fac->type = TC_BOOL;
        fac->llvm_value = LLVMConstInt(int1_type, 1, 1);
    }
    else if (is_token_type(parser, K_FALSE))
    {
        parser_eat(parser, K_FALSE);
        fac->ttype = K_FALSE;
        fac->type = TC_BOOL;
        fac->llvm_value = LLVMConstInt(int1_type, 0, 1);
    }
    else {
        return false;
    }
    return true;
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
        throw_error(concatf("Identifier \'%s\' is not declared in local or global scope.\n", id->id), parser->look_ahead);
        return false;
    }

    // Get Identifier from local or global
    *id = get_current_global_symbol(parser->sem, id->id, id->is_global);

    if (is_token_type(parser, T_LPAREN))
    {
        parser_eat(parser, T_LPAREN);

        // Confirmation that it's a procedure
        if (id->stype != ST_PROCEDURE)
        {
            throw_error(concatf("\'%s\' is not a procedure, and cannot be called.\n", id->id), parser->look_ahead);
            return false;
        }

        // Optional argument
        LLVMValueRef *args = NULL;
        args = argument_list(parser, id);
        if (error_flag)
        {
            return false;
        }

        if (!parser_eat(parser, T_RPAREN))
        {
            throw_error("Missing \')\' in procedure call.\n", parser->look_ahead);
            return false;
        }

        // Codegen: Procedure call
        id->llvm_value = LLVMBuildCall(llvm_builder, id->llvm_function, args, params_size(id), "");

    }
    else
    {
        // Confirm that it's a name
        if (id->stype != ST_VARIABLE)
        {
            throw_error(concatf("\'%s\' is not a variable.\n", id->id), parser->look_ahead);
            return false;
        }

        // Optional array index
        Symbol ind = *init_symbol();
        if (!array_index(parser, id, &ind))
        {
            return false;
        }

        if (!name_code_gen(parser, id, &ind))
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
        throw_error(concatf("Identifier \'%s\' is not declared in local or global scope.\n", id->id), parser->look_ahead);
        return false;
    }

    // Get Id
    *id = get_current_global_symbol(parser->sem, id->id, id->is_global);

    // Confirm that it is a name
    if (id->stype != ST_VARIABLE)
    {
        throw_error(concatf("\'%s\' is not a variable.\n", id->id), parser->look_ahead);
        return false;
    }

    Symbol ind = *init_symbol();
    if (!array_index(parser, id, &ind))
    {
        return false;
    }

    if (!name_code_gen(parser, id, &ind))
    {
        return false;
    }
    return true;
}

/*
 * Handler for array index [ [ <expression> ] ]
 */
bool array_index(parser_T* parser, Symbol* id, Symbol* ind)
{
    if (parser_eat(parser, T_LBRACKET))
    {
        if (!expression(parser, ind))
        {
            return false;
        }

        // Check valid array access
        if (!id->is_arr)
        {
            throw_error(concatf("Identifier \'%s\' is not an array. Invalid array access.\n", id->id), parser->look_ahead);
            return false;
        }
        else if (ind->type != TC_INT)
        {
            throw_error("Array index must be integer.\n", parser->look_ahead);
            return false;
        }

        // Code gen: check 0 <= exp value < arr bound
        LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);
        LLVMValueRef bound_val = LLVMConstInt(int32_type, id->arr_size, true);
        LLVMValueRef lt_bound = LLVMBuildICmp(llvm_builder, LLVMIntSLT, ind->llvm_value, bound_val, "");
        LLVMValueRef gte_zero = LLVMBuildICmp(llvm_builder, LLVMIntSGE, ind->llvm_value, zero_val, "");
        LLVMValueRef cond = LLVMBuildAnd(llvm_builder, lt_bound, gte_zero, "");

        LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;
        LLVMBasicBlockRef bound_err_block = LLVMAppendBasicBlockInContext(llvm_context, func, "boundErr");
        LLVMBasicBlockRef no_err_block = LLVMAppendBasicBlockInContext(llvm_context, func, "noErr");

        // If invalid index, display error and exit
        LLVMBuildCondBr(llvm_builder, cond, no_err_block, bound_err_block);
        LLVMPositionBuilderAtEnd(llvm_builder, bound_err_block);
        LLVMValueRef err_func = get_current_global_symbol(parser->sem, "_outOfBoundsError", true).llvm_function;
        LLVMBuildCall(llvm_builder, err_func, NULL, 0, "");
        // Need a terminator to satisfy LLVM, but it will exit(1) before reaching
        LLVMBuildBr(llvm_builder, no_err_block);
        LLVMPositionBuilderAtEnd(llvm_builder, no_err_block);

        id->is_indexed = true;

        if (!parser_eat(parser, T_RBRACKET))
        {
            throw_error("Missing \']\' in array index access.\n", parser->look_ahead);
            return false;
        }
    }
    return true;
}

bool name_code_gen(parser_T* parser, Symbol* id, Symbol* ind)
{
    if (id->is_arr) {
        if (!id->is_indexed) {
            // Either passing whole array as arg, or doing fancy array assignment
            return true;
        }

        LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);
        
        // Get pointer to the element of the array
        LLVMValueRef indices[] = { zero_val, ind->llvm_value};
        id->llvm_address = LLVMBuildInBoundsGEP(llvm_builder, id->llvm_address, indices, 2, "");
    }
    id->llvm_value = NULL;
    id->llvm_value = LLVMBuildLoad2(llvm_builder, create_llvm_type(id->type), id->llvm_address, "");
    return true;
}

/*
 * <argument_list> ::=
 *      <expression>, <argument_list>
 *    | <expression>
 */
LLVMValueRef* argument_list(parser_T* parser, Symbol* id)
{
    LLVMValueRef *arg_list = (LLVMValueRef *) malloc(sizeof(LLVMValueRef) * params_size(id));
    Symbol arg = *init_symbol();
    int arg_index = 0;

    if (!expression(parser, &arg))
    {
        if (arg_index != params_size(id))
        {
            throw_error(concatf("Too few arguments provided for \'%s\'.\n", id->id), parser->look_ahead);
        }
        return false;
    }

    // Grab parameter symbol at corresponding index
    Symbol sym = get_nth_param(id, arg_index);

    // Check for too much parameters 
    if (arg_index >= params_size(id))
    {
        throw_error(concatf("Too many arguments provivded to \'%s\'.\n", id->id), parser->look_ahead);
        return false;
    }
    // Type checking match parameter type
    else if (!type_checking(parser, &sym, &arg))
    {
        return false;
    }


    int arg_list_idx = 0;
    if (arg.is_arr && !arg.is_indexed)
    {
        // Passing the entire array as arg
        // procedure_body will copy in the values to the local array
        arg_list[arg_list_idx++] = arg.llvm_address;
    }
    else
    {
        arg_list[arg_list_idx++] = arg.llvm_value;
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
            throw_error("Invalid argument.\n", parser->look_ahead);
            return false;
        }

        // Grab parameter symbol at corresponding index
        sym = get_nth_param(id, arg_index);

        // Check for too much parameters 
        if (arg_index >= params_size(id))
        {
            throw_error(concatf("Too many arguments provivded to \'%s\'.\n", id->id), parser->look_ahead);
            return false;
        }
        // Type checking match parameter type
        else if (!type_checking(parser, &sym, &arg))
        {
            return false;
        }

        if (arg.is_arr && !arg.is_indexed)
        {
            // Passing the entire array as arg
            // procedure_body will copy in the values to the local array
            arg_list[arg_list_idx++] = arg.llvm_address;
        }
        else
        {
            arg_list[arg_list_idx++] = arg.llvm_value;
        }

        // Increment count
        arg_index++;
    }

    // Check number of params
    if (arg_index != params_size(id)) {
        throw_error(concatf("Too many arguments provivded to \'%s\'.\n", id->id), parser->look_ahead);
        return false;
    }

    return arg_list;
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
        num->llvm_value = LLVMConstInt(int32_type, parser->look_ahead->value.intVal, true);
        return parser_eat(parser, T_NUMBER_INT);
    }
    else if (is_token_type(parser, T_NUMBER_FLOAT))
    {
        num->type = TC_FLOAT;
        num->ttype = T_NUMBER_FLOAT;
        num->llvm_value =  LLVMConstReal(float_type, parser->look_ahead->value.floatVal);
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
        str->llvm_value = LLVMBuildGlobalStringPtr(llvm_builder, parser->look_ahead->value.stringVal, "");
    }
    // Eat token
    return parser_eat(parser, T_STRING);
}

/* 
 * A list of declaration for ( <declaration> ; )*
 */
bool declaration_list(parser_T* parser)
{
    TokenType tokens[] = { K_BEGIN, T_SEMI_COLON };
    // Zero or more declarations
    while (declaration(parser)) {
        if (!parser_eat(parser, T_SEMI_COLON)) {
            throw_error("Missing \';\' after declaration\n", parser->look_ahead);
            return false;
        }
    }

    if (error_flag && resync(parser, tokens, 2))
    {
        return declaration_list(parser);
    }
    return !error_flag;
}

/*
 * A list of statements for ( <statement> ; )*
 */
bool statement_list(parser_T* parser)
{
    TokenType tokens[] = { K_END, T_SEMI_COLON };
    // Zero or more statements
    while(statement(parser))
    {
        if (!parser_eat(parser, T_SEMI_COLON))
        {
            throw_error("Missing \';\' after statement\n", parser->look_ahead);
            return false;
        }
    }

    if (error_flag && resync(parser, tokens, 2))
    {
        return statement_list(parser);
    }
    return !error_flag;
}

/*
 * Type checking for relational operators < <= > >= == !=
 */
bool relation_type_checking(parser_T* parser, Symbol* lhs, Symbol* rhs, Token* op)
{
    bool compatible = false;
    // If int is present with float or bool, convert int to that type
    // Otherwise types must match exactly

    // Convert integer to float or bool for comparision
    LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);
    if ((lhs->is_arr && !lhs->is_indexed) || (rhs->is_arr && !rhs->is_indexed))
    {
        // Unindexed arrays do op on whole array
        return array_op_type_check(parser, lhs, rhs, op);
    }
    else if (lhs->type == TC_INT)
    {
        if (rhs->type == TC_BOOL)
        {
            compatible = true;
            // Convert to boolean
            lhs->type = TC_BOOL;
            // All non-zero values are true
            lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntNE, lhs->llvm_value, zero_val, "");
        }
        else if (rhs->type == TC_FLOAT)
        {
            compatible = true;
            //Convert to float
            lhs->type = TC_FLOAT;
            lhs->llvm_value = LLVMBuildSIToFP(llvm_builder, lhs->llvm_value, float_type, "");
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
            // Convert rhs to float
            rhs->type = TC_FLOAT;
            rhs->llvm_value = LLVMBuildSIToFP(llvm_builder, rhs->llvm_value, float_type, "");
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
            // All non-zero values are true
            rhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntNE, rhs->llvm_value, zero_val, "");
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
        throw_error("Types are not compatible for relational operations.\n", parser->look_ahead);
        return false;
    }

    // Code generation
    switch (op->type)
    {
        case T_LT:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFCmp(llvm_builder, LLVMRealOLT, lhs->llvm_value, rhs->llvm_value, "");
            }
            else if (lhs->type == TC_BOOL)
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntULT, lhs->llvm_value, rhs->llvm_value, "");
            }
            else // Int
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntSLT, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_LTEQ:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFCmp(llvm_builder, LLVMRealOLE, lhs->llvm_value, rhs->llvm_value, "");
            }
            else if (lhs->type == TC_BOOL)
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntULE, lhs->llvm_value, rhs->llvm_value, "");
            }
            else // Int
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntSLE, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_GT:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFCmp(llvm_builder, LLVMRealOGT, lhs->llvm_value, rhs->llvm_value, "");
            }
            else if (lhs->type == TC_BOOL)
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntUGT, lhs->llvm_value, rhs->llvm_value, "");
            }
            else // Int
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntSGT, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_GTEQ:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFCmp(llvm_builder, LLVMRealOGE, lhs->llvm_value, rhs->llvm_value, "");
            }
            else if (lhs->type == TC_BOOL)
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntUGE, lhs->llvm_value, rhs->llvm_value, "");
            }
            else // Int
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntSGE, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_EQ:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFCmp(llvm_builder, LLVMRealOEQ, lhs->llvm_value, rhs->llvm_value, "");
            }
            else if (lhs->type == TC_STRING)
            {
                lhs->llvm_value = string_comparison(parser, lhs, rhs);
            }
            else // Int or Bool
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntEQ, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_NOT_EQ:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFCmp(llvm_builder, LLVMRealONE, lhs->llvm_value, rhs->llvm_value, "");
            }
            else if (lhs->type == TC_STRING)
            {
                lhs->llvm_value = LLVMBuildNot(llvm_builder, string_comparison(parser, lhs, rhs), "");
            }
            else // Int or Bool
            {
                lhs->llvm_value = LLVMBuildICmp(llvm_builder, LLVMIntNE, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        default:
            throw_error("Invalid relational operator.\n", parser->look_ahead);
            return false;
    }
    return compatible;
}

/*
 * Compare strings character by character. Return LLVM value for whether they are equal.
 */
LLVMValueRef string_comparison(parser_T* parser, Symbol* lhs, Symbol* rhs)
{
    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;

    LLVMBasicBlockRef str_cmp_block = LLVMAppendBasicBlockInContext(llvm_context, func, "strCmp");
    LLVMBasicBlockRef str_cmp_merge_block = LLVMAppendBasicBlockInContext(llvm_context, func, "strCmpMerge");

    // Initial index = 0
    LLVMValueRef ind_addr = LLVMBuildAlloca(llvm_builder, create_llvm_type(TC_INT), "strCmpInd");
    LLVMValueRef index = LLVMConstInt(int32_type, 0, true);
    LLVMBuildStore(llvm_builder, index, ind_addr);

    LLVMBuildBr(llvm_builder, str_cmp_block);
    LLVMPositionBuilderAtEnd(llvm_builder, str_cmp_block);

    index = LLVMBuildLoad2(llvm_builder, create_llvm_type(TC_INT), ind_addr, "");

    // Get element pointer to string character, then load the character
    LLVMValueRef lhs_char_address = LLVMBuildInBoundsGEP(llvm_builder, lhs->llvm_value, &index, 1, "");
    LLVMValueRef rhs_char_address = LLVMBuildInBoundsGEP(llvm_builder, rhs->llvm_value, &index, 1, "");
    LLVMValueRef lhs_char_value = LLVMBuildLoad2(llvm_builder, int8_type, lhs_char_address, "");
    LLVMValueRef rhs_char_value = LLVMBuildLoad2(llvm_builder, int8_type, rhs_char_address, "");

    // Compare lhs == rhs
    LLVMValueRef cmp = LLVMBuildICmp(llvm_builder, LLVMIntEQ, lhs_char_value, rhs_char_value, "");

    // Null terminator char \0
    LLVMValueRef zero_val_8 = LLVMConstInt(int8_type, 0, true);

    // See if one char is null terminator.
    // Ignore the rhs char since if they're unequal it doesn't matter anyway
    LLVMValueRef not_null_term = LLVMBuildICmp(llvm_builder, LLVMIntNE, lhs_char_value, zero_val_8, "");

    // Increment index
    LLVMValueRef increment = LLVMConstInt(int32_type, 1, true);
    index = LLVMBuildAdd(llvm_builder, index, increment, "");
    LLVMBuildStore(llvm_builder, index, ind_addr);

    // Keep checking if not the end And lhs == rhs so far
    LLVMValueRef and_cond = LLVMBuildAdd(llvm_builder, cmp, not_null_term, "");
    LLVMBuildCondBr(llvm_builder, and_cond, str_cmp_block, str_cmp_merge_block);
    LLVMPositionBuilderAtEnd(llvm_builder, str_cmp_merge_block);
    return cmp;
}

/*
 * Type checkign for arithmetic operators + - * /
 */
bool arithmetic_type_checking(parser_T* parser, Symbol* lhs, Symbol* rhs, Token* op)
{
    if ((lhs->type != TC_INT && lhs->type != TC_FLOAT) || (rhs->type != TC_INT && rhs->type != TC_FLOAT))
    {
        throw_error("Arithmetic operators are only for int and float.\n", parser->look_ahead);
        return false;
    }

    if ((lhs->is_arr && !lhs->is_indexed) || (rhs->is_arr && !rhs->is_indexed))
    {
        // Unindexed arrays do op on the whole array
        return array_op_type_check(parser, lhs, rhs, op);
    }
    else if (lhs->type == TC_INT)
    {
        if (rhs->type == TC_FLOAT)
        {
            // Convert lhs to float
            lhs->type = TC_FLOAT;
            lhs->llvm_value = LLVMBuildSIToFP(llvm_builder, lhs->llvm_value, float_type, "");
        }
        // Else, both are int, matched
    }
    else // lhs is float
    {
        if (rhs->type == TC_INT)
        {
            // Convert rhs to float
            rhs->type = TC_FLOAT;
            rhs->llvm_value = LLVMBuildSIToFP(llvm_builder, rhs->llvm_value, float_type, "");
        }
        // Else, both are float, matched
    }

    // Code gen
    switch (op->type)
    {
        case T_PLUS:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFAdd(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            else
            {
                lhs->llvm_value = LLVMBuildAdd(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_MINUS:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFSub(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            else
            {
                lhs->llvm_value = LLVMBuildSub(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_MULTIPLY:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFMul(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            else
            {
                lhs->llvm_value = LLVMBuildMul(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        case T_DIVIDE:
            if (lhs->type == TC_FLOAT)
            {
                lhs->llvm_value = LLVMBuildFDiv(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            else
            {
                lhs->llvm_value = LLVMBuildSDiv(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            }
            break;
        default:
            throw_error("Invalid arithmetic operator.\n", parser->look_ahead);
            return false;
    }

    return true;
}

/*
 * Type checking for expression operators & |
 */
bool expression_type_checking(parser_T* parser, Symbol* lhs, Symbol* rhs, Token* op)
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
        throw_error("Expression operators are defined for bool and int only.\n", parser->look_ahead);
        return false;
    }

    if ((lhs->is_arr && !lhs->is_indexed) || (rhs->is_arr && !rhs->is_indexed))
    {
        // Unindexed arrays do op on whole array
        return array_op_type_check(parser, lhs, rhs, op);
    }

    // Code gen
    switch (op->type)
    {
        case T_AND:
            lhs->llvm_value = LLVMBuildAnd(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            break;
        case T_OR:
            lhs->llvm_value = LLVMBuildOr(llvm_builder, lhs->llvm_value, rhs->llvm_value, "");
            break;
        default:
            throw_error("Invalid expression operator.\n", parser->look_ahead);
            return false;
    }
    return compatible;
}

// Codegen to copy the elements from one array to another
void array_assignment_codegen(parser_T* parser, Symbol* dest, Symbol* exp)
{
    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;

    LLVMBasicBlockRef arr_copy_block = LLVMAppendBasicBlockInContext(llvm_context, func, "arrCopy");
    LLVMBasicBlockRef arr_copy_merge_block = LLVMAppendBasicBlockInContext(llvm_context, func, "arrCopyMerge");

    // Initial index = 0
    LLVMValueRef ind_addr = LLVMBuildAlloca(llvm_builder, create_llvm_type(TC_INT), "arrCopyInd");
    LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);
    LLVMValueRef index = zero_val;
    LLVMBuildStore(llvm_builder, index, ind_addr);

    // Max value of index is arr_size - 1
    LLVMValueRef loop_end = LLVMConstInt(int32_type, dest->arr_size, true);
    LLVMBuildBr(llvm_builder, arr_copy_block);
    LLVMPositionBuilderAtEnd(llvm_builder, arr_copy_block);

    index = LLVMBuildLoad2(llvm_builder, create_llvm_type(TC_INT), ind_addr, "");

    LLVMValueRef indices[] = { zero_val, index };
    // Get pointer to array element, and load the value
    LLVMValueRef exp_elem_addr = LLVMBuildInBoundsGEP(llvm_builder, exp->llvm_address, indices, 2, "");
    LLVMValueRef exp_elem_val = LLVMBuildLoad2(llvm_builder, create_llvm_type(dest->type), exp_elem_addr, "");

    // Get pointer to dest array element, and store the value
    LLVMValueRef dest_elem_addr = LLVMBuildInBoundsGEP(llvm_builder, dest->llvm_address, indices, 2, "");
    LLVMBuildStore(llvm_builder, exp_elem_val, dest_elem_addr);

    // Increment index
    LLVMValueRef increment = LLVMConstInt(int32_type, 1, true);
    index = LLVMBuildAdd(llvm_builder, index, increment, "");
    LLVMBuildStore(llvm_builder, index, ind_addr);

    // index < arr size
    LLVMValueRef cond = LLVMBuildICmp(llvm_builder, LLVMIntSLT, index, loop_end, "");
    LLVMBuildCondBr(llvm_builder, cond, arr_copy_block, arr_copy_merge_block);
    LLVMPositionBuilderAtEnd(llvm_builder, arr_copy_merge_block);
}

/*
 * Type checking for assignment operator
 * making destination and return type matches.
 * Also, matching parameters to arguments 
 */

bool type_checking(parser_T* parser, Symbol* dest, Symbol* exp)
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
                throw_error("Incompatible index match of arrays.\n", parser->look_ahead);
                return false;
            }
            else if (!dest->is_indexed)
            {
                // Both are unindexed. Array lengths must match
                if (dest->arr_size != exp->arr_size)
                {
                    throw_error("Array lengths must match.\n", parser->look_ahead);
                    return false;
                }
            }
            else
            {
                if (dest->type != exp->type)
                {
                    throw_error("Unindexed array types must match each other.\n", parser->look_ahead);
                    return false;
                }
            }
        }
        else
        {
            // One side is array
            // Array must be indexed
            if ((dest->is_arr && !dest->is_indexed) || (exp->is_arr && !exp->is_indexed))
            {
                throw_error("Array is not indexed.\n", parser->look_ahead);
                compatible = false;
            }
        }
    }
    // Both are not arrays

    /*
     * Both have compatible types, convert exp type to dest type
     * int <-> bool
     * int <-> float
     * Otherwise, match exactly
     * 
     * For unindexed arrays, only check if types are compatible, avoid doing conversion
     */
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
                exp->llvm_value = LLVMConstIntCast(exp->llvm_value, int32_type, false);
            }
        }
        else if (exp->type == TC_FLOAT)
        {
            compatible = true;
            if (!(exp->is_arr && !exp->is_indexed))
            {
                // Convert exp to int
                exp->type = TC_INT;
                exp->llvm_value = LLVMConstFPToSI(exp->llvm_value, int32_type);
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
                exp->llvm_value = LLVMConstSIToFP(exp->llvm_value, float_type);
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
                exp->llvm_value = LLVMConstICmp(LLVMIntNE , exp->llvm_value, LLVMConstInt(int32_type, 0, true));
            }
        }
    }


    if (!compatible)
    {
        throw_error(concatf(                           \
            "Incompatible types \'%s\' and \'%s\'.\n", \
            print_type_class(dest->type),              \
            print_type_class(exp->type)                \
        ), parser->look_ahead);                        \
    }

    return compatible;
}

/*
 * Ops done on unindexed arrays affect the whole array
 */
bool array_op_type_check(parser_T* parser, Symbol* lhs, Symbol* rhs, Token* op)
{
    // If both are arrays, size must be the same
    if (lhs->is_arr && !lhs->is_indexed && rhs->is_arr && !rhs->is_indexed && lhs->arr_size != rhs->arr_size)
    {
        throw_error("Operation with unindexed arrays must have the same size.\n", parser->look_ahead);
        return false;
    }

    // Get the correct type for the array
    // No need to check every type matching here.
    // Error will be thrown from type_checking function if invalid matches.
    LLVMTypeRef ty;
    TypeClass output_type;
    switch (op->type)
    {
        case T_PLUS:
        case T_MINUS:
        case T_MULTIPLY:
        case T_DIVIDE:
            if (lhs->type == TC_FLOAT || rhs->type == TC_FLOAT)
            {
                output_type = TC_FLOAT;
                ty = create_llvm_type(TC_FLOAT);
            }
            else
            {
                output_type = TC_INT;
                ty = create_llvm_type(TC_INT);
            }
            break;
        case T_LT:
        case T_LTEQ:
        case T_GT:
        case T_GTEQ:
        case T_EQ:
        case T_NOT_EQ:
            output_type = TC_BOOL;
            ty = create_llvm_type(TC_BOOL);
            break;
        case T_AND:
        case T_OR:
            if (lhs->type == TC_BOOL)
            {
                output_type = TC_BOOL;
                ty = create_llvm_type(TC_BOOL);
            }
            else
            {
                output_type = TC_INT;
                ty = create_llvm_type(TC_INT);
            }
            break;
        default:
            throw_error("Invalid unindexed array operator.\n", parser->look_ahead);
            return false;
    }

    int arr_size = lhs->arr_size;
    if (!(lhs->is_arr && !lhs->is_indexed)) 
    {
        arr_size = rhs->arr_size;
    }

    ty = LLVMArrayType(ty, arr_size);

    // Allocate a new array to store the result
    LLVMValueRef result_arr_address = LLVMBuildAlloca(llvm_builder, ty, "");

    LLVMValueRef func = get_current_procedure(parser->sem).llvm_function;
    LLVMBasicBlockRef arr_op_block = LLVMAppendBasicBlockInContext(llvm_context, func, "arrOp");
    LLVMBasicBlockRef arr_op_merge_block = LLVMAppendBasicBlockInContext(llvm_context, func, "arrOpMerge");

    // Intial index = 0
    LLVMValueRef ind_addr = LLVMBuildAlloca(llvm_builder, create_llvm_type(TC_INT), "arrOpInd");
    LLVMValueRef zero_val = LLVMConstInt(int32_type, 0, true);
    LLVMValueRef index = zero_val;
    LLVMBuildStore(llvm_builder, index, ind_addr);

    // Max value of index is arr_size - 1
    LLVMValueRef loop_end = LLVMConstInt(int32_type, arr_size, true);

    LLVMBuildBr(llvm_builder, arr_op_block);
    LLVMPositionBuilderAtEnd(llvm_builder, arr_op_block);

    index = LLVMBuildLoad2(llvm_builder, create_llvm_type(TC_INT), ind_addr, "");

    // If the operand is an unindexed array, load the element of the current index
    Symbol lhs_elem = *init_symbol_with_id_symbol_type("", lhs->ttype, lhs->stype, lhs->type);
    LLVMValueRef indices[] = { zero_val, index };
    if (lhs->is_arr && !lhs->is_indexed)
    {
        // Get pointer to array element, and load the value
        LLVMValueRef elem_addr = LLVMBuildInBoundsGEP(llvm_builder, lhs->llvm_address, indices, 2, "");
        lhs_elem.llvm_value = LLVMBuildLoad2(llvm_builder, create_llvm_type(lhs->type), elem_addr, "");
    }
    else
    {
        lhs_elem.llvm_value = lhs->llvm_value;
    }

    Symbol rhs_elem = *init_symbol_with_id_symbol_type("", rhs->ttype, rhs->stype, rhs->type);
    if (rhs->is_arr && !rhs->is_indexed)
    {
        // Get pointer to array element, and load the value
        LLVMValueRef elem_addr = LLVMBuildInBoundsGEP(llvm_builder, rhs->llvm_address, indices, 2, "");
        rhs_elem.llvm_value = LLVMBuildLoad2(llvm_builder, create_llvm_type(rhs->type), elem_addr, "");
    }
    else
    {
        rhs_elem.llvm_value = rhs->llvm_value;
    }

    switch (op->type)
    {
        case T_PLUS:
        case T_MINUS:
        case T_MULTIPLY:
        case T_DIVIDE:
            if (!arithmetic_type_checking(parser, &lhs_elem, &rhs_elem, op))
            {
                return false;
            }
            break;
        case T_LT:
        case T_LTEQ:
        case T_GT:
        case T_GTEQ:
        case T_EQ:
        case T_NOT_EQ:
            if (!relation_type_checking(parser, &lhs_elem, &rhs_elem, op))
            {
                return false;
            }
            break;
        case T_AND:
        case T_OR:
            if (!expression_type_checking(parser, &lhs_elem, &rhs_elem, op))
            {
                return false;
            }
            break;
        default:
            throw_error("Invalid unindexed array operator.\n", parser->look_ahead);
            return false;
    }


    // Get pointer to result array element, and store the result of the calculation
    LLVMValueRef elem_addr = LLVMBuildInBoundsGEP(llvm_builder, result_arr_address, indices, 2, "");
    LLVMBuildStore(llvm_builder, lhs_elem.llvm_value, elem_addr);

    // Increment index
    LLVMValueRef increment = LLVMConstInt(int32_type, 1, true);
    index = LLVMBuildAdd(llvm_builder, index, increment, "");
    LLVMBuildStore(llvm_builder, index, ind_addr);

    // if index < array size
    LLVMValueRef cond = LLVMBuildICmp(llvm_builder, LLVMIntSLT, index, loop_end, "");
    LLVMBuildCondBr(llvm_builder, cond, arr_op_block, arr_op_merge_block);

    LLVMPositionBuilderAtEnd(llvm_builder, arr_op_merge_block);

    // Update the result symbol taht will be passed up
    lhs->llvm_address = result_arr_address;
    lhs->is_arr = true;
    lhs->is_indexed = false;
    lhs->arr_size = arr_size;
    lhs->type = output_type;

    return true;
}