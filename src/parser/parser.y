%{
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/ast.h"
#include "../include/compiler.h"
extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern int yylineno;
extern char* yytext;
extern const char* current_input_filename; 
void yyerror(const char* s);
ASTNode* root;

static ASTNode* create_default_value_for_type(ASTNode* type_node, YYLTYPE* loc);
static ASTNode* build_type_alias_enum(const char* type_name, ASTNode* variants);
static ASTNode* mark_type_alias_public(ASTNode* program);
static ASTNode* clone_match_scrutinee(ASTNode* scrutinee);
static ASTNode* build_match_desugared(ASTNode* scrutinee, ASTNode* arms);

static int is_builtin_union_ctor_name(const char* name) {
    if (!name) return 0;
    return strcmp(name, "Some") == 0 || strcmp(name, "None") == 0 ||
           strcmp(name, "Ok") == 0 || strcmp(name, "Err") == 0;
}

static ASTNode* prepend_binding_to_match_body(ASTNode* body, const char* bind_name, ASTNode* scrutinee) {
    if (!body || !bind_name || !scrutinee) return body;

    ASTNode* bind_left = create_identifier_node(bind_name);
    ASTNode* bind_right = clone_match_scrutinee(scrutinee);
    if (!bind_left || !bind_right) return body;

    ASTNode* bind_decl = create_assign_node(bind_left, bind_right);
    if (bind_decl) bind_decl->data.assign.is_declaration = 1;

    ASTNode* wrapped = create_program_node();
    add_statement_to_program(wrapped, bind_decl);

    if (body->type == AST_PROGRAM) {
        for (int i = 0; i < body->data.program.statement_count; i++) {
            add_statement_to_program(wrapped, body->data.program.statements[i]);
        }
    } else {
        add_statement_to_program(wrapped, body);
    }

    return wrapped;
}

typedef enum {
    GENERIC_KIND_FUNCTION = 0,
    GENERIC_KIND_STRUCT = 1,
    GENERIC_KIND_TYPE = 2
} GenericKind;

typedef struct {
    char* name;
    GenericKind kind;
    int arity;
} GenericArityEntry;

static GenericArityEntry g_generic_arities[512];
static int g_generic_arity_count = 0;

static int node_list_count(ASTNode* list) {
    if (!list || list->type != AST_EXPRESSION_LIST) return 0;
    return list->data.expression_list.expression_count;
}

static int find_generic_arity_index(const char* name, GenericKind kind) {
    if (!name) return -1;
    for (int i = 0; i < g_generic_arity_count; i++) {
        if (g_generic_arities[i].kind == kind &&
            g_generic_arities[i].name &&
            strcmp(g_generic_arities[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void register_generic_arity(const char* name, GenericKind kind, int arity) {
    if (!name) return;
    int idx = find_generic_arity_index(name, kind);
    if (idx >= 0) {
        g_generic_arities[idx].arity = arity;
        return;
    }
    if (g_generic_arity_count >= (int)(sizeof(g_generic_arities) / sizeof(g_generic_arities[0]))) {
        return;
    }
    g_generic_arities[g_generic_arity_count].name = strdup(name);
    g_generic_arities[g_generic_arity_count].kind = kind;
    g_generic_arities[g_generic_arity_count].arity = arity;
    g_generic_arity_count++;
}

static int lookup_generic_arity(const char* name, GenericKind kind, int* found) {
    int idx = find_generic_arity_index(name, kind);
    if (idx < 0) {
        if (found) *found = 0;
        return 0;
    }
    if (found) *found = 1;
    return g_generic_arities[idx].arity;
}

static void check_generic_arity_usage(const char* name, GenericKind kind, ASTNode* type_args, YYLTYPE* loc) {
    int found = 0;
    int expected = lookup_generic_arity(name, kind, &found);
    if (!found) return;

    int actual = node_list_count(type_args);
    if (expected == actual) return;

    char msg[256];
    const char* what = (kind == GENERIC_KIND_FUNCTION) ? "function" :
                       (kind == GENERIC_KIND_STRUCT) ? "struct" : "type";
    snprintf(msg, sizeof(msg),
             "Generic %s '%s' expects %d type argument(s), but got %d",
             what, name ? name : "<unknown>", expected, actual);

    int line = (loc && loc->first_line > 0) ? loc->first_line : yylineno;
    int col = (loc && loc->first_column > 0) ? loc->first_column : 1;
    set_location_with_column(current_input_filename ? current_input_filename : "unknown", line, col);
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_SEMANTIC, msg);
}

static ASTNode* create_default_scalar_value(ASTNode* type_node, YYLTYPE* loc) {
    if (!type_node) {
        return create_num_int_node_with_yyltype(0, (void*)loc);
    }

    switch (type_node->type) {
        case AST_TYPE_INT8:
            return create_char_node_with_yyltype(0, (void*)loc);
        case AST_TYPE_INT32:
        case AST_TYPE_INT64:
            return create_num_int_node_with_yyltype(0, (void*)loc);
        case AST_TYPE_FLOAT32:
        case AST_TYPE_FLOAT64:
            return create_num_float_node_with_yyltype(0.0, (void*)loc);
        case AST_TYPE_STRING:
        case AST_TYPE_POINTER:
            return create_nil_node_with_yyltype((void*)loc);
        case AST_IDENTIFIER:
            if (type_node->data.identifier.name) {
                if (strcmp(type_node->data.identifier.name, "char") == 0 ||
                    strcmp(type_node->data.identifier.name, "i8") == 0 ||
                    strcmp(type_node->data.identifier.name, "u8") == 0) {
                    return create_char_node_with_yyltype(0, (void*)loc);
                }
                if (strcmp(type_node->data.identifier.name, "f32") == 0 ||
                    strcmp(type_node->data.identifier.name, "f64") == 0) {
                    return create_num_float_node_with_yyltype(0.0, (void*)loc);
                }
                if (strcmp(type_node->data.identifier.name, "str") == 0 ||
                    strcmp(type_node->data.identifier.name, "ptr") == 0) {
                    return create_nil_node_with_yyltype((void*)loc);
                }
            }
            return create_num_int_node_with_yyltype(0, (void*)loc);
        case AST_TYPE_FIXED_SIZE_LIST:
        case AST_TYPE_LIST:
            return create_default_value_for_type(type_node, loc);
        default:
            return create_num_int_node_with_yyltype(0, (void*)loc);
    }
}

static ASTNode* create_default_value_for_type(ASTNode* type_node, YYLTYPE* loc) {
    if (!type_node) {
        return create_num_int_node_with_yyltype(0, (void*)loc);
    }

    if (type_node->type == AST_TYPE_FIXED_SIZE_LIST) {
        ASTNode* list = create_expression_list_node_with_yyltype((void*)loc);
        long long size = type_node->data.fixed_size_list_type.size;
        ASTNode* elem_type = type_node->data.fixed_size_list_type.element_type;
        if (size < 0) size = 0;
        for (long long i = 0; i < size; i++) {
            add_expression_to_list(list, create_default_scalar_value(elem_type, loc));
        }
        return list;
    }

    if (type_node->type == AST_TYPE_LIST) {
        return create_expression_list_node_with_yyltype((void*)loc);
    }

    return create_default_scalar_value(type_node, loc);
}

static ASTNode* build_type_alias_enum(const char* type_name, ASTNode* variants) {
    (void)type_name;
    ASTNode* program = create_program_node();
    if (!variants || variants->type != AST_EXPRESSION_LIST) {
        return program;
    }

    for (int i = 0; i < variants->data.expression_list.expression_count; i++) {
        ASTNode* variant = variants->data.expression_list.expressions[i];
        if (!variant || variant->type != AST_IDENTIFIER || !variant->data.identifier.name) continue;
        ASTNode* left = create_identifier_node(variant->data.identifier.name);
        ASTNode* right = create_num_int_node(i);
        ASTNode* decl = create_const_node(left, right);
        add_statement_to_program(program, decl);
    }
    return program;
}

static ASTNode* mark_type_alias_public(ASTNode* program) {
    if (!program || program->type != AST_PROGRAM) return program;
    for (int i = 0; i < program->data.program.statement_count; i++) {
        ASTNode* stmt = program->data.program.statements[i];
        if (stmt && stmt->type == AST_CONST) {
            stmt->data.assign.is_public = 1;
        }
    }
    return program;
}

static ASTNode* clone_match_scrutinee(ASTNode* scrutinee) {
    if (!scrutinee) return NULL;
    switch (scrutinee->type) {
        case AST_IDENTIFIER:
            if (scrutinee->data.identifier.name) {
                return create_identifier_node(scrutinee->data.identifier.name);
            }//如果 scrutinee 或 arms 为空，或者 arms 不是表达式列表，返回 NULL
            return NULL;
        case AST_NUM_INT:
            return create_num_int_node(scrutinee->data.num_int.value);
        case AST_NUM_FLOAT:
            return create_num_float_node(scrutinee->data.num_float.value);
        case AST_CHAR:
            return create_char_node(scrutinee->data.character.value);
        case AST_STRING:
            if (scrutinee->data.string.value) {
                return create_string_node(scrutinee->data.string.value);
            }
            return NULL;
        case AST_NIL:
            return create_nil_node();
        default:
            return NULL;
    }
}

static ASTNode* build_match_desugared(ASTNode* scrutinee, ASTNode* arms) {
    if (!scrutinee || !arms || arms->type != AST_EXPRESSION_LIST) return NULL;

    int count = arms->data.expression_list.expression_count;
    ASTNode* chain = NULL;

    for (int i = count - 1; i >= 0; i--) {
        ASTNode* arm = arms->data.expression_list.expressions[i];
        if (!arm || arm->type != AST_ASSIGN || !arm->data.assign.left || !arm->data.assign.right) continue;

        ASTNode* pattern = arm->data.assign.left;
        ASTNode* body = arm->data.assign.right;
        if (pattern && pattern->type == AST_IDENTIFIER && pattern->data.identifier.name &&
            strcmp(pattern->data.identifier.name, "_") == 0) {
            if (i != count - 1) {
                int line = pattern->location.first_line > 0 ? pattern->location.first_line : yylineno;
                int col = pattern->location.first_column > 0 ? pattern->location.first_column : 1;
                set_location_with_column(current_input_filename ? current_input_filename : "unknown", line, col);
                report_simple_error(ERROR_LEVEL_WARNING, ERROR_WARNING,
                    "'_' match arm should be the last arm!");
            }
            chain = body;
            continue;
        }

        ASTNode* cond = NULL;

        if (pattern->type == AST_CALL && pattern->data.call.func &&
            pattern->data.call.func->type == AST_IDENTIFIER &&
            pattern->data.call.func->data.identifier.name) {
            const char* ctor_name = pattern->data.call.func->data.identifier.name;
            ASTNode* cond_left = clone_match_scrutinee(scrutinee);
            if (!cond_left) continue;

            if (pattern->data.call.args && pattern->data.call.args->type == AST_EXPRESSION_LIST &&
                pattern->data.call.args->data.expression_list.expression_count == 1) {
                ASTNode* bind_arg = pattern->data.call.args->data.expression_list.expressions[0];
                if (bind_arg && bind_arg->type == AST_IDENTIFIER && bind_arg->data.identifier.name) {
                    body = prepend_binding_to_match_body(body, bind_arg->data.identifier.name, scrutinee);
                }

                /* Payload constructor pattern: treat as non-zero for Option/Result-like values. */
                ASTNode* zero = create_num_int_node(0);
                cond = create_binop_node(OP_NE, cond_left, zero);
            } else {
                ASTNode* cond_right = create_identifier_node(ctor_name);
                if (is_builtin_union_ctor_name(ctor_name) && strcmp(ctor_name, "None") == 0) {
                    cond_right = create_num_int_node(0);
                }
                cond = create_binop_node(OP_EQ, cond_left, cond_right);
            }
        } else {
            ASTNode* cond_left = clone_match_scrutinee(scrutinee);//克隆 scrutinee 以构建条件表达式，确保不修改原始 scrutinee
            ASTNode* cond_right = clone_match_scrutinee(pattern);//克隆模式以构建条件表达式，确保不修改原始模式
            if (!cond_right && pattern->type == AST_IDENTIFIER && pattern->data.identifier.name) {
                cond_right = create_identifier_node(pattern->data.identifier.name);//如果模式是一个标识符但无法克隆，直接创建一个新的标识符节点
                if (is_builtin_union_ctor_name(pattern->data.identifier.name) &&
                    strcmp(pattern->data.identifier.name, "None") == 0) {
                    cond_right = create_num_int_node(0);
                }
            }

            if (!cond_left || !cond_right) {
                continue;
            }

            cond = create_binop_node(OP_EQ, cond_left, cond_right);//构建条件表达式：scrutinee == pattern
        }

        if (!cond) continue;
        chain = create_if_node(cond, body, chain);//构建 if-else 链：if (xxxx == xxxxx) { body } else do_something_e
    }

    return chain;
}
/*
build_type_alias_enum：将枚举类型转换为常量定义
clone_match_scrutinee：克隆 match 语句的 scrutinee，确保在生成条件表达式时不会修改原始 scrutinee
build_match_desugared：将 match 表达式转换为嵌套的 ifelse 表达式
*/
%}

%union {
    long long num_int;
    double num_float;
    char* str;
    struct ASTNode* node;
}

%define lr.type ielr

%token <str> IDENTIFIER STRING
%token STRUCT COLON
%token TYPE_KW MATCH PIPE
%token QUESTION
%token CONST LET MUT GLOBAL
%token IMPORT PUB
%token <num_int> NUMBER_INT CHAR_LITERAL
%token <num_float> NUMBER_FLOAT
%token PRINT INPUT TOINT TOFLOAT TYPE_I32 TYPE_I64 TYPE_I8 TYPE_F32 TYPE_F64 TYPE_STR TYPE_PTR FN ARROW RETURN TYPE_VOID NIL EXTERN DOTDOTDOT
%token AND OR
%token AT AMPERSAND
%token IF ELSE ELIF WHILE FOR BREAK CONTINUE IN
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN MODULO_ASSIGN
%token PLUS MINUS MULTIPLY DIVIDE MODULO POWER
%token EQ NE LT LE GT GE
%token DOT DOTDOT LBRACKET RBRACKET
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA
%token ERROR
%left OR
%left AND
%nonassoc EQ NE LT LE GT GE
%left PLUS MINUS
%left MULTIPLY DIVIDE MODULO
%right POWER

%type <node> program statement_list statement 
%type <node> extern_block extern_decl extern_decl_list
%type <node> struct_fields struct_field struct_init_fields struct_init_field
%type <node> type param_list function_definition pub_function_definition function_return_type
%type <node> print_statement assignment_statement compound_assignment_statement
%type <node> if_statement while_statement for_statement
%type <node> expression logical_expression comparison_expression additive_expression term factor power factor_unary
%type <node> literal identifier toint_expression tofloat_expression input_expression
%type <node> block_statement if_rest expression_list
%type <node> lvalue
%type <node> type_definition enum_variant_list match_statement match_arms match_arm match_arm_body match_target match_arm_pattern
%type <node> generic_param_list generic_type_args enum_variant
%type <node> type_list

%nonassoc IF
%nonassoc ELSE

%start program

%%

program
    : statement_list { root = $$ = $1; }
    ;

toint_expression
    : TOINT LPAREN expression RPAREN { $$ = create_toint_node_with_yyltype($3, (YYLTYPE*) &@$); }
    ;

tofloat_expression
    : TOFLOAT LPAREN expression RPAREN { $$ = create_tofloat_node_with_yyltype($3, (YYLTYPE*) &@$); }
    ;

statement_list
    : statement                  { 
                                   $$ = create_program_node_with_yyltype((YYLTYPE*) &@$);
                                   add_statement_to_program($$, $1);
                               }
    | statement_list statement   {
                                   add_statement_to_program($1, $2);
                                   $$ = $1;
                               }
    ;

statement
    : print_statement SEMICOLON     { $$ = $1; }
    | assignment_statement SEMICOLON { $$ = $1; }
    | LET identifier ASSIGN expression SEMICOLON {
        $$ = create_assign_node_with_yyltype($2, $4, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET identifier ASSIGN expression COLON type SEMICOLON {
        $$ = create_assign_node_with_yyltype($2, $4, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET MUT identifier ASSIGN expression SEMICOLON {
        $$ = create_assign_node_with_yyltype($3, $5, (YYLTYPE*) &@$);
        $3->mutability = MUTABILITY_MUTABLE;
        $$->data.assign.is_declaration = 1;
    }
    | LET identifier COLON type ASSIGN expression SEMICOLON {
        $$ = create_assign_node_with_yyltype($2, $6, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET identifier COLON type SEMICOLON {
        ASTNode* init = create_default_value_for_type($4, (YYLTYPE*) &@$);
        $$ = create_assign_node_with_yyltype($2, init, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET MUT identifier COLON type ASSIGN expression SEMICOLON {
        $$ = create_assign_node_with_yyltype($3, $7, (YYLTYPE*) &@$);
        $3->mutability = MUTABILITY_MUTABLE;
        $$->data.assign.is_declaration = 1;
    }
    | LET MUT identifier COLON type SEMICOLON {
        ASTNode* init = create_default_value_for_type($5, (YYLTYPE*) &@$);
        $$ = create_assign_node_with_yyltype($3, init, (YYLTYPE*) &@$);
        $3->mutability = MUTABILITY_MUTABLE;
        $$->data.assign.is_declaration = 1;
    }
    | identifier COLON expression SEMICOLON { $$ = create_assign_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    | identifier COLON type ASSIGN expression SEMICOLON { 
        $$ = create_assign_node_with_yyltype($1, $5, (YYLTYPE*) &@$); 
    }
    | MUT identifier ASSIGN expression SEMICOLON { 
        $$ = create_assign_node_with_yyltype($2, $4, (YYLTYPE*) &@$); 
        $2->mutability = MUTABILITY_MUTABLE;
    }
    | MUT identifier COLON type ASSIGN expression SEMICOLON { 
        $$ = create_assign_node_with_yyltype($2, $6, (YYLTYPE*) &@$); 
        $2->mutability = MUTABILITY_MUTABLE;
    }
    | IMPORT STRING SEMICOLON { $$ = create_import_node_with_yyltype($2, (YYLTYPE*) &@$); }
    | IMPORT STRING { $$ = create_import_node_with_yyltype($2, (YYLTYPE*) &@$); }
    | CONST identifier ASSIGN expression SEMICOLON { $$ = create_const_node_with_yyltype($2, $4, (YYLTYPE*) &@$); }
    | CONST identifier COLON type ASSIGN expression SEMICOLON { $$ = create_const_node_with_yyltype($2, $6, (YYLTYPE*) &@$); }
    | LET CONST identifier ASSIGN expression SEMICOLON { $$ = create_const_node_with_yyltype($3, $5, (YYLTYPE*) &@$); }
    | LET CONST identifier COLON type ASSIGN expression SEMICOLON { $$ = create_const_node_with_yyltype($3, $7, (YYLTYPE*) &@$); }
    | GLOBAL identifier ASSIGN expression SEMICOLON { $$ = create_global_node_with_yyltype($2, NULL, $4, (YYLTYPE*) &@$); }
    | GLOBAL identifier COLON type ASSIGN expression SEMICOLON { $$ = create_global_node_with_yyltype($2, $4, $6, (YYLTYPE*) &@$); }
    | PUB GLOBAL identifier ASSIGN expression SEMICOLON {
        $$ = create_global_node_with_yyltype($3, NULL, $5, (YYLTYPE*) &@$);
        $$->data.global_decl.is_public = 1;
    }
    | PUB GLOBAL identifier COLON type ASSIGN expression SEMICOLON {
        $$ = create_global_node_with_yyltype($3, $5, $7, (YYLTYPE*) &@$);
        $$->data.global_decl.is_public = 1;
    }
    | compound_assignment_statement SEMICOLON { $$ = $1; }
    | if_statement                  { $$ = $1; }
    | while_statement               { $$ = $1; }
    | for_statement               { $$ = $1; }
    | print_statement               { $$ = $1; }
    | assignment_statement          { $$ = $1; }
    | LET identifier ASSIGN expression {
        $$ = create_assign_node_with_yyltype($2, $4, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET identifier ASSIGN expression COLON type {
        $$ = create_assign_node_with_yyltype($2, $4, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET MUT identifier ASSIGN expression {
        $$ = create_assign_node_with_yyltype($3, $5, (YYLTYPE*) &@$);
        $3->mutability = MUTABILITY_MUTABLE;
        $$->data.assign.is_declaration = 1;
    }
    | LET identifier COLON type ASSIGN expression {
        $$ = create_assign_node_with_yyltype($2, $6, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET identifier COLON type {
        ASTNode* init = create_default_value_for_type($4, (YYLTYPE*) &@$);
        $$ = create_assign_node_with_yyltype($2, init, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 1;
    }
    | LET MUT identifier COLON type ASSIGN expression {
        $$ = create_assign_node_with_yyltype($3, $7, (YYLTYPE*) &@$);
        $3->mutability = MUTABILITY_MUTABLE;
        $$->data.assign.is_declaration = 1;
    }
    | LET MUT identifier COLON type {
        ASTNode* init = create_default_value_for_type($5, (YYLTYPE*) &@$);
        $$ = create_assign_node_with_yyltype($3, init, (YYLTYPE*) &@$);
        $3->mutability = MUTABILITY_MUTABLE;
        $$->data.assign.is_declaration = 1;
    }
    | identifier COLON expression { $$ = create_assign_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    | identifier COLON type ASSIGN expression { 
        $$ = create_assign_node_with_yyltype($1, $5, (YYLTYPE*) &@$); 
    }
    | compound_assignment_statement { $$ = $1; }
    | pub_function_definition           { $$ = $1; }
    | function_definition           { $$ = $1; }
    | extern_block                 { $$ = $1; }
    | STRUCT IDENTIFIER LBRACE struct_fields RBRACE {
        register_generic_arity($2, GENERIC_KIND_STRUCT, 0);
        $$ = create_struct_def_node_with_yyltype($2, $4, (YYLTYPE*) &@$);
    }
    | STRUCT IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LBRACE struct_fields RBRACE {
        register_generic_arity($2, GENERIC_KIND_STRUCT, node_list_count($5));
        $$ = create_struct_def_node_with_yyltype($2, $8, (YYLTYPE*) &@$);
    }
    | PUB STRUCT IDENTIFIER LBRACE struct_fields RBRACE {
        register_generic_arity($3, GENERIC_KIND_STRUCT, 0);
        $$ = create_struct_def_node_with_yyltype($3, $5, (YYLTYPE*) &@$);
        $$->data.struct_def.is_public = 1;
    }
    | PUB STRUCT IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LBRACE struct_fields RBRACE {
        register_generic_arity($3, GENERIC_KIND_STRUCT, node_list_count($6));
        $$ = create_struct_def_node_with_yyltype($3, $9, (YYLTYPE*) &@$);
        $$->data.struct_def.is_public = 1;
    }
    | type_definition              { $$ = $1; }
    | match_statement              { $$ = $1; }
    | RETURN expression SEMICOLON  { $$ = create_return_node_with_yyltype($2, (YYLTYPE*) &@$); }
    | RETURN expression            { $$ = create_return_node_with_yyltype($2, (YYLTYPE*) &@$); }
    | RETURN SEMICOLON             { $$ = create_return_node_with_yyltype(NULL, (YYLTYPE*) &@$); }
    | RETURN                       { $$ = create_return_node_with_yyltype(NULL, (YYLTYPE*) &@$); }
    | expression SEMICOLON         { $$ = $1; }
    | expression                   { $$ = $1; }
    | CONST identifier ASSIGN expression { $$ = create_const_node_with_yyltype($2, $4, (YYLTYPE*) &@$); }
    | CONST identifier COLON type ASSIGN expression { $$ = create_const_node_with_yyltype($2, $6, (YYLTYPE*) &@$); }
    | LET CONST identifier ASSIGN expression { $$ = create_const_node_with_yyltype($3, $5, (YYLTYPE*) &@$); }
    | LET CONST identifier COLON type ASSIGN expression { $$ = create_const_node_with_yyltype($3, $7, (YYLTYPE*) &@$); }
    | GLOBAL identifier ASSIGN expression { $$ = create_global_node_with_yyltype($2, NULL, $4, (YYLTYPE*) &@$); }
    | GLOBAL identifier COLON type ASSIGN expression { $$ = create_global_node_with_yyltype($2, $4, $6, (YYLTYPE*) &@$); }
    | PUB GLOBAL identifier ASSIGN expression {
        $$ = create_global_node_with_yyltype($3, NULL, $5, (YYLTYPE*) &@$);
        $$->data.global_decl.is_public = 1;
    }
    | PUB GLOBAL identifier COLON type ASSIGN expression {
        $$ = create_global_node_with_yyltype($3, $5, $7, (YYLTYPE*) &@$);
        $$->data.global_decl.is_public = 1;
    }
    | identifier COLON expression { $$ = create_assign_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    | MUT identifier ASSIGN expression { 
        $$ = create_assign_node_with_yyltype($2, $4, (YYLTYPE*) &@$); 
        $2->mutability = MUTABILITY_MUTABLE;
    }
    | MUT identifier COLON type ASSIGN expression { 
        $$ = create_assign_node_with_yyltype($2, $6, (YYLTYPE*) &@$); 
        $2->mutability = MUTABILITY_MUTABLE;
    }
    | BREAK SEMICOLON               { $$ = create_break_node_with_yyltype((YYLTYPE*) &@$); }
    | CONTINUE SEMICOLON            { $$ = create_continue_node_with_yyltype((YYLTYPE*) &@$); }
    | BREAK                         { $$ = create_break_node_with_yyltype((YYLTYPE*) &@$); }
    | CONTINUE                      { $$ = create_continue_node_with_yyltype((YYLTYPE*) &@$); }
    ;

type_definition
    : TYPE_KW IDENTIFIER ASSIGN enum_variant_list {
        register_generic_arity($2, GENERIC_KIND_TYPE, 0);
        $$ = build_type_alias_enum($2, $4);
    }
    | PUB TYPE_KW IDENTIFIER ASSIGN enum_variant_list {
        register_generic_arity($3, GENERIC_KIND_TYPE, 0);
        $$ = mark_type_alias_public(build_type_alias_enum($3, $5));
    }
    | TYPE_KW IDENTIFIER COLON LBRACKET generic_param_list RBRACKET ASSIGN enum_variant_list {
        register_generic_arity($2, GENERIC_KIND_TYPE, node_list_count($5));
        $$ = build_type_alias_enum($2, $8);
    }
    | PUB TYPE_KW IDENTIFIER COLON LBRACKET generic_param_list RBRACKET ASSIGN enum_variant_list {
        register_generic_arity($3, GENERIC_KIND_TYPE, node_list_count($6));
        $$ = mark_type_alias_public(build_type_alias_enum($3, $9));
    }
    ;

enum_variant_list
    : enum_variant {
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(list, $1);
        $$ = list;
    }
    | enum_variant_list PIPE enum_variant {
        add_expression_to_list($1, $3);
        $$ = $1;
    }
    ;

enum_variant
    : IDENTIFIER {
        $$ = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
    }
    | IDENTIFIER LPAREN type RPAREN {
        $$ = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
    }
    ;

generic_param_list
    : IDENTIFIER {
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(list, create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$));
        $$ = list;
    }
    | generic_param_list COMMA IDENTIFIER {
        add_expression_to_list($1, create_identifier_node_with_yyltype($3, (YYLTYPE*) &@$));
        $$ = $1;
    }
    ;

generic_type_args
    : type {
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(list, $1);
        $$ = list;
    }
    | generic_type_args COMMA type {
        add_expression_to_list($1, $3);
        $$ = $1;
    }
    ;

match_statement
    : MATCH match_target LBRACE match_arms RBRACE {
        $$ = build_match_desugared($2, $4);
    }
    ;

match_target
    : identifier { $$ = $1; }
    | literal { $$ = $1; }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

match_arms
    : match_arm {
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(list, $1);
        $$ = list;
    }
    | match_arms match_arm {
        add_expression_to_list($1, $2);
        $$ = $1;
    }
    ;

match_arm
    : match_arm_pattern ARROW match_arm_body {
        $$ = create_assign_node_with_yyltype($1, $3, (YYLTYPE*) &@$);
    }
    ;

match_arm_pattern
    : identifier { $$ = $1; }
    | literal { $$ = $1; }
    | IDENTIFIER LPAREN IDENTIFIER RPAREN {
        ASTNode* ctor = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
        ASTNode* args = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        ASTNode* bind_id = create_identifier_node_with_yyltype($3, (YYLTYPE*) &@$);
        add_expression_to_list(args, bind_id);
        $$ = create_call_node_with_yyltype(ctor, args, (YYLTYPE*) &@$);
    }
    ;

match_arm_body
    : block_statement { $$ = $1; }
    ;

input_expression
    : INPUT LPAREN STRING RPAREN { 
        ASTNode* prompt = create_string_node_with_yyltype($3, (YYLTYPE*) &@$);
        $$ = create_input_node_with_yyltype(prompt, (YYLTYPE*) &@$); 
    }
    | INPUT LPAREN RPAREN { 
        ASTNode* prompt = create_string_node_with_yyltype("", (YYLTYPE*) &@$);
        $$ = create_input_node_with_yyltype(prompt, (YYLTYPE*) &@$); 
    }
    ;
struct_field
    : IDENTIFIER COLON type {
        $$ = create_assign_node_with_yyltype(create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$), $3, (YYLTYPE*) &@$);
        $$->data.assign.is_declaration = 2;
    }
    ;

struct_fields
    : /* empty */ { $$ = create_expression_list_node_with_yyltype((YYLTYPE*) &@$); }
    | struct_field { ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$); add_expression_to_list(list, $1); $$ = list; }
    | struct_fields COMMA struct_field { add_expression_to_list($1, $3); $$ = $1; }
    | struct_fields struct_field { add_expression_to_list($1, $2); $$ = $1; }
    ;
struct_init_field
    : IDENTIFIER COLON expression { $$ = create_assign_node_with_yyltype(create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$), $3, (YYLTYPE*) &@$); }
    ;

struct_init_fields
    : /* empty */ { $$ = create_expression_list_node_with_yyltype((YYLTYPE*) &@$); }
    | struct_init_field { ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$); add_expression_to_list(list, $1); $$ = list; }
    | struct_init_fields COMMA struct_init_field { add_expression_to_list($1, $3); $$ = $1; }
    | struct_init_fields struct_init_field { add_expression_to_list($1, $2); $$ = $1; }
    ;

type_list
    : type {
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(list, $1);
        $$ = list;
    }
    | type_list COMMA type {
        add_expression_to_list($1, $3);
        $$ = $1;
    }
    ;

type
    : TYPE_I32 { $$ = create_type_node(AST_TYPE_INT32); }
    | TYPE_I64 { $$ = create_type_node(AST_TYPE_INT64); }
    | TYPE_I8 { $$ = create_type_node(AST_TYPE_INT8); }
    | TYPE_F32 { $$ = create_type_node(AST_TYPE_FLOAT32); }
    | TYPE_F64 { $$ = create_type_node(AST_TYPE_FLOAT64); }
    | TYPE_STR { $$ = create_type_node(AST_TYPE_STRING); }
    | TYPE_VOID { $$ = create_type_node(AST_TYPE_VOID); }
    | TYPE_PTR LPAREN type RPAREN { $$ = create_type_node(AST_TYPE_POINTER); }
    | AMPERSAND TYPE_I32 { $$ = create_type_node(AST_TYPE_POINTER); }
    | AMPERSAND TYPE_I64 { $$ = create_type_node(AST_TYPE_POINTER); }
    | AMPERSAND TYPE_I8 { $$ = create_type_node(AST_TYPE_POINTER); }
    | AMPERSAND TYPE_F32 { $$ = create_type_node(AST_TYPE_POINTER); }
    | AMPERSAND TYPE_F64 { $$ = create_type_node(AST_TYPE_POINTER); }
    | AMPERSAND TYPE_STR { $$ = create_type_node(AST_TYPE_POINTER); }
    | AMPERSAND IDENTIFIER { $$ = create_type_node(AST_TYPE_POINTER); }
    | LBRACKET type MULTIPLY NUMBER_INT RBRACKET { $$ = create_fixed_size_list_type_node($2, $4); }
    | LBRACKET type RBRACKET { $$ = create_list_type_node($2); }
    | QUESTION type { $$ = create_type_node(AST_TYPE_POINTER); }
    | FN LPAREN RPAREN COLON type { $$ = create_type_node(AST_TYPE_POINTER); }
    | FN LPAREN type_list RPAREN COLON type { $$ = create_type_node(AST_TYPE_POINTER); }
    | LPAREN type_list RPAREN { $$ = create_type_node(AST_TYPE_POINTER); }
    | IDENTIFIER {
        $$ = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$); 
    }
    | IDENTIFIER LBRACKET generic_type_args RBRACKET {
        check_generic_arity_usage($1, GENERIC_KIND_STRUCT, $3, (YYLTYPE*) &@$);
        check_generic_arity_usage($1, GENERIC_KIND_TYPE, $3, (YYLTYPE*) &@$);
        $$ = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
    }
    | IDENTIFIER COLON LBRACKET generic_type_args RBRACKET {
        check_generic_arity_usage($1, GENERIC_KIND_STRUCT, $4, (YYLTYPE*) &@$);
        check_generic_arity_usage($1, GENERIC_KIND_TYPE, $4, (YYLTYPE*) &@$);
        $$ = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
    }
    ;

param_list
    : identifier { ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$); add_expression_to_list(list, $1); $$ = list; }
    | identifier COLON type {
        ASTNode* id_node = create_identifier_node_with_yyltype($1->data.identifier.name, (YYLTYPE*) &@$);
        ASTNode* annotated_param = create_assign_node_with_yyltype(id_node, $3, (YYLTYPE*) &@$);
        annotated_param->data.assign.is_declaration = 2;
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(list, annotated_param);
        $$ = list;
    }
    | MUT identifier {
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        ASTNode* id_node = create_identifier_node_with_yyltype($2->data.identifier.name, (YYLTYPE*) &@$);
        add_expression_to_list(list, id_node);
        $$ = list;
    }
    | MUT identifier COLON type {
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        ASTNode* id_node = create_identifier_node_with_yyltype($2->data.identifier.name, (YYLTYPE*) &@$);
        ASTNode* annotated_param = create_assign_node_with_mutability(id_node, $4, MUTABILITY_MUTABLE);
        annotated_param->data.assign.is_declaration = 2;
        add_expression_to_list(list, annotated_param);
        $$ = list;
    }
    | param_list COMMA identifier { add_expression_to_list($1, $3); $$ = $1; }
    | param_list COMMA identifier COLON type {
        ASTNode* id_node = create_identifier_node_with_yyltype($3->data.identifier.name, (YYLTYPE*) &@$);
        ASTNode* annotated_param = create_assign_node_with_yyltype(id_node, $5, (YYLTYPE*) &@$);
        annotated_param->data.assign.is_declaration = 2;
        add_expression_to_list($1, annotated_param);
        $$ = $1;
    }
    | param_list COMMA MUT identifier {
        ASTNode* id_node = create_identifier_node_with_yyltype($4->data.identifier.name, (YYLTYPE*) &@$);
        add_expression_to_list($1, id_node);
        $$ = $1;
    }
    | param_list COMMA MUT identifier COLON type {
        ASTNode* id_node = create_identifier_node_with_yyltype($4->data.identifier.name, (YYLTYPE*) &@$);
        ASTNode* annotated_param = create_assign_node_with_mutability(id_node, $6, MUTABILITY_MUTABLE);
        annotated_param->data.assign.is_declaration = 2;
        add_expression_to_list($1, annotated_param);
        $$ = $1;
    }
    ;

function_return_type
    : ARROW type { $$ = $2; }
    | COLON type { $$ = $2; }
    ;

pub_function_definition
    : PUB FN IDENTIFIER LPAREN RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($3, GENERIC_KIND_FUNCTION, 0);
        $$ = create_public_function_node($3, NULL, $6, $8);
    }
    | PUB FN IDENTIFIER LPAREN param_list RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($3, GENERIC_KIND_FUNCTION, 0);
        $$ = create_public_function_node($3, $5, $7, $9);
    }
    | PUB FN IDENTIFIER LPAREN RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($3, GENERIC_KIND_FUNCTION, 0);
        $$ = create_public_function_node($3, NULL, void_type, $7);
    }
    | PUB FN IDENTIFIER LPAREN param_list RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($3, GENERIC_KIND_FUNCTION, 0);
        $$ = create_public_function_node($3, $5, void_type, $8);
    }
    | PUB FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($3, GENERIC_KIND_FUNCTION, node_list_count($6));
        $$ = create_public_function_node($3, NULL, $10, $12);
        $$->data.function.generic_params = $6;
    }
    | PUB FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN param_list RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($3, GENERIC_KIND_FUNCTION, node_list_count($6));
        $$ = create_public_function_node($3, $9, $11, $13);
        $$->data.function.generic_params = $6;
    }
    | PUB FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($3, GENERIC_KIND_FUNCTION, node_list_count($6));
        $$ = create_public_function_node($3, NULL, void_type, $11);
        $$->data.function.generic_params = $6;
    }
    | PUB FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN param_list RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($3, GENERIC_KIND_FUNCTION, node_list_count($6));
        $$ = create_public_function_node($3, $9, void_type, $12);
        $$->data.function.generic_params = $6;
    }
    ;

function_definition
    : FN IDENTIFIER LPAREN RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($2, GENERIC_KIND_FUNCTION, 0);
        $$ = create_function_node($2, NULL, $5, $7);
        $$->data.function.is_public = 0;
    }
    | FN IDENTIFIER LPAREN param_list RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($2, GENERIC_KIND_FUNCTION, 0);
        $$ = create_function_node($2, $4, $6, $8);
        $$->data.function.is_public = 0;
    }
    | FN IDENTIFIER LPAREN RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($2, GENERIC_KIND_FUNCTION, 0);
        $$ = create_function_node($2, NULL, void_type, $6);
        $$->data.function.is_public = 0;
    }
    | FN IDENTIFIER LPAREN param_list RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($2, GENERIC_KIND_FUNCTION, 0);
        $$ = create_function_node($2, $4, void_type, $7);
        $$->data.function.is_public = 0;
    }
    | FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($2, GENERIC_KIND_FUNCTION, node_list_count($5));
        $$ = create_function_node($2, NULL, $9, $11);
        $$->data.function.generic_params = $5;
        $$->data.function.is_public = 0;
    }
    | FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN param_list RPAREN function_return_type LBRACE statement_list RBRACE {
        register_generic_arity($2, GENERIC_KIND_FUNCTION, node_list_count($5));
        $$ = create_function_node($2, $8, $10, $12);
        $$->data.function.generic_params = $5;
        $$->data.function.is_public = 0;
    }
    | FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($2, GENERIC_KIND_FUNCTION, node_list_count($5));
        $$ = create_function_node($2, NULL, void_type, $10);
        $$->data.function.generic_params = $5;
        $$->data.function.is_public = 0;
    }
    | FN IDENTIFIER COLON LBRACKET generic_param_list RBRACKET LPAREN param_list RPAREN LBRACE statement_list RBRACE {
        ASTNode* void_type = create_type_node(AST_TYPE_VOID);
        register_generic_arity($2, GENERIC_KIND_FUNCTION, node_list_count($5));
        $$ = create_function_node($2, $8, void_type, $11);
        $$->data.function.generic_params = $5;
        $$->data.function.is_public = 0;
    }
    ;

extern_decl
    : FN IDENTIFIER LPAREN RPAREN function_return_type {
        $$ = create_extern_function_node($2, NULL, $5, NULL);
    }
    | FN IDENTIFIER LPAREN param_list RPAREN function_return_type {
        $$ = create_extern_function_node($2, $4, $6, NULL);
    }
    | FN IDENTIFIER LPAREN param_list COMMA DOTDOTDOT RPAREN function_return_type {
        ASTNode* fn = create_extern_function_node($2, $4, $8, NULL);
        fn->data.function.vararg = 1;
        $$ = fn;
    }
    | FN IDENTIFIER LPAREN DOTDOTDOT RPAREN function_return_type {
        ASTNode* fn = create_extern_function_node($2, NULL, $6, NULL);
        fn->data.function.vararg = 1;
        $$ = fn;
    }

extern_decl_list
    : /* empty */ { $$ = create_expression_list_node(); }
    | extern_decl { ASTNode* list = create_expression_list_node(); add_expression_to_list(list, $1); $$ = list; }
    | extern_decl_list extern_decl { add_expression_to_list($1, $2); $$ = $1; }

extern_block
    : EXTERN STRING LBRACE extern_decl_list RBRACE {
        ASTNode* prog = create_program_node();
        if ($4 && $4->type == AST_EXPRESSION_LIST) {
            int cnt = $4->data.expression_list.expression_count;
            for (int i = 0; i < cnt; i++) {
                ASTNode* fn = $4->data.expression_list.expressions[i];
                if (fn && fn->type == AST_FUNCTION) {
                    fn->data.function.is_extern = 1;
                    if ($2) {
                        if (fn->data.function.linkage) free(fn->data.function.linkage);
                        fn->data.function.linkage = malloc(strlen($2) + 1);
                        strcpy(fn->data.function.linkage, $2);
                    }
                    add_statement_to_program(prog, fn);
                }
            }
        }
        $$ = prog;
    }

print_statement
    : PRINT expression              { $$ = create_print_node_with_yyltype($2, (YYLTYPE*) &@$); }
    | PRINT LPAREN expression RPAREN { $$ = create_print_node_with_yyltype($3, (YYLTYPE*) &@$); }
    | PRINT LPAREN expression_list RPAREN { $$ = create_print_node_with_yyltype($3, (YYLTYPE*) &@$); }
    ;

assignment_statement
    : lvalue ASSIGN expression  { $$ = create_assign_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    | IDENTIFIER LBRACKET expression RBRACKET ASSIGN expression {
        ASTNode* target = create_index_node_with_yyltype(
            create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$),
            $3,
            (YYLTYPE*) &@$);
        $$ = create_assign_node_with_yyltype(target, $6, (YYLTYPE*) &@$);
    }
    ;

lvalue
    : identifier                    { $$ = $1; }
    | lvalue DOT identifier         { $$ = create_member_access_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    | lvalue LBRACKET expression RBRACKET { $$ = create_index_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    | AT lvalue                     { $$ = create_unaryop_node(OP_DEREF, $2); }
    ;

compound_assignment_statement
    : identifier PLUS_ASSIGN expression      { 
                                               ASTNode* left = create_identifier_node_with_yyltype($1->data.identifier.name, (YYLTYPE*) &@$);
                                               ASTNode* binop = create_binop_node_with_yyltype(OP_ADD, left, $3, (YYLTYPE*) &@$);
                                               $$ = create_assign_node_with_yyltype($1, binop, (YYLTYPE*) &@$);
                                             }
    | identifier MINUS_ASSIGN expression     { 
                                               ASTNode* left = create_identifier_node_with_yyltype($1->data.identifier.name, (YYLTYPE*) &@$);
                                               ASTNode* binop = create_binop_node_with_yyltype(OP_SUB, left, $3, (YYLTYPE*) &@$);
                                               $$ = create_assign_node_with_yyltype($1, binop, (YYLTYPE*) &@$);
                                             }
    | identifier MULTIPLY_ASSIGN expression  { 
                                               ASTNode* left = create_identifier_node_with_yyltype($1->data.identifier.name, (YYLTYPE*) &@$);
                                               ASTNode* binop = create_binop_node_with_yyltype(OP_MUL, left, $3, (YYLTYPE*) &@$);
                                               $$ = create_assign_node_with_yyltype($1, binop, (YYLTYPE*) &@$);
                                             }
    | identifier DIVIDE_ASSIGN expression    { 
                                               ASTNode* left = create_identifier_node_with_yyltype($1->data.identifier.name, (YYLTYPE*) &@$);
                                               ASTNode* binop = create_binop_node_with_yyltype(OP_DIV, left, $3, (YYLTYPE*) &@$);
                                               $$ = create_assign_node_with_yyltype($1, binop, (YYLTYPE*) &@$);
                                             }
    | identifier MODULO_ASSIGN expression    { 
                                               ASTNode* left = create_identifier_node_with_yyltype($1->data.identifier.name, (YYLTYPE*) &@$);
                                               ASTNode* binop = create_binop_node_with_yyltype(OP_MOD, left, $3, (YYLTYPE*) &@$);
                                               $$ = create_assign_node_with_yyltype($1, binop, (YYLTYPE*) &@$);
                                             }
    ;

block_statement
    : LBRACE statement_list RBRACE { $$ = $2; }
    | LBRACE RBRACE { $$ = create_program_node_with_yyltype((YYLTYPE*) &@$); }
    | statement                    { $$ = $1; }
    ;

if_statement
    : IF LPAREN expression RPAREN block_statement if_rest {
        /* $6 为 else/body（可能为 NULL），if_rest 构造最终 else_body */
        if ($6) {
            $$ = create_if_node_with_yyltype($3, $5, $6, (YYLTYPE*) &@$);
        } else {
            $$ = create_if_node_with_yyltype($3, $5, NULL, (YYLTYPE*) &@$);
        }
    }
    ;

if_rest
    : /* empty */ { $$ = NULL; }
    | ELSE block_statement { $$ = $2; }
    | ELIF LPAREN expression RPAREN block_statement if_rest {
        /* 将 ELIF 转换为 else: if(expr) then block else rest */
        ASTNode* nested_if = create_if_node($3, $5, $6);
        $$ = nested_if;
    }
    ;

while_statement
    : WHILE LPAREN expression RPAREN block_statement {
        $$ = create_while_node_with_yyltype($3, $5, (YYLTYPE*) &@$);
    }
    ;

for_statement
    : FOR LPAREN identifier SEMICOLON expression DOTDOT expression RPAREN block_statement {
        ASTNode* start = $5;
        ASTNode* end = $7;
        $$ = create_for_node_with_yyltype($3, start, end, $9, (YYLTYPE*) &@$);
    }
    | FOR LPAREN identifier IN expression DOTDOT expression RPAREN block_statement {
        ASTNode* start = $5;
        ASTNode* end = $7;
        $$ = create_for_node_with_yyltype($3, start, end, $9, (YYLTYPE*) &@$);
    }
    | FOR LPAREN identifier IN expression RPAREN block_statement {
        ASTNode* var = $3;
        ASTNode* iterable = $5;
        $$ = create_for_node_with_yyltype(var, iterable, NULL, $7, (YYLTYPE*) &@$);
    }
    | FOR LPAREN identifier SEMICOLON expression RPAREN block_statement {
        ASTNode* var = $3;
        ASTNode* iterable = $5;
        $$ = create_for_node_with_yyltype(var, iterable, NULL, $7, (YYLTYPE*) &@$);
    }
    | FOR LPAREN IDENTIFIER identifier SEMICOLON expression RPAREN block_statement {
        ASTNode* var = $4;
        ASTNode* iterable = $6;
        $$ = create_for_node_with_yyltype(var, iterable, NULL, $8, (YYLTYPE*) &@$);
    }
    ;

expression_list
    : expression                    { 
                                      ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
                                      add_expression_to_list(list, $1);
                                      $$ = list;
                                    }
    | expression COMMA expression   { 
                                      ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
                                      add_expression_to_list(list, $1);
                                      add_expression_to_list(list, $3);
                                      $$ = list;
                                    }
    | expression_list COMMA expression { 
                                      add_expression_to_list($1, $3);
                                      $$ = $1;
                                    }
    ;

logical_expression
    : comparison_expression                 { $$ = $1; }
    | logical_expression AND comparison_expression { $$ = create_binop_node_with_yyltype(OP_AND, $1, $3, (YYLTYPE*) &@$); }
    | logical_expression OR comparison_expression  { $$ = create_binop_node_with_yyltype(OP_OR, $1, $3, (YYLTYPE*) &@$); }
    ;

comparison_expression
    : additive_expression                                  { $$ = $1; }
    | additive_expression EQ additive_expression            { $$ = create_binop_node_with_yyltype(OP_EQ, $1, $3, (YYLTYPE*) &@$); }
    | additive_expression NE additive_expression            { $$ = create_binop_node_with_yyltype(OP_NE, $1, $3, (YYLTYPE*) &@$); }
    | additive_expression LT additive_expression            { $$ = create_binop_node_with_yyltype(OP_LT, $1, $3, (YYLTYPE*) &@$); }
    | additive_expression LE additive_expression            { $$ = create_binop_node_with_yyltype(OP_LE, $1, $3, (YYLTYPE*) &@$); }
    | additive_expression GT additive_expression            { $$ = create_binop_node_with_yyltype(OP_GT, $1, $3, (YYLTYPE*) &@$); }
    | additive_expression GE additive_expression            { $$ = create_binop_node_with_yyltype(OP_GE, $1, $3, (YYLTYPE*) &@$); }
    ;

expression
    : logical_expression                    { $$ = $1; }
    | lvalue ASSIGN expression              { $$ = create_assign_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    ;

additive_expression
    : term                                  { $$ = $1; }
    | additive_expression PLUS term         { $$ = create_binop_node_with_yyltype(OP_ADD, $1, $3, (YYLTYPE*) &@$); }
    | additive_expression MINUS term        { $$ = create_binop_node_with_yyltype(OP_SUB, $1, $3, (YYLTYPE*) &@$); }
    ;

term
    : factor                        { $$ = $1; }
    | term MULTIPLY factor          { $$ = create_binop_node_with_yyltype(OP_MUL, $1, $3, (YYLTYPE*) &@$); }
    | term DIVIDE factor            { $$ = create_binop_node_with_yyltype(OP_DIV, $1, $3, (YYLTYPE*) &@$); }
    | term MODULO factor            { $$ = create_binop_node_with_yyltype(OP_MOD, $1, $3, (YYLTYPE*) &@$); }
    ;

factor
    : power                         { $$ = $1; }
    ;

power
    : factor_unary                  { $$ = $1; }
    | factor_unary POWER factor     { $$ = create_binop_node_with_yyltype(OP_POW, $1, $3, (YYLTYPE*) &@$); }
    ;

factor_unary
    : IDENTIFIER LPAREN RPAREN      { 
        ASTNode* id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$); 
        $$ = create_call_node_with_yyltype(id, NULL, (YYLTYPE*) &@$); 
    }
    | IDENTIFIER LPAREN expression COMMA FN LPAREN IDENTIFIER COLON type RPAREN COLON type LBRACE statement_list RBRACE RPAREN {
        static int lambda_counter = 0;
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "__lambda_%d", lambda_counter++);

        ASTNode* params = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        ASTNode* id_node = create_identifier_node_with_yyltype($7, (YYLTYPE*) &@$);
        ASTNode* annotated_param = create_assign_node_with_yyltype(id_node, $9, (YYLTYPE*) &@$);
        annotated_param->data.assign.is_declaration = 2;
        add_expression_to_list(params, annotated_param);

        ASTNode* lambda_fn = create_function_node(name_buf, params, $12, $14);
        lambda_fn->data.function.is_public = 0;

        ASTNode* args = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(args, $3);
        add_expression_to_list(args, lambda_fn);

        ASTNode* id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
        $$ = create_call_node_with_yyltype(id, args, (YYLTYPE*) &@$);
    }
    | IDENTIFIER LPAREN expression_list RPAREN { 
        ASTNode* id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$); 
        $$ = create_call_node_with_yyltype(id, $3, (YYLTYPE*) &@$); 
    }
    | IDENTIFIER LBRACKET expression RBRACKET {
        ASTNode* id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
        $$ = create_index_node_with_yyltype(id, $3, (YYLTYPE*) &@$);
    }
    | IDENTIFIER COLON LBRACKET generic_type_args RBRACKET LPAREN RPAREN {
        check_generic_arity_usage($1, GENERIC_KIND_FUNCTION, $4, (YYLTYPE*) &@$);
        ASTNode* id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
        $$ = create_call_node_with_yyltype(id, NULL, (YYLTYPE*) &@$);
        $$->data.call.type_args = $4;
    }
    | IDENTIFIER COLON LBRACKET generic_type_args RBRACKET LPAREN expression_list RPAREN {
        check_generic_arity_usage($1, GENERIC_KIND_FUNCTION, $4, (YYLTYPE*) &@$);
        ASTNode* id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
        $$ = create_call_node_with_yyltype(id, $7, (YYLTYPE*) &@$);
        $$->data.call.type_args = $4;
    }
    | IDENTIFIER LBRACE RBRACE { ASTNode* type_id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$); ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$); $$ = create_struct_literal_node_with_yyltype(type_id, list, (YYLTYPE*) &@$); }
    | IDENTIFIER LBRACE struct_init_fields RBRACE { ASTNode* type_id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$); $$ = create_struct_literal_node_with_yyltype(type_id, $3, (YYLTYPE*) &@$); }
    | IDENTIFIER COLON LBRACKET generic_type_args RBRACKET LBRACE RBRACE {
        check_generic_arity_usage($1, GENERIC_KIND_STRUCT, $4, (YYLTYPE*) &@$);
        ASTNode* type_id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
        ASTNode* list = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        $$ = create_struct_literal_node_with_yyltype(type_id, list, (YYLTYPE*) &@$);
    }
    | IDENTIFIER COLON LBRACKET generic_type_args RBRACKET LBRACE struct_init_fields RBRACE {
        check_generic_arity_usage($1, GENERIC_KIND_STRUCT, $4, (YYLTYPE*) &@$);
        ASTNode* type_id = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
        $$ = create_struct_literal_node_with_yyltype(type_id, $7, (YYLTYPE*) &@$);
    }
    | IDENTIFIER {
        $$ = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$);
    }
    | FN LPAREN IDENTIFIER COLON type RPAREN function_return_type block_statement {
        static int lambda_counter = 0;
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "__lambda_%d", lambda_counter++);
        ASTNode* params = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        ASTNode* id_node = create_identifier_node_with_yyltype($3, (YYLTYPE*) &@$);
        ASTNode* annotated_param = create_assign_node_with_yyltype(id_node, $5, (YYLTYPE*) &@$);
        annotated_param->data.assign.is_declaration = 2;
        add_expression_to_list(params, annotated_param);
        ASTNode* fn = create_function_node(name_buf, params, $7, $8);
        fn->data.function.is_public = 0;
        $$ = fn;
    }
    | FN LPAREN param_list RPAREN function_return_type block_statement {
        static int lambda_counter = 0;
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "__lambda_%d", lambda_counter++);
        ASTNode* fn = create_function_node(name_buf, $3, $5, $6);
        fn->data.function.is_public = 0;
        $$ = fn;
    }
    | FN LPAREN RPAREN function_return_type block_statement {
        static int lambda_counter = 0;
        char name_buf[64];
        snprintf(name_buf, sizeof(name_buf), "__lambda_%d", lambda_counter++);
        ASTNode* fn = create_function_node(name_buf, NULL, $4, $5);
        fn->data.function.is_public = 0;
        $$ = fn;
    }
    | literal                       { $$ = $1; }
    | toint_expression              { $$ = $1; }
    | tofloat_expression            { $$ = $1; }
    | input_expression              { $$ = $1; }
    | PLUS factor_unary             { $$ = create_unaryop_node(OP_PLUS, $2); }
    | MINUS factor_unary            { $$ = create_unaryop_node(OP_MINUS, $2); }
    | MULTIPLY factor_unary         { $$ = create_unaryop_node(OP_DEREF, $2); }
    | AMPERSAND factor_unary        { $$ = create_unaryop_node(OP_ADDRESS, $2); }
    | AT factor_unary               { $$ = create_unaryop_node(OP_DEREF, $2); }
    | LPAREN expression RPAREN      { $$ = $2; }
    | LPAREN expression COMMA expression_list RPAREN {
        ASTNode* tuple = create_expression_list_node_with_yyltype((YYLTYPE*) &@$);
        add_expression_to_list(tuple, $2);
        if ($4 && $4->type == AST_EXPRESSION_LIST) {
            for (int i = 0; i < $4->data.expression_list.expression_count; i++) {
                add_expression_to_list(tuple, $4->data.expression_list.expressions[i]);
            }
        }
        $$ = tuple;
    }
    | factor_unary DOT IDENTIFIER { $$ = create_member_access_node_with_yyltype($1, create_identifier_node_with_yyltype($3, (YYLTYPE*) &@$), (YYLTYPE*) &@$); }
    | factor_unary DOT NUMBER_INT {
        char idx_name[32];
        snprintf(idx_name, sizeof(idx_name), "%lld", $3);
        $$ = create_member_access_node_with_yyltype($1, create_identifier_node_with_yyltype(idx_name, (YYLTYPE*) &@$), (YYLTYPE*) &@$);
    }
    | factor_unary DOT IDENTIFIER LPAREN RPAREN { 
        ASTNode* method = create_identifier_node_with_yyltype($3, (YYLTYPE*) &@$);
        ASTNode* mem = create_member_access_node_with_yyltype($1, method, (YYLTYPE*) &@$);
        $$ = create_call_node_with_yyltype(mem, NULL, (YYLTYPE*) &@$);
    }
    | factor_unary DOT IDENTIFIER LPAREN expression_list RPAREN { 
        ASTNode* method = create_identifier_node_with_yyltype($3, (YYLTYPE*) &@$);
        ASTNode* mem = create_member_access_node_with_yyltype($1, method, (YYLTYPE*) &@$);
        $$ = create_call_node_with_yyltype(mem, $5, (YYLTYPE*) &@$);
    }
    | factor_unary LBRACKET expression RBRACKET { $$ = create_index_node_with_yyltype($1, $3, (YYLTYPE*) &@$); }
    ;

literal
    : NUMBER_INT                    { $$ = create_num_int_node_with_yyltype($1, (YYLTYPE*) &@$); }
    | NUMBER_FLOAT                  { $$ = create_num_float_node_with_yyltype($1, (YYLTYPE*) &@$); }
    | STRING                        { $$ = create_string_node_with_yyltype($1, (YYLTYPE*) &@$); }
    | CHAR_LITERAL                  { $$ = create_char_node_with_yyltype((char)$1, (YYLTYPE*) &@$); }
    | NIL                           { $$ = create_nil_node_with_yyltype((YYLTYPE*) &@$); }
    | LBRACKET RBRACKET             { $$ = create_expression_list_node_with_yyltype((YYLTYPE*) &@$); }
    | LBRACKET expression_list RBRACKET { $$ = $2; }
    ;

identifier
    : IDENTIFIER                    { $$ = create_identifier_node_with_yyltype($1, (YYLTYPE*) &@$); }
    ;

%%

void yyerror(const char* s) {
    const char* filename = current_input_filename ? current_input_filename : "unknown";
    report_syntax_error_with_location(s, filename, yylineno);
}
