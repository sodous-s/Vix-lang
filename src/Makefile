CC = clang
CXX = clang
CFLAGS = -Wall -Wextra -std=c11 -fno-stack-protector -Werror
CXXFLAGS = -Wall -Wextra -std=c++17 -fno-stack-protector -lstdc++
CPPFLAGS = -I../include -D_POSIX_C_SOURCE=200809L
BISON = bison
FLEX = flex
LLVM_CONFIG = llvm-config
LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS = $(shell $(LLVM_CONFIG) --libs)
TARGET = vixc
AST_SRC = ast/ast.c ast/type_inference.c
SEMANTIC_SRC = semantic/semantic.c
PARSER_SRC = parser/parser.tab.c parser/lex.yy.c
IR_SRC = vic-ir/mir.c
LLVM_SRC = compiler/backend-llvm/LlvmEmit.cpp
UTILS_SRC = utils/error.c
C_SRC = main.c $(AST_SRC) $(SEMANTIC_SRC) $(PARSER_SRC) $(IR_SRC) $(OPT_SRC) $(UTILS_SRC)
CXX_SRC = $(LLVM_SRC)
C_OBJ = $(C_SRC:.c=.o)
CXX_OBJ = $(CXX_SRC:.cpp=.o)
OBJ = $(C_OBJ) $(CXX_OBJ)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LLVM_LDFLAGS) -o $@ $^ $(LLVM_LIBS) -lm

parser/parser.tab.c parser/parser.tab.h: parser/parser.y
	cd parser && $(BISON) -d parser.y

parser/lex.yy.c: parser/lexer.l
	cd parser && $(FLEX) lexer.l

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LLVM_CFLAGS) $(CPPFLAGS) -c $< -o $@

main.o: main.c ../include/ast.h ../include/parser.h ../include/compiler.h ../include/vic-ir/mir.h ../include/semantic.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ast/ast.o: ast/ast.c ../include/ast.h parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ast/type_inference.o: ast/type_inference.c ../include/type_inference.h ../include/bytecode.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

semantic/semantic.o: semantic/semantic.c ../include/semantic.h ../include/type_inference.h parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

utils/error.o: utils/error.c ../include/compiler.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

compiler/backend-llvm/LlvmEmit.o: compiler/backend-llvm/LlvmEmit.cpp ../include/llvm_emit.h
	$(CXX) $(CXXFLAGS) $(LLVM_CFLAGS) $(CPPFLAGS) -c $< -o $@

vic-ir/mir.o: vic-ir/mir.c ../include/vic-ir/mir.h ../include/bytecode.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

parser/parser.tab.o: parser/parser.tab.c parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

parser/lex.yy.o: parser/lex.yy.c parser/parser.tab.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(C_OBJ) $(CXX_OBJ)
	rm -f parser/parser.tab.c parser/parser.tab.h parser/lex.yy.c

.PHONY: all clean install uninstall