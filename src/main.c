/*
vix 语言 0.0.1版本完工
*/
//操,发现之前写的命名太长了，改了一点函数和变量的名字
/*
output_filename -> out_f
input_filename -> in_f
qbe_ir_filename -> qbe_f
llvm_ir_filename -> llvm_f
obj_filename -> obj_f
bytecode_filename -> bc_f
cpp_filename -> cpp_f
enable_debug_log -> dbg
is_vic_file -> is_vic
save_cpp_file -> save_c
keep_cpp_file -> keep_c
generate_llvm_ir -> gen_llvm
generate_object_file -> gen_obj
output_ast_only -> out_ast
bare_metal_mode -> bare
has_explicit_output_mode -> exp_mode
target_triple -> target
effective_target -> eff_t
compile_command -> ccmd
compile_result -> cres
llc_cmd -> lcmd
type_ctx -> t_ctx
global_table -> g_tbl
semantic_errors -> errs
\(^o^)/
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/ast.h"
#include "../include/parser.h"
#include "../include/compiler.h"
#include "../include/vic-ir/mir.h"
#include "../include/llvm_emit.h"
#include "../include/semantic.h"

extern FILE* yyin;
extern ASTNode* root;
void create_lib_files();
void analyze_ast(TypeInferenceContext* ctx, ASTNode* node);
const char* current_input_filename = NULL;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <input.vix> [-o output_file]\n", argv[0]);
        fprintf(stderr, "       %s <input.vix>  -o <output_file> [-kt]\n", argv[0]);
        fprintf(stderr, "       %s <input.vix>  -ir <vic_ir_file>\n", argv[0]);
        fprintf(stderr, "       %s <input.vix> -ll <llvm_ir_file>\n", argv[0]);
        fprintf(stderr, "       %s <input.vix> -obj [obj_file] (output object file via llc)\n", argv[0]);
        fprintf(stderr, "       %s <input.vix> -ast (output AST only)\n", argv[0]);
        fprintf(stderr, "       %s <input.vix> -ll (output LLVM IR only)\n", argv[0]);
        fprintf(stderr, "       %s <input.vix> -llvm (output LLVM IR only)\n", argv[0]);
        fprintf(stderr, "       %s <input.vix> --debug (enable debug logs)\n", argv[0]);
        fprintf(stderr, "       %s <input.vix> --target=<triple> (set codegen/link target)\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "init") == 0) {
        create_lib_files();
        return 0;
    }
    
    char* out_f = NULL;
    char* vic_f = NULL;
    char* llvm_f = NULL;
    char* obj_f = NULL;
    char* in_f = NULL;
    int is_vic = 0;
    int save_c = 0;
    int keep_c = 0;
    int gen_vic = 0;
    int gen_llvm = 0;
    int gen_obj = 0;
    int out_ast = 0;
    int out_llvm = 0;
    int dbg = 0;
    char* target = NULL;
    int no_std = 0;
    int no_main = 0;
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' && strcmp(argv[i], "init") != 0) {
            in_f = argv[i];
            break;
        }
    }
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--target=", 9) == 0) {
            target = argv[i] + 9;
        } else if (strcmp(argv[i], "--target") == 0) {
            if (i + 1 < argc) {
                target = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Er: --target option requires a target triple\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                out_f = argv[i + 1];
                save_c = 1;
                i++; 
            } else {
                fprintf(stderr, "Er: -o option requires a filename\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0 || strcmp(argv[i] , "-ver") == 0){
            printf("Vix Compiler 0.1.0_rc1_2 (Beta_26.01.01) by:Mincx1203 Copyright(c) 2025-2026\n");
            return 0;
        } else if (strcmp(argv[i], "-ir") == 0) {
            if (i + 1 < argc) {
                vic_f = argv[i + 1];
                gen_vic = 1;
                i++;
            } else {
                fprintf(stderr, "Er: -ir option requires a filename\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-llvm") == 0) {
            out_llvm = 1;
        } else if (strcmp(argv[i], "-obj") == 0) {
            gen_obj = 1;
            gen_llvm = 1;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                obj_f = argv[i + 1];
                i++;
            }
        } else if (strcmp(argv[i], "-ll") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                llvm_f = argv[i + 1];
                gen_llvm = 1;
                i++;
            } else {
                out_llvm = 1;
            }
        } else if (strcmp(argv[i], "-kt") == 0) {
            keep_c = 1;
        } else if (strcmp(argv[i], "-ast") == 0) {
            out_ast = 1;
        } else if (strcmp(argv[i], "--debug") == 0) {
            dbg = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            fprintf(stderr, "usage: %s <input.vix> [-o output_file]\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> [-o output_file] [-kt]\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> -ir <vic_ir_file>\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> -llvm <llvm_ir_file>\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> -ll <llvm_ir_file>\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> -obj [obj_file] (output object file via llc)\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> -ast (output AST only)\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> -ll (output LLVM IR only)\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> -llvm (output LLVM IR only)\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> --debug (enable debug logs)\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> --target=<triple> (set codegen/link target, e.g. x86_64-unknown-none)\n", argv[0]);
            fprintf(stderr, "       %s <input.vix> (LLVM backend is the default backend)\n", argv[0]);
            return 0;
        } else if (argv[i][0] == '-' && strcmp(argv[i], "-") != 0) {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s <input.vix> [-o output_file] [-kt] [-ir vic_file] [-llvm [llvm_file]] [-ll [llvm_file]] [-obj [obj_file]] [-ast] [--debug] [--target=<triple>]\n", argv[0]);
            return 1;
        } else {
            is_vic = strlen(argv[i]) > 4 && strcmp(argv[i] + strlen(argv[i]) - 4, ".vic") == 0;
        }
    }
    setenv("VIX_DEBUG", dbg ? "1" : "0", 1);//通过环境变量控制调试输出
    if (!in_f) {
        in_f = argv[1];
    }
    int exp_mode =
        out_ast ||
        out_llvm ||
        gen_vic ||
        gen_llvm ||
        gen_obj;

    if (!exp_mode &&
        !out_f &&
        save_c == 0) {
        char* dot = strrchr(in_f, '.');
        if (dot) {
            size_t len = dot - in_f;
            char* def_out = malloc(len + 1);
            if (def_out) {
                strncpy(def_out, in_f, len);
                def_out[len] = '\0';
                out_f = def_out;
                save_c = 1;
            }
        } else {
            out_f = in_f;
            save_c = 1;
        }
    }
    
    FILE* input_file = fopen(in_f, "r");
    if (!input_file) {
        perror("Failed to open file");
        if (out_f && out_f != in_f && out_f != argv[1]) {
            free(out_f);
        }
        return 1;
    }

    {
        char buf[1024];
        while (fgets(buf, sizeof(buf), input_file) != NULL) {
            if (strstr(buf, "#[no_std]") != NULL) {
                no_std = 1;
            }
            if (strstr(buf, "#[no_main]") != NULL) {
                no_main = 1;
            }
        }
        rewind(input_file);
    }

    const char* eff_t = target;
    if (!eff_t && (no_std || no_main)) {
        eff_t = "x86_64-unknown-none";
    }
    llvm_set_target_triple(eff_t);

    int bare = 0;
    if (eff_t && (strstr(eff_t, "unknown-none") != NULL || strstr(eff_t, "unknow-noe") != NULL)) {
        bare = 1;
    }
    if (no_std || no_main) {
        bare = 1;
    }
    
    if (save_c) {
        gen_llvm = 1;
        if (!llvm_f) {
            char* dot = strrchr(out_f, '.');
            if (dot) {
                size_t len = dot - out_f;
                char* llvm_name = malloc(len + 4);
                if (llvm_name) {
                    strncpy(llvm_name, out_f, len);
                    llvm_name[len] = '\0';
                    strcat(llvm_name, ".ll");
                    llvm_f = llvm_name;
                }
            } else {
                char* llvm_name = malloc(strlen(out_f) + 4);
                if (llvm_name) {
                    strcpy(llvm_name, out_f);
                    strcat(llvm_name, ".ll");
                    llvm_f = llvm_name;
                }
            }
        }
    }

    if (is_vic && gen_llvm) {
        char llvm_filename[256];
        if (strstr(llvm_f, ".ll") == NULL) {
            snprintf(llvm_filename, sizeof(llvm_filename), "%s.ll", llvm_f);
        } else {
            strcpy(llvm_filename, llvm_f);
        }
        
        FILE* llvm_file = fopen(llvm_filename, "w");
        if (!llvm_file) {
            fprintf(stderr, "Er: Cannot open LLVM IR file %s for writing\n", llvm_filename);
            fclose(input_file);
            return 1;
        }
        
        llvm_emit_from_ast(root, llvm_file);
        fclose(llvm_file);
        fclose(input_file);
        return 0;
    }
    
    current_input_filename = in_f;
    load_source_file(in_f);
    set_location_with_column(in_f, 1, 1);
    yyin = input_file;
    
    int result = yyparse();
    if (result == 0 && root) {
        inline_imports(root);
    }
    
    if (result == 0) {
        int errs = check_undefined_symbols(root);
        if (errs > 0) {
            fprintf(stderr, "Error: Found %d semantic error(s)\n", errs);
            if (root) {
                free_ast(root);
            }
            cleanup_error_handler();
            fclose(input_file);
            return 1;
        }
        SymbolTable* g_tbl = create_symbol_table(NULL);
        int uvars = check_unused_variables(root, g_tbl);
        destroy_symbol_table(g_tbl);
        if (uvars > 0) {
            fprintf(stderr, "\033[33mFound %d unused variable(s)\033[0m\n", uvars);
        }
        if (get_error_count() > 0) {
            fprintf(stderr, "Compilation failed with %d error(s)\n", get_error_count());
            if (root) {
                free_ast(root);
            }
            cleanup_error_handler();
            fclose(input_file);
            return 1;
        }

        if (gen_vic) {
            char vic_filename[256];
            if (strstr(vic_f, ".vic") == NULL) {
                snprintf(vic_filename, sizeof(vic_filename), "%s.vic", vic_f);
            } else {
                strcpy(vic_filename, vic_f);
            }
            FILE* vic_file = fopen(vic_filename, "w");
            if (!vic_file) {
                fclose(input_file);
                return 1;
            }
            vic_gen(root, vic_file);
            fclose(vic_file);
            if (root) free_ast(root);
            fclose(input_file);
            return 0;
        }
        
        if (gen_llvm) {
            char llvm_filename[2048];
            if (!llvm_f) {
                char* dot = strrchr(in_f, '.');
                if (dot) {
                    size_t len = dot - in_f;
                    snprintf(llvm_filename, sizeof(llvm_filename), "%.*s.ll", (int)len, in_f);
                } else {
                    snprintf(llvm_filename, sizeof(llvm_filename), "%s.ll", in_f);
                }
                llvm_f = llvm_filename;
            } else {
                if (strstr(llvm_f, ".ll") == NULL) {
                    snprintf(llvm_filename, sizeof(llvm_filename), "%s.ll", llvm_f);
                    llvm_f = llvm_filename;
                }
            }
            
            FILE* llvm_file = fopen(llvm_f, "w");
            if (!llvm_file) {
                fprintf(stderr, "Error: Cannot open LLVM IR file %s for writing\n", llvm_f);
                fclose(input_file);
                return 1;
            }
            
            llvm_emit_from_ast(root, llvm_file);
            fclose(llvm_file);

            if (get_error_count() > 0) {
                fprintf(stderr, "Compilation failed with %d error(s)\n", get_error_count());
                if (!keep_c) {
                    remove(llvm_f);
                }
                if (root) {
                    free_ast(root);
                }
                cleanup_error_handler();
                fclose(input_file);
                return 1;
            }

            if (gen_obj) {
                char oname[2048];
                const char* fobj = obj_f;
                if (!fobj) {
                    char* dot = strrchr(in_f, '.');
                    if (dot) {
                        size_t len = dot - in_f;
                        snprintf(oname, sizeof(oname), "%.*s.o", (int)len, in_f);
                    } else {
                        snprintf(oname, sizeof(oname), "%s.o", in_f);
                    }
                    fobj = oname;
                } else if (strstr(fobj, ".o") == NULL) {
                    snprintf(oname, sizeof(oname), "%s.o", fobj);
                    fobj = oname;
                }
                char lcmd[8192];
                if (eff_t) {
                    snprintf(lcmd, sizeof(lcmd),
                             "llc -filetype=obj -relocation-model=%s -mtriple=%s %s -o %s",
                             bare ? "static" : "pic",
                             eff_t, llvm_f, fobj);
                } else {
                    snprintf(lcmd, sizeof(lcmd),
                             "llc -filetype=obj -relocation-model=%s %s -o %s",
                             bare ? "static" : "pic",
                             llvm_f, fobj);
                }

                int lres = system(lcmd);
                if (lres != 0) {
                    fprintf(stderr, "Error: Failed to compile LLVM IR to object file via llc\n");
                    fclose(input_file);
                    return 1;
                }

                if (!save_c) {
                    if (root) {
                        free_ast(root);
                    }
                    fclose(input_file);
                    return 0;
                }
            }
            if (out_f && save_c) {
                const char* ls = "linker.ld";
                if (bare && access(ls, R_OK) != 0 && access("src/linker.ld", R_OK) == 0) {
                    ls = "src/linker.ld";
                }

                size_t ccmd_sz = 8192;
                char *ccmd = malloc(ccmd_sz);
                if (ccmd == NULL) {
                    fprintf(stderr, "Er: Failed to allocate memory for clang command\n");
                    fclose(input_file);
                    return 1;
                }

                if (bare) {
                    const char* f_t = eff_t ? eff_t : "x86_64-unknown-none";
                    snprintf(ccmd, ccmd_sz,
                             "clang -O2 %s -o %s -target %s -ffreestanding -fno-builtin -fno-pic -fno-pie -no-pie -nostdlib -nostartfiles -nodefaultlibs -static -Wl,--build-id=none -Wl,--no-dynamic-linker -Wl,-z,max-page-size=0x1000 -Wl,-e,_start -Wl,-T,%s",
                             llvm_f, out_f, f_t, ls);
                } else if (eff_t) {
                    snprintf(ccmd, ccmd_sz,
                             "clang -O2 %s -o %s -target %s $(llvm-config --ldflags --libs all) -lm -lstdc++",
                             llvm_f, out_f, eff_t);
                } else {
                    snprintf(ccmd, ccmd_sz, "clang -O2 %s -o %s $(llvm-config --ldflags --libs all) -lm -lstdc++", llvm_f, out_f);
                }
                
                int cres = system(ccmd);
                if (cres != 0) {
                    fprintf(stderr, "Error: Failed to compile LLVM IR to executable\n");
                    free(ccmd);
                    fclose(input_file);
                    return 1;
                }
                
                free(ccmd);
                
                if (!keep_c) {
                    remove(llvm_f);
                }
            }
            
            if (root) {
                free_ast(root);
            }//福瑞
            fclose(input_file);
            if (out_f && out_f != in_f && out_f != argv[1] && out_f != llvm_f) {
            }
            
            return 0;
        }

        if (out_ast) {
            printf("===========================AST=======================\n");
            print_ast(root, 0);
            printf("===================================================\n");
            if (root) free_ast(root);
            fclose(input_file);
            return 0;
        }

        if (out_llvm) {
            printf("=========================LLVM IR===================\n");
            llvm_emit_from_ast(root, stdout);
            printf("===================================================\n");
            if (root) free_ast(root);
            fclose(input_file);
            return 0;
        }
    } else {
        if (get_error_count() == 0) {
            const char* fname = current_input_filename ? current_input_filename : "unknown";
            report_syntax_error_with_location("parsing failed due to syntax errors", fname, 1);
        }
    }
    
    if (root) {
        free_ast(root);
    }
    cleanup_error_handler();
    fclose(input_file);
    if (out_f && out_f != in_f && out_f != argv[1]) {
        if (out_f != argv[1] && out_f[0] != '-') {
            //我不知道有没有malloc，所以不福瑞
        }
    }
    
    return result;
}

void analyze_ast(TypeInferenceContext* ctx, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.statement_count; i++) {
                analyze_ast(ctx, node->data.program.statements[i]);
            }
            break;
        case AST_CONST:
            infer_type(ctx, node->data.assign.right);
            if (node->data.assign.left->type == AST_IDENTIFIER) {
                set_variable_type(ctx, node->data.assign.left->data.identifier.name,
                    infer_type(ctx, node->data.assign.right));
            }
            analyze_ast(ctx, node->data.assign.left);
            analyze_ast(ctx, node->data.assign.right);
            break;
        case AST_PRINT:
            analyze_ast(ctx, node->data.print.expr);
            break;
            
        case AST_BINOP:
            analyze_ast(ctx, node->data.binop.left);
            analyze_ast(ctx, node->data.binop.right);
            break;
            
        case AST_UNARYOP:
            analyze_ast(ctx, node->data.unaryop.expr);
            break;
            
        case AST_IDENTIFIER: {
            InferredType type = get_variable_type(ctx, node->data.identifier.name);
            if (type == TYPE_UNKNOWN) {
                /* only report if variable truly not declared in context */
                if (!has_variable(ctx, node->data.identifier.name)) {
                    report_undefined_variable_with_location(
                        node->data.identifier.name,
                        current_input_filename ? current_input_filename : "unknown",
                        node->location.first_line
                    );
                }
            }
            break;
        }
            
        default:
            break;
    }
}

