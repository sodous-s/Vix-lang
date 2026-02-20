/*
vix 0.0.1 released!
*/
#include "../include/compiler.h"
#include "../include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_VARS 1024

#define MAX_RECURSION_DEPTH 1000
static int current_recursion_depth = 0;

static void emit_expression_with_context(FILE* out, ASTNode* node, int in_struct_literal, TypeInferenceContext* ctx);
static void compile_node(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node);
static void compile_program(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node);
static void compile_print(FILE* out, ASTNode* node, TypeInferenceContext* ctx);
static void compile_assign(TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node);
static void compile_const(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node);
static void compile_if(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node);
static void compile_function(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node);
static int check_function_has_return(ASTNode* node);
static const char* get_param_type_string(TypeInferenceContext* ctx, ASTNode* param);
static void compile_struct_def(FILE* out, ASTNode* node);


static void emit_expression_with_context(FILE* out, ASTNode* node, int in_struct_literal, TypeInferenceContext* ctx) {
    if (!node) { fprintf(out, "/*null*/0"); return; }
    
    // 检查递归深度，防止栈溢出
    if (current_recursion_depth >= MAX_RECURSION_DEPTH) {
        fprintf(out, "/*recursion limit reached*/0");
        return;
    }
    current_recursion_depth++;
    
    switch (node->type) {
        case AST_NUM_INT:
            fprintf(out, "%lldLL", node->data.num_int.value);
            break;
        case AST_NUM_FLOAT:
            fprintf(out, "%f", node->data.num_float.value);
            break;
        case AST_STRING:
            fprintf(out, "\"%s\"", node->data.string.value);
            break;
        case AST_UNARYOP:
            if (node->data.unaryop.op == OP_MINUS) {
                fprintf(out, "(-");
                emit_expression_with_context(out, node->data.unaryop.expr, in_struct_literal, ctx);
                fprintf(out, ")");
            } else if (node->data.unaryop.op == OP_PLUS) {
                fprintf(out, "(");
                emit_expression_with_context(out, node->data.unaryop.expr, in_struct_literal, ctx);
                fprintf(out, ")");
            } else if (node->data.unaryop.op == OP_ADDRESS) {
                fprintf(out, "(&(");
                emit_expression_with_context(out, node->data.unaryop.expr, in_struct_literal, ctx);
                fprintf(out, "))");
            } else if (node->data.unaryop.op == OP_DEREF) {
                fprintf(out, "*(");
                emit_expression_with_context(out, node->data.unaryop.expr, in_struct_literal, ctx);
                fprintf(out, ")");
            } else {
                fprintf(out, "/*unop*/0");
            }
            break;
        case AST_BINOP: {
            const char* op = "?";
            switch (node->data.binop.op) {
                case OP_ADD: op = " + "; break;
                case OP_SUB: op = " - "; break;
                case OP_MUL: op = " * "; break;
                case OP_DIV: op = " / "; break;
                case OP_MOD: op = " % "; break;
                case OP_POW: op = NULL; break;
                case OP_EQ:  op = " == "; break;
                case OP_NE:  op = " != "; break;
                case OP_LT:  op = " < "; break;
                case OP_LE:  op = " <= "; break;
                case OP_GT:  op = " > "; break;
                case OP_GE:  op = " >= "; break;
                default: op = " ? "; break;
            }
            if (node->data.binop.op == OP_POW) {
                fprintf(out, "pow(");
                emit_expression_with_context(out, node->data.binop.left, in_struct_literal, ctx);
                fprintf(out, ", ");
                emit_expression_with_context(out, node->data.binop.right, in_struct_literal, ctx);
                fprintf(out, ")");
            } else {
                if (node->data.binop.op == OP_ADD) {
                    int left_is_addr = (node->data.binop.left->type == AST_UNARYOP && node->data.binop.left->data.unaryop.op == OP_ADDRESS);
                    int right_is_num = (node->data.binop.right->type == AST_NUM_INT || node->data.binop.right->type == AST_NUM_FLOAT);
                    int left_is_num = (node->data.binop.left->type == AST_NUM_INT || node->data.binop.left->type == AST_NUM_FLOAT);
                    int right_is_addr = (node->data.binop.right->type == AST_UNARYOP && node->data.binop.right->data.unaryop.op == OP_ADDRESS);
                    if ((left_is_addr || right_is_addr) && (left_is_num || right_is_num)) {// 特殊处理：地址 + 数值 或 数值 + 地址
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.binop.left, in_struct_literal, ctx);
                        fprintf(out, ")");
                        fprintf(out, "%s", op);
                        emit_expression_with_context(out, node->data.binop.right, in_struct_literal, ctx);
                    } else {
                        emit_expression_with_context(out, node->data.binop.left, in_struct_literal, ctx);
                        fprintf(out, "%s", op);
                        emit_expression_with_context(out, node->data.binop.right, in_struct_literal, ctx);
                    }
                } else {
                    emit_expression_with_context(out, node->data.binop.left, in_struct_literal, ctx);
                    fprintf(out, "%s", op);
                    emit_expression_with_context(out, node->data.binop.right, in_struct_literal, ctx);
                }
            }
            break;
        }
        case AST_CALL: {
            /*支持成员调用，如 obj.method(...)，其中 func 可以是 AST_INDEX*/
            if (node->data.call.func->type == AST_INDEX) {
                ASTNode* target = node->data.call.func->data.index.target;
                ASTNode* method = node->data.call.func->data.index.index;
                if (!method || method->type != AST_IDENTIFIER) {
                    fprintf(out, "/*call: method name invalid*/0");
                    break;
                }
                const char* mname = method->data.identifier.name;

                if (strcmp(mname, "add!") == 0) {
                    /* obj.add!(idx, val) -> (target).add_inplace((size_t)idx, to_vstring(val)) */
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").add_inplace((size_t)(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 0) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[0], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "0");
                    }
                    fprintf(out, "), vconvert::to_vstring(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 1) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[1], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "\"\"");
                    }
                    fprintf(out, "))");
                } else if (strcmp(mname, "remove") == 0) {
                    /* obj.remove(idx) -> (target).remove((size_t)idx) */
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").remove((size_t)(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 0) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[0], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "0");
                    }
                    fprintf(out, "))");
                } else if (strcmp(mname, "remove!") == 0) {
                    /* obj.remove!(idx) -> (target).remove_inplace((size_t)idx)*/
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").remove_inplace((size_t)(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 0) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[0], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "0");
                    }
                    fprintf(out, "))");
                } else if (strcmp(mname, "push!") == 0) {
                    /* obj.push!(val) -> (target).push_inplace(to_vstring(val)) */
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").push_inplace(vconvert::to_vstring(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 0) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[0], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "\"\"");
                    }
                    fprintf(out, "))");
                } else if (strcmp(mname, "push") == 0) {
                    /* obj.push(val) -> (target).push(to_vstring(val)) */
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").push(vconvert::to_vstring(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 0) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[0], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "\"\"");
                    }
                    fprintf(out, "))");
                } else if (strcmp(mname, "pop") == 0) {
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").pop()");
                } else if (strcmp(mname, "pop!") == 0) {
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").pop_inplace()");
                } else if (strcmp(mname, "replace!") == 0) {
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").replace_inplace((size_t)(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 0) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[0], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "0");
                    }
                    fprintf(out, "), vconvert::to_vstring(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 1) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[1], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "\"\"");
                    }
                    fprintf(out, "))");
                } else if (strcmp(mname, "insert!") == 0) {
                    /* obj.insert!(idx, val) -> (target).add_inplace((size_t)idx, to_vstring(val)) */
                    fprintf(out, "("); emit_expression_with_context(out, target, in_struct_literal, ctx); fprintf(out, ").add_inplace((size_t)(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 0) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[0], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "0");
                    }
                    fprintf(out, "), vconvert::to_vstring(");
                    if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST && node->data.call.args->data.expression_list.expression_count > 1) {
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[1], in_struct_literal, ctx);
                    } else {
                        fprintf(out, "\"\"");
                    }
                    fprintf(out, "))");
                } else {
                    fprintf(out, "/*unknown method:%s*/0", mname);
                }
                break;
            }
            if (node->data.call.func->type == AST_IDENTIFIER) {
                fprintf(out, "%s(", node->data.call.func->data.identifier.name);
                if (node->data.call.args && node->data.call.args->type == AST_EXPRESSION_LIST) {
                    for (int i = 0; i < node->data.call.args->data.expression_list.expression_count; i++) {
                        if (i > 0) fprintf(out, ", ");
                        emit_expression_with_context(out, node->data.call.args->data.expression_list.expressions[i], in_struct_literal, ctx);
                    }
                }
                fprintf(out, ")");
            } else {
                fprintf(out, "/*call: func not identifier*/0");
            }
            break;
        }
        case AST_IDENTIFIER: {
            fprintf(out, "%s", node->data.identifier.name);
            break;
        }
        case AST_EXPRESSION_LIST: {
            fprintf(out, "vtypes::VList{ ");
            for (int i = 0; i < node->data.expression_list.expression_count; i++) {
                if (i) fprintf(out, ", ");
                ASTNode* elem = node->data.expression_list.expressions[i];
                if (elem->type == AST_STRING) {
                    fprintf(out, "vtypes::VString(\"%s\")", elem->data.string.value);
                } else {
                    fprintf(out, "vconvert::to_vstring(");
                    emit_expression_with_context(out, elem, in_struct_literal, ctx);
                    fprintf(out, ")");
                }
            }
            fprintf(out, " }");
            break;
        }
        case AST_INDEX: {
            if (node->data.index.index && node->data.index.index->type == AST_IDENTIFIER) {
                const char* field_name = node->data.index.index->data.identifier.name;
                if (strcmp(field_name, "length") == 0) {
                    fprintf(out, "(");
                    emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                    fprintf(out, ").items.size()");
                } else {
                    int is_struct = 0;
                    int is_list = 0;
                    if (node->data.index.target->type == AST_IDENTIFIER && ctx) {
                        InferredType targetType = get_variable_type(ctx, node->data.index.target->data.identifier.name);
                        if (targetType == TYPE_STRUCT) {
                            is_struct = 1;
                        } else if (targetType == TYPE_LIST) {
                            is_list = 1;
                        }
                    }
                    else if (node->data.index.target->type == AST_STRUCT_LITERAL) {
                        is_struct = 1;
                    }
                    else if (node->data.index.target->type == AST_EXPRESSION_LIST) {
                        is_list = 1;
                    }
                    
                    if (is_struct) {
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                        fprintf(out, ").%s", field_name);
                    } else if (is_list) {
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                        fprintf(out, ")[");
                        emit_expression_with_context(out, node->data.index.index, in_struct_literal, ctx);
                        fprintf(out, "]");
                    } else {
                        int is_primitive = 0;
                        if (node->data.index.target->type == AST_IDENTIFIER && ctx) {
                            InferredType targetType = get_variable_type(ctx, node->data.index.target->data.identifier.name);
                            if (targetType == TYPE_INT || targetType == TYPE_FLOAT) {
                                is_primitive = 1;
                            }
                        }
                        
                        if (is_primitive) {
                            fprintf(out, "/* ERROR: primitive type does not support indexing */0");
                        } else {
                            if (node->data.index.target->type == AST_IDENTIFIER && ctx) {
                                InferredType targetType = get_variable_type(ctx, node->data.index.target->data.identifier.name);
                                if (targetType == TYPE_STRUCT) {
                                    fprintf(out, "(");
                                    emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                                    fprintf(out, ").%s", field_name);
                                } else if (targetType == TYPE_LIST) {
                                    fprintf(out, "(");
                                    emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                                    fprintf(out, ")[(");
                                    emit_expression_with_context(out, node->data.index.index, in_struct_literal, ctx);
                                    fprintf(out, ")]");
                                } else {
                                    fprintf(out, "(");
                                    emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                                    fprintf(out, ")[");
                                    emit_expression_with_context(out, node->data.index.index, in_struct_literal, ctx);
                                    fprintf(out, "]");
                                }
                            } else {
                                fprintf(out, "(");
                                emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                                fprintf(out, ")[");
                                emit_expression_with_context(out, node->data.index.index, in_struct_literal, ctx);
                                fprintf(out, "]");
                            }
                        }
                    }
                }
            } else {
                int is_struct = 0;
                int is_list = 0;
                if (node->data.index.target->type == AST_IDENTIFIER && ctx) {
                    InferredType targetType = get_variable_type(ctx, node->data.index.target->data.identifier.name);
                    if (targetType == TYPE_STRUCT) {
                        is_struct = 1;
                    } else if (targetType == TYPE_LIST) {
                        is_list = 1;
                    }
                }
                else if (node->data.index.target->type == AST_STRUCT_LITERAL) {
                    is_struct = 1;
                }
                else if (node->data.index.target->type == AST_EXPRESSION_LIST) {
                    is_list = 1;
                }
                
                if (is_struct) {
                    fprintf(out, "/* ERROR: struct member access requires identifier */0");
                } else if (is_list) {
                    fprintf(out, "(");
                    emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                    fprintf(out, ")[");
                    emit_expression_with_context(out, node->data.index.index, in_struct_literal, ctx);
                    fprintf(out, "]");
                } else {
                    int is_primitive = 0;
                    if (node->data.index.target->type == AST_IDENTIFIER && ctx) {
                        InferredType targetType = get_variable_type(ctx, node->data.index.target->data.identifier.name);
                        if (targetType == TYPE_INT || targetType == TYPE_FLOAT) {
                            is_primitive = 1;
                        }
                    }
                    
                    if (is_primitive) {
                        fprintf(out, "/* ERROR: primitive type does not support indexing */0");
                    } else {
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.index.target, in_struct_literal, ctx);
                        fprintf(out, ")[");
                        emit_expression_with_context(out, node->data.index.index, in_struct_literal, ctx);
                        fprintf(out, "]");
                    }
                }
            }
            break;
        }
        case AST_MEMBER_ACCESS: {
            if (node->data.member_access.field && node->data.member_access.field->type == AST_IDENTIFIER) {
                const char* field_name = node->data.member_access.field->data.identifier.name;
                
                // 检查是否是.length属性访问
                if (strcmp(field_name, "length") == 0) {
                    // 使用预计算的数组长度
                    int array_length = get_array_length(node->data.member_access.object);
                    if (array_length >= 0) {
                        fprintf(out, "%d", array_length);
                    } else {
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.member_access.object, in_struct_literal, ctx);
                        fprintf(out, ").items.size()");
                    }
                } else {
                    int is_struct = 0;
                    int is_list = 0;
                    if (node->data.member_access.object->type == AST_IDENTIFIER && ctx) {
                        InferredType targetType = get_variable_type(ctx, node->data.member_access.object->data.identifier.name);
                        if (targetType == TYPE_STRUCT) {
                            is_struct = 1;
                        } else if (targetType == TYPE_LIST) {
                            is_list = 1;
                        }
                    }
                    else if (node->data.member_access.object->type == AST_STRUCT_LITERAL) {
                        is_struct = 1;
                    }
                    else if (node->data.member_access.object->type == AST_EXPRESSION_LIST) {
                        is_list = 1;
                    }
                    
                    if (is_struct) {
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.member_access.object, in_struct_literal, ctx);
                        fprintf(out, ").%s", field_name);
                    } else if (is_list) {
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.member_access.object, in_struct_literal, ctx);
                        fprintf(out, ").");
                        emit_expression_with_context(out, node->data.member_access.field, in_struct_literal, ctx);
                    } else {
                        fprintf(out, "(");
                        emit_expression_with_context(out, node->data.member_access.object, in_struct_literal, ctx);
                        fprintf(out, ").");
                        emit_expression_with_context(out, node->data.member_access.field, in_struct_literal, ctx);
                    }
                }
            } else {
                fprintf(out, "/* ERROR: struct member access requires identifier */0");
            }
            break;
        }
        case AST_STRUCT_LITERAL: {
            if (node->data.struct_literal.type_name && node->data.struct_literal.type_name->type == AST_IDENTIFIER) {
                const char* sname = node->data.struct_literal.type_name->data.identifier.name;
                fprintf(out, "([&]{ %s __tmp; ", sname);
                if (node->data.struct_literal.fields && node->data.struct_literal.fields->type == AST_EXPRESSION_LIST) {
                    int fc = node->data.struct_literal.fields->data.expression_list.expression_count;
                    for (int i = 0; i < fc; i++) {
                        ASTNode* f = node->data.struct_literal.fields->data.expression_list.expressions[i];
                        if (f->type == AST_ASSIGN && f->data.assign.left->type == AST_IDENTIFIER) {
                            fprintf(out, "__tmp.%s = ", f->data.assign.left->data.identifier.name);
                            emit_expression_with_context(out, f->data.assign.right, 1, ctx);
                            fprintf(out, "; ");
                        }
                    }
                }
                fprintf(out, "return __tmp; })()");
            } else {
                fprintf(out, "/*struct literal: invalid type*/0");
            }
            break;
        }
        default:
            fprintf(out, "/*expr_unhandled*/0");
            break;
    }
    
    // 递归完成后减少深度计数
    current_recursion_depth--;
}

