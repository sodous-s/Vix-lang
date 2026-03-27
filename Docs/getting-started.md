# 快速入门

本指南将帮助你快速上手 Vix 编程语言。我们将从安装开始，一步步带你完成第一个 Vix 程序。

## 安装 Vix

### 从源码编译

#### 系统要求

- **操作系统**：Windows、Linux 或 macOS
- **依赖项**：
  - LLVM（推荐版本 14+）
  - Make 构建工具
  - C 编译器（GCC 或 Clang）

#### 编译步骤

1. **克隆仓库**

```shell
git clone https://github.com/vix-lang/vix.git
cd vix
```

2. **进入源码目录并编译**

```shell
cd src
make
```

3. **验证安装**

```shell
vixc -v
```

如果看到版本信息，说明安装成功！

### 安装位置

编译完成后，可执行文件 `vixc` 位于 `src` 目录下。你可以将其添加到系统 PATH 中以便全局使用：

```shell
# Linux/macOS
export PATH=$PATH:/path/to/vix/src

# Windows (PowerShell)
$env:Path += ";C:\path\to\vix\src"
```

## 第一个程序

### Hello World

创建一个名为 `hello.vix` 的文件：

```vix
fn main() -> i32 {
    print("Hello, Vix!")
    return 0
}
```

### 编译和运行

```shell
# 编译
vixc hello.vix -o hello

# 运行
./hello        # Linux/macOS
hello.exe      # Windows
```

输出：
```
Hello, Vix!
```

## 基础语法速览

### 变量声明

```vix
// 不可变变量（使用 let）
let a = "Hello"
let b = 123
let c = 3.14

// 可变变量（使用 mut）
mut x = 10
x = 20  // 可以修改

// 带类型标注
let a: i32 = 10
let b: f64 = 3.14
let c: string = "Hello"
```

### 函数定义

```vix
// 基本函数
fn add(a: i32, b: i32) -> i32 {
    return a + b
}

// 主函数
fn main() -> i32 {
    let result = add(3, 5)
    print(result)
    return 0
}
```

### 控制流

```vix
// if 语句
if (x > 10) {
    print("x is greater than 10")
} elif (x > 5) {
    print("x is greater than 5")
} else {
    print("x is 5 or less")
}

// while 循环
let i = 0
while (i < 5) {
    print(i)
    i += 1
}

// for 循环
for (i in 1 .. 10) {
    print(i)
}
```

### 数组操作

```vix
// 固定大小数组
let arr: [i32 * 5] = [1, 2, 3, 4, 5]
print(arr[0])       // 1
print(arr.length)   // 5

// 动态列表
let list = [1, 2, 3]
list.push!(4)       // 添加元素
let last = list.pop() // 弹出最后一个元素
```

### 结构体

```vix
struct Person {
    name: string,
    age: i32
}

fn main() -> i32 {
    let p = Person { name: "Alice", age: 25 }
    print(p.name)
    print(p.age)
    return 0
}
```

## 编译器选项

### 常用命令

```shell
# 编译为可执行文件
vixc source.vix -o output

# 指定 LLVM 后端
vixc source.vix -o output --backend=llvm

# 生成 LLVM IR
vixc source.vix -ll output_ir

# 启用优化
vixc source.vix -o output -opt
```

### 初始化项目

```shell
vixc init
```

这将创建基本的项目结构和配置文件。

## 下一步

现在你已经掌握了 Vix 的基础知识，可以继续学习：

- [语言语法参考](syntax.md) - 完整的语法文档
- [类型系统](types.md) - 了解 Vix 的类型系统
- [函数详解](functions.md) - 深入了解函数特性
- [标准库](stdlib.md) - 探索内置功能

## 自举编译器

Vix 正在实现自举！`vixc0` 是用 Vix 编写的 Vix 编译器：

```shell
# 编译自举编译器
cd vixc0
vixc main.vix -o vixc0

# 使用 vixc0 编译代码
./vixc0 --emit-ll hello.vix hello.ll
```

了解更多：[什么是 Vix 语言](what-is-vix.md#自举编译器-vixc0)

## 示例程序

### 斐波那契数列

```vix
fn fib(n: i32) -> i64 {
    let a = 0
    let b = 1
    for (i in 1 .. n) {
        let c = a + b
        a = b
        b = c
    }
    return b
}

fn main() -> i32 {
    print(fib(40))
    return 0
}
```

### 命令行参数

```vix
fn main(argc: i32, argv: ptr) -> i32 {
    for (i in 0 .. argc) {
        print(argv[i])
    }
    return 0
}
```

### 文件操作

```vix
import "std/io.vix"

fn main() -> i32 {
    let file = fopen("test.txt", "w")
    if (file == nil) {
        puts("Failed to open file")
        return 1
    }
    let data = "Hello, Vix!"
    fwrite(data, 1, data.length, file)
    fclose(file)
    return 0
}
```

## 常见问题

### 编译时找不到 LLVM

确保已安装 LLVM 并设置正确的环境变量：

```shell
# Linux
export LLVM_HOME=/usr/lib/llvm-14

# macOS
export LLVM_HOME=/usr/local/opt/llvm
```

### 运行时找不到共享库

如果遇到运行时库加载问题，确保将库路径添加到系统搜索路径：

```shell
# Linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/libs

# macOS
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/path/to/libs
```

## 获取帮助

- **文档**：查阅本站点上的其他文档
- **示例**：查看项目的 `examples` 目录
- **社区**：加入 QQ 群 130577506
- **Issues**：在 GitHub 上提交问题

---

> 准备好了吗？让我们开始编写 Vix 程序！
