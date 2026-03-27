# 编译器使用

本文档详细介绍 Vix 编译器（vixc）的使用方法和各种选项。

## 目录

- [基本用法](#基本用法)
- [命令行选项](#命令行选项)
- [编译后端](#编译后端)
- [优化选项](#优化选项)
- [输出格式](#输出格式)
- [项目初始化](#项目初始化)
- [常见用法](#常见用法)
- [故障排除](#故障排除)

---

## 基本用法

### 基本命令格式

```shell
vixc [选项] <源文件>
```

### 快速编译

```shell
# 编译单个文件
vixc main.vix -o main

# 运行程序
./main          # Linux/macOS
main.exe        # Windows
```

### 查看版本

```shell
vixc -v
```

---

## 命令行选项

### 输出选项

| 选项 | 参数 | 描述 |
|------|------|------|
| `-o` | `<文件名>` | 指定输出可执行文件名 |
| `-q` | `<文件名>` | 输出 QBE IR 代码 |
| `-ll` | `<文件名>` | 输出 LLVM IR 代码 |

### 编译选项

| 选项 | 描述 |
|------|------|
| `--backend=<后端>` | 指定编译后端 |
| `-opt` | 启用代码优化 |
| `-kt` | 保留中间 C++ 文件 |

### 其他选项

| 选项 | 描述 |
|------|------|
| `init` | 初始化新项目 |
| `-v` | 显示版本信息 |

---

## 编译后端

Vix 支持多种编译后端。

### LLVM 后端（推荐）

LLVM 后端是最完善的后端，提供最好的优化能力：

```shell
vixc main.vix -o main --backend=llvm
```

**特点：**
- 最成熟的优化
- 支持最多平台
- 生成的代码性能最佳

### QBE 后端

QBE 是一个轻量级后端：

```shell
vixc main.vix -o main --backend=qbe
```

**特点：**
- 编译速度快
- 代码体积小
- 适合快速开发

> **注意**：QBE 后端计划在未来版本中移除。

### C++ 后端

通过生成 C++ 代码间接编译：

```shell
vixc main.vix -o main --backend=cpp
```

**特点：**
- 方便调试
- 可以查看生成的 C++ 代码
- 兼容性好

> **注意**：C++ 后端计划在未来版本中移除。

### 后端性能对比

Fibonacci 40 测试结果：

| 后端 | 执行时间 | 相对性能 |
|------|----------|----------|
| C++ (O2) | 0.132s | 1.0x (基准) |
| LLVM | 0.402s | 0.33x |
| QBE | 0.577s | 0.23x |

---

## 优化选项

### 启用优化

使用 `-opt` 标志启用代码优化：

```shell
vixc main.vix -o main -opt
```

### 优化效果

优化可以显著提高运行时性能：

```shell
# 未优化
vixc fib.vix -o fib_no_opt

# 优化后
vixc fib.vix -o fib_opt -opt
```

---

## 输出格式

### 可执行文件

默认输出可执行文件：

```shell
vixc main.vix -o main
```

### LLVM IR

生成 LLVM 中间表示：

```shell
vixc main.vix -ll output.ll
```

这会生成人类可读的 LLVM IR 文件，可用于：
- 调试编译器
- 分析优化效果
- 学习编译原理

### QBE IR

生成 QBE 中间表示：

```shell
vixc main.vix -q output.qbe
```

### 保留中间文件

保留编译过程中生成的 C++ 文件：

```shell
vixc main.vix -o main -kt
```

这会在同目录下保留 `.cpp` 文件，便于调试。

---

## 项目初始化

### 创建新项目

```shell
vixc init
```

这会创建基本的项目结构：

```
project/
├── main.vix
└── config.json
```

### 生成的文件

**main.vix** - 主程序入口：

```vix
fn main() -> i32 {
    print("Hello, Vix!")
    return 0
}
```

---

## 常见用法

### 编译单个文件

```shell
# 最简单的方式
vixc hello.vix -o hello

# 指定后端和优化
vixc hello.vix -o hello --backend=llvm -opt
```

### 编译多文件项目

```shell
# 主文件导入其他文件
# main.vix:
// import "utils.vix"
// import "math.vix"

vixc main.vix -o myapp
```

### 调试编译

```shell
# 生成 LLVM IR 用于调试
vixc debug.vix -ll debug.ll

# 保留 C++ 文件
vixc debug.vix -o debug -kt
```

### 性能测试编译

```shell
# 使用 LLVM 后端和优化
vixc benchmark.vix -o benchmark --backend=llvm -opt

# 运行测试
time ./benchmark
```

### 交叉编译

Vix 支持多平台编译：

```shell
# 在 Linux 上编译
vixc app.vix -o app_linux

# 在 Windows 上编译
vixc app.vix -o app.exe

# 在 macOS 上编译
vixc app.vix -o app_mac
```

---

## 故障排除

### 编译错误

#### 语法错误

```
Error: unexpected token at line 10
```

检查源文件语法，确保：
- 括号匹配
- 语句以正确方式结束
- 关键字拼写正确

#### 类型错误

```
Error: type mismatch
```

检查：
- 变量类型是否正确
- 函数参数类型是否匹配
- 返回值类型是否正确

#### 未定义的符号

```
Error: undefined symbol 'foo'
```

检查：
- 函数是否已定义
- 导入语句是否正确
- 拼写是否正确

### 后端错误

#### LLVM 未安装

```
Error: LLVM backend not available
```

解决方案：
1. 安装 LLVM
2. 设置环境变量

```shell
# Linux
export LLVM_HOME=/usr/lib/llvm-14

# macOS
export LLVM_HOME=/usr/local/opt/llvm
```

#### QBE 未找到

```
Error: QBE not found
```

解决方案：
1. 安装 QBE
2. 确保 qbe 在 PATH 中

### 链接错误

#### 未定义的引用

```
Error: undefined reference to 'printf'
```

确保正确声明了外部函数：

```vix
extern "C" {
    fn printf(format: ptr, ...) -> i32
}
```

### 运行时错误

#### 段错误

```
Segmentation fault
```

检查：
- 空指针访问
- 数组越界
- 未初始化的变量

#### 内存泄漏

使用 valgrind 检查：

```shell
valgrind ./myprogram
```

---

## 编译流程

了解 Vix 的编译流程有助于调试：

```
源代码 (.vix)
    ↓
词法分析 (Lexer)
    ↓
语法分析 (Parser) → AST
    ↓
语义分析 (Semantic)
    ↓
类型推断 (Type Inference)
    ↓
中间代码生成
    ↓
后端编译
    ↓
可执行文件
```

### 查看中间产物

```shell
# 查看 LLVM IR
vixc main.vix -ll main.ll
cat main.ll

# 查看 QBE IR
vixc main.vix -q main.qbe
cat main.qbe
```

---

## 最佳实践

### 1. 使用正确的后端

- **生产环境**：使用 LLVM 后端 + 优化
- **开发调试**：使用默认设置
- **快速原型**：可以使用 QBE 后端

### 2. 合理组织项目

```
project/
├── src/
│   ├── main.vix
│   └── modules/
├── build/
└── Makefile
```

### 3. 使用 Makefile 自动化

```makefile
# Makefile
CC = vixc
CFLAGS = --backend=llvm -opt

all: build

build:
	$(CC) src/main.vix -o build/app $(CFLAGS)

clean:
	rm -f build/*

run: build
	./build/app
```

### 4. 版本控制

在 `.gitignore` 中忽略编译产物：

```
# .gitignore
build/
*.ll
*.qbe
*.cpp
*.o
```

---

## 下一步

- [快速入门](getting-started.md) - 编写第一个程序
- [语法参考](syntax.md) - 语言语法
- [标准库](stdlib.md) - 内置功能