static const char* node_type_to_cpp_string(NodeType t, const char* type_name) {
    switch (t) {
        case AST_TYPE_INT32:
        case AST_TYPE_INT64:
            return "long long";
        case AST_TYPE_FLOAT32:
        case AST_TYPE_FLOAT64:
            return "float";
        case AST_TYPE_STRING:
            return "VString";
        case AST_TYPE_VOID:
            return "void";
        case AST_TYPE_POINTER:
            return "long long*";
        default:
            // 对于用户定义的类型（如结构体），返回类型名称
            if (type_name) {
                return type_name;
            }
            return "auto";
    }
}

static void compile_struct_def(FILE* out, ASTNode* node) {
    if (!node || node->type != AST_STRUCT_DEF) return;
    if (!node->data.struct_def.name) return;
    fprintf(out, "struct %s {\n", node->data.struct_def.name);
    if (node->data.struct_def.fields && node->data.struct_def.fields->type == AST_EXPRESSION_LIST) {
        int fc = node->data.struct_def.fields->data.expression_list.expression_count;
        for (int i = 0; i < fc; i++) {
            ASTNode* f = node->data.struct_def.fields->data.expression_list.expressions[i];
            if (f->type == AST_ASSIGN && f->data.assign.left->type == AST_IDENTIFIER) {
                const char* fname = f->data.assign.left->data.identifier.name;
                NodeType rt = f->data.assign.right->type;
                const char* cpp_t;
                if (f->data.assign.right->type == AST_IDENTIFIER) {
                    cpp_t = f->data.assign.right->data.identifier.name;
                } else {
                    cpp_t = node_type_to_cpp_string(rt, NULL);
                }
                
                fprintf(out, "    %s %s;\n", cpp_t, fname);
            }
        }
    }
    fprintf(out, "};\n\n");
}

