#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "../include/compiler.h"
// typedef enum {
//     ERROR_LEVEL_WARNING,
//     ERROR_LEVEL_ERROR,
//     ERROR_LEVEL_FATAL
// } ErrorLevel;
static const char* current_filename = "unknown";
static int current_line = 1;
static int error_count = 0;
static char* source_content = NULL;
static void vreport_error(ErrorLevel level, ErrorType error_type, const char* format, va_list args);
static int warning_count = 0;
static char* get_line_content(int line_number);
static void show_error_context_with_column(int line_number, int column);
static int current_column = 1;
static void show_error_context_with_column_and_length(int line_number, int column, int length);

/* ANSI styles for richer diagnostics. */
#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_DIM "\033[2m"
#define ANSI_RED "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"
#define ANSI_BG_RED "\033[41m"
#define ANSI_BG_YELLOW "\033[43m"

static const char* error_type_to_string(ErrorType error_type);
static const char* error_type_color(ErrorType error_type);
static const char* get_suggestion(ErrorType error_type);
static int line_number_width(int line_number);
static void print_suggestion_block(const char* suggestion);
static void print_diagnostic_header(ErrorLevel level, ErrorType error_type, const char* message);
void set_location(const char* filename, int line) {
    current_filename = filename ? filename : "unknown";
    current_line = line;
    current_column = 1;
}

void set_location_with_column(const char* filename, int line, int column) {
    current_filename = filename ? filename : "unknown";
    current_line = line;
    current_column = column > 0 ? column : 1;
}
void load_source_file(const char* filename) {
    if (!filename) {
        printf("error:cannot open file\n");
        return;
    }

    FILE* file = fopen(filename, "r");
    if (!file) return;
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    source_content = (char*)malloc(file_size + 1);
    if (source_content) {
        fread(source_content, 1, file_size, file);
        source_content[file_size] = '\0';
    }
    
    fclose(file);
}

static char* get_line_content(int line_number) {
    if (!source_content || line_number <= 0) return NULL;
    
    char* start = source_content;
    int current_line = 1;
    while (current_line < line_number && *start) {
        if (*start == '\n') {
            current_line++;
            start++;
        } else {
            start++;
        }
    }
    
    if (current_line != line_number) 
    {
        return NULL;
    }
    char* end = start;
    while (*end && *end != '\n') {
        end++;
    }
    
    int line_len = end - start;
    char* line_content = (char*)malloc(line_len + 1);
    if (line_content) {
        strncpy(line_content, start, line_len);
        line_content[line_len] = '\0';
        if (line_content[line_len - 1] == '\r') {
            line_content[line_len - 1] = '\0';
        }
    }
    
    return line_content;
}
static void show_error_context_with_column(int line_number, int column) {
    show_error_context_with_column_and_length(line_number, column, 1);
}
static void show_error_context_with_column_and_length(int line_number, int column, int length) {
    if (!source_content) return;

    int width = line_number_width(line_number + 1);
    char* prev_line = get_line_content(line_number - 1);
    char* line_content = get_line_content(line_number);
    char* next_line = get_line_content(line_number + 1);

    if (!line_content) {
        free(prev_line);
        free(next_line);
        return;
    }

    fprintf(stderr, "%s%*s |%s\n", ANSI_DIM, width, "", ANSI_RESET);
    if (prev_line) {
        fprintf(stderr, "%s%*d | %s%s\n", ANSI_DIM, width, line_number - 1, prev_line, ANSI_RESET);
    }
    fprintf(stderr, "%s%*d |%s %s%s%s\n", ANSI_BOLD ANSI_BLUE, width, line_number, ANSI_RESET, ANSI_WHITE, line_content, ANSI_RESET);
    if (next_line) {
        fprintf(stderr, "%s%*d | %s%s\n", ANSI_DIM, width, line_number + 1, next_line, ANSI_RESET);
    }

    fprintf(stderr, "%s%*s |%s ", ANSI_DIM, width, "", ANSI_RESET);
    for (int i = 0; i < column - 1 && line_content[i] != '\0'; i++) {
        if (line_content[i] == '\t') {
            fputc('\t', stderr);
        } else {
            fputc(' ', stderr);
        }
    }

    int caret_count = length > 0 ? length : 1;
    for (int i = 0; i < caret_count; i++) {
        fputc('^', stderr);
    }
    fprintf(stderr, " %s<-- column %d%s\n", ANSI_BOLD ANSI_CYAN, column, ANSI_RESET);
    fprintf(stderr, "%s%*s |%s\n", ANSI_DIM, width, "", ANSI_RESET);

    free(prev_line);
    free(line_content);
    free(next_line);
}

static void show_error_context(int line_number) {
    show_error_context_with_column_and_length(line_number, 1, 0);
}