void create_lib_files() {
#ifdef _WIN32
    system("mkdir lib 2>nul");
#else
    system("mkdir -p lib 2>/dev/null");
#endif
    FILE* vcore_file = fopen("lib/vcore.hpp", "w");
    if (vcore_file) {
        fprintf(vcore_file, "#ifndef VCORE_HPP\n");
        fprintf(vcore_file, "#define VCORE_HPP\n\n");
        fprintf(vcore_file, "#include <iostream>\n");
        fprintf(vcore_file, "#include <string>\n");
        fprintf(vcore_file, "#include <cstring>\n");
        fprintf(vcore_file, "#include <cctype>\n");
        fprintf(vcore_file, "#include <cstdlib>\n");
        fprintf(vcore_file, "#include <cerrno>\n");
        fprintf(vcore_file, "#include <limits>\n");
        fprintf(vcore_file, "#include <stdexcept>\n");
        fprintf(vcore_file, "#include \"vtypes.hpp\"\n\n");
        fprintf(vcore_file, "namespace vcore {\n");
        fprintf(vcore_file, "    template<typename T>\n");
        fprintf(vcore_file, "    inline void print(const T& value) {\n");
        fprintf(vcore_file, "        std::cout << value << '\\n';\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<>\n");
        fprintf(vcore_file, "    inline void print<bool>(const bool& value) {\n");
        fprintf(vcore_file, "        std::cout << (value ? \"true\" : \"false\") << '\\n';\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<>\n");
        fprintf(vcore_file, "    inline void print<char*>(char* const& value) {\n");
        fprintf(vcore_file, "        if (value == nullptr) {\n");
        fprintf(vcore_file, "            std::cout << \"nullptr\\n\";\n");
        fprintf(vcore_file, "        } else {\n");
        fprintf(vcore_file, "            std::cout << value << '\\n';\n");
        fprintf(vcore_file, "        }\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<>\n");
        fprintf(vcore_file, "    inline void print<const char*>(const char* const& value) {\n");
        fprintf(vcore_file, "        if (value == nullptr) {\n");
        fprintf(vcore_file, "            std::cout << \"nullptr\\n\";\n");
        fprintf(vcore_file, "        } else {\n");
        fprintf(vcore_file, "            std::cout << value << '\\n';\n");
        fprintf(vcore_file, "        }\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<>\n");
        fprintf(vcore_file, "    inline void print<std::string>(const std::string& value) {\n");
        fprintf(vcore_file, "        std::cout << value << '\\n';\n");
        fprintf(vcore_file, "    }\n");
        fprintf(vcore_file, "    template<typename T, typename... Args>\n");
        fprintf(vcore_file, "    inline void print(const T& first, const Args&... args) {\n");
        fprintf(vcore_file, "        std::cout << first;\n");
        fprintf(vcore_file, "        ((std::cout << ' ' << args), ...);\n");
        fprintf(vcore_file, "        std::cout << '\\n';\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<typename T>\n");
        fprintf(vcore_file, "    inline T add(const T& a, const T& b) {\n");
        fprintf(vcore_file, "        return a + b;\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<typename T>\n");
        fprintf(vcore_file, "    inline T sub(const T& a, const T& b) {\n");
        fprintf(vcore_file, "        return a - b;\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<typename T>\n");
        fprintf(vcore_file, "    inline T mul(const T& a, const T& b) {\n");
        fprintf(vcore_file, "        return a * b;\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    template<typename T>\n");
        fprintf(vcore_file, "    inline T div(const T& a, const T& b) {\n");
        fprintf(vcore_file, "        return a / b;\n");
        fprintf(vcore_file, "    }\n\n");
        fprintf(vcore_file, "    inline long long velox_to_int(const vtypes::VString &s) {\n");
        fprintf(vcore_file, "        const char* p = s.c_str();\n");
        fprintf(vcore_file, "        while (*p && std::isspace((unsigned char)*p)) ++p;\n");
        fprintf(vcore_file, "        const char* q = s.c_str() + s.size();\n");
        fprintf(vcore_file, "        while (q > p && std::isspace((unsigned char)*(q - 1))) --q;\n");
        fprintf(vcore_file, "        std::string trimmed(p, q);\n\n");
        fprintf(vcore_file, "        if (trimmed.empty()) return 0;\n\n");
        fprintf(vcore_file, "        char* endptr = nullptr;\n");
        fprintf(vcore_file, "        errno = 0;\n");
        fprintf(vcore_file, "        long long value = std::strtoll(trimmed.c_str(), &endptr, 10);\n");
        fprintf(vcore_file, "        if (endptr == trimmed.c_str()) {\n");
        fprintf(vcore_file, "            return 0;\n");
        fprintf(vcore_file, "        }\n");
        fprintf(vcore_file, "        if (errno == ERANGE) {\n");
        fprintf(vcore_file, "            if (trimmed.size() > 0 && trimmed[0] == '-') {\n");
        fprintf(vcore_file, "                return std::numeric_limits<long long>::min();\n");
        fprintf(vcore_file, "            } else {\n");
        fprintf(vcore_file, "                return std::numeric_limits<long long>::max();\n");
        fprintf(vcore_file, "            }\n");
        fprintf(vcore_file, "        }\n\n");
        fprintf(vcore_file, "        return value;\n");
        fprintf(vcore_file, "    }\n");
        fprintf(vcore_file, "}\n\n");
        fprintf(vcore_file, "#endif\n");
        fclose(vcore_file);
    }
    
    FILE* vtypes_file = fopen("lib/vtypes.hpp", "w");
    if (vtypes_file) {
        fprintf(vtypes_file, "#ifndef VTYPES_HPP\n");
        fprintf(vtypes_file, "#define VTYPES_HPP\n");
        fprintf(vtypes_file, "#include <string>\n");
        fprintf(vtypes_file, "#include <vector>\n");
        fprintf(vtypes_file, "#include <iostream>\n");
        fprintf(vtypes_file, "#include <stdexcept>\n\n");
        fprintf(vtypes_file, "namespace vtypes {\n\n");
        fprintf(vtypes_file, "class VString : public std::string {\n");
        fprintf(vtypes_file, "public:\n");
        fprintf(vtypes_file, "    using std::string::string;\n");
        fprintf(vtypes_file, "    VString() : std::string() {}\n");
        fprintf(vtypes_file, "    VString(const std::string &s) : std::string(s) {}\n");
        fprintf(vtypes_file, "    VString(const char* s) : std::string(s) {}\n\n");
        fprintf(vtypes_file, "    VString operator+(const VString &other) const {\n");
        fprintf(vtypes_file, "        return VString(static_cast<const std::string&>(*this) + static_cast<const std::string&>(other));\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    VString operator+(const std::string &other) const {\n");
        fprintf(vtypes_file, "        return VString(static_cast<const std::string&>(*this) + other);\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    friend VString operator+(const std::string &lhs, const VString &rhs) {\n");
        fprintf(vtypes_file, "        return VString(lhs + static_cast<const std::string&>(rhs));\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    VString operator*(long long times) const {\n");
        fprintf(vtypes_file, "        if (times < 0) throw std::invalid_argument(\"Multiplier must be non-negative\");\n");
        fprintf(vtypes_file, "        VString result;\n");
        fprintf(vtypes_file, "        result.reserve(this->size() * (size_t)times);\n");
        fprintf(vtypes_file, "        for (long long i = 0; i < times; ++i) result += *this;\n");
        fprintf(vtypes_file, "        return result;\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    friend VString operator*(long long times, const VString &s) {\n");
        fprintf(vtypes_file, "        return s * times;\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    VString& operator+=(const VString &other) {\n");
        fprintf(vtypes_file, "        static_cast<std::string&>(*this) += static_cast<const std::string&>(other);\n");
        fprintf(vtypes_file, "        return *this;\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    VString& operator+=(const std::string &other) {\n");
        fprintf(vtypes_file, "        static_cast<std::string&>(*this) += other;\n");
        fprintf(vtypes_file, "        return *this;\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    VString& operator*=(long long times) {\n");
        fprintf(vtypes_file, "        if (times < 0) throw std::invalid_argument(\"Multiplier must be non-negative\");\n");
        fprintf(vtypes_file, "        VString tmp = *this;\n");
        fprintf(vtypes_file, "        this->clear();\n");
        fprintf(vtypes_file, "        for (long long i = 0; i < times; ++i) *this += tmp;\n");
        fprintf(vtypes_file, "        return *this;\n");
        fprintf(vtypes_file, "    }\n\n");
        fprintf(vtypes_file, "    friend std::ostream& operator<<(std::ostream &os, const VString &s) {\n");
        fprintf(vtypes_file, "        os << static_cast<const std::string&>(s);\n");
        fprintf(vtypes_file, "        return os;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "};\n\n");
        fprintf(vtypes_file, "class VList {\n");
        fprintf(vtypes_file, "public:\n");
        fprintf(vtypes_file, "    std::vector<VString> items;\n");
        fprintf(vtypes_file, "    VList() : items(), _scalar_from_string(false) {}\n");
        fprintf(vtypes_file, "    VList(std::initializer_list<VString> init) : items(init), _scalar_from_string(false) {}\n");
        fprintf(vtypes_file, "    size_t size() const { return items.size(); }\n");
        fprintf(vtypes_file, "    VString operator[](size_t i) const { return items.at(i); }\n");
        fprintf(vtypes_file, "    VString& operator[](size_t i) { return items.at(i); }\n");
        fprintf(vtypes_file, "    void add_inplace(size_t idx, const VString& val) {\n");
        fprintf(vtypes_file, "        if (idx > items.size()) idx = items.size();\n");
        fprintf(vtypes_file, "        items.insert(items.begin() + idx, val);\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    VString remove(size_t idx) {\n");
        fprintf(vtypes_file, "        VString v = items.at(idx);\n");
        fprintf(vtypes_file, "        items.erase(items.begin() + idx);\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "        return v;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    VList& remove_inplace(size_t idx) {\n");
        fprintf(vtypes_file, "        items.erase(items.begin() + idx);\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "        return *this;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    void push_inplace(const VString& val) {\n");
        fprintf(vtypes_file, "        items.push_back(val);\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    VString push(const VString& val) {\n");
        fprintf(vtypes_file, "        items.push_back(val);\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "        return val;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    VString pop() {\n");
        fprintf(vtypes_file, "        VString v = items.back();\n");
        fprintf(vtypes_file, "        items.pop_back();\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "        return v;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    VList& pop_inplace() {\n");
        fprintf(vtypes_file, "        if (!items.empty()) {\n");
        fprintf(vtypes_file, "            items.pop_back();\n");
        fprintf(vtypes_file, "        }\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "        return *this;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    void replace_inplace(size_t idx, const VString& val) {\n");
        fprintf(vtypes_file, "        items.at(idx) = val;\n");
        fprintf(vtypes_file, "        _scalar_from_string = false;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    VList& operator=(const VString& s) {\n");
        fprintf(vtypes_file, "        items.clear();\n");
        fprintf(vtypes_file, "        items.push_back(s);\n");
        fprintf(vtypes_file, "        _scalar_from_string = true;\n");
        fprintf(vtypes_file, "        return *this;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    VList& operator=(VString&& s) {\n");
        fprintf(vtypes_file, "        items.clear();\n");
        fprintf(vtypes_file, "        items.push_back(std::move(s));\n");
        fprintf(vtypes_file, "        _scalar_from_string = true;\n");
        fprintf(vtypes_file, "        return *this;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "    friend std::ostream& operator<<(std::ostream &os, const VList &l) {\n");
        fprintf(vtypes_file, "        if (l._scalar_from_string && l.items.size() == 1) {\n");
        fprintf(vtypes_file, "            os << l.items[0];\n");
        fprintf(vtypes_file, "        } else {\n");
        fprintf(vtypes_file, "            os << \"[\";\n");
        fprintf(vtypes_file, "            for (size_t i = 0; i < l.items.size(); ++i) { if (i) os << \", \"; os << l.items[i]; }\n");
        fprintf(vtypes_file, "            os << \"]\";\n");
        fprintf(vtypes_file, "        }\n");
        fprintf(vtypes_file, "        return os;\n");
        fprintf(vtypes_file, "    }\n");
        fprintf(vtypes_file, "private:\n");
        fprintf(vtypes_file, "    bool _scalar_from_string;\n");
        fprintf(vtypes_file, "};\n\n");
        fprintf(vtypes_file, "} // namespace vtypes\n\n");
        fprintf(vtypes_file, "#endif\n");
        fclose(vtypes_file);
    }
    
    FILE* vconvert_file = fopen("lib/vconvert.hpp", "w");
    if (vconvert_file) {
        fprintf(vconvert_file, "#ifndef VCONVERT_HPP\n");
        fprintf(vconvert_file, "#define VCONVERT_HPP\n");
        fprintf(vconvert_file, "#include <string>\n");
        fprintf(vconvert_file, "#include <cstdlib>\n");
        fprintf(vconvert_file, "#include <typeinfo>\n");
        fprintf(vconvert_file, "#include <stdexcept>\n");
        fprintf(vconvert_file, "#include <type_traits>\n");
        fprintf(vconvert_file, "#include <cstdint>\n");
        fprintf(vconvert_file, "#include \"vtypes.hpp\"\n");
        fprintf(vconvert_file, "namespace detail {\n");
        fprintf(vconvert_file, "    inline void reverse_string(char* str, int len) {\n");
        fprintf(vconvert_file, "        int start = 0;\n");
        fprintf(vconvert_file, "        int end = len - 1;\n");
        fprintf(vconvert_file, "        while (start < end) {\n");
        fprintf(vconvert_file, "            char temp = str[start];\n");
        fprintf(vconvert_file, "            str[start] = str[end];\n");
        fprintf(vconvert_file, "            str[end] = temp;\n");
        fprintf(vconvert_file, "            start++;\n");
        fprintf(vconvert_file, "            end--;\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "    }\n\n");
        fprintf(vconvert_file, "    inline int int_to_string(long long num, char* str) {\n");
        fprintf(vconvert_file, "        int i = 0;\n");
        fprintf(vconvert_file, "        bool isNegative = false;\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        if (num == 0) {\n");
        fprintf(vconvert_file, "            str[i++] = '0';\n");
        fprintf(vconvert_file, "            str[i] = '\\0';\n");
        fprintf(vconvert_file, "            return i;\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        if (num < 0) {\n");
        fprintf(vconvert_file, "            isNegative = true;\n");
        fprintf(vconvert_file, "            num = -num;\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        while (num != 0) {\n");
        fprintf(vconvert_file, "            int rem = num %% 10;\n");
        fprintf(vconvert_file, "            str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';\n");
        fprintf(vconvert_file, "            num = num/10;\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        if (isNegative) {\n");
        fprintf(vconvert_file, "            str[i++] = '-';\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        str[i] = '\\0';\n");
        fprintf(vconvert_file, "        reverse_string(str, i);\n");
        fprintf(vconvert_file, "        return i;\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline int double_to_string(double num, char* str, int precision = 6) {\n");
        fprintf(vconvert_file, "        long long intPart = (long long)num;\n");
        fprintf(vconvert_file, "        double fracPart = num - intPart;\n");
        fprintf(vconvert_file, "        if (fracPart < 0) fracPart = -fracPart;\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        int i = 0;\n");
        fprintf(vconvert_file, "        if (intPart == 0) {\n");
        fprintf(vconvert_file, "            str[i++] = '0';\n");
        fprintf(vconvert_file, "        } else {\n");
        fprintf(vconvert_file, "            char intStr[32];\n");
        fprintf(vconvert_file, "            int len = int_to_string(intPart, intStr);\n");
        fprintf(vconvert_file, "            for (int j = 0; j < len; j++) {\n");
        fprintf(vconvert_file, "                str[i++] = intStr[j];\n");
        fprintf(vconvert_file, "            }\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        if (precision > 0) {\n");
        fprintf(vconvert_file, "            str[i++] = '.';\n");
        fprintf(vconvert_file, "            for (int j = 0; j < precision; j++) {\n");
        fprintf(vconvert_file, "                fracPart *= 10;\n");
        fprintf(vconvert_file, "                int digit = (int)fracPart;\n");
        fprintf(vconvert_file, "                str[i++] = '0' + digit;\n");
        fprintf(vconvert_file, "                fracPart -= digit;\n");
        fprintf(vconvert_file, "            }\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        str[i] = '\\0';\n");
        fprintf(vconvert_file, "        return i;\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "}\n\n");
        fprintf(vconvert_file, "namespace vconvert {\n");
        fprintf(vconvert_file, "    inline long long to_int(const vtypes::VString& s) { \n");
        fprintf(vconvert_file, "        return std::stoll(static_cast<const std::string&>(s)); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline long long to_int(const std::string& s) { \n");
        fprintf(vconvert_file, "        return std::stoll(s); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline long long to_int(long long v) { \n");
        fprintf(vconvert_file, "        return v; \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline long long to_int(int v) { \n");
        fprintf(vconvert_file, "        return v; \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline long long to_int(double v) { \n");
        fprintf(vconvert_file, "        return static_cast<long long>(v); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    inline long long to_int_rtti(const vtypes::VString& var) {\n");
        fprintf(vconvert_file, "        return std::stoll(static_cast<const std::string&>(var));\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline long long to_int_rtti(double var) {\n");
        fprintf(vconvert_file, "        return static_cast<long long>(var);\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline long long to_int_rtti(int var) {\n");
        fprintf(vconvert_file, "        return static_cast<long long>(var);\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    template<typename T>\n");
        fprintf(vconvert_file, "    inline long long to_int_rtti(const T& var) {\n");
        fprintf(vconvert_file, "        const std::type_info& t = typeid(var);\n");
        fprintf(vconvert_file, "        return static_cast<long long>(var);\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    inline double to_double(const vtypes::VString& s) { \n");
        fprintf(vconvert_file, "        return std::stod(static_cast<const std::string&>(s)); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline double to_double(const std::string& s) { \n");
        fprintf(vconvert_file, "        return std::stod(s); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline double to_double(double v) { \n");
        fprintf(vconvert_file, "        return v; \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline double to_double(long long v) { \n");
        fprintf(vconvert_file, "        return static_cast<double>(v); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline double to_double(int v) { \n");
        fprintf(vconvert_file, "        return static_cast<double>(v); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    class Convertible {\n");
        fprintf(vconvert_file, "    public:\n");
        fprintf(vconvert_file, "        virtual ~Convertible() = default;\n");
        fprintf(vconvert_file, "        virtual long long to_int() const = 0;\n");
        fprintf(vconvert_file, "        virtual double to_double() const = 0;\n");
        fprintf(vconvert_file, "        virtual vtypes::VString to_vstring() const = 0;\n");
        fprintf(vconvert_file, "    };\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    class IntConvertible : public Convertible {\n");
        fprintf(vconvert_file, "    private:\n");
        fprintf(vconvert_file, "        long long value;\n");
        fprintf(vconvert_file, "    public:\n");
        fprintf(vconvert_file, "        explicit IntConvertible(long long v) : value(v) {}\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        long long to_int() const override {\n");
        fprintf(vconvert_file, "            return value;\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        double to_double() const override {\n");
        fprintf(vconvert_file, "            return static_cast<double>(value);\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        vtypes::VString to_vstring() const override {\n");
        fprintf(vconvert_file, "            char buffer[32];\n");
        fprintf(vconvert_file, "            detail::int_to_string(value, buffer);\n");
        fprintf(vconvert_file, "            return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "    };\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    class DoubleConvertible : public Convertible {\n");
        fprintf(vconvert_file, "    private:\n");
        fprintf(vconvert_file, "        double value;\n");
        fprintf(vconvert_file, "    public:\n");
        fprintf(vconvert_file, "        explicit DoubleConvertible(double v) : value(v) {}\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        long long to_int() const override {\n");
        fprintf(vconvert_file, "            return static_cast<long long>(value);\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        double to_double() const override {\n");
        fprintf(vconvert_file, "            return value;\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        vtypes::VString to_vstring() const override {\n");
        fprintf(vconvert_file, "            char buffer[32];\n");
        fprintf(vconvert_file, "            detail::double_to_string(value, buffer);\n");
        fprintf(vconvert_file, "            return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "    };\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    class StringConvertible : public Convertible {\n");
        fprintf(vconvert_file, "    private:\n");
        fprintf(vconvert_file, "        vtypes::VString value;\n");
        fprintf(vconvert_file, "    public:\n");
        fprintf(vconvert_file, "        explicit StringConvertible(const vtypes::VString& v) : value(v) {}\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        long long to_int() const override {\n");
        fprintf(vconvert_file, "            try {\n");
        fprintf(vconvert_file, "                return std::stoll(static_cast<const std::string&>(value));\n");
        fprintf(vconvert_file, "            } catch (...) {\n");
        fprintf(vconvert_file, "                return 0;\n");
        fprintf(vconvert_file, "            }\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        double to_double() const override {\n");
        fprintf(vconvert_file, "            try {\n");
        fprintf(vconvert_file, "                return std::stod(static_cast<const std::string&>(value));\n");
        fprintf(vconvert_file, "            } catch (...) {\n");
        fprintf(vconvert_file, "                return 0.0;\n");
        fprintf(vconvert_file, "            }\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "        \n");
        fprintf(vconvert_file, "        vtypes::VString to_vstring() const override {\n");
        fprintf(vconvert_file, "            return value;\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "    };\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(const vtypes::VString& s) { \n");
        fprintf(vconvert_file, "        return s; \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(const std::string& s) { \n");
        fprintf(vconvert_file, "        return vtypes::VString(s); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(long long v) { \n");
        fprintf(vconvert_file, "        char buffer[32];\n");
        fprintf(vconvert_file, "        detail::int_to_string(v, buffer);\n");
        fprintf(vconvert_file, "        return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(int v) { \n");
        fprintf(vconvert_file, "        char buffer[32];\n");
        fprintf(vconvert_file, "        detail::int_to_string(v, buffer);\n");
        fprintf(vconvert_file, "        return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(double v) { \n");
        fprintf(vconvert_file, "        char buffer[32];\n");
        fprintf(vconvert_file, "        detail::double_to_string(v, buffer);\n");
        fprintf(vconvert_file, "        return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(float v) { \n");
        fprintf(vconvert_file, "        char buffer[32];\n");
        fprintf(vconvert_file, "        detail::double_to_string(v, buffer);\n");
        fprintf(vconvert_file, "        return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    template<size_t N> \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(const char (&s)[N]) { \n");
        fprintf(vconvert_file, "        return vtypes::VString(s); \n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "    \n");
        fprintf(vconvert_file, "    template<typename T> \n");
        fprintf(vconvert_file, "    inline vtypes::VString to_vstring(const T& v) { /*模板函数，用于将任意类型转换为 VString 类型*/\n");
        fprintf(vconvert_file, "        if constexpr (std::is_arithmetic_v<T>) {\n");
        fprintf(vconvert_file, "            if constexpr (std::is_floating_point_v<T>) {\n");
        fprintf(vconvert_file, "                char buffer[32];\n");
        fprintf(vconvert_file, "                detail::double_to_string(v, buffer);\n");
        fprintf(vconvert_file, "                return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "            } else {\n");
        fprintf(vconvert_file, "                char buffer[32];\n");
        fprintf(vconvert_file, "                detail::int_to_string(v, buffer);\n");
        fprintf(vconvert_file, "                return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "            }\n");
        fprintf(vconvert_file, "        } else if constexpr (std::is_pointer_v<T>) {\n");
        fprintf(vconvert_file, "            char buffer[32];\n");
        fprintf(vconvert_file, "            sprintf(buffer, \"0x%%p\", static_cast<void*>(const_cast<std::remove_pointer_t<T>*>(v)));\n");
        fprintf(vconvert_file, "            return vtypes::VString(buffer);\n");
        fprintf(vconvert_file, "        } else {\n");
        fprintf(vconvert_file, "            if constexpr (std::is_same_v<T, vtypes::VString> || \n");
        fprintf(vconvert_file, "                         std::is_same_v<T, std::string> ||\n");
        fprintf(vconvert_file, "                         std::is_convertible_v<T, std::string>) {\n");
        fprintf(vconvert_file, "                return vtypes::VString(static_cast<std::string>(v));\n");
        fprintf(vconvert_file, "            } else if constexpr (std::is_array_v<T> && std::is_same_v<std::decay_t<T>, char*>) {\n");
        fprintf(vconvert_file, "                return vtypes::VString(v);\n");
        fprintf(vconvert_file, "            } else {\n");
        fprintf(vconvert_file, "                return vtypes::VString(\"unknown_type!!!\");\n");
        fprintf(vconvert_file, "            }\n");
        fprintf(vconvert_file, "        }\n");
        fprintf(vconvert_file, "    }\n");
        fprintf(vconvert_file, "}\n\n");
        fprintf(vconvert_file, "#endif\n");
        fclose(vconvert_file);
    }//ai就是好用
}