void compile_ast_to_cpp_with_types(ByteCodeGen* gen, TypeInferenceContext* ctx, ASTNode* root, FILE* out) {
    fprintf(out, "// ast to cpp");
    fprintf(out, "#include <iostream>\n");
    fprintf(out, "#include <string>\n");
    fprintf(out, "#include <cmath>\n");
    fprintf(out, "#include \"lib/vcore.hpp\"\n");
    fprintf(out, "#include \"lib/vtypes.hpp\"\n");
    fprintf(out, "#include \"lib/vconvert.hpp\"\n\n");
    fprintf(out, "using namespace vcore;\n");
    fprintf(out, "using namespace vtypes;\n");
    fprintf(out, "using namespace vconvert;\n\n");
    if (root && root->type == AST_PROGRAM) {
        /* 先生成结构体定义 */
        for (int i = 0; i < root->data.program.statement_count; i++) {
            ASTNode* stmt = root->data.program.statements[i];
            if (stmt && stmt->type == AST_STRUCT_DEF) {
                compile_struct_def(out, stmt);
            }
        }

        for (int i = 0; i < root->data.program.statement_count; i++) {
            ASTNode* stmt = root->data.program.statements[i];
            if (stmt && stmt->type == AST_FUNCTION && 
                stmt->data.function.name && 
                strcmp(stmt->data.function.name, "main") != 0) {
                int *decl = calloc(MAX_VARS, sizeof(int));
                compile_function(gen, ctx, out, decl, stmt);
                free(decl);
            }
        }
    }
    fprintf(out, "int main() {\n");
    int max_vars = gen ? gen->var_count : 0;
    int *decl = calloc(MAX_VARS, sizeof(int));
    if (gen && max_vars > MAX_VARS) max_vars = MAX_VARS;
    if (root && root->type == AST_PROGRAM) {
        for (int i = 0; i < root->data.program.statement_count; i++) {
            ASTNode* stmt = root->data.program.statements[i];
            if (stmt && stmt->type != AST_FUNCTION) {
                compile_node(gen, ctx, out, decl, stmt);
            } else if (stmt && stmt->type == AST_FUNCTION && 
                       stmt->data.function.name && 
                       strcmp(stmt->data.function.name, "main") == 0) {
                if (stmt->data.function.body) {
                    if (stmt->data.function.body->type == AST_PROGRAM) {
                        for (int j = 0; j < stmt->data.function.body->data.program.statement_count; j++) {
                            compile_node(gen, ctx, out, decl, stmt->data.function.body->data.program.statements[j]);
                        }
                    } else {
                        compile_node(gen, ctx, out, decl, stmt->data.function.body);
                    }
                }
            }
        }
    }
    
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    free(decl);
}