static const char* error_type_to_string(ErrorType error_type) {
    switch (error_type) {
        case ERROR_SYNTAX:
            return "SyntaxError";
        case ERROR_LEXICAL:
            return "LexicalError";
        case ERROR_TYPE:
            return "TypeError";
        case ERROR_UNDEFINED:
        case ERROR_UNDEFINED_FUNC:
            return "NameError";
        case ERROR_REDEFINITION:
            return "RedefinitionError";
        case ERROR_SEMANTIC:
            return "SemanticError";
        case ERROR_RUNTIME:
            return "RuntimeError";
        case ERROR_WARNING:
            return "Warning";
        case ERROR_ARRAY_OUT_OF_BOUNDS:
            return "BoundsError";
        default:
            return "Error";
    }
}

static const char* error_type_color(ErrorType error_type) {
    switch (error_type) {
        case ERROR_SYNTAX:
        case ERROR_LEXICAL:
            return ANSI_MAGENTA;
        case ERROR_TYPE:
        case ERROR_SEMANTIC:
            return ANSI_BLUE;
        case ERROR_UNDEFINED:
        case ERROR_UNDEFINED_FUNC:
        case ERROR_REDEFINITION:
            return ANSI_CYAN;
        case ERROR_RUNTIME:
        case ERROR_ARRAY_OUT_OF_BOUNDS:
            return ANSI_RED;
        case ERROR_WARNING:
            return ANSI_YELLOW;
        default:
            return ANSI_WHITE;
    }
}

static const char* get_suggestion(ErrorType error_type) {
    switch (error_type) {
        case ERROR_UNDEFINED:
            return "Declare the identifier before use or check for typos";
        case ERROR_UNDEFINED_FUNC:
            return "Ensure the function is declared and linked, or check its name";
        case ERROR_REDEFINITION:
            return "Remove or rename the previous declaration to avoid conflicts";
        case ERROR_TYPE:
            return "Check types or add an explicit cast/convert the expression";
        case ERROR_SYNTAX:
            return "Check syntax near the indicated location (missing token/parenthesis)";
        case ERROR_LEXICAL:
            return "Remove unsupported characters or fix the token spelling";
        case ERROR_RUNTIME:
            return "Verify runtime conditions (null/overflow) or add guards";
        case ERROR_ARRAY_OUT_OF_BOUNDS:
            return "Check index range before accessing the array";
        default:
            return NULL;
    }
}

static int line_number_width(int line_number) {
    int width = 1;
    int n = line_number > 0 ? line_number : 1;
    while (n >= 10) {
        n /= 10;
        width++;
    }
    return width;
}

static void print_suggestion_block(const char* suggestion) {
    if (!suggestion) return;
    fprintf(stderr, "%sHint%s %s%s%s\n", ANSI_BOLD ANSI_CYAN, ANSI_RESET, ANSI_CYAN, suggestion, ANSI_RESET);
}

static void print_diagnostic_header(ErrorLevel level, ErrorType error_type, const char* message) {
    const char* level_str = (level == ERROR_LEVEL_WARNING) ? "WARNING" : "ERROR";
    const char* level_bg = (level == ERROR_LEVEL_WARNING) ? ANSI_BG_YELLOW : ANSI_BG_RED;
    const char* type_str = error_type_to_string(error_type);
    const char* type_color = error_type_color(error_type);

    fprintf(stderr,
            "%s%s %s %s%s%s %s%s%s\n",
            level_bg,
            ANSI_BOLD,
            level_str,
            ANSI_RESET,
            type_color,
            type_str,
            ANSI_RESET,
            ANSI_BOLD,
            ANSI_RESET);
    fprintf(stderr,
            "%s-->%s %s:%d:%d\n",
            ANSI_BOLD ANSI_BLUE,
            ANSI_RESET,
            current_filename,
            current_line,
            current_column);
    fprintf(stderr,
            "%sMessage:%s %s\n",
            ANSI_BOLD,
            ANSI_RESET,
            message ? message : "");
}
int get_error_count() {
    return error_count;
}
int get_warning_count() {
    return warning_count;
}
static void vreport_error(ErrorLevel level, ErrorType error_type, const char* format, va_list args) {
    char message[1024];
    switch (level) {
        case ERROR_LEVEL_WARNING:
            warning_count++;
            break;
        case ERROR_LEVEL_ERROR:
        case ERROR_LEVEL_FATAL:
            error_count++;
            break;
    }

    vsnprintf(message, sizeof(message), format, args);
    if (error_type == ERROR_SYNTAX) {
        strncat(message, ". Check if all parentheses are properly closed", sizeof(message) - strlen(message) - 1);
    }

    print_diagnostic_header(level, error_type, message);

    /* Show source context (line and caret) when available */
    if (source_content && current_line > 0) {
        if (current_column > 0) {
            show_error_context_with_column(current_line, current_column);
        } else {
            show_error_context(current_line);
        }
    }

    print_suggestion_block(get_suggestion(error_type));
}