static void compile_program(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node){
    for (int i = 0; i < node->data.program.statement_count; i++) {
        compile_node(gen, ctx, out, decl, node->data.program.statements[i]);
    }
}

static void compile_print(FILE* out, ASTNode* node, TypeInferenceContext* ctx) {
    ASTNode* e = node->data.print.expr;
    if (e->type == AST_EXPRESSION_LIST) {
        fprintf(out, "    std::cout << ");
        for (int i = 0; i < e->data.expression_list.expression_count; i++) {
            if (i) fprintf(out, " << \" \" << ");
            ASTNode* expr = e->data.expression_list.expressions[i];
            if (expr->type == AST_NUM_INT) {
                fprintf(out, "%lldLL", expr->data.num_int.value);
            } else if (expr->type == AST_NUM_FLOAT) {
                fprintf(out, "%f", expr->data.num_float.value);
            } else if (expr->type == AST_STRING) {
                fprintf(out, "\"%s\"", expr->data.string.value);
            } else if (expr->type == AST_IDENTIFIER) {
                fprintf(out, "%s", expr->data.identifier.name);
            } else {
                fprintf(out, "vconvert::to_vstring(");
                emit_expression_with_context(out, expr, 0, ctx);
                fprintf(out, ")");
            }
        }
        fprintf(out, " << std::endl;\n");
    } else {
        fprintf(out, "    std::cout << ");
        if (e->type == AST_NUM_INT) {
            fprintf(out, "%lldLL", e->data.num_int.value);
        } else if (e->type == AST_NUM_FLOAT) {
            fprintf(out, "%f", e->data.num_float.value);
        } else if (e->type == AST_STRING) {
            fprintf(out, "\"%s\"", e->data.string.value);
        } else if (e->type == AST_IDENTIFIER) {
            fprintf(out, "%s", e->data.identifier.name);
        } else {
            fprintf(out, "vconvert::to_vstring(");
            emit_expression_with_context(out, e, 0, ctx);
            fprintf(out, ")");
        }
        fprintf(out, " << std::endl;\n");
    }
}

static void compile_assign(TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node) {
    ASTNode* left = node->data.assign.left;
    ASTNode* right = node->data.assign.right;
    if (left->type == AST_UNARYOP && left->data.unaryop.op == OP_DEREF) {
        fprintf(out, "    *(");
        emit_expression_with_context(out, left->data.unaryop.expr, 0, ctx);
        fprintf(out, ") = ");
        emit_expression_with_context(out, right, 0, ctx);
        fprintf(out, ";\n");
        return;
    }
    
    if (left->type == AST_INDEX) {
        if (left->data.index.index && left->data.index.index->type == AST_IDENTIFIER) {
            const char* field_name = left->data.index.index->data.identifier.name;
            if (strcmp(field_name, "length") == 0) {
                fprintf(out, "    // Error: 'length' field is read-only\n");
                return;
            }
            int is_struct = 0;
            int is_list = 0;
            if (left->data.index.target->type == AST_IDENTIFIER && ctx) {
                InferredType targetType = get_variable_type(ctx, left->data.index.target->data.identifier.name);
                if (targetType == TYPE_STRUCT) {
                    is_struct = 1;
                } else if (targetType == TYPE_LIST) {
                    is_list = 1;
                }
            }
            else if (left->data.index.target->type == AST_STRUCT_LITERAL) {
                is_struct = 1;
            }
            else if (left->data.index.target->type == AST_EXPRESSION_LIST) {
                is_list = 1;
            }
            
            if (is_struct) {
                fprintf(out, "    (");
                emit_expression_with_context(out, left->data.index.target, 0, ctx);
                fprintf(out, ").%s = ", field_name);
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            } else if (is_list) {
                fprintf(out, "    ");
                emit_expression_with_context(out, left->data.index.target, 0, ctx);
                fprintf(out, " .");
                emit_expression_with_context(out, left->data.index.index, 0, ctx);
                fprintf(out, " = ");
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            } else {
                int is_primitive = 0;
                if (left->data.index.target->type == AST_IDENTIFIER && ctx) {
                    InferredType targetType = get_variable_type(ctx, left->data.index.target->data.identifier.name);
                    if (targetType == TYPE_INT || targetType == TYPE_FLOAT) {
                        is_primitive = 1;
                    }
                }
                
                if (is_primitive) {
                    fprintf(out, "    /* ERROR: primitive type does not support indexing */\n");
                } else {
                    if (left->data.index.target->type == AST_IDENTIFIER && ctx) {
                        InferredType targetType = get_variable_type(ctx, left->data.index.target->data.identifier.name);
                        if (targetType == TYPE_STRUCT) {
                            fprintf(out, "    (");
                            emit_expression_with_context(out, left->data.index.target, 0, ctx);
                            fprintf(out, ").%s = ", field_name);
                            emit_expression_with_context(out, right, 0, ctx);
                            fprintf(out, ";\n");
                        } else if (targetType == TYPE_LIST) {
                            fprintf(out, "    ");
                            emit_expression_with_context(out, left->data.index.target, 0, ctx);
                            fprintf(out, "[");
                            emit_expression_with_context(out, left->data.index.index, 0, ctx);
                            fprintf(out, "] = ");
                            emit_expression_with_context(out, right, 0, ctx);
                            fprintf(out, ";\n");
                        } else {
                            fprintf(out, "    ");
                            emit_expression_with_context(out, left->data.index.target, 0, ctx);
                            fprintf(out, "[");
                            emit_expression_with_context(out, left->data.index.index, 0, ctx);
                            fprintf(out, "] = ");
                            emit_expression_with_context(out, right, 0, ctx);
                            fprintf(out, ";\n");
                        }
                    } else {
                        fprintf(out, "    ");
                        emit_expression_with_context(out, left->data.index.target, 0, ctx);
                        fprintf(out, " . ");
                        emit_expression_with_context(out, left->data.index.index, 0, ctx);
                        fprintf(out, " = ");
                        emit_expression_with_context(out, right, 0, ctx);
                        fprintf(out, ";\n");
                    }
                }
            }
        } else {
            int is_struct = 0;
            int is_list = 0;
            if (left->data.index.target->type == AST_IDENTIFIER && ctx) {
                InferredType targetType = get_variable_type(ctx, left->data.index.target->data.identifier.name);
                if (targetType == TYPE_STRUCT) {
                    is_struct = 1;
                } else if (targetType == TYPE_LIST) {
                    is_list = 1;
                }
            }
            else if (left->data.index.target->type == AST_STRUCT_LITERAL)
            {
                is_struct = 1;
            }
            else if (left->data.index.target->type == AST_EXPRESSION_LIST) {
                is_list = 1;
            }
            
            if (is_struct) {
                fprintf(out, "    /* ERROR: struct member access requires identifier */\n");
            } else if (is_list) {
                fprintf(out, "    ");
                emit_expression_with_context(out, left->data.index.target, 0, ctx);
                fprintf(out, ")[");
                emit_expression_with_context(out, left->data.index.index, 0, ctx);
                fprintf(out, "]= ");
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            } else {
                int is_primitive = 0;
                if (left->data.index.target->type == AST_IDENTIFIER && ctx) {
                    InferredType targetType = get_variable_type(ctx, left->data.index.target->data.identifier.name);
                    if (targetType == TYPE_INT || targetType == TYPE_FLOAT) {
                        is_primitive = 1;
                    }
                }
                
                if (is_primitive) {
                    fprintf(out, "    /* ERROR: primitive type does not support indexing */\n");
                } else {
                    fprintf(out, "    ");
                    emit_expression_with_context(out, left->data.index.target, 0, ctx);
                    fprintf(out, "[");
                    emit_expression_with_context(out, left->data.index.index, 0, ctx);
                    fprintf(out, "] = ");
                    emit_expression_with_context(out, right, 0, ctx);
                    fprintf(out, ";\n");
                }
            }
        }
        return;
    }
    
    if (left->type == AST_MEMBER_ACCESS) {
        if (left->data.member_access.field && left->data.member_access.field->type == AST_IDENTIFIER) {
            const char* field_name = left->data.member_access.field->data.identifier.name;
            if (strcmp(field_name, "length") == 0) {
                fprintf(out, "    // Error: 'length' field is read-only\n");
                return;
            }
            int is_struct = 0;
            int is_list = 0;
            if (left->data.member_access.object->type == AST_IDENTIFIER && ctx) {
                InferredType targetType = get_variable_type(ctx, left->data.member_access.object->data.identifier.name);
                if (targetType == TYPE_STRUCT) {
                    is_struct = 1;
                } else if (targetType == TYPE_LIST) {
                    is_list = 1;
                }
            }
            else if (left->data.member_access.object->type == AST_STRUCT_LITERAL) {
                is_struct = 1;
            }
            else if (left->data.member_access.object->type == AST_EXPRESSION_LIST) {
                is_list = 1;
            }
            
            if (is_struct) {
                fprintf(out, "    (");
                emit_expression_with_context(out, left->data.member_access.object, 0, ctx);
                fprintf(out, ").%s = ", field_name);
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            } else if (is_list) {
                fprintf(out, "    (");
                emit_expression_with_context(out, left->data.member_access.object, 0, ctx);
                fprintf(out, ") .");
                emit_expression_with_context(out, left->data.member_access.field, 0, ctx);
                fprintf(out, " = ");
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            } else {
                fprintf(out, "    (");
                emit_expression_with_context(out, left->data.member_access.object, 0, ctx);
                fprintf(out, ") .");
                emit_expression_with_context(out, left->data.member_access.field, 0, ctx);
                fprintf(out, " = ");
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            }
        } else {
            fprintf(out, "    /* ERROR: struct member access requires identifier */\n");
        }
        return;
    }
    
    if (left->type != AST_IDENTIFIER) return;
    const char* name = left->data.identifier.name;
    int var_index = -1;
    unsigned int hash = 0;
    for (const char* p = name; *p; p++) {
        hash = hash * 31 + *p;
    }
    var_index = hash % MAX_VARS;
    InferredType t = ctx ? get_variable_type(ctx, name) : TYPE_UNKNOWN;
    const char* tstr = type_to_cpp_string(t);
    int is_void_call = 0;
    if (right->type == AST_CALL && right->data.call.func->type == AST_INDEX) {
        ASTNode* method = right->data.call.func->data.index.index;
        if (method && method->type == AST_IDENTIFIER) {
            const char* mname = method->data.identifier.name;
            if (strcmp(mname, "add!") == 0 || 
                strcmp(mname, "push!") == 0 || 
                strcmp(mname, "replace!") == 0 || 
                strcmp(mname, "insert!") == 0) {
                is_void_call = 1;
            }
        }
    }
    
    if (right->type == AST_INPUT) {
        if (var_index >= 0 && var_index < MAX_VARS && !decl[var_index]) {
            if (t != TYPE_UNKNOWN) {
                fprintf(out, "    %s %s;\n", tstr, name);
            } else {
                fprintf(out, "    VString %s;\n", name);
            }
            decl[var_index] = 1;
        }
        fprintf(out, "    {\n");
        fprintf(out, "        std::string __in_tmp;\n");
        if (right->data.input.prompt) {
            fprintf(out, "        std::cout << to_vstring(");
            emit_expression_with_context(out, right->data.input.prompt, 0, ctx);
            fprintf(out, ");\n");
        }
        fprintf(out, "        std::getline(std::cin, __in_tmp);\n");
        fprintf(out, "        %s = to_vstring(__in_tmp);\n", name);
        fprintf(out, "    }\n");
        return;
    }
    if (is_void_call) {
        fprintf(out, "    ");
        emit_expression_with_context(out, right, 0, ctx);
        fprintf(out, ";\n");
        return;
    }
    if (var_index >= 0 && var_index < MAX_VARS && !decl[var_index]) {
        if (t != TYPE_UNKNOWN && t != TYPE_STRUCT) {
            fprintf(out, "    %s %s = ", tstr, name);
        } else if (t == TYPE_STRUCT) {
            if (right->type == AST_STRUCT_LITERAL && right->data.struct_literal.type_name) {
                const char* struct_type_name = right->data.struct_literal.type_name->data.identifier.name;
                fprintf(out, "    %s %s = ", struct_type_name, name);
            } else {
                fprintf(out, "    auto %s = ", name);
            }
        } else {
            fprintf(out, "    auto %s = ", name);
        }
        emit_expression_with_context(out, right, 0, ctx);
        fprintf(out, ";\n");
        decl[var_index] = 1;
    } else {
        fprintf(out, "    %s = ", name);
        emit_expression_with_context(out, right, 0, ctx);
        fprintf(out, ";\n");
    }
}