void report_simple_error_with_length(ErrorLevel level, ErrorType error_type, const char* msg, int length) {
    switch (level) {
        case ERROR_LEVEL_WARNING:
            warning_count++;
            break;
        case ERROR_LEVEL_ERROR:
        case ERROR_LEVEL_FATAL:
            error_count++;
            break;
    }

    print_diagnostic_header(level, error_type, msg);
    if (source_content && current_line > 0) {
        if (current_column > 0) {
            show_error_context_with_column_and_length(current_line, current_column, length);
        } else {
            show_error_context(current_line);
        }
    }

    print_suggestion_block(get_suggestion(error_type));
}
void report_simple_error(ErrorLevel level, ErrorType error_type, const char* msg) {
    report_simple_error_with_length(level, error_type, msg, 1);
}

void report_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vreport_error(ERROR_LEVEL_WARNING, ERROR_WARNING, format, args);
    va_end(args);
}
void report_unused_variable_warning(const char* variable_name, const char* filename, int line) {
    if (variable_name && filename && line > 0) {
        set_location(filename, line);
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "unused variable '%s'", variable_name);
        report_simple_error(ERROR_LEVEL_WARNING, ERROR_WARNING, buffer);
    }
}
void report_unused_variable_warning_with_location(const char* variable_name, const char* filename, int line, int column) {
    if (variable_name && filename && line > 0) {
        const char* old_filename = current_filename;
        int old_line = current_line;
        int old_column = current_column;
        
        current_filename = filename;
        current_line = line;
        current_column = column;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "unused variable '%s'", variable_name);
        report_simple_error_with_length(ERROR_LEVEL_WARNING, ERROR_WARNING, buffer, variable_name ? strlen(variable_name) : 1);
        
        current_filename = old_filename;
        current_line = old_line;
        current_column = old_column;
    }
}


void report_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vreport_error(ERROR_LEVEL_ERROR, ERROR_SEMANTIC, format, args);
    va_end(args);
}
void report_fatal_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vreport_error(ERROR_LEVEL_FATAL, ERROR_SEMANTIC, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void internal_error(const char* format, ...) {
    va_list args;
    char message[1024];
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    fprintf(stderr, "%s%s INTERNAL%s\n", ANSI_BG_RED, ANSI_BOLD ANSI_WHITE, ANSI_RESET);
    fprintf(stderr, "%s-->%s %s:%d:%d\n", ANSI_BOLD ANSI_BLUE, ANSI_RESET, current_filename, current_line, current_column);
    fprintf(stderr, "%sMessage:%s %s\n", ANSI_BOLD, ANSI_RESET, message);

    if (source_content && current_line > 0) {
        if (current_column > 0) {
            show_error_context_with_column(current_line, current_column);
        } else {
            show_error_context(current_line);
        }
    }
    va_end(args);
    exit(EXIT_FAILURE);
}

void report_lexical_error_with_location(const char* message, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_LEXICAL, message);
    
    current_filename = old_filename;
    current_line = old_line;
}

void report_syntax_error_with_location(const char* token, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    char buffer[256];
    if (token) {
        snprintf(buffer, sizeof(buffer), "unexpected token '%s'", token);
    } else {
        snprintf(buffer, sizeof(buffer), "unexpected token");
    }
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_SYNTAX, buffer);
    
    current_filename = old_filename;
    current_line = old_line;
}
void report_undefined_identifier_with_location(const char* identifier, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "undefined identifier: '%s'", identifier);
    report_simple_error_with_length(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, buffer, identifier ? strlen(identifier) : 1);
    
    current_filename = old_filename;
    current_line = old_line;
}
void report_undefined_identifier_with_location_and_column(const char* identifier, const char* filename, int line, int column) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    int old_column = current_column;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    current_column = column;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "undefined identifier: '%s'", identifier);
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, buffer);
    
    current_filename = old_filename;
    current_line = old_line;
    current_column = old_column;
}

void report_undefined_function_with_location_and_column(const char* function_name, const char* filename, int line, int column) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    int old_column = current_column;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    current_column = column;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "call undefined function: '%s'", function_name);
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_UNDEFINED_FUNC, buffer);
    
    current_filename = old_filename;
    current_line = old_line;
    current_column = old_column;
}

void report_undefined_variable_with_location(const char* variable_name, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    if (variable_name) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "undefined variable: '%s'", variable_name);
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, buffer);
    } else {
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, "undefined variable");
    }
    
    current_filename = old_filename;
    current_line = old_line;
}

void report_redefinition_error_with_location(const char* identifier, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "redefinition of '%s'", identifier);
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_REDEFINITION, buffer);
    
    current_filename = old_filename;
    current_line = old_line;
}