static void compile_if(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node) {
    fprintf(out, "    if (");
    emit_expression_with_context(out, node->data.if_stmt.condition, 0, ctx);
    fprintf(out, ") {\n");
    compile_node(gen, ctx, out, decl, node->data.if_stmt.then_body);
    fprintf(out, "    }");
    if (node->data.if_stmt.else_body) {
        if (node->data.if_stmt.else_body->type == AST_IF) {
            fprintf(out, " else ");
            compile_if(gen, ctx, out, decl, node->data.if_stmt.else_body);
        } else {
            fprintf(out, " else {\n");
            compile_node(gen, ctx, out, decl, node->data.if_stmt.else_body);
            fprintf(out, "    }\n");
            return;
        }
    } else {
        fprintf(out, "\n");
    }
}

static void compile_while(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node) {
    fprintf(out, "    while (");
    emit_expression_with_context(out, node->data.while_stmt.condition, 0, ctx);
    fprintf(out, ") {\n");
    compile_node(gen, ctx, out, decl, node->data.while_stmt.body);
    fprintf(out, "    }\n");
}

static void compile_for(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node) {
    if (node->data.for_stmt.var->type != AST_IDENTIFIER) return;
    const char* var_name = node->data.for_stmt.var->data.identifier.name;
    int var_index = -1;
    if (gen) {
        for (int i = 0; i < gen->var_count; i++) {
            if (strcmp(gen->variables[i], var_name) == 0) {
                var_index = i;
                break;
            }
        }
    }
    char loop_var_name[256];
    snprintf(loop_var_name, sizeof(loop_var_name), "%s", var_name);
    if (var_index >= 0 && var_index < MAX_VARS) {
        decl[var_index] = 1;
    }
    if (node->data.for_stmt.end == NULL) {
        fprintf(out, "    for (long long %s = 0; %s < (long long)(", loop_var_name, loop_var_name);
        emit_expression_with_context(out, node->data.for_stmt.start, 0, ctx);
        fprintf(out, ").items.size(); %s++) {\n", loop_var_name);
        compile_node(gen, ctx, out, decl, node->data.for_stmt.body);
        fprintf(out, "    }\n");
        return;
    }

    if (node->data.for_stmt.start &&
        node->data.for_stmt.start->type == AST_IDENTIFIER &&
        strcmp(node->data.for_stmt.start->data.identifier.name, "char") == 0 &&
        node->data.for_stmt.end != NULL) {

        fprintf(out, "    {\n");
        fprintf(out, "        vtypes::VString __iter = to_vstring(");
        emit_expression_with_context(out, node->data.for_stmt.end, 0, ctx);
        fprintf(out, ");\n");
        fprintf(out, "        for (size_t __idx = 0; __idx < __iter.size(); ++__idx) {\n");
        fprintf(out, "            char %s = __iter[__idx];\n", loop_var_name);
        compile_node(gen, ctx, out, decl, node->data.for_stmt.body);
        fprintf(out, "        }\n");
        fprintf(out, "    }\n");
        return;
    }
    fprintf(out, "    for (long long %s = ", loop_var_name);
    emit_expression_with_context(out, node->data.for_stmt.start, 0, ctx);
    fprintf(out, "; %s < ", loop_var_name);
    emit_expression_with_context(out, node->data.for_stmt.end, 0, ctx);
    fprintf(out, "; %s++) {\n", loop_var_name);
    compile_node(gen, ctx, out, decl, node->data.for_stmt.body);
    fprintf(out, "    }\n");
}

static const char* get_param_type_string(TypeInferenceContext* ctx, ASTNode* param) {
    if (!param) return "auto";
    
    if (param->type == AST_IDENTIFIER) {
        InferredType t = ctx ? get_variable_type(ctx, param->data.identifier.name) : TYPE_UNKNOWN;
        if (t != TYPE_UNKNOWN) {
            const char* base_type = type_to_cpp_string(t);
            return base_type;
        }
        return "auto";
    }
    else if (param->type == AST_ASSIGN && param->data.assign.left->type == AST_IDENTIFIER) {
        ASTNode* right = param->data.assign.right;
        if (right->type == AST_UNARYOP && right->data.unaryop.op == OP_ADDRESS) {
            ASTNode* base_type_node = right->data.unaryop.expr;
            if (base_type_node->type == AST_TYPE_INT32 || base_type_node->type == AST_TYPE_INT64) {
                return "long long*";//use ptr not ref
            } else if (base_type_node->type == AST_TYPE_FLOAT32) {
                return "float*";
            } else if (base_type_node->type == AST_TYPE_FLOAT64) {
                return "double*";
            } else if (base_type_node->type == AST_TYPE_STRING) {
                return "VString*";
            } else {
                return "auto*";
            }
        }
        else if (param->mutability == MUTABILITY_MUTABLE) {
            if (right->type == AST_TYPE_INT32 || right->type == AST_TYPE_INT64) {
                return "long long*";
            } else if (right->type == AST_TYPE_FLOAT32 || right->type == AST_TYPE_FLOAT64) {
                return "float*";
            } else if (right->type == AST_TYPE_STRING) {
                return "VString*";
            } else if (right->type == AST_TYPE_LIST) {
                return "VList*";
            }
        }
        else if (right->type == AST_TYPE_LIST) {
            return "VList&";
        }
        else if (right->type == AST_TYPE_INT32 || right->type == AST_TYPE_INT64) {
            return "long long";
        }
        else if (right->type == AST_TYPE_FLOAT32 || right->type == AST_TYPE_FLOAT64) {
            return "float&";
        }
        else if (right->type == AST_TYPE_STRING) {
            return "VString&";
        }
        else if (right->type == AST_TYPE_POINTER) {
            return "auto&";
        }
        else if (right->type == AST_IDENTIFIER) {
            return right->data.identifier.name;
        }
    }
    else if (param->type == AST_ASSIGN && param->data.assign.right->type == AST_IDENTIFIER) {
        ASTNode* right = param->data.assign.right;
        if (right->type == AST_IDENTIFIER) {
            InferredType t = ctx ? get_variable_type(ctx, right->data.identifier.name) : TYPE_UNKNOWN;
            if (t != TYPE_UNKNOWN) {
                const char* base_type = type_to_cpp_string(t);
                if (strcmp(base_type, "long long") == 0) {
                    return "long long*";
                } else if (strcmp(base_type, "float") == 0) {
                    return "float*";
                } else if (strcmp(base_type, "double") == 0) {
                    return "double*";
                } else if (strcmp(base_type, "VString") == 0) {
                    return "VString*";
                } else {
                    return "auto*";
                }
            }
        }
    }
    
    return "auto";
}