void report_type_error_with_location(const char* expected, const char* actual, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "expected %s but got %s", expected, actual);
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_TYPE, buffer);
    
    current_filename = old_filename;
    current_line = old_line;
}

void report_semantic_error_with_location(const char* message, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_SEMANTIC, message);
    
    current_filename = old_filename;
    current_line = old_line;
}

void report_array_out_of_bounds_error_with_location(const char* array_name, int index, int size, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    char buffer[512];
    if (array_name) {
        snprintf(buffer, sizeof(buffer), 
            "Array index out of bounds: accessing index %d in array '%s' of size %d.\n"
            "Valid indices are from 0 to %d!", 
            index, array_name, size, size - 1);
    } else {
        snprintf(buffer, sizeof(buffer), 
            "Array index out of bounds: index %d.\n"
            "Check that the index is within the valid range!", 
            index);
    }
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_ARRAY_OUT_OF_BOUNDS, buffer);
    current_filename = old_filename;
    current_line = old_line;
}

void report_runtime_error_with_location(const char* message, const char* filename, int line) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    
    current_filename = filename ? filename : "unknown";
    current_line = line;
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_RUNTIME, message);
    
    current_filename = old_filename;
    current_line = old_line;
}
void report_syntax_error(const char* token) {
    char buffer[256];
    if (token) {
        snprintf(buffer, sizeof(buffer), "unexpected token '%s'", token);
    } else {
        snprintf(buffer, sizeof(buffer), "unexpected token");
    }
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_SYNTAX, buffer);
}
void report_lexical_error(const char* message) {
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_LEXICAL, message);
}

void report_type_error(const char* expected, const char* actual) {
    if (expected && actual) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "expected %s but got %s", expected, actual);
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_TYPE, buffer);
    } else {
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_TYPE, "type error");
    }
}
void report_undefined_identifier(const char* identifier) {
    if (identifier) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "undefined identifier: '%s'", identifier);
        report_simple_error_with_length(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, buffer, strlen(identifier));
    } else {
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, "undefined identifier");
    }
}
void report_undefined_function(const char* function_name) {
    if (function_name) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "call to undefined function: '%s'", function_name);
        report_simple_error_with_length(ERROR_LEVEL_ERROR, ERROR_UNDEFINED_FUNC, buffer, strlen(function_name));
    } else {
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_UNDEFINED_FUNC, "call to undefined function");
    }
}
void report_undefined_variable(const char* variable_name) {
    if (variable_name) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "undefined variable: '%s'", variable_name);
        report_simple_error_with_length(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, buffer, strlen(variable_name));
    } else {
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, "undefined variable");
    }
}
void report_redefinition_error(const char* identifier) {
    if (identifier) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "redefinition of '%s'", identifier);
        report_simple_error_with_length(ERROR_LEVEL_ERROR, ERROR_REDEFINITION, buffer, strlen(identifier));
    } else {
        report_simple_error(ERROR_LEVEL_ERROR, ERROR_REDEFINITION, "redefinition of identifier");
    }
}

void report_mismatched_parentheses() {
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_SYNTAX, "mismatched parentheses");
}

void print_error_summary() {
    if (error_count > 0 || warning_count > 0) {
        fprintf(stderr, "\nCompilation finished with ");
        if (error_count > 0) {
            fprintf(stderr, "%d error(s) ", error_count);
            if (warning_count > 0) {
                fprintf(stderr, "and ");
            }
        }
        if (warning_count > 0) {
            fprintf(stderr, "%d warning(s)", warning_count);
        }
        fprintf(stderr, ".\n");
    }
}

void cleanup_error_handler() {
    if (source_content) {
        free(source_content);
        source_content = NULL;
    }
}

void report_struct_field_missing_with_location_and_suggestion(const char* struct_name, const char* field_name, const char* suggestion, const char* filename, int line, int column) {
    const char* old_filename = current_filename;
    int old_line = current_line;
    int old_column = current_column;

    current_filename = filename ? filename : "unknown";
    current_line = line;
    current_column = column;

    char buffer[512];
    if (struct_name && field_name) {
        snprintf(buffer, sizeof(buffer), "struct '%s' has no field: '%s'", struct_name, field_name);
    } else if (field_name) {
        snprintf(buffer, sizeof(buffer), "struct has no field: '%s'", field_name);
    } else {
        snprintf(buffer, sizeof(buffer), "struct has no such field");
    }

    report_simple_error_with_length(ERROR_LEVEL_ERROR, ERROR_UNDEFINED, buffer, field_name ? (int)strlen(field_name) : 1);

    if (suggestion) {
        fprintf(stderr, "%sHint%s did you mean '%s'?\n", ANSI_BOLD ANSI_CYAN, ANSI_RESET, suggestion);
    }

    current_filename = old_filename;
    current_line = old_line;
    current_column = old_column;
}