static void compile_function(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node) {
    if (node->type != AST_FUNCTION) return;
    fprintf(out, "auto %s(", node->data.function.name);
    if (node->data.function.params && node->data.function.params->type == AST_EXPRESSION_LIST) {
        for (int i = 0; i < node->data.function.params->data.expression_list.expression_count; i++) {
            if (i > 0) fprintf(out, ", ");
            ASTNode* param = node->data.function.params->data.expression_list.expressions[i];
            const char* param_type = get_param_type_string(ctx, param);
            
            if (param->type == AST_IDENTIFIER) {
                fprintf(out, "%s %s", param_type, param->data.identifier.name);
            }
            else if (param->type == AST_ASSIGN && param->data.assign.left->type == AST_IDENTIFIER) {
                const char* param_name = param->data.assign.left->data.identifier.name;
                fprintf(out, "%s %s", param_type, param_name);
            }
            else {
                fprintf(out, "auto %s", param->data.identifier.name);
            }
        }
    }
    fprintf(out, ") {\n");
    if (node->data.function.body) {
        int temp_decl[MAX_VARS];
        for (int i = 0; i < MAX_VARS; i++) {
            temp_decl[i] = decl[i];
        }
        int has_return = check_function_has_return(node->data.function.body);
        
        compile_node(gen, ctx, out, decl, node->data.function.body);
        for (int i = 0; i < MAX_VARS; i++) {
            decl[i] = temp_decl[i];
        }
        if (!has_return) {
            fprintf(out, "    return 0;\n");
        }
    } else {
        fprintf(out, "    return 0;\n");
    }
    
    fprintf(out, "}\n\n");
}

static int check_function_has_return(ASTNode* node) {
    if (!node) return 0;
    
    if (node->type == AST_RETURN) {
        return 1;
    }
    
    if (node->type == AST_PROGRAM) {
        for (int i = 0; i < node->data.program.statement_count; i++) {
            if (check_function_has_return(node->data.program.statements[i])) {
                return 1;
            }
        }
    } else if (node->type == AST_IF) {
        if (check_function_has_return(node->data.if_stmt.then_body)) {
            return 1;
        }
        if (node->data.if_stmt.else_body && 
            check_function_has_return(node->data.if_stmt.else_body)) {
            return 1;
        }
    } else if (node->type == AST_WHILE) {
        if (check_function_has_return(node->data.while_stmt.body)) {
            return 1;
        }
    } else if (node->type == AST_FOR) {
        if (check_function_has_return(node->data.for_stmt.body)) {
            return 1;
        }
    }
    
    return 0;
}

static void compile_node(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            compile_program(gen, ctx, out, decl, node);
            break;
        case AST_PRINT:
            compile_print(out, node, ctx);
            break;
        case AST_ASSIGN:
            compile_assign(ctx, out, decl, node);
            break;
        case AST_CONST:
            compile_const(gen, ctx, out, decl, node);
            break;
        case AST_IF:
            compile_if(gen, ctx, out, decl, node);
            break;
        case AST_WHILE:
            compile_while(gen, ctx, out, decl, node);
            break;
        case AST_FOR:
            compile_for(gen, ctx, out, decl, node);
            break;
        case AST_BREAK:
            fprintf(out, "    break;\n");
            break;
        case AST_CONTINUE:
            fprintf(out, "    continue;\n");
            break;
        case AST_FUNCTION:
            compile_function(gen, ctx, out, decl, node);
            break;
        case AST_CALL: {
            fprintf(out, "    ");
            emit_expression_with_context(out, node, 0, ctx);
            fprintf(out, ";\n");
            break;
        }
        case AST_RETURN: {
            fprintf(out, "    return ");
            if (node->data.return_stmt.expr) {
                emit_expression_with_context(out, node->data.return_stmt.expr, 0, ctx);
            } else {
                fprintf(out, "0");
            }
            fprintf(out, ";\n");
            break;
        }
        case AST_EXPRESSION_LIST:
            break;
        default:
            break;
    }
}

static void compile_const(ByteCodeGen* gen, TypeInferenceContext* ctx, FILE* out, int* decl, ASTNode* node) {
    ASTNode* left = node->data.assign.left;
    ASTNode* right = node->data.assign.right;
    if (left->type != AST_IDENTIFIER) return;
    const char* name = left->data.identifier.name;
    int idx = get_variable_index(gen, name);
    InferredType t = ctx ? get_variable_type(ctx, name) : TYPE_UNKNOWN;
    const char* tstr = type_to_cpp_string(t);
    if (idx >= 0 && idx < MAX_VARS) {
        if (!decl[idx]) {
            if (t != TYPE_UNKNOWN) {
                fprintf(out, "    const %s %s = ", tstr, name);
                if ((t == TYPE_INT) && right->type == AST_STRING) {
                    fprintf(out, "to_int(");
                    emit_expression_with_context(out, right, 0, ctx);
                    fprintf(out, ")");
                } else if ((t == TYPE_FLOAT) && right->type == AST_STRING) {
                    fprintf(out, "to_double(");
                    emit_expression_with_context(out, right, 0, ctx);
                    fprintf(out, ")");
                } else if ((t == TYPE_STRING) && (right->type == AST_NUM_INT || right->type == AST_NUM_FLOAT)) {
                    fprintf(out, "to_vstring(");
                    emit_expression_with_context(out, right, 0, ctx);
                    fprintf(out, ")");
                } else {
                    emit_expression_with_context(out, right, 0, ctx);
                }
                fprintf(out, ";\n");
            } else {
                fprintf(out, "    const auto %s = ", name);
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            }
            decl[idx] = 1;
        } else {
            fprintf(out, "    /* error: redeclaration of const %s */\n", name);
        }
    } else {
        if (t != TYPE_UNKNOWN) {
            fprintf(out, "    const %s %s = ", tstr, name);
            if ((t == TYPE_INT) && right->type == AST_STRING) {
                fprintf(out, "to_int(");
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ")");
            } else if ((t == TYPE_FLOAT) && right->type == AST_STRING) {
                fprintf(out, "to_double(");
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ")");
            } else if ((t == TYPE_STRING) && (right->type == AST_NUM_INT || right->type == AST_NUM_FLOAT)) {
                fprintf(out, "to_vstring(");
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ")");
            } else {
                emit_expression_with_context(out, right, 0, ctx);
                fprintf(out, ";\n");
            }
        } else {
            fprintf(out, "    const auto %s = ", name);
            emit_expression_with_context(out, right, 0, ctx);
            fprintf(out, ";\n");
        }
    }
}